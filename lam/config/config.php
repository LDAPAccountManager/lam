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
	// boolean: use SSL-connection?
	var $SSL;
	// string: hostname
	var $Host;
	// int: port number
	var $Port;
	// array of strings: users with admin rights
	var $Admins;
	// string: password to edit preferences
	var $Passwd;

	
	// constructor, loads preferences from ../lam.conf
	function Config() {
		$this->reload();
		$this->save();
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
					$this->SSL = substr($line, 5, strlen($line)-5);
					continue;
				}
				if (substr($line, 0, 6) == "host: ") {
					$this->Host = substr($line, 6, strlen($line)-6);
					continue;
				}
				if (substr($line, 0, 6) == "port: ") {
					$this->Port = substr($line, 6, strlen($line)-6);
					continue;
				}
				if (substr($line, 0, 8) == "passwd: ") {
					$this->Passwd = substr($line, 8, strlen($line)-8);
					continue;
				}
				if (substr($line, 0, 8) == "admins: ") {
					$adminstr = substr($line, 8, strlen($line)-8);
					$this->Admins = explode(";", $adminstr);
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
			$save_ssl = $save_host = $save_port = $save_passwd = $save_admins = False;
			$file = fopen($conffile, "r");
			$file_array = array();
			while (!feof($file)) {
				array_push($file_array, fgets($file, 1024));
			}
			fclose($file);
			for ($i = 0; $i < sizeof($file_array); $i++) {
				if (($file_array[$i] == "\n")||($file_array[$i][0] == "#")) continue;
				if (substr($file_array[$i], 0, 5) == "ssl: ") {
					$file_array[$i] = "ssl: " . $this->SSL;
					$save_ssl = True;
					continue;
				}
				if (substr($file_array[$i], 0, 6) == "host: ") {
					$file_array[$i] = "host: " . $this->Host;
					$save_host = True;
					continue;
				}
				if (substr($file_array[$i], 0, 6) == "port: ") {
					$file_array[$i] = "port: " . $this->Port;
					$save_port = True;
					continue;
				}
				if (substr($file_array[$i], 0, 8) == "passwd: ") {
					$file_array[$i] = "passwd: " . $this->Passwd;
					$save_passwd = True;
					continue;
				}
				if (substr($file_array[$i], 0, 8) == "admins: ") {
					$file_array[$i] = "admins: " . implode(";", $this->Admins);
					$save_admins = True;
					continue;
				}
			}
			// check if we have to add new entries
			if (!$save_ssl == True) array_push($file_array, "# use SSL to connect, can be True or False\n" . "ssl: " . $this->SSL);
			if (!$save_host == True) array_push($file_array, "# hostname of LDAP server (e.g localhost)\n" . "ssl: " . $this->Host);
			if (!$save_port == True) array_push($file_array, "# portnumber of LDAP server (default 389)\n" . "ssl: " . $this->Port);
			if (!$save_passwd == True) array_push($file_array, "# password to change these preferences via webfrontend\n" . "ssl: " . $this->Passwd);
			if (!$save_admins == True) array_push($file_array, "# list of users who are allowed to use LDAP Account Manager\n
				# names have to be seperated by semicolons\n
				# e.g. admins: cn=admin,dc=yourdomain,dc=org;cn=root,dc=yourdomain,dc=org\n" . "ssl: " . $this->Admins);
			$file = fopen($conffile, "w");
			for ($i = 0; $i < sizeof($file_array); $i++) fputs($file, $file_array[$i]);
			fclose($file);
		}
	}
	
	// used by configuration wizard to save new preferences
	function input() {
	if (is_numeric($Port) && $Host!="" && $Admins!="") {
		$this->SSL = $SSL;
		$this->Host = $Host;
		$this->Port = $Port;
		$this->Admins = $Admins;
		$this->Passwd = $Passwd;
		$this->save();	
	}
	else echo _("Portnumber must be a number!");
	}
	
	// prints current preferences
	function printconf() {
		echo _("SSL: " ) . $this->SSL . "</br>";
		echo _("Host: ") . $this->Host . "</br>";
		echo _("Port: ") . $this->Port . "</br>";
		echo _("Passwd: ") . $this->Passwd . "</br>";
		echo _("Admins: "); for ($i = 0; $i < sizeof($this->Admins); $i++) { echo $this->Admins[$i] . "&nbsp;&nbsp;&nbsp;"; }
	}
	
	function get_SSL() {
		return $this->SSL;
	}
	
	function set_SSL($value) {
		if (is_bool($value)) $this->SSL = $value;
	}
	
	function get_Host() {
		return $this->Host;
	}
	
	function set_Host($value) {
		if (is_string($value)) $this->Host = $value;
	}
	
	function get_Port() {
		return $this->Port;
	}
	
	function set_Port($value) {
		if (is_numeric($value)) $this->Port = $value;
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
	
	function get_Passwd() {
		return $this->Passwd;
	}
	
	function set_Passwd($value) {
		if (is_string($value)) $this->Passwd = $value;
	}
	
}
