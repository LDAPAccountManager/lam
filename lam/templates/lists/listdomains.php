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
* This page displays a list of all Samba domains.
*
* @package tools
* @author Roland Gruber
*/


/** Access to configuration options */
include_once("../../lib/config.inc");
/** Access to LDAP connection */
include_once("../../lib/ldap.inc");
/** Used to print status messages */
include_once("../../lib/status.inc");
/** Basic list functions */
include_once("../../lib/lists.inc");

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

$scope = 'domain';

// copy HTTP-GET variables to HTTP-POST
$_POST = $_POST + $_GET;

$info = $_SESSION[$scope . 'info'];
$units = $_SESSION[$scope . '_units'];

// check if button was pressed and if we have to add/delete a domain
if (isset($_POST['new']) || isset($_POST['del'])){
	// add new domain
	if (isset($_POST['new'])){
		metaRefresh("../domain.php?action=new");
		exit;
	}
	// delete domain(s)
	if (isset($_POST['del'])){
		// search for checkboxes
		$domains = array_keys($_POST, "on");
		$domainstr = implode(";", $domains);
		if ($domainstr) {
			metaRefresh("../domain.php?action=delete&amp;DN='$domainstr'");
			exit;
		}
	}
}

echo $_SESSION['header'];
echo "<title>listdomains</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head><body>\n";

listPrintJavaScript();

// get current page
if (isset($_GET["page"])) $page = $_GET["page"];
else $page = 1;

// take maximum count of domain entries shown on one page out of session
if ($_SESSION["config"]->get_MaxListEntries() <= 0)
	$max_page_entries = 10;	// default setting, if not yet set
else
	$max_page_entries = $_SESSION["config"]->get_MaxListEntries();


// generate attribute and description tables
$attr_array = array();	// list of LDAP attributes to show
$desc_array = array();	// list of descriptions for the attributes
$attr_array[] = "sambaDomainName";
$attr_array[] = "sambaSID";
$attr_array[] = "dn";
$desc_array[] = strtoupper(_("Domain name"));
$desc_array[] = strtoupper(_("Domain SID"));
$desc_array[] = "DN";

if (isset($_GET["sort"])) $sort = $_GET["sort"];
else $sort = strtolower($attr_array[0]);

// check search suffix
if (isset($_POST['suffix'])) $suffix = $_POST['suffix'];  // new suffix selected via combobox
elseif (isset($_SESSION[$scope . '_suffix'])) $suffix = $_SESSION[$scope . '_suffix'];  // old suffix from session
else $suffix = $_SESSION["config"]->get_DomainSuffix();  // default suffix

$refresh = true;
if (isset($_GET['norefresh'])) $refresh = false;
if (isset($_POST['refresh'])) $refresh = true;

if ($refresh) {
	// configure search filter
	$filter = "(objectClass=sambaDomain)";
	$attrs = $attr_array;
	$sr = @ldap_search($_SESSION["ldap"]->server(), $suffix, $filter, $attrs);
	if (ldap_errno($_SESSION["ldap"]->server()) == 4) {
		StatusMessage("WARN", _("LDAP sizelimit exceeded, not all entries are shown."), _("See README.openldap.txt to solve this problem."));
	}
	if ($sr) {
		$info = ldap_get_entries($_SESSION["ldap"]->server, $sr);
		ldap_free_result($sr);
		if ($info["count"] == 0) StatusMessage("WARN", "", _("No Samba domains found!"));
		// delete first array entry which is "count"
		unset($info['count']);
		// sort rows by sort column ($sort)
		$info = listSort($sort, $attr_array, $info);
	}
	else StatusMessage("ERROR", _("LDAP Search failed! Please check your preferences."), _("No Samba domains found!"));
}
// use search result from session
else {
	if (sizeof($info) == 0) StatusMessage("WARN", "", _("No Samba domains found!"));
	// sort rows by sort column ($sort)
	if ($info) $info = listSort($sort, $attr_array, $info);
}

echo ("<form action=\"listdomains.php?norefresh=true\" method=\"post\">\n");

// draw navigation bar if domain accounts were found
if (sizeof($info) > 0) {
listDrawNavigationBar(sizeof($info), $max_page_entries, $page, $sort, '', "domain", _("%s Samba domain(s) found"));
echo ("<br>\n");
}

// print domain table header
echo "<table rules=\"all\" class=\"domainlist\" width=\"100%\">\n";
echo "<tr class=\"domainlist-head\"><th width=22 height=34></th><th></th>";
// table header
for ($k = 0; $k < sizeof($desc_array); $k++) {
	if (strtolower($attr_array[$k]) == $sort) {
		echo "<th class=\"domainlist-sort\"><a href=\"listdomains.php?".
			"sort=" . strtolower($attr_array[$k]) . "&amp;norefresh=y" . "\">" . $desc_array[$k] . "</a></th>";
	}
	else echo "<th><a href=\"listdomains.php?".
		"sort=" . strtolower($attr_array[$k]) . "&amp;norefresh=y" . "\">" . $desc_array[$k] . "</a></th>";
}
echo "</tr>\n";

// calculate which rows to show
$table_begin = ($page - 1) * $max_page_entries;
if (($page * $max_page_entries) > sizeof($info)) $table_end = sizeof($info);
else $table_end = ($page * $max_page_entries);

// print domain list
for ($i = $table_begin; $i < $table_end; $i++) {
	echo("<tr class=\"domainlist\" onMouseOver=\"list_over(this, '" . $info[$i]["dn"] . "', '" . $scope . "')\"" .
								" onMouseOut=\"list_out(this, '" . $info[$i]["dn"] . "', '" . $scope . "')\"" .
								" onClick=\"list_click(this, '" . $info[$i]["dn"] . "', '" . $scope . "')\"" .
								" onDblClick=\"parent.frames[1].location.href='../domain.php?action=edit&amp;DN=" . $info[$i]["dn"] . "'\">" .
								" <td height=22 align=\"center\"><input onClick=\"list_click(this, '" . $info[$i]["dn"] . "', '" . $scope . "')\" type=\"checkbox\" name=\"" . $info[$i]["dn"] . "\"></td>" .
								" <td align='center'><a href=\"../domain.php?action=edit&amp;DN='" . $info[$i]["dn"] . "'\">" . _("Edit") . "</a></td>");
	for ($k = 0; $k < sizeof($attr_array); $k++) {
		echo ("<td>");
		// print all attribute entries seperated by "; "
		if (sizeof($info[$i][strtolower($attr_array[$k])]) > 0) {
			// delete first array entry which is "count"
			if (is_array($info[$i][strtolower($attr_array[$k])])) unset($info[$i][strtolower($attr_array[$k])]['count']);
			if (is_array($info[$i][strtolower($attr_array[$k])])) echo implode("; ", $info[$i][strtolower($attr_array[$k])]);
			else echo $info[$i][strtolower($attr_array[$k])];
		}
		echo ("</td>");
	}
	echo("</tr>\n");
}
echo ("</table>");

echo ("<br>");

// draw navigation bar if domain accounts were found
if (sizeof($info) > 0) {
listDrawNavigationBar(sizeof($info), $max_page_entries, $page, $sort, '', "domain", _("%s Samba domain(s) found"));
echo ("<br>\n");
}

if ($refresh) {
	// generate list of possible suffixes
	$units = $_SESSION['ldap']->search_units($_SESSION["config"]->get_DomainSuffix());
}

// print combobox with possible sub-DNs
listShowOUSelection($units, $suffix);

echo ("<p align=\"left\">\n");
echo ("<input type=\"submit\" name=\"new\" value=\"" . _("New Domain") . "\">\n");
if (sizeof($info) > 0) echo ("<input type=\"submit\" name=\"del\" value=\"" . _("Delete Domain(s)") . "\">\n");
echo ("</p>\n");

echo ("</form>\n");
echo "</body></html>\n";



// save variables to session
$_SESSION[$scope . 'info'] = $info;
$_SESSION[$scope . '_units'] = $units;
$_SESSION[$scope . '_suffix'] = $suffix;

?>
