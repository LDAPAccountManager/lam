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

	/**
	* Returns an alias name for the module.
	*
	* This alias is used in various places instead of the less descriptive class name.
	* The alias also has less syntax restrictions and may contain spaces or special characters.
	*
	* @return string alias name
	*/
	function get_alias() {
		if (isset($this->meta['alias'])) return $this->meta['alias'];
		else return get_class($this);
	}

	/**
	* This function returns a list with all depending and conflicting modules.
	*
	* @return array list of dependencies and conflicts
	*/
	function get_dependencies() {
		if (isset($this->meta['dependencies'])) return $this->meta['dependencies'];
		else return array('depends' => array(), 'conflicts' => array());
		}

	/**
	* Returns a list of elements for the account profiles.
	*
	* @return profile elements
	*/
	function get_profileOptions() {
		if (isset($this->meta['profile_options'])) return $this->meta['profile_options'];
		else return array();
	}

	/**
	* Checks input values of account profiles.
	*
	* @return profile elements
	*/
	function check_profileOptions($options) {
		$messages = array();
		if (is_array($this->meta['profile_checks'])) {
			$identifiers = array_keys($this->meta['profile_checks']);
			for ($i = 0; $i < sizeof($identifiers); $i++) {
				// check if option is required
				if ($this->meta['profile_checks'][$identifiers[$i]]['required'] && ($options[$identifiers[$i]][0] == '')) {
					$messages[] = $this->meta['profile_checks'][$identifiers[$i]]['required_message'];
				}
				// check by regular expression (case insensitive)
				if ($this->meta['profile_checks'][$identifiers[$i]]['type'] == 'regex_i') {
					// ignore empty fileds
					if ($options[$identifiers[$i]][0] == '') continue;
					if (! eregi($this->meta['profile_checks'][$identifiers[$i]]['regex'], $options[$identifiers[$i]][0])) {
						$messages[] = $this->meta['profile_checks'][$identifiers[$i]]['error_message'];
					}
				}
				// check by regular expression (case sensitive)
				elseif ($this->meta['profile_checks'][$identifiers[$i]]['type'] == 'regex') {
					// ignore empty fileds
					if ($options[$identifiers[$i]][0] == '') continue;
					if (! ereg($this->meta['profile_checks'][$identifiers[$i]]['regex'], $options[$identifiers[$i]][0])) {
						$messages[] = $this->meta['profile_checks'][$identifiers[$i]]['error_message'];
					}
				}
				// check by integer comparison (greater)
				elseif ($this->meta['profile_checks'][$identifiers[$i]]['type'] == 'int_greater') {
					// ignore if both fields are empty
					if (($options[$this->meta['profile_checks'][$identifiers[$i]]['cmp_name1']][0] == '') && ($options[$this->meta['profile_checks'][$identifiers[$i]]['cmp_name2']][0] == '')) continue;
					// print error message if only one field is empty
					if (($options[$this->meta['profile_checks'][$identifiers[$i]]['cmp_name1']][0] == '') || ($options[$this->meta['profile_checks'][$identifiers[$i]]['cmp_name2']][0] == '')) {
						$messages[] = $this->meta['profile_checks'][$identifiers[$i]]['error_message'];
						continue;
					}
					// compare
					if (!(intval($options[$this->meta['profile_checks'][$identifiers[$i]]['cmp_name1']][0]) > intval($options[$this->meta['profile_checks'][$identifiers[$i]]['cmp_name2']][0]))) {
						$messages[] = $this->meta['profile_checks'][$identifiers[$i]]['error_message'];
					}
				}
				// check by integer comparison (greater or equal)
				elseif ($this->meta['profile_checks'][$identifiers[$i]]['type'] == 'int_greaterOrEqual') {
					// ignore if both fields are empty
					if (($options[$this->meta['profile_checks'][$identifiers[$i]]['cmp_name1']][0] == '') && ($options[$this->meta['profile_checks'][$identifiers[$i]]['cmp_name2']][0] == '')) continue;
					// print error message if only one field is empty
					if (($options[$this->meta['profile_checks'][$identifiers[$i]]['cmp_name1']][0] == '') || ($options[$this->meta['profile_checks'][$identifiers[$i]]['cmp_name2']][0] == '')) {
						$messages[] = $this->meta['profile_checks'][$identifiers[$i]]['error_message'];
						continue;
					}
					// compare
					if (!(intval($options[$this->meta['profile_checks'][$identifiers[$i]]['cmp_name1']][0]) >= intval($options[$this->meta['profile_checks'][$identifiers[$i]]['cmp_name2']][0]))) {
						$messages[] = $this->meta['profile_checks'][$identifiers[$i]]['error_message'];
					}
				}
				else {
					StatusMessage("ERROR", "Unsupported type!", $this->meta['profile_checks'][$identifiers[$i]]['type']);
				}
			}
		}
		return $messages;
	}

// TODO implement missing interface
}



?>