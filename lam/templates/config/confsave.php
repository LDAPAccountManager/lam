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
include_once ('../../lib/config.inc');

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

$conf = new Config($_SESSION['conf_filename']);

// get data from session
if ($_SESSION['conf_passwd']) $passwd = $_SESSION['conf_passwd'];
if ($_SESSION['conf_passwd1']) $passwd1 = $_SESSION['conf_passwd1'];
if ($_SESSION['conf_passwd2']) $passwd2 = $_SESSION['conf_passwd2'];
if ($_SESSION['conf_serverurl']) $serverurl = $_SESSION['conf_serverurl'];
if (isset($_SESSION['conf_cachetimeout'])) $cachetimeout = $_SESSION['conf_cachetimeout'];
if ($_SESSION['conf_admins']) $admins = $_SESSION['conf_admins'];
if ($_SESSION['conf_suffusers']) $suffusers = $_SESSION['conf_suffusers'];
if ($_SESSION['conf_suffgroups']) $suffgroups = $_SESSION['conf_suffgroups'];
if ($_SESSION['conf_suffhosts']) $suffhosts = $_SESSION['conf_suffhosts'];
if ($_SESSION['conf_suffdomains']) $suffdomains = $_SESSION['conf_suffdomains'];
if (isset($_SESSION['conf_minUID'])) $minUID = $_SESSION['conf_minUID'];
if ($_SESSION['conf_maxUID']) $maxUID = $_SESSION['conf_maxUID'];
if (isset($_SESSION['conf_minGID'])) $minGID = $_SESSION['conf_minGID'];
if ($_SESSION['conf_maxGID']) $maxGID = $_SESSION['conf_maxGID'];
if (isset($_SESSION['conf_minMach'])) $minMach = $_SESSION['conf_minMach'];
if ($_SESSION['conf_maxMach']) $maxMach = $_SESSION['conf_maxMach'];
if ($_SESSION['conf_usrlstattr']) $usrlstattr = $_SESSION['conf_usrlstattr'];
if ($_SESSION['conf_grplstattr']) $grplstattr = $_SESSION['conf_grplstattr'];
if ($_SESSION['conf_hstlstattr']) $hstlstattr = $_SESSION['conf_hstlstattr'];
if ($_SESSION['conf_maxlistentries']) $maxlistentries = $_SESSION['conf_maxlistentries'];
if ($_SESSION['conf_lang']) $lang = $_SESSION['conf_lang'];
if ($_SESSION['conf_scriptpath']) $scriptpath = $_SESSION['conf_scriptpath'];
if ($_SESSION['conf_scriptserver']) $scriptserver = $_SESSION['conf_scriptserver'];
if ($_SESSION['conf_samba3']) $samba3 = $_SESSION['conf_samba3'];
if ($_SESSION['conf_pwdhash']) $pwdhash = $_SESSION['conf_pwdhash'];
if ($_SESSION['conf_filename']) $filename = $_SESSION['conf_filename'];

// check if password is correct
// if not: load login page
if ($passwd != $conf->get_Passwd()) {
	require('conflogin.php');
	exit;
}

echo $_SESSION['header'];

echo "<html><head><title>listusers</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head><body>\n";

echo ("<p align=\"center\"><a href=\"http://lam.sf.net\" target=\"new_window\">".
	"<img src=\"../../graphics/banner.jpg\" border=1 alt=\"LDAP Account Manager\"></a></p><hr><br><br>");

// check new preferences
if (!$serverurl) {
	echo ("<font color=\"red\"><b>" . _("Server Address is empty!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!isset($cachetimeout) || !(is_numeric($cachetimeout)) || !($cachetimeout > -1)) {
	echo ("<font color=\"red\"><b>" . _("Cache timeout is empty!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$admins || !eregi("^([a-z0-9]|-)+=([a-z0-9]|-)+(,([a-z0-9]|-)+=([a-z0-9]|-)+)+(;([a-z0-9]|-)+=([a-z0-9]|-)+(,([a-z0-9]|-)+=([a-z0-9]|-)+)+)*$", $admins)) {
	echo ("<font color=\"red\"><b>" . _("List of admin users is empty or invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$suffusers || !eregi("^(([a-z]|-|[0-9])*=([a-z]|-|[0-9])*)(,([a-z]|-|[0-9])*=([a-z]|-|[0-9])*)*$", $suffusers)) {
	echo ("<font color=\"red\"><b>" . _("UserSuffix is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$suffgroups || !eregi("^(([a-z]|-|[0-9])*=([a-z]|-|[0-9])*)(,([a-z]|-|[0-9])*=([a-z]|-|[0-9])*)*$", $suffgroups)) {
	echo ("<font color=\"red\"><b>" . _("UserSuffix is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$suffhosts || !eregi("^(([a-z]|-|[0-9])*=([a-z]|-|[0-9])*)(,([a-z]|-|[0-9])*=([a-z]|-|[0-9])*)*$", $suffhosts)) {
	echo ("<font color=\"red\"><b>" . _("HostSuffix is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (($samba3 == "yes") && !eregi("^(([a-z]|-|[0-9])*=([a-z]|-|[0-9])*)(,([a-z]|-|[0-9])*=([a-z]|-|[0-9])*)*$", $suffdomains)) {
	echo ("<font color=\"red\"><b>" . _("DomainSuffix is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!isset($minUID) || !is_numeric($minUID)) {
	echo ("<font color=\"red\"><b>" . _("MinUID is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$maxUID || !is_numeric($maxUID)) {
	echo ("<font color=\"red\"><b>" . _("MaxUID is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!isset($minGID) || !is_numeric($minGID)) {
	echo ("<font color=\"red\"><b>" . _("MinGID is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$maxGID || !is_numeric($maxGID)) {
	echo ("<font color=\"red\"><b>" . _("MaxGID is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!isset($minMach) || !is_numeric($minMach)) {
	echo ("<font color=\"red\"><b>" . _("MinMachine is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$maxMach || !is_numeric($maxMach)) {
	echo ("<font color=\"red\"><b>" . _("MaxMachine is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$usrlstattr || !eregi("^((#[a-z]*)|([a-z]*:[a-z*]))(;((#[a-z]*)|([a-z]*:[a-z]*)))*$", $usrlstattr)) {
	echo ("<font color=\"red\"><b>" . _("User list attributes are invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$grplstattr || !eregi("^((#[a-z]*)|([a-z]*:[a-z*]))(;((#[a-z]*)|([a-z]*:[a-z]*)))*$", $grplstattr)) {
	echo ("<font color=\"red\"><b>" . _("Group list attributes are invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$hstlstattr || !eregi("^((#[a-z]*)|([a-z]*:[a-z*]))(;((#[a-z]*)|([a-z]*:[a-z]*)))*$", $hstlstattr)) {
	echo ("<font color=\"red\"><b>" . _("Host list attributes are invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$maxlistentries || !is_numeric($maxlistentries)) {
	echo ("<font color=\"red\"><b>" . _("Max list entries is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}

if (!$lang) {
	echo ("<font color=\"red\"><b>" . _("Language is not defined!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}

if (!$samba3) {
	echo ("<font color=\"red\"><b>" . _("Samba version is not defined!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}

if ($scriptpath && !eregi("^/[a-z0-9_\\-]+(/[a-z0-9_\\.\\-]+)+$", $scriptpath)) {
	echo ("<font color=\"red\"><b>" . _("Script path is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}

if ($scriptserver && !is_string($scriptserver)) {
	echo ("<font color=\"red\"><b>" . _("Script server is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}

// set new preferences
$conf->set_ServerURL($serverurl);
$conf->set_cacheTimeout($cachetimeout);
$conf->set_Adminstring($admins);
$conf->set_UserSuffix($suffusers);
$conf->set_GroupSuffix($suffgroups);
$conf->set_HostSuffix($suffhosts);
$conf->set_DomainSuffix($suffdomains);
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
$conf->set_defaultLanguage($lang);
$conf->set_samba3($samba3);
$conf->set_scriptpath($scriptpath);
$conf->set_scriptserver($scriptserver);
$conf->set_pwdhash($pwdhash);



// check if password was changed
if ($passwd1) {
	if ($passwd1 != $passwd2) {
		echo ("<b>" . _("Passwords are different!") . "</b>");
		exit;
	}
	// set new password
	if ($passwd1 != "") {
		$conf->set_Passwd($passwd1);
		echo ("<b>" . _("Password changed!") . "</b><br><br>");
	}
}

// save settings and display new settings
$conf->save();
echo ("<b>" . _("The following settings were saved to profile:") . " </b>" . $filename . "<br><br>");
$conf->printconf();
echo ("<br><br><br><br><br><a href=\"../login.php\" target=\"_top\">" . _("Back to Login") . "</a>");

echo("</body></html>");

// remove settings from session
unset($_SESSION['conf_passwd']);
unset($_SESSION['conf_passwd1']);
unset($_SESSION['conf_passwd2']);
unset($_SESSION['conf_serverurl']);
unset($_SESSION['conf_cachetimeout']);
unset($_SESSION['conf_admins']);
unset($_SESSION['conf_suffusers']);
unset($_SESSION['conf_suffgroups']);
unset($_SESSION['conf_suffhosts']);
unset($_SESSION['conf_suffdomains']);
unset($_SESSION['conf_minUID']);
unset($_SESSION['conf_maxUID']);
unset($_SESSION['conf_minGID']);
unset($_SESSION['conf_maxGID']);
unset($_SESSION['conf_minMach']);
unset($_SESSION['conf_maxMach']);
unset($_SESSION['conf_usrlstattr']);
unset($_SESSION['conf_grplstattr']);
unset($_SESSION['conf_hstlstattr']);
unset($_SESSION['conf_maxlistentries']);
unset($_SESSION['conf_lang']);
unset($_SESSION['conf_scriptpath']);
unset($_SESSION['conf_scriptserver']);
unset($_SESSION['conf_samba3']);
unset($_SESSION['conf_pwdhash']);
unset($_SESSION['conf_filename']);

?>
