<?php
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
 * This file includes functions to perform several security checks on each page load.
 *
 * @package lib
 * @author Roland Gruber
 */

/** configuration options */
include_once(__DIR__ . '/config.inc');
/** ldap connection */
include_once(__DIR__ . '/ldap.inc');
/** common functions */
include_once(__DIR__ . '/account.inc');

// check client IP address
checkClientIP();

setLAMHeaders();

/**
 * Starts a session and sets the cookie options.
 */
function lam_start_session(): void {
	$cookieOptions = lamDefaultCookieOptions();
	$cookieOptions['lifetime'] = 0;
	session_set_cookie_params($cookieOptions);
	session_start();
}

/**
 * Returns the default cookie options.
 *
 * @return array<string, string|bool> cookie options
 */
function lamDefaultCookieOptions(): array {
	$secureFlag = false;
	if (isset($_SERVER['HTTPS']) && strtolower($_SERVER['HTTPS']) === 'on') {
		$secureFlag = true;
	}
	return [
		'path' => '/',
		'domain' => '',
		'secure' => $secureFlag,
		'httponly' => true,
		'samesite' => 'Lax'
	];
}

/**
 * Starts a session and checks the environment.
 * The script is stopped if one of the checks fail (timeout redirection may be overridden).
 *
 * @param boolean $redirectToLogin redirect user to login page (default: true)
 * @param boolean $initSecureData init verification data like session ID and client IP (default: false)
 * @return boolean true if all ok, false if session expired
 */
function startSecureSession($redirectToLogin = true, $initSecureData = false) {
	// start session
	if (isset($_SESSION)) {
		unset($_SESSION);
	}
	if (isFileBasedSession()) {
		$sessionDir = __DIR__ . "/../sess";
		session_save_path($sessionDir);
		// enable garbage collection (fix for Debian based systems)
		if (@ini_get("session.gc_probability") == 0) {
			@ini_set("session.gc_probability", 1);
		}
	}
	lam_start_session();
	// init secure data if needed
	if ($initSecureData && !isset($_SESSION["sec_session_id"])) {
		$_SESSION["sec_session_id"] = session_id();
		$_SESSION["sec_client_ip"] = $_SERVER['REMOTE_ADDR'];
		$_SESSION['sec_sessionTime'] = time();
		$_SESSION['cfgMain'] = new LAMCfgMain();
	}
	// set error reporting
	if (empty($_SESSION['cfgMain']) || ($_SESSION['cfgMain']->errorReporting == LAMCfgMain::ERROR_REPORTING_DEFAULT)) {
		ini_set('error_reporting', (E_ALL & ~E_NOTICE));
	}
	elseif ($_SESSION['cfgMain']->errorReporting == LAMCfgMain::ERROR_REPORTING_ALL) {
		ini_set('error_reporting', E_ALL);
		ini_set('display_errors', 'On');
	}
	// check session id
	if (!isset($_SESSION["sec_session_id"]) || ($_SESSION["sec_session_id"] != session_id())) {
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
		if ($redirectToLogin) {
			logoffAndBackToLoginPage();
		}
		else {
			die();
		}
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
 * Returns if the session uses files storage.
 *
 * @return bool file based session
 */
function isFileBasedSession(): bool {
	return ((session_module_name() !== false) && (strtolower(session_module_name()) === 'files'));
}

/**
 * Checks if the client's IP address is on the list of allowed IPs.
 * The script is stopped if the host is not valid.
 */
function checkClientIP(): void {
	$cfg = $_SESSION['cfgMain'] ?? new LAMCfgMain();
	$allowedHosts = $cfg->allowedHosts;
	$url = getCallingURL();
	if (($url !== null) &&
		((str_contains($url, '/selfService/selfService'))
			|| ((str_contains($url, '/misc/ajax.php?')) && str_contains($url, 'selfservice=1')))) {
		// self-service pages have separate IP list
		$allowedHosts = $cfg->allowedHostsSelfService;
	}
	// skip test if no hosts are defined
	if (empty($allowedHosts) || empty($_SERVER['REMOTE_ADDR'])) {
		return;
	}
	$allowedHosts = explode(",", $allowedHosts);
	$grantAccess = false;
	for ($i = 0; $i < count($allowedHosts); $i++) {
		$host = $allowedHosts[$i];
		$ipRegex = '/^[0-9a-z\\.:\\*]+$/i';
		if (!preg_match($ipRegex, $host)) {
			continue;
		}
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
function logoffAndBackToLoginPage(): void {
	// log message
	if (isset($_SESSION['ldap'])) {
		$ldapUser = $_SESSION['ldap']->getUserName();
		logNewMessage(LOG_WARNING, 'Session of user ' . $ldapUser . ' expired.');
		// close LDAP connection
		@$_SESSION["ldap"]->destroy();
	}
	elseif (isset($_SESSION['selfService_clientDN']) || (str_contains($_SERVER['REQUEST_URI'], '/selfService/'))) {
		logNewMessage(LOG_WARNING, 'Self-service session of DN ' . lamDecrypt($_SESSION['selfService_clientDN'], 'SelfService') . ' expired.');
	}
	// delete key and iv in cookie
	$cookieOptions = lamDefaultCookieOptions();
	$cookieOptions['expires'] = 0;
	setcookie("Key", "", $cookieOptions);
	setcookie("IV", "", $cookieOptions);
	// link back to login page
	$paths = ['./', '../', '../../', '../../../', '../../../../'];
	$page = 'login.php';
	$pageSuffix = '?expired=yes';
	if (isset($_SESSION['selfService_clientDN']) || (str_contains($_SERVER['REQUEST_URI'], '/selfService/'))) {
		$scope = $_GET['scope'];
		$name = $_GET['name'];
		if (!preg_match('/^[0-9a-zA-Z _-]+$/', $scope) || !preg_match('/^[0-9a-zA-Z _-]+$/', $name)) {
			logNewMessage(LOG_ERR, 'GET parameters invalid: ' . $name . ' ' . $scope);
			die();
		}
		$page = 'selfServiceLogin.php';
		$pageSuffix = '?expired=yes&scope=' . $scope . '&name=' . $name;
	}
	elseif (isset($_SESSION['whitePages_clientDN']) || (str_contains($_SERVER['REQUEST_URI'], '/whitePages/'))) {
		$name = $_GET['name'];
		if (!preg_match('/^[0-9a-zA-Z _-]+$/', $name)) {
			logNewMessage(LOG_ERR, 'GET parameters invalid: ' . $name);
			die();
		}
		$page = 'whitePagesLogin.php';
		$pageSuffix = '?expired=yes&name=' . $name;
	}
	for ($i = 0; $i < count($paths); $i++) {
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
	// print the link if refresh does not work
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
 * Returns if debug messages are to be logged.
 *
 * @return bool debug enabled
 */
function isDebugLoggingEnabled(): bool {
	$cfg = $_SESSION['cfgMain'] ?? new LAMCfgMain();
	return $cfg->logLevel >= LOG_DEBUG;
}

/**
 * Puts a new message in the log file.
 *
 * @param int $level log level (LOG_DEBUG, LOG_NOTICE, LOG_WARNING, LOG_ERR)
 * @param string $message log message
 */
function logNewMessage($level, $message): void {
	$possibleLevels = [
		LOG_DEBUG => 'DEBUG',
		LOG_NOTICE => 'NOTICE',
		LOG_WARNING => 'WARNING',
		LOG_ERR => 'ERROR'];
	if (!in_array($level, array_keys($possibleLevels))) {
		StatusMessage('ERROR', 'Invalid log level!', htmlspecialchars((string) $level));
		return;
	}
	if (isset($_SESSION['cfgMain'])) {
		$cfg = $_SESSION['cfgMain'];
	}
	else {
		$cfg = new LAMCfgMain();
		$_SESSION['cfgMain'] = $cfg;
	}
	// check if logging is disabled
	if (($cfg->logDestination == 'NONE')
		// check if log level is high enough
		|| ($cfg->logLevel < $level)) {
		return;
	}
	// ok to log, build log message
	$prefix = "LDAP Account Manager (" . session_id() . ' - ' . getClientIPForLogging() . ' - ' . getLamLdapUser() . ") - " . $possibleLevels[$level] . ": ";
	$message = $prefix . $message;
	// Syslog logging
	if ($cfg->logDestination == 'SYSLOG') {
		syslog($level, str_replace(["\n", "\r"], ['', ''], $message));
	}
	// remote logging
	elseif (str_starts_with($cfg->logDestination, 'REMOTE')) {
		lamLogRemoteMessage($level, $message, $cfg);
	}
	// log to file
	elseif (LAMCfgMain::isValidLogFilename($cfg->logDestination)) {
		@touch($cfg->logDestination);
		if (is_writable($cfg->logDestination)) {
			$file = fopen($cfg->logDestination, 'a');
			if ($file) {
				$timeZone = 'UTC';
				$sysTimeZone = @date_default_timezone_get();
				if (!empty($sysTimeZone)) {
					$timeZone = $sysTimeZone;
				}
				$time = new DateTime('now', new DateTimeZone($timeZone));
				fwrite($file, $time->format('Y-m-d H:i:s') . ': ' . $message . "\n");
				fclose($file);
			}
		}
	}
}

/**
 * Logs a message to a remote logging service.
 *
 * @param int $level log level
 * @param string $message log message
 * @param LAMCfgMain $cfgMain main configuration
 */
function lamLogRemoteMessage($level, $message, $cfgMain): void {
	include_once __DIR__ . '/3rdParty/composer/autoload.php';
	$remoteParts = explode(':', $cfgMain->logDestination);
	$server = $remoteParts[1];
	$port = intval($remoteParts[2]);
	$output = "%channel%.%level_name%: %message%";
	$formatter = new Monolog\Formatter\LineFormatter($output);
	$logger = new Monolog\Logger('lam');
	$syslogHandler = new Monolog\Handler\SyslogUdpHandler($server, $port);
	$syslogHandler->setFormatter($formatter);
	$logger->pushHandler($syslogHandler);
	switch ($level) {
		case LOG_DEBUG:
			$logger->debug($message);
			break;
		case LOG_NOTICE:
			$logger->notice($message);
			break;
		case LOG_WARNING:
			$logger->warning($message);
			break;
		case LOG_ERR:
			$logger->error($message);
			break;
	}
}

function logAuditMessage($message): void {
	if (isset($_SESSION['cfgMain'])) {
		$cfg = $_SESSION['cfgMain'];
	}
	else {
		$cfg = new LAMCfgMain();
		$_SESSION['cfgMain'] = $cfg;
	}
	// check if logging is disabled
	if ($cfg->auditLogType === LOG_FILE_TARGET::NONE->name) {
		return;
	}
	$message = "LDAP Account Manager AuditLog (" . session_id() . ' - ' . getClientIPForLogging() . ' - ' . getLamLdapUser() . ") - " . $message;
	if ($cfg->auditLogType === LOG_FILE_TARGET::SYSLOG->name) {
		syslog(LOG_NOTICE, str_replace(["\n", "\r"], ['', ''], $message));
	}
	elseif (($cfg->auditLogType === LOG_FILE_TARGET::FILE->name) && LAMCfgMain::isValidLogFilename($cfg->auditLogTarget)) {
		@touch($cfg->auditLogTarget);
		if (is_writable($cfg->auditLogTarget)) {
			$file = fopen($cfg->auditLogTarget, 'a');
			if ($file) {
				$timeZone = 'UTC';
				$sysTimeZone = @date_default_timezone_get();
				if (!empty($sysTimeZone)) {
					$timeZone = $sysTimeZone;
				}
				$time = new DateTime('now', new DateTimeZone($timeZone));
				fwrite($file, $time->format('Y-m-d H:i:s') . ' ' . $message . "\n");
				fclose($file);
			}
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
		if (($scope == null)
			// check if write for this type is allowed
			|| !isset($typeSettings['readOnly_' . $scope])
			|| !$typeSettings['readOnly_' . $scope]) {
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
	return $_SESSION['config']->getAccessLevel() >= LAMConfig::ACCESS_PASSWORD_CHANGE;
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
 * @param string $password password
 * @param string|string[]|null $userNames user name(s)
 * @param string[] $otherUserAttrs user's first/last name
 * @return PasswordStrengthResult result
 */
function checkPasswordStrength($password, $userNames, $otherUserAttrs): PasswordStrengthResult {
	if (($userNames !== null) && !is_array($userNames)) {
		$userNames = [$userNames];
	}
	if ($password == null) {
		$password = "";
	}
	$cfg = $_SESSION['cfgMain'] ?? new LAMCfgMain();
	$result = new PasswordStrengthResult();
	$result->setCharacterClassesRulesCount($cfg->checkedRulesCount);
	// check length
	$minLength = $cfg->passwordMinLength;
	if (isset($_SESSION['config']) && is_numeric($_SESSION['config']->getPwdPolicyMinLength())) {
		$minLength = intval($_SESSION['config']->getPwdPolicyMinLength());
	}
	if (strlen($password) < $minLength) {
		$result->setLengthOk(false, $minLength);
	}
	else {
		$result->setLengthOk(true, $minLength);
	}
	// get number of characters per character class
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
		if (preg_match("/\\d/", $password[$i])) {
			$numeric++;
		}
		if (preg_match("/[^a-z0-9]/i", $password[$i])) {
			$symbols++;
		}
	}
	$rulesMatched = 0;
	// check lower-case
	$minLower = $cfg->passwordMinLower;
	if (isset($_SESSION['config']) && is_numeric($_SESSION['config']->getpwdPolicyMinLowercase())) {
		$minLower = intval($_SESSION['config']->getpwdPolicyMinLowercase());
	}
	if ($lower < $minLower) {
		$result->setLowerCaseCountOk(false, $minLower);
		if ($cfg->checkedRulesCount == -1) {
			$result->setCharacterClassesRulesCountOk(false);
		}
	}
	else {
		$result->setLowerCaseCountOk(true, $minLower);
		$rulesMatched++;
	}
	// check upper-case
	$minUpper = $cfg->passwordMinUpper;
	if (isset($_SESSION['config']) && is_numeric($_SESSION['config']->getPwdPolicyMinUppercase())) {
		$minUpper = $_SESSION['config']->getPwdPolicyMinUppercase();
	}
	if ($upper < $minUpper) {
		$result->setUpperCaseCountOk(false, $minUpper);
		if ($cfg->checkedRulesCount == -1) {
			$result->setCharacterClassesRulesCountOk(false);
		}
	}
	else {
		$result->setUpperCaseCountOk(true, $minUpper);
		$rulesMatched++;
	}
	// check numeric
	$minNumeric = $cfg->passwordMinNumeric;
	if (isset($_SESSION['config']) && is_numeric($_SESSION['config']->getPwdPolicyMinNumeric())) {
		$minNumeric = $_SESSION['config']->getPwdPolicyMinNumeric();
	}
	if ($numeric < $minNumeric) {
		$result->setDigitCountOk(false, $minNumeric);
		if ($cfg->checkedRulesCount == -1) {
			$result->setCharacterClassesRulesCountOk(false);
		}
	}
	else {
		$result->setDigitCountOk(true, $minNumeric);
		$rulesMatched++;
	}
	// check symbols
	$minSymbols = $cfg->passwordMinSymbol;
	if (isset($_SESSION['config']) && is_numeric($_SESSION['config']->getPwdPolicyMinSymbolic())) {
		$minSymbols = $_SESSION['config']->getPwdPolicyMinSymbolic();
	}
	if ($symbols < $minSymbols) {
		$result->setSymbolCountOk(false, $minSymbols);
		if ($cfg->checkedRulesCount == -1) {
			$result->setCharacterClassesRulesCountOk(false);
		}
	}
	else {
		$result->setSymbolCountOk(true, $minSymbols);
		$rulesMatched++;
	}
	// check classes
	$classes = getNumberOfCharacterClasses($password);
	if ($classes < $cfg->passwordMinClasses) {
		$result->setCharacterClassesCountOk(false, $cfg->passwordMinClasses);
		if ($cfg->checkedRulesCount == -1) {
			$result->setCharacterClassesRulesCountOk(false);
		}
	}
	else {
		$result->setCharacterClassesCountOk(true, $cfg->passwordMinClasses);
		$rulesMatched++;
	}
	// check rules count
	if (($cfg->checkedRulesCount != -1) && ($rulesMatched < $cfg->checkedRulesCount)) {
		$result->setCharacterClassesRulesCountOk(false);
	}
	$pwdLow = strtolower($password);
	// check user name
	if (($cfg->passwordMustNotContainUser == 'true') && !empty($userNames)) {
		foreach ($userNames as $userName) {
			$userLow = strtolower($userName);
			if (str_contains($pwdLow, $userLow)) {
				$result->setPasswordContainsUsername(true);
				break;
			}
		}
	}
	// check part of user name and additional attributes
	if (($cfg->passwordMustNotContain3Chars == 'true') && !empty($userNames)) {
		// check if contains part of user name
		foreach ($userNames as $userName) {
			if (strlen($userName) > 2) {
				$userLow = strtolower($userName);
				for ($i = 0; $i < strlen($userLow) - 3; $i++) {
					$part = substr($userLow, 0, 3);
					if (str_contains($pwdLow, $part)) {
						$result->setPasswordContainsPartialUsername(true);
						break;
					}
				}
			}
		}
	}
	if (($cfg->passwordMustNotContain3Chars == 'true') && !empty($otherUserAttrs)) {
		// check other attributes
		foreach ($otherUserAttrs as $other) {
			$low = strtolower($other);
			for ($i = 0; $i < strlen($low) - 3; $i++) {
				$part = substr($low, 0, 3);
				if (str_contains($pwdLow, $part)) {
					$result->setPasswordContainsPartialAttributes(true);
					break;
				}
			}
		}
	}
	// check external password service
	if (!checkPwdWithExternalPasswordService($cfg, $password)) {
		$result->setExternalPasswordCheckFailed(true);
	}
	return $result;
}

/**
 * Result of the password strength check.
 */
class PasswordStrengthResult {

	private bool $lengthOk = false;
	private int $minLength = 0;
	private bool $lowerCaseCountOk = false;
	private int $lowerCaseMinimumCount = 0;
	private bool $upperCaseCountOk = false;
	private int $upperCaseMinimumCount = 0;
	private bool $digitCountOk = false;
	private int $digitMinimumCount = 0;
	private bool $symbolCountOk = false;
	private int $symbolMinimumCount = 0;
	private bool $characterClassesCountOk = false;
	private int $characterClassesMinimumCount = 0;
	private bool $characterClassesRulesCountOk = true;
	private int $characterClassesRulesCount = -1;
	private bool $passwordContainsUsername = false;
	private bool $passwordContainsPartialUsername = false;
	private bool $passwordContainsPartialAttributes = false;
	private bool $externalPasswordCheckFailed = false;

	/**
	 * Returns if the password is valid.
	 *
	 * @return bool is valid
	 */
	public function isValid(): bool {
		if ($this->characterClassesRulesCount === -1) {
			$characterRulesCheckOk = $this->lowerCaseCountOk
				&& $this->upperCaseCountOk
				&& $this->digitCountOk
				&& $this->symbolCountOk
				&& $this->characterClassesCountOk;
		}
		else {
			$characterRulesCheckOk = $this->characterClassesRulesCountOk;
		}
		return $this->lengthOk
			&& $characterRulesCheckOk
			&& !$this->passwordContainsUsername
			&& !$this->passwordContainsPartialUsername
			&& !$this->passwordContainsPartialAttributes
			&& !$this->externalPasswordCheckFailed;
	}

	/**
	 * Returns details about failed rules.
	 *
	 * @return string[] issues
	 */
	public function getIssues(): array {
		$details = [];
		if (!$this->lengthOk) {
			$details[] = [sprintf(_('The password must contain at least %s characters.'), $this->minLength)];
		}
		if (($this->characterClassesRulesCount === -1) || !$this->characterClassesRulesCountOk) {
			if (!$this->lowerCaseCountOk) {
				$details[] = [sprintf(_('The password must contain at least %s lower-case characters.'), $this->lowerCaseMinimumCount)];
			}
			if (!$this->upperCaseCountOk) {
				$details[] = [sprintf(_('The password must contain at least %s upper-case characters.'), $this->upperCaseMinimumCount)];
			}
			if (!$this->digitCountOk) {
				$details[] = [sprintf(_('The password must contain at least %s numeric characters.'), $this->digitMinimumCount)];
			}
			if (!$this->symbolCountOk) {
				$details[] = [sprintf(_('The password must contain at least %s symbolic characters.'), $this->symbolMinimumCount)];
			}
			if (!$this->characterClassesCountOk) {
				$details[] = [sprintf(_('The password must contain at least %s different character classes.'), $this->characterClassesMinimumCount)];
			}
		}
		if ($this->passwordContainsUsername) {
			$details[] = [_('The password must not contain the username.')];
		}
		elseif ($this->passwordContainsPartialUsername) {
			$details[] = [_('The password must not contain parts of the username.')];
		}
		if ($this->passwordContainsPartialAttributes) {
			$details[] = [_('The password must not contain parts of the account\'s attributes.')];
		}
		if ($this->externalPasswordCheckFailed) {
			$details[] = [_('The password is known to be insecure.')];
		}
		return $details;
	}

	/**
	 * Returns the error message or null if valid.
	 *
	 * @return string|null message
	 */
	public function getTextMessage(): ?string {
		if (!$this->lengthOk) {
			return sprintf(_('The password is too short. You have to enter at least %s characters.'), $this->minLength);
		}
		if ($this->characterClassesRulesCount === -1) {
			if (!$this->lowerCaseCountOk) {
				return sprintf(_('The password is too weak. You have to enter at least %s lower-case characters.'), $this->lowerCaseMinimumCount);
			}
			if (!$this->upperCaseCountOk) {
				return sprintf(_('The password is too weak. You have to enter at least %s upper-case characters.'), $this->upperCaseMinimumCount);
			}
			if (!$this->digitCountOk) {
				return sprintf(_('The password is too weak. You have to enter at least %s numeric characters.'), $this->digitMinimumCount);
			}
			if (!$this->symbolCountOk) {
				return sprintf(_('The password is too weak. You have to enter at least %s symbolic characters.'), $this->symbolMinimumCount);
			}
			if (!$this->characterClassesCountOk) {
				return sprintf(_('The password is too weak. You have to enter at least %s different character classes (upper/lower-case, numbers and symbols).'), $this->characterClassesMinimumCount);
			}
		}
		if (!$this->characterClassesRulesCountOk) {
			return sprintf(_('The password is too weak. It needs to match at least %s password complexity rules.'), $this->characterClassesRulesCount);
		}
		if ($this->passwordContainsUsername) {
			return _('The password is too weak. You may not use the user name as part of the password.');
		}
		if ($this->passwordContainsPartialUsername) {
			return _('The password is too weak. You may not use parts of the user name for the password.');
		}
		if ($this->passwordContainsPartialAttributes) {
			return _('The password is too weak. You may not use parts of user attributes for the password.');
		}
		if ($this->externalPasswordCheckFailed) {
			return _('Your selected password is known to be insecure.');
		}
		return null;
	}

	/**
	 * Sets if the length check is ok.
	 *
	 * @param bool $isOk ok
	 * @param int $minLength minimum length
	 */
	public function setLengthOk(bool $isOk, int $minLength): void {
		$this->lengthOk = $isOk;
		$this->minLength = $minLength;
	}

	/**
	 * Sets the number of character class checks to fulfill.
	 *
	 * @param int $characterClassesRulesCount count
	 */
	public function setCharacterClassesRulesCount(int $characterClassesRulesCount): void {
		$this->characterClassesRulesCount = $characterClassesRulesCount;
	}

	/**
	 * Sets if the number of character class checks is fulfilled.
	 *
	 * @param bool $characterClassesRulesCountOk ok
	 */
	public function setCharacterClassesRulesCountOk(bool $characterClassesRulesCountOk): void {
		$this->characterClassesRulesCountOk = $characterClassesRulesCountOk;
	}

	/**
	 * The number of character classes is ok.
	 *
	 * @param bool $characterClassesCountOk ok
	 * @param int $characterClassesMinimumCount count
	 */
	public function setCharacterClassesCountOk(bool $characterClassesCountOk, int $characterClassesMinimumCount): void {
		$this->characterClassesCountOk = $characterClassesCountOk;
		$this->characterClassesMinimumCount = $characterClassesMinimumCount;
	}

	/**
	 * Sets if the number of digits is ok.
	 *
	 * @param bool $digitCountOk ok
	 * @param int $digitMinimumCount count
	 */
	public function setDigitCountOk(bool $digitCountOk, int $digitMinimumCount): void {
		$this->digitCountOk = $digitCountOk;
		$this->digitMinimumCount = $digitMinimumCount;
	}

	/**
	 * Sets if the number of lower-case characters is ok.
	 *
	 * @param bool $lowerCaseCountOk ok
	 * @param int $lowerCaseMinimumCount count
	 */
	public function setLowerCaseCountOk(bool $lowerCaseCountOk, int $lowerCaseMinimumCount): void {
		$this->lowerCaseCountOk = $lowerCaseCountOk;
		$this->lowerCaseMinimumCount = $lowerCaseMinimumCount;
	}

	/**
	 * Sets if the number of symbolic characters is ok.
	 *
	 * @param bool $symbolCountOk ok
	 * @param int $symbolMinimumCount count
	 */
	public function setSymbolCountOk(bool $symbolCountOk, int $symbolMinimumCount): void {
		$this->symbolCountOk = $symbolCountOk;
		$this->symbolMinimumCount = $symbolMinimumCount;
	}

	/**
	 * Sets if the number of upper-case characters is ok.
	 *
	 * @param bool $upperCaseCountOk ok
	 * @param int $upperCaseMinimumCount count
	 */
	public function setUpperCaseCountOk(bool $upperCaseCountOk, int $upperCaseMinimumCount): void {
		$this->upperCaseCountOk = $upperCaseCountOk;
		$this->upperCaseMinimumCount = $upperCaseMinimumCount;
	}

	/**
	 * Sets if the password contains the username.
	 *
	 * @param bool $passwordContainsUsername invalid value found
	 */
	public function setPasswordContainsUsername(bool $passwordContainsUsername): void {
		$this->passwordContainsUsername = $passwordContainsUsername;
	}

	/**
	 * Sets if the password contains a part of the username.
	 *
	 * @param bool $passwordContainsPartialUsername invalid value found
	 */
	public function setPasswordContainsPartialUsername(bool $passwordContainsPartialUsername): void {
		$this->passwordContainsPartialUsername = $passwordContainsPartialUsername;
	}

	/**
	 * Sets if the password contains a part of the user attributes.
	 *
	 * @param bool $passwordContainsPartialAttributes invalid value found
	 */
	public function setPasswordContainsPartialAttributes(bool $passwordContainsPartialAttributes): void {
		$this->passwordContainsPartialAttributes = $passwordContainsPartialAttributes;
	}

	/**
	 * Sets if the external password check failed.
	 *
	 * @param bool $externalPasswordCheckFailed failed
	 */
	public function setExternalPasswordCheckFailed(bool $externalPasswordCheckFailed): void {
		$this->externalPasswordCheckFailed = $externalPasswordCheckFailed;
	}

}

/**
 * Checks the password against the external password service.
 *
 * @param LAMCfgMain $cfg main configuration
 * @param string $password password
 * @return boolean password accepted as secure
 */
function checkPwdWithExternalPasswordService($cfg, $password) {
	if (!function_exists('curl_init') || empty($cfg->externalPwdCheckUrl)) {
		return true;
	}
	$sha1 = sha1($password);
	$sha1Prefix = substr($sha1, 0, 5);
	$sha1Suffix = substr($sha1, 5);
	$curl = curl_init();
	curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
	$url = $cfg->externalPwdCheckUrl;
	$url = str_replace('{SHA1PREFIX}', $sha1Prefix, $url);
	curl_setopt($curl, CURLOPT_URL, $url);
	$results = curl_exec($curl);
	$code = curl_errno($curl);
	if ($code !== 0) {
		logNewMessage(LOG_ERR, 'Error calling the external password service at ' . $url
			. '. ' . curl_error($curl));
		return true;
	}
	if (empty($results)) {
		return true;
	}
	$results = explode("\n", $results);
	foreach ($results as $result) {
		if (stripos($result, $sha1Suffix . ':') !== false) {
			return false;
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
function checkIfToolIsActive($tool): void {
	// check if hidden by config
	if (!$_SESSION['config']->isToolActive($tool)) {
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
 * Returns the login dn of the current user.
 *
 * @return string user DN
 */
function getLamLdapUser() {
	if (isset($_SESSION['ldap'])) {
		return $_SESSION['ldap']->getUserName();
	}
	elseif (isset($_SESSION['selfService_clientDN'])) {
		return lamDecrypt($_SESSION['selfService_clientDN'], 'SelfService');
	}
	return '';
}

/**
 * Adds a security token to the session to prevent CSRF attacks.
 *
 * @param boolean $overwrite overwrite existing token
 */
function addSecurityTokenToSession($overwrite = true): void {
	if (!empty($_SESSION[getSecurityTokenName()]) && !$overwrite) {
		return;
	}
	$_SESSION[getSecurityTokenName()] = generateRandomPassword(40);
}

/**
 * Checks if the security token from SESSION matches POST data.
 */
function validateSecurityToken(): void {
	if (empty($_POST)) {
		return;
	}
	if (empty($_POST[getSecurityTokenName()]) || ($_POST[getSecurityTokenName()] != $_SESSION[getSecurityTokenName()])) {
		logNewMessage(LOG_ERR, 'Security token does not match POST data.');
		die();
	}
}

/**
 * Adds a hidden input field to the given meta HTML table.
 * Should be used to add token at the end of table.
 *
 * @param htmlTable|htmlGroup|htmlResponsiveRow $container table
 */
function addSecurityTokenToMetaHTML($container): void {
	$token = new htmlHiddenInput(getSecurityTokenName(), $_SESSION[getSecurityTokenName()]);
	if ($container instanceof htmlResponsiveRow) {
		$container->add($token);
		return;
	}
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
function setLAMHeaders(): void {
	if (!headers_sent()) {
		header('X-Frame-Options: sameorigin');
		header('Content-Security-Policy: frame-ancestors \'self\'; form-action \'self\'; base-uri \'none\'; object-src \'none\'; frame-src \'self\' https://www.google.com/recaptcha/ https://hcaptcha.com https://*.hcaptcha.com https://api.friendlycaptcha.com; worker-src \'self\' blob:');
		header('X-Content-Type-Options: nosniff');
		header('X-XSS-Protection: 1; mode=block');
		header("Feature-Policy: ambient-light-sensor 'none'; autoplay 'none'; accelerometer 'none'; camera 'none'; encrypted-media 'none'; fullscreen 'self'; geolocation 'none'; gyroscope 'none'; magnetometer 'none'; microphone 'none'; midi 'none'; payment 'none'; picture-in-picture 'none'; speaker 'none'; sync-xhr 'self'; usb 'none'; vr 'none'");
	}
}

/**
 * Encrypts a string
 *
 * @param string $data string to encrypt
 * @param string $prefix prefix for cookie names
 * @return string encrypted string
 */
function lamEncrypt($data, $prefix = '') {
	if (empty($_COOKIE[$prefix . "IV"])) {
		return $data;
	}
	// read key and iv from cookie
	$iv = base64_decode($_COOKIE[$prefix . "IV"]);
	$key = base64_decode($_COOKIE[$prefix . "Key"]);
	// encrypt string
	return openssl_encrypt(base64_encode($data), lamEncryptionAlgo(), $key, 0, $iv);
}

/**
 * Decrypts a string
 *
 * @param string $data string to decrypt
 * @param string $prefix prefix for cookie names
 * @return string decrypted string
 */
function lamDecrypt($data, $prefix = '') {
	if (empty($_COOKIE[$prefix . "IV"])) {
		return $data;
	}
	// read key and iv from cookie
	$iv = base64_decode($_COOKIE[$prefix . "IV"]);
	$key = base64_decode($_COOKIE[$prefix . "Key"]);
	// decrypt string
	$ret = openssl_decrypt($data, lamEncryptionAlgo(), $key, 0, $iv);
	return base64_decode(str_replace(chr(00), "", $ret));
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

/**
 * Manages temporary files.
 */
class LamTemporaryFilesManager {

	private const SESSION_KEY = 'sec_registered_tmp_files';

	/**
	 * Creates a temporary file and registers it for the current session.
	 *
	 * @param string $extension file extension (including ".", e.g. ".svg")
	 * @param string $prefix file prefix (default: empty)
	 * @return string file name
	 * @throws LAMException unable to create temporary file
	 */
	public function registerTemporaryFile(string $extension, string $prefix = ''): string {
		$fileName = $prefix . time() . generateRandomText() . $extension;
		if (!$this->isValidFileName($fileName)) {
			throw new LAMException(_('Unable to create temporary file.'));
		}
		$path = __DIR__ . '/../tmp/' . $fileName;
		$handle = @fopen($path, "wb");
		if ($handle) {
			@chmod($path, 0600);
			fclose($handle);
			$_SESSION[self::SESSION_KEY][] = $fileName;
			return $fileName;
		}
		throw new LAMException(_('Unable to create temporary file.'));
	}

	/**
	 * Opens a temporary file for reading.
	 *
	 * @param string $fileName file name
	 * @return resource file handle
	 * @throws LAMException unable to open file
	 */
	public function openTemporaryFileForRead(string $fileName) {
		if (!$this->isRegisteredFile($fileName)) {
			throw new LAMException(_('Unable to read file.'));
		}
		$path = $this->getFileSystemPath($fileName);
		$handle = @fopen($path, "r");
		if ($handle) {
			return $handle;
		}
		throw new LAMException(_('Unable to read file.'));
	}

	/**
	 * Opens a temporary file for writing.
	 *
	 * @param string $fileName file name
	 * @return resource file handle
	 * @throws LAMException unable to open file
	 */
	public function openTemporaryFileForWrite(string $fileName) {
		if (!$this->isRegisteredFile($fileName)) {
			throw new LAMException(_('Unable to read file.'));
		}
		$path = $this->getFileSystemPath($fileName);
		$handle = @fopen($path, "wb");
		if ($handle) {
			return $handle;
		}
		throw new LAMException(_('Unable to read file.'));
	}

	/**
	 * Checks if the given file name is valid and registered.
	 *
	 * @param string $fileName file name
	 * @return bool is valid
	 */
	public function isRegisteredFile(string $fileName): bool {
		return $this->isValidFileName($fileName)
			&& !empty($_SESSION[self::SESSION_KEY])
			&& in_array($fileName, $_SESSION[self::SESSION_KEY]);
	}

	/**
	 * Checks if the given file name contains no special characters.
	 *
	 * @param string $fileName file name
	 * @return bool is valid
	 */
	private function isValidFileName(string $fileName): bool {
		return preg_match('/^[a-zA-Z0-9\\._-]+$/', $fileName);
	}

	/**
	 * Returns the download link to a file.
	 *
	 * @param string $fileName file name
	 * @param string $prefix prefix to folder "templates" (e.g. "../../" if called from templates/config/sub, default: "../")
	 * @return string download link
	 * @throws LAMException error reading file
	 */
	public function getDownloadLink(string $fileName, string $prefix = '../'): string {
		if (!$this->isRegisteredFile($fileName)) {
			throw new LAMException(_('Unable to read file.'));
		}
		$selfServicePart = isSelfService() ? '&selfservice=true' : '';
		return $prefix . 'misc/download.php?file=' . $fileName . '&download=true' . $selfServicePart;
	}

	/**
	 * Returns the plain link to a file.
	 *
	 * @param string $fileName file name
	 * @param string $prefix prefix to folder "templates" (e.g. "../../" if called from templates/config/sub, default: "../")
	 * @return string download link
	 * @throws LAMException error reading file
	 */
	public function getResourceLink(string $fileName, string $prefix = '../'): string {
		if (!$this->isRegisteredFile($fileName)) {
			throw new LAMException(_('Unable to read file.'));
		}
		$selfServicePart = isSelfService() ? '&selfservice=true' : '';
		return $prefix . 'misc/download.php?file=' . $fileName . $selfServicePart;
	}

	/**
	 * Returns if the extension is acceptable to be injected as resource on the page.
	 *
	 * @param string $extension extension (e.g. jpg)
	 * @return bool is acceptable
	 */
	public function isAcceptableResourceLinkExtension(string $extension): bool {
		return in_array_ignore_case($extension, ['jpg', 'jpeg', 'gif', 'png', 'pdf']);
	}

	/**
	 * Deletes all registered files of the user.
	 */
	public function deleteAllRegisteredFiles(): void {
		if (empty($_SESSION[self::SESSION_KEY])) {
			return;
		}
		foreach ($_SESSION[self::SESSION_KEY] as $fileName) {
			if (!$this->isValidFileName($fileName)) {
				continue;
			}
			$path = $this->getFileSystemPath($fileName);
			if (file_exists($path)) {
				unlink($path);
			}
		}
		unset($_SESSION[self::SESSION_KEY]);
	}

	/**
	 * Returns the raw path on file system.
	 * Only to be used if download/resource link cannot be used.
	 *
	 * @param string $fileName file name
	 * @return string path on file system
	 * @throws LAMException
	 */
	public function getFileSystemPath(string $fileName): string {
		if (!$this->isValidFileName($fileName) || !$this->isRegisteredFile($fileName)) {
			throw new LAMException(_('Unable to read file.'));
		}
		return __DIR__ . '/../tmp/' . $fileName;
	}

}

