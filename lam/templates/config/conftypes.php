<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2004 - 2006  Roland Gruber

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
* Here the user can select the account types.
*
* @package configuration
* @author Roland Gruber
*/


/** Access to config functions */
include_once ('../../lib/config.inc');
/** Access to account types */
include_once ('../../lib/types.inc');

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

// check if config is set
// if not: load login page
if (!isset($_SESSION['conf_config'])) {
	/** go back to login if password is invalid */
	require('conflogin.php');
	exit;
}

$conf = &$_SESSION['conf_config'];

// update type settings
if (isset($_POST['postAvailable'])) {
	$postKeys = array_keys($_POST);
	for ($i = 0; $i < sizeof($postKeys); $i++) {
		$key = $postKeys[$i];
		if (substr($key, 0, 7) == "suffix_") {
			$_SESSION['conf_typeSettings'][$key] = $_POST[$key];
		}
		elseif (substr($key, 0, 5) == "attr_") {
			$_SESSION['conf_typeSettings'][$key] = $_POST[$key];
		}
	}
}

$errors = array();
// user pressed submit/abort button
if ($_POST['submit']) {
	// check settings
	$allOK = true;
	$postKeys = array_keys($_POST);
	for ($i = 0; $i < sizeof($postKeys); $i++) {
		$key = $postKeys[$i];
		if (substr($key, 0, 7) == "suffix_") {
			$type = substr($postKeys[$i], 7);
			if (strlen($_POST[$key]) < 1) {
				$errors[] = array("ERROR", _("LDAP Suffix is invalid!"), getTypeAlias($type));
				$allOK = false;
			}
		}
		elseif (substr($key, 0, 5) == "attr_") {
			$type = substr($postKeys[$i], 5);
			if (!is_string($_POST[$key]) || !eregi("^((#[^:;]+)|([^:;]*:[^:;]+))(;((#[^:;]+)|([^:;]*:[^:;]+)))*$", $_POST[$key])) {
				$errors[] = array("ERROR", _("List attributes are invalid!"), getTypeAlias($type));
				$allOK = false;
			}
		}
	}
	//selection ok, back to other settings
	if ($allOK) {
		// check if there is a new type
		$addedType = false;
		for ($i = 0; $i < sizeof($_SESSION['conf_accountTypes']); $i++) {
			if (!in_array($_SESSION['conf_accountTypes'][$i], $_SESSION['conf_accountTypesOld'])) {
				$addedType = true;
				break;
			}
		}
		$_SESSION['conf_accountTypesOld'] = $_SESSION['conf_accountTypes'];
		$conf->set_ActiveTypes($_SESSION['conf_accountTypes']);
		$conf->set_typeSettings($_SESSION['conf_typeSettings']);
		if ($addedType) {
			metarefresh('confmain.php?typesback=true&amp;typeschanged=true');
		}
		else {
			metarefresh('confmain.php?typesback=true');
		}
		exit;
	}
}
// no changes
elseif ($_POST['abort']) {
	$_SESSION['conf_accountTypes'] = $_SESSION['conf_accountTypesOld'];
	metarefresh('confmain.php?typesback=true');
	exit;
}

// check if remove button was pressed
$postKeys = array_keys($_POST);
for ($i = 0; $i < sizeof($postKeys); $i++) {
	$key = $postKeys[$i];
	if (substr($key, 0, 4) == "rem_") {
		$type = substr($key, 4);
		$_SESSION['conf_accountTypes'] = array_flip($_SESSION['conf_accountTypes']);
		unset($_SESSION['conf_accountTypes'][$type]);
		$_SESSION['conf_accountTypes'] = array_flip($_SESSION['conf_accountTypes']);
		$_SESSION['conf_accountTypes'] = array_values($_SESSION['conf_accountTypes']);
	}
}

// check if add button was pressed
$postKeys = array_keys($_POST);
for ($i = 0; $i < sizeof($postKeys); $i++) {
	$key = $postKeys[$i];
	if (substr($key, 0, 4) == "add_") {
		$type = substr($key, 4);
		$_SESSION['conf_accountTypes'][] = $type;
	}
}


// get active and available types
$allTypes = getTypes();
$activeTypes = $_SESSION['conf_accountTypes'];
$availableTypes = array();
for ($i = 0; $i < sizeof($allTypes); $i++) {
	if (!in_array($allTypes[$i], $activeTypes)) $availableTypes[] = $allTypes[$i];
}

echo $_SESSION['header'];

echo "<title>" . _("LDAP Account Manager Configuration") . "</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
for ($i = 0; $i < sizeof($allTypes); $i++){
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/type_" . $allTypes[$i] . ".css\">\n";
}
echo "</head><body>\n";

echo ("<p align=\"center\"><a href=\"http://lam.sf.net\" target=\"new_window\">".
	"<img src=\"../../graphics/banner.jpg\" border=1 alt=\"LDAP Account Manager\"></a></p><hr><br>\n");

// print error messages
for ($i = 0; $i < sizeof($errors); $i++) call_user_func_array('StatusMessage', $errors[$i]);

echo ("<form action=\"conftypes.php\" method=\"post\">\n");
echo "<h1 align=\"center\">" . _("Account type selection") . "</h1>";

// show available types
if (sizeof($availableTypes) > 0) {
	echo "<fieldset><legend><b>" . _("Available account types") . "</b></legend>\n";
	echo "<table>\n";
	for ($i = 0; $i < sizeof($availableTypes); $i++) {
		echo "<tr>\n";
			echo "<td><b>" . getTypeAlias($availableTypes[$i]) . ": </b></td>\n";
			echo "<td>" . getTypeDescription($availableTypes[$i]) . "</td>\n";
			echo "<td><input type=\"submit\" name=\"add_" . $availableTypes[$i] ."\" value=\"" . _("Add") . "\"></td>\n";
		echo "</tr>\n";
	}
	echo "</table>\n";
	echo "</fieldset>\n";
	
	echo "<p><br><br></p>";
}

// show active types
if (sizeof($activeTypes) > 0) {
	echo "<fieldset><legend><b>" . _("Active account types") . "</b></legend><br>\n";
	for ($i = 0; $i < sizeof($activeTypes); $i++) {
		echo "<fieldset class=\"" . $activeTypes[$i] . "edit\">\n";
		echo "<legend>" . "<b>" . getTypeAlias($activeTypes[$i]) . ": </b>" . getTypeDescription($activeTypes[$i]) . "</legend>";
		echo "<br>\n";
		echo "<table>\n";
		// LDAP suffix
		echo "<tr>\n";
			echo "<td>" . _("LDAP suffix") . "</td>\n";
			echo "<td><input type=\"text\" size=\"40\" name=\"suffix_" . $activeTypes[$i] . "\" value=\"" . $_SESSION['conf_typeSettings']['suffix_' . $activeTypes[$i]] . "\"></td>\n";
			echo "<td>";
			echo "<a href=\"../help.php?HelpNumber=202\" target=\"lamhelp\">";
			echo "<img src=\"../../graphics/help.png\" alt=\"" . _('Help') . "\" title=\"" . _('Help') . "\">";
			echo "</a>\n";
			echo "</td>\n";
		echo "</tr>\n";
		// list attributes
		if (isset($_SESSION['conf_typeSettings']['attr_' . $activeTypes[$i]])) {
			$attributes = $_SESSION['conf_typeSettings']['attr_' . $activeTypes[$i]];
		}
		else {
			$attributes = getDefaultListAttributes($activeTypes[$i]);
		}
		echo "<tr>\n";
			echo "<td>" . _("List attributes") . "</td>\n";
			echo "<td><input type=\"text\" size=\"40\" name=\"attr_" . $activeTypes[$i] . "\" value=\"" . $attributes . "\"></td>\n";
			echo "<td>";
			echo "<a href=\"../help.php?HelpNumber=206\" target=\"lamhelp\">";
			echo "<img src=\"../../graphics/help.png\" alt=\"" . _('Help') . "\" title=\"" . _('Help') . "\">";
			echo "</a>\n";
			echo "</td>\n";
		echo "</tr>\n";
		echo "<tr><td colspan=\"2\">&nbsp;</td></tr>\n";
		// remove button
		echo "<tr>\n";
			echo "<td colspan=\"2\"><input type=\"submit\" name=\"rem_" . $activeTypes[$i] . "\" value=\"" . _("Remove this account type") . "\"></td>\n";
		echo "</tr>\n";
		echo "</table>\n";
		echo "</fieldset><br>\n";
	}
	echo "</fieldset>\n";
	echo "<p><br><br></p>\n";
}

// submit and abort button
echo "<p>";
echo "<input type=\"submit\" name=\"submit\" value=\"" . _("Submit") . "\">\n";
echo "<input type=\"submit\" name=\"abort\" value=\"" . _("Abort") . "\">\n";
echo "<input type=\"hidden\" name=\"postAvailable\" value=\"yes\">\n";
echo "</p>";

echo "<p><br><br></p>\n";
echo "</form>\n";
echo "</body>\n";
echo "</html>\n";




?>




