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

// Ldap provides basic functions to connect to the OpenLDAP server and get lists of users and groups.
include_once("../config/config.php");

class Ldap{

	// object of Config to access preferences
	var $conf;

	// constructor
	// $config has to be an object of Config (../config/config.php)
	function Ldap($config) {
		if (is_object($config)) $this->conf = $config;
		else { echo _("Ldap->Ldap failed!"); exit;}
	}

	// returns an array of strings with the DN entries
	// $base is optional and specifies the root from where to search for entries
	function getUsers($base = "") {
	}

	// returns an array of strings with the DN entries
	// $base is optional and specifies the root from where to search for entries
	function getGroups($base = "") {
	}
	
	// returns an array of strings with the DN entries
	// $base is optional and specifies the root from where to search for entries
	function getMachines($base = "") {
	}

	// connects to the server using the given username and password
	// $base is optional and specifies the root from where to search for entries
	function connect($user, $passwd) {
	}
	
	// closes connection to server
	// $base is optional and specifies the root from where to search for entries
	function close() {
	}
	
}


?>
 
