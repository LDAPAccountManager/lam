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

  // Samba 3 domain SIDs
  var $domainSID;

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
    $conffile = substr(__FILE__, 0, strlen(__FILE__) - 15) . "/config/" . $this->file . ".conf";
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
	if (substr($line, 0, 16) == "maxlistentries: ") {
	  $this->maxlistentries = chop(substr($line, 16, strlen($line)-16));
	  continue;
	}
	if (substr($line, 0, 17) == "defaultLanguage: ") {
	  $this->defaultLanguage = chop(substr($line, 17, strlen($line)-17));
	  continue;
	}
	if (substr($line, 0, 12) == "scriptPath: ") {
	  $this->scriptPath = chop(substr($line, 12, strlen($line)-12));
	  continue;
	}
	if (substr($line, 0, 14) == "scriptServer: ") {
	  $this->scriptServer = chop(substr($line, 14, strlen($line)-14));
	  continue;
	}
	if (substr($line, 0, 8) == "samba3: ") {
	  $this->samba3 = chop(substr($line, 8, strlen($line)-8));
	  continue;
	}
	if (substr($line, 0, 11) == "domainSID: ") {
	  $this->domainSID = chop(substr($line, 11, strlen($line)-11));
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
	$save_scriptPath = $save_scriptServer = $save_samba3 = $save_domainSID = False;
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
			$file_array[$i] = "admins: " . $this->Adminstring . "\n";
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
		if (substr($file_array[$i], 0, 16) == "maxlistentries: ") {
			$file_array[$i] = "maxlistentries: " . $this->maxlistentries . "\n";
			$save_maxlstent = True;
			continue;
		}
		if (substr($file_array[$i], 0, 17) == "defaultLanguage: ") {
			$file_array[$i] = "defaultLanguage: " . $this->defaultLanguage . "\n";
			$save_deflang = True;
			continue;
		}
		if (substr($file_array[$i], 0, 12) == "scriptPath: ") {
			$file_array[$i] = "scriptPath: " . $this->scriptPath . "\n";
			$save_scriptPath = True;
			continue;
		}
		if (substr($file_array[$i], 0, 14) == "scriptServer: ") {
			$file_array[$i] = "scriptServer: " . $this->scriptServer . "\n";
			$save_scriptServer = True;
			continue;
		}
		if (substr($file_array[$i], 0, 8) == "samba3: ") {
			$file_array[$i] = "samba3: " . $this->samba3 . "\n";
			$save_samba3 = True;
			continue;
		}
		if (substr($file_array[$i], 0, 11) == "domainSID: ") {
			$file_array[$i] = "domainSID: " . $this->domainSID . "\n";
			$save_domainSID = True;
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
	if (!$save_domainSID == True) array_push($file_array, "\n\n# Samba 3 domain SID. Set only if you use the new Samba 3.x schema.\n" . "domainSID: " . $this->domainSID . "\n");
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
    echo "<b>" . _("Server Address") . ": </b>" . $this->ServerURL . "<br>";
    echo "<b>" . _("Samba 3.x schema") . ": </b>" . $this->samba3 . "<br>";
    echo "<b>" . _("Domain SID (Samba 3 only)") . ": </b>" . $this->domainSID . "<br>";
    echo "<b>" . _("UserSuffix") . ": </b>" . $this->Suff_users . "<br>";
    echo "<b>" . _("GroupSuffix") . ": </b>" . $this->Suff_groups . "<br>";
    echo "<b>" . _("HostSuffix") . ": </b>" . $this->Suff_hosts . "<br>";
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
    echo "<b>" . _("Default Language") . ": </b>" . $this->defaultLanguage . "<br>";
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
    else StatusMessage("WARN", "", _("Config->set_ServerURL failed!") . " (" . $value . ")");
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
		else StatusMessage("WARN", "", _("Config->set_Admins failed!") . " (" . $value . ")");
	}

  // returns all admin users seperated by semicolons
  function get_Adminstring() {
    return $this->Adminstring;
  }

  // needs a string that contains all admin users seperated by semicolons
  function set_Adminstring($value) {
    if (is_string($value) &&
		ereg("^([a-zA-Z0-9]|-)+=([a-zA-Z0-9]|-)+(,([a-zA-Z0-9]|-)+=([a-zA-Z0-9]|-)+)+(;([a-zA-Z0-9]|-)+=([a-zA-Z0-9]|-)+(,([a-zA-Z0-9]|-)+=([a-zA-Z0-9]|-)+)+)*$", $value)) {
      $this->Adminstring = $value;
      $this->Admins = explode(";", $value);
    }
    else StatusMessage("WARN", "", _("Config->set_Adminstring failed!") . " (" . $value . ")");
  }

  // returns the password to access the preferences wizard
  function get_Passwd() {
    return $this->Passwd;
  }

  // sets the preferences wizard password
  function set_Passwd($value) {
    if (is_string($value)) $this->Passwd = $value;
    else StatusMessage("WARN", "", _("Config->set_Passwd failed!") . " (" . $value . ")");
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
    else StatusMessage("WARN", "", _("Config->set_UserSuffix failed!") . " (" . $value . ")");
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
	else StatusMessage("WARN", "", _("Config->set_GroupSuffix failed!") . " (" . $value . ")");
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
    else StatusMessage("WARN", "", _("Config->set_HostSuffix failed!") . " (" . $value . ")");
  }

  // returns the minimum UID to use when creating new users
  function get_minUID() {
    return $this->MinUID;
  }

  // sets the minimum UID to use when creating new users
  function set_minUID($value) {
    if (is_numeric($value)) $this->MinUID = $value;
    else StatusMessage("WARN", "", _("Config->set_minUID failed!") . " (" . $value . ")");
  }

  // returns the maximum UID to use when creating new users
  function get_maxUID() {
    return $this->MaxUID;
  }

  // sets the maximum UID to use when creating new users
  function set_maxUID($value) {
    if (is_numeric($value)) $this->MaxUID = $value;
    else StatusMessage("WARN", "", _("Config->set_maxUID failed!") . " (" . $value . ")");
  }

  // returns the minimum GID to use when creating new groups
  function get_minGID() {
    return $this->MinGID;
  }

  // sets the minimum GID to use when creating new groups
  function set_minGID($value) {
    if (is_numeric($value)) $this->MinGID = $value;
    else StatusMessage("WARN", "", _("Config->set_minGID failed!") . " (" . $value . ")");
  }

  // returns the maximum GID to use when creating new groups
  function get_maxGID() {
    return $this->MaxGID;
  }

  // sets the maximum GID to use when creating new groups
  function set_maxGID($value) {
    if (is_numeric($value)) $this->MaxGID = $value;
    else StatusMessage("WARN", "", _("Config->set_maxGID failed!") . " (" . $value . ")");
  }

  // returns the minimum UID to use when creating new Samba hosts
  function get_minMachine() {
    return $this->MinMachine;
  }

  // sets the minimum UID to use when creating new Samba hosts
  function set_minMachine($value) {
    if (is_numeric($value)) $this->MinMachine = $value;
    else StatusMessage("WARN", "", _("Config->set_minMachine failed!") . " (" . $value . ")");
  }

  // returns the maximum UID to use when creating new Samba hosts
  function get_maxMachine() {
    return $this->MaxMachine;
  }

  // sets the maximum UID to use when creating new Samba hosts
  function set_maxMachine($value) {
    if (is_numeric($value)) $this->MaxMachine = $value;
    else StatusMessage("WARN", "", _("Config->set_maxMachine failed!") . " (" . $value . ")");
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
    else StatusMessage("WARN", "", _("Config->set_userlistAttributes failed!") . " (" . $value . ")");
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
    else StatusMessage("WARN", "", _("Config->set_grouplistAttributes failed!") . " (" . $value . ")");
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
    else StatusMessage("WARN", "", _("Config->set_hostlistAttributes failed!") . " (" . $value . ")");
  }

  // returns the maximum number of rows in user/group/host lists
  function get_MaxListEntries() {
    return $this->maxlistentries;
  }

  // sets the maximum number of rows in user/group/host lists
  function set_MaxListEntries ($value) {
    if (is_numeric($value)) $this->maxlistentries = $value;
    else StatusMessage("WARN", "", _("Config->set_MaxListEntries failed!") . $value);
  }

  // returns the default language string
  function get_defaultLanguage() {
    return $this->defaultLanguage;
  }

  // sets the default language string
  function set_defaultLanguage($value) {
    if (is_string($value)) $this->defaultLanguage = $value;
    else StatusMessage("WARN", "", _("Config->set_defaultLanguage failed!") . " (" . $value . ")");
  }

  // returns the path to the external script
  function get_scriptPath() {
    return $this->scriptPath;
  }

  // sets the path to the external script
  function set_scriptPath($value) {
	if (!$value) $value = ""; // optional parameter
    if (is_string($value) && eregi("^()|(/([a-z]|[0-9]|-|_|/)*)$", $value)) $this->scriptPath = $value;
    else StatusMessage("WARN", "", _("Config->set_scriptPath failed!") . " (" . $value . ")");
  }

  // returns the server of the external script
  function get_scriptServer() {
    return $this->scriptServer;
  }

  // sets the server of the external script
  function set_scriptServer($value) {
	if (!$value) $value = ""; // optional parameter
    if (is_string($value) && (eregi("^[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}$", $value) || $value == "")) {
		$this->scriptServer = $value;
	}
    else StatusMessage("WARN", "", _("Config->set_scriptServer failed!") . " (" . $value . ")");
  }

  // returns "yes" if Samba 3.x schema is used, otherwise "no"
  function get_samba3() {
    return $this->samba3;
  }

  // set Samba version: "yes" means 3.x schema, "no" means 2.2.x schema
  function set_samba3($value) {
    if (is_string($value) && eregi("^(yes|no)$", $value)) {
		$this->samba3 = $value;
	}
    else StatusMessage("WARN", "", _("Config->set_samba3 failed!") . " (" . $value . ")");
  }

  // returns the Samba domain SID (Samba 3 only)
  function get_domainSID() {
    return $this->domainSID;
  }

  // sets the Samba domain SID (Samba 3 only)
  function set_domainSID($value) {
	if (!$value) { // optional parameter
		$value = "";
		$this->domainSID = $value;
	}
    elseif (is_string($value) && eregi("^S-[0-9]-[0-9]-[0-9]{2,2}-[0-9]*-[0-9]*-[0-9]*$", $value)) {
		$this->domainSID = $value;
	}
    else StatusMessage("WARN", "", _("Config->set_domainSID failed!") . " (" . $value . ")");
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
