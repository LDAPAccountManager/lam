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
* The account type for group accounts (e.g. Unix and Samba).
*
* @package types
* @author Roland Gruber
*/

/**
* The account type for group accounts (e.g. Unix and Samba).
*
* @package types
*/
class group extends baseType {
	
	/**
	* Returns the alias name of this account type.
	*
	* @return string alias name
	*/
	function getAlias() {
		return _("Groups");
	}
	
	/**
	* Returns the description of this account type.
	*
	* @return string description
	*/
	function getDescription() {
		return _("Group accounts (e.g. Unix and Samba)");
	}
		
	/**
	* Returns the class name for the list object.
	*
	* @return string class name
	*/
	function getListClassName() {
		return "lamGroupList";
	}
	
	/**
	* Returns the default attribute list for this account type.
	*
	* @return string attribute list
	*/
	function getDefaultListAttributes() {
		return "#cn;#gidNumber;#memberUID;#description";
	}

	/**
	* Returns a list of attributes which have a translated description.
	* This is used for the head row in the list view.
	*
	* @return array list of descriptions
	*/
	function getListAttributeDescriptions() {
		return array (
			"cn" => _("Group name"),
			"gidnumber" => _("GID number"),
			"memberuid" => _("Group members"),
			"member" => _("Group member DNs"),
			"description" => _("Group description")
			);
	}

}

/**
 * Generates the list view.
 *
 * @package lists
 * @author Roland Gruber
 * 
 */
class lamGroupList extends lamList {

	/**
	 * Constructor
	 *
	 * @param string $type account type
	 * @return lamList list object
	 */
	function lamGroupList($type) {
		parent::lamList($type);
		$this->labels = array(
			'nav' => _("%s group(s) found"),
			'error_noneFound' => _("No groups found!"),
			'newEntry' => _("New group"),
			'deleteEntry' => _("Delete group(s)"),
			'createPDF' => _("Create PDF for selected group(s)"),
			'createPDFAll' => _("Create PDF for all groups"));
	}
	
	/**
	 * Prints the content of a cell in the account list for a given LDAP entry and attribute.
	 *
	 * @param array $entry LDAP attributes
	 * @param string $attribute attribute name
	 */
	function listPrintTableCellContent(&$entry, &$attribute) {
		if ($attribute == "memberuid") {
			if (!is_array($entry[$attribute]) || (sizeof($entry[$attribute]) < 1)) return;
			if (isset($entry[$attribute]['count'])) unset($entry[$attribute]['count']);
			// sort array
			sort($entry[$attribute]);
			// make a link for each member of the group
			$linklist = array();
			for ($d = 0; $d < sizeof($entry[$attribute]); $d++) {
				$user = $entry[$attribute][$d]; // user name
				$linklist[$d] = "<a href=\"userlink.php?user='" . $user . "' \">" . $user . "</a>";
			}
			echo implode("; ", $linklist);
		}
		// print all other attributes
		else {
			parent::listPrintTableCellContent($entry, $attribute);
		}
	}

}


?>