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

// profiles.inc provides functions to load and save profiles for users/groups/hosts

include_once("config.inc");
include_once("account.inc");


// returns an array of String with all available user profiles (without .pru)
function getUserProfiles() {
	$dir = dir(substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/users");
	$ret = array();
	$pos = 0;
	while ($entry = $dir->read()){
		$ext = substr($entry, strlen($entry)-4, 4);
		$name = substr($entry, 0, strlen($entry)-4);
		if ($ext == ".pru") {
			$ret[$pos] = $name;
			$pos ++;
		}
	}
	return $ret;
}

// returns an array of String with all available group profiles (without .prg)
function getGroupProfiles() {
	$dir = dir(substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/groups");
	$ret = array();
	$pos = 0;
	while ($entry = $dir->read()){
		$ext = substr($entry, strlen($entry)-4, 4);
		$name = substr($entry, 0, strlen($entry)-4);
		if ($ext == ".prg") {
			$ret[$pos] = $name;
			$pos ++;
		}
	}
	return $ret;
}

// returns an array of String with all available host profiles (without .prh)
function getHostProfiles() {
	$dir = dir(substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/hosts");
	$ret = array();
	$pos = 0;
	while ($entry = $dir->read()){
		$ext = substr($entry, strlen($entry)-4, 4);
		$name = substr($entry, 0, strlen($entry)-4);
		if ($ext == ".prh") {
			$ret[$pos] = $name;
			$pos ++;
		}
	}
	return $ret;
}

// loads an user profile with name $profile (without .pru)
// the return value is an account object
function loadUserProfile($profile) {
	if (!eregi("^([0-9]|[a-z]|-|_)*$", $profile)) exit;
	$acc = new account();
	$file = substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/users/" . $profile . ".pru";
	if (is_file($file) == True) {
		$file = fopen($file, "r");
		while (!feof($file)) {
			$line = fgets($file, 1024);
			if (($line == "\n")||($line[0] == "#")) continue; // ignore comments
			// search keywords
			if (substr($line, 0, 15) == "general_group: ") {
				$acc->general_group = chop(substr($line, 15, strlen($line)-15));
				continue;
			}
			if (substr($line, 0, 18) == "general_groupadd: ") {
				$acc->general_groupadd = chop(substr($line, 18, strlen($line)-18));
				continue;
			}
			if (substr($line, 0, 17) == "general_homedir: ") {
				$acc->general_homedir = chop(substr($line, 17, strlen($line)-17));
				continue;
			}
			if (substr($line, 0, 15) == "general_shell: ") {
				$acc->general_shell = chop(substr($line, 15, strlen($line)-15));
				continue;
			}
			if (substr($line, 0, 18) == "unix_password_no: ") {
				$acc->unix_password_no = chop(substr($line, 18, strlen($line)-18));
				continue;
			}
			if (substr($line, 0, 14) == "unix_pwdwarn: ") {
				$acc->unix_pwdwarn = chop(substr($line, 14, strlen($line)-14));
				continue;
			}
			if (substr($line, 0, 20) == "unix_pwdallowlogin: ") {
				$acc->unix_pwdallowlogin = chop(substr($line, 20, strlen($line)-20));
				continue;
			}
			if (substr($line, 0, 16) == "unix_pwdminage: ") {
				$acc->unix_pwdminage = chop(substr($line, 16, strlen($line)-16));
				continue;
			}
			if (substr($line, 0, 16) == "unix_pwdmaxage: ") {
				$acc->unix_pwdmaxage = chop(substr($line, 16, strlen($line)-16));
				continue;
			}
			if (substr($line, 0, 20) == "unix_pwdexpire_day: ") {
				$acc->unix_pwdexpire_day = chop(substr($line, 20, strlen($line)-20));
				continue;
			}
			if (substr($line, 0, 20) == "unix_pwdexpire_mon: ") {
				$acc->unix_pwdexpire_mon = chop(substr($line, 20, strlen($line)-20));
				continue;
			}
			if (substr($line, 0, 20) == "unix_pwdexpire_yea: ") {
				$acc->unix_pwdexpire_yea = chop(substr($line, 20, strlen($line)-20));
				continue;
			}
			if (substr($line, 0, 18) == "unix_deactivated: ") {
				$acc->unix_deactivated = chop(substr($line, 18, strlen($line)-18));
				continue;
			}
			if (substr($line, 0, 17) == "smb_password_no: ") {
				$acc->smb_password_no = chop(substr($line, 17, strlen($line)-17));
				continue;
			}
			if (substr($line, 0, 16) == "smb_useunixpwd: ") {
				$acc->smb_useunixpwd = chop(substr($line, 16, strlen($line)-16));
				continue;
			}
			if (substr($line, 0, 18) == "smb_pwdcanchange: ") {
				$acc->smb_pwdcanchange = chop(substr($line, 18, strlen($line)-18));
				continue;
			}
			if (substr($line, 0, 19) == "smb_pwdmustchange: ") {
				$acc->smb_pwdmustchange = chop(substr($line, 19, strlen($line)-19));
				continue;
			}
			if (substr($line, 0, 15) == "smb_homedrive: ") {
				$acc->smb_homedrive = chop(substr($line, 15, strlen($line)-15));
				continue;
			}
			if (substr($line, 0, 16) == "smb_scriptpath: ") {
				$acc->smb_scriptpath = chop(substr($line, 16, strlen($line)-16));
				continue;
			}
			if (substr($line, 0, 25) == "smb_smbuserworkstations: ") {
				$acc->smb_smbuserworkstations = chop(substr($line, 25, strlen($line)-25));
				continue;
			}
			if (substr($line, 0, 13) == "smb_smbhome: ") {
				$acc->smb_smbhome = chop(substr($line, 13, strlen($line)-13));
				continue;
			}
			if (substr($line, 0, 12) == "smb_domain: ") {
				$acc->smb_domain = chop(substr($line, 12, strlen($line)-12));
				continue;
			}
		}
		fclose($file);
	}
	else {
		echo _("Unable to load profile! ") . $file ; echo "<br>";
	}
	return $acc;
}

// loads an group profile with name $profile (without .prg)
// the return value is an account object
function loadGroupProfile($profile) {
	// no group profiles yet
}

// loads an host profile with name $profile (without .prh)
// the return value is an account object
function loadHostProfile($profile) {
	if (!eregi("^([0-9]|[a-z]|-|_)*$", $profile)) exit;
	$acc = new account();
	$file = substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/hosts/" . $profile . ".prh";
	if (is_file($file) == True) {
		$file = fopen($file, "r");
		while (!feof($file)) {
			$line = fgets($file, 1024);
			if (($line == "\n")||($line[0] == "#")) continue; // ignore comments
			// search keywords
			if (substr($line, 0, 15) == "general_group: ") {
				$acc->general_group = chop(substr($line, 15, strlen($line)-15));
				continue;
			}
			if (substr($line, 0, 18) == "general_groupadd: ") {
				$acc->general_groupadd = chop(substr($line, 18, strlen($line)-18));
				continue;
			}
			if (substr($line, 0, 18) == "unix_password_no: ") {
				$acc->unix_password_no = chop(substr($line, 18, strlen($line)-18));
				continue;
			}
			if (substr($line, 0, 14) == "unix_pwdwarn: ") {
				$acc->unix_pwdwarn = chop(substr($line, 14, strlen($line)-14));
				continue;
			}
			if (substr($line, 0, 20) == "unix_pwdallowlogin: ") {
				$acc->unix_pwdallowlogin = chop(substr($line, 20, strlen($line)-20));
				continue;
			}
			if (substr($line, 0, 16) == "unix_pwdminage: ") {
				$acc->unix_pwdminage = chop(substr($line, 16, strlen($line)-16));
				continue;
			}
			if (substr($line, 0, 16) == "unix_pwdmaxage: ") {
				$acc->unix_pwdmaxage = chop(substr($line, 16, strlen($line)-16));
				continue;
			}
			if (substr($line, 0, 20) == "unix_pwdexpire_day: ") {
				$acc->unix_pwdexpire_day = chop(substr($line, 20, strlen($line)-20));
				continue;
			}
			if (substr($line, 0, 20) == "unix_pwdexpire_mon: ") {
				$acc->unix_pwdexpire_mon = chop(substr($line, 20, strlen($line)-20));
				continue;
			}
			if (substr($line, 0, 20) == "unix_pwdexpire_yea: ") {
				$acc->unix_pwdexpire_yea = chop(substr($line, 20, strlen($line)-20));
				continue;
			}
			if (substr($line, 0, 18) == "unix_deactivated: ") {
				$acc->unix_deactivated = chop(substr($line, 18, strlen($line)-18));
				continue;
			}
			if (substr($line, 0, 17) == "smb_password_no: ") {
				$acc->smb_password_no = chop(substr($line, 17, strlen($line)-17));
				continue;
			}
			if (substr($line, 0, 16) == "smb_useunixpwd: ") {
				$acc->smb_useunixpwd = chop(substr($line, 16, strlen($line)-16));
				continue;
			}
			if (substr($line, 0, 18) == "smb_pwdcanchange: ") {
				$acc->smb_pwdcanchange = chop(substr($line, 18, strlen($line)-18));
				continue;
			}
			if (substr($line, 0, 19) == "smb_pwdmustchange: ") {
				$acc->smb_pwdmustchange = chop(substr($line, 19, strlen($line)-19));
				continue;
			}
			if (substr($line, 0, 12) == "smb_domain: ") {
				$acc->smb_domain = chop(substr($line, 12, strlen($line)-12));
				continue;
			}
		}
		fclose($file);
	}
	else {
		echo _("Unable to load profile! ") . $file ; echo "<br>";
	}
	return $acc;
}

// saves an account object to an user profile with name $profile (without .pru)
// file is created, if needed
function saveUserProfile($account, $profile) {
	if (!eregi("^([0-9]|[a-z]|-|_)*$", $profile)) exit;
	if (!is_object($account)) {echo _("saveUserProfile: \$account has wrong type!"); exit;}
	$path = substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/users/" . $profile . ".pru";
	$file = fopen($path, "w");
	// write attributes
	if ($account->general_group) fputs($file, "general_group: " . $account->general_group . "\n");
	if ($account->general_groupadd) fputs($file, "general_groupadd: " . $account->general_groupadd . "\n");
	if ($account->general_homedir) fputs($file, "general_homedir: " . $account->general_homedir . "\n");
	if ($account->general_shell) fputs($file, "general_shell: " . $account->general_shell . "\n");
	if ($account->unix_password_no) fputs($file, "unix_password_no: " . $account->unix_password_no . "\n");
	if ($account->unix_pwdwarn) fputs($file, "unix_pwdwarn: " . $account->unix_pwdwarn . "\n");
	if ($account->unix_pwdallowlogin) fputs($file, "unix_pwdallowlogin: " . $account->unix_pwdallowlogin . "\n");
	if ($account->unix_pwdminage) fputs($file, "unix_pwdminage: " . $account->unix_pwdminage . "\n");
	if ($account->unix_pwdmaxage) fputs($file, "unix_pwdmaxage: " . $account->unix_pwdmaxage . "\n");
	if ($account->unix_pwdexpire_day) fputs($file, "unix_pwdexpire_day: " . $account->unix_pwdexpire_day . "\n");
	if ($account->unix_pwdexpire_mon) fputs($file, "unix_pwdexpire_mon: " . $account->unix_pwdexpire_mon . "\n");
	if ($account->unix_pwdexpire_yea) fputs($file, "unix_pwdexpire_yea: " . $account->unix_pwdexpire_yea . "\n");
	if ($account->unix_deactivated) fputs($file, "unix_deactivated: " . $account->unix_deactivated . "\n");
	if ($account->smb_password_no) fputs($file, "smb_password_no: " . $account->smb_password_no . "\n");
	if ($account->smb_useunixpwd) fputs($file, "smb_useunixpwd: " . $account->smb_useunixpwd . "\n");
	if ($account->smb_pwdcanchange) fputs($file, "smb_pwdcanchange: " . $account->smb_pwdcanchange . "\n");
	if ($account->smb_pwdmustchange) fputs($file, "smb_pwdmustchange: " . $account->smb_pwdmustchange . "\n");
	if ($account->smb_homedrive) fputs($file, "smb_homedrive: " . $account->smb_homedrive . "\n");
	if ($account->smb_scriptpath) fputs($file, "smb_scriptpath: " . $account->smb_scriptpath . "\n");
	if ($account->smb_smbuserworkstations) fputs($file, "smb_smbuserworkstations: " . $account->smb_smbuserworkstations . "\n");
	if ($account->smb_smbhome) fputs($file, "smb_smbhome: " . $account->smb_smbhome . "\n");
	if ($account->smb_domain) fputs($file, "smb_domain: " . $account->smb_domain . "\n");
	// close file
	fclose($file);
}

// saves an account object to an group profile with name $profile (without .prg)
// file is created, if needed
function saveGroupProfile($account, $profile) {
	// no group profiles yet
}

// saves an account object to an host profile with name $profile (without .prh)
// file is created, if needed
function saveHostProfile($account, $profile) {
	if (!eregi("^([0-9]|[a-z]|-|_)*$", $profile)) exit;
	if (!is_object($account)) {echo _("saveHostProfile: \$account has wrong type!"); exit;}
	$path = substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/hosts/" . $profile . ".prh";
	$file = fopen($path, "w");
	// write attributes
	if ($account->general_group) fputs($file, "general_group: " . $account->general_group . "\n");
	if ($account->general_groupadd) fputs($file, "general_groupadd: " . $account->general_groupadd . "\n");
	if ($account->unix_password_no) fputs($file, "unix_password_no: " . $account->unix_password_no . "\n");
	if ($account->unix_pwdwarn) fputs($file, "unix_pwdwarn: " . $account->unix_pwdwarn . "\n");
	if ($account->unix_pwdallowlogin) fputs($file, "unix_pwdallowlogin: " . $account->unix_pwdallowlogin . "\n");
	if ($account->unix_pwdminage) fputs($file, "unix_pwdminage: " . $account->unix_pwdminage . "\n");
	if ($account->unix_pwdmaxage) fputs($file, "unix_pwdmaxage: " . $account->unix_pwdmaxage . "\n");
	if ($account->unix_pwdexpire_day) fputs($file, "unix_pwdexpire_day: " . $account->unix_pwdexpire_day . "\n");
	if ($account->unix_pwdexpire_mon) fputs($file, "unix_pwdexpire_mon: " . $account->unix_pwdexpire_mon . "\n");
	if ($account->unix_pwdexpire_yea) fputs($file, "unix_pwdexpire_yea: " . $account->unix_pwdexpire_yea . "\n");
	if ($account->unix_deactivated) fputs($file, "unix_deactivated: " . $account->unix_deactivated . "\n");
	if ($account->smb_password_no) fputs($file, "smb_password_no: " . $account->smb_password_no . "\n");
	if ($account->smb_useunixpwd) fputs($file, "smb_useunixpwd: " . $account->smb_useunixpwd . "\n");
	if ($account->smb_pwdcanchange) fputs($file, "smb_pwdcanchange: " . $account->smb_pwdcanchange . "\n");
	if ($account->smb_pwdmustchange) fputs($file, "smb_pwdmustchange: " . $account->smb_pwdmustchange . "\n");
	if ($account->smb_domain) fputs($file, "smb_domain: " . $account->smb_domain . "\n");
	// close file
	fclose($file);
}

?>
