<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2026  Roland Gruber

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
 * ldap.inc provides basic functions to connect to the OpenLDAP server.
 *
 * @package LDAP
 * @author Roland Gruber
 */

/** Access to configuration data */

use LDAP\Connection;

include_once __DIR__ . "/config.inc";

/**
 * Ldap manages connection to LDAP and includes several helper functions.
 *
 * @package LDAP
 */
class Ldap {

	/** Object of Config to access preferences */
	private ?LAMConfig $conf = null;

	/** Server handle */
	private ?Connection $server = null;

	/** LDAP connection established */
	private bool $is_connected = false;

	/** LDAP username used for bind */
	private ?string $username = null;
	/** LDAP password used for bind */
	private ?string $password = null;
	/** is Active Directory */
	private ?bool $isActiveDirectory = null;

	/**
	 * Creates a new LDAP object.
	 *
	 * @param LAMConfig|null $config configuration
	 */
	public function __construct(?LAMConfig $config) {
		if ($config !== null) {
			$this->conf = $config;
		}
	}

	/**
	 * Connects to the server using the given username and password
	 *
	 * @param string|null $user user name
	 * @param string|null $passwd password
	 * @param boolean $allowAnonymous specifies if anonymous binds are allowed
	 * @throws LAMException unable to connect
	 */
	public function connect($user, $passwd, $allowAnonymous = false): void {
		// close any prior connection
		@$this->close();
		// do not allow anonymous bind
		if (!$allowAnonymous && (($user === null) || ($user === '') || ($passwd === null) || ($passwd === ''))) {
			throw new LAMException(_("Cannot connect to specified LDAP server. Please try again."));
		}
		// save password und username encrypted
		$this->encrypt_login($user, $passwd);
		$startTLS = $this->conf->getUseTLS();
		$startTLS = ($startTLS === 'yes');
		$this->server = connectToLDAP($this->conf->get_ServerURL(), $startTLS);
		if ($this->server != null) {
			// referral following
			$followReferrals = ($this->conf->getFollowReferrals() === 'true') ? 1 : 0;
			ldap_set_option($this->server, LDAP_OPT_REFERRALS, $followReferrals);
			$bind = @ldap_bind($this->server, $user, $passwd);
			if ($bind) {
				$this->is_connected = true;
				return;
			}
			// return error number
			$errorNumber = ldap_errno($this->server);
			$clientSource = empty($_SERVER['REMOTE_ADDR']) ? '' : $_SERVER['REMOTE_ADDR'];
			if ($errorNumber === 81) {
				// connection failed
				logNewMessage(LOG_ERR, 'User ' . $user . ' (' . $clientSource . ') failed to log in (LDAP error: ' . getDefaultLDAPErrorString($this->server) . ').');
				throw new LAMException(_("Cannot connect to specified LDAP server. Please try again."), '', null, $errorNumber);
			}
			elseif ($errorNumber === 49) {
				// username/password invalid. Return to login page.
				logNewMessage(LOG_ERR, 'User ' . $user . ' (' . $clientSource . ') failed to log in (wrong password). ' . getDefaultLDAPErrorString($this->server));
				throw new LAMException(_("Wrong password/user name combination. Please try again."), getDefaultLDAPErrorString($this->server), null, $errorNumber);
			}
			else {
				// other errors
				logNewMessage(LOG_ERR, 'User ' . $user . ' (' . $clientSource . ') failed to log in (LDAP error: ' . getDefaultLDAPErrorString($this->server) . ').');
				throw new LAMException(_("Cannot connect to specified LDAP server. Please try again."), "($errorNumber) " . getDefaultLDAPErrorString($this->server), null, $errorNumber);
			}
		}
		throw new LAMException(_("Cannot connect to specified LDAP server. Please try again."));
	}

	/**
	 * Checks if the given password is the new password to use for the current user.
	 *
	 * @param string $newPassword new password
	 */
	public function tryAndApplyNewPassword(string $newPassword): void {
		logNewMessage(LOG_DEBUG, 'Check if new LDAP password needs to be used: ' . $this->getUserName() . ' - ' . $newPassword[0] . '...');
		$bind = @ldap_bind($this->server, $this->getUserName(), $newPassword);
		if ($bind) {
			logNewMessage(LOG_DEBUG, 'Password updated');
			$this->encrypt_login($this->getUserName(), $newPassword);
		}
		else {
			logNewMessage(LOG_DEBUG, 'Keeping old password as bind failed. ' . getDefaultLDAPErrorString($this->server));
		}
	}

	/** Closes connection to server */
	public function close(): void {
		if ($this->server != null) {
			$this->is_connected = false;
			@ldap_close($this->server);
			$this->server = null;
		}
	}

	/**
	 * Returns the LDAP connection handle
	 *
	 * @return Connection|null connection handle
	 */
	public function server() {
		if (!$this->is_connected) {
			try {
				$this->connect($this->getUserName(), $this->getPassword());
				$this->is_connected = true;
			}
			catch (LAMException $e) {
				logNewMessage(LOG_ERR, $e->getTitle() . ' ' . $e->getMessage());
			}
		}
		return $this->server;
	}

	/**
	 * Closes the connection to LDAP server before serialization.
	 *
	 * @return array<string, string|LAMConfig|bool> data
	 */
	public function __serialize(): array {
		$this->close();
		// define which attributes to save
		return [
			'conf' => $this->conf,
			'username' => $this->username,
			'password' => $this->password,
			'isActiveDirectory' => $this->isActiveDirectory
		];
	}

	/**
	 * Reconnects to the LDAP server when deserialized.
	 *
	 * @param array<string, string|LAMConfig|bool> $data data
	 */
	public function __unserialize(array $data): void {
		$this->is_connected = false;
		$this->conf = $data['conf'];
		$this->username = $data['username'];
		$this->password = $data['password'];
		$this->isActiveDirectory = $data['isActiveDirectory'];
		// delete PDF files and images which are older than 15 min
		$tmpDir = __DIR__ . '/../tmp/';
		$time = time();
		$dir = @opendir($tmpDir);
		$file = @readdir($dir);
		while ($file) {
			$path = $tmpDir . $file;
			if ((!str_starts_with($file, '.'))
				&& !is_dir($path)
				&& ($time - filemtime($path) > 900)) {
				@unlink($path);
			}
			$file = @readdir($dir);
		}
		@closedir($dir);
		// clean internal files that are older than 24 hours
		$tmpDir = __DIR__ . '/../tmp/internal/';
		$time = time();
		$dir = @opendir($tmpDir);
		$file = @readdir($dir);
		while ($file) {
			if (str_ends_with($file, '.tmp')) {
				$path = $tmpDir . $file;
				if ($time - filemtime($path) > (3600 * 24)) {
					@unlink($path);
				}
			}
			$file = @readdir($dir);
		}
		@closedir($dir);
	}

	/**
	 * Encrypts username and password
	 *
	 * @param string $username LDAP user name
	 * @param string $password LDAP password
	 */
	public function encrypt_login($username, $password): void {
		// encrypt username and password
		$this->username = base64_encode(lamEncrypt($username));
		$this->password = base64_encode(lamEncrypt($password));
	}

	/**
	 * Returns the LDAP username.
	 *
	 * @return string|null user name
	 */
	public function getUserName(): ?string {
		if ($this->username === null) {
			return null;
		}
		return lamDecrypt(base64_decode($this->username));
	}

	/**
	 * Returns the LDAP password.
	 *
	 * @return string|null password
	 */
	public function getPassword(): ?string {
		if ($this->password === null) {
			return null;
		}
		return lamDecrypt(base64_decode($this->password));
	}

	/** Closes connection to LDAP server and deletes encrypted username/password */
	public function destroy(): void {
		$this->close();
		$this->username = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
		$this->password = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
	}

	/**
	 * Returns if the LDAP server is an Active Directory.
	 *
	 * @return bool is Active Directory
	 */
	public function isActiveDirectory(): bool {
		if ($this->isActiveDirectory !== null) {
			return $this->isActiveDirectory;
		}
		$rootDse = ldapGetDN('', ['*']);
		logNewMessage(LOG_DEBUG, 'Root DSE: ' . print_r($rootDse, true));
		$this->isActiveDirectory = isset($rootDse['domaincontrollerfunctionality']) || isset($rootDse['forestfunctionality']);
		return $this->isActiveDirectory;
	}

}
