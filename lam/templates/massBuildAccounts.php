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
* Creates the accounts by parsing the uploaded file.
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

// check if this tool may be run
checkIfToolIsActive('toolFileUpload');

// die if no write access
if (!checkIfWriteAccessIsAllowed()) die();

// Redirect to startpage if user is not loged in
if (!isset($_SESSION['loggedIn']) || ($_SESSION['loggedIn'] !== true)) {
	metaRefresh("login.php");
	exit;
}

// Set correct language, codepages, ....
setlanguage();

// show LDIF if requested
if (isset($_GET['showldif'])) {
	//download file
	if(isset($HTTP_SERVER_VARS['HTTP_USER_AGENT']) and strpos($HTTP_SERVER_VARS['HTTP_USER_AGENT'],'MSIE')) {
		Header('Content-Type: application/force-download');
	}
	else {
		Header('Content-Type: text/plain');
	}
	Header('Content-disposition: attachment; filename=lam.ldif');
	$accounts = unserialize($_SESSION['ldap']->decrypt($_SESSION['mass_accounts']));
	for ($i = 0; $i < sizeof($accounts); $i++) {
		echo "DN: " . $accounts[$i]['dn'] . "\n";
		unset($accounts[$i]['dn']);
		$keys = array_keys($accounts[$i]);
		for ($k = 0; $k < sizeof($keys); $k++) {
			if (strpos($keys[$k], 'INFO.') === 0) {
				continue;
			}
			if (is_array($accounts[$i][$keys[$k]])) {
				for ($x = 0; $x < sizeof($accounts[$i][$keys[$k]]); $x++) {
					echo $keys[$k] . ": " . $accounts[$i][$keys[$k]][$x] . "\n";
				}
			}
			else {
				echo $keys[$k] . ": " . $accounts[$i][$keys[$k]] . "\n";
			}
		}
		echo "\n";
	}
	exit;
}

include 'main_header.php';
$scope = htmlspecialchars($_POST['scope']);

// check if account type is ok
if (isAccountTypeHidden($scope)) {
	logNewMessage(LOG_ERR, 'User tried to access hidden upload: ' . $scope);
	die();
}
if (!checkIfNewEntriesAreAllowed($scope)) {
	logNewMessage(LOG_ERR, 'User tried to access forbidden upload: ' . $scope);
	die();
}

echo '<form enctype="multipart/form-data" action="masscreate.php" method="post">';
echo '<div class="' . $scope . '-bright smallPaddingContent">';
$container = new htmlTable();

$selectedModules = explode(',', $_POST['selectedModules']);
if ($_FILES['inputfile'] && ($_FILES['inputfile']['size'] > 0)) {
	// check if input file is well formated
	$data = array();  // input values without first row
	$ids = array();  // <column name> => <column number for $data>
	// get input fields from modules
	$columns = getUploadColumns($scope, $selectedModules);
	// read input file
	$handle = fopen ($_FILES['inputfile']['tmp_name'], "r");
	if (($head = fgetcsv($handle, 2000)) !== false ) { // head row
		for ($i = 0; $i < sizeof($head); $i++) {
			$ids[$head[$i]] = $i;
		}
	}
	while (($line = fgetcsv($handle, 2000)) !== false ) { // account rows
		$data[] = $line;
	}
	
	$errors = array();
	
	// check if all required columns are present
	$checkcolumns = array();
	$columns = call_user_func_array('array_merge', $columns);
	for ($i = 0; $i < sizeof($columns); $i++) {
		if (isset($columns[$i]['required']) && ($columns[$i]['required'] == true)) {
			if (isset($ids[$columns[$i]['name']])) $checkcolumns[] = $ids[$columns[$i]['name']];
			else $errors[] = array(_("A required column is missing in your CSV file."), $columns[$i]['name']);
		}
	}
	
	// check if all required attributes are given
	$invalidColumns = array();
	$id_names = array_keys($ids);
	for ($i = 0; $i < sizeof($checkcolumns); $i++) {
		for ($r = 0; $r < sizeof($data); $r++) {
			if ($data[$r][$checkcolumns[$i]] == "") {
				$invalidColumns[] = $id_names[$checkcolumns[$i]];
				break;
			}
		}
	}
	for ($i = 0; $i < sizeof($data); $i++) {
		if ($data[$i][$ids['dn_rdn']] == "") {
			$invalidColumns[] = 'dn_rdn';
			break;
		}
	}
	for ($i = 0; $i < sizeof($invalidColumns); $i++) {
		$errors[] = array(_("One or more values of the required column \"$invalidColumns[$i]\" are missing."), "");
	}
	
	// check if values in unique columns are correct
	for ($i = 0; $i < sizeof($columns); $i++) {
		if (isset($columns[$i]['unique']) && ($columns[$i]['unique'] == true)) {
			$colNumber = $ids[$columns[$i]['name']];
			$values_given = array();
			for ($r = 0; $r < sizeof($data); $r++) {
				$values_given[] = $data[$r][$colNumber];
			}
			$values_unique = array_unique($values_given);
			if (sizeof($values_given) != sizeof($values_unique)) {
				$duplicates = array();
				foreach ($values_given as $key => $value) {
					if (!isset($values_unique[$key])) {
						$duplicates[] = htmlspecialchars($value);
					}
				}
				$duplicates = array_values(array_unique($duplicates));
				$errors[] = array(_("This column is defined to include unique entries but duplicates were found:") . ' ' . $columns[$i]['name'], implode(', ', $duplicates));
			}
		}
	}
	
	// if input data is invalid just display error messages (max 50)
	if (sizeof($errors) > 0) {
		for ($i = 0; $i < sizeof($errors); $i++) {
			$container->addElement(new htmlStatusMessage("ERROR", $errors[$i][0], $errors[$i][1]), true);
		}
		$container->addElement(new htmlSpacer(null, '10px'), true);
		massPrintBackButton($scope, $selectedModules, $container);
	}
	
	// let modules build accounts
	else {
		$accounts = buildUploadAccounts($scope, $data, $ids, $selectedModules);
		if ($accounts != false) {
			$rdnList = getRDNAttributes($scope, $selectedModules);
			$suffix = $_SESSION['config']->get_Suffix($scope);
			// set DN
			for ($i = 0; $i < sizeof($accounts); $i++) {
				// check against list of possible RDN attributes
				if (!in_array($data[$i][$ids['dn_rdn']], $rdnList)) {
					$errors[] = array(_('Account %s:') . ' dn_rdn' . $accounts[$i][$data[$i][$ids['dn_rdn']]], _("Invalid RDN attribute!"), array($i));
				}
				else {
					$account_dn = $data[$i][$ids['dn_rdn']] . "=" . $accounts[$i][$data[$i][$ids['dn_rdn']]] . ",";
					if ($data[$i][$ids['dn_suffix']] == "") $account_dn = $account_dn . $suffix;
					else $account_dn = $account_dn . $data[$i][$ids['dn_suffix']];
					$accounts[$i]['dn'] = $account_dn;
				}
			}
			// print errors if DN could not be built
			if (sizeof($errors) > 0) {
				for ($i = 0; $i < sizeof($errors); $i++) {
					$container->addElement(new htmlStatusMessage("ERROR", $errors[$i][0], $errors[$i][1], $errors[$i][2]), true);
				}
			}
			else {
				// store accounts in session
				$_SESSION['mass_accounts'] = $_SESSION['ldap']->encrypt(serialize($accounts));
				$_SESSION['mass_counter'] = 0;
				$_SESSION['mass_errors'] = array();
				$_SESSION['mass_failed'] = array();
				$_SESSION['mass_postActions'] = array();
				$_SESSION['mass_data'] = $_SESSION['ldap']->encrypt(serialize($data));
				$_SESSION['mass_ids'] = $ids;
				$_SESSION['mass_scope'] = $scope;
				$_SESSION['mass_selectedModules'] = $selectedModules;
				if (isset($_SESSION['mass_pdf'])) {
					unset($_SESSION['mass_pdf']);
				}
				if (isset($_POST['createPDF']) && ($_POST['createPDF'] == 'on')) {
					$_SESSION['mass_pdf']['structure'] = $_POST['pdfStructure'];
					$_SESSION['mass_pdf']['counter'] = 0;
					$_SESSION['mass_pdf']['file'] = '../tmp/lam_pdf' . $_SESSION['ldap']->new_rand() . '.zip';
				}
				else {
					$_SESSION['mass_pdf']['structure'] = null;
				}
				// show links for upload and LDIF export
				$container->addElement(new htmlTitle(_("LAM has checked your input and is now ready to create the accounts.")), true);
				$container->addElement(new htmlSpacer(null, '10px'), true);
				$buttonContainer = new htmlTable();
				$buttonContainer->addElement(new htmlLink(_("Upload accounts to LDAP"), 'massDoUpload.php', '../graphics/up.gif', true));
				$buttonContainer->addElement(new htmlLink(_("Show LDIF file"), 'massBuildAccounts.php?showldif=true', '../graphics/edit.png', true));
				$buttonContainer->addElement(new htmlSpacer('10px', null));
				massPrintBackButton($scope, $selectedModules, $buttonContainer);
				$container->addElement($buttonContainer, true);
			}
		}
		else {
			$container->addElement(new htmlSpacer(null, '10px'), true);
			massPrintBackButton($scope, $selectedModules, $container);
		}
	}
}
else {
	$container->addElement(new htmlStatusMessage('ERROR', _('Please provide a file to upload.')), true);
	$container->addElement(new htmlSpacer(null, '10px'), true);
	massPrintBackButton($scope, $selectedModules, $container);
}

$tabindex = 1;
parseHtml(null, $container, array(), false, $tabindex, $scope);

echo '</div>';
echo '</form>';
include 'main_footer.php';

/**
 * Prints a back button to the page where the user enters a file to upload.
 *
 * @param String $scope account type (e.g. user)
 * @param array $selectedModules selected modules for upload
 * @param htmlTable $container table container
 */
function massPrintBackButton($scope, $selectedModules, &$container) {
	$backButton = new htmlButton('submit', _('Back'));
	$backButton->setIconClass('backButton');
	$container->addElement($backButton);
	$container->addElement(new htmlHiddenInput('type', $scope));
	$createPDF = 0;
	if (isset($_POST['createPDF']) && ($_POST['createPDF'] == 'on')) {
		$createPDF = 1;
	}
	$container->addElement(new htmlHiddenInput('createPDF', $createPDF));
	$container->addElement(new htmlHiddenInput('pdfStructure', $_POST['pdfStructure']));
	for ($i = 0; $i < sizeof($selectedModules); $i++) {
		$container->addElement(new htmlHiddenInput($scope . '_' . $selectedModules[$i], 'on'));
	}
}

?>