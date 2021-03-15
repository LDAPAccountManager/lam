<?php
namespace LAM\INIT;

use LAM\PDF\PdfStructurePersistenceManager;
use LAM\PROFILES\AccountProfilePersistenceManager;
use LAMException;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2021  Roland Gruber

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
* This page redirects to the correct start page after login.
*
* @package main
* @author Roland Gruber
*/

/** config object */
include_once '../lib/config.inc';
/** profiles */
include_once '../lib/profiles.inc';

// start session
startSecureSession();
enforceUserIsLoggedIn();

setlanguage();

$accountProfilePersistenceManager = new AccountProfilePersistenceManager();
$pdfStructurePersistenceManager = new PdfStructurePersistenceManager();
try {
	$accountProfilePersistenceManager->installAccountProfileTemplates($_SESSION['config']->getName());
	$pdfStructurePersistenceManager->installPDFTemplates($_SESSION['config']->getName());
} catch (LAMException $e) {
	logNewMessage(LOG_ERR, $e->getTitle());
}

$conf = $_SESSION['config'];

// check if user password is not expired
if (!$conf->isHidePasswordPromptForExpiredPasswords()) {
	$userDn = $_SESSION['ldap']->getUserName();
	$userData = ldapGetDN($userDn, array('*', '+', 'pwdReset', 'passwordExpirationTime'));
	$ldapErrorCode = ldap_errno($_SESSION['ldap']->server());
	logNewMessage(LOG_DEBUG, 'Expired password check: Reading ' . $userDn . ' with return code ' . $ldapErrorCode . ' and data: ' . print_r($userData, true));
	if (($ldapErrorCode != 32) && ($ldapErrorCode != 34)) {
		$pwdResetMarker = (!empty($userData['pwdreset'][0]) && ($userData['pwdreset'][0] == 'TRUE'));
		$pwdExpiration = (!empty($userData)) && class_exists('\locking389ds') && \locking389ds::isPasswordExpired($userData);
		if (($userData === null) || $pwdResetMarker || $pwdExpiration) {
			metaRefresh("changePassword.php");
			exit();
		}
	}
}

// check if all suffixes in conf-file exist
$new_suffs = array();
// get list of active types
$typeManager = new \LAM\TYPES\TypeManager();
$types = $typeManager->getConfiguredTypes();
foreach ($types as $type) {
	$info = @ldap_read($_SESSION['ldap']->server(), escapeDN($type->getSuffix()), "(objectClass=*)", array('objectClass'), 0, 0, 0, LDAP_DEREF_NEVER);
	if (($info === false) && !in_array($type->getSuffix(), $new_suffs)) {
		$new_suffs[] = $type->getSuffix();
		continue;
	}
	$res = ldap_get_entries($_SESSION['ldap']->server(), $info);
	if (!$res && !in_array($type->getSuffix(), $new_suffs)) {
		$new_suffs[] = $type->getSuffix();
	}
}

// display page to add suffixes, if needed
if ((sizeof($new_suffs) > 0) && checkIfWriteAccessIsAllowed()) {
	metaRefresh("initsuff.php?suffs='" . implode(";", $new_suffs));
	exit();
}

if (sizeof($types) > 0) {
	foreach ($types as $type) {
		if ($type->isHidden()) {
			continue;
		}
		metaRefresh("lists/list.php?type=" . $type->getId());
		exit();
	}
}

metaRefresh("tree/treeViewContainer.php");
