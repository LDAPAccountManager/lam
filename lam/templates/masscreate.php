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
* Start page of file upload
*
* @author Roland Gruber
* @package tools
*/

/** security functions */
include_once("../lib/security.inc");
/** access to configuration */
include_once('../lib/config.inc');
/** status messages */
include_once('../lib/status.inc');
/** account modules */
include_once('../lib/modules.inc');
/** Used to get PDF information. */
include_once('../lib/pdfstruct.inc');


// Start session
startSecureSession();

// check if this tool may be run
checkIfToolIsActive('toolFileUpload');

// die if no write access
if (!checkIfWriteAccessIsAllowed()) die();

checkIfToolIsActive('toolFileUpload');

// Redirect to startpage if user is not loged in
if (!isset($_SESSION['loggedIn']) || ($_SESSION['loggedIn'] !== true)) {
	metaRefresh("login.php");
	exit;
}

// Set correct language, codepages, ....
setlanguage();

// show CSV if requested
if (isset($_GET['getCSV'])) {
	//download file
	if(isset($HTTP_SERVER_VARS['HTTP_USER_AGENT']) and strpos($HTTP_SERVER_VARS['HTTP_USER_AGENT'],'MSIE')) {
		Header('Content-Type: application/force-download');
	}
	else {
		Header('Content-Type: application/msexcel');
	}
	Header('Content-disposition: attachment; filename=lam.csv');
	echo $_SESSION['mass_csv'];
	exit;
}

include 'main_header.php';

// get possible types and remove those which do not support file upload
$types = $_SESSION['config']->get_ActiveTypes();
for ($i = 0; $i < sizeof($types); $i++) {
	$myType = new $types[$i]();
	if (!$myType->supportsFileUpload() || isAccountTypeHidden($types[$i])) {
		unset($types[$i]);
	}
}
$types = array_values($types);

// check if account specific page should be shown
if (isset($_POST['type'])) {
	// get selected type
	$scope = htmlspecialchars($_POST['type']);
	// get selected modules
	$selectedModules = array();
	$checkedBoxes = array_keys($_POST, 'on');
	for ($i = 0; $i < sizeof($checkedBoxes); $i++) {
		if (strpos($checkedBoxes[$i], $scope . '_') === 0) {
			$selectedModules[] = substr($checkedBoxes[$i], strlen($scope) + 1);
		}
	}
	$deps = getModulesDependencies($scope);
	$depErrors = check_module_depends($selectedModules, $deps);
	if (is_array($depErrors) && (sizeof($depErrors) > 0)) {
		for ($i = 0; $i < sizeof($depErrors); $i++) {
			StatusMessage('ERROR', _("Unsolved dependency:") . ' ' .
							getModuleAlias($depErrors[$i][0], $scope) . " (" .
							getModuleAlias($depErrors[$i][1], $scope) . ")");
		}
	}
	else {
		showMainPage($scope, $selectedModules);
		exit;
	}
}

// show start page
$divClass = 'user';
if (isset($_REQUEST['type'])) {
	$divClass = $_REQUEST['type'];
}
echo '<div class="' . $divClass . '-bright smallPaddingContent">';
echo "<div class=\"title\">\n";
echo "<h2 class=\"titleText\">" . _("Account creation via file upload") . "</h2>\n";
echo "</div>";
echo "<p>&nbsp;</p>\n";

echo "<p>\n";
	echo _("Here you can create multiple accounts by providing a CSV file.");
echo "</p>\n";

echo "<p>&nbsp;</p>\n";

echo "<form enctype=\"multipart/form-data\" action=\"masscreate.php\" method=\"post\">\n";

$tabindex = 1;
$table = new htmlTable();

// account type
$typeList = array();
for ($i = 0; $i < sizeof($types); $i++) {
	$typeList[getTypeAlias($types[$i])] = $types[$i];
}
$selectedType = array();
if (isset($_REQUEST['type'])) {
	$selectedType[] = $_REQUEST['type'];
}
else {
	$selectedType[] = $types[0];
}
$typeSelect = new htmlTableExtendedSelect('type', $typeList, $selectedType, _("Account type"));
$typeSelect->setHasDescriptiveElements(true);
$typeSelect->setOnchangeEvent('changeVisibleModules(this);');
$table->addElement($typeSelect, true);
$table->addElement(new htmlSpacer(null, '10px'), true);

// module selection
$moduleLabel = new htmlOutputText(_('Selected modules'));
$moduleLabel->alignment = htmlElement::ALIGN_TOP;
$table->addElement($moduleLabel);
$moduleGroup = new htmlGroup();
for ($i = 0; $i < sizeof($types); $i++) {
	$divClasses = array('typeOptions');
	if ((!isset($_REQUEST['type']) && ($i != 0)) || (isset($_REQUEST['type']) && ($_REQUEST['type'] != $types[$i]))) {
		$divClasses[] = 'hidden';
	}
	$innerTable = new htmlTable();
	$modules = $_SESSION['config']->get_AccountModules($types[$i]);
	for ($m = 0; $m < sizeof($modules); $m++) {
		if (($m != 0) && ($m%3 == 0)) {
			echo $innerTable->addNewLine();
		}
		$module = new $modules[$m]($types[$i]);
		$iconImage = '../graphics/' . $module->getIcon();
		$innerTable->addElement(new htmlImage($iconImage));
		$enabled = true;
		if (is_base_module($modules[$m], $types[$i])) {
			$enabled = false;
		}
		$checked = true;
		if (isset($_POST['submit']) && !isset($_POST[$types[$i] . '_' . $modules[$m]])) {
			$checked = false;
		}
		$checkbox = new htmlTableExtendedInputCheckbox($types[$i] . '_' . $modules[$m], $checked, getModuleAlias($modules[$m], $types[$i]), null, false);
		$checkbox->setIsEnabled($enabled);
		if ($enabled) {
			$innerTable->addElement($checkbox);
		}
		else {
			$boxGroup = new htmlGroup();
			$boxGroup->addElement($checkbox);
			// add hidden field to fake disabled checkbox value
			$boxGroup->addElement(new htmlHiddenInput($types[$i] . '_' . $modules[$m], 'on'));
			$innerTable->addElement($boxGroup);
		}
		$innerTable->addElement(new htmlSpacer('10px', null));
	}
	$typeDiv = new htmlDiv($types[$i], $innerTable);
	$typeDiv->setCSSClasses($divClasses);
	$moduleGroup->addElement($typeDiv);
}
$table->addElement($moduleGroup, true);

// ok button
$table->addElement(new htmlSpacer(null, '20px'), true);
$table->addElement(new htmlButton('submit', _('Ok')), true);

parseHtml(null, $table, array(), false, $tabindex, 'user');
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
include 'main_footer.php';

/**
* Displays the acount type specific main page of the upload.
*
* @param string $scope account type
* @param array $selectedModules list of selected account modules
*/
function showMainPage($scope, $selectedModules) {
	echo '<div class="' . $scope . '-bright smallPaddingContent">';
	// get input fields from modules
	$columns = getUploadColumns($scope, $selectedModules);
	$modules = array_keys($columns);
	
	echo "<form enctype=\"multipart/form-data\" action=\"massBuildAccounts.php\" method=\"post\">\n";
	$container = new htmlTable();
	// title
	$container->addElement(new htmlTitle(_("File upload")), true);
	$container->addElement(new htmlSpacer(null, '10px'), true);
	// instructions
	$container->addElement(new htmlOutputText(_("Please provide a CSV formated file with your account data. The cells in the first row must be filled with the column identifiers. The following rows represent one account for each row.")), true);
	$container->addElement(new htmlOutputText(_("Check your input carefully. LAM will only do some basic checks on the upload data.")), true);
	$container->addElement(new htmlSpacer(null, '10px'), true);
	$container->addElement(new htmlOutputText(_("Hint: Format all cells as text in your spreadsheet program and turn off auto correction.")), true);
	$container->addElement(new htmlSpacer(null, '10px'), true);
	// upload elements
	$inputContainer = new htmlTable();
	$inputContainer->addElement(new htmlOutputText(_("CSV file")));
	$inputContainer->addElement(new htmlInputFileUpload('inputfile'));
	$inputContainer->addElement(new htmlSpacer('10px', null));
	$inputContainer->addElement(new htmlLink(_("Download sample CSV file"), 'masscreate.php?getCSV=1', '../graphics/save.png', true));
	$inputContainer->addElement(new htmlHiddenInput('scope', $scope));
	$inputContainer->addElement(new htmlHiddenInput('selectedModules', implode(',', $selectedModules)), true);
	// PDF
	$createPDF = false;
	if (isset($_POST['createPDF']) && ($_POST['createPDF'] === '1')) {
		$createPDF = true;
	}
	$pdfCheckbox = new htmlTableExtendedInputCheckbox('createPDF', $createPDF, _('Create PDF files'));
	$pdfCheckbox->setTableRowsToShow(array('pdfStructure'));
	$inputContainer->addElement($pdfCheckbox, true);
	$pdfStructures = getPDFStructureDefinitions($scope);
	$pdfSelected = array();
	if (isset($_POST['pdfStructure'])) {
		$pdfSelected = array($_POST['pdfStructure']);
	}
	else if (in_array('default', $pdfStructures)) {
		$pdfSelected = array('default');
	}
	$inputContainer->addElement(new htmlTableExtendedSelect('pdfStructure', $pdfStructures, $pdfSelected, _('PDF structure')), true);
	$inputContainer->addElement(new htmlSpacer(null, '5px'), true);
	$uploadButton = new htmlButton('submitfile', _('Upload file and create accounts'));
	$uploadButton->setIconClass('upButton');
	$inputContainer->addElement($uploadButton);
	$container->addElement($inputContainer, true);
	$container->addElement(new htmlSpacer(null, '10px'), true);
	// column list
	$columnSpacer = new htmlSpacer('10px', null);
	$container->addElement(new htmlTitle(_("Columns")), true);
	$columnContainer = new htmlTable();
	$columnContainer->setCSSClasses(array($scope . 'list', 'collapse'));
	// DN options
	$dnTitle = new htmlSubTitle(_("DN settings"), '../graphics/logo32.png');
	$dnTitle->colspan = 20;
	$columnContainer->addElement($dnTitle, true);
	$columnContainer->addElement($columnSpacer);
	$columnContainer->addElement(new htmlOutputText(''));
	$columnContainer->addElement($columnSpacer);
	$header0 = new htmlOutputText(_('Name'));
	$header0->alignment = htmlElement::ALIGN_LEFT;
	$columnContainer->addElement($header0, false, true);
	$columnContainer->addElement($columnSpacer);
	$header1 = new htmlOutputText(_("Identifier"));
	$header1->alignment = htmlElement::ALIGN_LEFT;
	$columnContainer->addElement($header1, false, true);
	$columnContainer->addElement($columnSpacer);
	$header2 = new htmlOutputText(_("Example value"));
	$header2->alignment = htmlElement::ALIGN_LEFT;
	$columnContainer->addElement($header2, false, true);
	$columnContainer->addElement($columnSpacer);
	$header3 = new htmlOutputText(_("Default value"));
	$header3->alignment = htmlElement::ALIGN_LEFT;
	$columnContainer->addElement($header3, false, true);
	$columnContainer->addElement($columnSpacer);
	$header4 = new htmlOutputText(_("Possible values"));
	$header4->alignment = htmlElement::ALIGN_LEFT;
	$columnContainer->addElement($header4, false, true);
	$dnSuffixRowCells = array();
	$dnSuffixRowCells[] = $columnSpacer;
	$dnSuffixRowCells[] = new htmlHelpLink('361');
	$dnSuffixRowCells[] = $columnSpacer;
	$dnSuffixRowCells[] = new htmlOutputText(_("DN suffix"));
	$dnSuffixRowCells[] = $columnSpacer;
	$dnSuffixRowCells[] = new htmlOutputText('dn_suffix');
	$dnSuffixRowCells[] = $columnSpacer;
	$dnSuffixRowCells[] = new htmlOutputText($_SESSION['config']->get_Suffix($scope));
	$dnSuffixRowCells[] = $columnSpacer;
	$dnSuffixRowCells[] = new htmlOutputText($_SESSION['config']->get_Suffix($scope));
	$dnSuffixRowCells[] = $columnSpacer;
	$dnSuffixRowCells[] = new htmlOutputText('');
	$dnSuffixRowCells[] = new htmlSpacer(null, '25px');
	$dnSuffixRow = new htmlTableRow($dnSuffixRowCells);
	$dnSuffixRow->setCSSClasses(array($scope . '-dark'));
	$columnContainer->addElement($dnSuffixRow);
	$dnRDNRowCells = array();
	$dnRDNRowCells[] = $columnSpacer;
	$dnRDNRowCells[] = new htmlHelpLink('301');
	$dnRDNRowCells[] = $columnSpacer;
	$rdnText = new htmlOutputText(_("RDN identifier"));
	$rdnText->setMarkAsRequired(true);
	$dnRDNRowCells[] = $rdnText;
	$dnRDNRowCells[] = $columnSpacer;
	$dnRDNRowCells[] = new htmlOutputText('dn_rdn');
	$dnRDNRowCells[] = $columnSpacer;
	$rdnAttributes = getRDNAttributes($scope, $selectedModules);
	$dnRDNRowCells[] = new htmlOutputText($rdnAttributes[0]);
	$dnRDNRowCells[] = $columnSpacer;
	$dnRDNRowCells[] = new htmlOutputText('');
	$dnRDNRowCells[] = $columnSpacer;
	$dnRDNRowCells[] = new htmlOutputText(implode(", ", $rdnAttributes));
	$dnRDNRowCells[] = new htmlSpacer(null, '25px');
	$dnRDNRow = new htmlTableRow($dnRDNRowCells);
	$dnRDNRow->setCSSClasses(array($scope . '-bright'));
	$columnContainer->addElement($dnRDNRow);
	// module options
	for ($m = 0; $m < sizeof($modules); $m++) {
		// skip modules without upload columns
		if (sizeof($columns[$modules[$m]]) < 1) {
			continue;
		}
		$columnContainer->addElement(new htmlSpacer(null, '10px'), true);
		$icon = '';
		$module = new $modules[$m]($scope);
		$iconImage = $module->getIcon();
		if ($iconImage != null) {
			$icon = '../graphics/' . $iconImage;
		}
		$moduleTitle = new htmlSubTitle(getModuleAlias($modules[$m], $scope), $icon);
		$moduleTitle->colspan = 20;
		$columnContainer->addElement($moduleTitle, true);
		$columnContainer->addElement(new htmlOutputText(''));
		$columnContainer->addElement(new htmlOutputText(''));
		$columnContainer->addElement(new htmlOutputText(''));
		$nameOut = new htmlOutputText(_('Name'));
		$nameOut->alignment = htmlElement::ALIGN_LEFT;
		$columnContainer->addElement($nameOut, false, true);
		$columnContainer->addElement(new htmlOutputText(''));
		$idOut = new htmlOutputText(_('Identifier'));
		$idOut->alignment = htmlElement::ALIGN_LEFT;
		$columnContainer->addElement($idOut, false, true);
		$columnContainer->addElement(new htmlOutputText(''));
		$exampleOut = new htmlOutputText(_('Example value'));
		$exampleOut->alignment = htmlElement::ALIGN_LEFT;
		$columnContainer->addElement($exampleOut, false, true);
		$columnContainer->addElement(new htmlOutputText(''));
		$defaultOut = new htmlOutputText(_('Default value'));
		$defaultOut->alignment = htmlElement::ALIGN_LEFT;
		$columnContainer->addElement($defaultOut, false, true);
		$columnContainer->addElement(new htmlOutputText(''));
		$possibleOut = new htmlOutputText(_('Possible values'));
		$possibleOut->alignment = htmlElement::ALIGN_LEFT;
		$columnContainer->addElement($possibleOut, false, true);
		$odd = true;
		for ($i = 0; $i < sizeof($columns[$modules[$m]]); $i++) {
			$required = false;
			if (isset($columns[$modules[$m]][$i]['required']) && ($columns[$modules[$m]][$i]['required'] == true)) {
				$required = true;
			}
			$rowCells = array();
			$rowCells[] = $columnSpacer;
			$rowCells[] = new htmlHelpLink($columns[$modules[$m]][$i]['help'], $modules[$m], $scope);
			$rowCells[] = $columnSpacer;
			$descriptionText = new htmlOutputText($columns[$modules[$m]][$i]['description']);
			$descriptionText->setMarkAsRequired($required);
			$descriptionText->setNoWrap(true);
			$rowCells[] = $descriptionText;
			$rowCells[] = $columnSpacer;
			$rowCells[] = new htmlOutputText($columns[$modules[$m]][$i]['name']);
			$rowCells[] = $columnSpacer;
			$example = '';
			if (isset($columns[$modules[$m]][$i]['example'])) {
				$example = $columns[$modules[$m]][$i]['example'];
			}
			$rowCells[] = new htmlOutputText($example);
			$rowCells[] = $columnSpacer;
			if (isset($columns[$modules[$m]][$i]['default'])) {
				$rowCells[] = new htmlOutputText($columns[$modules[$m]][$i]['default']);
			}
			else {
				$rowCells[] = new htmlOutputText('');
			}
			$rowCells[] = $columnSpacer;
			if (isset($columns[$modules[$m]][$i]['values'])) {
				$rowCells[] = new htmlOutputText($columns[$modules[$m]][$i]['values']);
			}
			else {
				$rowCells[] = new htmlOutputText('');
			}
			$rowCells[] = new htmlSpacer(null, '25px');
			$row = new htmlTableRow($rowCells);
			if ($odd) {
				$row->setCSSClasses(array($scope . '-dark'));
			}
			else {
				$row->setCSSClasses(array($scope . '-bright'));
			}
			$odd = !$odd;
			$columnContainer->addElement($row);
		}
	}
	$container->addElement($columnContainer, true);
	
	$tabindex = 1;
	parseHtml(null, $container, array(), false, $tabindex, $scope);
	
	echo "</form>\n";

	// build sample CSV
	$sampleCSV_head = array();
	$sampleCSV_row = array();
		// DN attributes
		$sampleCSV_head[] = "\"dn_suffix\"";
		$sampleCSV_head[] = "\"dn_rdn\"";
		// module attributes
		for ($m = 0; $m < sizeof($modules); $m++) {
			if (sizeof($columns[$modules[$m]]) < 1) continue;
			for ($i = 0; $i < sizeof($columns[$modules[$m]]); $i++) {
				$sampleCSV_head[] = "\"" . $columns[$modules[$m]][$i]['name'] . "\"";
			}
		}
		$RDNs = getRDNAttributes($scope, $selectedModules);
		// DN attributes
		$sampleCSV_row[] = "\"" . $_SESSION['config']->get_Suffix($scope) . "\"";
		$sampleCSV_row[] = "\"" . $RDNs[0] . "\"";
		// module attributes
		for ($m = 0; $m < sizeof($modules); $m++) {
			if (sizeof($columns[$modules[$m]]) < 1) continue;
			for ($i = 0; $i < sizeof($columns[$modules[$m]]); $i++) {
				if (isset($columns[$modules[$m]][$i]['example'])) {
					$sampleCSV_row[] = '"' . $columns[$modules[$m]][$i]['example'] . '"';
				}
				else {
					$sampleCSV_row[] = '""';
				}
			}
		}
	$sampleCSV = implode(",", $sampleCSV_head) . "\n" . implode(",", $sampleCSV_row) . "\n";
	$_SESSION['mass_csv'] = $sampleCSV;
	
	echo '</div>';
	include 'main_footer.php';
	die;
}

?>
