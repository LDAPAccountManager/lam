<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2007 - 2010  Roland Gruber

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
include_once("../../lib/security.inc");
/** Needed to find DNs of users */
include_once("../../lib/ldap.inc");
/** Used to display error messages */
include_once("../../lib/status.inc");

// start session
startSecureSession();

setlanguage();

// get account name and type
$dn = $_GET['DN'];
$type = $_GET['type'];
if (!preg_match('/^[a-z0-9_]+$/i', $type)) {
	logNewMessage(LOG_ERR, 'Invalid type: ' . $type);
	die();
}

if (isset($dn) && isset($type)) {
	$dn = str_replace("\\", '',$dn);
	$dn = str_replace("'", '',$dn);
	$_SESSION['delete_dn'] = array($dn);
	// redirect to delete.php
	metaRefresh("../delete.php?type=" . htmlspecialchars($type));

}
else {
	// print error message if arguments are missing
	include '../main_header.php';
	StatusMessage("ERROR", "No account or type given.");
	include '../main_footer.php';
}

?>
