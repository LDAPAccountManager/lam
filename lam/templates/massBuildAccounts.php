<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2004  Roland Gruber

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

/** access to configuration */
include_once('../lib/config.inc');
/** status messages */
include_once('../lib/status.inc');
/** account modules */
include_once('../lib/modules.inc');


// Start session
session_save_path('../sess');
@session_start();

// Redirect to startpage if user is not loged in
if (!isset($_SESSION['loggedIn'])) {
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

echo $_SESSION['header'];
echo "<title>account upload</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\">\n";
echo "</head>\n";
echo "<body>\n";

if ($_FILES['inputfile'] && ($_FILES['inputfile']['size'] > 0)) {
	// check if input file is well formated
	$data = array();
	$ids = array();
	// get input fields from modules
	$columns = getUploadColumns($_POST['scope']);
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
	// check if all required attributes are given
	$errors = array();
	$checkcolumns = array();
	$columns = call_user_func_array('array_merge', $columns);
	for ($i = 0; $i < sizeof($columns); $i++) {
		if ($columns[$i]['required'] == true) {
			if (isset($ids[$columns[$i]['name']])) $checkcolumns[] = $ids[$columns[$i]['name']];
			else $errors[] = array(_("A required column is missing in your CSV file."), $columns[$i]['name']);
		}
	}
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
	// let modules build accounts
	if (sizeof($errors) < 1) {
		$accounts = buildUploadAccounts($_POST['scope'], $data, $ids);
		if ($accounts != false) {
			// set DN
			for ($i = 0; $i < sizeof($accounts); $i++) {
				if (!isset($accounts[$i][$data[$i][$ids['dn_rdn']]])) $errors[] = array(_("Data field for RDN is empty for account $i!"), "");
				// TODO check against list of possible RDN attributes
				else {
					$account_dn = $data[$i][$ids['dn_rdn']] . "=" . $accounts[$i][$data[$i][$ids['dn_rdn']]] . ",";
					if ($data[$i][$ids['dn_suffix']] == "") $account_dn = $account_dn . call_user_func(array($_SESSION['config'], "get_" . ucfirst($_POST['scope']) . "Suffix"));
					else $account_dn = $account_dn . $data[$i][$ids['dn_suffix']];
					$accounts[$i]['dn'] = $account_dn;
				}
			}
			// accounts were built, now add them to LDAP
			// TODO add to LDAP
		}
	}
	print_r($accounts);
	// if input data is invalid just display error messages (max 50)
	if (sizeof($errors) > 0) {
		for ($i = 0; (($i < sizeof($errors)) || ($i > 49)); $i++) StatusMessage("ERROR", $errors[$i][0], $errors[$i][1]);
	}
	else {
		// store accounts in session
		$_SESSION['mass_accounts'] = $_SESSION['ldap']->encrypt(serialize($accounts));
		// show links for upload and LDIF export
		echo "<h1 align=\"center\">" . _("LAM has checked your input and is now ready to create the accounts.") . "</h1>\n";
		echo "<p>&nbsp;</p>\n";
		echo "<p align=\"center\">\n";
		echo "<table align=\"center\" width=\"80%\"><tr>\n";
			echo "<td align=\"center\" width=\"50%\">\n";
			echo "<a href=\"massDoUpload.php\"><b>" . _("Upload accounts to LDAP") . "</b></a>";
			echo "</td>\n";
			echo "<td align=\"center\" width=\"50%\">\n";
			echo "<a href=\"massBuildAccounts.php?showldif=true\"><b>" . _("Show LDIF file") . "</b></a>";
			echo "</td>\n";
		echo "</tr></table>\n";
		echo "</p>\n";
	}
}

echo "</body>\n";
echo "</html>\n";


?>