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
if ($_SESSION['passwd']) $passwd = $_SESSION['passwd'];
if ($_SESSION['passwd1']) $passwd1 = $_SESSION['passwd1'];
if ($_SESSION['passwd2']) $passwd2 = $_SESSION['passwd2'];
if ($_SESSION['serverurl']) $serverurl = $_SESSION['serverurl'];
if ($_SESSION['admins']) $admins = $_SESSION['admins'];
if ($_SESSION['suffusers']) $suffusers = $_SESSION['suffusers'];
if ($_SESSION['suffgroups']) $suffgroups = $_SESSION['suffgroups'];
if ($_SESSION['suffhosts']) $suffhosts = $_SESSION['suffhosts'];
if ($_SESSION['minUID']) $minUID = $_SESSION['minUID'];
if ($_SESSION['maxUID']) $maxUID = $_SESSION['maxUID'];
if ($_SESSION['minGID']) $minGID = $_SESSION['minGID'];
if ($_SESSION['maxGID']) $maxGID = $_SESSION['maxGID'];
if ($_SESSION['minMach']) $minMach = $_SESSION['minMach'];
if ($_SESSION['maxMach']) $maxMach = $_SESSION['maxMach'];
if ($_SESSION['usrlstattr']) $usrlstattr = $_SESSION['usrlstattr'];
if ($_SESSION['grplstattr']) $grplstattr = $_SESSION['grplstattr'];
if ($_SESSION['hstlstattr']) $hstlstattr = $_SESSION['hstlstattr'];
if ($_SESSION['maxlistentries']) $maxlistentries = $_SESSION['maxlistentries'];
if ($_SESSION['language']) $language = $_SESSION['language'];
if ($_SESSION['scriptpath']) $scriptpath = $_SESSION['scriptpath'];
if ($_SESSION['scriptserver']) $scriptserver = $_SESSION['scriptserver'];
if ($_SESSION['samba3']) $samba3 = $_SESSION['samba3'];
if ($_SESSION['domainSID']) $domainSID = $_SESSION['domainSID'];

// check if password is correct
// if not: load login page
if ($passwd != $conf->get_Passwd()) {
	require('conflogin.php');
	exit;
}

echo ("<?xml version=\"1.0\" encoding=\"ISO-8859-15\"?>\n");
echo ("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n");

echo "<html><head><title>listusers</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head><body>\n";

echo ("<p align=\"center\"><a href=\"http://lam.sf.net\" target=\"new_window\">".
	"<img src=\"../../graphics/banner.jpg\" border=1 alt=\"LDAP Account Manager\"></a></p><hr><br><br>");

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

if (chop($language) == "") {
	echo _("<font color=\"red\"><b>" . _("Language is not defined!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}

if (chop($samba3) == "") {
	echo _("<font color=\"red\"><b>" . _("Samba version is not defined!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}

if ((chop($samba3) == "yes") && (($domainSID == "") || (!$domainSID))) {
	echo _("<font color=\"red\"><b>" . _("Samba 3 needs a domain SID!") . "</b></font>");
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
$conf->set_defaultLanguage($language);
$conf->set_samba3($samba3);
$conf->set_domainSID($domainSID);
// optional
if ($_SESSION['scriptpath']) $conf->set_scriptpath($scriptpath);
else $conf->set_scriptpath("");
if ($_SESSION['scriptserver']) $conf->set_scriptserver($scriptserver);
else $conf->set_scriptserver("");



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

echo("</body></html>");

// remove settings from session
unset($_SESSION['passwd']);
unset($_SESSION['passwd1']);
unset($_SESSION['passwd2']);
unset($_SESSION['serverurl']);
unset($_SESSION['admins']);
unset($_SESSION['suffusers']);
unset($_SESSION['suffgroups']);
unset($_SESSION['suffhosts']);
unset($_SESSION['minUID']);
unset($_SESSION['maxUID']);
unset($_SESSION['minGID']);
unset($_SESSION['maxGID']);
unset($_SESSION['minMach']);
unset($_SESSION['maxMach']);
unset($_SESSION['usrlstattr']);
unset($_SESSION['grplstattr']);
unset($_SESSION['hstlstattr']);
unset($_SESSION['maxlistentries']);
unset($_SESSION['language']);
unset($_SESSION['scriptpath']);
unset($_SESSION['scriptserver']);
unset($_SESSION['samba3']);
unset($_SESSION['domainSID']);

?>
