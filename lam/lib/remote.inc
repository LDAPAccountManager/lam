<?php
namespace LAM\REMOTE;
use Exception;
use \LAMException;
use \phpseclib3\Net\SSH2;
use \phpseclib3\Crypt\PublicKeyLoader;
use \phpseclib3\Exception\NoKeyLoadedException;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2017 - 2023  Roland Gruber

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
* This file includes functions to control LAM remote executions.
*
* @author Roland Gruber
*
* @package modules
*/

/**
 * Runs remote commands.
 *
 * @author Roland Gruber
 */
class Remote {

	/** @var RemoteServerConfiguration */
	private $serverConfig;

	/**
	 * Constructor
	 */
	public function __construct() {
		include_once __DIR__ . '/3rdParty/composer/autoload.php';
	}

	/** SSH2 server handle */
	private $server;

	/**
	* Sends commands to remote script.
	*
	* @param string $command command to execute
	* @return string output of remote script
	*/
	public function execute($command) {
		if ($this->server == null) {
			return array();
		}
		return $this->server->exec("sudo " . $this->serverConfig->getScriptPath() . ' ' . escapeshellarg($command));
	}

	/**
	 * Connects to the given SSH server.
	 *
	 * @param RemoteServerConfiguration $server server (e.g. localhost or localhost,1234)
	 * @throws LAMException error connecting to remote server
	 */
	public function connect($server) {
		if ($server === null) {
			logNewMessage(LOG_ERR, 'No server provided for lamdaemon');
			throw new LAMException(_("Unable to connect to remote server!"));
		}
		$this->serverConfig = $server;
		$serverNameParts = explode(",", $this->serverConfig->getServer());
		if (sizeof($serverNameParts) > 1) {
			$handle = @new SSH2($serverNameParts[0], $serverNameParts[1]);
		}
		else {
			$handle = @new SSH2($serverNameParts[0]);
		}
		if (!$handle) {
			throw new LAMException(_("Unable to connect to remote server!"));
		}
		$this->loginSSH($handle);
		$this->server = $handle;
	}

	/**
	 * Closes the connection.
	 */
	public function disconnect() {
		if ($this->server == null) {
			return;
		}
		$this->server->disconnect();
	}

	/**
	 * Performs a login to the provided SSH handle.
	 *
	 * @param SSH2 $handle SSH handle
	 * @throws Exception login failed
	 * @throws LAMException error on login
	 */
	private function loginSSH($handle) {
		$username = $this->serverConfig->getUserName();
		if (empty($username)) {
			// get user name from current LAM user
			$ldapUser = $_SESSION['ldap']->getUserName();
			$sr = @ldap_read($_SESSION['ldap']->server(), $ldapUser, "objectClass=posixAccount", array('uid'), 0, 0, 0, LDAP_DEREF_NEVER);
			if ($sr) {
				$entry = @ldap_get_entries($_SESSION['ldap']->server(), $sr);
				if (!empty($entry[0]['uid'])) {
					$username = $entry[0]['uid'][0];
				}
			}
			if (empty($username)) {
				throw new LAMException(sprintf(_("Your LAM admin user (%s) must be a valid Unix account to work with lamdaemon!"), getAbstractDN($ldapUser)));
			}
		}
		$keyPath = $this->serverConfig->getSshKey();
		$keyPassword = $this->serverConfig->getSshKeyPassword();
		if (!empty($keyPath)) {
			$password = $this->loadKey($keyPath, $keyPassword);
		}
		else {
			$password = $_SESSION['ldap']->getPassword();
		}
		try {
			$login = @$handle->login($username, $password);
			if (!$login) {
				$errors = $handle->getErrors();
				if (!empty($errors)) {
					$errors = implode(' ', $errors);
				} else {
					$errors = null;
				}
				throw new LAMException(_("Unable to login to lamdaemon server."), $errors);
			}
		}
		catch (Exception $e) {
			throw new LAMException(_("Unable to login to lamdaemon server."), $e->getMessage());
		}
	}

	/**
	 * Loads the key
	 *
	 * @param string $keyPath file name
	 * @param string $keyPassword password
	 * @throws LAMException error loading key
	 * @return \phpseclib3\Crypt\Common\AsymmetricKey key object
	 */
	public function loadKey($keyPath, $keyPassword) {
		// use key authentication
		if (!file_exists($keyPath) || !is_readable($keyPath)) {
			throw new LAMException(sprintf(_("Unable to read %s."), htmlspecialchars($keyPath)));
		}
		$keyData = file_get_contents($keyPath);
		try {
			if (empty($keyPassword)) {
				return PublicKeyLoader::load($keyData);
			}
			else {
				return PublicKeyLoader::load($keyData, $keyPassword);
			}
		}
		catch (NoKeyLoadedException $e) {
			logNewMessage(LOG_ERR, 'Unable to load key %s: %s', $keyPath, $e->getMessage());
			throw new LAMException(sprintf(_("Unable to load key %s."), htmlspecialchars($keyPath)));
		}
	}

}

/**
 * Server configuration.
 *
 * @author Roland Gruber
 */
class RemoteServerConfiguration {

	private $server;
	private $label;
	private $homeDirPrefix;
	private $scriptPath;
	private $userName;
	private $sshKey;
	private $sshKeyPassword;

	/**
	 * Constructor
	 *
	 * @param string $server server DNS name
	 * @param string $label label for GUI
	 * @param string $homeDirPrefix prefix for home directories
	 * @param string $scriptPath script path
	 * @param string $userName user name
	 * @param string $sshKey SSH key path
	 * @param string $sshKeyPassword SSH key password
	 */
	public function __construct($server, $label, $homeDirPrefix, $scriptPath, $userName, $sshKey, $sshKeyPassword) {
		$this->server = $server;
		$this->label = $label;
		if (empty($label)) {
			$this->label = $server;
		}
		$this->homeDirPrefix = $homeDirPrefix;
		if (empty($homeDirPrefix)) {
			$this->homeDirPrefix = '';
		}
		$this->scriptPath = $scriptPath;
		$this->userName = $userName;
		$this->sshKey = $sshKey;
		$this->sshKeyPassword = $sshKeyPassword;
	}

	/**
	 * Returns the server's DNS name.
	 *
	 * @return string server name
	 */
	public function getServer() {
		return $this->server;
	}

	/**
	 * Returns a descriptive label.
	 *
	 * @return string label
	 */
	public function getLabel() {
		return $this->label;
	}

	/**
	 * Returns the prefix for user home directories.
	 *
	 * @return string prefix for user home directories
	 */
	public function getHomeDirPrefix() {
		return $this->homeDirPrefix;
	}

	/**
	 * Returns the script path.
	 *
	 * @return string script path
	 */
	public function getScriptPath() {
		return $this->scriptPath;
	}

	/**
	 * Returns the user name to connect.
	 *
	 * @return string user name
	 */
	public function getUserName() {
		return $this->userName;
	}

	/**
	 * Returns the path to the SSH key.
	 *
	 * @return string path
	 */
	public function getSshKey() {
		return $this->sshKey;
	}

	/**
	 * Returns the SSH key password.
	 *
	 * @return string password
	 */
	public function getSshKeyPassword() {
		return $this->sshKeyPassword;
	}

}
