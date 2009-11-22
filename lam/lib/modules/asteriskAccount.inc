<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2009  Pavel Pozdnyak
                2009  Roland Gruber

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
 * Description of asteriskAccount
 *
 * @author Pavel Pozdnyak
 * @author Roland Gruber
 */
class asteriskAccount extends baseModule implements passwordService {

	/**
	* Creates a new asteriskAccount object.
	*
	* @param string $scope account type (user, group, host)
	*/
	function __construct($scope) {
		// call parent constructor
		parent::__construct($scope);
		$this->autoAddObjectClasses = false;
	}

	/**
	 * Returns meta data that is interpreted by parent class
	 *
	 * @return array array with meta data
	 */
	function get_metaData() {
		$return = array();	
		// manages users accounts
		$return["account_types"] = array("user");
		$return["is_base"] = false;
		// alias name
		$return["alias"] = _("Asterisk");
		// module dependencies
		$return['dependencies'] = array('depends' => array('inetOrgPerson'), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('AsteriskSIPUser');
		// managed attributes
		$return['attributes'] = array('AstAccountCallerID', 'AstAccountHost',
			'AstAccountRealmedPassword', 'AstAccountContext');
		// icon
		$return['icon'] = 'asterisk.png';
		// help
		$return['help'] = array(
			'AstAccountCallerID' => array(
			"Headline" => _("Caller ID"),
			"Text" => _("This is the ID of the user in the Asterisk database. It may contain digits and letters (e.g. user1 or 200134).")
			),
			'AstAccountHost' => array(
			"Headline" => _("Host"),
			"Text" => _("This is the machine id (e.g. IP address or host name) from which the user can call/receive calls.")
			),
			'AstAccountContext' => array(
			"Headline" => _("Account context"),
			"Text" => _("The account context stores information about the dial plan.")
			),
			'AstAccountRealmedPassword' => array(
				"Headline" => _("Password"),
				"Text" => _("Please enter the password which you want to set for this account.")
			),
			);
		// profile options
		$return['profile_options'] = array(
			array(
			array('kind' => 'text', 'text' => _('Host') . ":"),
			array('kind' => 'input', 'name' => 'asteriskAccount_AstAccountHost', 'type' => 'text', 'size' => '30'),
			array('kind' => 'help', 'value' => 'AstAccountHost')),
			array(
			array('kind' => 'text', 'text' => _('Account context') . ":"),
			array('kind' => 'input', 'name' => 'asteriskAccount_AstAccountContext', 'type' => 'text', 'size' => '30'),
			array('kind' => 'help', 'value' => 'AstAccountContext'))
		);
		// profile mappings
		$return['profile_mappings'] = array(
			'asteriskAccount_AstAccountHost' => 'AstAccountHost',
			'asteriskAccount_AstAccountContext' => 'AstAccountContext'
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'AstAccountCallerID', 'AstAccountContext', 'AstAccountHost',
		);
		// upload dependencies
		$return['upload_preDepends'] = array('posixAccount', 'inetOrgPerson');
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'asteriskAccount_AstAccountCallerID',
				'description' => _('Caller ID'),
				'help' => 'AstAccountCallerID',
				'example' => '12345',
				'required' => true
			),
			array(
				'name' => 'asteriskAccount_AstAccountContext',
				'description' => _('Account context'),
				'help' => 'AstAccountContext',
				'example' => _('default'),
				'required' => true
			),
			array(
				'name' => 'asteriskAccount_AstAccountHost',
				'description' => _('Host'),
				'help' => 'AstAccountHost',
				'example' => 'dynamic',
				'default' => 'dynamic',
			),
			array(
				'name' => 'asteriskAccount_AstAccountRealmedPassword',
				'description' => _('Password'),
				'help' => 'AstAccountRealmedPassword',
				'example' => _('secret'),
			),
		);
		return $return;
	}

	/**
	 * This function fills the error message array with messages
	 */
	function load_Messages() {
		$this->messages['AstAccountCallerID'][0] = array('ERROR', _('Please enter a caller ID.'));
		$this->messages['AstAccountCallerID'][1] = array('ERROR', _('The caller ID format is invalid.'));
		$this->messages['AstAccountCallerID'][2] = array('ERROR', _('There is already another user with this caller ID.'));
		$this->messages['AstAccountCallerID'][3] = array('ERROR', _('Account %s:') . ' asteriskAccount_AstAccountCallerID', _('The caller ID format is invalid.'));
		$this->messages['AstAccountContext'][0] = array('ERROR', _('Please enter the extension context.'));
		$this->messages['AstAccountContext'][1] = array('ERROR', _('The extension context is invalid.'));
		$this->messages['AstAccountContext'][2] = array('ERROR', _('Account %s:') . ' asteriskAccount_AstAccountContext', _('The extension context is invalid.'));
		$this->messages['AstAccountHost'][0] = array('ERROR', _('Please enter the host name.'));
		$this->messages['AstAccountHost'][1] = array('ERROR', _('The host name is invalid.'));
		$this->messages['AstAccountHost'][2] = array('ERROR', _('Account %s:') . ' asteriskAccount_AstAccountHost', _('The host name is invalid.'));
	}

	/**
	 * This function will create the meta HTML code to show a page with all attributes.
	 */
	function display_html_attributes() {
		if (isset($_POST['form_subpage_' . get_class($this) . '_attributes_addObjectClass'])) {
			$this->attributes['objectClass'][] = 'AsteriskSIPUser';
		}

		$return = array();
		if (in_array('AsteriskSIPUser', $this->attributes['objectClass'])) {
			$return[] = array(
				array('kind' => 'text', 'text' => _("Caller ID").'*'),
				array('kind' => 'input', 'name' => 'AstAccountCallerID', 'type' => 'text', 'size' => '30',  'value' => $this->attributes['AstAccountCallerID'][0]),
				array('kind' => 'help', 'value' => 'AstAccountCallerID'));
			$return[] = array(
				array('kind' => 'text', 'text' => _("Host").'*'),
				array('kind' => 'input', 'name' => 'AstAccountHost', 'type' => 'text', 'size' => '30',  'value' => $this->attributes['AstAccountHost'][0]),
				array('kind' => 'help', 'value' => 'AstAccountHost'));

			$return[] = array(array('kind' => 'text', 'td' => array('colspan' => 3)));

			$return[] = array(
				array('kind' => 'text', 'text' => _("Account context").'*'),
				array('kind' => 'input', 'name' => 'AstAccountContext', 'type' => 'text', 'size' => '30',  'value' => $this->attributes['AstAccountContext'][0]),
				array('kind' => 'help', 'value' => 'AstAccountContext'));
		}else {
			$return[] = array(
				array('kind' => 'text', 'text' => '&nbsp;'),
				array('kind' => 'input', 'type' => 'submit', 'name' => 'form_subpage_' . get_class($this) . '_attributes_addObjectClass', 'value' => _('Add Asterisk account'))
			);
		}
		return $return;
	}

	/**
	 * Write variables into object and do some regex checks
	 */
	function process_attributes() {
		if (!in_array('AsteriskSIPUser', $this->attributes['objectClass'])) {
			return array();
		}
		$errors = array();
		$this->attributes['AstAccountCallerID'] = array();
		$this->attributes['AstAccountHost'] = array();
		$this->attributes['AstAccountContext'] = array();
		if (isset($_POST['AstAccountCallerID'])) {
			$this->attributes['AstAccountCallerID'][0] = $_POST['AstAccountCallerID'];
			// check if caller ID is empty
			if($this->attributes['AstAccountCallerID'][0] == '') {
				$errors[] = $this->messages['AstAccountCallerID'][0];
			}
			// check format
			else if (!get_preg($this->attributes['AstAccountCallerID'][0], 'username')) {
				$errors[] = $this->messages['AstAccountCallerID'][1];
			}
			// check for duplicate caller ID
			else if (!isset($this->orig['AstAccountCallerID'][0]) || (($this->orig['AstAccountCallerID'][0] != $this->attributes['AstAccountCallerID'][0]))) {
				$searchroot =  $_SESSION['config']->get_Suffix('user');
				$filter = '(& (objectClass=AsteriskSIPUser) (AstAccountCallerID=' . $this->attributes['AstAccountCallerID'][0] .'))';
				$ldapc =  $_SESSION['ldap']->server();
				$attributes = array("DN", "AstAccountCallerID");
				$res = ldap_search($ldapc,$searchroot,$filter,$attributes,0,0,0,LDAP_DEREF_NEVER);
				if (!$res) {
					return array(array("ERROR", _("Unable to load LDAP entry:") . " " .$searchroot, ldap_error($_SESSION['ldap']->server())));
				}
				$count = ldap_count_entries($ldapc, $res);
				if ($count > 0) {
					$errors[] = $this->messages['AstAccountCallerID'][2];
				}
			}
		}
		if (isset($_POST['AstAccountHost'])) {
			$this->attributes['AstAccountHost'][0] = $_POST['AstAccountHost'];
			if($this->attributes['AstAccountHost'][0] == '') {
				$errors[] = $this->messages['AstAccountHost'][0];
			}
			elseif (!get_preg($this->attributes['AstAccountHost'][0], 'hostname')) {
				$errors[] = $this->messages['AstAccountHost'][1];
			}
		}
		if (isset($_POST['AstAccountContext'])) {
			$this->attributes['AstAccountContext'][0] = $_POST['AstAccountContext'];
			if($this->attributes['AstAccountContext'][0] == '') {
				$errors[] = $this->messages['AstAccountContext'][0];
			}
			elseif (!get_preg($this->attributes['AstAccountContext'][0], 'username')) {
				$errors[] = $this->messages['AstAccountContext'][1];
			}
		}
		return $errors;
	}

	/**
	* Returns a list of PDF entries
	*/
	function get_pdfEntries() {
		$return = array();
		$return[get_class($this) . '_AstAccountCallerID'] = array('<block><key>' . _('Caller ID') . '</key><value>' . $this->attributes['AstAccountCallerID'][0] . '</value></block>');
		$return[get_class($this) . '_AstAccountContext'] = array('<block><key>' . _('Account context') . '</key><value>' . $this->attributes['AstAccountContext'][0] . '</value></block>');
		$return[get_class($this) . '_AstAccountHost'] = array('<block><key>' . _('Host') . '</key><value>' . $this->attributes['AstAccountHost'][0] . '</value></block>');
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
			if (!in_array("AsteriskSIPUser", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "AsteriskSIPUser";
			// add account caller id
			if (get_preg($rawAccounts[$i][$ids['asteriskAccount_AstAccountCallerID']], 'username')) {
				$partialAccounts[$i]['AstAccountCallerID'] = $rawAccounts[$i][$ids['asteriskAccount_AstAccountCallerID']];
			}
			else {
				$errMsg = $this->messages['AstAccountCallerID'][3];
				array_push($errMsg, array($i));
				$messages[] = $errMsg;
			}
			// add host
			if ($rawAccounts[$i][$ids['asteriskAccount_AstAccountHost']] == "") {
				// default value
				$partialAccounts[$i]['AstAccountHost'] = 'dynamic';
			}
			elseif (get_preg($rawAccounts[$i][$ids['asteriskAccount_AstAccountHost']], 'realname')) {
				$partialAccounts[$i]['AstAccountHost'] = $rawAccounts[$i][$ids['asteriskAccount_AstAccountHost']];
			}
			else {
				$errMsg = $this->messages['AstAccountHost'][2];
				array_push($errMsg, array($i));
				$messages[] = $errMsg;
			}
			//add context
			if (($rawAccounts[$i][$ids['asteriskAccount_AstAccountContext']] != "") && (get_preg($rawAccounts[$i][$ids['asteriskAccount_AstAccountContext']], 'realname')) ) {
				$partialAccounts[$i]['AstAccountContext'] = $rawAccounts[$i][$ids['asteriskAccount_AstAccountContext']];
			}
			else {
				$errMsg = $this->messages['AstAccountContext'][2];
				array_push($errMsg, array($i));
				$messages[] = $errMsg;
			}
			//add password
			if ($rawAccounts[$i][$ids['asteriskAccount_AstAccountRealmedPassword']] != "") {
				$partialAccounts[$i]['AstAccountRealmedPassword'] = $this->hashPassword($rawAccounts[$i][$ids['asteriskAccount_AstAccountRealmedPassword']]);
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
		return true;
	}

	/**
	 * This function is called whenever the password should be changed. Account modules
	 * must change their password attributes only if the modules list contains their module name.
	 *
	 * @param String $password new password
	 * @param $modules list of modules for which the password should be changed
	 * @return array list of error messages if any as parameter array for StatusMessage
	 *			   e.g. return arrray(array('ERROR', 'Password change failed.'))
	 * @see passwordService::passwordChangeRequested
	 */
	public function passwordChangeRequested($password, $modules) {
		if (!in_array(get_class($this), $modules)) {
			return array();
		}
		$this->attributes['AstAccountRealmedPassword'][0] = $this->hashPassword($password);
		return array();
	}
	
	/**
	 * Hashes a password value to Asterisk format.
	 *
	 * @param String $password password
	 * @return String hash
	 */
	private function hashPassword($password) {
		return base64_encode(hex2bin(md5($password)));
	}

}

?>
