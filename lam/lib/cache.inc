<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Tilo Lutz

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


  LDAP Account Manager functions used by account.php
*/

/* This class contains all functions
* which are needed to manage the ldap cache
*/
class cache {
	function cache() {
		$this->time = 0;
		$this->attributes = array();
		}

	var $ldapcache; // This variable contains the cache
	var $attributes; // This variable contains a list and their scope of attributes which should be cached
	var $time; // This is the laste timestamp ldap cache has been refreshed

	/* This function adds attributes to cache
	* syntax of $attributes is array( scope1 => array ( attributes ), scope2 => array ( attributes ), ...)
	*/
	function add_cache($attributes) {
		// Check input variable
		$allowed_types = array ( 'user', 'group', 'host', 'domain', '*' );
		if (!is_array($attributes)) trigger_error('Argument of add_cache must be : array ( scope => array(attribute1(string), attribute2(string), ..), scope => ... ).', E_USER_ERROR);
		foreach ($attributes as $attribute) {
			if (!is_array($attribute)) trigger_error('Argument of add_cache must be : array ( scope => array(attribute1(string), attribute2(string), ..), scope => ... ).', E_USER_ERROR);
			foreach ($attribute as $singleattribute) {
				if (!is_string($singleattribute)) trigger_error('Argument of add_cache must be : array ( scope => array(attribute1(string), attribute2(string), ..), scope => ... ).', E_USER_ERROR);
				}
			}
		$scopes = array_keys($attributes);
		foreach ($scopes as $scope) {
			if (!@in_array($scope, $allowed_types)) trigger_error(sprintf('Invalid scope. Valid scopes are %s.', implode(" ", $allowed_types)), E_USER_ERROR);
			}
		// Everything seems to be OK, start processing data
		foreach ($scopes as $scope) {
			for ($i=0; $i<count($attributes[$scope]); $i++ ) {
				if (!@in_array($attributes[$scope][$i] ,$this->attributes[$scope])) $this->attributes[$scope][] = $attributes[$scope][$i];
				}
			}
		// Rebuild cache
		$this->refresh_cache(true);
		}

	/* This function returns an array ( dn1 => array(uidnumber1), dn2 => array(uidnumber2), ... )
	*
	*/
	function get_cache($attribute, $objectClass, $singlescope) {
		$this->refresh_cache();
		// Check input variables
		$allowed_types = array ( 'user', 'group', 'host', 'domain', '*' );
		if (!in_array($singlescope, $allowed_types)) trigger_error(sprintf('Invalid scope. Valid scopes are %s.', implode(" ", $allowed_types)), E_USER_ERROR);
		$line=-1;
		for ($i=0; $i<count($_SESSION['ldap']->objectClasses) || $i==-1; $i++) {
			if (strpos($_SESSION['ldap']->objectClasses[$i], "NAME '$objectClass'")) $line = $i;
			}
		// Return error if objectClass isn't found
		if ($line==-1) trigger_error (sprintf(_("objectClass %s required but not defined in ldap."), $objectClass), E_USER_WARNING);

		// Create list of all allowed attributes
		for ($i=0; $i<count($_SESSION['ldap']->objectClasses); $i++ ) {
			if (strpos($_SESSION['ldap']->objectClasses[$i],  'MUST (')) {
				$string_withtail = substr($_SESSION['ldap']->objectClasses[$i], strpos($_SESSION['ldap']->objectClasses[$i],  'MUST (')+6);
				// Now we have a string with all must-attributes
				$string = substr($string_withtail, 0, strpos($string_withtail, ')'));
				$string = trim($string);
				$allowed_attributes = array_merge($allowed_attributes, explode(" $ ", $string));
				}
			// create array with may-attributes
			// Get startposition in string
			if (strpos($_SESSION['ldap']->objectClasses[$i],  'MAY (')) {
				$string_withtail = substr($_SESSION['ldap']->objectClasses[$i], strpos($_SESSION['ldap']->objectClasses[$i],  'MAY (')+5);
				// Now we have a string with all must-attributes
				$string = substr($string_withtail, 0, strpos($string_withtail, ')'));
				$string = trim($string);
				$allowed_attributes = array_merge($allowed_attributes, explode(" $ ", $string));
				}
			}
		$allowed_attributes = array_unique($allowed_attributes);
		if (!in_array($attribute, $allowed_attributes)) trigger_error(_('Attribute not defined in LDAP.'), E_USER_WARNING);

		// Everything seems to be OK, start processing data
		$this->refresh_cache();
		if ($singlescope == '*') $scopes = $allowed_types;
			else $scopes = array ( $singlescope );
		// Add cache entry dynamic
		foreach ($scopes as $scope) {
			if (!@in_array($attribute ,$this->attributes[$scope])) $add[$scope][] = $attribute;
			}
		if (count($add)!=0) $this->add_cache($add);

		foreach ($scopes as $scope) {
			if (isset($this->ldapcache[$scope])) {
				$DNs = array_keys($this->ldapcache[$scope]);
				foreach ($DNs as $dn) {
					if (isset($this->ldapcache[$scope][$dn][$attribute]) && in_array($objectClass, $this->ldapcache[$scope][$dn]['objectClass'])) {
						// return string if only attribute exists only once
						if (count($this->ldapcache[$scope][$dn][$attribute])==1) $return[$dn] = array($this->ldapcache[$scope][$dn][$attribute][0]);
						else {
							// else return array with all attributes
							$return[$dn] = $this->ldapcache[$scope][$dn][$attribute];
							}
						}
					}
				}
			}
		return $return;
		}

	/* This functions returns the dn if a dn with $attribute=$value is found
	* $values is the value $attribute is set to
	* $scope is the scope where to search
	*/
	function in_cache($value, $attribute, $singlescope) {
		$this->refresh_cache();
		// Check input variables
		$allowed_types = array ( 'user', 'group', 'host', 'domain', '*' );
		if (!in_array($singlescope, $allowed_types)) trigger_error(sprintf('Invalid scope. Valid scopes are %s.', implode(" ", $allowed_types)), E_USER_ERROR);
		// Create list of all allowed attributes
		for ($i=0; $i<count($_SESSION['ldap']->objectClasses); $i++ ) {
			if (strpos($_SESSION['ldap']->objectClasses[$i],  'MUST (')) {
				$string_withtail = substr($_SESSION['ldap']->objectClasses[$i], strpos($_SESSION['ldap']->objectClasses[$i],  'MUST (')+6);
				// Now we have a string with all must-attributes
				$string = substr($string_withtail, 0, strpos($string_withtail, ')'));
				$string = trim($string);
				$allowed_attributes = array_merge($allowed_attributes, explode(" $ ", $string));
				}
			// create array with may-attributes
			// Get startposition in string
			if (strpos($_SESSION['ldap']->objectClasses[$i],  'MAY (')) {
				$string_withtail = substr($_SESSION['ldap']->objectClasses[$i], strpos($_SESSION['ldap']->objectClasses[$i],  'MAY (')+5);
				// Now we have a string with all must-attributes
				$string = substr($string_withtail, 0, strpos($string_withtail, ')'));
				$string = trim($string);
				$allowed_attributes = array_merge($allowed_attributes, explode(" $ ", $string));
				}
			}
		$allowed_attributes = array_unique($allowed_attributes);
		if (!in_array($attribute, $allowed_attributes)) trigger_error(_('Attribute not defined in LDAP.'), E_USER_WARNING);

		// Everything seems to be OK, start processing data
		$this->refresh_cache();
		if ($singlescope == '*') $scopes = $allowed_types;
			else $scopes = array ( $singlescope );
		// Add cache entry dynamic
		foreach ($scopes as $scope) {
			if (!@in_array($attribute ,$this->attributes[$scope])) $add[$scope][] = $attribute;
			}
		if (count($add)!=0) $this->add_cache($add);

		foreach ($scopes as $scope) {
			if (isset($this->ldapcache[$scope])) {
				$DNs = array_keys($this->ldapcache[$scope]);
				foreach ($DNs as $dn) {
					if (is_array($this->ldapcache[$scope][$dn][$attribute])) {
						if (in_array($value, $this->ldapcache[$scope][$dn][$attribute])) {
							// Return value if value was found
							return $dn;
							}
						}
					}
				}
			}
		// Return false if value wasn't found
		return false;
		}


	/* This functions refreshs the cache
	*/
	function refresh_cache($rebuild=false) {
		if ($this->time + $_SESSION['config']->get_cacheTimeoutSec() < time() || $rebuild) {
			// unset old cache
			unset ($this->ldapcache);
			$scopes = array_keys($this->attributes);
			foreach ($scopes as $scope) {
				// Get Scope
				If ($scope != '*')
					$suffix = call_user_func(array(&$_SESSION['config'], 'get_'.ucfirst($scope).'Suffix'));
					else $suffix = '';
				// Get Data from ldap
				$search = $this->attributes[$scope];
				$search[] = 'objectClass';
				$result = @ldap_search($_SESSION['ldap']->server(), $suffix, 'objectClass=*', $search, 0);
				// Write search result in array
				$entry = @ldap_first_entry($_SESSION['ldap']->server(), $result);
				while ($entry) {
					$dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
					$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
					// unset every count entry
					unset ($attr['count']);
					$attributes = array_keys($attr);
					foreach ($attributes as $attribute) {
						unset ($attr[$attribute]['count']);
					}
					// unset double entries
					for ($i=0; $i<count($attr); $i++) {
						if (isset($attr[$i])) unset($attr[$i]);
						}
					// Write new cache entry
					$addcache = $attr;
					unset ($addcache['objectClass']);
					if (count($addcache)!=0) $this->ldapcache[$scope][$dn] = $attr;
					$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
					}
				}
			$this->time = time();
			}
		}

	/* This function update the cache when changes were
	* made without refrehing the complete cache
	*/
	function update_cache($dn, $mode, $attributes=false) {
		$allowed_modes = array ( 'add', 'remove', 'modify', 'delete_dn' );
		$allowed_types = array ( 'user', 'group', 'host', '*' );
		for ($i=0; $i<count($allowed_types); $i++) {
			if ($allowed_types[$i]!='*') {
				// *** fixme, where is get_DomainSuffix
				If ($scope != '*')
					$suffix = call_user_func(array(&$_SESSION['config'], 'get_'.ucfirst($allowed_types[$i]).'Suffix'));
					else $suffix = '';
				if (substr($suffix, $dn)) $singlescope = $allowed_types[$i];
				}
			}
		if (!in_array($singlescope, $allowed_types)) trigger_error(sprintf('Invalid scope. Valid scopes are %s.', implode(" ", $allowed_types)), E_USER_ERROR);
		if (!in_array($mode, $allowed_modes)) trigger_error(sprintf('Invalid mode. Valid modes are %s.', implode(" ", $allowed_modes)), E_USER_ERROR);
		// Everything seems to be OK, start processing data
		// Get Scope
		foreach ($allowed_types as $scope) {
			if ($scope!='*') {
				$suffix = call_user_func(array(&$_SESSION['config'], 'get_'.ucfirst($scope).'Suffix'));
				if (strpos($dn, $suffix)) $singlescope = $scope;
				}
			}
		if (!isset($singlescope)) trigger_error(sprintf('Invalid dn: %s. DN not covered by any suffix.', $dn), E_USER_WARN);
		// Refresh Cache
		$this->refresh_cache();
		if (is_array($attributes))
			switch ($mode) {
				case 'add':
					$list = array_keys($attributes);
					for ($i=0; $i<count($list); $i++)
						foreach ($attributes[$list[$i]] as $attribute)
							$this->ldapcache[$singlescope][$dn][$list[$i]][] = $attributes[$list[$i]];
					break;
				case 'remove':
					$list = array_keys($attributes);
					for ($i=0; $i<count($list); $i++)
						foreach ($attributes[$list[$i]] as $attribute)
							if (isset($this->ldapcache[$singlescope][$dn][$list[$i]][$attributes[$list[$i]]]))
								unset($this->ldapcache[$singlescope][$dn][$list[$i]][$attributes[$list[$i]]]);
					break;
				case 'modify':
					$list = array_keys($attributes);
					for ($i=0; $i<count($list); $i++) {
						if (isset($this->ldapcache[$singlescope][$dn][$list[$i]])) unset($this->ldapcache[$singlescope][$dn][$list[$i]]);
						foreach ($attributes[$list[$i]] as $attribute)
							$this->ldapcache[$singlescope][$dn][$list[$i]][] = $attributes[$list[$i]];
						}
				}
		else {
			if ($mode=='delete_dn')
				if (isset($this->ldapcache[$singlescope][$dn])) unset($this->ldapcache[$singlescope][$dn]);
			}
		}


	/* This function will return the gidNumber to an existing groupname
	* gidNumbers are taken from cache-array
	*/
	function getgid($groupname) {
		$dn_groups = $_SESSION['cache']->get_cache('gidNumber', 'posixGroup', 'group');
		$DNs = array_keys($dn_groups);
		foreach ($DNs as $DN) {
			// TODO doesn't work when groupname is part of DN
			if (strpos($DN, $groupname))
				return $dn_groups[$DN][0];
			}
		}


	/* This function will return an array with all groupnames
	* found in ldap. Groupnames are taken from cache-array.
	*/
	function findgroups() {
		$dn_groups = $_SESSION['cache']->get_cache('cn', 'posixGroup', 'group');
		if (is_array($dn_groups)) {
			$DNs = array_keys($dn_groups);
			foreach ($DNs as $DN)
				$return[] = $dn_groups[$DN][0];
			return $return;
			}
		return array();
		}


	/* This function will return the groupname to an existing gidNumber
	* groupnames are taken from cache-array
	*/
	function getgrnam($gidNumber) {
		$dn_groups = $_SESSION['cache']->get_cache('gidNumber', 'posixGroup', 'group');
		if (is_array($dn_groups)) {
			$DNs = array_keys($dn_groups);
			foreach ($DNs as $DN) {
				if ($dn_groups[$DN][0]==$gidNumber)
					$return = substr($DN, 3, strpos($DN, ',')-3);
				}
			return $return;
			}
		else return -1;
		}

	}

?>
