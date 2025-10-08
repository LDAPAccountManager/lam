<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2025  Roland Gruber

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

namespace LAM\WHITE_PAGES;

use LAM\LIB\TWO_FACTOR\TwoFactorProviderService;
use LAM\PERSISTENCE\ConfigurationDatabase;
use LAMCfgMain;
use LAMException;
use PDO;
use PDOException;
use function LAM\PERSISTENCE\dbTableExists;

/**
 * Supported authentication types.
 */
enum WP_AUTHENTICATION {

	/**
	 * Anonymous authentication
	 */
	case AUTH_ANONYMOUS;

	/**
	 * User and password authentication.
	 */
	case AUTH_USER_PASSWORD;

	/**
	 * Trust the 2FA provider for authentication.
	 */
	case AUTH_2FA_ONLY;

}

/**
 * Display modes for white pages.
 */
enum WP_DISPLAY_MODE {
	case LIST_GALLERY;
	case LIST_ONLY;
	case GALLERY_ONLY;
}

/**
 * Defines the display of a single item.
 */
enum WP_ITEM_TYPE {

	case TITLE;
	case IMAGE_JPG;
	case TEXT;
	case EMAIL;
	case TELEPHONE;
	case LINK;

}

/**
 * White pages profile.
 */
class WhitePagesProfile {

	/**
	 * @var string LDAP server URL
	 */
	public string $serverUrl = 'ldap://example.com';
	/**
	 * @var bool activate TLS
	 */
	public bool $useTLS = false;
	/**
	 * @var bool follow referrals
	 */
	public bool $followReferrals = true;
	/**
	 * @var string LDAP technical user for connection
	 */
	public string $ldapConnectionUser = '';
	/**
	 * @var string password of technical user
	 */
	public string $ldapConnectionPassword = '';
	/**
	 * @var bool use technical user for all operations
	 */
	public bool $useForAllOperations = false;
	/**
	 * @var string language
	 */
	public string $language = 'en_gb';

	/**
	 * @var string authentication type
	 */
	public string $authenticationType = 'AUTH_ANONYMOUS';
	/**
	 * @var string LDAP suffix for users
	 */
	public string $authenticationLdapSuffix = 'ou=people,dc=example,dc=com';
	/**
	 * @var string LDAP search attribute to find the user for authentication
	 */
	public string $authenticationLdapSearchAttribute = 'uid';
	/**
	 * @var string additional LDAP search filter for user search
	 */
	public string $authenticationLdapAdditionalSearchFilter = '';

	/**
	 * @var string provider type
	 */
	public string $twoFactorAuthenticationType = TwoFactorProviderService::TWO_FACTOR_NONE;
	/**
	 * @var string authentication URL
	 */
	public string $twoFactorAuthenticationURL = 'https://localhost';
	/**
	 * @var bool allow insecure connection
	 */
	public bool $twoFactorAuthenticationInsecure = false;
	/**
	 * @var string label
	 */
	public string $twoFactorAuthenticationLabel = '';
	/**
	 * @var string caption
	 */
	public string $twoFactorAuthenticationCaption = '';
	/**
	 * @var string client ID
	 */
	public string $twoFactorAuthenticationClientId = '';
	/**
	 * @var string secret key
	 */
	public string $twoFactorAuthenticationSecretKey = '';
	/**
	 * @var string attribute that contains 2FA username
	 */
	public string $twoFactorAuthenticationAttribute = 'uid';
	/**
	 * @var string 2FA domain
	 */
	public string $twoFactorAuthenticationDomain = '';
	/**
	 * @var bool allow remembering the device
	 */
	public bool $twoFactorAllowToRememberDevice = false;
	/**
	 * @var int timeout for remembering the device
	 */
	public int $twoFactorRememberDeviceDuration = 28800;
	/**
	 * @var string password to reset remembered devices
	 */
	public string $twoFactorRememberDevicePassword = '';

	/** provider for captcha (-/google/hCaptcha/friendlyCaptcha) */
	public $captchaProvider = '-';
	/** site key */
	public $captchaSiteKey = '';
	/** secret key */
	public $captchaSecretKey = '';

	/** describing text for search attribute */
	public string $loginAttributeText = '';
	/** label for password input */
	public string $passwordLabel = '';
	/** describing text for user login */
	public string $loginCaption = '<p><strong>Welcome to LAM white pages. Please enter your user name and password.</strong></p>';
	/** header HTML for login */
	public string $loginHeader = '<p><a href="https://www.ldap-account-manager.org/" target="new_window"><img alt="help" class="align-middle" src="../../graphics/logo24.png" style="height:24px; width:24px" /> LDAP Account Manager </a></p><p>&nbsp;</p>';
	/** describing text for user login */
	public string $loginFooter = '<p style="text-align: left;">Powered by LDAP Account Manager</p>';

	/**
	 * @var string header HTML
	 */
	public string $header = '<p><a href="https://www.ldap-account-manager.org/" target="new_window"><img alt="help" class="align-middle" src="../../graphics/logo24.png" style="height:24px; width:24px" /> LDAP Account Manager </a></p><p>&nbsp;</p>';
	/**
	 * @var string footer HTML
	 */
	public string $footer = '<p style="text-align: left;">Powered by LDAP Account Manager</p>';
	/**
	 * @var string custom CSS
	 */
	public string $customCss = '';
	/**
	 * @var string base color
	 */
	public string $baseColor = '';
	/**
	 * @var string secondary color
	 */
	public string $secondaryColor = '';
	/**
	 * @var string background image
	 */
	public string $backgroundImage = '';

	/**
	 * @var WhitePagesTab[] tabs for white pages
	 */
	public array $tabs = [];

	/**
	 * Imports data to a new profile.
	 *
	 * @param array $data data as array
	 * @return WhitePagesProfile profile
	 */
	public static function import(array $data): WhitePagesProfile {
		$profile = new WhitePagesProfile();
		$vars = get_class_vars(WhitePagesProfile::class);
		foreach ($data as $key => $value) {
			if ($key === 'tabs') {
				foreach ($value as $tabData) {
					$profile->tabs[] = WhitePagesTab::import($tabData);
				}
			}
			elseif (array_key_exists($key, $vars)) {
				$profile->$key = $value;
			}
		}
		return $profile;
	}

	/**
	 * Exports the profile as an array.
	 *
	 * @return array data
	 */
	public function export(): array {
		return json_decode(json_encode($this), true);
	}

}

/**
 * Single tab inside white pages.
 */
class WhitePagesTab {

	/**
	 * @var string LDAP suffix to find the entries to display
	 */
	public string $ldapSuffix = 'ou=people,dc=example,dc=com';
	/**
	 * @var string additional LDAP search filter for entry search
	 */
	public string $additionalSearchFilter = '';

	/**
	 * @var string display label
	 */
	public string $label = 'Users';
	/**
	 * @var string[] searchable LDAP attributes
	 */
	public array $searchableAttributes = ['cn', 'sn', 'gn', 'mail', 'telephoneNumber'];

	/**
	 * @var WhitePagesDisplayItem[] list items
	 */
	public array $listItems = [];

	/**
	 * @var WhitePagesDisplayItem[] gallery items
	 */
	public array $galleryItems = [];

	/**
	 * @var WhitePagesDisplayItem[] detail view items
	 */
	public array $detailViewItems = [];

	/**
	 * Creates a new default tab.
	 *
	 * @return WhitePagesTab tab
	 */
	public static function createDefault(): WhitePagesTab {
		$tab = new WhitePagesTab();
		return $tab;
	}

	/**
	 * Imports data to a new tab.
	 *
	 * @param array $data data as array
	 * @return WhitePagesTab profile
	 */
	public static function import(array $data): WhitePagesTab {
		$tab = new WhitePagesTab();
		$vars = get_class_vars(WhitePagesTab::class);
		foreach ($data as $key => $value) {
			if (array_key_exists($key, $vars)) {
				$tab->$key = $value;
			}
		}
		return $tab;
	}

}

/**
 * Specifies how a line/column should be rendered.
 */
class WhitePagesDisplayItem {

	/**
	 * @var WP_ITEM_TYPE render type of item
	 */
	public WP_ITEM_TYPE $TYPE = WP_ITEM_TYPE::TEXT;

	/**
	 * @var string label of this item
	 */
	public string $label = 'First name';

	/**
	 * @var string attribute syntax of this item
	 */
	public string $attributes = 'gn';

}

/**
 * Manages reading and writing white pages profiles.
 */
class WhitePagesPersistence {

	private WhitePagesPersistenceStrategy $strategy;

	/**
	 * Constructor
	 *
	 * @throws LAMException error connecting database
	 */
	public function __construct() {
		$configDb = new ConfigurationDatabase(new LAMCfgMain());
		if ($configDb->useRemoteDb()) {
			try {
				$this->strategy = new WhitePagesPersistenceStrategyPdo($configDb->getPdo());
			}
			catch (PDOException $e) {
				logNewMessage(LOG_ERR, _('Unable to connect to configuration database.') . ' ' . $e->getMessage());
				throw new LAMException(_('Unable to connect to configuration database.'), '', $e);
			}
		}
		else {
			$this->strategy = new WhitePagesPersistenceStrategyFileSystem();
		}
	}

	/**
	 * Returns a list of available white pages profiles.
	 *
	 * @return string[] profile names
	 */
	public function getProfiles(): array {
		$profiles = $this->strategy->getProfiles();
		natcasesort($profiles);
		return array_values($profiles);
	}

	/**
	 * Returns if the profile with given name can be written.
	 *
	 * @param string $name profile name
	 * @return bool can be written
	 */
	public function canWrite(string $name): bool {
		return $this->strategy->canWrite($name);
	}

	/**
	 * Loads the given white pages profile.
	 *
	 * @param string $name profile name
	 * @return WhitePagesProfile profile
	 * @throws LAMException error during loading
	 */
	public function load(string $name): WhitePagesProfile {
		return $this->strategy->load($name);
	}

	/**
	 * Stores the given profile.
	 *
	 * @param string $name profile name
	 * @param WhitePagesProfile $profile profile
	 * @throws LAMException error during saving
	 */
	public function save(string $name, WhitePagesProfile $profile): void {
		$this->strategy->save($name, $profile);
	}

	/**
	 * Deletes a white pages profile.
	 *
	 * @param string $name profile name
	 * @throws LAMException error deleting profile
	 */
	public function delete(string $name): void {
		$profileList = $this->getProfiles();
		if (!preg_match("/^[a-z0-9_-]+$/i", $name)
			|| !in_array($name, $profileList)) {
			throw new LAMException(_("Unable to delete profile!"));
		}
		$this->strategy->delete($name);
	}

	/**
	 * Renames a white pages profile.
	 *
	 * @param string $oldName existing profile name
	 * @param string $newName new profile name
	 * @throws LAMException error renaming profile
	 */
	public function rename(string $oldName, string $newName): void {
		$profileList = $this->getProfiles();
		if (!self::isValidProfileName($newName)
			|| !in_array($oldName, $profileList)) {
			throw new LAMException(_("Profile name is invalid!"));
		}
		$this->strategy->rename($oldName, $newName);
	}

	/**
	 * Checks if the profile name is valid.
	 *
	 * @param string $name profile name
	 * @return bool is valid
	 */
	public static function isValidProfileName(string $name): bool {
		return preg_match("/^[a-z0-9_-]+$/i", $name);
	}

}

/**
 * Interface for white pages profile persistence.
 */
interface WhitePagesPersistenceStrategy {

	/**
	 * Returns a list of available white pages profiles.
	 *
	 * @return string[] profile names
	 */
	public function getProfiles(): array;

	/**
	 * Returns if the profile with given name can be written.
	 *
	 * @param string $name profile name
	 * @return bool can be written
	 */
	public function canWrite(string $name): bool;

	/**
	 * Loads the given white pages profile.
	 *
	 * @param string $name profile name
	 * @return WhitePagesProfile profile
	 * @throws LAMException error during loading
	 */
	public function load(string $name): WhitePagesProfile;

	/**
	 * Stores the given profile.
	 *
	 * @param string $name profile name
	 * @param WhitePagesProfile $profile profile
	 * @throws LAMException error during saving
	 */
	public function save(string $name, WhitePagesProfile $profile): void;

	/**
	 * Deletes a white pages profile.
	 *
	 * @param string $name profile name
	 * @throws LAMException error renaming profile
	 */
	public function delete(string $name): void;

	/**
	 * Renames a white pages profile.
	 *
	 * @param string $oldName existing profile name
	 * @param string $newName new profile name
	 * @throws LAMException error renaming profile
	 */
	public function rename(string $oldName, string $newName): void;

}

/**
 * Uses the local file system for storing white pages profiles.
 */
class WhitePagesPersistenceStrategyFileSystem implements WhitePagesPersistenceStrategy {

	/**
	 * @inheritDoc
	 */
	public function getProfiles(): array {
		$dir = dir(__DIR__ . "/../config/whitePages");
		$ret = [];
		if ($dir === false) {
			logNewMessage(LOG_ERR, 'Unable to read white pages profiles');
			return $ret;
		}
		while ($entry = $dir->read()) {
			$name = substr($entry, 0, strrpos($entry, '.'));
			if (str_ends_with($entry, '.json')) {
				$ret[] = $name;
			}
		}
		return $ret;
	}

	/**
	 * @inheritDoc
	 */
	public function canWrite(string $name): bool {
		// check the profile name
		if (!WhitePagesPersistence::isValidProfileName($name)) {
			return false;
		}
		$path = __DIR__ . "/../config/whitePages/" . $name . ".json";
		return is_writable($path);
	}

	/**
	 * @inheritDoc
	 */
	public function load(string $name): WhitePagesProfile {
		if (!WhitePagesPersistence::isValidProfileName($name)) {
			throw new LAMException(_("Profile name is invalid!"));
		}
		$file = __DIR__ . "/../config/whitePages/" . $name . ".json";
		if (is_file($file)) {
			$file = @fopen($file, "r");
			if ($file) {
				$data = fread($file, 10000000);
				$profileData = @json_decode($data, true);
				if ($profileData === null) {
					logNewMessage(LOG_ERR, "Unable to load profile because not in JSON format: " . $name);
					throw new LAMException(_("Unable to load profile!"), $name);
				}
				$profile = WhitePagesProfile::import($profileData);
				fclose($file);
				return $profile;
			}
			else {
				throw new LAMException(_("Unable to load profile!"), $name);
			}
		}
		else {
			throw new LAMException(_("Unable to load profile!"), $name);
		}
	}

	/**
	 * @inheritDoc
	 */
	public function save(string $name, WhitePagesProfile $profile): void {
		// check the profile name
		if (!WhitePagesPersistence::isValidProfileName($name)) {
			throw new LAMException(_("Profile name is invalid!"));
		}
		$path = __DIR__ . "/../config/whitePages/" . $name . ".json";
		$file = @fopen($path, "w");
		if ($file) {
			// write settings to file
			fwrite($file, json_encode($profile->export(), JSON_PRETTY_PRINT));
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
	public function delete(string $name): void {
		if (!unlink("../../config/whitePages/" . $name . ".json")) {
			throw new LAMException(_("Unable to delete profile!"));
		}
	}

	/**
	 * @inheritDoc
	 */
	public function rename(string $oldName, string $newName): void {
		if (!@rename(__DIR__ . "/../config/whitePages/" . $oldName . ".json",
			__DIR__ . "/../config/whitePages/" . $newName . ".json")) {
			throw new LAMException(_("Could not rename file!"));
		}
	}

}

/**
 * Uses PDO for storing white pages profiles.
 */
class WhitePagesPersistenceStrategyPdo implements WhitePagesPersistenceStrategy {

	private PDO $pdo;

	private const TABLE_NAME = 'white_pages_profiles';

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
	 * Checks if the schema has the latest version.
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
		$sql = 'insert into ' . ConfigurationDatabase::TABLE_SCHEMA_VERSIONS . ' (name, version) VALUES ("white_pages", 1);';
		$this->pdo->exec($sql);
	}

	/**
	 * @inheritDocm
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
	public function canWrite(string $name): bool {
		return true;
	}

	/**
	 * @inheritDoc
	 */
	public function load(string $name): WhitePagesProfile {
		// check the profile name
		if (!WhitePagesPersistence::isValidProfileName($name)) {
			throw new LAMException(_("Profile name is invalid!"));
		}
		$statement = $this->pdo->prepare("SELECT data FROM " . self::TABLE_NAME . " WHERE name = ?");
		$statement->execute([$name]);
		$results = $statement->fetchAll();
		if (empty($results)) {
			logNewMessage(LOG_ERR, 'Self service profile not found');
			throw new LAMException(_("Unable to load profile!"), $name);
		}
		$data = json_decode($results[0]['data'], true);
		return WhitePagesProfile::import($data);
	}

	/**
	 * @inheritDoc
	 */
	public function save(string $name, WhitePagesProfile $profile): void {
		// check the profile name
		if (!WhitePagesPersistence::isValidProfileName($name)) {
			throw new LAMException(_("Profile name is invalid!"));
		}
		$data = json_encode($profile->export(), JSON_PRETTY_PRINT);
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
	public function delete(string $name): void {
		$statement = $this->pdo->prepare("DELETE FROM " . self::TABLE_NAME . " WHERE name = ?");
		$statement->execute([$name]);
	}

	/**
	 * @inheritDoc
	 */
	public function rename(string $oldName, string $newName): void {
		$statement = $this->pdo->prepare("UPDATE " . self::TABLE_NAME . " SET name = ? WHERE name = ?");
		$statement->execute([$newName, $oldName]);
	}

}
