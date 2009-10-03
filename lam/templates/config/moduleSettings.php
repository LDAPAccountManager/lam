<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2009  Roland Gruber

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
* Here the user can edit the module settings.
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
if (isset($_POST['saveSettings']) || isset($_POST['editmodules'])
	|| isset($_POST['edittypes']) || isset($_POST['generalSettingsButton'])
	|| isset($_POST['moduleSettings'])) {
	if (sizeof($errorsToDisplay) == 0) {
		// go to final page
		if (isset($_POST['saveSettings'])) {
			metaRefresh("confsave.php");
			exit;
		}
		// go to types page
		elseif (isset($_POST['edittypes'])) {
			metaRefresh("conftypes.php");
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

$allTypes = getTypes();

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

echo ("<form action=\"moduleSettings.php\" method=\"post\">\n");
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
	echo "<tr><td onclick=\"document.getElementsByName('edittypes')[0].click();\"";
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
	// module settings
	echo "<td style=\"padding-bottom:0px;padding-right:5px;padding-left:5px;padding-top:10px;\">\n";
	echo "<table class=\"settingsTab\" width=\"100%\">\n";
	echo "<tr><td class=\"settingsActiveTab\" onclick=\"document.getElementsByName('moduleSettings')[0].click();\"";
	echo " align=\"center\">\n";
	$buttonStyle = 'background-image: url(../../graphics/moduleSettings.png);';
	echo "<input style=\"" . $buttonStyle . "\" name=\"moduleSettings\" type=\"submit\" value=\"" . $buttonSpace . _('Module settings') . "\"";
	echo ">\n";
	echo "</td></tr></table>\n";
	echo '</td>';
	echo "<td width=\"100%\">&nbsp;</td>";
	// spacer
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

// module settings
$types = $conf->get_ActiveTypes();

// get list of scopes of modules
$scopes = array();
for ($m = 0; $m < sizeof($types); $m++) {
	$mods = $conf->get_AccountModules($types[$m]);
	for ($i = 0; $i < sizeof($mods); $i++) $scopes[$mods[$i]][] = $types[$m];
}

// get module options
$options = getConfigOptions($scopes);
// get current setting
$old_options = $conf->get_moduleSettings();


// display module boxes
$modules = array_keys($options);
$_SESSION['conf_types'] = array();
for ($i = 0; $i < sizeof($modules); $i++) {
	if (sizeof($options[$modules[$i]]) < 1) continue;
	echo "<fieldset>\n";
	$icon = '';
	$module = new $modules[$i]('none');
	$iconImage = $module->getIcon();
	if ($iconImage != null) {
		$icon = '<img align="middle" src="../../graphics/' . $iconImage . '" alt="' . $iconImage . '"> ';
	}
	echo "<legend>$icon<b>" . getModuleAlias($modules[$i], "none") . "</b></legend><br>\n";
	$configTypes = parseHtml($modules[$i], $options[$modules[$i]], $old_options, true, $tabindex, 'config');
	$_SESSION['conf_types'] = array_merge($configTypes, $_SESSION['conf_types']);
	echo "</fieldset>\n";
	echo "<br>";
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
	$conf = &$_SESSION['conf_config'];
	$types = $conf->get_ActiveTypes();
	
	// check module options
	// create option array to check and save
	$options = array();
	$opt_keys = array_keys($_SESSION['conf_types']);
	for ($i = 0; $i < sizeof($opt_keys); $i++) {
		$element = $opt_keys[$i];
		// text fields
		if ($_SESSION['conf_types'][$element] == "text") {
			$options[$element] = array($_POST[$element]);
		}
		// checkboxes
		elseif ($_SESSION['conf_types'][$element] == "checkbox") {
			if (isset($_POST[$element]) && ($_POST[$element] == "on")) $options[$element] = array('true');
			else $options[$element] = array('false');
		}
		// dropdownbox
		elseif ($_SESSION['conf_types'][$element] == "select") {
			$options[$element] = array($_POST[$element]);
		}
		// multiselect
		elseif ($_SESSION['conf_types'][$element] == "multiselect") {
			$options[$element] = $_POST[$element];  // value is already an array
		}
		// textarea
		elseif ($_SESSION['conf_types'][$element] == "textarea") {
			$options[$element] = explode("\r\n", $_POST[$element]);
		}
	}

	// get list of scopes of modules
	$scopes = array();
	for ($m = 0; $m < sizeof($types); $m++) {
		$mods = $conf->get_AccountModules($types[$m]);
		for ($i = 0; $i < sizeof($mods); $i++) $scopes[$mods[$i]][] = $types[$m];
	}
	// check options
	$errors = checkConfigOptions($scopes, $options);
	$conf->set_moduleSettings($options);
	return $errors;
}

?>




