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
$list = $_GET['list'];

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
		echo("<meta http-equiv=\"refresh\" content=\"0; URL=../delete.php?type=group&DN='$groupstr'\">");
		}
		exit;
}

echo "<html><head><title>listgroups</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head><body>\n";
echo "<script src=\"../../lib/functions.js\" type=\"text/javascript\" language=\"javascript\"></script>\n";

// generate attribute-description table
$attr_array;	// list of LDAP attributes to show
$desc_array;	// list of descriptions for the attributes
$attr_string = $_SESSION["config"]->get_grouplistAttributes();
$temp_array = explode(";", $attr_string);
$hash_table = $_SESSION["ldap"]->attributeGroupArray();

// get current page
$page = $_GET["page"];
if (!$page) $page = 1;
// take maximum count of user entries shown on one page out of session
if ($_SESSION["config"]->get_MaxListEntries() <= 0)
	$max_pageentrys = 10;	// default setting, if not yet set
else
	$max_pageentrys = $_SESSION["config"]->get_MaxListEntries();

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
$sr = @ldap_search($_SESSION["ldap"]->server(),
	$_SESSION["config"]->get_GroupSuffix(),
	$filter, $attrs);
if ($sr) {
	$info = ldap_get_entries($_SESSION["ldap"]->server, $sr);
	ldap_free_result($sr);
	if ($info["count"] == 0) StatusMessage("WARN", "", _("No Groups found!"));
	// delete first array entry which is "count"
	array_shift($info);
	// sort rows by sort column ($list)
	usort($info, "cmp_array");
}
else StatusMessage("ERROR", _("LDAP Search failed! Please check your preferences."), _("No Groups found!"));

echo ("<form action=\"listgroups.php\" method=\"post\">\n");

draw_navigation_bar(sizeof($info));
echo ("<br>");

// print group table header
echo "<table rules=\"all\" class=\"grouplist\" width=\"100%\">\n";
echo "<tr class=\"grouplist_head\"><th width=22 height=34></th><th></th>";
// table header
for ($k = 0; $k < sizeof($desc_array); $k++) {
	if (strtolower($attr_array[$k]) == $list) {
		echo "<th class=\"grouplist_sort\"><a href=\"listgroups.php?list=" . strtolower($attr_array[$k]) . "\">" . $desc_array[$k] . "</a></th>";
	}
	else echo "<th><a href=\"listgroups.php?list=" . strtolower($attr_array[$k]) . "\">" . $desc_array[$k] . "</a></th>";
}
echo "</tr>\n";

// print filter row
echo "<tr align=\"center\" class=\"grouplist\"><td width=22 height=34></td><td>";
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
if (($page * $max_pageentrys) > sizeof($info)) $table_end = sizeof($info);
else $table_end = ($page * $max_pageentrys);

// print group list
for ($i = $table_begin; $i < $table_end; $i++) {
	echo("<tr class=\"grouplist\" onMouseOver=\"group_over(this, '" . $info[$i]["dn"] . "')\"" .
								" onMouseOut=\"group_out(this, '" . $info[$i]["dn"] . "')\"" .
								" onClick=\"group_click(this, '" . $info[$i]["dn"] . "')\"" .
								" onDblClick=parent.frames[1].location.href=\"../account.php?type=group&DN='" . $info[$i]["dn"] . "'\">" .
								" <td height=22><input onClick=\"group_click(this, '" . $info[$i]["dn"] . "')\" type=\"checkbox\" name=\"" . $info[$i]["dn"] . "\"></td>" .
								" <td align='center'><a href=\"../account.php?type=group&DN='" . $info[$i]["dn"] . "'\">" . _("Edit") . "</a></td>");
	for ($k = 0; $k < sizeof($attr_array); $k++) {
		echo ("<td>");
		// print all attribute entries seperated by "; "
		if (sizeof($info[$i][strtolower($attr_array[$k])]) > 0) {
			// delete first array entry which is "count"
			array_shift($info[$i][strtolower($attr_array[$k])]);
			// generate links for group members
			if (strtolower($attr_array[$k]) == "memberuid") {
				$linklist = array();
				for ($d = 0; $d < sizeof($info[$i][strtolower($attr_array[$k])]); $d++) {
					$user = $info[$i][strtolower($attr_array[$k])][$d]; // user name
					$dn = $_SESSION["ldap"]->search_username($user); // DN entry
					// if user was found in LDAP make link, otherwise just print name
					if ($dn) {
						$linklist[$d] = "<a href=../account.php?type=user&DN='" . $dn . "' >" .
										$info[$i][strtolower($attr_array[$k])][$d] . "</a>";
					}
					else $linklist[$d] = $user;
				}
				echo implode("; ", $linklist);
			}
			// print all other attributes
			else {
				echo implode("; ", $info[$i][strtolower($attr_array[$k])]);
			}
		}
		echo ("</td>");
	}
	echo("</tr>\n");
}
echo ("</table>");
echo ("<br>");

draw_navigation_bar(sizeof($info));

echo ("<br>\n");
echo ("<table align=\"left\" border=\"0\"\n>");
echo ("<tr><td align=\"left\"><input type=\"submit\" name=\"new_group\" value=\"" . _("New Group") . "\"></td>\n");
echo ("<td align=\"left\"><input type=\"submit\" name=\"del_group\" value=\"" . _("Delete Group(s)") . "\"></td></tr>\n");
echo ("</table>\n");
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
  global $list;

  echo ("<table class=\"groupnav\" width=\"100%\" border=\"0\">\n");
  echo ("<tr>\n");
  echo ("<td><input type=\"submit\" name=\"refresh\" value=\"" . _("Refresh") . "\">&nbsp;&nbsp;");
  if ($page != 1)
    echo ("<a align=\"right\" class=\"userlist\" href=\"listgroups.php?page=" . ($page - 1) . "&list=" . $list . "\"><=</a>\n");
  else
    echo ("<=");
  echo ("&nbsp;");

  if ($page < ($count / $max_pageentrys))
    echo ("<a align=\"right\" class=\"userlist\" href=\"listgrous.php?page=" . ($page + 1) . "&list=" . $list . "\">=></a>\n");
  else
    echo ("=></td>");

  echo ("<td style=\"color:red\" align=\"right\">");
  for ($i = 0; $i < ($count / $max_pageentrys); $i++) {
    if ($i == $page - 1)
      echo ("&nbsp;" . ($i + 1));
    else
      echo ("&nbsp;<a align=\"right\" class=\"userlist\" href=\"listgroups.php?page=" . ($i + 1) .
	    "&list=" . $list . "\">" . ($i + 1) . "</a>\n");
  }
  echo ("</td></tr></table>\n");
}

// compare function used for usort-method
// rows are sorted with the first attribute entry of the sort column
// if objects have attributes with multiple values the others are ignored
function cmp_array($a, $b) {
	// list specifies the sort column
	global $list;
	global $attr_array;
	// sort by first attribute with name $list
	if (!$list) $list = strtolower($attr_array[0]);
	if ($a[$list][0] == $b[$list][0]) return 0;
	else if ($a[$list][0] == max($a[$list][0], $b[$list][0])) return 1;
	else return -1;
}

?>
