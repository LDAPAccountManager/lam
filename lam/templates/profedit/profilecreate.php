<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Roland Gruber

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
* Saves new/modified profiles.
*
* @package profiles
* @author Roland Gruber
*/

/** Used to display status messages */
include_once("../../lib/status.inc");
/** access to account modules */
include_once("../../lib/modules.inc");
/** helper functions for profiles */
include_once("../../lib/profiles.inc");
/** access to LDAP server */
include_once("../../lib/ldap.inc");
/** access to configuration options */
include_once("../../lib/config.inc");

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

// abort button was pressed in profileuser/~host.php
// back to profile editor
if ($_POST['abort']) {
	metaRefresh("profilemain.php");
	exit;
}

// check if user is logged in, if not go to login
if (!$_SESSION['ldap'] || !$_SESSION['ldap']->server()) {
	metaRefresh("../login.php");
	exit;
}

// print header
echo $_SESSION['header'];
echo "<title></title>\n<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head>\n<body>\n<br>\n";

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
		if ($_POST[$element] == "on") $options[$element] = array('true');
		else $options[$element] = array('false');
	}
	// dropdownbox
	elseif ($_SESSION['profile_types'][$element] == "select") {
		$options[$element] = array($_POST[$element]);
	}
	// multiselect
	elseif ($_SESSION['profile_types'][$element] == "multiselect") {
		$options[$element] = $_POST[$element];  // value is already an array
	}
}

// remove double slashes if magic quotes are on
if (get_magic_quotes_gpc() == 1) {
	foreach ($opt_keys as $element) {
		if (is_string($options[$element][0])) $options[$element][0] = stripslashes($options[$element][0]);
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
}
else {  // input data is valid, save profile
	// save profile
	if (saveAccountProfile($options, $_POST['profname'], $_POST['accounttype'])) {
		echo StatusMessage("INFO", _("Profile was saved."), $_POST['profname']);
	}
	else StatusMessage("ERROR", _("Unable to save profile!"), $_POST['profname']);
	echo ("<br><p><a href=\"profilemain.php\">" . _("Back to Profile Editor") . "</a></p>");
}

echo ("</body></html>\n");

?>
