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

// ldap.inc provides basic functions to connect to the OpenLDAP server.

include_once("config.inc");


// manages connection to LDAP and several helper functions
class Ldap{

	// object of Config to access preferences
	var $conf;

	// server handle
	var $server;

	// LDAP username and password used for bind
	var $username;
	var $password;

	// Arrays that contain LDAP attributes and their descriptions which are translated
	var $ldapUserAttributes;
	var $ldapGroupAttributes;
	var $ldapHostAttributes;

	// constructor
	// $config has to be an object of Config (../config/config.php)
	function Ldap($config) {
		setlanguage();
		if (is_object($config)) $this->conf = $config;
		else return false;
		// construct arrays with known LDAP attributes
		$this->ldapUserAttributes = array (
			"uid" => _("User ID"),
			"uidnumber" => _("UID number"),
			"gidnumber" => _("GID number"),
			"cn" => _("Username"),
			"host" => _("Allowed hosts"),
			"givenname" => _("First name"),
			"sn" => _("Last name"),
			"homedirectory" => _("Home directory"),
			"loginshell" => _("Login shell"),
			"mail" => _("E-Mail"),
			"gecos" => _("Description")
			);
		$this->ldapGroupAttributes = array (
			"cn" => _("Group name"),
			"gidnumber" => _("GID number"),
			"memberuid" => _("Group members"),
			"member" => _("Group member DNs"),
			"description" => _("Group description")
			);
		$this->ldapHostAttributes = array (
			"uid" => _("Host username"),
			"cn" => _("Host name"),
			"rid" => _("RID (Windows UID)"),
			"description" => _("Host description")
			);
		return true;
	}

	// connects to the server using the given username and password
	// $base is optional and specifies the root from where to search for entries
	// if connect succeeds the server handle is returned
	function connect($user, $passwd) {
		// close any prior connection
		@$this->close();
		// do not allow anonymous bind
		if ((!$user)||($user == "")||(!$passwd)) {
			return false;
		}
		// save password und username encrypted
		$this->encrypt($user, $passwd);
		$this->server = @ldap_connect($this->conf->get_ServerURL());
		if ($this->server) {
			// use LDAPv3
			ldap_set_option($this->server, LDAP_OPT_PROTOCOL_VERSION, 3);
			// start TLS if possible
			if (function_exists('ldap_start_tls')) {
				@ldap_start_tls($this->server);
				// connect without TLS if it failed
				if (ldap_errno($this->server) > 0) {
					@ldap_close($this->server);
					$this->server = @ldap_connect($this->conf->get_ServerURL());
					ldap_set_option($this->server, LDAP_OPT_PROTOCOL_VERSION, 3);
				}
			}
			//echo ldap_error($this->server) . " | " . ldap_errno($this->server);
			$bind = @ldap_bind($this->server, $user, $passwd);
			if ($bind) {
				// return server handle
				return $this->server;
			}
		}
	}

	// closes connection to server
	function close() {
		@ldap_close($this->server);
	}

	// searches LDAP for a specific user name
	// and returns its DN entry
	function search_username($name) {
		if ($this->conf->get_samba3() == "yes") {
			// users have the attribute "posixAccount" or "sambaSamAccount" and uid $name
			$filter = "(&(objectClass=sambaSamAccount) (uid=$name))";
		}
		else {
			// users have the attribute "posixAccount" or "sambaAccount" and uid $name
			$filter = "(&(objectClass=sambaAccount) (uid=$name))";
		}
		$attrs = array();
		$sr = @ldap_search($this->server, $this->conf->get_UserSuffix(), $filter, $attrs);
		if ($sr) {
			$info = ldap_get_entries($this->server, $sr);
			// return only first DN entry
			$ret = $info[0]["dn"];
			ldap_free_result($sr);
			return $ret;
		}
	}

	// returns an array with all organizational units under the given suffix
	function search_units($suffix) {
		$ret = array();
		$sr = @ldap_search($_SESSION["ldap"]->server(), $suffix, "objectClass=organizationalunit", array("DN"));
		if ($sr) {
			$units = ldap_get_entries($_SESSION["ldap"]->server, $sr);
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

	// returns an array with all Samba 3 domain entries under the given suffix
	function search_domains($suffix) {
		$ret = array();
		$attr = array("DN", "sambaDomainName", "sambaSID", "sambaNextRid", "sambaNextGroupRid",
			"sambaNextUserRid", "sambaAlgorithmicRidBase");
		$sr = @ldap_search($_SESSION["ldap"]->server(), $suffix, "objectClass=sambaDomain", $attr);
		if ($sr) {
			$units = ldap_get_entries($_SESSION["ldap"]->server, $sr);
			// delete count entry
			array_shift($units);
			// extract attributes
			for ($i = 0; $i < sizeof($units); $i++) {
				$ret[$i] = new samba3domain();
				$ret[$i]->dn = $units[$i]['dn'];
				$ret[$i]->name = $units[$i]['sambadomainname'][0];
				$ret[$i]->SID = $units[$i]['sambasid'][0];
				$ret[$i]->nextRID = $units[$i]['sambanextrid'][0];
				$ret[$i]->nextGroupRID = $units[$i]['sambanextgrouprid'][0];
				$ret[$i]->nextUserRID = $units[$i]['sambanextuserrid'][0];
				if (isset($units[$i]['sambaalgorithmicridbase'][0])) $ret[$i]->RIDbase = $units[$i]['sambaalgorithmicridbase'][0];
			}
			// sort array by domain name
			usort($ret, array($this,"cmp_domain"));
		}
		return $ret;
	}

	// returns the LDAP connection handle
	function server() {
		return $this->server;
	}

	// closes connection to LDAP server before serialization
	function __sleep() {
		$this->close();
		// define which attributes to save
		return array("conf", "username", "password", "ldapUserAttributes", "ldapGroupAttributes", "ldapHostAttributes");
	}

	// reconnects to LDAP server when deserialized
	function __wakeup() {
		$data = $this->decrypt();
		$this->connect($data[0], $data[1]);
	}

	// encrypts username and password
	function encrypt($username, $password) {
		// read key and iv from cookie
		$iv = base64_decode($_COOKIE["IV"]);
		$key = base64_decode($_COOKIE["Key"]);
		// encrypt username and password
		$this->username = base64_encode(mcrypt_encrypt(MCRYPT_RIJNDAEL_256, $key, $username, MCRYPT_MODE_ECB, $iv));
		$this->password = base64_encode(mcrypt_encrypt(MCRYPT_RIJNDAEL_256, $key, $password, MCRYPT_MODE_ECB, $iv));
	}

	// decrypts username and password
	function decrypt() {
		// read key and iv from cookie
		$iv = base64_decode($_COOKIE["IV"]);
		$key = base64_decode($_COOKIE["Key"]);
		// decrypt username and password
		$username = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($this->username), MCRYPT_MODE_ECB, $iv);
		$password = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($this->password), MCRYPT_MODE_ECB, $iv);
		$ret = array($username, $password);
		$ret[0] = str_replace(chr(00), "", $ret[0]);
		$ret[1] = str_replace(chr(00), "", $ret[1]);
		return $ret;
	}

	// closes connection to LDAP server and deletes encrypted username/password
	function destroy() {
		$this->close();
		$this->username="xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
		$this->password="xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
	}

	// returns an array that contains LDAP attribute names and their description
	function attributeUserArray() {
		return $this->ldapUserAttributes;
	}

	// returns an array that contains LDAP attribute names and their description
	function attributeGroupArray() {
		return $this->ldapGroupAttributes;
	}

	// returns an array that contains LDAP attribute names and their description
	function attributeHostArray() {
		return $this->ldapHostAttributes;
	}


	// helper function to sort the unit DNs
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
	}

	// helper function to sort the domains
	function cmp_domain($a, $b) {
		if ($a->name == $b->name) return 0;
		elseif ($a->name == max($a->name, $b->name)) return 1;
		else return -1;
	}


}

// represents a Samba 3 domain entry
class samba3domain {

	// DN
	var $dn;

	// domain name
	var $name;

	// domain SID
	var $SID;

	// next RID
	var $nextRID;

	// next user RID
	var $nextUserRID;

	// next group RID
	var $nextGroupRID;

	// RID base to calculate RIDs, default 1000
	var $RIDbase=1000;
}

?>
