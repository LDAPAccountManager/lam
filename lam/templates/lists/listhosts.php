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
* This page displays a list of all hosts.
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

// get sorting column when register_globals is off
$sort = $_GET['sort'];

// copy HTTP-GET variables to HTTP-POST
$_POST = $_POST + $_GET;

$hst_info = $_SESSION['hst_info'];
$hst_units = $_SESSION['hst_units'];

// check if button was pressed and if we have to add/delete a host
if ($_POST['new_host'] || $_POST['del_host'] || $_POST['pdf_host'] || $_POST['pdf_all']){
	// add new host
	if ($_POST['new_host']){
		metaRefresh("../account/edit.php?type=host");
		exit;
	}
	// delete host(s)
	elseif ($_POST['del_host']){
		// search for checkboxes
		$hosts = array_keys($_POST, "on");
		$_SESSION['delete_dn'] = $hosts;
		if (sizeof($hosts) > 0) {
			metaRefresh("../delete.php?type=host");
			exit;
		}
	}
	// PDF for selected hosts
	elseif ($_POST['pdf_host']){
		$pdf_structure = $_POST['pdf_structure'];
		// search for checkboxes
		$hosts = array_keys($_POST, "on");
		$list = array();
		// load hosts from LDAP
		for ($i = 0; $i < sizeof($hosts); $i++) {
			$_SESSION["accountPDF-$i"] = new accountContainer("host", "accountPDF-$i");
			$_SESSION["accountPDF-$i"]->load_account($hosts[$i]);
			$list[$i] = $_SESSION["accountPDF-$i"];
		}
		if (sizeof($list) > 0) {
			createModulePDF($list,$pdf_structure);
			exit;
		}
	}
	// PDF for all hosts
	elseif ($_POST['pdf_all']){
		$list = array();
		for ($i = 0; $i < sizeof($_SESSION['hst_info']); $i++) {
			$_SESSION["accountPDF-$i"] = new accountContainer("host", "accountPDF-$i");
			$_SESSION["accountPDF-$i"]->load_account($_SESSION['hst_info'][$i]['dn']);
			$list[$i] = $_SESSION["accountPDF-$i"];
		}
		if (sizeof($list) > 0) {
			createModulePDF($list,$_POST['pdf_structure']);
			exit;
		}
	}
}

echo $_SESSION['header'];
echo "<title>listhosts</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head><body>\n";
echo "<script src=\"../../lib/functions.js\" type=\"text/javascript\" language=\"javascript\"></script>\n";

// generate attribute-description table
$attr_array = array();	// list of LDAP attributes to show
$desc_array = array();	// list of descriptions for the attributes
$attr_string = $_SESSION["config"]->get_hostlistAttributes();
$temp_array = explode(";", $attr_string);
$hash_table = listGetAttributeHostArray();

// get current page
$page = $_GET["page"];
if (!$page) $page = 1;
// take maximum count of host entries shown on one page out of session
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
if ($_POST['hst_suffix']) $hst_suffix = $_POST['hst_suffix'];  // new suffix selected via combobox
elseif ($_SESSION['hst_suffix']) $hst_suffix = $_SESSION['hst_suffix'];  // old suffix from session
else $hst_suffix = $_SESSION["config"]->get_HostSuffix();  // default suffix

// generate search filter for sort links
$searchFilter = "";
for ($k = 0; $k < sizeof($desc_array); $k++) {
	if (eregi("^([0-9a-z_\\*\\+\\-])+$", $_POST["filter" . strtolower($attr_array[$k])])) {
		$searchFilter = $searchFilter . "&amp;filter" . strtolower($attr_array[$k]) . "=".
			$_POST["filter" . strtolower($attr_array[$k])];
	}
}

if (! $_GET['norefresh']) {
	// configure search filter
	$module_filter = get_ldap_filter("host");  // basic filter is provided by modules
	$filter = "(&" . $module_filter;
	for ($k = 0; $k < sizeof($desc_array); $k++) {
	if (eregi("^([0-9a-z_\\*\\+\\-])+$", $_POST["filter" . strtolower($attr_array[$k])]))
		$filter = $filter . "(" . strtolower($attr_array[$k]) . "=" .
		$_POST["filter" . strtolower($attr_array[$k])] . ")";
	else
		$_POST["filter" . strtolower($attr_array[$k])] = "";
	}
	$filter = $filter . ")";
	$attrs = $attr_array;
	$sr = @ldap_search($_SESSION["ldap"]->server(), $hst_suffix, $filter, $attrs);
	if (ldap_errno($_SESSION["ldap"]->server()) == 4) {
		StatusMessage("WARN", _("LDAP sizelimit exceeded, not all entries are shown."), _("See README.openldap.txt to solve this problem."));
	}
	if ($sr) {
		$hst_info = ldap_get_entries($_SESSION["ldap"]->server, $sr);
		ldap_free_result($sr);
		if ($hst_info["count"] == 0) StatusMessage("WARN", "", _("No Samba Hosts found!"));
		// delete first array entry which is "count"
		unset($hst_info['count']);
		// sort rows by sort column ($sort)
		$hst_info = listSort($sort, $attr_array, $hst_info);
	}
	else {
		$hst_info = array();
		$_SESSION['hst_info'] = array();
		StatusMessage("ERROR", _("LDAP Search failed! Please check your preferences."), _("No Samba Hosts found!"));
		}
}
else {
	if (sizeof($hst_info) == 0) StatusMessage("WARN", "", _("No Samba Hosts found!"));
	// sort rows by sort column ($sort)
	if ($hst_info) $hst_info = listSort($sort, $attr_array, $hst_info);
}

echo ("<form action=\"listhosts.php\" method=\"post\">\n");

// draw navigation bar if host accounts were found
if (sizeof($hst_info) > 0) {
listDrawNavigationBar(sizeof($hst_info), $max_page_entries, $page, $sort, $searchFilter, "host", _("%s host(s) found"));
echo ("<br>\n");
}

// account table head
listPrintTableHeader("host", $searchFilter, $desc_array, $attr_array, $_POST, $sort);

// calculate which rows to show
$table_begin = ($page - 1) * $max_page_entries;
if (($page * $max_page_entries) > sizeof($hst_info)) $table_end = sizeof($hst_info);
else $table_end = ($page * $max_page_entries);

if (sizeof($hst_info) > 0) {
	// print host list
	for ($i = $table_begin; $i < $table_end; $i++) {
		echo("<tr class=\"hostlist\" onMouseOver=\"host_over(this, '" . $hst_info[$i]["dn"] . "')\"" .
									" onMouseOut=\"host_out(this, '" . $hst_info[$i]["dn"] . "')\"" .
									" onClick=\"host_click(this, '" . $hst_info[$i]["dn"] . "')\"" .
									" onDblClick=\"parent.frames[1].location.href='../account/edit.php?type=host&amp;DN=" . $hst_info[$i]["dn"] . "'\">");
		if ($_GET['selectall'] == "yes") {
		echo " <td height=22 align=\"center\"><input onClick=\"host_click(this, '" . $hst_info[$i]["dn"] . "')\"" .
					" type=\"checkbox\" checked name=\"" . $hst_info[$i]["dn"] . "\"></td>";
		}
		else {
		echo " <td height=22 align=\"center\"><input onClick=\"host_click(this, '" . $hst_info[$i]["dn"] . "')\"" .
					" type=\"checkbox\" name=\"" . $hst_info[$i]["dn"] . "\"></td>";
		}
		echo (" <td align='center'><a href=\"../account/edit.php?type=host&amp;DN='" . $hst_info[$i]["dn"] . "'\">" . _("Edit") . "</a></td>");
		for ($k = 0; $k < sizeof($attr_array); $k++) {
			echo ("<td>");
			// print all attribute entries seperated by "; "
			if (sizeof($hst_info[$i][strtolower($attr_array[$k])]) > 0) {
				// delete "count" entry
				unset($hst_info[$i][strtolower($attr_array[$k])]['count']);
				if (is_array($hst_info[$i][strtolower($attr_array[$k])])) {
					// sort array
					sort($hst_info[$i][strtolower($attr_array[$k])]);
					echo implode("; ", $hst_info[$i][strtolower($attr_array[$k])]);
				}
				else echo $hst_info[$i][strtolower($attr_array[$k])];
			}
			echo ("</td>");
		}
		echo("</tr>\n");
	}
	// display select all link
	$colspan = sizeof($attr_array) + 1;
	echo "<tr class=\"hostlist\">\n";
	echo "<td align=\"center\"><img src=\"../../graphics/select.png\" alt=\"select all\"></td>\n";
	echo "<td colspan=$colspan>&nbsp;<a href=\"listhosts.php?norefresh=y&amp;page=" . $page . "&amp;sort=" . $sort .
		$searchFilter . "&amp;selectall=yes\">" .
		"<font color=\"black\"><b>" . _("Select all") . "</b></font></a></td>\n";
	echo "</tr>\n";
}
echo ("</table>");

echo ("<br>");

// draw navigation bar if host accounts were found
if (sizeof($hst_info) > 0) {
listDrawNavigationBar(sizeof($hst_info), $max_page_entries, $page, $sort, $searchFilter, "host", _("%s host(s) found"));
echo ("<br>\n");
}

if (! $_GET['norefresh']) {
	// generate list of possible suffixes
$hst_units = $_SESSION['ldap']->search_units($_SESSION["config"]->get_HostSuffix());
}

// print combobox with possible sub-DNs
if (sizeof($hst_units) > 1) {
echo ("<p align=\"left\">\n");
echo ("<b>" . _("Suffix") . ": </b>");
echo ("<select size=1 name=\"hst_suffix\">\n");
for ($i = 0; $i < sizeof($hst_units); $i++) {
	if ($hst_suffix == $hst_units[$i]) echo ("<option selected>" . $hst_units[$i] . "</option>\n");
	else echo("<option>" . $hst_units[$i] . "</option>\n");
}
echo ("</select>\n");
echo ("<input type=\"submit\" name=\"refresh\" value=\"" . _("Change Suffix") . "\">");
echo ("</p>\n");
echo ("<p>&nbsp;</p>\n");
}

// add/delete/PDF buttons
echo ("<input type=\"submit\" name=\"new_host\" value=\"" . _("New Host") . "\">\n");
if (sizeof($hst_info) > 0) {
	echo ("<input type=\"submit\" name=\"del_host\" value=\"" . _("Delete Host(s)") . "\">\n");
	echo ("<br><br><br>\n");
	echo "<fieldset><legend><b>PDF</b></legend>\n";
	echo ("<b>" . _('PDF structure') . ":</b>&nbsp;&nbsp;<select name=\"pdf_structure\">\n");
	$pdf_structures = getAvailablePDFStructures('host');
	foreach($pdf_structures as $pdf_structure) {
		echo "<option value=\"" . $pdf_structure . "\"" . (($pdf_structure == 'default.xml') ? " selected" : "") . ">" . substr($pdf_structure,0,strlen($pdf_structure)-4) . "</option>";
	}
	echo "</select>&nbsp;&nbsp;&nbsp;&nbsp;\n";
	echo ("<input type=\"submit\" name=\"pdf_host\" value=\"" . _("Create PDF for selected host(s)") . "\">\n");
	echo "&nbsp;";
	echo ("<input type=\"submit\" name=\"pdf_all\" value=\"" . _("Create PDF for all hosts") . "\">\n");
	echo "</fieldset>";
}

echo ("</form>\n");
echo "</body></html>\n";

// save variables to session
$_SESSION['hst_info'] = $hst_info;
$_SESSION['hst_units'] = $hst_units;
$_SESSION['hst_suffix'] = $hst_suffix;

?>
