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
    if (is_object($config)) $this->conf = $config;
    else { echo _("Ldap->Ldap failed!"); exit;}
	// construct arrays with known LDAP attributes
	$this->ldapUserAttributes = array (
		"uid" => _("User ID"),
		"uidNumber" => _("UID Number"),
		"gidNumber" => _("GID Number"),
		"cn" => _("User Name"),
		"host" => _("Allowed Hosts"),
		"givenName" => _("First Name"),
		"sn" => _("Last Name"),
		"homeDirectory" => _("Home Directory"),
		"loginShell" => _("Login Shell"),
		"mail" => _("E-Mail"),
		"gecos" => _("description")
		);
	$this->ldapGroupAttributes = array (
		"cn" => _("Group Name"),
		"gidNumber" => _("GID Number"),
		"memberUID" => _("Group Members"),
		"member" => _("Group Member DNs"),
		"description" => _("Group Description")
		);
	$this->ldapHostAttributes = array (
		"uid" => _("Host Username"),
		"cn" => _("Host Name"),
		"rid" => _("RID (Windows UID)"),
		"description" => _("Host Description")
		);
  }

  // connects to the server using the given username and password
  // $base is optional and specifies the root from where to search for entries
  // if connect succeeds the server handle is returned
  function connect($user, $passwd) {
    // close any prior connection
    @$this->close();
    // do not allow anonymous bind
    if ((!$user)||($user == "")||(!$passwd)) {
		echo ("<html><head><title></title>");
		echo ("<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\"></head><body>");
		StatusMessage("ERROR", "", _("No username was specified or password is empty!"));
		echo ("<br><br><a href=\"login.php\">" . _("Back to Login") . "</a></body></html>");
		exit;
    }
	// save password und username encrypted
	$this->encrypt($user, $passwd);

	$this->server = @ldap_connect($this->conf->get_ServerURL());
    if ($this->server) {
      // use LDAPv3
      ldap_set_option($this->server, LDAP_OPT_PROTOCOL_VERSION, 3);
      $bind = @ldap_bind($this->server, $user, $passwd);
      if ($bind) {
	// return server handle
	return $this->server;
      }
    }
  }

  // closes connection to server
  function close() {
    ldap_close($this->server);
  }

  // searches LDAP for a specific user name
  // and returns its DN entry
  function search_username($name) {
	if ($this->conf->get_samba3() == "yes") {
		// users have the attribute "posixAccount" or "sambaSamAccount" and uid $name
		$filter = "(&(|(objectClass=posixAccount) (objectClass=sambaSamAccount)) (uid=$name))";
	}
	else {
		// users have the attribute "posixAccount" or "sambaAccount" and uid $name
		$filter = "(&(|(objectClass=posixAccount) (objectClass=sambaAccount)) (uid=$name))";
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
	$sr = @ldap_search($_SESSION["ldap"]->server(), $suffix, "objectClass=organizationalunit", array("DN"));
	if ($sr) {
		$units = ldap_get_entries($_SESSION["ldap"]->server, $sr);
		// delete first array entry which is "count"
		array_shift($units);
		// remove sub arrays
		for ($i = 0; $i < sizeof($units); $i++) $units[$i] = $units[$i]['dn'];
		// add root suffix if needed
		if (!in_array($suffix, $units)) {
			array_push($units, $suffix);
		}
	}
	return $units;
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

}


?>
