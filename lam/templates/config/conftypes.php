<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2004 - 2009  Roland Gruber

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
include_once('../../lib/config.inc');
/** Access to account types */
include_once('../../lib/types.inc');

// start session
if (strtolower(session_module_name()) == 'files') {
	session_save_path("../../sess");
}
@session_start();

setlanguage();

// check if config is set
// if not: load login page
if (!isset($_SESSION['conf_config'])) {
	/** go back to login if password is invalid */
	require('conflogin.php');
	exit;
}

// check if user canceled editing
if (isset($_POST['cancelSettings'])) {
	metaRefresh("../login.php");
	exit;
}

$conf = &$_SESSION['conf_config'];

$errorsToDisplay = checkInput();

// check if button was pressed and if we have to save the settings or go to another tab
if (isset($_POST['saveSettings']) || isset($_POST['editmodules']) || isset($_POST['edittypes']) || isset($_POST['generalSettingsButton'])) {
	if (sizeof($errorsToDisplay) == 0) {
		// check if all types have modules
		$activeTypes = $conf->get_ActiveTypes();
		for ($i = 0; $i < sizeof($activeTypes); $i++) {
			$selectedModules = $conf->get_AccountModules($activeTypes[$i]);
			if (sizeof($selectedModules) == 0) {
				// go to module selection
				metaRefresh("confmodules.php");
				exit;
			}
		}
		// go to final page
		if (isset($_POST['saveSettings'])) {
			metaRefresh("confsave.php");
			exit;
		}
		// go to modules page
		elseif (isset($_POST['editmodules'])) {
			metaRefresh("confmodules.php");
			exit;
		}
		// go to types page
		elseif (isset($_POST['generalSettingsButton'])) {
			metaRefresh("confmain.php");
			exit;
		}
	}
}

$typeSettings = $conf->get_typeSettings();
$allTypes = getTypes();
$activeTypes = $conf->get_ActiveTypes();
$availableTypes = array();
for ($i = 0; $i < sizeof($allTypes); $i++) {
	if (!in_array($allTypes[$i], $activeTypes)) $availableTypes[] = $allTypes[$i];
}

echo $_SESSION['header'];

echo "<title>" . _("LDAP Account Manager Configuration") . "</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"../../graphics/favicon.ico\">\n";
for ($i = 0; $i < sizeof($allTypes); $i++){
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/type_" . $allTypes[$i] . ".css\">\n";
}
echo "</head><body>\n";
echo "<script type=\"text/javascript\" src=\"../wz_tooltip.js\"></script>\n";

echo ("<p align=\"center\"><a href=\"http://www.ldap-account-manager.org/\" target=\"new_window\">".
	"<img src=\"../../graphics/banner.jpg\" border=1 alt=\"LDAP Account Manager\"></a></p><hr>\n<p>&nbsp;</p>\n");

// print error messages
for ($i = 0; $i < sizeof($errorsToDisplay); $i++) call_user_func_array('StatusMessage', $errorsToDisplay[$i]);

echo ("<form action=\"conftypes.php\" method=\"post\">\n");
echo "<table border=0 width=\"100%\" style=\"border-collapse: collapse;\">\n";
echo "<tr valign=\"top\"><td style=\"border-bottom: 1px solid;padding:0px;\" colspan=2>";
// show tabs
echo "<table width=\"100%\" border=0 style=\"border-collapse: collapse;\">";
echo "<tr>\n";
	$buttonSpace = '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;';
	// general settings
	echo "<td style=\"padding-bottom:0px;padding-right:5px;padding-left:5px;padding-top:10px;\">\n";
	echo "<table class=\"settingsTab\" width=\"100%\">\n";
	echo "<tr><td onclick=\"document.getElementsByName('generalSettingsButton')[0].click();\"";
	echo " align=\"center\">\n";
	$buttonStyle = 'background-image: url(../../graphics/bigTools.png);';
	echo "<input style=\"" . $buttonStyle . "\" name=\"generalSettingsButton\" type=\"submit\" value=\"" . $buttonSpace . _('General settings') . "\"";
	echo ">\n";
	echo "</td></tr></table>\n";
	echo '</td>';
	// account types
	echo "<td style=\"padding-bottom:0px;padding-right:5px;padding-left:5px;padding-top:10px;\">\n";
	echo "<table class=\"settingsTab\" width=\"100%\">\n";
	echo "<tr><td class=\"settingsActiveTab\" onclick=\"document.getElementsByName('edittypes')[0].click();\"";
	echo " align=\"center\">\n";
	$buttonStyle = 'background-image: url(../../graphics/gear.png);';
	echo "<input style=\"" . $buttonStyle . "\" name=\"edittypes\" type=\"submit\" value=\"" . $buttonSpace . _('Account types') . "\"";
	echo ">\n";
	echo "</td></tr></table>\n";
	echo '</td>';
	// module selection
	echo "<td style=\"padding-bottom:0px;padding-right:5px;padding-left:5px;padding-top:10px;\">\n";
	echo "<table class=\"settingsTab\" width=\"100%\">\n";
	echo "<tr><td onclick=\"document.getElementsByName('editmodules')[0].click();\"";
	echo " align=\"center\">\n";
	$buttonStyle = 'background-image: url(../../graphics/modules.png);';
	echo "<input style=\"" . $buttonStyle . "\" name=\"editmodules\" type=\"submit\" value=\"" . $buttonSpace . _('Modules') . "\"";
	echo ">\n";
	echo "</td></tr></table>\n";
	echo '</td>';
	echo "<td width=\"100%\">&nbsp;</td>";
	// save button
	echo "<td style=\"padding-bottom:0px;padding-right:5px;padding-left:5px;padding-top:10px;\">\n";
	echo "<table class=\"settingsTab\" width=\"100%\">\n";
	echo "<tr><td onclick=\"document.getElementsByName('saveSettings')[0].click();\"";
	echo " align=\"center\">\n";
	$buttonStyle = 'background-image: url(../../graphics/pass.png);';
	echo "<input style=\"" . $buttonStyle . "\" name=\"saveSettings\" type=\"submit\" value=\"" . $buttonSpace . _('Save') . "\"";
	echo ">\n";
	echo "</td></tr></table>\n";
	echo '</td>';
	// cancel button
	echo "<td style=\"padding-bottom:0px;padding-right:5px;padding-left:5px;padding-top:10px;\">\n";
	echo "<table class=\"settingsTab\" width=\"100%\">\n";
	echo "<tr><td onclick=\"document.getElementsByName('cancelSettings')[0].click();\"";
	echo " align=\"center\">\n";
	$buttonStyle = 'background-image: url(../../graphics/fail.png);';
	echo "<input style=\"" . $buttonStyle . "\" name=\"cancelSettings\" type=\"submit\" value=\"" . $buttonSpace . _('Cancel') . "\"";
	echo ">\n";
	echo "</td></tr></table>\n";
	echo '</td>';
	echo "</tr></table>\n";		
// end tabs
echo "</td></tr>\n";

echo "<tr><td><br><br>\n";

// show available types
if (sizeof($availableTypes) > 0) {
	echo "<fieldset><legend><b>" . _("Available account types") . "</b></legend>\n";
	echo "<table>\n";
	for ($i = 0; $i < sizeof($availableTypes); $i++) {
		$icon = '<img alt="' . $availableTypes[$i] . '" src="../../graphics/' . $availableTypes[$i] . '.png">&nbsp;';
		echo "<tr>\n";
			echo "<td>$icon<b>" . getTypeAlias($availableTypes[$i]) . ": </b></td>\n";
			echo "<td>" . getTypeDescription($availableTypes[$i]) . "</td>\n";
			echo "<td><input type=\"submit\" name=\"add_" . $availableTypes[$i] ."\" title=\"" . _("Add") . "\" value=\" \"" .
				" style=\"background-image: url(../../graphics/add.png);background-position: 2px center;background-repeat: no-repeat;width:24px;height:24px;background-color:transparent\"></td>\n";
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
		$icon = '<img alt="' . $activeTypes[$i] . '" src="../../graphics/' . $activeTypes[$i] . '.png">&nbsp;';
		echo "<legend>" . $icon . "<b>" . getTypeAlias($activeTypes[$i]) . ": </b>" . getTypeDescription($activeTypes[$i]) . " " .
			"<input type=\"submit\" name=\"rem_" . $activeTypes[$i] . "\" value=\" \" title=\"" . _("Remove this account type") . "\" " .
			"style=\"background-image: url(../../graphics/del.png);background-position: 2px center;background-repeat: no-repeat;width:24px;height:24px;background-color:transparent\">" .
			"</legend>";
		echo "<br>\n";
		echo "<table>\n";
		// LDAP suffix
		echo "<tr>\n";
			echo "<td>" . _("LDAP suffix") . "</td>\n";
			echo "<td><input type=\"text\" size=\"40\" name=\"suffix_" . $activeTypes[$i] . "\" value=\"" . $typeSettings['suffix_' . $activeTypes[$i]] . "\"></td>\n";
			echo "<td>";
			printHelpLink(getHelp('', '202'), '202');
			echo "</td>\n";
		echo "</tr>\n";
		// list attributes
		if (isset($typeSettings['attr_' . $activeTypes[$i]])) {
			$attributes = $typeSettings['attr_' . $activeTypes[$i]];
		}
		else {
			$attributes = getDefaultListAttributes($activeTypes[$i]);
		}
		echo "<tr>\n";
			echo "<td>" . _("List attributes") . "</td>\n";
			echo "<td><input type=\"text\" size=\"40\" name=\"attr_" . $activeTypes[$i] . "\" value=\"" . $attributes . "\"></td>\n";
			echo "<td>";
			printHelpLink(getHelp('', '206'), '206');
			echo "</td>\n";
		echo "</tr>\n";
		echo "</table>\n";
		echo "</fieldset><br>\n";
	}
	echo "</fieldset>\n";
	echo "<p><br><br></p>\n";
}

echo "<input type=\"hidden\" name=\"postAvailable\" value=\"yes\">\n";

echo "<p><br><br></p>\n";
echo '</td></tr></table>';
echo "</form>\n";
echo "</body>\n";
echo "</html>\n";


/**
 * Checks user input and saves the entered settings.
 *
 * @return array list of errors
 */
function checkInput() {
	if (!isset($_POST['postAvailable'])) {
		return array();
	}
	$errors = array();
	$conf = &$_SESSION['conf_config'];
	$typeSettings = $conf->get_typeSettings();
	$accountTypes = $conf->get_ActiveTypes();
	$postKeys = array_keys($_POST);
	for ($i = 0; $i < sizeof($postKeys); $i++) {
		$key = $postKeys[$i];
		// check if remove button was pressed
		if (substr($key, 0, 4) == "rem_") {
			$type = substr($key, 4);
			$accountTypes = array_flip($accountTypes);
			unset($accountTypes[$type]);
			$accountTypes = array_flip($accountTypes);
			$accountTypes = array_values($accountTypes);
		}
		// check if add button was pressed
		else if (substr($key, 0, 4) == "add_") {
			$type = substr($key, 4);
			$accountTypes[] = $type;
		}
		// set suffixes
		elseif (substr($key, 0, 7) == "suffix_") {
			$typeSettings[$key] = $_POST[$key];
			$type = substr($postKeys[$i], 7);
			if (strlen($_POST[$key]) < 1) {
				$errors[] = array("ERROR", _("LDAP Suffix is invalid!"), getTypeAlias($type));
			}
		}
		elseif (substr($key, 0, 5) == "attr_") {
			$typeSettings[$key] = $_POST[$key];
			$type = substr($postKeys[$i], 5);
			if (!is_string($_POST[$key]) || !preg_match("/^((#[^:;]+)|([^:;]*:[^:;]+))(;((#[^:;]+)|([^:;]*:[^:;]+)))*$/", $_POST[$key])) {
				$errors[] = array("ERROR", _("List attributes are invalid!"), getTypeAlias($type));
			}
		}
	}
	// save input
	$conf->set_typeSettings($typeSettings);
	$conf->set_ActiveTypes($accountTypes);
	return $errors;
}

?>




