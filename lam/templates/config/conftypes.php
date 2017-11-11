<?php
namespace LAM\CONFIG;
use \htmlTable;
use \htmlSubTitle;
use \htmlImage;
use \htmlOutputText;
use \htmlSpacer;
use \htmlButton;
use \htmlElement;
use \htmlGroup;
use \htmlTableExtendedInputField;
use \LAMConfig;
use \htmlTableExtendedInputCheckbox;
/*
  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2004 - 2017  Roland Gruber

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
for ($i = 0; $i < sizeof($errorsToDisplay); $i++) call_user_func_array('StatusMessage', $errorsToDisplay[$i]);

echo ("<form action=\"conftypes.php\" method=\"post\">\n");

printConfigurationPageTabs(ConfigurationPageTab::TYPES);

$container = new htmlTable();

// show available types
if (sizeof($availableScopes) > 0) {
	$container->addElement(new htmlSubTitle(_("Available account types")), true);
	$availableContainer = new htmlTable();
	foreach ($availableScopes as $availableScope) {
		$availableContainer->addElement(new htmlImage('../../graphics/' . $availableScope->getIcon()));
		$availableContainer->addElement(new htmlOutputText($availableScope->getAlias()));
		$availableContainer->addElement(new htmlSpacer('10px', null));
		$availableContainer->addElement(new htmlOutputText($availableScope->getDescription()));
		$button = new htmlButton('add_' . $availableScope->getScope(), 'add.png', true);
		$button->setTitle(_("Add"));
		$availableContainer->addElement($button, true);
	}
	$availableContainer->addElement(new htmlSpacer(null, '20px'), true);
	$container->addElement($availableContainer, true);
}

$_SESSION['conftypes_optionTypes'] = array();
// show active types
if (sizeof($activeTypes) > 0) {
	$container->addElement(new htmlSubTitle(_("Active account types")), true);
	$activeContainer = new htmlTable();
	foreach ($activeTypes as $activeType) {
		// title
		$titleGroup = new htmlGroup();
		$titleGroup->colspan = 6;
		$titleGroup->addElement(new htmlImage('../../graphics/' . $activeType->getIcon()));
		$titleText = new htmlOutputText($activeType->getAlias());
		$titleText->setIsBold(true);
		$titleGroup->addElement($titleText);
		$titleGroup->addElement(new htmlSpacer('10px', null));
		$titleGroup->addElement(new htmlOutputText($activeType->getBaseType()->getDescription()));
		$activeContainer->addElement($titleGroup);
		// delete button
		$delButton = new htmlButton('rem_'. $activeType->getId(), 'del.png', true);
		$delButton->alignment = htmlElement::ALIGN_RIGHT;
		$delButton->setTitle(_("Remove this account type"));
		$activeContainer->addElement($delButton, true); //del.png
		$activeContainer->addElement(new htmlSpacer(null, '5px'), true);
		// LDAP suffix
		$suffixInput = new htmlTableExtendedInputField(_("LDAP suffix"), 'suffix_' . $activeType->getId(), $typeSettings['suffix_' . $activeType->getId()], '202');
		$suffixInput->setFieldSize(40);
		$activeContainer->addElement($suffixInput);
		$activeContainer->addElement(new htmlSpacer('20px', null));
		// list attributes
		if (isset($typeSettings['attr_' . $activeType->getId()])) {
			$attributes = $typeSettings['attr_' . $activeType->getId()];
		}
		else {
			$attributes = $activeType->getBaseType()->getDefaultListAttributes();
		}
		$attrsInput = new htmlTableExtendedInputField(_("List attributes"), 'attr_' . $activeType->getId(), $attributes, '206');
		$attrsInput->setFieldSize(40);
		$attrsInput->setFieldMaxLength(1000);
		$activeContainer->addElement($attrsInput, true);
		// custom label
		$customLabel = '';
		if (isset($typeSettings['customLabel_' . $activeType->getId()])) {
			$customLabel = $typeSettings['customLabel_' . $activeType->getId()];
		}
		$customLabelInput = new htmlTableExtendedInputField(_('Custom label'), 'customLabel_' . $activeType->getId(), $customLabel, '264');
		$customLabelInput->setFieldSize(40);
		$activeContainer->addElement($customLabelInput);
		$activeContainer->addElement(new htmlSpacer('20px', null));
		// LDAP filter
		$filter = '';
		if (isset($typeSettings['filter_' . $activeType->getId()])) {
			$filter = $typeSettings['filter_' . $activeType->getId()];
		}
		$filterInput = new htmlTableExtendedInputField(_("Additional LDAP filter"), 'filter_' . $activeType->getId(), $filter, '260');
		$filterInput->setFieldSize(40);
		$activeContainer->addElement($filterInput, true);
		// type options
		$typeConfigOptions = $activeType->getBaseType()->get_configOptions();
		if (!empty($typeConfigOptions)) {
			foreach ($typeConfigOptions as $typeConfigOption) {
				$activeContainer->addElement($typeConfigOption, true);
			}
			// save option types to session
			ob_start();
			$dummyIndex = 1;
			$typeConfigOptionTypes = parseHtml(null, $typeConfigOptions, array(), true, $dummyIndex, 'user');
			ob_end_clean();
			$_SESSION['conftypes_optionTypes'] = array_merge($_SESSION['conftypes_optionTypes'], $typeConfigOptionTypes);
		}
		// advanced options
		$advancedOptions = new htmlTable();
		$advancedOptions->colspan = 30;
		// read-only
		if (isLAMProVersion() && ($conf->getAccessLevel() == LAMConfig::ACCESS_ALL)) {
			$isReadOnly = false;
			if (isset($typeSettings['readOnly_' . $activeType->getId()])) {
				$isReadOnly = $typeSettings['readOnly_' . $activeType->getId()];
			}
			$readOnly = new htmlTableExtendedInputCheckbox('readOnly_' . $activeType->getId(), $isReadOnly, _('Read-only'), '265');
			$readOnly->setElementsToDisable(array('hideNewButton_' . $activeType->getId(), 'hideDeleteButton_' . $activeType->getId()));
			$advancedOptions->addElement($readOnly);
			$advancedOptions->addElement(new htmlSpacer('20px', null));
		}
		// hidden type
		$hidden = false;
		if (isset($typeSettings['hidden_' . $activeType->getId()])) {
			$hidden = $typeSettings['hidden_' . $activeType->getId()];
		}
		$advancedOptions->addElement(new htmlTableExtendedInputCheckbox('hidden_' . $activeType->getId(), $hidden, _('Hidden'), '261'));
		if (isLAMProVersion() && ($conf->getAccessLevel() == LAMConfig::ACCESS_ALL)) {
			$advancedOptions->addElement(new htmlSpacer('20px', null));
			// hide button to create new accounts
			$hideNewButton = false;
			if (isset($typeSettings['hideNewButton_' . $activeType->getId()])) {
				$hideNewButton = $typeSettings['hideNewButton_' . $activeType->getId()];
			}
			$advancedOptions->addElement(new htmlTableExtendedInputCheckbox('hideNewButton_' . $activeType->getId(), $hideNewButton, _('No new entries'), '262'));
			$advancedOptions->addElement(new htmlSpacer('20px', null));
			// hide button to delete accounts
			$hideDeleteButton = false;
			if (isset($typeSettings['hideDeleteButton_' . $activeType->getId()])) {
				$hideDeleteButton = $typeSettings['hideDeleteButton_' . $activeType->getId()];
			}
			$advancedOptions->addElement(new htmlTableExtendedInputCheckbox('hideDeleteButton_' . $activeType->getId(), $hideDeleteButton, _('Disallow delete'), '263'), true);
		}
		$activeContainer->addElement($advancedOptions, true);

		$activeContainer->addElement(new htmlSpacer(null, '40px'), true);
	}
	$container->addElement($activeContainer, true);
}

$tabindex = 1;
$dynamicTypeOptions = array();
foreach ($_SESSION['conftypes_optionTypes'] as $key => $value) {
	if (isset($typeSettings[$key])) {
		$dynamicTypeOptions[$key] = explode(LAMConfig::LINE_SEPARATOR, $typeSettings[$key]);
	}
}
parseHtml(null, $container, $dynamicTypeOptions, false, $tabindex, 'user');

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
		$typeObj = $typeManager->getConfiguredType($accountType)->getBaseType();
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
