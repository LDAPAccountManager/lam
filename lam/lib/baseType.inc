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
* This is the parent class of all account types.
*
* @package types
* @author Roland Gruber
*/

/**
* This is the parent class of all account types.
*
* @package types
*/
class baseType {
	
	/**
	* Returns the alias name of this account type.
	* This function must be overwritten by the child classes.
	*
	* @return string alias name
	*/
	function getAlias() {
		return "baseType";
	}
	
	/**
	* Returns the description of this account type.
	* This function must be overwritten by the child classes.
	*
	* @return string description
	*/
	function getDescription() {
		return "base type";
	}
	
	/**
	* Returns the class name for the list object.
	*
	* @return string class name
	*/
	function getListClassName() {
		return "lamList";
	}
	
	/**
	* Returns the default attribute list for this account type.
	* This function must be overwritten by the child classes.
	*
	* @return string attribute list
	*/
	function getDefaultListAttributes() {
		return "dn:DN,objectClass:Object classes";
	}

	/**
	* Returns a list of attributes which have a translated description.
	* This is used for the head row in the list view.
	*
	* @return array list of descriptions
	*/
	function getListAttributeDescriptions() {
		return array();
	}


}

?>