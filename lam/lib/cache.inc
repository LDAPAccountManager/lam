<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Tilo Lutz
  Copyright (C) 2007 Roland Gruber

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
* Provides a cache for LDAP attributes.
*
* @author Tilo Lutz
* @author Roland Gruber
* @package lib
*/

/** en/decryption functions */
include_once('ldap.inc');


/**
* This class contains all functions which are needed to manage the LDAP cache.
*
* @package lib
*/
class cache {
	
	/** This variable contains the cache */
	private $ldapcache;
	/** This variable contains a list and their scope of attributes which should be cached */
	private $attributes;
	/** This is the last timestamp on which the LDAP cache has been refreshed */
	private $time;

	/**
	 * Constructor.
	 *
	 * @return cache cache object
	 */
	function __construct() {
		$this->time = 0;
		$this->attributes = array();
	}

	/**
	 * This function adds attributes to the cache.
	 * 
	 * @param array $attributes syntax: is array( scope1 => array ( attributes ), scope2 => array ( attributes ), ...)
	 */
	private function add_cache($attributes) {
		if (!is_array($attributes)) trigger_error('Argument of add_cache must be : array ( scope => array(attribute1(string), attribute2(string), ..), scope => ... ).', E_USER_ERROR);
		foreach ($attributes as $attribute) {
			if (!is_array($attribute)) trigger_error('Argument of add_cache must be : array ( scope => array(attribute1(string), attribute2(string), ..), scope => ... ).', E_USER_ERROR);
			foreach ($attribute as $singleattribute) {
				if (!is_string($singleattribute)) trigger_error('Argument of add_cache must be : array ( scope => array(attribute1(string), attribute2(string), ..), scope => ... ).', E_USER_ERROR);
				}
			}
		$scopes = array_keys($attributes);
		foreach ($scopes as $scope) {
			for ($i=0; $i<count($attributes[$scope]); $i++ ) {
				if (!@in_array($attributes[$scope][$i] ,$this->attributes[$scope])) $this->attributes[$scope][] = $attributes[$scope][$i];
				}
			}
		// Rebuild cache
		$this->refresh_cache(true);
	}

	/**
	* Queries the cache for a list of LDAP entries and their attributes.
	*
	* @param mixed $attributes One (string) or many (array) attribute names.
	* @param string $objectClass The resulting entries need to contain this object class.
	* @param mixed $scopelist the account type(s) as string or array, all scopes if NULL given
	* @return array The found LDAP entries.
	* <br>Format: array(dn1 => array(uidnumber1), dn2 => array(uidnumber2), ... ) if $attributes is of type string
	* <br>or array(dn1 => array(uid => array(myuid), uidNumber => array(1234)), ... ) if $attributes is an array
	*
	*/
	public function get_cache($attributes, $objectClass, $scopelist) {
		$return = array();
		$this->refresh_cache();
		if (is_array($scopelist)) $scopes = $scopelist;
		elseif (is_string($scopelist)) $scopes = array($scopelist);
		else $scopes = getTypes();
		// Add cache entry dynamic
		if (!is_array($attributes)) $attributes = array($attributes);
		$add = array();
		foreach ($scopes as $scope) {
			for ($i = 0; $i < sizeof($attributes); $i++) {
				if (!@in_array($attributes[$i], $this->attributes[$scope])) $add[$scope][] = $attributes[$i];
			}
		}
		if (count($add)!=0) $this->add_cache($add);

		foreach ($scopes as $scope) {
			if (isset($this->ldapcache[$scope])) {
				$DNs = array_keys($this->ldapcache[$scope]);
				foreach ($DNs as $dn) {
					// skip entries which do not fit to search
					if (!in_array($objectClass, $this->ldapcache[$scope][$dn]['objectClass'])) continue;
					for ($i = 0; $i < sizeof($attributes); $i++) {
						if (isset($this->ldapcache[$scope][$dn][$attributes[$i]])) {
							if (sizeof($attributes) > 1) {
								$return[$dn][$attributes[$i]] = $this->ldapcache[$scope][$dn][$attributes[$i]];
							}
							else {
								$return[$dn] = $this->ldapcache[$scope][$dn][$attributes[$i]];
							}
						}
					}
				}
			}
		}
		return $return;
	}

	/**
	 * This function returns the DN if an LDAP entry with $attribute=$value is found.
	 * 
	 * @param string $value is the searched value of the attribute $attribute
	 * @param string $attribute name of the LDAP attribute
	 * @param mixed $scopelist the account type(s) as string or array, all scopes if NULL given
	 */
	function in_cache($value, $attribute, $scopelist) {
		$this->refresh_cache();
		if (is_array($scopelist)) $scopes = $scopelist;
		elseif (is_string($scopelist)) $scopes = array($scopelist);
		else $scopes = getTypes();
		// Add cache entry dynamic
		$add = array();
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


	/**
	 *  This function refreshes the cache.
	 * 
	 * @param boolean $rebuild forces a refresh if set to true
	 */
	public function refresh_cache($rebuild=false) {
		if ($this->time + $_SESSION['config']->get_cacheTimeoutSec() < time() || $rebuild) {
			// unset old cache
			unset ($this->ldapcache);
			$scopes = array_keys($this->attributes);
			foreach ($scopes as $scope) {
				// Get suffix
				$suffix = $_SESSION['config']->get_Suffix($scope);
				// Get Data from ldap
				$search = $this->attributes[$scope];
				$search[] = 'objectClass';
				$result = @ldap_search($_SESSION['ldap']->server(), escapeDN($suffix), 'objectClass=*', $search, 0, 0, 0, LDAP_DEREF_NEVER);
				if ($result) {
					// Write search result in array
					$entry = @ldap_first_entry($_SESSION['ldap']->server(), $result);
					while ($entry) {
						$dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
						$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
						// unset double entries
						for ($i=0; $i<count($attr); $i++) {
							if (isset($attr[$i])) unset($attr[$i]);
						}
						// unset every count entry
						unset ($attr['count']);
						$attributes = array_keys($attr);
						foreach ($attributes as $attribute) {
							unset ($attr[$attribute]['count']);
						}
						// Write new cache entry
						$addcache = $attr;
						unset ($addcache['objectClass']);
						if (count($addcache)!=0) $this->ldapcache[$scope][$dn] = $attr;
						$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
					}
					ldap_free_result($result);
				}
			}
			$this->time = time();
		}
	}

	/**
	* Encrypts LDAP cache before saving to session file.
	*
	* @return array list of variables to save
	*/
	function __sleep() {
		$this->ldapcache = $_SESSION['ldap']->encrypt(serialize($this->ldapcache));
		// define which attributes to save
		return array("ldapcache", "attributes", "time");
	}
	
	/**
	* Decrypts LDAP cache after loading from session file.
	*/
	function __wakeup() {
		$this->ldapcache = unserialize($_SESSION['ldap']->decrypt($this->ldapcache));
	}

}

?>
