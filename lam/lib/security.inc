<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2006  Roland Gruber

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

// check client IP address
checkClientIP();

/**
 * Starts a session and checks the environment.
 * The script is stopped if one of the checks fail.
 */
function startSecureSession() {
	// start session
	if (isset($_SESSION)) unset($_SESSION);
	$sessionDir = substr(__FILE__, 0, strlen(__FILE__) - 17) . "/sess";
	session_save_path($sessionDir);
	@session_start();
	// check session id
	if (! isset($_SESSION["sec_session_id"]) || ($_SESSION["sec_session_id"] != session_id())) {
		// session id is invalid
		die();
	}
	// check if client IP has not changed
	if (!isset($_SESSION["sec_client_ip"]) || ($_SESSION["sec_client_ip"] != $_SERVER['REMOTE_ADDR'])) {
		// IP is invalid
		die();
	}
	// check if session time has not expired
	if (($_SESSION['sec_sessionTime'] + (60 * $_SESSION['cfgMain']->sessionTimeout)) > time()) {
		// ok, update time
		$_SESSION['sec_sessionTime'] = time();
	}
	else {
		// session expired, logoff user
		logoffAndBackToLoginPage();
	}
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
	// skip test if no hosts are defined
	if ($allowedHosts == "") return;
	$allowedHosts = explode(",", $allowedHosts);
	$grantAccess = false;
	for ($i = 0; $i < sizeof($allowedHosts); $i++) {
		$host = $allowedHosts[$i];
		$ipRegex = '^[0-9\\.\\*]+$';
		if (!ereg($ipRegex, $host)) continue;
		$hostRegex = str_replace(".", "\\.", $host);
		$hostRegex = '^' . str_replace("*", ".*", $hostRegex) . '$';
		$clientIP = $_SERVER['REMOTE_ADDR'];
		if (ereg($hostRegex, $clientIP)) {
			// client is allowed to access LAM
			$grantAccess = true;
		}
	}
	// stop script is client may not access LAM
	if (!$grantAccess) die();
}

/**
 * Logs off the user and displays the login page.
 *
 */
function logoffAndBackToLoginPage() {
	// log message
	$ldapUser = $_SESSION['ldap']->decrypt_login();
	logNewMessage(LOG_WARNING, 'Session of user ' . $ldapUser[0] . ' expired.');
	// delete key and iv in cookie
	if (function_exists('mcrypt_create_iv')) {
		setcookie("Key", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", 0, "/");
		setcookie("IV", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", 0, "/");
	}
	// close LDAP connection
	@$_SESSION["ldap"]->destroy();
	// link back to login page
	$paths = array('./', '../', '../../', '../../../');
	$page = 'login.php';
	for ($i = 0; $i < sizeof($paths); $i++) {
		if (file_exists($paths[$i] . $page)) {
			$page = $paths[$i] . $page;
			break;
		}
	}
	$page .= "?expired=yes";
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
 * @param string $level log level (LOG_NOTICE, LOG_WARNING, LOG_ERR)
 * @param string $message log message
 */
function logNewMessage($level, $message) {
	$possibleLevels = array(LOG_NOTICE => 'NOTICE', LOG_WARNING => 'WARNING', LOG_ERR => 'ERROR');
	if (!in_array($level, array_keys($possibleLevels))) StatusMessage('ERROR', 'Invalid log level!', $level);
	if (isset($_SESSION['cfgMain'])) $cfg = $_SESSION['cfgMain'];
	else $cfg = new LAMCfgMain();
	// check if logging is disabled
	if ($cfg->logDestination == 'NONE') return;
	// check if log level is high enough
	elseif ($cfg->logLevel < $level) return;
	// ok to log, build log message
	$prefix = "LDAP Account Manager (" . session_id() . ") - " . $possibleLevels[$level] . ": ";
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
				fwrite($file, $message . "\n");
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
 * @return boolean true, if allowed
 */
function checkIfWriteAccessIsAllowed() {
	if (!isset($_SESSION['config'])) {
		return false;
	}
	if ($_SESSION['config']->getAccessLevel() >= LAMConfig::ACCESS_ALL) {
		return true;
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
 * Checks if the password fulfills the password policies.
 * 
 * @param string $password password
 * @return mixed true if ok, string with error message if not valid
 */
function checkPasswordStrength($password) {
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
		if (ereg("[a-z]", $password[$i])) {
			$lower++;
		}
		if (ereg("[A-Z]", $password[$i])) {
			$upper++;
		}
		if (ereg("[0-9]", $password[$i])) {
			$numeric++;
		}
		if (eregi("[^a-z0-9]", $password[$i])) {
			$symbols++;
		}
	}
	// check lower case
	if ($lower < $cfg->passwordMinLower) {
		return sprintf(_('The password is too weak. You have to enter at least %s lower case characters.'), $cfg->passwordMinLower);
	}
	// check upper case
	if ($upper < $cfg->passwordMinUpper) {
		return sprintf(_('The password is too weak. You have to enter at least %s upper case characters.'), $cfg->passwordMinUpper);
	}
	// check numeric
	if ($numeric < $cfg->passwordMinNumeric) {
		return sprintf(_('The password is too weak. You have to enter at least %s numeric characters.'), $cfg->passwordMinNumeric);
	}
	// check symbols
	if ($symbols < $cfg->passwordMinSymbol) {
		return sprintf(_('The password is too weak. You have to enter at least %s symbolic characters.'), $cfg->passwordMinSymbol);
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
	if ($classes < $cfg->passwordMinClasses) {
		return sprintf(_('The password is too weak. You have to enter at least %s different character classes (upper/lower case, numbers and symbols).'), $cfg->passwordMinClasses);
	}
	return true;
}

?>