<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003 - 2004  Roland Gruber

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
* This page displays a list of all groups.
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

$scope = 'group';

// get sorting column when register_globals is off
$sort = $_GET['sort'];

// copy HTTP-GET variables to HTTP-POST
$_POST = $_POST + $_GET;

$info = $_SESSION[$scope . 'info'];
$grp_units = $_SESSION['grp_units'];

// check if button was pressed and if we have to add/delete a group
if ($_POST['new_group'] || $_POST['del_group'] || $_POST['pdf_group'] || $_POST['pdf_all']){
	// add new group
	if ($_POST['new_group']){
		metaRefresh("../account/edit.php?type=group");
		exit;
	}
	// delete group(s)
	elseif ($_POST['del_group']){
		// search for checkboxes
		$groups = array_keys($_POST, "on");
		$_SESSION['delete_dn'] = $groups;
		if (sizeof($groups) > 0) {
			metaRefresh("../delete.php?type=group");
			exit;
		}
	}
	// PDF for selected groups
	elseif ($_POST['pdf_group']){
		$pdf_structure = $_POST['pdf_structure'];
		// search for checkboxes
		$groups = array_keys($_POST, "on");
		$list = array();
		// load groups from LDAP
		for ($i = 0; $i < sizeof($groups); $i++) {
			$_SESSION["accountPDF-$i"] = new accountContainer("group", "accountPDF-$i");
			$_SESSION["accountPDF-$i"]->load_account($groups[$i]);
			$list[$i] = $_SESSION["accountPDF-$i"];
		}
		if (sizeof($list) > 0) {
			createModulePDF($list,$pdf_structure);
			exit;
		}
	}
	// PDF for all groups
	elseif ($_POST['pdf_all']){
		$list = array();
		for ($i = 0; $i < sizeof($_SESSION[$scope . 'info']); $i++) {
			$_SESSION["accountPDF-$i"] = new accountContainer("group", "accountPDF-$i");
			$_SESSION["accountPDF-$i"]->load_account($_SESSION[$scope . 'info'][$i]['dn']);
			$list[$i] = $_SESSION["accountPDF-$i"];
		}
		if (sizeof($list) > 0) {
			createModulePDF($list,$_POST['pdf_structure']);
			exit;
		}
	}
}

echo $_SESSION['header'];
echo "<title>listgroups</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head><body>\n";
echo "<script src=\"../../lib/functions.js\" type=\"text/javascript\" language=\"javascript\"></script>\n";

// generate attribute-description table
$attr_array = array();	// list of LDAP attributes to show
$desc_array = array();	// list of descriptions for the attributes
$attr_string = $_SESSION["config"]->get_grouplistAttributes();
$temp_array = explode(";", $attr_string);
$hash_table = listGetAttributeGroupArray();

// get current page
$page = $_GET["page"];
if (!$page) $page = 1;
// take maximum count of group entries shown on one page out of session
if ($_SESSION["config"]->get_MaxListEntries() <= 0)
	$max_page_entries = 10;	// default setting, if not yet set
else
	$max_page_entries = $_SESSION["config"]->get_MaxListEntries();

// generate column attributes and descriptions
for ($i = 0; $i < sizeof($temp_array); $i++) {
	// if value is predifined, look up description in hash_table
	if (substr($temp_array[$i],0,1) == "#") {
		$attr = strtolower(substr($temp_array[$i],1));
		$attr_array[$i] = $attr;
		if ($hash_table[$attr]) $desc_array[] = strtoupper($hash_table[$attr]);
		else $desc_array[] = strtoupper($attr);
	}
	// if not predefined, the attribute is seperated by a ":" from description
	else {
		$attr = explode(":", $temp_array[$i]);
		$attr_array[$i] = $attr[0];
		if ($attr[1]) $desc_array[$i] = strtoupper($attr[1]);
		else $desc_array[$i] = strtoupper($attr[0]);
	}
}

// check search suffix
if ($_POST['grp_suffix']) $grp_suffix = $_POST['grp_suffix'];  // new suffix selected via combobox
elseif ($_SESSION['grp_suffix']) $grp_suffix = $_SESSION['grp_suffix'];  // old suffix from session
else $grp_suffix = $_SESSION["config"]->get_GroupSuffix();  // default suffix

$refresh = true;
if ($_GET['norefresh']) $refresh = false;
if ($_POST['refresh']) $refresh = true;

if ($refresh) {
	// configure search filter
	$module_filter = get_ldap_filter("group");  // basic filter is provided by modules
	$filter = "(&" . $module_filter . ")";
	$attrs = $attr_array;
	$sr = @ldap_search($_SESSION["ldap"]->server(), $grp_suffix, $filter, $attrs);
	if (ldap_errno($_SESSION["ldap"]->server()) == 4) {
		StatusMessage("WARN", _("LDAP sizelimit exceeded, not all entries are shown."), _("See README.openldap.txt to solve this problem."));
	}
	if ($sr) {
		$info = ldap_get_entries($_SESSION["ldap"]->server, $sr);
		ldap_free_result($sr);
		// delete first array entry which is "count"
		unset($info['count']);
		// save results
		$_SESSION[$scope . 'info'] = $info;
	}
	else {
		$info = array();
		$_SESSION[$scope . 'info'] = array();
		StatusMessage("ERROR", _("LDAP Search failed! Please check your preferences."), _("No groups found!"));
		}
}

$filter = listBuildFilter($_POST, $attr_array);
$info = listFilterAccounts($info, $filter);
if (sizeof($info) == 0) StatusMessage("WARN", "", _("No groups found!"));
// sort rows by sort column ($sort)
if ($info) $info = listSort($sort, $attr_array, $info);

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

echo ("<form action=\"listgroups.php?norefresh=true\" method=\"post\">\n");

// draw navigation bar if group accounts were found
if (sizeof($info) > 0) {
listDrawNavigationBar(sizeof($info), $max_page_entries, $page, $sort, $searchFilter, "group", _("%s group(s) found"));
echo ("<br>");
}

// account table head
listPrintTableHeader("group", $searchFilter, $desc_array, $attr_array, $_POST, $sort);

// calculate which rows to show
$table_begin = ($page - 1) * $max_page_entries;
if (($page * $max_page_entries) > sizeof($info)) $table_end = sizeof($info);
else $table_end = ($page * $max_page_entries);

if (sizeof($info) > 0) {
	// print group list
	for ($i = $table_begin; $i < $table_end; $i++) {
		echo("<tr class=\"grouplist\" onMouseOver=\"group_over(this, '" . $info[$i]["dn"] . "')\"" .
									" onMouseOut=\"group_out(this, '" . $info[$i]["dn"] . "')\"" .
									" onClick=\"group_click(this, '" . $info[$i]["dn"] . "')\"" .
									" onDblClick=\"parent.frames[1].location.href='../account/edit.php?type=group&amp;DN=" . $info[$i]["dn"] . "'\">");
		if ($_GET['selectall'] == "yes") {
		echo " <td height=22 align=\"center\"><input onClick=\"group_click(this, '" . $info[$i]["dn"] . "')\" type=\"checkbox\"" .
			" name=\"" . $info[$i]["dn"] . "\" checked></td>";
		}
		else {
		echo " <td height=22 align=\"center\"><input onClick=\"group_click(this, '" . $info[$i]["dn"] . "')\" type=\"checkbox\"" .
			" name=\"" . $info[$i]["dn"] . "\"></td>";
		}
		echo (" <td align='center'><a href=\"../account/edit.php?type=group&amp;DN='" . $info[$i]["dn"] . "'\">" . _("Edit") . "</a></td>");
		for ($k = 0; $k < sizeof($attr_array); $k++) {
			echo ("<td>");
			// print all attribute entries seperated by "; "
			if (sizeof($info[$i][strtolower($attr_array[$k])]) > 0) {
				// delete first array entry which is "count"
				if (is_array($info[$i][strtolower($attr_array[$k])])) unset($info[$i][strtolower($attr_array[$k])]['count']);
				// generate links for group members
				if (strtolower($attr_array[$k]) == "memberuid") {
					// sort array
					sort($info[$i][strtolower($attr_array[$k])]);
					// make a link for each member of the group
					$linklist = array();
					for ($d = 0; $d < sizeof($info[$i][strtolower($attr_array[$k])]); $d++) {
						$user = $info[$i][strtolower($attr_array[$k])][$d]; // user name
						$linklist[$d] = "<a href=\"userlink.php?user='" . $user . "' \">" . $user . "</a>";
					}
					echo implode("; ", $linklist);
				}
				// print all other attributes
				else {
					if (is_array($info[$i][strtolower($attr_array[$k])])) {
						// delete "count" entry
						unset($info[$i][strtolower($attr_array[$k])]['count']);
						// sort array
						sort($info[$i][strtolower($attr_array[$k])]);
						echo implode("; ", $info[$i][strtolower($attr_array[$k])]);
					}
					else echo $info[$i][strtolower($attr_array[$k])];
				}
			}
			echo ("</td>");
		}
		echo("</tr>\n");
	}
	// display select all link
	$colspan = sizeof($attr_array) + 1;
	echo "<tr class=\"grouplist\">\n";
	echo "<td align=\"center\"><img src=\"../../graphics/select.png\" alt=\"select all\"></td>\n";
	echo "<td colspan=$colspan>&nbsp;<a href=\"listgroups.php?norefresh=y&amp;page=" . $page . "&amp;sort=" . $sort .
		$searchFilter . "&amp;selectall=yes\">" .
		"<font color=\"black\"><b>" . _("Select all") . "</b></font></a></td>\n";
	echo "</tr>\n";
}
echo ("</table>");
echo ("<br>");

// draw navigation bar if group accounts were found
if (sizeof($info) > 0) {
listDrawNavigationBar(sizeof($info), $max_page_entries, $page, $sort, $searchFilter, "group", _("%s group(s) found"));
echo ("<br>\n");
}

if (! $_GET['norefresh']) {
	// generate list of possible suffixes
	$grp_units = $_SESSION['ldap']->search_units($_SESSION["config"]->get_GroupSuffix());
}

// print combobox with possible sub-DNs
if (sizeof($grp_units) > 1) {
	echo ("<p align=\"left\">\n");
	echo ("<b>" . _("Suffix") . ": </b>");
	echo ("<select size=1 name=\"grp_suffix\">\n");
	for ($i = 0; $i < sizeof($grp_units); $i++) {
		if ($grp_suffix == $grp_units[$i]) echo ("<option selected>" . $grp_units[$i] . "</option>\n");
		else echo("<option>" . $grp_units[$i] . "</option>\n");
	}
	echo ("</select>\n");
	echo ("<input type=\"submit\" name=\"refresh\" value=\"" . _("Change Suffix") . "\">");
	echo ("</p>\n");
	echo ("<p>&nbsp;</p>\n");
}

echo ("<input type=\"submit\" name=\"new_group\" value=\"" . _("New Group") . "\">\n");
if (sizeof($info) > 0) {
	echo ("<input type=\"submit\" name=\"del_group\" value=\"" . _("Delete Group(s)") . "\">\n");
	echo ("<br><br><br>\n");
	echo "<fieldset><legend><b>PDF</b></legend>\n";
	echo ("<b>" . _('PDF structure') . ":</b>&nbsp;&nbsp;<select name=\"pdf_structure\">\n");
	$pdf_structures = getAvailablePDFStructures('group');
	foreach($pdf_structures as $pdf_structure) {
		echo "<option value=\"" . $pdf_structure . "\"" . (($pdf_structure == 'default.xml') ? " selected" : "") . ">" . substr($pdf_structure,0,strlen($pdf_structure)-4) . "</option>";
	}
	echo "</select>&nbsp;&nbsp;&nbsp;&nbsp;\n";
	echo ("<input type=\"submit\" name=\"pdf_group\" value=\"" . _("Create PDF for selected group(s)") . "\">\n");
	echo "&nbsp;";
	echo ("<input type=\"submit\" name=\"pdf_all\" value=\"" . _("Create PDF for all groups") . "\">\n");
	echo "</fieldset>";
}

echo ("</form>\n");
echo "</body></html>\n";

// save variables to session
$_SESSION['grp_units'] = $grp_units;
$_SESSION['grp_suffix'] = $grp_suffix;

?>
