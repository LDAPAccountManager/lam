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
@session_start();

echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\" />";

// Samba hosts have the attribute "sambaAccount" and end with "$"
$filter = "(objectClass=posixGroup)";
$attrs = array("cn", "gidNumber", "memberUID", "description");
$sr = ldap_search($_SESSION["ldap"]->server(),
	$_SESSION["config"]->get_GroupSuffix(),
	$filter, $attrs);
$info = ldap_get_entries($_SESSION["ldap"]->server, $sr);
ldap_free_result($sr);

// print host table header
echo "<table width=\"100%\">\n";
echo "<tr>";
echo "<th class=\"userlist\">" . _("Grup Name") . "</th>";
echo "<th class=\"userlist\">" . _("GID Number") . "</th>";
echo "<th class=\"userlist\">" . _("Group Members") . "</th>";
echo "<th class=\"userlist\">" . _("Description") . "</th>";
echo "</tr>";
// print host list
for ($i = 0; $i < sizeof($info)-1; $i++) { // ignore last entry in array which is "count"
	echo("<tr>");
	echo ("<td class=\"userlist\">" . $info[$i]["cn"][0] . "</td>");
	echo ("<td class=\"userlist\">" . $info[$i]["gidnumber"][0] . "</td>");
	// create list of group members
	array_shift($info[$i]["memberuid"]); // delete count entry
	$grouplist = implode("; ", $info[$i]["memberuid"]);
	echo ("<td class=\"userlist\">" . $grouplist . "</td>");
	echo ("<td class=\"userlist\">" . $info[$i]["description"][0] . "</td>");
	echo("</tr>");
}
echo ("</table>");
?>
