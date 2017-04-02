<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2006 - 2016  Roland Gruber

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
* This file includes functions to perform several security checks on each page load.
*
* @package lib
* @author Roland Gruber
*/

/** configuration options */
include_once('config.inc');
/** ldap connection */
include_once('ldap.inc');
/** common functions */
include_once('account.inc');

// check client IP address
checkClientIP();

setLAMHeaders();

/**
 * Starts a session and checks the environment.
 * The script is stopped if one of the checks fail (timeout redirection may be overriden).
 *
 * @param boolean $redirectToLogin redirect user to login page (default: true)
 * @param boolean $initSecureData init verification data like session ID and client IP (default: false)
 * @return boolean true if all ok, false if session expired
 */
function startSecureSession($redirectToLogin = true, $initSecureData = false) {
	// start session
	if (isset($_SESSION)) unset($_SESSION);
	if (strtolower(session_module_name()) == 'files') {
		$sessionDir = dirname(__FILE__) . "/../sess";
		session_save_path($sessionDir);
		// enable garbage collection (fix for Debian based systems)
		if (@ini_get("session.gc_probability") == 0) {
			@ini_set("session.gc_probability", 1);
		}
	}
	@session_start();
	// init secure data if needed
	if ($initSecureData && !isset($_SESSION["sec_session_id"])) {
		$_SESSION["sec_session_id"] = session_id();
		$_SESSION["sec_client_ip"] = $_SERVER['REMOTE_ADDR'];
		$_SESSION['sec_sessionTime'] = time();
		$_SESSION['cfgMain'] = new LAMCfgMain();
	}
	// set error reporting
	if (empty($_SESSION['cfgMain']) || ($_SESSION['cfgMain']->errorReporting == LAMCfgMain::ERROR_REPORTING_DEFAULT)) {
		ini_set('error_reporting', 'E_ALL & ~E_NOTICE');
	}
	elseif ($_SESSION['cfgMain']->errorReporting == LAMCfgMain::ERROR_REPORTING_ALL) {
		ini_set('error_reporting', E_ALL | E_STRICT);
		ini_set('display_errors', 'On');
	}
	// check session id
	if (! isset($_SESSION["sec_session_id"]) || ($_SESSION["sec_session_id"] != session_id())) {
		// session id is invalid
		logNewMessage(LOG_WARNING, "Invalid session ID, access denied (" . getClientIPForLogging() . ")");
		if ($redirectToLogin) {
			logoffAndBackToLoginPage();
		}
		else {
			die();
		}
	}
	// check if client IP has not changed
	if (!isset($_SESSION["sec_client_ip"]) || ($_SESSION["sec_client_ip"] != $_SERVER['REMOTE_ADDR'])) {
		// IP is invalid
		logNewMessage(LOG_WARNING, "Client IP changed, access denied (" . getClientIPForLogging() . ")");
		die();
	}
	// check if session time has not expired
	if (($_SESSION['sec_sessionTime'] + (60 * $_SESSION['cfgMain']->sessionTimeout)) > time()) {
		// ok, update time
		$_SESSION['sec_sessionTime'] = time();
	}
	elseif ($redirectToLogin) {
		// session expired, logoff user
		logoffAndBackToLoginPage();
	}
	else {
		return false;
	}
	setSSLCaCert();
	return true;
}

/**
 * Checks if the client's IP address is on the list of allowed IPs.
 * The script is stopped if the host is not valid.
 *
 */
function checkClientIP() {
	if (isset($_SESSION['cfgMain'])) $cfg = $_SESSION['cfgMain'];
	else $cfg = new LAMCfgMain();
	$allowedHosts = $cfg->allowedHosts;
	$url = getCallingURL();
	if ((strpos($url, '/selfService/selfService') !== false) || ((strpos($url, '/misc/ajax.php?') !== false) && strpos($url, 'selfservice=1') !== false)) {
		// self service pages have separate IP list
		$allowedHosts = $cfg->allowedHostsSelfService;
	}
	// skip test if no hosts are defined
	if (empty($allowedHosts) || empty($_SERVER['REMOTE_ADDR'])) {
		return;
	}
	$allowedHosts = explode(",", $allowedHosts);
	$grantAccess = false;
	for ($i = 0; $i < sizeof($allowedHosts); $i++) {
		$host = $allowedHosts[$i];
		$ipRegex = '/^[0-9a-z\\.:\\*]+$/i';
		if (!preg_match($ipRegex, $host)) continue;
		$hostRegex = str_replace(".", "\\.", $host);
		$hostRegex = '/^' . str_replace("*", ".*", $hostRegex) . '$/';
		$clientIP = $_SERVER['REMOTE_ADDR'];
		if (preg_match($hostRegex, $clientIP)) {
			// client is allowed to access LAM
			$grantAccess = true;
		}
	}
	// stop script is client may not access LAM
	if (!$grantAccess) {
		logNewMessage(LOG_WARNING, "Invalid client IP, access denied (" . getClientIPForLogging() . ")");
		die();
	}
}

/**
 * Logs off the user and displays the login page.
 *
 */
function logoffAndBackToLoginPage() {
	// log message
	if (isset($_SESSION['ldap'])) {
		$ldapUser = $_SESSION['ldap']->decrypt_login();
		logNewMessage(LOG_WARNING, 'Session of user ' . $ldapUser[0] . ' expired.');
		// close LDAP connection
		@$_SESSION["ldap"]->destroy();
	}
	elseif (isset($_SESSION['selfService_clientDN']) || (strpos($_SERVER['REQUEST_URI'], '/selfService/') !== false)) {
		logNewMessage(LOG_WARNING, 'Self service session of DN ' . lamDecrypt($_SESSION['selfService_clientDN'], 'SelfService') . ' expired.');
	}
	// delete key and iv in cookie
	if (function_exists('openssl_random_pseudo_bytes')) {
		setcookie("Key", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", 0, "/", null, null, true);
		setcookie("IV", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", 0, "/", null, null, true);
	}
	// link back to login page
	$paths = array('./', '../', '../../', '../../../', '../../../../');
	$page = 'login.php';
	$pageSuffix = '?expired=yes';
	if (isset($_SESSION['selfService_clientDN']) || (strpos($_SERVER['REQUEST_URI'], '/selfService/') !== false)) {
		$scope = $_GET['scope'];
		$name = $_GET['name'];
		if (!preg_match('/^[0-9a-zA-Z _-]+$/', $scope) || !preg_match('/^[0-9a-zA-Z _-]+$/', $name)) {
			logNewMessage(LOG_ERR, 'GET parameters invalid: ' . $name . ' ' . $scope);
			die();
		}
		$page = 'selfServiceLogin.php';
		$pageSuffix = '?expired=yes&scope=' . $scope . '&name=' . $name;
	}
	for ($i = 0; $i < sizeof($paths); $i++) {
		if (file_exists($paths[$i] . $page)) {
			$page = $paths[$i] . $page;
			break;
		}
	}
	$page .= $pageSuffix;
	echo $_SESSION['header'];
	echo "<title></title>\n";
	echo "</head>\n";
	echo "<body>\n";
	// print JavaScript refresh
	echo "<script type=\"text/javascript\">\n";
	echo "top.location.href = \"" . $page . "\";\n";
	echo "</script>\n";
	// print link if refresh does not work
	echo "<p>\n";
	echo "<a target=\"_top\" href=\"" . $page . "\">" . _("Your session expired, click here to go back to the login page.") . "</a>\n";
	echo "</p>\n";
	echo "</body>\n";
	echo "</html>\n";
	// destroy session
	session_destroy();
	unset($_SESSION);
	die();
}

/**
 * Puts a new message in the log file.
 *
 * @param string $level log level (LOG_DEBUG, LOG_NOTICE, LOG_WARNING, LOG_ERR)
 * @param string $message log message
 */
function logNewMessage($level, $message) {
	$possibleLevels = array(LOG_DEBUG => 'DEBUG', LOG_NOTICE => 'NOTICE', LOG_WARNING => 'WARNING', LOG_ERR => 'ERROR');
	if (!in_array($level, array_keys($possibleLevels))) StatusMessage('ERROR', 'Invalid log level!', $level);
	if (isset($_SESSION['cfgMain'])) $cfg = $_SESSION['cfgMain'];
	else $cfg = new LAMCfgMain();
	// check if logging is disabled
	if ($cfg->logDestination == 'NONE') return;
	// check if log level is high enough
	elseif ($cfg->logLevel < $level) return;
	// ok to log, build log message
	$prefix = "LDAP Account Manager (" . session_id() . ' - ' . getClientIPForLogging() . ") - " . $possibleLevels[$level] . ": ";
	$message = $prefix . $message;
	// Syslog logging
	if ($cfg->logDestination == 'SYSLOG') {
		syslog($level, $message);
	}
	// log to file
	else {
		@touch($cfg->logDestination);
		if (is_writable($cfg->logDestination)) {
			$file = fopen($cfg->logDestination, 'a');
			if ($file) {
				$timeZone = 'UTC';
				$sysTimeZone = @date_default_timezone_get();
				if (!empty($sysTimeZone)) {
					$timeZone = $sysTimeZone;
				}
				$time = new DateTime(null, new DateTimeZone($timeZone));
				fwrite($file, $time->format('Y-m-d H:i:s') . ': ' . $message . "\n");
				fclose($file);
			}
		}
		else {
			StatusMessage('ERROR', 'Unable to write to log file!', $cfg->logDestination);
		}
	}
}

/**
 * Checks if write access to LDAP is allowed.
 *
 * @param String $scope account type (e.g. user)
 * @return boolean true, if allowed
 */
function checkIfWriteAccessIsAllowed($scope = null) {
	if (!isset($_SESSION['config'])) {
		return false;
	}
	if ($_SESSION['config']->getAccessLevel() >= LAMConfig::ACCESS_ALL) {
		$typeSettings = $_SESSION['config']->get_typeSettings();
		if ($scope == null) {
			return true;
		}
		elseif (!isset($typeSettings['readOnly_' . $scope]) || !$typeSettings['readOnly_' . $scope]) {
			// check if write for this type is allowed
			return true;
		}
	}
	return false;
}

/**
 * Checks if passwords may be changed.
 *
 * @return boolean true, if allowed
 */
function checkIfPasswordChangeIsAllowed() {
	if (!isset($_SESSION['config'])) {
		return false;
	}
	if ($_SESSION['config']->getAccessLevel() >= LAMConfig::ACCESS_PASSWORD_CHANGE) {
		return true;
	}
	return false;
}

/**
 * Checks if it is allowed to create new LDAP entries of the given type.
 * This also checks if general write access is enabled.
 *
 * @param String $scope account type (e.g. 'user')
 * @return boolean true, if new entries are allowed
 */
function checkIfNewEntriesAreAllowed($scope) {
	if (!isLAMProVersion()) {
		return true;
	}
	if (!isset($_SESSION['config']) || empty($scope)) {
		return false;
	}
	$typeSettings = $_SESSION['config']->get_typeSettings();
	if (isset($typeSettings['hideNewButton_' . $scope]) && $typeSettings['hideNewButton_' . $scope]) {
		return false;
	}
	return checkIfWriteAccessIsAllowed();
}

/**
 * Checks if it is allowed to delete LDAP entries of the given type.
 *
 * @param String $scope account type (e.g. 'user')
 * @return boolean true, if entries may be deleted
 */
function checkIfDeleteEntriesIsAllowed($scope) {
	if (!isLAMProVersion()) {
		return true;
	}
	if (!isset($_SESSION['config']) || empty($scope)) {
		return false;
	}
	$typeSettings = $_SESSION['config']->get_typeSettings();
	if (isset($typeSettings['hideDeleteButton_' . $scope]) && $typeSettings['hideDeleteButton_' . $scope]) {
		return false;
	}
	return checkIfWriteAccessIsAllowed();
}

/**
 * Checks if the password fulfills the password policies.
 *
 * @param String $password password
 * @param String $userName user name
 * @param array $otherUserAttrs user's first/last name
 * @return mixed true if ok, string with error message if not valid
 */
function checkPasswordStrength($password, $userName, $otherUserAttrs) {
	if ($password == null) {
		$password = "";
	}
	if (isset($_SESSION['cfgMain'])) $cfg = $_SESSION['cfgMain'];
	else $cfg = new LAMCfgMain();
	// check length
	if (strlen($password) < $cfg->passwordMinLength) {
		return sprintf(_('The password is too short. You have to enter at least %s characters.'), $cfg->passwordMinLength);
	}
	// get number of characers per character class
	$lower = 0;
	$upper = 0;
	$numeric = 0;
	$symbols = 0;
	for ($i = 0; $i < strlen($password); $i++) {
		if (preg_match("/[a-z]/", $password[$i])) {
			$lower++;
		}
		if (preg_match("/[A-Z]/", $password[$i])) {
			$upper++;
		}
		if (preg_match("/[0-9]/", $password[$i])) {
			$numeric++;
		}
		if (preg_match("/[^a-z0-9]/i", $password[$i])) {
			$symbols++;
		}
	}
	$rulesMatched = 0;
	$rulesFailed = 0;
	// check lower case
	if (($cfg->checkedRulesCount == -1) && ($lower < $cfg->passwordMinLower)) {
		return sprintf(_('The password is too weak. You have to enter at least %s lower case characters.'), $cfg->passwordMinLower);
	}
	if ($lower < $cfg->passwordMinLower) {
		$rulesFailed++;
	}
	else {
		$rulesMatched++;
	}
	// check upper case
	if (($cfg->checkedRulesCount == -1) && ($upper < $cfg->passwordMinUpper)) {
		return sprintf(_('The password is too weak. You have to enter at least %s upper case characters.'), $cfg->passwordMinUpper);
	}
	if ($upper < $cfg->passwordMinUpper) {
		$rulesFailed++;
	}
	else {
		$rulesMatched++;
	}
	// check numeric
	if (($cfg->checkedRulesCount == -1) && ($numeric < $cfg->passwordMinNumeric)) {
		return sprintf(_('The password is too weak. You have to enter at least %s numeric characters.'), $cfg->passwordMinNumeric);
	}
	if ($numeric < $cfg->passwordMinNumeric) {
		$rulesFailed++;
	}
	else {
		$rulesMatched++;
	}
	// check symbols
	if (($cfg->checkedRulesCount == -1) && ($symbols < $cfg->passwordMinSymbol)) {
		return sprintf(_('The password is too weak. You have to enter at least %s symbolic characters.'), $cfg->passwordMinSymbol);
	}
	if ($symbols < $cfg->passwordMinSymbol) {
		$rulesFailed++;
	}
	else {
		$rulesMatched++;
	}
	// check classes
	$classes = 0;
	if ($lower > 0) {
		$classes++;
	}
	if ($upper > 0) {
		$classes++;
	}
	if ($numeric > 0) {
		$classes++;
	}
	if ($symbols > 0) {
		$classes++;
	}
	if (($cfg->checkedRulesCount == -1) && ($classes < $cfg->passwordMinClasses)) {
		return sprintf(_('The password is too weak. You have to enter at least %s different character classes (upper/lower case, numbers and symbols).'), $cfg->passwordMinClasses);
	}
	if ($classes < $cfg->passwordMinClasses) {
		$rulesFailed++;
	}
	else {
		$rulesMatched++;
	}
	// check rules count
	if (($cfg->checkedRulesCount != -1) && ($rulesMatched < $cfg->checkedRulesCount)) {
		return sprintf(_('The password is too weak. It needs to match at least %s password complexity rules.'), $cfg->checkedRulesCount);
	}
	// check user name
	if (($cfg->passwordMustNotContainUser == 'true') && !empty($userName)) {
		$pwdLow = strtolower($password);
		$userLow = strtolower($userName);
		if (strpos($pwdLow, $userLow) !== false) {
			return _('The password is too weak. You may not use the user name as part of the password.');
		}
	}
	// check part of user name and additional attributes
	if (($cfg->passwordMustNotContain3Chars == 'true') && (!empty($userName) || !empty($otherUserAttrs))) {
		$pwdLow = strtolower($password);
		// check if contains part of user name
		if (!empty($userName) && (strlen($userName) > 2)) {
			$userLow = strtolower($userName);
			for ($i = 0; $i < strlen($userLow) - 3; $i++) {
				$part = substr($userLow, 0, 3);
				if (strpos($pwdLow, $part) !== false) {
					return _('The password is too weak. You may not use parts of the user name for the password.');
				}
			}
		}
		// check other attributes
		foreach ($otherUserAttrs as $other) {
			$low = strtolower($other);
			for ($i = 0; $i < strlen($low) - 3; $i++) {
				$part = substr($low, 0, 3);
				if (strpos($pwdLow, $part) !== false) {
					return _('The password is too weak. You may not use parts of user attributes for the password.');
				}
			}
		}
	}
	return true;
}

/**
 * Checks if the given tool is active.
 * Otherwise, an error message is logged and the execution is stopped (die()).
 *
 * @param String $tool tool class name (e.g. toolFileUpload)
 */
function checkIfToolIsActive($tool) {
	$toolSettings = $_SESSION['config']->getToolSettings();
		// check if hidden by config
	if (isset($toolSettings['tool_hide_' . $tool]) && ($toolSettings['tool_hide_' . $tool] == 'true')) {
		logNewMessage(LOG_ERR, 'Unauthorized access to tool ' . $tool . ' denied.');
		die();
	}
}

/**
 * Returns if the user is logged in.
 *
 * @return boolean is logged in
 */
function isLoggedIn() {
	return (isset($_SESSION['loggedIn']) && ($_SESSION['loggedIn'] === true));
}

/**
 * Returns the client IP and comma separated proxy IPs if any (HTTP_X_FORWARDED_FOR, HTTP_X_REAL_IP).
 *
 * @return String client IP (e.g. 10.10.10.10,11.11.11.11)
 */
function getClientIPForLogging() {
	$ip = empty($_SERVER['REMOTE_ADDR']) ? '' : $_SERVER['REMOTE_ADDR'];
	if (!empty($_SERVER['HTTP_X_FORWARDED_FOR']) && (strlen($_SERVER['HTTP_X_FORWARDED_FOR']) < 100)) {
		$ip .= ',' . $_SERVER['HTTP_X_FORWARDED_FOR'];
	}
	if (!empty($_SERVER['HTTP_X_REAL_IP']) && (strlen($_SERVER['HTTP_X_REAL_IP']) < 100)) {
		$ip .= ',' . $_SERVER['HTTP_X_REAL_IP'];
	}
	return $ip;
}

/**
 * Adds a security token to the session to prevent CSRF attacks.
 */
function addSecurityTokenToSession() {
	$_SESSION[getSecurityTokenName()] = getRandomNumber();
}

/**
 * Checks if the security token from SESSION matches POST data.
 *
 * @param $post use POST, set to false for GET (default: true)
 */
function validateSecurityToken($post = true) {
	$vars = $post ? $_POST : $_GET;
	if (empty($vars)) {
		return;
	}
	if (empty($vars[getSecurityTokenName()]) || ($vars[getSecurityTokenName()] != $_SESSION[getSecurityTokenName()])) {
		$method = $post ? 'POST' : 'GET';
		logNewMessage(LOG_ERR, 'Security token does not match ' . $method . ' data.');
		die();
	}
}

/**
 * Adds a hidden input field to the given meta HTML table.
 * Should be used to add token at the end of table.
 *
 * @param htmlTable|htmlGroup $container table
 */
function addSecurityTokenToMetaHTML(&$container) {
	$token = new htmlHiddenInput(getSecurityTokenName(), $_SESSION[getSecurityTokenName()]);
	$container->addElement($token, true);
}

/**
 * Returns the name of the security token parameter.
 *
 * @return String name
 */
function getSecurityTokenName() {
	return 'sec_token';
}

/**
 * Returns the value of the security token parameter.
 *
 * @return String value
 */
function getSecurityTokenValue() {
	return $_SESSION[getSecurityTokenName()];
}

/**
 * Sets the X-Frame-Options and Content-Security-Policy header to prevent clickjacking.
 */
function setLAMHeaders() {
	if (!headers_sent()) {
		header('X-Frame-Options: sameorigin');
		header('Content-Security-Policy: frame-ancestors \'self\'');
	}
}

/**
* Encrypts a string
*
* @param string $data string to encrypt
* @param string $prefix prefix for cookie names
* @return object encrypted string
*/
function lamEncrypt($data, $prefix='') {
	// use OpenSSL if available
	if (function_exists('openssl_random_pseudo_bytes')) {
		// OpenSSL may have been enabled in a running session
		if (!isset($_COOKIE[$prefix . "IV"]) || ($_COOKIE[$prefix . "IV"] == '')) return $data;
		if ($_COOKIE[$prefix . "IV"] == "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx") {
			return $data;
		}
		// read key and iv from cookie
		$iv = base64_decode($_COOKIE[$prefix . "IV"]);
		$key = base64_decode($_COOKIE[$prefix . "Key"]);
		// encrypt string
		return openssl_encrypt(base64_encode($data), lamEncryptionAlgo(), $key, 0, $iv);
	}
	// otherwise do not encrypt
	else {
		return $data;
	}
}

/**
* Decrypts a string
*
* @param object $data string to decrypt
* @param string $prefix prefix for cookie names
* @return string decrypted string
*/
function lamDecrypt($data, $prefix='') {
	// use OpenSSL if available
	if (function_exists('openssl_random_pseudo_bytes')) {
		// OpenSSL may have been enabled in a running session
		if (!isset($_COOKIE[$prefix . "IV"]) || ($_COOKIE[$prefix . "IV"] == '')) return $data;
		if ($_COOKIE[$prefix . "IV"] == "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx") {
			return $data;
		}
		// read key and iv from cookie
		$iv = base64_decode($_COOKIE[$prefix . "IV"]);
		$key = base64_decode($_COOKIE[$prefix . "Key"]);
		// decrypt string
		$ret = openssl_decrypt($data, lamEncryptionAlgo(), $key, 0, $iv);
		$ret = base64_decode(str_replace(chr(00), "", $ret));
		return $ret;
	}
	// otherwise do not decrypt
	else {
		return $data;
	}
}

/**
 * Returns the encryption algorithm to use.
 *
 * @return string algorithm name
 */
function lamEncryptionAlgo() {
	$possibleAlgos = openssl_get_cipher_methods();
	if (in_array('AES-256-CTR', $possibleAlgos)) {
		return 'AES-256-CTR';
	}
	elseif (in_array('AES-256-CBC', $possibleAlgos)) {
		return 'AES-256-CBC';
	}
	return 'AES256';
}

?>