<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003-04  Roland Gruber

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


	Interface between modules and other parts of LAM.

*/

// returns an hash array (module => options) describing the options for an user profile
// the options follow the format specified in modules.spec (TODO filename)
function getUserProfileOptions() {
	return array();
}

// returns an hash array (module => options) describing the options for an group profile
// the options follow the format specified in modules.spec (TODO filename)
function getGroupProfileOptions() {
	return array();
}

// returns an hash array (module => options) describing the options for an host profile
// the options follow the format specified in modules.spec (TODO filename)
function getHostProfileOptions() {
	return array();
}

// checks if the user profile values are correct
// $options is a hash array (attribute => value) with the entered values
// the values are all arrays with one or more elements
// returns an hash array containing the error messages, if any
// format of hash: attribute => array(message type, message header, message body, replacement array)
function checkUserProfileOptions($options) {
}

// checks if the group profile values are correct
// $options is a hash array (attribute => value) with the entered values
// the values are all arrays with one or more elements
// returns an hash array containing the error messages, if any
// format of hash: attribute => array(message type, message header, message body, replacement array)
function checkGroupProfileOptions($options) {
}

// checks if the host profile values are correct
// $options is a hash array (attribute => value) with the entered values
// the values are all arrays with one or more elements
// returns an hash array containing the error messages, if any
// format of hash: attribute => array(message type, message header, message body, replacement array)
function checkHostProfileOptions($options) {
}

// returns the alias name of a module
function getModuleAlias($name) {
	return $name;
}

// returns a hash array (module name => dependencies) of all user module dependencies
// dependencies contains an array with two sub arrays: depends, conflicts
// the elements of depends/conflicts are either module names or an array of module names (OR-case)
function getUserModuleDependencies() {
	return array("um1" => array("depends" => "um4"),
		"um2" => array("depends" => "um4", "conflicts" => "um1"),
		"um3" => array("conflicts" => "um1"),
		"um4" => array());
}

// returns a hash array (module name => dependencies) of all group module dependencies
// dependencies contains an array with two sub arrays: depends, conflicts
// the elements of depends/conflicts are either module names or an array of module names (OR-case)
function getGroupModuleDependencies() {
	return array("gm1" => array("depends" => "gm4"),
		"gm2" => array("depends" => "gm4", "conflicts" => "gm1"),
		"gm3" => array("conflicts" => "gm1"),
		"gm4" => array());
}

// returns a hash array (module name => dependencies) of all host module dependencies
// dependencies contains an array with two sub arrays: depends, conflicts
// the elements of depends/conflicts are either module names or an array of module names (OR-case)
function getHostModuleDependencies() {
	return array("hm1" => array("depends" => "hm4"),
		"hm2" => array("depends" => "hm4", "conflicts" => "hm1"),
		"hm3" => array("conflicts" => "hm1"),
		"hm4" => array());
}

// returns an array with all available user module names
function getAvailableUserModules() {
	return array("um1", "um2", "um3", "um4");
}

// returns an array with all available group module names
function getAvailableGroupModules() {
	return array("gm1", "gm2", "gm3", "gm4");
}

// returns an array with all available host module names
function getAvailableHostModules() {
	return array("hm1", "hm2", "hm3", "hm4");
}


?>
