<?php
namespace LAM\ACCOUNT;
use accountContainer;
use DateTime;
use LAM\TYPES\TypeManager;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Tilo Lutz
                2005 - 2021  Roland Gruber

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


  LDAP Account Manager displays table for creating or modifying accounts in LDAP
*/

/**
* Displays the account detail page.
*
* @package modules
* @author Tilo Lutz
* @author Roland Gruber
*/

/** security functions */
include_once(__DIR__ . "/../../lib/security.inc");
/** configuration options */
include_once(__DIR__ . '/../../lib/config.inc');
/** functions to load and save profiles */
include_once(__DIR__ . '/../../lib/profiles.inc');
/** Return error-message */
include_once(__DIR__ . '/../../lib/status.inc');
/** Return a pdf-file */
include_once(__DIR__ . '/../../lib/pdf.inc');
/** module functions */
include_once(__DIR__ . '/../../lib/modules.inc');

// Start session
startSecureSession();
enforceUserIsLoggedIn();

// Redirect to startpage if user is not logged in
if (!isLoggedIn()) {
	metaRefresh("../login.php");
	exit;
	}

// Set correct language, codepages, ....
setlanguage();

$sessionAccountPrefix = 'editContainer';
if (isset($_GET['editKey'])) {
	$sessionKey = htmlspecialchars($_GET['editKey']);
}
else {
	$sessionKey = $sessionAccountPrefix . (new DateTime(null, getTimeZone()))->getTimestamp() . getRandomNumber();
}

// cleanup account containers in session
$cleanupCandidates = array();
foreach ($_SESSION as $key => $value) {
	if (strpos($key, $sessionAccountPrefix) === 0) {
		$cleanupCandidates[] = $key;
	}
	$candidateCount = sizeof($cleanupCandidates);
	if ($candidateCount > 100) {
		$numToDelete = $candidateCount - 100;
		natsort($cleanupCandidates);
		for ($i = 0; $i < $numToDelete; $i++) {
			$toDelete = array_shift($cleanupCandidates);
			unset($_SESSION[$toDelete]);
		}
	}
}

$typeManager = new TypeManager();
//load account
if (isset($_GET['DN'])) {
	$type = $typeManager->getConfiguredType($_GET['type']);
	$DN = str_replace("\\'", '', $_GET['DN']);
	if ($type->isHidden()) {
		logNewMessage(LOG_ERR, 'User tried to access hidden account type: ' . $type->getId());
		die();
	}
	if ($_GET['DN'] == $DN) {
		if (substr($DN, 0, 1) === "'") {
			$DN = substr($DN, 1);
		}
		if (substr($DN, -1, 1) === "'") {
			$DN = substr($DN, 0, -1);
		}
	}
	$suffix = strtolower($type->getSuffix());
	$DNlower = strtolower($DN);
	if (strpos($DNlower, $suffix) !== (strlen($DNlower) - strlen($suffix))) {
		logNewMessage(LOG_ERR, 'User tried to access entry of type ' . $type->getId() . ' outside suffix ' . $suffix);
		die();
	}
	$_SESSION[$sessionKey] = new accountContainer($type, $sessionKey);
	$result = $_SESSION[$sessionKey]->load_account($DN);
	if (sizeof($result) > 0) {
		include __DIR__ . '/../../lib/adminHeader.inc';
		foreach ($result as $message) {
			call_user_func_array("StatusMessage", $message);
		}
		include __DIR__ . '/../../lib/adminFooter.inc';
		die();
	}
}
// new account
elseif (empty($_POST)) {
	$type = $typeManager->getConfiguredType($_GET['type']);
	if ($type->isHidden()) {
		logNewMessage(LOG_ERR, 'User tried to access hidden account type: ' . $type->getId());
		die();
	}
	elseif (!checkIfNewEntriesAreAllowed($type->getId())) {
		logNewMessage(LOG_ERR, 'User tried to create entry of forbidden account type: ' . $type->getId());
		die();
	}
	$_SESSION[$sessionKey] = new accountContainer($type, $sessionKey);
	$_SESSION[$sessionKey]->new_account();
}

// show account page
$_SESSION[$sessionKey]->continue_main();

