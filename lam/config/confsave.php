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

  
  
  
*/

include_once ('config.php');
$conf = new Config();

// check if password is correct 
if ($passwd != $conf->get_Passwd()) {
	require('confmain.php');
	exit;
}

// check new preferences
if (chop($host) == "") {
	echo _("<b>" . _("Hostname is empty!") . "</b>");
	exit;
}
if (chop($port) == "") {
	echo _("<b>" . _("Portnumber is empty!") . "</b>");
	exit;
}
if (chop($admins) == "") {
	echo _("<b>" . _("List of admin users is empty!") . "</b>");
	exit;
}

// set new preferences
$conf->set_Host($host);
$conf->set_Port($port);
$conf->set_Adminstring($admins);
if ($ssl == "on") $conf->set_SSL("True");
else $conf->set_SSL("False");

echo ("<br><b><big><big><p align=\"center\"> LDAP Account Manager</b></big></big></p><br><br>");


// check if password was changed
if ($pass1 != $pass2) {
	echo _("<b>" . _("Passwords are different!") . "</b>");
	exit;
}
if ($pass1 != "") {
	$conf->set_Passwd($pass1);
	echo ("<b>" . _("Password changed!") . "</b><br><br>");
}
// save settings
echo ("<b>" . _("Saving the following settings:") . "</b><br><br>");
$conf->printconf();
?> 
