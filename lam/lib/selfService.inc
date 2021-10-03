<?php
use \LAM\LIB\TWO_FACTOR\TwoFactorProviderService;
use LAM\PERSISTENCE\ConfigurationDatabase;
use function LAM\PERSISTENCE\dbTableExists;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2006 - 2021  Roland Gruber

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
* Interface between modules and self service pages.
* This file also includes the self service profile class and helper functions.
*
* @package selfService
* @author Roland Gruber
*/

/** modules */
include_once "modules.inc";
/** account types */
include_once "types.inc";
/** 2-factor */
include_once '2factor.inc';

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
 * Returns a list of possible search attributes for the self service.
 *
 * @param string $scope account type
 * @return array attributes
 */
function getSelfServiceSearchAttributes($scope) {
	$return = array();
	$modules = getAvailableModules($scope);
	for ($i = 0; $i < sizeof($modules); $i++) {
		$m = moduleCache::getModule($modules[$i], $scope);
		$attributes = $m->getSelfServiceSearchAttributes();
		$return = array_merge($return, $attributes);
	}
	$return = array_unique($return);
	return array_values($return);
}

/**
 * Returns the field settings for the self service.
 *
 * @param string $scope account type
 * @return array settings
 */
function getSelfServiceFieldSettings($scope) {
	$return = array();
	$modules = getAvailableModules($scope);
	for ($i = 0; $i < sizeof($modules); $i++) {
		$m = moduleCache::getModule($modules[$i], $scope);
		$settings = $m->getSelfServiceFields();
		if (sizeof($settings) > 0) {
			$return[$modules[$i]] = $settings;
		}
	}
	return $return;
}

/**
 * Returns meta HTML code for each self service field.
 *
 * @param string $scope account type
 * @param array $fields input fields (array(<moduleName> => array(<field1>, <field2>, ...)))
 * @param array $attributes LDAP attributes (attribute names in lower case)
 * @param boolean $passwordChangeOnly indicates that the user is only allowed to change his password and no LDAP content is readable
 * @param array $readOnlyFields list of read-only fields
 * @return array meta HTML code (array(<moduleName> => htmlResponsiveRow))
 */
function getSelfServiceOptions($scope, $fields, $attributes, $passwordChangeOnly, $readOnlyFields) {
	$return = array();
	$modules = getAvailableModules($scope);
	for ($i = 0; $i < sizeof($modules); $i++) {
		if (!isset($fields[$modules[$i]])) {
			continue;
		}
		$m = moduleCache::getModule($modules[$i], $scope);
		$modReadOnlyFields = array();
		for ($r = 0; $r < sizeof($readOnlyFields); $r++) {
			$parts = explode('_', $readOnlyFields[$r]);
			if ($parts[0] == $modules[$i]) {
				$modReadOnlyFields[] = $parts[1];
			}
		}
		$code = $m->getSelfServiceOptions($fields[$modules[$i]], $attributes, $passwordChangeOnly, $modReadOnlyFields);
		if (sizeof($code) > 0) {
			$return[$modules[$i]] = $code;
		}
	}
	return $return;
}

/**
 * Checks if all input values are correct and returns the LDAP commands which should be executed.
 *
 * @param string $scope account type
 * @param string $fields input fields (array(<moduleName> => array(<field1>, <field2>, ...)))
 * @param array $attributes LDAP attributes
 * @param boolean $passwordChangeOnly indicates that the user is only allowed to change his password and no LDAP content is readable
 * @param array $readOnlyFields list of read-only fields
 * @return array messages and LDAP commands (array('messages' => array(), 'add' => array(), 'del' => array(), 'mod' => array()))
 */
function checkSelfServiceOptions($scope, $fields, $attributes, $passwordChangeOnly, $readOnlyFields) {
	$return = array('messages' => array(), 'add' => array(), 'del' => array(), 'mod' => array(), 'info' => array());
	$modules = getAvailableModules($scope);
	for ($i = 0; $i < sizeof($modules); $i++) {
		if (!isset($fields[$modules[$i]])) {
			continue;
		}
		$m = moduleCache::getModule($modules[$i], $scope);
		$modReadOnlyFields = array();
		for ($r = 0; $r < sizeof($readOnlyFields); $r++) {
			$parts = explode('_', $readOnlyFields[$r]);
			if ($parts[0] == $modules[$i]) {
				$modReadOnlyFields[] = $parts[1];
			}
		}
		$result = $m->checkSelfServiceOptions($fields[$modules[$i]], $attributes, $passwordChangeOnly, $modReadOnlyFields);
		if (sizeof($result['messages']) > 0) {
			$return['messages'] = array_merge($result['messages'], $return['messages']);
		}
		if (sizeof($result['add']) > 0) {
			$return['add'] = array_merge($result['add'], $return['add']);
		}
		if (sizeof($result['del']) > 0) {
			$return['del'] = array_merge($result['del'], $return['del']);
		}
		if (sizeof($result['mod']) > 0) {
			$return['mod'] = array_merge($result['mod'], $return['mod']);
		}
		if (sizeof($result['info']) > 0) {
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
* @return array configuration options
*/
function getSelfServiceSettings($scope, $profile) {
	$return = array();
	$modules = getAvailableModules($scope);
	for ($i = 0; $i < sizeof($modules); $i++) {
		$m = moduleCache::getModule($modules[$i], $scope);
		$return[$modules[$i]] = $m->getSelfServiceSettings($profile);
	}
	return $return;
}

/**
* Checks if the self service settings are valid
*
* @param string $scope account type
* @param array $options hash array containing all options (name => array(...))
* @param selfServiceProfile $profile profile
* @return array list of error messages
*/
function checkSelfServiceSettings($scope, &$options, &$profile) {
	$return = array();
	$modules = getAvailableModules($scope);
	for ($i = 0; $i < sizeof($modules); $i++) {
		$m = moduleCache::getModule($modules[$i], $scope);
		$errors = $m->checkSelfServiceSettings($options, $profile);
		$return = array_merge($return, $errors);
	}
	return $return;
}

/**
 * Returns if script runs inside self service.
 *
 * @return boolean is self service
 */
function isSelfService() {
	return session_name() == 'SELFSERVICE';
}

/**
 * Opens the LDAP connection and returns the handle. No bind is done.
 *
 * @param selfServiceProfile $profile profile
 * @return handle LDAP handle or null if connection failed
 */
function openSelfServiceLdapConnection($profile) {
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
 * @param handle $handle LDAP handle
 * @param selfServiceProfile profile
 * @param string $userDn bind DN
 * @param string $password bind password
 * @return boolean binding successful
 */
function bindLdapUser($handle, $profile, $userDn, $password) {
	if ($profile->useForAllOperations) {
		$userDn = $profile->LDAPUser;
		$password = deobfuscateText($profile->LDAPPassword);
	}
	return @ldap_bind($handle, $userDn, $password);
}

/**
 * Manages reading and writing self service profiles.
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
				logNewMessage(LOG_ERR, _('Unable to connect to configuration database.'), $e->getMessage());
				throw new LAMException(_('Unable to connect to configuration database.'));
			}
		}
		else {
			$this->strategy = new SelfServicePersistenceStrategyFileSystem();
		}
	}

	/**
	 * Returns a list of available self service profiles.
	 *
	 * @return array profile names (array('account type' => array('profile1', 'profile2')))
	 */
	public function getProfiles() : array {
		$profiles = $this->strategy->getProfiles();
		ksort($profiles);
		foreach ($profiles as $key => $value) {
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
	public function canWrite(string $name, string $scope) : bool {
		return $this->strategy->canWrite($name, $scope);
	}

	/**
	 * Loads the given self service profile.
	 *
	 * @param string $name profile name
	 * @param string $scope user/group/host
	 * @return selfServiceProfile profile
	 * @throws LAMException error during loading
	 */
	public function load(string $name, string $scope) : selfServiceProfile {
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
	public function save(string $name, string $scope, selfServiceProfile $profile) : void {
		$this->strategy->save($name, $scope, $profile);
	}

	/**
	 * Deletes a self service profile.
	 *
	 * @param string $name profile name
	 * @param string $scope user/group/host
	 * @throws LAMException error deleting profile
	 */
	public function delete(string $name, string $scope) : void {
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
	 * Renames a self service profile.
	 *
	 * @param string $oldName existing profile name
	 * @param string $newName new profile name
	 * @param string $scope user/group/host
	 * @throws LAMException error renaming profile
	 */
	public function rename(string $oldName, string $newName, string $scope) : void {
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
 * Interface for self service profile persistence.
 */
interface SelfServicePersistenceStrategy {

	/**
	 * Returns a list of available self service profiles.
	 *
	 * @return array profile names (array('account type' => array('profile1', 'profile2')))
	 */
	public function getProfiles() : array;

	/**
	 * Returns if the profile with given name can be written.
	 *
	 * @param string $name profile name
	 * @param string $scope user/group/host
	 * @return bool can be written
	 */
	public function canWrite(string $name, string $scope) : bool;

	/**
	 * Loads the given self service profile.
	 *
	 * @param string $name profile name
	 * @param string $scope user/group/host
	 * @return selfServiceProfile profile
	 * @throws LAMException error during loading
	 */
	public function load(string $name, string $scope) : selfServiceProfile;

	/**
	 * Stores the given profile.
	 *
	 * @param string $name profile name
	 * @param string $scope user/group/host
	 * @param selfServiceProfile $profile profile
	 * @throws LAMException error during saving
	 */
	public function save(string $name, string $scope, selfServiceProfile $profile) : void;

	/**
	 * Deletes a self service profile.
	 *
	 * @param string $name profile name
	 * @param string $scope account type
	 * @throws LAMException error renaming profile
	 */
	public function delete(string $name, string $scope) : void;

	/**
	 * Renames a self service profile.
	 *
	 * @param string $oldName existing profile name
	 * @param string $newName new profile name
	 * @param string $scope user/group/host
	 * @throws LAMException error renaming profile
	 */
	public function rename(string $oldName, string $newName, string $scope) : void;

}

/**
 * Uses local file system for storing self service profiles.
 */
class SelfServicePersistenceStrategyFileSystem implements SelfServicePersistenceStrategy {

	/**
	 * @inheritDoc
	 */
	public function getProfiles(): array {
		$types = LAM\TYPES\getTypes();
		$dir = dir(__DIR__ . "/../config/selfService");
		$ret = array();
		if ($dir === false) {
			logNewMessage(LOG_ERR, 'Unable to read self service profiles');
			return $ret;
		}
		while ($entry = $dir->read()){
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
		if (is_file($file) === True) {
			$file = @fopen($file, "r");
			if ($file) {
				$data = fread($file, 10000000);
				$profileData = @json_decode($data, true);
				if ($profileData === null) {
					$profile = unserialize($data);
				}
				else {
					$profile = selfServiceProfile::import($profileData);
				}
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
		if (!get_class($profile) === 'selfServiceProfile') {
			throw new LAMException('Invalid data');
		}
		$path = __DIR__ . "/../config/selfService/" . $name . "." . $scope;
		$file = @fopen($path, "w");
		if ($file) {
			// write settings to file
			fputs($file, json_encode($profile->export()));
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
 * Uses PDO for storing self service profiles.
 */
class SelfServicePersistenceStrategyPdo implements SelfServicePersistenceStrategy {

	/**
	 * @var PDO
	 */
	private $pdo;

	const TABLE_NAME = 'self_service_profiles';

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
		$profiles = array();
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
		$statement->execute(array($scope, $name));
		$results = $statement->fetchAll();
		if (empty($results)) {
			logNewMessage(LOG_ERR, 'Self service profile not found');
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
		$statement->execute(array($scope, $name));
		$results = $statement->fetchAll();
		if (empty($results)) {
			$statement = $this->pdo->prepare("INSERT INTO " . self::TABLE_NAME . " (scope, name, data) VALUES (?, ?, ?)");
			$statement->execute(array($scope, $name, $data));
		}
		else {
			$statement = $this->pdo->prepare("UPDATE " . self::TABLE_NAME . " SET data = ? WHERE scope = ? AND name = ?");
			$statement->execute(array($data, $scope, $name));
		}
	}

	/**
	 * @inheritDoc
	 */
	public function delete(string $name, string $scope): void {
		$statement = $this->pdo->prepare("DELETE FROM " . self::TABLE_NAME . " WHERE scope = ? AND name = ?");
		$statement->execute(array($scope, $name));
	}

	/**
	 * @inheritDoc
	 */
	public function rename(string $oldName, string $newName, string $scope): void {
		$statement = $this->pdo->prepare("UPDATE " . self::TABLE_NAME . " SET name = ? WHERE scope = ? AND name = ?");
		$statement->execute(array($newName, $scope, $oldName));
	}

}

/**
 * Includes all settings of a self service profile.
 *
 * @package selfService
 */
class selfServiceProfile {

	/** server address */
	public $serverURL;

	/** use TLS */
	public $useTLS;

	/** LDAP suffix */
	public $LDAPSuffix;

	/** LDAP user DN*/
	public $LDAPUser;

	/** LDAP password */
	public $LDAPPassword;

	/** use bind user also for read/modify operations */
	public $useForAllOperations;

	/** LDAP search attribute */
	public $searchAttribute;

	/** HTTP authentication */
	public $httpAuthentication;

	/** header for self service pages */
	public $pageHeader;

	/** base color */
	public $baseColor = '#fffde2';

	/** list of additional CSS links (separated by \n) */
	public $additionalCSS;

	/** describing text for user login */
	public $loginCaption;

	/** describing text for user login */
	public $loginFooter;

	/** label for password input */
	public $passwordLabel;

	/** describing text for search attribute */
	public $loginAttributeText;

	/** additional LDAP filter for accounts */
	public $additionalLDAPFilter;

	/** describing text for self service main page */
	public $mainPageText;

	/** describing text for self service main page */
	public $mainPageFooter;

	/** input fields
	 * Format: array(
	 * <br> array(array('name' => <group name 1>, 'fields' => array(<field1>, <field2>))),
	 * <br> array(array('name' => <group name 2>, 'fields' => array(<field3>, <field4>)))
	 * <br> )
	 *
	 */
	public $inputFields;

	/**
	 * List of fields that are set in read-only mode.
	 */
	public $readOnlyFields;

	/** List of override values for field labels: array(<field ID> => label) */
	public $relabelFields;

	/** configuration settings of modules */
	public $moduleSettings;

	/** language for self service */
	public $language = 'en_GB.utf8';
	/** disallow user to change language */
	public $enforceLanguage = false;

	public $followReferrals = 0;

	public $timeZone = 'Europe/London';

	public $twoFactorAuthentication = TwoFactorProviderService::TWO_FACTOR_NONE;
	public $twoFactorAuthenticationURL = 'https://localhost';
	public $twoFactorAuthenticationInsecure = false;
	public $twoFactorAuthenticationLabel = null;
	public $twoFactorAuthenticationOptional = false;
	public $twoFactorAuthenticationCaption = '';
	public $twoFactorAuthenticationClientId = '';
	public $twoFactorAuthenticationSecretKey = '';
	public $twoFactorAuthenticationAttribute = 'uid';
	public $twoFactorAuthenticationDomain = '';

	/** provider for captcha (-/google) */
	public $captchaProvider = '-';
	/** Google reCAPTCHA site key */
	public $reCaptchaSiteKey = '';
	/** Google reCAPTCHA secret key */
	public $reCaptchaSecretKey = '';
	/** enable captcha on self service login */
	public $captchaOnLogin = false;

	/** base URL for the website (e.g. https://example.com) for link generation */
	private $baseUrl = '';

	/**
	 * Path to external lamdaemon script on server where it is executed
	 *
	 * This is used for managing quota and home directories.
	 * optional setting, may not be defined
	 */
	public $scriptPath;

	/**
	 * The rights for the home directory
	 */
	public $scriptRights = '750';

	/**
	 * Servers where lamdaemon script is executed
	 *
	 * This is used for managing quota and home directories.
	 * optional setting, may not be defined
	 */
	public $scriptServer;

	/**
	 * user name for lamdaemon
	 */
	public $scriptUserName;

	/**
	 * File name of SSH key for lamdaemon.
	 */
	public $scriptSSHKey;

	/**
	 * Password for lamdaemon SSH key.
	 */
	public $scriptSSHKeyPassword;

	/**
	 * Constructor
	 *
	 * @return selfServiceProfile
	 */
	function __construct() {
		// set default values
		$this->serverURL = "localhost";
		$this->useTLS = false;
		$this->LDAPSuffix = "dc=my-domain,dc=com";
		$this->LDAPUser = "";
		$this->LDAPPassword = "";
		$this->useForAllOperations = false;
		$this->searchAttribute = "uid";
		$this->additionalLDAPFilter = '';
		$this->httpAuthentication = false;
		$this->pageHeader = '<p><a href="https://www.ldap-account-manager.org/" target="new_window"><img alt="help" class="align-middle" src="../../graphics/logo24.png" style="height:24px; width:24px" /> LDAP Account Manager </a></p><p>&nbsp;</p>';
		$this->additionalCSS = '';
		$this->baseColor = '#fffde2';
		$this->loginCaption = '<b>' . _("Welcome to LAM self service. Please enter your user name and password.") . '</b>';
		$this->loginAttributeText = _('User name');
		$this->passwordLabel = '';
		$this->mainPageText = "<h1>LAM self service</h1>\n" . _("Here you can change your personal settings.");
		$this->inputFields = array(
			array('name' => _('Personal data'),
				'fields' => array('inetOrgPerson_firstName', 'inetOrgPerson_lastName', 'inetOrgPerson_mail',
					'inetOrgPerson_telephoneNumber', 'inetOrgPerson_mobile', 'inetOrgPerson_faxNumber',
					'inetOrgPerson_street', 'inetOrgPerson_postalAddress')),
			array('name' => _('Password'),
				'fields' => array('posixAccount_password'))
		);
		$this->readOnlyFields = array();
		$this->relabelFields = array();
		$this->moduleSettings = array();
		$this->language = 'en_GB.utf8';
		$this->enforceLanguage = true;
		$this->followReferrals = 0;
		$this->timeZone = 'Europe/London';
		$this->twoFactorAuthentication = TwoFactorProviderService::TWO_FACTOR_NONE;
		$this->twoFactorAuthenticationURL = 'https://localhost';
		$this->twoFactorAuthenticationInsecure = false;
		$this->twoFactorAuthenticationLabel = null;
		$this->twoFactorAuthenticationOptional = false;
		$this->twoFactorAuthenticationCaption = '';
		$this->twoFactorAuthenticationClientId = '';
		$this->twoFactorAuthenticationSecretKey = '';
		$this->twoFactorAuthenticationAttribute = 'uid';
		$this->twoFactorAuthenticationDomain = '';
		$this->captchaProvider = '-';
		$this->reCaptchaSiteKey = '';
		$this->reCaptchaSecretKey = '';
		$this->captchaOnLogin = false;
		$this->baseUrl = '';
	}

	/**
	 * Converts the export data back to a self service profile.
	 *
	 * @param array $data export data
	 * @return selfServiceProfile profile
	 */
	public static function import($data) {
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
	 * Returns a representation of the self service profile.
	 *
	 * @return array self service profile data
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
		$matches = array();
		if (preg_match('/^(http(s)?:\\/\\/[^\\/]+)\\/.+$/', $callingUrl, $matches)) {
			return $matches[1];
		}
	}

	/**
	 * Sets the server's base URL (e.g. https://www.example.com).
	 *
	 * @param string $url URL
	 */
	public function setBaseUrl($url) {
		$this->baseUrl = $url;
		if (!empty($url) && (substr($url, -1, 1) === '/')) {
			$this->baseUrl = substr($url, 0, -1);
		}
	}

}
