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
	var $suff_users;
	
	// suffix for groups
	var $suff_groups;
	
	// suffix for Samba hosts
	var $suff_hosts;


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
				if (($line == "\n")||($line[0] == "#")) continue;
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
					$this->suff_users = chop(substr($line, 12, strlen($line)-12));
					continue;
				}
				if (substr($line, 0, 13) == "groupsuffix: ") {
					$this->suff_groups = chop(substr($line, 13, strlen($line)-13));
					continue;
				}
				if (substr($line, 0, 12) == "hostsuffix: ") {
					$this->suff_hosts = chop(substr($line, 12, strlen($line)-12));
					continue;
				}
			}
			fclose($file);
		}
		else {
			echo _("Unable to load lam.conf!"); echo "</br>";
		}
	}
	
	// saves preferences to ../lam.conf
	function save() {
		$conffile = "../lam.conf";
		if (is_file($conffile) == True) {
			$save_ssl = $save_host = $save_port = $save_passwd = $save_admins = $save_suffusr = $save_suffgrp = $save_suffhst = False;
			$file = fopen($conffile, "r");
			$file_array = array();
			while (!feof($file)) {
				array_push($file_array, fgets($file, 1024));
			}
			fclose($file);
			for ($i = 0; $i < sizeof($file_array); $i++) {
				if (($file_array[$i] == "\n")||($file_array[$i][0] == "#")) continue;
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
					$file_array[$i] = "usersuffix: " . $this->suff_users . "\n";
					$save_suffusr = True;
					continue;
				}
				if (substr($file_array[$i], 0, 13) == "groupsuffix: ") {
					$file_array[$i] = "groupsuffix: " . $this->suff_groups . "\n";
					$save_suffgrp = True;
					continue;
				}
				if (substr($file_array[$i], 0, 12) == "hostsuffix: ") {
					$file_array[$i] = "hostsuffix: " . $this->suff_hosts . "\n";
					$save_suffhst = True;
					continue;
				}
			}
			// check if we have to add new entries (e.g. if user upgraded LAM)
			if (!$save_ssl == True) array_push($file_array, "\n# use SSL to connect, can be True or False\n" . "ssl: " . $this->SSL);
			if (!$save_host == True) array_push($file_array, "\n# hostname of LDAP server (e.g localhost)\n" . "host: " . $this->Host);
			if (!$save_port == True) array_push($file_array, "\n# portnumber of LDAP server (default 389)\n" . "port: " . $this->Port);
			if (!$save_passwd == True) array_push($file_array, "\n# password to change these preferences via webfrontend\n" . "passwd: " . $this->Passwd);
			if (!$save_admins == True) array_push($file_array, "\n# list of users who are allowed to use LDAP Account Manager\n" . 
				"# names have to be seperated by semicolons\n" . 
				"# e.g. admins: cn=admin,dc=yourdomain,dc=org;cn=root,dc=yourdomain,dc=org\n" . "admins: " . $this->Admins);
			if (!$save_suffusr == True) array_push($file_array, "\n# suffix of users\n" . 
				"# e.g. ou=People,dc=yourdomain,dc=org\n" . "usersuffix: " . $this->suff_users);
			if (!$save_suffgrp == True) array_push($file_array, "\n# suffix of groups\n" . 
				"# e.g. ou=Groups,dc=yourdomain,dc=org\n" . "groupsuffix: " . $this->suff_groups);
			if (!$save_suffhst == True) array_push($file_array, "\n# suffix of hosts\n" . 
				"# e.g. ou=machines,dc=yourdomain,dc=org\n" . "hostsuffix: " . $this->suff_hosts);
			$file = fopen($conffile, "w");
			for ($i = 0; $i < sizeof($file_array); $i++) fputs($file, $file_array[$i]);
			fclose($file);
		}
	}
	
	// prints current preferences
	function printconf() {
		echo _("<b>SSL: </b>" ) . $this->SSL . "</br>";
		echo _("<b>Host: </b>") . $this->Host . "</br>";
		echo _("<b>Port: </b>") . $this->Port . "</br>";
		echo _("<b>Admins: </b>") . $this->Adminstring . "</br>";
		echo _("<b>UserSuffix: </b>") . $this->suff_users . "</br>";
		echo _("<b>GroupSuffix: </b>") . $this->suff_groups . "</br>";
		echo _("<b>HostSuffix: </b>") . $this->suff_hosts;
	}
	
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
		if (is_array($value)) {
			$b = true;
			for($i = 0; $i < sizeof($value); $i++){
				if (is_string($value[$i]) == false) {
					$b = false;
					break;
				}
			}
			if ($b) $this->Admins = $value;
		}
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
		return $this->suff_users;
	}
	
	function set_UserSuffix($value) {
		if (is_string($value)) $this->suff_users = $value;
		else echo _("Config->set_UserSuffix failed!");
	}
	
	function get_GroupSuffix() {
		return $this->suff_groups;
	}
	
	function set_GroupSuffix($value) {
		if (is_string($value)) $this->suff_groups = $value;
		else echo _("Config->set_GroupSuffix failed!");
	}
	
	function get_HostSuffix() {
		return $this->suff_hosts;
	}
	
	function set_HostSuffix($value) {
		if (is_string($value)) $this->suff_hosts = $value;
		else echo _("Config->set_HostSuffix failed!");
	}
	
}

?>
