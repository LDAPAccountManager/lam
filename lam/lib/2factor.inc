<?php
namespace LAM\LIB\TWO_FACTOR;
use htmlLinkCss;
use \htmlResponsiveRow;
use \LAM\LOGIN\WEBAUTHN\WebauthnManager;
use \selfServiceProfile;
use \LAMConfig;
use \htmlScript;
use \htmlIframe;
use \htmlImage;
use \htmlButton;
use \htmlJavaScript;
use \htmlStatusMessage;
use \htmlOutputText;
use \htmlDiv;
use \LAMException;
use \Webauthn\PublicKeyCredentialCreationOptions;

/*
  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2017 - 2020  Roland Gruber

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

	/**
	 * Returns if the service has a custom input form.
	 * In this case the token field is not displayed.
	 *
	 * @return has custom input form
	 */
	public function hasCustomInputForm();

	/**
	 * Adds the custom input fields to the form.
	 *
	 * @param htmlResponsiveRow $row row where to add the input fields
	 * @param string $userDn user DN
	 */
	public function addCustomInput(&$row, $userDn);

	/**
	 * Returns if the submit button should be shown.
	 *
	 * @return bool show submit button
	 */
	public function isShowSubmitButton();
}

/**
 * Base class for 2-factor authentication providers.
 *
 * @author Roland Gruber
 */
abstract class BaseProvider implements TwoFactorProvider {

	protected $config;

	/**
	 * {@inheritDoc}
	 * @see \LAM\LIB\TWO_FACTOR\TwoFactorProvider::hasCustomInputForm()
	 */
	public function hasCustomInputForm() {
		return false;
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\LIB\TWO_FACTOR\TwoFactorProvider::addCustomInput()
	 */
	public function addCustomInput(&$row, $userDn) {
		// must be filled by subclass if used
	}

	/**
	 * Returns the value of the user attribute in LDAP.
	 *
	 * @param string $userDn user DN
	 * @return string user name
	 */
	protected function getLoginAttributeValue($userDn) {
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
	 * @see \LAM\LIB\TWO_FACTOR\TwoFactorProvider::isShowSubmitButton()
	 */
	public function isShowSubmitButton() {
		return true;
	}

}

/**
 * Provider for privacyIDEA.
 */
class PrivacyIDEAProvider extends BaseProvider {

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
class YubicoProvider extends BaseProvider {

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
		$urls = $this->config->twoFactorAuthenticationURL;
		shuffle($urls);
		$httpsverify = !$this->config->twoFactorAuthenticationInsecure;
		$clientId = $this->config->twoFactorAuthenticationClientId;
		$secretKey = $this->config->twoFactorAuthenticationSecretKey;
		foreach ($urls as $url) {
			try {
				$auth = new \Auth_Yubico($clientId, $secretKey, $url, $httpsverify);
				$auth->verify($twoFactorInput);
				return true;
			}
			catch (LAMException $e) {
				logNewMessage(LOG_DEBUG, 'Unable to verify 2FA: ' . $e->getMessage());
			}
		}
		return false;
	}

}

/**
 * Provider for DUO.
 */
class DuoProvider extends BaseProvider {

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
		return array('DUO');
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\LIB\TWO_FACTOR\TwoFactorProvider::isShowSubmitButton()
	 */
	public function isShowSubmitButton() {
		return false;
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\LIB\TWO_FACTOR\TwoFactorProvider::hasCustomInputForm()
	 */
	public function hasCustomInputForm() {
		return true;
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\LIB\TWO_FACTOR\BaseProvider::addCustomInput()
	 */
	public function addCustomInput(&$row, $userDn) {
		$loginAttribute = $this->getLoginAttributeValue($userDn);
		$aKey = $this->getAKey();
		include_once(__DIR__ . "/3rdParty/duo/Web.php");
		$signedRequest = \Duo\Web::signRequest($this->config->twoFactorAuthenticationClientId,
				$this->config->twoFactorAuthenticationSecretKey,
				$aKey,
				$loginAttribute);
		if ($this->config->isSelfService) {
			$row->add(new htmlScript("../lib/extra/duo/Duo-Web-v2.js", false, false), 12);
		}
		else {
			$row->add(new htmlScript("lib/extra/duo/Duo-Web-v2.js", false, false), 12);
		}
		$iframe = new htmlIframe('duo_iframe');
		$iframe->addDataAttribute('host', $this->config->twoFactorAuthenticationURL);
		$iframe->addDataAttribute('sig-request', $signedRequest);
		$row->add($iframe, 12);
	}

	/**
	 * Returns the aKey.
	 *
	 * @return String aKey
	 */
	private function getAKey() {
		if (empty($_SESSION['duo_akey'])) {
			$_SESSION['duo_akey'] = generateRandomPassword(40);
		}
		return $_SESSION['duo_akey'];
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\LIB\TWO_FACTOR\TwoFactorProvider::verify2ndFactor()
	 */
	public function verify2ndFactor($user, $password, $serial, $twoFactorInput) {
		logNewMessage(LOG_DEBUG, 'DuoProvider: Checking 2nd factor for ' . $user);
		$loginAttribute = $this->getLoginAttributeValue($user);
		$response = $_POST['sig_response'];
		include_once(__DIR__ . "/3rdParty/duo/Web.php");
		$result = \Duo\Web::verifyResponse(
				$this->config->twoFactorAuthenticationClientId,
				$this->config->twoFactorAuthenticationSecretKey,
				$this->getAKey(),
				$response);
		if ($result === $loginAttribute) {
			return true;
		}
		logNewMessage(LOG_ERR, 'DUO authentication failed');
		return false;
	}

}

/**
 * Provider for Okta.
 */
class OktaProvider extends BaseProvider {

	/**
	 * Constructor.
	 *
	 * @param TwoFactorConfiguration $config configuration
	 */
	public function __construct(&$config) {
		$this->config = $config;
		if (empty($this->config->twoFactorAuthenticationSerialAttributeName)) {
			$this->config->twoFactorAuthenticationSerialAttributeName = 'mail';
		}
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\LIB\TWO_FACTOR\TwoFactorProvider::getSerials()
	 */
	public function getSerials($user, $password) {
		return array('OKTA');
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\LIB\TWO_FACTOR\TwoFactorProvider::isShowSubmitButton()
	 */
	public function isShowSubmitButton() {
		return false;
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\LIB\TWO_FACTOR\TwoFactorProvider::hasCustomInputForm()
	 */
	public function hasCustomInputForm() {
		return true;
	}

	/**
	 * {@inheritDoc}
	 * @throws LAMException error building custom input
	 * @see \LAM\LIB\TWO_FACTOR\BaseProvider::addCustomInput()
	 */
	public function addCustomInput(&$row, $userDn) {
		$loginAttribute = $this->getLoginAttributeValue($userDn);
		if (empty($loginAttribute)) {
			throw new LAMException('Unable to read login attribute from ' . $userDn);
		}
		$content = new htmlResponsiveRow();
		$content->add(new htmlDiv('lam-okta-container', new htmlOutputText('')), 12);
		$content->add(new htmlScript('https://global.oktacdn.com/okta-signin-widget/4.5.2/js/okta-sign-in.min.js'), 12);
		$content->add(new htmlLinkCss('https://global.oktacdn.com/okta-signin-widget/4.5.2/css/okta-sign-in.min.css'), 12);
		$jsBlock = '
			var interval = setInterval(function() {
			    if ((typeof OktaSignIn === \'function\')) {
			        clearInterval(interval);
					var signIn = new OktaSignIn({
					    el: \'#lam-okta-container\',
					    logo: \'../graphics/logo_192x192.png\',
					    username: \'' . $loginAttribute . '\',
					    baseUrl: \'' . $this->config->twoFactorAuthenticationURL . '\'
					});
					signIn.showSignInToGetTokens({
						clientId: \'' . $this->config->twoFactorAuthenticationClientId . '\'
						});
			    }
			  }, 200);
		';
		$content->add(new htmlJavaScript($jsBlock), 12);
		$row->add($content, 12);
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\LIB\TWO_FACTOR\TwoFactorProvider::verify2ndFactor()
	 */
	public function verify2ndFactor($user, $password, $serial, $twoFactorInput) {
		logNewMessage(LOG_DEBUG, 'DuoProvider: Checking 2nd factor for ' . $user);
		$loginAttribute = $this->getLoginAttributeValue($user);
		$response = $_POST['sig_response'];
		include_once(__DIR__ . "/3rdParty/duo/Web.php");
		$result = \Duo\Web::verifyResponse(
			$this->config->twoFactorAuthenticationClientId,
			$this->config->twoFactorAuthenticationSecretKey,
			$this->getAKey(),
			$response);
		if ($result === $loginAttribute) {
			return true;
		}
		logNewMessage(LOG_ERR, 'DUO authentication failed');
		return false;
	}

}

/**
 * Provider for Webauthn.
 */
class WebauthnProvider extends BaseProvider {

	/**
	 * Constructor.
	 *
	 * @param TwoFactorConfiguration $config configuration
	 */
	public function __construct($config) {
		$this->config = $config;
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\LIB\TWO_FACTOR\TwoFactorProvider::getSerials()
	 */
	public function getSerials($user, $password) {
		return array('WEBAUTHN');
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\LIB\TWO_FACTOR\TwoFactorProvider::isShowSubmitButton()
	 */
	public function isShowSubmitButton() {
		return false;
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\LIB\TWO_FACTOR\TwoFactorProvider::hasCustomInputForm()
	 */
	public function hasCustomInputForm() {
		return true;
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\LIB\TWO_FACTOR\BaseProvider::addCustomInput()
	 */
	public function addCustomInput(&$row, $userDn) {
		if (version_compare(phpversion(), '7.2.0') < 0) {
			$row->add(new htmlStatusMessage('ERROR', 'WebAuthn requires PHP 7.2.'), 12);
			return;
		}
		if (!extension_loaded('PDO')) {
			$row->add(new htmlStatusMessage('ERROR', 'WebAuthn requires the PDO extension for PHP.'), 12);
			return;
		}
		$pdoDrivers = \PDO::getAvailableDrivers();
		if (!in_array('sqlite', $pdoDrivers)) {
			$row->add(new htmlStatusMessage('ERROR', 'WebAuthn requires the sqlite PDO driver for PHP.'), 12);
			return;
		}
		include_once __DIR__ . '/webauthn.inc';
		$webauthnManager = $this->getWebauthnManager();
		$hasTokens = $webauthnManager->isRegistered($userDn);
		if ($hasTokens) {
			$row->add(new htmlStatusMessage('INFO', _('Please authenticate with your security device.')), 12);
		}
		else {
			$row->add(new htmlStatusMessage('INFO', _('Please register a security device.')), 12);
		}
		$row->addVerticalSpacer('2rem');
		$pathPrefix = $this->config->isSelfService ? '../' : '';
		$selfServiceParam = $this->config->isSelfService ? 'true' : 'false';
		$row->add(new htmlImage($pathPrefix . '../graphics/webauthn.svg'), 12);
		$row->addVerticalSpacer('1rem');
		$registerButton = new htmlButton('register_webauthn', _('Register new key'));
		$registerButton->setCSSClasses(array('fullwidth hidden'));
		$row->add($registerButton, 12);
		$loginButton = new htmlButton('login_webauthn', _('Login'));
		$loginButton->setCSSClasses(array('fullwidth hidden'));
		$row->add($loginButton, 12);
		$errorMessage = new htmlStatusMessage('ERROR', '', _('This service requires a browser with "WebAuthn" support.'));
		$row->add(new htmlDiv(null, $errorMessage, array('hidden webauthn-error')), 12);
		if (($this->config->twoFactorAuthenticationOptional === true) && !$hasTokens) {
			$skipButton = new htmlButton('skip_webauthn', _('Skip'));
			$skipButton->setCSSClasses(array('fullwidth'));
			$row->add($skipButton, 12);
		}
		$errorMessageDiv = new htmlDiv('generic-webauthn-error', new htmlOutputText(''));
		$errorMessageDiv->addDataAttribute('button', _('Ok'));
		$errorMessageDiv->addDataAttribute('title', _('WebAuthn failed'));
		$row->add($errorMessageDiv, 12);
		$row->add(new htmlJavaScript('window.lam.webauthn.start(\'' . $pathPrefix . '\', ' . $selfServiceParam . ');'), 0);
	}

	/**
	 * Returns the webauthn manager.
	 *
	 * @return WebauthnManager manager
	 */
	public function getWebauthnManager() {
		return new WebauthnManager();
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\LIB\TWO_FACTOR\TwoFactorProvider::verify2ndFactor()
	 */
	public function verify2ndFactor($user, $password, $serial, $twoFactorInput) {
		logNewMessage(LOG_DEBUG, 'WebauthnProvider: Checking 2nd factor for ' . $user);
		include_once __DIR__ . '/webauthn.inc';
		$webauthnManager = $this->getWebauthnManager();
		if (!empty($_SESSION['ldap'])) {
			$userDn = $_SESSION['ldap']->getUserName();
		}
		else {
			$userDn = lamDecrypt($_SESSION['selfService_clientDN'], 'SelfService');
		}
		$hasTokens = $webauthnManager->isRegistered($userDn);
		if (!$hasTokens) {
			if ($this->config->twoFactorAuthenticationOptional && !$webauthnManager->isRegistered($user) && ($_POST['sig_response'] === 'skip')) {
				logNewMessage(LOG_DEBUG, 'Skipped 2FA for ' . $user . ' as no devices are registered and 2FA is optional.');
				return true;
			}
			$response = base64_decode($_POST['sig_response']);
			$registrationObject = PublicKeyCredentialCreationOptions::createFromString($_SESSION['webauthn_registration']);
			return $webauthnManager->storeNewRegistration($registrationObject, $response);
		}
		else {
			logNewMessage(LOG_DEBUG, 'Checking WebAuthn response of ' . $userDn);
			$response = base64_decode($_POST['sig_response']);
			return $webauthnManager->isValidAuthentication($response, $userDn);
		}
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
	/** 2factor authentication via DUO */
	const TWO_FACTOR_DUO = 'duo';
	/** 2factor authentication via webauthn */
	const TWO_FACTOR_WEBAUTHN = 'webauthn';
	/** 2factor authentication via Okta */
	const TWO_FACTOR_OKTA = 'okta';

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
		elseif ($this->config->twoFactorAuthentication == TwoFactorProviderService::TWO_FACTOR_DUO) {
			return new DuoProvider($this->config);
		}
		elseif ($this->config->twoFactorAuthentication == TwoFactorProviderService::TWO_FACTOR_WEBAUTHN) {
			return new WebauthnProvider($this->config);
		}
		elseif ($this->config->twoFactorAuthentication == TwoFactorProviderService::TWO_FACTOR_OKTA) {
			return new OktaProvider($this->config);
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
		$tfConfig->isSelfService = true;
		$tfConfig->twoFactorAuthentication = $profile->twoFactorAuthentication;
		$tfConfig->twoFactorAuthenticationInsecure = $profile->twoFactorAuthenticationInsecure;
		$tfConfig->twoFactorAuthenticationOptional = $profile->twoFactorAuthenticationOptional;
		if ($tfConfig->twoFactorAuthentication == TwoFactorProviderService::TWO_FACTOR_YUBICO) {
			$tfConfig->twoFactorAuthenticationURL = explode("\r\n", $profile->twoFactorAuthenticationURL);
		}
		else {
			$tfConfig->twoFactorAuthenticationURL = $profile->twoFactorAuthenticationURL;
		}
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
		if (($tfConfig->twoFactorAuthentication == TwoFactorProviderService::TWO_FACTOR_PRIVACYIDEA)
				|| ($tfConfig->twoFactorAuthentication == TwoFactorProviderService::TWO_FACTOR_DUO)
				|| ($tfConfig->twoFactorAuthentication == TwoFactorProviderService::TWO_FACTOR_OKTA)) {
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
		$tfConfig->isSelfService = false;
		$tfConfig->twoFactorAuthentication = $conf->getTwoFactorAuthentication();
		$tfConfig->twoFactorAuthenticationInsecure = $conf->getTwoFactorAuthenticationInsecure();
		$tfConfig->twoFactorAuthenticationOptional = $conf->getTwoFactorAuthenticationOptional();
		if ($tfConfig->twoFactorAuthentication == TwoFactorProviderService::TWO_FACTOR_YUBICO) {
			$tfConfig->twoFactorAuthenticationURL = explode("\r\n", $conf->getTwoFactorAuthenticationURL());
		}
		else {
			$tfConfig->twoFactorAuthenticationURL = $conf->getTwoFactorAuthenticationURL();
		}
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
		if (($tfConfig->twoFactorAuthentication == TwoFactorProviderService::TWO_FACTOR_PRIVACYIDEA)
				|| ($tfConfig->twoFactorAuthentication == TwoFactorProviderService::TWO_FACTOR_DUO)
				|| ($tfConfig->twoFactorAuthentication == TwoFactorProviderService::TWO_FACTOR_OKTA)) {
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
	 * @var bool is self service
	 */
	public $isSelfService = false;

	/**
	 * @var string provider id
	 */
	public $twoFactorAuthentication = null;

	/**
	 * @var string|array service URL(s)
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

	/**
	 * @var bool 2FA is optional
	 */
	public $twoFactorAuthenticationOptional = false;

}
