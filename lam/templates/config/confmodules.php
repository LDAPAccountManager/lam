<?php
namespace LAM\CONFIG;
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
  Copyright (C) 2004 - 2018  Roland Gruber

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
echo "</head><body class=\"admin\">\n";
// include all JavaScript files
printJsIncludes('../..');
printConfigurationPageHeaderBar($conf);

// print error messages
for ($i = 0; $i < sizeof($errorsToDisplay); $i++) {
	call_user_func_array('StatusMessage', $errorsToDisplay[$i]);
}

echo "<form id=\"inputForm\" action=\"confmodules.php\" method=\"post\" onSubmit=\"saveScrollPosition('inputForm')\">\n";

printConfigurationPageTabs(ConfigurationPageTab::MODULES);

$typeManager = new \LAM\TYPES\TypeManager($conf);
$types = $typeManager->getConfiguredTypes();

$container = new htmlResponsiveRow();
foreach ($types as $type) {
	config_showAccountModules($type, $container);
}

$legendContainer = new htmlGroup();
$legendContainer->addElement(new htmlOutputText("(*) " . _("Base module")));
$legendContainer->addElement(new \htmlSpacer('2rem', null));
$legendContainer->addElement(new htmlHelpLink('237'));
$container->add($legendContainer, 12);
$container->add(new htmlHiddenInput('postAvailable', 'yes'), 12);

$tabindex = 1;
parseHtml(null, $container, array(), false, $tabindex, 'user');

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

if ((sizeof($errorsToDisplay) == 0) && isset($_POST['scrollPositionTop']) && isset($_POST['scrollPositionLeft'])) {
	// scroll to last position
	echo '<script type="text/javascript">
		jQuery(document).ready(function() {
			jQuery(window).scrollTop(' . $_POST['scrollPositionTop'] . ');
			jQuery(window).scrollLeft('. $_POST['scrollPositionLeft'] . ');
	});
	</script>';
}

echo "</form>\n";
echo "</body>\n";
echo "</html>\n";


/**
* Displays the module selection boxes and checks if dependencies are fulfilled.
*
* @param \LAM\TYPES\ConfiguredType $type account type
* @param htmlResponsiveRow $container meta HTML container
*/
function config_showAccountModules($type, &$container) {
	// account modules
	$available = getAvailableModules($type->getScope(), true);
	$selected = $type->getModules();
	$sortedAvailable = array();
	for ($i = 0; $i < sizeof($available); $i++) {
		$sortedAvailable[$available[$i]] = getModuleAlias($available[$i], $type->getScope());
	}
	natcasesort($sortedAvailable);

	// build options for selected and available modules
	$selOptions = array();
	for ($i = 0; $i < sizeof($selected); $i++) {
		if (in_array($selected[$i], $available)) {  // selected modules must be available
			if (is_base_module($selected[$i], $type->getScope())) {  // mark base modules
				$selOptions[getModuleAlias($selected[$i], $type->getScope()) . " (" . $selected[$i] .  ")(*)"] = $selected[$i];
			}
			else {
				$selOptions[getModuleAlias($selected[$i], $type->getScope()) . " (" . $selected[$i] .  ")"] = $selected[$i];
			}
		}
	}
	$availOptions = array();
	foreach ($sortedAvailable as $key => $value) {
		if (! in_array($key, $selected)) {  // display non-selected modules
			if (is_base_module($key, $type->getScope())) {  // mark base modules
				$availOptions[$value . " (" . $key .  ")(*)"] = $key;
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
		$listElements = array();
		foreach ($selOptions as $key => $value) {
			$el = new htmlTable('100%');
			$mod = new $value($type->getScope());
			$availModImage = new htmlImage('../../graphics/' . $mod->getIcon());
			$availModImage->setCSSClasses(array('size16', 'margin-right5-mobile-only'));
			$el->addElement($availModImage);
			$el->addElement(new htmlOutputText($key));
			$delButton = new htmlButton('del_' . $type->getId() . '_' . $value, 'del.png', true);
			$delButton->alignment = htmlElement::ALIGN_RIGHT;
			$el->addElement($delButton);
			$listElements[] = $el;
		}
		$selSortable = new htmlSortableList($listElements, $type->getId() . '_selected', null);
		$selSortable->alignment = htmlElement::ALIGN_TOP;
		$selSortable->setOnUpdate('updateModulePositions(\'positions_' . $type->getId() . '\', ui.item.data(\'posOrig\'), ui.item.index());');
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
			$availModImage->setCSSClasses(array('size16', 'margin10'));
			$availTable->addElement($availModImage);
			$availTable->addElement(new htmlOutputText($text));
			$addButton = new htmlButton('add_' . $type->getId() . '_' . $key, 'add.png', true);
			$addButton->alignment = htmlElement::ALIGN_RIGHT;
			$availTable->addElement($addButton, true);
		}
		$availDiv = new htmlDiv(null, $availTable);
		$availDiv->alignment = htmlElement::ALIGN_TOP;
		$availDiv->setCSSClasses(array('confModList'));
		$container->add($availDiv, 12, 6);
	}
	$positions = array();
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
 * @return array list of errors
 */
function checkInput() {
	if (!isset($_POST['postAvailable'])) {
		return array();
	}
	$errors = array();
	$conf = &$_SESSION['conf_config'];
	$typeSettings = $conf->get_typeSettings();
	$typeManager = new \LAM\TYPES\TypeManager($conf);
	$accountTypes = $typeManager->getConfiguredTypes();
	foreach ($accountTypes as $type) {
		$scope = $type->getScope();
		$typeId = $type->getId();
		$available = getAvailableModules($scope, true);
		$selected_temp = (isset($typeSettings['modules_' . $typeId])) ? $typeSettings['modules_' . $typeId] : '';
		$selected_temp = explode(',', $selected_temp);
		$selected = array();
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
			$sortTmp = array();
			foreach ($sorting as $pos) {
				$sortTmp[] = $selected[$pos];
			}
			$selected = $sortTmp;
		}
		// remove modules from selection
		$new_selected = array();
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
				$errors[] = array('ERROR', $type->getAlias(), _("Unsolved dependency:") . ' ' .
					$depends[$i][0] . " (" . $depends[$i][1] . ")");
			}
		}
		// check conflicts
		$conflicts = check_module_conflicts($selected, getModulesDependencies($scope));
		if ($conflicts !== false) {
			for ($i = 0; $i < sizeof($conflicts); $i++) {
				$errors[] = array('ERROR', $type->getAlias(), _("Conflicting module:") . ' ' .
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
			$errors[] = array('ERROR', $type->getAlias(), _("No or more than one base module selected!"));
		}
	}
	$conf->set_typeSettings($typeSettings);

	return $errors;
}

?>




