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


// Config supplies access to the configuration data.

class Config {

	// string: can be "True" or "False"
	//         use SSL-connection?
	var $SSL;
	
	// string: hostname
	var $Host;
	
	// string: port number
	var $Port;
	
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
	
	// default shell and list of possible shells
	var $DefaultShell;
	var $ShellList;

	// constructor, loads preferences from ../lam.conf
	function Config() {
		$this->reload();
	}
	
	// reloads preferences from ../lam.conf
	function reload() {
		$conffile = "../lam.conf";
		if (is_file($conffile) == True) {
			$file = fopen($conffile, "r");
			while (!feof($file)) {
				$line = fgets($file, 1024);
				if (($line == "\n")||($line[0] == "#")) continue; // ignore comments
				if (substr($line, 0, 5) == "ssl: ") {
					$this->SSL = chop(substr($line, 5, strlen($line)-5));
					continue;
				}
				if (substr($line, 0, 6) == "host: ") {
					$this->Host = chop(substr($line, 6, strlen($line)-6));
					continue;
				}
				if (substr($line, 0, 6) == "port: ") {
					$this->Port = chop(substr($line, 6, strlen($line)-6));
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
				if (substr($line, 0, 14) == "defaultShell: ") {
					$this->DefaultShell = chop(substr($line, 14, strlen($line)-14));
					continue;
				}
				if (substr($line, 0, 11) == "shellList: ") {
					$this->ShellList = chop(substr($line, 11, strlen($line)-11));
					continue;
				}
			}
			fclose($file);
		}
		else {
			echo _("Unable to load lam.conf!"); echo "<br>";
		}
	}
	
	// saves preferences to ../lam.conf
	function save() {
		$conffile = "../lam.conf";
		if (is_file($conffile) == True) {
			$save_ssl = $save_host = $save_port = $save_passwd = $save_admins = $save_suffusr = $save_suffgrp = $save_suffhst =
				$save_minUID = $save_maxUID = $save_minGID = $save_maxGID = $save_minMach = $save_maxMach = $save_defShell = $save_shellList = False;
			$file = fopen($conffile, "r");
			$file_array = array();
			while (!feof($file)) {
				array_push($file_array, fgets($file, 1024));
			}
			fclose($file);
			for ($i = 0; $i < sizeof($file_array); $i++) {
				if (($file_array[$i] == "\n")||($file_array[$i][0] == "#")) continue; // ignore comments
				if (substr($file_array[$i], 0, 5) == "ssl: ") {
					$file_array[$i] = "ssl: " . $this->SSL . "\n";
					$save_ssl = True;
					continue;
				}
				if (substr($file_array[$i], 0, 6) == "host: ") {
					$file_array[$i] = "host: " . $this->Host . "\n";
					$save_host = True;
					continue;
				}
				if (substr($file_array[$i], 0, 6) == "port: ") {
					$file_array[$i] = "port: " . $this->Port . "\n";
					$save_port = True;
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
				if (substr($file_array[$i], 0, 14) == "defaultShell: ") {
					$file_array[$i] = "defaultShell: " . $this->DefaultShell . "\n";
					$save_defShell = True;
					continue;
				}
				if (substr($file_array[$i], 0, 11) == "shellList: ") {
					$file_array[$i] = "shellList: " . $this->ShellList . "\n";
					$save_shellList = True;
					continue;
				}
			}
			// check if we have to add new entries (e.g. if user upgraded LAM)
			if (!$save_ssl == True) array_push($file_array, "\n\n# use SSL to connect, can be True or False\n" . "ssl: " . $this->SSL);
			if (!$save_host == True) array_push($file_array, "\n\n# hostname of LDAP server (e.g localhost)\n" . "host: " . $this->Host);
			if (!$save_port == True) array_push($file_array, "\n\n# portnumber of LDAP server (default 389)\n" . "port: " . $this->Port);
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
			if (!$save_defShell == True) array_push($file_array, "\n\n# default shell when creating new user\n" . "defaultShell: " . $this->DefaultShell);
			if (!$save_shellList == True) array_push($file_array, "\n\n# list of possible shells\n" . "shellList: " . $this->ShellList);
			$file = fopen($conffile, "w");
			for ($i = 0; $i < sizeof($file_array); $i++) fputs($file, $file_array[$i]);
			fclose($file);
		}
	}
	
	// prints current preferences
	function printconf() {
		echo _("<b>SSL: </b>" ) . $this->SSL . "<br>";
		echo _("<b>Host: </b>") . $this->Host . "<br>";
		echo _("<b>Port: </b>") . $this->Port . "<br>";
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
		echo _("<b>Default Shell: </b>") . $this->DefaultShell . "<br>";
		echo _("<b>Shell list: </b>") . $this->ShellList;
	}

// functions to read/write preferences
	
	function get_SSL() {
		return $this->SSL;
	}
	
	function set_SSL($value) {
		if (($value == "True") || ($value == "False")) $this->SSL = $value;
		else echo _("Config->set_SSL failed!");
	}
	
	function get_Host() {
		return $this->Host;
	}
	
	function set_Host($value) {
		if (is_string($value)) $this->Host = $value;
		else echo _("Config->set_Host failed!");
	}
	
	function get_Port() {
		return $this->Port;
	}
	
	function set_Port($value) {
		if (is_numeric($value)) $this->Port = $value;
		else echo _("Config->set_Port failed!");
	}
	
	function get_Admins() {
		return $this->Admins;
	}
	
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
	
	function get_Adminstring() {
		return $this->Adminstring;
	}
	
	function set_Adminstring($value) {
		if (is_string($value)) {
			$this->Adminstring = $value;
			$this->Admins = explode(";", $value);
		}
		else echo _("Config->set_Adminstring failed!");
	}
	
	function get_Passwd() {
		return $this->Passwd;
	}
	
	function set_Passwd($value) {
		if (is_string($value)) $this->Passwd = $value;
		else echo _("Config->set_Passwd failed!");
	}
	
	function get_UserSuffix() {
		return $this->Suff_users;
	}
	
	function set_UserSuffix($value) {
		if (is_string($value)) $this->Suff_users = $value;
		else echo _("Config->set_UserSuffix failed!");
	}
	
	function get_GroupSuffix() {
		return $this->Suff_groups;
	}
	
	function set_GroupSuffix($value) {
		if (is_string($value)) $this->Suff_groups = $value;
		else echo _("Config->set_GroupSuffix failed!");
	}
	
	function get_HostSuffix() {
		return $this->Suff_hosts;
	}
	
	function set_HostSuffix($value) {
		if (is_string($value)) $this->Suff_hosts = $value;
		else echo _("Config->set_HostSuffix failed!");
	}
	
	function get_minUID() {
	return $this->MinUID;
	}
	
	function set_minUID($value) {
		if (is_numeric($value)) $this->MinUID = $value;
		else echo _("Config->set_minUID failed!");
	}

	function get_maxUID() {
	return $this->MaxUID;
	}
	
	function set_maxUID($value) {
		if (is_numeric($value)) $this->MaxUID = $value;
		else echo _("Config->set_maxUID failed!");
	}

	function get_minGID() {
	return $this->MinGID;
	}
	
	function set_minGID($value) {
		if (is_numeric($value)) $this->MinGID = $value;
		else echo _("Config->set_minGID failed!");
	}

	function get_maxGID() {
	return $this->MaxGID;
	}
	
	function set_maxGID($value) {
		if (is_numeric($value)) $this->MaxGID = $value;
		else echo _("Config->set_maxGID failed!");
	}

	function get_minMachine() {
	return $this->MinMachine;
	}
	
	function set_minMachine($value) {
		if (is_numeric($value)) $this->MinMachine = $value;
		else echo _("Config->set_minMachine failed!");
	}

	function get_maxMachine() {
	return $this->MaxMachine;
	}
	
	function set_maxMachine($value) {
		if (is_numeric($value)) $this->MaxMachine = $value;
		else echo _("Config->set_maxMachine failed!");
	}
	
	function get_defaultShell() {
	return $this->DefaultShell;
	}
	
	function set_defaultShell($value) {
		if (is_string($value)) $this->DefaultShell = $value;
		else echo _("Config->set_shellList failed!");
	}

	function get_shellList() {
	return $this->ShellList;
	}
	
	function set_shellList($value) {
		if (is_string($value)) $this->ShellList = $value;
		else echo _("Config->set_shellList failed!");
	}

}

?>
