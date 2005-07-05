<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Roland Gruber, Leonhard Walchshäusl
  Copyright (C) 2004  Roland Gruber

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more detaexils.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/


/**
* This page displays a list of all users.
*
* @package lists
* @author Roland Gruber
*/


/** Access to configuration options */
include_once ("../../lib/config.inc");
/** Access to LDAP connection */
include_once ("../../lib/ldap.inc");
/** Used to print status messages */
include_once ("../../lib/status.inc");
/** Used to create PDF files */
include_once("../../lib/pdf.inc");
/** Access to account modules */
include_once("../../lib/modules.inc");
/** Basic list functions */
include_once("../../lib/lists.inc");


// start session
session_save_path("../../sess");
@session_start();

setlanguage();

$scope = 'user';

// copy HTTP-GET variables to HTTP-POST
$_POST = $_POST + $_GET;

$refresh = true;
if (isset($_GET['norefresh'])) $refresh = false;
if (isset($_POST['refresh'])) $refresh = true;

// check if primary group should be translated
if (isset($_POST['trans_primary']) && ($_POST['trans_primary'] == 'on')) $trans_primary = "on";
else $trans_primary = "off";
if (isset($_SESSION['trans_primary_hash'])) $trans_primary_hash = $_SESSION['trans_primary_hash'];
else $trans_primary_hash = array();
// generate hash table for group translation
if ($trans_primary == "on" && ($refresh || (sizeof($trans_primary_hash) == 0))) {
	$trans_primary_hash = array();
	$grp_suffix = $_SESSION['config']->get_Suffix('group');
	$filter = "objectClass=posixGroup";
	$attrs = array("cn", "gidNumber");
	$sr = @ldap_search($_SESSION["ldap"]->server(), $grp_suffix, $filter, $attrs);
	if ($sr) {
		$info = @ldap_get_entries($_SESSION["ldap"]->server(), $sr);
		unset($info['count']); // delete count entry
		for ($i = 0; $i < sizeof($info); $i++) {
			$trans_primary_hash[$info[$i]['gidnumber'][0]] = $info[$i]['cn'][0];
		}
		$_SESSION['trans_primary_hash'] = $trans_primary_hash;
	}
}


if (isset($_SESSION[$scope . 'info'])) $info = $_SESSION[$scope . 'info'];
if (isset($_SESSION[$scope . '_units'])) $units = $_SESSION[$scope . '_units'];

listDoPost($scope);

echo $_SESSION['header'];

echo "<title>listusers</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head><body>\n";

listPrintJavaScript();

// get current page
if (isset($_GET["page"])) $page = $_GET["page"];
else $page = 1;

// take maximum count of user entries shown on one page out of session
if ($_SESSION["config"]->get_MaxListEntries() <= 0) {
	$max_page_entries = 10;	// default setting, if not yet set
}
else $max_page_entries = $_SESSION["config"]->get_MaxListEntries();

// generate attribute-description table
$temp_array = listGetAttributeDescriptionList($scope);
$attr_array = array_keys($temp_array);	// list of LDAP attributes to show
$desc_array = array_values($temp_array);	// list of descriptions for the attributes

if (isset($_GET["sort"])) $sort = $_GET["sort"];
else $sort = strtolower($attr_array[0]);

// check search suffix
if (isset($_POST['suffix'])) $suffix = $_POST['suffix'];  // new suffix selected via combobox
elseif (isset($_SESSION[$scope . '_suffix'])) $suffix = $_SESSION[$scope . '_suffix'];  // old suffix from session
else $suffix = $_SESSION["config"]->get_Suffix($scope);  // default suffix


// configure search filter for LDAP
$module_filter = get_ldap_filter($scope);  // basic filter is provided by modules
$filter = "(&" . $module_filter . ")";

if ($refresh) {
	$attrs = $attr_array;
	$sr = @ldap_search($_SESSION["ldap"]->server(), $suffix, $filter, $attrs);
	if (ldap_errno($_SESSION["ldap"]->server()) == 4) {
		StatusMessage("WARN", _("LDAP sizelimit exceeded, not all entries are shown."), _("See README.openldap.txt to solve this problem."));
	}
	if ($sr) {
		$info = ldap_get_entries ($_SESSION["ldap"]->server, $sr);
		ldap_free_result ($sr);
		// delete first array entry which is "count"
		unset($info['count']);
		// save position in original $info
		for ($i = 0; $i < sizeof($info); $i++) $info[$i]['LAM_ID'] = $i;
		// save results
		$_SESSION[$scope . 'info'] = $info;
	}
	else {
		$_SESSION[$scope . 'info'] = array();
		$info = array();
		StatusMessage("ERROR",
			_("LDAP Search failed! Please check your preferences."),
			_("No users found!"));
	}
}

$filter = listBuildFilter($attr_array);
$info = listFilterAccounts($info, $filter);
if (sizeof($info) == 0) StatusMessage("WARN", "", _("No users found!"));
// sort rows by sort column ($sort)
if ($info) {
	$info = listSort($sort, $attr_array, $info);
}

// build filter URL
$searchFilter = array();
$filterAttributes = array_keys($filter);
for ($i = 0; $i < sizeof($filterAttributes); $i++) {
	$searchFilter[] = "filter" . $filterAttributes[$i] . "=" . $filter[$filterAttributes[$i]]['original'];
}
if (sizeof($searchFilter) > 0) {
	$searchFilter = "&amp;" . implode("&amp;", $searchFilter);
}
else {
	$searchFilter = "";
}

echo ("<form action=\"listusers.php?norefresh=true\" method=\"post\">\n");

// display table only if users exist in LDAP
if (sizeof($info) != 0) {
	// create navigation bar on top of user table
	listDrawNavigationBar(sizeof($info), $max_page_entries, $page, $sort,
		$searchFilter . "&amp;trans_primary=" . $trans_primary, $scope, _("%s user(s) found"));
	echo ("<br />");
}

// account table head
listPrintTableHeader($scope, $searchFilter . "&amp;trans_primary=" . $trans_primary, $desc_array, $attr_array, $_POST, $sort);

// calculate which rows to show
$table_begin = ($page - 1) * $max_page_entries;
if (($page * $max_page_entries) > sizeof($info)) $table_end = sizeof($info);
else $table_end = ($page * $max_page_entries);

if (sizeof($info) != 0) {
	// translate GIDs and resort array if selected
	if ($trans_primary == "on") {
		// translate GIDs
		for ($i = 0; $i < sizeof($info); $i++) {
			if (isset($trans_primary_hash[$info[$i]['gidnumber'][0]])) {
				$info[$i]['gidnumber'][0] = $trans_primary_hash[$info[$i]['gidnumber'][0]];
			}
		}
		// resort if needed
		if ($sort == "gidnumber") {
			$info = listSort($sort, $attr_array, $info);
		}
	}
	// print user list
	for ($i = $table_begin; $i < $table_end; $i++) {
		echo("<tr class=\"userlist\"\nonMouseOver=\"list_over(this, '" . $info[$i]['LAM_ID'] . "', '" . $scope . "')\"\n" .
			"onMouseOut=\"list_out(this, '" . $info[$i]['LAM_ID'] . "', '" . $scope . "')\"\n" .
			"onClick=\"list_click(this, '" . $info[$i]['LAM_ID'] . "', '" . $scope . "')\"\n" .
			"onDblClick=\"parent.frames[1].location.href='../account/edit.php?type=user&amp;DN=" . $info[$i]['dn'] . "'\">\n");
		// checkboxes if selectall = "yes"
		if (isset($_GET['selectall'])) {
			echo "<td height=22 align=\"center\">\n<input onClick=\"list_click(this, '" . $info[$i]['LAM_ID'] . "', '" . $scope . "')\" type=\"checkbox\" name=\"" .
				$info[$i]['LAM_ID'] . "\" checked>\n</td>\n";
		}
		else {
			echo "<td height=22 align=\"center\">\n<input onClick=\"list_click(this, '" . $info[$i]['LAM_ID'] . "', '" . $scope . "')\" type=\"checkbox\" name=\"" .
				$info[$i]['LAM_ID'] . "\">\n</td>\n";
		}
		echo ("<td align='center'>\n<a href=\"../account/edit.php?type=user&amp;DN='" . $info[$i]['dn'] . "'\">" .
			_("Edit") . "</a>\n</td>\n");
		for ($k = 0; $k < sizeof($attr_array); $k++) {
			echo ("<td>\n");
			// print attribute values
			$attrName = strtolower($attr_array[$k]);
			if (isset($info[$i][$attrName]) && sizeof($info[$i][$attrName]) > 0) {
				if (is_array($info[$i][$attrName])) {
					// delete first array entry which is "count"
					unset($info[$i][$attrName]['count']);
					// sort array
					sort($info[$i][$attrName]);
					// print all attribute entries seperated by "; "
					echo implode("; ", $info[$i][$attrName]) . "\n";
				}
				else echo $info[$i][$attrName] . "\n";
			}
		echo ("</td>\n");
		}
	echo("</tr>\n");
	}
	// display select all link
	$colspan = sizeof($attr_array) + 1;
	echo "<tr class=\"userlist\">\n";
	echo "<td align=\"center\"><img src=\"../../graphics/select.png\" alt=\"select all\"></td>\n";
	echo "<td colspan=$colspan>&nbsp;<a href=\"listusers.php?norefresh=1&amp;page=" . $page . "&amp;sort=" . $sort .
		$searchFilter . "&amp;trans_primary=" . $trans_primary . "&amp;selectall=yes\">" .
		"<font color=\"black\"><b>" . _("Select all") . "</b></font></a></td>\n";
	echo "</tr>\n";
}
echo ("</table>\n");

echo ("<br>");
if (sizeof($info) != 0) {
	listDrawNavigationBar(sizeof($info), $max_page_entries, $page, $sort,
		$searchFilter . "&amp;trans_primary=" . $trans_primary, $scope, _("%s user(s) found"));
	echo ("<br>");
}

if ($refresh) {
	// generate list of possible suffixes
	$units = $_SESSION['ldap']->search_units($_SESSION["config"]->get_Suffix($scope));
}

// print combobox with possible sub-DNs
listShowOUSelection($units, $suffix);

// show translate GID to group name box if there is a column with gidnumber
if (in_array("gidnumber", $attr_array)) {
	echo "<p align=\"left\">\n";
	echo "<b>" . _("Translate GID number to group name") . ": </b>";
	if ($trans_primary == "on") {
		echo "<input type=\"checkbox\" name=\"trans_primary\" checked>";
	}
	else echo "<input type=\"checkbox\" name=\"trans_primary\">";
	echo ("&nbsp;&nbsp;<input type=\"submit\" name=\"apply\" value=\"" . _("Apply") . "\">");
	echo "</p>\n";
}

echo ("<p>&nbsp;</p>\n");

// new/delete/PDF buttons
echo ("<input type=\"submit\" name=\"new\" value=\"" . _("New user") . "\">\n");
if (sizeof($info) != 0) {
	echo ("<input type=\"submit\" name=\"del\" value=\"" . _("Delete user(s)") . "\">\n");
	echo ("<br><br><br>\n");
	echo "<fieldset><legend><b>PDF</b></legend>\n";
	echo ("<b>" . _('PDF structure') . ":</b>&nbsp;&nbsp;<select name=\"pdf_structure\">\n");
	$pdf_structures = getAvailablePDFStructures($scope);
	foreach($pdf_structures as $pdf_structure) {
		echo "<option value=\"" . $pdf_structure . "\"" . (($pdf_structure == 'default.xml') ? " selected" : "") . ">" . substr($pdf_structure,0,strlen($pdf_structure)-4) . "</option>";
	}
	echo "</select>&nbsp;&nbsp;&nbsp;&nbsp;\n";
	echo ("<input type=\"submit\" name=\"pdf\" value=\"" . _("Create PDF for selected user(s)") . "\">\n");
	echo "&nbsp;";
	echo ("<input type=\"submit\" name=\"pdf_all\" value=\"" . _("Create PDF for all users") . "\">\n");
	echo "</fieldset>";
}

echo ("<p>&nbsp;</p>\n");

echo ("</form>\n");
echo "</body></html>\n";


// save variables to session
$_SESSION[$scope . '_units'] = $units;
$_SESSION[$scope . '_suffix'] = $suffix;

?>
