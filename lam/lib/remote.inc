<?php
namespace LAM\REMOTE;
use \LAMException;
use \phpseclib\Net\SSH2;
use \phpseclib\Crypt\RSA;
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

	/** SSH2 server handle */
	private $server = null;

	/**
	 * Constructor, include SSH library.
	 */
	public function __construct() {
		$this->includeSshLibrary();
	}

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
		return $this->server->exec("sudo " . $_SESSION['config']->get_scriptPath() . ' ' . escapeshellarg($command));
	}

	/**
	 * Connects to the given SSH server.
	 *
	 * @param RemoteServerConfiguration $server server (e.g. localhost or localhost,1234)
	 */
	public function connect($server) {
		if ($server === null) {
			logNewMessage(LOG_ERR, 'No server provided for lamdaemon');
			throw new LAMException(_("Unable to connect to remote server!"));
		}
		$serverNameParts = explode(",", $server->getServer());
		$handle = false;
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
	 */
	private function loginSSH($handle) {
		$username = $_SESSION['config']->getScriptUserName();
		$ldapUser = $_SESSION['ldap']->getUserName();
		if (empty($username)) {
			// get user name from current LAM user
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
		$password = $_SESSION['ldap']->getPassword();
		$keyPath = $_SESSION['config']->getScriptSSHKey();
		$keyPassword = $_SESSION['config']->getScriptSSHKeyPassword();
		if (!empty($keyPath)) {
			$password = $this->loadKey($keyPath, $keyPassword);
		}
		$login = @$handle->login($username, $password);
		if (!$login) {
			throw new LAMException(_("Unable to login to remote server!"));
		}
	}

	/**
	 * Include the SSH files.
	 */
	private function includeSshLibrary() {
		$prefix = dirname(__FILE__) . '/3rdParty/phpseclib/';
		require_once($prefix . 'Crypt/Base.php');
		require_once($prefix . 'Crypt/Blowfish.php');
		require_once($prefix . 'Crypt/Hash.php');
		require_once($prefix . 'Crypt/Random.php');
		require_once($prefix . 'Crypt/RC4.php');
		require_once($prefix . 'Crypt/Rijndael.php');
		require_once($prefix . 'Crypt/AES.php');
		require_once($prefix . 'Crypt/RSA.php');
		require_once($prefix . 'Crypt/DES.php');
		require_once($prefix . 'Crypt/TripleDES.php');
		require_once($prefix . 'Crypt/Twofish.php');
		require_once($prefix . 'Math/BigInteger.php');
		require_once($prefix . 'System/SSH/Agent.php');
		require_once($prefix . 'Net/SSH2.php');
	}

	/**
	 * Loads the key
	 *
	 * @param string $keyPath file name
	 * @param string $keyPassword password
	 * @throws LAMException error loading key
	 * @return \phpseclib\Crypt\RSA key object
	 */
	public function loadKey($keyPath, $keyPassword) {
		// use key authentication
		if (!file_exists($keyPath) || !is_readable($keyPath)) {
			throw new LAMException(sprintf(_("Unable to read %s."), htmlspecialchars($keyPath)));
		}
		$key = file_get_contents($keyPath);
		$rsa = new RSA();
		if (!empty($keyPassword)) {
			$rsa->setPassword($keyPassword);
		}
		if (!$rsa->loadKey($key)) {
			throw new LAMException(sprintf(_("Unable to load key %s."), htmlspecialchars($keyPath)));
		}
		return $rsa;
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

	/**
	 * Constructor
	 *
	 * @param string $server server DNS name
	 * @param string $label label for GUI
	 * @param string $homeDirPrefix prefix for home directories
	 */
	public function __construct($server, $label, $homeDirPrefix) {
		$this->server = $server;
		$this->label = $label;
		if (empty($label)) {
			$this->label = $server;
		}
		$this->homeDirPrefix = $homeDirPrefix;
		if (empty($homeDirPrefix)) {
			$this->homeDirPrefix = '';
		}
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

}

?>
