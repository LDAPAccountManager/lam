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

// get sorting column when register_globals is off
$list = $_GET['list'];

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
		echo("<meta http-equiv=\"refresh\" content=\"0; URL=../delete.php?type=host&DN='$hoststr'\">");
		}
		exit;
}

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

// Samba hosts have the attribute "sambaAccount" and end with "$"
$filter = "(&(objectClass=sambaAccount) (uid=*$))";
$attrs = $attr_array;
$sr = @ldap_search($_SESSION["ldap"]->server(),
	$_SESSION["config"]->get_HostSuffix(),
	$filter, $attrs);
if ($sr) {
	$info = ldap_get_entries($_SESSION["ldap"]->server, $sr);
	ldap_free_result($sr);
	if ($info["count"] == 0) StatusMessage("WARN", "", _("No Samba Hosts found!"));
	// delete first array entry which is "count"
	array_shift($info);
	// sort rows by sort column ($list)
	usort($info, "cmp_array");
}
else StatusMessage("ERROR", _("LDAP Search failed! Please check your preferences."), _("No Samba Hosts found!"));

echo ("<form action=\"listhosts.php\" method=\"post\">\n");

// print host table header
echo "<table rules=\"all\" class=\"hostlist\" width=\"100%\">\n";
echo "<tr class=\"hostlist_head\"><th width=22 height=34></th><th></th>";
// table header
for ($k = 0; $k < sizeof($desc_array); $k++) {
	echo "<th><a href=\"listhosts.php?list=" . strtolower($attr_array[$k]) . "\">" . $desc_array[$k] . "</a></th>";
}
echo "</tr>\n";

// print host list
for ($i = 0; $i < sizeof($info); $i++) {
	echo("<tr class=\"hostlist\" onMouseOver=\"host_over(this, '" . $info[$i]["dn"] . "')\"" .
								" onMouseOut=\"host_out(this, '" . $info[$i]["dn"] . "')\"" .
								" onClick=\"host_click(this, '" . $info[$i]["dn"] . "')\"" .
								" onDblClick=parent.frames[1].location.href=\"../account.php?type=host&DN='" . $info[$i]["dn"] . "'\">" .
								" <td height=22><input onClick=\"host_click(this, '" . $info[$i]["dn"] . "')\" type=\"checkbox\" name=\"" . $info[$i]["dn"] . "\"></td>" .
								" <td align='center'><a href=\"../account.php?type=host&DN='" . $info[$i]["dn"] . "'\">" . _("Edit") . "</a></td>");
	for ($k = 0; $k < sizeof($attr_array); $k++) {
		echo ("<td>");
		// print all attribute entries seperated by "; "
		if (sizeof($info[$i][strtolower($attr_array[$k])]) > 0) {
			// delete first array entry which is "count"
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
echo ("<tr><td align=\"left\"><input type=\"submit\" name=\"new_host\" value=\"" . _("New Host") . "\"></td>");
echo ("<td align=\"left\"><input type=\"submit\" name=\"del_host\" value=\"" . _("Delete Host(s)") . "\"></td></tr>");
echo ("</table>\n");
echo ("</form>\n");
echo "</body></html>\n";

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
