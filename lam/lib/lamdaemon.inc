<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2004 - 2016  Roland Gruber

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
* This file includes functions to control lamdaemon.
*
* @author Tilo Lutz
* @author Roland Gruber
* @author Thomas Manninger
*
* @package modules
*/

/**
* Sends commands to lamdaemon script.
*
* @param array $command command to execute
* @param string $server remote server
* @return array Output of lamdaemon
*
*/
function lamdaemon($command, $server) {
	if ($server == '') {
		return array();
	}
	// add phpseclib to include path
	set_include_path(get_include_path() . PATH_SEPARATOR . dirname(__FILE__) . '/3rdParty/phpseclib');
	include_once('Net/SSH2.php');
	try {
		$handle = lamConnectSSH($server);
	}
	catch (Exception $e) {
		return array("ERROR," . $e->getMessage() . "," . $server);
	}
	$output = $handle->exec("sudo " . $_SESSION['config']->get_scriptPath() . ' ' . escapeshellarg($command));
	return array($output);
}

/**
 * Connects to the given SSH server.
 *
 * @param String $server server name (e.g. localhost or localhost,1234)
 * @return object handle
 */
function lamConnectSSH($server) {
	// add phpseclib to include path
	set_include_path(get_include_path() . PATH_SEPARATOR . dirname(__FILE__) . '/3rdParty/phpseclib');
	include_once('Net/SSH2.php');
	include_once('Crypt/RSA.php');
	$serverNameParts = explode(",", $server);
	$handle = false;
	if (sizeof($serverNameParts) > 1) {
		$handle = @new Net_SSH2($serverNameParts[0], $serverNameParts[1]);
	}
	else {
		$handle = @new Net_SSH2($server);
	}
	if (!$handle) {
		throw new Exception(_("Unable to connect to remote server!"));
	}
	lamLoginSSH($handle);
	return $handle;
}

/**
 * Performs a login to the provided SSH handle.
 *
 * @param handle $handle SSH handle
 * @throws Exception login failed
 */
function lamLoginSSH($handle) {
	$username = $_SESSION['config']->getScriptUserName();
	$credentials = $_SESSION['ldap']->decrypt_login();
	if (empty($username)) {
		// get user name from current LAM user
		$sr = @ldap_read($_SESSION['ldap']->server(), $credentials[0], "objectClass=posixAccount", array('uid'), 0, 0, 0, LDAP_DEREF_NEVER);
		if ($sr) {
			$entry = @ldap_get_entries($_SESSION['ldap']->server(), $sr);
			$username = $entry[0]['uid'][0];
		}
		if (empty($username)) {
			throw new Exception(sprintf(_("Your LAM admin user (%s) must be a valid Unix account to work with lamdaemon!"), getAbstractDN($credentials[0])));
		}
	}
	$password = $credentials[1];
	$keyPath = $_SESSION['config']->getScriptSSHKey();
	if (!empty($keyPath)) {
		// use key authentication
		if (!file_exists($keyPath) || !is_readable($keyPath)) {
			throw new Exception(sprintf(_("Unable to read %s."), htmlspecialchars($keyPath)));
		}
		$key = file_get_contents($keyPath);
		$rsa = new Crypt_RSA();
		$keyPassword = $_SESSION['config']->getScriptSSHKeyPassword();
		if (!empty($keyPassword)) {
			$rsa->setPassword($keyPassword);
		}
		if (!$rsa->loadKey($key)) {
			throw new Exception(sprintf(_("Unable to load key %s."), htmlspecialchars($keyPath)));
		}
		$password = $rsa;
	}
	$login = @$handle->login($username, $password);
	if (!$login) {
		throw new Exception(_("Unable to login to remote server!"));
	}
}

?>
