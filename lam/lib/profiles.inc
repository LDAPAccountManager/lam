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
* This file provides functions to load and save account profiles for users/groups/hosts.
*
* @package configuration
* @author Roland Gruber
*/


/**
* Returns an array of string with all available user profiles (without .pru)
*
* @return array profile names
*/
function getUserProfiles() {
	$dir = @dir(substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/users");
	$ret = array();
	$pos = 0;
	if ($dir) {
		while ($entry = $dir->read()){
			$ext = substr($entry, strlen($entry)-4, 4);
			$name = substr($entry, 0, strlen($entry)-4);
			if ($ext == ".pru") {
				$ret[$pos] = $name;
				$pos ++;
			}
		}
	}
	return $ret;
}

/**
* Returns an array of String with all available group profiles (without .prg)
*
* @return array profile names
*/
function getGroupProfiles() {
	$dir = @dir(substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/groups");
	$ret = array();
	$pos = 0;
	if ($dir) {
		while ($entry = $dir->read()){
			$ext = substr($entry, strlen($entry)-4, 4);
			$name = substr($entry, 0, strlen($entry)-4);
			if ($ext == ".prg") {
				$ret[$pos] = $name;
				$pos ++;
			}
		}
	}
	return $ret;
}

/**
* Returns an array of String with all available host profiles (without .prh)
*
* @return array profile names
*/
function getHostProfiles() {
	$dir = @dir(substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/hosts");
	$ret = array();
	$pos = 0;
	if ($dir) {
		while ($entry = $dir->read()){
			$ext = substr($entry, strlen($entry)-4, 4);
			$name = substr($entry, 0, strlen($entry)-4);
			if ($ext == ".prh") {
				$ret[$pos] = $name;
				$pos ++;
			}
		}
	}
	return $ret;
}

/**
* Loads an user profile
*
* @param string $profile name of the profile (without .pru)
* @return array hash array (attribute => value)
*/
function loadUserProfile($profile) {
	if (!eregi("^[0-9a-z_\\-]+$", $profile)) return false;
	$settings = array();
	$file = substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/users/" . $profile . ".pru";
	if (is_file($file) == True) {
		$file = @fopen($file, "r");
		if ($file) {
			while (!feof($file)) {
				$line = fgets($file, 1024);
				if (($line == "\n")||($line[0] == "#")) continue; // ignore comments
				// search keywords
				$parts = array();
				$parts = split(": ", $line);
				if (sizeof($parts) != 2) continue;  // ignore malformed settings
				else {
					$option = $parts[0];
					$value = $parts[1];
					// remove line ends
					$value = chop($value);
					$settings[$option] = explode("+::+", $value);
				}
			}
			fclose($file);
		}
		else {
			StatusMessage("ERROR", "", _("Unable to load profile!") . " " . $file);
		}
	}
	else {
		StatusMessage("ERROR", "", _("Unable to load profile!") . " " . $file);
	}
	return $settings;
}

/**
* Loads an group profile
*
* @param string $profile name of the profile (without .prg)
* @return array hash array (attribute => value)
*/
function loadGroupProfile($profile) {
	if (!eregi("^[0-9a-z_\\-]+$", $profile)) return false;
	$settings = array();
	$file = substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/groups/" . $profile . ".prg";
	if (is_file($file) == True) {
		$file = @fopen($file, "r");
		if ($file) {
			while (!feof($file)) {
				$line = fgets($file, 1024);
				if (($line == "\n")||($line[0] == "#")) continue; // ignore comments
				// search keywords
				$parts = array();
				$parts = split(": ", $line);
				if (sizeof($parts) != 2) continue;  // ignore malformed settings
				else {
					$option = $parts[0];
					$value = $parts[1];
					// remove line ends
					$value = chop($value);
					$settings[$option] = explode("+::+", $value);
				}
			}
			fclose($file);
		}
		else {
			StatusMessage("ERROR", "", _("Unable to load profile!") . " " . $file);
		}
	}
	else {
		StatusMessage("ERROR", "", _("Unable to load profile!") . " " . $file);
	}
	return $settings;
}

/**
* Loads an host profile
*
* @param string $profile name of the profile (without .prh)
* @return array hash array (attribute => value)
*/
function loadHostProfile($profile) {
	if (!eregi("^[0-9a-z_\\-]+$", $profile)) return false;
	$settings = array();
	$file = substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/hosts/" . $profile . ".prh";
	if (is_file($file) == True) {
		$file = @fopen($file, "r");
		if ($file) {
			while (!feof($file)) {
				$line = fgets($file, 1024);
				if (($line == "\n")||($line[0] == "#")) continue; // ignore comments
				// search keywords
				$parts = array();
				$parts = split(": ", $line);
				if (sizeof($parts) != 2) continue;  // ignore malformed settings
				else {
					$option = $parts[0];
					$value = $parts[1];
					// remove line ends
					$value = chop($value);
					$settings[$option] = explode("+::+", $value);
				}
			}
			fclose($file);
		}
		else {
			StatusMessage("ERROR", "", _("Unable to load profile!") . " " . $file);
		}
	}
	else {
		StatusMessage("ERROR", "", _("Unable to load profile!") . " " . $file);
	}
	return $settings;
}

/**
* Saves an hash array (attribute => value) to an user profile
*
* file is created, if needed
*
* @param string $profile name of the user profile (without .pru)
* @param array $attributes hash array (attribute => value)
* @return boolean true, if saving succeeded
*/
function saveUserProfile($attributes, $profile) {
	if (!$_SESSION['loggedIn'] == true) return false;
	// check profile name
	if (!eregi("^[0-9a-z_-]+$", $profile)) return false;
	if (!is_array($attributes)) {
		return false;
	}
	$path = substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/users/" . $profile . ".pru";
	$file = @fopen($path, "w");
	if ($file) {
  	// write attributes
		$keys = array_keys($attributes);
		for ($i = 0; $i < sizeof($keys); $i++) {
			if (isset($attributes[$keys[$i]])) {
				$line = $keys[$i] . ": " . implode("+::+", $attributes[$keys[$i]]) . "\n";
			}
			else {
				$line = $keys[$i] . ": ";
			}
			fputs($file, $line);
		}
		// close file
		fclose($file);
	}
	else {
		return false;
	}
	return true;
}

/**
* Saves an hash array (attribute => value) to an group profile
*
* file is created, if needed
*
* @param string $profile name of the group profile (without .prg)
* @param array $attributes hash array (attribute => value)
* @return boolean true, if saving succeeded
*/
function saveGroupProfile($attributes, $profile) {
	if (!$_SESSION['loggedIn'] == true) return false;
	// check profile name
	if (!eregi("^[0-9a-z_-]+$", $profile)) return false;
	if (!is_array($attributes)) {
		return false;
	}
	$path = substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/groups/" . $profile . ".prg";
	$file = @fopen($path, "w");
	if ($file) {
		// write attributes
		$keys = array_keys($attributes);
		for ($i = 0; $i < sizeof($keys); $i++) {
			$line = $keys[$i] . ": " . implode("+::+", $attributes[$keys[$i]]) . "\n";
			fputs($file, $line);
		}
		// close file
		fclose($file);
	}
	else {
		return false;
	}
	return true;
}

/**
* Saves an hash array (attribute => value) to an host profile
*
* file is created, if needed
*
* @param string $profile name of the host profile (without .prh)
* @param array $attributes hash array (attribute => value)
* @return boolean true, if saving succeeded
*/
function saveHostProfile($attributes, $profile) {
	if (!$_SESSION['loggedIn'] == true) return false;
	// check profile name
	if (!eregi("^[0-9a-z_-]+$", $profile)) return false;
	if (!is_array($attributes)) {
		return false;
	}
	$path = substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/hosts/" . $profile . ".prh";
	$file = @fopen($path, "w");
	if ($file) {
		// write attributes
		$keys = array_keys($attributes);
		for ($i = 0; $i < sizeof($keys); $i++) {
			$line = $keys[$i] . ": " . implode("+::+", $attributes[$keys[$i]]) . "\n";
			fputs($file, $line);
		}
		// close file
		fclose($file);
	}
	else {
		return false;
	}
	return true;
}

/**
* Deletes a user profile
*
* @param string $file name of profile (Without .pru)
*/
function delUserProfile($file) {
	if (!$_SESSION['loggedIn'] == true) return false;
	if (!eregi("^[0-9a-z\\-_]+$", $file)) return false;
	$prof = substr(__FILE__, 0, strlen(__FILE__) - 16) . "config/profiles/users/".$file.".pru";
	if (is_file($prof)) {
		return @unlink($prof);
	}
}

/**
* Deletes a group profile
*
* @param string $file name of profile (Without .prg)
*/
function delGroupProfile($file) {
	if (!$_SESSION['loggedIn'] == true) return false;
	if (!eregi("^[0-9a-z\\-_]+$", $file)) return false;
	$prof = substr(__FILE__, 0, strlen(__FILE__) - 16) . "config/profiles/groups/".$file.".prg";
	if (is_file($prof)) {
		return @unlink($prof);
	}
}

/**
* Deletes a host profile
*
* @param string $file name of profile (Without .prh)
*/
function delHostProfile($file) {
	if (!$_SESSION['loggedIn'] == true) return false;
	if (!eregi("^[0-9a-z\\-_]+$", $file)) return false;
	$prof = substr(__FILE__, 0, strlen(__FILE__) - 16) . "config/profiles/hosts/".$file.".prh";
	if (is_file($prof)) {
		return @unlink($prof);
	}
}

?>
