<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2010  Roland Gruber

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
* Manages deletion of profiles.
*
* @package profiles
* @author Roland Gruber
*/

/** security functions */
include_once("../../lib/security.inc");
/** helper functions for profiles */
include_once("../../lib/profiles.inc");
/** access to LDAP server */
include_once("../../lib/ldap.inc");
/** access to configuration options */
include_once("../../lib/config.inc");

// start session
startSecureSession();

// die if no write access
if (!checkIfWriteAccessIsAllowed()) die();

setlanguage();

// check if user is logged in, if not go to login
if (!$_SESSION['ldap'] || !$_SESSION['ldap']->server()) {
	metaRefresh("../login.php");
	exit;
}

// check if admin has submited delete operation
if (isset($_POST['submit'])) {
	// delete profile
	if (!delAccountProfile($_POST['del'], $_POST['type'])) {
		metaRefresh('profilemain.php?deleteScope=' . $_POST['type'] . '&amp;deleteFailed=' . $_POST['del']);
		exit();
	}
	else {
		metaRefresh('profilemain.php?deleteScope=' . $_POST['type'] . '&amp;deleteSucceeded=' . $_POST['del']);
		exit();
	}
}

// check if admin has aborted delete operation
if (isset($_POST['abort'])) {
	metaRefresh('profilemain.php');
	exit;
}

// print standard header
include '../main_header.php';
echo "<div class=\"userlist-bright smallPaddingContent\">\n";
echo "<form action=\"profiledelete.php\" method=\"post\">\n";

$type = $_GET['type'];

$container = new htmlTable();
$container->addElement(new htmlOutputText(_("Do you really want to delete this profile?")), true);
$container->addElement(new htmlSpacer(null, '10px'), true);

$subContainer = new htmlTable();
$subContainer->addElement(new htmlOutputText(_("Profile name") . ': '));
$subContainer->addElement(new htmlOutputText($_GET['del']), true);
$container->addElement($subContainer, true);
$container->addElement(new htmlSpacer(null, '10px'), true);

$buttonContainer = new htmlTable();
$buttonContainer->addElement(new htmlButton('submit', _("Ok")));
$buttonContainer->addElement(new htmlButton('abort', _("Cancel")));
$buttonContainer->addElement(new htmlHiddenInput('type', $type));
$buttonContainer->addElement(new htmlHiddenInput('del', $_GET['del']));
$container->addElement($buttonContainer);

$tabindex = 1;
parseHtml(null, $container, array(), false, $tabindex, 'user');

echo "</form>\n";
echo "</div>";
include '../main_footer.php';
