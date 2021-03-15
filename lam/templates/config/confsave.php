<?php
namespace LAM\CONFIG;
use \LAMConfig;
use \htmlStatusMessage;
use LAMException;
use ServerProfilePersistenceManager;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2009 - 2021  Roland Gruber

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


/**
* End page of configuration.
*
* @package configuration
* @author Roland Gruber
*/


/** Access to config functions */
include_once(__DIR__ . "/../../lib/config.inc");

/** access to module settings */
include_once(__DIR__ . "/../../lib/modules.inc");

// start session
if (strtolower(session_module_name()) == 'files') {
	session_save_path("../../sess");
}
lam_start_session();

setlanguage();

// check if password was entered
// if not: load login page
if (!isset($_SESSION['conf_isAuthenticated']) || ($_SESSION['conf_config']->getName() !== $_SESSION['conf_isAuthenticated'])) {
	$_SESSION['conf_message'] = new htmlStatusMessage('ERROR', _("No password was entered!"));
	/** go back to login if password is empty */
	require('conflogin.php');
	exit;
}

$conf = &$_SESSION['conf_config'];
$confName = $_SESSION['conf_isAuthenticated'];

$serverProfilePersistenceManager = new ServerProfilePersistenceManager();
try {
	$serverProfilePersistenceManager->saveProfile($conf, $confName);
	metaRefresh('../login.php?configSaveOk=1&configSaveFile=' . $confName);
}
catch (LAMException $e) {
	metaRefresh('../login.php?configSaveFailed=1&configSaveFile=' . $confName);
}
finally {
	// remove settings from session
	$sessionKeys = array_keys($_SESSION);
	for ($i = 0; $i < sizeof($sessionKeys); $i++) {
		if (substr($sessionKeys[$i], 0, 5) == "conf_") {
			unset($_SESSION[$sessionKeys[$i]]);
		}
	}
}
