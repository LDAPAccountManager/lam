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

// class representing local user entry with attributes of ldap user entry
include_once("userentry.php");

echo "<head>";

echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\" />";
echo "<script src=\"./functions.js\" type=\"text/javascript\" language=\"javascript\"></script>";
echo "</head>";
echo "<body bgcolor=\"#F5F5F5\">";
// config object should be in session!!!
$config = new Config();
$ldap = new Ldap($config);

// username/password should also be in session!!!
$username = "cn=admin,o=test,c=de";
$passwd = "secret";
$result = $ldap->connect ($username, $passwd);

$user_dn_list = $ldap->getUsers ($config->get_UserSuffix());

echo "<table width=\"100%\">\n";

// print attribute headers
echo "<tr>";
echo "<th class=\"userlist\"></th>";
echo "<th class=\"userlist\">Vorname</th>";
echo "<th class=\"userlist\">Nachname</th>";
echo "<th class=\"userlist\">Uid</th>";
echo "<th class=\"userlist\">Home Verzeichnis</th>";
echo "</tr>";
$row_number = 0;
foreach ($user_dn_list as $user_dn) {

echo "<tr onmouseover=\"setPointer(this, " . $row_number . ", 'over', '#DDDDDD', '#CCFFCC', '#FFCC99');\" onmouseout=\"setPointer(this, " . $row_number . ", 'out', '#DDDDDD', '#CCFFCC', '#FFCC99');\" onmousedown=\"setPointer(this, " . $row_number . ", 'click', '#DDDDDD', '#CCFFCC', '#FFCC99');\">\n";
$row_number++;

  $userentry = new UserEntry();
  $userentry = $ldap->getEntry ($user_dn, $userentry);
  $ldap->getEntry ($user_dn, $userentry);
  echo ("<td class=\"userlist\">" . "<input type=\"checkbox\" name=\"..\"" . "</td>");
  echo ("<td class=\"userlist\">" . current ($userentry->getGivenName()) . "</td>");
  echo ("<td class=\"userlist\">" . current ($userentry->getSn()) . "</td>");    
  echo ("<td class=\"userlist\">" . current ($userentry->getUid()) . "</td>");    
  echo ("<td class=\"userlist\">" . current ($userentry->gethomeDirectory()) . "</td>");
  echo "</tr>\n";
}
echo "</table>";
echo "</body>";

$ldap->close();

?>
 
