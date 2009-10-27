<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
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
class phpGroupwareUser extends baseModule implements passwordService {
	
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
				"Headline" => _("Add phpGroupWare extension"),
				"Text" => _("If you set this to \"true\" then the phpGroupware extension will be added.")
			),
			'phpgwAccountStatus' => array(
				"Headline" => _("Account status"),
				"Text" => _("Here you can specify if the account is active or inactive.")
			),
			'phpgwAccountExpires' => array(
				"Headline" => _("Account expiration date"),
				"Text" => _("This is the date when the account will expire. Format: DD-MM-YYYY")
			)
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'phpgwAccountStatus', 'phpgwAccountExpires',
			'phpgwLastLoginFrom', 'phpgwLastLogin'
		);
		// upload dependencies
		$return['upload_preDepends'] = array('posixAccount');
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'phpGroupwareUser_extension',
				'description' => _('Add phpGroupWare extension'),
				'help' => 'extension',
				'example' => 'true',
				'values' => 'true, false'
			),
			array(
				'name' => 'phpGroupwareUser_accountStatus',
				'description' => _('Account status'),
				'help' => 'phpgwAccountStatus',
				'example' => 'active',
				'values' => 'active, inactive'
			),
			array(
				'name' => 'phpGroupwareUser_accountExpires',
				'description' => _('Account expiration date'),
				'help' => 'phpgwAccountExpires',
				'example' => '23-07-2011'
			)
		);
		return $return;
	}
	
	/**
	* This function builds up the message array.
	*/
	function load_Messages() {
		// error messages for input checks
		$this->messages['phpgwAccountStatus'][0] = array('ERROR', _('Account %s:') . ' phpGroupwareUser_accountStatus', _('Please enter "active" or "inactive".'));
		$this->messages['phpgwAccountExpires'][0] = array('ERROR',  _('Account %s:') . ' phpGroupwareUser_accountExpires', _('The expiration date is invalid.'));
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return array HTML meta data
	 */
	public function display_html_attributes() {
		$return = array();
		if (isset($this->attributes['objectClass']) && in_array('phpgwAccount', $this->attributes['objectClass'])) {
			$phpgwAccountExpires = '';
			if (isset($this->attributes['phpgwAccountExpires'][0]) && ($this->attributes['phpgwAccountExpires'][0] != "-1")) {
				$date = getdate($this->attributes['phpgwAccountExpires'][0]);
				$phpgwAccountExpires = $date['mday'] . '.' . $date['mon'] . '.' . $date['year'];
			}
			$return[] = array(
				array('kind' => 'text', 'text' => _('Account expiration date')),
				array('kind' => 'table', 'value' => array(array(
					array('kind' => 'text', 'text' => $phpgwAccountExpires),
					array('kind' => 'input', 'type' => 'submit', 'name' => 'form_subpage_phpGroupwareUser_time_phpgwAccountExpires', 'value' => _('Change'))
				)))
			);
			$return[] = array(
				array('kind' => 'text', 'text' => _('Account status')),
				array('kind' => 'select', 'submit', 'name' => 'phpgwAccountStatus',
					'options' => array(array('A', _('active')), array('I', _('inactive'))),
					'options_selected' => array($this->attributes['phpgwAccountStatus'][0]), 'descriptiveOptions' => true)
			);
			$phpgwLastLogin = '';
			if (isset($this->attributes['phpgwLastLogin'][0])) {
				$date = getdate($this->attributes['phpgwLastLogin'][0]);
				$phpgwLastLogin = $date['mday'] . '.' . $date['mon'] . '.' . $date['year'];
			}
			$return[] = array(
				array('kind' => 'text', 'text' => _('Last login')),
				array('kind' => 'text', 'text' => $phpgwLastLogin)
			);
			$phpgwLastLoginFrom = '';
			if (isset($this->attributes['phpgwLastLoginFrom'][0])) {
				$phpgwLastLoginFrom = $this->attributes['phpgwLastLoginFrom'][0];
			}
			$return[] = array(
				array('kind' => 'text', 'text' => _('Last login from')),
				array('kind' => 'text', 'text' => $phpgwLastLoginFrom)
			);
			$return[] = array(
				array('kind' => 'text', 'text' => '')
			);
			$return[] = array(
				array('kind' => 'text', 'text' => '')
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
		if (isset($this->attributes['objectClass']) && in_array('phpgwAccount', $this->attributes['objectClass'])) {
			$this->attributes['phpgwAccountStatus'][0] = $_POST['phpgwAccountStatus'];
		}
		if (isset($_POST['form_subpage_phpGroupwareUser_attributes_addObjectClass'])) {
			$this->attributes['objectClass'][] = 'phpgwAccount';
			$this->attributes['phpgwAccountExpires'][0] = "-1";
			$this->attributes['phpgwLastPasswordChange'][0] = time();
		}
		elseif (isset($_POST['form_subpage_phpGroupwareUser_attributes_remObjectClass'])) {
			for ($i = 0; $i < sizeof($this->attributes['objectClass']); $i++) {
				if ($this->attributes['objectClass'][$i] == 'phpgwAccount') {
					unset($this->attributes['objectClass'][$i]);
					break;
				}
			}
		}
		return array();
	}

	
	/**
	* This function will create the meta HTML code to show a page to change time values.
	*
	* @return array meta HTML code
	*/
	function display_html_time() {
		$return = array();
		// determine attribute
		if (isset($_POST['form_subpage_phpGroupwareUser_time_phpgwAccountExpires'])) {
			$attr = 'phpgwAccountExpires';
			$text = _('Account expiration date');
			$help = "phpgwAccountExpires";
		}
		$time = time();
		if (isset($this->attributes[$attr][0]) && ($this->attributes[$attr][0] != "-1")) {
			$time = $this->attributes[$attr][0];
		}
		$date = getdate($time);
		for ( $i=1; $i<=31; $i++ ) $mday[] = $i;
		for ( $i=1; $i<=12; $i++ ) $mon[] = $i;
		for ( $i=2003; $i<=2030; $i++ ) $year[] = $i;
		$return[] = array(
			array('kind' => 'text', 'text' => $text),
			array('kind' => 'table', 'value' => array(array(
				array('kind' => 'select', 'name' => 'expire_day', 'options' => $mday, 'options_selected' => $date['mday']),
				array('kind' => 'select', 'name' => 'expire_mon', 'options' => $mon, 'options_selected' => $date['mon']),
				array('kind' => 'select', 'name' => 'expire_yea', 'options' => $year, 'options_selected' => $date['year'])))),
			array('kind' => 'help', 'value' => $help));
		$buttons = array();
		$buttons[] = array('kind' => 'input', 'name' => 'form_subpage_phpGroupwareUser_attributes_change' . $attr, 'type' => 'submit', 'value' => _('Change'));
		if (isset($this->attributes[$attr][0])) {
			$buttons[] = array('kind' => 'input', 'name' => 'form_subpage_phpGroupwareUser_attributes_del' . $attr, 'type' => 'submit', 'value' => _('Remove'));
		}
		$buttons[] = array('kind' => 'input', 'name' => 'form_subpage_phpGroupwareUser_attributes_back' . $attr, 'type' => 'submit', 'value' => _('Cancel'));
		$return[] = array(
			array('kind' => 'table', 'td' => array('colspan' => 3), 'value' => array($buttons))
		);
		return $return;	
	}

	/**
	* Processes user input of the time selection page.
	*
	* @return array list of info/error messages
	*/
	function process_time() {
		$return = array();
		// find button name
		$buttonName = '';
		$postKeys = array_keys($_POST);
		for ($i = 0; $i < sizeof($postKeys); $i++) {
			if (strpos($postKeys[$i], 'form_subpage_phpGroupwareUser_attributes_') !== false) {
				$buttonName = $postKeys[$i];
			}
		}
		if (($buttonName == '') || (strpos($buttonName, '_back') !== false)) return array();
		// get attribute name
		$attr = '';
		if (strpos($buttonName, 'phpgwAccountExpires') !== false) {
			$attr = 'phpgwAccountExpires';
		}
		if ($attr == '') return array();
		// determine action
		if (strpos($buttonName, '_change') !== false) {
			// set new time
			$this->attributes[$attr][0] = gmmktime(0, 0, 0, intval($_POST['expire_mon']), intval($_POST['expire_day']),
				intval($_POST['expire_yea']));
		}
		elseif (strpos($buttonName, '_del') !== false) {
			// remove attribute value
			$this->attributes[$attr][0] = "-1";
		}
		return $return;
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
	 * @see baseModule::postModifyActions()
	 *
	 * @param boolean $newAccount
	 * @param array $attributes LDAP attributes of this entry
	 */
	public function postModifyActions($newAccount, $attributes) {
		// check if extension was removed
		if (!$newAccount &&
			(in_array('phpgwAccount', $this->orig['objectClass']) && !in_array('phpgwAccount', $this->attributes['objectClass']))) {
			$dn = $this->getAccountContainer()->finalDN;
			$myattributes = array_merge(array('objectClass'), $this->meta['attributes']);
			$sr = @ldap_read($_SESSION['ldap']->server(), $dn, 'objectClass=*', $myattributes);
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
	* Returns the PDF entries for this module.
	*
	* @return array list of possible PDF entries
	*/
	function get_pdfEntries() {
		$return = array();
		if (isset($this->attributes['phpgwAccountStatus'][0])) {
			if ($this->attributes['phpgwAccountStatus'][0] == 'A') {
				$status = _('active');
			}
			else {
				$status = _('inactive');
			}
			$return['phpGroupwareUser_phpgwAccountStatus'] = array('<block><key>' . _('Account status') . '</key><value>' . $status . '</value></block>');
		}
		if (isset($this->attributes['phpgwAccountExpires'][0])) {
			$date = getdate($this->attributes['phpgwAccountExpires'][0]);
			$phpgwAccountExpires = $date['mday'] . '.' . $date['mon'] . '.' . $date['year'];
			$return['phpGroupwareUser_phpgwAccountExpires'] = array('<block><key>' . _('Account expiration date') . '</key><value>' . $phpgwAccountExpires . '</value></block>');
		}
		if (isset($this->attributes['phpgwLastLoginFrom'][0])) {
			$return['phpGroupwareUser_phpgwLastLoginFrom'] = array('<block><key>' . _('Last login from') . '</key><value>' . $this->attributes['phpgwLastLoginFrom'][0] . '</value></block>');
		}
		if (isset($this->attributes['phpgwLastLogin'][0])) {
			$date = getdate($this->attributes['phpgwLastLogin'][0]);
			$phpgwLastLogin = $date['mday'] . '.' . $date['mon'] . '.' . $date['year'];
			$return['phpGroupwareUser_phpgwLastLogin'] = array('<block><key>' . _('Last login') . '</key><value>' . $phpgwLastLogin . '</value></block>');
		}
		return $return;
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
		$messages = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			if (!isset($rawAccounts[$i][$ids['phpGroupwareUser_extension']]) 
				|| !(strtolower($rawAccounts[$i][$ids['phpGroupwareUser_extension']]) == "true")) {
					continue;
			}
			$partialAccounts[$i]['objectClass'][] = 'phpgwAccount';
			$partialAccounts[$i]['phpgwAccountID'][0] = $partialAccounts[$i]['uidNumber'];
			$partialAccounts[$i]['phpgwLastPasswordChange'] = array(time());
			// account status
			if ($rawAccounts[$i][$ids['phpGroupwareUser_accountStatus']] != '') {
				$status = $rawAccounts[$i][$ids['phpGroupwareUser_accountStatus']];
				if (($status == 'active') || ($status == 'inactive')) {
					$partialAccounts[$i]['phpgwAccountStatus'] = array($status);
				}
				else {
					$errMsg = $this->messages['phpgwAccountStatus'][0];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			// expiration date
			if ($rawAccounts[$i][$ids['phpGroupwareUser_accountExpires']] != '') {
				if (get_preg($rawAccounts[$i][$ids['phpGroupwareUser_accountExpires']], 'date')) {
					$parts = explode('-', $rawAccounts[$i][$ids['phpGroupwareUser_accountExpires']]);
					$partialAccounts[$i]['phpgwAccountExpires'] = mktime(0, 0, 0, intval($parts[1]), intval($parts[0]), intval($parts[2]));
				}
				else {
					$errMsg = $this->messages['phpgwAccountExpires'][0];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			else {
				$partialAccounts[$i]['phpgwAccountExpires'] = "-1";
			}
		}
		return $messages;
	}

	/**
	 * This method specifies if a module manages password attributes.
	 * @see passwordService::managesPasswordAttributes
	 *
	 * @return boolean true if this module manages password attributes
	 */
	public function managesPasswordAttributes() {
		// only listen to password changes
		return false;
	}

	/**
	 * This function is called whenever the password should be changed. Account modules
	 * must change their password attributes only if the modules list contains their module name.
	 *
	 * @param String $password new password
	 * @param $modules list of modules for which the password should be changed
	 * @return array list of error messages if any as parameter array for StatusMessage
	 *               e.g. return arrray(array('ERROR', 'Password change failed.'))
	 * @see passwordService::passwordChangeRequested
	 */
	public function passwordChangeRequested($password, $modules) {
		// update password timestamp when Unix password was updated
		if (!in_array('posixAccount', $modules)) {
			return array();
		}
		$this->attributes['phpgwLastPasswordChange'][0] = time();
		return array();
	}
		
}

?>