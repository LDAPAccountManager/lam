<?php

use LAM\LIB\TWO_FACTOR\TwoFactorProviderService;
use LAM\PDF\PdfStructurePersistenceManager;
use LAM\PERSISTENCE\ConfigurationDatabase;
use LAM\PROFILES\AccountProfilePersistenceManager;
use LAM\REMOTE\RemoteServerConfiguration;
use LAM\TYPES\TypeManager;
use function LAM\PERSISTENCE\dbTableExists;
use function LAM\TYPES\getScopeFromTypeId;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2026  Roland Gruber

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

/**
 * Defines the possible LAM user interfaces.
 */
enum LAM_INTERFACE {
	/** admin pages to manage any type aof entry */
	case ADMIN;
	/** user self-service to manage own data */
	case SELF_SERVICE;
	/** white-pages to display users */
	case WHITE_PAGES;
}

/** persistence */
include_once __DIR__ . '/persistence.inc';
/** Used to print messages. */
include_once __DIR__ . "/status.inc";
/** Used to get module information. */
include_once __DIR__ . "/modules.inc";
/** Used to get type information. */
include_once __DIR__ . "/types.inc";
/** 2-factor */
include_once __DIR__ . '/2factor.inc';

/**
 * Checks if the configuration password is secure.
 *
 * @param string $password password
 * @return bool is secure
 */
function isValidConfigurationPassword(string $password): bool {
	return preg_match('/[a-zA-Z]/', $password)
		&& preg_match('/\d/', $password)
		&& preg_match('/[^a-zA-Z0-9]/', $password)
		&& (strlen($password) >= 8);
}

/**
 * Sets the environment variables for custom SSL CA certificates.
 */
function setSSLCaCert(): void {
	$config = $_SESSION['cfgMain'] ?? new LAMCfgMain();
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
function setlanguage(): void {
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
	$setLocaleResult = setlocale(LC_ALL, [$code, $fallbackCode]);  // set LC_ALL
	if ($setLocaleResult === false) {
		logNewMessage(LOG_WARNING, "Unable to set locale, check if 'locale -a' returns $code");
	}
	$locdir = substr(__FILE__, 0, strlen(__FILE__) - 15) . "/locale";  // set path to translations
	bindtextdomain("messages", $locdir);
	$bindTextResult = bind_textdomain_codeset("messages", $encoding);
	if (!is_string($bindTextResult)) {
		logNewMessage(LOG_WARNING, "Unable to bind text domain, check if 'locale -a' returns $code");
	}
	textdomain("messages");
	header("Content-type: text/html; charset=" . $encoding);
}

/**
 * Checks whether a specific flag in the rights string is set.
 *
 * @param string $right read, write or execute
 * @param string $target owner, group or other
 * @param string $chmod the chmod rights
 *
 * @return bool true, if the chmod $right for $target were set
 */
function checkChmod(string $right, string $target, string $chmod): bool {
	$right_arr = ["read", "write", "execute"];
	$target_arr = ["owner", "group", "other"];

	// Check, if $right and $target has right parameters
	if (!in_array($right, $right_arr) || !in_array($target, $target_arr)) {
		return false;
	}

	$chmod_num = -1;
	// owner:
	if ($target === "owner") {
		$chmod_num = 0;
	}
	if ($target === "group") {
		$chmod_num = 1;
	}
	if ($target === "other") {
		$chmod_num = 2;
	}

	// Cut the number from the chmod:
	$chmod_num = $chmod[$chmod_num];

	// Now check, if the chmod_num can be right with the $right
	// What numbers allow "read"
	$read = [4, 5, 6, 7];
	// What numbers allow "write"
	$write = [2, 3, 6, 7];
	// What numbers allow "execute"
	$execute = [1, 3, 5, 7];
	return (($right === "read") && in_array($chmod_num, $read))
		|| (($right === "write") && in_array($chmod_num, $write))
		|| (($right === "execute") && in_array($chmod_num, $execute));
}

/**
 * Returns the version number of this LAM installation.
 * Format: <major version>.<minor version>.<patch level>
 * <br> Major/minor version are always numbers, patch level may contain letters for unofficial releases only (e.g. 0.5.alpha1).
 *
 * @return string version number
 */
function LAMVersion(): string {
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
 * @param array<string, string> $confTypes array (option name => type (e.g. multiselect))
 * @return array<string, string[]> list of config options (name =>[values])
 */
function extractConfigOptionsFromPOST(array $confTypes): array {
	$options = [];
	foreach ($confTypes as $element => $type) {
		// text fields
		if ($type == "text") {
			$options[$element] = [$_POST[$element]];
		}
		// text fields
		elseif ($type == "text_obfuscated") {
			$options[$element] = [obfuscateText($_POST[$element])];
		}
		// hidden fields
		elseif ($type == "hidden") {
			$options[$element] = [$_POST[$element]];
		}
		// checkboxes
		elseif ($type == "checkbox") {
			$options[$element] = isset($_POST[$element]) && ($_POST[$element] == "on") ? ['true'] : ['false'];
		}
		// dropdownbox
		elseif ($type == "select") {
			$options[$element] = [$_POST[$element]];
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
function metaRefresh(string $page): void {
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
 * @param string $typeId account type (e.g. user)
 * @return bool is hidden
 */
function isAccountTypeHidden(string $typeId): bool {
	$typeSettings = $_SESSION['config']->get_typeSettings();
	return isset($typeSettings['hidden_' . $typeId]) && ($typeSettings['hidden_' . $typeId]);
}

/**
 * Returns a list of all supported languages.
 *
 * @return LAMLanguage[] languages
 */
function getLanguages(): array {
	$languages = [];
	// loading available languages from language.conf file
	$languagefile = __DIR__ . "/../config/language";
	if (is_file($languagefile)) {
		$file = fopen($languagefile, "r");
		while (!feof($file)) {
			$line = fgets($file, 1024);
			if ($line == "" || $line === "\n" || $line[0] === "#") {
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
	public string $code;
	/** character encoding (e.g. UTF-8) */
	public string $encoding;
	/** description for GUI */
	public string $description;

	/**
	 * Constructor
	 *
	 * @param string $code language code (e.g. en_GB.utf8)
	 * @param string $encoding character encoding (e.g. UTF-8)
	 * @param string $description description for GUI
	 */
	public function __construct(string $code, string $encoding, string $description) {
		$this->code = $code;
		$this->encoding = $encoding;
		$this->description = $description;
	}

}

/**
 * Manages the persistence of server profiles.
 */
class ServerProfilePersistenceManager {

	private ServerProfilePersistenceStrategy $strategy;

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
	public function getProfiles(): array {
		return $this->strategy->getProfiles();
	}

	/**
	 * Loads the given server profile
	 *
	 * @param string $name profile name
	 * @return LAMConfig profile
	 * @throws LAMException error loading profile
	 */
	public function loadProfile(string $name): LAMConfig {
		$config = $this->strategy->loadProfile($name);
		$config->setName($name);
		return $config;
	}

	/**
	 * Saves a server profile.
	 *
	 * @param LAMConfig $profile profile
	 * @param string $name profile name
	 * @throws LAMException error saving profile
	 */
	public function saveProfile(LAMConfig $profile, string $name): void {
		$this->strategy->saveProfile($profile, $name);
	}

	/**
	 * Returns an array of string with all available configuration templates (without .sample.conf)
	 *
	 * @return array<int, string> template names
	 */
	public function getConfigTemplates(): array {
		$dir = dir(__DIR__ . "/../config");
		$ret = [];
		$pos = 0;
		while ($entry = $dir->read()) {
			$ext = substr($entry, strlen($entry) - 12, 12);
			$name = substr($entry, 0, strlen($entry) - 12);
			// check if extension is right, add to profile list
			if ($ext === ".sample.conf") {
				$ret[$pos] = $name;
				$pos++;
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
	public function createProfileFromTemplate(string $name, string $templateName, string $password): void {
		if (!LAMConfig::isValidName($name) || !preg_match("/^[a-z0-9\\._-]+$/i", $templateName) || in_array($name, $this->getProfiles())) {
			throw new LAMException(_("Profile name is invalid!"));
		}
		$copyFromTemplate = true;
		$existingTemplateNames = $this->getConfigTemplates();
		if (str_contains($templateName, '.sample')) {
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
	public function renameProfile(string $oldName, string $newName): void {
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
	public function deleteProfile(string $name): void {
		$this->strategy->deleteProfile($name);
	}

	/**
	 * Returns if the configuration is writable.
	 *
	 * @param string $name profile name
	 * @return bool is writable
	 */
	public function isWritable(string $name): bool {
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
	public function getProfiles(): array;

	/**
	 * Loads the given server profile
	 *
	 * @param string $name profile name
	 * @return LAMConfig profile
	 * @throws LAMException error loading profile
	 */
	public function loadProfile(string $name): LAMConfig;

	/**
	 * Saves a server profile.
	 *
	 * @param LAMConfig $profile profile
	 * @param string $name profile name
	 * @throws LAMException error saving profile
	 */
	public function saveProfile(LAMConfig $profile, string $name): void;

	/**
	 * Renames an existing server profile.
	 *
	 * @param string $oldName old profile name
	 * @param string $newName new profile name
	 * @throws LAMException error saving profile
	 */
	public function renameProfile(string $oldName, string $newName): void;

	/**
	 * Deletes a server profile.
	 *
	 * @param string $name profile name
	 * @throws LAMException error deleting profile
	 */
	public function deleteProfile(string $name): void;

	/**
	 * Returns if the configuration is writable.
	 *
	 * @param string $name profile name
	 * @return bool is writable
	 */
	public function isWritable(string $name): bool;

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
		$ret = [];
		while ($entry = $dir->read()) {
			$ext = substr($entry, strlen($entry) - 5, 5);
			$name = substr($entry, 0, strlen($entry) - 5);
			// check if extension is right, add to profile list
			if (($ext === ".conf") && (!str_contains($name, '.sample')) && is_readable($dirName . '/' . $entry)) {
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
		$confFilePath = $this->getPath($name);
		if (!is_file($confFilePath)) {
			logNewMessage(LOG_ERR, 'No such file ' . $confFilePath);
			throw new LAMException(_('Unable to read file.'));
		}
		$file = @fopen($confFilePath, "r");
		if (!$file) {
			logNewMessage(LOG_ERR, 'Unable to open for reading: ' . $confFilePath);
			throw new LAMException(_('Unable to read file.'));
		}
		$json = fread($file, 1000000);
		$data = json_decode($json, true);
		if ($data === null) {
			logNewMessage(LOG_ERR, 'Unable to read JSON from ' . $confFilePath);
			throw new LAMException(_('Unable to read file.'));
		}
		fclose($file);
		$config->importData($data);
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
			$file = @fopen($confFile, "w");
			if ($file) {
				fwrite($file, json_encode($profile->exportData(), JSON_PRETTY_PRINT));
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
		$subDirs = [
			$dir . 'pdf/' . $name . '/logos',
			$dir . 'pdf/' . $name,
			$dir . 'profiles/' . $name
		];
		for ($i = 0; $i < count($subDirs); $i++) {
			if (is_dir($subDirs[$i]) && is_readable($subDirs[$i])) {
				$dirHandle = @opendir($subDirs[$i]);
				while (false !== ($path = readdir($dirHandle))) {
					if (($path !== '.') && ($path !== '..') && !@unlink($subDirs[$i] . '/' . $path)) {
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
		return is_writable($this->getPath($name));
	}

	/**
	 * Returns the path to the config file.
	 *
	 * @param string $name profile name
	 * @return string path on filesystem
	 */
	private function getPath(string $name): string {
		return __DIR__ . "/../config/" . $name . ".conf";
	}

}

/**
 * Stores server profiles in a database.
 */
class ServerProfilePersistenceStrategyPdo implements ServerProfilePersistenceStrategy {

	public const TABLE_NAME = 'server_profiles';

	/**
	 * @var PDO $pdo PDO
	 */
	private PDO $pdo;

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
	private function checkSchema(): void {
		if (!dbTableExists($this->pdo, self::TABLE_NAME)) {
			$this->createInitialSchema();
		}
	}

	/**
	 * Creates the initial schema.
	 */
	public function createInitialSchema(): void {
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
		$profiles = [];
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
		$statement->execute([$name]);
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
		$statement->execute([$name]);
		$results = $statement->fetchAll();
		if (empty($results)) {
			$statement = $this->pdo->prepare("INSERT INTO " . self::TABLE_NAME . " (name, data) VALUES (?, ?)");
			$statement->execute([$name, $data]);
		}
		else {
			$statement = $this->pdo->prepare("UPDATE " . self::TABLE_NAME . " SET data = ? WHERE name = ?");
			$statement->execute([$data, $name]);
		}
	}

	/**
	 * @inheritDoc
	 */
	public function renameProfile(string $oldName, string $newName): void {
		$statement = $this->pdo->prepare("UPDATE " . self::TABLE_NAME . " SET name = ? WHERE name = ?");
		$statement->execute([$newName, $oldName]);
	}

	/**
	 * @inheritDoc
	 */
	public function deleteProfile(string $name): void {
		$statement = $this->pdo->prepare("DELETE FROM " . self::TABLE_NAME . " WHERE name = ?");
		$statement->execute([$name]);
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
	public const ACCESS_ALL = '100';
	public const ACCESS_PASSWORD_CHANGE = '20';
	public const ACCESS_READ_ONLY = '0';

	/* login method: predefined list or LDAP search */
	public const LOGIN_LIST = 'list';
	public const LOGIN_SEARCH = 'search';

	/** line separator */
	public const LINE_SEPARATOR = '+::+';

	/** show password on screen by default */
	public const PWDRESET_DEFAULT_SCREEN = '1';
	/** send password via email by default */
	public const PWDRESET_DEFAULT_MAIL = '2';
	/** send password via SMS by default */
	public const PWDRESET_DEFAULT_SMS = '4';
	/** show password on screen and send via email by default */
	public const PWDRESET_DEFAULT_ALL = '3';

	/** Server address (e.g. ldap://127.0.0.1:389) */
	private string $ServerURL = '';

	/** Display name of LDAP server */
	private string $serverDisplayName = '';

	/** enables/disables TLS encryption */
	private string $useTLS = 'no';

	/** automatically follow referrals */
	private string $followReferrals = 'false';

	/** use paged results */
	private string $pagedResults = 'false';

	/** Active Directory: show deleted entries */
	private string $adShowDeleted = 'false';

	/** overlay for referential integrity is activated */
	private string $referentialIntegrityOverlay = 'false';

	/** hide password prompt for expired passwords */
	private string $hidePasswordPromptForExpiredPasswords = 'false';

	/** list of users with admin rights, separated by semicolon */
	private string $Admins = '';

	/** Password to edit preferences */
	private ?string $Passwd = null;

	/** Default language */
	private string $defaultLanguage = '';
	/** time zone */
	private string $timeZone = 'Europe/London';

	/** @var array<string, string[]> $moduleSettings module settings */
	private array $moduleSettings = [];

	/** @var array<string, string> $typeSettings type settings */
	private array $typeSettings = [];

	/** @var array<string, string> $toolSettings tool settings */
	private array $toolSettings = [];

	/**
	 * Path to external lamdaemon script on server where it is executed
	 *
	 * This is used for managing quota and home directories.
	 * optional setting, may not be defined
	 */
	private string $scriptPath = '';

	/**
	 * The rights for the home directory
	 */
	private string $scriptRights = '750';

	/**
	 * Servers where lamdaemon script is executed
	 *
	 * This is used for managing quota and home directories.
	 * optional setting, may not be defined
	 */
	private string $scriptServer = '';

	/**
	 * user name for lamdaemon
	 */
	private string $scriptUserName = '';

	/**
	 * File name of SSH key for lamdaemon.
	 */
	private string $scriptSSHKey = '';

	/**
	 * Password for lamdaemon SSH key.
	 */
	private string $scriptSSHKeyPassword = '';

	/** LDAP search limit */
	private int $searchLimit = 0;

	/** Active account types */
	private string $activeTypes = "";

	/** Name of configuration file */
	private string $name;

	/** access level */
	private string $accessLevel = LAMConfig::ACCESS_ALL;

	/** login method */
	private string $loginMethod = LAMConfig::LOGIN_LIST;

	/** search suffix for login */
	private string $loginSearchSuffix = 'dc=yourdomain,dc=org';

	/** search filter for login */
	private string $loginSearchFilter = 'uid=%USER%';

	/** bind user for login search */
	private string $loginSearchDN = '';

	/** bind password for login search */
	private string $loginSearchPassword = '';

	/** specifies if HTTP authentication should be used */
	private string $httpAuthentication = 'false';

	/** email address for sender of password reset mails */
	private string $lamProMailFrom = '';
	/** reply-to email address for password reset mails */
	private string $lamProMailReplyTo = '';
	/** subject for password reset mails */
	private string $lamProMailSubject = '';
	/** treat password reset mail body as HTML */
	private string $lamProMailIsHTML = 'false';
	/** allow sending mails to an alternative address */
	private string $lamProMailAllowAlternateAddress = 'true';
	/** mail body for password reset mails */
	private string $lamProMailText = '';

	/** password reset page: allow to set a specific password */
	private string $pwdResetAllowSpecificPassword = 'true';
	/** password reset page: allow to show password on screen */
	private string $pwdResetAllowScreenPassword = 'true';
	/** password reset page: force password change by default */
	private string $pwdResetForcePasswordChange = 'true';
	/** password reset page: default selection for password output
	 * PWDRESET_DEFAULT_SCREEN, PWDRESET_DEFAULT_MAIL, PWDRESET_DEFAULT_BOTH */
	private string $pwdResetDefaultPasswordOutput = LAMConfig::PWDRESET_DEFAULT_MAIL;

	/** LDAP user for jobs */
	private ?string $jobsBindUser = '';
	/** LDAP password for jobs */
	private ?string $jobsBindPassword = '';
	/** database type for jobs */
	private ?string $jobsDatabase = '';
	/** host of job database */
	private ?string $jobsDBHost = '';
	/** port of job database */
	private ?string $jobsDBPort = '';
	/** CA path of job database */
	private ?string $jobsDBCaPath = '';
	/** user of job database */
	private ?string $jobsDBUser = '';
	/** password of job database */
	private ?string $jobsDBPassword = '';
	/** name of job database */
	private ?string $jobsDBName = '';
	/** @var array<string, string[]> $jobSettings job configuration */
	private array $jobSettings = [];

	private string $twoFactorAuthentication = TwoFactorProviderService::TWO_FACTOR_NONE;
	private string $twoFactorAuthenticationURL = 'https://localhost';
	private string $twoFactorAuthenticationClientId = '';
	private string $twoFactorAuthenticationSecretKey = '';
	private string $twoFactorAuthenticationDomain = '';
	private bool $twoFactorAuthenticationInsecure = false;
	private string $twoFactorAuthenticationLabel = '';
	private bool $twoFactorAuthenticationOptional = false;
	private string $twoFactorAuthenticationCaption = '';
	private string $twoFactorAuthenticationAttribute = '';
	private string $twoFactorAllowToRememberDevice = 'false';
	private string $twoFactorRememberDeviceDuration = '28800';
	private string $twoFactorRememberDevicePassword = '';

	private string $hideDnPart = '';

	private string $pwdPolicyMinLength = '';
	private string $pwdPolicyMinLowercase = '';
	private string $pwdPolicyMinUppercase = '';
	private string $pwdPolicyMinNumeric = '';
	private string $pwdPolicyMinSymbolic = '';

	/** @var string[] $settings List of all settings in config file */
	private array $settings = ["ServerURL", "useTLS", "followReferrals", 'pagedResults', 'adShowDeleted', "Passwd", "Admins",
		"defaultLanguage", "scriptPath", "scriptServer", "scriptRights", 'serverDisplayName',
		"modules", "activeTypes", "types", "tools", "accessLevel", 'loginMethod', 'loginSearchSuffix',
		'loginSearchFilter', 'searchLimit', 'lamProMailFrom', 'lamProMailReplyTo', 'lamProMailSubject',
		'lamProMailText', 'lamProMailIsHTML', 'lamProMailAllowAlternateAddress', 'httpAuthentication', 'loginSearchDN',
		'loginSearchPassword', 'timeZone', 'jobsBindUser', 'jobsBindPassword', 'jobsDatabase', 'jobs',
		'jobsDBHost', 'jobsDBPort', 'jobsDBCaPath', 'jobsDBUser', 'jobsDBPassword', 'jobsDBName', 'pwdResetAllowSpecificPassword',
		'pwdResetAllowScreenPassword', 'pwdResetForcePasswordChange', 'pwdResetDefaultPasswordOutput',
		'scriptUserName', 'scriptSSHKey', 'scriptSSHKeyPassword', 'twoFactorAuthentication', 'twoFactorAuthenticationURL',
		'twoFactorAuthenticationInsecure', 'twoFactorAuthenticationLabel', 'twoFactorAuthenticationOptional',
		'twoFactorAuthenticationCaption', 'twoFactorAuthenticationClientId', 'twoFactorAuthenticationSecretKey',
		'twoFactorAuthenticationDomain', 'twoFactorAuthenticationAttribute', 'twoFactorAllowToRememberDevice',
		'twoFactorRememberDeviceDuration', 'twoFactorRememberDevicePassword', 'referentialIntegrityOverlay',
		'hidePasswordPromptForExpiredPasswords', 'hideDnPart', 'pwdPolicyMinLength', 'pwdPolicyMinLowercase',
		'pwdPolicyMinUppercase', 'pwdPolicyMinNumeric', 'pwdPolicyMinSymbolic'
	];

	/**
	 * Returns the server profile data.
	 *
	 * @return array<string, string|int|bool|null|object|array<string, string|string[]>> data
	 */
	public function exportData(): array {
		$data = [];
		$settingsToIgnore = ['modules', 'types', 'tools', 'jobs'];
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
	 * @param array<string, string|int|bool|null|object|array<string, string|string[]>> $data config data
	 * @throws LAMException import error
	 */
	public function importData(array $data): void {
		$settingsToIgnore = ['modules', 'types', 'tools', 'jobs', 'typeSettings',
			'moduleSettings', 'toolSettings', 'jobSettings'];
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
			if ($dataValue !== null) {
				$this->$dataKey = $dataValue;
			}
			else {
				logNewMessage(LOG_DEBUG, 'Ignored null value for ' . $dataKey);
			}
		}
		$typeSettingsData = !empty($data['typeSettings']) && is_array($data['typeSettings']) ? $data['typeSettings'] : [];
		$this->typeSettings = $typeSettingsData;
		$moduleSettingsData = !empty($data['moduleSettings']) && is_array($data['moduleSettings']) ? $data['moduleSettings'] : [];
		$this->moduleSettings = $moduleSettingsData;
		$toolSettingsData = !empty($data['toolSettings']) && is_array($data['toolSettings']) ? $data['toolSettings'] : [];
		$this->toolSettings = $toolSettingsData;
		$jobSettingsData = !empty($data['jobSettings']) && is_array($data['jobSettings']) ? $data['jobSettings'] : [];
		$this->jobSettings = $jobSettingsData;
	}

	/**
	 * Removes any non-existing types from the configuration.
	 */
	public function removeInvalidTypes(): void {
		$allTypes = LAM\TYPES\getTypes();
		$activeTypes = $this->get_ActiveTypes();
		for ($i = 0; $i < count($activeTypes); $i++) {
			if (!in_array(getScopeFromTypeId($activeTypes[$i]), $allTypes)) {
				unset($activeTypes[$i]);
			}
		}
		$activeTypes = array_values($activeTypes);
		$this->set_ActiveTypes($activeTypes);
	}

	/**
	 * Removes any non-existing modules from the configuration.
	 */
	public function removeInvalidModules(): void {
		$types = $this->get_ActiveTypes();
		$availableByScope = [];
		foreach ($types as $type) {
			$scope = getScopeFromTypeId($type);
			$moduleVar = "modules_" . $type;
			if (isset($this->typeSettings[$moduleVar])) {
				$modules = explode(",", $this->typeSettings[$moduleVar]);
				if (empty($availableByScope[$scope])) {
					$availableByScope[$scope] = getAvailableModules($scope);
				}
				$available = $availableByScope[$scope];
				// only return available modules
				$ret = [];
				for ($i = 0; $i < count($modules); $i++) {
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
	public static function isValidName(string $name): bool {
		return preg_match("/^[a-z0-9_-]+$/i", $name);
	}

	/**
	 * Returns the name of the config file
	 *
	 * @return string name
	 */
	public function getName(): string {
		return $this->name;
	}

	/**
	 * Sets the name of the config file
	 *
	 * @param string $name name
	 */
	public function setName(string $name): void {
		$this->name = $name;
	}

	// functions to read/write preferences

	/**
	 * Returns the server address as string
	 *
	 * @return string server address
	 */
	public function get_ServerURL(): string {
		return $this->ServerURL;
	}

	/**
	 * Sets the server address
	 *
	 * @param string|null $value new server address
	 * @return bool true if $value has correct format
	 */
	public function set_ServerURL(?string $value): bool {
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
	public function getServerDisplayNameGUI(): string {
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
	public function getServerDisplayName(): string {
		return $this->serverDisplayName;
	}

	/**
	 * Sets the server display name
	 *
	 * @param string|null $value new server display name
	 * @return bool true if $value has correct format
	 */
	public function setServerDisplayName(?string $value): bool {
		if (is_string($value)) {
			$this->serverDisplayName = $value;
			return true;
		}
		return false;
	}

	/**
	 * Returns if TLS is activated.
	 *
	 * @return string yes or no
	 */
	public function getUseTLS(): string {
		return $this->useTLS;
	}

	/**
	 * Sets if TLS is activated.
	 *
	 * @param string $useTLS yes or no
	 * @return bool true if $useTLS has correct format
	 */
	public function setUseTLS(string $useTLS): bool {
		if (($useTLS === "yes") || ($useTLS === "no")) {
			$this->useTLS = $useTLS;
			return true;
		}
		return false;
	}

	/**
	 * Returns if referrals should be followed.
	 *
	 * @return string true or false
	 */
	public function getFollowReferrals(): string {
		return $this->followReferrals;
	}

	/**
	 * Sets if referrals should be followed.
	 *
	 * @param string $followReferrals true or false
	 */
	public function setFollowReferrals(string $followReferrals): void {
		$this->followReferrals = $followReferrals;
	}

	/**
	 * Returns if paged results should be used.
	 *
	 * @return string true or false
	 */
	public function getPagedResults(): string {
		return $this->pagedResults;
	}

	/**
	 * Sets if paged results should be used.
	 *
	 * @param string $pagedResults true or false
	 */
	public function setPagedResults(string $pagedResults): void {
		$this->pagedResults = $pagedResults;
	}

	/**
	 * Returns if deleted entries should be shown.
	 *
	 * @return string true or false
	 */
	public function getAdShowDeleted(): string {
		return $this->adShowDeleted;
	}

	/**
	 * Sets if deleted entries should be shown.
	 *
	 * @param string $adShowDeleted true or false
	 */
	public function setAdShowDeleted(string $adShowDeleted): void {
		$this->adShowDeleted = $adShowDeleted;
	}

	/**
	 * Returns if referential integrity overlay is in place.
	 *
	 * @return string true or false
	 */
	public function getReferentialIntegrityOverlay(): string {
		return $this->referentialIntegrityOverlay;
	}

	/**
	 * Sets if referential integrity overlay is in place.
	 *
	 * @param string $referentialIntegrityOverlay true or false
	 */
	public function setReferentialIntegrityOverlay(string $referentialIntegrityOverlay): void {
		$this->referentialIntegrityOverlay = $referentialIntegrityOverlay;
	}

	/**
	 * Returns if referential integrity overlay is in place.
	 *
	 * @return bool overlay in place
	 */
	public function isReferentialIntegrityOverlayActive(): bool {
		return $this->referentialIntegrityOverlay === 'true';
	}

	/**
	 * Hide password prompt for expired passwords.
	 *
	 * @return string true or false
	 */
	public function getHidePasswordPromptForExpiredPasswords(): string {
		return $this->hidePasswordPromptForExpiredPasswords;
	}

	/**
	 * Sets if password prompt for expired passwords is hidden.
	 *
	 * @param string $hidePasswordPromptForExpiredPasswords true or false
	 */
	public function setHidePasswordPromptForExpiredPasswords(string $hidePasswordPromptForExpiredPasswords): void {
		$this->hidePasswordPromptForExpiredPasswords = $hidePasswordPromptForExpiredPasswords;
	}

	/**
	 * Hide password prompt for expired passwords.
	 *
	 * @return bool is hidden
	 */
	public function isHidePasswordPromptForExpiredPasswords(): bool {
		return $this->hidePasswordPromptForExpiredPasswords === 'true';
	}

	/**
	 * Returns an array of string with all admin names
	 *
	 * @return string[] the admin names
	 */
	public function get_Admins(): array {
		return explode(";", $this->Admins);
	}

	/**
	 * Returns all admin users separated by semicolons
	 *
	 * @return string the admin string
	 */
	public function get_Adminstring(): string {
		return $this->Admins;
	}

	/**
	 * Sets the admin string
	 *
	 * @param string|null $value new admin string that contains all admin users separated by semicolons
	 * @return bool true if $value has correct format
	 */
	public function set_Adminstring(?string $value): bool {
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
	 * @param string $password password
	 * @return bool true, if matches
	 */
	public function check_Passwd(string $password): bool {
		if (str_starts_with($this->Passwd, "{SSHA}")) {
			$value = substr($this->Passwd, strlen("{SSHA}"));
			$parts = explode(" ", $value);
			$salt = base64_decode($parts[1]);
			$hash = "{SSHA}" . base64_encode(hex2bin(sha1($password . $salt))) . " " . base64_encode($salt);
			return ($hash === $this->Passwd);
		}
		elseif (str_starts_with($this->Passwd, "{CRYPT-SHA512}")) {
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
	 * @param string|null $value new password
	 * @return bool true if $value has correct format
	 */
	public function set_Passwd(?string $value): bool {
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
	public function getPasswd(): string {
		return $this->Passwd;
	}

	/**
	 * Returns the hashed password.
	 *
	 * @param string $password password
	 * @param string $salt salt
	 * @return string hash value
	 */
	private function hashPassword(string $password, string $salt): string {
		return "{CRYPT-SHA512}" . crypt($password, '$6$' . $salt) . " " . base64_encode($salt);
	}

	/**
	 * Returns if the server profile has a password set.
	 *
	 * @return bool password is set
	 */
	public function hasPasswordSet(): bool {
		return ($this->Passwd !== null) && ($this->Passwd !== '');
	}

	/**
	 * Returns the LDAP suffix for the given account type
	 *
	 * @param string $typeId account type
	 * @return string the LDAP suffix
	 */
	public function get_Suffix(string $typeId): string {
		return $this->typeSettings['suffix_' . $typeId] ?? '';
	}

	/**
	 * Sets the LDAP suffix where accounts are saved
	 *
	 * @param string $typeId account type ID
	 * @param string|null $value new LDAP suffix
	 * @return bool true if $value has correct format
	 */
	public function set_Suffix(string $typeId, ?string $value): bool {
		if ($value === null) {
			$value = "";
		}
		$this->typeSettings['suffix_' . $typeId] = $value;
		return true;
	}

	/**
	 * Returns the list of attributes to show in given list.
	 *
	 * @param string $typeId account type ID
	 * @return string the attribute list
	 */
	public function get_listAttributes(string $typeId): string {
		return $this->typeSettings['attr_' . $typeId] ?? '';
	}

	/**
	 * Sets the list of attributes to show in user list
	 *
	 * @param string|null $value new attribute string
	 * @param string $typeId account type ID
	 * @return bool true if $value has correct format
	 */
	public function set_listAttributes(?string $value, string $typeId): bool {
		if (is_string($value) && preg_match("/^((#[^:;]+)|([^:;]*:[^:;]+))(;((#[^:;]+)|([^:;]*:[^:;]+)))*$/", $value)) {
			$this->typeSettings['attr_' . $typeId] = $value;
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
	public function get_defaultLanguage(): string {
		return $this->defaultLanguage;
	}

	/**
	 * Sets the default language string
	 *
	 * @param string|null $value new default language
	 * @return bool true if $value has correct format
	 */
	public function set_defaultLanguage(?string $value): bool {
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
	public function getTimeZone(): string {
		return ($this->timeZone == null) ? 'Europe/London' : $this->timeZone;
	}

	/**
	 * Sets the time zone name.
	 *
	 * @param string|null $value new time zone
	 * @return bool true if $value has correct format
	 */
	public function setTimeZone(?string $value): bool {
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
	public function get_scriptPath(): string {
		return $this->scriptPath;
	}

	/**
	 * Sets the path to the external script
	 *
	 * @param string|null $value new script path
	 * @return bool true if $value has correct format
	 */
	public function set_scriptPath(?string $value): bool {
		if (empty($value)) {
			$this->scriptPath = ""; // optional parameter
		}
		elseif (preg_match("/^\\/([a-z0-9_-])+(\\/([a-z0-9_\\.-])+)+$/i", $value)) {
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
	public function get_scriptServers(): string {
		return $this->scriptServer;
	}

	/**
	 * Returns the list of configured servers.
	 *
	 * @return RemoteServerConfiguration[] servers
	 */
	public function getConfiguredScriptServers(): array {
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
	public static function parseConfiguredScriptServers(string $value, string $scriptPath, string $userName, string $sshKey, string $sshKeyPassword): array {
		$servers = [];
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
			$serverLabel = $serverSettings[1] ?? '';
			$serverHomedirPrefix = $serverSettings[2] ?? '';
			$servers[] = new RemoteServerConfiguration($serverName, $serverLabel, $serverHomedirPrefix, $scriptPath, $userName, $sshKey, $sshKeyPassword);
		}
		return $servers;
	}

	/**
	 * Returns the script server object for the given name.
	 *
	 * @param string $serverName server name
	 * @return RemoteServerConfiguration|null server
	 */
	public function getScriptServerByName(string $serverName): ?RemoteServerConfiguration {
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
	 * @return bool true if $value has correct format
	 */
	public function set_scriptServers(string $value): bool {
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
	 * @param string $value input field value
	 * @return string|false value to store in configuration
	 */
	public static function parseScriptServerValue(string $value): false|string {
		if (empty($value)) {
			return ""; // optional parameter
		}
		// Explode the value to an array
		$array_string = explode(";", $value);
		// Check all IPs in the exploded Array
		$valid_ips = [];
		foreach ($array_string as $arr_value) {
			// Explode name and IP, if a name exists
			if (str_contains($arr_value, ":")) {
				$arr_value_explode = explode(":", $arr_value);
				$servername = $arr_value_explode[0];
				$label = $arr_value_explode[1];
				$homedirPrefix = $arr_value_explode[2] ?? '';
			}
			else {
				$servername = $arr_value;
				$label = '';
				$homedirPrefix = '';
			}
			if (preg_match("/^[a-z0-9-]+(\\.[a-z0-9-]+)*(,[0-9]+)?$/i", $servername)) {
				$serverData = [$servername];
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
		if ($valid_ips !== []) {
			return implode(";", $valid_ips);
		}
		else {
			// The array is empty, there was no valid IP
			return false;
		}
	}

	/**
	 * Returns the chmod value for new home directories.
	 *
	 * @return string rights
	 */
	public function get_scriptRights() {
		if (empty($this->scriptRights)) {
			return '750';
		}
		return $this->scriptRights;
	}

	/**
	 * Sets the rights for the home directory.
	 *
	 * @param string $chmod the rights
	 * @return bool true if values has correct format
	 */
	public function set_scriptRights(string $chmod): bool {
		// check if the chmod is correct:
		if ($chmod > 0 && $chmod <= 777) {
			$this->scriptRights = $chmod;
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
	public function getScriptSSHKey(): string {
		return $this->scriptSSHKey;
	}

	/**
	 * Sets the path to lamdamon SSH key.
	 *
	 * @param string $value key path
	 */
	public function setScriptSSHKey(string $value): void {
		$this->scriptSSHKey = $value;
	}

	/**
	 * Returns the password for the lamdamon SSH key.
	 *
	 * @return string password
	 */
	public function getScriptSSHKeyPassword(): string {
		return deobfuscateText($this->scriptSSHKeyPassword);
	}

	/**
	 * Sets the password for the lamdamon SSH key.
	 *
	 * @param string $value password
	 */
	public function setScriptSSHKeyPassword(string $value): void {
		$this->scriptSSHKeyPassword = obfuscateText($value);
	}

	/**
	 * Returns the lamdaemon user name.
	 *
	 * @return string user name
	 */
	public function getScriptUserName(): string {
		return $this->scriptUserName;
	}

	/**
	 * Sets the lamdaemon user name.
	 *
	 * @param string $value user name
	 */
	public function setScriptUserName(string $value): void {
		$this->scriptUserName = $value;
	}

	/**
	 * Returns the LDAP search limit.
	 *
	 * @return int search limit
	 */
	public function get_searchLimit(): int {
		return $this->searchLimit;
	}

	/**
	 * Sets the LDAP search limit.
	 *
	 * @param int $value new search limit
	 * @return bool true if $value has correct format
	 */
	public function set_searchLimit(int $value): bool {
		if ($value > -1) {
			$this->searchLimit = $value;
			return true;
		}
		return false;
	}

	/**
	 * Returns an array of all selected account modules
	 *
	 * @param string $scope account type
	 * @return class-string<baseModule>[] user modules
	 */
	public function get_AccountModules($scope) {
		if (isset($this->typeSettings["modules_" . $scope])) {
			$modulesTmp = explode(",", $this->typeSettings["modules_" . $scope]);
			$modules = [];
			foreach ($modulesTmp as $mod) {
				if (trim($mod) !== '') {
					$modules[] = $mod;
				}
			}
			return $modules;
		}
		else {
			return [];
		}
	}

	/**
	 * Sets the settings for the account modules.
	 *
	 * @param array<string, string[]> $settings list of module setting [name => value]
	 */
	public function set_moduleSettings(array $settings): void {
		$this->moduleSettings = $settings;
	}

	/**
	 * Returns a list of saved module settings
	 *
	 * @return array<string, string[]> list of settings: [name => value]
	 */
	public function get_moduleSettings(): array {
		return $this->moduleSettings;
	}

	/**
	 * Returns a list of active account types.
	 *
	 * @return string[] list of types
	 */
	public function get_ActiveTypes(): array {
		if (empty($this->activeTypes)) {
			return [];
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
	public function getActiveTypesString(): string {
		return $this->activeTypes;
	}

	/**
	 * Sets the list of active types.
	 *
	 * @param string[] $types list of types
	 */
	public function set_ActiveTypes(array $types): void {
		$this->activeTypes = implode(",", $types);
	}

	/**
	 * Sets the settings for the account types.
	 *
	 * @param array<string, string> $settings list of type setting [name => value]
	 */
	public function set_typeSettings(array $settings): void {
		$this->typeSettings = $settings;
	}

	/**
	 * Returns a list of saved type settings
	 *
	 * @return array<string, string> list of settings: [name => value]
	 */
	public function get_typeSettings(): array {
		return $this->typeSettings;
	}

	/**
	 * Returns the tool settings.
	 *
	 * @return array<string, string> $toolSettings tool settings
	 */
	public function getToolSettings(): array {
		return $this->toolSettings;
	}

	/**
	 * Sets the tool settings.
	 *
	 * @param array<string,string> $toolSettings tool settings
	 */
	public function setToolSettings(array $toolSettings): void {
		$this->toolSettings = $toolSettings;
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
	 * @return string level
	 */
	public function getAccessLevel(): string {
		return $this->accessLevel;
	}

	/**
	 * Sets the access level for this profile.
	 *
	 * @param string $level level
	 */
	public function setAccessLevel(string $level): void {
		$this->accessLevel = $level;
	}

	/**
	 * Returns the login method.
	 *
	 * @return string login method
	 * @see LAMconfig::LOGIN_LIST
	 * @see LAMconfig::LOGIN_SEARCH
	 */
	public function getLoginMethod(): string {
		return $this->loginMethod;
	}

	/**
	 * Sets the login method.
	 *
	 * @param string $loginMethod login method
	 */
	public function setLoginMethod(string $loginMethod): void {
		$this->loginMethod = $loginMethod;
	}

	/**
	 * Returns the login search filter.
	 *
	 * @return string search filter
	 */
	public function getLoginSearchFilter(): string {
		return $this->loginSearchFilter;
	}

	/**
	 * Sets the login search filter.
	 *
	 * @param string $loginSearchFilter search filter
	 */
	public function setLoginSearchFilter(string $loginSearchFilter): void {
		$this->loginSearchFilter = $loginSearchFilter;
	}

	/**
	 * Returns the login search suffix.
	 *
	 * @return string suffix
	 */
	public function getLoginSearchSuffix(): string {
		return $this->loginSearchSuffix;
	}

	/**
	 * Sets the login search suffix.
	 *
	 * @param string $loginSearchSuffix suffix
	 */
	public function setLoginSearchSuffix(string $loginSearchSuffix): void {
		$this->loginSearchSuffix = $loginSearchSuffix;
	}

	/**
	 * Sets the DN for the login search bind user.
	 *
	 * @param string $loginSearchDN DN
	 * @return bool true if DN is valid
	 */
	public function setLoginSearchDN(string $loginSearchDN): bool {
		$this->loginSearchDN = $loginSearchDN;
		return ($loginSearchDN === '') || get_preg($loginSearchDN, 'dn');
	}

	/**
	 * Returns the DN for the login search bind user.
	 *
	 * @return string DN
	 */
	public function getLoginSearchDN(): string {
		return $this->loginSearchDN;
	}

	/**
	 * Sets the password for the login search bind user.
	 *
	 * @param string $loginSearchPassword password
	 */
	public function setLoginSearchPassword(string $loginSearchPassword): void {
		$this->loginSearchPassword = obfuscateText($loginSearchPassword);
	}

	/**
	 * Returns the password for the login search bind user.
	 *
	 * @return string password
	 */
	public function getLoginSearchPassword(): string {
		return deobfuscateText($this->loginSearchPassword);
	}

	/**
	 * Returns if HTTP authentication should be used.
	 *
	 * @return string $httpAuthentication use HTTP authentication ('true' or 'false')
	 */
	public function getHttpAuthentication(): string {
		return $this->httpAuthentication;
	}

	/**
	 * Specifies if HTTP authentication should be used.
	 *
	 * @param string $httpAuthentication use HTTP authentication ('true' or 'false')
	 */
	public function setHttpAuthentication(string $httpAuthentication): void {
		$this->httpAuthentication = $httpAuthentication;
	}

	/**
	 * Returns the sender address for password reset mails.
	 *
	 * @return string mail address
	 */
	public function getLamProMailFrom(): string {
		if (empty($this->lamProMailFrom)) {
			return 'noreply@example.com';
		}
		return $this->lamProMailFrom;
	}

	/**
	 * Sets the sender address for password reset mails.
	 *
	 * @param string $lamProMailFrom mail address
	 * @return bool true if address is valid
	 */
	public function setLamProMailFrom(string $lamProMailFrom): bool {
		$this->lamProMailFrom = $lamProMailFrom;
		if (($lamProMailFrom !== '') && !get_preg($lamProMailFrom, 'email') && !get_preg($lamProMailFrom, 'emailWithName')) {
			return false;
		}
		return true;
	}

	/**
	 * Returns the reply-to address for password reset mails.
	 *
	 * @return string mail address
	 */
	public function getLamProMailReplyTo(): string {
		return $this->lamProMailReplyTo;
	}

	/**
	 * Sets the reply-to address for password reset mails.
	 *
	 * @param string $lamProMailReplyTo mail address
	 * @return bool true if address is valid
	 */
	public function setLamProMailReplyTo(string $lamProMailReplyTo): bool {
		$this->lamProMailReplyTo = $lamProMailReplyTo;
		if (($lamProMailReplyTo !== '') && !get_preg($lamProMailReplyTo, 'email') && !get_preg($lamProMailReplyTo, 'emailWithName')) {
			return false;
		}
		return true;
	}

	/**
	 * Returns the subject for password reset mails.
	 *
	 * @return string subject
	 */
	public function getLamProMailSubject(): string {
		return $this->lamProMailSubject;
	}

	/**
	 * Sets the subject for password reset mails.
	 *
	 * @param string $lamProMailSubject subject
	 */
	public function setLamProMailSubject(string $lamProMailSubject): void {
		$this->lamProMailSubject = $lamProMailSubject;
	}

	/**
	 * Returns if the password reset mail content should be treated as HTML.
	 *
	 * @return string HTML or text
	 */
	public function getLamProMailIsHTML(): string {
		return $this->lamProMailIsHTML;
	}

	/**
	 * Sets if the password reset mail content should be treated as HTML.
	 *
	 * @param string $lamProMailIsHTML true if HTML
	 */
	public function setLamProMailIsHTML(string $lamProMailIsHTML): void {
		$this->lamProMailIsHTML = $lamProMailIsHTML;
	}

	/**
	 * Returns if sending to an alternate address is allowed.
	 *
	 * @return string alternate address allowed
	 */
	public function getLamProMailAllowAlternateAddress(): string {
		return $this->lamProMailAllowAlternateAddress;
	}

	/**
	 * Sets if sending to an alternate address is allowed.
	 *
	 * @param string $lamProMailAllowAlternateAddress alternate address allowed
	 */
	public function setLamProMailAllowAlternateAddress(string $lamProMailAllowAlternateAddress): void {
		$this->lamProMailAllowAlternateAddress = $lamProMailAllowAlternateAddress;
	}

	/**
	 * Returns the mail body for password reset mails.
	 *
	 * @return string body
	 */
	public function getLamProMailText(): string {
		return implode("\r\n", explode(LAMConfig::LINE_SEPARATOR, $this->lamProMailText));
	}

	/**
	 * Returns the mail body for password reset mails without formatting.
	 *
	 * @return string body
	 */
	public function getLamProMailTextRaw(): string {
		return $this->lamProMailText;
	}

	/**
	 * Sets the mail body for password reset mails.
	 *
	 * @param string $lamProMailText body
	 */
	public function setLamProMailText(string $lamProMailText): void {
		$this->lamProMailText = implode(LAMConfig::LINE_SEPARATOR, explode("\r\n", $lamProMailText));
	}

	/**
	 * Returns the bind user for jobs.
	 *
	 * @return string $jobsBindUser bind user
	 */
	public function getJobsBindUser(): string {
		return $this->jobsBindUser ?? '';
	}

	/**
	 * Sets the bind user for jobs.
	 *
	 * @param string $jobsBindUser bind user
	 */
	public function setJobsBindUser(string $jobsBindUser): void {
		$this->jobsBindUser = $jobsBindUser;
	}

	/**
	 * Returns the bind password for jobs.
	 *
	 * @return string $jobsBindPassword password
	 */
	public function getJobsBindPassword(): string {
		return $this->jobsBindPassword ?? '';
	}

	/**
	 * Sets the bind password for jobs.
	 *
	 * @param string $jobsBindPassword password
	 */
	public function setJobsBindPassword(string $jobsBindPassword): void {
		$this->jobsBindPassword = $jobsBindPassword;
	}

	/**
	 * Returns the database type for jobs.
	 *
	 * @return string $jobsDatabase database type
	 */
	public function getJobsDatabase(): string {
		if (empty($this->jobsDatabase)) {
			return 'SQLite';
		}
		return $this->jobsDatabase;
	}

	/**
	 * Sets the database type for jobs.
	 *
	 * @param string $jobsDatabase database type
	 */
	public function setJobsDatabase(string $jobsDatabase): void {
		$this->jobsDatabase = $jobsDatabase;
	}

	/**
	 * Returns the host.
	 *
	 * @return string host
	 */
	public function getJobsDBHost(): string {
		return $this->jobsDBHost ?? '';
	}

	/**
	 * Sets the host.
	 *
	 * @param string $jobsDBHost host
	 */
	public function setJobsDBHost(string $jobsDBHost): void {
		$this->jobsDBHost = $jobsDBHost;
	}

	/**
	 * Returns the port.
	 *
	 * @return string port
	 */
	public function getJobsDBPort(): string {
		return $this->jobsDBPort ?? '';
	}

	/**
	 * Sets the port.
	 *
	 * @param string $jobsDBPort port
	 */
	public function setJobsDBPort(string $jobsDBPort): void {
		$this->jobsDBPort = $jobsDBPort;
	}

	/**
	 * Returns the CA certificate path.
	 *
	 * @return string path
	 */
	public function getJobsDBCaPath(): string {
		return $this->jobsDBCaPath ?? '';
	}

	/**
	 * Sets the CA certificate path.
	 *
	 * @param string $jobsDBCaPath path
	 */
	public function setJobsDBCaPath(string $jobsDBCaPath): void {
		$this->jobsDBCaPath = $jobsDBCaPath;
	}

	/**
	 * Returns the DB user.
	 *
	 * @return string user name
	 */
	public function getJobsDBUser(): string {
		return $this->jobsDBUser ?? '';
	}

	/**
	 * Sets the DB user.
	 *
	 * @param string $jobsDBUser user name
	 */
	public function setJobsDBUser(string $jobsDBUser): void {
		$this->jobsDBUser = $jobsDBUser;
	}

	/**
	 * Returns the DB password.
	 *
	 * @return string password
	 */
	public function getJobsDBPassword(): string {
		return $this->jobsDBPassword ?? '';
	}

	/**
	 * Sets the DB password.
	 *
	 * @param string $jobsDBPassword password
	 */
	public function setJobsDBPassword(string $jobsDBPassword): void {
		$this->jobsDBPassword = $jobsDBPassword;
	}

	/**
	 * Returns the database name.
	 *
	 * @return string DB name
	 */
	public function getJobsDBName(): string {
		return $this->jobsDBName ?? '';
	}

	/**
	 * Sets the database name
	 *
	 * @param string $jobsDBName DB name
	 */
	public function setJobsDBName(string $jobsDBName): void {
		$this->jobsDBName = $jobsDBName;
	}

	/**
	 * Sets the settings for the jobs.
	 *
	 * @param array<string, string[]> $settings list of job settings [name => value]
	 */
	public function setJobSettings(array $settings): void {
		$this->jobSettings = $settings;
	}

	/**
	 * Returns a list of saved job settings.
	 *
	 * @return array<string, string[]> list of settings: [name => value]
	 */
	public function getJobSettings(): array {
		return $this->jobSettings;
	}

	/**
	 * Returns if setting a specific password is allowed on password reset page.
	 *
	 * @return string 'true' or 'false'
	 */
	public function getPwdResetAllowSpecificPassword(): string {
		return $this->pwdResetAllowSpecificPassword;
	}

	/**
	 * Sets if setting a specific password is allowed on password reset page.
	 *
	 * @param string $pwdResetAllowSpecificPassword 'true' or 'false'
	 */
	public function setPwdResetAllowSpecificPassword(string $pwdResetAllowSpecificPassword): void {
		$this->pwdResetAllowSpecificPassword = $pwdResetAllowSpecificPassword;
	}

	/**
	 * Returns if displaying password on screen is allowed on password reset page.
	 *
	 * @return string 'true' or 'false'
	 */
	public function getPwdResetAllowScreenPassword(): string {
		return $this->pwdResetAllowScreenPassword;
	}

	/**
	 * Sets if displaying password on screen is allowed on password reset page.
	 *
	 * @param string $pwdResetAllowScreenPassword 'true' or 'false'
	 */
	public function setPwdResetAllowScreenPassword(string $pwdResetAllowScreenPassword): void {
		$this->pwdResetAllowScreenPassword = $pwdResetAllowScreenPassword;
	}

	/**
	 * Returns if force password change is set by default on password reset page.
	 *
	 * @return string 'true' or 'false'
	 */
	public function getPwdResetForcePasswordChange(): string {
		return $this->pwdResetForcePasswordChange;
	}

	/**
	 * Sets if force password change is set by default on password reset page.
	 *
	 * @param string $pwdResetForcePasswordChange 'true' or 'false'
	 */
	public function setPwdResetForcePasswordChange(string $pwdResetForcePasswordChange): void {
		$this->pwdResetForcePasswordChange = $pwdResetForcePasswordChange;
	}

	/**
	 * Returns default password output method on password reset page.
	 *
	 * @return string LAMConfig::PWDRESET_DEFAULT_SCREEN/PWDRESET_DEFAULT_MAIL/PWDRESET_DEFAULT_BOTH
	 */
	public function getPwdResetDefaultPasswordOutput(): string {
		return $this->pwdResetDefaultPasswordOutput;
	}

	/**
	 * Sets default password output method on password reset page.
	 *
	 * @param string $pwdResetDefaultPasswordOutput LAMConfig::PWDRESET_DEFAULT_SCREEN/PWDRESET_DEFAULT_MAIL/PWDRESET_DEFAULT_BOTH
	 */
	public function setPwdResetDefaultPasswordOutput(string $pwdResetDefaultPasswordOutput): void {
		$this->pwdResetDefaultPasswordOutput = $pwdResetDefaultPasswordOutput;
	}

	/**
	 * Returns the authentication type.
	 *
	 * @return string $twoFactorAuthentication authentication type
	 */
	public function getTwoFactorAuthentication(): string {
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
	public function setTwoFactorAuthentication(string $twoFactorAuthentication): void {
		$this->twoFactorAuthentication = $twoFactorAuthentication;
	}

	/**
	 * Returns the authentication URL.
	 *
	 * @return string authentication URL
	 */
	public function getTwoFactorAuthenticationURL(): string {
		return implode("\r\n", explode(LAMConfig::LINE_SEPARATOR, $this->twoFactorAuthenticationURL));
	}

	/**
	 * Returns the authentication URL without formatting.
	 *
	 * @return string authentication URL
	 */
	public function getTwoFactorAuthenticationURLRaw(): string {
		return $this->twoFactorAuthenticationURL;
	}

	/**
	 * Sets the authentication URL.
	 *
	 * @param string $twoFactorAuthenticationURL authentication URL
	 */
	public function setTwoFactorAuthenticationURL(string $twoFactorAuthenticationURL): void {
		$this->twoFactorAuthenticationURL = implode(LAMConfig::LINE_SEPARATOR, explode("\r\n", $twoFactorAuthenticationURL));
	}

	/**
	 * Sets the client id.
	 *
	 * @param string $clientId client id
	 */
	public function setTwoFactorAuthenticationClientId(string $clientId): void {
		$this->twoFactorAuthenticationClientId = $clientId;
	}

	/**
	 * Returns the client id.
	 *
	 * @return string client id
	 */
	public function getTwoFactorAuthenticationClientId(): string {
		return $this->twoFactorAuthenticationClientId;
	}

	/**
	 * Sets the secret key.
	 *
	 * @param string $secretKey secret key
	 */
	public function setTwoFactorAuthenticationSecretKey(string $secretKey): void {
		$this->twoFactorAuthenticationSecretKey = $secretKey;
	}

	/**
	 * Returns the secret key.
	 *
	 * @return string secret key
	 */
	public function getTwoFactorAuthenticationSecretKey(): string {
		return $this->twoFactorAuthenticationSecretKey;
	}

	/**
	 * Sets the domain.
	 *
	 * @param string $domain domain
	 */
	public function setTwoFactorAuthenticationDomain(string $domain): void {
		$this->twoFactorAuthenticationDomain = $domain;
	}

	/**
	 * Returns the domain.
	 *
	 * @return string domain
	 */
	public function getTwoFactorAuthenticationDomain(): string {
		return $this->twoFactorAuthenticationDomain;
	}

	/**
	 * Returns if SSL certificate verification is turned off.
	 *
	 * @return bool SSL certificate verification is turned off
	 */
	public function getTwoFactorAuthenticationInsecure(): bool {
		return $this->twoFactorAuthenticationInsecure;
	}

	/**
	 * Sets if SSL certificate verification is turned off.
	 *
	 * @param bool $twoFactorAuthenticationInsecure SSL certificate verification is turned off
	 */
	public function setTwoFactorAuthenticationInsecure(bool $twoFactorAuthenticationInsecure): void {
		$this->twoFactorAuthenticationInsecure = $twoFactorAuthenticationInsecure;
	}

	/**
	 * Returns the authentication label.
	 *
	 * @return string authentication label
	 */
	public function getTwoFactorAuthenticationLabel(): string {
		return $this->twoFactorAuthenticationLabel;
	}

	/**
	 * Sets the authentication label.
	 *
	 * @param string $twoFactorAuthenticationLabel authentication label
	 */
	public function setTwoFactorAuthenticationLabel(string $twoFactorAuthenticationLabel): void {
		$this->twoFactorAuthenticationLabel = $twoFactorAuthenticationLabel;
	}

	/**
	 * Returns if 2nd factor is optional.
	 *
	 * @return bool 2nd factor is optional
	 */
	public function getTwoFactorAuthenticationOptional(): bool {
		return $this->twoFactorAuthenticationOptional;
	}

	/**
	 * Sets if 2nd factor is optional.
	 *
	 * @param bool $twoFactorAuthenticationOptional 2nd factor is optional
	 */
	public function setTwoFactorAuthenticationOptional(bool $twoFactorAuthenticationOptional): void {
		$this->twoFactorAuthenticationOptional = $twoFactorAuthenticationOptional;
	}

	/**
	 * Returns the caption HTML.
	 *
	 * @return string caption HTML
	 */
	public function getTwoFactorAuthenticationCaption(): string {
		return $this->twoFactorAuthenticationCaption;
	}

	/**
	 * Sets the caption HTML.
	 *
	 * @param string $twoFactorAuthenticationCaption caption HTML
	 */
	public function setTwoFactorAuthenticationCaption(string $twoFactorAuthenticationCaption): void {
		$this->twoFactorAuthenticationCaption = $twoFactorAuthenticationCaption;
	}

	/**
	 * Returns the user attribute.
	 *
	 * @return string user attribute
	 */
	public function getTwoFactorAuthenticationAttribute(): string {
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
	public function setTwoFactorAuthenticationAttribute(string $twoFactorAuthenticationAttribute): void {
		$this->twoFactorAuthenticationAttribute = $twoFactorAuthenticationAttribute;
	}

	/**
	 * Returns if remembering devices is allowed.
	 *
	 * @return string remembering allowed
	 */
	public function getTwoFactorAllowToRememberDevice(): string {
		if (empty($this->twoFactorAllowToRememberDevice)) {
			return 'false';
		}
		return $this->twoFactorAllowToRememberDevice;
	}

	/**
	 * Sets the user attribute.
	 *
	 * @param string $twoFactorAllowToRememberDevice remembering allowed
	 */
	public function setTwoFactorAllowToRememberDevice(string $twoFactorAllowToRememberDevice): void {
		$this->twoFactorAllowToRememberDevice = $twoFactorAllowToRememberDevice;
	}

	/**
	 * Returns the duration for device remembering in hours.
	 *
	 * @return string duration
	 */
	public function getTwoFactorRememberDeviceDuration(): string {
		if (empty($this->twoFactorRememberDeviceDuration)) {
			return strval(8 * 60 * 60);
		}
		return $this->twoFactorRememberDeviceDuration;
	}

	/**
	 * Sets the duration for device remembering in hours.
	 *
	 * @param string $twoFactorRememberDeviceDuration duration in hours
	 */
	public function setTwoFactorRememberDeviceDuration(string $twoFactorRememberDeviceDuration): void {
		$this->twoFactorRememberDeviceDuration = $twoFactorRememberDeviceDuration;
	}

	/**
	 * Returns the password for device remembering.
	 *
	 * @return string password
	 */
	public function getTwoFactorRememberDevicePassword(): string {
		if (empty($this->twoFactorRememberDevicePassword)) {
			return generateRandomPassword(30);
		}
		return $this->twoFactorRememberDevicePassword;
	}

	/**
	 * Sets the password for device remembering.
	 *
	 * @param string $twoFactorRememberDevicePassword password
	 */
	public function setTwoFactorRememberDevicePassword(string $twoFactorRememberDevicePassword): void {
		$this->twoFactorRememberDevicePassword = $twoFactorRememberDevicePassword;
	}

	/**
	 * Returns the DN part to hide.
	 *
	 * @return string DN part
	 */
	public function getHideDnPart(): string {
		return $this->hideDnPart;
	}

	/**
	 * Sets the DN part to hide.
	 *
	 * @param string $hideDnPart DN part
	 */
	public function setHideDnPart(string $hideDnPart): void {
		$this->hideDnPart = $hideDnPart;
	}

	/**
	 * Returns the minimum password length.
	 *
	 * @return string minimum length
	 */
	public function getPwdPolicyMinLength(): string {
		return $this->pwdPolicyMinLength;
	}

	/**
	 * Sets the minimum password length.
	 *
	 * @param string $pwdPolicyMinLength minimum length
	 */
	public function setPwdPolicyMinLength(string $pwdPolicyMinLength): void {
		$this->pwdPolicyMinLength = $pwdPolicyMinLength;
	}

	/**
	 * Returns the minimum of lowercase characters.
	 *
	 * @return string minimum
	 */
	public function getpwdPolicyMinLowercase(): string {
		return $this->pwdPolicyMinLowercase;
	}

	/**
	 * Sets the minimum of lowercase characters.
	 *
	 * @param string $pwdPolicyMinLowercase minimum
	 */
	public function setPwdPolicyMinLowercase(string $pwdPolicyMinLowercase): void {
		$this->pwdPolicyMinLowercase = $pwdPolicyMinLowercase;
	}

	/**
	 * Returns the minimum of uppercase characters.
	 *
	 * @return string minimum
	 */
	public function getPwdPolicyMinUppercase(): string {
		return $this->pwdPolicyMinUppercase;
	}

	/**
	 * Sets the minimum of uppercase characters.
	 *
	 * @param string $pwdPolicyMinUppercase minimum
	 */
	public function setPwdPolicyMinUppercase(string $pwdPolicyMinUppercase): void {
		$this->pwdPolicyMinUppercase = $pwdPolicyMinUppercase;
	}

	/**
	 * Returns the minimum of numeric characters.
	 *
	 * @return string minimum
	 */
	public function getPwdPolicyMinNumeric(): string {
		return $this->pwdPolicyMinNumeric;
	}

	/**
	 * Sets the minimum of numeric characters.
	 *
	 * @param string $pwdPolicyMinNumeric minimum
	 */
	public function setPwdPolicyMinNumeric(string $pwdPolicyMinNumeric): void {
		$this->pwdPolicyMinNumeric = $pwdPolicyMinNumeric;
	}

	/**
	 * Returns the minimum of symbolic characters.
	 *
	 * @return string minimum
	 */
	public function getPwdPolicyMinSymbolic(): string {
		return $this->pwdPolicyMinSymbolic;
	}

	/**
	 * Sets the minimum of symbolic characters.
	 *
	 * @param string $pwdPolicyMinSymbolic minimum
	 */
	public function setPwdPolicyMinSymbolic(string $pwdPolicyMinSymbolic): void {
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
	public const ERROR_REPORTING_DEFAULT = 'default';
	/** PHP error reporting setting from php.ini */
	public const ERROR_REPORTING_SYSTEM = 'system';
	/** PHP error reporting setting as E_ALL */
	public const ERROR_REPORTING_ALL = 'all';

	/** send license warnings via email */
	public const LICENSE_WARNING_EMAIL = 'email';
	/** display license warnings on screen */
	public const LICENSE_WARNING_SCREEN = 'screen';
	/** send license warnings via email + display on screen */
	public const LICENSE_WARNING_ALL = 'all';
	/** no license warning */
	public const LICENSE_WARNING_NONE = 'none';

	/** SSL encryption for mail sending */
	public const SMTP_SSL = 'SSL';
	/** TLS encryption for mail sending */
	public const SMTP_TLS = 'TLS';
	/** No encryption for mail sending */
	public const SMTP_NONE = 'NONE';

	/** store configuration on local file system */
	public const DATABASE_FILE_SYSTEM = 'files';
	/** store configuration in MySQL database */
	public const DATABASE_MYSQL = 'mysql';

	public const MAIL_ATTRIBUTE_DEFAULT = 'mail';
	public const MAIL_BACKUP_ATTRIBUTE_DEFAULT = 'passwordselfresetbackupmail';

	public const SMS_ATTRIBUTES_DEFAULT = 'mobileTelephoneNumber;mobile';

	/** Default profile */
	public ?string $default = null;

	/** Password to change config.cfg */
	private ?string $password = null;

	/** Time of inactivity before session times out (minutes) */
	public int $sessionTimeout = 30;

	/** @var string hides detail messages for login errors */
	public string $hideLoginErrorDetails = 'false';

	/** log level */
	public int $logLevel = LOG_NOTICE;

	/** log destination ("SYSLOG":syslog, "/...":file, "NONE":none, "REMOTE":server:port) */
	public string $logDestination = "SYSLOG";

	/** type of audit log */
	public string $auditLogType = 'NONE';

	/** target of audit log */
	public string $auditLogTarget = '';

	/** list of hosts which may access LAM */
	public string $allowedHosts = "";

	/** list of hosts which may access LAM Pro self-service */
	public string $allowedHostsSelfService = '';

	/** minimum length for passwords */
	public int $passwordMinLength = 0;

	/** minimum uppercase characters */
	public int $passwordMinUpper = 0;

	/** minimum lowercase characters */
	public int $passwordMinLower = 0;

	/** minimum numeric characters */
	public int $passwordMinNumeric = 0;

	/** minimum symbol characters */
	public int $passwordMinSymbol = 0;

	/** minimum character classes (upper, lower, numeric, symbols) */
	public int $passwordMinClasses = 0;

	/** number of password rules that must match (-1 = all) */
	public int $checkedRulesCount = -1;

	/** password may contain the user name */
	public string $passwordMustNotContainUser = 'false';

	/** password may contain more than 2 characters of user/first/last name */
	public string $passwordMustNotContain3Chars = 'false';

	/** external URL for password checking (e.g. https://domain.com/url/{SHA1}) */
	public ?string $externalPwdCheckUrl = null;

	/** path to config file */
	private string $conffile;

	/** uploaded SSL certificate that is stored to disk on save() */
	private ?string $uploadedSSLCaCert = null;

	/** SSL certificate should be deleted on save() */
	private bool $delSSLCaCert = false;

	/** error reporting */
	public string $errorReporting = self::ERROR_REPORTING_DEFAULT;

	/** license data */
	private string $license = '';

	/** license warning email from address */
	public string $licenseEmailFrom = '';

	/** license warning email TO address(es) separated by ";" */
	public string $licenseEmailTo = '';

	/** the license warning email was last sent for this expiration date */
	public string $licenseEmailDateSent = '0';

	/** type of license warning (email/screen/both/none) */
	public string $licenseWarningType = '';

	/** mail server (server:port) */
	public string $mailServer = '';

	/** mail server user */
	public string $mailUser = '';

	/** mail server password */
	public string $mailPassword = '';

	/**
	 * @var string encryption type for SMTP connection
	 */
	public string $mailEncryption = '';

	/**
	 * @var string attribute to use for user mail address
	 */
	public string $mailAttribute = self::MAIL_ATTRIBUTE_DEFAULT;

	/**
	 * @var string attribute to use for user backup mail address
	 */
	public string $mailBackupAttribute = self::MAIL_BACKUP_ATTRIBUTE_DEFAULT;

	/** @var string SMS provider ID */
	public string $smsProvider = '';

	/** @var string SMS API key */
	public string $smsApiKey = '';

	/** @var string SMS token */
	public string $smsToken = '';

	/** @var string SMS account ID */
	public string $smsAccountId = '';

	/** @var string SMS region */
	public string $smsRegion = '';

	/** @var string SMS from */
	public string $smsFrom = '';

	/** @var string SMS subject line */
	public string $smsSubject = '';

	/** @var string SMS number attributes */
	public string $smsAttributes = self::SMS_ATTRIBUTES_DEFAULT;

	/** @var string default country prefix */
	public string $smsDefaultCountryPrefix = '';

	/** database type */
	public string $configDatabaseType = self::DATABASE_FILE_SYSTEM;

	/** database server name */
	public string $configDatabaseServer = '';

	/** database port */
	public string $configDatabasePort = '';

	/** database name */
	public string $configDatabaseName = '';

	/** database user */
	public string $configDatabaseUser = '';

	/** database password */
	public string $configDatabasePassword = '';

	/** database options */
	public string $configDatabaseSSLCA = '';

	/** database password */
	private string $moduleSettings = '';

	/** @var string[] $settings list of data fields to save in config file */
	private array $settings = ["password", "default", "sessionTimeout", "hideLoginErrorDetails",
		"logLevel", "logDestination", "allowedHosts", "passwordMinLength",
		'auditLogType', 'auditLogTarget',
		"passwordMinUpper", "passwordMinLower", "passwordMinNumeric",
		"passwordMinClasses", "passwordMinSymbol", 'checkedRulesCount',
		'passwordMustNotContainUser', 'passwordMustNotContain3Chars',
		'externalPwdCheckUrl',
		'errorReporting', 'allowedHostsSelfService',
		'license', 'licenseEmailFrom', 'licenseEmailTo', 'licenseWarningType', 'licenseEmailDateSent',
		'mailServer', 'mailUser', 'mailPassword', 'mailEncryption',
		'mailAttribute', 'mailBackupAttribute',
		'configDatabaseType',
		'configDatabaseServer', 'configDatabasePort', 'configDatabaseName', 'configDatabaseUser',
		'configDatabasePassword', 'configDatabaseSSLCA', 'moduleSettings',
		'smsProvider', 'smsApiKey', 'smsToken', 'smsAccountId', 'smsRegion', 'smsFrom', 'smsSubject', 'smsAttributes', 'smsDefaultCountryPrefix'
	];

	/** @var string[] $persistenceSettings persistence settings are always stored on local file system */
	private array $persistenceSettings = [
		'configDatabaseType', 'configDatabaseServer',
		'configDatabasePort', 'configDatabaseName', 'configDatabaseUser',
		'configDatabasePassword', 'configDatabaseSSLCA', 'license'
	];

	/**
	 * Loads preferences from config file
	 *
	 * @param string|null $fileName file path for config file
	 */
	public function __construct(?string $fileName = null) {
		$this->conffile = $fileName ?? __DIR__ . "/../config/config.cfg";
		try {
			$this->reload();
		}
		catch (LAMException $e) {
			syslog(LOG_ERR, 'Error loading main config: ' . $e->getTitle() . ' ' . $e->getMessage());
		}
	}

	/**
	 * Exports the configuration data.
	 *
	 * @param bool $skipPersistenceSettings do not include persistence settings
	 * @return array<string, string|string[]|int|bool|null> config data
	 */
	public function exportData(bool $skipPersistenceSettings = false): array {
		$data = [];
		foreach ($this->settings as $setting) {
			if ($skipPersistenceSettings && in_array($setting, $this->persistenceSettings)) {
				continue;
			}
			$data[$setting] = $this->$setting;
		}
		return $data;
	}

	/**
	 * Exports the configuration data, persistence settings only.
	 *
	 * @return array<string, string|string[]|int|bool|null> config data
	 */
	private function exportPersistenceData(): array {
		$data = [];
		foreach ($this->settings as $setting) {
			if (!in_array($setting, $this->persistenceSettings)) {
				continue;
			}
			$data[$setting] = $this->$setting;
		}
		return $data;
	}

	/**
	 * Imports configuration data.
	 *
	 * @param array<string, string|string[]|int|bool|null|object> $data config data
	 * @throws LAMException import error
	 */
	public function importData(array $data): void {
		foreach ($data as $dataKey => $dataValue) {
			if (!in_array($dataKey, $this->settings)) {
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
	public function exportCertificates(): ?string {
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
	public function importCertificates(?string $certsContent): void {
		$fileName = $this->getSSLCaCertPath();
		if (empty($certsContent)) {
			unlink($fileName);
			return;
		}
		$fileName = $this->getInternalSSLCaCertFileName();
		$handle = @fopen($fileName, "wb");
		if ($handle) {
			fwrite($handle, $certsContent);
			fclose($handle);
			@chmod($fileName, 0600);
		}
		else {
			throw new LAMException(sprintf(_('Unable to write file %s.'), $fileName));
		}
	}

	/**
	 * Reloads preferences from config file config.cfg
	 *
	 * @throws LAMException error reading config
	 */
	private function reload(): void {
		if (is_file($this->conffile)) {
			$file = @fopen($this->conffile, "r");
			if (!$file) {
				throw new LAMException(_('The config file is not readable.'));
			}
			$json = fread($file, 1000000);
			$data = json_decode($json, true);
			if ($data !== null) {
				$this->importData($data);
			}
			else {
				throw new LAMException(_('The config file is not readable.'));
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
	 * @throws LAMException error reading config
	 */
	private function loadFromDb(): void {
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
			throw new LAMException(_('Unable to connect to configuration database.'), '', $e);
		}
		catch (LAMException $e) {
			syslog(LOG_ERR, 'Unable to import main config: ' . $e->getMessage());
			throw new LAMException(_('Unable to connect to configuration database.'), '', $e);
		}
	}

	/**
	 * Saves the configuration to the persistence layer.
	 * @throws LAMException error saving config
	 */
	public function save(): void {
		if ($this->configDatabaseType === self::DATABASE_MYSQL) {
			$this->saveLocal(true);
			$this->saveDb();
		}
		else {
			$this->saveLocal(false);
		}
		// store SSL certificate
		if ($this->uploadedSSLCaCert !== null) {
			$sslPath = $this->getInternalSSLCaCertFileName();
			$file = @fopen($sslPath, "w");
			if ($file) {
				fwrite($file, $this->uploadedSSLCaCert);
				fclose($file);
				@chmod($sslPath, 0600);
			}
			else {
				throw new LAMException(_("Cannot write certificate file. Please check the permissions of config/serverCerts.pem."));
			}
		}
		// delete SSL certificate
		if ($this->delSSLCaCert) {
			$sslPath = $this->getInternalSSLCaCertFileName();
			$result = @unlink($sslPath);
			if (!$result) {
				throw new LAMException(_("Cannot write certificate file. Please check the permissions of config/serverCerts.pem."));
			}
		}
	}

	/**
	 * Saves the config data to the database.
	 */
	private function saveDb(): void {
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
		$stmt->execute([$nameKey, $json]);
		$pdo->commit();
	}

	/**
	 * Saves preferences to config file config.cfg
	 *
	 * @param bool $persistenceOnly store only persistence related data
	 * @throws LAMException error saving config
	 */
	public function saveLocal(bool $persistenceOnly): void {
		$data = $persistenceOnly ? $this->exportPersistenceData() : $this->exportData();
		$json = json_encode($data, JSON_PRETTY_PRINT);

		$file = @fopen($this->conffile, "w");
		if ($file) {
			fwrite($file, $json);
			fclose($file);
			chmod($this->conffile, 0600);
		}
		else {
			throw new LAMException(_("Cannot open config file!") . " (" . $this->conffile . ")");
		}
	}

	/**
	 * Returns if the main config has a password set.
	 *
	 * @return bool password is set
	 */
	public function hasPasswordSet(): bool {
		return ($this->password !== null) && ($this->password !== '');
	}

	/**
	 * Sets a new config password.
	 *
	 * @param string $password new password
	 */
	public function setPassword(string $password): void {
		$this->password = $this->hashPassword($password, generateSalt(16));
	}

	/**
	 * Checks if the given password matches.
	 *
	 * @param string $password password
	 * @return bool true, if password matches
	 */
	public function checkPassword(string $password): bool {
		if (str_starts_with($this->password, "{SSHA}")) {
			$value = substr($this->password, strlen("{SSHA}"));
			$parts = explode(" ", $value);
			$salt = base64_decode($parts[1]);
			$hash = "{SSHA}" . base64_encode(hex2bin(sha1($password . $salt))) . " " . base64_encode($salt);
			return ($hash === $this->password);
		}
		elseif (str_starts_with($this->password, "{CRYPT-SHA512}")) {
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
	 * @param string $password password
	 * @param string $salt salt
	 * @return string hash value
	 */
	private function hashPassword(string $password, string $salt): string {
		return "{CRYPT-SHA512}" . crypt($password, '$6$' . $salt) . " " . base64_encode($salt);
	}

	/**
	 * Returns if the configuration file is writable.
	 *
	 * @return boolean writable
	 */
	public function isWritable(): bool {
		return is_writable($this->conffile);
	}

	/**
	 * Returns if the configuration file is existing.
	 *
	 * @return bool exists
	 */
	public function isConfigFileExisting(): bool {
		return file_exists($this->conffile);
	}

	/**
	 * Tries to copy the config file from sample config.
	 *
	 * @return bool copied
	 */
	public function installSampleConfig(): bool {
		$samplePath = __DIR__ . '/../config/config.cfg.sample';
		return file_exists($samplePath) && copy($samplePath, $this->conffile);
	}

	/**
	 * Returns the path to the SSL CA certificate file that overrides the system certificates.
	 *
	 * @return string|null path to certificate file or null if certificate is not overridden
	 */
	public function getSSLCaCertPath(): ?string {
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
		return __DIR__ . '/../config/serverCerts.pem';
	}

	/**
	 * Uploads a new SSL CA cert.
	 *
	 * @param String $cert file content in DER/PEM format
	 * @return true|string true if format is correct, error message if file is not accepted
	 */
	public function uploadSSLCaCert($cert) {
		if (!str_contains($cert, '-----BEGIN CERTIFICATE-----')) {
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
	 * @return string|null file name or null if no certificate was set
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
		try {
			$temporaryFilesManager = new LamTemporaryFilesManager();
			$fileName = $temporaryFilesManager->registerTemporaryFile('.pem');
			$handle = $temporaryFilesManager->openTemporaryFileForWrite($fileName);
			fwrite($handle, $content);
			fclose($handle);
			return $fileName;
		}
		catch (LAMException $e) {
			logNewMessage(LOG_ERR, 'Unable to store SSL CA file: ' . $e->getTitle());
			return null;
		}
	}

	/**
	 * Marks a single or all SSL CA certificate files for deletion.
	 * The changes take effect on save().
	 *
	 * @param int|null $index certificate index, null deletes all certificates (default: null)
	 */
	public function deleteSSLCaCert(?int $index = null): void {
		if ($index == null) {
			// delete all
			$this->delSSLCaCert = true;
			return;
		}
		$content = $this->getSSLCaCertificateContent();
		$list = $this->splitSSLCaCertificateContent($content);
		unset($list[$index]);
		if (count($list) < 1) {
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
	 * @return array<int, array<string, string[][]>> list of certificates as output of openssl_x509_parse()
	 */
	public function getSSLCaCertificates(): array {
		if ($this->delSSLCaCert) {
			return [];
		}
		$content = $this->getSSLCaCertificateContent();
		if (empty($content)) {
			return [];
		}
		$list = $this->splitSSLCaCertificateContent($content);
		for ($i = 0; $i < count($list); $i++) {
			$list[$i] = @openssl_x509_parse($list[$i]);
		}
		return $list;
	}

	/**
	 * Returns the content of the certificate file or uploaded data.
	 *
	 * @return string|null null or certificate content
	 */
	private function getSSLCaCertificateContent(): ?string {
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
	 * @param string $content PEM file content
	 * @return array<int, string> one element for each certificate chunk
	 */
	private function splitSSLCaCertificateContent(string $content): array {
		if (empty($content)) {
			return [];
		}
		$content = str_replace("\n\n", "\n", $content);
		if (!(str_starts_with($content, '-----BEGIN CERTIFICATE-----'))) {
			return [];
		}
		$lines = explode("\n", $content);
		$list = [];
		$pos = -1;
		foreach ($lines as $line) {
			if (str_starts_with($line, '-----BEGIN CERTIFICATE-----')) {
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
	 * @return string[] license
	 */
	public function getLicenseLines(): array {
		return explode(LAMConfig::LINE_SEPARATOR, $this->license);
	}

	/**
	 * Sets the license key as multiple lines.
	 *
	 * @param string[] $licenseLines license lines
	 */
	public function setLicenseLines(array $licenseLines): void {
		$this->license = implode(LAMConfig::LINE_SEPARATOR, $licenseLines);
	}

	/**
	 * Returns the license warning type (screen/email/both/none).
	 *
	 * @return string warning type
	 */
	public function getLicenseWarningType(): string {
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
	public function showLicenseWarningOnScreen(): bool {
		$type = $this->getLicenseWarningType();
		return ($type === self::LICENSE_WARNING_ALL) || ($type === self::LICENSE_WARNING_SCREEN);
	}

	/**
	 * Returns if the license warning should be sent via email.
	 *
	 * @return bool send via email
	 */
	public function sendLicenseWarningByEmail(): bool {
		$type = $this->getLicenseWarningType();
		return ($type === self::LICENSE_WARNING_ALL) || ($type === self::LICENSE_WARNING_EMAIL);
	}

	/**
	 * Returns if the license warning was already sent.
	 *
	 * @param int $timeStamp time stamp
	 */
	public function wasLicenseWarningSent(int $timeStamp): bool {
		if (empty($this->licenseEmailDateSent)) {
			return false;
		}
		return $timeStamp === intval($this->licenseEmailDateSent);
	}

	/**
	 * Hide error details for failed logins.
	 *
	 * @return bool hide details
	 */
	public function isHideLoginErrorDetails(): bool {
		return $this->hideLoginErrorDetails === 'true';
	}

	/**
	 * Returns the mail attribute.
	 *
	 * @return string attribute name
	 */
	public function getMailAttribute(): string {
		if (!empty($this->mailAttribute)) {
			return $this->mailAttribute;
		}
		return self::MAIL_ATTRIBUTE_DEFAULT;
	}

	/**
	 * Returns the mail backup attribute.
	 *
	 * @return string attribute name
	 */
	public function getMailBackupAttribute(): string {
		if (!empty($this->mailBackupAttribute)) {
			return $this->mailBackupAttribute;
		}
		return self::MAIL_BACKUP_ATTRIBUTE_DEFAULT;
	}

	/**
	 * Returns the SMS attributes.
	 *
	 * @return string[] attribute names
	 */
	public function getSmsAttributes(): array {
		if (!empty($this->smsAttributes)) {
			return preg_split('/;[ ]*/', $this->smsAttributes);
		}
		return preg_split('/;[ ]*/', self::SMS_ATTRIBUTES_DEFAULT);
	}

	/**
	 * Returns a list of module settings.
	 *
	 * @return array<string, array<string, string>> module settings
	 */
	public function getModuleSettings(): array {
		if (empty($this->moduleSettings)) {
			return [];
		}
		return json_decode(base64_decode($this->moduleSettings), true);
	}

	/**
	 * Sets the module settings.
	 *
	 * @param array<string, array<string, string>> $settings module settings
	 */
	public function setModuleSettings(array $settings): void {
		$this->moduleSettings = base64_encode(json_encode($settings));
	}

	/**
	 * Checks if the log filename is valid.
	 *
	 * @param string $path path
	 * @return bool is valid
	 */
	public static function isValidLogFilename(string $path): bool {
		$realPath = file_exists($path) ? realpath($path) : $path;
		$blockedPrefixes = ['/usr', '/etc', '/dev', '/boot', '/lib', '/proc', '/root', '/run', '/sys', '/snap'];
		if (!empty($_SERVER['DOCUMENT_ROOT'])) {
			$blockedPrefixes[] = $_SERVER['DOCUMENT_ROOT'];
		}
		foreach ($blockedPrefixes as $blockedPrefix) {
			if (str_starts_with($realPath, $blockedPrefix)) {
				return false;
			}
		}
		return !empty($realPath)
			&& preg_match("/^[a-z0-9\\/._-]+$/i", $realPath)
			&& (str_ends_with($realPath, '.log') || str_ends_with($realPath, '.txt'))
			&& !str_contains($realPath, '..')
			&& !str_starts_with($realPath, './');
	}

}

enum LOG_FILE_TARGET: string {

	/** audit log entries are sent to file */
	case FILE = 'FILE';
	/** audit log entries are sent to remote server */
	case REMOTE = 'REMOTE';
	/** audit log entries are sent to syslog */
	case SYSLOG = 'SYSLOG';
	/** audit log is disabled */
	case NONE = 'NONE';

}
