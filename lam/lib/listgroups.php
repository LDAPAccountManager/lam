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
include_once ('../config/config.php');
include_once("ldap.php");

// start session
session_save_path("../sess");
@session_start();

echo "<html><head><title>hjhj</title>";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\">\n";
echo "</head><body bgcolor=\"#F4F4F4\">\n";
echo "<script src=\"./functions.js\" type=\"text/javascript\" language=\"javascript\"></script>\n";

// generate attribute-description table
$attr_array;	// list of LDAP attributes to show
$desc_array;	// list of descriptions for the attributes
$attr_string = $_SESSION["config"]->get_grouplistAttributes();
$temp_array = explode(";", $attr_string);
$hash_table = $_SESSION["ldap"]->attributeGroupArray();
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

// Groups have the attribute "posixGroup"
$filter = "(objectClass=posixGroup)";
$attrs = array("cn", "gidNumber", "memberUID", "description");
$sr = @ldap_search($_SESSION["ldap"]->server(),
	$_SESSION["config"]->get_GroupSuffix(),
	$filter, $attrs);
if ($sr) {
	$info = ldap_get_entries($_SESSION["ldap"]->server, $sr);
	ldap_free_result($sr);
	if ($info["count"] == 0) echo ("<br><br><font color=\"red\"><b>" . _("No Grous found!") . "</b></font><br><br>");
}
else echo ("<br><br><font color=\"red\"><b>" . _("LDAP Search failed! Please check your preferences. <br> No Groups found!") . "</b></font><br><br>");

echo ("<form action=\"../templates/account.php?type=group\" method=\"post\">\n");

// print host table header
echo "<table rules='all' class=\"grouplist\" width=\"100%\">\n";
echo "<tr class=\"grouplist_head\"><th width=22 height=34></th><th></th>";
// table header
for ($k = 0; $k < sizeof($desc_array); $k++) {
	echo "<th>" . $desc_array[$k] . "</th>";
}
echo "</tr>\n";
// print group list
for ($i = 0; $i < sizeof($info)-1; $i++) { // ignore last entry in array which is "count"
	echo("<tr class=\"grouplist\" onMouseOver=\"group_over(this, '" . $info[$i]["dn"] . "')\"" .
								"onMouseOut=\"group_out(this, '" . $info[$i]["dn"] . "')\"" .
								"onClick=\"group_click(this, '" . $info[$i]["dn"] . "')\">" .
								"<td height=22><input type=\"checkbox\" name=\"" . $info[$i]["dn"] . "\"></td>" .
								"<td align='center'><a href=\"../templates/account.php?type=group,DN='" . $info[$i]["dn"] . "'\">" . _("Edit") . "</a></td>");
	for ($k = 0; $k < sizeof($attr_array); $k++) {
		echo ("<td>");
		// print all attribute entries seperated by "; "
		if (sizeof($info[$i][strtolower($attr_array[$k])]) > 0) {
			array_shift($info[$i][strtolower($attr_array[$k])]);
			echo implode("; ", $info[$i][strtolower($attr_array[$k])]);
		}
		echo ("</td>");
	}
	echo("</tr>\n");
}
echo ("</table>");
echo ("<p>&nbsp</p>\n");
echo ("<table align=\"left\" border=\"0\">");
echo ("<tr><td align=\"left\"><input type=\"button\" name=\"newgroup\" value=\"" . _("New Group") . "\" onClick=\"self.location.href='../templates/account.php?type=group'\">");
echo ("&nbsp<input type=\"button\" name=\"delgroup\" value=\"" . _("Delete Group(s)") . "\" onClick=\"self.location.href='../templates/account.php?type=delete'\"></td></tr>\n");
echo ("</table>\n");
echo ("</form>\n");
echo "</body></html>\n";
?>
