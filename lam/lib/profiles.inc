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

// profiles.inc provides functions to load and save profiles for users/groups/hosts

include_once("config.inc");


// returns an array of String with all available user profiles
function getUserProfiles() {
	$dir = dir(getLAMPath() . "/config/profiles/users");
	$ret = array();
	$pos = 0;
	while ($entry = $dir->read()){
		$ext = substr($entry, strlen($entry)-4, 4);
		$name = substr($entry, 0, strlen($entry)-4);
		if ($ext == ".pru") {
			$ret[$pos] = $name;
			$pos ++;
		}
	}
	return $ret;
}

// returns an array of String with all available group profiles
function getGroupProfiles() {
	$dir = dir(getLAMPath() . "/config/profiles/groups");
	$ret = array();
	$pos = 0;
	while ($entry = $dir->read()){
		$ext = substr($entry, strlen($entry)-4, 4);
		$name = substr($entry, 0, strlen($entry)-4);
		if ($ext == ".prg") {
			$ret[$pos] = $name;
			$pos ++;
		}
	}
	return $ret;
}

// returns an array of String with all available host profiles
function getHostProfiles() {
	$dir = dir(getLAMPath() . "/config/profiles/hosts");
	$ret = array();
	$pos = 0;
	while ($entry = $dir->read()){
		$ext = substr($entry, strlen($entry)-4, 4);
		$name = substr($entry, 0, strlen($entry)-4);
		if ($ext == ".prh") {
			$ret[$pos] = $name;
			$pos ++;
		}
	}
	return $ret;
}

// loads an user profile with name $profile
// the return value is an account object
function loadUserProfile($profile) {
}

// loads an group profile with name $profile
// the return value is an account object
function loadGroupProfile($profile) {
}

// loads an host profile with name $profile
// the return value is an account object
function loadHostProfile($profile) {
}

// saves an account object to an user profile with name $profile
// file is created, if needed
function saveUserProfile($account, $profile) {
}

// saves an account object to an group profile with name $profile
// file is created, if needed
function saveGroupProfile($account, $profile) {
}

// saves an account object to an host profile with name $profile
// file is created, if needed
function saveHostProfile($account, $profile) {
}

?>
