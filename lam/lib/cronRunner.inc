<?php

namespace LAM\Cron;


/*

 This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
 Copyright (C) 2024  Roland Gruber

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

use Exception;
use LAM\DB\CronDatabase;
use LAM\JOB\JobResultLog;
use LAMCfgMain;
use LAMConfig;
use LAMException;
use Ldap;
use PDO;
use PDOException;
use ServerProfilePersistenceManager;

/**
 * Cron jobs for LAM.
 *
 * @package jobs
 * @author Roland Gruber
 */

/** access to configuration data */
include_once(__DIR__ . "/config.inc");
/** access to ldap data */
include_once(__DIR__ . "/ldap.inc");
/** access to modules */
include_once(__DIR__ . "/modules.inc");
/** Access to database */
include_once(__DIR__ . "/database.inc");

/**
 * Executes cron jobs.
 */
class CronRunner {

	private bool $isDryRun;

	/**
	 * Constructor
	 *
	 * @param bool $isDryRun dry-run
	 */
	public function __construct(bool $isDryRun) {
		$this->isDryRun = $isDryRun;
		if ($isDryRun) {
			echo "Dry run: no changes, only logging.\n";
		}
		$_SESSION['cfgMain'] = new LAMCfgMain();
		// set certificates variables
		setSSLCaCert();
	}

	/**
	 * Runs all global cron actions.
	 * @throws LAMException error running actions
	 */
	public function runGlobalCronJobs(): void {
		if ($this->isDryRun) {
			echo "Global actions started\n\n";
		}
		$errorsOccurred = false;
		$allModules = getAllModules();
		foreach ($allModules as $module) {
			if (!$module->supportsGlobalCronJob()) {
				continue;
			}
			try {
				if ($this->isDryRun) {
					echo "Started actions for " . $module->get_alias() . "\n";
				}
				logNewMessage(LOG_NOTICE, 'Started actions for ' . $module->get_alias());
				$module->runGlobalCronActions($this->isDryRun);
				if ($this->isDryRun) {
					echo "Finished actions for " . $module->get_alias() . "\n";
				}
				logNewMessage(LOG_NOTICE, 'Finished actions for ' . $module->get_alias());
			}
			catch (Exception $e) {
				$errorsOccurred = true;
				echo "Error in " . $module->get_alias() . ': ' . $e->getMessage() . "\n" . $e->getTraceAsString() . "\n";
			}
		}
		if ($errorsOccurred) {
			throw new LAMException("Problems occurred during cron execution");
		}
		if ($this->isDryRun) {
			echo "\nGlobal actions done\n\n";
		}
	}

	/**
	 * Runs the actions of a single server profile.
	 *
	 * @param string $serverProfileName profile name
	 * @throws LAMException error running actions
	 */
	public function runSingleServerProfileActions(string $serverProfileName): void {
		$serverProfilePersistenceManager = new ServerProfilePersistenceManager();
		$this->validateServerProfileName($serverProfileName, $serverProfilePersistenceManager);
		if ($this->isDryRun) {
			echo "Server profile started: " . $serverProfileName . "\n\n";
		}

		try {
			$conf = $serverProfilePersistenceManager->loadProfile($serverProfileName);
			$_SESSION['config'] = $conf;
		}
		catch (LAMException $e) {
			echo $e->getTitle();
			exit(1);
		}

		// test database connection
		try {
			$cronDatabase = new CronDatabase($conf);
			$cronDatabase->connect();
			$pdo = $cronDatabase->getPdo();
		}
		catch (PDOException $e) {
			throw new LAMException("Unable to connect to database. " . $e->getMessage());
		}
		catch (LAMException $e) {
			throw new LAMException("Unable to connect to database. " . $e->getTitle());
		}
		// get jobs to run
		if (empty($cronDatabase->getJobs())) {
			if ($this->isDryRun) {
				echo "No jobs configured for " . $serverProfileName . "\n\n";
			}
			return;
		}
		$this->openLdapConnection();
		if ($this->isDryRun) {
			echo "\nServer profile done: " . $serverProfileName . "\n\n";
		}
		$this->runServerProfileJobs($conf, $cronDatabase, $pdo, $serverProfileName);
	}

	/**
	 * Validates the server profile.
	 *
	 * @param string $serverProfileName profile name
	 * @param ServerProfilePersistenceManager $serverProfilePersistenceManager persistence manager
	 * @throws LAMException invalid profile
	 */
	private function validateServerProfileName(string $serverProfileName, ServerProfilePersistenceManager $serverProfilePersistenceManager): void {
		if (!LAMConfig::isValidName($serverProfileName)) {
			throw new LAMException("Invalid config name: " . $serverProfileName);
		}
		$serverProfiles = [];
		try {
			$serverProfiles = $serverProfilePersistenceManager->getProfiles();
		}
		catch (LAMException $e) {
			logNewMessage(LOG_ERR, 'Unable to read server profiles: ' . $e->getTitle());
		}
		if (!in_array($serverProfileName, $serverProfiles)) {
			throw new LAMException("Config does not exist: " . $serverProfileName);
		}
	}

	/**
	 * Opens the LDAP connection.
	 * @throws LAMException unable to connect to LDAP
	 */
	private function openLdapConnection(): void {
		$ldap = new Ldap($_SESSION['config']);
		try {
			$ldap->connect($_SESSION['config']->getJobsBindUser(), deobfuscateText($_SESSION['config']->getJobsBindPassword()), true);
		}
		catch (LAMException) {
			if ($ldap->server() === null) {
				throw new LAMException("Unable to connect to LDAP");
			}
			throw new LAMException("Unable to connect to LDAP with error: " . ldap_error($ldap->server()) . "\n" . getExtendedLDAPErrorMessage($ldap->server()));
		}
		$_SESSION['ldap'] = $ldap;
	}

	/**
	 * Runs the actions of a server profile.
	 *
	 * @param LAMConfig $conf config
	 * @param CronDatabase $cronDatabase cron DB
	 * @param PDO $pdo PDO
	 * @param string $serverProfileName server profile name
	 * @throws LAMException error executing job
	 */
	private function runServerProfileJobs(LAMConfig $conf, CronDatabase $cronDatabase, PDO $pdo, string $serverProfileName): void {
		$errorsOccurred = false;
		foreach ($cronDatabase->getJobs() as $jobId => $job) {
			$resultLog = new JobResultLog();
			$jobName = $job->getName();
			try {
				logNewMessage(LOG_NOTICE, 'Running job ' . $jobName . ' for server profile ' . $serverProfileName);
				$resultLog->logInfo('Running job ' . $jobName . ' for server profile ' . $serverProfileName);
				$job->execute($jobId, $conf->getJobSettings(), $pdo, $this->isDryRun, $resultLog);
				if (!$this->isDryRun) {
					if ($resultLog->hasError()) {
						$cronDatabase->saveJobRun($jobName, $jobId, 1, implode("\n", $resultLog->getMessages()));
					}
					else {
						$cronDatabase->saveJobRun($jobName, $jobId, 0, implode("\n", $resultLog->getMessages()));
					}
				}
				else {
					// for dry-run print messages to console
					echo implode("\n", $resultLog->getMessages()) . "\n\n";
				}
				logNewMessage(LOG_NOTICE, 'Finished job ' . $jobName . ' for server profile ' . $serverProfileName);
				$resultLog->logInfo('Finished job ' . $jobName . ' for server profile ' . $serverProfileName);
			}
			catch (Exception $e) {
				$errorsOccurred = true;
				echo "Error in job " . $jobName . ': ' . $e->getMessage() . "\n" . $e->getTraceAsString() . "\n";
				if (!$this->isDryRun) {
					$cronDatabase->saveJobRun($jobName, $jobId, 1, implode("\n", $resultLog->getMessages()) . "\n" . $e->getMessage());
				}
			}
		}
		if ($errorsOccurred) {
			throw new LAMException("Problems occurred during job execution for server profile " . $serverProfileName);
		}
	}

}
