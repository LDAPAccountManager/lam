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

use LAMConfig;
use LAMException;
use ServerProfilePersistenceManager;

/**
 * Global cron job for LAM.
 *
 * @package jobs
 * @author Roland Gruber
 */

/** config */
include_once(__DIR__ . "/config.inc");
/** cron runner */
include_once(__DIR__ . "/cronRunner.inc");

// check parameters
$isDryRun = isset($argv) && (in_array('--dryRun', $argv) || in_array('--dry-run', $argv));
$cronRunner = new CronRunner($isDryRun);

$runGlobalActions = isset($argv[2]) && ($argv[2] === 'global');
$runSingleServerProfileActions = isset($argv[2]) && ($argv[2] === 'serverProfile');
$runAll = isset($argv[2]) && ($argv[2] === 'all');
if ($runAll) {
	try {
		$cronRunner->runGlobalCronJobs();
		$serverProfilePersistenceManager = new ServerProfilePersistenceManager();
		foreach ($serverProfilePersistenceManager->getProfiles() as $serverProfile) {
			$cronRunner->runSingleServerProfileActions($serverProfile);
		}
	}
	catch (LAMException $e) {
		echo escapeshellcmd($e->getTitle()) . "\n";
		exit(1);
	}
}
elseif ($runGlobalActions) {
	try {
		$cronRunner->runGlobalCronJobs();
	}
	catch (LAMException $e) {
		echo escapeshellcmd($e->getTitle()) . "\n";
		exit(1);
	}
}
elseif ($runSingleServerProfileActions && !empty($argv[3]) && LAMConfig::isValidName($argv[3])) {
	try {
		$cronRunner->runSingleServerProfileActions($argv[3]);
	}
	catch (LAMException $e) {
		echo escapeshellcmd($e->getTitle()) . "\n";
		exit(1);
	}
}
else {
	echo "LAM - Run cron jobs\n"
		. "\n"
		. "Usage:\n"
		. "\n"
		. "Run all cron jobs (global and server profiles):\n"
		. "\n"
		. " runCronJobs.sh all\n"
		. "\n"
		. "Run all cron jobs (global and server profiles) in dry-run:\n"
		. "\n"
		. " runCronJobs.sh all --dry-run\n"
		. "\n"
		. "Run all global cron jobs:\n"
		. "\n"
		. " runCronJobs.sh global\n"
		. "\n"
		. "Run all global cron jobs in dry-run:\n"
		. "\n"
		. " runCronJobs.sh global --dry-run\n"
		. "\n"
		. "Run cron jobs of a server profile:\n"
		. "\n"
		. " runCronJobs.sh serverProfile NAME\n"
		. "\n"
		. "Run cron jobs of a server profile in dry-run:\n"
		. "\n"
		. " runCronJobs.sh serverProfile NAME --dry-run\n"
		. "\n"
		. "Run cron jobs of all server profiles:\n"
		. "\n"
		. " runCronJobs.sh allServerProfiles\n"
		. "\n"
		. "Run cron jobs of all server profiles in dry-run:\n"
		. "\n"
		. " runCronJobs.sh allServerProfiles --dry-run\n"
		. "\n"
		. "\n";
}
