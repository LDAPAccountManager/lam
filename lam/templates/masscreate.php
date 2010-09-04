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
echo "<h1>" . _("Account creation via file upload") . "</h1>\n";
echo "<p>&nbsp;</p>\n";

echo "<p>\n";
	echo _("Here you can create multiple accounts by providing a CSV file.");
echo "</p>\n";

echo "<p>&nbsp;</p>\n";

echo '<script type="text/javascript">';
echo 'function changeVisibleModules(element) {';
echo 'jQuery(\'div.typeOptions\').toggle(false);';
echo 'jQuery(\'div#\' + element.options[element.selectedIndex].value).toggle();';
echo '}';
echo '</script>';

echo "<form enctype=\"multipart/form-data\" action=\"masscreate.php\" method=\"post\">\n";

echo "<table style=\"border-color: grey\" cellpadding=\"10\" border=\"0\" cellspacing=\"0\">\n";
	echo "<tr><td>\n";
	echo '<b>' . _("Account type") . ':</b>';
	echo "</td>\n";
	echo "<td>\n";
	echo "<select class=\"user\" name=\"type\" onChange=\"changeVisibleModules(this);\">\n";
	$sortedTypes = array();
	for ($i = 0; $i < sizeof($types); $i++) {
		$sortedTypes[$types[$i]] = getTypeAlias($types[$i]);
	}
	natcasesort($sortedTypes);
	foreach ($sortedTypes as $key => $value) {
		$selected = '';
		if (isset($_POST['type']) && ($_POST['type'] == $key)) {
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
		if ((!isset($_POST['type']) && ($counter == 0)) || (isset($_POST['type']) && ($_POST['type'] == $type))) {
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
			echo "</td>";
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
		echo "<input class=\"user\" type=\"submit\" name=\"submit\" value=\"". _("Ok") . "\">\n";
	echo "</td></tr>\n";
echo "</table>\n";
echo "</form>\n";

include 'main_footer.php';

/**
* Displays the acount type specific main page of the upload.
*
* @param string $scope account type
* @param array $selectedModules list of selected account modules
*/
function showMainPage($scope, $selectedModules) {
	echo "<h1>" . _("File upload") . "</h1>";
	echo "<p>\n";
		echo _("Please provide a CSV formated file with your account data. The cells in the first row must be filled with the column identifiers. The following rows represent one account for each row.");
		echo "<br>";
		echo _("Check your input carefully. LAM will only do some basic checks on the upload data.");
		echo "<br><br>";
		echo _("Hint: Format all cells as text in your spreadsheet program and turn off auto correction.");
	echo "</p>\n";
	
	echo "<p>&nbsp;</p>\n";

	echo "<form enctype=\"multipart/form-data\" action=\"massBuildAccounts.php\" method=\"post\">\n";
	echo "<p>\n";
	echo "<b>" . _("CSV file:") . "</b> <input class=\"$scope\" name=\"inputfile\" type=\"file\">&nbsp;&nbsp;";
	echo "<input class=\"$scope\" name=\"submitfile\" type=\"submit\" value=\"" . _('Upload file and create accounts') . "\">\n";
	echo "<input type=\"hidden\" name=\"scope\" value=\"$scope\">\n";
	echo "<input type=\"hidden\" name=\"selectedModules\" value=\"" . implode(',', $selectedModules) . "\">\n";
	echo "&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"masscreate.php?getCSV=1\"><b>" . _("Download sample CSV file") . "</b></a>\n";
	echo "</p>\n";
	echo "</form>\n";

	echo "<p>&nbsp;</p>\n";
	
	echo _("Here is a list of possible columns. The red columns must be included in the CSV file and filled with data for all accounts.");

	echo "<p><big><b>" . _("Columns:") . "</b></big></p>\n";

	// DN options
	echo "<fieldset class=\"" . $scope . "edit\">\n<legend><b>" . _("DN settings") . "</b></legend><br>\n";
	echo "<table width=\"100%\">\n";
		echo "<tr valign=\"top\">\n";
			echo "<td width=\"50%\">\n";
			echo "<b>" . _("DN suffix") . "</b>\n";
			// help link
			echo "&nbsp;";
			printHelpLink(getHelp('', '361'), '361');
			echo "<br>\n";
				echo "<ul>\n";
					echo "<li><b>" . _("Identifier") . ":</b> " . "dn_suffix</li>\n";
					echo "<li><b>" . _("Example value") . ":</b> " . _("ou=accounts,dc=yourdomain,dc=org") . "</li>\n";
					echo "<li><b>" . _("Default value") . ":</b> " . $_SESSION['config']->get_Suffix($scope) . "</li>\n";
				echo "</ul>\n";
			echo "</td>\n";
			echo "<td width=\"50%\">\n";
			echo "<b><font color=\"red\">" . _("RDN identifier") . "</font></b>\n";
			// help link
			echo "&nbsp;";
			printHelpLink(getHelp('', '301'), '301');
			echo "<br>\n";
				echo "<ul>\n";
					echo "<li><b>" . _("Identifier") . ":</b> " . "dn_rdn</li>\n";
					echo "<li><b>" . _("Possible values") . ":</b> " . implode(", ", getRDNAttributes($scope, $selectedModules)) . "</li>\n";
				echo "</ul>\n";
			echo "</td>\n";
		echo "</tr>\n";
	echo "</table>\n";
	echo "</fieldset><br>\n";
	
	// get input fields from modules
	$columns = getUploadColumns($scope, $selectedModules);

	// print input fields
	$modules = array_keys($columns);
	for ($m = 0; $m < sizeof($modules); $m++) {
		// skip modules without upload columns
		if (sizeof($columns[$modules[$m]]) < 1) {
			continue;
		}
		$icon = '';
		$module = new $modules[$m]($scope);
		$iconImage = $module->getIcon();
		if ($iconImage != null) {
			$icon = '<img align="middle" src="../graphics/' . $iconImage . '" alt="' . $iconImage . '"> ';
		}
		echo "<fieldset class=\"" . $scope . "edit\">\n<legend>$icon<b>" . getModuleAlias($modules[$m], $scope) . "</b></legend><br>\n";
		echo "<table width=\"100%\">\n";
		for ($i = 0; $i < sizeof($columns[$modules[$m]]); $i++) {
			echo "<tr valign=\"top\">\n";
				echo "<td width=\"33%\">\n";
					showColumnData($modules[$m], $columns[$modules[$m]][$i], $scope);
				echo "</td>\n";
				$i++;
				if ($i < sizeof($columns[$modules[$m]])) {
					echo "<td width=\"33%\">\n";
						showColumnData($modules[$m], $columns[$modules[$m]][$i], $scope);
					echo "</td>\n";
					$i++;
					if ($i < sizeof($columns[$modules[$m]])) {
						echo "<td width=\"33%\">\n";
							showColumnData($modules[$m], $columns[$modules[$m]][$i], $scope);
						echo "</td>\n";
					}
					else echo "<td width=\"33%\"></td>"; // empty cell if no more fields
				}
				else echo "<td width=\"33%\"></td><td width=\"33%\"></td>"; // empty cell if no more fields
			echo "</tr>\n";
		}
		echo "</table>\n";
		echo "</fieldset><br>";
	}

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
				$sampleCSV_row[] = "\"" . $columns[$modules[$m]][$i]['example'] . "\"";
			}
		}
	$sampleCSV = implode(",", $sampleCSV_head) . "\n" . implode(",", $sampleCSV_row) . "\n";
	$_SESSION['mass_csv'] = $sampleCSV;
	
	include 'main_footer.php';
	die;
}

/**
* Prints the properties of one input field.
*
* @param string $module account module name
* @param array $data field data from modules
* @param string $scope account type
*/
function showColumnData($module, $data, $scope) {
	if (isset($data['required']) && ($data['required'] == true)) {
		echo "<font color=\"red\"><b>\n";
			echo $data['description'];
		echo "</b></font>\n";
	}
	else {
		echo "<b>\n";
			echo $data['description'];
		echo "</b>\n";
	}
	// help link
	echo "&nbsp;";
	printHelpLink(getHelp($module, $data['help'], $scope), $data['help'], $module, $scope);
	echo "<br>\n";
	echo "<ul>\n";
		echo "<li>\n";
			echo "<b>" . _("Identifier") . ":</b> " . $data['name'] . "\n";
		echo "</li>\n";
		if (isset($data['values'])) {
			echo "<li>\n";
				echo "<b>" . _("Possible values") . ":</b> " . $data['values'] . "\n";
			echo "</li>\n";
		}
		echo "<li>\n";
			echo "<b>" . _("Example value") . ":</b> " . $data['example'] . "\n";
		echo "</li>\n";
		if (isset($data['default'])) {
			echo "<li>\n";
				echo "<b>" . _("Default value") . ":</b> " . $data['default'] . "\n";
			echo "</li>\n";
		}

	echo "</ul>\n";
}

?>
