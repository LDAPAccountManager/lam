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
* Start page of file upload
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

echo $_SESSION['header'];
echo "<title>account upload</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\">\n";
echo "</head>\n";
echo "<body>\n";

// check if account specific page should be shown
if (isset($_POST['user'])) showMainPage('user');
elseif (isset($_POST['group'])) showMainPage('group');
elseif (isset($_POST['host'])) showMainPage('host');
// show start page
else {
	echo "<h1 align=\"center\">" . _("Account creation via file upload") . "</h1>\n";
	echo "<p>&nbsp;</p>\n";
	echo "<p>&nbsp;</p>\n";
	
	echo "<p>\n";
		echo "Here you can create multiple accounts by providing a CSV file.";
	echo "</p>\n";
	
	echo "<p>&nbsp;</p>\n";
	
	echo "<p><b>\n";
	echo _("Please select your account type:");
	echo "</b></p>\n";
	
	echo "<form enctype=\"multipart/form-data\" action=\"masscreate.php\" method=\"post\">\n";
	echo "<table style=\"border-color: grey\" cellpadding=\"10\" border=\"2\" cellspacing=\"0\">\n";
	echo "<tr>\n";
		echo "<th class=\"userlist-sort\">\n";
			echo "<input type=\"submit\" name=\"user\" value=\"" . _("Create user accounts") . "\">\n";
		echo "</th>\n";
		echo "<th class=\"grouplist-sort\">\n";
			echo "<input type=\"submit\" name=\"group\" value=\"" . _("Create group accounts") . "\">\n";
		echo "</th>\n";
		echo "<th class=\"hostlist-sort\">\n";
			echo "<input type=\"submit\" name=\"host\" value=\"" . _("Create host accounts") . "\">\n";
		echo "</th>\n";
	echo "</tr>\n";
	echo "</table>\n";
	echo "</form>\n";
	
	echo "</body>\n";
	echo "</html>\n";
}


/**
* Displays the acount type specific main page of the upload.
*
* @param string $scope account type
*/
function showMainPage($scope) {
	echo "<p>\n";
		echo _("Please provide a CSV formated file with your account data. The cells in the first row must be filled with the column identifiers. The following rows represent one account for each row.");
	echo "</p>\n";
	
	echo "<p>&nbsp;</p>\n";

	echo "<form enctype=\"multipart/form-data\" action=\"massBuildAccounts.php\" method=\"post\">\n";
	echo "<p>\n";
	echo "<b>" . _("CSV file:") . "</b> <input name=\"inputfile\" type=\"file\">&nbsp;&nbsp;";
	echo "<input name=\"submitfile\" type=\"submit\" value=\"" . _('Upload file and create accounts') . "\">\n";
	echo "<input type=\"hidden\" name=\"scope\" value=\"$scope\">\n";
	echo "</p>\n";
	echo "</form>\n";

	echo "<p>&nbsp;</p>\n";

	echo "<big><b>" . _("Columns:") . "</b></big>\n";

	// DN options
	echo "<fieldset>\n<legend><b>" . _("DN settings") . "</b></legend>\n";
	echo "<table width=\"100%\">\n";
		echo "<tr>\n";
			echo "<td width=\"50%\">\n";
			echo "<b>" . _("DN suffix") . "</b>\n";
			echo "<br>\n";
				echo "<ul>\n";
					echo "<li><b>" . _("Identifier") . ":</b> " . "dn_suffix</li>\n";
					echo "<li><b>" . _("Example value") . ":</b> " . "ou=accounts,dc=yourdomain,dc=org</li>\n";
					echo "<li>\n";
						echo "<a href=\"help.php?HelpNumber=361\" target=\"lamhelp\">" . _("Help") . "</a>\n";
					echo "</li>\n";
				echo "</ul>\n";
			echo "</td>\n";
			echo "<td width=\"50%\">\n";
			echo "<b><font color=\"red\">" . _("RDN identifier") . "</font></b>\n";
			echo "<br>\n";
				echo "<ul>\n";
					echo "<li><b>" . _("Identifier") . ":</b> " . "dn_rdn</li>\n";
					echo "<li><b>" . _("Possible values") . ":</b> " . "uid, cn, sambaSID</li>\n";  // TODO fill with correct values
					echo "<li><b>" . _("Example value") . ":</b> " . "uid</li>\n";
					echo "<li>\n";
						echo "<a href=\"help.php?HelpNumber=301\" target=\"lamhelp\">" . _("Help") . "</a>\n";
					echo "</li>\n";
				echo "</ul>\n";
			echo "</td>\n";
		echo "</tr>\n";
	echo "</table>\n";
	echo "</fieldset>\n";
	
	// get input fields from modules
	$columns = getUploadColumns($scope);

	// print input fields
	$modules = array_keys($columns);
	for ($m = 0; $m < sizeof($modules); $m++) {
		if (sizeof($columns[$modules[$m]]) < 1) continue;
		echo "<fieldset>\n<legend><b>" . getModuleAlias($modules[$m], $scope) . "</b></legend>\n";
		echo "<table width=\"100%\">\n";
		for ($i = 0; $i < sizeof($columns[$modules[$m]]); $i++) {
			echo "<tr>\n";
				echo "<td width=\"33%\">\n";
					showColumnData($modules[$m], $columns[$modules[$m]][$i]);
				echo "</td>\n";
				$i++;
				if ($i < sizeof($columns[$modules[$m]])) {
					echo "<td width=\"33%\">\n";
						showColumnData($modules[$m], $columns[$modules[$m]][$i]);
					echo "</td>\n";
					$i++;
					if ($i < sizeof($columns[$modules[$m]])) {
						echo "<td width=\"33%\">\n";
							showColumnData($modules[$m], $columns[$modules[$m]][$i]);
						echo "</td>\n";
					}
					else echo "<td width=\"33%\"></td>"; // empty cell if no more fields
				}
				else echo "<td width=\"33%\"></td><td width=\"33%\"></td>"; // empty cell if no more fields
			echo "</tr>\n";
		}
		echo "</table>\n";
		echo "</fieldset>";
	}

	echo "<p>&nbsp;</p>\n";

	// print table example and build sample CSV
	$sampleCSV = "";
	$sampleCSV_head = array();
	$sampleCSV_row = array();
	echo "<big><b>" . _("This is an example how it would look in your spreadsheet program before you convert to CSV:") . "</b></big>\n";

	echo "<table style=\"border-color: grey\" cellpadding=\"10\" border=\"2\" cellspacing=\"0\">\n";
		echo "<tr>\n";
			for ($m = 0; $m < sizeof($modules); $m++) {
				if (sizeof($columns[$modules[$m]]) < 1) continue;
				for ($i = 0; $i < sizeof($columns[$modules[$m]]); $i++) {
					$sampleCSV_head[] = "\"" . $columns[$modules[$m]][$i]['name'] . "\"";
					echo "<td>\n";
						echo $columns[$modules[$m]][$i]['name'];
					echo "</td>\n";
				}
			}
		echo "</tr>\n";
		echo "<tr>\n";
			for ($m = 0; $m < sizeof($modules); $m++) {
				if (sizeof($columns[$modules[$m]]) < 1) continue;
				for ($i = 0; $i < sizeof($columns[$modules[$m]]); $i++) {
					$sampleCSV_row[] = "\"" . $columns[$modules[$m]][$i]['example'] . "\"";
					echo "<td>\n";
						echo $columns[$modules[$m]][$i]['example'];
					echo "</td>\n";
				}
			}
		echo "</tr>\n";
	echo "</table>\n";
	$sampleCSV = implode(",", $sampleCSV_head) . "\n" . implode(",", $sampleCSV_row) . "\n";
	$_SESSION['mass_csv'] = $sampleCSV;
	
	// link to CSV sample
	echo "<p>\n";
	echo "<br><br>\n";
	echo "<a href=\"masscreate.php?getCSV=1\"><b>" . _("Download sample CSV file") . "</b></a>\n";
	echo "<br><br>\n";
	echo "<p>\n";

	echo "</body>\n";
	echo "</html>\n";
	die;
}

/**
* Prints the properties of one input field.
*
* @param string $module account module name
* @param array $data field data from modules
*/
function showColumnData($module, $data) {
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
		echo "<li>\n";
			echo "<a href=\"help.php?module=" . $module . "&amp;HelpNumber=" . $data['help'] . "\" target=\"lamhelp\">" . _("Help") . "</a>\n";
		echo "</li>\n";

	echo "</ul>\n";
}

?>
