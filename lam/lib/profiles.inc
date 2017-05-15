<?php
namespace LAM\PROFILES;
use \LAMException;
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2017  Roland Gruber

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
* @param string $typeId account type
* @param string $profile server profile name
* @return array profile names
*/
function getAccountProfiles($typeId, $profile = null) {
	if (!isset($profile)) {
		$profile = $_SESSION['config']->getName();
	}

	$dir = @dir(dirname(__FILE__) . "/../config/profiles/" . $profile);

	$ret = array();
	$pos = 0;
	if ($dir) {
		$entry = $dir->read();
		while ($entry){
			// check if filename ends with .<typeId>
			if (strrpos($entry, '.')) {
				$pos = strrpos($entry, '.');
				if (substr($entry, $pos + 1) == $typeId) {
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
* @param string $typeId account type
* @return array hash array (attribute => value)
*/
function loadAccountProfile($profile, $typeId) {
	$typeManager = new \LAM\TYPES\TypeManager();
	$type = $typeManager->getConfiguredType($typeId);
	if (!isValidProfileName($profile) || !preg_match("/^[a-z0-9_]+$/i", $typeId) || ($type == null)) {
		return false;
	}
	$settings = array();
	$file = substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/"  . $_SESSION['config']->getName() . '/' . $profile . "." . $typeId;
	if (is_file($file) == True) {
		$file = @fopen($file, "r");
		if ($file) {
			while (!feof($file)) {
				$line = fgets($file, 1024);
				if (($line == "\n")||($line[0] == "#")) continue; // ignore comments
				// search keywords
				$parts = array();
				$parts = explode(": ", $line);
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
* @param array $attributes hash array (attribute => value)
* @param string $profile name of the account profile (without .<scope> extension)
* @param string $typeId account type
* @return boolean true, if saving succeeded
*/
function saveAccountProfile($attributes, $profile, $typeId) {
	if (!isLoggedIn()) return false;
	// check profile name and type id
	$typeManager = new \LAM\TYPES\TypeManager();
	$type = $typeManager->getConfiguredType($typeId);
	if (!isValidProfileName($profile) || !preg_match("/^[a-z0-9_]+$/i", $typeId) || ($type == null)) {
		return false;
	}
	if (!is_array($attributes)) {
		return false;
	}
	$path = substr(__FILE__, 0, strlen(__FILE__) - 17) . "/config/profiles/" . $_SESSION['config']->getName() . '/' . $profile . "." . $typeId;
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
* @param string $typeId account type
* @return boolean true if profile was deleted
*/
function delAccountProfile($file, $typeId) {
	if (!isLoggedIn()) {
		return false;
	}
	$typeManager = new \LAM\TYPES\TypeManager();
	$type = $typeManager->getConfiguredType($typeId);
	if (!isValidProfileName($file) || !preg_match("/^[a-z0-9_]+$/i", $typeId) || ($type == null)) {
		return false;
	}
	$prof = substr(__FILE__, 0, strlen(__FILE__) - 16) . "config/profiles/". $_SESSION['config']->getName() . '/' . $file . "." . $typeId;
	if (is_file($prof)) {
		return @unlink($prof);
	}
	else return false;
}

/**
 * Returns if the given profile name is valid.
 *
 * @param string $name profile name
 */
function isValidProfileName($name) {
	return preg_match("/^[0-9a-z _-]+$/i", $name);
}

/**
 * Copies an account profile from the given source to target.
 *
 * @param \LAM\TYPES\ConfiguredType $sourceType source type
 * @param string $sourceProfileName profile name
 * @param \LAM\TYPES\ConfiguredType $targetType target type
 * @throws Exception
 */
function copyAccountProfile($sourceType, $sourceProfileName, $targetType) {
	if (!isValidProfileName($sourceProfileName)) {
		throw new LAMException(_('Failed to copy'));
	}
	$sourceConfig = $sourceType->getTypeManager()->getConfig()->getName();
	$sourceTypeId = $sourceType->getId();
	$targetConfig = $targetType->getTypeManager()->getConfig()->getName();
	$targetTypeId = $targetType->getId();
	$profilePath = dirname(__FILE__) . '/../config/profiles/';
	$src = $profilePath . $sourceConfig . '/' . $sourceProfileName . '.' . $sourceTypeId;
	$dst = $profilePath . $targetConfig . '/' . $sourceProfileName . '.' . $targetTypeId;
	if (!@copy($src, $dst)) {
		throw new LAMException(_('Failed to copy'), $sourceConfig . ': ' . $sourceProfileName);
	}
}

/**
 * Copies an account profile from the given source to global templates.
 *
 * @param \LAM\TYPES\ConfiguredType $sourceType source type
 * @param string $sourceProfileName profile name
 * @throws Exception
 */
function copyAccountProfileToTemplates($sourceType, $sourceProfileName) {
	if (!isValidProfileName($sourceProfileName)) {
		throw new LAMException(_('Failed to copy'));
	}
	$sourceConfig = $sourceType->getTypeManager()->getConfig()->getName();
	$sourceTypeId = $sourceType->getId();
	$profilePath = dirname(__FILE__) . '/../config/profiles/';
	$templatePath = dirname(__FILE__) . '/../config/templates/profiles/';
	$src = $profilePath . $sourceConfig . '/' . $sourceProfileName . '.' . $sourceTypeId;
	$dst = $templatePath . $sourceProfileName . '.' . $sourceType->getScope();
	if (!@copy($src, $dst)) {
		throw new LAMException(_('Failed to copy'), $sourceConfig . ': ' . $sourceProfileName);
	}
}

/**
 * Installs template profiles to the current server profile.
 */
function installProfileTemplates() {
	$templatePath = dirname(__FILE__) . '/../config/templates/profiles';
	$templateDir = @dir($templatePath);
	$allTemplates = array();
	if ($templateDir) {
		$entry = $templateDir->read();
		while ($entry){
			$parts = explode('.', $entry);
			if ((strlen($entry) > 3) && (sizeof($parts) == 2)) {
				$name = $parts[0];
				$scope = $parts[1];
				$allTemplates[$scope][] = $name;
			}
			$entry = $templateDir->read();
		}
	}
	$basePath = dirname(__FILE__) . '/../config/profiles/' . $_SESSION['config']->getName();
	if (!file_exists($basePath)) {
		mkdir($basePath, 0700, true);
	}
	$typeManager = new \LAM\TYPES\TypeManager();
	foreach ($typeManager->getConfiguredTypes() as $type) {
		if (empty($allTemplates[$type->getScope()])) {
			continue;
		}
		foreach ($allTemplates[$type->getScope()] as $templateName) {
			$path = $basePath . '/' . $templateName . '.' . $type->getId();
			if (!is_file($path)) {
				$template = $templatePath . '/' . $templateName . '.' . $type->getScope();
				logNewMessage(LOG_DEBUG, 'Copy template ' . $template . ' to ' . $path);
				@copy($template, $path);
			}
		}
	}
}

?>
