<?php
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
	// string: language identifier
//	var $Language 


	// loads preferences from ../lam.conf
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
		}
		else {
			echo _("Unable to load lam.conf!"); echo "</br>";
		}
	}
	
	// saves preferences to ../lam.conf
	function save() {
	// save configs...
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
		echo _("SSL: " ); echo $this->SSL; echo "</br>";
		echo _("Host: "); echo $this->Host; echo "</br>";
		echo _("Port: "); echo $this->Port; echo "</br>";
		echo _("Passwd: "); echo $this->Passwd; echo "</br>";
		echo _("Admins: "); for ($i = 0; $i < sizeof($this->Admins); $i++) { echo $this->Admins[$i]; echo "&nbsp;&nbsp;&nbsp;"; }
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
	
/*	function get_Language() {
		return $this->Language;
	}
	
	function set_Language($value) {
		$this->Language = $value;
	}
*/	
	
	}
