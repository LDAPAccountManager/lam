<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Leonhard Walchshäusl

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


include_once ('../config/config.php');
include_once("ldap.php");

// start session
session_save_path("../sess");
@session_start();

// class representing local user entry with attributes of ldap user entry
include_once("userentry.php");

echo "<head>";

echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\" />";
echo "<script src=\"./functions.js\" type=\"text/javascript\" language=\"javascript\"></script>";
echo "</head>";
echo "<body bgcolor=\"#F5F5F5\">";

// generate attribute-description table
$attr_array;	// list of LDAP attributes to show
$desc_array;	// list of descriptions for the attributes
$attr_string = $_SESSION["config"]->get_userlistAttributes();
$temp_array = explode(";", $attr_string);
$hash_table = $_SESSION["ldap"]->attributeUserArray();
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

$user_dn_list = $_SESSION["ldap"]->getUsers
  ($_SESSION["config"]->get_UserSuffix());


echo ("<form action=\"../templates/account.php?type=user\" method=\"post\">\n");
echo "<table width=\"100%\">\n";

// print attribute headers
echo "<tr>";
echo "<th class=\"userlist\"></th>";
for ($k = 0; $k < sizeof($desc_array); $k++) {
	echo "<th class=\"userlist\">" . $desc_array[$k] . "</th>";
}
echo "</tr>";

$row_number = 0;

foreach ($user_dn_list as $user_dn) {

echo "<tr onmouseover=\"setPointer(this, " . $row_number . ", 'over', '#DDDDDD', '#CCCCFF', '#FFCCCC');\" onmouseout=\"setPointer(this, " . $row_number . ", 'out', '#DDDDDD', '#CCCCFF', '#FFCCCC');\" onmousedown=\"setPointer(this, " . $row_number . ", 'click', '#DDDDDD', '#CCCCFF', '#FFCCCC');\">\n";
$row_number++;

  $userentry = new UserEntry();
  $userentry = $_SESSION["ldap"]->getEntry ($user_dn, $userentry);
//  $ldap->getEntry ($user_dn, $userentry);
  echo ("<td bgcolor=\"#DDDDDD\" >" . "<input type=\"checkbox\" name=\"..\"" . "</td>");
  echo ("<td bgcolor=\"#DDDDDD\" class=\"userlist\"><a href=\"../templates/account.php?type=edituser&uid=" . current ($userentry->getUid()) . "\">" . current ($userentry->getUid()) . "</a></td>");
  echo ("<td bgcolor=\"#DDDDDD\" class=\"userlist\">" . current ($userentry->getCn()) . "</td>");    
  echo ("<td bgcolor=\"#DDDDDD\" class=\"userlist\">" . current ($userentry->getSn()) . "</td>");    
  echo ("<td bgcolor=\"#DDDDDD\" class=\"userlist\">" . current ($userentry->gethomeDirectory()) . "</td>");
  echo "</tr>\n";
}
echo "</table>";

echo ("<br />");
echo ("<table cellspacing=\"4\" align=\"left\" border=\"0\">");
echo ("<tr><td>");
echo ("<input type=\"button\" name=\"deluser\" value=\"" . _("Delete selected Users") . "\" onClick=\"self.location.href='../templates/account.php?type=delete'\"></td>\n");
echo ("<td>");
echo ("<input type=\"button\" name=\"newuser\" value=\"" . _("New User") . "\" onClick=\"self.location.href='../templates/account.php?type=user'\">");
echo ("</td></tr>");
echo ("</table>\n");
echo ("</form>\n");

echo "</body>";
echo "</html>";
?>
 
