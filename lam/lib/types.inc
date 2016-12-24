<?php
namespace LAM\TYPES;
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2005 - 2016  Roland Gruber

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
* Returns the alias name of an account type.
*
* @param string $type type name
* @return string type alias
*/
function getTypeAlias($type) {
	if (!empty($_SESSION['config'])) {
		$typeSettings = $_SESSION['config']->get_typeSettings();
		if (!empty($typeSettings['customLabel_' . $type])) {
			return $typeSettings['customLabel_' . $type];
		}
	}
	$obj = new $type();
	return $obj->getAlias();
}

/**
* Returns the description of an account type.
*
* @param string $type type name
* @return string type description
*/
function getTypeDescription($type) {
	$obj = new $type();
	return $obj->getDescription();
}

/**
* Returns the class name for the list object.
*
* @param string $type account type
* @return string class name
*/
function getListClassName($type) {
	$obj = new $type();
	return $obj->getListClassName();
}

/**
* Returns the default attribute list for an account type.
* It is used as default value for the configuration editor.
*
* @param string $type account type
* @return string attribute list
*/
function getDefaultListAttributes($type) {
	$obj = new $type();
	return $obj->getDefaultListAttributes();
}

/**
* Returns a list of attributes which have a translated description.
* This is used for the head row in the list view.
*
* @param string $type account type
* @return array list of descriptions
*/
function getListAttributeDescriptions($type) {
	$obj = new $type();
	return $obj->getListAttributeDescriptions();
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

	private $suffix;

	private $attributes;

	private $alias;

	private $additionalLdapFilter;

	private $hidden;

	private $baseType;

	/**
	 * Constructor
	 *
	 * @param string $scope account type
	 * @param string $id unique ID for this configuration
	 * @param string $suffix LDAP base suffix
	 * @param array $attributes list of ListAttribute
	 * @param string $alias alias name for display
	 * @param string $ldapFilter additional LDAP filter
	 * @param boolean $hidden hidden in GUI
	 */
	public function __construct($scope, $id, $suffix, $attributes, $alias,
			$ldapFilter, $hidden) {
		$this->scope = $scope;
		$this->id = $id;
		$this->suffix = $suffix;
		$this->attributes = $attributes;
		$this->alias = $alias;
		$this->additionalLdapFilter = $ldapFilter;
		$this->hidden = $hidden;
	}

	/**
	 * Returns the account type (e.g. 'user').
	 *
	 * @return string $scope account type
	 */
	public function getScope() {
		return $this->scope;
	}

	/**
	 * Returns a unique id for this configuration.
	 *
	 * @return string $id unique id
	 */
	public function getId() {
		return $this->id;
	}

	/**
	 * Returns the LDAP suffix.
	 *
	 * @return string $suffix LDAP suffix
	 */
	public function getSuffix() {
		return $this->suffix;
	}

	/**
	 * Returns a list of configured attributes.
	 *
	 * @return ListAttribute[] $attributes list of ListAttribute
	 */
	public function getAttributes() {
		return $this->attributes;
	}

	/**
	 * Returns the alias name.
	 *
	 * @return string $alias alias name
	 */
	public function getAlias() {
		return $this->alias;
	}

	/**
	 * Returns the additional LDAP filter.
	 *
	 * @return string $ldapFilter LDAP filter
	 */
	public function getAdditionalLdapFilter() {
		return $this->additionalLdapFilter;
	}

	/**
	 * Returns if this configuration is hidden.
	 *
	 * @return boolean $hidden hidden
	 */
	public function isHidden() {
		return $this->hidden;
	}

	/**
	 * Returns the base type of this configured type.
	 *
	 * @return baseType base type
	 */
	public function getBaseType() {
		if ($this->baseType != null) {
			return $this->baseType;
		}
		$scope = $this->scope;
		$this->baseType = new $scope();
		return $this->baseType;
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

	private $scope;

	/**
	 * Constructor.
	 *
	 * @param string $attributeSpec spec of attribute (e.g. '#uid' or 'uid:User')
	 * @param string $scope account type (e.g. 'user')
	 */
	public function __construct($attributeSpec, $scope) {
		$this->attributeSpec = $attributeSpec;
		$this->scope = $scope;
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
			$name = substr($this->attributeSpec, 1);
			$hash_table = getListAttributeDescriptions($this->scope);
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

	public function getConfiguredType($typeId) {
		$configuredTypes = array();
		$activeTypes = $_SESSION['config']->get_ActiveTypes();
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
		$activeTypes = $_SESSION['config']->get_ActiveTypes();
		foreach ($activeTypes as $typeId) {
			$configuredTypes[] = $this->buildConfiguredType($typeId);
		}
		return $configuredTypes;
	}

	/**
	 * Builds a configured account type.
	 *
	 * @param string $typeId type id
	 */
	private function buildConfiguredType($typeId) {
		$parts = explode('_', $typeId);
		$scope = $parts[0];
		$suffix = $_SESSION['config']->get_Suffix($typeId);
		$attributes = $this->getAttributes($typeId, $scope);
		$alias = getTypeAlias($typeId);
		$ldapFilter = $_SESSION['config']->get_Suffix($typeId);
		$hidden = isAccountTypeHidden($typeId);
		return new ConfiguredType($scope, $typeId, $suffix, $attributes, $alias, $ldapFilter, $hidden);
	}

	/**
	 * Builds the list of account list attributes.
	 *
	 * @param string $typeId type id
	 * @param string $scope account type
	 * @return \LAM\TYPES\ListAttribute[] list attributes
	 */
	private function getAttributes($typeId, $scope) {
		$attributeString = $_SESSION['config']->get_listAttributes($typeId);
		$attributeSpecs = explode(';', $attributeString);
		$attributes = array();
		foreach ($attributeSpecs as $attributeSpec) {
			$attributes[] = new ListAttribute($attributeSpec, $scope);
		}
		return $attributes;
	}

}

?>