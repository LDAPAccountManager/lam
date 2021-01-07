<?php
use \LAM\LIB\TWO_FACTOR\TwoFactorProviderService;
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2006 - 2020  Roland Gruber

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
* Returns a list of all available self service profiles (without .conf)
*
* @return array profile names (array('account type' => array('profile1', 'profile2')))
*/
function getSelfServiceProfiles() {
	$types = LAM\TYPES\getTypes();
	$dir = dir(substr(__FILE__, 0, strlen(__FILE__) - 20) . "/config/selfService");
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
	ksort($ret);
	foreach ($ret as $key => $value) {
		sort($ret[$key]);
	}
	return $ret;
}


/**
* Loads all settings of a self service profile.
*
* @param string $name profile name
* @param string $scope account type
* @return false|selfServiceProfile profile or false if file was not readable
*/
function loadSelfServiceProfile($name, $scope) {
	if (!preg_match("/^[0-9a-z _-]+$/i", $name) || !preg_match("/^[0-9a-z _-]+$/i", $scope)) {
		return false;
	}
	$profile = new selfServiceProfile();
	$file = substr(__FILE__, 0, strlen(__FILE__) - 20) . "/config/selfService/" . $name . "." . $scope;
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
			StatusMessage("ERROR", "", _("Unable to load profile!") . " " . $name);
		}
	}
	else {
		StatusMessage("ERROR", "", _("Unable to load profile!") . " " . $name);
	}
	return $profile;
}


/**
* Saves a self service profile.
*
* File is created, if needed
*
* @param string $name name of the account profile
* @param string $scope account type
* @param selfServiceProfile $profile self service profile
* @return boolean true, if saving succeeded
*/
function saveSelfServiceProfile($name, $scope, $profile) {
	// check profile name
	if (!preg_match("/^[0-9a-z _-]+$/i", $scope) || !preg_match("/^[0-9a-z _-]+$/i", $name)) {
		return false;
	}
	if (!get_class($profile) === 'selfServiceProfile') {
		return false;
	}
	$path = substr(__FILE__, 0, strlen(__FILE__) - 20) . "/config/selfService/" . $name . "." . $scope;
	$file = @fopen($path, "w");
	if ($file) {
	  	// write settings to file
		fputs($file, json_encode($profile->export()));
		// close file
		fclose($file);
	}
	else {
		return false;
	}
	return true;
}

/**
* Checks if a service profile is writable.
*
* @param string $name profile name
* @param string $scope account type
* @return boolean true if file is writable
*/
function isSelfServiceProfileWritable($name, $scope) {
	// check profile name
	if (!preg_match("/^[0-9a-z _-]+$/i", $scope) || !preg_match("/^[0-9a-z _-]+$/i", $name)) {
		return false;
	}
	$path = substr(__FILE__, 0, strlen(__FILE__) - 20) . "/config/selfService/" . $name . "." . $scope;
	return is_writable($path);
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
		$this->pageHeader = '<table border=0 width="100%" class="lamHeader ui-corner-all"><tr><td align="left" height="30"><a class="lamLogo" href="http://www.ldap-account-manager.org/" target="new_window">LDAP Account Manager</a></td></tr></table><br>';
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
			if (in_array($key, $vars)) {
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

?>