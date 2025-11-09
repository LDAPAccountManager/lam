<?php

namespace LAM\CONFIG;

use htmlJavaScript;
use LAM\TYPES\TypeManager;
use LAMConfig;
use moduleCache;
use htmlSpacer;
use htmlTable;
use htmlButton;
use htmlResponsiveRow;
use htmlSubTitle;
use function LAM\TYPES\getTypes;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2009 - 2025  Roland Gruber

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
include_once(__DIR__ . '/../../lib/config.inc');
/** Access to account types */
include_once(__DIR__ . '/../../lib/types.inc');
/** common functions */
include_once __DIR__ . '/../../lib/configPages.inc';

// start session
if (isFileBasedSession()) {
	session_save_path("../../sess");
}
lam_start_session();

setlanguage();

// check if config is set
// if not: load login page
if (!isset($_SESSION['conf_config'])) {
	/** go back to login if password is invalid */
	require(__DIR__ . '/conflogin.php');
	exit;
}

// check if user canceled editing
if (isset($_POST['cancelSettings'])) {
	metaRefresh("../login.php");
	exit;
}

$conf = &$_SESSION['conf_config'];
if (!($conf instanceof LAMConfig)) {
    die();
}

$errorsToDisplay = checkInput();

// check if button was pressed and if we have to save the settings or go to another tab
if ((isset($_POST['saveSettings']) || isset($_POST['editmodules'])
		|| isset($_POST['edittypes']) || isset($_POST['generalSettingsButton'])
		|| isset($_POST['moduleSettings']) || isset($_POST['jobs']))
    && (count($errorsToDisplay) === 0)) {
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
	// go to jobs page
    elseif (isset($_POST['jobs'])) {
		metaRefresh("jobs.php");
		exit;
	}
}

echo $_SESSION['header'];
printHeaderContents(_("LDAP Account Manager Configuration"), '../..');
echo "</head><body>\n";
printJsIncludes('../..');
printConfigurationPageHeaderBar($conf);

// print error messages
for ($i = 0; $i < count($errorsToDisplay); $i++) {
	call_user_func_array(StatusMessage(...), $errorsToDisplay[$i]);
}

echo "<form id=\"inputForm\" action=\"moduleSettings.php\" method=\"post\" autocomplete=\"off\" onSubmit=\"window.lam.utility.saveScrollPosition('inputForm')\" novalidate=\"novalidate\">\n";

printConfigurationPageTabs(ConfigurationPageTab::MODULE_SETTINGS);

?>
    <input type="text" name="hiddenPreventAutocomplete" autocomplete="false" class="hidden" value="">
    <input type="password" name="hiddenPreventAutocompletePwd1" autocomplete="false" class="hidden" value="123">
    <input type="password" name="hiddenPreventAutocompletePwd2" autocomplete="false" class="hidden" value="321">
	<?php


// module settings
$typeManager = new TypeManager($conf);
$types = $typeManager->getConfiguredTypes();

// get list of scopes of modules
$scopes = [];
foreach ($types as $type) {
	$mods = $conf->get_AccountModules($type->getId());
	for ($i = 0; $i < count($mods); $i++) {
		$scopes[$mods[$i]][] = $type->getId();
	}
}

// get module options
$options = getConfigOptions($scopes);
// get current setting
$old_options = $conf->get_moduleSettings();


// display module boxes
$modules = array_keys($options);
$_SESSION['conf_types'] = [];
for ($i = 0; $i < count($modules); $i++) {
	if (empty($options[$modules[$i]])) {
		continue;
	}
	$module = moduleCache::getModule($modules[$i], null);
    if ($module === null) {
        continue;
    }
	$iconImage = $module->getIcon();
	if (($iconImage != null) && !(str_starts_with($iconImage, 'http')) && !(str_starts_with($iconImage, '/'))) {
		$iconImage = '../../graphics/' . $iconImage;
	}
	$row = new htmlResponsiveRow();
	$row->add(new htmlSubTitle($module->get_alias(), $iconImage, null, true));
	if (is_array($options[$modules[$i]])) {
		foreach ($options[$modules[$i]] as $option) {
			$row->add($option);
		}
	}
	else {
		$row->add($options[$modules[$i]]);
	}
	$configTypes = parseHtml($modules[$i], $row, $old_options, false, null);
	$_SESSION['conf_types'] = array_merge($configTypes, $_SESSION['conf_types']);
	echo "<br>";
}

echo "<input type=\"hidden\" name=\"postAvailable\" value=\"yes\">\n";

echo "</div></div>";

$buttonContainer = new htmlTable();
$buttonContainer->addElement(new htmlSpacer(null, '10px'), true);
$saveButton = new htmlButton('saveSettings', _('Save'));
$saveButton->setCSSClasses(['lam-primary']);
$buttonContainer->addElement($saveButton);
$cancelButton = new htmlButton('cancelSettings', _('Cancel'));
$buttonContainer->addElement($cancelButton, true);
$buttonContainer->addElement(new htmlSpacer(null, '10px'), true);

if (empty($errorsToDisplay) && isset($_POST['scrollPositionTop']) && get_preg($_POST['scrollPositionTop'], 'digit')
        && isset($_POST['scrollPositionLeft']) && get_preg($_POST['scrollPositionLeft'], 'digit')) {
	// scroll to last position
	$buttonContainer->addElement(new htmlJavaScript('window.lam.utility.restoreScrollPosition(' . $_POST['scrollPositionTop'] . ', ' . $_POST['scrollPositionLeft'] . ')'));
}

parseHtml(null, $buttonContainer, [], false, null);

echo "</form>\n";
echo "</body>\n";
echo "</html>\n";


/**
 * Checks user input and saves the entered settings.
 *
 * @return array<mixed> list of errors
 */
function checkInput(): array {
	if (!isset($_POST['postAvailable'])) {
		return [];
	}
	$conf = &$_SESSION['conf_config'];
    if (!($conf instanceof LAMConfig)) {
        return [];
    }
	$typeManager = new TypeManager($conf);
	$types = $typeManager->getConfiguredTypes();

	// check module options
	// create option array to check and save
	$options = extractConfigOptionsFromPOST($_SESSION['conf_types']);

	// get list of scopes of modules
	$scopes = [];
	foreach ($types as $type) {
		$mods = $conf->get_AccountModules($type->getId());
		for ($i = 0; $i < count($mods); $i++) {
			$scopes[$mods[$i]][] = $type->getId();
		}
	}
	// check options
	$errors = checkConfigOptions($scopes, $options);
	$conf->set_moduleSettings($options);
	return $errors;
}
