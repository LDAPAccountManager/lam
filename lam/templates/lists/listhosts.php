<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Roland Gruber

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

  This code displays a list of all Samba hosts.

*/
include_once ("../../lib/config.inc");
include_once ("../../lib/ldap.inc");
include_once ("../../lib/status.inc");

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

// get sorting column when register_globals is off
$sort = $_GET['sort'];

// copy HTTP-GET variables to HTTP-POST
$_POST = $_POST + $_GET;

$hst_info = $_SESSION['hst_info'];
session_register('hst_info');
$hst_units = $_SESSION['hst_units'];
session_register('hst_units');

// check if button was pressed and if we have to add/delete a host
if ($_POST['new_host'] || $_POST['del_host']){
	// add new host
	if ($_POST['new_host']){
		echo("<meta http-equiv=\"refresh\" content=\"0; URL=../account.php?type=host\">");
		exit;
	}
	// delete host(s)
	if ($_POST['del_host']){
		// search for checkboxes
		$hosts = array_keys($_POST, "on");
		$hoststr = implode(";", $hosts);
		echo("<meta http-equiv=\"refresh\" content=\"0; URL=../delete.php?type=host&amp;DN='$hoststr'\">");
		}
		exit;
}

echo ("<?xml version=\"1.0\" encoding=\"ISO-8859-15\"?>\n");
echo ("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n");
echo "<html><head><title>listhosts</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head><body>\n";
echo "<script src=\"../../lib/functions.js\" type=\"text/javascript\" language=\"javascript\"></script>\n";

// generate attribute-description table
$attr_array;	// list of LDAP attributes to show
$desc_array;	// list of descriptions for the attributes
$attr_string = $_SESSION["config"]->get_hostlistAttributes();
$temp_array = explode(";", $attr_string);
$hash_table = $_SESSION["ldap"]->attributeHostArray();

// get current page
$page = $_GET["page"];
if (!$page) $page = 1;
// take maximum count of user entries shown on one page out of session
if ($_SESSION["config"]->get_MaxListEntries() <= 0)
	$max_pageentrys = 10;	// default setting, if not yet set
else
	$max_pageentrys = $_SESSION["config"]->get_MaxListEntries();

// generate column attributes and descriptions
for ($i = 0; $i < sizeof($temp_array); $i++) {
// if value is predifined, look up description in hash_table
if (substr($temp_array[$i],0,1) == "#") {
	$attr = substr($temp_array[$i],1);
	$attr_array[$i] = $attr;
	$desc_array[] = $hash_table[$attr];
}
// if not predefined, the attribute is seperated by a ":" from description
else {
	$attr = explode(":", $temp_array[$i]);
	$attr_array[$i] = $attr[0];
	$desc_array[$i] = $attr[1];
}
}

// check search suffix
if ($_POST['hst_suffix']) $hst_suffix = $_POST['hst_suffix'];  // new suffix selected via combobox
elseif ($_SESSION['hst_suffix']) $hst_suffix = $_SESSION['hst_suffix'];  // old suffix from session
else $hst_suffix = $_SESSION["config"]->get_HostSuffix();  // default suffix
session_register('hst_suffix');

// generate search filter for sort links
$searchfilter = "";
for ($k = 0; $k < sizeof($desc_array); $k++) {
	if ($_POST["filter" . strtolower($attr_array[$k])]) {
		$searchfilter = $searchfilter . "&amp;filter" . strtolower($attr_array[$k]) . "=".
			$_POST["filter" . strtolower($attr_array[$k])];
	}
}

if (! $_GET['norefresh']) {
	// configure search filter
	if ($_SESSION['config']->get_samba3() == "yes") {
		// Samba hosts have the attribute "sambaSamAccount" and end with "$"
		$filter = "(&(objectClass=sambaSamAccount) (uid=*$)";
	}
	else {
		// Samba hosts have the attribute "sambaAccount" and end with "$"
		$filter = "(&(objectClass=sambaAccount) (uid=*$)";
	}
	for ($k = 0; $k < sizeof($desc_array); $k++) {
	if ($_POST["filter" . strtolower($attr_array[$k])])
		$filter = $filter . "(" . strtolower($attr_array[$k]) . "=" .
		$_POST["filter" . strtolower($attr_array[$k])] . ")";
	else
		$_POST["filter" . strtolower($attr_array[$k])] = "";
	}
	$filter = $filter . ")";
	$attrs = $attr_array;
	$sr = @ldap_search($_SESSION["ldap"]->server(),
		$hst_suffix,
		$filter, $attrs);
	if ($sr) {
		$hst_info = ldap_get_entries($_SESSION["ldap"]->server, $sr);
		ldap_free_result($sr);
		if ($hst_info["count"] == 0) StatusMessage("WARN", "", _("No Samba Hosts found!"));
		// delete first array entry which is "count"
		array_shift($hst_info);
		// sort rows by sort column ($sort)
		usort($hst_info, "cmp_array");
	}
	else StatusMessage("ERROR", _("LDAP Search failed! Please check your preferences."), _("No Samba Hosts found!"));
}
else {
	if (sizeof($hst_info) == 0) StatusMessage("WARN", "", _("No Hosts found!"));
	// sort rows by sort column ($sort)
	if ($hst_info) usort($hst_info, "cmp_array");
}

echo ("<form action=\"listhosts.php\" method=\"post\">\n");

// draw navigation bar if host accounts were found
if (sizeof($hst_info) > 0) {
draw_navigation_bar(sizeof($hst_info));
echo ("<br>\n");
}

// print host table header
echo "<table rules=\"all\" class=\"hostlist\" width=\"100%\">\n";
echo "<tr class=\"hostlist-head\"><th width=22 height=34></th><th></th>";
// table header
for ($k = 0; $k < sizeof($desc_array); $k++) {
	if (strtolower($attr_array[$k]) == $sort) {
		echo "<th class=\"hostlist-sort\"><a href=\"listhosts.php?".
			"sort=" . strtolower($attr_array[$k]) . $searchfilter . "&norefresh=y" . "\">" . $desc_array[$k] . "</a></th>";
	}
	else echo "<th><a href=\"listhosts.php?".
		"sort=" . strtolower($attr_array[$k]) . $searchfilter . "&norefresh=y" . "\">" . $desc_array[$k] . "</a></th>";
}
echo "</tr>\n";

// print filter row
echo "<tr align=\"center\" class=\"hostlist\"><td width=22 height=34></td><td>";
echo "<input type=\"submit\" name=\"apply_filter\" value=\"" . _("Apply") . "\">";
echo "</td>";
// print input boxes for filters
for ($k = 0; $k < sizeof ($desc_array); $k++) {
  echo "<td>";
  echo ("<input type=\"text\" name=\"filter" . strtolower ($attr_array[$k]) .
	"\" value=\"" . $_POST["filter" . strtolower($attr_array[$k])] . "\">");
  echo "</td>";
}
echo "</tr>\n";

// calculate which rows to show
$table_begin = ($page - 1) * $max_pageentrys;
if (($page * $max_pageentrys) > sizeof($hst_info)) $table_end = sizeof($hst_info);
else $table_end = ($page * $max_pageentrys);

// print host list
for ($i = $table_begin; $i < $table_end; $i++) {
	echo("<tr class=\"hostlist\" onMouseOver=\"host_over(this, '" . $hst_info[$i]["dn"] . "')\"" .
								" onMouseOut=\"host_out(this, '" . $hst_info[$i]["dn"] . "')\"" .
								" onClick=\"host_click(this, '" . $hst_info[$i]["dn"] . "')\"" .
								" onDblClick=\"parent.frames[1].location.href='../account.php?type=host&amp;DN=" . $hst_info[$i]["dn"] . "'\">" .
								" <td height=22><input onClick=\"host_click(this, '" . $hst_info[$i]["dn"] . "')\" type=\"checkbox\" name=\"" . $hst_info[$i]["dn"] . "\"></td>" .
								" <td align='center'><a href=\"../account.php?type=host&amp;DN='" . $hst_info[$i]["dn"] . "'\">" . _("Edit") . "</a></td>");
	for ($k = 0; $k < sizeof($attr_array); $k++) {
		echo ("<td>");
		// print all attribute entries seperated by "; "
		if (sizeof($hst_info[$i][strtolower($attr_array[$k])]) > 0) {
			// delete first array entry which is "count"
			if (! $_GET['norefresh']) array_shift($hst_info[$i][strtolower($attr_array[$k])]);
			echo implode("; ", $hst_info[$i][strtolower($attr_array[$k])]);
		}
		echo ("</td>");
	}
	echo("</tr>\n");
}
echo ("</table>");

echo ("<br>");

// draw navigation bar if host accounts were found
if (sizeof($hst_info) > 0) {
draw_navigation_bar(sizeof($hst_info));
echo ("<br>\n");
}

if (! $_GET['norefresh']) {
	// generate list of possible suffixes
	$sr = @ldap_search($_SESSION["ldap"]->server(),
		$_SESSION["config"]->get_HostSuffix(),
		"objectClass=organizationalunit", array("DN"));
	if ($sr) {
		$hst_units = ldap_get_entries($_SESSION["ldap"]->server, $sr);
		// delete first array entry which is "count"
		array_shift($hst_units);
		// remove sub arrays
		for ($i = 0; $i < sizeof($hst_units); $i++) $hst_units[$i] = $hst_units[$i]['dn'];
		// add root suffix from config
		if (!in_array($_SESSION["config"]->get_HostSuffix(), $hst_units)) {
			array_push($hst_units, $_SESSION["config"]->get_HostSuffix());
		}
	}
}

echo ("<p align=\"left\">\n");
echo ("<input type=\"submit\" name=\"new_host\" value=\"" . _("New Host") . "\">\n");
if (sizeof($hst_info) > 0) echo ("<input type=\"submit\" name=\"del_host\" value=\"" . _("Delete Host(s)") . "\">\n");
// print combobox with possible sub-DNs
if (sizeof($hst_units) > 1) {
echo ("&nbsp;&nbsp;&nbsp;&nbsp;<b>" . _("Suffix") . ": </b>");
echo ("<select size=1 name=\"hst_suffix\">\n");
for ($i = 0; $i < sizeof($hst_units); $i++) {
	if ($hst_suffix == $hst_units[$i]) echo ("<option selected>" . $hst_units[$i] . "</option>\n");
	else echo("<option>" . $hst_units[$i] . "</option>\n");
}
echo ("</select>\n");
echo ("<input type=\"submit\" name=\"refresh\" value=\"" . _("Change Suffix") . "\">");
}
echo ("</p>\n");
echo ("</form>\n");
echo "</body></html>\n";

/**
 * @brief draws a navigation bar to switch between pages
 *
 *
 * @return void
 */
function draw_navigation_bar ($count) {
  global $max_pageentrys;
  global $page;
  global $sort;
  global $searchfilter;

  echo ("<table class=\"hostnav\" width=\"100%\" border=\"0\">\n");
  echo ("<tr>\n");
  echo ("<td><input type=\"submit\" name=\"refresh\" value=\"" . _("Refresh") . "\">&nbsp;&nbsp;");
  if ($page != 1)
    echo ("<a href=\"listhosts.php?page=" . ($page - 1) . "&amp;sort=" . $sort . $searchfilter . "\">&lt;=</a>\n");
  else
    echo ("&lt;=");
  echo ("&nbsp;");

  if ($page < ($count / $max_pageentrys))
    echo ("<a href=\"listhosts.php?page=" . ($page + 1) . "&amp;sort=" . $sort . $searchfilter . "\">=&gt;</a>\n");
  else
    echo ("=&gt;</td>");

  echo ("<td class=\"hostnav-text\">");
  echo "&nbsp;" . $count . " " .  _("Samba Host(s) found");
  echo ("</td>");

  echo ("<td class=\"hostlist_activepage\" align=\"right\">");
  for ($i = 0; $i < ($count / $max_pageentrys); $i++) {
    if ($i == $page - 1)
      echo ("&nbsp;" . ($i + 1));
    else
      echo ("&nbsp;<a href=\"listhosts.php?page=" . ($i + 1) .
	    "&amp;sort=" . $sort . "\">" . ($i + 1) . "</a>\n");
  }
  echo ("</td></tr></table>\n");
}

// compare function used for usort-method
// rows are sorted with the first attribute entry of the sort column
// if objects have attributes with multiple values the others are ignored
function cmp_array($a, $b) {
	// sort specifies the sort column
	global $sort;
	global $attr_array;
	// sort by first attribute with name $sort
	if (!$sort) $sort = strtolower($attr_array[0]);
	if ($a[$sort][0] == $b[$sort][0]) return 0;
	else if ($a[$sort][0] == max($a[$sort][0], $b[$sort][0])) return 1;
	else return -1;
}

?>
