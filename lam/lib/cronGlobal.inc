<?php

namespace LAM\Cron;

use Exception;
use LAM\DB\CronDatabase;
use LAMCfgMain;
use LAMConfig;
use LAMException;
use PDOException;
use Ldap;
use LAM\JOB\JobResultLog;
use ServerProfilePersistenceManager;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2024  Roland Gruber

*/

/**
 * Global cron job for LAM.
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

// check parameters
$isDryRun = (isset($argv[2]) && ($argv[2] == '--dryRun')) ? true : false;

$_SESSION['cfgMain'] = new LAMCfgMain();

if ($isDryRun) {
	echo "Dry run: no changes, only logging.\n";
}

// execute jobs
$errorsOccurred = false;
$allModules = getAllModules();
foreach ($allModules as $module) {
	if (!$module->supportsGlobalCronJob()) {
		continue;
	}
	try {
		if ($isDryRun) {
			echo "Started actions for " . $module->get_alias() . "\n";
		}
		logNewMessage(LOG_NOTICE, 'Started actions for ' . $module->get_alias());
		$module->runGlobalCronActions($isDryRun);
		if ($isDryRun) {
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
	echo "Problems occurred during cron execution\n";
	die(1);
}
