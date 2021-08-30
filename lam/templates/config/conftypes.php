<?php
namespace LAM\CONFIG;
use \htmlTable;
use \htmlSubTitle;
use \htmlImage;
use \htmlOutputText;
use \htmlSpacer;
use \htmlButton;
use \htmlGroup;
use \htmlDiv;
use \htmlResponsiveInputCheckbox;
use \LAMConfig;
use \htmlResponsiveRow;
use \htmlResponsiveInputField;
/*
  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2004 - 2021  Roland Gruber

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
include_once '../../lib/config.inc';
/** Access to account types */
include_once '../../lib/types.inc';
/** common functions */
include_once '../../lib/configPages.inc';

// start session
if (strtolower(session_module_name()) == 'files') {
	session_save_path("../../sess");
}
lam_start_session();

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
	|| isset($_POST['moduleSettings']) || isset($_POST['jobs'])) {
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
		// go to jobs page
		elseif (isset($_POST['jobs'])) {
			metaRefresh("jobs.php");
			exit;
		}
	}
}

$typeSettings = $conf->get_typeSettings();
$allScopes = \LAM\TYPES\getTypes();
$typeManager = new \LAM\TYPES\TypeManager($conf);
$activeTypes = $typeManager->getConfiguredTypes();
$activeScopes = array();
foreach ($activeTypes as $activeType) {
	$activeScopes[] = $activeType->getScope();
}
$activeScopes = array_unique($activeScopes);
$availableScopes = array();
foreach ($allScopes as $scope) {
	$scopeObj = new $scope(null);
	if (!in_array($scope, $activeScopes) || $scopeObj->supportsMultipleConfigs()) {
		$availableScopes[] = $scopeObj;
	}
}
usort($availableScopes, '\LAM\CONFIG\compareTypesByAlias');

echo $_SESSION['header'];
printHeaderContents(_("LDAP Account Manager Configuration"), '../..');
echo "</head><body class=\"admin\">\n";
// include all JavaScript files
printJsIncludes('../..');
printConfigurationPageHeaderBar($conf);

// print error messages
for ($i = 0; $i < sizeof($errorsToDisplay); $i++) {
	call_user_func_array('StatusMessage', $errorsToDisplay[$i]);
}

echo "<form action=\"conftypes.php\" method=\"post\">\n";

printConfigurationPageTabs(ConfigurationPageTab::TYPES);

$tabindex = 1;
$row = new htmlResponsiveRow();

// show available types
if (sizeof($availableScopes) > 0) {
	$row->add(new htmlSubTitle(_("Available account types")), 12);
	foreach ($availableScopes as $availableScope) {
		$availableLabelGroup = new htmlGroup();
		$availableLabelGroup->addElement(new htmlImage('../../graphics/' . $availableScope->getIcon()));
		$availableLabelGroup->addElement(new htmlSpacer('0.5rem', null));
		$availableLabelGroup->addElement(new htmlOutputText($availableScope->getAlias()));
		$row->addField($availableLabelGroup);
		$availableDescriptionRow = new htmlResponsiveRow();
		$availableDescriptionRow->add(new htmlOutputText($availableScope->getDescription()), 10, 10, 10, 'responsiveField');
		$button = new htmlButton('add_' . $availableScope->getScope(), 'add.png', true);
		$button->setTitle(_("Add"));
		$button->setCSSClasses(array('size16'));
		$availableDescriptionRow->add($button, 2, 2, 2, 'responsiveField');
		$row->addField($availableDescriptionRow);
		$row->addVerticalSpacer('1rem');
	}
	$row->addVerticalSpacer('2rem');
}
parseHtml(null, $row, array(), false, $tabindex, 'user');

$container = new htmlResponsiveRow();

$_SESSION['conftypes_optionTypes'] = array();
// show active types
if (sizeof($activeTypes) > 0) {
	$container->add(new htmlSubTitle(_("Active account types")), 12);
	$index = 0;
	foreach ($activeTypes as $activeType) {
		// title
		$titleGroup = new htmlGroup();
		$titleGroup->addElement(new htmlImage('../../graphics/' . $activeType->getIcon()));
		$titleGroup->addElement(new htmlSpacer('0.5rem', null));
		$titleText = new htmlOutputText($activeType->getAlias());
		$titleText->setIsBold(true);
		$titleGroup->addElement($titleText);
		$container->addField($titleGroup);
		$descriptionRow = new htmlResponsiveRow();
		$descriptionRow->add(new htmlOutputText($activeType->getBaseType()->getDescription()), 12, 12, 9, 'responsiveField');
		$buttons = new htmlGroup();
		// move buttons
		if ($index > 0) {
			$upButton = new htmlButton('moveup_'. $activeType->getId(), 'up.gif', true);
			$upButton->setTitle(_("Up"));
			$upButton->setCSSClasses(array('size16'));
			$buttons->addElement($upButton);
		}
		if ($index < (sizeof($activeTypes) - 1)) {
			$upButton = new htmlButton('movedown_'. $activeType->getId(), 'down.gif', true);
			$upButton->setTitle(_("Down"));
			$upButton->setCSSClasses(array('size16'));
			$buttons->addElement($upButton);
		}
		// delete button
		$delButton = new htmlButton('rem_'. $activeType->getId(), 'del.png', true);
		$delButton->setTitle(_("Remove this account type"));
		$delButton->setCSSClasses(array('size16'));
		$buttons->addElement($delButton);
		$buttonsDiv = new htmlDiv(null, $buttons, array('text-right'));
		$descriptionRow->add($buttonsDiv, 12, 12, 3, 'responsiveLabel');
		$container->addField($descriptionRow);
		$container->addVerticalSpacer('0.5rem');
		// LDAP suffix
		$suffixInput = new htmlResponsiveInputField(_("LDAP suffix"), 'suffix_' . $activeType->getId(), $typeSettings['suffix_' . $activeType->getId()], '202');
		$container->add($suffixInput, 12);
		// list attributes
		if (isset($typeSettings['attr_' . $activeType->getId()])) {
			$attributes = $typeSettings['attr_' . $activeType->getId()];
		}
		else {
			$attributes = $activeType->getBaseType()->getDefaultListAttributes();
		}
		$attrsInput = new htmlResponsiveInputField(_("List attributes"), 'attr_' . $activeType->getId(), $attributes, '206');
		$attrsInput->setFieldMaxLength(1000);
		$container->add($attrsInput, 12);
		// custom label
		$customLabel = '';
		if (isset($typeSettings['customLabel_' . $activeType->getId()])) {
			$customLabel = $typeSettings['customLabel_' . $activeType->getId()];
		}
		$customLabelInput = new htmlResponsiveInputField(_('Custom label'), 'customLabel_' . $activeType->getId(), $customLabel, '264');
		$container->add($customLabelInput, 12);
		// LDAP filter
		$filter = '';
		if (isset($typeSettings['filter_' . $activeType->getId()])) {
			$filter = $typeSettings['filter_' . $activeType->getId()];
		}
		$filterInput = new htmlResponsiveInputField(_("Additional LDAP filter"), 'filter_' . $activeType->getId(), $filter, '260');
		$container->add($filterInput, 12);
		// type options
		$typeConfigOptions = $activeType->getBaseType()->get_configOptions();
		if (!empty($typeConfigOptions)) {
			foreach ($typeConfigOptions as $typeConfigOption) {
				$container->add($typeConfigOption, 12);
			}
			// save option types to session
			ob_start();
			$dummyIndex = 1;
			$typeConfigOptionTypes = parseHtml(null, $typeConfigOptions, array(), true, $dummyIndex, 'user');
			ob_end_clean();
			$_SESSION['conftypes_optionTypes'] = array_merge($_SESSION['conftypes_optionTypes'], $typeConfigOptionTypes);
		}
		// advanced options
		$advancedOptions = new htmlResponsiveRow();
		// read-only
		if (isLAMProVersion() && ($conf->getAccessLevel() == LAMConfig::ACCESS_ALL)) {
			$isReadOnly = false;
			if (isset($typeSettings['readOnly_' . $activeType->getId()])) {
				$isReadOnly = $typeSettings['readOnly_' . $activeType->getId()];
			}
			$readOnly = new htmlResponsiveInputCheckbox('readOnly_' . $activeType->getId(), $isReadOnly, _('Read-only'), '265');
			$readOnly->setElementsToDisable(array('hideNewButton_' . $activeType->getId(), 'hideDeleteButton_' . $activeType->getId()));
			$advancedOptions->add($readOnly, 12);
		}
		// hidden type
		$hidden = false;
		if (isset($typeSettings['hidden_' . $activeType->getId()])) {
			$hidden = $typeSettings['hidden_' . $activeType->getId()];
		}
		$advancedOptions->add(new htmlResponsiveInputCheckbox('hidden_' . $activeType->getId(), $hidden, _('Hidden'), '261'), 12);
		if (isLAMProVersion() && ($conf->getAccessLevel() == LAMConfig::ACCESS_ALL)) {
			// hide button to create new accounts
			$hideNewButton = false;
			if (isset($typeSettings['hideNewButton_' . $activeType->getId()])) {
				$hideNewButton = $typeSettings['hideNewButton_' . $activeType->getId()];
			}
			$advancedOptions->add(new htmlResponsiveInputCheckbox('hideNewButton_' . $activeType->getId(), $hideNewButton, _('No new entries'), '262'), 12);
			// hide button to delete accounts
			$hideDeleteButton = false;
			if (isset($typeSettings['hideDeleteButton_' . $activeType->getId()])) {
				$hideDeleteButton = $typeSettings['hideDeleteButton_' . $activeType->getId()];
			}
			$advancedOptions->add(new htmlResponsiveInputCheckbox('hideDeleteButton_' . $activeType->getId(), $hideDeleteButton, _('Disallow delete'), '263'), 12);
		}
		$container->add($advancedOptions, 12);

		$container->addVerticalSpacer('2rem');
		$index++;
	}
}

$dynamicTypeOptions = array();
foreach ($_SESSION['conftypes_optionTypes'] as $key => $value) {
	if (isset($typeSettings[$key])) {
		$dynamicTypeOptions[$key] = explode(LAMConfig::LINE_SEPARATOR, $typeSettings[$key]);
	}
}
parseHtml(null, $container, $dynamicTypeOptions, false, $tabindex, 'user');

echo "</div></div>";

echo "<input type=\"hidden\" name=\"postAvailable\" value=\"yes\">\n";

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
	$typeManager = new \LAM\TYPES\TypeManager($conf);
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
	    // check if up button was pressed
		elseif (substr($key, 0, 7) == "moveup_") {
			$type = substr($key, 7);
			$pos = array_search($type, $accountTypes);
			$temp = $accountTypes[$pos - 1];
			$accountTypes[$pos - 1] = $accountTypes[$pos];
			$accountTypes[$pos] = $temp;
		}
		// check if down button was pressed
		elseif (substr($key, 0, 9) == "movedown_") {
			$type = substr($key, 9);
			$pos = array_search($type, $accountTypes);
			$temp = $accountTypes[$pos + 1];
			$accountTypes[$pos + 1] = $accountTypes[$pos];
			$accountTypes[$pos] = $temp;
		}
		// set suffixes
		elseif (substr($key, 0, 7) == "suffix_") {
			$typeSettings[$key] = trim($_POST[$key]);
			$type = $typeManager->getConfiguredType(substr($postKeys[$i], 7));
			if (strlen($_POST[$key]) < 1) {
				$errors[] = array("ERROR", _("LDAP Suffix is invalid!"), $type->getAlias());
			}
		}
		// set attributes
		elseif (substr($key, 0, 5) == "attr_") {
			$typeSettings[$key] = $_POST[$key];
			$type = $typeManager->getConfiguredType(substr($postKeys[$i], 5));
			if (!is_string($_POST[$key]) || !preg_match("/^((#[^:;]+)|([^:;]*:[^:;]+))(;((#[^:;]+)|([^:;]*:[^:;]+)))*$/", $_POST[$key])) {
				$errors[] = array("ERROR", _("List attributes are invalid!"), $type->getAlias());
			}
		}
		// set filter
		elseif (substr($key, 0, strlen('filter_')) == "filter_") {
			$typeSettings[$key] = $_POST[$key];
		}
		// set custom label
		elseif (strpos($key, 'customLabel_') === 0) {
			$typeSettings[$key] = $_POST[$key];
		}
	}
	$typeConfigOptions = extractConfigOptionsFromPOST($_SESSION['conftypes_optionTypes']);
	foreach ($accountTypes as $accountType) {
		// set hidden
		$key = "hidden_" . $accountType;
		$typeSettings[$key] = (isset($_POST[$key]) && ($_POST[$key] == 'on'));
		if (isLAMProVersion() && ($conf->getAccessLevel() == LAMConfig::ACCESS_ALL)) {
			// set if new entries are allowed
			$key = "hideNewButton_" . $accountType;
			$typeSettings[$key] = (isset($_POST[$key]) && ($_POST[$key] == 'on'));
			// set if deletion of entries is allowed
			$key = "hideDeleteButton_" . $accountType;
			$typeSettings[$key] = (isset($_POST[$key]) && ($_POST[$key] == 'on'));
			// set if account type is read-only
			$key = "readOnly_" . $accountType;
			$typeSettings[$key] = (isset($_POST[$key]) && ($_POST[$key] == 'on'));
		}
		// check dynamic type settings
		$configuredType = $typeManager->getConfiguredType($accountType);
		if ($configuredType === null) {
			continue;
		}
		$typeObj = $configuredType->getBaseType();
		$typeMessages = $typeObj->check_configOptions($typeConfigOptions);
		if (!empty($typeMessages)) {
			$errors = array_merge($errors, $typeMessages);
		}
	}
	// new type
	foreach ($_POST as $key => $value) {
		// check if add button was pressed
		if (substr($key, 0, 4) == "add_") {
			$scope = substr($key, 4);
			$accountTypes[] = $typeManager->generateNewTypeId($scope);
		}
	}
	// add dynamic type settings
	foreach ($typeConfigOptions as $key => $value) {
		$typeSettings[$key] = implode(LAMConfig::LINE_SEPARATOR, $value);
	}
	// save input
	$conf->set_typeSettings($typeSettings);
	$conf->set_ActiveTypes($accountTypes);
	// check for duplicate type aliases
	$aliasNames = array();
	foreach ($typeManager->getConfiguredTypes() as $type) {
		if (in_array($type->getAlias(), $aliasNames)) {
			$errors[] = array('ERROR', _('Please set a unique label for the account types.'), htmlspecialchars($type->getAlias()));
		}
		$aliasNames[] = $type->getAlias();
	}
	return $errors;
}

/**
 * Compares types by alias for sorting.
 *
 * @param \baseType $a first type
 * @param \baseType $b second type
 */
function compareTypesByAlias($a, $b) {
	return strnatcasecmp($a->getAlias(), $b->getAlias());
}

?>
