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
$passwd = $_SESSION['conf_passwd'];
$passwd1 = $_SESSION['conf_passwd1'];
$passwd2 = $_SESSION['conf_passwd2'];
$serverurl = $_SESSION['conf_serverurl'];
$cachetimeout = $_SESSION['conf_cachetimeout'];
$admins = $_SESSION['conf_admins'];
$suffusers = $_SESSION['conf_suffusers'];
$suffgroups = $_SESSION['conf_suffgroups'];
$suffhosts = $_SESSION['conf_suffhosts'];
$suffdomains = $_SESSION['conf_suffdomains'];
$minUID = $_SESSION['conf_minUID'];
$maxUID = $_SESSION['conf_maxUID'];
$minGID = $_SESSION['conf_minGID'];
$maxGID = $_SESSION['conf_maxGID'];
$minMach = $_SESSION['conf_minMach'];
$maxMach = $_SESSION['conf_maxMach'];
$usrlstattr = $_SESSION['conf_usrlstattr'];
$grplstattr = $_SESSION['conf_grplstattr'];
$hstlstattr = $_SESSION['conf_hstlstattr'];
$maxlistentries = $_SESSION['conf_maxlistentries'];
$lang = $_SESSION['conf_lang'];
$scriptpath = $_SESSION['conf_scriptpath'];
$scriptserver = $_SESSION['conf_scriptserver'];
$samba3 = $_SESSION['conf_samba3'];
$pwdhash = $_SESSION['conf_pwdhash'];
$pdftext = $_SESSION['conf_pdf_usertext'];
$filename = $_SESSION['conf_filename'];

// check if password is correct
// if not: load login page
if ($passwd != $conf->get_Passwd()) {
	require('conflogin.php');
	exit;
}

echo $_SESSION['header'];

echo "<title>" . _("LDAP Account Manager Configuration") . "</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head><body>\n";

echo ("<p align=\"center\"><a href=\"http://lam.sf.net\" target=\"new_window\">".
	"<img src=\"../../graphics/banner.jpg\" border=1 alt=\"LDAP Account Manager\"></a></p><hr><br><br>");

// check new preferences
if (!$conf->set_samba3($samba3)) {
	echo ("<font color=\"red\"><b>" . _("Samba version is not defined!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}

if (!$conf->set_ServerURL($serverurl)) {
	echo ("<font color=\"red\"><b>" . _("Server Address is empty!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$conf->set_cacheTimeout($cachetimeout)) {
	echo ("<font color=\"red\"><b>" . _("Cache timeout is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$conf->set_Adminstring($admins)) {
	echo ("<font color=\"red\"><b>" . _("List of admin users is empty or invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$conf->set_UserSuffix($suffusers)) {
	echo ("<font color=\"red\"><b>" . _("UserSuffix is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$conf->set_GroupSuffix($suffgroups)) {
	echo ("<font color=\"red\"><b>" . _("GroupSuffix is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$conf->set_HostSuffix($suffhosts)) {
	echo ("<font color=\"red\"><b>" . _("HostSuffix is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$conf->set_DomainSuffix($suffdomains)) {
	echo ("<font color=\"red\"><b>" . _("DomainSuffix is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$conf->set_minUID($minUID)) {
	echo ("<font color=\"red\"><b>" . _("Minimum UID number is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$conf->set_maxUID($maxUID)) {
	echo ("<font color=\"red\"><b>" . _("Maximum UID number is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$conf->set_minGID($minGID)) {
	echo ("<font color=\"red\"><b>" . _("Minimum GID number is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$conf->set_maxGID($maxGID)) {
	echo ("<font color=\"red\"><b>" . _("Maximum GID number is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$conf->set_minMachine($minMach)) {
	echo ("<font color=\"red\"><b>" . _("Minimum Machine number is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$conf->set_maxMachine($maxMach)) {
	echo ("<font color=\"red\"><b>" . _("Maximum Machine number is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$conf->set_userlistAttributes($usrlstattr)) {
	echo ("<font color=\"red\"><b>" . _("User list attributes are invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$conf->set_grouplistAttributes($grplstattr)) {
	echo ("<font color=\"red\"><b>" . _("Group list attributes are invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$conf->set_hostlistAttributes($hstlstattr)) {
	echo ("<font color=\"red\"><b>" . _("Host list attributes are invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$conf->set_MaxListEntries($maxlistentries)) {
	echo ("<font color=\"red\"><b>" . _("Max list entries is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}

if (!$conf->set_defaultLanguage($lang)) {
	echo ("<font color=\"red\"><b>" . _("Language is not defined!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}

if (!$conf->set_scriptpath($scriptpath)) {
	echo ("<font color=\"red\"><b>" . _("Logon script is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}

if (!$conf->set_scriptserver($scriptserver)) {
	echo ("<font color=\"red\"><b>" . _("Script server is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}

if (!$conf->set_pwdhash($pwdhash)) {
	echo ("<font color=\"red\"><b>" . _("Password hash is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}

if (!$conf->set_pdftext($pdftext)) {
	echo ("<font color=\"red\"><b>" . _("Saving PDF text failed!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}


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
unset($_SESSION['conf_pdf_usertext']);
unset($_SESSION['conf_filename']);

?>
