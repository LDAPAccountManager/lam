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

echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\" />";

// Samba hosts have the attribute "sambaAccount" and end with "$"
$filter = "(objectClass=posixGroup)";
$attrs = array("cn", "gidNumber", "memberUID", "description");
$sr = @ldap_search($_SESSION["ldap"]->server(),
	$_SESSION["config"]->get_GroupSuffix(),
	$filter, $attrs);
if ($sr) {
	$info = ldap_get_entries($_SESSION["ldap"]->server, $sr);
	ldap_free_result($sr);
}
else echo ("<br><br><font color=\"red\"><b>" . _("No Groups found!") . "</b></font><br><br>");

// print host table header
echo "<table width=\"100%\">\n";
echo "<tr><th class=\"userlist\" width=12></th>";
echo "<th class=\"userlist\">" . _("Grup Name") . "</th>";
echo "<th class=\"userlist\">" . _("GID Number") . "</th>";
echo "<th class=\"userlist\">" . _("Group Members") . "</th>";
echo "<th class=\"userlist\">" . _("Description") . "</th>";
echo "</tr>\n";
echo ("<form action=\"../templates/account.php?type=group\" method=\"post\">\n");
// print group list
for ($i = 0; $i < sizeof($info)-1; $i++) { // ignore last entry in array which is "count"
	echo("<tr><td class=\"userlist\"><input type=\"radio\" name=\"DN\" value=\"" . $info[$i]["dn"] . "\"></td>");
	echo ("<td class=\"userlist\">" . $info[$i]["cn"][0] . "</td>");
	echo ("<td class=\"userlist\">" . $info[$i]["gidnumber"][0] . "</td>");
	// create list of group members
	if (sizeof($info[$i]["memberuid"]) > 0) {
		array_shift($info[$i]["memberuid"]); // delete count entry
		$grouplist = implode("; ", $info[$i]["memberuid"]);
	}
	else $grouplist = "";
	echo ("<td class=\"userlist\">" . $grouplist . "</td>");
	echo ("<td class=\"userlist\">" . $info[$i]["description"][0] . "</td>");
	echo("</tr>\n");
}
echo ("</table>");
echo ("<p>&nbsp</p>\n");
echo ("<p>&nbsp</p>\n");
echo ("<table align=\"left\" border=\"0\">");
echo ("<tr><td align=\"left\"><input type=\"submit\" name=\"editgroup\" value=\"" . _("Edit Group") . "\">");
echo ("&nbsp<input type=\"button\" name=\"newgroup\" value=\"" . _("New Group") . "\" onClick=\"self.location.href='../templates/account.php?type=group'\">");
echo ("&nbsp<input type=\"button\" name=\"delgroup\" value=\"" . _("Delete Group") . "\" onClick=\"self.location.href='../templates/account.php?type=delete'\"></td></tr>\n");
echo ("</table>\n");
echo ("</form>\n");
?>
