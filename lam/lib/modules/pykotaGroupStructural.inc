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
* Manages PyKota group accounts.
* This module is used when the structural object class pykotaObject should be used.
*
* @package modules
* @author Roland Gruber
*/

/** include parent class */
include_once("pykotaGroup.inc");

/**
* Manages PyKota group accounts.
*
* @package modules
*/
class pykotaGroupStructural extends pykotaGroup {
	
	/**
	 * Returns if this module also manages the structural object class pykotaObject.
	 * This is overridden by a submodule that must provide the structural object class.
	 * 
	 * @return boolean structural usage
	 */
	public function isStructural() {
		return true;
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
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array('pykotaGroup'));
		// additional object class and attribute
		$return['objectClasses'][] = 'pykotaObject';
		// RDN attribute
		$return["RDN"] = array("cn" => "normal");
		return $return;
	}

}
