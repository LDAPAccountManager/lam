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
$conf = new Config('test');
echo "<html><head><title></title><link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\"></head><body>";
echo ("<b> Current Config</b><br><br>");
$conf->printconf();
echo ("<br><br><big><b> Starting Test...</b></big><br><br>");
// now all prferences are loaded
echo ("Loading preferences...");
$ServerURL = $conf->get_ServerURL();
$cache_timeout = $conf->get_cacheTimeout();
$Admins = $conf->get_Admins();
$Passwd = $conf->get_Passwd();
$Adminstring = $conf->get_Adminstring();
$Suff_users = $conf->get_UserSuffix();
$Suff_groups = $conf->get_GroupSuffix();
$Suff_hosts = $conf->get_HostSuffix();
$Suff_domains = $conf->get_DomainSuffix();
$MinUID = $conf->get_minUID();
$MaxUID = $conf->get_maxUID();
$MinGID = $conf->get_minGID();
$MaxGID = $conf->get_maxGID();
$MinMachine = $conf->get_minMachine();
$MaxMachine = $conf->get_maxMachine();
$userlistAttributes = $conf->get_userlistAttributes();
$grouplistAttributes = $conf->get_grouplistAttributes();
$hostlistAttributes = $conf->get_hostlistAttributes();
$maxlistentries = $conf->get_maxlistentries();
$defaultlanguage = $conf->get_defaultlanguage();
$scriptpath = $conf->get_scriptPath();
$scriptServer = $conf->get_scriptServer();
$samba3 = $conf->get_samba3();
echo ("done<br>");
// next we modify them and save lam.conf
echo ("Changing preferences...");
$conf->set_ServerURL("ldap://123.345.678.123:777");
$conf->set_cacheTimeout("33");
$conf->set_Admins(array("uid=test,o=test,dc=org","uid=root,o=test2,c=de"));
$conf->set_Passwd("123456abcde");
$conf->set_Adminstring("uid=test,o=test,dc=org;uid=root,o=test2,c=de");
$conf->set_UserSuffix("ou=test,o=test,c=de");
$conf->set_GroupSuffix("ou=testgrp,o=test,c=de");
$conf->set_HostSuffix("ou=testhst,o=test,c=de");
$conf->set_DomainSuffix("ou=testdom,o=test,c=de");
$conf->set_minUID("25");
$conf->set_maxUID("254");
$conf->set_minGID("253");
$conf->set_maxGID("1234");
$conf->set_minMachine("3");
$conf->set_maxMachine("47");
$conf->set_userlistAttributes("#uid;#cn");
$conf->set_grouplistAttributes("#gidNumber;#cn;#memberUID");
$conf->set_hostlistAttributes("#cn;#uid;#description");
$conf->set_maxlistentries("54");
$conf->set_defaultlanguage("de_AT:iso639_de:Deutsch (Oesterreich)");
$conf->set_scriptPath("/var/www/lam/lib/script");
$conf->set_scriptServer("127.0.0.1");
$conf->set_samba3("yes");
$conf->save();
echo ("done<br>");
// at last all preferences are read from lam.conf and compared
echo ("Loading and comparing...");
$conf2 = new Config('test');
if ($conf2->get_ServerURL() != "ldap://123.345.678.123:777") echo ("<br><font color=\"#FF0000\">Saving ServerURL failed!</font><br>");
if ($conf2->get_cacheTimeout() != "33") echo ("<br><font color=\"#FF0000\">Saving Cache timeout failed!</font><br>");
$adm_arr = $conf2->get_Admins();
if ($adm_arr[0] != "uid=test,o=test,dc=org") echo ("<br><font color=\"#FF0000\">Saving admins failed!" . $adm_arr[0] . "</font><br>");
if ($adm_arr[1] != "uid=root,o=test2,c=de") echo ("<br><font color=\"#FF0000\">Saving admins failed!</font><br>");
if ($conf2->get_Passwd() != "123456abcde") echo ("<br><font color=\"#FF0000\">Saving password failed!</font><br>");
if ($conf2->get_Adminstring() != "uid=test,o=test,dc=org;uid=root,o=test2,c=de") echo ("<br><font color=\"#FF0000\">Saving admin string failed!</font><br>");
if ($conf2->get_UserSuffix() != "ou=test,o=test,c=de") echo ("<br><font color=\"#FF0000\">Saving user suffix failed!</font><br>");
if ($conf2->get_GroupSuffix() != "ou=testgrp,o=test,c=de") echo ("<br><font color=\"#FF0000\">Saving group suffix failed!</font><br>");
if ($conf2->get_HostSuffix() != "ou=testhst,o=test,c=de") echo ("<br><font color=\"#FF0000\">Saving host suffix failed!</font><br>");
if ($conf2->get_DomainSuffix() != "ou=testdom,o=test,c=de") echo ("<br><font color=\"#FF0000\">Saving domain suffix failed!</font><br>");
if ($conf2->get_minUID() != "25") echo ("<br><font color=\"#FF0000\">Saving minUID failed!</font><br>");
if ($conf2->get_maxUID() != "254") echo ("<br><font color=\"#FF0000\">Saving maxUID failed!</font><br>");
if ($conf2->get_minGID() != "253") echo ("<br><font color=\"#FF0000\">Saving minGID failed!</font><br>");
if ($conf2->get_maxGID() != "1234") echo ("<br><font color=\"#FF0000\">Saving maxGID failed!</font><br>");
if ($conf2->get_minMachine() != "3") echo ("<br><font color=\"#FF0000\">Saving maxMachine failed!</font><br>");
if ($conf2->get_maxMachine() != "47") echo ("<br><font color=\"#FF0000\">Saving minMachine failed!</font><br>");
if ($conf2->get_userlistAttributes() != "#uid;#cn") echo ("<br><font color=\"#FF0000\">Saving userlistAttributes failed!</font><br>");
if ($conf2->get_grouplistAttributes() != "#gidNumber;#cn;#memberUID") echo ("<br><font color=\"#FF0000\">Saving grouplistAttributes failed!</font><br>");
if ($conf2->get_hostlistAttributes() != "#cn;#uid;#description") echo ("<br><font color=\"#FF0000\">Saving hostlistAttributes failed!</font><br>");
if ($conf2->get_maxlistentries() != "54") echo ("<br><font color=\"#FF0000\">Saving maxlistentries failed!</font><br>");
if ($conf2->get_defaultlanguage() != "de_AT:iso639_de:Deutsch (Oesterreich)") echo ("<br><font color=\"#FF0000\">Saving default language failed!</font><br>");
if ($conf2->get_scriptPath() != "/var/www/lam/lib/script") echo ("<br><font color=\"#FF0000\">Saving script path failed!</font><br>");
if ($conf2->get_scriptServer() != "127.0.0.1") echo ("<br><font color=\"#FF0000\">Saving script server failed!</font><br>");
if ($conf2->get_samba3() != "yes") echo ("<br><font color=\"#FF0000\">Saving samba3 failed!</font><br>");
echo ("done<br>");
// restore old values
echo ("Restoring old preferences...");
$conf2->set_ServerURL($ServerURL);
$conf2->set_cacheTimeout($cache_timeout);
$conf2->set_Admins($Admins);
$conf2->set_Passwd($Passwd);
$conf2->set_Adminstring($Adminstring);
$conf2->set_UserSuffix($Suff_users);
$conf2->set_GroupSuffix($Suff_groups);
$conf2->set_HostSuffix($Suff_hosts);
$conf2->set_DomainSuffix($Suff_domains);
$conf2->set_minUID($MinUID);
$conf2->set_maxUID($MaxUID);
$conf2->set_minGID($MinGID);
$conf2->set_maxGID($MaxGID);
$conf2->set_minMachine($MinMachine);
$conf2->set_maxMachine($MaxMachine);
$conf2->set_userlistAttributes($userlistAttributes);
$conf2->set_grouplistAttributes($grouplistAttributes);
$conf2->set_hostlistAttributes($hostlistAttributes);
$conf2->set_maxlistentries($maxlistentries);
$conf2->set_defaultLanguage($defaultlanguage);
$conf2->set_scriptPath($scriptpath);
$conf2->set_scriptServer($scriptserver);
$conf2->set_samba3($samba3);
$conf2->save();
echo ("done<br>");
// finished
echo ("<br><b><font color=\"#00C000\">Test is complete.</font></b>");
echo ("<br><br><b> Current Config</b><br><br>");
$conf2->printconf();

?>
