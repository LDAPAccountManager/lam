<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003 - 2004  Roland Gruber

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
* This is the parent class for all account modules.
*
* It implements the complete module interface and uses meta-data
* provided by the account modules for its functions.
*
* @package modules
* @author Roland Gruber
*/

/**
* Parent class of all account modules
*
* @package modules
*/
class baseModule {

	/** includes all meta data provided by the sub class */
	var $meta;

	/** the account type of this module (user, group, host) */
	var $scope;

	/**
	* Creates a new base module class
	*
	* @param string $scope the account type (user, group, host)
	*/
	function baseModule($scope) {
		$this->scope = $scope;
		$this->meta = $this->get_metaData();
	}

	/**
	* Dummy function, meta data is provided by sub classes.
	*
	* @return array empty array
	*/
	function get_metaData() {
		return array();
	}

	/**
	* Returns the account type of this module (user, group, host)
	*
	* @return string account type
	*/
	function get_scope() {
		return $this->scope;
	}

	/**
	* Returns true if this module fits for the current scope.
	*
	* @return boolean true if module fits
	*/
	function can_manage() {
		if (is_array($this->meta["account_types"]) && in_array($this->scope, $this->meta["account_types"])) return true;
		else return false;
	}

	/**
	* Returns true if this module is enough to provide a sensible account.
	*
	* There is no relation to the name of this class.
	*
	* @return boolean true if base module
	*/
	function is_base_module() {
		if ($this->meta['is_base'] == true) return true;
		else return false;
	}

	/**
	* returns an LDAP filter for the account lists
	*
	* @return string LDAP filter
	*/
	function get_ldap_filter() {
		if (isset($this->meta['ldap_filter'])) return $this->meta['ldap_filter'];
		else return "";
	}

// TODO implement missing interface
}



?>