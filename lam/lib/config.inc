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

	
	Config supplies access to the configuration data.

*/



class Config {

	// server address (e.g. ldap://127.0.0.1:389)
	var $ServerURL;

	// array of strings: users with admin rights
	var $Admins;
	
	// string: password to edit preferences
	var $Passwd;

	// single line with the names of all admin users
	var $Adminstring;
	
	// suffix for users
	var $Suff_users;
	
	// suffix for groups
	var $Suff_groups;
	
	// suffix for Samba hosts
	var $Suff_hosts;

	// minimum/maximum numbers for UID, GID and UID of Samba Hosts
	var $MinUID;
	var $MaxUID;
	var $MinGID;
	var $MaxGID;
	var $MinMachine;
	var $MaxMachine;

	// attributes that are shown in the user/group/host tables
	var $userlistAttributes;
	var $grouplistAttributes;
	var $hostlistAttributes;

	// constructor, loads preferences from ../config/lam.conf
	function Config() {
		$this->reload();
	}
	
	// reloads preferences from ../config/lam.conf
	function reload() {
		$conffile = getLAMPath() . "/config/lam.conf";
		if (is_file($conffile) == True) {
			$file = fopen($conffile, "r");
			while (!feof($file)) {
				$line = fgets($file, 1024);
				if (($line == "\n")||($line[0] == "#")) continue; // ignore comments
				// search keywords
				if (substr($line, 0, 11) == "serverURL: ") {
					$this->ServerURL = chop(substr($line, 11, strlen($line)-11));
					continue;
				}
				if (substr($line, 0, 8) == "passwd: ") {
					$this->Passwd = chop(substr($line, 8, strlen($line)-8));
					continue;
				}
				if (substr($line, 0, 8) == "admins: ") {
					$adminstr = chop(substr($line, 8, strlen($line)-8));
					$this->Adminstring = $adminstr;
					$this->Admins = explode(";", $adminstr);
					continue;
				}
				if (substr($line, 0, 12) == "usersuffix: ") {
					$this->Suff_users = chop(substr($line, 12, strlen($line)-12));
					continue;
				}
				if (substr($line, 0, 13) == "groupsuffix: ") {
					$this->Suff_groups = chop(substr($line, 13, strlen($line)-13));
					continue;
				}
				if (substr($line, 0, 12) == "hostsuffix: ") {
					$this->Suff_hosts = chop(substr($line, 12, strlen($line)-12));
					continue;
				}
				if (substr($line, 0, 8) == "minUID: ") {
					$this->MinUID = chop(substr($line, 8, strlen($line)-8));
					continue;
				}
				if (substr($line, 0, 8) == "maxUID: ") {
					$this->MaxUID = chop(substr($line, 8, strlen($line)-8));
					continue;
				}
				if (substr($line, 0, 8) == "minGID: ") {
					$this->MinGID = chop(substr($line, 8, strlen($line)-8));
					continue;
				}
				if (substr($line, 0, 8) == "maxGID: ") {
					$this->MaxGID = chop(substr($line, 8, strlen($line)-8));
					continue;
				}
				if (substr($line, 0, 12) == "minMachine: ") {
					$this->MinMachine = chop(substr($line, 12, strlen($line)-12));
					continue;
				}
				if (substr($line, 0, 12) == "maxMachine: ") {
					$this->MaxMachine = chop(substr($line, 12, strlen($line)-12));
					continue;
				}
				if (substr($line, 0, 20) == "userlistAttributes: ") {
					$this->userlistAttributes = chop(substr($line, 20, strlen($line)-20));
					continue;
				}
				if (substr($line, 0, 21) == "grouplistAttributes: ") {
					$this->grouplistAttributes = chop(substr($line, 21, strlen($line)-21));
					continue;
				}
				if (substr($line, 0, 20) == "hostlistAttributes: ") {
					$this->hostlistAttributes = chop(substr($line, 20, strlen($line)-20));
					continue;
				}
			}
			fclose($file);
		}
		else {
			echo _("Unable to load lam.conf! ") . $conffile ; echo "<br>";
		}
	}
	
	// saves preferences to ../config/lam.conf
	function save() {
		// Get path of lam.conf
		$path = getcwd();
		$path = explode("/", substr($path,1));
		for($i = 0; $i <= array_search("lam", $path); $i++) {
			$conffile .= "/" . $path[$i];
		}
		echo ;
		$conffile .= "/config/lam.conf";
		if (is_file($conffile) == True) {
			// booleans to check if value was already saved
			$save_serverURL = $save_passwd = $save_admins = $save_suffusr = $save_suffgrp = $save_suffhst =
				$save_minUID = $save_maxUID = $save_minGID = $save_maxGID = $save_minMach = $save_maxMach =
				$save_usrlstatrr =  $save_grplstatrr = $save_hstlstatrr = False;
			$file = fopen($conffile, "r");
			$file_array = array();
			while (!feof($file)) {
				array_push($file_array, fgets($file, 1024));
			}
			fclose($file);
			for ($i = 0; $i < sizeof($file_array); $i++) {
				if (($file_array[$i] == "\n")||($file_array[$i][0] == "#")) continue; // ignore comments
				// search for keywords
				if (substr($file_array[$i], 0, 11) == "serverURL: ") {
					$file_array[$i] = "serverURL: " . $this->ServerURL . "\n";
					$save_serverURL = True;
					continue;
				}
				if (substr($file_array[$i], 0, 8) == "passwd: ") {
					$file_array[$i] = "passwd: " . $this->Passwd . "\n";
					$save_passwd = True;
					continue;
				}
				if (substr($file_array[$i], 0, 8) == "admins: ") {
					$file_array[$i] = "admins: " . implode(";", $this->Admins) . "\n";
					$save_admins = True;
					continue;
				}
				if (substr($file_array[$i], 0, 12) == "usersuffix: ") {
					$file_array[$i] = "usersuffix: " . $this->Suff_users . "\n";
					$save_suffusr = True;
					continue;
				}
				if (substr($file_array[$i], 0, 13) == "groupsuffix: ") {
					$file_array[$i] = "groupsuffix: " . $this->Suff_groups . "\n";
					$save_suffgrp = True;
					continue;
				}
				if (substr($file_array[$i], 0, 12) == "hostsuffix: ") {
					$file_array[$i] = "hostsuffix: " . $this->Suff_hosts . "\n";
					$save_suffhst = True;
					continue;
				}
				if (substr($file_array[$i], 0, 8) == "minUID: ") {
					$file_array[$i] = "minUID: " . $this->MinUID . "\n";
					$save_minUID = True;
					continue;
				}
				if (substr($file_array[$i], 0, 8) == "maxUID: ") {
					$file_array[$i] = "maxUID: " . $this->MaxUID . "\n";
					$save_maxUID = True;
					continue;
				}
				if (substr($file_array[$i], 0, 8) == "minGID: ") {
					$file_array[$i] = "minGID: " . $this->MinGID . "\n";
					$save_minGID = True;
					continue;
				}
				if (substr($file_array[$i], 0, 8) == "maxGID: ") {
					$file_array[$i] = "maxGID: " . $this->MaxGID . "\n";
					$save_maxGID = True;
					continue;
				}
				if (substr($file_array[$i], 0, 12) == "minMachine: ") {
					$file_array[$i] = "minMachine: " . $this->MinMachine . "\n";
					$save_minMach = True;
					continue;
				}
				if (substr($file_array[$i], 0, 12) == "maxMachine: ") {
					$file_array[$i] = "maxMachine: " . $this->MaxMachine . "\n";
					$save_maxMach = True;
					continue;
				}
				if (substr($file_array[$i], 0, 20) == "userlistAttributes: ") {
					$file_array[$i] = "userlistAttributes: " . $this->userlistAttributes . "\n";
					$save_usrlstattr = True;
					continue;
				}
				if (substr($file_array[$i], 0, 21) == "grouplistAttributes: ") {
					$file_array[$i] = "grouplistAttributes: " . $this->grouplistAttributes . "\n";
					$save_grplstattr = True;
					continue;
				}
				if (substr($file_array[$i], 0, 20) == "hostlistAttributes: ") {
					$file_array[$i] = "hostlistAttributes: " . $this->hostlistAttributes . "\n";
					$save_hstlstattr = True;
					continue;
				}
			}
			// check if we have to add new entries (e.g. if user upgraded LAM and has an old lam.conf)
			if (!$save_serverURL == True) array_push($file_array, "\n\n# server address (e.g. ldap://localhost:389 or ldaps://localhost:636)\n" . "serverURL: " . $this->ServerURL);
			if (!$save_passwd == True) array_push($file_array, "\n\n# password to change these preferences via webfrontend\n" . "passwd: " . $this->Passwd);
			if (!$save_admins == True) array_push($file_array, "\n\n# list of users who are allowed to use LDAP Account Manager\n" .
				"# names have to be seperated by semicolons\n" . 
				"# e.g. admins: cn=admin,dc=yourdomain,dc=org;cn=root,dc=yourdomain,dc=org\n" . "admins: " . $this->Admins);
			if (!$save_suffusr == True) array_push($file_array, "\n\n# suffix of users\n" . 
				"# e.g. ou=People,dc=yourdomain,dc=org\n" . "usersuffix: " . $this->Suff_users);
			if (!$save_suffgrp == True) array_push($file_array, "\n\n# suffix of groups\n" . 
				"# e.g. ou=Groups,dc=yourdomain,dc=org\n" . "groupsuffix: " . $this->Suff_groups);
			if (!$save_suffhst == True) array_push($file_array, "\n\n# suffix of Samba hosts\n" . 
				"# e.g. ou=machines,dc=yourdomain,dc=org\n" . "hostsuffix: " . $this->Suff_hosts);
			if (!$save_minUID == True) array_push($file_array, "\n\n# minimum UID number\n" . "minUID: " . $this->MinUID);
			if (!$save_maxUID == True) array_push($file_array, "\n\n# maximum UID number\n" . "maxUID: " . $this->MaxUID);
			if (!$save_minGID == True) array_push($file_array, "\n\n# minimum GID number\n" . "minGID: " . $this->MinGID);
			if (!$save_maxGID == True) array_push($file_array, "\n\n# maximum GID number\n" . "maxGID: " . $this->MaxGID);
			if (!$save_minMach == True) array_push($file_array, "\n\n# minimum UID number for Samba hosts\n" . "minMachine: " . $this->MinMachine);
			if (!$save_maxMach == True) array_push($file_array, "\n\n# maximum UID number for Samba hosts\n" . "maxMachine: " . $this->MaxMachine);
			if (!$save_usrlstattr == True) array_push($file_array, "\n\n# list of attributes to show in user list\n# entries can either be predefined values (e.g. '#cn' or '#uid')" .
			"\n# or individual ones (e.g. 'uid:User ID' or 'host:Host Name')\n# values have to be seperated by semicolons\n" . "userlistAttributes: " . $this->userlistAttributes);
			if (!$save_grplstattr == True) array_push($file_array, "\n\n# list of attributes to show in group list\n# entries can either be predefined values (e.g. '#cn' or '#gidNumber')" .
			"\n# or individual ones (e.g. 'cn:Group Name')\n# values have to be seperated by semicolons\n" . "grouplistAttributes: " . $this->grouplistAttributes);
			if (!$save_hstlstattr == True) array_push($file_array, "\n\n# list of attributes to show in host list\n# entries can either be predefined values (e.g. '#cn' or '#uid')" .
			"\n# or individual ones (e.g. 'cn:Host Name')\n# values have to be seperated by semicolons\n" . "hostlistAttributes: " . $this->hostlistAttributes);
			$file = fopen($conffile, "w");
			if ($file) {
				for ($i = 0; $i < sizeof($file_array); $i++) fputs($file, $file_array[$i]);
				fclose($file);
			}
			else {
				echo _("<br><font color=\"red\">Cannot open config file!</font>");
				exit;
			}
		}
	}

	// prints current preferences
	function printconf() {
		echo _("<b>ServerURL: </b>") . $this->ServerURL . "<br>";
		echo _("<b>Admins: </b>") . $this->Adminstring . "<br>";
		echo _("<b>UserSuffix: </b>") . $this->Suff_users . "<br>";
		echo _("<b>GroupSuffix: </b>") . $this->Suff_groups . "<br>";
		echo _("<b>HostSuffix: </b>") . $this->Suff_hosts . "<br>";
		echo _("<b>minUID: </b>") . $this->MinUID . "<br>";
		echo _("<b>maxUID: </b>") . $this->MaxUID . "<br>";
		echo _("<b>minGID: </b>") . $this->MinGID . "<br>";
		echo _("<b>maxGID: </b>") . $this->MaxGID . "<br>";
		echo _("<b>minMachine: </b>") . $this->MinMachine . "<br>";
		echo _("<b>maxMachine: </b>") . $this->MaxMachine . "<br>";
		echo _("<b>userlistAttributes: </b>") . $this->userlistAttributes . "<br>";
		echo _("<b>grouplistAttributes: </b>") . $this->grouplistAttributes . "<br>";
		echo _("<b>hostlistAttributes: </b>") . $this->hostlistAttributes;
	}

// functions to read/write preferences
	
	// returns the server address as string
	function get_ServerURL() {
		return $this->ServerURL;
	}

	// sets the server address
	function set_ServerURL($value) {
		if (is_string($value)) $this->ServerURL = $value;
		else echo _("Config->set_ServerURL failed!");
	}
	
	// returns an array of string with all admin names
	function get_Admins() {
		return $this->Admins;
	}
	
	// needs an array of string containing all admin users
	function set_Admins($value) {
		if (is_array($value)) { // check if $value is array of strings
			$b = true;
			for($i = 0; $i < sizeof($value); $i++){
				if (is_string($value[$i]) == false) {
					$b = false;
					break;
				}
			}
			if ($b) $this->Admins = $value;
		}
		else echo _("Config->set_Admins failed!");
	}
	
	// returns all admin users seperated by semicolons
	function get_Adminstring() {
		return $this->Adminstring;
	}
	
	// needs a string that contains all admin users seperated by semicolons
	function set_Adminstring($value) {
		if (is_string($value)) {
			$this->Adminstring = $value;
			$this->Admins = explode(";", $value);
		}
		else echo _("Config->set_Adminstring failed!");
	}
	
	// returns the password to access the preferences wizard
	function get_Passwd() {
		return $this->Passwd;
	}
	
	// sets the preferences wizard password
	function set_Passwd($value) {
		if (is_string($value)) $this->Passwd = $value;
		else echo _("Config->set_Passwd failed!");
	}
	
	// returns the LDAP suffix where users are saved
	function get_UserSuffix() {
		return $this->Suff_users;
	}
	
	// sets the LDAP suffix where users are saved
	function set_UserSuffix($value) {
		if (is_string($value)) $this->Suff_users = $value;
		else echo _("Config->set_UserSuffix failed!");
	}
	
	// returns the LDAP suffix where groups are saved
	function get_GroupSuffix() {
		return $this->Suff_groups;
	}
	
	// sets the LDAP suffix where groups are saved
	function set_GroupSuffix($value) {
		if (is_string($value)) $this->Suff_groups = $value;
		else echo _("Config->set_GroupSuffix failed!");
	}
	
	// returns the LDAP suffix where hosts are saved
	function get_HostSuffix() {
		return $this->Suff_hosts;
	}
	
	// sets the LDAP suffix where hosts are saved
	function set_HostSuffix($value) {
		if (is_string($value)) $this->Suff_hosts = $value;
		else echo _("Config->set_HostSuffix failed!");
	}
	
	// returns the minimum UID to use when creating new users
	function get_minUID() {
	return $this->MinUID;
	}
	
	// sets the minimum UID to use when creating new users
	function set_minUID($value) {
		if (is_numeric($value)) $this->MinUID = $value;
		else echo _("Config->set_minUID failed!");
	}

	// returns the maximum UID to use when creating new users
	function get_maxUID() {
	return $this->MaxUID;
	}
	
	// sets the maximum UID to use when creating new users
	function set_maxUID($value) {
		if (is_numeric($value)) $this->MaxUID = $value;
		else echo _("Config->set_maxUID failed!");
	}

	// returns the minimum GID to use when creating new groups
	function get_minGID() {
	return $this->MinGID;
	}
	
	// sets the minimum GID to use when creating new groups
	function set_minGID($value) {
		if (is_numeric($value)) $this->MinGID = $value;
		else echo _("Config->set_minGID failed!");
	}

	// returns the maximum GID to use when creating new groups
	function get_maxGID() {
	return $this->MaxGID;
	}
	
	// sets the maximum GID to use when creating new groups
	function set_maxGID($value) {
		if (is_numeric($value)) $this->MaxGID = $value;
		else echo _("Config->set_maxGID failed!");
	}

	// returns the minimum UID to use when creating new Samba hosts
	function get_minMachine() {
	return $this->MinMachine;
	}
	
	// sets the minimum UID to use when creating new Samba hosts
	function set_minMachine($value) {
		if (is_numeric($value)) $this->MinMachine = $value;
		else echo _("Config->set_minMachine failed!");
	}

	// returns the maximum UID to use when creating new Samba hosts
	function get_maxMachine() {
	return $this->MaxMachine;
	}
	
	// sets the maximum UID to use when creating new Samba hosts
	function set_maxMachine($value) {
		if (is_numeric($value)) $this->MaxMachine = $value;
		else echo _("Config->set_maxMachine failed!");
	}
	
	// returns the list of attributes to show in user list
	function get_userlistAttributes() {
	return $this->userlistAttributes;
	}

	// sets the list of attributes to show in user list
	function set_userlistAttributes($value) {
		if (is_string($value)) $this->userlistAttributes = $value;
		else echo _("Config->set_userlistAttributes failed!");
	}

	// returns the list of attributes to show in group list
	function get_grouplistAttributes() {
	return $this->grouplistAttributes;
	}

	// sets the list of attributes to show in group list
	function set_grouplistAttributes($value) {
		if (is_string($value)) $this->grouplistAttributes = $value;
		else echo _("Config->set_grouplistAttributes failed!");
	}

	// returns the list of attributes to show in host list
	function get_hostlistAttributes() {
	return $this->hostlistAttributes;
	}

	// sets the list of attributes to show in host list
	function set_hostlistAttributes($value) {
		if (is_string($value)) $this->hostlistAttributes = $value;
		else echo _("Config->set_hostlistAttributes failed!");
	}

}

// Get path of LAM directory without "/" at the end
function getLAMPath() {
	$lampath = "";
	$path = getcwd();
	$path = explode("/", substr($path,1));
	for($i = 0; $i <= array_search("lam", $path); $i++) {
		$lampath .= "/" . $path[$i];
	}
	return $lampath;
}

?>
