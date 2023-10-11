<?php
namespace LAM\UPLOAD;
use htmlInputCheckbox;
use htmlLabel;
use \htmlResponsiveTable;
use \htmlOutputText;
use \htmlGroup;
use \htmlImage;
use \htmlResponsiveInputCheckbox;
use \htmlDiv;
use \htmlHiddenInput;
use \htmlButton;
use \htmlTitle;
use \htmlResponsiveInputFileUpload;
use \htmlLink;
use \htmlSubTitle;
use \htmlHelpLink;
use \htmlResponsiveRow;
use \htmlResponsiveSelect;
use \htmlSpacer;
use LAM\PDF\PdfStructurePersistenceManager;
use \moduleCache;
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2004 - 2022  Roland Gruber

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
* Start page of file upload
*
* @author Roland Gruber
* @package tools
*/

/** security functions */
include_once(__DIR__ . "/../../lib/security.inc");
/** access to configuration */
include_once(__DIR__ . '/../../lib/config.inc');
/** status messages */
include_once(__DIR__ . '/../../lib/status.inc');
/** account modules */
include_once(__DIR__ . '/../../lib/modules.inc');
/** Used to get PDF information. */
include_once(__DIR__ . '/../../lib/pdfstruct.inc');
/** upload functions */
include_once(__DIR__ . '/../../lib/upload.inc');

// Start session
startSecureSession();
enforceUserIsLoggedIn();

// check if this tool may be run
checkIfToolIsActive('toolFileUpload');

// die if no write access
if (!checkIfWriteAccessIsAllowed()) die();

checkIfToolIsActive('toolFileUpload');

// Redirect to startpage if user is not logged in
if (!isLoggedIn()) {
	metaRefresh("../login.php");
	exit;
}

// Set correct language, codepages, ....
setlanguage();

if (!empty($_POST)) {
	validateSecurityToken();
}

// show CSV if requested
if (isset($_GET['getCSV'])) {
	//download file
	header('Content-Type: application/msexcel');
	header('Content-disposition: attachment; filename=lam.csv');
	echo $_SESSION['mass_csv'];
	exit;
}

Uploader::cleanSession();

include __DIR__ . '/../../lib/adminHeader.inc';

// get possible types and remove those which do not support file upload
$typeManager = new \LAM\TYPES\TypeManager();
$types = $typeManager->getConfiguredTypes();
$count = sizeof($types);
for ($i = 0; $i < $count; $i++) {
	$myType = $types[$i];
	if (!$myType->getBaseType()->supportsFileUpload() || $myType->isHidden()
			|| !checkIfNewEntriesAreAllowed($myType->getId()) || !checkIfWriteAccessIsAllowed($myType->getId())) {
		unset($types[$i]);
	}
}
$types = array_values($types);

// check if account specific page should be shown
if (isset($_POST['type'])) {
	// get selected type
	$typeId = htmlspecialchars($_POST['type']);
	$type = $typeManager->getConfiguredType($typeId);
	// get selected modules
	$selectedModules = array();
	$checkedBoxes = array_keys($_POST, 'on');
	foreach ($checkedBoxes as $checkedBox) {
		if (strpos($checkedBox, $typeId . '___') === 0) {
			$selectedModules[] = substr($checkedBox, strlen($typeId) + strlen('___'));
		}
	}
	$deps = getModulesDependencies($type->getScope());
	$depErrors = check_module_depends($selectedModules, $deps);
	if (is_array($depErrors) && (sizeof($depErrors) > 0)) {
		foreach ($depErrors as $depError) {
			StatusMessage('ERROR', _("Unsolved dependency:") . ' ' .
							getModuleAlias($depError[0], $type->getScope()) . " (" .
							getModuleAlias($depError[1], $type->getScope()) . ")");
		}
	}
	else {
		showMainPage($type, $selectedModules);
		exit;
	}
}

// show start page
$divClass = 'user';
if (isset($_REQUEST['type'])) {
	$divClass = htmlspecialchars(\LAM\TYPES\getScopeFromTypeId($_REQUEST['type']));
}
echo '<div class="smallPaddingContent">';
echo "<form enctype=\"multipart/form-data\" action=\"masscreate.php\" method=\"post\">\n";

$row = new htmlResponsiveRow();
$row->add(new htmlTitle(_("Account creation via file upload")), 12);
$row->add(new htmlOutputText(_("Here you can create multiple accounts by providing a CSV file.")), 12);
$row->addVerticalSpacer('4rem');

// account type
$typeList = array();
foreach ($types as $type) {
	$typeList[$type->getAlias()] = $type->getId();
}
$selectedType = null;
if (isset($_REQUEST['type'])) {
	$selectedType = $_REQUEST['type'];
}
elseif (!empty($types)) {
	$selectedType = $types[0]->getId();
}
$typeSelect = new htmlResponsiveSelect('type', $typeList, array($selectedType), _("Account type"));
$typeSelect->setHasDescriptiveElements(true);
$typeSelect->setOnchangeEvent('changeVisibleModules(this);');
$row->add($typeSelect, 12);
$row->addVerticalSpacer('1rem');

$row->add(new htmlSubTitle(_('Selected modules')), 12);

// module selection
foreach ($types as $type) {
	$divClasses = array('typeOptions');
	if ($selectedType != $type->getId()) {
		$divClasses[] = 'hidden';
	}
	$innerRow = new htmlResponsiveRow();
	$modules = $_SESSION['config']->get_AccountModules($type->getId());
	foreach ($modules as $moduleName) {
		$moduleGroup = new htmlGroup();
		$module = moduleCache::getModule($moduleName, $type->getScope());
		$iconImage = $module->getIcon();
		if (!is_null($iconImage) && !(strpos($iconImage, 'http') === 0) && !(strpos($iconImage, '/') === 0)) {
			$iconImage = '../../graphics/' . $iconImage;
		}
		$image = new htmlImage($iconImage, '32px', '32px');
		$image->setCSSClasses(array('margin3'));
		$moduleGroup->addElement($image);
		$enabled = true;
		if (is_base_module($moduleName, $type->getScope())) {
			$enabled = false;
		}
		$checked = true;
		if (isset($_POST['submit']) && !isset($_POST[$type->getId() . '___' . $moduleName])) {
			$checked = false;
		}
		$checkbox = new htmlInputCheckbox($type->getId() . '___' . $moduleName, $checked);
		$checkbox->setIsEnabled($enabled);
		if ($enabled) {
			$moduleGroup->addElement($checkbox);
		}
		else {
			$boxGroup = new htmlGroup();
			$boxGroup->addElement($checkbox);
			// add hidden field to fake disabled checkbox value
			$boxGroup->addElement(new htmlHiddenInput($type->getId() . '___' . $moduleName, 'on'));
			$moduleGroup->addElement($boxGroup);
		}
		$moduleGroup->addElement(new htmlLabel($type->getId() . '___' . $moduleName, getModuleAlias($moduleName, $type->getScope())));
		$innerRow->add($moduleGroup, 12, 6, 4);
	}
	$moduleCount = sizeof($modules);
	if ($moduleCount%3 == 2) {
		$innerRow->add(new htmlOutputText('&nbsp;', false), 0, 0, 4);
	}
	if ($moduleCount%3 == 1) {
		$innerRow->add(new htmlOutputText('&nbsp;', false), 0, 0, 4);
	}
	if ($moduleCount%2 == 1) {
		$innerRow->add(new htmlOutputText('&nbsp;', false), 0, 6, 0);
	}
	$typeDiv = new htmlDiv($type->getId(), $innerRow);
	$typeDiv->setCSSClasses($divClasses);
	$row->add($typeDiv, 12);
}

// ok button
$row->addVerticalSpacer('3rem');
if (!empty($types)) {
    $okButton = new htmlButton('submit', _('Ok'));
    $okButton->setCSSClasses(array('lam-primary'));
	$row->add($okButton);
}

addSecurityTokenToMetaHTML($row);
parseHtml(null, $row, array(), false, 'user');

?>
<script type="text/javascript">
function changeVisibleModules(element) {
	jQuery('div.typeOptions').toggle(false);
	jQuery('div#' + element.options[element.selectedIndex].value).toggle();
}
</script>
<?php
echo "</form>\n";

echo '</div>';
include __DIR__ . '/../../lib/adminFooter.inc';

/**
* Displays the account type specific main page of the upload.
*
* @param \LAM\TYPES\ConfiguredType $type account type
* @param string[] $selectedModules list of selected account modules
*/
function showMainPage(\LAM\TYPES\ConfiguredType $type, array $selectedModules): void {
	$scope = $type->getScope();
	echo '<div class="smallPaddingContent">';
	// get input fields from modules
	$columns = getUploadColumns($type, $selectedModules);
	$modules = array_keys($columns);

	echo "<form enctype=\"multipart/form-data\" action=\"massBuildAccounts.php\" method=\"post\">\n";
	$row = new htmlResponsiveRow();
	$row->setCSSClasses(array('maxrow'));

	// title
	$row->add(new htmlTitle(_("File upload")), 12);

	// instructions
	$row->add(new htmlOutputText(_("Please provide a CSV formatted file with your account data. The cells in the first row must be filled with the column identifiers. The following rows represent one account for each row.")), 12);
	$row->add(new htmlOutputText(_("Check your input carefully. LAM will only do some basic checks on the upload data.")), 12);
	$row->addVerticalSpacer('1rem');
	$row->add(new htmlOutputText(_("Hint: Format all cells as text in your spreadsheet program and turn off auto correction.")), 12);
	$row->addVerticalSpacer('1rem');

	// upload elements
	$row->addLabel(new htmlOutputText(_("Download sample CSV file")));
	$saveLink = new htmlLink('', 'masscreate.php?getCSV=1', '../../graphics/save.svg');
	$saveLink->setCSSClasses(array('icon'));
	$row->addField($saveLink);
	$row->addVerticalSpacer('3rem');
	$row->add(new htmlResponsiveInputFileUpload('inputfile', _("CSV file"), null, true), 12);
	$row->add(new htmlHiddenInput('typeId', $type->getId()), 12);
	$row->add(new htmlHiddenInput('selectedModules', implode(',', $selectedModules)), 12);

	// PDF
	$pdfStructurePersistenceManager = new PdfStructurePersistenceManager();
	$pdfStructures = $pdfStructurePersistenceManager->getPDFStructures($_SESSION['config']->getName(), $type->getId());
	if (!empty($pdfStructures)) {
		$createPDF = false;
		if (isset($_POST['createPDF']) && ($_POST['createPDF'] === '1')) {
			$createPDF = true;
		}
		$pdfCheckbox = new htmlResponsiveInputCheckbox('createPDF', $createPDF, _('Create PDF files'));
		$pdfCheckbox->setTableRowsToShow(array('pdfStructure', 'pdf_font'));
		$row->add($pdfCheckbox);
		$pdfSelected = array();
		if (isset($_POST['pdfStructure'])) {
			$pdfSelected = array($_POST['pdfStructure']);
		}
		else if (in_array('default', $pdfStructures)) {
			$pdfSelected = array('default');
		}
		$row->add(new htmlResponsiveSelect('pdfStructure', $pdfStructures, $pdfSelected, _('PDF structure')));
		$fonts = \LAM\PDF\getPdfFonts();
		$fontSelection = new htmlResponsiveSelect('pdf_font', $fonts, array(), _('Font'), '411');
		$fontSelection->setCSSClasses(array('lam-save-selection'));
		$fontSelection->setHasDescriptiveElements(true);
		$fontSelection->setSortElements(false);
		$row->add($fontSelection, 12);
	}
	$row->addVerticalSpacer('1rem');

	$uploadButton = new htmlButton('submitfile', _('Upload file and create accounts'));
	$uploadButton->setCSSClasses(array('lam-primary'));
	$row->addLabel($uploadButton);
	$row->addField(new htmlOutputText('&nbsp;', false));
	$row->addVerticalSpacer('2rem');

	$row->add(new htmlTitle(_("Columns")), 12);

	// DN options
	$dnTitle = new htmlSubTitle(_("DN settings"), '../../graphics/logo32.png');
	$row->add($dnTitle, 12);
	$titles = array(_('Name'), _("Identifier"), _("Example value"), _("Default value"), _("Possible values"));
	$data = array();
	// DN suffix
	$dnSuffixRowCells = array();
	$nameGroup = new htmlGroup();
	$help = new htmlHelpLink('361');
	$help->setCSSClasses(array('hide-on-mobile'));
	$nameGroup->addElement($help);
	$nameGroup->addElement(new htmlSpacer('0.25rem', '16px'));
	$nameGroup->addElement(new htmlOutputText(_("DN suffix")));
	$help = new htmlHelpLink('361');
	$help->setCSSClasses(array('hide-on-tablet'));
	$nameGroup->addElement($help);
	$dnSuffixRowCells[] = $nameGroup;
	$dnSuffixRowCells[] = new htmlOutputText('dn_suffix');
	$dnSuffixRowCells[] = new htmlOutputText($type->getSuffix());
	$dnSuffixRowCells[] = new htmlOutputText($type->getSuffix());
	$dnSuffixRowCells[] = new htmlOutputText('');
	$data[] = $dnSuffixRowCells;
	// RDN
	$dnRDNRowCells = array();
	$rdnText = new htmlOutputText(_("RDN identifier"));
	$rdnText->setMarkAsRequired(true);
	$nameGroup = new htmlGroup();
	$help = new htmlHelpLink('301');
	$help->setCSSClasses(array('hide-on-mobile'));
	$nameGroup->addElement($help);
	$nameGroup->addElement(new htmlSpacer('0.25rem', '16px'));
	$nameGroup->addElement($rdnText);
	$help = new htmlHelpLink('301');
	$help->setCSSClasses(array('hide-on-tablet'));
	$nameGroup->addElement($help);
	$dnRDNRowCells[] = $nameGroup;
	$dnRDNRowCells[] = new htmlOutputText('dn_rdn');
	$rdnAttributes = getRDNAttributes($type->getId(), $selectedModules);
	$dnRDNRowCells[] = new htmlOutputText($rdnAttributes[0]);
	$dnRDNRowCells[] = new htmlOutputText('');
	$dnRDNRowCells[] = new htmlOutputText(implode(", ", $rdnAttributes));
	$dnRDNRowCells[] = new htmlHelpLink('301');
	$data[] = $dnRDNRowCells;
	// replace existing
	$replaceRowCells = array();
	$nameGroup = new htmlGroup();
	$help = new htmlHelpLink('302');
	$help->setCSSClasses(array('hide-on-mobile'));
	$nameGroup->addElement($help);
	$nameGroup->addElement(new htmlSpacer('0.25rem', '16px'));
	$nameGroup->addElement(new htmlOutputText(_("Overwrite")));
	$help = new htmlHelpLink('302');
	$help->setCSSClasses(array('hide-on-tablet'));
	$nameGroup->addElement($help);
	$replaceRowCells[] = $nameGroup;
	$replaceRowCells[] = new htmlOutputText('overwrite');
	$replaceRowCells[] = new htmlOutputText('false');
	$replaceRowCells[] = new htmlOutputText('false');
	$replaceRowCells[] = new htmlOutputText('true, false');
	$data[] = $replaceRowCells;

	$table = new htmlResponsiveTable($titles, $data);
	$table->setCSSClasses(array('alternating-color'));
	$row->add($table, 12);

	// module options
	foreach ($modules as $moduleName) {
		// skip modules without upload columns
		if (sizeof($columns[$moduleName]) < 1) {
			continue;
		}
		$data = array();
		$row->addVerticalSpacer('2rem');
		$module = moduleCache::getModule($moduleName, $scope);
		$icon = $module->getIcon();
		if (!empty($icon) && !(strpos($icon, 'http') === 0) && !(strpos($icon, '/') === 0)) {
			$icon = '../../graphics/' . $icon;
		}
		$moduleTitle = new htmlSubTitle(getModuleAlias($moduleName, $scope), $icon);
		$moduleTitle->colspan = 20;
		$row->add($moduleTitle, 12);
		foreach ($columns[$moduleName] as $column) {
			$required = false;
			if (isset($column['required']) && ($column['required'] === true)) {
				$required = true;
			}
			$rowCells = array();
			$descriptionText = new htmlOutputText($column['description']);
			$descriptionText->setMarkAsRequired($required);
			$nameGroup = new htmlGroup();
			$help = new htmlHelpLink($column['help'], $moduleName, $scope);
			$help->setCSSClasses(array('hide-on-mobile'));
			$nameGroup->addElement($help);
			$nameGroup->addElement(new htmlSpacer('0.25rem', '16px'));
			$nameGroup->addElement($descriptionText);
			$help = new htmlHelpLink($column['help'], $moduleName, $scope);
			$help->setCSSClasses(array('hide-on-tablet'));
			$nameGroup->addElement($help);
			$rowCells[] = $nameGroup;
			$rowCells[] = new htmlOutputText($column['name']);
			$example = '';
			if (isset($column['example'])) {
				$example = $column['example'];
			}
			$rowCells[] = new htmlOutputText($example);
			if (isset($column['default'])) {
				$rowCells[] = new htmlOutputText($column['default']);
			}
			else {
				$rowCells[] = new htmlOutputText('');
			}
			if (isset($column['values'])) {
				$rowCells[] = new htmlOutputText($column['values']);
			}
			else {
				$rowCells[] = new htmlOutputText('');
			}
			$data[] = $rowCells;
		}
		$table = new htmlResponsiveTable($titles, $data);
		$table->setCSSClasses(array('alternating-color'));
		$row->add($table, 12);
	}

	addSecurityTokenToMetaHTML($row);
	parseHtml(null, $row, array(), false, $scope);

	echo "</form>\n";

	// build sample CSV
	$sampleCSV_head = array();
	$sampleCSV_row = array();
		// DN attributes
		$sampleCSV_head[] = "\"dn_suffix\"";
		$sampleCSV_head[] = "\"dn_rdn\"";
		$sampleCSV_head[] = "\"overwrite\"";
		// module attributes
		foreach ($modules as $moduleName) {
			if (sizeof($columns[$moduleName]) < 1) {
				continue;
			}
			foreach ($columns[$moduleName] as $column) {
				$sampleCSV_head[] = "\"" . $column['name'] . "\"";
			}
		}
		$RDNs = getRDNAttributes($type->getId(), $selectedModules);
		// DN attributes
		$sampleCSV_row[] = "\"" . $type->getSuffix() . "\"";
		$sampleCSV_row[] = "\"" . $RDNs[0] . "\"";
		$sampleCSV_row[] = "\"false\"";
		// module attributes
		foreach ($modules as $moduleName) {
			if (sizeof($columns[$moduleName]) < 1) {
				continue;
			}
			foreach ($columns[$moduleName] as $column) {
				if (isset($column['example'])) {
					$sampleCSV_row[] = '"' . $column['example'] . '"';
				}
				else {
					$sampleCSV_row[] = '""';
				}
			}
		}
	$sampleCSV = implode(",", $sampleCSV_head) . "\n" . implode(",", $sampleCSV_row) . "\n";
	$_SESSION['mass_csv'] = $sampleCSV;

	echo '</div>';
	include __DIR__ . '/../../lib/adminFooter.inc';
	die;
}
