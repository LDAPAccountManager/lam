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

// ldap.php provides basic functions to connect to the OpenLDAP server and get lists of users and groups.
include_once("../config/config.php");

// class representing local user entry with attributes of ldap user entry
include_once("userentry.php");

class Ldap{

  // object of Config to access preferences
  var $conf;
	
  // server handle
  var $server;

  // LDAP username and password used for bind
  var $username;
  var $password;
  
  // constructor
  // $config has to be an object of Config (../config/config.php)
  function Ldap($config) {
    if (is_object($config)) $this->conf = $config;
    else { echo _("Ldap->Ldap failed!"); exit;}
  }

  // returns an array of strings with the DN entries of all users
  // $base is optional and specifies the root from where to search for entries
  function getUsers($base = "") {
    if ($base == "") $base = $this->conf->get_UserSuffix();
    // users have the attribute "posixAccount" or "sambaAccount" and do not end with "$"
    $filter = "(&(|(objectClass=posixAccount) (objectClass=sambaAccount)) (!(uid=*$)))";
    $attrs = array();
    $sr = ldap_search($this->server, $base, $filter, $attrs);
    $info = ldap_get_entries($this->server, $sr);
    $ret = array();
    for ($i = 0; $i < $info["count"]; $i++) $ret[$i] = $info[$i]["dn"];
    ldap_free_result($sr);
    return $ret;
  }

  // returns an array of strings with the DN entries of all groups
  // $base is optional and specifies the root from where to search for entries
  function getGroups($base = "") {
    if ($base == "") $base = $this->conf->get_GroupSuffix(); 
    // groups have the attribute "posixGroup"
    $filter = "(objectClass=posixGroup)";
    $attrs = array();
    $sr = ldap_search($this->server, $base, $filter, $attrs);
    $info = ldap_get_entries($this->server, $sr);
    $ret = array();
    for ($i = 0; $i < $info["count"]; $i++) $ret[$i] = $info[$i]["dn"];
    ldap_free_result($sr);
    return $ret;
  }
	
  // returns an array of strings with the DN entries of all Samba hosts
  // $base is optional and specifies the root from where to search for entries
  function getMachines($base = "") {
    if ($base == "") $base = $this->conf->get_HostSuffix();
    // Samba hosts have the attribute "sambaAccount" and end with "$"
    $filter = "(&(objectClass=sambaAccount) (uid=*$))";
    $attrs = array();
    $sr = ldap_search($this->server, $base, $filter, $attrs);
    $info = ldap_get_entries($this->server, $sr);
    $ret = array();
    for ($i = 0; $i < $info["count"]; $i++) $ret[$i] = $info[$i]["dn"];
    ldap_free_result($sr);
    return $ret;
  }

  // connects to the server using the given username and password
  // $base is optional and specifies the root from where to search for entries
  // if connect succeeds the server handle is returned
  function connect($user, $passwd) {
    // close any prior connection
    @$this->close();
    // do not allow anonymous bind
    if ((!$user)||($user == "")) {
      echo _("No username was specified!");
      exit;
    }
	// save password und username encrypted
	$this->encrypt($user, $passwd);
    if ($this->conf->get_SSL() == "True") $this->server = @ldap_connect("ldaps://" . $this->conf->get_Host(), $this->conf->get_Port());
    else $this->server = @ldap_connect("ldap://" . $this->conf->get_Host(), $this->conf->get_Port());
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


  /** 
   * @brief Populates any given object with the available attributes in the
   * LDAP. The names of the member variables of the object must correspond to
   * the attribute names in the LDAP server.
   * 
   * @param in_entry_dn distinguished name of entry in ldap
   * @param in_object input object to populate
   * 
   * @return populated object
   */
  function getEntry ($in_entry_dn, $in_object) {
    
    // read all variables of given object to $vararray
    $vararray = get_object_vars ($in_object);

    // set attributefilter only to attributes present in given object
    $attributefilter = array();
    foreach (array_keys ($vararray) as $varname)
      $attributefilter[] = $varname;

    // filter doesn't matter (we only read one entry)
    $filter = "(objectClass=*)";
    $resource = ldap_read ($this->server,
			   $in_entry_dn, $filter, $attributefilter);
    $entry = ldap_first_entry ($this->server, $resource);

    foreach (array_keys ($vararray) as $varname)
      $in_object->$varname = ldap_get_values ($this->server, $entry, $varname);

    return $in_object;
  }

	
  // closes connection to server
  function close() {
    ldap_close($this->server);
  }
	
  // returns the LDAP connection handle
  function server() {
    return $this->server;
  }
  
  // closes connection to LDAP server before serialization
  function __sleep() {
  	$this->close();
	return array("conf", "username", "password");
  }
  
  // reconnects to LDAP server when deserialized
  function __wakeup() {
  	$data = $this->decrypt();
  	$this->connect($data[0], $data[1]);
  }
  
  // encrypts username and password
  // TODO: implement encryption algorithm
  function encrypt($username, $password) {
	$this->username = $username;
	$this->password = $password;
  }

  // decrypts username and password
  // TODO: implement encryption algorithm
  function decrypt() {
  	$ret = array($this->username, $this->password);
	return $ret;
  }
  
  // closes connection to LDAP server and deletes encrypted username/password
  function destroy() {
  	$this->close();
	$this->username="xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
	$this->password="xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
	// TODO: delete encryption key
  }

}


?>
 
