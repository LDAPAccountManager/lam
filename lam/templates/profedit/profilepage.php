<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003 - 2006  Roland Gruber

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
* Manages creating/changing of profiles.
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
/** access to account modules */
include_once("../../lib/modules.inc");
/** Used to display status messages */
include_once("../../lib/status.inc");

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

// copy type and profile name from POST to GET
if (isset($_POST['profname'])) $_GET['edit'] = $_POST['profname'];
if (isset($_POST['accounttype'])) $_GET['type'] = $_POST['accounttype'];

// abort button was pressed
// back to profile editor
if (isset($_POST['abort'])) {
	metaRefresh("profilemain.php");
	exit;
}

// print header
echo $_SESSION['header'];
echo "<title>Profile editor</title>\n<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "\n<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/type_" . $_GET['type'] . ".css\">\n";
echo "</head><body><br>\n";

// save button was presed
if (isset($_POST['save'])) {
	// create option array to check and save
	$options = array();
	$opt_keys = array_keys($_SESSION['profile_types']);
	foreach ($opt_keys as $element) {
		// text fields
		if ($_SESSION['profile_types'][$element] == "text") {
			$options[$element] = array($_POST[$element]);
		}
		// checkboxes
		elseif ($_SESSION['profile_types'][$element] == "checkbox") {
			if (isset($_POST[$element]) && ($_POST[$element] == "on")) $options[$element] = array('true');
			else $options[$element] = array('false');
		}
		// dropdownbox
		elseif ($_SESSION['profile_types'][$element] == "select") {
			$options[$element] = array($_POST[$element]);
		}
		// multiselect
		elseif ($_SESSION['profile_types'][$element] == "multiselect") {
			if (isset($_POST[$element])) $options[$element] = $_POST[$element];  // value is already an array
			else $options[$element] = array();
		}
	}
	
	// remove double slashes if magic quotes are on
	if (get_magic_quotes_gpc() == 1) {
		foreach ($opt_keys as $element) {
			if (isset($options[$element][0]) && is_string($options[$element][0])) $options[$element][0] = stripslashes($options[$element][0]);
		}
	}
	
	// check options
	$errors = checkProfileOptions($_POST['accounttype'], $options);
	// print error messages if any
	if (sizeof($errors) > 0) {
		for ($i = 0; $i < sizeof($errors); $i++) {
			if (sizeof($errors[$i]) > 3) {  // messages with additional variables
				StatusMessage($errors[$i][0], $errors[$i][1], $errors[$i][2], $errors[$i][3]);
			}
			else {
				StatusMessage($errors[$i][0], $errors[$i][1], $errors[$i][2]);
			}
		}
		echo "<br>\n";
	}
	else {  // input data is valid, save profile
		// save profile
		if (saveAccountProfile($options, $_POST['profname'], $_POST['accounttype'])) {
			echo StatusMessage("INFO", _("Profile was saved."), $_POST['profname']);
			echo ("<br><p><a href=\"profilemain.php\">" . _("Back to profile editor") . "</a></p>");
			echo "</body></html>";
			exit();
		}
		else StatusMessage("ERROR", _("Unable to save profile!"), $_POST['profname']);
	}
}

// empty list of attribute types
$_SESSION['profile_types'] = array();

// check if account type is valid
$type = $_GET['type'];

// get module options
$options = getProfileOptions($type);

// load old profile or POST values if needed
$old_options = array();
if (isset($_POST['save'])) {
	$postKeys = array_keys($_POST);
	for ($i = 0; $i < sizeof($postKeys); $i++) {
		if (!is_array($_POST[$postKeys[$i]])) {
			if (get_magic_quotes_gpc() == 1) {
				$old_options[$postKeys[$i]] = array(stripslashes($_POST[$postKeys[$i]]));
			}
			else {
				$old_options[$postKeys[$i]] = array($_POST[$postKeys[$i]]);
			}
		}
		else {
			$old_options[$postKeys[$i]] = $_POST[$postKeys[$i]];
		}
	}
}
elseif (isset($_GET['edit'])) {
	$old_options = loadAccountProfile($_GET['edit'], $type);
}

// display formular
echo ("<form action=\"profilepage.php?type=$type\" method=\"post\">\n");

// suffix box
// get root suffix
$rootsuffix = $_SESSION['config']->get_Suffix($type);
// get subsuffixes
$suffixes = array();
foreach ($_SESSION['ldap']->search_units($rootsuffix) as $suffix) {
	$suffixes[] = $suffix;
}
// get RDNs
$rdns = getRDNAttributes($type);

echo "<fieldset class=\"" . $type . "edit\">\n";
echo "<legend><img align=\"middle\" src=\"../../graphics/logo32.png\" alt=\"logo32.png\"> <b>" . _("LDAP") . "</b></legend>\n";
	echo "<table border=0>";
	echo "<tr><td>";
	// LDAP suffix
	echo _("LDAP suffix") . ":";
	echo "</td><td>";
	echo "<select name=\"ldap_suffix\" tabindex=\"1\">";
	for ($i = 0; $i < sizeof($suffixes); $i++) {
		if (isset($old_options['ldap_suffix']) && ($old_options['ldap_suffix'][0] == $suffixes[$i])) {
			echo "<option selected>" . $suffixes[$i] . "</option>\n";
		}
		else {
			echo "<option>" . $suffixes[$i] . "</option>\n";
		}
	}
	echo "</select>\n";
	echo "</td><td>";
	// help link
	echo "&nbsp;<a href=\"../help.php?HelpNumber=361\" target=\"lamhelp\">";
	echo "<img src=\"../../graphics/help.png\" alt=\"" . _('Help') . "\" title=\"" . _('Help') . "\">";
	echo "</a><br>\n";
	echo "</td></tr>";
	// LDAP RDN
	echo "<tr><td>";
	echo _("RDN identifier") . ":";
	echo "</td><td>";
	echo "<select name=\"ldap_rdn\" tabindex=\"1\">";
	for ($i = 0; $i < sizeof($rdns); $i++) {
		if (isset($old_options['ldap_rdn']) && ($old_options['ldap_rdn'][0] == $rdns[$i])) {
			echo "<option selected>" . $rdns[$i] . "</option>\n";
		}
		else {
			echo "<option>" . $rdns[$i] . "</option>\n";
		}
	}
	echo "</select>\n";
	echo "</td><td>";
	// help link
	echo "&nbsp;<a href=\"../help.php?HelpNumber=301\" target=\"lamhelp\">";
	echo "<img src=\"../../graphics/help.png\" alt=\"" . _('Help') . "\" title=\"" . _('Help') . "\">";
	echo "</a><br>\n";
	echo "</td></tr>";
	echo "</table>";
echo "</fieldset>\n<br>\n";
$_SESSION['profile_types']['ldap_suffix'] = 'select';
$_SESSION['profile_types']['ldap_rdn'] = 'select';

// index for tab order (1 is LDAP suffix)
$tabindex = 2;
$tabindexLink = 1000;	// links are at the end

// display module options
$modules = array_keys($options);
for ($m = 0; $m < sizeof($modules); $m++) {
	// ignore modules without options
	if (sizeof($options[$modules[$m]]) < 1) continue;
	echo "<fieldset class=\"" . $type . "edit\">\n";
	$icon = '';
	$module = new $modules[$m]($type);
	$iconImage = $module->getIcon();
	if ($iconImage != null) {
		$icon = '<img align="middle" src="../../graphics/' . $iconImage . '" alt="' . $iconImage . '"> ';
	}
	echo "<legend>$icon<b>" . getModuleAlias($modules[$m], $type) . "</b></legend>\n";
	$profileTypes = parseHtml($modules[$m], $options[$modules[$m]], $old_options, true, $tabindex, $tabindexLink, $type);
	$_SESSION['profile_types'] = array_merge($profileTypes, $_SESSION['profile_types']);
	echo "</fieldset>\n";
	echo "<br>";
}

// profile name and submit/abort buttons
echo ("<b>" . _("Profile name") . ":</b> \n");
$tabindex++;
echo ("<input tabindex=\"$tabindex\" type=\"text\" name=\"profname\" value=\"" . $_GET['edit'] . "\">\n");
// help link
echo "<a href=\"../help.php?HelpNumber=360\" target=\"lamhelp\">";
echo "<img src=\"../../graphics/help.png\" alt=\"" . _('Help') . "\" title=\"" . _('Help') . "\">";
echo "</a><br><br>\n";
$tabindex++;
echo ("<input tabindex=\"$tabindex\" type=\"submit\" name=\"save\" value=\"" . _("Save") . "\">\n");
$tabindex++;
echo ("<input tabindex=\"$tabindex\" type=\"reset\" name=\"reset\" value=\"" . _("Reset") . "\">\n");
$tabindex++;
echo ("<input tabindex=\"$tabindex\" type=\"submit\" name=\"abort\" value=\"" . _("Cancel") . "\">\n");
echo "<input type=\"hidden\" name=\"accounttype\" value=\"$type\">\n";

echo ("</form></body></html>\n");

?>
