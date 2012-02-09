<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2005 - 2012  Roland Gruber

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
* The account type for host accounts (e.g. Samba).
*
* @package types
* @author Roland Gruber
*/

/**
* The account type for host accounts (e.g. Samba).
*
* @package types
*/
class host extends baseType {
	
	/**
	 * Constructs a new host type object.
	 */
	public function __construct() {
		parent::__construct();
		$this->LABEL_CREATE_ANOTHER_ACCOUNT = _('Create another host');
		$this->LABEL_BACK_TO_ACCOUNT_LIST = _('Back to host list');
	}
	
	/**
	* Returns the alias name of this account type.
	*
	* @return string alias name
	*/
	function getAlias() {
		return _("Hosts");
	}
	
	/**
	* Returns the description of this account type.
	*
	* @return string description
	*/
	function getDescription() {
		return _("Host accounts (e.g. Samba)");
	}
		
	/**
	* Returns the class name for the list object.
	*
	* @return string class name
	*/
	function getListClassName() {
		return "lamHostList";
	}
	
	/**
	* Returns the default attribute list for this account type.
	*
	* @return string attribute list
	*/
	function getDefaultListAttributes() {
		return "#cn;#description;#uidNumber;#gidNumber";
	}

	/**
	* Returns a list of attributes which have a translated description.
	* This is used for the head row in the list view.
	*
	* @return array list of descriptions
	*/
	function getListAttributeDescriptions() {
		return array (
			"uid" => _("Host name"),
			"cn" => _("Host name"),
			"rid" => _("RID (Windows UID)"),
			"description" => _("Host description"),
			"uidnumber" => _("UID number"),
			"gidnumber" => _("GID number")
			);
	}

	/**
	 * Returns the the title text for the title bar on the new/edit page.
	 *
	 * @param array $attributes list of LDAP attributes for the displayed account (null, if new account)
	 * @return String title text
	 */
	public function getTitleBarTitle($attributes) {
		if ($attributes == null) {
			return _("New host");
		}
		// check if a user name is set
		if (isset($attributes['uid'][0])) {
			return htmlspecialchars($attributes['uid'][0]);
		}
		// fall back to default
		return parent::getTitleBarTitle($attributes);
	}

	/**
	 * Returns the the title text for the title bar on the new/edit page.
	 *
	 * @param array $attributes list of LDAP attributes for the displayed account (null, if new account)
	 * @return String title text
	 */
	public function getTitleBarSubtitle($attributes) {
		if ($attributes == null) {
			return null;
		}
		// check if a common name is set
		if (isset($attributes['cn'][0])) {
			return htmlspecialchars($attributes['cn'][0]);
		}
		// check if a display name is set
		if (isset($attributes['displayName'][0])) {
			return htmlspecialchars($attributes['displayName'][0]);
		}
		// fall back to default
		return parent::getTitleBarSubtitle($attributes);
	}

}


/**
 * Generates the list view.
 *
 * @package lists
 * @author Roland Gruber
 * 
 */
class lamHostList extends lamList {

	/**
	 * Constructor
	 *
	 * @param string $type account type
	 * @return lamList list object
	 */
	function __construct($type) {
		parent::__construct($type);
		$this->labels = array(
			'nav' => _("Host count: %s"),
			'error_noneFound' => _("No hosts found!"),
			'newEntry' => _("New host"),
			'deleteEntry' => _("Delete selected hosts"));
	}

}


?>