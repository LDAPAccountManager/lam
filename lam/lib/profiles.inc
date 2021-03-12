<?php
namespace LAM\PROFILES;
use LAM\PERSISTENCE\ConfigurationDatabase;
use LAM\TYPES\TypeManager;
use LAMCfgMain;
use LAMConfig;
use \LAMException;
use PDO;
use ServerProfilePersistenceManager;
use function LAM\PERSISTENCE\dbTableExists;
use function PHPUnit\Framework\isEmpty;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2021  Roland Gruber

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
* This file provides functions to load and save account profiles.
*
* @package profiles
* @author Roland Gruber
*/

/**
 * Manages the persistence of account profiles.
 *
 * @package LAM\PROFILES
 */
class AccountProfilePersistenceManager {

	/**
	 * @var AccountProfilePersistenceStrategy
	 */
	private $strategy;

	/**
	 * Constructor
	 */
	public function __construct() {
		$configDb = new ConfigurationDatabase(new LAMCfgMain());
		if ($configDb->useRemoteDb()) {
			$this->strategy = new AccountProfilePersistenceStrategyPdo($configDb->getPdo());
		}
		else {
			$this->strategy = new AccountProfilePersistenceStrategyFiles();
		}
	}

	/**
	 * Returns the names of existing account profile templates.
	 *
	 * @return array scope => names (e.g. array('user' => array('default')))
	 * @throws LAMException error reading templates
	 */
	public function getAccountProfileTemplateNames() : array {
		return $this->strategy->getAccountProfileTemplateNames();
	}

	/**
	 * Deletes an account profile template.
	 *
	 * @param string $scope user/group/host
	 * @param string $name account profile name
	 * @throws LAMException error deleting template
	 */
	public function deleteAccountProfileTemplate(string $scope, string $name) : void {
		if (!$this->isValidAccountProfileName($name) || !TypeManager::isValidTypeId($scope) || ($name == 'default')) {
			logNewMessage(LOG_NOTICE, "Invalid account profile name: $name:$scope");
			throw new LAMException(_("Unable to delete profile!"));
		}
		$this->strategy->deleteAccountProfileTemplate($scope, $name);
	}

	/**
	 * Reads an account profile template.
	 *
	 * @param string $scope
	 * @param string $name
	 * @return array
	 * @throws LAMException error reading template
	 */
	public function loadAccountProfileTemplate(string $scope, string $name) : array {
		if (!$this->isValidAccountProfileName($name) || !TypeManager::isValidTypeId($scope)) {
			logNewMessage(LOG_NOTICE, "Invalid account profile name: $name:$scope");
			throw new LAMException(_("Unable to load profile!"));
		}
		return $this->strategy->loadAccountProfileTemplate($scope, $name);
	}

	/**
	 * Writes an account profile template.
	 *
	 * @param string $scope
	 * @param string $name
	 * @param array $data profile data
	 * @throws LAMException error writing template
	 */
	public function writeAccountProfileTemplate(string $scope, string $name, array $data) : void {
		if (!$this->isValidAccountProfileName($name) || !TypeManager::isValidTypeId($scope)) {
			logNewMessage(LOG_NOTICE, "Invalid account profile name: $name:$scope");
			throw new LAMException(_('Unable to write account profile template.'));
		}
		$this->strategy->writeAccountProfileTemplate($scope, $name, $data);
	}

	/**
	 * Installs templates to the given server profile.
	 *
	 * @param string $confName server profile name
	 * @throws LAMException
	 */
	public function installAccountProfileTemplates(string $confName) {
		if (!LAMConfig::isValidName($confName)) {
			logNewMessage(LOG_NOTICE, "Invalid server profile name: $confName");
			throw new LAMException(_("Unable to write account profile template."));
		}
		$allTemplates = $this->getAccountProfileTemplateNames();
		$serverProfilesPersistenceManager = new ServerProfilePersistenceManager();
		$config = $serverProfilesPersistenceManager->loadProfile($confName);
		$typeManager = new TypeManager($config);
		foreach ($typeManager->getConfiguredTypes() as $type) {
			if (empty($allTemplates[$type->getScope()])) {
				continue;
			}
			foreach ($allTemplates[$type->getScope()] as $templateName) {
				if ($this->isAccountProfileExisting($type->getId(), $templateName, $confName)) {
					continue;
				}
				$data = $this->loadAccountProfileTemplate($type->getScope(), $templateName);
				$this->writeAccountProfile($type->getId(), $templateName, $confName, $data);
			}
		}
	}

	/**
	 * Returns all account profile names for the given type id.
	 *
	 * @param string $typeId type ID
	 * @param string $confName server profile name
	 * @return array names
	 * @throws LAMException error reading profiles
	 */
	public function getAccountProfileNames(string $typeId, string $confName) : array {
		if (!TypeManager::isValidTypeId($typeId) || !LAMConfig::isValidName($confName)) {
			logNewMessage(LOG_NOTICE, "Invalid account profile name: $confName:$typeId");
			throw new LAMException(_("Unable to read profiles."));
		}
		return $this->strategy->getAccountProfileNames($typeId, $confName);
	}

	/**
	 * Writes an account profile.
	 *
	 * @param $typeId $scope
	 * @param string $name account profile name
	 * @param string $confName server profile name
	 * @param array $data profile data
	 * @throws LAMException error writing template
	 */
	public function writeAccountProfile(string $typeId, string $name, string $confName, array $data) : void {
		if (!$this->isValidAccountProfileName($name)
			|| !TypeManager::isValidTypeId($typeId)
			|| !LAMConfig::isValidName($confName)) {
			logNewMessage(LOG_NOTICE, "Invalid account profile name: $name:$typeId");
			throw new LAMException(_("Unable to save profile!"));
		}
		$this->strategy->writeAccountProfile($typeId, $name, $confName, $data);
	}

	/**
	 * Returns if the given profile name is valid.
	 *
	 * @param string $name profile name
	 * @return bool is valid profile
	 */
	private function isValidAccountProfileName(string $name) : bool {
		return preg_match("/^[0-9a-z _-]+$/i", $name) === 1;
	}

	/**
	 * Checks if the given profile is already existing.
	 *
	 * @param string $typeId user/group/host
	 * @param string $name profile name
	 * @param string $confName server profile name
	 * @throws LAMException error while checking
	 * @return bool profile exists
	 * @throws LAMException error writing template
	 */
	public function isAccountProfileExisting(string $typeId, string $name, string $confName) : bool {
		if (!$this->isValidAccountProfileName($name)
			|| !TypeManager::isValidTypeId($typeId)
			|| !LAMConfig::isValidName($confName)) {
			logNewMessage(LOG_NOTICE, "Invalid account profile name: $name:$typeId");
			return false;
		}
		return $this->strategy->isAccountProfileExisting($typeId, $name, $confName);
	}

	/**
	 * Loads an account profile.
	 *
	 * @param string $typeId user/group/host
	 * @param string $name account profile name
	 * @param string $confName server profile name
	 * @return array profile data
	 * @throws LAMException error writing template
	 */
	public function loadAccountProfile(string $typeId, string $name, string $confName) : array {
		if (!$this->isValidAccountProfileName($name)
			|| !TypeManager::isValidTypeId($typeId)
			|| !LAMConfig::isValidName($confName)) {
			logNewMessage(LOG_NOTICE, "Invalid account profile name: $name:$typeId");
			throw new LAMException(_("Unable to load profile!"));
		}
		return $this->strategy->loadAccountProfile($typeId, $name, $confName);
	}

	/**
	 * Deletes an account profile.
	 *
	 * @param string $typeId user/group/host
	 * @param string $name account profile name
	 * @param string $confName server profile name
	 * @throws LAMException error deleting profile
	 */
	public function deleteAccountProfile(string $typeId, string $name, string $confName) : void {
		if (!isLoggedIn() || !$this->isValidAccountProfileName($name)
			|| !TypeManager::isValidTypeId($typeId)
			|| !LAMConfig::isValidName($confName)) {
			logNewMessage(LOG_NOTICE, "Invalid account profile name: $name:$typeId");
			throw new LAMException(_('Unable to delete profile!'));
		}
		$this->strategy->deleteAccountProfile($typeId, $name, $confName);
	}

}

/**
 * Reads and writes account profiles.
 *
 * @package LAM\PROFILES
 */
interface AccountProfilePersistenceStrategy {

	/**
	 * Returns the names of existing account profile templates.
	 *
	 * @return array scope => names (e.g. array('user' => array('default')))
	 * @throws LAMException error reading templates
	 */
	public function getAccountProfileTemplateNames() : array;

	/**
	 * Deletes an account profile template.
	 *
	 * @param string $scope user/group/host
	 * @param string $name account profile name
	 * @throws LAMException error deleting template
	 */
	public function deleteAccountProfileTemplate(string $scope, string $name) : void;

	/**
	 * Reads an account profile template.
	 *
	 * @param string $scope
	 * @param string $name
	 * @return array
	 * @throws LAMException error reading template
	 */
	public function loadAccountProfileTemplate(string $scope, string $name) : array;

	/**
	 * Writes an account profile template.
	 *
	 * @param string $scope
	 * @param string $name
	 * @param array $data profile data
	 * @throws LAMException error writing template
	 */
	public function writeAccountProfileTemplate(string $scope, string $name, array $data) : void;

	/**
	 * Checks if the given profile is already existing.
	 *
	 * @param string $typeId user/group/host
	 * @param string $name profile name
	 * @param string $confName server profile name
	 * @throws LAMException error while checking
	 * @return bool profile exists
	 * @throws LAMException error writing template
	 */
	public function isAccountProfileExisting(string $typeId, string $name, string $confName) : bool;

	/**
	 * Returns all account profile names for the given type id.
	 *
	 * @param string $typeId type ID
	 * @param string $confName server profile name
	 * @return array names
	 * @throws LAMException error reading profiles
	 */
	public function getAccountProfileNames(string $typeId, string $confName) : array;

	/**
	 * Writes an account profile.
	 *
	 * @param $typeId $scope
	 * @param string $name
	 * @param string $confName server profile name
	 * @param array $data profile data
	 * @throws LAMException error writing template
	 */
	public function writeAccountProfile(string $typeId, string $name, string $confName, array $data) : void;

	/**
	 * Loads an account profile.
	 *
	 * @param string $typeId user/group/host
	 * @param string $name account profile name
	 * @param string $confName server profile name
	 * @return array profile data
	 * @throws LAMException error writing template
	 */
	public function loadAccountProfile(string $typeId, string $name, string $confName) : array;

	/**
	 * Deletes an account profile.
	 *
	 * @param string $typeId user/group/host
	 * @param string $name account profile name
	 * @param string $confName server profile name
	 * @throws LAMException error deleting profile
	 */
	public function deleteAccountProfile(string $typeId, string $name, string $confName) : void;

}

/**
 * Manages account profiles on file system.
 *
 * @package LAM\PROFILES
 */
class AccountProfilePersistenceStrategyFiles implements AccountProfilePersistenceStrategy {

	/**
	 * @inheritDoc
	 */
	public function getAccountProfileTemplateNames(): array {
		$templatePath = __DIR__ . '/../config/templates/profiles';
		if (!is_dir($templatePath) || !is_readable($templatePath)) {
			throw new LAMException(_('Unable to read templates.'));
		}
		$templateDir = @dir($templatePath);
		$templates = array();
		if ($templateDir) {
			$entry = $templateDir->read();
			while ($entry){
				$parts = explode('.', $entry);
				if ((strlen($entry) > 3) && (sizeof($parts) === 2)) {
					$name = $parts[0];
					$scope = $parts[1];
					$templates[$scope][] = $name;
				}
				$entry = $templateDir->read();
			}
		}
		return $templates;
	}

	/**
	 * @inheritDoc
	 */
	public function deleteAccountProfileTemplate(string $scope, string $name): void {
		$fileName = $this->getAccountProfileTemplateFileName($scope, $name);
		$deleted = @unlink($fileName);
		if (!$deleted) {
			throw new LAMException(_("Unable to delete profile!"));
		}
	}

	/**
	 * Returns the file name of a global template.
	 *
	 * @param string $scope e.g. user
	 * @param string $name profile name
	 * @return string file name
	 */
	private function getAccountProfileTemplateFileName(string $scope, string $name): string {
		return __DIR__ . '/../config/templates/profiles' . '/' . $name . '.' . $scope;
	}

	/**
	 * @inheritDoc
	 */
	public function loadAccountProfileTemplate(string $scope, string $name): array {
		$fileName = $this->getAccountProfileTemplateFileName($scope, $name);
		return $this->readAccountProfileFile($fileName);
	}

	/**
	 * @inheritDoc
	 */
	public function writeAccountProfileTemplate(string $scope, string $name, array $data): void {
		$fileName = $this->getAccountProfileTemplateFileName($scope, $name);
		$this->writeAccountProfileDataToFile($fileName, $data);
	}

	/**
	 * Reads an account profile from the given file name.
	 *
	 * @param string $fileName file name
	 * @return array hash array (attribute => value)
	 * @throws LAMException error reading file
	 */
	private function readAccountProfileFile(string $fileName): array {
		$settings = array();
		if (is_file($fileName)) {
			$file = @fopen($fileName, "r");
			if ($file) {
				while (!feof($file)) {
					$line = fgets($file, 1024);
					if (($line === false) || ($line == '') || ($line == "\n") || ($line[0] == "#")) {
						continue; // ignore comments
					}
					// search keywords
					$parts = explode(": ", $line);
					if (sizeof($parts) == 2) {
						$option = $parts[0];
						if ($option === 'profname') {
							continue;
						}
						$value = $parts[1];
						// remove line ends
						$value = chop($value);
						$settings[$option] = explode("+::+", $value);
					}
				}
				fclose($file);
				return $settings;
			}
			else {
				throw new LAMException(_("Unable to load profile!"), $fileName);
			}
		}
		else {
			throw new LAMException(_("Unable to load profile!"), $fileName);
		}
	}

	/**
	 * Writes the profile data to the given file.
	 *
	 * @param string $fileName file name
	 * @param array $data profile data
	 * @throws LAMException error writing file
	 */
	private function writeAccountProfileDataToFile(string $fileName, array $data) : void {
		$basePath = dirname($fileName);
		if (!file_exists($basePath)) {
			mkdir($basePath, 0700, true);
		}
		$file = @fopen($fileName, "w");
		if (!$file) {
			logNewMessage(LOG_ERR, 'Unable to write ' . $fileName);
			throw new LAMException(_('Unable to save profile!'));
		}
		// write attributes
		$keys = array_keys($data);
		for ($i = 0; $i < sizeof($keys); $i++) {
			if (isset($data[$keys[$i]])) {
				$line = $keys[$i] . ": " . implode("+::+", $data[$keys[$i]]) . "\n";
			}
			else {
				$line = $keys[$i] . ": \n";
			}
			fputs($file, $line);
		}
		// close file
		fclose($file);
	}

	/**
	 * Returns the file name of a global template.
	 *
	 * @param string $typeId e.g. user
	 * @param string $name profile name
	 * @param string $confName server profile name
	 * @return string file name
	 */
	private function getAccountProfileFileName(string $typeId, string $name, string $confName): string {
		return __DIR__ . "/../config/profiles/" . $confName . '/' . $name . "." . $typeId;
	}

	/**
	 * @inheritDoc
	 */
	public function isAccountProfileExisting(string $typeId, string $name, string $confName): bool {
		$fileName = $this->getAccountProfileFileName($typeId, $name, $confName);
		return file_exists($fileName) && is_readable($fileName);
	}

	/**
	 * @inheritDoc
	 */
	public function writeAccountProfile(string $typeId, string $name, string $confName, array $data): void {
		$fileName = $this->getAccountProfileFileName($typeId, $name, $confName);
		$this->writeAccountProfileDataToFile($fileName, $data);
	}

	/**
	 * @inheritDoc
	 */
	public function loadAccountProfile(string $typeId, string $name, string $confName): array {
		$fileName = $this->getAccountProfileFileName($typeId, $name, $confName);
		return $this->readAccountProfileFile($fileName);
	}

	/**
	 * @inheritDoc
	 */
	public function deleteAccountProfile(string $typeId, string $name, string $confName): void {
		$fileName = $this->getAccountProfileFileName($typeId, $name, $confName);
		$deleted = @unlink($fileName);
		if (!$deleted) {
			throw new LAMException(_("Unable to delete profile!"));
		}
	}

	/**
	 * @inheritDoc
	 */
	public function getAccountProfileNames(string $typeId, string $confName): array {
		$dir = @dir(__DIR__ . "/../config/profiles/" . $confName);
		$ret = array();
		if ($dir) {
			$entry = $dir->read();
			while ($entry){
				// check if filename ends with .<typeId>
				if (strrpos($entry, '.')) {
					$pos = strrpos($entry, '.');
					if (substr($entry, $pos + 1) == $typeId) {
						$name = substr($entry, 0, $pos);
						$ret[] = $name;
					}
				}
				$entry = $dir->read();
			}
		}
		return $ret;
	}

}

/**
 * Manages account profiles via PDO.
 *
 * @package LAM\PROFILES
 */
class AccountProfilePersistenceStrategyPdo implements AccountProfilePersistenceStrategy {

	const TABLE_NAME = 'account_profiles';
	const TABLE_NAME_TEMPLATES = 'account_profiles_templates';

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
			. 'position int NOT NULL,'
			. 'confname VARCHAR(300) NOT NULL,'
			. 'typeid VARCHAR(300) NOT NULL,'
			. 'name VARCHAR(300) NOT NULL,'
			. 'data TEXT NOT NULL,'
			. 'PRIMARY KEY(position)'
			. ');';
		$this->pdo->exec($sql);
		logNewMessage(LOG_DEBUG, 'Creating database table ' . self::TABLE_NAME_TEMPLATES);
		$sql = 'create table ' . self::TABLE_NAME_TEMPLATES . '('
			. 'scope VARCHAR(100) NOT NULL,'
			. 'name VARCHAR(300) NOT NULL,'
			. 'data TEXT NOT NULL,'
			. 'PRIMARY KEY(scope,name)'
			. ');';
		$this->pdo->exec($sql);
		$sql = 'insert into ' . ConfigurationDatabase::TABLE_SCHEMA_VERSIONS . ' (name, version) VALUES ("account_profiles", 1);';
		$this->pdo->exec($sql);
	}

	/**
	 * @inheritDoc
	 */
	public function getAccountProfileTemplateNames(): array {
		$statement = $this->pdo->prepare("SELECT scope, name FROM " . self::TABLE_NAME_TEMPLATES);
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
	public function deleteAccountProfileTemplate(string $scope, string $name): void {
		$statement = $this->pdo->prepare("DELETE FROM " . self::TABLE_NAME_TEMPLATES . " WHERE scope = ? AND name = ?");
		$statement->execute(array($scope, $name));
	}

	/**
	 * @inheritDoc
	 */
	public function loadAccountProfileTemplate(string $scope, string $name): array {
		$statement = $this->pdo->prepare("SELECT data FROM " . self::TABLE_NAME_TEMPLATES . " WHERE scope = ? AND name = ?");
		$statement->execute(array($scope, $name));
		$results = $statement->fetchAll();
		if (empty($results)) {
			logNewMessage(LOG_ERR, "Account profile template not found: $scope-$name");
			throw new LAMException(_("Unable to load profile!"), $name);
		}
		return json_decode($results[0]['data'], true);
	}

	/**
	 * @inheritDoc
	 */
	public function writeAccountProfileTemplate(string $scope, string $name, array $data): void {
		$json = json_encode($data);
		$statement = $this->pdo->prepare("SELECT name FROM " . self::TABLE_NAME_TEMPLATES . " WHERE scope = ? AND name = ?");
		$statement->execute(array($scope, $name));
		$results = $statement->fetchAll();
		if (empty($results)) {
			$statement = $this->pdo->prepare("INSERT INTO " . self::TABLE_NAME_TEMPLATES . " (scope, name, data) VALUES (?, ?, ?)");
			$statement->execute(array($scope, $name, $json));
		}
		else {
			$statement = $this->pdo->prepare("UPDATE " . self::TABLE_NAME_TEMPLATES . " SET data = ? WHERE scope = ? AND name = ?");
			$statement->execute(array($json, $scope, $name));
		}
	}

	/**
	 * @inheritDoc
	 */
	public function isAccountProfileExisting(string $typeId, string $name, string $confName): bool {
		$statement = $this->pdo->prepare("SELECT data FROM " . self::TABLE_NAME . " WHERE confname = ? AND typeid = ? AND name = ?");
		$statement->execute(array($confName, $typeId, $name));
		$results = $statement->fetchAll();
		return !empty($results);
	}

	/**
	 * @inheritDoc
	 */
	public function getAccountProfileNames(string $typeId, string $confName): array {
		$statement = $this->pdo->prepare("SELECT name FROM " . self::TABLE_NAME . " WHERE confname = ? AND typeid = ?");
		$statement->execute(array($confName, $typeId));
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
	public function writeAccountProfile(string $typeId, string $name, string $confName, array $data): void {
		$json = json_encode($data);
		$isExisting = $this->isAccountProfileExisting($typeId, $name, $confName);
		if ($isExisting) {
			$statement = $this->pdo->prepare("UPDATE " . self::TABLE_NAME . " SET data = ? WHERE confname = ? AND typeid = ? AND name = ?");
			$statement->execute(array($json, $confName, $typeId, $name));
		}
		else {
			$positionStatement = $this->pdo->prepare("SELECT MAX(position) AS position FROM " . self::TABLE_NAME);
			$positionStatement->execute();
			$positionResult = $positionStatement->fetchAll();
			$position = $positionResult[0]['position'] + 1;
			$statement = $this->pdo->prepare("INSERT INTO " . self::TABLE_NAME . " (position, confname, typeid, name, data) VALUES (?, ?, ?, ?, ?)");
			$statement->execute(array($position, $confName, $typeId, $name, $json));
		}
	}

	/**
	 * @inheritDoc
	 */
	public function loadAccountProfile(string $typeId, string $name, string $confName): array {
		$statement = $this->pdo->prepare("SELECT data FROM " . self::TABLE_NAME . " WHERE confname = ? AND typeid = ? AND name = ?");
		$statement->execute(array($confName, $typeId, $name));
		$results = $statement->fetchAll();
		if (empty($results)) {
			logNewMessage(LOG_ERR, "Account profile not found: $typeId-$name");
			throw new LAMException(_("Unable to load profile!"), $name);
		}
		return json_decode($results[0]['data'], true);
	}

	/**
	 * @inheritDoc
	 */
	public function deleteAccountProfile(string $typeId, string $name, string $confName): void {
		$statement = $this->pdo->prepare("DELETE FROM " . self::TABLE_NAME . " WHERE confname = ? AND typeid = ? AND name = ?");
		$statement->execute(array($confName, $typeId, $name));
	}

}
