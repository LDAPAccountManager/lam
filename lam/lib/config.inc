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
		putenv("LANG=" . $language[0]);
		setlocale(LC_ALL, $language[0]);
		$locdir = substr(__FILE__, 0, strlen(__FILE__) - 15) . "/locale";
		bindtextdomain("messages", $locdir);
		textdomain("messages");
	}
	else echo _("Language not defined in session!");
}

// returns an array of String with all available configuration profiles (without .conf)
function getConfigProfiles() {
	$dir = dir(substr(__FILE__, 0, strlen(__FILE__) - 15) . "/config");
	$ret = array();
	$pos = 0;
	while ($entry = $dir->read()){
		$ext = substr($entry, strlen($entry)-5, 5);
		$name = substr($entry, 0, strlen($entry)-5);
		if ($ext == ".conf") {
			$ret[$pos] = $name;
			$pos ++;
		}
	}
	sort($ret);
	return $ret;
}

// print meta refresh
// $page is the target page
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

  // single line with the names of all admin users
  var $Adminstring;

  // suffix for users
  var $Suff_users;

  // suffix for groups
  var $Suff_groups;

  // suffix for Samba hosts
  var $Suff_hosts;

  // suffix for domains (Samba 3)
  var $Suff_domains;

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
  // optional settings, may not be defined
  var $scriptPath;
  var $scriptServer;

  // if "yes" use the new LDAP schema for Samba 3.x
  var $samba3;

  // LDAP cache timeout
  var $cachetimeout;

  // name of configuration file
  var $file;

  // constructor, loads preferences from config file
  function Config($file=0) {
    if (!is_string($file)) {
		$profiles = getConfigProfiles();
		$file = $profiles[0];
	}
	$this->file = $file;
    $this->reload();
  }

	// reloads preferences from config file
	function reload() {
		// list of all settings in config file
		$settings = array("ServerURL", "Passwd", "Adminstring", "usersuffix", "groupsuffix", "hostsuffix",
			"domainsuffix", "minuid", "maxuid", "mingid", "maxgid", "minmachine", "maxmachine",
			"userlistattributes", "grouplistattributes", "hostlistattributes", "maxlistentries",
			"defaultlanguage", "scriptpath", "scriptServer", "samba3", "cachetimeout");
		$conffile = substr(__FILE__, 0, strlen(__FILE__) - 15) . "/config/" . $this->file . ".conf";
		if (is_file($conffile) == True) {
			$file = fopen($conffile, "r");
			while (!feof($file)) {
				$line = fgets($file, 1024);
				if (($line == "\n")||($line[0] == "#")) continue; // ignore comments
				// search keywords
				if (strtolower(substr($line, 0, 11)) == "serverurl: ") {
				$this->ServerURL = chop(substr($line, 11, strlen($line)-11));
				continue;
				}
				if (strtolower(substr($line, 0, 8)) == "passwd: ") {
				$this->Passwd = chop(substr($line, 8, strlen($line)-8));
				continue;
				}
				if (strtolower(substr($line, 0, 8)) == "admins: ") {
				$adminstr = chop(substr($line, 8, strlen($line)-8));
				$this->Adminstring = $adminstr;
				$this->Admins = explode(";", $adminstr);
				continue;
				}
				if (strtolower(substr($line, 0, 12)) == "usersuffix: ") {
				$this->Suff_users = chop(substr($line, 12, strlen($line)-12));
				continue;
				}
				if (strtolower(substr($line, 0, 13)) == "groupsuffix: ") {
				$this->Suff_groups = chop(substr($line, 13, strlen($line)-13));
				continue;
				}
				if (strtolower(substr($line, 0, 12)) == "hostsuffix: ") {
				$this->Suff_hosts = chop(substr($line, 12, strlen($line)-12));
				continue;
				}
				if (strtolower(substr($line, 0, 14)) == "domainsuffix: ") {
				$this->Suff_domains = chop(substr($line, 14, strlen($line)-14));
				continue;
				}
				if (strtolower(substr($line, 0, 8)) == "minuid: ") {
				$this->MinUID = chop(substr($line, 8, strlen($line)-8));
				continue;
				}
				if (strtolower(substr($line, 0, 8)) == "maxuid: ") {
				$this->MaxUID = chop(substr($line, 8, strlen($line)-8));
				continue;
				}
				if (strtolower(substr($line, 0, 8)) == "mingid: ") {
				$this->MinGID = chop(substr($line, 8, strlen($line)-8));
				continue;
				}
				if (strtolower(substr($line, 0, 8)) == "maxgid: ") {
				$this->MaxGID = chop(substr($line, 8, strlen($line)-8));
				continue;
				}
				if (strtolower(substr($line, 0, 12)) == "minmachine: ") {
				$this->MinMachine = chop(substr($line, 12, strlen($line)-12));
				continue;
				}
				if (strtolower(substr($line, 0, 12)) == "maxmachine: ") {
				$this->MaxMachine = chop(substr($line, 12, strlen($line)-12));
				continue;
				}
				if (strtolower(substr($line, 0, 20)) == "userlistattributes: ") {
				$this->userlistAttributes = chop(substr($line, 20, strlen($line)-20));
				continue;
				}
				if (strtolower(substr($line, 0, 21)) == "grouplistattributes: ") {
				$this->grouplistAttributes = chop(substr($line, 21, strlen($line)-21));
				continue;
				}
				if (strtolower(substr($line, 0, 20)) == "hostlistattributes: ") {
				$this->hostlistAttributes = chop(substr($line, 20, strlen($line)-20));
				continue;
				}
				if (strtolower(substr($line, 0, 16)) == "maxlistentries: ") {
				$this->maxlistentries = chop(substr($line, 16, strlen($line)-16));
				continue;
				}
				if (strtolower(substr($line, 0, 17)) == "defaultlanguage: ") {
				$this->defaultLanguage = chop(substr($line, 17, strlen($line)-17));
				continue;
				}
				if (strtolower(substr($line, 0, 12)) == "scriptpath: ") {
				$this->scriptPath = chop(substr($line, 12, strlen($line)-12));
				continue;
				}
				if (strtolower(substr($line, 0, 14)) == "scriptserver: ") {
				$this->scriptServer = chop(substr($line, 14, strlen($line)-14));
				continue;
				}
				if (strtolower(substr($line, 0, 8)) == "samba3: ") {
				$this->samba3 = strtolower(chop(substr($line, 8, strlen($line)-8)));
				continue;
				}
				if (strtolower(substr($line, 0, 14)) == "cachetimeout: ") {
				$this->cachetimeout = chop(substr($line, 14, strlen($line)-14));
				continue;
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
			// booleans to check if value was already saved
			$save_serverURL = $save_passwd = $save_admins = $save_suffusr = $save_suffgrp = $save_suffhst =
			$save_minUID = $save_maxUID = $save_minGID = $save_maxGID = $save_minMach = $save_maxMach =
			$save_usrlstatrr =  $save_grplstatrr = $save_hstlstatrr = $save_maxlstent = $save_deflang =
			$save_scriptPath = $save_scriptServer = $save_samba3 = $save_suffdom = $save_cachetime = False;
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
				if (strtolower(substr($file_array[$i], 0, 11)) == "serverurl: ") {
					$file_array[$i] = "serverURL: " . $this->ServerURL . "\n";
					$save_serverURL = True;
					continue;
				}
				if (strtolower(substr($file_array[$i], 0, 8)) == "passwd: ") {
					$file_array[$i] = "passwd: " . $this->Passwd . "\n";
					$save_passwd = True;
					continue;
				}
				if (strtolower(substr($file_array[$i], 0, 8)) == "admins: ") {
					$file_array[$i] = "admins: " . $this->Adminstring . "\n";
					$save_admins = True;
					continue;
				}
				if (strtolower(substr($file_array[$i], 0, 12)) == "usersuffix: ") {
					$file_array[$i] = "usersuffix: " . $this->Suff_users . "\n";
					$save_suffusr = True;
					continue;
				}
				if (strtolower(substr($file_array[$i], 0, 13)) == "groupsuffix: ") {
					$file_array[$i] = "groupsuffix: " . $this->Suff_groups . "\n";
					$save_suffgrp = True;
					continue;
				}
				if (strtolower(substr($file_array[$i], 0, 12)) == "hostsuffix: ") {
					$file_array[$i] = "hostsuffix: " . $this->Suff_hosts . "\n";
					$save_suffhst = True;
					continue;
				}
				if (strtolower(substr($file_array[$i], 0, 14)) == "domainsuffix: ") {
					$file_array[$i] = "domainsuffix: " . $this->Suff_domains . "\n";
					$save_suffdom = True;
					continue;
				}
				if (strtolower(substr($file_array[$i], 0, 8)) == "minuid: ") {
					$file_array[$i] = "minUID: " . $this->MinUID . "\n";
					$save_minUID = True;
					continue;
				}
				if (strtolower(substr($file_array[$i], 0, 8)) == "maxuid: ") {
					$file_array[$i] = "maxUID: " . $this->MaxUID . "\n";
					$save_maxUID = True;
					continue;
				}
				if (strtolower(substr($file_array[$i], 0, 8)) == "mingid: ") {
					$file_array[$i] = "minGID: " . $this->MinGID . "\n";
					$save_minGID = True;
					continue;
				}
				if (strtolower(substr($file_array[$i], 0, 8)) == "maxgid: ") {
					$file_array[$i] = "maxGID: " . $this->MaxGID . "\n";
					$save_maxGID = True;
					continue;
				}
				if (strtolower(substr($file_array[$i], 0, 12)) == "minmachine: ") {
					$file_array[$i] = "minMachine: " . $this->MinMachine . "\n";
					$save_minMach = True;
					continue;
				}
				if (strtolower(substr($file_array[$i], 0, 12)) == "maxmachine: ") {
					$file_array[$i] = "maxMachine: " . $this->MaxMachine . "\n";
					$save_maxMach = True;
					continue;
				}
				if (strtolower(substr($file_array[$i], 0, 20)) == "userlistattributes: ") {
					$file_array[$i] = "userlistAttributes: " . $this->userlistAttributes . "\n";
					$save_usrlstattr = True;
					continue;
				}
				if (strtolower(substr($file_array[$i], 0, 21)) == "grouplistattributes: ") {
					$file_array[$i] = "grouplistAttributes: " . $this->grouplistAttributes . "\n";
					$save_grplstattr = True;
					continue;
				}
				if (strtolower(substr($file_array[$i], 0, 20)) == "hostlistattributes: ") {
					$file_array[$i] = "hostlistAttributes: " . $this->hostlistAttributes . "\n";
					$save_hstlstattr = True;
					continue;
				}
				if (strtolower(substr($file_array[$i], 0, 16)) == "maxlistentries: ") {
					$file_array[$i] = "maxlistentries: " . $this->maxlistentries . "\n";
					$save_maxlstent = True;
					continue;
				}
				if (strtolower(substr($file_array[$i], 0, 17)) == "defaultlanguage: ") {
					$file_array[$i] = "defaultLanguage: " . $this->defaultLanguage . "\n";
					$save_deflang = True;
					continue;
				}
				if (strtolower(substr($file_array[$i], 0, 12)) == "scriptpath: ") {
					$file_array[$i] = "scriptPath: " . $this->scriptPath . "\n";
					$save_scriptPath = True;
					continue;
				}
				if (strtolower(substr($file_array[$i], 0, 14)) == "scriptserver: ") {
					$file_array[$i] = "scriptServer: " . $this->scriptServer . "\n";
					$save_scriptServer = True;
					continue;
				}
				if (strtolower(substr($file_array[$i], 0, 8)) == "samba3: ") {
					$file_array[$i] = "samba3: " . $this->samba3 . "\n";
					$save_samba3 = True;
					continue;
				}
				if (strtolower(substr($file_array[$i], 0, 14)) == "cachetimeout: ") {
					$file_array[$i] = "cacheTimeout: " . $this->cachetimeout . "\n";
					$save_cachetime = True;
					continue;
				}
			}
			// check if we have to add new entries (e.g. if user upgraded LAM and has an old config file)
			if (!$save_serverURL == True) array_push($file_array, "\n\n# server address (e.g. ldap://localhost:389 or ldaps://localhost:636)\n" . "serverURL: " . $this->ServerURL . "\n");
			if (!$save_passwd == True) array_push($file_array, "\n\n# password to change these preferences via webfrontend\n" . "passwd: " . $this->Passwd . "\n");
			if (!$save_admins == True) array_push($file_array, "\n\n# list of users who are allowed to use LDAP Account Manager\n" .
								"# names have to be seperated by semicolons\n" .
								"# e.g. admins: cn=admin,dc=yourdomain,dc=org;cn=root,dc=yourdomain,dc=org\n" . "admins: " . $this->Adminstring . "\n");
			if (!$save_suffusr == True) array_push($file_array, "\n\n# suffix of users\n" .
								"# e.g. ou=People,dc=yourdomain,dc=org\n" . "usersuffix: " . $this->Suff_users . "\n");
			if (!$save_suffgrp == True) array_push($file_array, "\n\n# suffix of groups\n" .
								"# e.g. ou=Groups,dc=yourdomain,dc=org\n" . "groupsuffix: " . $this->Suff_groups . "\n");
			if (!$save_suffhst == True) array_push($file_array, "\n\n# suffix of Samba hosts\n" .
								"# e.g. ou=machines,dc=yourdomain,dc=org\n" . "hostsuffix: " . $this->Suff_hosts . "\n");
			if (!$save_suffdom == True) array_push($file_array, "\n\n# suffix of Samba 3 domains\n" .
								"# e.g. ou=domains,dc=yourdomain,dc=org\n" . "domainsuffix: " . $this->Suff_domains . "\n");
			if (!$save_minUID == True) array_push($file_array, "\n\n# minimum UID number\n" . "minUID: " . $this->MinUID . "\n");
			if (!$save_maxUID == True) array_push($file_array, "\n\n# maximum UID number\n" . "maxUID: " . $this->MaxUID . "\n");
			if (!$save_minGID == True) array_push($file_array, "\n\n# minimum GID number\n" . "minGID: " . $this->MinGID . "\n");
			if (!$save_maxGID == True) array_push($file_array, "\n\n# maximum GID number\n" . "maxGID: " . $this->MaxGID . "\n");
			if (!$save_minMach == True) array_push($file_array, "\n\n# minimum UID number for Samba hosts\n" . "minMachine: " . $this->MinMachine . "\n");
			if (!$save_maxMach == True) array_push($file_array, "\n\n# maximum UID number for Samba hosts\n" . "maxMachine: " . $this->MaxMachine . "\n");
			if (!$save_usrlstattr == True) array_push($file_array, "\n\n# list of attributes to show in user list\n# entries can either be predefined values (e.g. '#cn' or '#uid')" .
								"\n# or individual ones (e.g. 'uid:User ID' or 'host:Host Name')\n# values have to be seperated by semicolons\n" . "userlistAttributes: " . $this->userlistAttributes . "\n");
			if (!$save_grplstattr == True) array_push($file_array, "\n\n# list of attributes to show in group list\n# entries can either be predefined values (e.g. '#cn' or '#gidNumber')" .
								"\n# or individual ones (e.g. 'cn:Group Name')\n# values have to be seperated by semicolons\n" . "grouplistAttributes: " . $this->grouplistAttributes . "\n");
			if (!$save_hstlstattr == True) array_push($file_array, "\n\n# list of attributes to show in host list\n# entries can either be predefined values (e.g. '#cn' or '#uid')" .
								"\n# or individual ones (e.g. 'cn:Host Name')\n# values have to be seperated by semicolons\n" . "hostlistAttributes: " . $this->hostlistAttributes . "\n");
			if (!$save_maxlstent == True) array_push($file_array, "\n\n# maximum number of rows to show in user/group/host lists\n" . "maxlistentries: " . $this->maxlistentries . "\n");
			if (!$save_deflang == True) array_push($file_array, "\n\n# default language (a line from config/language)\n" . "defaultLanguage: " . $this->defaultLanguage . "\n");
			if (!$save_scriptPath == True) array_push($file_array, "\n\n# Path to external Script\n" . "scriptPath: " . $this->scriptPath . "\n");
			if (!$save_scriptServer == True) array_push($file_array, "\n\n# Server of external Script\n" . "scriptServer: " . $this->scriptServer . "\n");
			if (!$save_samba3 == True) array_push($file_array, "\n\n# Set to \"yes\" only if you use the new Samba 3.x schema.\n" . "samba3: " . $this->samba3 . "\n");
			if (!$save_cachetime == True) array_push($file_array, "\n\n# Number of minutes LAM caches LDAP searches.\n" . "cacheTimeout: " . $this->cachetimeout . "\n");
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

	// prints current preferences
	function printconf() {
		echo "<b>" . _("Server address") . ": </b>" . $this->ServerURL . "<br>";
		echo "<b>" . _("Cache timeout") . ": </b>" . $this->cachetimeout . "<br>";
		echo "<b>" . _("Samba 3.x schema") . ": </b>" . $this->samba3 . "<br>";
		echo "<b>" . _("UserSuffix") . ": </b>" . $this->Suff_users . "<br>";
		echo "<b>" . _("GroupSuffix") . ": </b>" . $this->Suff_groups . "<br>";
		echo "<b>" . _("HostSuffix") . ": </b>" . $this->Suff_hosts . "<br>";
		echo "<b>" . _("DomainSuffix") . ": </b>" . $this->Suff_domains . "<br>";
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
		echo "<b>" . _("List of valid users") . ": </b>" . $this->Adminstring;
	}

	// functions to read/write preferences

	// returns the server address as string
	function get_ServerURL() {
		return $this->ServerURL;
	}

	// sets the server address
	function set_ServerURL($value) {
		if (is_string($value)) $this->ServerURL = $value;
		else return false;
		return true;
	}

	// returns an array of string with all admin names
	function get_Admins() {
		return explode(";", $this->Adminstring);
	}

	// returns all admin users seperated by semicolons
	function get_Adminstring() {
		return $this->Adminstring;
	}

	// needs a string that contains all admin users seperated by semicolons
	function set_Adminstring($value) {
		if (is_string($value) &&
		eregi("^([a-z0-9]|-)+=([a-z0-9]|-)+(,([a-z0-9]|-)+=([a-z0-9]|-)+)+(;([a-z0-9]|-)+=([a-z0-9]|-)+(,([a-z0-9]|-)+=([a-z0-9]|-)+)+)*$", $value)) {
			$this->Adminstring = $value;
			$this->Admins = explode(";", $value);
		}
		else return false;
		return true;
	}

	// returns the password to access the preferences wizard
	function get_Passwd() {
		return $this->Passwd;
	}

	// sets the preferences wizard password
	function set_Passwd($value) {
		if (is_string($value)) $this->Passwd = $value;
		else return false;
		return true;
	}

	// returns the LDAP suffix where users are saved
	function get_UserSuffix() {
		return $this->Suff_users;
	}

	// sets the LDAP suffix where users are saved
	function set_UserSuffix($value) {
		if (is_string($value) && (eregi("^(([a-z]|-|[0-9])*=([a-z]|-|[0-9])*)(,([a-z]|-|[0-9])*=([a-z]|-|[0-9])*)*$", $value))) {
			$this->Suff_users = $value;
		}
		else return false;
		return true;
	}

	// returns the LDAP suffix where groups are saved
	function get_GroupSuffix() {
		return $this->Suff_groups;
	}

	// sets the LDAP suffix where groups are saved
	function set_GroupSuffix($value) {
		if (is_string($value) && (eregi("^(([a-z]|-|[0-9])*=([a-z]|-|[0-9])*)(,([a-z]|-|[0-9])*=([a-z]|-|[0-9])*)*$", $value))) {
			$this->Suff_groups = $value;
		}
		else return false;
		return true;
	}

	// returns the LDAP suffix where hosts are saved
	function get_HostSuffix() {
		return $this->Suff_hosts;
	}

	// sets the LDAP suffix where hosts are saved
	function set_HostSuffix($value) {
		if (is_string($value) && (eregi("^(([a-z]|-|[0-9])*=([a-z]|-|[0-9])*)(,([a-z]|-|[0-9])*=([a-z]|-|[0-9])*)*$", $value))) {
		$this->Suff_hosts = $value;
	}
		else return false;
		return true;
	}

	// returns the LDAP suffix where domains are saved
	function get_DomainSuffix() {
		return $this->Suff_domains;
	}

	// sets the LDAP suffix where domains are saved
	function set_DomainSuffix($value) {
		if (!$value && ($this->get_Samba3() == "no")) $this->Suff_domains = "";
		elseif (is_string($value) && (eregi("^(([a-z]|-|[0-9])*=([a-z]|-|[0-9])*)(,([a-z]|-|[0-9])*=([a-z]|-|[0-9])*)*$", $value))) {
			$this->Suff_domains = $value;
		}
		else return false;
		return true;
	}

	// returns the minimum UID to use when creating new users
	function get_minUID() {
		return $this->MinUID;
	}

	// sets the minimum UID to use when creating new users
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
	function set_userlistAttributes($value) {
		if (is_string($value) && eregi("^((#[a-z]*)|([a-z]*:[a-z*]))(;((#[a-z]*)|([a-z]*:[a-z]*)))*$", $value)) {
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
	function set_grouplistAttributes($value) {
		if (is_string($value) && eregi("^((#[a-z]*)|([a-z]*:[a-z*]))(;((#[a-z]*)|([a-z]*:[a-z]*)))*$", $value)) {
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
	function set_hostlistAttributes($value) {
		if (is_string($value) && eregi("^((#[a-z]*)|([a-z]*:[a-z*]))(;((#[a-z]*)|([a-z]*:[a-z]*)))*$", $value)) {
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
	function set_scriptServer($value) {
		if (!$value) $value = ""; // optional parameter
		if (is_string($value)) {
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
	function set_samba3($value) {
		if (is_string($value) && eregi("^(yes|no)$", $value)) {
			$this->samba3 = $value;
		}
		else return false;
		return true;
	}

	// returns the LDAP cache timeout in minutes
	function get_cacheTimeout() {
		return $this->cachetimeout;
	}

	// returns the LDAP cache timeout in seconds
	function get_cacheTimeoutSec() {
		return $this->cachetimeout * 60;
	}

	// sets the LDAP cache timeout in minutes
	function set_cacheTimeout($value) {
		if (is_numeric($value) && ($value > -1)) {
			$this->cachetimeout = $value;
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
				if (($line == "\n")||($line[0] == "#")) continue; // ignore comments
				// search keywords
				if (substr($line, 0, 10) == "password: ") {
					$this->password = chop(substr($line, 10, strlen($line)-10));
					continue;
				}
				if (substr($line, 0, 9) == "default: ") {
					$this->default = chop(substr($line, 9, strlen($line)-9));
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
