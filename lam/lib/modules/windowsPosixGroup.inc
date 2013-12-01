<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2013  Roland Gruber

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
* Manages Unix groups in Windows LDAP schema.
*
* @package modules
* @author Roland Gruber
*/

/** include parent class */
include_once("posixGroup.inc");

/**
* Manages Unix groups in Windows LDAP schema.
*
* @package modules
*/
class windowsPosixGroup extends posixGroup {
	
	/**
	* Creates a new windowsPosixGroup object.
	*
	* @param string $scope account type (user, group, host)
	*/
	public function __construct($scope) {
		// call parent constructor
		parent::__construct($scope);
		// do not manage cn and description (managed by windowsGroup)
		$this->manageCnAttribute = false;
		$this->manageDescriptionAttribute = false;
		// make optional
		$this->autoAddObjectClasses = false;
	}
	
	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	* 
	* @see baseModule::get_metaData()
	*/
	function get_metaData() {
		$return = parent::get_metaData();
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array('posixGroup'));
		// this is no base module (in contrast to parent class)
		$return["is_base"] = false;
		// no RDN attribute setting
		$return["RDN"] = array();
		// managed attributes
		$return['attributes'] = array('gidNumber', 'userPasswordUnix', 'memberUid');
		return $return;
	}

	/**
	* Controls if the module button the account page is visible and activated.
	*
	* @return string status ("enabled", "disabled", "hidden")
	*/
	function getButtonStatus() {
		return "enabled";
	}

	/**
	* This functions is used to check if all settings for this module have been made.
	*
	* @return boolean true, if settings are complete
	*/
	function module_complete() {
		return true;
	}

}

?>