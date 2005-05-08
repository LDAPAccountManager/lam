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
* This file provides functions to load and save account profiles.
*
* @package profiles
* @author Roland Gruber
*/


/**
* Returns an array of string with all available profiles for the given account type
*
* @param string $scope account type
* @return array profile names
*/
function getAccountProfiles($scope) {
	$dir = @dir(substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles");
	$ret = array();
	$pos = 0;
	if ($dir) {
		$entry = $dir->read();
		while ($entry){
			// check if filename ends with .<scope>
			if (strrpos($entry, '.')) {
				$pos = strrpos($entry, '.');
				if (substr($entry, $pos + 1) == $scope) {
					$name = substr($entry, 0, $pos);
					$ret[] = $name;
				}
			}
			$entry = $dir->read();
		}
	}
	return $ret;
}

/**
* Loads an profile of the given account type
*
* @param string $profile name of the profile (without .<scope> extension)
* @param string $scope account type
* @return array hash array (attribute => value)
*/
function loadAccountProfile($profile, $scope) {
	if (!eregi("^[0-9a-z_-]+$", $profile) || !eregi("^[a-z]+$", $scope)) return false;
	$settings = array();
	$file = substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/" . $profile . "." . $scope;
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
* Saves an hash array (attribute => value) to an account profile
*
* file is created, if needed
*
* @param string $profile name of the account profile (without .<scope> extension)
* @param array $attributes hash array (attribute => value)
* @param string $scope account type
* @return boolean true, if saving succeeded
*/
function saveAccountProfile($attributes, $profile, $scope) {
	if (!$_SESSION['loggedIn'] == true) return false;
	// check profile name
	if (!eregi("^[0-9a-z_-]+$", $profile) || !eregi("^[a-z]+$", $scope)) return false;
	if (!is_array($attributes)) {
		return false;
	}
	$path = substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/" . $profile . "." . $scope;
	$file = @fopen($path, "w");
	if ($file) {
  	// write attributes
		$keys = array_keys($attributes);
		for ($i = 0; $i < sizeof($keys); $i++) {
			if (isset($attributes[$keys[$i]])) {
				$line = $keys[$i] . ": " . implode("+::+", $attributes[$keys[$i]]) . "\n";
			}
			else {
				$line = $keys[$i] . ": \n";
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
* Deletes an account profile
*
* @param string $file name of profile (Without .<scope> extension)
* @param string $scope account type
* @return boolean true if profile was deleted
*/
function delAccountProfile($file, $scope) {
	if (!$_SESSION['loggedIn'] == true) return false;
	if (!eregi("^[0-9a-z\\-_]+$", $file) || !eregi("^[0-9a-z\\-_]+$", $scope)) return false;
	$prof = substr(__FILE__, 0, strlen(__FILE__) - 16) . "config/profiles/" . $file . "." . $scope;
	if (is_file($prof)) {
		return @unlink($prof);
	}
	else return false;
}


?>
