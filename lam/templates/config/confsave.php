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

*/


/**
* confsave saves the new preferences to lam.conf
*
* @package configuration
* @author Roland Gruber
*/


/** Access to config functions */
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
$sufftree = $_SESSION['conf_sufftree'];
$usrlstattr = $_SESSION['conf_usrlstattr'];
$grplstattr = $_SESSION['conf_grplstattr'];
$hstlstattr = $_SESSION['conf_hstlstattr'];
$maxlistentries = $_SESSION['conf_maxlistentries'];
$lang = $_SESSION['conf_lang'];
$scriptpath = $_SESSION['conf_scriptpath'];
$scriptserver = $_SESSION['conf_scriptserver'];
$filename = $_SESSION['conf_filename'];

// check if password is correct
// if not: load login page
if ($passwd != $conf->get_Passwd()) {
	/** go back to login if password is invalid */
	require('conflogin.php');
	exit;
}

echo $_SESSION['header'];

echo "<title>" . _("LDAP Account Manager Configuration") . "</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head><body>\n";

echo ("<p align=\"center\"><a href=\"http://lam.sf.net\" target=\"new_window\">".
	"<img src=\"../../graphics/banner.jpg\" border=1 alt=\"LDAP Account Manager\"></a></p><hr><br><br>");

// remove double slashes if magic quotes are on
if (get_magic_quotes_gpc() == 1) {
	$suffusers = stripslashes($suffusers);
	$suffgroups = stripslashes($suffgroups);
	$suffhosts = stripslashes($suffhosts);
	$suffdomains = stripslashes($suffdomains);
}

// check new preferences
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
if (!$conf->set_Suffix('user', $suffusers)) {
	echo ("<font color=\"red\"><b>" . _("UserSuffix is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$conf->set_Suffix('group', $suffgroups)) {
	echo ("<font color=\"red\"><b>" . _("GroupSuffix is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$conf->set_Suffix('host', $suffhosts)) {
	echo ("<font color=\"red\"><b>" . _("HostSuffix is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$conf->set_Suffix('domain', $suffdomains)) {
	echo ("<font color=\"red\"><b>" . _("DomainSuffix is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
if (!$conf->set_Suffix("tree", $sufftree)) {
	echo ("<font color=\"red\"><b>" . _("TreeSuffix is invalid!") . "</b></font>");
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
	echo ("<font color=\"red\"><b>" . _("Script path is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}

if (!$conf->set_scriptserver($scriptserver)) {
	echo ("<font color=\"red\"><b>" . _("Script server is invalid!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}

if (! $conf->set_AccountModules($_SESSION['conf_usermodules'], 'user')) {
	echo ("<font color=\"red\"><b>" . _("Saving user modules failed!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}

if (! $conf->set_AccountModules($_SESSION['conf_groupmodules'], 'group')) {
	echo ("<font color=\"red\"><b>" . _("Saving group modules failed!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}

if (! $conf->set_AccountModules($_SESSION['conf_hostmodules'], 'host')) {
	echo ("<font color=\"red\"><b>" . _("Saving host modules failed!") . "</b></font>");
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}

// check module options
// create option array to check and save
$options = array();
$opt_keys = array_keys($_SESSION['config_types']);
foreach ($opt_keys as $element) {
	// text fields
	if ($_SESSION['config_types'][$element] == "text") {
		$options[$element] = array($_SESSION['config_moduleSettings'][$element]);
	}
	// checkboxes
	elseif ($_SESSION['config_types'][$element] == "checkbox") {
		if ($_SESSION['config_moduleSettings'][$element] == "on") $options[$element] = array('true');
		else $options[$element] = array('false');
	}
	// dropdownbox
	elseif ($_SESSION['config_types'][$element] == "select") {
		$options[$element] = array($_SESSION['config_moduleSettings'][$element]);
	}
	// multiselect
	elseif ($_SESSION['config_types'][$element] == "multiselect") {
		$options[$element] = $_SESSION['config_moduleSettings'][$element];  // value is already an array
	}
}

// remove double slashes if magic quotes are on
if (get_magic_quotes_gpc() == 1) {
	foreach ($opt_keys as $element) {
		if (is_string($options[$element][0])) $options[$element][0] = stripslashes($options[$element][0]);
	}
}

// check options
$errors = checkConfigOptions($_SESSION['config_scopes'], $options);
// print error messages if any
if (sizeof($errors) > 0) {
	for ($i = 0; $i < sizeof($errors); $i++) {
		if (sizeof($errors[$i]) > 3) {  // messages with additional variables
			StatusMessage($errors[$i][0], $errors[$i][1], $errors[$i][2], $errors[$i][3]);
		}
		else {
			StatusMessage($errors[$i][0], $errors[$i][1], $errors[$i][2]);
		}
	}
	echo ("\n<br><br><br><a href=\"javascript:history.back()\">" . _("Back to preferences...") . "</a>");
	exit;
}
// save module setting
$conf->set_moduleSettings($options);

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
unset($_SESSION['conf_sufftree']);
unset($_SESSION['conf_usrlstattr']);
unset($_SESSION['conf_grplstattr']);
unset($_SESSION['conf_hstlstattr']);
unset($_SESSION['conf_maxlistentries']);
unset($_SESSION['conf_lang']);
unset($_SESSION['conf_scriptpath']);
unset($_SESSION['conf_scriptserver']);
unset($_SESSION['conf_filename']);
unset($_SESSION['conf_usermodules']);
unset($_SESSION['conf_groupmodules']);
unset($_SESSION['conf_hostmodules']);

?>
