<?
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
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	This test tries to connect to the LDAP server and prints a list of all groups, users and machines (DNs).  
	Username and password have to be individually changed.  
*/

include_once ("../lib/ldap.php");
include_once ("../config/config.php");
$conf = new Config();
$test = new ldap($conf);
// connect to LDAP server
$test->connect("cn=admin,o=test,c=de", "linux");
$gr_arr = $test->getGroups();
$usr_arr = $test->getUsers();
$hst_arr = $test->getMachines();
$test->close();
// print lists
echo ("<big><b>GROUPS</b></big><br>");
for ($i = 0; $i < count($gr_arr); $i++) echo($gr_arr[$i] . "<br>");
echo ("<hr>");
echo ("<big><b>USERS</b></big><br>");
for ($i = 0; $i < count($usr_arr); $i++) echo($usr_arr[$i] . "<br>");
echo ("<hr>");
echo ("<big><b>HOSTS</b></big><br>");
for ($i = 0; $i < count($hst_arr); $i++) echo($hst_arr[$i] . "<br>");

?> 
