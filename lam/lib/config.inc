<?php
use \LAM\LIB\TWO_FACTOR\TwoFactorProviderService;
use LAM\PDF\PdfStructurePersistenceManager;
use LAM\PERSISTENCE\ConfigurationDatabase;
use LAM\PROFILES\AccountProfilePersistenceManager;
use \LAM\REMOTE\RemoteServerConfiguration;
use LAM\TYPES\TypeManager;
use function LAM\PERSISTENCE\dbTableExists;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2022  Roland Gruber

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

/** persistence */
include_once 'persistence.inc';
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
	$config = null;
	if (isset($_SESSION['cfgMain'])) {
		$config = $_SESSION['cfgMain'];
	}
	else {
		$config = new LAMCfgMain();
	}
	// set SSL certificate if set
	$sslCaPath = $config->getSSLCaCertPath();
	if ($sslCaPath != null) {
		putenv('LDAPTLS_CACERT=' . $sslCaPath);
		putenv('TLS_CACERT=' . $sslCaPath);
	}
}

/**
* Sets language settings for automatic translation
*/
function setlanguage() {
	$code = 'en_GB.utf8';
	$fallbackCode = 'en_GB.UTF-8';
	$encoding = 'UTF-8';
	if (!isset($_SESSION['language'])) {
		$_SESSION['language'] = "en_GB.utf8";
	}
	$possibleLanguages = getLanguages();
	foreach ($possibleLanguages as $lang) {
		if ($lang->code == $_SESSION['language']) {
			$code = $lang->code;
			$fallbackCode = str_replace('utf8', 'UTF-8', $code);
			$encoding = $lang->encoding;
			break;
		}
	}
	putenv("LANG=$code");  // e.g. LANG=de_DE
	putenv("LC_ALL=$code");
	putenv("LC_LANG=$code");
	putenv("LC_LANGUAGE=$code");
	$setLocaleResult = setlocale(LC_ALL, array($code, $fallbackCode));  // set LC_ALL
	if ($setLocaleResult === false) {
		logNewMessage(LOG_WARNING, "Unable to set locale, check if 'locale -a' returns $code");
	}
	$locdir = substr(__FILE__, 0, strlen(__FILE__) - 15) . "/locale";  // set path to translations
	bindtextdomain("messages", $locdir);
	$bindTextResult = bind_textdomain_codeset("messages", $encoding);
	if (!is_string($bindTextResult)) {
		logNewMessage(LOG_WARNING, "Unable to bind text domain, check if 'locale -a' returns $code");
	}
	$textDomainResult = textdomain("messages");
	if (!is_string($textDomainResult)) {
		logNewMessage(LOG_WARNING, "Unable to set text domain, check if 'locale -a' returns $code");
	}
	header("Content-type: text/html; charset=" . $encoding, true);
}

/**
 * Checks whether a specific flag in the rights string is set.
 *
 * @param string $right read, write or execute
 * @param string $target owner, group or other
 * @param string $chmod the chmod rights
 *
 * @return true, if the chmod $right for $target were set
 */
function checkChmod($right, $target, $chmod) {
	$right_arr = array("read","write","execute");
	$target_arr = array("owner","group","other");

	// Check, if $right and $target has right parameters
	if (!in_array($right, $right_arr) ||!in_array($target, $target_arr)) {
		return false;
	}

	$chmod_num = -1;
	// owner:
	if ($target == "owner") {
		$chmod_num = 0;
	}
	if ($target == "group") {
		$chmod_num = 1;
	}
	if ($target == "other") {
		$chmod_num = 2;
	}

	// Cut the number from the chmod:
	$chmod_num = $chmod[$chmod_num];

	// Now check, if the chmod_num can be right with the $right
	// What numbers allow "read"
	$read = array(4,5,6,7);
	// What numbers allow "write"
	$write = array(2,3,6,7);
	// What numbers allow "execute"
	$execute = array(1,3,5,7);
	if ((($right == "read") && in_array($chmod_num, $read))
			|| (($right == "write") && in_array($chmod_num, $write))
			|| (($right == "execute") && in_array($chmod_num, $execute))) {
		return true;
	}
	return false;
}

/**
* Returns the version number of this LAM installation.
* Format: <major version>.<minor version>.<patch level>
* <br> Major/minor version are always numbers, patch level may contain letters for unofficial releases only (e.g. 0.5.alpha1).
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
			if (isset($_POST[$element]) && ($_POST[$element] == "on")) {
				$options[$element] = array('true');
			}
			else {
				$options[$element] = array('false');
			}
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
	if (!headers_sent()) {
		header('Location: ' . $page);
		return;
	}
	if (isset($_SESSION['header'])) {
		echo $_SESSION['header'];
	}
	else {
		echo '<!DOCTYPE html>';
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
	return isset($typeSettings['hidden_' . $type]) && ($typeSettings['hidden_' . $type]);
}

/**
 * Returns a list of all supported languages.
 *
 * @return LAMLanguage[] languages
 */
function getLanguages() {
	$languages = array();
	// loading available languages from language.conf file
	$languagefile = dirname(__FILE__) . "/../config/language";
	if(is_file($languagefile)) {
		$file = fopen($languagefile, "r");
		while(!feof($file)) {
			$line = fgets($file, 1024);
			if($line == "" || $line == "\n" || $line[0] == "#") {
				continue; // ignore comment and empty lines
			}
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
 * Manages the persistence of server profiles.
 */
class ServerProfilePersistenceManager {

	/**
	 * @var ServerProfilePersistenceStrategy
	 */
	private $strategy;

	public function __construct() {
		$configDb = new ConfigurationDatabase(new LAMCfgMain());
		if ($configDb->useRemoteDb()) {
			$this->strategy = new ServerProfilePersistenceStrategyPdo($configDb->getPdo());
		}
		else {
			$this->strategy = new ServerProfilePersistenceStrategyFiles();
		}
	}

	/**
	 * Returns a lis of available server profiles.
	 *
	 * @return string[] list of server profile names
	 * @throws LAMException error listing profiles
	 */
	public function getProfiles() : array {
		return $this->strategy->getProfiles();
	}

	/**
	 * Loads the given server profile
	 *
	 * @param string $name profile name
	 * @return LAMConfig profile
	 * @throws LAMException error loading profile
	 */
	public function loadProfile(string $name) : LAMConfig {
		$config = $this->strategy->loadProfile($name);
		$config->setName($name);
		return $config;
	}

	/**
	 * Saves a server profile.
	 *
	 * @param LAMConfig $profile profile
	 * @param $name profile name
	 * @throws LAMException error saving profile
	 */
	public function saveProfile(LAMConfig $profile, $name) : void {
		$this->strategy->saveProfile($profile, $name);
	}

	/**
	 * Returns an array of string with all available configuration templates (without .sample.conf)
	 *
	 * @return array template names
	 */
	function getConfigTemplates() : array {
		$dir = dir(__DIR__ . "/../config");
		$ret = array();
		$pos = 0;
		while ($entry = $dir->read()){
			$ext = substr($entry, strlen($entry)-12, 12);
			$name = substr($entry, 0, strlen($entry) - 12);
			// check if extension is right, add to profile list
			if ($ext == ".sample.conf") {
				$ret[$pos] = $name;
				$pos ++;
			}
		}
		sort($ret);
		return $ret;
	}

	/**
	 * Creates a new server profile using a template/existing profile.
	 *
	 * @param string $name profile name
	 * @param string $templateName template name
	 * @param string $password new profile password
	 * @throws LAMException error saving profile
	 */
	public function createProfileFromTemplate(string $name, string $templateName, string $password) : void {
		if (!LAMConfig::isValidName($name) || !preg_match("/^[a-z0-9\\._-]+$/i", $templateName) || in_array($name, $this->getProfiles())) {
			throw new LAMException(_("Profile name is invalid!"));
		}
		$copyFromTemplate = true;
		$existingTemplateNames = $this->getConfigTemplates();
		if (strpos($templateName, '.sample') !== false) {
			$templateNameShort = str_replace('.sample', '', $templateName);
			if (!in_array($templateNameShort, $existingTemplateNames)) {
				throw new LAMException(_("Profile name is invalid!"));
			}
			// built-in templates are loaded via file system
			$filesStrategy = new ServerProfilePersistenceStrategyFiles();
			$config = $filesStrategy->loadProfile($templateName);
		}
		else {
			$copyFromTemplate = false;
			$config = $this->loadProfile($templateName);
		}
		$config->set_Passwd($password);
		$this->saveProfile($config, $name);
		$accountProfilePersistenceManager = new AccountProfilePersistenceManager();
		$pdfStructurePersistenceManager = new PdfStructurePersistenceManager();
		if (!$copyFromTemplate) {
			$typeManager = new TypeManager($config);
			foreach ($typeManager->getConfiguredTypes() as $type) {
				$profilesToCopy = $accountProfilePersistenceManager->getAccountProfileNames($type->getId(), $templateName);
				foreach ($profilesToCopy as $profileToCopy) {
					$profile = $accountProfilePersistenceManager->loadAccountProfile($type->getId(), $profileToCopy, $templateName);
					$accountProfilePersistenceManager->writeAccountProfile($type->getId(), $profileToCopy, $name, $profile);
				}
				$structuresToCopy = $pdfStructurePersistenceManager->getPDFStructures($templateName, $type->getId());
				foreach ($structuresToCopy as $structureToCopy) {
					$structure = $pdfStructurePersistenceManager->readPdfStructure($templateName, $type->getId(), $structureToCopy);
					$pdfStructurePersistenceManager->savePdfStructure($name, $type->getId(), $structureToCopy, $structure);
				}
				$logosToCopy = $pdfStructurePersistenceManager->getPdfLogos($templateName);
				foreach ($logosToCopy as $logoToCopy) {
					$binary = $pdfStructurePersistenceManager->getPdfLogoBinary($templateName, $logoToCopy->getName());
					$pdfStructurePersistenceManager->savePdfLogo($name, $logoToCopy->getName(), $binary);
				}
			}
		}
		$accountProfilePersistenceManager->installAccountProfileTemplates($name);
		$pdfStructurePersistenceManager->installPDFTemplates($name);
	}

	/**
	 * Renames an existing server profile.
	 *
	 * @param string $oldName old profile name
	 * @param string $newName new profile name
	 * @throws LAMException error during rename
	 */
	public function renameProfile(string $oldName, string $newName) : void {
		$this->strategy->renameProfile($oldName, $newName);
		// update default profile setting if needed
		$mainConfig = new LAMCfgMain();
		if ($mainConfig->default == $oldName) {
			$mainConfig->default = $newName;
			$mainConfig->save();
		}
	}

	/**
	 * Deletes a server profile.
	 *
	 * @param string $name profile name
	 * @throws LAMException error deleting profile
	 */
	public function deleteProfile(string $name) : void {
		$this->strategy->deleteProfile($name);
	}

	/**
	 * Returns if the configuration is writable.
	 *
	 * @param string $name profile name
	 * @return bool is writable
	 */
	public function isWritable(string $name) : bool {
		return $this->strategy->isWritable($name);
	}

}

/**
 * Interface to store server profiles.
 */
interface ServerProfilePersistenceStrategy {

	/**
	 * Returns a lis of available server profiles.
	 *
	 * @return string[] list of server profile names
	 * @throws LAMException error listing profiles
	 */
	public function getProfiles() : array;

	/**
	 * Loads the given server profile
	 *
	 * @param string $name profile name
	 * @return LAMConfig profile
	 * @throws LAMException error loading profile
	 */
	public function loadProfile(string $name) : LAMConfig;

	/**
	 * Saves a server profile.
	 *
	 * @param LAMConfig $profile profile
	 * @param string $name profile name
	 * @throws LAMException error saving profile
	 */
	public function saveProfile(LAMConfig $profile, string $name) : void;

	/**
	 * Renames an existing server profile.
	 *
	 * @param string $oldName old profile name
	 * @param string $newName new profile name
	 * @throws LAMException error saving profile
	 */
	public function renameProfile(string $oldName, string $newName) : void;

	/**
	 * Deletes a server profile.
	 *
	 * @param string $name profile name
	 * @throws LAMException error deleting profile
	 */
	public function deleteProfile(string $name) : void;

	/**
	 * Returns if the configuration is writable.
	 *
	 * @param string $name profile name
	 * @return bool is writable
	 */
	public function isWritable(string $name) : bool;

}

/**
 * Uses local file system to store server profiles.
 */
class ServerProfilePersistenceStrategyFiles implements ServerProfilePersistenceStrategy {

	/**
	 * @inheritDoc
	 */
	public function getProfiles(): array {
		$dirName = __DIR__ . "/../config";
		$dir = dir($dirName);
		$ret = array();
		while ($entry = $dir->read()) {
			$ext = substr($entry, strlen($entry) - 5, 5);
			$name = substr($entry, 0, strlen($entry) - 5);
			// check if extension is right, add to profile list
			if (($ext == ".conf") && (strpos($name, '.sample') === false) && is_readable($dirName . '/' . $entry)) {
				$ret[] = $name;
			}
		}
		sort($ret);
		return $ret;
	}

	/**
	 * @inheritDoc
	 */
	public function loadProfile(string $name): LAMConfig {
		$config = new LAMConfig();
		$reflectionConfig = new ReflectionObject($config);
		$confFilePath = $this->getPath($name);
		if (!is_file($confFilePath)) {
			throw new LAMException(_('Unable to read file.'));
		}
		$file = @fopen($confFilePath, "r");
		if (!$file) {
			throw new LAMException(_('Unable to read file.'));
		}
		$moduleSettings = array();
		$typeSettings = array();
		$toolSettings = array();
		$jobSettings = array();
		while (!feof($file)) {
			$line = fgets($file, 1000000);
			$line = trim($line);  // remove spaces at the beginning and end
			if (($line == "")||($line[0] == "#")) {
				continue; // ignore comments and empty lines
			}
			// search keywords
			$parts = explode(': ', $line);
			$keyword = $parts[0];
			$startIndex = strlen($keyword) + 2;
			if (sizeof($parts) == 1) {
				// empty global settings
				try {
					$property = $reflectionConfig->getProperty($keyword);
					$property->setAccessible(true);
					$property->setValue($config, '');
				} catch (ReflectionException $e) {
					// ignore
				}
			}
			elseif ((sizeof($parts) > 1) && !in_array($keyword, array('modules', 'types', 'tools', 'jobs'))) {
				// global setting with value
				try {
					$property = $reflectionConfig->getProperty($keyword);
					$property->setAccessible(true);
					$property->setValue($config, substr($line, $startIndex));
				} catch (ReflectionException $e) {
					// ignore
				}
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
					$moduleSettings[$subKeyword] = explode(LAMConfig::LINE_SEPARATOR, $option);
				}
				// type settings
				if ($keyword == 'types') {
					$typeSettings[$subKeyword] = $option;
				}
				// tool settings
				if ($keyword == 'tools') {
					$toolSettings[$subKeyword] = $option;
				}
				// job settings
				if ($keyword == 'jobs') {
					$jobSettings[$subKeyword] = explode(LAMConfig::LINE_SEPARATOR, $option);
				}
			}
		}
		fclose($file);
		$config->set_moduleSettings($moduleSettings);
		$config->set_typeSettings($typeSettings);
		$config->setToolSettings($toolSettings);
		$config->setJobSettings($jobSettings);
		$config->removeInvalidTypes();
		$config->removeInvalidModules();
		return $config;
	}

	/**
	 * @inheritDoc
	 */
	public function saveProfile(LAMConfig $profile, string $name): void {
		$confFile = $this->getPath($name);
		if (!file_exists($confFile)) {
			$newFile = fopen($confFile, 'wb');
			fclose($newFile);
		}
		if (is_file($confFile) && is_readable($confFile)) {
			$file_array = array();
			$file_array[] = "\n" . "ServerURL: " . $profile->get_ServerURL() . "\n";
			$file_array[] = "\n" . "serverDisplayName: " . $profile->getServerDisplayName() . "\n";
			$file_array[] = "\n" . "useTLS: " . $profile->getUseTLS() . "\n";
			$file_array[] = "\n" . "followReferrals: " . $profile->getFollowReferrals() . "\n";
			$file_array[] = "\n" . "pagedResults: " . $profile->getPagedResults() . "\n";
			$file_array[] = "\n" . "referentialIntegrityOverlay: " . $profile->getReferentialIntegrityOverlay() . "\n";
			$file_array[] = "\n" . "hidePasswordPromptForExpiredPasswords: " . $profile->getHidePasswordPromptForExpiredPasswords() . "\n";
			$file_array[] = "\n" . "Passwd: " . $profile->getPasswd() . "\n";
			$file_array[] = "\n" . "Admins: " . $profile->get_Adminstring() . "\n";
			$file_array[] = "\n" . "defaultLanguage: " . $profile->get_defaultLanguage() . "\n";
			$file_array[] = "\n" . "timeZone: " . $profile->getTimeZone() . "\n";
			$file_array[] = "\n" . "scriptPath: " . $profile->get_scriptPath() . "\n";
			$file_array[] = "\n" . "scriptServer: " . $profile->get_scriptServers() . "\n";
			$file_array[] = "\n" . "scriptRights: " . $profile->get_scriptRights() . "\n";
			$file_array[] = "\n" . "scriptUserName: " . $profile->getScriptUserName() . "\n";
			$file_array[] = "\n" . "scriptSSHKey: " . $profile->getScriptSSHKey() . "\n";
			$file_array[] = "\n" . "scriptSSHKeyPassword: " . $profile->getScriptSSHKeyPassword() . "\n";
			$file_array[] = "\n" . "searchLimit: " . $profile->get_searchLimit() . "\n";
			$file_array[] = "\n" . "activeTypes: " . $profile->getActiveTypesString() . "\n";
			$file_array[] = "\n" . "accessLevel: " . $profile->getAccessLevel() . "\n";
			$file_array[] = "\n" . "loginMethod: " . $profile->getLoginMethod() . "\n";
			$file_array[] = "\n" . "loginSearchSuffix: " . $profile->getLoginSearchSuffix() . "\n";
			$file_array[] = "\n" . "loginSearchFilter: " . $profile->getLoginSearchFilter() . "\n";
			$file_array[] = "\n" . "loginSearchDN: " . $profile->getLoginSearchDN() . "\n";
			$file_array[] = "\n" . "loginSearchPassword: " . $profile->getLoginSearchPassword() . "\n";
			$file_array[] = "\n" . "httpAuthentication: " . $profile->getHttpAuthentication() . "\n";
			$file_array[] = "\n" . "lamProMailFrom: " . $profile->getLamProMailFrom() . "\n";
			$file_array[] = "\n" . "lamProMailReplyTo: " . $profile->getLamProMailReplyTo() . "\n";
			$file_array[] = "\n" . "lamProMailSubject: " . $profile->getLamProMailSubject() . "\n";
			$file_array[] = "\n" . "lamProMailIsHTML: " . $profile->getLamProMailIsHTML() . "\n";
			$file_array[] = "\n" . "lamProMailAllowAlternateAddress: " . $profile->getLamProMailAllowAlternateAddress() . "\n";
			$file_array[] = "\n" . "lamProMailText: " . $profile->getLamProMailTextRaw() . "\n";
			$file_array[] = "\n" . "jobsBindPassword: " . $profile->getJobsBindPassword() . "\n";
			$file_array[] = "\n" . "jobsBindUser: " . $profile->getJobsBindUser() . "\n";
			$file_array[] = "\n" . "jobsDatabase: " . $profile->getJobsDatabase() . "\n";
			$file_array[] = "\n" . "jobsDBHost: " . $profile->getJobsDBHost() . "\n";
			$file_array[] = "\n" . "jobsDBPort: " . $profile->getJobsDBPort() . "\n";
			$file_array[] = "\n" . "jobsDBUser: " . $profile->getJobsDBUser() . "\n";
			$file_array[] = "\n" . "jobsDBPassword: " . $profile->getJobsDBPassword() . "\n";
			$file_array[] = "\n" . "jobsDBName: " . $profile->getJobsDBName() . "\n";
			$file_array[] = "\n" . "jobToken: " . $profile->getJobToken() . "\n";
			$file_array[] = "\n" . "pwdResetAllowSpecificPassword: " . $profile->getPwdResetAllowSpecificPassword() . "\n";
			$file_array[] = "\n" . "pwdResetAllowScreenPassword: " . $profile->getPwdResetAllowScreenPassword() . "\n";
			$file_array[] = "\n" . "pwdResetForcePasswordChange: " . $profile->getPwdResetForcePasswordChange() . "\n";
			$file_array[] = "\n" . "pwdResetDefaultPasswordOutput: " . $profile->getPwdResetDefaultPasswordOutput() . "\n";
			$file_array[] = "\n" . "twoFactorAuthentication: " . $profile->getTwoFactorAuthentication() . "\n";
			$file_array[] = "\n" . "twoFactorAuthenticationURL: " . $profile->getTwoFactorAuthenticationURLRaw() . "\n";
			$file_array[] = "\n" . "twoFactorAuthenticationClientId: " . $profile->getTwoFactorAuthenticationClientId() . "\n";
			$file_array[] = "\n" . "twoFactorAuthenticationSecretKey: " . $profile->getTwoFactorAuthenticationSecretKey() . "\n";
			$file_array[] = "\n" . "twoFactorAuthenticationDomain: " . $profile->getTwoFactorAuthenticationDomain() . "\n";
			$file_array[] = "\n" . "twoFactorAuthenticationInsecure: " . $profile->getTwoFactorAuthenticationInsecure() . "\n";
			$file_array[] = "\n" . "twoFactorAuthenticationLabel: " . $profile->getTwoFactorAuthenticationLabel() . "\n";
			$file_array[] = "\n" . "twoFactorAuthenticationOptional: " . $profile->getTwoFactorAuthenticationOptional() . "\n";
			$file_array[] = "\n" . "twoFactorAuthenticationCaption: " . $profile->getTwoFactorAuthenticationCaption() . "\n";
			$file_array[] = "\n" . "twoFactorAuthenticationAttribute: " . $profile->getTwoFactorAuthenticationAttribute() . "\n";
			$file_array[] = "\n" . "hideDnPart: " . $profile->getHideDnPart() . "\n";
			$file_array[] = "\n" . "pwdPolicyMinLength: " . $profile->getPwdPolicyMinLength() . "\n";
			$file_array[] = "\n" . "pwdPolicyMinLowercase: " . $profile->getPwdPolicyMinLowercase() . "\n";
			$file_array[] = "\n" . "pwdPolicyMinUppercase: " . $profile->getPwdPolicyMinUppercase() . "\n";
			$file_array[] = "\n" . "pwdPolicyMinNumeric: " . $profile->getPwdPolicyMinNumeric() . "\n";
			$file_array[] = "\n" . "pwdPolicyMinSymbolic: " . $profile->getPwdPolicyMinSymbolic() . "\n";
			// module settings
			$moduleSettings = $profile->get_moduleSettings();
			foreach ($moduleSettings as $moduleSettingKey => $moduleSettingValue) {
				$file_array[] = "modules: " . $moduleSettingKey . ": " . implode(LAMConfig::LINE_SEPARATOR, $moduleSettingValue) . "\n";
			}
			// type settings
			$typeSettings = $profile->get_typeSettings();
			foreach ($typeSettings as $typeSettingKey => $typeSettingValue) {
				$file_array[] = "types: " . $typeSettingKey . ": " . $typeSettingValue . "\n";
			}
			// tool settings
			$toolSettings = $profile->getToolSettings();
			foreach ($toolSettings as $toolSettingKey => $toolSettingValue) {
				$file_array[] = "tools: " . $toolSettingKey . ": " . $toolSettingValue . "\n";
			}
			// job settings
			$jobSettings = $profile->getJobSettings();
			foreach ($jobSettings as $jobSettingKey => $jobSettingValue) {
				$file_array[] = "jobs: " . $jobSettingKey . ": " . implode(LAMConfig::LINE_SEPARATOR, $jobSettingValue) . "\n";
			}
			$file = @fopen($confFile, "w");
			if ($file) {
				foreach ($file_array as $line) {
					fputs($file, $line);
				}
				fclose($file);
				@chmod($confFile, 0600);
			}
			else {
				throw new LAMException(_('Unable to save server profile.'), $name);
			}
		}
	}

	/**
	 * @inheritDoc
	 */
	public function renameProfile(string $oldName, string $newName): void {
		$existingProfileNames = $this->getProfiles();
		if (!LAMConfig::isValidName($oldName)
			|| !LAMConfig::isValidName($newName)
			|| in_array($newName, $existingProfileNames)) {
			throw new LAMException(_("Profile name is invalid!"));
		}
		if (rename("../../config/" . $oldName . ".conf", "../../config/" . $newName . ".conf")) {
			// rename pdf and profiles folder
			rename("../../config/profiles/" . $oldName, "../../config/profiles/" . $newName);
			rename("../../config/pdf/" . $oldName, "../../config/pdf/" . $newName);
			// rename sqlite database if any
			if (file_exists("../../config/" . $oldName . ".sqlite")) {
				rename("../../config/" . $oldName . ".sqlite", "../../config/" . $newName . ".sqlite");
			}
		}
		else {
			throw new LAMException(_("Could not rename file!"));
		}
	}

	/**
	 * @inheritDoc
	 */
	public function deleteProfile(string $name): void {
		if (!LAMConfig::isValidName($name)) {
			throw new LAMException(_("Unable to delete profile!"));
		}
		$dir = __DIR__ . "/../config/";
		// delete account profiles and PDF structures
		$subDirs = array($dir . 'pdf/' . $name . '/logos', $dir . 'pdf/' . $name, $dir . 'profiles/' . $name);
		for ($i = 0; $i < sizeof($subDirs); $i++) {
			if (is_dir($subDirs[$i]) && is_readable($subDirs[$i])) {
				$dirHandle = @opendir($subDirs[$i]);
				while (false !== ($path = readdir($dirHandle))) {
					if (($path != '.')  && ($path != '..') && !@unlink($subDirs[$i] . '/' . $path)) {
						logNewMessage(LOG_ERR, 'Unable to delete ' . $subDirs[$i] . '/' . $path);
						throw new LAMException(_("Unable to delete profile!"));
					}
				}
				@closedir($dirHandle);
				if (!@rmdir($subDirs[$i])) {
					logNewMessage(LOG_ERR, 'Unable to delete ' . $subDirs[$i]);
					throw new LAMException(_("Unable to delete profile!"));
				}
			}
		}
		// delete config file
		$confFile = $dir . $name . ".conf";
		if (!@unlink($confFile)) {
			logNewMessage(LOG_ERR, 'Unable to delete ' . $confFile);
			throw new LAMException(_("Unable to delete profile!"));
		}
	}

	/**
	 * @inheritDoc
	 */
	public function isWritable(string $name): bool {
		return is_writeable($this->getPath($name));
	}

	/**
	 * Returns the path to the config file.
	 *
	 * @param string $name profile name
	 * @return string path on filesystem
	 */
	private function getPath(string $name) : string {
		return __DIR__ . "/../config/" . $name . ".conf";
	}

}

/**
 * Stores server profiles in a database.
 */
class ServerProfilePersistenceStrategyPdo implements ServerProfilePersistenceStrategy {

	const TABLE_NAME = 'server_profiles';

	/**
	 * @var PDO
	 */
	private $pdo;

	/**
	 * Constructor
	 * 
	 * @param PDO $pdo PDO
	 */
	public function __construct(PDO $pdo) {
		$this->pdo = $pdo;
		$this->checkSchema();
	}

	/**
	 * Checks if the schema has latest version.
	 */
	private function checkSchema() : void {
		if (!dbTableExists($this->pdo, self::TABLE_NAME)) {
			$this->createInitialSchema();
		}
	}

	/**
	 * Creates the initial schema.
	 */
	public function createInitialSchema() : void {
		logNewMessage(LOG_DEBUG, 'Creating database table ' . self::TABLE_NAME);
		$sql = 'create table ' . self::TABLE_NAME . '('
			. 'name VARCHAR(300) NOT NULL,'
			. 'data TEXT NOT NULL,'
			. 'PRIMARY KEY(name)'
			. ');';
		$this->pdo->exec($sql);
		$sql = 'insert into ' . ConfigurationDatabase::TABLE_SCHEMA_VERSIONS . ' (name, version) VALUES ("server_profiles", 1);';
		$this->pdo->exec($sql);
	}

	/**
	 * @inheritDoc
	 */
	public function getProfiles(): array {
		$statement = $this->pdo->prepare("SELECT name FROM " . self::TABLE_NAME);
		$statement->execute();
		$results = $statement->fetchAll();
		$profiles = array();
		foreach ($results as $result) {
			$profiles[] = $result['name'];
		}
		return $profiles;
	}

	/**
	 * @inheritDoc
	 */
	public function loadProfile(string $name): LAMConfig {
		$statement = $this->pdo->prepare("SELECT data FROM " . self::TABLE_NAME . " WHERE name = ?");
		$statement->execute(array($name));
		$results = $statement->fetchAll();
		if (empty($results)) {
			logNewMessage(LOG_ERR, 'Server profile not found: ' . $name);
			throw new LAMException(_("Unable to load profile!"), $name);
		}
		$data = json_decode($results[0]['data'], true);
		$config = new LAMConfig();
		$config->importData($data);
		return $config;
	}

	/**
	 * @inheritDoc
	 */
	public function saveProfile(LAMConfig $profile, string $name): void {
		$data = json_encode($profile->exportData());
		$statement = $this->pdo->prepare("SELECT data FROM " . self::TABLE_NAME . " WHERE name = ?");
		$statement->execute(array($name));
		$results = $statement->fetchAll();
		if (empty($results)) {
			$statement = $this->pdo->prepare("INSERT INTO " . self::TABLE_NAME . " (name, data) VALUES (?, ?)");
			$statement->execute(array($name, $data));
		}
		else {
			$statement = $this->pdo->prepare("UPDATE " . self::TABLE_NAME . " SET data = ? WHERE name = ?");
			$statement->execute(array($data, $name));
		}
	}

	/**
	 * @inheritDoc
	 */
	public function renameProfile(string $oldName, string $newName): void {
		$statement = $this->pdo->prepare("UPDATE " . self::TABLE_NAME . " SET name = ? WHERE name = ?");
		$statement->execute(array($newName, $oldName));
	}

	/**
	 * @inheritDoc
	 */
	public function deleteProfile(string $name): void {
		$statement = $this->pdo->prepare("DELETE FROM " . self::TABLE_NAME . " WHERE name = ?");
		$statement->execute(array($name));
	}

	/**
	 * @inheritDoc
	 */
	public function isWritable(string $name): bool {
		return true;
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

	/** overlay for referential integrity is activated */
	private $referentialIntegrityOverlay = 'false';

	/** hide password prompt for expired passwords */
	private $hidePasswordPromptForExpiredPasswords = 'false';

	/** Array of string: users with admin rights */
	private $Admins;

	/** Password to edit preferences */
	private $Passwd;

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

	/** LDAP search limit */
	private $searchLimit = 0;

	/** Active account types */
	private $activeTypes = "";

	/** Name of configuration file */
	private $name;

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
	private $twoFactorAuthenticationClientId = null;
	private $twoFactorAuthenticationSecretKey = null;
	private $twoFactorAuthenticationDomain = null;
	private $twoFactorAuthenticationInsecure = false;
	private $twoFactorAuthenticationLabel = null;
	private $twoFactorAuthenticationOptional = false;
	private $twoFactorAuthenticationCaption = '';
	private $twoFactorAuthenticationAttribute = '';

	private $hideDnPart = '';

	private $pwdPolicyMinLength = '';
	private $pwdPolicyMinLowercase = '';
	private $pwdPolicyMinUppercase = '';
	private $pwdPolicyMinNumeric = '';
	private $pwdPolicyMinSymbolic = '';

	/** List of all settings in config file */
	private $settings = array("ServerURL", "useTLS", "followReferrals", 'pagedResults', "Passwd", "Admins",
		"defaultLanguage", "scriptPath", "scriptServer", "scriptRights", 'serverDisplayName',
		"modules", "activeTypes", "types", "tools", "accessLevel", 'loginMethod', 'loginSearchSuffix',
		'loginSearchFilter', 'searchLimit', 'lamProMailFrom', 'lamProMailReplyTo', 'lamProMailSubject',
		'lamProMailText', 'lamProMailIsHTML', 'lamProMailAllowAlternateAddress', 'httpAuthentication', 'loginSearchDN',
		'loginSearchPassword', 'timeZone', 'jobsBindUser', 'jobsBindPassword', 'jobsDatabase', 'jobToken', 'jobs',
		'jobsDBHost', 'jobsDBPort', 'jobsDBUser', 'jobsDBPassword', 'jobsDBName', 'pwdResetAllowSpecificPassword',
		'pwdResetAllowScreenPassword', 'pwdResetForcePasswordChange', 'pwdResetDefaultPasswordOutput',
		'scriptUserName', 'scriptSSHKey', 'scriptSSHKeyPassword', 'twoFactorAuthentication', 'twoFactorAuthenticationURL',
		'twoFactorAuthenticationInsecure', 'twoFactorAuthenticationLabel', 'twoFactorAuthenticationOptional',
		'twoFactorAuthenticationCaption', 'twoFactorAuthenticationClientId', 'twoFactorAuthenticationSecretKey',
		'twoFactorAuthenticationDomain', 'twoFactorAuthenticationAttribute', 'referentialIntegrityOverlay',
		'hidePasswordPromptForExpiredPasswords', 'hideDnPart', 'pwdPolicyMinLength', 'pwdPolicyMinLowercase',
		'pwdPolicyMinUppercase', 'pwdPolicyMinNumeric', 'pwdPolicyMinSymbolic'
	);

	/**
	 * Returns the server profile data.
	 *
	 * @return array data
	 */
	public function exportData() {
		$data = array();
		$settingsToIgnore = array('modules', 'types', 'tools', 'jobs');
		foreach ($this->settings as $setting) {
			if (in_array($setting, $settingsToIgnore)) {
				continue;
			}
			$data[$setting] = $this->$setting;
		}
		$data['typeSettings'] = $this->typeSettings;
		$data['moduleSettings'] = $this->moduleSettings;
		$data['toolSettings'] = $this->toolSettings;
		$data['jobSettings'] = $this->jobSettings;
		return $data;
	}

	/**
	 * Imports server profile data.
	 *
	 * @param array $data config data
	 * @throws LAMException import error
	 */
	public function importData($data) {
		$settingsToIgnore = array('modules', 'types', 'tools', 'jobs', 'typeSettings',
			'moduleSettings', 'toolSettings', 'jobSettings');
		foreach ($data as $dataKey => $dataValue) {
			if (in_array($dataKey, $settingsToIgnore)) {
				continue;
			}
			if (!in_array($dataKey, $this->settings)) {
				logNewMessage(LOG_WARNING, 'Ignored setting during import: ' . $dataKey);
				continue;
			}
			if (!(($dataValue === null) || is_array($dataValue) || is_string($dataValue) || is_int($dataValue) || is_bool($dataValue))) {
				throw new LAMException('Invalid import data type for ' . htmlspecialchars($dataKey) . ': ' . gettype($dataValue));
			}
			$this->$dataKey = $dataValue;
		}
		$typeSettingsData = !empty($data['typeSettings']) && is_array($data['typeSettings']) ? $data['typeSettings'] : array();
		$this->typeSettings = $typeSettingsData;
		$moduleSettingsData = !empty($data['moduleSettings']) && is_array($data['moduleSettings']) ? $data['moduleSettings'] : array();
		$this->moduleSettings = $moduleSettingsData;
		$toolSettingsData = !empty($data['toolSettings']) && is_array($data['toolSettings']) ? $data['toolSettings'] : array();
		$this->toolSettings = $toolSettingsData;
		$jobSettingsData = !empty($data['jobSettings']) && is_array($data['jobSettings']) ? $data['jobSettings'] : array();
		$this->jobSettings = $jobSettingsData;
	}

	/**
	 * Removes any non-existing types from the configuration.
	 */
	public function removeInvalidTypes() {
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
	public function removeInvalidModules() {
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
					if (in_array($modules[$i], $available)) {
						$ret[] = $modules[$i];
					}
				}
				$this->typeSettings[$moduleVar] = implode(",", $ret);
			}
		}
	}

	/**
	 * Checks if the given server profile name is valid.
	 *
	 * @param string $name server profile name
	 * @return bool is valid
	 */
	public static function isValidName(string $name) : bool {
		return preg_match("/^[a-z0-9_-]+$/i", $name);
	}

	/**
	 * Returns the name of the config file
	 *
	 * @return string name
	 */
	public function getName() : string {
		return $this->name;
	}

	/**
	 * Sets the name of the config file
	 *
	 * @param string $name name
	 */
	public function setName(string $name) : void {
		$this->name = $name;
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
		if (is_string($value)) {
			$this->ServerURL = $value;
			return true;
		}
		return false;
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
		if (is_string($value)) {
			$this->serverDisplayName = $value;
			return true;
		}
		return false;
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
	 * Returns if referential integrity overlay is in place.
	 *
	 * @return String true or false
	 */
	public function getReferentialIntegrityOverlay() {
		return $this->referentialIntegrityOverlay;
	}

	/**
	 * Sets if referential integrity overlay is in place.
	 *
	 * @param String $referentialIntegrityOverlay true or false
	 */
	public function setReferentialIntegrityOverlay($referentialIntegrityOverlay) {
		$this->referentialIntegrityOverlay = $referentialIntegrityOverlay;
	}

	/**
	 * Returns if referential integrity overlay is in place.
	 *
	 * @return bool overlay in place
	 */
	public function isReferentialIntegrityOverlayActive() {
		return $this->referentialIntegrityOverlay === 'true';
	}

	/**
	 * Hide password prompt for expired passwords.
	 *
	 * @return String true or false
	 */
	public function getHidePasswordPromptForExpiredPasswords() {
		return $this->hidePasswordPromptForExpiredPasswords;
	}

	/**
	 * Sets if password prompt for expired passwords is hidden.
	 *
	 * @param String $hidePasswordPromptForExpiredPasswords true or false
	 */
	public function setHidePasswordPromptForExpiredPasswords($hidePasswordPromptForExpiredPasswords) {
		$this->hidePasswordPromptForExpiredPasswords = $hidePasswordPromptForExpiredPasswords;
	}

	/**
	 * Hide password prompt for expired passwords.
	 *
	 * @return bool is hidden
	 */
	public function isHidePasswordPromptForExpiredPasswords() {
		return $this->hidePasswordPromptForExpiredPasswords === 'true';
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
	* Returns all admin users separated by semicolons
	*
	* @return string the admin string
	*/
	public function get_Adminstring() {
		return $this->Admins;
	}

	/**
	* Sets the admin string
	*
	* @param string $value new admin string that contains all admin users separated by semicolons
	* @return boolean true if $value has correct format
	*/
	public function set_Adminstring($value) {
		if (is_string($value) &&
			preg_match("/^[^;]+(;[^;]+)*$/", $value)) {
			$this->Admins = $value;
			return true;
		}
		return false;
	}

	/**
	 * Checks if the given password matches.
	 *
	 * @param String $password
	 * @return boolean true, if matches
	 */
	public function check_Passwd($password) {
		if (strpos($this->Passwd, "{SSHA}") === 0) {
			$value = substr($this->Passwd, strlen("{SSHA}"));
			$parts = explode(" ", $value);
			$salt = base64_decode($parts[1]);
			$hash = "{SSHA}" . base64_encode(hex2bin(sha1($password . $salt))) . " " . base64_encode($salt);
			return ($hash === $this->Passwd);
		}
		elseif (strpos($this->Passwd, "{CRYPT-SHA512}") === 0) {
			$value = substr($this->Passwd, strlen("{CRYPT-SHA512}"));
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
			$this->Passwd = $this->hashPassword($value, generateSalt(16));
			return true;
		}
		else {
			return false;
		}
	}

	/**
	 * Returns the password hash.
	 *
	 * @return string password hash
	 */
	public function getPasswd() {
		return $this->Passwd;
	}

	/**
	 * Returns the hashed password.
	 *
	 * @param String $password password
	 * @param String $salt salt
	 * @return String hash value
	 */
	private function hashPassword($password, $salt) {
		return "{CRYPT-SHA512}" . crypt($password, '$6$' . $salt) . " " . base64_encode($salt);
	}

	/**
	* Returns the LDAP suffix for the given account type
	*
	* @param string $typeId account type
	* @return string the LDAP suffix
	*/
	public function get_Suffix($typeId) {
		return $this->typeSettings['suffix_' . $typeId];
	}

	/**
	* Sets the LDAP suffix where accounts are saved
	*
	* @param string $scope account type
	* @param string $value new LDAP suffix
	* @return boolean true if $value has correct format
	*/
	public function set_Suffix($scope, $value) {
		if (!$value) {
			$value = "";
		}
		elseif (!is_string($value)) {
			return false;
		}
		$this->typeSettings['suffix_' . $scope] = $value;
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
		if (is_string($value)) {
			$this->defaultLanguage = $value;
		}
		else {
			return false;
		}
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
		if (!$value) {
			$this->scriptPath = ""; // optional parameter
		}
		elseif (is_string($value) && preg_match("/^\\/([a-z0-9_-])+(\\/([a-z0-9_\\.-])+)+$/i", $value)) {
			$this->scriptPath = $value;
		}
		else {
			return false;
		}
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
	 * Returns the list of configureed servers.
	 *
	 * @return RemoteServerConfiguration[] servers
	 */
	public function getConfiguredScriptServers() {
		return LAMConfig::parseConfiguredScriptServers($this->scriptServer, $this->get_scriptPath(),
			$this->getScriptUserName(), $this->getScriptSSHKey(), $this->getScriptSSHKeyPassword());
	}

	/**
	 * Parses the configured server config.
	 *
	 * @param string $value configured value
	 * @param string $scriptPath script path
	 * @param string $userName user name
	 * @param string $sshKey SSH key path
	 * @param string $sshKeyPassword SSH key password
	 * @return RemoteServerConfiguration[] configurations
	 */
	public static function parseConfiguredScriptServers($value, $scriptPath, $userName, $sshKey, $sshKeyPassword) : array {
		$servers = array();
		if (empty($value)) {
			return $servers;
		}
		$serverChunks = explode(';', $value);
		foreach ($serverChunks as $serverChunk) {
			if (empty($serverChunk)) {
				continue;
			}
			$serverSettings = explode(':', $serverChunk);
			$serverName = $serverSettings[0];
			$serverLabel = isset($serverSettings[1]) ? $serverSettings[1] : null;
			$serverHomedirPrefix = isset($serverSettings[2]) ? $serverSettings[2] : null;
			$servers[] = new RemoteServerConfiguration($serverName, $serverLabel, $serverHomedirPrefix, $scriptPath, $userName, $sshKey, $sshKeyPassword);
		}
		return $servers;
	}

	/**
	 * Returns the script server object for the given name.
	 *
	 * @param string $serverName server name
	 * @return RemoteServerConfiguration server
	 */
	public function getScriptServerByName($serverName) {
		$serverList = $this->getConfiguredScriptServers();
		foreach ($serverList as $server) {
			if ($serverName === $server->getServer()) {
				return $server;
			}
		}
		return null;
	}

	/**
	* Sets the servers of the external script
	*
	* @param string $value new script servers
	* @return boolean true if $value has correct format
	*/
	public function set_scriptServers($value) {
		$parsedValue = LAMConfig::parseScriptServerValue($value);
		if ($parsedValue === false) {
			return false;
		}
		$this->scriptServer = $parsedValue;
		return true;
	}

	/**
	 * Parses the script server value from input field.
	 *
	 * @param $value input field value
	 * @return string|false value to store in configuration
	 */
	public static function parseScriptServerValue($value) {
		if (!$value) {
			return ""; // optional parameter
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
					$servername = $arr_value_explode[0];
					$label = $arr_value_explode[1];
					$homedirPrefix = isset($arr_value_explode[2]) ? $arr_value_explode[2] : '';
				}
				else {
					$servername = $arr_value;
					$label = '';
					$homedirPrefix = '';
				}
				if (isset($servername) && is_string($servername) && preg_match("/^[a-z0-9-]+(\\.[a-z0-9-]+)*(,[0-9]+)?$/i", $servername)) {
					$serverData = array($servername);
					if (!empty($label)) {
						$serverData[] = $label;
					}
					if (!empty($homedirPrefix)) {
						$serverData[] = $homedirPrefix;
					}
					$serverChunk = implode(':', $serverData);
					$valid_ips[] = $serverChunk;
				}
				else {
					// wrong format
					return false;
				}
			}
			// Check that the array is not empty
			if (!empty($array_string)) {
				return implode(";", $valid_ips);
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
		if (!isset($this->scriptRights)) {
			return '755';
		}
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
		return deobfuscateText($this->scriptSSHKeyPassword);
	}

	/**
	* Sets the password for the lamdamon SSH key.
	*
	* @param string $value password
	*/
	public function setScriptSSHKeyPassword($value) {
		$this->scriptSSHKeyPassword = obfuscateText($value);
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
			return true;
		}
		return false;
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
		if (!is_array($modules)) {
			return false;
		}
		// check module names
		$available = getAvailableModules($scope);
		for ($i = 0; $i < sizeof($modules); $i++) {
			if (! in_array($modules[$i], $available)) return false;
		}
		// check depends/conflicts
		if (check_module_conflicts($modules, getModulesDependencies($scope))) {
			return false;
		}
		if (check_module_depends($modules, getModulesDependencies($scope))) {
			return false;
		}
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
		if (!is_array($settings)) {
			return false;
		}
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
	* @return string[] list of types
	*/
	public function get_ActiveTypes() {
		if (($this->activeTypes == '') || !isset($this->activeTypes)) {
			return array();
		}
		else {
			return explode(",", $this->activeTypes);
		}
	}

	/**
	 * Returns a list of active account types as string.
	 *
	 * @return string list of types
	 */
	public function getActiveTypesString() {
		return $this->activeTypes;
	}

	/**
	* Sets the list of active types.
	*
	* @param string[] list of types
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
		if (!is_array($settings)) {
			return false;
		}
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
		if (!is_array($toolSettings)) {
			return false;
		}
		$this->toolSettings = $toolSettings;
		return true;
	}

	/**
	 * Checks if the given tool is activated.
	 *
	 * @param string $toolName tool name
	 * @return bool is active
	 */
	public function isToolActive(string $toolName): bool {
		return !isset($this->toolSettings['tool_hide_' . $toolName]) || ($this->toolSettings['tool_hide_' . $toolName] !== 'true');
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
		$this->loginSearchPassword = obfuscateText($loginSearchPassword);
	}

	/**
	 * Returns the password for the login search bind user.
	 *
	 * @return String password
	 */
	public function getLoginSearchPassword() {
		return deobfuscateText($this->loginSearchPassword);
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
	 * Returns the mail body for password reset mails without formatting.
	 *
	 * @return String body
	 */
	public function getLamProMailTextRaw() {
		return $this->lamProMailText;
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
	 * @return string authentication URL
	 */
	public function getTwoFactorAuthenticationURL() {
		return implode("\r\n", explode(LAMConfig::LINE_SEPARATOR, $this->twoFactorAuthenticationURL));
	}

	/**
	 * Returns the authentication URL without formatting.
	 *
	 * @return string authentication URL
	 */
	public function getTwoFactorAuthenticationURLRaw() {
		return $this->twoFactorAuthenticationURL;
	}

	/**
	 * Sets the authentication URL.
	 *
	 * @param string $twoFactorAuthenticationURL authentication URL
	 */
	public function setTwoFactorAuthenticationURL($twoFactorAuthenticationURL) {
		$this->twoFactorAuthenticationURL = implode(LAMConfig::LINE_SEPARATOR, explode("\r\n", $twoFactorAuthenticationURL));
	}

	/**
	 * Sets the client id.
	 *
	 * @param string $clientId client id
	 */
	public function setTwoFactorAuthenticationClientId($clientId) {
		$this->twoFactorAuthenticationClientId = $clientId;
	}

	/**
	 * Returns the client id.
	 *
	 * @return string client id
	 */
	public function getTwoFactorAuthenticationClientId() {
		return $this->twoFactorAuthenticationClientId;
	}

	/**
	 * Sets the secret key.
	 *
	 * @param string $secretKey secret key
	 */
	public function setTwoFactorAuthenticationSecretKey($secretKey) {
		$this->twoFactorAuthenticationSecretKey = $secretKey;
	}

	/**
	 * Returns the secret key.
	 *
	 * @return string secret key
	 */
	public function getTwoFactorAuthenticationSecretKey() {
		return $this->twoFactorAuthenticationSecretKey;
	}

	/**
	 * Sets the domain.
	 *
	 * @param string $domain domain
	 */
	public function setTwoFactorAuthenticationDomain($domain) {
		$this->twoFactorAuthenticationDomain = $domain;
	}

	/**
	 * Returns the domain.
	 *
	 * @return string domain
	 */
	public function getTwoFactorAuthenticationDomain() {
		return $this->twoFactorAuthenticationDomain;
	}

	/**
	 * Returns if SSL certificate verification is turned off.
	 *
	 * @return bool SSL certificate verification is turned off
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
	 * @return string authentication label
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
	 * @return bool 2nd factor is optional
	 */
	public function getTwoFactorAuthenticationOptional() {
		return boolval($this->twoFactorAuthenticationOptional);
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
	 * @return string caption HTML
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

	/**
	 * Returns the user attribute.
	 *
	 * @return string user attribute
	 */
	public function getTwoFactorAuthenticationAttribute() {
		if (empty($this->twoFactorAuthenticationAttribute)) {
			return 'uid';
		}
		return $this->twoFactorAuthenticationAttribute;
	}

	/**
	 * Sets the user attribute.
	 *
	 * @param string $twoFactorAuthenticationAttribute user attribute
	 */
	public function setTwoFactorAuthenticationAttribute($twoFactorAuthenticationAttribute) {
		$this->twoFactorAuthenticationAttribute = $twoFactorAuthenticationAttribute;
	}

	/**
	 * Returns the DN part to hide.
	 *
	 * @return string DN part
	 */
	public function getHideDnPart() {
		return $this->hideDnPart;
	}

	/**
	 * Sets the DN part to hide.
	 *
	 * @param string $hideDnPart DN part
	 */
	public function setHideDnPart($hideDnPart) {
		$this->hideDnPart = $hideDnPart;
	}

	/**
	 * Returns the minimum password length.
	 *
	 * @return string minimum length
	 */
	public function getPwdPolicyMinLength() {
		return $this->pwdPolicyMinLength;
	}

	/**
	 * Sets the minimum password length.
	 *
	 * @param string $pwdPolicyMinLength minimum length
	 */
	public function setPwdPolicyMinLength($pwdPolicyMinLength) {
		$this->pwdPolicyMinLength = $pwdPolicyMinLength;
	}

	/**
	 * Returns the minimum of lowercase characters.
	 *
	 * @return string minimum
	 */
	public function getpwdPolicyMinLowercase() {
		return $this->pwdPolicyMinLowercase;
	}

	/**
	 * Sets the minimum of lowercase characters.
	 *
	 * @param string $pwdPolicyMinLowercase minimum
	 */
	public function setPwdPolicyMinLowercase($pwdPolicyMinLowercase) {
		$this->pwdPolicyMinLowercase = $pwdPolicyMinLowercase;
	}

	/**
	 * Returns the minimum of uppercase characters.
	 *
	 * @return string minimum
	 */
	public function getPwdPolicyMinUppercase() {
		return $this->pwdPolicyMinUppercase;
	}

	/**
	 * Sets the minimum of uppercase characters.
	 *
	 * @param string $pwdPolicyMinUppercase minimum
	 */
	public function setPwdPolicyMinUppercase($pwdPolicyMinUppercase) {
		$this->pwdPolicyMinUppercase = $pwdPolicyMinUppercase;
	}

	/**
	 * Returns the minimum of numeric characters.
	 *
	 * @return string minimum
	 */
	public function getPwdPolicyMinNumeric() {
		return $this->pwdPolicyMinNumeric;
	}

	/**
	 * Sets the minimum of numeric characters.
	 *
	 * @param string $pwdPolicyMinNumeric minimum
	 */
	public function setPwdPolicyMinNumeric($pwdPolicyMinNumeric) {
		$this->pwdPolicyMinNumeric = $pwdPolicyMinNumeric;
	}

	/**
	 * Returns the minimum of symbolic characters.
	 *
	 * @return string minimum
	 */
	public function getPwdPolicyMinSymbolic() {
		return $this->pwdPolicyMinSymbolic;
	}

	/**
	 * Sets the minimum of symbolic characters.
	 *
	 * @param string $pwdPolicyMinSymbolic minimum
	 */
	public function setPwdPolicyMinSymbolic($pwdPolicyMinSymbolic) {
		$this->pwdPolicyMinSymbolic = $pwdPolicyMinSymbolic;
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

	/** send license warnings via email */
	const LICENSE_WARNING_EMAIL = 'email';
	/** display license warnings on screen */
	const LICENSE_WARNING_SCREEN = 'screen';
	/** send license warnings via email + display on screen */
	const LICENSE_WARNING_ALL = 'all';
	/** no license warning */
	const LICENSE_WARNING_NONE = 'none';

	/** SSL encryption for mail sending */
	const SMTP_SSL = 'SSL';
	/** TLS encryption for mail sending */
	const SMTP_TLS = 'TLS';
	/** No encryption for mail sending */
	const SMTP_NONE = 'NONE';

	/** store configuration on local file system */
	const DATABASE_FILE_SYSTEM = 'files';
	/** store configuration in MySQL database */
	const DATABASE_MYSQL = 'mysql';

	/** Default profile */
	public $default;

	/** Password to change config.cfg */
	private $password;

	/** Time of inactivity before session times out (minutes) */
	public $sessionTimeout;

	/** log level */
	public $logLevel;

	/** log destination ("SYSLOG":syslog, "/...":file, "NONE":none, "REMOTE":server:port) */
	public $logDestination;

	/** list of hosts which may access LAM */
	public $allowedHosts;

	/** list of hosts which may access LAM Pro self service */
	public $allowedHostsSelfService;

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

	/** external URL for password checking (e.g. https://domain.com/url/{SHA1}) */
	public $externalPwdCheckUrl = null;

	/** path to config file */
	private $conffile;

	/** uploaded SSL certificate that is stored to disk on save() */
	private $uploadedSSLCaCert = null;

	/** SSL certificate should be deleted on save() */
	private $delSSLCaCert = false;

	/** error reporting */
	public $errorReporting = self::ERROR_REPORTING_DEFAULT;

	/** license data */
	private $license = '';

	/** license warning email from address */
	public $licenseEmailFrom = '';

	/** license warning email TO address(es) separated by ";" */
	public $licenseEmailTo = '';

	/** license warning email was last sent for this expiration date */
	public $licenseEmailDateSent = '';

	/** type of license warning (email/screen/both/none) */
	public $licenseWarningType = '';

	/** mail server (server:port) */
	public $mailServer = '';

	/** mail server user */
	public $mailUser = '';

	/** mail server password */
	public $mailPassword = '';

	/**
	 * @var string encryption type for SMTP connection
	 */
	public $mailEncryption = '';

	/** database type */
	public $configDatabaseType = self::DATABASE_FILE_SYSTEM;

	/** database server name */
	public $configDatabaseServer = '';

	/** database port */
	public $configDatabasePort = '';

	/** database name */
	public $configDatabaseName = '';

	/** database user */
	public $configDatabaseUser = '';

	/** database password */
	public $configDatabasePassword = '';

	/** list of data fields to save in config file */
	private $settings = array("password", "default", "sessionTimeout",
		"logLevel", "logDestination", "allowedHosts", "passwordMinLength",
		"passwordMinUpper", "passwordMinLower", "passwordMinNumeric",
		"passwordMinClasses", "passwordMinSymbol", 'checkedRulesCount',
		'passwordMustNotContainUser', 'passwordMustNotContain3Chars',
		'externalPwdCheckUrl',
		'errorReporting', 'allowedHostsSelfService',
		'license', 'licenseEmailFrom', 'licenseEmailTo', 'licenseWarningType', 'licenseEmailDateSent',
		'mailServer', 'mailUser', 'mailPassword', 'mailEncryption', 'configDatabaseType',
		'configDatabaseServer', 'configDatabasePort', 'configDatabaseName', 'configDatabaseUser',
		'configDatabasePassword'
	);

	/** persistence settings are always stored on local file system */
	private $persistenceSettings = array(
		'configDatabaseType', 'configDatabaseServer',
		'configDatabasePort', 'configDatabaseName', 'configDatabaseUser',
		'configDatabasePassword', 'license'
	);

	/**
	 * Loads preferences from config file
	 *
	 * @param string $fileName file path for config file
	 */
	function __construct($fileName = null) {
		if ($fileName === null) {
			$this->conffile = __DIR__ . "/../config/config.cfg";
		}
		else {
			$this->conffile = $fileName;
		}
		// set default values
		$this->sessionTimeout = 30;
		$this->logLevel = LOG_NOTICE;
		$this->logDestination = "SYSLOG";
		$this->allowedHosts = "";
		$this->allowedHostsSelfService = '';
		try {
			$this->reload();
		} catch (LAMException $e) {
			syslog(LOG_ERR, 'Error loading main config: ' . $e->getTitle() . ' ' . $e->getMessage());
		}
	}

	/**
	 * Exports the configuration data.
	 *
	 * @return array config data
	 */
	public function exportData($skipPersistenceSettings = false): array {
		$data = array();
		foreach ($this->settings as $setting) {
			if ($skipPersistenceSettings && in_array($setting, $this->persistenceSettings)) {
				continue;
			}
			$data[$setting] = $this->$setting;
		}
		return $data;
	}

	/**
	 * Imports configuration data.
	 *
	 * @param array $data config data
	 * @throws LAMException import error
	 */
	public function importData($data) {
		foreach ($data as $dataKey => $dataValue) {
			if (!in_array($dataKey, $this->settings)) {
				logNewMessage(LOG_WARNING, 'Ignored setting during import: ' . $dataKey);
				continue;
			}
			if (!(($dataValue === null) || is_array($dataValue) || is_string($dataValue) || is_int($dataValue) || is_bool($dataValue))) {
				throw new LAMException('Invalid import data type for ' . htmlspecialchars($dataKey) . ': ' . gettype($dataValue));
			}
			$this->$dataKey = $dataValue;
		}
	}

	/**
	 * Returns the content of the server certificates file
	 *
	 * @return null|string certificates
	 */
	public function exportCertificates() {
		$fileName = $this->getSSLCaCertPath();
		if ($fileName === null) {
			return null;
		}
		$content = null;
		$handle = @fopen($fileName, "r");
		if ($handle) {
			$content = fread($handle, 10000000);
			fclose($handle);
		}
		return $content;
	}

	/**
	 * Imports the server certificates.
	 *
	 * @param null|string $certsContent certificates
	 * @throws LAMException write to file failed
	 */
	public function importCertificates($certsContent) {
		$fileName = $this->getSSLCaCertPath();
		if (empty($certsContent)) {
			if ($fileName !== null) {
				unlink($fileName);
			}
			return;
		}
		$fileName = $this->getInternalSSLCaCertFileName();
		$handle = @fopen($fileName, "wb");
		if ($handle) {
			fputs($handle, $certsContent);
			fclose($handle);
			@chmod($fileName, 0600);
		}
		else {
			throw new LAMException(printf(_('Unable to write file %s.'), $fileName));
		}
	}

	/**
	 * Reloads preferences from config file config.cfg
	 *
	 * @throws LAMException error reading config
	 */
	private function reload() : void {
		if (is_file($this->conffile)) {
			$file = @fopen($this->conffile, "r");
			if (!$file) {
				throw new LAMException(_('The config file is not readable.'));
			}
			while (!feof($file)) {
				$line = fgets($file, 1000000);
				$line = trim($line);  // remove spaces at the beginning and end
				if (($line == "")||($line[0] == "#")) {
					continue; // ignore comments
				}
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
		if ($this->configDatabaseType === self::DATABASE_MYSQL) {
			$this->loadFromDb();
		}
	}

	/**
	 * Loads the settings from the database.
	 * Persistence settings are ignored as they must be on local file system.
	 *
	 * @return bool true when ok
	 * @throws LAMException error reading config
	 */
	private function loadFromDb() : void {
		$configDb = new ConfigurationDatabase($this);
		try {
			$pdo = $configDb->getPdo();
			$result = $pdo->query("SELECT value FROM " . ConfigurationDatabase::TABLE_MAIN_CONFIG . ' WHERE name=\'config\'');
			if ($result === false) {
				syslog(LOG_ERR, 'Main configuration table query failed.');
				throw new LAMException(_('Unable to connect to configuration database.'));
			}
			$data = $result->fetchAll();
			if (empty($data)) {
				syslog(LOG_ERR, 'Main configuration table is empty.');
				throw new LAMException(_('Unable to connect to configuration database.'));
			}
			$jsonData = json_decode($data[0]['value'], true);
			foreach ($this->persistenceSettings as $persistenceSetting) {
				if (isset($jsonData[$persistenceSetting])) {
					unset($jsonData[$persistenceSetting]);
				}
			}
			$this->importData($jsonData);
		}
		catch (PDOException $e) {
			syslog(LOG_ERR, 'Unable to read main config: ' . $e->getMessage());
			throw new LAMException(_('Unable to connect to configuration database.'));
		}
		catch (LAMException $e) {
			syslog(LOG_ERR, 'Unable to import main config: ' . $e->getMessage());
			throw new LAMException(_('Unable to connect to configuration database.'));
		}
	}

	/**
	 * Saves the configuration to the persistence layer.
	 */
	public function save() {
		if ($this->configDatabaseType === self::DATABASE_MYSQL) {
			$this->saveLocal(true);
			$this->saveDb();
		}
		else {
			$this->saveLocal(false);
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
	 * Saves the config data to the database.
	 */
	private function saveDb() : void {
		$configDb = new ConfigurationDatabase($this);
		$pdo = $configDb->getPdo();
		$json = json_encode($this->exportData(true));
		$nameKey = 'config';
		$pdo->beginTransaction();
		// remove existing data
		$sql = 'delete from ' . ConfigurationDatabase::TABLE_MAIN_CONFIG . ' where name = "' . $nameKey . '";';
		$pdo->exec($sql);
		// new data
		$stmt = $pdo->prepare('insert into ' . ConfigurationDatabase::TABLE_MAIN_CONFIG . ' (name, value) VALUES (?, ?)');
		$stmt->execute(array($nameKey, $json));
		$pdo->commit();
	}

	/**
	 * Saves preferences to config file config.cfg
	 *
	 * @param bool $persistenceOnly store only persistence related data
	 */
	public function saveLocal(bool $persistenceOnly) : void {
		if (is_file($this->conffile)) {
			$file = fopen($this->conffile, "r");
			$file_array = array();
			// read config file
			while (!feof($file)) {
				$configLine = fgets($file, 1000000);
				if (($configLine === false) || ($configLine === "") || ($configLine === "\n") || ($configLine[0] == "#")) {
					continue; // ignore comments and empty lines
				}
				array_push($file_array, $configLine);
			}
			fclose($file);
			// generate new configuration file
			$saved = array();
			for ($i = 0; $i < sizeof($file_array); $i++) {
				$line = trim($file_array[$i]);
				if (($line == "")||($line[0] == "#")) {
					continue; // ignore comments and empty lines
				}
				// search keywords
				for ($k = 0; $k < sizeof($this->settings); $k++) {
					$keyword = $this->settings[$k];
					if ($persistenceOnly && !in_array($keyword, $this->persistenceSettings)) {
						continue;
					}
					$keylen = strlen($keyword);
					if (strtolower(substr($line, 0, $keylen + 1)) == strtolower($keyword . ":")) {
						$file_array[$i] = $keyword . ": " . $this->$keyword . "\n";
						$saved[] = $keyword;	// mark keyword as saved
						break;
					}
				}
			}
		}
		if (!$persistenceOnly) {
			// check if we have to add new entries (e.g. if user upgraded LAM and has an old config file)
			if (!in_array("password", $saved)) {
				array_push($file_array, "password: " . $this->password . "\n");
			}
			if (!in_array("default", $saved)) {
				array_push($file_array, "default: " . $this->default . "\n");
			}
			if (!in_array("sessionTimeout", $saved)) {
				array_push($file_array, "sessionTimeout: " . $this->sessionTimeout . "\n");
			}
			if (!in_array("logLevel", $saved)) {
				array_push($file_array, "logLevel: " . $this->logLevel . "\n");
			}
			if (!in_array("logDestination", $saved)) {
				array_push($file_array, "logDestination: " . $this->logDestination . "\n");
			}
			if (!in_array("allowedHosts", $saved)) {
				array_push($file_array, "allowedHosts: " . $this->allowedHosts . "\n");
			}
			if (!in_array("allowedHostsSelfService", $saved)) {
				array_push($file_array, "allowedHostsSelfService: " . $this->allowedHostsSelfService . "\n");
			}
			if (!in_array("passwordMinLength", $saved)) {
				array_push($file_array, "passwordMinLength: " . $this->passwordMinLength . "\n");
			}
			if (!in_array("passwordMinUpper", $saved)) {
				array_push($file_array, "passwordMinUpper: " . $this->passwordMinUpper . "\n");
			}
			if (!in_array("passwordMinLower", $saved)) {
				array_push($file_array, "passwordMinLower: " . $this->passwordMinLower . "\n");
			}
			if (!in_array("passwordMinNumeric", $saved)) {
				array_push($file_array, "passwordMinNumeric: " . $this->passwordMinNumeric . "\n");
			}
			if (!in_array("passwordMinSymbol", $saved)) {
				array_push($file_array, "passwordMinSymbol: " . $this->passwordMinSymbol . "\n");
			}
			if (!in_array("passwordMinClasses", $saved)) {
				array_push($file_array, "passwordMinClasses: " . $this->passwordMinClasses . "\n");
			}
			if (!in_array("checkedRulesCount", $saved)) {
				array_push($file_array, "checkedRulesCount: " . $this->checkedRulesCount . "\n");
			}
			if (!in_array("passwordMustNotContain3Chars", $saved)) {
				array_push($file_array, "passwordMustNotContain3Chars: " . $this->passwordMustNotContain3Chars . "\n");
			}
			if (!in_array("passwordMustNotContainUser", $saved)) {
				array_push($file_array, "passwordMustNotContainUser: " . $this->passwordMustNotContainUser . "\n");
			}
			if (!in_array("externalPwdCheckUrl", $saved)) {
				array_push($file_array, "externalPwdCheckUrl: " . $this->externalPwdCheckUrl . "\n");
			}
			if (!in_array("errorReporting", $saved)) {
				array_push($file_array, "errorReporting: " . $this->errorReporting . "\n");
			}
			if (!in_array("license", $saved)) {
				array_push($file_array, "license: " . $this->license . "\n");
			}
			if (!in_array("licenseEmailFrom", $saved)) {
				array_push($file_array, "licenseEmailFrom: " . $this->licenseEmailFrom . "\n");
			}
			if (!in_array("licenseEmailTo", $saved)) {
				array_push($file_array, "licenseEmailTo: " . $this->licenseEmailTo . "\n");
			}
			if (!in_array("licenseEmailDateSent", $saved)) {
				array_push($file_array, "licenseEmailDateSent: " . $this->licenseEmailDateSent . "\n");
			}
			if (!in_array("licenseWarningType", $saved)) {
				array_push($file_array, "licenseWarningType: " . $this->licenseWarningType . "\n");
			}
			if (!in_array("mailServer", $saved)) {
				array_push($file_array, "mailServer: " . $this->mailServer . "\n");
			}
			if (!in_array("mailUser", $saved)) {
				array_push($file_array, "mailUser: " . $this->mailUser . "\n");
			}
			if (!in_array("mailPassword", $saved)) {
				array_push($file_array, "mailPassword: " . $this->mailPassword . "\n");
			}
			if (!in_array("mailEncryption", $saved)) {
				array_push($file_array, "mailEncryption: " . $this->mailEncryption . "\n");
			}
		}
		if (!in_array("configDatabaseType", $saved)) {
			array_push($file_array, "configDatabaseType: " . $this->configDatabaseType . "\n");
		}
		if (!in_array("configDatabaseServer", $saved)) {
			array_push($file_array, "configDatabaseServer: " . $this->configDatabaseServer . "\n");
		}
		if (!in_array("configDatabasePort", $saved)) {
			array_push($file_array, "configDatabasePort: " . $this->configDatabasePort . "\n");
		}
		if (!in_array("configDatabaseName", $saved)) {
			array_push($file_array, "configDatabaseName: " . $this->configDatabaseName . "\n");
		}
		if (!in_array("configDatabaseUser", $saved)) {
			array_push($file_array, "configDatabaseUser: " . $this->configDatabaseUser . "\n");
		}
		if (!in_array("configDatabasePassword", $saved)) {
			array_push($file_array, "configDatabasePassword: " . $this->configDatabasePassword . "\n");
		}

		$file = @fopen($this->conffile, "w");
		if ($file) {
			for ($i = 0; $i < sizeof($file_array); $i++) {
				fputs($file, $file_array[$i]);
			}
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
		$this->password = $this->hashPassword($password, generateSalt(16));
	}

	/**
	 * Checks if the given password matches.
	 *
	 * @param String $password password
	 * @return boolean true, if password matches
	 */
	public function checkPassword($password) {
		if (strpos($this->password, "{SSHA}") === 0) {
			$value = substr($this->password, strlen("{SSHA}"));
			$parts = explode(" ", $value);
			$salt = base64_decode($parts[1]);
			$hash = "{SSHA}" . base64_encode(hex2bin(sha1($password . $salt))) . " " . base64_encode($salt);
			return ($hash === $this->password);
		}
		elseif (strpos($this->password, "{CRYPT-SHA512}") === 0) {
			$value = substr($this->password, strlen("{CRYPT-SHA512}"));
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
		return "{CRYPT-SHA512}" . crypt($password, '$6$' . $salt) . " " . base64_encode($salt);
	}

	/**
	 * Returns if the configuration file is writable.
	 *
	 * @return boolean writable
	 */
	public function isWritable() : bool {
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
	 * @param String[] $licenseLines license lines
	 */
	public function setLicenseLines($licenseLines) {
		$this->license = implode(LAMConfig::LINE_SEPARATOR, $licenseLines);
	}

	/**
	 * Returns the license warning type (screen/email/both/none).
	 *
	 * @return string warning type
	 */
	public function getLicenseWarningType() {
		if (empty($this->licenseWarningType)) {
			return self::LICENSE_WARNING_SCREEN;
		}
		return $this->licenseWarningType;
	}

	/**
	 * Returns if the license warning should be shown on screen.
	 *
	 * @return bool show on screen
	 */
	public function showLicenseWarningOnScreen() {
		$type = $this->getLicenseWarningType();
		return ($type === self::LICENSE_WARNING_ALL) || ($type === self::LICENSE_WARNING_SCREEN);
	}

	/**
	 * Returns if the license warning should be sent via email.
	 *
	 * @return bool send via email
	 */
	public function sendLicenseWarningByEmail() {
		$type = $this->getLicenseWarningType();
		return ($type === self::LICENSE_WARNING_ALL) || ($type === self::LICENSE_WARNING_EMAIL);
	}

	/**
	 * Returns if the license warning was already sent.
	 *
	 * @param int $timeStamp time stamp
	 */
	public function wasLicenseWarningSent($timeStamp) {
		if (empty($this->licenseEmailDateSent)) {
			return false;
		}
		return $timeStamp == $this->licenseEmailDateSent;
	}

}
