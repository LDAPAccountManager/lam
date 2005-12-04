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


*/

/**
* This file includes functions to manage the configuration files.
*
* @package configuration
* @author Roland Gruber
*/

/** Used to print messages. */
include_once("status.inc");
/** Used to get module information. */
include_once("modules.inc");

/**
* Sets language settings for automatic translation
*/
function setlanguage() {
	if (!isset($_SESSION['language'])) {
		$_SESSION['language'] = "en_GB.utf8:UTF-8:English (Great Britain)";
	}
	$language = explode(":", $_SESSION['language']);
	putenv("LANG=" . $language[0]);  // e.g. LANG=de_DE
	setlocale(LC_ALL, $language[0]);  // set LC_ALL to de_DE
	$locdir = substr(__FILE__, 0, strlen(__FILE__) - 15) . "/locale";  // set path to translations
	bindtextdomain("messages", $locdir);
	bind_textdomain_codeset("messages", $language[1]);
	textdomain("messages");
	header("Content-type: text/html; charset=" . $language[1], true);
}

/**
* Returns an array of string with all available configuration profiles (without .conf)
*
* @return array profile names
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
* Returns the version number of this LAM installation.
* Format: <major version>.<minor version>.<patch level>
* <br> Major/minor version are always numbers, patch level may contain letters for inofficial releases only (e.g. 0.5.alpha1).
*
* @return string version number
*/
function LAMVersion() {
	$file = substr(__FILE__, 0, strlen(__FILE__) - 15) . "/VERSION";
	if (is_readable($file)) {
		$handle = fopen($file, "r");
		if (!feof($handle)) {
			return trim(fgets($handle, 20));
		}
	}
	// file was not readable
	return '0.0.unknown';
}


/**
* Prints a meta refresh page
*
* @param string $page the URL of the target page
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
*
* @package configuration
*/
class Config {

	/** Server address (e.g. ldap://127.0.0.1:389) */
	var $ServerURL;

	/** Array of string: users with admin rights */
	var $Admins;

	/** Password to edit preferences */
	var $Passwd;

	/** LDAP suffix for users */
	var $usersuffix;

	/** LDAP suffix for groups */
	var $groupsuffix;

	/** LDAP suffix for Samba hosts */
	var $hostsuffix;

	/** LDAP suffix for Samba 3 domains */
	var $domainsuffix;

	/** LDAP suffix for tree view */
	var $treesuffix;

	/** Attributes that are shown in the user list */
	var $userlistAttributes;
	/** Attributes that are shown in the group list */
	var $grouplistAttributes;
	/** Attributes that are shown in the host list */
	var $hostlistAttributes;

	/** Maximum number of rows shown in user/group/host lists */
	var $maxlistentries;

	/** Default language */
	var $defaultLanguage;

	/** module settings */
	var $moduleSettings = array();

	/**
	* Path to external lamdaemon script on server where it is executed
	*
	* This is used for managing quota and home directories.
	* optional setting, may not be defined
	*/
	var $scriptPath;

	/**
	* Server where lamdaemon script is executed
	*
	* This is used for managing quota and home directories.
	* optional setting, may not be defined
	*/
	var $scriptServer;

	/** LDAP cache timeout */
	var $cachetimeout;

	var $usermodules = "posixAccount,shadowAccount,quota";
	/** Account modules for groups */
	var $groupmodules = "posixGroup,quota";
	/** Account modules for hosts */
	var $hostmodules = "account,sambaSamAccount";

	/** Name of configuration file */
	var $file;

	/** List of all settings in config file */
	var $settings = array("ServerURL", "Passwd", "Admins", "usersuffix", "groupsuffix", "hostsuffix", "treesuffix",
		"domainsuffix", "userlistAttributes", "grouplistAttributes", "hostlistAttributes", "maxlistentries",
		"defaultLanguage", "scriptPath", "scriptServer", "cachetimeout",
		"usermodules", "groupmodules", "hostmodules", "modules");


	/**
	* Loads preferences from config file
	*
	* @param integer $file Index number in config file array
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

	/**
	* Reloads preferences from config file
	*
	* @return boolean true if file was readable
	*/
	function reload() {
		$conffile = substr(__FILE__, 0, strlen(__FILE__) - 15) . "/config/" . $this->file . ".conf";
		if (is_file($conffile) == True) {
			$file = @fopen($conffile, "r");
			if (!$file) return false; // abort if file is not readable
			while (!feof($file)) {
				$line = fgets($file, 1024);
				$line = trim($line);  // remove spaces at the beginning and end
				if (($line == "")||($line[0] == "#")) continue; // ignore comments and empty lines
				// search keywords
				for ($i = 0; $i < sizeof($this->settings); $i++) {
					$keyword = $this->settings[$i];
					$keylen = strlen($keyword);
					if (strtolower(substr($line, 0, $keylen + 2)) == strtolower($keyword . ": ")) {
						// module settings
						if (strtolower(substr($line, 0, $keylen + 2)) == "modules: ") {
							$option = substr($line, $keylen + 2, strlen($line) - $keylen - 2);
							$pos = strpos($option, ":");
							$this->moduleSettings[substr($option, 0, $pos)] = explode("+::+", substr($option, $pos + 2, strlen($option) - $pos - 2));
						}
						// general settings
						else {
							$this->$keyword = substr($line, $keylen + 2, strlen($line) - $keylen - 2);
						}
						break;
					}
				}
			}
			fclose($file);
		}
		// check modules
		$scopes = array('user', 'group', 'host');
		for ($s = 0; $s < sizeof($scopes); $s++) {
			$scope = $scopes[$s];
			$moduleVar = $scope . "modules";
			$modules = explode(",", $this->$moduleVar);
			$available = getAvailableModules($scope);
			// only return available modules
			$ret = array();
			for ($i = 0; $i < sizeof($modules); $i++) {
				if (in_array($modules[$i], $available)) $ret[] = $modules[$i];
			}
			$this->$moduleVar = implode(",", $ret);
		}
		return true;
	}

	/** Saves preferences to config file */
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
			$mod_saved = array();	// includes all module settings which have been saved
			for ($i = 0; $i < sizeof($file_array); $i++) {
				$line = trim($file_array[$i]);
				if (($line == "")||($line[0] == "#")) continue; // ignore comments and empty lines
				// search for keywords
				for ($k = 0; $k < sizeof($this->settings); $k++) {
					$keyword = $this->settings[$k];
					$keylen = strlen($keyword);
					if (strtolower(substr($line, 0, $keylen + 1)) == strtolower($keyword . ":")) {
						// module settings
						if (strtolower(substr($line, 0, $keylen + 2)) == "modules: ") {
							$option = substr($line, $keylen + 2, strlen($line) - $keylen - 2);
							$pos = strpos($option, ":");
							$name = substr($option, 0, $pos);
							if (!isset($this->moduleSettings[$name])) continue;
							$file_array[$i] = "modules: " . $name . ": " . implode("+::+", $this->moduleSettings[$name]) . "\n";
							$mod_saved[] = $name;	// mark keyword as saved
						}
						// general settings
						else {
							$file_array[$i] = $keyword . ": " . $this->$keyword . "\n";
							$saved[] = $keyword;	// mark keyword as saved
						}
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
			if (!in_array("hostsuffix", $saved)) array_push($file_array, "\n\n# suffix of hosts\n" .
								"# e.g. ou=machines,dc=yourdomain,dc=org\n" . "hostsuffix: " . $this->hostsuffix . "\n");
			if (!in_array("domainsuffix", $saved)) array_push($file_array, "\n\n# suffix of Samba 3 domains\n" .
								"# e.g. ou=domains,dc=yourdomain,dc=org\n" . "domainsuffix: " . $this->domainsuffix . "\n");
			if (!in_array("treesuffix", $saved)) array_push($file_array, "\n\n# suffix of tree view\n" .
								"# e.g. dc=yourdomain,dc=org\n" . "treesuffix: " . $this->treesuffix . "\n");
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
			if (!in_array("usermodules", $saved)) array_push($file_array, "\n\n# List of used user modules\n" . "usermodules: " . $this->usermodules . "\n");
			if (!in_array("groupmodules", $saved)) array_push($file_array, "\n\n# List of used group modules\n" . "groupmodules: " . $this->groupmodules . "\n");
			if (!in_array("hostmodules", $saved)) array_push($file_array, "\n\n# List of used host modules\n" . "hostmodules: " . $this->hostmodules . "\n");
			// check if all module settings were added
			$m_settings = array_keys($this->moduleSettings);
			for ($i = 0; $i < sizeof($m_settings); $i++) {
				if (!in_array($m_settings[$i], $mod_saved)) {
					array_push($file_array, "modules: " . $m_settings[$i] . ": " . implode("+::+", $this->moduleSettings[$m_settings[$i]]) . "\n");
				}
			}
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

	/** Prints current preferences */
	function printconf() {
		echo "<b>" . _("Server address") . ": </b>" . $this->ServerURL . "<br>\n";
		echo "<b>" . _("Cache timeout") . ": </b>" . $this->cachetimeout . "<br>\n";
		echo "<b>" . _("UserSuffix") . ": </b>" . $this->usersuffix . "<br>\n";
		echo "<b>" . _("GroupSuffix") . ": </b>" . $this->groupsuffix . "<br>\n";
		echo "<b>" . _("HostSuffix") . ": </b>" . $this->hostsuffix . "<br>\n";
		echo "<b>" . _("DomainSuffix") . ": </b>" . $this->domainsuffix . "<br>\n";
		echo "<b>" . _("TreeSuffix") . ": </b>" . $this->treesuffix . "<br>\n";
		echo "<b>" . _("Attributes in User List") . ": </b>" . $this->userlistAttributes . "<br>\n";
		echo "<b>" . _("Attributes in Group List") . ": </b>" . $this->grouplistAttributes . "<br>\n";
		echo "<b>" . _("Attributes in Host List") . ": </b>" . $this->hostlistAttributes . "<br>\n";
		echo "<b>" . _("Maximum list entries") . ": </b>" . $this->maxlistentries . "<br>\n";
		echo "<b>" . _("Default language") . ": </b>" . $this->defaultLanguage . "<br>\n";
		echo "<b>" . _("Path to external script") . ": </b>" . $this->scriptPath . "<br>\n";
		echo "<b>" . _("Server of external script") . ": </b>" . $this->scriptServer . "<br>\n";
		echo "<b>" . _("List of valid users") . ": </b>" . $this->Admins . "<br>\n";
		echo "<b>" . _("User modules") . ": </b>" . $this->usermodules . "<br>\n";
		echo "<b>" . _("Group modules") . ": </b>" . $this->groupmodules . "<br>\n";
		echo "<b>" . _("Host modules") . ": </b>" . $this->hostmodules . "<br><br>\n";
		echo "<b>" . _("Module settings") . ": </b><br>\n";
		echo "<ul>\n";
		$names = array_keys($this->moduleSettings);
		$descriptions = getConfigDescriptions();
		$descriptions = $descriptions['descriptions'];
		for ($i = 0; $i < sizeof($names); $i++) {
			echo "<li><b>";
			// print description if available
			if (isset($descriptions[$names[$i]])) echo $descriptions[$names[$i]];
			else echo $names[$i];
			echo ": </b>" . implode(", ", $this->moduleSettings[$names[$i]]) . "</li>\n";
		}
		echo "</ul>\n";
	}

	// functions to read/write preferences

	/**
	* Returns the server address as string
	*
	* @return string server address
	*/
	function get_ServerURL() {
		return $this->ServerURL;
	}

	/**
	* Sets the server address
	*
	* @param string $value new server address
	* @return boolean true if $value has correct format
	*/
	function set_ServerURL($value) {
		if (is_string($value)) $this->ServerURL = $value;
		else return false;
		return true;
	}

	/**
	* Returns an array of string with all admin names
	*
	* @return array the admin names
	*/
	function get_Admins() {
		return explode(";", $this->Admins);
	}

	/**
	* Returns all admin users seperated by semicolons
	*
	* @return string the admin string
	*/
	function get_Adminstring() {
		return $this->Admins;
	}

	/**
	* Sets the admin string
	*
	* @param string $value new admin string that contains all admin users seperated by semicolons
	* @return boolean true if $value has correct format
	*/
	function set_Adminstring($value) {
		if (is_string($value) &&
		eregi("^[^;]+(;[^;]+)*$", $value)) {
			$this->Admins = $value;
		}
		else return false;
		return true;
	}

	/**
	* Returns the password to access the preferences wizard
	*
	* @return string the password
	*/
	function get_Passwd() {
		return $this->Passwd;
	}

	/**
	* Sets the preferences wizard password
	*
	* @param string $value new password
	* @return boolean true if $value has correct format
	*/
	function set_Passwd($value) {
		if (is_string($value)) $this->Passwd = $value;
		else return false;
		return true;
	}

	/**
	* Returns the LDAP suffix for the given account type
	*
	* @param string $scope account type
	* @return string the LDAP suffix
	*/
	function get_Suffix($scope) {
		switch ($scope) {
			case 'user':
					return $this->usersuffix;
				break;
			case 'group':
					return $this->groupsuffix;
				break;
			case 'host':
					return $this->hostsuffix;
				break;
			case 'domain':
					return $this->domainsuffix;
				break;
			case 'tree':
					return $this->treesuffix;
				break;
		}
		return "";
	}

	/**
	* Sets the LDAP suffix where accounts are saved
	*
	* @param string $scope account type
	* @param string $value new LDAP suffix
	* @return boolean true if $value has correct format
	*/
	function set_Suffix($scope, $value) {
		if (!$value) $value = "";
		elseif (!is_string($value)) {
			return false;
		}
		switch ($scope) {
			case 'user':
					$this->usersuffix = $value;
				break;
			case 'group':
					$this->groupsuffix = $value;
				break;
			case 'host':
					$this->hostsuffix = $value;
				break;
			case 'domain':
					$this->domainsuffix = $value;
				break;
			case 'tree':
					$this->treesuffix = $value;
				break;
		}
		return true;
	}

	/**
	* Returns the list of attributes to show in user list
	*
	* @param string $scope account type
	* @return string the attribute list
	*/
	function get_listAttributes($scope) {
		switch ($scope) {
			case 'user':
				return $this->userlistAttributes;
				break;
			case 'group':
				return $this->grouplistAttributes;
				break;
			case 'host':
				return $this->hostlistAttributes;
				break;
			default:
				return '';
				break;
		}
	}

	/**
	* Sets the list of attributes to show in user list
	*
	* @param string $value new attribute string
	* @param string $scope account type
	* @return boolean true if $value has correct format
	*/
	function set_listAttributes($value, $scope) {
		if (is_string($value) && eregi("^((#[^:;]+)|([^:;]*:[^:;]+))(;((#[^:;]+)|([^:;]*:[^:;]+)))*$", $value)) {
			switch ($scope) {
				case 'user':
						$this->userlistAttributes = $value;
					break;
				case 'group':
						$this->grouplistAttributes = $value;
					break;
				case 'host':
						$this->hostlistAttributes = $value;
					break;
				default:
					return false;
					break;
			}
			return true;
		}
		else {
			return false;
		}
	}

	/**
	* Sets the list of attributes to show in group list
	*
	* @param string $value new attribute string
	* @return boolean true if $value has correct format
	*/
	function set_grouplistAttributes($value) {
		if (is_string($value) && eregi("^((#[^:;]+)|([^:;]*:[^:;]+))(;((#[^:;]+)|([^:;]*:[^:;]+)))*$", $value)) {
			$this->grouplistAttributes = $value;
		}
		else return false;
		return true;
	}

	/**
	* Returns the maximum number of rows in user/group/host lists
	*
	* @return integer maximum number
	*/
	function get_MaxListEntries() {
		return $this->maxlistentries;
	}

	/**
	* Sets the maximum number of rows in user/group/host lists
	*
	* @param integer $value new maximum value
	* @return boolean true if $value has correct format
	*/
	function set_MaxListEntries ($value) {
		if (is_numeric($value)) $this->maxlistentries = $value;
		else return false;
		return true;
	}

	/**
	* Returns the default language string
	*
	* @return string default language
	*/
	function get_defaultLanguage() {
		return $this->defaultLanguage;
	}

	/**
	* Sets the default language string
	*
	* @param string $value new default language
	* @return boolean true if $value has correct format
	*/
	function set_defaultLanguage($value) {
		if (is_string($value)) $this->defaultLanguage = $value;
		else return false;
		return true;
	}

	/**
	* Returns the path to the external script
	*
	* @return string script path
	*/
	function get_scriptPath() {
		return $this->scriptPath;
	}

	/**
	* Sets the path to the external script
	*
	* @param string $value new script path
	* @return boolean true if $value has correct format
	*/
	function set_scriptPath($value) {
		if (!$value) $this->scriptPath = ""; // optional parameter
		elseif (is_string($value) && eregi("^/([a-z0-9_\\-])+(/([a-z0-9_\\.\\-])+)+$", $value)) $this->scriptPath = $value;
		else return false;
		return true;
	}

	/**
	* Returns the server of the external script
	*
	* @return string script server
	*/
	function get_scriptServer() {
		return $this->scriptServer;
	}

	/**
	* Sets the server of the external script
	*
	* @param string $value new script server
	* @return boolean true if $value has correct format
	*/
	function set_scriptServer($value) {
		if (!$value) $this->scriptServer = ""; // optional parameter
		elseif (is_string($value) && eregi("^[a-z0-9\\-]+(\\.[a-z0-9\\-]+)*$", $value)) {
			$this->scriptServer = $value;
		}
		else return false;
		return true;
	}

	/**
	* Returns the LDAP cache timeout in minutes
	*
	* @return integer cache time
	*/
	function get_cacheTimeout() {
		if (isset($this->cachetimeout)) return $this->cachetimeout;
		else return 5;
	}

	/**
	* Returns the LDAP cache timeout in seconds
	*
	* @return integer cache time
	*/
	function get_cacheTimeoutSec() {
		return $this->cachetimeout * 60;
	}

	/**
	* Sets the LDAP cache timeout in minutes (0,1,2,5,10,15)
	*
	* @param integer $value new cache timeout
	* @return boolean true if $value has correct format
	*/
	function set_cacheTimeout($value) {
		if (is_numeric($value) && ($value > -1)) {
			$this->cachetimeout = $value;
		}
		else return false;
		return true;
	}

	/**
	* Returns an array of all selected account modules
	*
	* @param string $scope account type
	* @return array user modules
	*/
	function get_AccountModules($scope) {
		switch ($scope) {
			case 'user':
				return explode(",", $this->usermodules);
				break;		
			case 'group':
				return explode(",", $this->groupmodules);
				break;		
			case 'host':
				return explode(",", $this->hostmodules);
				break;		
			default:
				return array();
				break;
		}
		
	}

	/**
	* Sets the selected account modules
	*
	* @param array $modules array with module names (not aliases!)
	* @param string $scope account type
	* @return boolean true if $modules has correct format
	*/
	function set_AccountModules($modules, $scope) {
		if (! is_array($modules)) return false;
		// check module names
		$available = getAvailableModules($scope);
		for ($i = 0; $i < sizeof($modules); $i++) {
			if (! in_array($modules[$i], $available)) return false;
		}
		// check depends/conflicts
		if (check_module_conflicts($modules, getModulesDependencies($scope)) != false) return false;
		if (check_module_depends($modules, getModulesDependencies($scope)) != false) return false;
		switch ($scope) {
			case 'user':
				$this->usermodules = implode(",", $modules);
				break;		
			case 'group':
				$this->groupmodules = implode(",", $modules);
				break;		
			case 'host':
				$this->hostmodules = implode(",", $modules);
				break;		
			default:
				break;
		}
		return true;
	}

	/**
	* Sets the settings for the account modules.
	*
	* @param array $settings list of module setting array(name => value)
	* @return boolean true if $settings has correct format
	*/
	function set_moduleSettings($settings) {
		if (!is_array($settings)) return false;
		$this->moduleSettings = $settings;
		return true;
	}

	/**
	* Returns a list of saved module settings
	*
	* @return array list of settings: array(name => value)
	*/
	function get_moduleSettings() {
		return $this->moduleSettings;
	}

}


/**
* This class manages config.cfg.
*
* @package configuration
*/
class CfgMain {

	/** Default profile */
	var $default;

	/** Password to change config.cfg */
	var $password;

	/**
	* Loads preferences from config file
	*/
	function CfgMain() {
		$this->reload();
	}

	/**
	* Reloads preferences from config file config.cfg
	*
	* @return boolean true if file was readable
	*/
	function reload() {
		$conffile = substr(__FILE__, 0, strlen(__FILE__) - 15) . "/config/config.cfg";
		if (is_file($conffile) == True) {
			$file = @fopen($conffile, "r");
			if (!$file) return false; // abort if file is not readable
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
		return true;
	}

	/**
	* Saves preferences to config file config.cfg
	*/
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
