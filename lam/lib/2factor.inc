<?php
namespace LAM\LIB\TWO_FACTOR;
use \selfServiceProfile;
use \LAMConfig;

/*
  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2017 - 2019  Roland Gruber

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
* 2-factor authentication
*
* @package two_factor
* @author Roland Gruber
*/

interface TwoFactorProvider {

	/**
	 * Returns a list of serial numbers of the user's tokens.
	 *
	 * @param string $user user name
	 * @param string $password password
	 * @return string[] serials
	 * @throws \Exception error getting serials
	 */
	public function getSerials($user, $password);

	/**
	 * Verifies if the provided 2nd factor is valid.
	 *
	 * @param string $user user name
	 * @param string $password password
	 * @param string $serial serial number of token
	 * @param string $twoFactorInput input for 2nd factor
	 * @return boolean true if verified and false if verification failed
	 * @throws \Exception error during check
	 */
	public function verify2ndFactor($user, $password, $serial, $twoFactorInput);

}

/**
 * Provider for privacyIDEA.
 */
class PrivacyIDEAProvider implements TwoFactorProvider {

	private $config;

	/**
	 * Constructor.
	 *
	 * @param TwoFactorConfiguration $config configuration
	 */
	public function __construct(&$config) {
		$this->config = $config;
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\LIB\TWO_FACTOR\TwoFactorProvider::getSerials()
	 */
	public function getSerials($user, $password) {
		logNewMessage(LOG_DEBUG, 'PrivacyIDEAProvider: Getting serials for ' . $user);
		$loginAttribute = $this->getLoginAttributeValue($user);
		$token = $this->authenticate($loginAttribute, $password);
		return $this->getSerialsForUser($loginAttribute, $token);
	}

	/**
	 * Returns the value of the user attribute in LDAP.
	 *
	 * @param string $userDn user DN
	 * @return string user name
	 */
	private function getLoginAttributeValue($userDn) {
		$attrName = $this->config->twoFactorAuthenticationSerialAttributeName;
		$userData = ldapGetDN($userDn, array($attrName));
		if (empty($userData[$attrName])) {
			return null;
		}
		if (is_array($userData[$attrName])) {
			return $userData[$attrName][0];
		}
		return $userData[$attrName];
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\LIB\TWO_FACTOR\TwoFactorProvider::verify2ndFactor()
	 */
	public function verify2ndFactor($user, $password, $serial, $twoFactorInput) {
		logNewMessage(LOG_DEBUG, 'PrivacyIDEAProvider: Checking 2nd factor for ' . $user);
		$loginAttribute = $this->getLoginAttributeValue($user);
		$token = $this->authenticate($loginAttribute, $password);
		return $this->verify($token, $serial, $twoFactorInput);
	}

	/**
	 * Authenticates against the server
	 *
	 * @param string $user user name
	 * @param string $password password
	 * @return string token
	 * @throws \Exception error during authentication
	 */
	private function authenticate($user, $password) {
		$curl = $this->getCurl();
		$url = $this->config->twoFactorAuthenticationURL . "/auth";
		curl_setopt($curl, CURLOPT_URL, $url);
		$header = array('Accept: application/json');
		curl_setopt($curl, CURLOPT_HTTPHEADER, $header);
		$options = array(
			'username' => $user,
			'password' => $password,
		);
		curl_setopt($curl, CURLOPT_POSTFIELDS, $options);
		$json = curl_exec($curl);
		curl_close($curl);
		if (empty($json)) {
			throw new \Exception("Unable to get server response from $url.");
		}
		$output = json_decode($json);
		if (empty($output) || !isset($output->result) || !isset($output->result->status)) {
			throw new \Exception("Unable to get json from $url.");
		}
		$status = $output->result->status;
		if ($status != 1) {
			$errCode = isset($output->result->error) && isset($output->result->error->code) ? $output->result->error->code : '';
			$errMessage = isset($output->result->error) && isset($output->result->error->message) ? $output->result->error->message : '';
			throw new \Exception("Unable to login: " . $errCode . ' ' . $errMessage);
		}
		if (!isset($output->result->value) || !isset($output->result->value->token)) {
			throw new \Exception("Unable to get token.");
		}
		return $output->result->value->token;
	}

	/**
	 * Returns the curl object.
	 *
	 * @return object curl handle
	 * @throws \Exception error during curl creation
	 */
	private function getCurl() {
		$curl = curl_init();
		if ($this->config->twoFactorAuthenticationInsecure) {
			curl_setopt($curl, CURLOPT_SSL_VERIFYPEER, false);
			curl_setopt($curl, CURLOPT_SSL_VERIFYHOST, 0);
		}
		curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
		return $curl;
	}

	/**
	 * Returns the serial numbers of the user.
	 *
	 * @param string $user user name
	 * @param string $token login token
	 * @return string[] serials
	 * @throws \Exception error during serial reading
	 */
	private function getSerialsForUser($user, $token) {
		$curl = $this->getCurl();
		$url = $this->config->twoFactorAuthenticationURL . "/token/?user=" . $user;
		curl_setopt($curl, CURLOPT_URL, $url);
		$header = array('Authorization: ' . $token, 'Accept: application/json');
		curl_setopt($curl, CURLOPT_HTTPHEADER, $header);
		$json = curl_exec($curl);
		curl_close($curl);
		if (empty($json)) {
			throw new \Exception("Unable to get server response from $url.");
		}
		$output = json_decode($json);
		if (empty($output) || !isset($output->result) || !isset($output->result->status)) {
			throw new \Exception("Unable to get json from $url.");
		}
		$status = $output->result->status;
		if (($status != 1) || !isset($output->result->value) || !isset($output->result->value->tokens)) {
			$errCode = isset($output->result->error) && isset($output->result->error->code) ? $output->result->error->code : '';
			$errMessage = isset($output->result->error) && isset($output->result->error->message) ? $output->result->error->message : '';
			throw new \Exception("Unable to get serials: " . $errCode . ' ' . $errMessage);
		}
		$serials = array();
		foreach ($output->result->value->tokens as $tokenEntry) {
			if (!isset($tokenEntry->active) || ($tokenEntry->active != 1) || !isset($tokenEntry->serial)) {
				continue;
			}
			$serials[] = $tokenEntry->serial;
		}
		return $serials;
	}

	/**
	 * Verifies if the given 2nd factor input is valid.
	 *
	 * @param string $token login token
	 * @param string $serial serial number
	 * @param string $twoFactorInput 2factor pin + password
	 */
	private function verify($token, $serial, $twoFactorInput) {
		$curl = $this->getCurl();
		$url = $this->config->twoFactorAuthenticationURL . "/validate/check";
		curl_setopt($curl, CURLOPT_URL, $url);
		$options = array(
			'pass' => $twoFactorInput,
			'serial' => $serial,
		);
		curl_setopt($curl, CURLOPT_POSTFIELDS, $options);
		$header = array('Authorization: ' . $token, 'Accept: application/json');
		curl_setopt($curl, CURLOPT_HTTPHEADER, $header);
		$json = curl_exec($curl);
		curl_close($curl);
		$output = json_decode($json);
		if (empty($output) || !isset($output->result) || !isset($output->result->status) || !isset($output->result->value)) {
			throw new \Exception("Unable to get json from $url.");
		}
		$status = $output->result->status;
		$value = $output->result->value;
		if (($status == 'true') && ($value == 'true')) {
			return true;
		}
		logNewMessage(LOG_DEBUG, "Unable to verify token: " . print_r($output, true));
		return false;
	}

}

/**
 * Authentication via YubiKeys.
 *
 * @author Roland Gruber
 */
class YubicoProvider implements TwoFactorProvider {

	private $config;

	/**
	 * Constructor.
	 *
	 * @param TwoFactorConfiguration $config configuration
	 */
	public function __construct(&$config) {
		$this->config = $config;
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\LIB\TWO_FACTOR\TwoFactorProvider::getSerials()
	 */
	public function getSerials($user, $password) {
		$keyAttributeName = strtolower($this->config->twoFactorAuthenticationSerialAttributeName);
		if (isset($_SESSION['selfService_clientDN'])) {
			$loginDn = lamDecrypt($_SESSION['selfService_clientDN'], 'SelfService');
		}
		else {
			$loginDn = $_SESSION['ldap']->getUserName();
		}
		$handle = getLDAPServerHandle();
		$ldapData = ldapGetDN($loginDn, array($keyAttributeName), $handle);
		if (empty($ldapData[$keyAttributeName])) {
			return array();
		}
		return array(implode(', ', $ldapData[$keyAttributeName]));
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\LIB\TWO_FACTOR\TwoFactorProvider::verify2ndFactor()
	 */
	public function verify2ndFactor($user, $password, $serial, $twoFactorInput) {
		include_once(__DIR__ . "/3rdParty/yubico/Yubico.php");
		$serialData = $this->getSerials($user, $password);
		if (empty($serialData)) {
			return false;
		}
		$serials = explode(', ', $serialData[0]);
		$serialMatched = false;
		foreach ($serials as $serial) {
			if (strpos($twoFactorInput, $serial) === 0) {
				$serialMatched = true;
				break;
			}
		}
		if (!$serialMatched) {
			throw new \Exception(_('YubiKey id does not match allowed list of key ids.'));
		}
		$url = $this->config->twoFactorAuthenticationURL;
		$httpsverify = !$this->config->twoFactorAuthenticationInsecure;
		$clientId = $this->config->twoFactorAuthenticationClientId;
		$secretKey = $this->config->twoFactorAuthenticationSecretKey;
		$auth = new \Auth_Yubico($clientId, $secretKey, $url, $httpsverify);
		$auth->verify($twoFactorInput);
		return true;
	}

}

/**
 * Returns the correct 2 factor provider.
 */
class TwoFactorProviderService {

	/** 2factor authentication disabled */
	const TWO_FACTOR_NONE = 'none';
	/** 2factor authentication via privacyIDEA */
	const TWO_FACTOR_PRIVACYIDEA = 'privacyidea';
	/** 2factor authentication via YubiKey */
	const TWO_FACTOR_YUBICO = 'yubico';

	private $config;

	/**
	 * Constructor.
	 *
	 * @param selfServiceProfile|LAMConfig $configObj profile
	 */
	public function __construct(&$configObj) {
		if ($configObj instanceof selfServiceProfile) {
			$this->config = $this->getConfigSelfService($configObj);
		}
		else {
			$this->config = $this->getConfigAdmin($configObj);
		}
	}

	/**
	 * Returns the provider for the given type.
	 *
	 * @param string $type authentication type
	 * @return TwoFactorProvider provider
	 * @throws \Exception unable to get provider
	 */
	public function getProvider() {
		if ($this->config->twoFactorAuthentication == TwoFactorProviderService::TWO_FACTOR_PRIVACYIDEA) {
			return new PrivacyIDEAProvider($this->config);
		}
		elseif ($this->config->twoFactorAuthentication == TwoFactorProviderService::TWO_FACTOR_YUBICO) {
			return new YubicoProvider($this->config);
		}
		throw new \Exception('Invalid provider: ' . $this->config->twoFactorAuthentication);
	}

	/**
	 * Returns the configuration from self service.
	 *
	 * @param selfServiceProfile $profile profile
	 * @return TwoFactorConfiguration configuration
	 */
	private function getConfigSelfService(&$profile) {
		$tfConfig = new TwoFactorConfiguration();
		$tfConfig->twoFactorAuthentication = $profile->twoFactorAuthentication;
		$tfConfig->twoFactorAuthenticationInsecure = $profile->twoFactorAuthenticationInsecure;
		$tfConfig->twoFactorAuthenticationURL = $profile->twoFactorAuthenticationURL;
		$tfConfig->twoFactorAuthenticationClientId = $profile->twoFactorAuthenticationClientId;
		$tfConfig->twoFactorAuthenticationSecretKey = $profile->twoFactorAuthenticationSecretKey;
		if ($tfConfig->twoFactorAuthentication == TwoFactorProviderService::TWO_FACTOR_YUBICO) {
			$moduleSettings = $profile->moduleSettings;
			if (!empty($moduleSettings['yubiKeyUser_attributeName'][0])) {
				$tfConfig->twoFactorAuthenticationSerialAttributeName = $moduleSettings['yubiKeyUser_attributeName'][0];
			}
			else {
				$tfConfig->twoFactorAuthenticationSerialAttributeName = 'yubiKeyId';
			}
		}
		if ($tfConfig->twoFactorAuthentication == TwoFactorProviderService::TWO_FACTOR_PRIVACYIDEA) {
			$attrName = $profile->twoFactorAuthenticationAttribute;
			if (empty($attrName)) {
				$attrName = 'uid';
			}
			$tfConfig->twoFactorAuthenticationSerialAttributeName = strtolower($attrName);
		}
		return $tfConfig;
	}

	/**
	 * Returns the configuration for admin interface.
	 *
	 * @param LAMConfig $conf configuration
	 * @return TwoFactorConfiguration configuration
	 */
	private function getConfigAdmin($conf) {
		$tfConfig = new TwoFactorConfiguration();
		$tfConfig->twoFactorAuthentication = $conf->getTwoFactorAuthentication();
		$tfConfig->twoFactorAuthenticationInsecure = $conf->getTwoFactorAuthenticationInsecure();
		$tfConfig->twoFactorAuthenticationURL = $conf->getTwoFactorAuthenticationURL();
		$tfConfig->twoFactorAuthenticationClientId = $conf->getTwoFactorAuthenticationClientId();
		$tfConfig->twoFactorAuthenticationSecretKey = $conf->getTwoFactorAuthenticationSecretKey();
		if ($tfConfig->twoFactorAuthentication == TwoFactorProviderService::TWO_FACTOR_YUBICO) {
			$moduleSettings = $conf->get_moduleSettings();
			if (!empty($moduleSettings['yubiKeyUser_attributeName'][0])) {
				$tfConfig->twoFactorAuthenticationSerialAttributeName = $moduleSettings['yubiKeyUser_attributeName'][0];
			}
			else {
				$tfConfig->twoFactorAuthenticationSerialAttributeName = 'yubiKeyId';
			}
		}
		if ($tfConfig->twoFactorAuthentication == TwoFactorProviderService::TWO_FACTOR_PRIVACYIDEA) {
			$tfConfig->twoFactorAuthenticationSerialAttributeName = strtolower($conf->getTwoFactorAuthenticationAttribute());
		}
		return $tfConfig;
	}

}

/**
 * Configuration settings for 2-factor authentication.
 *
 * @author Roland Gruber
 */
class TwoFactorConfiguration {

	/**
	 * @var string provider id
	 */
	public $twoFactorAuthentication = null;

	/**
	 * @var service URL
	 */
	public $twoFactorAuthenticationURL = null;

	/**
	 * @var disable certificate check
	 */
	public $twoFactorAuthenticationInsecure = false;

	/**
	 * @var client ID for API access
	 */
	public $twoFactorAuthenticationClientId = null;

	/**
	 * @var secret key for API access
	 */
	public $twoFactorAuthenticationSecretKey = null;

	/**
	 * @var LDAP attribute name that stores the token serials
	 */
	public $twoFactorAuthenticationSerialAttributeName = null;

}
