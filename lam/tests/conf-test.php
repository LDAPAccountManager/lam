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
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	This test reads all preferences from lam.conf. Then it writes new values and verifies
	if they were written. At last the old values are restored.
*/

include ("../lib/config.inc");
$conf = new Config();
echo ("<b> Current Config</b><br><br>");
$conf->printconf();
echo ("<br><br><big><b> Starting Test...</b></big><br><br>");
// now all prferences are loaded
echo ("Loading preferences...");
$ServerURL = $conf->get_ServerURL();
$Admins = $conf->get_Admins();
$Passwd = $conf->get_Passwd();
$Adminstring = $conf->get_Adminstring();
$Suff_users = $conf->get_UserSuffix();
$Suff_groups = $conf->get_GroupSuffix();
$Suff_hosts = $conf->get_HostSuffix();
$MinUID = $conf->get_minUID();
$MaxUID = $conf->get_maxUID();
$MinGID = $conf->get_minGID();
$MaxGID = $conf->get_maxGID();
$MinMachine = $conf->get_minMachine();
$MaxMachine = $conf->get_maxMachine();
$userlistAttributes = $conf->get_userlistAttributes();
$grouplistAttributes = $conf->get_grouplistAttributes();
$hostlistAttributes = $conf->get_hostlistAttributes();
echo ("done<br>");
// next we modify them and save lam.conf
echo ("Changing preferences...");
$conf->set_ServerURL("ldap://123.345.678.123:777");
$conf->set_Admins(array("uid=test,o=test,dc=org","uid=root,o=test2,c=de"));
$conf->set_Passwd("123456abcde");
$conf->set_Adminstring("uid=test,o=test,dc=org;uid=root,o=test2,c=de");
$conf->set_UserSuffix("ou=test,o=test,c=de");
$conf->set_GroupSuffix("ou=testgrp,o=test,c=de");
$conf->set_HostSuffix("ou=testhst,o=test,c=de");
$conf->set_minUID("25");
$conf->set_maxUID("254");
$conf->set_minGID("253");
$conf->set_maxGID("1234");
$conf->set_minMachine("3");
$conf->set_maxMachine("47");
$conf->set_userlistAttributes("#uid;#cn");
$conf->set_grouplistAttributes("#gidNumber;#cn;#memberUID");
$conf->set_hostlistAttributes("#cn;#uid;#description");
$conf->save();
echo ("done<br>");
// at last all preferences are read from lam.conf and compared
echo ("Loading and comparing...");
$conf = new Config();
if ($conf->get_ServerURL() != "ldap://123.345.678.123:777") echo ("<br><font color=\"#FF0000\">Saving ServerURL failed!</font><br>");
$adm_arr = $conf->get_Admins();
if ($adm_arr[0] != "uid=test,o=test,dc=org") echo ("<br><font color=\"#FF0000\">Saving admins failed!" . $adm_arr[0] . "</font><br>");
if ($adm_arr[1] != "uid=root,o=test2,c=de") echo ("<br><font color=\"#FF0000\">Saving admins failed!</font><br>");
if ($conf->get_Passwd() != "123456abcde") echo ("<br><font color=\"#FF0000\">Saving password failed!</font><br>");
if ($conf->get_Adminstring() != "uid=test,o=test,dc=org;uid=root,o=test2,c=de") echo ("<br><font color=\"#FF0000\">Saving admin string failed!</font><br>");
if ($conf->get_UserSuffix() != "ou=test,o=test,c=de") echo ("<br><font color=\"#FF0000\">Saving user suffix failed!</font><br>");
if ($conf->get_GroupSuffix() != "ou=testgrp,o=test,c=de") echo ("<br><font color=\"#FF0000\">Saving group suffix failed!</font><br>");
if ($conf->get_HostSuffix() != "ou=testhst,o=test,c=de") echo ("<br><font color=\"#FF0000\">Saving host suffix failed!</font><br>");
if ($conf->get_minUID() != "25") echo ("<br><font color=\"#FF0000\">Saving minUID failed!</font><br>");
if ($conf->get_maxUID() != "254") echo ("<br><font color=\"#FF0000\">Saving maxUID failed!</font><br>");
if ($conf->get_minGID() != "253") echo ("<br><font color=\"#FF0000\">Saving minGID failed!</font><br>");
if ($conf->get_maxGID() != "1234") echo ("<br><font color=\"#FF0000\">Saving maxGID failed!</font><br>");
if ($conf->get_minMachine() != "3") echo ("<br><font color=\"#FF0000\">Saving maxMachine failed!</font><br>");
if ($conf->get_maxMachine() != "47") echo ("<br><font color=\"#FF0000\">Saving minMachine failed!</font><br>");
if ($conf->get_userlistAttributes() != "#uid;#cn") echo ("<br><font color=\"#FF0000\">Saving userlistAttributes failed!</font><br>");
if ($conf->get_grouplistAttributes() != "#gidNumber;#cn;#memberUID") echo ("<br><font color=\"#FF0000\">Saving grouplistAttributes failed!</font><br>");
if ($conf->get_hostlistAttributes() != "#cn;#uid;#description") echo ("<br><font color=\"#FF0000\">Saving hostlistAttributes failed!</font><br>");
echo ("done<br>");
// restore old values
echo ("Restoring old preferences...");
$conf->set_ServerURL($ServerURL);
$conf->set_Admins($Admins);
$conf->set_Passwd($Passwd);
$conf->set_Adminstring($Adminstring);
$conf->set_UserSuffix($Suff_users);
$conf->set_GroupSuffix($Suff_groups);
$conf->set_HostSuffix($Suff_hosts);
$conf->set_minUID($MinUID);
$conf->set_maxUID($MaxUID);
$conf->set_minGID($MinGID);
$conf->set_maxGID($MaxGID);
$conf->set_minMachine($MinMachine);
$conf->set_maxMachine($MaxMachine);
$conf->set_userlistAttributes($userlistAttributes);
$conf->set_grouplistAttributes($grouplistAttributes);
$conf->set_hostlistAttributes($hostlistAttributes);
$conf->save();
echo ("done<br>");
// finished
echo ("<br><b><font color=\"#00C000\">Test is complete.</font></b>");
echo ("<br><br><b> Current Config</b><br><br>");
$conf->printconf();

?>
