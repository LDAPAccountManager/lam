<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2011  Roland Gruber

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
* @author Thomas Manninger
*/

/** Used to print messages. */
include_once("status.inc");
/** Used to get module information. */
include_once("modules.inc");
/** Used to get type information. */
include_once("types.inc");

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
 * Checks whether a specific flag in the rights string is set.
 *
 * @param $right read,write or execute
 * @param $target owner,group or other
 * @param $chmod the chmod rights
 *
 * @return true, if the chmod $right for $target were set
 */
function checkChmod($right, $target, $chmod) {
	$right_arr=array("read","write","execute");
	$target_arr=array("owner","group","other");

	// Check, if $right and $target has right parameters
	if (!in_array($right, $right_arr) ||!in_array($target, $target_arr)) {
		return false;
	}
	
	$chmod_num = -1;
	// owner:
	if ($target == "owner") $chmod_num = 0;
	if ($target == "group") $chmod_num = 1;
	if ($target == "other") $chmod_num = 2;
	
	// Cut the number from the chmod:
	$chmod_num = $chmod{$chmod_num};
	
	// Now check, if the chmod_num can be right with the $right
	// What numbers allow "read"
	$read = array(4,5,6,7);
	// What numbers allow "write"
	$write = array(2,3,6,7);
	// What numbers allow "execute"
	$execute = array(1,3,5,7);
	if (($right == "read") && in_array($chmod_num, $read)) return true;
	elseif (($right == "write") && in_array($chmod_num, $write)) return true;
	elseif (($right == "execute") && in_array($chmod_num, $execute)) return true;
	else return false;
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
	if (isset($_SESSION['header'])) {
		echo $_SESSION['header'];
	}
	else {
		echo '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">';
		echo "<html><head>\n";
	}
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
class LAMConfig {
	
	/* access levels */
	const ACCESS_ALL = 100;
	const ACCESS_PASSWORD_CHANGE = 20;
	const ACCESS_READ_ONLY = 0;
	
	/* return codes for saving configuration file */
	const SAVE_OK = 0;
	const SAVE_FAIL = 1;
	
	/* login method: predefined list or LDAP search */
	const LOGIN_LIST = 'list';
	const LOGIN_SEARCH = 'search';
	
	/** line separator */
	const LINE_SEPARATOR = '+::+';

	/** Server address (e.g. ldap://127.0.0.1:389) */
	private $ServerURL;
	
	/** enables/disables TLS encryption */
	private $useTLS;

	/** Array of string: users with admin rights */
	private $Admins;

	/** Password to edit preferences */
	private $Passwd;

	/** LDAP suffix for tree view */
	private $treesuffix;

	/** Default language */
	private $defaultLanguage;

	/** module settings */
	private $moduleSettings = array();

	/** type settings */
	private $typeSettings = array();

	/**
	* Path to external lamdaemon script on server where it is executed
	*
	* This is used for managing quota and home directories.
	* optional setting, may not be defined
	*/
	private $scriptPath;

	/**
	 * The rights for the home directory
	 */
	private $scriptRights = '750';

	/**
	* Servers where lamdaemon script is executed
	*
	* This is used for managing quota and home directories.
	* optional setting, may not be defined
	*/
	private $scriptServer;

	/** LDAP cache timeout */
	private $cachetimeout;
	
	/** LDAP search limit */
	private $searchLimit = 0;

	/** Active account types */
	private $activeTypes = "user,group,host,smbDomain";

	/** Name of configuration file */
	private $file;
	
	/** access level */
	private $accessLevel = LAMconfig::ACCESS_ALL;
	
	/** login method */
	private $loginMethod = LAMconfig::LOGIN_LIST;
	
	/** search suffix for login */
	private $loginSearchSuffix = 'dc=yourdomain,dc=org';
	
	/** search filter for login */
	private $loginSearchFilter = 'uid=%USER%';
	
	/** specifies if HTTP authentication should be used */
	private $httpAuthentication = 'false';
	
	/** email address for sender of password reset mails */
	private $lamProMailFrom = '';
	
	/** reply-to email address for password reset mails */
	private $lamProMailReplyTo = '';
	
	/** subject for password reset mails */
	private $lamProMailSubject = '';
	
	/** treat password reset mail body as HTML */
	private $lamProMailIsHTML = 'false';
	
	/** mail body for password reset mails */
	private $lamProMailText = '';

	/** List of all settings in config file */
	private $settings = array("ServerURL", "useTLS", "Passwd", "Admins", "treesuffix",
		"defaultLanguage", "scriptPath", "scriptServer", "scriptRights", "cachetimeout",
		"modules", "activeTypes", "types", "accessLevel", 'loginMethod', 'loginSearchSuffix',
		'loginSearchFilter', 'searchLimit', 'lamProMailFrom', 'lamProMailReplyTo', 'lamProMailSubject',
		'lamProMailText', 'lamProMailIsHTML', 'httpAuthentication');


	/**
	* Loads preferences from config file
	*
	* @param integer $file Index number in config file array
	*/
	function __construct($file = 0) {
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
	private function reload() {
		$conffile = $this->getPath();
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
							$this->moduleSettings[substr($option, 0, $pos)] = explode(LAMConfig::LINE_SEPARATOR, substr($option, $pos + 2));
						}
						// type settings
						elseif (strtolower(substr($line, 0, $keylen + 2)) == "types: ") {
							$option = substr($line, $keylen + 2, strlen($line) - $keylen - 2);
							$pos = strpos($option, ":");
							$this->typeSettings[substr($option, 0, $pos)] = substr($option, $pos + 2);
						}
						// general settings
						else {
							$this->$keyword = substr($line, $keylen + 2, strlen($line) - $keylen - 2);
						}
						break;
					}
					elseif (strtolower($line) == strtolower($keyword . ":")) {
						// set empty options
						$this->$keyword = '';
					}
				}
			}
			fclose($file);
		}
		// check modules
		$scopes = $this->get_ActiveTypes();
		for ($s = 0; $s < sizeof($scopes); $s++) {
			$scope = $scopes[$s];
			$moduleVar = "modules_" . $scope;
			if (isset($this->typeSettings[$moduleVar])){
				$modules = explode(",", $this->typeSettings[$moduleVar]);
				$available = getAvailableModules($scope);
				// only return available modules
				$ret = array();
				for ($i = 0; $i < sizeof($modules); $i++) {
					if (in_array($modules[$i], $available)) $ret[] = $modules[$i];
				}
				$this->$moduleVar = implode(",", $ret);
			}
		}
		return true;
	}

	/** Saves preferences to config file */
	public function save() {
		$conffile = $this->getPath();
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
							$file_array[$i] = "modules: " . $name . ": " . implode(LAMConfig::LINE_SEPARATOR, $this->moduleSettings[$name]) . "\n";
							$mod_saved[] = $name;	// mark keyword as saved
						}
						// type settings
						elseif (strtolower(substr($line, 0, $keylen + 2)) == "types: ") {
							$option = substr($line, $keylen + 2, strlen($line) - $keylen - 2);
							$pos = strpos($option, ":");
							$name = substr($option, 0, $pos);
							if (!isset($this->typeSettings[$name])) continue;
							$file_array[$i] = "types: " . $name . ": " . $this->typeSettings[$name] . "\n";
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
			if (!in_array("useTLS", $saved)) array_push($file_array, "\n\n# enable TLS encryption\n" . "useTLS: " . $this->useTLS . "\n");
			if (!in_array("Passwd", $saved)) array_push($file_array, "\n\n# password to change these preferences via webfrontend\n" . "passwd: " . $this->Passwd . "\n");
			if (!in_array("Admins", $saved)) array_push($file_array, "\n\n# list of users who are allowed to use LDAP Account Manager\n" .
								"# names have to be seperated by semicolons\n" .
								"# e.g. admins: cn=admin,dc=yourdomain,dc=org;cn=root,dc=yourdomain,dc=org\n" . "admins: " . $this->Admins . "\n");
			if (!in_array("treesuffix", $saved)) array_push($file_array, "\n\n# suffix of tree view\n" .
								"# e.g. dc=yourdomain,dc=org\n" . "treesuffix: " . $this->treesuffix . "\n");
			if (!in_array("defaultLanguage", $saved)) array_push($file_array, "\n\n# default language (a line from config/language)\n" . "defaultLanguage: " . $this->defaultLanguage . "\n");
			if (!in_array("scriptPath", $saved)) array_push($file_array, "\n\n# Path to external Script\n" . "scriptPath: " . $this->scriptPath . "\n");
			if (!in_array("scriptServer", $saved)) array_push($file_array, "\n\n# Servers of external script\n" . "scriptServer: " . $this->scriptServer . "\n");
			if (!in_array("scriptRights", $saved)) array_push($file_array, "\n\n# Access rights for home directories\n" . "scriptRights: " . $this->scriptRights . "\n");
			if (!in_array("cachetimeout", $saved)) array_push($file_array, "\n\n# Number of minutes LAM caches LDAP searches.\n" . "cacheTimeout: " . $this->cachetimeout . "\n");
			if (!in_array("searchLimit", $saved)) array_push($file_array, "\n\n# LDAP search limit.\n" . "searchLimit: " . $this->searchLimit . "\n");
			if (!in_array("activeTypes", $saved)) array_push($file_array, "\n\n# List of active account types.\n" . "activeTypes: " . $this->activeTypes . "\n");
			if (!in_array("accessLevel", $saved)) array_push($file_array, "\n\n# Access level for this profile.\n" . "accessLevel: " . $this->accessLevel . "\n");
			if (!in_array("loginMethod", $saved)) array_push($file_array, "\n\n# Login method.\n" . "loginMethod: " . $this->loginMethod . "\n");
			if (!in_array("loginSearchSuffix", $saved)) array_push($file_array, "\n\n# Search suffix for LAM login.\n" . "loginSearchSuffix: " . $this->loginSearchSuffix . "\n");
			if (!in_array("loginSearchFilter", $saved)) array_push($file_array, "\n\n# Search filter for LAM login.\n" . "loginSearchFilter: " . $this->loginSearchFilter . "\n");
			if (!in_array("httpAuthentication", $saved)) array_push($file_array, "\n\n# HTTP authentication for LAM login.\n" . "httpAuthentication: " . $this->httpAuthentication . "\n");
			if (!in_array("lamProMailFrom", $saved)) array_push($file_array, "\n\n# Password mail from\n" . "lamProMailFrom: " . $this->lamProMailFrom . "\n");
			if (!in_array("lamProMailReplyTo", $saved)) array_push($file_array, "\n\n# Password mail reply-to\n" . "lamProMailReplyTo: " . $this->lamProMailReplyTo . "\n");
			if (!in_array("lamProMailSubject", $saved)) array_push($file_array, "\n\n# Password mail subject\n" . "lamProMailSubject: " . $this->lamProMailSubject . "\n");
			if (!in_array("lamProMailIsHTML", $saved)) array_push($file_array, "\n\n# Password mail is HTML\n" . "lamProMailIsHTML: " . $this->lamProMailIsHTML . "\n");
			if (!in_array("lamProMailText", $saved)) array_push($file_array, "\n\n# Password mail text\n" . "lamProMailText: " . $this->lamProMailText . "\n");
			// check if all module settings were added
			$m_settings = array_keys($this->moduleSettings);
			for ($i = 0; $i < sizeof($m_settings); $i++) {
				if (!in_array($m_settings[$i], $mod_saved)) {
					array_push($file_array, "modules: " . $m_settings[$i] . ": " . implode(LAMConfig::LINE_SEPARATOR, $this->moduleSettings[$m_settings[$i]]) . "\n");
				}
			}
			// check if all type settings were added
			$t_settings = array_keys($this->typeSettings);
			for ($i = 0; $i < sizeof($t_settings); $i++) {
				if (!in_array($t_settings[$i], $mod_saved)) {
					array_push($file_array, "types: " . $t_settings[$i] . ": " . $this->typeSettings[$t_settings[$i]] . "\n");
				}
			}
			$file = @fopen($conffile, "w");
			if ($file) {
				for ($i = 0; $i < sizeof($file_array); $i++) fputs($file, $file_array[$i]);
				fclose($file);
				@chmod ($conffile, 0600);
				return LAMConfig::SAVE_OK;
			}
			else {
				return LAMConfig::SAVE_FAIL;
			}
		}
	}
	
	/**
	 * Returns the name of the config file
	 *
	 * @return String name
	 */
	public function getName() {
		return $this->file;
	}
	
	/**
	 * Returns if the file can be written on the filesystem.
	 *
	 * @return boolean true if file is writable
	 */
	public function isWritable() {
		return is_writeable($this->getPath());
	}
	
	/**
	 * Returns the path to the config file.
	 *
	 * @return string path on filesystem
	 */
	public function getPath() {
		return substr(__FILE__, 0, strlen(__FILE__) - 15) . "/config/" . $this->file . ".conf";
	}

	// functions to read/write preferences

	/**
	* Returns the server address as string
	*
	* @return string server address
	*/
	public function get_ServerURL() {
		return $this->ServerURL;
	}

	/**
	* Sets the server address
	*
	* @param string $value new server address
	* @return boolean true if $value has correct format
	*/
	public function set_ServerURL($value) {
		if (is_string($value)) $this->ServerURL = $value;
		else return false;
		return true;
	}
	
	/**
	 * Returns if TLS is activated.
	 * 
	 * @return String yes or no
	 */
	public function getUseTLS() {
		return $this->useTLS;
	}
	
	/**
	 * Sets if TLS is activated.
	 * 
	 * @param String yes or no
	 * @return boolean true if $useTLS has correct format
	 */
	public function setUseTLS($useTLS) {
		if (($useTLS == "yes") || ($useTLS == "no")) {
			$this->useTLS = $useTLS;
			return true;
		}
		return false;
	}


	/**
	* Returns an array of string with all admin names
	*
	* @return array the admin names
	*/
	public function get_Admins() {
		return explode(";", $this->Admins);
	}

	/**
	* Returns all admin users seperated by semicolons
	*
	* @return string the admin string
	*/
	public function get_Adminstring() {
		return $this->Admins;
	}

	/**
	* Sets the admin string
	*
	* @param string $value new admin string that contains all admin users seperated by semicolons
	* @return boolean true if $value has correct format
	*/
	public function set_Adminstring($value) {
		if (is_string($value) &&
		preg_match("/^[^;]+(;[^;]+)*$/", $value)) {
			$this->Admins = $value;
		}
		else return false;
		return true;
	}

	/**
	 * Checks if the given password matches.
	 *
	 * @param String $password
	 * @return boolean true, if matches
	 */
	public function check_Passwd($password) {
		if (substr($this->Passwd, 0, 6) == "{SSHA}") {
			// check hashed password
			$value = substr($this->Passwd, 6);
			$parts = explode(" ", $value);
			$salt = base64_decode($parts[1]);
			return ($this->hashPassword($password, $salt) === $this->Passwd);
		}
		else {
			// old nonhashed password
			return ($password === $this->Passwd);
		}
	}

	/**
	* Sets the preferences wizard password
	*
	* @param string $value new password
	* @return boolean true if $value has correct format
	*/
	public function set_Passwd($value) {
		if (is_string($value)) {
			mt_srand((microtime() * 1000000));
			$rand = mt_rand();
			$salt0 = substr(pack("h*", md5($rand)), 0, 8);
			$salt = substr(pack("H*", sha1($salt0 . $value)), 0, 4);
			$this->Passwd = $this->hashPassword($value, $salt);
			return true;
		}
		else {
			return false;		
		}
	}

	/**
	 * Returns the hashed password.
	 *
	 * @param String $password password
	 * @param String $salt salt
	 * @return String hash value
	 */
	private function hashPassword($password, $salt) {
		return "{SSHA}" . base64_encode(hex2bin(sha1($password . $salt))) . " " . base64_encode($salt);
	}

	/**
	* Returns the LDAP suffix for the given account type
	*
	* @param string $scope account type
	* @return string the LDAP suffix
	*/
	public function get_Suffix($scope) {
		if ($scope == "tree") {
			return $this->treesuffix;
		}
		else {
			return $this->typeSettings['suffix_' . $scope];
		}
	}

	/**
	* Sets the LDAP suffix where accounts are saved
	*
	* @param string $scope account type
	* @param string $value new LDAP suffix
	* @return boolean true if $value has correct format
	*/
	public function set_Suffix($scope, $value) {
		if (!$value) $value = "";
		elseif (!is_string($value)) {
			return false;
		}
		if ($scope == "tree") {
			$this->treesuffix = $value;
		}
		else {
			$this->typeSettings['suffix_' . $scope] = $value;
		}
		return true;
	}

	/**
	* Returns the list of attributes to show in user list
	*
	* @param string $scope account type
	* @return string the attribute list
	*/
	public function get_listAttributes($scope) {
		return $this->typeSettings['attr_' . $scope];
	}

	/**
	* Sets the list of attributes to show in user list
	*
	* @param string $value new attribute string
	* @param string $scope account type
	* @return boolean true if $value has correct format
	*/
	public function set_listAttributes($value, $scope) {
		if (is_string($value) && preg_match("/^((#[^:;]+)|([^:;]*:[^:;]+))(;((#[^:;]+)|([^:;]*:[^:;]+)))*$/", $value)) {
			$this->typeSettings['attr_' . $scope] = $value;
			return true;
		}
		else {
			return false;
		}
	}

	/**
	* Returns the default language string
	*
	* @return string default language
	*/
	public function get_defaultLanguage() {
		return $this->defaultLanguage;
	}

	/**
	* Sets the default language string
	*
	* @param string $value new default language
	* @return boolean true if $value has correct format
	*/
	public function set_defaultLanguage($value) {
		if (is_string($value)) $this->defaultLanguage = $value;
		else return false;
		return true;
	}

	/**
	* Returns the path to the external script
	*
	* @return string script path
	*/
	public function get_scriptPath() {
		return $this->scriptPath;
	}

	/**
	* Sets the path to the external script
	*
	* @param string $value new script path
	* @return boolean true if $value has correct format
	*/
	public function set_scriptPath($value) {
		if (!$value) $this->scriptPath = ""; // optional parameter
		elseif (is_string($value) && preg_match("/^\\/([a-z0-9_-])+(\\/([a-z0-9_\\.-])+)+$/i", $value)) $this->scriptPath = $value;
		else return false;
		return true;
	}
	
	/**
	* Returns the servers of the external script as a Array
	*
	* @return string script servers
	*/
	public function get_scriptServers() {
	    return $this->scriptServer;
	}
	
	/**
	* Sets the servers of the external script
	*
	* @param string $value new script servers
	* @return boolean true if $value has correct format
	*/
	public function set_scriptServers($value) {
		if (!$value) {
			$this->scriptServer = ""; // optional parameter
			return true;
		}
		// Explode the value to an array
		$array_string = explode(";", $value);
		if (count($array_string) > 0) {
			// Check all IPs in the exploded Array
			$valid_ips = array();
			foreach($array_string as $arr_value) {
				// Explode name and IP, if a name exists
				if (preg_match("/:/", $arr_value)) {
					$arr_value_explode = explode(":", $arr_value);
					$ip = $arr_value_explode[1];
					$servername = $arr_value_explode[0];
				}
				else {
			 		$ip = $arr_value;
					$servername = "";
				}
				if (isset($ip) && is_string($ip) && preg_match("/^[a-z0-9-]+(\\.[a-z0-9-]+)*(,[0-9]+)?$/i", $ip)) {
					// Check if the IP has a server name
					if (!empty($servername)) {
						$valid_ips[] = $servername.":".$ip;
					}
					else {
						$valid_ips[] = $ip;
					}
				}
				else {
					// wrong format
					return false;
				}
			}
			// Check that the array is not empty
			if ($array_string > 0) {
				$this->scriptServer = implode(";", $valid_ips);
				return true;
			}
			else {
				// The array is empty, there was no valid IP
				return false;
			}
		}
		else {
			return false;
		}
	}
	
	/**
	 * Returns the chmod value for new home directories.
	 * 
	 * @return string rights
	 */
	public function get_scriptRights() {
		if (!isset($this->scriptRights)) return '755';
		return $this->scriptRights;
	}

	/**
	* Sets the rights for the home directory.
	*
	* @param string $chmod the rights
	* @return boolean true if values has correct format
	*/
	public function set_scriptRights($chmod) {
		// check if the chmod is correct:
		if ($chmod > 0 && $chmod <=777) {
		    $this->scriptRights=$chmod;
		    return true;
		}
		else {
		    return false;
		}
	}

	/**
	* Returns the LDAP cache timeout in minutes
	*
	* @return integer cache time
	*/
	public function get_cacheTimeout() {
		if (isset($this->cachetimeout)) return $this->cachetimeout;
		else return 5;
	}

	/**
	* Returns the LDAP cache timeout in seconds
	*
	* @return integer cache time
	*/
	public function get_cacheTimeoutSec() {
		return $this->cachetimeout * 60;
	}

	/**
	* Sets the LDAP cache timeout in minutes (0,1,2,5,10,15)
	*
	* @param integer $value new cache timeout
	* @return boolean true if $value has correct format
	*/
	public function set_cacheTimeout($value) {
		if (is_numeric($value) && ($value > -1)) {
			$this->cachetimeout = $value;
		}
		else return false;
		return true;
	}

	/**
	* Returns the LDAP search limit.
	*
	* @return integer search limit
	*/
	public function get_searchLimit() {
		return $this->searchLimit;
	}

	/**
	* Sets the LDAP search limit.
	*
	* @param integer $value new search limit
	* @return boolean true if $value has correct format
	*/
	public function set_searchLimit($value) {
		if (is_numeric($value) && ($value > -1)) {
			$this->searchLimit = $value;
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
	public function get_AccountModules($scope) {
		if (isset($this->typeSettings["modules_" . $scope])) {
			return explode(",", $this->typeSettings["modules_" . $scope]);
		}
		else {
			return array();
		}
	}

	/**
	* Sets the selected account modules
	*
	* @param array $modules array with module names (not aliases!)
	* @param string $scope account type
	* @return boolean true if $modules has correct format
	*/
	public function set_AccountModules($modules, $scope) {
		if (! is_array($modules)) return false;
		// check module names
		$available = getAvailableModules($scope);
		for ($i = 0; $i < sizeof($modules); $i++) {
			if (! in_array($modules[$i], $available)) return false;
		}
		// check depends/conflicts
		if (check_module_conflicts($modules, getModulesDependencies($scope)) != false) return false;
		if (check_module_depends($modules, getModulesDependencies($scope)) != false) return false;
		$this->typeSettings["modules_" . $scope] = implode(",", $modules);
		return true;
	}

	/**
	* Sets the settings for the account modules.
	*
	* @param array $settings list of module setting array(name => value)
	* @return boolean true if $settings has correct format
	*/
	public function set_moduleSettings($settings) {
		if (!is_array($settings)) return false;
		$this->moduleSettings = $settings;
		return true;
	}

	/**
	* Returns a list of saved module settings
	*
	* @return array list of settings: array(name => value)
	*/
	public function get_moduleSettings() {
		return $this->moduleSettings;
	}

	/**
	* Returns a list of active account types.
	*
	* @return array list of types
	*/
	public function get_ActiveTypes() {
		if (($this->activeTypes == '') || !isset($this->activeTypes)) return array();
		else return explode(",", $this->activeTypes);
	}

	/**
	* Sets the list of active types.
	*
	* @param array list of types
	*/
	public function set_ActiveTypes($types) {
		$this->activeTypes = implode(",", $types);
	}

	/**
	* Sets the settings for the account types.
	*
	* @param array $settings list of type setting array(name => value)
	* @return boolean true if $settings has correct format
	*/
	public function set_typeSettings($settings) {
		if (!is_array($settings)) return false;
		$this->typeSettings = $settings;
		return true;
	}

	/**
	* Returns a list of saved type settings
	*
	* @return array list of settings: array(name => value)
	*/
	public function get_typeSettings() {
		return $this->typeSettings;
	}

	/**
	 * Returns the access level for this profile.
	 *
	 * @return int level
	 */
	public function getAccessLevel() {
		return $this->accessLevel;
	}
	
	/**
	 * Sets the access level for this profile.
	 *
	 * @param int $level level
	 */
	public function setAccessLevel($level) {
		$this->accessLevel = $level;
	}
	
	/**
	 * Returns the login method.
	 * 
	 * @return String login method
	 * @see LAMconfig::LOGIN_LIST
	 * @see LAMconfig::LOGIN_SEARCH
	 */
	public function getLoginMethod() {
		return $this->loginMethod;
	}
	
	/**
	 * Sets the login method.
	 * 
	 * @param String $loginMethod
	 */
	public function setLoginMethod($loginMethod) {
		$this->loginMethod = $loginMethod;
	}
	
	/**
	 * Returns the login search filter.
	 * 
	 * @return String search filter
	 */
	public function getLoginSearchFilter() {
		return $this->loginSearchFilter;
	}
	
	/**
	 * Sets the login search filter.
	 * 
	 * @param String $loginSearchFilter search filter
	 */
	public function setLoginSearchFilter($loginSearchFilter) {
		$this->loginSearchFilter = $loginSearchFilter;
	}
	
	/**
	 * Returns if HTTP authentication should be used.
	 * 
	 * @return String $httpAuthentication use HTTP authentication ('true' or 'false')
	 */
	public function getHttpAuthentication() {
		return $this->httpAuthentication;
	}

	/**
	 * Specifies if HTTP authentication should be used.
	 * 
	 * @param String $httpAuthentication use HTTP authentication ('true' or 'false')
	 */
	public function setHttpAuthentication($httpAuthentication) {
		$this->httpAuthentication = $httpAuthentication;
	}

	/**
	 * Returns the login search suffix.
	 * 
	 * @return String suffix
	 */
	public function getLoginSearchSuffix() {
		return $this->loginSearchSuffix;
	}
	
	/**
	 * Sets the login search suffix.
	 * 
	 * @param String $loginSearchSuffix suffix
	 */
	public function setLoginSearchSuffix($loginSearchSuffix) {
		$this->loginSearchSuffix = $loginSearchSuffix;
	}
	
	/**
	 * Returns the sender address for password reset mails.
	 * 
	 * @return String mail address
	 */
	public function getLamProMailFrom() {
		return $this->lamProMailFrom;
	}
	
	/**
	 * Sets the sender address for password reset mails.
	 * 
	 * @param String $lamProMailFrom mail address
	 * @return boolean true if address is valid
	 */
	public function setLamProMailFrom($lamProMailFrom) {
		$this->lamProMailFrom = $lamProMailFrom;
		if (($lamProMailFrom != '') && !get_preg($lamProMailFrom, 'email') && !get_preg($lamProMailFrom, 'emailWithName')) {
			return false;
		}
		return true;
	}
	
	/**
	 * Returns the reply-to address for password reset mails.
	 * 
	 * @return String mail address
	 */
	public function getLamProMailReplyTo() {
		return $this->lamProMailReplyTo;
	}

	/**
	 * Sets the reply-to address for password reset mails.
	 * 
	 * @param String $lamProMailReplyTo mail address
	 * @return boolean true if address is valid
	 */
	public function setLamProMailReplyTo($lamProMailReplyTo) {
		$this->lamProMailReplyTo = $lamProMailReplyTo;
		if (($lamProMailReplyTo != '') && !get_preg($lamProMailReplyTo, 'email') && !get_preg($lamProMailReplyTo, 'emailWithName')) {
			return false;
		}
		return true;
	}

	/**
	 * Returns the subject for password reset mails.
	 * 
	 * @return String subject
	 */
	public function getLamProMailSubject() {
		return $this->lamProMailSubject;
	}
	
	/**
	 * Sets the subject for password reset mails.
	 * 
	 * @param String $lamProMailSubject subject
	 */
	public function setLamProMailSubject($lamProMailSubject) {
		$this->lamProMailSubject = $lamProMailSubject;
	}

	/**
	 * Returns if the password reset mail content should be treated as HTML.
	 * 
	 * @return boolean HTML or text
	 */
	public function getLamProMailIsHTML() {
		return $this->lamProMailIsHTML;
	}

	/**
	 * Sets if the password reset mail content should be treated as HTML.
	 * 
	 * @param boolean $lamProMailIsHTML
	 */
	public function setLamProMailIsHTML($lamProMailIsHTML) {
		$this->lamProMailIsHTML = $lamProMailIsHTML;
	}
	
	/**
	 * Returns the mail body for password reset mails.
	 * 
	 * @return String body
	 */
	public function getLamProMailText() {
		return implode("\r\n", explode(LAMConfig::LINE_SEPARATOR, $this->lamProMailText));
	}
	
	/**
	 * Sets the mail body for password reset mails.
	 * 
	 * @param String $lamProMailText body
	 */
	public function setLamProMailText($lamProMailText) {
		$this->lamProMailText = implode(LAMConfig::LINE_SEPARATOR, explode("\r\n", $lamProMailText));
	}

}


/**
* This class manages config.cfg.
*
* @package configuration
*/
class LAMCfgMain {

	/** Default profile */
	public $default;

	/** Password to change config.cfg */
	private $password;

	/** Time of inactivity before session times out (minutes) */
	public $sessionTimeout;

	/** log level */
	public $logLevel;

	/** log destination ("SYSLOG":syslog, "/...":file, "NONE":none) */
	public $logDestination;

	/** list of hosts which may access LAM */
	public $allowedHosts;
	
	/** minimum length for passwords */
	public $passwordMinLength = 0;
	
	/** minimum uppercase characters */
	public $passwordMinUpper = 0;

	/** minimum lowercase characters */
	public $passwordMinLower = 0;

	/** minimum numeric characters */
	public $passwordMinNumeric = 0;

	/** minimum symbol characters */
	public $passwordMinSymbol = 0;

	/** minimum character classes (upper, lower, numeric, symbols) */
	public $passwordMinClasses = 0;
	
	private $conffile;

	/** list of data fields to save in config file */
	private $settings = array("password", "default", "sessionTimeout",
		"logLevel", "logDestination", "allowedHosts", "passwordMinLength",
		"passwordMinUpper", "passwordMinLower", "passwordMinNumeric",
		"passwordMinClasses", "passwordMinSymbol");

	/**
	* Loads preferences from config file
	*/
	function __construct() {
		$this->conffile = substr(__FILE__, 0, strlen(__FILE__) - 15) . "/config/config.cfg";
		// set default values
		$this->sessionTimeout = 30;
		$this->logLevel = LOG_NOTICE;
		$this->logDestination = "SYSLOG";
		$this->allowedHosts = "";
		$this->reload();
	}

	/**
	* Reloads preferences from config file config.cfg
	*
	* @return boolean true if file was readable
	*/
	private function reload() {
		if (is_file($this->conffile) == True) {
			$file = @fopen($this->conffile, "r");
			if (!$file) return false; // abort if file is not readable
			while (!feof($file)) {
				$line = fgets($file, 1024);
				$line = trim($line);  // remove spaces at the beginning and end
				if (($line == "")||($line[0] == "#")) continue; // ignore comments
				// search keywords
				for ($i = 0; $i < sizeof($this->settings); $i++) {
					$keyword = $this->settings[$i];
					$keylen = strlen($keyword);
					if (strtolower(substr($line, 0, $keylen + 2)) == strtolower($keyword . ": ")) {
						$this->$keyword = substr($line, $keylen + 2, strlen($line) - $keylen - 2);
						break;
					}
				}
			}
			fclose($file);
		}
		return true;
	}

	/**
	* Saves preferences to config file config.cfg
	*/
	public function save() {
		if (is_file($this->conffile) == True) {
			$file = fopen($this->conffile, "r");
			$file_array = array();
			// read config file
			while (!feof($file)) {
				array_push($file_array, fgets($file, 1024));
			}
			fclose($file);
			// generate new configuration file
			$saved = array();
			for ($i = 0; $i < sizeof($file_array); $i++) {
				$line = trim($file_array[$i]);
				if (($line == "")||($line[0] == "#")) continue; // ignore comments and empty lines
				// search keywords
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
		}
		// check if we have to add new entries (e.g. if user upgraded LAM and has an old config file)
		if (!in_array("password", $saved)) array_push($file_array, "\n\n# password to add/delete/rename configuration profiles\n" . "password: " . $this->password);
		if (!in_array("default", $saved)) array_push($file_array, "\n\n# default profile, without \".conf\"\n" . "default: " . $this->default);
		if (!in_array("sessionTimeout", $saved)) array_push($file_array, "\n\n# session timeout in minutes\n" . "sessionTimeout: " . $this->sessionTimeout);
		if (!in_array("logLevel", $saved)) array_push($file_array, "\n\n# log level\n" . "logLevel: " . $this->logLevel);
		if (!in_array("logDestination", $saved)) array_push($file_array, "\n\n# log destination\n" . "logDestination: " . $this->logDestination);
		if (!in_array("allowedHosts", $saved)) array_push($file_array, "\n\n# list of hosts which may access LAM\n" . "allowedHosts: " . $this->allowedHosts);
		if (!in_array("passwordMinLength", $saved)) array_push($file_array, "\n\n# Password: minimum password length\n" . "passwordMinLength: " . $this->passwordMinLength);
		if (!in_array("passwordMinUpper", $saved)) array_push($file_array, "\n\n# Password: minimum uppercase characters\n" . "passwordMinUpper: " . $this->passwordMinUpper);
		if (!in_array("passwordMinLower", $saved)) array_push($file_array, "\n\n# Password: minimum lowercase characters\n" . "passwordMinLower: " . $this->passwordMinLower);
		if (!in_array("passwordMinNumeric", $saved)) array_push($file_array, "\n\n# Password: minimum numeric characters\n" . "passwordMinNumeric: " . $this->passwordMinNumeric);
		if (!in_array("passwordMinSymbol", $saved)) array_push($file_array, "\n\n# Password: minimum symbolic characters\n" . "passwordMinSymbol: " . $this->passwordMinSymbol);
		if (!in_array("passwordMinClasses", $saved)) array_push($file_array, "\n\n# Password: minimum character classes (0-4)\n" . "passwordMinClasses: " . $this->passwordMinClasses);
		$file = @fopen($this->conffile, "w");
		if ($file) {
			for ($i = 0; $i < sizeof($file_array); $i++) fputs($file, $file_array[$i]);
			fclose($file);
		}
		else {
			StatusMessage("ERROR", "", _("Cannot open config file!") . " (" . $this->conffile . ")");
		}
	}
	
	/**
	 * Sets a new config password.
	 *
	 * @param String $password new password
	 */
	public function setPassword($password) {
		mt_srand((microtime() * 1000000));
		$rand = mt_rand();
		$salt0 = substr(pack("h*", md5($rand)), 0, 8);
		$salt = substr(pack("H*", sha1($salt0 . $password)), 0, 4);
		$this->password = $this->hashPassword($password, $salt);	
	}
	
	/**
	 * Checks if the given password matches.
	 *
	 * @param String $password password
	 * @return boolean true, if password matches
	 */
	public function checkPassword($password) {
		if (substr($this->password, 0, 6) == "{SSHA}") {
			// check hashed password
			$value = substr($this->password, 6);
			$parts = explode(" ", $value);
			$salt = base64_decode($parts[1]);
			return ($this->hashPassword($password, $salt) === $this->password);
		}
		else {
			// old nonhashed password
			return ($password === $this->password);
		}
	}
	
	/**
	 * Returns the hashed password.
	 *
	 * @param String $password password
	 * @param String $salt salt
	 * @return String hash value
	 */
	private function hashPassword($password, $salt) {
		return "{SSHA}" . base64_encode(hex2bin(sha1($password . $salt))) . " " . base64_encode($salt);
	}
	
	/**
	 * Returns if the configuration file is writable.
	 *
	 * @return boolean writable
	 */
	public function isWritable() {
		return is_writeable($this->conffile);
	}
	
}

?>
