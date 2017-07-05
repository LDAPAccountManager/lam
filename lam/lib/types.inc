<?php
namespace LAM\TYPES;
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2005 - 2017  Roland Gruber

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
* This file is the interface to the different account types.
*
* @package types
* @author Roland Gruber
*/

/** parent class of account types */
include_once("baseType.inc");
/** parent class of list views */
include_once("lists.inc");
/** Used to check if this is a LAM Pro release. */
include_once("selfService.inc");


/**
* This includes all type definitions.
*/
$typesINC_dirname = substr(__FILE__, 0, strlen(__FILE__) - 10) . "/types";
$typesINC_dir = dir($typesINC_dirname);
// get module names.
while ($entry = $typesINC_dir->read())
if ((substr($entry, strlen($entry) - 4, 4) == '.inc') && is_file($typesINC_dirname . '/'.$entry)) {
	include_once($typesINC_dirname . '/'.$entry);
}


/**
* Returns a list of available account types.
*
* @return array list of types
*/
function getTypes() {
	$dirname = substr(__FILE__, 0, strlen(__FILE__) - 10) . "/types";
	$dir = dir($dirname);
	$return = array();
	// get type names.
	while ($entry = $dir->read()) {
		if ((substr($entry, strlen($entry) - 4, 4) == '.inc') && is_file($dirname . '/'.$entry)) {
			$entry = substr($entry, 0, strpos($entry, '.'));
			$return[] = $entry;
		}
	}
	$dir->close();
	return $return;
}

/**
* Returns the description of an account type.
*
* @param string $type type name
* @return string type description
*/
function getTypeDescription($type) {
	$obj = new $type(null);
	return $obj->getDescription();
}

/**
 * Returns the account type for a given type id.
 *
 * @param string $typeId type id (e.g. user_1)
 * @return string scope (e.g. user)
 */
function getScopeFromTypeId($typeId) {
	$parts = explode('_', $typeId);
	return $parts[0];
}

/**
 * Represents a configured account type variant.
 *
 * @package types
 * @author Roland Gruber
 */
class ConfiguredType {

	private $scope;

	private $id;

	private $suffix = null;

	private $attributes = null;

	private $alias = null;

	private $additionalLdapFilter = null;

	private $hidden = null;

	private $baseType;

	private $typeManager;

	/**
	 * Constructor
	 *
	 * @param TypeManager $typeManager type manager
	 * @param string $scope account type
	 * @param string $id unique ID for this configuration
	 */
	public function __construct(&$typeManager, $scope, $id) {
		$this->typeManager = &$typeManager;
		$this->scope = $scope;
		$this->id = $id;
	}

	/**
	 * Returns the owning type manager.
	 *
	 * @return TypeManager type manager
	 */
	public function getTypeManager() {
		return $this->typeManager;
	}

	/**
	 * Returns the account type (e.g. 'user').
	 *
	 * @return string account type
	 */
	public function getScope() {
		return $this->scope;
	}

	/**
	 * Returns a unique id for this configuration.
	 *
	 * @return string unique id
	 */
	public function getId() {
		return $this->id;
	}

	/**
	 * Returns the LDAP suffix.
	 *
	 * @return string LDAP suffix
	 */
	public function getSuffix() {
		if ($this->suffix !== null) {
			return $this->suffix;
		}
		$this->suffix = $this->typeManager->getConfig()->get_Suffix($this->id);
		return $this->suffix;
	}

	/**
	 * Returns a list of configured attributes.
	 *
	 * @return ListAttribute[] list of ListAttribute
	 */
	public function getAttributes() {
		if ($this->attributes !== null) {
			return $this->attributes;
		}
		$attributeString = $this->typeManager->getConfig()->get_listAttributes($this->id);
		$attributeSpecs = explode(';', $attributeString);
		$attributes = array();
		foreach ($attributeSpecs as $attributeSpec) {
			$attributes[] = new ListAttribute($attributeSpec, $this);
		}
		$this->attributes = $attributes;
		return $this->attributes;
	}

	/**
	 * Returns the alias name.
	 *
	 * @return string alias name
	 */
	public function getAlias() {
		if ($this->alias !== null) {
			return $this->alias;
		}
		$typeSettings = $this->typeManager->getConfig()->get_typeSettings();
		if (!empty($typeSettings['customLabel_' . $this->id])) {
			return $typeSettings['customLabel_' . $this->id];
		}
		return $this->getBaseType()->getAlias();
	}

	/**
	 * Returns the additional LDAP filter.
	 *
	 * @return string LDAP filter
	 */
	public function getAdditionalLdapFilter() {
		if ($this->additionalLdapFilter !== null) {
			return $this->additionalLdapFilter;
		}
		$typeSettings = $this->typeManager->getConfig()->get_typeSettings();
		$this->additionalLdapFilter = isset($typeSettings['filter_' . $this->id]) ? $typeSettings['filter_' . $this->id] : '';
		return $this->additionalLdapFilter;
	}

	/**
	 * Returns if this configuration is hidden.
	 *
	 * @return boolean hidden
	 */
	public function isHidden() {
		if ($this->hidden !== null) {
			return $this->hidden;
		}
		$this->hidden = isAccountTypeHidden($this->id);
		return $this->hidden;
	}

	/**
	 * Returns the base type of this configured type.
	 *
	 * @return \baseType base type
	 */
	public function getBaseType() {
		if ($this->baseType != null) {
			return $this->baseType;
		}
		$scope = $this->scope;
		$this->baseType = new $scope($this);
		return $this->baseType;
	}

	/**
	 * Returns a list of LDAP suffixes for this type.
	 *
	 * @return array sorted list of possible suffixes for this type.
	 */
	public function getSuffixList() {
		$connection = $_SESSION["ldap"]->server();
		$ret = array();
		$filter = $this->getBaseType()->getSuffixFilter();
		$sr = @ldap_search($connection, escapeDN($this->getSuffix()), $filter, array('dn', 'objectClass'), 0, 0, 0, LDAP_DEREF_NEVER);
		if ($sr) {
			$units = ldap_get_entries($connection, $sr);
			cleanLDAPResult($units);
			// extract Dns
			$count = sizeof($units);
			for ($i = 0; $i < $count; $i++) {
				if (in_array('container', $units[$i]['objectclass'])) {
					// Active Directory fix, hide system containers
					if (preg_match('/.*cn=system,dc=.+/i', $units[$i]['dn']) || preg_match('/.*CN=program data,dc=.+/i', $units[$i]['dn'])) {
						continue;
					}
				}
				$ret[] = $units[$i]['dn'];
			}
		}
		// add root suffix if needed
		$found = false;
		for ($i = 0; $i < sizeof($ret); $i++) { // search suffix case-intensitive
			if (strtolower($this->getSuffix()) == strtolower($ret[$i])) {
				$found = true;
				break;
			}
		}
		if (!$found) {
			$ret[] = $this->getSuffix();
		}
		usort($ret, 'compareDN');
		return $ret;
	}

	/**
	 * Returns the names of the active modules for this type.
	 *
	 * @return string[] module names
	 */
	public function getModules() {
		$typeSettings = $this->typeManager->getConfig()->get_typeSettings();
		$modules = !empty($typeSettings['modules_' . $this->getId()]) ? $typeSettings['modules_' . $this->getId()] : '';
		return explode(',', $modules);
	}

	/**
	 * Returns the file name of the type icon.
	 * It is 16x16px and located in graphics folder.
	 *
	 * @return string file name
	 */
	public function getIcon() {
		$baseType = $this->getBaseType();
		return $baseType->getIcon();
	}

}

/**
 * An attribute definition for the account list.
 *
 * @package types
 * @author Roland Gruber
 */
class ListAttribute {

	private $attributeSpec;

	private $type;

	/**
	 * Constructor.
	 *
	 * @param string $attributeSpec spec of attribute (e.g. '#uid' or 'uid:User')
	 * @param ConfiguredType $type account type (e.g. 'user')
	 */
	public function __construct($attributeSpec, $type) {
		$this->attributeSpec = $attributeSpec;
		$this->type = $type;
	}
	/**
	 * Returns the name of the LDAP attribute.
	 *
	 * @return string $attributeName name
	 */
	public function getAttributeName() {
		if ($this->isPredefined()) {
			return substr($this->attributeSpec, 1);
		}
		$parts = explode(':', $this->attributeSpec);
		return $parts[0];
	}

	/**
	 * Returns the display value.
	 *
	 * @return string $alias display value
	 */
	public function getAlias() {
		if ($this->isPredefined()) {
			$name = strtolower(substr($this->attributeSpec, 1));
			$hash_table = $this->type->getBaseType()->getListAttributeDescriptions();
			$hash_table = array_change_key_case($hash_table, CASE_LOWER);
			if (isset($hash_table[$name])) {
				return $hash_table[$name];
			}
			return $name;
		}
		$parts = explode(':', $this->attributeSpec);
		return $parts[1];
	}

	/**
	 * Returns if this is a predefined attribute name.
	 *
	 * @return boolean is predefined
	 */
	private function isPredefined() {
		return strpos($this->attributeSpec, '#') === 0;
	}

}

/**
 * Provides utility functions to get e.g. configured types.
 *
 * @package types
 * @author Roland Gruber
 */
class TypeManager {

	private $config;

	/**
	 * Constructor
	 *
	 * @param \LAMConfig $config configuration (uses $_SESSION['config'] by default)
	 */
	public function __construct(&$config = null) {
		if ($config == null) {
			$config = &$_SESSION['config'];
		}
		$this->config = &$config;
	}

	/**
	 * Returns the configured type with the given id or null.
	 *
	 * @param string $typeId type id
	 * @return \LAM\TYPES\ConfiguredType|NULL type
	 */
	public function getConfiguredType($typeId) {
		if ($this->config == null) {
			return null;
		}
		$activeTypes = $this->config->get_ActiveTypes();
		if (in_array($typeId, $activeTypes)) {
			return $this->buildConfiguredType($typeId);
		}
		return null;
	}

	/**
	 * Returns a list of configured account types.
	 *
	 * @return \LAM\TYPES\ConfiguredType[] list of ConfiguredType
	 */
	public function getConfiguredTypes() {
		$configuredTypes = array();
		$activeTypes = $this->config->get_ActiveTypes();
		foreach ($activeTypes as $typeId) {
			$configuredTypes[] = $this->buildConfiguredType($typeId);
		}
		return $configuredTypes;
	}

	/**
	 * Returns a list of configured types for this scope.
	 *
	 * @param string $scope scope (e.g. user)
	 * @return \LAM\TYPES\ConfiguredType[] list of ConfiguredType
	 */
	public function getConfiguredTypesForScope($scope) {
		$allTypes = $this->getConfiguredTypes();
		$scopedTypes = array();
		foreach ($allTypes as $type) {
			if ($type->getScope() == $scope) {
				$scopedTypes[] = $type;
			}
		}
		return $scopedTypes;
	}

	/**
	 * Returns a list of configured types for these scopes.
	 *
	 * @param array $scopes scopes (e.g. user)
	 * @return \LAM\TYPES\ConfiguredType[] list of ConfiguredType
	 */
	public function getConfiguredTypesForScopes($scopes) {
		$allTypes = $this->getConfiguredTypes();
		$scopedTypes = array();
		foreach ($allTypes as $type) {
			if (in_array($type->getScope(), $scopes)) {
				$scopedTypes[] = $type;
			}
		}
		return $scopedTypes;
	}

	/**
	 * Builds a configured account type.
	 *
	 * @param string $typeId type id
	 */
	private function buildConfiguredType($typeId) {
		$scope = getScopeFromTypeId($typeId);
		return new ConfiguredType($this, $scope, $typeId);
	}

	/**
	 * Generates a new unique type id for the given scope.
	 *
	 * @param string $scope account type (e.g. user)
	 */
	public function generateNewTypeId($scope) {
		$activeTypes = $this->config->get_ActiveTypes();
		if (!in_array($scope, $activeTypes)) {
			return $scope;
		}
		$counter = 1;
		while (in_array($scope . '_' . $counter, $activeTypes)) {
			$counter++;
		}
		return $scope . '_' . $counter;
	}

	/**
	 * Returns the associated config object.
	 *
	 * @return \LAMConfig config
	 */
	public function getConfig() {
		return $this->config;
	}

	/**
	 * Returns if configuration is loaded.
	 *
	 * @return boolean configured
	 */
	public function hasConfig() {
		return !empty($this->config);
	}

}

?>