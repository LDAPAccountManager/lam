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
include_once("ldap.inc");


// returns an array of String with all available user profiles (without .pru)
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

// returns an array of String with all available group profiles (without .prg)
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

// returns an array of String with all available host profiles (without .prh)
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

// loads an user profile
// $profile: name of the profile (without .pru)
// the return value is an hash array (attribute => value)
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

// loads an group profile
// $profile: name of the group profile (without .prg)
// the return value is an hash array (attribute => value)
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

// loads an host profile
// $profile: name of the host profile (without .prh)
// the return value is an hash array (attribute => value)
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

// saves an hash array (attribute => value) to an user profile
// file is created, if needed
// $profile: name of the user profile (without .pru)
// $attributes: hash array(attribute => value)
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
			$line = $keys[$i] . ": " . implode("+::+", $attributes[$keys[$i]]) . "\n";
			fputs($file, $line);
		}
		// close file
		fclose($file);
	}
	else {
		StatusMessage("ERROR", _("Unable to save profile!"), $path);
		return false;
	}
	return true;
}

// saves an hash array (attribute => value) to an group profile
// file is created, if needed
// $profile: name of the group profile (without .prg)
// $attributes: hash array(attribute => value)
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
		StatusMessage("ERROR", _("Unable to save profile!"), $path);
		return false;
	}
	return true;
}

// saves an hash array (attribute => value) to an host profile
// file is created, if needed
// $profile: name of the host profile (without .prh)
// $attributes: hash array(attribute => value)
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
		StatusMessage("ERROR", _("Unable to save profile!"), $path);
		return false;
	}
	return true;
}

// deletes a user profile
function delUserProfile($file) {
	if (!$_SESSION['loggedIn'] == true) return false;
	if (!eregi("^[0-9a-z\\-_]+$", $file)) return false;
	$prof = substr(__FILE__, 0, strlen(__FILE__) - 16) . "config/profiles/users/".$file.".pru";
	if (is_file($prof)) {
		return @unlink($prof);
	}
}

// deletes a group profile
function delGroupProfile($file) {
	if (!$_SESSION['loggedIn'] == true) return false;
	if (!eregi("^[0-9a-z\\-_]+$", $file)) return false;
	$prof = substr(__FILE__, 0, strlen(__FILE__) - 16) . "config/profiles/groups/".$file.".prg";
	if (is_file($prof)) {
		return @unlink($prof);
	}
}

// deletes a host profile
function delHostProfile($file) {
	if (!$_SESSION['loggedIn'] == true) return false;
	if (!eregi("^[0-9a-z\\-_]+$", $file)) return false;
	$prof = substr(__FILE__, 0, strlen(__FILE__) - 16) . "config/profiles/hosts/".$file.".prh";
	if (is_file($prof)) {
		return @unlink($prof);
	}
}

?>
