<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003 - 2006  Roland Gruber

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
/** LDAP schema */
include_once("schema.inc");

/**
* Converts a HEX string to a binary value
*
* @param string $value HEX string
* @return binary result binary
*/
function hex2bin($value) {
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
	function Ldap($config) {
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
	* @return mixed if connect succeeds the server handle is returned, else false
	*/
	function connect($user, $passwd) {
		// close any prior connection
		@$this->close();
		// do not allow anonymous bind
		if ((!$user)||($user == "")||(!$passwd)) {
			return false;
		}
		// save password und username encrypted
		$this->encrypt_login($user, $passwd);
		$this->server = @ldap_connect($this->conf->get_ServerURL());
		if ($this->server) {
			// use LDAPv3
			ldap_set_option($this->server, LDAP_OPT_PROTOCOL_VERSION, 3);
			// start TLS if possible
			if (function_exists('ldap_start_tls')) {
				@ldap_start_tls($this->server);
				// connect without TLS if it failed
				if (ldap_errno($this->server) != 0) {
					@ldap_close($this->server);
					$this->server = @ldap_connect($this->conf->get_ServerURL());
					ldap_set_option($this->server, LDAP_OPT_PROTOCOL_VERSION, 3);
				}
			}
			$bind = @ldap_bind($this->server, $user, $passwd);
			if ($bind) {
				$return = ldap_errno($this->server);
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
	* Returns an array with all organizational units under the given suffix
	*
	* @param string $suffix search suffix
	* @return array DNs of organizational units
	*/
	function search_units($suffix) {
		$ret = array();
		$sr = @ldap_search($this->server(), $suffix, "objectClass=organizationalunit", array("DN"));
		if ($sr) {
			$units = ldap_get_entries($this->server, $sr);
			unset($units['count']);
			// extract Dns
			for ($i = 0; $i < sizeof($units); $i++) {
				if ($units[$i]['dn']) $ret[] = $units[$i]['dn'];
			}
		}
		// add root suffix if needed
		$found = false;
		for ($i = 0; $i < sizeof($ret); $i++) { // search suffix case-intensitive
			if (strtolower($suffix) == strtolower($ret[$i])) {
				$found = true;
				break;
			}
		}
		if (!$found) {
			$ret[] = $suffix;
		}
		usort($ret, array($this,"cmp_array"));
		return $ret;
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
		// delete PDF files which are older than 10 min
		if (isset($_SESSION['lampath'])) {
			$relpath = $_SESSION['lampath'] . 'tmp/';
			$time = time();
			$dir = @opendir($relpath);
			$file = @readdir($dir);
			while ($file) {
				if ((substr($file, -4) == '.pdf') || (substr($file, -4) == '.jpg')) {
					$path = $relpath . $file;
					if ($time - filemtime($path) > 600) {
						@unlink($path);
					}
				}
				$file = @readdir($dir);
			}
			@closedir($dir);
		}
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
	* @return object encrypted string
	*/
	function encrypt($data) {
		// use MCrypt if available
		if (function_exists('mcrypt_create_iv')) {
			// MCrypt may have been enabled in a running session
			if (!isset($_COOKIE["IV"]) || ($_COOKIE["IV"] == '')) return $data;
			if ($_COOKIE["IV"] == "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx") {
				return $data;
			}
			// read key and iv from cookie
			$iv = base64_decode($_COOKIE["IV"]);
			$key = base64_decode($_COOKIE["Key"]);
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
	* @return string decrypted string
	*/
	function decrypt($data) {
		// use MCrypt if available
		if (function_exists('mcrypt_create_iv')) {
			// MCrypt may have been enabled in a running session
			if (!isset($_COOKIE["IV"]) || ($_COOKIE["IV"] == '')) return $data;
			if ($_COOKIE["IV"] == "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx") {
				return $data;
			}
			// read key and iv from cookie
			$iv = base64_decode($_COOKIE["IV"]);
			$key = base64_decode($_COOKIE["Key"]);
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


	/**
	* Helper function to sort the unit DNs
	*
	* @param string $a first argument to compare
	* @param string $b second argument to compare
	* @return integer 0 if equal, 1 if $a is greater, -1 if $b is greater
	*/
	function cmp_array($a, $b) {
		// split DNs
		$array_a = explode(",", $a);
		$array_b = explode(",", $b);
		$len_a = sizeof($array_a);
		$len_b = sizeof($array_b);
		// check how many parts to compare
		$len = min($len_a, $len_b);
		// compare from last part on
		for ($i = 0; $i < $len; $i++) {
			// get parts to compare
			$part_a = strtolower($array_a[$len_a - $i - 1]);
			$part_b = strtolower($array_b[$len_b - $i - 1]);
			// compare parts
			if ($part_a == $part_b) { // part is identical
				if ($i == ($len - 1)) {
					if ($len_a > $len_b) return 1;
					elseif ($len_a < $len_b) return -1;
					else return 0; // DNs are identical
				}
			}
			elseif ($part_a == max($part_a, $part_b)) return 1;
			else return -1;
		}
		return -1;
	}


}

?>
