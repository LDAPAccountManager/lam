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

  This code displays a list of all groups.

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

$grp_info = $_SESSION['grp_info'];
$grp_units = $_SESSION['grp_units'];

// check if button was pressed and if we have to add/delete a group
if ($_POST['new_group'] || $_POST['del_group']){
	// add new group
	if ($_POST['new_group']){
		echo("<meta http-equiv=\"refresh\" content=\"0; URL=../account.php?type=group\">");
		exit;
	}
	// delete group(s)
	if ($_POST['del_group']){
		// search for checkboxes
		$groups = array_keys($_POST, "on");
		$groupstr = implode(";", $groups);
		echo("<meta http-equiv=\"refresh\" content=\"0; URL=../delete.php?type=group&amp;DN='$groupstr'\">");
		}
		exit;
}

echo $_SESSION['header'];
echo "<html><head><title>listgroups</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head><body>\n";
echo "<script src=\"../../lib/functions.js\" type=\"text/javascript\" language=\"javascript\"></script>\n";

// generate attribute-description table
$attr_array = array();	// list of LDAP attributes to show
$desc_array = array();	// list of descriptions for the attributes
$attr_string = $_SESSION["config"]->get_grouplistAttributes();
$temp_array = explode(";", $attr_string);
$hash_table = $_SESSION["ldap"]->attributeGroupArray();

// get current page
$page = $_GET["page"];
if (!$page) $page = 1;
// take maximum count of group entries shown on one page out of session
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
		$desc_array[] = strtoupper($hash_table[$attr]);
	}
	// if not predefined, the attribute is seperated by a ":" from description
	else {
		$attr = explode(":", $temp_array[$i]);
		$attr_array[$i] = $attr[0];
		$desc_array[$i] = strtoupper($attr[1]);
	}
}

// check search suffix
if ($_POST['grp_suffix']) $grp_suffix = $_POST['grp_suffix'];  // new suffix selected via combobox
elseif ($_SESSION['grp_suffix']) $grp_suffix = $_SESSION['grp_suffix'];  // old suffix from session
else $grp_suffix = $_SESSION["config"]->get_GroupSuffix();  // default suffix

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
	// Groups have the attribute "posixGroup"
	$filter = "(&(objectClass=posixGroup)";
	for ($k = 0; $k < sizeof($desc_array); $k++) {
	if ($_POST["filter" . strtolower($attr_array[$k])])
		$filter = $filter . "(" . strtolower($attr_array[$k]) . "=" .
		$_POST["filter" . strtolower($attr_array[$k])] . ")";
	else
		$_POST["filter" . strtolower($attr_array[$k])] = "";
	}
	$filter = $filter . ")";
	$attrs = $attr_array;
	$sr = @ldap_search($_SESSION["ldap"]->server(), $grp_suffix, $filter, $attrs);
	if (ldap_errno($_SESSION["ldap"]->server()) == 4) {
		StatusMessage("WARN", _("LDAP sizelimit exceeded, not all entries are shown."), "See README.openldap to solve this problem.");
	}
	if ($sr) {
		$grp_info = ldap_get_entries($_SESSION["ldap"]->server, $sr);
		ldap_free_result($sr);
		if ($grp_info["count"] == 0) StatusMessage("WARN", "", _("No Groups found!"));
		// delete first array entry which is "count"
		array_shift($grp_info);
		// sort rows by sort column ($sort)
		usort($grp_info, "cmp_array");
	}
	else StatusMessage("ERROR", _("LDAP Search failed! Please check your preferences."), _("No Groups found!"));
}
else {
	if (sizeof($grp_info) == 0) StatusMessage("WARN", "", _("No Groups found!"));
	// sort rows by sort column ($sort)
	if ($grp_info) usort($grp_info, "cmp_array");
}

echo ("<form action=\"listgroups.php\" method=\"post\">\n");

// draw navigation bar if group accounts were found
if (sizeof($grp_info) > 0) {
draw_navigation_bar(sizeof($grp_info));
echo ("<br>");
}

// print group table header
echo "<table rules=\"all\" class=\"grouplist\" width=\"100%\">\n";
echo "<tr class=\"grouplist-head\"><th width=22 height=34></th><th></th>";
// table header
for ($k = 0; $k < sizeof($desc_array); $k++) {
	if (strtolower($attr_array[$k]) == $sort) {
		echo "<th class=\"grouplist-sort\"><a href=\"listgroups.php?".
			"sort=" . strtolower($attr_array[$k]) . $searchfilter . "&amp;norefresh=y" . "\">" . $desc_array[$k] . "</a></th>";
	}
	else echo "<th><a href=\"listgroups.php?".
		"sort=" . strtolower($attr_array[$k]) . $searchfilter . "&amp;norefresh=y" . "\">" . $desc_array[$k] . "</a></th>";
}
echo "</tr>\n";

// print filter row
echo "<tr align=\"center\" class=\"grouplist\"><td width=22 height=34></td><td>";
echo "<input type=\"submit\" name=\"apply_filter\" value=\"" . _("Filter") . "\">";
echo "</td>";
// print input boxes for filters
for ($k = 0; $k < sizeof ($desc_array); $k++) {
  echo "<td>";
  echo ("<input type=\"text\" size=15 name=\"filter" . strtolower ($attr_array[$k]) .
	"\" value=\"" . $_POST["filter" . strtolower($attr_array[$k])] . "\">");
  echo "</td>";
}
echo "</tr>\n";

// calculate which rows to show
$table_begin = ($page - 1) * $max_pageentrys;
if (($page * $max_pageentrys) > sizeof($grp_info)) $table_end = sizeof($grp_info);
else $table_end = ($page * $max_pageentrys);

// print group list
for ($i = $table_begin; $i < $table_end; $i++) {
	echo("<tr class=\"grouplist\" onMouseOver=\"group_over(this, '" . $grp_info[$i]["dn"] . "')\"" .
								" onMouseOut=\"group_out(this, '" . $grp_info[$i]["dn"] . "')\"" .
								" onClick=\"group_click(this, '" . $grp_info[$i]["dn"] . "')\"" .
								" onDblClick=\"parent.frames[1].location.href='../account.php?type=group&amp;DN=" . $grp_info[$i]["dn"] . "'\">" .
								" <td height=22><input onClick=\"group_click(this, '" . $grp_info[$i]["dn"] . "')\" type=\"checkbox\" name=\"" . $grp_info[$i]["dn"] . "\"></td>" .
								" <td align='center'><a href=\"../account.php?type=group&amp;DN='" . $grp_info[$i]["dn"] . "'\">" . _("Edit") . "</a></td>");
	for ($k = 0; $k < sizeof($attr_array); $k++) {
		echo ("<td>");
		// print all attribute entries seperated by "; "
		if (sizeof($grp_info[$i][strtolower($attr_array[$k])]) > 0) {
			// delete first array entry which is "count"
			if ((! $_GET['norefresh']) && (is_array($grp_info[$i][strtolower($attr_array[$k])]))) array_shift($grp_info[$i][strtolower($attr_array[$k])]);
			// generate links for group members
			if (strtolower($attr_array[$k]) == "memberuid") {
				$linklist = array();
				for ($d = 0; $d < sizeof($grp_info[$i][strtolower($attr_array[$k])]); $d++) {
					$user = $grp_info[$i][strtolower($attr_array[$k])][$d]; // user name
					// make a link for each member of the group
					$linklist[$d] = "<a href=\"userlink.php?user='" . $user . "' \">" . $user . "</a>";
				}
				echo implode("; ", $linklist);
			}
			// print all other attributes
			else {
				if (is_array($grp_info[$i][strtolower($attr_array[$k])])) {
					echo implode("; ", $grp_info[$i][strtolower($attr_array[$k])]);
				}
				else echo $grp_info[$i][strtolower($attr_array[$k])];
			}
		}
		echo ("</td>");
	}
	echo("</tr>\n");
}
echo ("</table>");
echo ("<br>");

// draw navigation bar if group accounts were found
if (sizeof($grp_info) > 0) {
draw_navigation_bar(sizeof($grp_info));
echo ("<br>\n");
}

if (! $_GET['norefresh']) {
	// generate list of possible suffixes
	$grp_units = $_SESSION['ldap']->search_units($_SESSION["config"]->get_GroupSuffix());
}

echo ("<p align=\"left\">\n");
echo ("<input type=\"submit\" name=\"new_group\" value=\"" . _("New Group") . "\">\n");
if (sizeof($grp_info) > 0) echo ("<input type=\"submit\" name=\"del_group\" value=\"" . _("Delete Group(s)") . "\">\n");
// print combobox with possible sub-DNs
if (sizeof($grp_units) > 1) {
echo ("&nbsp;&nbsp;&nbsp;&nbsp;<b>" . _("Suffix") . ": </b>");
echo ("<select size=1 name=\"grp_suffix\">\n");
for ($i = 0; $i < sizeof($grp_units); $i++) {
	if ($grp_suffix == $grp_units[$i]) echo ("<option selected>" . $grp_units[$i] . "</option>\n");
	else echo("<option>" . $grp_units[$i] . "</option>\n");
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

  echo ("<table class=\"groupnav\" width=\"100%\" border=\"0\">\n");
  echo ("<tr>\n");
  echo ("<td><input type=\"submit\" name=\"refresh\" value=\"" . _("Refresh") . "\">&nbsp;&nbsp;");
  if ($page != 1)
    echo ("<a href=\"listgroups.php?page=" . ($page - 1) . "&amp;sort=" . $sort . $searchfilter . "\">&lt;=</a>\n");
  else
    echo ("&lt;=");
  echo ("&nbsp;");

  if ($page < ($count / $max_pageentrys))
    echo ("<a href=\"listgroups.php?page=" . ($page + 1) . "&amp;sort=" . $sort . $searchfilter . "\">=&gt;</a>\n");
  else
    echo ("=&gt;</td>");

  echo ("<td class=\"groupnav-text\">");
  echo "&nbsp;" . $count . " " .  _("Group(s) found");
  echo ("</td>");

  echo ("<td class=\"groupnav-activepage\" align=\"right\">");
  for ($i = 0; $i < ($count / $max_pageentrys); $i++) {
    if ($i == $page - 1)
      echo ("&nbsp;" . ($i + 1));
    else
      echo ("&nbsp;<a href=\"listgroups.php?page=" . ($i + 1) .
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

// save variables to session
$_SESSION['grp_info'] = $grp_info;
$_SESSION['grp_units'] = $grp_units;
$_SESSION['grp_suffix'] = $grp_suffix;

?>
