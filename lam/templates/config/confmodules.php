<?php
namespace LAM\CONFIG;
use htmlInputField;
use htmlJavaScript;
use \htmlTable;
use \htmlOutputText;
use \htmlHelpLink;
use \htmlHiddenInput;
use \htmlButton;
use \htmlSpacer;
use \htmlElement;
use \htmlImage;
use \htmlSortableList;
use \htmlSubTitle;
use \htmlDiv;
use \htmlResponsiveRow;
use \htmlGroup;
/*
  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2004 - 2024  Roland Gruber

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
/** common functions */
include_once '../../lib/configPages.inc';

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
		// go to jobs page
		elseif (isset($_POST['jobs'])) {
			metaRefresh("jobs.php");
			exit;
		}
	}
}

echo $_SESSION['header'];
printHeaderContents(_("LDAP Account Manager Configuration"), '../..');
echo "</head><body>\n";
// include all JavaScript files
printJsIncludes('../..');
printConfigurationPageHeaderBar($conf);

// print error messages
for ($i = 0; $i < sizeof($errorsToDisplay); $i++) {
	call_user_func_array(StatusMessage(...), $errorsToDisplay[$i]);
}

echo "<form id=\"inputForm\" action=\"confmodules.php\" method=\"post\" onSubmit=\"window.lam.utility.saveScrollPosition('inputForm')\" novalidate=\"novalidate\">\n";

printConfigurationPageTabs(ConfigurationPageTab::MODULES);

$typeManager = new \LAM\TYPES\TypeManager($conf);
$types = $typeManager->getConfiguredTypes();

$container = new htmlResponsiveRow();
foreach ($types as $type) {
	config_showAccountModules($type, $container);
}

$legendContainer = new htmlGroup();
$legendContainer->addElement(new htmlOutputText("* " . _("Base module")));
$legendContainer->addElement(new \htmlSpacer('2rem', null));
$legendContainer->addElement(new htmlHelpLink('237'));
$container->add($legendContainer, 12);
$container->add(new htmlHiddenInput('postAvailable', 'yes'), 12);

parseHtml(null, $container, [], false, 'user');

echo "</div></div>";

$buttonContainer = new htmlTable();
$buttonContainer->addElement(new htmlSpacer(null, '10px'), true);
$saveButton = new htmlButton('saveSettings', _('Save'));
$saveButton->setCSSClasses(['lam-primary']);
$buttonContainer->addElement($saveButton);
$cancelButton = new htmlButton('cancelSettings', _('Cancel'));
$buttonContainer->addElement($cancelButton, true);
$buttonContainer->addElement(new htmlSpacer(null, '10px'), true);

if (empty($errorsToDisplay) && isset($_POST['scrollPositionTop']) && isset($_POST['scrollPositionLeft'])) {
	// scroll to last position
	$buttonContainer->addElement(new htmlJavaScript('window.lam.utility.restoreScrollPosition(' . $_POST['scrollPositionTop'] .', ' . $_POST['scrollPositionLeft'] . ')'));
}

parseHtml(null, $buttonContainer, [], false, 'user');

echo "</form>\n";
echo "</body>\n";
echo "</html>\n";


/**
* Displays the module selection boxes and checks if dependencies are fulfilled.
*
* @param \LAM\TYPES\ConfiguredType $type account type
* @param htmlResponsiveRow $container meta HTML container
*/
function config_showAccountModules($type, &$container): void {
	// account modules
	$available = getAvailableModules($type->getScope(), true);
	$selected = $type->getModules();
	$sortedAvailable = [];
	for ($i = 0; $i < sizeof($available); $i++) {
		$sortedAvailable[$available[$i]] = getModuleAlias($available[$i], $type->getScope());
	}
	natcasesort($sortedAvailable);

	// build options for selected and available modules
	$selOptions = [];
	for ($i = 0; $i < sizeof($selected); $i++) {
		if (in_array($selected[$i], $available)) {  // selected modules must be available
			if (is_base_module($selected[$i], $type->getScope())) {  // mark base modules
				$selOptions[getModuleAlias($selected[$i], $type->getScope()) . " (" . $selected[$i] .  ")*"] = $selected[$i];
			}
			else {
				$selOptions[getModuleAlias($selected[$i], $type->getScope()) . " (" . $selected[$i] .  ")"] = $selected[$i];
			}
		}
	}
	$availOptions = [];
	foreach ($sortedAvailable as $key => $value) {
		if (! in_array($key, $selected)) {  // display non-selected modules
			if (is_base_module($key, $type->getScope())) {  // mark base modules
				$availOptions[$value . " (" . $key .  ")*"] = $key;
			}
			else {
				$availOptions[$value . " (" . $key .  ")"] = $key;
			}
		}
	}

	// add account module selection
	$container->add(new htmlSubTitle($type->getAlias(), '../../graphics/' . $type->getIcon()), 12);
	if (sizeof($selOptions) > 0) {
		$container->add(new htmlOutputText(_("Selected modules")), 12, 6);
	}
	if (sizeof($availOptions) > 0) {
		$container->add(new htmlOutputText(_("Available modules")), 0, 6);
	}
	$container->addVerticalSpacer('1rem');
	// selected modules
	if (sizeof($selOptions) > 0) {
		$listElements = [];
		foreach ($selOptions as $key => $value) {
			$el = new htmlTable('100%');
			$mod = new $value($type->getScope());
			$availModImage = new htmlImage('../../graphics/' . $mod->getIcon());
			$availModImage->setCSSClasses(['size16', 'margin-right5-mobile-only']);
			$el->addElement($availModImage);
			$el->addElement(new htmlOutputText($key));
			$delButton = new htmlButton('del_' . $type->getId() . '_' . $value, 'del.svg', true);
			$delButton->alignment = htmlElement::ALIGN_RIGHT;
			$el->addElement($delButton);
			$listElements[] = $el;
		}
		$selSortable = new htmlSortableList($listElements, $type->getId() . '_selected');
		$selSortable->alignment = htmlElement::ALIGN_TOP;
		$selSortable->setCSSClasses(['module-list']);
		$selSortable->setOnUpdate('function() {updateModulePositions(\'positions_' . $type->getId() . '\', \'' . $type->getId() . '_selected' . '\');}');
		$container->add($selSortable, 12, 6);
	}
	else {
		$container->add(new htmlOutputText(''), 12, 6);
	}
	// available modules
	if (sizeof($availOptions) > 0) {
		$container->add(new htmlSpacer(null, '2rem'), 12, 0, 0, 'hide-on-tablet');
		$container->add(new htmlOutputText(_("Available modules")), 12, 0, 0, 'hide-on-tablet');
		$container->add(new htmlSpacer(null, '1rem'), 12, 0, 0, 'hide-on-tablet');
		$availTable = new htmlTable();
		foreach ($availOptions as $text => $key) {
			$mod = new $key($type->getScope());
			$availModImage = new htmlImage('../../graphics/' . $mod->getIcon());
			$availModImage->setCSSClasses(['size16', 'margin10']);
			$availTable->addElement($availModImage);
			$availTable->addElement(new htmlOutputText($text));
			$addButton = new htmlButton('add_' . $type->getId() . '_' . $key, 'add.svg', true);
			$addButton->alignment = htmlElement::ALIGN_RIGHT;
			$availTable->addElement($addButton, true);
		}
		$availRow = new htmlResponsiveRow();
		$availDiv = new htmlDiv(null, $availTable);
		$availDiv->alignment = htmlElement::ALIGN_TOP;
		$availDiv->setCSSClasses(['confModList']);
		$availRow->add($availDiv);
		if (sizeof($availOptions) >= 10) {
			$availRow->addVerticalSpacer('1rem');
			$filterGroup = new htmlGroup();
			$filterGroup->addElement(new htmlOutputText(_('Filter')));
			$filterInput = new htmlInputField('filter_' . $type->getId());
			$filterInput->setOnInput('window.lam.config.updateModuleFilter(this); return false;');
			$filterInput->setTransient(true);
			$filterGroup->addElement($filterInput);
			$availRow->add($filterGroup);
		}
		$container->add($availRow, 12, 6);
	}
	$positions = [];
	for ($i = 0; $i < sizeof($selOptions); $i++) {
		$positions[] = $i;
	}
	$container->add(new htmlHiddenInput('positions_' . $type->getId(), implode(',', $positions)), 12);
	// spacer to next account type
	$container->addVerticalSpacer('2rem');
}

/**
 * Checks user input and saves the entered settings.
 *
 * @return array<mixed> list of errors
 */
function checkInput(): array {
	if (!isset($_POST['postAvailable'])) {
		return [];
	}
	$errors = [];
	$conf = &$_SESSION['conf_config'];
	$typeSettings = $conf->get_typeSettings();
	$typeManager = new \LAM\TYPES\TypeManager($conf);
	$accountTypes = $typeManager->getConfiguredTypes();
	foreach ($accountTypes as $type) {
		$scope = $type->getScope();
		$typeId = $type->getId();
		$available = getAvailableModules($scope, true);
		$selected_temp = $typeSettings['modules_' . $typeId] ?? '';
		$selected_temp = explode(',', $selected_temp);
		$selected = [];
		// only use available modules as selected
		for ($i = 0; $i < sizeof($selected_temp); $i++) {
			if (in_array($selected_temp[$i], $available)) {
				$selected[] = $selected_temp[$i];
			}
		}
		// reorder based on sortable list
		$sorting = $_POST['positions_' . $typeId];
		if (!empty($sorting)) {
			$sorting = explode(',', $sorting);
			$sortTmp = [];
			foreach ($sorting as $pos) {
				$sortTmp[] = $selected[intval($pos)];
			}
			$selected = $sortTmp;
		}
		// remove modules from selection
		$new_selected = [];
		for ($i = 0; $i < sizeof($selected); $i++) {
			if (!isset($_POST['del_' . $typeId . '_' . $selected[$i]])) {
				$new_selected[] = $selected[$i];
			}
		}
		$selected = $new_selected;
		$typeSettings['modules_' . $typeId] = implode(',', $selected);
		// add modules to selection
		foreach ($available as $modName) {
			if (isset($_POST['add_' . $typeId . '_' . $modName])) {
				$selected[] = $modName;
				$typeSettings['modules_' . $typeId] = implode(',', $selected);
				break;
			}
		}
		// check dependencies
		$depends = check_module_depends($selected, getModulesDependencies($scope));
		if ($depends !== false) {
			for ($i = 0; $i < sizeof($depends); $i++) {
				$errors[] = ['ERROR', $type->getAlias(), _("Unsolved dependency:") . ' ' .
					$depends[$i][0] . " (" . $depends[$i][1] . ")"];
			}
		}
		// check conflicts
		$conflicts = check_module_conflicts($selected, getModulesDependencies($scope));
		if ($conflicts !== false) {
			for ($i = 0; $i < sizeof($conflicts); $i++) {
				$errors[] = ['ERROR', $type->getAlias(), _("Conflicting module:") . ' ' .
					$conflicts[$i][0] . " (" . $conflicts[$i][1] . ")"];
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
			$errors[] = ['ERROR', $type->getAlias(), _("No or more than one base module selected!")];
		}
	}
	$conf->set_typeSettings($typeSettings);

	return $errors;
}
