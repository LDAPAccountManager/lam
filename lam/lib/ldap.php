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

class Ldap{

	// object of Config to access preferences
	var $conf;
	
	// server handle
	var $server;

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
		if ($this->conf->get_SSL() == "True") $this->server = ldap_connect("ldaps://" . $this->conf->get_Host(), $this->conf->get_Port());
		else $this->server = ldap_connect("ldap://" . $this->conf->get_Host(), $this->conf->get_Port());
		if ($this->server) {
			$bind = ldap_bind($this->server, $user, $passwd);
			if ($bind) {
			// return server handle
			return $this->server;
			}
			else {
				echo _("Unable to bind to Server!");
				exit;
			}
		}
		else {
			echo _("Unable to connect to Server!");
			exit;
		}
	}
	
	// closes connection to server
	function close() {
		ldap_close($this->server);
	}
	
	// returns the LDAP connection handle
	function server() {
		return $this->server;
	}
	
}


?>
 
