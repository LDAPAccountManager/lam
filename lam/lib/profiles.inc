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
include_once("ldap.inc");


// returns an array of String with all available user profiles (without .pru)
function getUserProfiles() {
	$dir = @dir(substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/users");
	$ret = array();
	$pos = 0;
	if ($dir) {
		while ($entry = $dir->read()){
			$ext = substr($entry, strlen($entry)-4, 4);
			$name = substr($entry, 0, strlen($entry)-4);
			if ($ext == ".pru") {
				$ret[$pos] = $name;
				$pos ++;
			}
		}
	}
	return $ret;
}

// returns an array of String with all available group profiles (without .prg)
function getGroupProfiles() {
	$dir = @dir(substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/groups");
	$ret = array();
	$pos = 0;
	if ($dir) {
		while ($entry = $dir->read()){
			$ext = substr($entry, strlen($entry)-4, 4);
			$name = substr($entry, 0, strlen($entry)-4);
			if ($ext == ".prg") {
				$ret[$pos] = $name;
				$pos ++;
			}
		}
	}
	return $ret;
}

// returns an array of String with all available host profiles (without .prh)
function getHostProfiles() {
	$dir = @dir(substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/hosts");
	$ret = array();
	$pos = 0;
	if ($dir) {
		while ($entry = $dir->read()){
			$ext = substr($entry, strlen($entry)-4, 4);
			$name = substr($entry, 0, strlen($entry)-4);
			if ($ext == ".prh") {
				$ret[$pos] = $name;
				$pos ++;
			}
		}
	}
	return $ret;
}

// loads an user profile with name $profile (without .pru)
// the return value is an account object
// if $timestamps is true, smb_pwdcanchange and smb_pwdmustchange are returned as timestamp
function loadUserProfile($profile) {
	if (!eregi("^([0-9]|[a-z]|-|_)*$", $profile)) exit;
	$acc = new account();
	$file = substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/users/" . $profile . ".pru";
	if (is_file($file) == True) {
		$file = @fopen($file, "r");
		if ($file) {
			while (!feof($file)) {
				$line = fgets($file, 1024);
				if (($line == "\n")||($line[0] == "#")) continue; // ignore comments
				// search keywords
				if (substr($line, 0, 15) == "general_group: ") {
					$acc->general_group = chop(substr($line, 15, strlen($line)-15));
					continue;
				}
				if (substr($line, 0, 18) == "general_groupadd: ") {
					$acc->general_groupadd = explode(";", chop(substr($line, 18, strlen($line)-18)));
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
				if (substr($line, 0, 16) == "unix_pwdexpire: ") {
					$acc->unix_pwdexpire = chop(substr($line, 16, strlen($line)-16));
					continue;
				}
				if (substr($line, 0, 18) == "unix_deactivated: ") {
					$acc->unix_deactivated = chop(substr($line, 18, strlen($line)-18));
					continue;
				}
				if (substr($line, 0, 11) == "unix_host: ") {
					$acc->unix_host = chop(substr($line, 11, strlen($line)-11));
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
				if (substr($line, 0, 12) == "smb_flagsD: ") {
					$acc->smb_flagsD = chop(substr($line, 12, strlen($line)-12));
					continue;
				}
				if (substr($line, 0, 12) == "smb_flagsX: ") {
					$acc->smb_flagsX = chop(substr($line, 12, strlen($line)-12));
					continue;
				}
				if (substr($line, 0, 15) == "smb_homedrive: ") {
					$acc->smb_homedrive = chop(substr($line, 15, strlen($line)-15));
					continue;
				}
				if (substr($line, 0, 16) == "smb_scriptPath: ") {
					$acc->smb_scriptPath = chop(substr($line, 16, strlen($line)-16));
					continue;
				}
				if (substr($line, 0, 17) == "smb_profilePath: ") {
					$acc->smb_profilePath = chop(substr($line, 17, strlen($line)-17));
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
					if ($_SESSION['config']->is_samba3()) {
						$dn = chop(substr($line, 12, strlen($line)-12));
						if (! stristr($dn, "sambaDomainName=")) {  // Samba 3 and Samba 2 profile
							// get domain list
							$doms = $_SESSION['ldap']->search_domains($_SESSION['config']->get_domainSuffix());
							// search domain name
							for ($d = 0; $d < sizeof($doms); $d++) {
								if (strtolower($doms[$d]->name) == strtolower($dn)) {
									$acc->smb_domain = $doms[$d];
									break;
								}
							}
						}
						else {  // Samba 3 and Samba 3 profile
							// load domain object
							$dom = new samba3domain();
							$sr = @ldap_search($_SESSION['ldap']->server, $dn, "objectClass=sambaDomain");
							if ($sr) {
								$info = @ldap_get_entries($_SESSION['ldap']->server, $sr);
								if ($info) {
									// get domain attributes
									@array_shift($info);
									$dom->dn = $dn;
									$dom->name = $info[0]['sambadomainname'][0];
									$dom->SID = $info[0]['sambasid'][0];
									$dom->nextRID = $info[0]['sambanextrid'][0];
									$dom->nextUserRID = $info[0]['sambanextuserrid'][0];
									$dom->nextGroupRID = $info[0]['sambanextgrouprid'][0];
									if (isset($dom->RIDbase)) $dom->RIDbase = $info[0]['sambaalgorithmicridbase'][0];
								}
							}
							$acc->smb_domain = $dom;
						}
					}
					else {
						$dom_name =  chop(substr($line, 12, strlen($line)-12));
						if (substr($dom_name, 0, 16) == "sambaDomainName=") {  // Samba 2 and Samba 3 profile
							// extract domain name from DN
							$dom_name = substr($dom_name, 16, strlen($dom_name) - 16);
							$pos = strpos($dom_name, ",");
							$dom_name = substr($dom_name, 0, $pos);
							$acc->smb_domain = $dom_name;
						}
						else {  // Samba 2 and Samba 2 profile
							$acc->smb_domain = $dom_name;
						}
					}
					continue;
				}
				if (substr($line, 0, 7) == "quota: ") {
					$acc->quota = chop(substr($line, 7, strlen($line)-7));
					// split mountpoints
					$acc->quota = explode(";", $acc->quota);
					// split attributes
					for ($i = 0; $i < sizeof($acc->quota); $i++) {
						$temp = explode(",", $acc->quota[$i]);
						$acc->quota[$i] = array();
						$acc->quota[$i][0] = $temp[0];
						$acc->quota[$i][2] = $temp[1];
						$acc->quota[$i][3] = $temp[2];
						$acc->quota[$i][6] = $temp[3];
						$acc->quota[$i][7] = $temp[4];
					}
				}
			}
			fclose($file);
		}
		else {
			StatusMessage("ERROR", "", _("Unable to load profile!") . " " . $file);
		}
	}
	else {
		StatusMessage("ERROR", "", _("Unable to load profile!") . " " . $file);
	}
	return $acc;
}

// loads an group profile with name $profile (without .prg)
// the return value is an account object
function loadGroupProfile($profile) {
	if (!eregi("^([0-9]|[a-z]|-|_)*$", $profile)) exit;
	$acc = new account();
	$file = substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/groups/" . $profile . ".prg";
	if (is_file($file) == True) {
		$file = @fopen($file, "r");
		if ($file) {
			while (!feof($file)) {
				$line = fgets($file, 1024);
				if (($line == "\n")||($line[0] == "#")) continue; // ignore comments
				// search keywords
				if (substr($line, 0, 12) == "smb_domain: ") {
					if ($_SESSION['config']->is_samba3()) {
						$dn = chop(substr($line, 12, strlen($line)-12));
						// load domain object
						$dom = new samba3domain();
						$sr = @ldap_search($_SESSION['ldap']->server, $dn, "objectClass=sambaDomain");
						if ($sr) {
							$info = @ldap_get_entries($_SESSION['ldap']->server, $sr);
							if ($info) {
								// get domain attributes
								@array_shift($info);
								$dom->dn = $dn;
								$dom->name = $info[0]['sambadomainname'][0];
								$dom->SID = $info[0]['sambasid'][0];
								$dom->nextRID = $info[0]['sambanextrid'][0];
								$dom->nextUserRID = $info[0]['sambanextuserrid'][0];
								$dom->nextGroupRID = $info[0]['sambanextgrouprid'][0];
								if (isset($dom->RIDbase)) $dom->RIDbase = $info[0]['sambaalgorithmicridbase'][0];
							}
						}
						$acc->smb_domain = $dom;
					}
				}
				if (substr($line, 0, 7) == "quota: ") {
					if ($_SESSION['config']->get_scriptPath()) {
						$acc->quota = chop(substr($line, 7, strlen($line)-7));
						// split mountpoints
						$acc->quota = explode(";", $acc->quota);
						// split attributes
						for ($i = 0; $i < sizeof($acc->quota); $i++) {
							$temp = explode(",", $acc->quota[$i]);
							$acc->quota[$i] = array();
							$acc->quota[$i][0] = $temp[0];
							$acc->quota[$i][2] = $temp[1];
							$acc->quota[$i][3] = $temp[2];
							$acc->quota[$i][6] = $temp[3];
							$acc->quota[$i][7] = $temp[4];
						}
					}
				}
			}
			fclose($file);
		}
		else {
			StatusMessage("ERROR", "", _("Unable to load profile!") . " " . $file);
		}
	}
	else {
		StatusMessage("ERROR", "", _("Unable to load profile!") . " " . $file);
	}
	return $acc;
}

// loads an host profile with name $profile (without .prh)
// the return value is an account object
function loadHostProfile($profile) {
	if (!eregi("^([0-9]|[a-z]|-|_)*$", $profile)) exit;
	$acc = new account();
	$file = substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/hosts/" . $profile . ".prh";
	if (is_file($file) == True) {
		$file = @fopen($file, "r");
		if ($file) {
			while (!feof($file)) {
				$line = fgets($file, 1024);
				if (($line == "\n")||($line[0] == "#")) continue; // ignore comments
				// search keywords
				if (substr($line, 0, 15) == "general_group: ") {
					$acc->general_group = chop(substr($line, 15, strlen($line)-15));
					continue;
				}
				if (substr($line, 0, 12) == "smb_domain: ") {
					if ($_SESSION['config']->is_samba3()) {
						$dn = chop(substr($line, 12, strlen($line)-12));
						// load domain object
						$dom = new samba3domain();
						$sr = @ldap_search($_SESSION['ldap']->server, $dn, "objectClass=sambaDomain");
						if ($sr) {
							$info = @ldap_get_entries($_SESSION['ldap']->server, $sr);
							if ($info) {
								// get domain attributes
								@array_shift($info);
								$dom->dn = $dn;
								$dom->name = $info[0]['sambadomainname'][0];
								$dom->SID = $info[0]['sambasid'][0];
								$dom->nextRID = $info[0]['sambanextrid'][0];
								$dom->nextUserRID = $info[0]['sambanextuserrid'][0];
								$dom->nextGroupRID = $info[0]['sambanextgrouprid'][0];
								if (isset($dom->RIDbase)) $dom->RIDbase = $info[0]['sambaalgorithmicridbase'][0];
							}
						}
						$acc->smb_domain = $dom;
					}
					else $acc->smb_domain = chop(substr($line, 12, strlen($line)-12));
					continue;
				}
			}
			fclose($file);
		}
		else {
			StatusMessage("ERROR", "", _("Unable to load profile!") . " " . $file);
		}
	}
	else {
		StatusMessage("ERROR", "", _("Unable to load profile!") . " " . $file);
	}
	return $acc;
}

// saves an account object to an user profile with name $profile (without .pru)
// file is created, if needed
function saveUserProfile($account, $profile) {
	if (!eregi("^([0-9]|[a-z]|-|_)*$", $profile)) exit;
	if (!is_object($account)) {
		StatusMessage("ERROR", "", _("saveUserProfile: account has wrong type!"));
		exit;
	}
	$path = substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/users/" . $profile . ".pru";
	$file = @fopen($path, "w");
	if ($file) {
		// replace user and group names
		$homedir = $account->general_homedir;
		$profpath = $account->smb_profilePath;
		$scriptPath = $account->smb_scriptPath;
		$smbhome = $account->smb_smbhome;
		if ($account->general_username) {
			$homedir = str_replace($account->general_username, "\$user", $homedir);
			$profpath = str_replace($account->general_username, "\$user", $profpath);
			$scriptPath = str_replace($account->general_username, "\$user", $scriptPath);
			$smbhome = str_replace($account->general_username, "\$user", $smbhome);
		}
		if ($account->general_group) {
			$homedir = str_replace($account->general_group, "\$group", $homedir);
			$profpath = str_replace($account->general_group, "\$group", $profpath);
			$scriptPath = str_replace($account->general_group, "\$group", $scriptPath);
			$smbhome = str_replace($account->general_group, "\$group", $smbhome);
		}
		// write attributes
		if (isset($account->general_group)) fputs($file, "general_group: " . $account->general_group . "\n");
		if (isset($account->general_groupadd)) fputs($file, "general_groupadd: " . implode(";", $account->general_groupadd) . "\n");
		if (isset($account->general_homedir)) fputs($file, "general_homedir: " . $homedir . "\n");
		if (isset($account->general_shell)) fputs($file, "general_shell: " . $account->general_shell . "\n");
		if (isset($account->unix_password_no)) fputs($file, "unix_password_no: " . $account->unix_password_no . "\n");
		if (isset($account->unix_pwdwarn)) fputs($file, "unix_pwdwarn: " . $account->unix_pwdwarn . "\n");
		if (isset($account->unix_pwdallowlogin)) fputs($file, "unix_pwdallowlogin: " . $account->unix_pwdallowlogin . "\n");
		if (isset($account->unix_pwdminage)) fputs($file, "unix_pwdminage: " . $account->unix_pwdminage . "\n");
		if (isset($account->unix_pwdmaxage)) fputs($file, "unix_pwdmaxage: " . $account->unix_pwdmaxage . "\n");
		if (isset($account->unix_pwdexpire)) fputs($file, "unix_pwdexpire: " . $account->unix_pwdexpire . "\n");
		if (isset($account->unix_deactivated)) fputs($file, "unix_deactivated: " . $account->unix_deactivated . "\n");
		if (isset($account->unix_host)) fputs($file, "unix_host: " . $account->unix_host . "\n");
		if (isset($account->smb_password_no)) fputs($file, "smb_password_no: " . $account->smb_password_no . "\n");
		if (isset($account->smb_useunixpwd)) fputs($file, "smb_useunixpwd: " . $account->smb_useunixpwd . "\n");
		if (isset($account->smb_flagsD)) fputs($file, "smb_flagsD: " . $account->smb_flagsD . "\n");
		if (isset($account->smb_flagsX)) fputs($file, "smb_flagsX: " . $account->smb_flagsX . "\n");
		if (isset($account->smb_homedrive)) fputs($file, "smb_homedrive: " . $account->smb_homedrive . "\n");
		if (isset($account->smb_scriptPath)) fputs($file, "smb_scriptPath: " . $scriptPath . "\n");
		if (isset($account->smb_profilePath)) fputs($file, "smb_profilePath: " . $profpath . "\n");
		if (isset($account->smb_smbuserworkstations)) fputs($file, "smb_smbuserworkstations: " . $account->smb_smbuserworkstations . "\n");
		if (isset($account->smb_smbhome)) fputs($file, "smb_smbhome: " . $smbhome . "\n");
		if (isset($account->smb_domain)) fputs($file, "smb_domain: " . $account->smb_domain . "\n");
		if (isset($account->quota)) {
			// convert array to string
			for ($i = 0; $i < sizeof($account->quota); $i++) {
				$account->quota[$i] = implode(",", $account->quota[$i]);
			}
			$temp = implode(";", $account->quota);
			fputs($file, "quota: " . $temp . "\n");
		}
		// close file
		fclose($file);
	}
	else {
		StatusMessage("ERROR", _("Unable to save profile!"), $path);
		return false;
	}
	return true;
}

// saves an account object to an group profile with name $profile (without .prg)
// file is created, if needed
function saveGroupProfile($account, $profile) {
	if (!eregi("^([0-9]|[a-z]|-|_)*$", $profile)) exit;
	if (!is_object($account)) {
		StatusMessage("ERROR", "", _("saveGroupProfile: account has wrong type!"));
		exit;
	}
	$path = substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/groups/" . $profile . ".prg";
	$file = @fopen($path, "w");
	if ($file) {
		// write attributes
		if (isset($account->smb_domain)) fputs($file, "smb_domain: " . $account->smb_domain . "\n");
		if (isset($account->quota)) {
			// convert array to string
			for ($i = 0; $i < sizeof($account->quota); $i++) {
				$account->quota[$i] = implode(",", $account->quota[$i]);
			}
			$temp = implode(";", $account->quota);
			fputs($file, "quota: " . $temp . "\n");
		}
		// close file
		fclose($file);
	}
	else {
		StatusMessage("ERROR", _("Unable to save profile!"), $path);
		return false;
	}
	return true;
}

// saves an account object to an host profile with name $profile (without .prh)
// file is created, if needed
function saveHostProfile($account, $profile) {
	if (!eregi("^[0-9a-z\\-_]+$", $profile)) exit;
	if (!is_object($account)) {
		StatusMessage ("ERROR", "", _("saveHostProfile: account has wrong type!"));
		exit;
	}
	$path = substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/hosts/" . $profile . ".prh";
	$file = @fopen($path, "w");
	if ($file) {
		// write attributes
		if (isset($account->general_group)) fputs($file, "general_group: " . $account->general_group . "\n");
		if (isset($account->smb_domain)) fputs($file, "smb_domain: " . $account->smb_domain . "\n");
		// close file
		fclose($file);
	}
	else {
		StatusMessage("ERROR", _("Unable to save profile!"), $path);
		return false;
	}
	return true;
}

// deletes a user profile
function delUserProfile($file) {
	if (!eregi("^([0-9]|[a-z]|-|_)*$", $profile)) exit;
	$prof = substr(__FILE__, 0, strlen(__FILE__) - 16) . "config/profiles/users/".$file.".pru";
	if (is_file($prof)) {
		return @unlink($prof);
	}
}

// deletes a group profile
function delGroupProfile($file) {
	if (!eregi("^([0-9]|[a-z]|-|_)*$", $profile)) exit;
	$prof = substr(__FILE__, 0, strlen(__FILE__) - 16) . "config/profiles/groups/".$file.".prg";
	if (is_file($prof)) {
		return @unlink($prof);
	}
}

// deletes a host profile
function delHostProfile($file) {
	if (!eregi("^([0-9]|[a-z]|-|_)*$", $profile)) exit;
	$prof = substr(__FILE__, 0, strlen(__FILE__) - 16) . "config/profiles/hosts/".$file.".prh";
	if (is_file($prof)) {
		return @unlink($prof);
	}
}

?>
