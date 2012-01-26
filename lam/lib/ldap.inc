<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2012  Roland Gruber

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
	
	private $is_connected = false;

	/** LDAP username used for bind */
	private $username;
	/** LDAP password used for bind */
	private $password;

	/** Random number (changes on every page request) */
	private $rand;

	/**
	* @param object $config an object of class Config
	*/
	function __construct($config) {
		setlanguage();
		if (is_object($config)) $this->conf = $config;
		else return false;
		mt_srand((double)microtime()*1000000);
		$this->rand = mt_rand();
		return true;
	}

	/**
	* Connects to the server using the given username and password
	*
	* @param string $user user name
	* @param string $passwd password
	* @param boolean $allowAnonymous specifies if anonymous binds are allowed
	* @return mixed if connect succeeds the server handle is returned, else false
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
		@ldap_close($this->server);
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
		return array("conf", "username", "password", "rand");
	}

	/** Reconnects to LDAP server when deserialized */
	function __wakeup() {
		$this->is_connected = false;
		// change random number
		mt_srand($this->rand + (microtime() * 1000000));
		$this->rand = mt_rand();
		// delete PDF files and images which are older than 10 min
		$tmpDir = dirname(__FILE__) . '/../tmp/';
		$time = time();
		$dir = @opendir($tmpDir);
		$file = @readdir($dir);
		while ($file) {
			if ((substr($file, -4) == '.pdf') || (substr($file, -4) == '.jpg')) {
				$path = $tmpDir . $file;
				if ($time - filemtime($path) > 600) {
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
	* Calculates a new value for rand
	*
	* @return int New random value
	*/
	function new_rand() {
		// change random number
		mt_srand($this->rand + (microtime() * 1000000));
		$r = mt_rand();
		$this->rand = $r;
		return $r;
	}

	/**
	* Encrypts a string
	*
	* @param string $data string to encrypt
	* @param string $prefix prefix for cookie names
	* @return object encrypted string
	*/
	public static function encrypt($data, $prefix='') {
		// use MCrypt if available
		if (function_exists('mcrypt_create_iv')) {
			// MCrypt may have been enabled in a running session
			if (!isset($_COOKIE[$prefix . "IV"]) || ($_COOKIE[$prefix . "IV"] == '')) return $data;
			if ($_COOKIE[$prefix . "IV"] == "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx") {
				return $data;
			}
			// read key and iv from cookie
			$iv = base64_decode($_COOKIE[$prefix . "IV"]);
			$key = base64_decode($_COOKIE[$prefix . "Key"]);
			// encrypt string
			return mcrypt_encrypt(MCRYPT_RIJNDAEL_256, $key, base64_encode($data), MCRYPT_MODE_ECB, $iv);
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
	public static function decrypt($data, $prefix='') {
		// use MCrypt if available
		if (function_exists('mcrypt_create_iv')) {
			// MCrypt may have been enabled in a running session
			if (!isset($_COOKIE[$prefix . "IV"]) || ($_COOKIE[$prefix . "IV"] == '')) return $data;
			if ($_COOKIE[$prefix . "IV"] == "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx") {
				return $data;
			}
			// read key and iv from cookie
			$iv = base64_decode($_COOKIE[$prefix . "IV"]);
			$key = base64_decode($_COOKIE[$prefix . "Key"]);
			// decrypt string
			$ret = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, $data, MCRYPT_MODE_ECB, $iv);
			$ret = base64_decode(str_replace(chr(00), "", $ret));
			return $ret;
		}
		// otherwise do not decrypt
		else {
			return $data;
		}
	}

	/**
	* Encrypts username and password
	*
	* @param string $username LDAP user name
	* @param string $password LDAP password
	*/
	function encrypt_login($username, $password) {
		// encrypt username and password
		$this->username = base64_encode($this->encrypt($username));
		$this->password = base64_encode($this->encrypt($password));
	}

	/**
	* Decrypts username and password
	*
	* @return array array(user name, password)
	*/
	function decrypt_login() {
		// decrypt username and password
		$username = $this->decrypt(base64_decode($this->username));
		$password = $this->decrypt(base64_decode($this->password));
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
