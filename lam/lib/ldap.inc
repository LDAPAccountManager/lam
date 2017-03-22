<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2017  Roland Gruber

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
include_once("config.inc");

/**
* Converts a HEX string to a binary value
*
* @param string $value HEX string
* @return binary result binary
*/
function convertHex2bin($value) {
	return pack("H*", $value);
}


/**
* Ldap manages connection to LDAP and includes several helper functions.
*
* @package LDAP
*/
class Ldap{

	/** Object of Config to access preferences */
	private $conf;

	/** Server handle */
	private $server;

	/** LDAP connection established */
	private $is_connected = false;

	/** LDAP username used for bind */
	private $username;
	/** LDAP password used for bind */
	private $password;

	/**
	* Creates a new LDAP object.
	*
	* @param object $config an object of class Config
	*/
	function __construct($config) {
		if (is_object($config)) {
			$this->conf = $config;
		}
		else {
			return false;
		}
		return true;
	}

	/**
	* Connects to the server using the given username and password
	*
	* @param string $user user name
	* @param string $passwd password
	* @param boolean $allowAnonymous specifies if anonymous binds are allowed
	* @return mixed if connect succeeds the 0 is returned, else false or error number
	*/
	function connect($user, $passwd, $allowAnonymous=false) {
		// close any prior connection
		@$this->close();
		// do not allow anonymous bind
		if (!$allowAnonymous && ((!$user)||($user == "")||(!$passwd))) {
			return false;
		}
		// save password und username encrypted
		$this->encrypt_login($user, $passwd);
		$this->server = @ldap_connect($this->conf->get_ServerURL());
		if ($this->server) {
			// use LDAPv3
			ldap_set_option($this->server, LDAP_OPT_PROTOCOL_VERSION, 3);
			// referral following
			$followReferrals = ($this->conf->getFollowReferrals() === 'true') ? 1 : 0;
			ldap_set_option($this->server,LDAP_OPT_REFERRALS, $followReferrals);
			// start TLS if specified
			$useTLS = $this->conf->getUseTLS();
			if (isset($useTLS) && ($useTLS == "yes")) {
				@ldap_start_tls($this->server);
				if (ldap_errno($this->server) != 0) {
					logNewMessage(LOG_ERR, 'Unable to start TLS encryption. Please check if your server certificate is valid and if the LDAP server supports TLS at all.');
					return ldap_errno($this->server);
				}
			}
			$bind = @ldap_bind($this->server, $user, $passwd);
			if ($bind) {
				$return = ldap_errno($this->server);
				$this->is_connected = true;
				// return success number
				return $return;
			}
			// return error number
			else return ldap_errno($this->server);
		}
		else return false;
	}

	/** Closes connection to server */
	function close() {
		if ($this->server != null) {
			@ldap_close($this->server);
		}
	}

	/**
	* Returns the LDAP connection handle
	*
	* @return object connection handle
	*/
	function server() {
		if (!$this->is_connected) {
			$data = $this->decrypt_login();
			$this->connect($data[0], $data[1]);
			$this->is_connected = true;
		}
		return $this->server;
	}

	/** Closes connection to LDAP server before serialization */
	function __sleep() {
		$this->close();
		// define which attributes to save
		return array("conf", "username", "password");
	}

	/** Reconnects to LDAP server when deserialized */
	function __wakeup() {
		$this->is_connected = false;
		// delete PDF files and images which are older than 15 min
		$tmpDir = dirname(__FILE__) . '/../tmp/';
		$time = time();
		$dir = @opendir($tmpDir);
		$file = @readdir($dir);
		while ($file) {
			$path = $tmpDir . $file;
			if ((substr($file, 0, 1) != '.') && !is_dir($path)) {
				if ($time - filemtime($path) > 900) {
					@unlink($path);
				}
			}
			$file = @readdir($dir);
		}
		@closedir($dir);
		// clean internal files that are older than 24 hours
		$tmpDir = dirname(__FILE__) . '/../tmp/internal/';
		$time = time();
		$dir = @opendir($tmpDir);
		$file = @readdir($dir);
		while ($file) {
			if ((substr($file, -4) == '.tmp')) {
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
	function encrypt_login($username, $password) {
		// encrypt username and password
		$this->username = base64_encode(lamEncrypt($username));
		$this->password = base64_encode(lamEncrypt($password));
	}

	/**
	* Decrypts username and password
	*
	* @return array array(user name, password)
	*/
	function decrypt_login() {
		// decrypt username and password
		$username = lamDecrypt(base64_decode($this->username));
		$password = lamDecrypt(base64_decode($this->password));
		$ret = array($username, $password);
		return $ret;
	}

	/** Closes connection to LDAP server and deletes encrypted username/password */
	function destroy() {
		$this->close();
		$this->username="xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
		$this->password="xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
	}


}

?>
