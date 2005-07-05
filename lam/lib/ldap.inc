<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Roland Gruber

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
/** Encryption functions */
include_once("blowfish.inc");
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
	var $conf;

	/** Server handle */
	var $server;

	/** LDAP username used for bind */
	var $username;
	/** LDAP password used for bind */
	var $password;

	/** Array with all objectClass strings from the LDAP server */
	var $objectClasses;
	/** Array with all attribute strings from the LDAP server */
	var $attributes;

	// Capabilities of the LDAP server
	/** Host attribute in inetOrgPerson */
	var $supports_unix_hosts = false;
	/** ObjectClass sambaAccount */
	var $supports_samba2_schema = false;
	/** ObjectClass sambaSamAccount */
	var $supports_samba3_schema = false;

	/** Random number (changes on every page request) */
	var $rand;

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
				// read objectClasses from server and update capabilities if needed
				if (! $this->objectClasses) {
					$this->updateClasses();
					$this->updateCapabilities();
					// update schema
					get_schema_objectclasses();
					get_schema_attributes();
					get_schema_matching_rules();
					get_schema_syntaxes();
				}
				// return success number
				return ldap_errno($this->server);
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

	/** Reads the array of objectClasses from the LDAP server */
	function updateClasses() {
		// read from default cn
		$sr = @ldap_read($this->server, 'cn=subschema', '(objectClass=*)', array('objectclasses'));
		// if default was not correct check different cn
		if (!$sr) $sr = @ldap_read($this->server, 'cn=schema', '(objectClass=*)', array('objectclasses'));
		if ($sr) {
			// get search result and save it
			$info = @ldap_get_entries($this->server,$sr);
			if ($info) {
				$this->objectClasses = $info[0]['objectclasses'];
				array_shift($this->objectClasses);
			}
		}
		// if search failed save empty result
		else $this->objectClasses = array();

		// read from default cn
		$sr = @ldap_read($this->server, 'cn=subschema', '(objectClass=*)', array('attributetypes'));
		// if default was not correct check different cn
		if (!$sr) $sr = @ldap_read($this->server, 'cn=schema', '(objectClass=*)', array('attributetypes'));
		if ($sr) {
			// get search result and save it
			$info = @ldap_get_entries($this->server,$sr);
			if ($info) {
				$attributes = $info[0]['attributetypes'];
				array_shift($attributes);
			}
		}
		// build Attribute list
		for ($i=0; $i<count($attributes); $i++) {
			// TODO: is it save to use while in lower code?
			// find oid of attribute
			$start = 0;
			while (!get_preg($attributes[$i][$start], 'digit')) $start++;
			$end = $start;
			while ($attributes[$i][$end+1] != ' ') $end++; // find limiter
			$values['oid'] = substr($attributes[$i], $start, $end-$start);
			// find DESC of attribute			
			$start = strpos($attributes[$i], 'DESC');
			if ($start) {
				$start = $start + 6;
				$end = $start;
				while ($attributes[$i][$end]!='\'') $end++; // find limiter
				$values['DESC'] = substr($attributes[$i], $start, $end-$start);
			}
			// find SYNTAX of attribute			
			$start = strpos($attributes[$i], 'SYNTAX');
			if ($start) {
				$start = $start + 7;
				$end = $start;
				while ($attributes[$i][$end]!='{' && $attributes[$i][$end]!=' ') $end++; // find limiter
				$values['SYNTAX'] = substr($attributes[$i], $start, $end-$start);
			}
			// find length of attribute			
			$start = strpos($attributes[$i], 'SYNTAX');
			if ($start) {
				$start = $start + 8;
				while ($attributes[$i][$start]!='{' && $attributes[$i][$start]!=' ') $start++; // find limiter
				if ($attributes[$i][$start]=='{') {
					$end = $start;
					while ($attributes[$i][$end]!='}') $end++; // find limiter
					$values['LENGTH'] = substr($attributes[$i], $start, $end-$start);
				}
			}
			$start = strpos($attributes[$i], "NAME") + 6;
			if ($attributes[$i][$start-1]=='(') {
				// found multiple possible names
				$start = $start +2;
				$count = 1;
				// repeat until all names are found
				while ($attributes[$i][$start-1]!=')') {
					// search for end
					$end = $start;
					while ($attributes[$i][$end]!='\'') $end++; // find limiter
					$count++;
					$name = substr($attributes[$i], $start, $end-$start);
					$this->attributes[$name] = $values;
					$start = $end + 3;
				}
			}
			else {
				$end = $start;
				while ($attributes[$i][$end]!='\'') $end++;
				$name = substr($attributes[$i], $start, $end-$start);
				$this->attributes[$name] = $values;
			}
		}
	}

	/** Updates the capabilities values (var $supports_*) */
	function updateCapabilities() {
		for ($i = 0; $i < sizeof($this->objectClasses); $i++) {
			$line = $this->objectClasses[$i];
			// search keywords
			if (strpos($line, "NAME 'inetOrgPerson'") && strpos($line, " host ")) $this->supports_unix_hosts = true;
			if (strpos($line, "NAME 'sambaAccount'")) $this->supports_samba2_schema = true;
			if (strpos($line, "NAME 'sambaSamAccount'")) $this->supports_samba3_schema = true;
		}
	}

	/**
	* Returns the LDAP connection handle
	*
	* @return object connection handle
	*/
	function server() {
		return $this->server;
	}

	/** Closes connection to LDAP server before serialization */
	function __sleep() {
		$this->close();
		// define which attributes to save
		return array("conf", "username", "password", "objectClasses", "attributes",
			"supports_unix_hosts", "supports_samba2_schema", "supports_samba3_schema", "rand");
	}

	/** Reconnects to LDAP server when deserialized */
	function __wakeup() {
		$data = $this->decrypt_login();
		$this->connect($data[0], $data[1]);
		// change random number
		mt_srand($this->rand + (microtime() * 1000000));
		$this->rand = mt_rand();
		// delete PDF files which are older than 10 min
		if (isset($_SESSION['lampath'])) {
			$relpath = $_SESSION['lampath'] . 'tmp/';
			$time = time();
			$dir = @opendir($relpath);
			while ($file = @readdir($dir)) {
					if ((substr($file, -4) == '.pdf') || (substr($file, -4) == '.jpg')) {
					$path = $relpath . $file;
					if ($time - filemtime($path) > 600) {
						@unlink($path);
					}
				}
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
			// read key and iv from cookie
			$iv = base64_decode($_COOKIE["IV"]);
			$key = base64_decode($_COOKIE["Key"]);
			// encrypt string
			return mcrypt_encrypt(MCRYPT_RIJNDAEL_256, $key, $data, MCRYPT_MODE_ECB, $iv);
		}
		// use Blowfish if MCrypt is not available
		else {
			// read key and iv from cookie
			$iv = base64_decode($_COOKIE["IV"]);
			$key = base64_decode($_COOKIE["Key"]);
			$b_key = $iv . $key;
			// encrypt string
			$b_fish = new Cipher_blowfish();
			return $b_fish->encrypt($data, $b_key);
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
			// read key and iv from cookie
			$iv = base64_decode($_COOKIE["IV"]);
			$key = base64_decode($_COOKIE["Key"]);
			// decrypt string
			$ret = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, $data, MCRYPT_MODE_ECB, $iv);
			$ret = str_replace(chr(00), "", $ret);
			return $ret;
		}
		// use Blowfish if MCrypt is not available
		else {
			// read key and iv from cookie
			$iv = base64_decode($_COOKIE["IV"]);
			$key = base64_decode($_COOKIE["Key"]);
			$b_key = $iv . $key;
			// decrypt string
			$b_fish = new Cipher_blowfish();
			return $b_fish->decrypt($data, $b_key);
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
