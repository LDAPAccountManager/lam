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
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/


include_once ('../config/config.php');
include_once("ldap.php");

$ldapconnection = new Ldap(new Config());
$userlist = $ldapconnection->getUsers();

//$bla = array (attr1, attr2);
//$userlist = array ($bla, $bla, $bla, $bla);

echo "<table>\n";

// print attribute headers
echo "<tr>\n";
foreach ($userlist[0] as $attributes) {
     echo ("<td>" . $attributes["dn"] . "</td>");
}
echo "</tr>\n";

// print user list
foreach ($userlist as $user_attributes) {
  echo "<tr>\n";
    foreach ($user_attributes as $attribute) {
      echo "<td>$attribute</td>\n";
    }
  echo "</tr>\n";
}
echo "</table>\n";


?>
 
