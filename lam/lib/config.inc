<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003 - 2004  Roland Gruber

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
include_once("modules.inc");

/**
* sets language settings for automatic translation
*/
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

/**
* returns an array of string with all available configuration profiles (without .conf)
*/
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

/**
* print meta refresh
*
* @param $page the URL of the target page
*/
function metaRefresh($page) {
	echo $_SESSION['header'];
	echo "<meta http-equiv=\"refresh\" content=\"0; URL=" . $page . "\">\n";
	echo "<title></title>\n";
	echo "</head>\n";
	echo "<body>\n";
	// print link if refresh does not work
	echo "<p>\n";
	echo "<a href=\"" . $page . "\">" . _("Click here if you are not directed to the next page.") . "</a>\n";
	echo "</p>\n";
	echo "</body>\n";
	echo "</html>\n";
}


/**
* This class manages .conf files.
*/
class Config {

	/** server address (e.g. ldap://127.0.0.1:389) */
	var $ServerURL;

	/** array of strings: users with admin rights */
	var $Admins;

	/** string: password to edit preferences */
	var $Passwd;

	/** suffix for users */
	var $usersuffix;

	/** suffix for groups */
	var $groupsuffix;

	/** suffix for Samba hosts */
	var $hostsuffix;

	/** suffix for domains (Samba 3) */
	var $domainsuffix;

	/** minimum UID number for users */
	var $MinUID;
	/** maximum UID number for users */
	var $MaxUID;
	/** minimum GID number for groups */
	var $MinGID;
	/** maximum GID number for groups */
	var $MaxGID;
	/** minimum UID number for Samba hosts */
	var $MinMachine;
	/** maximum UID number for Samba hosts */
	var $MaxMachine;

	/** attributes that are shown in the user list */
	var $userlistAttributes;
	/** attributes that are shown in the group list */
	var $grouplistAttributes;
	/** attributes that are shown in the host list */
	var $hostlistAttributes;

	/** maximum number of rows shown in user/group/host lists */
	var $maxlistentries;

	/** default language */
	var $defaultLanguage;

	/**
	* Path to external lamdaemon script on server where it is executed
	* used for managing quota and home directories
	* optional setting, may not be defined
	*/
	var $scriptPath;

	/**
	* server where lamdaemon script is executed
	* used for managing quota and home directories
	* optional setting, may not be defined
	*/
	var $scriptServer;

	/** LDAP cache timeout */
	var $cachetimeout;

	/** password hash algorithm */
	var $pwdhash;

	/** text to include in user PDF files */
	var $pdftext = "";

	/** account modules
	** TODO add default modules for LAM <0.5 
	*/
	var $usermodules = "um1,um2,um3";
	var $groupmodules = "gm1,gm2,gm3";
	var $hostmodules = "hm1,hm2,hm3";

	/** name of configuration file */
	var $file;

	/** list of all settings in config file */
	var $settings = array("ServerURL", "Passwd", "Admins", "usersuffix", "groupsuffix", "hostsuffix",
		"domainsuffix", "MinUID", "MaxUID", "MinGID", "MaxGID", "MinMachine", "MaxMachine",
		"userlistAttributes", "grouplistAttributes", "hostlistAttributes", "maxlistentries",
		"defaultLanguage", "scriptPath", "scriptServer", "cachetimeout", "pwdhash",
		"usermodules", "groupmodules", "hostmodules");


	/**
	* constructor, loads preferences from config file
	*
	* @param $file Index number in config file array
	*/
	function Config($file = 0) {
		// load first profile if none is given
		if (!is_string($file)) {
			$profiles = getConfigProfiles();
			$file = $profiles[0];
		}
		$this->file = $file;
		$this->reload();
	}

	/** reloads preferences from config file */
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
		// load user PDF text
		$pdffile = substr(__FILE__, 0, strlen(__FILE__) - 15) . "/config/pdf/" . $this->file . ".txt";
		if (is_file($pdffile) == True) {
			$pdfstring = @file($pdffile);
			$pdfstring = @implode("", $pdfstring);
			if ($pdfstring) $this->pdftext = $pdfstring;
			else $this->pdftext = "";
		}
	}

	/** saves preferences to config file */
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
			if (!in_array("cachetimeout", $saved)) array_push($file_array, "\n\n# Number of minutes LAM caches LDAP searches.\n" . "cacheTimeout: " . $this->cachetimeout . "\n");
			if (!in_array("pwdhash", $saved)) array_push($file_array, "\n\n# Password hash algorithm (CRYPT/MD5/SMD5/SHA/SSHA/PLAIN).\n" . "pwdhash: " . $this->pwdhash . "\n");
			if (!in_array("usermodules", $saved)) array_push($file_array, "\n\n# List of used user modules\n" . "usermodules: " . $this->usermodules . "\n");
			if (!in_array("groupmodules", $saved)) array_push($file_array, "\n\n# List of used group modules\n" . "groupmodules: " . $this->groupmodules . "\n");
			if (!in_array("hostmodules", $saved)) array_push($file_array, "\n\n# List of used host modules\n" . "hostmodules: " . $this->hostmodules . "\n");
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

	/** prints current preferences */
	function printconf() {
		echo "<b>" . _("Server address") . ": </b>" . $this->ServerURL . "<br>";
		echo "<b>" . _("Password hash type") . ": </b>" . $this->pwdhash . "<br>";
		echo "<b>" . _("Cache timeout") . ": </b>" . $this->cachetimeout . "<br>";
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
		echo "<b>" . _("List of valid users") . ": </b>" . $this->Admins . "<br>";
		echo "<b>" . _("User modules") . ": </b>" . $this->usermodules . "<br>";
		echo "<b>" . _("Group modules") . ": </b>" . $this->groupmodules . "<br>";
		echo "<b>" . _("Host modules") . ": </b>" . $this->hostmodules . "<br><br>";
		echo "<b>" . _("Text for user PDF") . ": </b>" . $this->get_pdftext();
	}

	// functions to read/write preferences

	/**
	* returns the server address as string
	*
	* @return server address
	*/
	function get_ServerURL() {
		return $this->ServerURL;
	}

	/**
	* sets the server address
	*
	* @param $value new server address
	* @return true if $value has correct format
	*/
	function set_ServerURL($value) {
		if (is_string($value)) $this->ServerURL = $value;
		else return false;
		return true;
	}

	/**
	* returns an array of string with all admin names
	*
	* @return the admin names
	*/
	function get_Admins() {
		return explode(";", $this->Admins);
	}

	/**
	* returns all admin users seperated by semicolons
	*
	* @return the admin string
	*/
	function get_Adminstring() {
		return $this->Admins;
	}

	/**
	* sets the admin string
	*
	* @param $value new admin string that contains all admin users seperated by semicolons
	* @return true if $value has correct format
	*/
	function set_Adminstring($value) {
		if (is_string($value) &&
		eregi("^[a-z0-9\\-]+=[a-z0-9\\-]+(,[a-z0-9\\-]+=[a-z0-9 \\-]+)+(;[a-z0-9\\-]+=[a-z0-9\\-]+(,[a-z0-9\\-]+=[a-z0-9 \\-]+)+)*$", $value)) {
			$this->Admins = $value;
		}
		else return false;
		return true;
	}

	/**
	* returns the password to access the preferences wizard
	*
	* @return the password
	*/
	function get_Passwd() {
		return $this->Passwd;
	}

	/**
	* sets the preferences wizard password
	*
	* @param $value new password
	* @return true if $value has correct format
	*/
	function set_Passwd($value) {
		if (is_string($value)) $this->Passwd = $value;
		else return false;
		return true;
	}

	/**
	* returns the LDAP suffix where users are saved
	*
	* @return the user suffix
	*/
	function get_UserSuffix() {
		return $this->usersuffix;
	}

	/**
	* sets the LDAP suffix where users are saved
	*
	* @param $value new user suffix
	* @return true if $value has correct format
	*/
	function set_UserSuffix($value) {
		if (is_string($value) && (eregi("^(([a-z0-9 \\-])*=([a-z0-9 \\-])*)(,([a-z0-9 \\-])*=([a-z0-9 \\-])*)*$", $value))) {
			$this->usersuffix = $value;
		}
		else return false;
		return true;
	}

	/**
	* returns the LDAP suffix where groups are saved
	*
	* @return the group suffix
	*/
	function get_GroupSuffix() {
		return $this->groupsuffix;
	}

	/**
	* sets the LDAP suffix where groups are saved
	*
	* @param $value new group suffix
	* @return true if $value has correct format
	*/
	function set_GroupSuffix($value) {
		if (is_string($value) && (eregi("^(([a-z0-9 \\-])*=([a-z0-9 \\-])*)(,([a-z0-9 \\-])*=([a-z0-9 \\-])*)*$", $value))) {
			$this->groupsuffix = $value;
		}
		else return false;
		return true;
	}

	/**
	* returns the LDAP suffix where hosts are saved
	*
	* @return the host suffix
	*/
	function get_HostSuffix() {
		return $this->hostsuffix;
	}

	/**
	* sets the LDAP suffix where hosts are saved
	*
	* @param $value new host suffix
	* @return true if $value has correct format
	*/
	function set_HostSuffix($value) {
		if (! $value) $this->hostsuffix = "";
		elseif (is_string($value) && (eregi("^(([a-z0-9 \\-])*=([a-z0-9 \\-])*)(,([a-z0-9 \\-])*=([a-z0-9 \\-])*)*$", $value))) {
		$this->hostsuffix = $value;
	}
		else return false;
		return true;
	}

	/**
	* returns the LDAP suffix where domains are saved
	*
	* @return the domain suffix
	*/
	function get_DomainSuffix() {
		return $this->domainsuffix;
	}

	/**
	* sets the LDAP suffix where domains are saved
	*
	* @param $value new domain suffix
	* @return true if $value has correct format
	*/
	function set_DomainSuffix($value) {
		if (!$value && ($this->get_Samba3() == "no")) $this->domainsuffix = "";
		elseif (is_string($value) && (eregi("^(([a-z0-9 \\-])*=([a-z0-9 \\-])*)(,([a-z0-9 \\-])*=([a-z0-9 \\-])*)*$", $value))) {
			$this->domainsuffix = $value;
		}
		else return false;
		return true;
	}

	/**
	* returns the minimum UID to use when creating new users
	*
	* @return the minimum UID number
	*/
	function get_minUID() {
		return $this->MinUID;
	}

	/**
	* sets the minimum UID to use when creating new users
	*
	* @param $value new minimum UID number
	* @return true if $value has correct format
	*/
	function set_minUID($value) {
		if (is_numeric($value)) $this->MinUID = $value;
		else return false;
		return true;
	}

	/**
	* returns the maximum UID to use when creating new users
	*
	* @return the maximum UID number
	*/
	function get_maxUID() {
		return $this->MaxUID;
	}

	/**
	* sets the maximum UID to use when creating new users
	*
	* @param $value new maximum UID number
	* @return true if $value has correct format
	*/
	function set_maxUID($value) {
		if (is_numeric($value)) $this->MaxUID = $value;
		else return false;
		return true;
	}

	/**
	* returns the minimum GID to use when creating new groups
	*
	* @return the minimum GID number
	*/
	function get_minGID() {
		return $this->MinGID;
	}

	/**
	* sets the minimum GID to use when creating new groups
	*
	* @param $value new minimum GID number
	* @return true if $value has correct format
	*/
	function set_minGID($value) {
		if (is_numeric($value)) $this->MinGID = $value;
		else return false;
		return true;
	}

	/**
	* returns the maximum GID to use when creating new groups
	*
	* @return the maximum GID number
	*/
	function get_maxGID() {
		return $this->MaxGID;
	}

	/**
	* sets the maximum GID to use when creating new groups
	*
	* @param $value new maximum GID number
	* @return true if $value has correct format
	*/
	function set_maxGID($value) {
		if (is_numeric($value)) $this->MaxGID = $value;
		else return false;
		return true;
	}

	/**
	* returns the minimum UID to use when creating new Samba hosts
	*
	* @return the minimum UID number
	*/
	function get_minMachine() {
		return $this->MinMachine;
	}

	/**
	* sets the minimum UID to use when creating new Samba hosts
	*
	* @param $value new minimum UID number
	* @return true if $value has correct format
	*/
	function set_minMachine($value) {
		if (! $value && ($this->hostsuffix == "")) $this->MinMachine = "";
		elseif (is_numeric($value)) $this->MinMachine = $value;
		else return false;
		return true;
	}

	/**
	* returns the maximum UID to use when creating new Samba hosts
	*
	* @return the maximum UID number
	*/
	function get_maxMachine() {
		return $this->MaxMachine;
	}

	/**
	* sets the maximum UID to use when creating new Samba hosts
	*
	* @param $value new maximum UID number
	* @return true if $value has correct format
	*/
	function set_maxMachine($value) {
		if (! $value && ($this->hostsuffix == "")) $this->MaxMachine = "";
		elseif (is_numeric($value)) $this->MaxMachine = $value;
		else return false;
		return true;
	}

	/**
	* returns the list of attributes to show in user list
	*
	* @return the attribute list
	*/
	function get_userlistAttributes() {
		return $this->userlistAttributes;
	}

	/**
	* sets the list of attributes to show in user list
	*
	* @param $value new attribute string
	* @return true if $value has correct format
	*/
	function set_userlistAttributes($value) {
		if (is_string($value) && eregi("^((#[^:;]+)|([^:;]*:[^:;]+))(;((#[^:;]+)|([^:;]*:[^:;]+)))*$", $value)) {
			$this->userlistAttributes = $value;
		}
		else return false;
		return true;
	}

	/**
	* returns the list of attributes to show in group list
	*
	* @return the attribute list
	*/
	function get_grouplistAttributes() {
		return $this->grouplistAttributes;
	}

	/**
	* sets the list of attributes to show in group list
	*
	* @param $value new attribute string
	* @return true if $value has correct format
	*/
	function set_grouplistAttributes($value) {
		if (is_string($value) && eregi("^((#[^:;]+)|([^:;]*:[^:;]+))(;((#[^:;]+)|([^:;]*:[^:;]+)))*$", $value)) {
			$this->grouplistAttributes = $value;
		}
		else return false;
		return true;
	}

	/**
	* returns the list of attributes to show in host list
	*
	* @return the attribute list
	*/
	function get_hostlistAttributes() {
		return $this->hostlistAttributes;
	}

	/**
	* sets the list of attributes to show in host list
	*
	* @param $value new attribute string
	* @return true if $value has correct format
	*/
	function set_hostlistAttributes($value) {
		if (! $value && ($this->hostsuffix == "")) $this->hostlistAttributes = "";
		elseif (is_string($value) && eregi("^((#[^:;]+)|([^:;]*:[^:;]+))(;((#[^:;]+)|([^:;]*:[^:;]+)))*$", $value)) {
			$this->hostlistAttributes = $value;
		}
		else return false;
		return true;
	}

	/**
	* returns the maximum number of rows in user/group/host lists
	*
	* @return maximum number
	*/
	function get_MaxListEntries() {
		return $this->maxlistentries;
	}

	/**
	* sets the maximum number of rows in user/group/host lists
	*
	* @param $value new attribute string
	* @return true if $value has correct format
	*/
	function set_MaxListEntries ($value) {
		if (is_numeric($value)) $this->maxlistentries = $value;
		else return false;
		return true;
	}

	/**
	* returns the default language string
	*
	* @return default language
	*/
	function get_defaultLanguage() {
		return $this->defaultLanguage;
	}

	/**
	* sets the default language string
	*
	* @param $value new default language
	* @return true if $value has correct format
	*/
	function set_defaultLanguage($value) {
		if (is_string($value)) $this->defaultLanguage = $value;
		else return false;
		return true;
	}

	/**
	* returns the path to the external script
	*
	* @return script path
	*/
	function get_scriptPath() {
		return $this->scriptPath;
	}

	/**
	* sets the path to the external script
	*
	* @param $value new script path
	* @return true if $value has correct format
	*/
	function set_scriptPath($value) {
		if (!$value) $this->scriptPath = ""; // optional parameter
		elseif (is_string($value) && eregi("^/([a-z0-9_\\-])+(/([a-z0-9_\\.\\-])+)+$", $value)) $this->scriptPath = $value;
		else return false;
		return true;
	}

	/**
	* returns the server of the external script
	*
	* @return script server
	*/
	function get_scriptServer() {
		return $this->scriptServer;
	}

	/**
	* sets the server of the external script
	*
	* @param $value new script server
	* @return true if $value has correct format
	*/
	function set_scriptServer($value) {
		if (!$value) $this->scriptServer = ""; // optional parameter
		elseif (is_string($value) && eregi("^[a-z0-9\\-]+(\\.[a-z0-9\\-]+)*$", $value)) {
			$this->scriptServer = $value;
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

	// returns text for user PDF files
	function get_pdftext($name = "User") {
		if ($this->pdftext) return $this->pdftext;
		else return "";
	}

	// set the text for user PDF files
	// $value: string containing the text
	// returns true if $value has correct format and could be saved
	function set_pdftext($value) {
		// check if text changed
		if ($value == $this->pdftext) return true;
		if (is_string($value)) {
			// save to file
			$pdffile = substr(__FILE__, 0, strlen(__FILE__) - 15) . "/config/pdf/" . $this->file . ".txt";
			$file = @fopen($pdffile, "w");
			if ($file) {
				@fputs($file, $value);
				fclose($file);
				$this->pdftext = $value;
			}
			else return false;
			$this->pdftext = $value;
		}
		else return false;
		return true;
	}

	// returns an array of all selected user modules
	function get_UserModules() {
		$modules = explode(",", $this->usermodules);
		$available = getAvailableModules('user');
		// only return available modules
		$ret = array();
		for ($i = 0; $i < sizeof($modules); $i++) {
			if (in_array($modules[$i], $available)) $ret[] = $modules[$i];
		}
		return $ret;
	}

	// sets the selected user modules
	// $modules: array with module names (not aliases!)
	// returns true if $modules has correct format
	function set_UserModules($modules) {
		if (! is_array($modules)) return false;
		// check module names
		$available = getAvailableModules('user');
		for ($i = 0; $i < sizeof($modules); $i++) {
			if (! in_array($modules[$i], $available)) return false;
		}
		// TODO: check depends/conflicts
		$this->usermodules = implode(",", $modules);
		return true;
	}

	// returns an array of all selected group modules
	function get_GroupModules() {
		$modules = explode(",", $this->groupmodules);
		$available = getAvailableModules('group');
		// only return available modules
		$ret = array();
		for ($i = 0; $i < sizeof($modules); $i++) {
			if (in_array($modules[$i], $available)) $ret[] = $modules[$i];
		}
		return $ret;
	}

	// sets the selected group modules
	// $modules: array with module names (not aliases!)
	// returns true if $modules has correct format
	function set_GroupModules($modules) {
		if (! is_array($modules)) return false;
		// check module names
		$available = getAvailableModules('group');
		for ($i = 0; $i < sizeof($modules); $i++) {
			if (! in_array($modules[$i], $available)) return false;
		}
		// TODO: check depends/conflicts
		$this->groupmodules = implode(",", $modules);
		return true;
	}

	// returns an array of all selected host modules
	function get_HostModules() {
		$modules = explode(",", $this->hostmodules);
		$available = getAvailableModules('host');
		// only return available modules
		$ret = array();
		for ($i = 0; $i < sizeof($modules); $i++) {
			if (in_array($modules[$i], $available)) $ret[] = $modules[$i];
		}
		return $ret;
	}

	// sets the selected host modules
	// $modules: array with module names (not aliases!)
	// returns true if $modules has correct format
	function set_HostModules($modules) {
		if (! is_array($modules)) return false;
		// check module names
		$available = getAvailableModules('host');
		for ($i = 0; $i < sizeof($modules); $i++) {
			if (! in_array($modules[$i], $available)) return false;
		}
		// TODO: check depends/conflicts
		$this->hostmodules = implode(",", $modules);
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
