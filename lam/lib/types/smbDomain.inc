<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2005 - 2010  Roland Gruber

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
* The account type for Samba domains.
*
* @package types
* @author Roland Gruber
*/

/**
* The account type for Samba domains.
*
* @package types
*/
class smbDomain extends baseType {
	
	/**
	 * Constructs a new domain type object.
	 */
	public function __construct() {
		parent::__construct();
		$this->LABEL_CREATE_ANOTHER_ACCOUNT = _('Create another domain');
		$this->LABEL_BACK_TO_ACCOUNT_LIST = _('Back to domain list');
	}
	
	/**
	* Returns the alias name of this account type.
	*
	* @return string alias name
	*/
	function getAlias() {
		return _("Samba domains");
	}
	
	/**
	* Returns the description of this account type.
	*
	* @return string description
	*/
	function getDescription() {
		return _("Samba 3 domain entries");
	}
		
	/**
	* Returns the class name for the list object.
	*
	* @return string class name
	*/
	function getListClassName() {
		return "lamSmbDomainList";
	}
	
	/**
	* Returns the default attribute list for this account type.
	*
	* @return string attribute list
	*/
	function getDefaultListAttributes() {
		return "#sambaDomainName;#sambaSID";
	}

	/**
	* Returns a list of attributes which have a translated description.
	* This is used for the head row in the list view.
	*
	* @return array list of descriptions
	*/
	function getListAttributeDescriptions() {
		return array(
			"sambasid" => _("Domain SID"),
			"sambadomainname" => _("Domain name")
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
			return _("New domain");
		}
		// check if a domain name is set
		if (isset($attributes['sambaDomainName'][0])) {
			return htmlspecialchars($attributes['sambaDomainName'][0]);
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
		$subtitle = '';
		// check if a SID can be shown
		if (isset($attributes['sambaSID'][0])) {
			$subtitle .= htmlspecialchars($attributes['sambaSID'][0]);
		}
		if ($subtitle == '') {
			return null;
		}
		return $subtitle;
	}

}


/**
 * Generates the list view.
 *
 * @package lists
 * @author Roland Gruber
 * 
 */
class lamSmbDomainList extends lamList {

	/**
	 * Constructor
	 *
	 * @param string $type account type
	 * @return lamList list object
	 */
	function __construct($type) {
		parent::__construct($type);
		$this->labels = array(
			'nav' => _("%s domain(s) found"),
			'error_noneFound' => _("No domains found!"),
			'newEntry' => _("New domain"),
			'deleteEntry' => _("Delete domain(s)"));
	}

}

?>