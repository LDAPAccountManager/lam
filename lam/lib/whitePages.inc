<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2025 - 2026  Roland Gruber

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

use htmlResponsiveInputField;
use htmlResponsiveRow;
use LAM\LIB\TWO_FACTOR\TwoFactorProviderService;
use LAM\PERSISTENCE\ConfigurationDatabase;
use LAMCfgMain;
use LamColorUtil;
use LAMException;
use LDAP\Connection;
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
enum WP_DISPLAY_MODE: string {
	case LIST_GALLERY = 'LIST_GALLERY';
	case LIST_ONLY = 'LIST_ONLY';
	case GALLERY_ONLY = 'GALLERY_ONLY';
}

/**
 * Defines the display of a single item.
 */
enum WP_ITEM_TYPE: string {

	case IMAGE_JPG = 'IMAGE_JPG';
	case TEXT = 'TEXT';
	case EMAIL = 'EMAIL';
	case TELEPHONE = 'TELEPHONE';
	case LINK = 'LINK';

}

enum WP_THEMES: string {
	case OCEAN = 'OCEAN';
	case STARS = 'STARS';
	case GRAND_CANYON = 'GRAND_CANYON';
	case RAIN_FOREST = 'RAIN_FOREST';
	case COAST = 'COAST';
	case MOUNTAIN = 'MOUNTAIN';
	case CITY = 'CITY';

	/**
	 * Returns the image name.
	 *
	 * @return string image name
	 */
	public function getImage(): string {
		return match($this) {
			self::OCEAN => 'background_jellyfish.jpg',
			self::STARS => 'background_milkyway.jpg',
			self::GRAND_CANYON => 'background_grandcanyon.jpg',
			self::RAIN_FOREST => 'background_rainforest.jpg',
			self::COAST => 'background_skyline.jpg',
			self::MOUNTAIN => 'background_mountain.jpg',
			self::CITY => 'background_city.jpg',
		};
	}

	/**
	 * Returns the primary color.
	 *
	 * @return string color
	 */
	public function getPrimaryColor(): string {
		return match($this) {
			self::OCEAN => '#1a5fb4',
			self::STARS => '#2E4053',
			self::GRAND_CANYON => '#E2786F',
			self::RAIN_FOREST => '#2d5001',
			self::COAST => '#03395f',
			self::MOUNTAIN => '#45B3FA',
			self::CITY => '#333333',
		};
	}

	/**
	 * Returns the background color.
	 *
	 * @return string color
	 */
	public function getBackgroundColor(): string {
		return match($this) {
			self::OCEAN => '#f9fdff',
			self::STARS => '#DEDEDE',
			self::GRAND_CANYON => '#F5F5DC',
			self::RAIN_FOREST => '#EFF2E1',
			self::COAST => '#f1fafc',
			self::MOUNTAIN => '#FFFFFF',
			self::CITY => '#F4F4ED',
		};
	}

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
	public string $captchaProvider = '-';
	/** site key */
	public string $captchaSiteKey = '';
	/** secret key */
	public string $captchaSecretKey = '';

	/** describing text for search attribute */
	public string $loginAttributeText = '';
	/** label for password input */
	public string $passwordLabel = '';
	/** describing text for user login */
	public string $loginCaption = '<p><strong>Welcome to LAM white pages. Please enter your user name and password.</strong></p>';
	/** header HTML for login */
	public string $loginHeader = '<p><a href="https://www.ldap-account-manager.org/" target="new_window"><img alt="help" class="align-middle" src="../../graphics/logo.svg" style="height:24px; width:24px"> <span style="color: rgb(255, 255, 255);">LDAP Account Manager </span></a></p><p>&nbsp;<br></p>';
	/** describing text for user login */
	public string $loginFooter = '<p style="text-align: left;"><span style="color: rgb(255, 255, 255);">Powered by LDAP Account Manager</span></p>';

	/**
	 * @var string header HTML
	 */
	public string $header = '<p><a href="https://www.ldap-account-manager.org/" target="new_window"><img alt="help" class="align-middle" src="../../graphics/logo.svg" style="height:24px; width:24px"> <span style="color: rgb(255, 255, 255);">LDAP Account Manager </span></a></p><p>&nbsp;<br></p>';
	/**
	 * @var string footer HTML
	 */
	public string $footer = '<p style="text-align: left;"><span style="color: rgb(255, 255, 255);">Powered by LDAP Account Manager</span></p>';
	/**
	 * @var string custom CSS
	 */
	public string $customCss = '';
	/**
	 * @var string primary color
	 */
	public string $primaryColor = '';
	/**
	 * @var string background color
	 */
	public string $backgroundColor = '';
	/**
	 * @var string background image
	 */
	public string $backgroundImage = '';
	/**
	 * @var string display mode
	 */
	public string $displayMode = 'LIST_GALLERY';

	/**
	 * @var WhitePagesTab[] tabs for white pages
	 */
	public array $tabs = [];

	/**
	 * Imports data to a new profile.
	 *
	 * @param array<string, int|string|array<string, string|string[]|array<string, string>[]>[]> $data data as array
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
	 * @return array<string, int|string|array<string, string|string[]|array<string, string>[]>[]> data
	 */
	public function export(): array {
		$data = [];
		foreach (get_object_vars($this) as $key => $value) {
			if ($key === 'tabs') {
				$data[$key] = [];
				foreach ($this->tabs as $tab) {
					$data[$key][] = $tab->export();
				}
			}
			else {
				$data[$key] = $value;
			}
		}
		return $data;
	}

	/**
	 * Returns the login handler.
	 *
	 * @return WhitePagesLoginHandler handler
	 */
	public function getLoginHandler(): WhitePagesLoginHandler {
		return match ($this->authenticationType) {
			WP_AUTHENTICATION::AUTH_ANONYMOUS->name => new WhitePagesAnonymousAuthLoginHandler(),
			WP_AUTHENTICATION::AUTH_2FA_ONLY->name => new WhitePages2FaLoginHandler($this),
			default => new WhitePagesUserPasswordLoginHandler($this),
		};
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
	public array $searchableAttributes = ['cn', 'sn', 'givenName', 'mail', 'telephoneNumber'];

	/**
	 * @var string value to use for initial sorting
	 */
	public string $initialSorting = '';

	/**
	 * @var WhitePagesDisplayItem[] list items
	 */
	public array $listItems = [];

	/**
	 * @var string title for gallery view
	 */
	public string $galleryTitle = '$givenName $sn';

	/**
	 * @var WhitePagesDisplayItem[] gallery items
	 */
	public array $galleryItems = [];

	/**
	 * @var string title for gallery view
	 */
	public string $detailViewTitle = '$givenName $sn';

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
		$tab->listItems = [
			new WhitePagesDisplayItem('First name', WP_ITEM_TYPE::TEXT, 'givenName'),
			new WhitePagesDisplayItem('Last name', WP_ITEM_TYPE::TEXT, 'sn'),
			new WhitePagesDisplayItem('Email', WP_ITEM_TYPE::EMAIL, 'mail'),
		];
		$tab->galleryItems = [
			new WhitePagesDisplayItem('Photo', WP_ITEM_TYPE::IMAGE_JPG, 'jpegPhoto'),
			new WhitePagesDisplayItem('Name', WP_ITEM_TYPE::TEXT, '$givenName$ $sn$'),
		];
		$tab->detailViewItems = [
			new WhitePagesDisplayItem('Photo', WP_ITEM_TYPE::IMAGE_JPG, 'jpegPhoto'),
			new WhitePagesDisplayItem('Full name', WP_ITEM_TYPE::TEXT, '$givenName$ $sn$'),
			new WhitePagesDisplayItem('First name', WP_ITEM_TYPE::TEXT, 'givenName'),
			new WhitePagesDisplayItem('Last name', WP_ITEM_TYPE::TEXT, 'sn'),
			new WhitePagesDisplayItem('Email', WP_ITEM_TYPE::EMAIL, 'mail'),
			new WhitePagesDisplayItem('Telephone', WP_ITEM_TYPE::TELEPHONE, 'telephoneNumber'),
			new WhitePagesDisplayItem('Manager', WP_ITEM_TYPE::LINK, 'manager:cn'),
			new WhitePagesDisplayItem('Groups', WP_ITEM_TYPE::LINK, 'memberOf:cn'),
		];
		return $tab;
	}

	/**
	 * Imports data to a new tab.
	 *
	 * @param array<string, string|string[]|array<string, string>[]> $data data as array
	 * @return WhitePagesTab profile
	 */
	public static function import(array $data): WhitePagesTab {
		$tab = new WhitePagesTab();
		$vars = get_class_vars(WhitePagesTab::class);
		foreach ($data as $key => $value) {
			if (in_array($key, ['listItems', 'galleryItems', 'detailViewItems'])) {
				$tab->$key = [];
				foreach ($value as $item) {
					$tab->$key[] = WhitePagesDisplayItem::import($item);
				}
			}
			elseif (array_key_exists($key, $vars)) {
				$tab->$key = $value;
			}
		}
		return $tab;
	}

	/**
	 * Exports the tab as an array.
	 *
	 * @return array<string, string|string[]|array<string, string>[]> data
	 */
	public function export(): array {
		$data = [];
		foreach (get_object_vars($this) as $key => $value) {
			if (in_array($key, ['listItems', 'galleryItems', 'detailViewItems'])) {
				$itemsData = [];
				foreach ($value as $item) {
					$itemsData[] = $item->export();
				}
				$data[$key] = $itemsData;
			}
			else {
				$data[$key] = $value;
			}
		}
		return $data;
	}

}

/**
 * Specifies how a line/column should be rendered.
 */
class WhitePagesDisplayItem {

	/**
	 * @var WP_ITEM_TYPE render type of item
	 */
	public WP_ITEM_TYPE $type;

	/**
	 * @var string label of this item
	 */
	public string $label;

	/**
	 * @var string syntax of this item
	 */
	public string $value;

	/**
	 * Constructor
	 *
	 * @param string $label label
	 * @param WP_ITEM_TYPE $type render type
	 * @param string $value value
	 */
	public function __construct(string $label = 'First name', WP_ITEM_TYPE $type = WP_ITEM_TYPE::TEXT, string $value = 'givenName') {
		$this->label = $label;
		$this->type = $type;
		$this->value = $value;
	}

	/**
	 * Imports the display item from a string array.
	 *
	 * @param array<string, string> $data data
	 * @return WhitePagesDisplayItem item
	 */
	public static function import(array $data): WhitePagesDisplayItem {
		$label = $data['label'] ?? '';
		$type = isset($data['type']) ? WP_ITEM_TYPE::from($data['type']) : WP_ITEM_TYPE::TEXT;
		$value = $data['value'] ?? '';
		return new WhitePagesDisplayItem($label, $type, $value);
	}

	/**
	 * Exports the item as an array.
	 *
	 * @return array<string, string> data
	 */
	public function export(): array {
		return [
			'label' => $this->label,
			'type' => $this->type->name,
			'value' => $this->value
		];
	}

}

/**
 * Manages the reading and writing of white pages profiles.
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
		}
		throw new LAMException(_("Unable to load profile!"), $name);
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

	private const TABLE_NAME = 'white_pages_profiles';
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
			logNewMessage(LOG_ERR, 'White pages profile not found');
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

/**
 * Login handler for white pages
 */
interface WhitePagesLoginHandler {

	/**
	 * Adds the necessary fields to the login dialog (e.g. user + password).
	 *
	 * @param htmlResponsiveRow $content dialog content
	 */
	public function addLoginFields(htmlResponsiveRow $content): void;

	/**
	 * Returns the login name.
	 *
	 * @return string login name
	 */
	public function getLoginName(): string;

	/**
	 * Returns the login password.
	 *
	 * @return string password
	 */
	public function getLoginPassword(): string;

	/**
	 * Returns if the login handler manages authentication on its own.
	 *
	 * @return bool manages authentication
	 */
	public function managesAuthentication(): bool;

	/**
	 * Returns if the authentication was successful.
	 * Only valid if managesAuthentication() returns true.
	 *
	 * @return bool authentication successful
	 * @throws LAMException error during authentication
	 */
	public function isAuthenticationSuccessful(): bool;

	/**
	 * Authorizes a user provided by the 2FA provider.
	 *
	 * @param string $userName user name
	 * @throws LAMException error during authentication
	 */
	public function authorize2FaUser(string $userName): void;

}

/**
 * Performs login with user and password.
 */
class WhitePagesUserPasswordLoginHandler implements WhitePagesLoginHandler {

	private WhitePagesProfile $profile;

	/**
	 * Constructor
	 *
	 * @param WhitePagesProfile $profile profile
	 */
	public function __construct(WhitePagesProfile $profile) {
		$this->profile = $profile;
	}

	/**
	 * @inheritDoc
	 */
	public function addLoginFields(htmlResponsiveRow $content): void {
		// user name
		$userNameVal = empty($_POST['username']) ? '' : $_POST['username'];
		$userText = _("User name");
		if (!empty($this->profile->loginAttributeText)) {
			$userText = $this->profile->loginAttributeText;
		}
		$userField = new htmlResponsiveInputField($userText, 'username', $userNameVal);
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
	public function getLoginName(): string {
		return $_POST['username'];
	}

	/**
	 * @inheritDoc
	 */
	public function getLoginPassword(): string {
		return $_POST['password'];
	}

	/**
	 * @inheritDoc
	 */
	public function managesAuthentication(): bool {
		return false;
	}

	/**
	 * @inheritDoc
	 */
	public function isAuthenticationSuccessful(): bool {
		throw new LAMException('Not implemented');
	}

	/**
	 * @inheritDoc
	 */
	public function authorize2FaUser(string $userName): void {
		// no action
	}

}

/**
 * Performs login with pure 2FA.
 */
class WhitePages2FaLoginHandler implements WhitePagesLoginHandler {

	private WhitePagesProfile $profile;

	/**
	 * Constructor
	 *
	 * @param WhitePagesProfile $profile profile
	 */
	public function __construct(WhitePagesProfile $profile) {
		$this->profile = $profile;
	}

	/**
	 * @inheritDoc
	 */
	public function addLoginFields(htmlResponsiveRow $content): void {
		// no input fields
	}

	/**
	 * @inheritDoc
	 */
	public function getLoginName(): string {
		return '';
	}

	/**
	 * @inheritDoc
	 */
	public function getLoginPassword(): string {
		return '';
	}

	/**
	 * @inheritDoc
	 */
	public function managesAuthentication(): bool {
		return true;
	}

	/**
	 * @inheritDoc
	 */
	public function isAuthenticationSuccessful(): bool {
		if (($this->profile->twoFactorAuthenticationType !== TwoFactorProviderService::TWO_FACTOR_OKTA)
			&& ($this->profile->twoFactorAuthenticationType !== TwoFactorProviderService::TWO_FACTOR_OPENID)) {
			logNewMessage(LOG_ERR, 'Unsupported 2FA provider: ' . $this->profile->twoFactorAuthenticationType);
			return false;
		}
		if (!$this->profile->useForAllOperations) {
			logNewMessage(LOG_ERR, 'Use for all operations must be set');
			return false;
		}
		// authentication will be checked on the 2FA page
		return true;
	}

	/**
	 * @inheritDoc
	 */
	public function authorize2FaUser(string $userName): void {
		$bindUser = $this->profile->ldapConnectionUser;
		if (empty($bindUser)) {
			throw new LAMException('WhitePages2FaLoginHandler', 'No bind user set');
		}
		$bindPassword = deobfuscateText($this->profile->ldapConnectionPassword);
		$server = connectToLDAP($this->profile->serverUrl, $this->profile->useTLS);
		if ($server === null) {
			throw new LAMException('WhitePages2FaLoginHandler', 'Unable to match provided user with LDAP entry - LDAP connect failed');
		}
		ldap_set_option($server, LDAP_OPT_REFERRALS, $this->profile->followReferrals);
		$bind = @ldap_bind($server, $bindUser, $bindPassword);
		if (!$bind) {
			throw new LAMException('WhitePages2FaLoginHandler', 'Unable to match provided user with LDAP entry - LDAP bind failed');
		}
		$filter = '(' . $this->profile->twoFactorAuthenticationAttribute . "=" . ldap_escape($userName, '', LDAP_ESCAPE_FILTER) . ')';
		if (!empty($this->profile->additionalLDAPFilter)) {
			$filter = '(&' . $filter . $this->profile->additionalLDAPFilter . ')';
		}
		$result = @ldap_search($server, $this->profile->authenticationLdapSuffix, $filter, ['DN'], 0, 1, 0, LDAP_DEREF_NEVER);
		if ($result === false) {
			throw new LAMException('WhitePages2FaLoginHandler', 'Unable to match provided user with LDAP entry - LDAP search failed');
		}
		$entries = @ldap_get_entries($server, $result);
		if ($entries === false) {
			throw new LAMException('WhitePages2FaLoginHandler', 'Unable to match provided user with LDAP entry - LDAP search failed');
		}
		$info = $entries;
		cleanLDAPResult($info);
		if (count($info) === 1) {
			$userDN = $info[0]['dn'];
			$_SESSION['whitePages_clientDN'] = lamEncrypt($userDN, 'WhitePages');
			return;
		}
		throw new LAMException('WhitePages2FaLoginHandler', 'Multiple or no results for ' . $userName . ' ' . print_r($info, true));
	}

}

/**
 * Performs anonymous access.
 */
class WhitePagesAnonymousAuthLoginHandler implements WhitePagesLoginHandler {

	/**
	 * @inheritDoc
	 */
	public function addLoginFields(htmlResponsiveRow $content): void {
		// no input fields
	}

	/**
	 * @inheritDoc
	 */
	public function getLoginName(): string {
		return '';
	}

	/**
	 * @inheritDoc
	 */
	public function getLoginPassword(): string {
		return '';
	}

	/**
	 * @inheritDoc
	 */
	public function managesAuthentication(): bool {
		return true;
	}

	/**
	 * @inheritDoc
	 */
	public function isAuthenticationSuccessful(): bool {
		// authentication is not needed
		return true;
	}

	/**
	 * @inheritDoc
	 */
	public function authorize2FaUser(string $userName): void {
		// not used
	}

}

/**
 * Creates styles for white pages.
 */
class WhitePagesStyling {

	/**
	 * Creates the main style tag for the page header.
	 *
	 * @param WhitePagesProfile $profile profile
	 * @return string style tag
	 */
	public static function getMainStyleTag(WhitePagesProfile $profile): string {
		if (empty($profile->backgroundImage)) {
			$image = 'background-color: ' . $profile->backgroundColor . ';';
		}
		elseif (str_starts_with($profile->backgroundImage, 'http') || str_starts_with($profile->backgroundImage, '/')) {
			$image = 'background-image: url(' . htmlspecialchars($profile->backgroundImage) . ');';
		}
		else {
			$image = 'background-image: url(../../graphics/' . htmlspecialchars($profile->backgroundImage) . ');';
		}
		$backGroundColor = LamColorUtil::parseColor($profile->backgroundColor);
		return '<style>
        :root {
            --lam-background-color-primary: ' . htmlspecialchars($profile->primaryColor) . ';
            --lam-background-color-default: ' . htmlspecialchars($profile->backgroundColor) . ';
            --swal2-background: ' . htmlspecialchars($profile->backgroundColor) . ';
            --lam-border-color-primary: ' . htmlspecialchars(LamColorUtil::adjustBrightness($profile->primaryColor, 0.8)) . ';
            --lam-text-color-primary: ' . htmlspecialchars(LamColorUtil::getTextColor($profile->primaryColor)) . ';
            --lam-text-color-default: ' . htmlspecialchars(LamColorUtil::getTextColor($profile->backgroundColor)) . ';
            --lam-whitepages-color-primary: ' . htmlspecialchars($profile->primaryColor) . ';
            --lam-whitepages-color-background: ' . htmlspecialchars($profile->backgroundColor) . ';
            --lam-whitepages-color-panel: rgba(' . $backGroundColor->red . ', ' . $backGroundColor->green . ', ' . $backGroundColor->blue . ');
            --lam-whitepages-color-border: ' . htmlspecialchars(LamColorUtil::adjustBrightness($profile->backgroundColor, 0.8)) . ';
            --lam-whitepages-color-card: ' . htmlspecialchars(LamColorUtil::adjustBrightness($profile->backgroundColor, 2)) . ';
            --lam-whitepages-blur: 4px;
			--lam-table-background-color-bright: ' . htmlspecialchars(LamColorUtil::adjustBrightness($profile->backgroundColor, 1.5)) . ';
			--lam-table-background-color-dark: ' . htmlspecialchars(LamColorUtil::adjustBrightness($profile->backgroundColor, 0.95)) . ';
			--lam-table-background-color-hover: ' . htmlspecialchars(LamColorUtil::adjustBrightness($profile->backgroundColor, 0.8)) . ';
			--lam-table-text-color-hover: ' . htmlspecialchars(LamColorUtil::getTextColor(LamColorUtil::adjustBrightness($profile->backgroundColor, 0.8))) . ';
			--lam-table-border-color: ' . htmlspecialchars(LamColorUtil::adjustBrightness($profile->backgroundColor, 0.8)) . ';
			--lam-border-color: ' . htmlspecialchars(LamColorUtil::adjustBrightness($profile->backgroundColor, 0.8)) . ';
        }
        
		div.lam-full-size-background {
		  ' . $image . '
		  background-size: cover;
		  background-repeat: no-repeat;
		  background-position: center top;
		  width: 100vw;
		  height: 100vh;
		  position: absolute;
		  overflow: hidden;
		  top: 0;
		  left: 0;
		  z-index: -1;
		  filter: blur(var(--lam-whitepages-blur));
		}
    </style>';
	}

	/**
	 * Returns a list of installed background image names.
	 *
	 * @return string[] image file names
	 */
	public static function getBackgroundImageNames(): array {
		$dir = dir(__DIR__ . '/../graphics');
		if ($dir === false) {
			return [];
		}
		$imageNames = [];
		$fileName = $dir->read();
		while ($fileName !== false) {
			if (str_starts_with($fileName, 'background_')) {
				$imageNames[] = $fileName;
			}
			$fileName = $dir->read();
		}
		return $imageNames;
	}

}

/**
 * LDAP connection for white pages.
 */
class WhitePagesLdapConnection {

	private ?Connection $server = null;

	private WhitePagesProfile $profile;

	private bool $bindDone = false;

	/**
	 * Constructor
	 *
	 * @param WhitePagesProfile $profile profile
	 */
	public function __construct(WhitePagesProfile $profile) {
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
		$this->server = $this->openLdapConnection($this->profile);
		return $this->server;
	}

	/**
	 * Opens the LDAP connection and returns the handle. No bind is done.
	 *
	 * @param WhitePagesProfile $profile profile
	 * @return Connection|null LDAP handle or null if the connection failed
	 */
	private function openLdapConnection(WhitePagesProfile $profile): ?Connection {
		$server = connectToLDAP($profile->serverUrl, $profile->useTLS);
		if ($server != null) {
			// follow referrals
			ldap_set_option($server, LDAP_OPT_REFERRALS, $profile->followReferrals);
		}
		return $server;
	}

	/**
	 * Binds with the given credentials.
	 *
	 * @param string $userDn user DN
	 * @param string $password password
	 * @return bool bind was successful
	 */
	public function bind(string $userDn, string $password): bool {
		if ($this->profile->useForAllOperations) {
			$userDn = $this->profile->ldapConnectionUser;
			$password = deobfuscateText($this->profile->ldapConnectionPassword);
		}
		logNewMessage(LOG_DEBUG, 'White pages, bind with ' . $userDn);
		$result = @ldap_bind($this->getServer(), $userDn, $password);
		if ($result) {
			$this->bindDone = true;
		}
		return $result;
	}

	/**
	 * Closes connection to the LDAP server before serialization.
	 *
	 * @return array<string, WhitePagesProfile> data
	 */
	public function __serialize(): array {
		if ($this->server !== null) {
			@ldap_close($this->server);
			$this->server = null;
		}
		// define which attributes to save
		return ['profile' => $this->profile];
	}

	/**
	 * Restores the profile.
	 *
	 * @param array<string, WhitePagesProfile> $data data
	 */
	public function __unserialize(array $data): void {
		$this->profile = $data['profile'];
	}

	/**
	 * Returns if a bind was already done.
	 *
	 * @return bool bind was done
	 */
	public function isBinded(): bool {
		return $this->bindDone;
	}

}

/**
 * Utility class for search support.
 */
class WhitePagesSearch {

	private const SEPARATOR = '|';

	/**
	 * Generates the search data.
	 *
	 * @param array<string, string[]|string> $attributes LDAP attributes
	 * @param string[] $searchAttributes LDAP attributes to add to search (lower-case)
	 * @return string search string
	 */
	public static function getSearchData(array $attributes, array $searchAttributes): string {
		$data = '';
		foreach ($searchAttributes as $searchAttribute) {
			if (!isset($attributes[$searchAttribute])) {
				continue;
			}
			if (is_array($attributes[$searchAttribute])) {
				$data .= self::SEPARATOR . implode(self::SEPARATOR, $attributes[$searchAttribute]);
			}
			else {
				$data .= self::SEPARATOR . $attributes[$searchAttribute];
			}
		}
		return $data;
	}

}
