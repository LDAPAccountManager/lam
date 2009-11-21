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
			)
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
		return $return;
	}

	/**
	 * This function fills the error message array with messages
	 */
	function load_Messages() {
		$this->messages['AstAccountCallerID'][0] = array('ERROR', _('Please enter a caller ID.'));
		$this->messages['AstAccountCallerID'][1] = array('ERROR', _('The caller ID format is invalid.'));
		$this->messages['AstAccountCallerID'][2] = array('ERROR', _('There is already another user with this caller ID.'));
		$this->messages['AstAccountContext'][0] = array('ERROR', _('Please enter the extension context.'));
		$this->messages['AstAccountHost'][0] = array('ERROR', _('Please enter the host name.'));
	}

	/**
	 * This function will create the meta HTML code to show a page with all attributes.
	 *
	 * @param array $_POST HTTP-POST values
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
	 *
	 * @param array $_POST HTTP-POST values
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
			if($this->attributes['AstAccountCallerID'][0] == '') {
				$errors[] = $this->messages['AstAccountCallerID'][0];
			}
			if (!isset($this->orig['AstAccountCallerID'][0]) || (($this->orig['AstAccountCallerID'][0] != $this->attributes['AstAccountCallerID'][0]))) {
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
		}
		if (isset($_POST['AstAccountContext'])) {
			$this->attributes['AstAccountContext'][0] = $_POST['AstAccountContext'];
			if($this->attributes['AstAccountContext'][0] == '') {
				$errors[] = $this->messages['AstAccountContext'][0];
			}
		}
		return $errors;
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
			// add asterisk account caller id
			// check format
			if (get_preg($rawAccounts[$i][$ids['AstAccountCallerID']], 'realname')) {
				$partialAccounts[$i]['AstAccountCallerID'] = $rawAccounts[$i][$ids['AstAccountCallerID']];
			}
			else {
				$errMsg = $this->messages['AstAccountCallerID'][1];
				array_push($errMsg, array($i));
				$messages[] = $errMsg;
			}
			// add AstAccountHost
			if (($rawAccounts[$i][$ids['AstAccountHost']] != "") && (get_preg($rawAccounts[$i][$ids['AstAccountHost']], 'realname')) ) {
				$partialAccounts[$i]['AstAccountHost'] = $rawAccounts[$i][$ids['AstAccountHost']];
			}
			//add AstAccountContext
			if (($rawAccounts[$i][$ids['AstAccountContext']] != "") && (get_preg($rawAccounts[$i][$ids['AstAccountContext']], 'realname')) ) {
				$partialAccounts[$i]['AstAccountContext'] = $rawAccounts[$i][$ids['AstAccountContext']];
			}
			//add AstAccountRealmedPassword
			if (($rawAccounts[$i][$ids['AstAccountRealmedPassword']] != "") && (get_preg($rawAccounts[$i][$ids['AstAccountRealmedPassword']], 'password')) ) {
				$partialAccounts[$i]['AstAccountRealmedPassword'] = $rawAccounts[$i][$ids['AstAccountRealmedPassword']];
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
		$this->attributes['AstAccountRealmedPassword'][0] = base64_encode(hex2bin(md5($password)));
		return array();
	}

}

?>
