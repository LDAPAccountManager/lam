<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003 - 2006  Tilo Lutz
                2005 - 2007  Roland Gruber

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
* Manages the object class "account" for users and hosts.
*
* @package modules
*
* @author Tilo Lutz
* @author Roland Gruber
* @author Michael Duergner
*/

/**
* Manages the object class "account" for users and hosts.
*
* @package modules
*/
class account extends baseModule {

	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	*/
	function get_metaData() {
		if ($_SESSION['loggedIn']) {
			$modules = $_SESSION['config']->get_AccountModules($this->get_scope());
		}
		$return = array();
		// manages host accounts
		$return["account_types"] = array("host", "user");
		// alias name
		$return["alias"] = _('Account');
		// this is a base module
		$return["is_base"] = true;
		// LDAP filter
		$return["ldap_filter"] = array('or' => "(objectClass=account)");
		// RDN attribute
		$return["RDN"] = array("uid" => "low");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('account');
		// LDAP aliases
		$return['LDAPaliases'] = array('userid' => 'uid');
		// managed attributes
		$return['attributes'] = array('uid', 'description');
		// available PDF fields
		$return['PDF_fields'] = array(
			'description'
		);
		if ($_SESSION['loggedIn'] && !in_array('posixAccount', $modules)) {
			$return['PDF_fields'][] = 'uid';
		}
		// help Entries
		$return['help'] = array (
			'host' => array(
				'uid' => array(
					"Headline" => _("Host name"),
					"Text" => _("Host name of the host which should be created. Valid characters are: a-z,0-9, .-_$. LAM does not allow a number as first character because useradd also does not allow it. LAM does not allow capital letters A-Z because it can cause several problems. Host names are always ending with $. If last character is not $ it will be added. If host name is already used host name will be expanded with a number. The next free number will be used.")
				),
				'description'	=>	array (
					"Headline" => _("Description"),
					"Text" => _("Host description. If left empty host name will be used.")
				)
			),
			'user' => array(
				'uid' => array(
					"Headline" => _("User name"),
					"Text" => _("User name of the user who should be created. Valid characters are: a-z,0-9, .-_. LAM does not allow a number as first character because useradd also does not allow it. LAM does not allow capital letters A-Z because it can cause several problems. If user name is already used user name will be expanded with a number. The next free number will be used. Warning: Older systems have problems with user names longer than 8 characters. You can not log in to Windows if user name is longer than 16 characters.")
				),
				'description'	=>	array (
					"Headline" => _("Description"),
					"Text" => _("User description. If left empty user name will be used.")
				)
			)
		);
		// upload columns
		if ($_SESSION['loggedIn']) {
			if ($this->get_scope() == 'user') {
				$return['upload_columns'][] = array(
	                'name' => 'account_hosts',
	                'description' => _('Host list'),
	                'help' => 'host'
				);
			}
			if (!in_array('posixAccount', $modules)) {
				$return['upload_columns'][] = array(
	                'name' => 'account_uid',
	                'description' => _('User name'),
	                'help' => 'uid',
	                'required' => true
				);
				$return['upload_columns'] = array(
					array(
	                'name' => 'account_description',
	                'description' => _('Description'),
	                'help' => 'description'
					)
				);
			}
		}
		return $return;
	}

	/**
	* This function fills the message array.
	*/
	function load_Messages() {
		$this->messages['uid'][0] = array('ERROR', _('User name'), _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['uid'][1] = array('ERROR', _('Account %s:') . ' posixAccount_userName', _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['uid'][2] = array('WARN', _('User name'), _('You are using a capital letters. This can cause problems because windows isn\'t case-sensitive.'));
		$this->messages['uid'][3] = array('ERROR', _('User name'), _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
	}

	/**
	* This functions returns true if all needed settings are done.
	*
	* @return boolean true if LDAP operation can be done
	*/
	function module_complete() {
		$modules = $_SESSION['config']->get_AccountModules($this->get_scope());
		if (!in_array('posixAccount', $modules) && $this->attributes['uid'][0] == '') return false;
		return true;
	}

	/**
	* Controls if the module button the account page is visible and activated.
	*
	* @return string status ("enabled", "disabled", "hidden")
	*/
	function getButtonStatus() {
		if (!$_SESSION[$this->base]->isNewAccount) {
			// check if account is based on our object class
			$objectClasses = $_SESSION[$this->base]->attributes_orig['objectClass'];
			if (is_array($objectClasses) && !in_array('account', $objectClasses)) {
				return "disabled";
			}
		}
		return "enabled";
	}

	/* This function returns an array with 4 entries:
	* array( DN1 ('add' => array($attr), 'remove' => array($attr), 'modify' => array($attr), 'lamdaemon' => array(cmds)), DN2 .... )
	* DN is the DN to change. It may be possible to change several DNs,
	* e.g. create a new user and add him to some groups via attribute memberUid
	* add are attributes which have to be added to ldap entry
	* remove are attributes which have to be removed from ldap entry
	* lamdaemon are lamdaemon commands to modify homedir, quotas, ...
	*/
	function save_attributes() {
		// skip saving if account is based on another structural object class
		if (!$_SESSION[$this->base]->isNewAccount && !in_array('account', $_SESSION[$this->base]->attributes_orig['objectClass'])) {
			return array();
		}
		// Get easy attributes
		$return = $_SESSION[$this->base]->save_module_attributes($this->attributes, $this->orig);
		// Return attributes
		return $return;
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_attributes() {
		$errors = array();
		// Load attributes
		$this->attributes['description'][0] = $_POST['description'];
		// user name if no posixAccount
		$modules = $_SESSION['config']->get_AccountModules($this->get_scope());
		if (!in_array('posixAccount', $modules)) {
			$this->attributes['uid'][0] = $_POST['uid'];
			if (!get_preg($this->attributes['uid'][0], '!upper')) $errors[] = $this->messages['uid'][2];
			if (!get_preg($this->attributes['uid'][0], 'username')) $errors[] = $this->messages['uid'][3];
		}
		return $errors;
	}

	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_attributes() {
		// user name if no posixAccount
		$modules = $_SESSION['config']->get_AccountModules($this->get_scope());
		if (!in_array('posixAccount', $modules)) {
			$return[] = array (
				0 => array('kind' => 'text', 'text' => _("User name").'*'),
				1 => array('kind' => 'input', 'name' => 'uid', 'type' => 'text', 'size' => '30', 'maxlength' => '20',
					'value' => $this->attributes['uid'][0]),
				2 => array('kind' => 'help', 'value' => 'uid'));
		}
		// description
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Description') ),
			1 => array ( 'kind' => 'input', 'name' => 'description', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $this->attributes['description'][0] ),
			2 => array ('kind' => 'help', 'value' => 'description'));
		return $return;
	}

	/**
	* Returns the PDF entries for this module.
	*
	* @return array list of possible PDF entries
	*/
	function get_pdfEntries() {
		$return = array();
		$return['account_description'] = array('<block><key>' . _('Description') . '</key><value>' . $this->attributes['description'][0] . '</value></block>');
		$return['account_uid'] = array('<block><key>' . _('User name') . '</key><value>' . $this->attributes['uid'][0] . '</value></block>');
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
			// add object class
			if (!in_array("account", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "account";
			$modules = $_SESSION['config']->get_AccountModules($this->get_scope());
			if (!in_array('posixAccount', $modules)) {
				// description
				if ($rawAccounts[$i][$ids['account_description']] && ($rawAccounts[$i][$ids['account_description']] != '')) {
					$partialAccounts[$i]['description'] = $rawAccounts[$i][$ids['account_description']];
				}
				else {
					$partialAccounts[$i]['description'] = $rawAccounts[$i][$ids['account_uid']];
				}
				// user name
				if (get_preg($rawAccounts[$i][$ids['account_uid']], 'username')) {
					$partialAccounts[$i]['uid'] = $rawAccounts[$i][$ids['account_uid']];
				}
				else {
					$errMsg = $this->messages['uid'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
		}
		return $messages;
	}

}

?>
