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

include_once("status.inc");

// sets language settings for automatic translation
function setlanguage() {
	if ($_SESSION['language']) {
		$language = explode(":", $_SESSION['language']);
		putenv("LANG=" . $language[0]);  // e.g. LANG=de_DE
		setlocale(LC_ALL, $language[0]);  // set LC_ALL to de_DE
		$locdir = substr(__FILE__, 0, strlen(__FILE__) - 15) . "/locale";  // set path to translations
		bindtextdomain("messages", $locdir);
		textdomain("messages");
	}
	else echo _("Language not defined in session!");
}

// returns an array of string with all available configuration profiles (without .conf)
function getConfigProfiles() {
	$dir = dir(substr(__FILE__, 0, strlen(__FILE__) - 15) . "/config");
	$ret = array();
	$pos = 0;
	while ($entry = $dir->read()){
		$ext = substr($entry, strlen($entry)-5, 5);
		$name = substr($entry, 0, strlen($entry)-5);
		// check if extension is right, add to profile list
		if ($ext == ".conf") {
			$ret[$pos] = $name;
			$pos ++;
		}
	}
	sort($ret);
	return $ret;
}

// print meta refresh
// $page is the URL of the target page
function metaRefresh($page) {
	echo $_SESSION['header'];
	echo "<html>\n";
	echo "<head>\n";
	echo "<title></title>\n";
	echo "<meta http-equiv=\"refresh\" content=\"0; URL=" . $page . "\">\n";
	echo "</head>\n";
	echo "<body>\n";
	// print link if refresh does not work
	echo "<p>\n";
	echo "<a href=\"" . $page . "\">" . _("Click here if you are not directed to the next page.") . "</a>\n";
	echo "</p>\n";
	echo "</body>\n";
	echo "</html>\n";
}


/*
// class Config
*/
// manages .conf files
class Config {

  // server address (e.g. ldap://127.0.0.1:389)
  var $ServerURL;

  // array of strings: users with admin rights
  var $Admins;

  // string: password to edit preferences
  var $Passwd;

  // suffix for users
  var $usersuffix;

  // suffix for groups
  var $groupsuffix;

  // suffix for Samba hosts
  var $hostsuffix;

  // suffix for domains (Samba 3)
  var $domainsuffix;

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

  // maximum number of rows shown in user/group/host list
  var $maxlistentries;

  // default language
  var $defaultLanguage;

  // Path to external script and server where it is executed
  // used for managing quota and home directories
  // optional settings, may not be defined
  var $scriptPath;
  var $scriptServer;

  // if "yes" use the new LDAP schema for Samba 3.x
  var $samba3;

  // LDAP cache timeout
  var $cachetimeout;

  // password hash algorithm
  var $pwdhash;

  // name of configuration file
  var $file;

	// list of all settings in config file
	var $settings = array("ServerURL", "Passwd", "Admins", "usersuffix", "groupsuffix", "hostsuffix",
		"domainsuffix", "MinUID", "MaxUID", "MinGID", "MaxGID", "MinMachine", "MaxMachine",
		"userlistAttributes", "grouplistAttributes", "hostlistAttributes", "maxlistentries",
		"defaultLanguage", "scriptPath", "scriptServer", "samba3", "cachetimeout", "pwdhash");


	// constructor, loads preferences from config file
	function Config($file=0) {
		// load first profile if none is given
		if (!is_string($file)) {
			$profiles = getConfigProfiles();
			$file = $profiles[0];
		}
		$this->file = $file;
		$this->reload();
	}

	// reloads preferences from config file
	function reload() {
		$conffile = substr(__FILE__, 0, strlen(__FILE__) - 15) . "/config/" . $this->file . ".conf";
		if (is_file($conffile) == True) {
			$file = fopen($conffile, "r");
			while (!feof($file)) {
				$line = fgets($file, 1024);
				$line = trim($line);  // remove spaces at the beginning and end
				if (($line == "")||($line[0] == "#")) continue; // ignore comments and empty lines
				// search keywords
				for ($i = 0; $i < sizeof($this->settings); $i++) {
					$keyword = $this->settings[$i];
					$keylen = strlen($keyword);
					if (strtolower(substr($line, 0, $keylen + 2)) == strtolower($keyword . ": ")) {
						$this->$keyword = substr($line, $keylen + 2, strlen($line) - $keylen -2);
						break;
					}
				}
			}
			fclose($file);
		}
		else {
			StatusMessage("ERROR", "", _("Unable to load configuration!") . " (" . $conffile . ")");
		}
	}

	// saves preferences to config file
	function save() {
		$conffile = substr(__FILE__, 0, strlen(__FILE__) - 15) . "/config/" . $this->file . ".conf";
		if (is_file($conffile) == True) {
			$file = fopen($conffile, "r");
			$file_array = array();
			// read config file
			while (!feof($file)) {
				array_push($file_array, fgets($file, 1024));
			}
			fclose($file);
			// generate new configuration file
			$saved = array();	// includes all settings which have been saved
			for ($i = 0; $i < sizeof($file_array); $i++) {
				$line = trim($file_array[$i]);
				if (($line == "")||($line[0] == "#")) continue; // ignore comments and empty lines
				// search for keywords
				for ($k = 0; $k < sizeof($this->settings); $k++) {
					$keyword = $this->settings[$k];
					$keylen = strlen($keyword);
					if (strtolower(substr($line, 0, $keylen + 1)) == strtolower($keyword . ":")) {
						$file_array[$i] = $keyword . ": " . $this->$keyword . "\n";
						$saved[] = $keyword;	// mark keyword as saved
						break;
					}
				}
			}
			// check if we have to add new entries (e.g. if user upgraded LAM and has an old config file)
			if (!in_array("ServerURL", $saved)) array_push($file_array, "\n\n# server address (e.g. ldap://localhost:389 or ldaps://localhost:636)\n" . "serverURL: " . $this->ServerURL . "\n");
			if (!in_array("Passwd", $saved)) array_push($file_array, "\n\n# password to change these preferences via webfrontend\n" . "passwd: " . $this->Passwd . "\n");
			if (!in_array("Admins", $saved)) array_push($file_array, "\n\n# list of users who are allowed to use LDAP Account Manager\n" .
								"# names have to be seperated by semicolons\n" .
								"# e.g. admins: cn=admin,dc=yourdomain,dc=org;cn=root,dc=yourdomain,dc=org\n" . "admins: " . $this->Admins . "\n");
			if (!in_array("usersuffix", $saved)) array_push($file_array, "\n\n# suffix of users\n" .
								"# e.g. ou=People,dc=yourdomain,dc=org\n" . "usersuffix: " . $this->usersuffix . "\n");
			if (!in_array("groupsuffix", $saved)) array_push($file_array, "\n\n# suffix of groups\n" .
								"# e.g. ou=Groups,dc=yourdomain,dc=org\n" . "groupsuffix: " . $this->groupsuffix . "\n");
			if (!in_array("hostsuffix", $saved)) array_push($file_array, "\n\n# suffix of Samba hosts\n" .
								"# e.g. ou=machines,dc=yourdomain,dc=org\n" . "hostsuffix: " . $this->hostsuffix . "\n");
			if (!in_array("domainsuffix", $saved)) array_push($file_array, "\n\n# suffix of Samba 3 domains\n" .
								"# e.g. ou=domains,dc=yourdomain,dc=org\n" . "domainsuffix: " . $this->domainsuffix . "\n");
			if (!in_array("MinUID", $saved)) array_push($file_array, "\n\n# minimum UID number\n" . "minUID: " . $this->MinUID . "\n");
			if (!in_array("MaxUID", $saved)) array_push($file_array, "\n\n# maximum UID number\n" . "maxUID: " . $this->MaxUID . "\n");
			if (!in_array("MinGID", $saved)) array_push($file_array, "\n\n# minimum GID number\n" . "minGID: " . $this->MinGID . "\n");
			if (!in_array("MaxGID", $saved)) array_push($file_array, "\n\n# maximum GID number\n" . "maxGID: " . $this->MaxGID . "\n");
			if (!in_array("MinMachine", $saved)) array_push($file_array, "\n\n# minimum UID number for Samba hosts\n" . "minMachine: " . $this->MinMachine . "\n");
			if (!in_array("MaxMachine", $saved)) array_push($file_array, "\n\n# maximum UID number for Samba hosts\n" . "maxMachine: " . $this->MaxMachine . "\n");
			if (!in_array("userlistAttributes", $saved)) array_push($file_array, "\n\n# list of attributes to show in user list\n# entries can either be predefined values (e.g. '#cn' or '#uid')" .
								"\n# or individual ones (e.g. 'uid:User ID' or 'host:Host Name')\n# values have to be seperated by semicolons\n" . "userlistAttributes: " . $this->userlistAttributes . "\n");
			if (!in_array("grouplistAttributes", $saved)) array_push($file_array, "\n\n# list of attributes to show in group list\n# entries can either be predefined values (e.g. '#cn' or '#gidNumber')" .
								"\n# or individual ones (e.g. 'cn:Group Name')\n# values have to be seperated by semicolons\n" . "grouplistAttributes: " . $this->grouplistAttributes . "\n");
			if (!in_array("hostlistAttributes", $saved)) array_push($file_array, "\n\n# list of attributes to show in host list\n# entries can either be predefined values (e.g. '#cn' or '#uid')" .
								"\n# or individual ones (e.g. 'cn:Host Name')\n# values have to be seperated by semicolons\n" . "hostlistAttributes: " . $this->hostlistAttributes . "\n");
			if (!in_array("maxlistentries", $saved)) array_push($file_array, "\n\n# maximum number of rows to show in user/group/host lists\n" . "maxlistentries: " . $this->maxlistentries . "\n");
			if (!in_array("defaultLanguage", $saved)) array_push($file_array, "\n\n# default language (a line from config/language)\n" . "defaultLanguage: " . $this->defaultLanguage . "\n");
			if (!in_array("scriptPath", $saved)) array_push($file_array, "\n\n# Path to external Script\n" . "scriptPath: " . $this->scriptPath . "\n");
			if (!in_array("scriptServer", $saved)) array_push($file_array, "\n\n# Server of external Script\n" . "scriptServer: " . $this->scriptServer . "\n");
			if (!in_array("samba3", $saved)) array_push($file_array, "\n\n# Set to \"yes\" only if you use the new Samba 3.x schema.\n" . "samba3: " . $this->samba3 . "\n");
			if (!in_array("cachetimeout", $saved)) array_push($file_array, "\n\n# Number of minutes LAM caches LDAP searches.\n" . "cacheTimeout: " . $this->cachetimeout . "\n");
			if (!in_array("pwdhash", $saved)) array_push($file_array, "\n\n# Password hash algorithm (CRYPT/MD5/SMD5/SHA/SSHA/PLAIN).\n" . "pwdhash: " . $this->pwdhash . "\n");
			$file = fopen($conffile, "w");
			if ($file) {
				for ($i = 0; $i < sizeof($file_array); $i++) fputs($file, $file_array[$i]);
				fclose($file);
				@chmod ($conffile, 0600);
			}
			else {
				StatusMessage("ERROR", "", _("Cannot open config file!") . " (" . $conffile . ")");
				exit;
			}
		}
	}

	// prints current preferences
	function printconf() {
		echo "<b>" . _("Server address") . ": </b>" . $this->ServerURL . "<br>";
		echo "<b>" . _("Password hash type") . ": </b>" . $this->pwdhash . "<br>";
		echo "<b>" . _("Cache timeout") . ": </b>" . $this->cachetimeout . "<br>";
		echo "<b>" . _("Samba 3.x schema") . ": </b>" . $this->samba3 . "<br>";
		echo "<b>" . _("UserSuffix") . ": </b>" . $this->usersuffix . "<br>";
		echo "<b>" . _("GroupSuffix") . ": </b>" . $this->groupsuffix . "<br>";
		echo "<b>" . _("HostSuffix") . ": </b>" . $this->hostsuffix . "<br>";
		echo "<b>" . _("DomainSuffix") . ": </b>" . $this->domainsuffix . "<br>";
		echo "<b>" . _("Minimum UID number") . ": </b>" . $this->MinUID . "<br>";
		echo "<b>" . _("Maximum UID number") . ": </b>" . $this->MaxUID . "<br>";
		echo "<b>" . _("Minimum GID number") . ": </b>" . $this->MinGID . "<br>";
		echo "<b>" . _("Maximum GID number") . ": </b>" . $this->MaxGID . "<br>";
		echo "<b>" . _("Minimum Machine number") . ": </b>" . $this->MinMachine . "<br>";
		echo "<b>" . _("Maximum Machine number") . ": </b>" . $this->MaxMachine . "<br>";
		echo "<b>" . _("Attributes in User List") . ": </b>" . $this->userlistAttributes . "<br>";
		echo "<b>" . _("Attributes in Group List") . ": </b>" . $this->grouplistAttributes . "<br>";
		echo "<b>" . _("Attributes in Host List") . ": </b>" . $this->hostlistAttributes . "<br>";
		echo "<b>" . _("Maximum list entries") . ": </b>" . $this->maxlistentries . "<br>";
		echo "<b>" . _("Default language") . ": </b>" . $this->defaultLanguage . "<br>";
		echo "<b>" . _("Path to external script") . ": </b>" . $this->scriptPath . "<br>";
		echo "<b>" . _("Server of external script") . ": </b>" . $this->scriptServer . "<br>";
		echo "<b>" . _("List of valid users") . ": </b>" . $this->Admins;
	}

	// functions to read/write preferences

	// returns the server address as string
	function get_ServerURL() {
		return $this->ServerURL;
	}

	// sets the server address
	// $value: new server address
	// returns true if $value has correct format
	function set_ServerURL($value) {
		if (is_string($value)) $this->ServerURL = $value;
		else return false;
		return true;
	}

	// returns an array of string with all admin names
	function get_Admins() {
		return explode(";", $this->Admins);
	}

	// returns all admin users seperated by semicolons
	function get_Adminstring() {
		return $this->Admins;
	}

	// needs a string that contains all admin users seperated by semicolons
	// $value: new admin string
	// returns true if $value has correct format
	function set_Adminstring($value) {
		if (is_string($value) &&
		eregi("^[a-z0-9\\-]+=[a-z0-9\\-]+(,[a-z0-9\\-]+=[a-z0-9 \\-]+)+(;[a-z0-9\\-]+=[a-z0-9\\-]+(,[a-z0-9\\-]+=[a-z0-9 \\-]+)+)*$", $value)) {
			$this->Admins = $value;
		}
		else return false;
		return true;
	}

	// returns the password to access the preferences wizard
	function get_Passwd() {
		return $this->Passwd;
	}

	// sets the preferences wizard password
	// $value: new password
	// returns true if $value has correct format
	function set_Passwd($value) {
		if (is_string($value)) $this->Passwd = $value;
		else return false;
		return true;
	}

	// returns the LDAP suffix where users are saved
	function get_UserSuffix() {
		return $this->usersuffix;
	}

	// sets the LDAP suffix where users are saved
	// $value: new user suffix
	// returns true if $value has correct format
	function set_UserSuffix($value) {
		if (is_string($value) && (eregi("^(([a-z0-9 \\-])*=([a-z0-9 \\-])*)(,([a-z0-9 \\-])*=([a-z0-9 \\-])*)*$", $value))) {
			$this->usersuffix = $value;
		}
		else return false;
		return true;
	}

	// returns the LDAP suffix where groups are saved
	function get_GroupSuffix() {
		return $this->groupsuffix;
	}

	// sets the LDAP suffix where groups are saved
	// $value: new group suffix
	// returns true if $value has correct format
	function set_GroupSuffix($value) {
		if (is_string($value) && (eregi("^(([a-z0-9 \\-])*=([a-z0-9 \\-])*)(,([a-z0-9 \\-])*=([a-z0-9 \\-])*)*$", $value))) {
			$this->groupsuffix = $value;
		}
		else return false;
		return true;
	}

	// returns the LDAP suffix where hosts are saved
	function get_HostSuffix() {
		return $this->hostsuffix;
	}

	// sets the LDAP suffix where hosts are saved
	// $value: new host suffix
	// returns true if $value has correct format
	function set_HostSuffix($value) {
		if (is_string($value) && (eregi("^(([a-z0-9 \\-])*=([a-z0-9 \\-])*)(,([a-z0-9 \\-])*=([a-z0-9 \\-])*)*$", $value))) {
		$this->hostsuffix = $value;
	}
		else return false;
		return true;
	}

	// returns the LDAP suffix where domains are saved
	function get_DomainSuffix() {
		return $this->domainsuffix;
	}

	// sets the LDAP suffix where domains are saved
	// $value: new domain suffix
	// returns true if $value has correct format
	function set_DomainSuffix($value) {
		if (!$value && ($this->get_Samba3() == "no")) $this->domainsuffix = "";
		elseif (is_string($value) && (eregi("^(([a-z0-9 \\-])*=([a-z0-9 \\-])*)(,([a-z0-9 \\-])*=([a-z0-9 \\-])*)*$", $value))) {
			$this->domainsuffix = $value;
		}
		else return false;
		return true;
	}

	// returns the minimum UID to use when creating new users
	function get_minUID() {
		return $this->MinUID;
	}

	// sets the minimum UID to use when creating new users
	// $value: new minimum UID number
	// returns true if $value has correct format
	function set_minUID($value) {
		if (is_numeric($value)) $this->MinUID = $value;
		else return false;
		return true;
	}

	// returns the maximum UID to use when creating new users
	function get_maxUID() {
		return $this->MaxUID;
	}

	// sets the maximum UID to use when creating new users
	// $value: new maximum UID number
	// returns true if $value has correct format
	function set_maxUID($value) {
		if (is_numeric($value)) $this->MaxUID = $value;
		else return false;
		return true;
	}

	// returns the minimum GID to use when creating new groups
	function get_minGID() {
		return $this->MinGID;
	}

	// sets the minimum GID to use when creating new groups
	// $value: new minimum GID number
	// returns true if $value has correct format
	function set_minGID($value) {
		if (is_numeric($value)) $this->MinGID = $value;
		else return false;
		return true;
	}

	// returns the maximum GID to use when creating new groups
	function get_maxGID() {
		return $this->MaxGID;
	}

	// sets the maximum GID to use when creating new groups
	// $value: new maximum GID number
	// returns true if $value has correct format
	function set_maxGID($value) {
		if (is_numeric($value)) $this->MaxGID = $value;
		else return false;
		return true;
	}

	// returns the minimum UID to use when creating new Samba hosts
	function get_minMachine() {
		return $this->MinMachine;
	}

	// sets the minimum UID to use when creating new Samba hosts
	// $value: new minimum machine number
	// returns true if $value has correct format
	function set_minMachine($value) {
		if (is_numeric($value)) $this->MinMachine = $value;
		else return false;
		return true;
	}

	// returns the maximum UID to use when creating new Samba hosts
	function get_maxMachine() {
		return $this->MaxMachine;
	}

	// sets the maximum UID to use when creating new Samba hosts
	// $value: new maximum machine number
	// returns true if $value has correct format
	function set_maxMachine($value) {
		if (is_numeric($value)) $this->MaxMachine = $value;
		else return false;
		return true;
	}

	// returns the list of attributes to show in user list
	function get_userlistAttributes() {
		return $this->userlistAttributes;
	}

	// sets the list of attributes to show in user list
	// $value: new attribute string
	// returns true if $value has correct format
	function set_userlistAttributes($value) {
		if (is_string($value) && eregi("^((#[a-z]+)|([a-z]*:[a-z_\\-]+))(;((#[a-z]+)|([a-z]*:[a-z_\\-]+)))*$", $value)) {
			$this->userlistAttributes = $value;
		}
		else return false;
		return true;
	}

	// returns the list of attributes to show in group list
	function get_grouplistAttributes() {
		return $this->grouplistAttributes;
	}

	// sets the list of attributes to show in group list
	// $value: new attribute string
	// returns true if $value has correct format
	function set_grouplistAttributes($value) {
		if (is_string($value) && eregi("^((#[a-z]+)|([a-z]*:[a-z_\\-]+))(;((#[a-z]+)|([a-z]*:[a-z_\\-]+)))*$", $value)) {
			$this->grouplistAttributes = $value;
		}
		else return false;
		return true;
	}

	// returns the list of attributes to show in host list
	function get_hostlistAttributes() {
		return $this->hostlistAttributes;
	}

	// sets the list of attributes to show in host list
	// $value: new attribute string
	// returns true if $value has correct format
	function set_hostlistAttributes($value) {
		if (is_string($value) && eregi("^((#[a-z]+)|([a-z]*:[a-z_\\-]+))(;((#[a-z]+)|([a-z]*:[a-z_\\-]+)))*$", $value)) {
			$this->hostlistAttributes = $value;
		}
		else return false;
		return true;
	}

	// returns the maximum number of rows in user/group/host lists
	function get_MaxListEntries() {
		return $this->maxlistentries;
	}

	// sets the maximum number of rows in user/group/host lists
	// $value: new attribute string
	// returns true if $value has correct format
	function set_MaxListEntries ($value) {
		if (is_numeric($value)) $this->maxlistentries = $value;
		else return false;
		return true;
	}

	// returns the default language string
	function get_defaultLanguage() {
		return $this->defaultLanguage;
	}

	// sets the default language string
	// $value: new default language
	// returns true if $value has correct format
	function set_defaultLanguage($value) {
		if (is_string($value)) $this->defaultLanguage = $value;
		else return false;
		return true;
	}

	// returns the path to the external script
	function get_scriptPath() {
		return $this->scriptPath;
	}

	// sets the path to the external script
	// $value: new script path
	// returns true if $value has correct format
	function set_scriptPath($value) {
		if (!$value) $this->scriptPath = ""; // optional parameter
		elseif (is_string($value) && eregi("^/([a-z0-9_\\-])+(/([a-z0-9_\\.\\-])+)+$", $value)) $this->scriptPath = $value;
		else return false;
		return true;
	}

	// returns the server of the external script
	function get_scriptServer() {
		return $this->scriptServer;
	}

	// sets the server of the external script
	// $value: new script server
	// returns true if $value has correct format
	function set_scriptServer($value) {
		if (!$value) $this->scriptServer = ""; // optional parameter
		elseif (is_string($value) && eregi("^[a-z0-9\\-]+(\\.[a-z0-9\\-]+)*$", $value)) {
			$this->scriptServer = $value;
		}
		else return false;
		return true;
	}

	// returns "yes" if Samba 3.x schema is used, otherwise "no"
	function get_samba3() {
		return $this->samba3;
	}

	// returns true if Samba 3, else false
	function is_samba3() {
		if ($this->samba3 == "yes") return true;
		else return false;
	}

	// set Samba version: "yes" means 3.x schema, "no" means 2.2.x schema
	// $value: "yes"/"no"
	// returns true if $value has correct format
	function set_samba3($value) {
		if (is_string($value) && eregi("^(yes|no)$", $value)) {
			$this->samba3 = $value;
		}
		else return false;
		return true;
	}

	// returns the LDAP cache timeout in minutes
	function get_cacheTimeout() {
		if (isset($this->cachetimeout)) return $this->cachetimeout;
		else return 5;
	}

	// returns the LDAP cache timeout in seconds
	function get_cacheTimeoutSec() {
		return $this->cachetimeout * 60;
	}

	// sets the LDAP cache timeout in minutes (0,1,2,5,10,15)
	// $value: new cache timeout
	// returns true if $value has correct format
	function set_cacheTimeout($value) {
		if (is_numeric($value) && ($value > -1)) {
			$this->cachetimeout = $value;
		}
		else return false;
		return true;
	}

	// returns the password hash type
	function get_pwdhash() {
		if ($this->pwdhash) return strtoupper($this->pwdhash);
		else return "SSHA";
	}

	// set the password hash type (CRYPT/SHA/SSHA/MD5/SMD5)
	// $value: new password hash algorithm
	// returns true if $value has correct format
	function set_pwdhash($value) {
		if (is_string($value) && eregi("^(crypt|sha|ssha|md5|smd5|plain)$", $value)) {
			$this->pwdhash = $value;
		}
		else return false;
		return true;
	}

}

/*
// class CfgMain
*/
// manages config.cfg
class CfgMain {

	// default profile
	var $default;

	// password to change config.cfg
	var $password;

	// constructor, loads preferences from config file
	function CfgMain() {
		$this->reload();
	}

	// reloads preferences from config file config.cfg
	function reload() {
		$conffile = substr(__FILE__, 0, strlen(__FILE__) - 15) . "/config/config.cfg";
		if (is_file($conffile) == True) {
			$file = fopen($conffile, "r");
			while (!feof($file)) {
				$line = fgets($file, 1024);
				$line = trim($line);  // remove spaces at the beginning and end
				if (($line == "")||($line[0] == "#")) continue; // ignore comments
				// search keywords
				if (substr($line, 0, 10) == "password: ") {
					$this->password = substr($line, 10, strlen($line)-10);
					continue;
				}
				if (substr($line, 0, 9) == "default: ") {
					$this->default = substr($line, 9, strlen($line)-9);
					continue;
				}
			}
			fclose($file);
		}
		else {
			StatusMessage("ERROR", "", _("Unable to load configuration!") . " (" . $conffile . ")");
		}
	}

	// saves preferences to config file config.cfg
	function save() {
		$conffile = substr(__FILE__, 0, strlen(__FILE__) - 15) . "/config/config.cfg";
		if (is_file($conffile) == True) {
			// booleans to check if value was already saved
			$save_password = $save_default = False;
			$file = fopen($conffile, "r");
			$file_array = array();
			// read config file
			while (!feof($file)) {
				array_push($file_array, fgets($file, 1024));
			}
			fclose($file);
			// generate new configuration file
			for ($i = 0; $i < sizeof($file_array); $i++) {
				if (($file_array[$i] == "\n")||($file_array[$i][0] == "#")) continue; // ignore comments
				// search for keywords
				if (substr($file_array[$i], 0, 10) == "password: ") {
					$file_array[$i] = "password: " . $this->password . "\n";
					$save_password = True;
					continue;
				}
				if (substr($file_array[$i], 0, 9) == "default: ") {
					$file_array[$i] = "default: " . $this->default . "\n";
					$save_default = True;
					continue;
				}
			}
		}
		// check if we have to add new entries (e.g. if user upgraded LAM and has an old config file)
		if (!$save_password == True) array_push($file_array, "\n\n# password to add/delete/rename configuration profiles\n" . "password: " . $this->password);
		if (!$save_default == True) array_push($file_array, "\n\n# default profile, without \".conf\"\n" . "default: " . $this->default);
		$file = fopen($conffile, "w");
		if ($file) {
			for ($i = 0; $i < sizeof($file_array); $i++) fputs($file, $file_array[$i]);
			fclose($file);
		}
		else {
			StatusMessage("ERROR", "", _("Cannot open config file!") . " (" . $conffile . ")");
			exit;
		}
	}

}

?>
