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


  confsave saves the new preferences to lam.conf

*/

// start session
session_save_path("../../sess");
session_start();

include_once ('../../lib/config.inc');
$conf = new Config();

// get data if register_globals is off
if ($_POST['passwd']) $passwd = $_POST['passwd'];
if ($_POST['passwd1']) $passwd1 = $_POST['passwd1'];
if ($_POST['passwd2']) $passwd2 = $_POST['passwd2'];
if ($_POST['serverurl']) $serverurl = $_POST['serverurl'];
if ($_POST['admins']) $admins = $_POST['admins'];
if ($_POST['suffusers']) $suffusers = $_POST['suffusers'];
if ($_POST['suffgroups']) $suffgroups = $_POST['suffgroups'];
if ($_POST['suffhosts']) $suffhosts = $_POST['suffhosts'];
if ($_POST['minUID']) $minUID = $_POST['minUID'];
if ($_POST['maxUID']) $maxUID = $_POST['maxUID'];
if ($_POST['minGID']) $minGID = $_POST['minGID'];
if ($_POST['maxGID']) $maxGID = $_POST['maxGID'];
if ($_POST['minMach']) $minMach = $_POST['minMach'];
if ($_POST['maxMach']) $maxMach = $_POST['maxMach'];
if ($_POST['usrlstattr']) $usrlstattr = $_POST['usrlstattr'];
if ($_POST['grplstattr']) $grplstattr = $_POST['grplstattr'];
if ($_POST['hstlstattr']) $hstlstattr = $_POST['hstlstattr'];
if ($_POST['maxlistentries']) $maxlistentries = $_POST['maxlistentries'];

// check if password is correct
// if not: load login page
if ($passwd != $conf->get_Passwd()) {
	require('conflogin.php');
	exit;
}

echo ("<p align=\"center\"><a href=\"http://lam.sf.net\" target=\"new_window\"><img src=\"../../graphics/banner.jpg\" border=1></a></p><hr><br><br>");

// check new preferences
if (chop($serverurl) == "") {
	echo _("<font color=\"red\"><b>" . _("Server Address is empty!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (chop($admins) == "") {
	echo _("<font color=\"red\"><b>" . _("List of admin users is empty!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (chop($suffusers) == "") {
	echo _("<font color=\"red\"><b>" . _("UserSuffix is empty!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (chop($suffgroups) == "") {
	echo _("<font color=\"red\"><b>" . _("UserSuffix is empty!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (chop($suffhosts) == "") {
	echo _("<font color=\"red\"><b>" . _("HostSuffix is empty!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (chop($minUID) == "") {
	echo _("<font color=\"red\"><b>" . _("MinUID is empty!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (chop($maxUID) == "") {
	echo _("<font color=\"red\"><b>" . _("MaxUID is empty!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (chop($minGID) == "") {
	echo _("<font color=\"red\"><b>" . _("MinGID is empty!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (chop($maxGID) == "") {
	echo _("<font color=\"red\"><b>" . _("MaxGID is empty!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (chop($minMach) == "") {
	echo _("<font color=\"red\"><b>" . _("MinMachine is empty!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (chop($maxMach) == "") {
	echo _("<font color=\"red\"><b>" . _("MaxMachine is empty!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (chop($usrlstattr) == "") {
	echo _("<font color=\"red\"><b>" . _("No attributes in user list!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (chop($grplstattr) == "") {
	echo _("<font color=\"red\"><b>" . _("No attributes in group list!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (chop($hstlstattr) == "") {
	echo _("<font color=\"red\"><b>" . _("No attributes in host list!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (chop($maxlistentries) == "") {
	echo _("<font color=\"red\"><b>" . _("Max list entries is empty!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}

// set new preferences
$conf->set_ServerURL($serverurl);
$conf->set_Adminstring($admins);
$conf->set_UserSuffix($suffusers);
$conf->set_GroupSuffix($suffgroups);
$conf->set_HostSuffix($suffhosts);
$conf->set_minUID($minUID);
$conf->set_maxUID($maxUID);
$conf->set_minGID($minGID);
$conf->set_maxGID($maxGID);
$conf->set_minMachine($minMach);
$conf->set_maxMachine($maxMach);
$conf->set_userlistAttributes($usrlstattr);
$conf->set_grouplistAttributes($grplstattr);
$conf->set_hostlistAttributes($hstlstattr);
$conf->set_MaxListEntries($maxlistentries);

// check if password was changed
if ($pass1 != $pass2) {
	echo _("<b>" . _("Passwords are different!") . "</b>");
	exit;
}
// set new password
if ($pass1 != "") {
	$conf->set_Passwd($pass1);
	echo ("<b>" . _("Password changed!") . "</b><br><br>");
}
// save settings and display new settings
$conf->save();
echo ("<b>" . _("The following settings were saved:") . "</b><br><br>");
$conf->printconf();
echo ("<br><br><br><br><br><a href=\"../login.php\" target=\"_top\">" . _("Back to Login") . "</a>");

?>
