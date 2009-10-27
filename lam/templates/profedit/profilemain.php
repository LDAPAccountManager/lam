<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2008  Roland Gruber

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
for ($i = 0; $i < sizeof($types); $i++) {
	$profileClasses[] = array(
		'scope' => $types[$i],
		'title' => getTypeAlias($types[$i]),
		'profiles' => "");
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
	$profiles = "";
	for ($l = 0; $l < sizeof($profileList); $l++) {
		$profiles = $profiles . "<option>" . $profileList[$l] . "</option>\n";
	}
	$profileClasses[$i]['profiles'] = $profiles;
}

echo $_SESSION['header'];


echo "<title>LDAP Account Manager</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
for ($i = 0; $i < sizeof($profileClasses); $i++) {
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/type_" . $profileClasses[$i]['scope'] . ".css\">\n";
}
echo "</head>\n";
echo "<body>\n";

echo "<br>\n";
echo "<h1>" . _('Profile editor') . "</h1>\n";
echo "<br>\n";

echo "<form action=\"profilemain.php\" method=\"post\">\n";

// new profile
echo "<fieldset class=\"useredit\">\n";
echo "<legend>\n";
echo "<b>" . _('Create a new profile') . "</b>\n";
echo "</legend>\n";
echo "<br><table border=0>\n";
	echo "<tr><td>\n";
		echo "<select class=\"user\" name=\"createProfile\">\n";
			for ($i = 0; $i < sizeof($profileClasses); $i++) {
				echo "<option value=\"" . $profileClasses[$i]['scope'] . "\">" . $profileClasses[$i]['title'] . "</option>\n";
			}
		echo "</select>\n";
	echo "</td>\n";
	echo "<td>\n";
		echo "<input type=\"submit\" name=\"createProfileButton\" value=\"" . _('Create') . "\">";
	echo "</td></tr>\n";
echo "</table>\n";
echo "</fieldset>\n";
echo "<br>\n";

// existing profiles
echo "<fieldset class=\"useredit\">\n";
echo "<legend>\n";
echo "<b>" . _('Manage existing profiles') . "</b>\n";
echo "</legend>\n";
echo "<br><table border=0>\n";
for ($i = 0; $i < sizeof($profileClasses); $i++) {
	if ($i > 0) {
		echo "<tr><td colspan=3>&nbsp;</td></tr>\n";
	}
	echo "<tr>\n";
		echo "<td>";
			echo "<img alt=\"" . $profileClasses[$i]['title'] . "\" src=\"../../graphics/" . $profileClasses[$i]['scope'] . ".png\">&nbsp;\n";
			echo $profileClasses[$i]['title'];
		echo "</td>\n";
		echo "<td>&nbsp;";
			echo "<select class=\"user\" style=\"width: 20em;\" name=\"profile_" . $profileClasses[$i]['scope'] . "\">\n";
				echo $profileClasses[$i]['profiles'];
			echo "</select>\n";
		echo "</td>\n";
		echo "<td>&nbsp;";
			echo "<input type=\"image\" src=\"../../graphics/edit.png\" name=\"editProfile_" . $profileClasses[$i]['scope'] . "\" " .
			 "alt=\"" . _('Edit') . "\" title=\"" . _('Edit') . "\">";
			echo "&nbsp;";
			echo "<input type=\"image\" src=\"../../graphics/delete.png\" name=\"deleteProfile_" . $profileClasses[$i]['scope'] . "\" " .
			"alt=\"" . _('Delete') . "\" title=\"" . _('Delete') . "\">";
		echo "</td>\n";
	echo "</tr>\n";
}
echo "</table>\n";
echo "</fieldset>\n";
echo "<br>\n";

echo "</form>\n";
echo "</body>\n";
echo "</html>\n";

?>
