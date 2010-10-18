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
* This is the main window of the profile editor.
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

$types = $_SESSION['config']->get_ActiveTypes();
$profileClasses = array();
$profileClassesTemp = array();
for ($i = 0; $i < sizeof($types); $i++) {
	$profileClassesTemp[getTypeAlias($types[$i])] = array(
		'scope' => $types[$i],
		'title' => getTypeAlias($types[$i]),
		'profiles' => "");
}
$profileClassesKeys = array_keys($profileClassesTemp);
natcasesort($profileClassesKeys);
$profileClassesKeys = array_values($profileClassesKeys);
for ($i = 0; $i < sizeof($profileClassesKeys); $i++) {
	$profileClasses[] = $profileClassesTemp[$profileClassesKeys[$i]];
}

// check if user is logged in, if not go to login
if (!$_SESSION['ldap'] || !$_SESSION['ldap']->server()) {
	metaRefresh("../login.php");
	exit;
}

// check if new profile should be created
elseif (isset($_POST['createProfileButton'])) {
	metaRefresh("profilepage.php?type=" . $_POST['createProfile']);
	exit;
}
// check if a profile should be edited
for ($i = 0; $i < sizeof($profileClasses); $i++) {
	if (isset($_POST['editProfile_' . $profileClasses[$i]['scope']]) || isset($_POST['editProfile_' . $profileClasses[$i]['scope'] . '_x'])) {
		metaRefresh("profilepage.php?type=" . $profileClasses[$i]['scope'] .
					"&amp;edit=" . $_POST['profile_' . $profileClasses[$i]['scope']]);
		exit;
	}
}
// check if a profile should be deleted
for ($i = 0; $i < sizeof($profileClasses); $i++) {
	if (isset($_POST['deleteProfile_' . $profileClasses[$i]['scope']]) || isset($_POST['deleteProfile_' . $profileClasses[$i]['scope'] . '_x'])) {
		metaRefresh("profiledelete.php?type=" . $profileClasses[$i]['scope'] .
					"&amp;del=" . $_POST['profile_' . $profileClasses[$i]['scope']]);
		exit;
	}
}

// get list of profiles for each account type
for ($i = 0; $i < sizeof($profileClasses); $i++) {
	$profileList = getAccountProfiles($profileClasses[$i]['scope']);
	natcasesort($profileList);
	$profileClasses[$i]['profiles'] = $profileList;
}

include '../main_header.php';
echo "<div class=\"userlist-bright smallPaddingContent\">\n";
echo "<form action=\"profilemain.php\" method=\"post\">\n";

$container = new htmlTable();

if (isset($_GET['savedSuccessfully'])) {
	$message = new htmlStatusMessage("INFO", _("Profile was saved."), htmlspecialchars($_GET['savedSuccessfully']));
	$message->colspan = 10;
	$container->addElement($message, true);
}
if (isset($_GET['deleteFailed'])) {
	$message = new htmlStatusMessage('ERROR', _('Unable to delete profile!'), getTypeAlias($_GET['deleteScope']) . ': ' . htmlspecialchars($_GET['deleteFailed']));
	$message->colspan = 10;
	$container->addElement($message, true);
}
if (isset($_GET['deleteSucceeded'])) {
	$message = new htmlStatusMessage('INFO', _('Deleted profile.'), getTypeAlias($_GET['deleteScope']) . ': ' . htmlspecialchars($_GET['deleteSucceeded']));
	$message->colspan = 10;
	$container->addElement($message, true);
}

// new profile
$container->addElement(new htmlSubTitle(_('Create a new profile')), true);
$sortedTypes = array();
for ($i = 0; $i < sizeof($profileClasses); $i++) {
	$sortedTypes[$profileClasses[$i]['title']] = $profileClasses[$i]['scope'];
}
natcasesort($sortedTypes);
$newContainer = new htmlTable();
$newProfileSelect = new htmlSelect('createProfile', $sortedTypes);
$newProfileSelect->setHasDescriptiveElements(true);
$newProfileSelect->setWidth('15em');
$newContainer->addElement($newProfileSelect);
$newContainer->addElement(new htmlSpacer('10px', null));
$newContainer->addElement(new htmlButton('createProfileButton', _('Create')), true);
$container->addElement($newContainer, true);

$container->addElement(new htmlSpacer(null, '10px'), true);

// existing profiles
$container->addElement(new htmlSubTitle(_('Manage existing profiles')), true);
$existingContainer = new htmlTable();
$existingContainer->colspan = 5;
for ($i = 0; $i < sizeof($profileClasses); $i++) {
	if ($i > 0) {
		$existingContainer->addElement(new htmlSpacer(null, '10px'), true);
	}
	$existingContainer->addElement(new htmlImage('../../graphics/' . $profileClasses[$i]['scope']));
	$existingContainer->addElement(new htmlSpacer('3px', null));
	$existingContainer->addElement(new htmlOutputText($profileClasses[$i]['title']));
	$existingContainer->addElement(new htmlSpacer('3px', null));
	$select = new htmlSelect('profile_' . $profileClasses[$i]['scope'], $profileClasses[$i]['profiles']);
	$select->setWidth('15em');
	$existingContainer->addElement($select);
	$existingContainer->addElement(new htmlSpacer('3px', null));
	$editButton = new htmlButton('editProfile_' . $profileClasses[$i]['scope'], 'edit.png', true);
	$editButton->setTitle(_('Edit'));
	$existingContainer->addElement($editButton);
	$deleteButton = new htmlButton('deleteProfile_' . $profileClasses[$i]['scope'], 'delete.png', true);
	$deleteButton->setTitle(_('Delete'));
	$existingContainer->addElement($deleteButton);
	$existingContainer->addNewLine();
}
$container->addElement($existingContainer);

// generate content
$tabindex = 1;
parseHtml(null, $container, array(), false, $tabindex, 'user');

echo "</form>\n";
echo "</div>\n";
include '../main_footer.php';

?>
