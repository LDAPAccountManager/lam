<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2004 - 2010  Roland Gruber

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
* confmodules lets the user select the account modules
*
* @package configuration
* @author Roland Gruber
*/


/** Access to config functions */
include_once('../../lib/config.inc');
/** Access to module lists */
include_once('../../lib/modules.inc');

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
		// go to general page
		elseif (isset($_POST['generalSettingsButton'])) {
			metaRefresh("confmain.php");
			exit;
		}
		// go to module settings page
		elseif (isset($_POST['moduleSettings'])) {
			metaRefresh("moduleSettings.php");
			exit;
		}
	}
}

$types = $conf->get_ActiveTypes();

echo $_SESSION['header'];

echo "<title>" . _("LDAP Account Manager Configuration") . "</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"../../graphics/favicon.ico\">\n";
for ($i = 0; $i < sizeof($types); $i++){
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/type_" . $types[$i] . ".css\">\n";
}
echo "</head><body>\n";
// include all JavaScript files
$jsDirName = dirname(__FILE__) . '/../lib';
$jsDir = dir($jsDirName);
while ($jsEntry = $jsDir->read()) {
	if (substr($jsEntry, strlen($jsEntry) - 3, 3) != '.js') continue;
	echo "<script type=\"text/javascript\" src=\"../lib/" . $jsEntry . "\"></script>\n";
}

?>
		<table border=0 width="100%" class="lamHeader">
			<tr>
				<td align="left" height="30">
					<a class="lamHeader" href="http://www.ldap-account-manager.org/" target="new_window">&nbsp;<img src="../../graphics/logo32.png" width=24 height=24 class="align-middle" alt="LDAP Account Manager">&nbsp;&nbsp;LDAP Account Manager</a>
				</td>
			</tr>
		</table>
		<br>
<?php

// print error messages
for ($i = 0; $i < sizeof($errorsToDisplay); $i++) call_user_func_array('StatusMessage', $errorsToDisplay[$i]);

echo ("<form action=\"confmodules.php\" method=\"post\">\n");
echo "<table border=0 width=\"100%\" style=\"border-collapse: collapse;\">\n";
echo "<tr valign=\"top\"><td style=\"border-bottom: 1px solid;padding:0px;\" colspan=2>";
// show tabs
echo "<table width=\"100%\" border=0 style=\"border-collapse: collapse;\">";
echo "<tr>\n";
	$buttonSpace = '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;';
	// general settings
	echo "<td class=\"settingsTab\">\n";
	echo "<table class=\"settingsTab\" width=\"100%\">\n";
	echo "<tr><td onclick=\"document.getElementsByName('generalSettingsButton')[0].click();\"";
	echo " align=\"center\">\n";
	$buttonStyle = 'background-image: url(../../graphics/bigTools.png);';
	echo "<input style=\"" . $buttonStyle . "\" name=\"generalSettingsButton\" type=\"submit\" value=\"" . $buttonSpace . _('General settings') . "\"";
	echo ">\n";
	echo "</td></tr></table>\n";
	echo '</td>';
	// account types
	echo "<td class=\"settingsTab\">\n";
	echo "<table class=\"settingsTab\" width=\"100%\">\n";
	echo "<tr><td onclick=\"document.getElementsByName('edittypes')[0].click();\"";
	echo " align=\"center\">\n";
	$buttonStyle = 'background-image: url(../../graphics/gear.png);';
	echo "<input style=\"" . $buttonStyle . "\" name=\"edittypes\" type=\"submit\" value=\"" . $buttonSpace . _('Account types') . "\"";
	echo ">\n";
	echo "</td></tr></table>\n";
	echo '</td>';
	// module selection
	echo "<td class=\"settingsTab\">\n";
	echo "<table class=\"settingsTab\" width=\"100%\">\n";
	echo "<tr><td class=\"settingsActiveTab\" onclick=\"document.getElementsByName('editmodules')[0].click();\"";
	echo " align=\"center\">\n";
	$buttonStyle = 'background-image: url(../../graphics/modules.png);';
	echo "<input style=\"" . $buttonStyle . "\" name=\"editmodules\" type=\"submit\" value=\"" . $buttonSpace . _('Modules') . "\"";
	echo ">\n";
	echo "</td></tr></table>\n";
	echo '</td>';
	// module settings
	echo "<td class=\"settingsTab\">\n";
	echo "<table class=\"settingsTab\" width=\"100%\">\n";
	echo "<tr><td onclick=\"document.getElementsByName('moduleSettings')[0].click();\"";
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
	echo "<td class=\"settingsTab\">\n";
	echo "<table class=\"settingsTab\" width=\"100%\">\n";
	echo "<tr><td onclick=\"document.getElementsByName('saveSettings')[0].click();\"";
	echo " align=\"center\">\n";
	$buttonStyle = 'background-image: url(../../graphics/pass.png);';
	echo "<input style=\"" . $buttonStyle . "\" name=\"saveSettings\" type=\"submit\" value=\"" . $buttonSpace . _('Save') . "\"";
	echo ">\n";
	echo "</td></tr></table>\n";
	echo '</td>';
	// cancel button
	echo "<td class=\"settingsTab\">\n";
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


$account_list = array();
for ($i = 0; $i < sizeof($types); $i++) {
	$account_list[] = array($types[$i], getTypeAlias($types[$i]));
}

for ($i = 0; $i < sizeof($account_list); $i++) {
	config_showAccountModules($account_list[$i][0], $account_list[$i][1]);
}


echo "<p>\n";
echo "(*) " . _("Base module");
// help link
echo "&nbsp;";
printHelpLink(getHelp('', '237'), '237');
echo "<br><br><br><br><br></p>\n";

echo '</td></tr></table>';

echo "<input type=\"hidden\" name=\"postAvailable\" value=\"yes\">\n";

echo "</form>\n";
echo "</body>\n";
echo "</html>\n";


/**
* Displays the module selection boxes and checks if dependencies are fulfilled.
*
* @param string $scope account type
* @param string $title title for module selection (e.g. "User modules")
*/
function config_showAccountModules($scope, $title) {
	$conf = &$_SESSION['conf_config'];
	$typeSettings = $conf->get_typeSettings();
	// account modules
	$available = getAvailableModules($scope);
	$selected = $typeSettings['modules_' . $scope];
	if (isset($selected) && ($selected != '')) {
		$selected = explode(',', $selected);
	}
	else {
		$selected = array();
	}
	$sortedAvailable = array();
	for ($i = 0; $i < sizeof($available); $i++) {
		$sortedAvailable[$available[$i]] = getModuleAlias($available[$i], $scope);
	}
	natcasesort($sortedAvailable);

	// show account modules
	$icon = '<img alt="' . $scope . '" src="../../graphics/' . $scope . '.png">&nbsp;';
	echo "<fieldset class=\"" . $scope . "edit\"><legend>$icon<b>" . $title . "</b></legend><br>\n";
	echo "<table border=0 width=\"100%\">\n";
		// select boxes
		echo "<tr>\n";
			echo "<td width=\"5%\"></td>\n";
			echo "<td width=\"40%\">\n";
				echo "<fieldset class=\"" . $scope . "edit\">\n";
					echo "<legend>" . _("Selected modules") . "</legend><br>\n";
					echo "<select class=\"" . $scope . "edit\" name=\"" . $scope . "_selected[]\" size=5 multiple>\n";
						for ($i = 0; $i < sizeof($selected); $i++) {
							if (in_array($selected[$i], $available)) {  // selected modules must be available
								if (is_base_module($selected[$i], $scope)) {  // mark base modules
									echo "<option value=\"" . $selected[$i] . "\">";
									echo getModuleAlias($selected[$i], $scope) . " (" . $selected[$i] .  ")(*)";
									echo "</option>\n";
								}
								else {
									echo "<option value=\"" . $selected[$i] . "\">";
									echo getModuleAlias($selected[$i], $scope) . " (" . $selected[$i] .  ")";
									echo "</option>\n";
								}
							}
						}
					echo "</select>\n";
				echo "</fieldset>\n";
			echo "</td>\n";
			echo "<td width=\"10%\" align=\"center\">\n";
				echo "<p>";
					echo "<input type=submit title=\"" . _('Add') . "\" value=\" \" name=\"" . $scope . "_add\"" .
					"style=\"background-image: url(../../graphics/back.gif);background-position: 2px center;background-repeat: no-repeat;width:24px;height:24px;background-color:transparent\">";
					echo "<br>";
					echo "<input type=submit title=\"" . _('Remove') . "\" value=\" \" name=\"" . $scope . "_remove\"" .
					"style=\"background-image: url(../../graphics/forward.gif);background-position: 2px center;background-repeat: no-repeat;width:24px;height:24px;background-color:transparent\">";
				echo "</p>\n";
			echo "</td>\n";
			echo "<td width=\"40%\">\n";
				echo "<fieldset class=\"" . $scope . "edit\">\n";
					echo "<legend>" . _("Available modules") . "</legend><br>\n";
					echo "<select class=\"" . $scope . "edit\" name=\"" . $scope . "_available[]\" size=5 multiple>\n";
						foreach ($sortedAvailable as $key => $value) {
							if (! in_array($key, $selected)) {  // display non-selected modules
								if (is_base_module($key, $scope)) {  // mark base modules
									echo "<option value=\"" . $key . "\">";
									echo $value . " (" . $key .  ")(*)";
									echo "</option>\n";
								}
								else {
									echo "<option value=\"" . $key . "\">";
									echo $value . " (" . $key .  ")";
									echo "</option>\n";
								}
							}
						}
					echo "</select>\n";
				echo "</fieldset>\n";
			echo "</td>\n";
			echo "<td width=\"5%\"></td>\n";
		echo "</tr>\n";
	echo "</table>\n";
	
	echo "</fieldset>\n";
	
	echo "<br>\n";
}

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
	for ($t = 0; $t < sizeof($accountTypes); $t++) {
		$scope = $accountTypes[$t];
		$available = getAvailableModules($scope);
		$selected_temp = $typeSettings['modules_' . $scope];
		if (isset($selected_temp)) $selected_temp = explode(',', $selected_temp);
		$selected = array();
		// only use available modules as selected
		for ($i = 0; $i < sizeof($selected_temp); $i++) {
			if (in_array($selected_temp[$i], $available)) {
				$selected[] = $selected_temp[$i];
			}
		}
		// remove modules from selection
		if (isset($_POST[$scope . '_selected']) && isset($_POST[$scope . '_remove'])) {
			$new_selected = array();
			for ($i = 0; $i < sizeof($selected); $i++) {
				if (! in_array($selected[$i], $_POST[$scope . '_selected'])) $new_selected[] = $selected[$i];
			}
			$selected = $new_selected;
			$typeSettings['modules_' . $scope] = implode(',', $selected);
		}
		// add modules to selection
		elseif (isset($_POST[$scope . '_available']) && isset($_POST[$scope . '_add'])) {
			$new_selected = $selected;
			for ($i = 0; $i < sizeof($_POST[$scope . '_available']); $i++) {
				if (! in_array($_POST[$scope . '_available'][$i], $selected)) $new_selected[] = $_POST[$scope . '_available'][$i];
			}
			$selected = $new_selected;
			$typeSettings['modules_' . $scope] = implode(',', $selected);
		}
		// check dependencies
		$depends = check_module_depends($selected, getModulesDependencies($scope));
		if ($depends != false) {
			for ($i = 0; $i < sizeof($depends); $i++) {
				$errors[] = array('ERROR', getTypeAlias($scope), _("Unsolved dependency:") . ' ' .
					$depends[$i][0] . " (" . $depends[$i][1] . ")");
			}
		}
		// check conflicts
		$conflicts = check_module_conflicts($selected, getModulesDependencies($scope));
		if ($conflicts != false) {
			for ($i = 0; $i < sizeof($conflicts); $i++) {
				$errors[] = array('ERROR', getTypeAlias($scope), _("Conflicting module:") . ' ' .
					$conflicts[$i][0] . " (" . $conflicts[$i][1] . ")");
			}
		}
		// check for base module
		$baseCount = 0;
		for ($i = 0; $i < sizeof($selected); $i++) {
			if (is_base_module($selected[$i], $scope)) {
				$baseCount++;
			}
		}
		if ($baseCount != 1) {
			$errors[] = array('ERROR', getTypeAlias($scope), _("No or more than one base module selected!"));
		}	
	}
	$conf->set_typeSettings($typeSettings);

	return $errors;
}

?>




