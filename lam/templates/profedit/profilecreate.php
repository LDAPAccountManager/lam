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

  Saves new/modified profiles.

*/

include_once("../../lib/status.inc");
include_once("../../lib/account.inc");
include_once("../../lib/profiles.inc");
include_once("../../lib/ldap.inc");
include_once("../../lib/config.inc");

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

// abort button was pressed in profileuser/~host.php
// back to profile editor
if ($_POST['abort']) {
	metaRefresh("profilemain.php");
	exit;
}

// check if user is logged in, if not go to login
if (!$_SESSION['ldap'] || !$_SESSION['ldap']->server()) {
	metaRefresh("../login.php");
	exit;
}

// print header
echo $_SESSION['header'];
echo ("<html><head>\n<title></title>\n<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n</head><body><br>\n");

// save user profile
if ($_GET['type'] == "user") {
	$acct = new account();
	// check input
	if ($_POST['general_group'] && eregi("^[a-z]([a-z0-9_\\-])*$", $_POST['general_group'])) {
		$acct->general_group = $_POST['general_group'];
		}
	else {
		StatusMessage("ERROR", _("Primary group name is invalid!"), $_POST['general_group']);
		echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
		exit;
	}
	if ($_POST['general_groupadd']) {
		$acct->general_groupadd = $_POST['general_groupadd'];
	}
	if ($_POST['general_homedir'] && eregi("^[/]([a-z0-9])+([/][a-z0-9_\\-\\$]+)*$", $_POST['general_homedir'])) {
		$acct->general_homedir = $_POST['general_homedir'];
	}
	elseif ($_POST['general_homedir']) {
		StatusMessage("ERROR", _("Homedir is invalid!"), $_POST['general_homedir']);
		echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
		exit;
	}
	if ($_POST['general_shell'] && eregi("^[/]([a-z])+([/][a-z]+)*$", $_POST['general_shell'])) {
		$acct->general_shell = $_POST['general_shell'];
	}
	else {
		StatusMessage("ERROR", _("Shell is invalid!"), $_POST['general_shell']);
		echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
		exit;
	}
	if (($_POST['unix_password_no'] == "1") || ($_POST['unix_password_no'] == "0")) {
		$acct->unix_password_no = $_POST['unix_password_no'];
	}
	else {
		StatusMessage("ERROR", _("Wrong parameter for login disable!"), $_POST['unix_password_no']);
		echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
		exit;
	}
	if ($_POST['unix_pwdwarn'] && is_numeric($_POST['unix_pwdwarn'])) {
		$acct->unix_pwdwarn = $_POST['unix_pwdwarn'];
	}
	elseif ($_POST['unix_pwdwarn']) {
		StatusMessage("ERROR", _("Wrong parameter for Unix password warning!"), $_POST['unix_pwdwarn']);
		echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
		exit;
	}
	if ($_POST['unix_pwdminage'] && is_numeric($_POST['unix_pwdminage'])) {
		$acct->unix_pwdminage = $_POST['unix_pwdminage'];
	}
	elseif ($_POST['unix_pwdminage']) {
		StatusMessage("ERROR", _("Password minimum age is not numeric!"), $_POST['unix_pwdminage']);
		echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
		exit;
	}
	if ($_POST['unix_pwdmaxage'] && is_numeric($_POST['unix_pwdmaxage'])) {
		$acct->unix_pwdmaxage = $_POST['unix_pwdmaxage'];
	}
	elseif ($_POST['unix_pwdmaxage']) {
		StatusMessage("ERROR", _("Password maximum age is not numeric!"), $_POST['unix_pwdmaxage']);
		echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
		exit;
	}
	if (is_numeric($_POST['unix_pwdexpire_day']) && is_numeric($_POST['unix_pwdexpire_mon']) && is_numeric($_POST['unix_pwdexpire_yea'])) {
		$acct->unix_pwdexpire = mktime(0, 0, 0, $_POST['unix_pwdexpire_mon'], $_POST['unix_pwdexpire_day'], $_POST['unix_pwdexpire_yea']);
	}
	else {
		StatusMessage("ERROR", _("Wrong parameter for Unix password expiry!"));
		echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
		exit;
	}
	if ($_POST['unix_host'] && eregi("^[a-z0-9]+(,[a-z0-9]+)*$", $_POST['unix_host'])) {
		$acct->unix_host = $_POST['unix_host'];
	}
	elseif ($_POST['unix_host']) {
		StatusMessage("ERROR", _("Unix workstations are invalid!"), $_POST['unix_host']);
		echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
		exit;
	}
	if (($_POST['unix_deactivated'] == "1") || ($_POST['unix_deactivated'] == "0")) {
		$acct->unix_deactivated = $_POST['unix_deactivated'];
	}
	else {
		StatusMessage("ERROR", _("Wrong parameter for Unix account activation!"), $_POST['unix_deactivated']);
		echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
		exit;
	}
	if ($_POST['unix_pwdallowlogin'] && is_numeric($_POST['unix_pwdallowlogin'])) {
		$acct->unix_pwdallowlogin = $_POST['unix_pwdallowlogin'];
	}
	elseif ($_POST['unix_pwdallowlogin']) {
		StatusMessage("ERROR", _("Password expiry is not numeric!"), $_POST['unix_pwdallowlogin']);
		echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
		exit;
	}
	if (($_POST['smb_password_no'] == "1") || ($_POST['smb_password_no'] == "0")) {
		$acct->smb_password_no = $_POST['smb_password_no'];
	}
	else {
		StatusMessage("ERROR", _("Wrong parameter for Samba option: Set Samba Password!"), $_POST['smb_password_no']);
		echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
		exit;
	}
	if (($_POST['smb_useunixpwd'] == "1") || ($_POST['smb_useunixpwd'] == "0")) {
		$acct->smb_useunixpwd = $_POST['smb_useunixpwd'];
	}
	else {
		StatusMessage("ERROR", _("Wrong parameter for Samba option: Set Unix Password for Samba!"), $_POST['smb_useunixpwd']);
		echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
		exit;
	}
	if (($_POST['smb_flagsD'] == "1") || ($_POST['smb_flagsD'] == "0")) {
		$acct->smb_flagsD = $_POST['smb_flagsD'];
	}
	else {
		StatusMessage("ERROR", _("Wrong parameter for Samba option: Account does not expire!"), $_POST['smb_flagsD']);
		echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
		exit;
	}
	if ($_POST['smb_homedrive'] && ereg("^[D-Z]:$", $_POST['smb_homedrive'])) {
		$acct->smb_homedrive = $_POST['smb_homedrive'];
	}
	else {
		StatusMessage("ERROR", _("Wrong parameter for Samba option: home drive!"), $_POST['smb_homedrive']);
		echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
		exit;
	}
	// replace double \'s with \
	$_POST['smb_smbhome'] = str_replace('\\\\', '\\', $_POST['smb_smbhome']);
	if ($_POST['smb_smbhome'] && eregi("^[\][\]([a-z0-9])+([\][a-z0-9_\\-\\$%]+)+$", $_POST['smb_smbhome'])) {
		$acct->smb_smbhome = $_POST['smb_smbhome'];
	}
	elseif ($_POST['smb_smbhome']) {
		StatusMessage("ERROR", _("Samba home directory is invalid!"), $_POST['smb_smbhome']);
		echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
		exit;
	}
	// replace double \'s with \
	$_POST['smb_profilepath'] = str_replace('\\\\', '\\', $_POST['smb_profilepath']);
	if ($_POST['smb_profilepath'] && eregi("^[\][\]([a-z0-9])+([\][a-z0-9_\\-\\$%]+)+$", $_POST['smb_profilepath'])) {
		$acct->smb_profilePath = $_POST['smb_profilepath'];
	}
	elseif ($_POST['smb_profilepath']) {
		StatusMessage("ERROR", _("Profile path is invalid!"), $_POST['smb_profilepath']);
		echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
		exit;
	}
	// replace double \'s with \
	$_POST['smb_scriptPath'] = str_replace('\\\\', '\\', $_POST['smb_scriptPath']);
	if ($_POST['smb_scriptPath'] && eregi("^[\][\]([a-z0-9])+([\][a-z0-9_\\-\\$%.]+)+$", $_POST['smb_scriptPath'])) {
		$acct->smb_scriptPath = $_POST['smb_scriptPath'];
	}
	elseif ($_POST['smb_scriptPath']) {
		StatusMessage("ERROR", _("Script path is invalid!"), $_POST['smb_scriptPath']);
		echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
		exit;
	}
	if ($_POST['smb_smbuserworkstations'] && eregi("^[a-z0-9\\.\\-_]+( [a-z0-9\\.\\-_]+)*$", $_POST['smb_smbuserworkstations'])) {
		$acct->smb_smbuserworkstations = $_POST['smb_smbuserworkstations'];
	}
	elseif ($_POST['smb_smbuserworkstations']) {
		StatusMessage("ERROR", _("Samba workstations are invalid!"), $_POST['smb_smbuserworkstations']);
		echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
		exit;
	}
	if ($_POST['smb_domain'] && is_string($_POST['smb_domain'])) {
		$acct->smb_domain = $_POST['smb_domain'];
	}
	elseif ($_POST['smb_domain']) {
		StatusMessage("ERROR", _("Domain name is invalid!"), $_POST['smb_domain']);
		echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
		exit;
	}

	// check quota settings if script is given
	if ($_SESSION['config']->get_scriptPath()) {
		if ($_POST['quotacount'] && ($_POST['quotacount'] > 0)) {
			for ($i = 0; $i < $_POST['quotacount']; $i++) {
				$acct->quota[$i][0] = $_POST['f_quota_'.$i.'_0'];
				$acct->quota[$i][2] = $_POST['f_quota_'.$i.'_2'];
				$acct->quota[$i][3] = $_POST['f_quota_'.$i.'_3'];
				$acct->quota[$i][6] = $_POST['f_quota_'.$i.'_6'];
				$acct->quota[$i][7] = $_POST['f_quota_'.$i.'_7'];
				// Check if values are OK
				if (!ereg('^([0-9])*$', $acct->quota[$i][2])) {
					StatusMessage('ERROR', _('Block soft quota'), _('Block soft quota contains invalid characters. Only natural numbers are allowed'));
					echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
					exit;
				}
				if (!ereg('^([0-9])*$', $acct->quota[$i][3])) {
					StatusMessage('ERROR', _('Block hard quota'), _('Block hard quota contains invalid characters. Only natural numbers are allowed'));
					echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
					exit;
				}
				if (!ereg('^([0-9])*$', $acct->quota[$i][6])) {
					StatusMessage('ERROR', _('Inode soft quota'), _('Inode soft quota contains invalid characters. Only natural numbers are allowed'));
					echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
					exit;
				}
				if (!ereg('^([0-9])*$', $acct->quota[$i][7])) {
					StatusMessage('ERROR', _('Inode hard quota'), _('Inode hard quota contains invalid characters. Only natural numbers are allowed'));
					echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
					exit;
				}
			}
		}
	}

	if ($_POST['profname'] && eregi("^[0-9a-z\\-_]+$", $_POST['profname'])) {
		$profname = $_POST['profname'];
	}
	else {
		StatusMessage("ERROR", _("Invalid profile name!"), $_POST['profname']);
		echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
		exit;
	}

	// save profile
	if (saveUserProfile($acct, $profname)) {
		StatusMessage("INFO", _("Profile was saved."), $profname);
	}
	else StatusMessage("ERROR", _("Unable to save profile!"), $profname);

	echo ("<br><p><a href=\"profilemain.php\">" . _("Back to Profile Editor") . "</a></p>");
}


// save group profile
elseif ($_GET['type'] == "group") {
	$acct = new account();
	// check input
	if ($_POST['smb_domain'] && is_string($_POST['smb_domain'])) {
		$acct->smb_domain = $_POST['smb_domain'];
	}
	elseif ($_POST['smb_domain']) {
		StatusMessage("ERROR", _("Domain name is invalid!"), $_POST['smb_domain']);
		echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
		exit;
	}
	// check quota settings if script is given
	if ($_SESSION['config']->get_scriptPath()) {
		if ($_POST['quotacount'] && ($_POST['quotacount'] > 0)) {
			for ($i = 0; $i < $_POST['quotacount']; $i++) {
				$acct->quota[$i][0] = $_POST['f_quota_'.$i.'_0'];
				$acct->quota[$i][2] = $_POST['f_quota_'.$i.'_2'];
				$acct->quota[$i][3] = $_POST['f_quota_'.$i.'_3'];
				$acct->quota[$i][6] = $_POST['f_quota_'.$i.'_6'];
				$acct->quota[$i][7] = $_POST['f_quota_'.$i.'_7'];
				// Check if values are OK
				if (!ereg('^([0-9])*$', $acct->quota[$i][2])) {
					StatusMessage('ERROR', _('Block soft quota'), _('Block soft quota contains invalid characters. Only natural numbers are allowed'));
					echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
					exit;
				}
				if (!ereg('^([0-9])*$', $acct->quota[$i][3])) {
					StatusMessage('ERROR', _('Block hard quota'), _('Block hard quota contains invalid characters. Only natural numbers are allowed'));
					echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
					exit;
				}
				if (!ereg('^([0-9])*$', $acct->quota[$i][6])) {
					StatusMessage('ERROR', _('Inode soft quota'), _('Inode soft quota contains invalid characters. Only natural numbers are allowed'));
					echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
					exit;
				}
				if (!ereg('^([0-9])*$', $acct->quota[$i][7])) {
					StatusMessage('ERROR', _('Inode hard quota'), _('Inode hard quota contains invalid characters. Only natural numbers are allowed'));
					echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
					exit;
				}
			}
		}
	}

	if ($_POST['profname'] && eregi("^[0-9a-z\\-_]+$", $_POST['profname'])) {
		$profname = $_POST['profname'];
	}
	else {
		StatusMessage("ERROR", _("Invalid profile name!"), $_POST['profname']);
		echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
		exit;
	}

	// save profile
	if (savegroupProfile($acct, $profname)) {
		StatusMessage("INFO", _("Profile was saved."), $profname);
	}
	else StatusMessage("ERROR", _("Unable to save profile!"), $profname);

	echo ("<br><p><a href=\"profilemain.php\">" . _("Back to Profile Editor") . "</a></p>");
}


// save host profile
elseif ($_GET['type'] == "host") {
	$acct = new account();
	// check input
	if ($_POST['general_group'] && eregi("^[a-z]([a-z0-9_\\-])*$", $_POST['general_group'])) {
		$acct->general_group = $_POST['general_group'];
		}
	else {
		StatusMessage("ERROR", _("Primary group name is invalid!"), $_POST['general_group']);
		echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
		exit;
	}
	if ($_POST['smb_domain'] && is_string($_POST['smb_domain'])) {
		$acct->smb_domain = $_POST['smb_domain'];
	}
	elseif ($_POST['smb_domain']) {
		StatusMessage("ERROR", _("Domain name is invalid!"), $_POST['smb_domain']);
		echo ("<br><br><a href=\"javascript:history.back()\">" . _("Back to Profile Editor") . "</a>");
		exit;
	}
	// save profile
	if (saveHostProfile($acct, $profname)) {
		echo StatusMessage("INFO", _("Profile was saved."), $profname);
	}
	echo ("<br><p><a href=\"profilemain.php\">" . _("Back to Profile Editor") . "</a></p>");
}

// error: no or wrong type
else StatusMessage("ERROR", "", _("No type specified!"));

echo ("</body></html>\n");

?>
