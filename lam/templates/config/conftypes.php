<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2004 - 2013  Roland Gruber

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
if (isset($_POST['saveSettings']) || isset($_POST['editmodules'])
	|| isset($_POST['edittypes']) || isset($_POST['generalSettingsButton'])
	|| isset($_POST['moduleSettings'])) {
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

$typeSettings = $conf->get_typeSettings();
$allTypes = getTypes();
$activeTypes = $conf->get_ActiveTypes();
$availableTypes = array();
for ($i = 0; $i < sizeof($allTypes); $i++) {
	if (!in_array($allTypes[$i], $activeTypes)) {
		$availableTypes[$allTypes[$i]] = getTypeAlias($allTypes[$i]);
	}
}
natcasesort($availableTypes);

echo $_SESSION['header'];

echo "<title>" . _("LDAP Account Manager Configuration") . "</title>\n";

// include all CSS files
$cssDirName = dirname(__FILE__) . '/../../style';
$cssDir = dir($cssDirName);
$cssFiles = array();
$cssEntry = $cssDir->read();
while ($cssEntry !== false) {
	if (substr($cssEntry, strlen($cssEntry) - 4, 4) == '.css') {
		$cssFiles[] = $cssEntry;
	}
	$cssEntry = $cssDir->read();
}
sort($cssFiles);
foreach ($cssFiles as $cssEntry) {
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/" . $cssEntry . "\">\n";
}

echo "<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"../../graphics/favicon.ico\">\n";
echo "</head><body>\n";
// include all JavaScript files
$jsDirName = dirname(__FILE__) . '/../lib';
$jsDir = dir($jsDirName);
$jsFiles = array();
while ($jsEntry = $jsDir->read()) {
	if (substr($jsEntry, strlen($jsEntry) - 3, 3) != '.js') continue;
	$jsFiles[] = $jsEntry;
}
sort($jsFiles);
foreach ($jsFiles as $jsEntry) {
	echo "<script type=\"text/javascript\" src=\"../lib/" . $jsEntry . "\"></script>\n";
}

?>
		<table border=0 width="100%" class="lamHeader ui-corner-all">
			<tr>
				<td align="left" height="30">
					<a class="lamHeader" href="http://www.ldap-account-manager.org/" target="new_window">&nbsp;<img src="../../graphics/logo32.png" width=24 height=24 class="align-middle" alt="LDAP Account Manager">&nbsp;&nbsp;LDAP Account Manager</a>
				</td>
				<td align="right">
					<?php echo _('Server profile') . ': ' . $conf->getName(); ?>
					&nbsp;&nbsp;
				</td>
			</tr>
		</table>
		<br>
<?php

// print error messages
for ($i = 0; $i < sizeof($errorsToDisplay); $i++) call_user_func_array('StatusMessage', $errorsToDisplay[$i]);

echo ("<form action=\"conftypes.php\" method=\"post\">\n");

// hidden submit buttons which are clicked by tabs
echo "<div style=\"display: none;\">\n";
	echo "<input name=\"generalSettingsButton\" type=\"submit\" value=\" \">";
	echo "<input name=\"edittypes\" type=\"submit\" value=\" \">";
	echo "<input name=\"editmodules\" type=\"submit\" value=\" \">";
	echo "<input name=\"moduleSettings\" type=\"submit\" value=\" \">";
echo "</div>\n";
	
// tabs
echo '<div class="ui-tabs ui-widget ui-widget-content ui-corner-all">';

echo '<ul class="ui-tabs-nav ui-helper-reset ui-helper-clearfix ui-widget-header ui-corner-all">';
echo '<li id="generalSettingsButton" class="ui-state-default ui-corner-top" onmouseover="jQuery(this).addClass(\'tabs-hover\');" onmouseout="jQuery(this).removeClass(\'tabs-hover\');">';
	echo '<a href="#" onclick="document.getElementsByName(\'generalSettingsButton\')[0].click();"><img src="../../graphics/tools.png" alt=""> ';
	echo _('General settings') . '</a>';
echo '</li>';
echo '<li id="edittypes" class="ui-state-default ui-corner-top">';
	echo '<a href="#" onclick="document.getElementsByName(\'edittypes\')[0].click();"><img src="../../graphics/gear.png" alt=""> ';
	echo _('Account types') . '</a>';
echo '</li>';
echo '<li id="editmodules" class="ui-state-default ui-corner-top" onmouseover="jQuery(this).addClass(\'tabs-hover\');" onmouseout="jQuery(this).removeClass(\'tabs-hover\');">';
	echo '<a href="#" onclick="document.getElementsByName(\'editmodules\')[0].click();"><img src="../../graphics/modules.png" alt=""> ';
	echo _('Modules') . '</a>';
echo '</li>';
echo '<li id="moduleSettings" class="ui-state-default ui-corner-top" onmouseover="jQuery(this).addClass(\'tabs-hover\');" onmouseout="jQuery(this).removeClass(\'tabs-hover\');">';
	echo '<a href="#" onclick="document.getElementsByName(\'moduleSettings\')[0].click();"><img src="../../graphics/modules.png" alt=""> ';
	echo _('Module settings') . '</a>';
echo '</li>';
echo '</ul>';

?>

<script type="text/javascript">
jQuery(document).ready(function() {
	jQuery('#edittypes').addClass('ui-tabs-selected');
	jQuery('#edittypes').addClass('ui-state-active');
	jQuery('#edittypes').addClass('user-bright');
});
</script>

<div class="ui-tabs-panel ui-widget-content ui-corner-bottom user-bright">
<?php

$container = new htmlTable();

// show available types
if (sizeof($availableTypes) > 0) {
	$container->addElement(new htmlSubTitle(_("Available account types")), true);
	$availableContainer = new htmlTable();
	foreach ($availableTypes as $key => $value) {
		$availableContainer->addElement(new htmlImage('../../graphics/' . $key . '.png'));
		$availableContainer->addElement(new htmlOutputText($value));
		$availableContainer->addElement(new htmlSpacer('10px', null));
		$availableContainer->addElement(new htmlOutputText(getTypeDescription($key)));
		$button = new htmlButton('add_' . $key, 'add.png', true);
		$button->setTitle(_("Add"));
		$availableContainer->addElement($button, true);
	}
	$availableContainer->addElement(new htmlSpacer(null, '20px'), true);
	$container->addElement($availableContainer, true);
}

// show active types
if (sizeof($activeTypes) > 0) {
	$container->addElement(new htmlSubTitle(_("Active account types")), true);
	$activeContainer = new htmlTable();
	for ($i = 0; $i < sizeof($activeTypes); $i++) {
		// title
		$activeContainer->addElement(new htmlImage('../../graphics/' . $activeTypes[$i] . '.png'));
		$titleText = new htmlOutputText(getTypeAlias($activeTypes[$i]));
		$titleText->setIsBold(true);
		$activeContainer->addElement($titleText);
		$activeContainer->addElement(new htmlSpacer('10px', null));
		$activeContainer->addElement(new htmlOutputText(getTypeDescription($activeTypes[$i])), true);
		// LDAP suffix
		$suffixText = new htmlOutputText(_("LDAP suffix"));
		$suffixText->colspan = 2;
		$activeContainer->addElement($suffixText);
		$activeContainer->addElement(new htmlSpacer('10px', null));
		$suffixInput = new htmlInputField('suffix_' . $activeTypes[$i], $typeSettings['suffix_' . $activeTypes[$i]]);
		$suffixInput->setFieldSize(40);
		$activeContainer->addElement($suffixInput);
		$activeContainer->addElement(new htmlHelpLink('202'));
		$activeContainer->addElement(new htmlSpacer('10px', null));
		// LDAP filter
		$filter = '';
		if (isset($typeSettings['filter_' . $activeTypes[$i]])) {
			$filter = $typeSettings['filter_' . $activeTypes[$i]];
		}
		$filterText = new htmlOutputText(_("Additional LDAP filter"));
		$filterText->colspan = 2;
		$activeContainer->addElement($filterText);
		$activeContainer->addElement(new htmlSpacer('10px', null));
		$filterInput = new htmlInputField('filter_' . $activeTypes[$i], $filter);
		$filterInput->setFieldSize(40);
		$activeContainer->addElement($filterInput);
		$activeContainer->addElement(new htmlHelpLink('260'));
		$activeContainer->addNewLine();
		// list attributes
		if (isset($typeSettings['attr_' . $activeTypes[$i]])) {
			$attributes = $typeSettings['attr_' . $activeTypes[$i]];
		}
		else {
			$attributes = getDefaultListAttributes($activeTypes[$i]);
		}
		$attrsText = new htmlOutputText(_("List attributes"));
		$attrsText->colspan = 2;
		$activeContainer->addElement($attrsText);
		$activeContainer->addElement(new htmlSpacer('10px', null));
		$attrsInput = new htmlInputField('attr_' . $activeTypes[$i], $attributes);
		$attrsInput->setFieldSize(40);
		$activeContainer->addElement($attrsInput);
		$activeContainer->addElement(new htmlHelpLink('206'));
		$activeContainer->addElement(new htmlSpacer('10px', null));
		// hidden type
		$hidden = false;
		if (isset($typeSettings['hidden_' . $activeTypes[$i]])) {
			$hidden = $typeSettings['hidden_' . $activeTypes[$i]];
		}
		$hiddenText = new htmlOutputText(_('Hidden'));
		$hiddenText->colspan = 2;
		$activeContainer->addElement($hiddenText);
		$activeContainer->addElement(new htmlSpacer('10px', null));
		$activeContainer->addElement(new htmlInputCheckbox('hidden_' . $activeTypes[$i], $hidden));
		$activeContainer->addElement(new htmlHelpLink('261'));
		$activeContainer->addNewLine();
		// delete button
		$delButton = new htmlButton('rem_'. $activeTypes[$i], _("Remove this account type"));
		$delButton->colspan = 5;
		$delButton->setIconClass('deleteButton');
		$activeContainer->addElement($delButton, true); //del.png
		
		$activeContainer->addElement(new htmlSpacer(null, '40px'), true);
	}
	$container->addElement($activeContainer, true);
}

$tabindex = 1;
parseHtml(null, $container, array(), false, $tabindex, 'user');

echo "<input type=\"hidden\" name=\"postAvailable\" value=\"yes\">\n";

echo "</div></div>";

$buttonContainer = new htmlTable();
$buttonContainer->addElement(new htmlSpacer(null, '10px'), true);
$saveButton = new htmlButton('saveSettings', _('Save'));
$saveButton->setIconClass('saveButton');
$buttonContainer->addElement($saveButton);
$cancelButton = new htmlButton('cancelSettings', _('Cancel'));
$cancelButton->setIconClass('cancelButton');
$buttonContainer->addElement($cancelButton, true);
$buttonContainer->addElement(new htmlSpacer(null, '10px'), true);
parseHtml(null, $buttonContainer, array(), false, $tabindex, 'user');

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
		// set attributes
		elseif (substr($key, 0, 5) == "attr_") {
			$typeSettings[$key] = $_POST[$key];
			$type = substr($postKeys[$i], 5);
			if (!is_string($_POST[$key]) || !preg_match("/^((#[^:;]+)|([^:;]*:[^:;]+))(;((#[^:;]+)|([^:;]*:[^:;]+)))*$/", $_POST[$key])) {
				$errors[] = array("ERROR", _("List attributes are invalid!"), getTypeAlias($type));
			}
		}
		// set filter
		elseif (substr($key, 0, 7) == "filter_") {
			$typeSettings[$key] = $_POST[$key];
		}
	}
	// set hidden
	for ($i = 0; $i < sizeof($accountTypes); $i++) {
		$key = "hidden_" . $accountTypes[$i];
		if (isset($_POST[$key]) && ($_POST[$key] == 'on')) {
			$typeSettings[$key] = true;
		}
		else {
			$typeSettings[$key] = false;
		}
	}
	// save input
	$conf->set_typeSettings($typeSettings);
	$conf->set_ActiveTypes($accountTypes);
	return $errors;
}

?>




