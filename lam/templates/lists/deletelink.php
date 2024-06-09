<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2007 - 2024  Roland Gruber

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more detaexils.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

/**
* This page will redirect to delete.php if the given user is valid.
*
* It is called from the list views via the delete links.
*
* @package lists
* @author Roland Gruber
*/

/** security functions */
include_once(__DIR__ . "/../../lib/security.inc");
/** Needed to find DNs of users */
include_once(__DIR__ . "/../../lib/ldap.inc");
/** Used to display error messages */
include_once(__DIR__ . "/../../lib/status.inc");

// start session
startSecureSession();
enforceUserIsLoggedIn();

setlanguage();

// get account name and type
$dn = (string) $_GET['DN'];
$type = (string) $_GET['type'];
if (!preg_match('/^[a-z0-9_]+$/i', $type)) {
	logNewMessage(LOG_ERR, 'Invalid type: ' . $type);
	die();
}

if (!empty($dn) && !empty($type)) {
	if (str_starts_with($dn, "'")) {
		$dn = substr($dn, 1);
	}
	if (str_ends_with($dn, "'")) {
		$dn = substr($dn, 0, -1);
	}
	$_SESSION['delete_dn'] = [$dn];
	// redirect to delete.php
	metaRefresh("../delete.php?type=" . htmlspecialchars($type));

}
else {
	// print error message if arguments are missing
	include __DIR__ . '/../../lib/adminHeader.inc';
	StatusMessage("ERROR", "No account or type given.");
	include __DIR__ . '/../../lib/adminFooter.inc';
}
