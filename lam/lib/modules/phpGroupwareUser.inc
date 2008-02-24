<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2008  Roland Gruber

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
* Manages phpGroupware users.
*
* @package modules
* @author Roland Gruber
*/

/**
* Manages phpGroupware users.
*
* @package modules
*/
class phpGroupwareUser extends baseModule {
	
	/**
	* Creates a new kolabUser object.
	*
	* @param string $scope account type (user, group, host)
	*/
	public function __construct($scope) {
		parent::__construct($scope);
		$this->autoAddObjectClasses = false;
	}
	
	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	* 
	* @see baseModule::get_metaData()
	*/
	public function get_metaData() {
		$return = array();
		// icon
		$return['icon'] = 'phpGroupware.png';
		// manages host accounts
		$return["account_types"] = array("user");
		// alias name
		$return["alias"] = "phpGroupWare";
		// module dependencies
		$return['dependencies'] = array('depends' => array('posixAccount'), 'conflicts' => array());
		// LDAP filter
		$return["ldap_filter"] = array('or' => "(objectClass=phpgwAccount)");
		// managed object classes
		$return['objectClasses'] = array('phpgwAccount');
		// managed attributes
		$return['attributes'] = array('phpgwAccountID', 'phpgwAccountStatus', 'phpgwAccountExpires',
			'phpgwLastPasswordChange', 'phpgwLastLoginFrom', 'phpgwLastLogin');
		// help Entries
		$return['help'] = array(
			'extension' => array(
				"Headline" => _("Adds the phpGroupWare extension"),
				"Text" => _("If you set this to true then the phpGroupware extension will be added.")
			)
		);
		// upload dependencies
		$return['upload_preDepends'] = array('posixAccount');
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'phpGroupwareUser_extension',
				'description' => _('Adds the phpGroupWare extension'),
				'help' => 'extension',
				'example' => 'true',
				'values' => 'true, false'
			)
		);
		return $return;
	}
	
	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return array HTML meta data
	 */
	public function display_html_attributes() {
		$return = array();
		if (isset($this->attributes['objectClass']) && in_array('phpgwAccount', $this->attributes['objectClass'])) {
			$statusSelected = array();
			$return[] = array(
				array('kind' => 'select', 'submit', 'name' => 'phpgwAccountStatus',
					'options' => array(array('A', _('active')), array('I', _('inactive'))),
					'options_selected' => array($this->attributes['phpgwAccountStatus'][0]), 'descriptiveOptions' => true)
			);
			$return[] = array(
				array('kind' => 'text', 'value' => '')
			);
			$return[] = array(
				array('kind' => 'input', 'type' => 'submit', 'name' => 'form_subpage_phpGroupwareUser_attributes_remObjectClass', 'value' => _('Remove phpGroupWare extension'))
			);
		}
		else {
			$return[] = array(
				array('kind' => 'input', 'type' => 'submit', 'name' => 'form_subpage_phpGroupwareUser_attributes_addObjectClass', 'value' => _('Add phpGroupWare extension'))
			);			
		}
		return $return;
	}
	
	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	public function process_attributes() {
		if (isset($_POST['form_subpage_phpGroupwareUser_attributes_addObjectClass'])) {
			$this->attributes['objectClass'][] = 'phpgwAccount';
		}
		elseif (isset($_POST['form_subpage_phpGroupwareUser_attributes_remObjectClass'])) {
			for ($i = 0; $i < sizeof($this->attributes['objectClass']); $i++) {
				if ($this->attributes['objectClass'][$i] == 'phpgwAccount') {
					unset($this->attributes['objectClass'][$i]);
					break;
				}
			}
		}
		if (isset($this->attributes['objectClass']) && in_array('phpgwAccount', $this->attributes['objectClass'])) {
			$this->attributes['phpgwAccountStatus'][0] = $_POST['phpgwAccountStatus'];
		}
		return array();
	}

	
	/**
	* Returns a list of modifications which have to be made to the LDAP account.
	*
	* @return array list of modifications
	* <br>This function returns an array with 3 entries:
	* <br>array( DN1 ('add' => array($attr), 'remove' => array($attr), 'modify' => array($attr)), DN2 .... )
	* <br>DN is the DN to change. It may be possible to change several DNs (e.g. create a new user and add him to some groups via attribute memberUid)
	* <br>"add" are attributes which have to be added to LDAP entry
	* <br>"remove" are attributes which have to be removed from LDAP entry
	* <br>"modify" are attributes which have to been modified in LDAP entry
	*/
	function save_attributes() {
		if (!in_array('phpgwAccount', $this->attributes['objectClass'])) {
			return parent::save_attributes();
		}
		// set phpgwAccountID to UID number for new accounts
		$attrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
		$this->attributes['phpgwAccountID'][0] = $attrs['uidNumber'][0];
		return parent::save_attributes();
	}
	
	/**
	 * Allows the module to run commands after the LDAP entry is changed or created.
	 * 
	 * Calling this method requires the existence of an enclosing {@link accountContainer}.
	 *
	 * @param boolean $newAccount new account
	 */
	public function postModifyActions($newAccount) {
		// check if extension was removed
		if (!$newAccount &&
			(in_array('phpgwAccount', $this->orig['objectClass']) && !in_array('phpgwAccount', $this->attributes['objectClass']))) {
			$dn = $this->getAccountContainer()->finalDN;
			$attributes = array_merge(array('objectClass'), $this->meta['attributes']);
			$sr = @ldap_read($_SESSION['ldap']->server(), $dn, 'objectClass=*', $attributes);
			if (!$sr) {
				StatusMessage('ERROR', sprintf(_('Was unable to remove attribtues from DN: %s.'), $dn), ldap_error($_SESSION['ldap']->server()));				
				return;
			}
			$entry = ldap_get_entries($_SESSION['ldap']->server(), $sr);
			$newAttributes = array();
			$newAttributes['objectclass'] = $entry[0]['objectclass'];
			unset($newAttributes['objectclass']['count']);
			for ($i = 0; $i < sizeof($newAttributes['objectclass']); $i++) {
				if ($newAttributes['objectclass'][$i] == 'phpgwAccount') {
					unset($newAttributes['objectclass'][$i]);
					break;
				}
			}
			for ($i = 0; $i < sizeof($this->meta['attributes']); $i++) {
				if (isset($entry[0][strtolower($this->meta['attributes'][$i])])) {
					$newAttributes[$this->meta['attributes'][$i]] = array();
				}
			}
			$success = @ldap_mod_replace($_SESSION['ldap']->server(), $dn, $newAttributes);
			if (!$success) {
				StatusMessage('ERROR', sprintf(_('Was unable to remove attribtues from DN: %s.'), $dn), ldap_error($_SESSION['ldap']->server()));
			}
		}
		return;
	}

	/**
	* In this function the LDAP account is built up.
	*
	* @param array $rawAccounts list of hash arrays (name => value) from user input
	* @param array $partialAccounts list of hash arrays (name => value) which are later added to LDAP
	* @param array $ids list of IDs for column position (e.g. "posixAccount_uid" => 5)
	* @return array list of error messages if any
	*/
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts) {
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			if (!isset($rawAccounts[$i][$ids['phpGroupwareGroup_extension']]) 
				|| !(strtolower($rawAccounts[$i][$ids['phpGroupwareGroup_extension']]) == "true")) {
					continue;
			}
			$partialAccounts[$i]['objectClass'][] = 'phpgwAccount';
			$partialAccounts[$i]['phpgwAccountID'][0] = $partialAccounts[$i]['uidNumber'];
		}
		return array();
	}

}

?>