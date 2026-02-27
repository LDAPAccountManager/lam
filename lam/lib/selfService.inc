<?php

use LAM\LIB\TWO_FACTOR\TwoFactorProviderService;
use LAM\PERSISTENCE\ConfigurationDatabase;
use LDAP\Connection;
use function LAM\PERSISTENCE\dbTableExists;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2006 - 2026  Roland Gruber

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
 * Interface between modules and self-service pages.
 * This file also includes the self-service profile class and helper functions.
 *
 * @package selfService
 * @author Roland Gruber
 */

/** modules */
include_once __DIR__ . "/modules.inc";
/** account types */
include_once __DIR__ . "/types.inc";
/** 2-factor */
include_once __DIR__ . '/2factor.inc';

/**
 * Returns if this is a LAM Pro installation.
 *
 * @return boolean LAM Pro installation
 */
function isLAMProVersion() {
	$dir = substr(__FILE__, 0, strlen(__FILE__) - 20) . "/templates/selfService";
	return is_dir($dir);
}

/**
 * Returns a list of possible search attributes for the self-service.
 *
 * @param string $scope account type
 * @return string[] attributes
 */
function getSelfServiceSearchAttributes($scope): array {
	$return = [];
	$modules = getAvailableModules($scope);
	for ($i = 0; $i < count($modules); $i++) {
		$m = moduleCache::getModule($modules[$i], $scope);
		$attributes = $m->getSelfServiceSearchAttributes();
		$return = array_merge($return, $attributes);
	}
	$return = array_unique($return);
	return array_values($return);
}

/**
 * Returns the field settings for the self-service.
 *
 * @param string $scope account type
 * @return array<string, array<string, string>> settings
 */
function getSelfServiceFieldSettings($scope) {
	$return = [];
	$modules = getAvailableModules($scope);
	for ($i = 0; $i < count($modules); $i++) {
		$m = moduleCache::getModule($modules[$i], $scope);
		$settings = $m->getSelfServiceFields();
		if (count($settings) > 0) {
			$return[$modules[$i]] = $settings;
		}
	}
	return $return;
}

/**
 * Returns meta HTML code for each self-service field.
 *
 * @param string $scope account type
 * @param array<string, string[]> $fields input fields (array(<moduleName> => array(<field1>, <field2>, ...)))
 * @param array<string, string[]|string> $attributes LDAP attributes (attribute names in lower-case)
 * @param boolean $passwordChangeOnly indicates that the user is only allowed to change his password and no LDAP content is readable
 * @param string[] $readOnlyFields list of read-only fields
 * @return array<string, array<string, htmlResponsiveRow>> meta HTML code (array(<moduleName> => array(<optionName> => htmlResponsiveRow)))
 */
function getSelfServiceOptions($scope, $fields, $attributes, $passwordChangeOnly, $readOnlyFields) {
	$return = [];
	$modules = getAvailableModules($scope);
	foreach ($modules as $module) {
		if (!isset($fields[$module])) {
			continue;
		}
		$m = moduleCache::getModule($module, $scope);
		$modReadOnlyFields = [];
		for ($r = 0; $r < count($readOnlyFields); $r++) {
			$parts = explode('_', $readOnlyFields[$r]);
			if ($parts[0] == $module) {
				$modReadOnlyFields[] = $parts[1];
			}
		}
		$code = $m->getSelfServiceOptions($fields[$module], $attributes, $passwordChangeOnly, $modReadOnlyFields);
		if (count($code) > 0) {
			$return[$module] = $code;
		}
	}
	return $return;
}

/**
 * Checks if all input values are correct and returns the LDAP commands which should be executed.
 *
 * @param string $scope account type
 * @param array<string, string[]> $fields input fields (array(<moduleName> => array(<field1>, <field2>, ...)))
 * @param array<string, string[]|string> $attributes LDAP attributes
 * @param boolean $passwordChangeOnly indicates that the user is only allowed to change his password and no LDAP content is readable
 * @param string[] $readOnlyFields list of read-only fields
 * @return array<string, string[]|array<string, string[]|string>> messages and LDAP commands (array('messages' => [], 'add' => [], 'del' => [], 'mod' => []))
 */
function checkSelfServiceOptions($scope, $fields, $attributes, $passwordChangeOnly, $readOnlyFields) {
	$return = ['messages' => [], 'add' => [], 'del' => [], 'mod' => [], 'info' => []];
	$modules = getAvailableModules($scope);
	for ($i = 0; $i < count($modules); $i++) {
		if (!isset($fields[$modules[$i]])) {
			continue;
		}
		$m = moduleCache::getModule($modules[$i], $scope);
		$modReadOnlyFields = [];
		for ($r = 0; $r < count($readOnlyFields); $r++) {
			$parts = explode('_', $readOnlyFields[$r]);
			if ($parts[0] == $modules[$i]) {
				$modReadOnlyFields[] = $parts[1];
			}
		}
		$result = $m->checkSelfServiceOptions($fields[$modules[$i]], $attributes, $passwordChangeOnly, $modReadOnlyFields);
		if (count($result['messages']) > 0) {
			$return['messages'] = array_merge($result['messages'], $return['messages']);
		}
		if (count($result['add']) > 0) {
			$return['add'] = array_merge($result['add'], $return['add']);
		}
		if (count($result['del']) > 0) {
			$return['del'] = array_merge($result['del'], $return['del']);
		}
		if (count($result['mod']) > 0) {
			$return['mod'] = array_merge($result['mod'], $return['mod']);
		}
		if (count($result['info']) > 0) {
			$return['info'] = array_merge($result['info'], $return['info']);
		}
	}
	return $return;
}

/**
 * Returns a hash array (module name => elements) of all module options for the configuration page.
 *
 * @param string $scope account type
 * @param selfServiceProfile $profile currently edited profile
 * @return array<string, htmlElement> configuration options
 */
function getSelfServiceSettings(string $scope, selfServiceProfile $profile): array {
	$return = [];
	$modules = getAvailableModules($scope);
	for ($i = 0; $i < count($modules); $i++) {
		$m = moduleCache::getModule($modules[$i], $scope);
		$modSettings = $m->getSelfServiceSettings($profile);
		if ($modSettings === null) {
			continue;
		}
		$return[$modules[$i]] = $modSettings;
	}
	return $return;
}

/**
 * Checks if the self-service settings are valid
 *
 * @param string $scope account type
 * @param array<string, string[]> $options hash array containing all options (name => array(...))
 * @param selfServiceProfile $profile profile
 * @return string[][] list of error messages
 */
function checkSelfServiceSettings($scope, &$options, &$profile) {
	$return = [];
	$modules = getAvailableModules($scope);
	for ($i = 0; $i < count($modules); $i++) {
		$m = moduleCache::getModule($modules[$i], $scope);
		$errors = $m->checkSelfServiceSettings($options, $profile);
		$return = array_merge($return, $errors);
	}
	return $return;
}

/**
 * Returns if script runs inside self-service.
 *
 * @return boolean is self-service
 */
function isSelfService() {
	return session_name() == 'SELFSERVICE';
}

/**
 * Opens the LDAP connection and returns the handle. No bind is done.
 *
 * @param selfServiceProfile $profile profile
 * @return Connection|null LDAP handle or null if connection failed
 */
function openSelfServiceLdapConnection($profile): ?Connection {
	$server = connectToLDAP($profile->serverURL, $profile->useTLS);
	if ($server != null) {
		// follow referrals
		ldap_set_option($server, LDAP_OPT_REFERRALS, $profile->followReferrals);
	}
	return $server;
}

/**
 * Binds the LDAP connections with given user and password.
 *
 * @param Connection $handle LDAP handle
 * @param selfServiceProfile $profile profile
 * @param string $userDn bind DN
 * @param string $password bind password
 * @return bool binding successful
 */
function bindLdapUser($handle, $profile, $userDn, $password): bool {
	if ($profile->useForAllOperations) {
		$userDn = $profile->LDAPUser;
		$password = deobfuscateText($profile->LDAPPassword);
	}
	return @ldap_bind($handle, $userDn, $password);
}

/**
 * Manages reading and writing self-service profiles.
 */
class SelfServicePersistence {

	/**
	 * @var SelfServicePersistenceStrategy
	 */
	private $strategy;

	/**
	 * Constructor
	 *
	 * @throws LAMException error connecting database
	 */
	public function __construct() {
		$configDb = new ConfigurationDatabase(new LAMCfgMain());
		if ($configDb->useRemoteDb()) {
			try {
				$this->strategy = new SelfServicePersistenceStrategyPdo($configDb->getPdo());
			}
			catch (PDOException $e) {
				logNewMessage(LOG_ERR, _('Unable to connect to configuration database.') . ' ' . $e->getMessage());
				throw new LAMException(_('Unable to connect to configuration database.'), '', $e);
			}
		}
		else {
			$this->strategy = new SelfServicePersistenceStrategyFileSystem();
		}
	}

	/**
	 * Returns a list of available self-service profiles.
	 *
	 * @return array<string, string[]> profile names (array('account type' => array('profile1', 'profile2')))
	 */
	public function getProfiles(): array {
		$profiles = $this->strategy->getProfiles();
		ksort($profiles);
		foreach (array_keys($profiles) as $key) {
			sort($profiles[$key]);
		}
		return $profiles;
	}

	/**
	 * Returns if the profile with given name can be written.
	 *
	 * @param string $name profile name
	 * @param string $scope user/group/host
	 * @return bool can be written
	 */
	public function canWrite(string $name, string $scope): bool {
		return $this->strategy->canWrite($name, $scope);
	}

	/**
	 * Loads the given self-service profile.
	 *
	 * @param string $name profile name
	 * @param string $scope user/group/host
	 * @return selfServiceProfile profile
	 * @throws LAMException error during loading
	 */
	public function load(string $name, string $scope): selfServiceProfile {
		return $this->strategy->load($name, $scope);
	}

	/**
	 * Stores the given profile.
	 *
	 * @param string $name profile name
	 * @param string $scope user/group/host
	 * @param selfServiceProfile $profile profile
	 * @throws LAMException error during saving
	 */
	public function save(string $name, string $scope, selfServiceProfile $profile): void {
		$this->strategy->save($name, $scope, $profile);
	}

	/**
	 * Deletes a self-service profile.
	 *
	 * @param string $name profile name
	 * @param string $scope user/group/host
	 * @throws LAMException error deleting profile
	 */
	public function delete(string $name, string $scope): void {
		$profileList = $this->getProfiles();
		if (!preg_match("/^[a-z]+$/i", $scope)
			|| !preg_match("/^[a-z0-9_-]+$/i", $name)
			|| empty($profileList[$scope])
			|| !in_array($name, $profileList[$scope])) {
			throw new LAMException(_("Unable to delete profile!"));
		}
		$this->strategy->delete($name, $scope);
	}

	/**
	 * Renames a self-service profile.
	 *
	 * @param string $oldName existing profile name
	 * @param string $newName new profile name
	 * @param string $scope user/group/host
	 * @throws LAMException error renaming profile
	 */
	public function rename(string $oldName, string $newName, string $scope): void {
		$profileList = $this->getProfiles();
		if (!preg_match("/^[a-z]+$/i", $scope)
			|| !preg_match("/^[a-z0-9_-]+$/i", $newName)
			|| empty($profileList[$scope])
			|| !in_array($oldName, $profileList[$scope])) {
			throw new LAMException(_("Profile name is invalid!"));
		}
		$this->strategy->rename($oldName, $newName, $scope);
	}

}

/**
 * Interface for self-service profile persistence.
 */
interface SelfServicePersistenceStrategy {

	/**
	 * Returns a list of available self-service profiles.
	 *
	 * @return array<string, string[]> profile names (array('account type' => array('profile1', 'profile2')))
	 */
	public function getProfiles(): array;

	/**
	 * Returns if the profile with the given name can be written.
	 *
	 * @param string $name profile name
	 * @param string $scope user/group/host
	 * @return bool can be written
	 */
	public function canWrite(string $name, string $scope): bool;

	/**
	 * Loads the given self-service profile.
	 *
	 * @param string $name profile name
	 * @param string $scope user/group/host
	 * @return selfServiceProfile profile
	 * @throws LAMException error during loading
	 */
	public function load(string $name, string $scope): selfServiceProfile;

	/**
	 * Stores the given profile.
	 *
	 * @param string $name profile name
	 * @param string $scope user/group/host
	 * @param selfServiceProfile $profile profile
	 * @throws LAMException error during saving
	 */
	public function save(string $name, string $scope, selfServiceProfile $profile): void;

	/**
	 * Deletes a self-service profile.
	 *
	 * @param string $name profile name
	 * @param string $scope account type
	 * @throws LAMException error renaming profile
	 */
	public function delete(string $name, string $scope): void;

	/**
	 * Renames a self-service profile.
	 *
	 * @param string $oldName existing profile name
	 * @param string $newName new profile name
	 * @param string $scope user/group/host
	 * @throws LAMException error renaming profile
	 */
	public function rename(string $oldName, string $newName, string $scope): void;

}

/**
 * Uses local file system for storing self-service profiles.
 */
class SelfServicePersistenceStrategyFileSystem implements SelfServicePersistenceStrategy {

	/**
	 * @inheritDoc
	 */
	public function getProfiles(): array {
		$types = LAM\TYPES\getTypes();
		$dir = dir(__DIR__ . "/../config/selfService");
		$ret = [];
		if ($dir === false) {
			logNewMessage(LOG_ERR, 'Unable to read self-service profiles');
			return $ret;
		}
		while ($entry = $dir->read()) {
			$ext = substr($entry, strrpos($entry, '.') + 1);
			$name = substr($entry, 0, strrpos($entry, '.'));
			// check if extension is right, add to profile list
			if (in_array($ext, $types)) {
				$ret[$ext][] = $name;
			}
		}
		return $ret;
	}

	/**
	 * @inheritDoc
	 */
	public function canWrite(string $name, string $scope): bool {
		// check profile name
		if (!preg_match("/^[0-9a-z _-]+$/i", $scope) || !preg_match("/^[0-9a-z _-]+$/i", $name)) {
			return false;
		}
		$path = __DIR__ . "/../config/selfService/" . $name . "." . $scope;
		return is_writable($path);
	}

	/**
	 * @inheritDoc
	 */
	public function load(string $name, string $scope): selfServiceProfile {
		if (!preg_match("/^[0-9a-z _-]+$/i", $name) || !preg_match("/^[0-9a-z _-]+$/i", $scope)) {
			throw new LAMException(_("Profile name is invalid!"));
		}
		$profile = new selfServiceProfile();
		$file = __DIR__ . "/../config/selfService/" . $name . "." . $scope;
		if (is_file($file)) {
			$file = @fopen($file, "r");
			if ($file) {
				$data = fread($file, 10000000);
				$profileData = @json_decode($data, true);
				if ($profileData === null) {
					logNewMessage(LOG_ERR, "Unable to load profile because not in JSON format: " . $name);
					throw new LAMException(_("Unable to load profile!"), $name);
				}
				$profile = selfServiceProfile::import($profileData);
				fclose($file);
			}
			else {
				throw new LAMException(_("Unable to load profile!"), $name);
			}
		}
		else {
			throw new LAMException(_("Unable to load profile!"), $name);
		}
		return $profile;
	}

	/**
	 * @inheritDoc
	 */
	public function save(string $name, string $scope, selfServiceProfile $profile): void {
		// check profile name
		if (!preg_match("/^[0-9a-z _-]+$/i", $scope) || !preg_match("/^[0-9a-z _-]+$/i", $name)) {
			throw new LAMException(_("Profile name is invalid!"));
		}
		$path = __DIR__ . "/../config/selfService/" . $name . "." . $scope;
		$file = @fopen($path, "w");
		if ($file) {
			// write settings to file
			fwrite($file, json_encode($profile->export()));
			// close file
			fclose($file);
			@chmod($path, 0600);
		}
		else {
			throw new LAMException(_("Unable to save profile!"));
		}
	}

	/**
	 * @inheritDoc
	 */
	public function delete(string $name, string $scope): void {
		if (!unlink("../../config/selfService/" . $name . "." . $scope)) {
			throw new LAMException(_("Unable to delete profile!"));
		}
	}

	/**
	 * @inheritDoc
	 */
	public function rename(string $oldName, string $newName, string $scope): void {
		if (!@rename(__DIR__ . "/../config/selfService/" . $oldName . "." . $scope,
			__DIR__ . "/../config/selfService/" . $newName . "." . $scope)) {
			throw new LAMException(_("Could not rename file!"));
		}
	}

}

/**
 * Uses PDO for storing self-service profiles.
 */
class SelfServicePersistenceStrategyPdo implements SelfServicePersistenceStrategy {

	/**
	 * @var PDO
	 */
	private $pdo;

	private const TABLE_NAME = 'self_service_profiles';

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
			. 'scope VARCHAR(100) NOT NULL,'
			. 'name VARCHAR(300) NOT NULL,'
			. 'data TEXT NOT NULL,'
			. 'PRIMARY KEY(scope,name)'
			. ');';
		$this->pdo->exec($sql);
		$sql = 'insert into ' . ConfigurationDatabase::TABLE_SCHEMA_VERSIONS . ' (name, version) VALUES ("self_service", 1);';
		$this->pdo->exec($sql);
	}

	/**
	 * @inheritDocm
	 */
	public function getProfiles(): array {
		$statement = $this->pdo->prepare("SELECT scope, name FROM " . self::TABLE_NAME);
		$statement->execute();
		$results = $statement->fetchAll();
		$profiles = [];
		foreach ($results as $result) {
			$profiles[$result['scope']][] = $result['name'];
		}
		return $profiles;
	}

	/**
	 * @inheritDoc
	 */
	public function canWrite(string $name, string $scope): bool {
		return true;
	}

	/**
	 * @inheritDoc
	 */
	public function load(string $name, string $scope): selfServiceProfile {
		$statement = $this->pdo->prepare("SELECT data FROM " . self::TABLE_NAME . " WHERE scope = ? AND name = ?");
		$statement->execute([$scope, $name]);
		$results = $statement->fetchAll();
		if (empty($results)) {
			logNewMessage(LOG_ERR, 'Self-service profile not found');
			throw new LAMException(_("Unable to load profile!"), $name);
		}
		$data = json_decode($results[0]['data'], true);
		return selfServiceProfile::import($data);
	}

	/**
	 * @inheritDoc
	 */
	public function save(string $name, string $scope, selfServiceProfile $profile): void {
		$data = json_encode($profile->export());
		$statement = $this->pdo->prepare("SELECT data FROM " . self::TABLE_NAME . " WHERE scope = ? AND name = ?");
		$statement->execute([$scope, $name]);
		$results = $statement->fetchAll();
		if (empty($results)) {
			$statement = $this->pdo->prepare("INSERT INTO " . self::TABLE_NAME . " (scope, name, data) VALUES (?, ?, ?)");
			$statement->execute([$scope, $name, $data]);
		}
		else {
			$statement = $this->pdo->prepare("UPDATE " . self::TABLE_NAME . " SET data = ? WHERE scope = ? AND name = ?");
			$statement->execute([$data, $scope, $name]);
		}
	}

	/**
	 * @inheritDoc
	 */
	public function delete(string $name, string $scope): void {
		$statement = $this->pdo->prepare("DELETE FROM " . self::TABLE_NAME . " WHERE scope = ? AND name = ?");
		$statement->execute([$scope, $name]);
	}

	/**
	 * @inheritDoc
	 */
	public function rename(string $oldName, string $newName, string $scope): void {
		$statement = $this->pdo->prepare("UPDATE " . self::TABLE_NAME . " SET name = ? WHERE scope = ? AND name = ?");
		$statement->execute([$newName, $scope, $oldName]);
	}

}

/**
 * Includes all settings of a self-service profile.
 *
 * @package selfService
 */
class selfServiceProfile {

	/** server address */
	public string $serverURL = "localhost";

	/** use TLS */
	public bool $useTLS = false;

	/** LDAP suffix */
	public string $LDAPSuffix = "dc=my-domain,dc=com";

	/** LDAP user DN*/
	public string $LDAPUser = "";

	/** LDAP password */
	public string $LDAPPassword = "";

	/** use bind user also for read/modify operations */
	public ?bool $useForAllOperations = false;

	/** LDAP search attribute */
	public string $searchAttribute = "uid";

	/**
	 * @var string|null login handler ID
	 */
	public ?string $loginHandler = SelfServiceUserPasswordLoginHandler::ID;

	/** header for self-service pages */
	public string $pageHeader = '<p><a href="https://www.ldap-account-manager.org/" target="new_window"><img alt="help" class="align-middle" src="../../graphics/logo.svg" style="height:24px; width:24px" /> LDAP Account Manager </a></p><p>&nbsp;</p>';

	/** base color */
	public string $baseColor = '#fffde2';

	/** list of additional CSS links (separated by \n) */
	public string $additionalCSS = '';

	/** describing text for user login */
	public string $loginCaption = '';

	/** describing text for user login */
	public ?string $loginFooter = '';

	/** label for password input */
	public ?string $passwordLabel = '';

	/** describing text for search attribute */
	public string $loginAttributeText = '';

	/** additional LDAP filter for accounts */
	public ?string $additionalLDAPFilter = '';

	/** describing text for self-service main page */
	public string $mainPageText;

	/** describing text for self-service main page */
	public ?string $mainPageFooter = '';

	/**
	 * @var array<string, string|string[]>[] $inputFields input fields
	 * Format: array(
	 * <br> array(array('name' => <group name 1>, 'fields' => array(<field1>, <field2>))),
	 * <br> array(array('name' => <group name 2>, 'fields' => array(<field3>, <field4>)))
	 * <br> )
	 */
	public array $inputFields = [];

	/**
	 * @var string[] $readOnlyFields list of fields that are set in read-only mode.
	 */
	public array $readOnlyFields = [];

	/** @var array<string, string>|null $relabelFields list of override values for field labels: array(<field ID> => label) */
	public ?array $relabelFields = [];

	/** @var array<string, string[]> $moduleSettings configuration settings of modules */
	public array $moduleSettings = [];

	/** language for self-service */
	public string $language = 'en_GB.utf8';
	/** disallow user to change language */
	public bool $enforceLanguage = false;

	public int $followReferrals = 0;

	public string $timeZone = 'Europe/London';

	public string $twoFactorAuthentication = TwoFactorProviderService::TWO_FACTOR_NONE;
	public string $twoFactorAuthenticationURL = 'https://localhost';
	public bool $twoFactorAuthenticationInsecure = false;
	public ?string $twoFactorAuthenticationLabel = '';
	public bool $twoFactorAuthenticationOptional = false;
	public string $twoFactorAuthenticationCaption = '';
	public string $twoFactorAuthenticationClientId = '';
	public string $twoFactorAuthenticationSecretKey = '';
	public string $twoFactorAuthenticationAttribute = 'uid';
	public string $twoFactorAuthenticationDomain = '';
	public string $twoFactorAllowToRememberDevice = 'false';
	public string $twoFactorRememberDeviceDuration = '28800';
	public string $twoFactorRememberDevicePassword = '';

	/** provider for captcha (-/google/hCaptcha/friendlyCaptcha) */
	public string $captchaProvider = '-';
	/** Google reCAPTCHA site key */
	public string $reCaptchaSiteKey = '';
	/** Google reCAPTCHA secret key */
	public string $reCaptchaSecretKey = '';
	/** enable captcha on self-service login */
	public bool $captchaOnLogin = false;

	/** base URL for the website (e.g. https://example.com) for link generation */
	public string $baseUrl = '';

	/**
	 * Path to external lamdaemon script on server where it is executed
	 *
	 * This is used for managing quota and home directories.
	 * optional setting, may not be defined
	 */
	public ?string $scriptPath = '';

	/**
	 * The rights for the home directory
	 */
	public string $scriptRights = '750';

	/**
	 * Servers where lamdaemon script is executed
	 *
	 * This is used for managing quota and home directories.
	 * optional setting, may not be defined
	 */
	public ?string $scriptServer = '';

	/**
	 * user name for lamdaemon
	 */
	public ?string $scriptUserName = '';

	/**
	 * File name of SSH key for lamdaemon.
	 */
	public ?string $scriptSSHKey = '';

	/**
	 * Password for lamdaemon SSH key.
	 */
	public ?string $scriptSSHKeyPassword = '';

	/**
	 * Constructor
	 */
	public function __construct() {
		$this->loginCaption = '<b>' . _("Welcome to LAM self-service. Please enter your user name and password.") . '</b>';
		$this->loginAttributeText = _('User name');
		$this->mainPageText = "<h1>LAM self-service</h1>\n" . _("Here you can change your personal settings.");
		$this->inputFields = [
			['name' => _('Personal data'),
				'fields' => ['inetOrgPerson_firstName', 'inetOrgPerson_lastName', 'inetOrgPerson_mail',
					'inetOrgPerson_telephoneNumber', 'inetOrgPerson_mobile', 'inetOrgPerson_faxNumber',
					'inetOrgPerson_street', 'inetOrgPerson_postalAddress']],
			['name' => _('Password'),
				'fields' => ['posixAccount_password']]
		];
	}

	/**
	 * Converts the export data back to a self-service profile.
	 *
	 * @param array<string, string|int> $data export data
	 * @return selfServiceProfile profile
	 */
	public static function import($data): selfServiceProfile {
		$profile = new selfServiceProfile();
		$vars = get_class_vars(selfServiceProfile::class);
		foreach ($data as $key => $value) {
			if (array_key_exists($key, $vars)) {
				$profile->$key = $value;
			}
		}
		return $profile;
	}

	/**
	 * Returns a representation of the self-service profile.
	 *
	 * @return array<string, string|int> self-service profile data
	 */
	public function export() {
		return json_decode(json_encode($this), true);
	}

	/**
	 * Returns the server's base URL (e.g. https://www.example.com).
	 *
	 * @return string URL
	 */
	public function getBaseUrl() {
		if (!empty($this->baseUrl)) {
			return $this->baseUrl;
		}
		$callingUrl = getCallingURL();
		$matches = [];
		if (preg_match('/^(http(s)?:\\/\\/[^\\/]+)\\/.+$/', $callingUrl, $matches)) {
			return $matches[1];
		}
		return '';
	}

	/**
	 * Sets the server's base URL (e.g. https://www.example.com).
	 *
	 * @param string $url URL
	 */
	public function setBaseUrl($url): void {
		$this->baseUrl = $url;
		if (!empty($url) && (str_ends_with($url, '/'))) {
			$this->baseUrl = substr($url, 0, -1);
		}
	}

	/**
	 * Returns the login handler.
	 *
	 * @return SelfServiceLoginHandler handler
	 */
	public function getLoginHandler(): SelfServiceLoginHandler {
		return match ($this->loginHandler) {
			SelfServiceHttpAuthLoginHandler::ID => new SelfServiceHttpAuthLoginHandler($this),
			SelfService2FaLoginHandler::ID => new SelfService2FaLoginHandler($this),
			default => new SelfServiceUserPasswordLoginHandler($this),
		};
	}

}

/**
 * LDAP connection for self-service.
 */
class SelfServiceLdapConnection {

	private Connection|null $server = null;

	private selfServiceProfile $profile;

	/**
	 * Constructor
	 *
	 * @param selfServiceProfile $profile profile
	 */
	public function __construct(selfServiceProfile $profile) {
		$this->profile = $profile;
	}

	/**
	 * Returns the LDAP handle.
	 *
	 * @return Connection|null LDAP handle
	 */
	public function getServer(): ?Connection {
		if ($this->server !== null) {
			return $this->server;
		}
		$this->server = openSelfServiceLdapConnection($this->profile);
		return $this->server;
	}

	/**
	 * Binds with the given credentials.
	 *
	 * @param string $userDn user DN
	 * @param string $password password
	 * @return bool bind successful
	 */
	public function bind(string $userDn, string $password): bool {
		return bindLdapUser($this->getServer(), $this->profile, $userDn, $password);
	}

	/**
	 * Closes connection to LDAP server before serialization
	 */
	public function __sleep(): array {
		if ($this->server !== null) {
			@ldap_close($this->server);
			$this->server = null;
		}
		// define which attributes to save
		return ['profile'];
	}

}

/**
 * Login handler for self-service
 */
interface SelfServiceLoginHandler {

	/**
	 * Returns a unique ID for the handler.
	 *
	 * @return string ID
	 */
	function getId(): string;

	/**
	 * Adds necessary fields to the login dialog (e.g. user + password).
	 *
	 * @param htmlResponsiveRow $content dialog content
	 */
	function addLoginFields(htmlResponsiveRow $content): void;

	/**
	 * Returns the login name.
	 *
	 * @return string login name
	 */
	function getLoginName(): string;

	/**
	 * Returns the login password.
	 *
	 * @return string password
	 */
	function getLoginPassword(): string;

	/**
	 * Returns if the login handler manages authentication on its own.
	 *
	 * @return bool manages authentication
	 */
	function managesAuthentication(): bool;

	/**
	 * Returns if the authentication was successful.
	 * Only valid if managesAuthentication() returns true.
	 *
	 * @return bool authentication successful
	 * @throws LAMException error during authentication
	 */
	function isAuthenticationSuccessful(): bool;

	/**
	 * Authorizes an user that was provided by 2FA provider.
	 *
	 * @param string $userName user name
	 * @throws LAMException error during authentication
	 */
	function authorize2FaUser(string $userName): void;

}

/**
 * Performs login with user and password.
 */
class SelfServiceUserPasswordLoginHandler implements SelfServiceLoginHandler {

	public const ID = "user_password";

	private selfServiceProfile $profile;

	/**
	 * Constructor
	 *
	 * @param selfServiceProfile $profile profile
	 */
	function __construct(selfServiceProfile $profile) {
		$this->profile = $profile;
	}

	/**
	 * @inheritDoc
	 */
	function getId(): string {
		return self::ID;
	}

	/**
	 * @inheritDoc
	 */
	function addLoginFields(htmlResponsiveRow $content): void {
		// user name
		$userNameVal = empty($_POST['username']) ? '' : $_POST['username'];
		$userField = new htmlResponsiveInputField($this->profile->loginAttributeText, 'username', $userNameVal);
		$userField->setCSSClasses(['lam-initial-focus']);
		$content->add($userField);
		// password field
		$passwordText = _("Password");
		if (!empty($this->profile->passwordLabel)) {
			$passwordText = $this->profile->passwordLabel;
		}
		$passwordVal = empty($_POST['password']) ? '' : $_POST['password'];
		$passwordField = new htmlResponsiveInputField($passwordText, 'password', $passwordVal);
		$passwordField->setIsPassword(true);
		$content->add($passwordField);
	}

	/**
	 * @inheritDoc
	 */
	function getLoginName(): string {
		return $_POST['username'];
	}

	/**
	 * @inheritDoc
	 */
	function getLoginPassword(): string {
		return $_POST['password'];
	}

	/**
	 * @inheritDoc
	 */
	function managesAuthentication(): bool {
		return false;
	}

	/**
	 * @inheritDoc
	 */
	function isAuthenticationSuccessful(): bool {
		throw new LAMException('Not implemented');
	}

	/**
	 * @inheritDoc
	 */
	function authorize2FaUser(string $userName): void {
		// no action
	}

}

/**
 * Performs login with HTTP authentication.
 */
class SelfServiceHttpAuthLoginHandler implements SelfServiceLoginHandler {

	public const ID = "http_auth";

	private selfServiceProfile $profile;

	/**
	 * Constructor
	 *
	 * @param selfServiceProfile $profile profile
	 */
	function __construct(selfServiceProfile $profile) {
		$this->profile = $profile;
	}

	/**
	 * @inheritDoc
	 */
	function getId(): string {
		return self::ID;
	}

	/**
	 * @inheritDoc
	 */
	function addLoginFields(htmlResponsiveRow $content): void {
		// user name
		$userLabel = new htmlLabel('username', $this->profile->loginAttributeText);
		$content->addLabel($userLabel);
		$userName = empty($_SERVER['PHP_AUTH_USER']) ? '-' : $_SERVER['PHP_AUTH_USER'];
		$userField = new htmlOutputText($userName);
		$content->addField($userField);
		// password field
		$passwordText = _("Password");
		if (!empty($this->profile->passwordLabel)) {
			$passwordText = $this->profile->passwordLabel;
		}
		$passwordLabel = new htmlLabel('password', $passwordText);
		$content->addLabel($passwordLabel);
		$passwordField = new htmlOutputText('**********');
		$content->addField($passwordField);
	}

	/**
	 * @inheritDoc
	 */
	function getLoginName(): string {
		return $_SERVER['PHP_AUTH_USER'];
	}

	/**
	 * @inheritDoc
	 */
	function getLoginPassword(): string {
		return $_SERVER['PHP_AUTH_PW'];
	}

	/**
	 * @inheritDoc
	 */
	function managesAuthentication(): bool {
		return false;
	}

	/**
	 * @inheritDoc
	 */
	function isAuthenticationSuccessful(): bool {
		throw new LAMException('Not implemented');
	}

	/**
	 * @inheritDoc
	 */
	function authorize2FaUser(string $userName): void {
		// no action
	}

}

/**
 * Performs login with pure 2FA.
 */
class SelfService2FaLoginHandler implements SelfServiceLoginHandler {

	public const ID = "2fa";

	private selfServiceProfile $profile;

	/**
	 * Constructor
	 *
	 * @param selfServiceProfile $profile profile
	 */
	function __construct(selfServiceProfile $profile) {
		$this->profile = $profile;
	}

	/**
	 * @inheritDoc
	 */
	function getId(): string {
		return self::ID;
	}

	/**
	 * @inheritDoc
	 */
	function addLoginFields(htmlResponsiveRow $content): void {
		// no input fields
	}

	/**
	 * @inheritDoc
	 */
	function getLoginName(): string {
		return '';
	}

	/**
	 * @inheritDoc
	 */
	function getLoginPassword(): string {
		return '';
	}

	/**
	 * @inheritDoc
	 */
	function managesAuthentication(): bool {
		return true;
	}

	/**
	 * @inheritDoc
	 */
	function isAuthenticationSuccessful(): bool {
		if (($this->profile->twoFactorAuthentication !== TwoFactorProviderService::TWO_FACTOR_OKTA)
			&& ($this->profile->twoFactorAuthentication !== TwoFactorProviderService::TWO_FACTOR_OPENID)) {
			logNewMessage(LOG_ERR, 'Unsupported 2FA provider: ' . $this->profile->twoFactorAuthentication);
			return false;
		}
		if (!$this->profile->useForAllOperations) {
			logNewMessage(LOG_ERR, 'Use for all operations must be set');
			return false;
		}
		// authentication will be checked on 2FA page
		return true;
	}

	/**
	 * @inheritDoc
	 */
	function authorize2FaUser(string $userName): void {
		$bindUser = $this->profile->LDAPUser;
		if (empty($bindUser)) {
			throw new LAMException('SelfService2FaLoginHandler', 'No bind user set');
		}
		$bindPassword = deobfuscateText($this->profile->LDAPPassword);
		$server = connectToLDAP($this->profile->serverURL, $this->profile->useTLS);
		if ($server === null) {
			throw new LAMException('SelfService2FaLoginHandler', 'Unable to match provided user with LDAP entry - LDAP connect failed');
		}
		ldap_set_option($server, LDAP_OPT_REFERRALS, $this->profile->followReferrals);
		$bind = @ldap_bind($server, $bindUser, $bindPassword);
		if (!$bind) {
			throw new LAMException('SelfService2FaLoginHandler', 'Unable to match provided user with LDAP entry - LDAP bind failed');
		}
		$filter = '(' . $this->profile->twoFactorAuthenticationAttribute . "=" . ldap_escape($userName, '', LDAP_ESCAPE_FILTER) . ')';
		if (!empty($this->profile->additionalLDAPFilter)) {
			$filter = '(&' . $filter . $this->profile->additionalLDAPFilter . ')';
		}
		$result = @ldap_search($server, $this->profile->LDAPSuffix, $filter, ['DN'], 0, 1, 0, LDAP_DEREF_NEVER);
		if ($result === false) {
			throw new LAMException('SelfService2FaLoginHandler', 'Unable to match provided user with LDAP entry - LDAP search failed');
		}
		$entries = @ldap_get_entries($server, $result);
		if ($entries === false) {
			throw new LAMException('SelfService2FaLoginHandler', 'Unable to match provided user with LDAP entry - LDAP search failed');
		}
		$info = $entries;
		cleanLDAPResult($info);
		if (count($info) === 1) {
			$userDN = $info[0]['dn'];
			$_SESSION['selfService_clientDN'] = lamEncrypt($userDN, 'SelfService');
			return;
		}
		throw new LAMException('SelfService2FaLoginHandler', 'Multiple or no results for ' . $userName . ' ' . print_r($info, true));
	}

}
