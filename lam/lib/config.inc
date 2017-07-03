<?php
use \LAM\LIB\TWO_FACTOR\TwoFactorProviderService;
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2017  Roland Gruber

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
include_once "status.inc";
/** Used to get module information. */
include_once "modules.inc";
/** Used to get type information. */
include_once "types.inc";
/** 2-factor */
include_once '2factor.inc';

/**
 * Sets the environment variables for custom SSL CA certificates.
 */
function setSSLCaCert() {
	// set SSL certificate if set
	if (isset($_SESSION['cfgMain'])) {
		$sslCaPath = $_SESSION['cfgMain']->getSSLCaCertPath();
		if ($sslCaPath != null) {
			putenv('LDAPTLS_CACERT=' . $sslCaPath);
			putenv('TLS_CACERT=' . $sslCaPath);
		}
	}
}

/**
* Sets language settings for automatic translation
*/
function setlanguage() {
	$code = 'en_GB.utf8';
	$encoding = 'UTF-8';
	if (!isset($_SESSION['language'])) {
		$_SESSION['language'] = "en_GB.utf8";
	}
	$possibleLanguages = getLanguages();
	foreach ($possibleLanguages as $lang) {
		if ($lang->code == $_SESSION['language']) {
			$code = $lang->code;
			$encoding = $lang->encoding;
			break;
		}
	}
	putenv("LANG=" . $code);  // e.g. LANG=de_DE
	setlocale(LC_ALL, $code);  // set LC_ALL
	// workaround for buggy PHP with Turkish
	if (($code == 'tr_TR.utf8') && (version_compare(phpversion(), '5.5') < 0)) {
		setlocale(LC_CTYPE, 'en_GB');
	}
	$locdir = substr(__FILE__, 0, strlen(__FILE__) - 15) . "/locale";  // set path to translations
	bindtextdomain("messages", $locdir);
	bind_textdomain_codeset("messages", $encoding);
	textdomain("messages");
	header("Content-type: text/html; charset=" . $encoding, true);
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
* Returns an array of string with all available configuration templates (without .conf.sample)
*
* @return array template names
*/
function getConfigTemplates() {
	$dir = dir(dirname(__FILE__) . "/../config");
	$ret = array();
	$pos = 0;
	while ($entry = $dir->read()){
		$ext = substr($entry, strlen($entry)-12, 12);
		$name = substr($entry, 0, strlen($entry) - 12);
		// check if extension is right, add to profile list
		if ($ext == ".conf.sample") {
			$ret[$pos] = $name;
			$pos ++;
		}
	}
	sort($ret);
	return $ret;
}

/**
 * Creates a new server profile.
 *
 * @param String $name profile name
 * @param String $password profile password
 * @param String $template name of template file
 * @return mixed Boolean TRUE if creation was ok, error message if not
 */
function createConfigProfile($name, $password, $template) {
	if (!preg_match("/^[a-z0-9_-]+$/i", $name) || !preg_match("/^[a-z0-9\\._-]+$/i", $template) || in_array($name, getConfigProfiles())) {
		return _("Profile name is invalid!");
	}
	$dir = dirname(dirname(__FILE__)) . "/config/";
	// check if template exists
	if (!is_file($dir . $template)) {
		return "The file config/$template was not found. Please restore it.";
	}
	// create new profile file
	$path = $dir . $name . ".conf";
	@copy($dir . $template, $path);
	@chmod ($path, 0600);
	$file = is_file($path);
	if ($file) {
		// load as config and write new password
		$conf = new LAMConfig($name);
		$conf->set_Passwd($password);
		$conf->save();
	}
	else {
		return _("Unable to create new profile!");
	}
	return true;
}

/**
 * Deletes the given server profile.
 *
 * @param String $name profile name
 * @return String null if success or error message if failed
 */
function deleteConfigProfile($name) {
	if (!preg_match("/^[a-z0-9_-]+$/i", $name)) {
		return _("Unable to delete profile!");
	}
	$dir = dirname(dirname(__FILE__)) . "/config/";
	// delete account profiles and PDF structures
	$subDirs = array($dir . 'pdf/' . $name . '/logos', $dir . 'pdf/' . $name, $dir . 'profiles/' . $name);
	for ($i = 0; $i < sizeof($subDirs); $i++) {
		if (is_dir($subDirs[$i])) {
			$dirHandle = @opendir($subDirs[$i]);
			while (false !== ($path = readdir($dirHandle))) {
				if ($path != '.'  && $path != '..') {
					if (!@unlink($subDirs[$i] . '/' . $path)) {
						logNewMessage(LOG_ERR, 'Unable to delete ' . $subDirs[$i] . '/' . $path);
						return _("Unable to delete profile!");
					}
				}
			}
			@closedir($dirHandle);
			if (!@rmdir($subDirs[$i])) {
				logNewMessage(LOG_ERR, 'Unable to delete ' . $subDirs[$i]);
				return _("Unable to delete profile!");
			}
		}
	}
	// delete config file
	$confFile = $dir . $name . ".conf";
	if (!@unlink($confFile)) {
		logNewMessage(LOG_ERR, 'Unable to delete ' . $confFile);
		return _("Unable to delete profile!");
	}
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
 * Extracts config options from HTTP POST data.
 *
 * @param array $confTypes array (option name => type (e.g. multiselect))
 * @return array list of config options (name => array(values))
 */
function extractConfigOptionsFromPOST($confTypes) {
	$options = array();
	foreach ($confTypes as $element => $type) {
		// text fields
		if ($type == "text") {
			$options[$element] = array($_POST[$element]);
		}
		// text fields
		elseif ($type == "text_obfuscated") {
			$options[$element] = array(obfuscateText($_POST[$element]));
		}
		// hidden fields
		elseif ($type == "hidden") {
			$options[$element] = array($_POST[$element]);
		}
		// checkboxes
		elseif ($type == "checkbox") {
			if (isset($_POST[$element]) && ($_POST[$element] == "on")) $options[$element] = array('true');
			else $options[$element] = array('false');
		}
		// dropdownbox
		elseif ($type == "select") {
			$options[$element] = array($_POST[$element]);
		}
		// multiselect
		elseif ($type == "multiselect") {
			$options[$element] = $_POST[$element];  // value is already an array
		}
		// textarea
		elseif ($type == "textarea") {
			$options[$element] = explode("\r\n", $_POST[$element]);
		}
	}
	return $options;
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
 * Checks if the given account type is hidden.
 *
 * @param String $type account type (e.g. user)
 * @return boolean is hidden
 */
function isAccountTypeHidden($type) {
	$typeSettings = $_SESSION['config']->get_typeSettings();
	return isset($typeSettings['hidden_' . $type]) && ($typeSettings['hidden_' . $type] == true);
}

/**
 * Returns a list of all supported languages.
 *
 * @return array languages
 */
function getLanguages() {
	$languages = array();
	// loading available languages from language.conf file
	$languagefile = dirname(__FILE__) . "/../config/language";
	if(is_file($languagefile) == true) {
		$file = fopen($languagefile, "r");
		while(!feof($file)) {
			$line = fgets($file, 1024);
			if($line == "" || $line == "\n" || $line[0] == "#") continue; // ignore comment and empty lines
			$value = explode(":", $line);
			$languages[] = new LAMLanguage($value[0], $value[1], $value[2]);
		}
		fclose($file);
	}
	return $languages;
}

/**
 * Represents a supported language.
 *
 * @package configuration
 */
class LAMLanguage {
	/** language code (e.g. en_GB.utf8) */
	public $code;
	/** character encoding (e.g. UTF-8) */
	public $encoding;
	/** description for GUI */
	public $description;

	/**
	 * Constructor
	 *
	 * @param String $code language code (e.g. en_GB.utf8)
	 * @param String $encoding character encoding (e.g. UTF-8)
	 * @param String $description description for GUI
	 */
	public function __construct($code, $encoding, $description) {
		$this->code = $code;
		$this->encoding = $encoding;
		$this->description = $description;
	}

}

/**
* This class manages conf files.
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

	/** show password on screen by default */
	const PWDRESET_DEFAULT_SCREEN = 1;
	/** send password via email by default */
	const PWDRESET_DEFAULT_MAIL = 2;
	/** show password on screen and send via email by default */
	const PWDRESET_DEFAULT_BOTH = 3;

	/** Server address (e.g. ldap://127.0.0.1:389) */
	private $ServerURL;

	/** Display name of LDAP server */
	private $serverDisplayName;

	/** enables/disables TLS encryption */
	private $useTLS;

	/** automatically follow referrals */
	private $followReferrals = 'false';

	/** use paged results */
	private $pagedResults = 'false';

	/** Array of string: users with admin rights */
	private $Admins;

	/** Password to edit preferences */
	private $Passwd;

	/** LDAP suffix for tree view */
	private $treesuffix;

	/** Default language */
	private $defaultLanguage;
	/** time zone */
	private $timeZone = 'Europe/London';

	/** module settings */
	private $moduleSettings = array();

	/** type settings */
	private $typeSettings = array();

	/** tool settings */
	private $toolSettings = array();

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

	/**
	 * user name for lamdaemon
	 */
	private $scriptUserName;

	/**
	 * File name of SSH key for lamdaemon.
	 */
	private $scriptSSHKey;

	/**
	 * Password for lamdaemon SSH key.
	 */
	private $scriptSSHKeyPassword;

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

	/** bind user for login search */
	private $loginSearchDN = '';

	/** bind password for login search */
	private $loginSearchPassword = '';

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
	/** allow sending mails to an alternative address */
	private $lamProMailAllowAlternateAddress = 'true';
	/** mail body for password reset mails */
	private $lamProMailText = '';

	/** password reset page: allow to set a specific password */
	private $pwdResetAllowSpecificPassword = 'true';
	/** password reset page: allow to show password on screen */
	private $pwdResetAllowScreenPassword = 'true';
	/** password reset page: force password change by default */
	private $pwdResetForcePasswordChange = 'true';
	/** password reset page: default selection for password output
	 * PWDRESET_DEFAULT_SCREEN, PWDRESET_DEFAULT_MAIL, PWDRESET_DEFAULT_BOTH */
	private $pwdResetDefaultPasswordOutput = LAMconfig::PWDRESET_DEFAULT_MAIL;

	/** LDAP user for jobs */
	private $jobsBindUser = null;
	/** LDAP password for jobs */
	private $jobsBindPassword = null;
	/** database type for jobs */
	private $jobsDatabase = null;
	/** host of job database */
	private $jobsDBHost = null;
	/** port of job database */
	private $jobsDBPort = null;
	/** user of job database */
	private $jobsDBUser = null;
	/** password of job database */
	private $jobsDBPassword = null;
	/** name of job database */
	private $jobsDBName = null;
	/** random job token */
	private $jobToken = null;
	/** job configuration */
	private $jobSettings = array();

	private $twoFactorAuthentication = TwoFactorProviderService::TWO_FACTOR_NONE;
	private $twoFactorAuthenticationURL = 'https://localhost';
	private $twoFactorAuthenticationInsecure = false;
	private $twoFactorAuthenticationLabel = null;
	private $twoFactorAuthenticationOptional = false;
	private $twoFactorAuthenticationCaption = '';

	/** List of all settings in config file */
	private $settings = array("ServerURL", "useTLS", "followReferrals", 'pagedResults', "Passwd", "Admins", "treesuffix",
		"defaultLanguage", "scriptPath", "scriptServer", "scriptRights", "cachetimeout", 'serverDisplayName',
		"modules", "activeTypes", "types", "tools", "accessLevel", 'loginMethod', 'loginSearchSuffix',
		'loginSearchFilter', 'searchLimit', 'lamProMailFrom', 'lamProMailReplyTo', 'lamProMailSubject',
		'lamProMailText', 'lamProMailIsHTML', 'lamProMailAllowAlternateAddress', 'httpAuthentication', 'loginSearchDN',
		'loginSearchPassword', 'timeZone', 'jobsBindUser', 'jobsBindPassword', 'jobsDatabase', 'jobToken', 'jobs',
		'jobsDBHost', 'jobsDBPort', 'jobsDBUser', 'jobsDBPassword', 'jobsDBName', 'pwdResetAllowSpecificPassword',
		'pwdResetAllowScreenPassword', 'pwdResetForcePasswordChange', 'pwdResetDefaultPasswordOutput',
		'scriptUserName', 'scriptSSHKey', 'scriptSSHKeyPassword', 'twoFactorAuthentication', 'twoFactorAuthenticationURL',
		'twoFactorAuthenticationInsecure', 'twoFactorAuthenticationLabel', 'twoFactorAuthenticationOptional',
		'twoFactorAuthenticationCaption'
	);


	/**
	* Loads preferences from config file
	*
	* @param String $file file name without ".conf" (e.g. lam)
	*/
	function __construct($file) {
		if (empty($file) || !preg_match("/^[a-z0-9_-]+$/i", $file)) {
			logNewMessage(LOG_ERR, 'Invalid config file name: ' . $file);
			die();
		}
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
				$line = fgets($file, 1000000);
				$line = trim($line);  // remove spaces at the beginning and end
				if (($line == "")||($line[0] == "#")) continue; // ignore comments and empty lines
				// search keywords
				$parts = explode(': ', $line);
				$keyword = $parts[0];
				if (!in_array($keyword, $this->settings)) {
					continue;
				}
				$startIndex = strlen($keyword) + 2;
				if (sizeof($parts) == 1) {
					// empty global settings
					$this->$keyword = '';
				}
				elseif ((sizeof($parts) > 1) && !in_array($keyword, array('modules', 'types', 'tools', 'jobs'))) {
					// global setting with value
					$this->$keyword = substr($line, $startIndex);
				}
				else {
					$subKeyword = $parts[1];
					$startIndex = $startIndex + strlen($subKeyword) + 2;
					$option = substr($line, $startIndex);
					if (empty($option)) {
						continue;
					}
					// module settings
					if ($keyword == 'modules') {
						$this->moduleSettings[$subKeyword] = explode(LAMConfig::LINE_SEPARATOR, $option);
					}
					// type settings
					if ($keyword == 'types') {
						$this->typeSettings[$subKeyword] = $option;
					}
					// tool settings
					if ($keyword == 'tools') {
						$this->toolSettings[$subKeyword] = $option;
					}
					// job settings
					if ($keyword == 'jobs') {
						$this->jobSettings[$subKeyword] = explode(LAMConfig::LINE_SEPARATOR, $option);
					}
				}
			}
			fclose($file);
		}
		$this->removeInvalidTypes();
		$this->removeInvalidModules();
		return true;
	}

	/**
	 * Removes any non-existing types from the configuration.
	 */
	private function removeInvalidTypes() {
		$allTypes = LAM\TYPES\getTypes();
		$activeTypes = $this->get_ActiveTypes();
		for ($i = 0; $i < sizeof($activeTypes); $i++) {
			if (!in_array(\LAM\TYPES\getScopeFromTypeId($activeTypes[$i]), $allTypes)) {
				unset($activeTypes[$i]);
			}
		}
		$activeTypes = array_values($activeTypes);
		$this->set_ActiveTypes($activeTypes);
	}

	/**
	 * Removes any non-existing modules from the configuration.
	 */
	private function removeInvalidModules() {
		$types = $this->get_ActiveTypes();
		$availableByScope = array();
		foreach ($types as $type) {
			$scope = \LAM\TYPES\getScopeFromTypeId($type);
			$moduleVar = "modules_" . $type;
			if (isset($this->typeSettings[$moduleVar])){
				$modules = explode(",", $this->typeSettings[$moduleVar]);
				if (empty($availableByScope[$scope])) {
					$availableByScope[$scope] = getAvailableModules($scope);
				}
				$available = $availableByScope[$scope];
				// only return available modules
				$ret = array();
				for ($i = 0; $i < sizeof($modules); $i++) {
					if (in_array($modules[$i], $available)) $ret[] = $modules[$i];
				}
				$this->typeSettings[$moduleVar] = implode(",", $ret);
			}
		}
	}

	/** Saves preferences to config file */
	public function save() {
		$conffile = $this->getPath();
		if (is_file($conffile) == True) {
			$file = fopen($conffile, "r");
			$file_array = array();
			// read config file
			while (!feof($file)) {
				array_push($file_array, fgets($file, 1000000));
			}
			fclose($file);
			// generate new configuration file
			$saved = array();	// includes all settings which have been saved
			$mod_saved = array();	// includes all module settings which have been saved
			$count = sizeof($file_array);
			for ($i = 0; $i < $count; $i++) {
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
							if (!isset($this->moduleSettings[$name])) {
								unset($file_array[$i]);
								continue;
							}
							$file_array[$i] = "modules: " . $name . ": " . implode(LAMConfig::LINE_SEPARATOR, $this->moduleSettings[$name]) . "\n";
							$mod_saved[] = $name;	// mark keyword as saved
						}
						// type settings
						elseif (strtolower(substr($line, 0, $keylen + 2)) == "types: ") {
							$option = substr($line, $keylen + 2, strlen($line) - $keylen - 2);
							$pos = strpos($option, ":");
							$name = substr($option, 0, $pos);
							if (!isset($this->typeSettings[$name])) {
								unset($file_array[$i]);
								continue;
							}
							$file_array[$i] = "types: " . $name . ": " . $this->typeSettings[$name] . "\n";
							$mod_saved[] = $name;	// mark keyword as saved
						}
						// tool settings
						elseif (strtolower(substr($line, 0, $keylen + 2)) == "tools: ") {
							$option = substr($line, $keylen + 2, strlen($line) - $keylen - 2);
							$pos = strpos($option, ":");
							$name = substr($option, 0, $pos);
							if (!isset($this->toolSettings[$name])) {
								unset($file_array[$i]);
								continue;
							}
							$file_array[$i] = "tools: " . $name . ": " . $this->toolSettings[$name] . "\n";
							$mod_saved[] = $name;	// mark keyword as saved
						}
						// job settings
						elseif (strtolower(substr($line, 0, $keylen + 2)) == "jobs: ") {
							$option = substr($line, $keylen + 2, strlen($line) - $keylen - 2);
							$pos = strpos($option, ":");
							$name = substr($option, 0, $pos);
							if (!isset($this->jobSettings[$name])) {
								unset($file_array[$i]);
								continue;
							}
							$file_array[$i] = "jobs: " . $name . ": " . implode(LAMConfig::LINE_SEPARATOR, $this->jobSettings[$name]) . "\n";
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
			if (!in_array("serverDisplayName", $saved)) array_push($file_array, "\n\nserverDisplayName: " . $this->serverDisplayName . "\n");
			if (!in_array("useTLS", $saved)) array_push($file_array, "\n\n# enable TLS encryption\n" . "useTLS: " . $this->useTLS . "\n");
			if (!in_array("followReferrals", $saved)) array_push($file_array, "\n\n# follow referrals\n" . "followReferrals: " . $this->followReferrals . "\n");
			if (!in_array("pagedResults", $saved)) array_push($file_array, "\n\n# paged results\n" . "pagedResults: " . $this->pagedResults . "\n");
			if (!in_array("Passwd", $saved)) array_push($file_array, "\n\n# password to change these preferences via webfrontend\n" . "passwd: " . $this->Passwd . "\n");
			if (!in_array("Admins", $saved)) array_push($file_array, "\n\n# list of users who are allowed to use LDAP Account Manager\n" .
								"# names have to be seperated by semicolons\n" .
								"# e.g. admins: cn=admin,dc=yourdomain,dc=org;cn=root,dc=yourdomain,dc=org\n" . "admins: " . $this->Admins . "\n");
			if (!in_array("treesuffix", $saved)) array_push($file_array, "\n\n# suffix of tree view\n" .
								"# e.g. dc=yourdomain,dc=org\n" . "treesuffix: " . $this->treesuffix . "\n");
			if (!in_array("defaultLanguage", $saved)) array_push($file_array, "\n\n# default language (a line from config/language)\n" . "defaultLanguage: " . $this->defaultLanguage . "\n");
			if (!in_array("timeZone", $saved)) array_push($file_array, "\n\n# time zone\n" . "timeZone: " . $this->timeZone . "\n");
			if (!in_array("scriptPath", $saved)) array_push($file_array, "\n\n# Path to external Script\n" . "scriptPath: " . $this->scriptPath . "\n");
			if (!in_array("scriptServer", $saved)) array_push($file_array, "\n\n# Servers of external script\n" . "scriptServer: " . $this->scriptServer . "\n");
			if (!in_array("scriptRights", $saved)) array_push($file_array, "\n\n# Access rights for home directories\n" . "scriptRights: " . $this->scriptRights . "\n");
			if (!in_array("scriptUserName", $saved)) array_push($file_array, "\n" . "scriptUserName: " . $this->scriptUserName . "\n");
			if (!in_array("scriptSSHKey", $saved)) array_push($file_array, "\n" . "scriptSSHKey: " . $this->scriptSSHKey . "\n");
			if (!in_array("scriptSSHKeyPassword", $saved)) array_push($file_array, "\n" . "scriptSSHKeyPassword: " . $this->scriptSSHKeyPassword . "\n");
			if (!in_array("cachetimeout", $saved)) array_push($file_array, "\n\n# Number of minutes LAM caches LDAP searches.\n" . "cacheTimeout: " . $this->cachetimeout . "\n");
			if (!in_array("searchLimit", $saved)) array_push($file_array, "\n\n# LDAP search limit.\n" . "searchLimit: " . $this->searchLimit . "\n");
			if (!in_array("activeTypes", $saved)) array_push($file_array, "\n\n# List of active account types.\n" . "activeTypes: " . $this->activeTypes . "\n");
			if (!in_array("accessLevel", $saved)) array_push($file_array, "\n\n# Access level for this profile.\n" . "accessLevel: " . $this->accessLevel . "\n");
			if (!in_array("loginMethod", $saved)) array_push($file_array, "\n\n# Login method.\n" . "loginMethod: " . $this->loginMethod . "\n");
			if (!in_array("loginSearchSuffix", $saved)) array_push($file_array, "\n\n# Search suffix for LAM login.\n" . "loginSearchSuffix: " . $this->loginSearchSuffix . "\n");
			if (!in_array("loginSearchFilter", $saved)) array_push($file_array, "\n\n# Search filter for LAM login.\n" . "loginSearchFilter: " . $this->loginSearchFilter . "\n");
			if (!in_array("loginSearchDN", $saved)) array_push($file_array, "\n\n# Bind DN for login search.\n" . "loginSearchDN: " . $this->loginSearchDN . "\n");
			if (!in_array("loginSearchPassword", $saved)) array_push($file_array, "\n\n# Bind password for login search.\n" . "loginSearchPassword: " . $this->loginSearchPassword . "\n");
			if (!in_array("httpAuthentication", $saved)) array_push($file_array, "\n\n# HTTP authentication for LAM login.\n" . "httpAuthentication: " . $this->httpAuthentication . "\n");
			if (!in_array("lamProMailFrom", $saved)) array_push($file_array, "\n\n# Password mail from\n" . "lamProMailFrom: " . $this->lamProMailFrom . "\n");
			if (!in_array("lamProMailReplyTo", $saved)) array_push($file_array, "\n\n# Password mail reply-to\n" . "lamProMailReplyTo: " . $this->lamProMailReplyTo . "\n");
			if (!in_array("lamProMailSubject", $saved)) array_push($file_array, "\n\n# Password mail subject\n" . "lamProMailSubject: " . $this->lamProMailSubject . "\n");
			if (!in_array("lamProMailIsHTML", $saved)) array_push($file_array, "\n\n# Password mail is HTML\n" . "lamProMailIsHTML: " . $this->lamProMailIsHTML . "\n");
			if (!in_array("lamProMailAllowAlternateAddress", $saved)) array_push($file_array, "\n\n# Allow alternate address\n" . "lamProMailAllowAlternateAddress: " . $this->lamProMailAllowAlternateAddress . "\n");
			if (!in_array("lamProMailText", $saved)) array_push($file_array, "\n\n# Password mail text\n" . "lamProMailText: " . $this->lamProMailText . "\n");
			if (!in_array("jobsBindPassword", $saved)) array_push($file_array, "\n" . "jobsBindPassword: " . $this->jobsBindPassword . "\n");
			if (!in_array("jobsBindUser", $saved)) array_push($file_array, "\n" . "jobsBindUser: " . $this->jobsBindUser . "\n");
			if (!in_array("jobsDatabase", $saved)) array_push($file_array, "\n" . "jobsDatabase: " . $this->jobsDatabase . "\n");
			if (!in_array("jobsDBHost", $saved)) array_push($file_array, "\n" . "jobsDBHost: " . $this->jobsDBHost . "\n");
			if (!in_array("jobsDBPort", $saved)) array_push($file_array, "\n" . "jobsDBPort: " . $this->jobsDBPort . "\n");
			if (!in_array("jobsDBUser", $saved)) array_push($file_array, "\n" . "jobsDBUser: " . $this->jobsDBUser . "\n");
			if (!in_array("jobsDBPassword", $saved)) array_push($file_array, "\n" . "jobsDBPassword: " . $this->jobsDBPassword . "\n");
			if (!in_array("jobsDBName", $saved)) array_push($file_array, "\n" . "jobsDBName: " . $this->jobsDBName . "\n");
			if (!in_array("jobToken", $saved)) array_push($file_array, "\n" . "jobToken: " . $this->getJobToken() . "\n");
			if (!in_array("pwdResetAllowSpecificPassword", $saved)) array_push($file_array, "\n" . "pwdResetAllowSpecificPassword: " . $this->pwdResetAllowSpecificPassword . "\n");
			if (!in_array("pwdResetAllowScreenPassword", $saved)) array_push($file_array, "\n" . "pwdResetAllowScreenPassword: " . $this->pwdResetAllowScreenPassword . "\n");
			if (!in_array("pwdResetForcePasswordChange", $saved)) array_push($file_array, "\n" . "pwdResetForcePasswordChange: " . $this->pwdResetForcePasswordChange . "\n");
			if (!in_array("pwdResetDefaultPasswordOutput", $saved)) array_push($file_array, "\n" . "pwdResetDefaultPasswordOutput: " . $this->pwdResetDefaultPasswordOutput . "\n");
			if (!in_array("twoFactorAuthentication", $saved)) array_push($file_array, "\n" . "twoFactorAuthentication: " . $this->twoFactorAuthentication . "\n");
			if (!in_array("twoFactorAuthenticationURL", $saved)) array_push($file_array, "\n" . "twoFactorAuthenticationURL: " . $this->twoFactorAuthenticationURL . "\n");
			if (!in_array("twoFactorAuthenticationInsecure", $saved)) array_push($file_array, "\n" . "twoFactorAuthenticationInsecure: " . $this->twoFactorAuthenticationInsecure . "\n");
			if (!in_array("twoFactorAuthenticationLabel", $saved)) array_push($file_array, "\n" . "twoFactorAuthenticationLabel: " . $this->twoFactorAuthenticationLabel . "\n");
			if (!in_array("twoFactorAuthenticationOptional", $saved)) array_push($file_array, "\n" . "twoFactorAuthenticationOptional: " . $this->twoFactorAuthenticationOptional . "\n");
			if (!in_array("twoFactorAuthenticationCaption", $saved)) array_push($file_array, "\n" . "twoFactorAuthenticationCaption: " . $this->twoFactorAuthenticationCaption . "\n");
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
			// check if all tool settings were added
			$tool_settings = array_keys($this->toolSettings);
			for ($i = 0; $i < sizeof($tool_settings); $i++) {
				if (!in_array($tool_settings[$i], $mod_saved)) {
					array_push($file_array, "tools: " . $tool_settings[$i] . ": " . $this->toolSettings[$tool_settings[$i]] . "\n");
				}
			}
			// check if all job settings were added
			$jobSettings = array_keys($this->jobSettings);
			for ($i = 0; $i < sizeof($jobSettings); $i++) {
				if (!in_array($jobSettings[$i], $mod_saved)) {
					array_push($file_array, "jobs: " . $jobSettings[$i] . ": " . implode(LAMConfig::LINE_SEPARATOR, $this->jobSettings[$jobSettings[$i]]) . "\n");
				}
			}
			$file = @fopen($conffile, "w");
			$saveResult = LAMConfig::SAVE_OK;
			if ($file) {
				foreach ($file_array as $line) {
					fputs($file, $line);
				}
				fclose($file);
				@chmod($conffile, 0600);
			}
			else {
				$saveResult = LAMConfig::SAVE_FAIL;
			}
			return $saveResult;
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
	* Returns the server display name. Defaults to server URL if empty display name.
	*
	* @return string server display name
	*/
	public function getServerDisplayNameGUI() {
		if (empty($this->serverDisplayName)) {
			return $this->ServerURL;
		}
		return $this->serverDisplayName;
	}

	/**
	* Returns the server display name.
	*
	* @return string server display name
	*/
	public function getServerDisplayName() {
		return $this->serverDisplayName;
	}

	/**
	* Sets the server display name
	*
	* @param string $value new server display name
	* @return boolean true if $value has correct format
	*/
	public function setServerDisplayName($value) {
		if (is_string($value)) $this->serverDisplayName = $value;
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
	 * @param String $useTLS yes or no
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
	 * Returns if referrals should be followed.
	 *
	 * @return String true or false
	 */
	public function getFollowReferrals() {
		return $this->followReferrals;
	}

	/**
	 * Sets if referrals should be followed.
	 *
	 * @param String $followReferrals true or false
	 */
	public function setFollowReferrals($followReferrals) {
		$this->followReferrals = $followReferrals;
	}

	/**
	 * Returns if paged results should be used.
	 *
	 * @return String true or false
	 */
	public function getPagedResults() {
		return $this->pagedResults;
	}

	/**
	 * Sets if paged results should be used.
	 *
	 * @param String $pagedResults true or false
	 */
	public function setPagedResults($pagedResults) {
		$this->pagedResults = $pagedResults;
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
			$rand = getRandomNumber();
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
		return "{SSHA}" . base64_encode(convertHex2bin(sha1($password . $salt))) . " " . base64_encode($salt);
	}

	/**
	* Returns the LDAP suffix for the given account type
	*
	* @param string $typeId account type
	* @return string the LDAP suffix
	*/
	public function get_Suffix($typeId) {
		if ($typeId == "tree") {
			return $this->treesuffix;
		}
		else {
			return $this->typeSettings['suffix_' . $typeId];
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
	* Returns the time zone name.
	*
	* @return string time zone
	*/
	public function getTimeZone() {
		return ($this->timeZone == null) ? 'Europe/London' : $this->timeZone;
	}

	/**
	* Sets the time zone name.
	*
	* @param string $value new time zone
	* @return boolean true if $value has correct format
	*/
	public function setTimeZone($value) {
		if (is_string($value)) {
			$this->timeZone = $value;
			return true;
		}
		return false;
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
	* Returns the path to lamdamon SSH key.
	*
	* @return string key path
	*/
	public function getScriptSSHKey() {
		return $this->scriptSSHKey;
	}

	/**
	* Sets the path to lamdamon SSH key.
	*
	* @param string $value key path
	*/
	public function setScriptSSHKey($value) {
		$this->scriptSSHKey = $value;
	}

	/**
	* Returns the password for the lamdamon SSH key.
	*
	* @return string password
	*/
	public function getScriptSSHKeyPassword() {
		return $this->scriptSSHKeyPassword;
	}

	/**
	* Sets the password for the lamdamon SSH key.
	*
	* @param string $value password
	*/
	public function setScriptSSHKeyPassword($value) {
		$this->scriptSSHKeyPassword = $value;
	}

	/**
	* Returns the lamdaemon user name.
	*
	* @return string user name
	*/
	public function getScriptUserName() {
		return $this->scriptUserName;
	}

	/**
	* Sets the lamdaemon user name.
	*
	* @param string $value user name
	*/
	public function setScriptUserName($value) {
		$this->scriptUserName = $value;
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
			$modulesTmp = explode(",", $this->typeSettings["modules_" . $scope]);
			$modules = array();
			foreach ($modulesTmp as $mod) {
				if (trim($mod) != '') {
					$modules[] = $mod;
				}
			}
			return $modules;
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
	 * Returns the tool settings.
	 *
	 * @return array $toolSettings tool settings
	 */
	public function getToolSettings() {
		return $this->toolSettings;
	}

	/**
	 * Sets the tool settings.
	 *
	 * @param array $toolSettings tool settings
	 * @return boolean true if ok
	 */
	public function setToolSettings($toolSettings) {
		if (!is_array($toolSettings)) return false;
		$this->toolSettings = $toolSettings;
		return true;
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
	 * Sets the DN for the login search bind user.
	 *
	 * @param String $loginSearchDN DN
	 * @return boolean true if DN is valid
	 */
	public function setLoginSearchDN($loginSearchDN) {
		$this->loginSearchDN = $loginSearchDN;
		if (($loginSearchDN == '') || get_preg($loginSearchDN, 'dn')) {
			return true;
		}
		return false;
	}

	/**
	 * Returns the DN for the login search bind user.
	 *
	 * @return String DN
	 */
	public function getLoginSearchDN() {
		return $this->loginSearchDN;
	}

	/**
	 * Sets the password for the login search bind user.
	 *
	 * @param String $loginSearchPassword password
	 */
	public function setLoginSearchPassword($loginSearchPassword) {
		$this->loginSearchPassword = $loginSearchPassword;
	}

	/**
	 * Returns the password for the login search bind user.
	 *
	 * @return String password
	 */
	public function getLoginSearchPassword() {
		return $this->loginSearchPassword;
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
	 * @param boolean $lamProMailIsHTML true if HTML
	 */
	public function setLamProMailIsHTML($lamProMailIsHTML) {
		$this->lamProMailIsHTML = $lamProMailIsHTML;
	}

	/**
	 * Returns if sending to an alternate address is allowed.
	 *
	 * @return boolean alternate address allowed
	 */
	public function getLamProMailAllowAlternateAddress() {
		return $this->lamProMailAllowAlternateAddress;
	}

	/**
	 * Sets if sending to an alternate address is allowed.
	 *
	 * @param boolean $lamProMailAllowAlternateAddress alternate address allowed
	 */
	public function setLamProMailAllowAlternateAddress($lamProMailAllowAlternateAddress) {
		$this->lamProMailAllowAlternateAddress = $lamProMailAllowAlternateAddress;
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

	/**
	 * Returns the bind user for jobs.
	 *
	 * @return String $jobsBindUser bind user
	 */
	public function getJobsBindUser() {
		return $this->jobsBindUser;
	}

	/**
	 * Sets the bind user for jobs.
	 *
	 * @param String $jobsBindUser bind user
	 */
	public function setJobsBindUser($jobsBindUser) {
		$this->jobsBindUser = $jobsBindUser;
	}

	/**
	 * Returns the bind password for jobs.
	 *
	 * @return String $jobsBindPassword password
	 */
	public function getJobsBindPassword() {
		return $this->jobsBindPassword;
	}

	/**
	 * Sets the bind password for jobs.
	 *
	 * @param String $jobsBindPassword password
	 */
	public function setJobsBindPassword($jobsBindPassword) {
		$this->jobsBindPassword = $jobsBindPassword;
	}

	/**
	 * Returns the database type for jobs.
	 *
	 * @return String $jobsDatabase database type
	 */
	public function getJobsDatabase() {
		if (empty($this->jobsDatabase)) {
			return 'SQLite';
		}
		return $this->jobsDatabase;
	}

	/**
	 * Sets the database type for jobs.
	 *
	 * @param String $jobsDatabase database type
	 */
	public function setJobsDatabase($jobsDatabase) {
		$this->jobsDatabase = $jobsDatabase;
	}

	/**
	 * Returns the host.
	 *
	 * @return String host
	 */
	public function getJobsDBHost() {
		return $this->jobsDBHost;
	}

	/**
	 * Sets the host.
	 *
	 * @param String $jobsDBHost host
	 */
	public function setJobsDBHost($jobsDBHost) {
		$this->jobsDBHost = $jobsDBHost;
	}

	/**
	 * Returns the port.
	 *
	 * @return String port
	 */
	public function getJobsDBPort() {
		return $this->jobsDBPort;
	}

	/**
	 * Sets the port.
	 *
	 * @param int $jobsDBPort port
	 */
	public function setJobsDBPort($jobsDBPort) {
		$this->jobsDBPort = $jobsDBPort;
	}

	/**
	 * Returns the DB user.
	 *
	 * @return String user name
	 */
	public function getJobsDBUser() {
		return $this->jobsDBUser;
	}

	/**
	 * Sets the DB user.
	 *
	 * @param String $jobsDBUser user name
	 */
	public function setJobsDBUser($jobsDBUser) {
		$this->jobsDBUser = $jobsDBUser;
	}

	/**
	 * Returns the DB password.
	 *
	 * @return String password
	 */
	public function getJobsDBPassword() {
		return $this->jobsDBPassword;
	}

	/**
	 * Sets the DB password.
	 *
	 * @param String $jobsDBPassword password
	 */
	public function setJobsDBPassword($jobsDBPassword) {
		$this->jobsDBPassword = $jobsDBPassword;
	}

	/**
	 * Returns the database name.
	 *
	 * @return String DB name
	 */
	public function getJobsDBName() {
		return $this->jobsDBName;
	}

	/**
	 * Sets the database name
	 *
	 * @param String $jobsDBName DB name
	 */
	public function setJobsDBName($jobsDBName) {
		$this->jobsDBName = $jobsDBName;
	}

	/**
	* Sets the settings for the jobs.
	*
	* @param array $settings list of job settings array(name => value)
	* @return boolean true if $settings has correct format
	*/
	public function setJobSettings($settings) {
		if (!is_array($settings)) {
			return false;
		}
		$this->jobSettings = $settings;
		return true;
	}

	/**
	* Returns a list of saved job settings.
	*
	* @return array list of settings: array(name => value)
	*/
	public function getJobSettings() {
		return $this->jobSettings;
	}

	/**
	 * Returns the job token.
	 *
	 * @return String job token
	 */
	public function getJobToken() {
		if (empty($this->jobToken)) {
			$this->jobToken = getRandomNumber();
		}
		return $this->jobToken;
	}

	/**
	 * Returns if setting a specific password is allowed on password reset page.
	 *
	 * @return String 'true' or 'false'
	 */
	public function getPwdResetAllowSpecificPassword() {
		return $this->pwdResetAllowSpecificPassword;
	}

	/**
	 * Sets if setting a specific password is allowed on password reset page.
	 *
	 * @param String $pwdResetAllowSpecificPassword 'true' or 'false'
	 */
	public function setPwdResetAllowSpecificPassword($pwdResetAllowSpecificPassword) {
		$this->pwdResetAllowSpecificPassword = $pwdResetAllowSpecificPassword;
	}

	/**
	 * Returns if displaying password on screen is allowed on password reset page.
	 *
	 * @return String 'true' or 'false'
	 */
	public function getPwdResetAllowScreenPassword() {
		return $this->pwdResetAllowScreenPassword;
	}

	/**
	 * Sets if displaying password on screen is allowed on password reset page.
	 *
	 * @param String $pwdResetAllowScreenPassword 'true' or 'false'
	 */
	public function setPwdResetAllowScreenPassword($pwdResetAllowScreenPassword) {
		$this->pwdResetAllowScreenPassword = $pwdResetAllowScreenPassword;
	}

	/**
	 * Returns if force password change is set by default on password reset page.
	 *
	 * @return String 'true' or 'false'
	 */
	public function getPwdResetForcePasswordChange() {
		return $this->pwdResetForcePasswordChange;
	}

	/**
	 * Sets if force password change is set by default on password reset page.
	 *
	 * @param String $pwdResetForcePasswordChange 'true' or 'false'
	 */
	public function setPwdResetForcePasswordChange($pwdResetForcePasswordChange) {
		$this->pwdResetForcePasswordChange = $pwdResetForcePasswordChange;
	}

	/**
	 * Returns default password output method on password reset page.
	 *
	 * @return integer LAMConfig::PWDRESET_DEFAULT_SCREEN/PWDRESET_DEFAULT_MAIL/PWDRESET_DEFAULT_BOTH
	 */
	public function getPwdResetDefaultPasswordOutput() {
		return $this->pwdResetDefaultPasswordOutput;
	}

	/**
	 * Sets default password output method on password reset page.
	 *
	 * @param integer $pwdResetDefaultPasswordOutput LAMConfig::PWDRESET_DEFAULT_SCREEN/PWDRESET_DEFAULT_MAIL/PWDRESET_DEFAULT_BOTH
	 */
	public function setPwdResetDefaultPasswordOutput($pwdResetDefaultPasswordOutput) {
		$this->pwdResetDefaultPasswordOutput = $pwdResetDefaultPasswordOutput;
	}
	/**
	 * Returns the authentication type.
	 *
	 * @return string $twoFactorAuthentication authentication type
	 */
	public function getTwoFactorAuthentication() {
		if (empty($this->twoFactorAuthentication)) {
			return TwoFactorProviderService::TWO_FACTOR_NONE;
		}
		return $this->twoFactorAuthentication;
	}

	/**
	 * Sets the authentication type.
	 *
	 * @param string $twoFactorAuthentication authentication type
	 */
	public function setTwoFactorAuthentication($twoFactorAuthentication) {
		$this->twoFactorAuthentication = $twoFactorAuthentication;
	}

	/**
	 * Returns the authentication URL.
	 *
	 * @return string $twoFactorAuthenticationURL authentication URL
	 */
	public function getTwoFactorAuthenticationURL() {
		return $this->twoFactorAuthenticationURL;
	}

	/**
	 * Sets the authentication URL.
	 *
	 * @param string $twoFactorAuthenticationURL authentication URL
	 */
	public function setTwoFactorAuthenticationURL($twoFactorAuthenticationURL) {
		$this->twoFactorAuthenticationURL = $twoFactorAuthenticationURL;
	}

	/**
	 * Returns if SSL certificate verification is turned off.
	 *
	 * @return bool $twoFactorAuthenticationInsecure SSL certificate verification is turned off
	 */
	public function getTwoFactorAuthenticationInsecure() {
		return $this->twoFactorAuthenticationInsecure;
	}

	/**
	 * Sets if SSL certificate verification is turned off.
	 *
	 * @param boolean $twoFactorAuthenticationInsecure SSL certificate verification is turned off
	 */
	public function setTwoFactorAuthenticationInsecure($twoFactorAuthenticationInsecure) {
		$this->twoFactorAuthenticationInsecure = $twoFactorAuthenticationInsecure;
	}

	/**
	 * Returns the authentication label.
	 *
	 * @return string $twoFactorAuthenticationLabel authentication label
	 */
	public function getTwoFactorAuthenticationLabel() {
		return $this->twoFactorAuthenticationLabel;
	}

	/**
	 * Sets the authentication label.
	 *
	 * @param string $twoFactorAuthenticationLabel authentication label
	 */
	public function setTwoFactorAuthenticationLabel($twoFactorAuthenticationLabel) {
		$this->twoFactorAuthenticationLabel = $twoFactorAuthenticationLabel;
	}

	/**
	 * Returns if 2nd factor is optional.
	 *
	 * @return bool $twoFactorAuthenticationOptional 2nd factor is optional
	 */
	public function getTwoFactorAuthenticationOptional() {
		return $this->twoFactorAuthenticationOptional;
	}

	/**
	 * Sets if 2nd factor is optional.
	 *
	 * @param boolean $twoFactorAuthenticationOptional 2nd factor is optional
	 */
	public function setTwoFactorAuthenticationOptional($twoFactorAuthenticationOptional) {
		$this->twoFactorAuthenticationOptional = $twoFactorAuthenticationOptional;
	}

	/**
	 * Returns the caption HTML.
	 *
	 * @return string $twoFactorAuthenticationCaption caption HTML
	 */
	public function getTwoFactorAuthenticationCaption() {
		return $this->twoFactorAuthenticationCaption;
	}

	/**
	 * Sets the caption HTML.
	 *
	 * @param string $twoFactorAuthenticationCaption caption HTML
	 */
	public function setTwoFactorAuthenticationCaption($twoFactorAuthenticationCaption) {
		$this->twoFactorAuthenticationCaption = $twoFactorAuthenticationCaption;
	}

}


/**
* This class manages config.cfg.
*
* @package configuration
*/
class LAMCfgMain {

	/** PHP error reporting setting as E_ALL & ~E_NOTICE */
	const ERROR_REPORTING_DEFAULT = 'default';
	/** PHP error reporting setting from php.ini */
	const ERROR_REPORTING_SYSTEM = 'system';
	/** PHP error reporting setting as E_ALL | E_STRICT */
	const ERROR_REPORTING_ALL = 'all';

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

	/** list of hosts which may access LAM Pro self service */
	public $allowedHostsSelfService;

	/** session encryption */
	public $encryptSession;

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

	/** number of password rules that must match (-1 = all) */
	public $checkedRulesCount = -1;

	/** password may contain the user name */
	public $passwordMustNotContainUser = 'false';

	/** password may contain more than 2 characters of user/first/last name */
	public $passwordMustNotContain3Chars = 'false';

	/** path to config file */
	private $conffile;

	/** uploaded SSL certificate that is stored to disk on save() */
	private $uploadedSSLCaCert = null;

	/** SSL certificate should be deleted on save() */
	private $delSSLCaCert = false;

	/** EOL for emails (default/unix) */
	public $mailEOL = 'default';

	/** error reporting */
	public $errorReporting = self::ERROR_REPORTING_DEFAULT;

	/** license data */
	private $license = '';

	/** list of data fields to save in config file */
	private $settings = array("password", "default", "sessionTimeout",
		"logLevel", "logDestination", "allowedHosts", "passwordMinLength",
		"passwordMinUpper", "passwordMinLower", "passwordMinNumeric",
		"passwordMinClasses", "passwordMinSymbol", 'checkedRulesCount',
		'passwordMustNotContainUser', 'passwordMustNotContain3Chars',
		"mailEOL", 'errorReporting', 'encryptSession', 'allowedHostsSelfService',
		'license'
	);

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
		$this->allowedHostsSelfService = '';
		$this->encryptSession = 'true';
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
		if (!in_array("allowedHostsSelfService", $saved)) array_push($file_array, "\n\n# list of hosts which may access LAM Pro self service\n" . "allowedHostsSelfService: " . $this->allowedHostsSelfService);
		if (!in_array("encryptSession", $saved)) array_push($file_array, "\n\n# encrypt session data\n" . "encryptSession: " . $this->encryptSession);
		if (!in_array("passwordMinLength", $saved)) array_push($file_array, "\n\n# Password: minimum password length\n" . "passwordMinLength: " . $this->passwordMinLength);
		if (!in_array("passwordMinUpper", $saved)) array_push($file_array, "\n\n# Password: minimum uppercase characters\n" . "passwordMinUpper: " . $this->passwordMinUpper);
		if (!in_array("passwordMinLower", $saved)) array_push($file_array, "\n\n# Password: minimum lowercase characters\n" . "passwordMinLower: " . $this->passwordMinLower);
		if (!in_array("passwordMinNumeric", $saved)) array_push($file_array, "\n\n# Password: minimum numeric characters\n" . "passwordMinNumeric: " . $this->passwordMinNumeric);
		if (!in_array("passwordMinSymbol", $saved)) array_push($file_array, "\n\n# Password: minimum symbolic characters\n" . "passwordMinSymbol: " . $this->passwordMinSymbol);
		if (!in_array("passwordMinClasses", $saved)) array_push($file_array, "\n\n# Password: minimum character classes (0-4)\n" . "passwordMinClasses: " . $this->passwordMinClasses);
		if (!in_array("checkedRulesCount", $saved)) array_push($file_array, "\n\n# Password: checked rules\n" . "checkedRulesCount: " . $this->checkedRulesCount);
		if (!in_array("passwordMustNotContain3Chars", $saved)) array_push($file_array, "\n\n# Password: must not contain part of user name\n" . "passwordMustNotContain3Chars: " . $this->passwordMustNotContain3Chars);
		if (!in_array("passwordMustNotContainUser", $saved)) array_push($file_array, "\n\n# Password: must not contain user name\n" . "passwordMustNotContainUser: " . $this->passwordMustNotContainUser);
		if (!in_array("mailEOL", $saved)) array_push($file_array, "\n\n# Email format (default/unix)\n" . "mailEOL: " . $this->mailEOL);
		if (!in_array("errorReporting", $saved)) array_push($file_array, "\n\n# PHP error reporting (default/system)\n" . "errorReporting: " . $this->errorReporting);
		if (!in_array("license", $saved)) array_push($file_array, "\n\n# License\n" . "license: " . $this->license);
		$file = @fopen($this->conffile, "w");
		if ($file) {
			for ($i = 0; $i < sizeof($file_array); $i++) fputs($file, $file_array[$i]);
			fclose($file);
		}
		else {
			StatusMessage("ERROR", "", _("Cannot open config file!") . " (" . $this->conffile . ")");
		}
		// store SSL certificate
		if ($this->uploadedSSLCaCert != null) {
			$sslPath = $this->getInternalSSLCaCertFileName();
			$file = @fopen($sslPath, "w");
			if ($file) {
				fputs($file, $this->uploadedSSLCaCert);
				fclose($file);
				@chmod($sslPath, 0600);
			}
			else {
				StatusMessage("ERROR", _("Cannot write certificate file. Please check the permissions of config/serverCerts.pem."));
			}
		}
		// delete SSL certificate
		if ($this->delSSLCaCert === true) {
			$sslPath = $this->getInternalSSLCaCertFileName();
			$result = @unlink($sslPath);
			if (!$result) {
				StatusMessage("ERROR", _("Cannot write certificate file. Please check the permissions of config/serverCerts.pem."));
			}
		}
	}

	/**
	 * Sets a new config password.
	 *
	 * @param String $password new password
	 */
	public function setPassword($password) {
		mt_srand((microtime() * 1000000));
		$rand = getRandomNumber();
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
		return "{SSHA}" . base64_encode(convertHex2bin(sha1($password . $salt))) . " " . base64_encode($salt);
	}

	/**
	 * Returns if the configuration file is writable.
	 *
	 * @return boolean writable
	 */
	public function isWritable() {
		return is_writeable($this->conffile);
	}

	/**
	 * Returns if the configuration file is existing.
	 *
	 * @return boolean exists
	 */
	public function isConfigFileExisting() {
		return file_exists($this->conffile);
	}

	/**
	 * Tries to copy the config file from sample config.
	 *
	 * @return boolean copied
	 */
	public function installSampleConfig() {
		$samplePath = dirname(dirname(__FILE__)) . '/config/config.cfg.sample';
		return file_exists($samplePath) && copy($samplePath, $this->conffile);
	}

	/**
	 * Returns the path to the SSL CA certificate file that overrides the system certificates.
	 *
	 * @return String path to certificate file or null if certificate is not overridden
	 */
	public function getSSLCaCertPath() {
		$path = $this->getInternalSSLCaCertFileName();
		if (file_exists($path)) {
			return $path;
		}
		return null;
	}

	/**
	 * Returns the file name that will be used internally to store the CA file.
	 *
	 * @return String file name
	 */
	private function getInternalSSLCaCertFileName() {
		return dirname(__FILE__) . '/../config/serverCerts.pem';
	}

	/**
	 * Uploads a new SSL CA cert.
	 *
	 * @param String $cert file content in DER/PEM format
	 * @return mixed TRUE if format is correct, error message if file is not accepted
	 */
	public function uploadSSLCaCert($cert) {
		if (strpos($cert, '-----BEGIN CERTIFICATE-----') === false) {
			$pem = @chunk_split(@base64_encode($cert), 64, "\n");
			$cert = "-----BEGIN CERTIFICATE-----\n" . $pem . "-----END CERTIFICATE-----\n";
		}
		else {
			// remove any junk before first "-----BEGIN CERTIFICATE-----"
			$pos = strpos($cert, '-----BEGIN CERTIFICATE-----');
			$cert = substr($cert, $pos);
		}
		$pemData = @openssl_x509_parse($cert);
		if ($pemData === false) {
			return _('Please provide a file in DER or PEM format.');
		}
		$existingCerts = $this->getSSLCaCertificateContent();
		if (!empty($existingCerts)) {
			// merge with existing certificates
			$existingList = $this->splitSSLCaCertificateContent($existingCerts);
			$newList = $this->splitSSLCaCertificateContent($cert);
			$this->uploadedSSLCaCert = implode("\n", array_unique(array_merge($existingList, $newList)));
		}
		else {
			$this->uploadedSSLCaCert = $cert;
		}
		$this->delSSLCaCert = false;
		return true;
	}

	/**
	 * Returns the name of a temporary file in tmp that contains the SSL certificate.
	 * The file contains either the stored data in serverCerts or the uploaded data.
	 *
	 * @return String file name or null if no certificate was set
	 */
	public function getSSLCaCertTempFileName() {
		if ($this->delSSLCaCert) {
			return null;
		}
		// get certificate data
		$content = $this->getSSLCaCertificateContent();
		if ($content == null) {
			return null;
		}
		// write to temp file
		$fileName = time() . getRandomNumber() . '.pem';
		$path = dirname(__FILE__) . '/../tmp/' . $fileName;
		$handle = @fopen($path, "wb");
		@chmod($path, 0600);
		if ($handle) {
			$content = fputs($handle, $content);
			fclose($handle);
		}
		else {
			return null;
		}
		return $fileName;
	}

	/**
	 * Marks a single or all SSL CA certificate files for deletion.
	 * The changes take effect on save().
	 *
	 * @param int $index certificate index, null deletes all certificates (default: null)
	 */
	public function deleteSSLCaCert($index = null) {
		if ($index == null) {
			// delete all
			$this->delSSLCaCert = true;
			return;
		}
		$content = $this->getSSLCaCertificateContent();
		$list = $this->splitSSLCaCertificateContent($content);
		unset($list[$index]);
		if (sizeof($list) < 1) {
			$this->delSSLCaCert = true;
			$this->uploadedSSLCaCert = null;
		}
		else {
			$this->uploadedSSLCaCert = implode("\n", $list);
		}
	}

	/**
	 * Returns a list of all CA certificates.
	 *
	 * @return array list of certificates as output of openssl_x509_parse()
	 */
	public function getSSLCaCertificates() {
		if ($this->delSSLCaCert) {
			return array();
		}
		$content = $this->getSSLCaCertificateContent();
		if (empty($content)) {
			return array();
		}
		$list = $this->splitSSLCaCertificateContent($content);
		for ($i = 0; $i < sizeof($list); $i++) {
			$list[$i] = @openssl_x509_parse($list[$i]);
		}
		return $list;
	}

	/**
	 * Returns the content of the certificate file or uploaded data.
	 *
	 * @return String null or certificate content
	 */
	private function getSSLCaCertificateContent() {
		$content = null;
		if ($this->delSSLCaCert) {
			return null;
		}
		if ($this->uploadedSSLCaCert != null) {
			$content = $this->uploadedSSLCaCert;
		}
		elseif ($this->getSSLCaCertPath() != null) {
			$path = $this->getSSLCaCertPath();
			$handle = @fopen($path, "r");
			if ($handle) {
				$content = fread($handle, 10000000);
				fclose($handle);
			}
		}
		return $content;
	}

	/**
	 * Splits the certificate content into single PEM data chunks.
	 *
	 * @param String $content PEM file content
	 * @return array one element for each certificate chunk
	 */
	private function splitSSLCaCertificateContent($content) {
		if (empty($content)) {
			return array();
		}
		$content = str_replace("\n\n", "\n", $content);
		if (empty($content)) {
			return array();
		}
		if (!(strpos($content, '-----BEGIN CERTIFICATE-----') === 0)) {
			return array();
		}
		$lines = explode("\n", $content);
		$list = array();
		$pos = -1;
		foreach ($lines as $line) {
			if (strpos($line, '-----BEGIN CERTIFICATE-----') === 0) {
				$pos++;
			}
			if (!isset($list[$pos])) {
				$list[$pos] = '';
			}
			$list[$pos] .= $line . "\n";
		}
		return $list;
	}

	/**
	 * Returns the license key as multiple lines.
	 *
	 * @return String license
	 */
	public function getLicenseLines() {
		return explode(LAMConfig::LINE_SEPARATOR, $this->license);
	}

	/**
	 * Sets the license key as multiple lines.
	 *
	 * @param String $license license
	 */
	public function setLicenseLines($licenseLines) {
		$this->license = implode(LAMConfig::LINE_SEPARATOR, $licenseLines);
	}

}

?>
