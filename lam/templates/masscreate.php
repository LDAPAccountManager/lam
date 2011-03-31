<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2004 - 2011  Roland Gruber

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


// Start session
startSecureSession();

// die if no write access
if (!checkIfWriteAccessIsAllowed()) die();

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
	if (!$myType->supportsFileUpload()) {
		unset($types[$i]);
	}
}
$types = array_values($types);

// check if account specific page should be shown
if (isset($_POST['type'])) {
	// get selected type
	$scope = $_POST['type'];
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
echo '<div class="' . $divClass . 'list-bright smallPaddingContent">';
echo "<div class=\"title\">\n";
echo "<h2 class=\"titleText\">" . _("Account creation via file upload") . "</h2>\n";
echo "</div>";
echo "<p>&nbsp;</p>\n";

echo "<p>\n";
	echo _("Here you can create multiple accounts by providing a CSV file.");
echo "</p>\n";

echo "<p>&nbsp;</p>\n";

echo "<form enctype=\"multipart/form-data\" action=\"masscreate.php\" method=\"post\">\n";

echo "<table style=\"border-color: grey\" cellpadding=\"10\" border=\"0\" cellspacing=\"0\">\n";
	echo "<tr><td>\n";
	echo '<b>' . _("Account type") . ':</b>';
	echo "</td>\n";
	echo "<td>\n";
	echo "<select class=\"$divClass\" name=\"type\" onChange=\"changeVisibleModules(this);\">\n";
	$sortedTypes = array();
	for ($i = 0; $i < sizeof($types); $i++) {
		$sortedTypes[$types[$i]] = getTypeAlias($types[$i]);
	}
	natcasesort($sortedTypes);
	foreach ($sortedTypes as $key => $value) {
		$selected = '';
		if (isset($_REQUEST['type']) && ($_REQUEST['type'] == $key)) {
			$selected = 'selected';
		}
		echo "<option value=\"" . $key . "\" $selected>" . $value . "</option>\n";
	}
	echo "</select>\n";
	echo "</td></tr>\n";
	echo "<tr><td valign=\"top\">\n";
		echo '<b>' . _('Selected modules') . ':</b>';
	echo "</td>\n";
	echo "<td>\n";
	// generate one DIV for each account type
	$counter = 0;
	foreach ($sortedTypes as $type => $label) {
		$style = 'style="display:none;"';
		if ((!isset($_REQUEST['type']) && ($counter == 0)) || (isset($_REQUEST['type']) && ($_REQUEST['type'] == $type))) {
			// show first account type or last selected one
			$style = '';
		}
		echo "<div $style id=\"" . $type . "\" class=\"typeOptions\">\n";
		echo "<table border=0>";
		$modules = $_SESSION['config']->get_AccountModules($type);
		for ($m = 0; $m < sizeof($modules); $m++) {
			if ($m%3 == 0) {
				echo "<tr>\n";
			}
			echo "<td>";
				$module = new $modules[$m]($type);
				$iconImage = $module->getIcon();
				echo '<img align="middle" src="../graphics/' . $iconImage . '" alt="' . $iconImage . '">';
			echo "</td><td>\n";
				if (is_base_module($modules[$m], $type)) {
					echo "<input type=\"hidden\" name=\"" . $type . '_' . $modules[$m] . "\" value=\"on\"><input type=\"checkbox\" checked disabled>";
				}
				else {
					$checked = 'checked';
					if (isset($_POST['submit']) && !isset($_POST[$type . '_' . $modules[$m]])) {
						$checked = '';
					}
					echo "<input type=\"checkbox\" name=\"" . $type . '_' . $modules[$m] . "\" $checked>";
				}
				echo getModuleAlias($modules[$m], $type);
			echo "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>";
			if (($m%3 == 2) && ($m != (sizeof($modules) - 1))) {
				echo "</tr>\n";
			}
		}
		echo "</tr>";
		echo "</table>\n";
		echo "</div>\n";
		$counter++;
	}
	echo "</td></tr>\n";
	echo "<tr><td>\n";
		echo "<button id=\"okButton\" class=\"smallPadding\" name=\"submit\">". _("Ok") . "</button>\n";
		?>
		<script type="text/javascript">
		jQuery(document).ready(function() {
			jQuery('#okButton').button();
		});
		function changeVisibleModules(element) {
			jQuery('div.typeOptions').toggle(false);
			jQuery('div#' + element.options[element.selectedIndex].value).toggle();
		}
		</script>
		<?php
	echo "</td></tr>\n";
echo "</table>\n";
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
	echo '<div class="' . $scope . 'list-bright smallPaddingContent">';
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
	$inputContainer->addElement(new htmlButton('submitfile', _('Upload file and create accounts')));
	$inputContainer->addElement(new htmlSpacer('10px', null));
	$inputContainer->addElement(new htmlLink(_("Download sample CSV file"), 'masscreate.php?getCSV=1'));
	$inputContainer->addElement(new htmlHiddenInput('scope', $scope));
	$inputContainer->addElement(new htmlHiddenInput('selectedModules', implode(',', $selectedModules)), true);
	$container->addElement($inputContainer, true);
	$container->addElement(new htmlSpacer(null, '10px'), true);
	// column list
	$columnSpacer = new htmlSpacer('10px', null);
	$container->addElement(new htmlTitle(_("Columns")), true);
	$columnContainer = new htmlTable();
	$columnContainer->setCSSClasses($scope . 'list');
	$columnContainer->addElement($columnSpacer);
	$columnContainer->addElement(new htmlOutputText(''));
	$columnContainer->addElement($columnSpacer);
	$columnContainer->addElement(new htmlOutputText(''));
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
	$columnContainer->addElement($header4, true, true);
	// DN options
	$dnTitle = new htmlSubTitle(_("DN settings"), '../graphics/logo32.png');
	$dnTitle->colspan = 20;
	$columnContainer->addElement($dnTitle);
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
	$dnSuffixRow->setCSSClasses($scope . 'list-dark');
	$columnContainer->addElement($dnSuffixRow);
	$dnRDNRowCells = array();
	$dnRDNRowCells[] = $columnSpacer;
	$dnRDNRowCells[] = new htmlHelpLink('301');
	$dnRDNRowCells[] = $columnSpacer;
	$dnRDNRowCells[] = new htmlOutputText(_("RDN identifier") . '*');
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
	$dnRDNRow->setCSSClasses($scope . 'list-bright');
	$columnContainer->addElement($dnRDNRow);
	// module options
	for ($m = 0; $m < sizeof($modules); $m++) {
		// skip modules without upload columns
		if (sizeof($columns[$modules[$m]]) < 1) {
			continue;
		}
		$icon = '';
		$module = new $modules[$m]($scope);
		$iconImage = $module->getIcon();
		if ($iconImage != null) {
			$icon = '../graphics/' . $iconImage;
		}
		$moduleTitle = new htmlSubTitle(getModuleAlias($modules[$m], $scope), $icon);
		$moduleTitle->colspan = 20;
		$columnContainer->addElement($moduleTitle);
		$odd = true;
		for ($i = 0; $i < sizeof($columns[$modules[$m]]); $i++) {
			$required = '';
			if (isset($columns[$modules[$m]][$i]['required']) && ($columns[$modules[$m]][$i]['required'] == true)) {
				$required = '*';
			}
			$rowCells = array();
			$rowCells[] = $columnSpacer;
			$rowCells[] = new htmlHelpLink($columns[$modules[$m]][$i]['help'], $modules[$m], $scope);
			$rowCells[] = $columnSpacer;
			$rowCells[] = new htmlOutputText($columns[$modules[$m]][$i]['description'] . $required);
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
				$row->setCSSClasses($scope . 'list-dark');
			}
			else {
				$row->setCSSClasses($scope . 'list-bright');
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
