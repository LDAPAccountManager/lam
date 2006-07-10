<?PHP
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2006  Roland Gruber

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
* Interface between modules and self service pages.
* This file also includes the self service profile class and helper functions.
* 
* @package selfService
* @author Roland Gruber
*/

/** modules */
include_once("modules.inc");
/** account types */
include_once("types.inc");


/**
 * Returns a list of possible search attributes for the self service.
 *
 * @param string $scope account type
 * @return array attributes
 */
function getSelfServiceSearchAttributes($scope) {
	$return = array();
	$modules = getAvailableModules($scope);
	for ($i = 0; $i < sizeof($modules); $i++) {
		$m = new $modules[$i]($scope);
		$attributes = $m->getSelfServiceSearchAttributes();
		$return = array_merge($return, $attributes);
	}
	return $return;
}


/**
* Returns a list of all available self service profiles (without .conf)
*
* @return array profile names (array(<account type> => array(<profile1>, <profile2>, ...)))
*/
function getSelfServiceProfiles() {
	$types = getTypes();
	$dir = dir(substr(__FILE__, 0, strlen(__FILE__) - 20) . "/config/selfService");
	$ret = array();
	while ($entry = $dir->read()){
		$ext = substr($entry, strrpos($entry, '.') + 1);
		$name = substr($entry, 0, strrpos($entry, '.'));
		// check if extension is right, add to profile list
		if (in_array($ext, $types)) {
			$ret[$ext][] = $name;
		}
	}
	ksort($ret);
	return $ret;
}


/**
* Loads all settings of a self service profile.
*
* @param string $name profile name
* @param string $scope account type
* @return selfServiceProfile true if file was readable
*/
function loadSelfServiceProfile($name, $scope) {
	if (!eregi("^[0-9a-z _-]+$", $name)) return false;
	if (!eregi("^[0-9a-z _-]+$", $scope)) return false;
	$profile = new selfServiceProfile();
	$settings = new selfServiceProfile();
	$file = substr(__FILE__, 0, strlen(__FILE__) - 20) . "/config/selfService/" . $name . "." . $scope;
	if (is_file($file) === True) {
		$file = @fopen($file, "r");
		if ($file) {
			$data = fread($file, 10000);
			$profile = unserialize($data);
			fclose($file);
		}
		else {
			StatusMessage("ERROR", "", _("Unable to load profile!") . " " . $file);
		}
	}
	else {
		StatusMessage("ERROR", "", _("Unable to load profile!") . " " . $file);
	}
	return $profile;
}


/**
* Saves a self service profile.
*
* File is created, if needed
*
* @param string $name name of the account profile
* @param string $scope account type
* @param selfServiceProfile $profile self service profile
* @return boolean true, if saving succeeded
*/
function saveSelfServiceProfile($name, $scope, $profile) {
	// check profile name
	if (!eregi("^[0-9a-z _-]+$", $scope)) return false;
	if (!eregi("^[0-9a-z _-]+$", $name)) return false;
	if (!get_class($profile) === 'selfServiceProfile') {
		return false;
	}
	$path = substr(__FILE__, 0, strlen(__FILE__) - 20) . "/config/selfService/" . $name . "." . $scope;
	$file = @fopen($path, "w");
	if ($file) {
	  	// write settings to file
		fputs($file, serialize($profile));
		// close file
		fclose($file);
	}
	else {
		return false;
	}
	return true;
}


/**
 * Includes all settings of a self service profile.
 *
 * @package selfService
 */
class selfServiceProfile {
	
	/** server address */
	var $serverURL;
	
	/** LDAP suffix */
	var $LDAPSuffix;
	
	/** LDAP search attribute */
	var $searchAttribute;
	
	/** describing text for user login */
	var $loginCaption;
	
	/** describing text for search attribute */
	var $loginAttributeText;
	
	/**
	 * Constructor
	 *
	 * @return selfServiceProfile
	 */
	function selfServiceProfile() {
		// set default values
		$this->serverURL = "localhost";
		$this->LDAPSuffix = "";
		$this->searchAttribute = "uid";
		$this->loginCaption = "Welcome to LAM self service. Please enter your user name and password.";
		$this->loginAttributeText = "User name";
	}
	
}

?>