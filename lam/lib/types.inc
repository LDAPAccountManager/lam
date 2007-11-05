<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2005 - 2006  Roland Gruber

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
	while ($entry = $dir->read())
		if ((substr($entry, strlen($entry) - 4, 4) == '.inc') && is_file($dirname . '/'.$entry)) {
			$entry = substr($entry, 0, strpos($entry, '.'));
			$return[] = $entry;
		}
	return $return;
}

/**
* Returns the alias name of an account type.
*
* @param string $type type name
* @return string type alias
*/
function getTypeAlias($type) {
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

?>