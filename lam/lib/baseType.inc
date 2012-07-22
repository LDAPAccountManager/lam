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
	
	/** label to create another account */
	public $LABEL_CREATE_ANOTHER_ACCOUNT;
	/** label to return to account list */
	public $LABEL_BACK_TO_ACCOUNT_LIST;

	/**
	 * Creates a new type object.
	 */
	public function __construct() {
		$this->LABEL_CREATE_ANOTHER_ACCOUNT = _('Create another account');
		$this->LABEL_BACK_TO_ACCOUNT_LIST = _('Back to account list');
	}
	
	/**
	* Returns the alias name of this account type.
	* This function must be overwritten by the child classes.
	*
	* @return string alias name
	*/
	public function getAlias() {
		return "baseType";
	}
	
	/**
	* Returns the description of this account type.
	* This function must be overwritten by the child classes.
	*
	* @return string description
	*/
	public function getDescription() {
		return "base type";
	}
	
	/**
	* Returns the class name for the list object.
	*
	* @return string class name
	*/
	public function getListClassName() {
		return "lamList";
	}
	
	/**
	* Returns the default attribute list for this account type.
	* This function must be overwritten by the child classes.
	*
	* @return string attribute list
	*/
	public function getDefaultListAttributes() {
		return "dn:DN,objectClass:Object classes";
	}

	/**
	* Returns a list of attributes which have a translated description.
	* This is used for the head row in the list view.
	*
	* @return array list of descriptions
	*/
	public function getListAttributeDescriptions() {
		return array();
	}
	
	/**
	 * Returns if entries of this type may be created via file upload.
	 *
	 * @return boolean true, if file upload is supported
	 */
	public function supportsFileUpload() {
		return true;
	}
	
	/**
	 * Returns the the title text for the title bar on the new/edit page.
	 *
	 * @param accountContainer $container account container
	 * @return String title text
	 */
	public function getTitleBarTitle($container) {
		if ($container->dn_orig == null) {
			return null;
		}
		return htmlspecialchars(getAbstractDN($container->dn_orig));
	}

	/**
	 * Returns the the title text for the title bar on the new/edit page.
	 *
	 * @param accountContainer $container account container
	 * @return String title text
	 */
	public function getTitleBarSubtitle($container) {
		return null;
	}
	
	/**
	 * Returns a list of LDAP suffixes for this type.
	 * 
	 * @return array sorted list of possible suffixes for this type.
	 */
	public function getSuffixList() {
		$suffix = $_SESSION["config"]->get_Suffix(get_class($this));
		$ret = array();
		$sr = @ldap_search($_SESSION["ldap"]->server(), escapeDN($suffix), "(|(objectClass=organizationalunit)(objectClass=country)(objectClass=organization))", array("DN"), 0, 0, 0, LDAP_DEREF_NEVER);
		if ($sr) {
			$units = ldap_get_entries($_SESSION["ldap"]->server(), $sr);
			cleanLDAPResult($units);
			// extract Dns
			for ($i = 0; $i < sizeof($units); $i++) {
				if ($units[$i]['dn']) $ret[] = $units[$i]['dn'];
			}
		}
		// add root suffix if needed
		$found = false;
		for ($i = 0; $i < sizeof($ret); $i++) { // search suffix case-intensitive
			if (strtolower($suffix) == strtolower($ret[$i])) {
				$found = true;
				break;
			}
		}
		if (!$found) {
			$ret[] = $suffix;
		}
		usort($ret, 'compareDN');
		return $ret;
	}
	
	/**
	 * This function is called after the edit page is processed and before the page content is generated.
	 * This can be used to run custom handlers after each page processing.
	 * 
	 * @param accountContainer $container account container
	 */
	public function runEditPagePostAction($container) {
		
	}

}

?>