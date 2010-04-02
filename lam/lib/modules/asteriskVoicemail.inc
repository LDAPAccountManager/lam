<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2010  Pavel Pozdnyak
				2010  Roland Gruber

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
 * Manages the Asterisk extension of user accounts.
 *
 * @package modules
 *
 * @author Pavel Pozdnyak
 * @author Roland Gruber
 */

/**
 * Manages the Asterisk extension of user accounts.
 *
 * @package modules
 */
class asteriskVoicemail extends baseModule implements passwordService {

	/**
	 * Creates a new asteriskVoicemail object.
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
		$return["alias"] = _("Asterisk voicemail");
		// module dependencies
		$return['dependencies'] = array('depends' => array('inetOrgPerson'), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('AsteriskVoiceMail');
		// managed attributes
		$return['attributes'] = array('AstContext', 'AstVoicemailMailbox',
				'AstVoicemailPassword', 'AstVoicemailFullname', 'AstVoicemailEmail',
				'AstVoicemailPager', 'AstVoicemailOptions', 'AstVoicemailContext');
		// icon
		$return['icon'] = 'asterisk.png';
		// help
		$return['help'] = array(
			'AstContext' => array(
				"Headline" => _("Account context"),
				"Text" => _("The account context stores information about the dial plan.")
			),
			'AstVoicemailMailbox' => array(
				"Headline" => _("Mailbox"),
				"Text" => _("Voicemail mailbox for this account.")
			),
			'AstVoicemailPassword' => array(
				"Headline" => _("Password"),
				"Text" => _("Password for voicemail mailbox.")
			),
			'AstVoicemailFullname' => array(
				"Headline" => _("Full name"),
				"Text" => _("Full name for Asterisk voicemail mailbox.")
			),
			'AstVoicemailEmail' => array(
				"Headline" => _("Email address"),
				"Text" => _("Email address for this voicemail account.")
			),
			'AstVoicemailPager' => array(
				"Headline" => _("Pager"),
				"Text" => _("Pager number for Asterisk voicemail.")
			),
			'AstVoicemailOptions' => array(
				"Headline" => _("Options"),
				"Text" => _("Options for Asterisk voicemail account (e.g. sendvoicemail=yes).")
			),
			'AstVoicemailContext' => array(
				"Headline" => _("Voicemail context"),
				"Text" => _("Asterisk voicemail context.")
			),
		);
		// profile options
		$return['profile_options'] = array(
			array(
				array('kind' => 'text', 'text' => _('Options') . ":"),
				array('kind' => 'input', 'name' => 'asteriskVoicemail_AstVoicemailOptions', 'type' => 'text', 'size' => '30'),
				array('kind' => 'help', 'value' => 'AstVoicemailOptions')),
			array(
				array('kind' => 'text', 'text' => _('Voicemail context') . ":"),
				array('kind' => 'input', 'name' => 'asteriskVoicemail_AstVoicemailContext', 'type' => 'text', 'size' => '30'),
				array('kind' => 'help', 'value' => 'AstContext')),
			array(
				array('kind' => 'text', 'text' => _('Account context') . ":"),
				array('kind' => 'input', 'name' => 'asteriskAccount_AstAccountContext', 'type' => 'text', 'size' => '30'),
				array('kind' => 'help', 'value' => 'AstContext'))
		);
		// profile mappings
		$return['profile_mappings'] = array(
				'asteriskAccount_AstAccountContext' => 'AstContext',
				'asteriskVoicemail_AstVoicemailContext' => 'AstVoicemailContext',
				'asteriskVoicemail_AstVoicemailOptions' => 'AstVoicemailOptions'
		);
		// available PDF fields
		$return['PDF_fields'] = array(
				'AstContext', 'AstVoicemailMailbox',
				'AstVoicemailFullname', 'AstVoicemailEmail',
				'AstVoicemailPager', 'AstVoicemailOptions',
				'AstVoicemailContext'
		);
		// upload dependencies
		$return['upload_preDepends'] = array('posixAccount', 'inetOrgPerson');
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'asteriskVoicemail_AstContext',
				'description' => _('Account context'),
				'help' => 'AstContext',
				'example' => 'default',
				'required' => true
			),
			array(
				'name' => 'asteriskVoicemail_AstVoicemailMailbox',
				'description' => _('Mailbox'),
				'help' => 'AstVoicemailMailbox',
				'example' => '12345',
				'required' => true
			),
			array(
				'name' => 'asteriskVoicemail_AstVoicemailPassword',
				'description' => _('Password'),
				'help' => 'AstVoicemailPassword',
				'example' => _('secret'),
				'required' => true
			),
			array(
				'name' => 'asteriskVoicemail_AstVoicemailFullname',
				'description' => _('Full name'),
				'help' => 'AstVoicemailFullname',
				'example' => _('Steve Miller')
			),
			array(
				'name' => 'asteriskVoicemail_AstVoicemailEmail',
				'description' => _('Email address'),
				'help' => 'AstVoicemailEmail',
				'example' => _('user@company.com')
			),
			array(
				'name' => 'asteriskVoicemail_AstVoicemailPager',
				'description' => _('Pager'),
				'help' => 'AstVoicemailPager',
				'example' => _('123-123-1234')
			),
			array(
				'name' => 'asteriskVoicemail_AstVoicemailOptions',
				'description' => _('Options'),
				'help' => 'AstVoicemailOptions',
				'example' => 'sendvoicemail=yes'
			),
			array(
				'name' => 'asteriskVoicemail_AstVoicemailContext',
				'description' => _('Voicemail context'),
				'help' => 'AstVoicemailContext',
				'example' => 'default'
			)
		);
		return $return;
	}

	/**
	 * This function fills the error message array with messages
	 */
	function load_Messages() {
		//messages for voicemail
		$this->messages['AstVoicemailMailbox'][0] = array('ERROR', _('Please enter a mailbox.'));
		$this->messages['AstVoicemailMailbox'][1] = array('ERROR', _('The mailbox format is invalid.'));
		$this->messages['AstVoicemailMailbox'][2] = array('ERROR', _('There is already another user with this mailbox name.'));

		$this->messages['AstContext'][0] = array('ERROR', _('Please enter the account context.'));
		$this->messages['AstContext'][1] = array('ERROR', _('The account context is invalid.'));
		$this->messages['AstContext'][2] = array('ERROR', _('Account %s:') . ' asteriskVoicemail_AstContext', _('The account context is invalid.'));

		$this->messages['AstVoicemailFullname'][0] = array('ERROR', _('The full name name is invalid.'));
		$this->messages['AstVoicemailEmail'][01] = array('ERROR', _('The email address is invalid.'));
		$this->messages['AstVoicemailPager'][0] = array('ERROR', _('The pager number has bad format.'));
		$this->messages['AstVoicemailOptions'][0] = array('ERROR', _('The options have bad format.'));
		$this->messages['AstVoicemailOptions'][1] = array('ERROR', _('Account %s:') . ' asteriskVoicemail_AstVoicemailOptions', _('The options have bad format.'));
		$this->messages['AstVoicemailContext'][0] = array('ERROR', _('The voicemail context name is invalid.'));
		$this->messages['AstVoicemailContext'][1] = array('ERROR', _('Account %s:') . ' asteriskVoicemail_AstVoicemailContext', _('The voicemail context name is invalid.'));
		$this->messages['AstVoicemailPassword'][0] = array('INFO', _('Please set the voicemail password with "Set password" before saving.'));
	}

	/**
	 * This function will create the meta HTML code to show a page with all attributes.
	 */
	function display_html_attributes() {
		$return = array();
		if (in_array('AsteriskVoiceMail', $this->attributes['objectClass'])) {
			if (!isset($this->attributes['AstVoicemailPassword'])) {
				call_user_func_array('StatusMessage', $this->messages['AstVoicemailPassword'][0]);
				$return[] = array(array('kind' => 'text', 'td' => array('colspan' => 3)));
			}
			$return[] = array(
					array('kind' => 'text', 'text' => _("Mailbox").'*'),
					array('kind' => 'input', 'name' => 'AstVoicemailMailbox', 'type' => 'text', 'size' => '30', 'value' => $this->attributes['AstVoicemailMailbox'][0]),
					array('kind' => 'help', 'value' => 'AstVoicemailMailbox'));
			$return[] = array(
					array('kind' => 'text', 'text' => _("Account context").'*'),
					array('kind' => 'input', 'name' => 'AstContext', 'type' => 'text', 'size' => '30', 'value' => $this->attributes['AstContext'][0]),
					array('kind' => 'help', 'value' => 'AstContext'));

			$return[] = array(array('kind' => 'text', 'td' => array('colspan' => 3)));

			$return[] = array(
					array('kind' => 'text', 'text' => _("Full name")),
					array('kind' => 'input', 'name' => 'AstVoicemailFullname', 'type' => 'text', 'size' => '30', 'value' => $this->attributes['AstVoicemailFullname'][0]),
					array('kind' => 'help', 'value' => 'AstVoicemailFullname'));
			$return[] = array(
					array('kind' => 'text', 'text' => _("Email address")),
					array('kind' => 'input', 'name' => 'AstVoicemailEmail', 'type' => 'text', 'size' => '30', 'value' => $this->attributes['AstVoicemailEmail'][0]),
					array('kind' => 'help', 'value' => 'AstVoicemailEmail'));
			$return[] = array(
					array('kind' => 'text', 'text' => _("Pager")),
					array('kind' => 'input', 'name' => 'AstVoicemailPager', 'type' => 'text', 'size' => '30', 'value' => $this->attributes['AstVoicemailPager'][0]),
					array('kind' => 'help', 'value' => 'AstVoicemailPager'));
			$return[] = array(
					array('kind' => 'text', 'text' => _("Options")),
					array('kind' => 'input', 'name' => 'AstVoicemailOptions', 'type' => 'text', 'size' => '30', 'value' => $this->attributes['AstVoicemailOptions'][0]),
					array('kind' => 'help', 'value' => 'AstVoicemailOptions'));
			$return[] = array(
					array('kind' => 'text', 'text' => _("Voicemail context")),
					array('kind' => 'input', 'name' => 'AstVoicemailContext', 'type' => 'text', 'size' => '30', 'value' => $this->attributes['AstVoicemailContext'][0]),
					array('kind' => 'help', 'value' => 'AstVoicemailContext'));
			$return[] = array(
					array('kind' => 'text', 'text' => '&nbsp;'),
					array('kind' => 'input', 'type' => 'submit', 'name' => 'form_subpage_' . get_class($this) . '_attributes_remVoicemailObjectClass', 'value' => _('Remove Asterisk voicemail extension'))
			);
		}
		else {
			$return[] = array(
					array('kind' => 'text', 'text' => '&nbsp;'),
					array('kind' => 'input', 'type' => 'submit', 'name' => 'form_subpage_' . get_class($this) . '_attributes_addVoicemailObjectClass', 'value' => _('Add Asterisk voicemail extension'))
			);
		}
		return $return;
	}

	/**
	 * Write variables into object and do some regex checks
	 */
	function process_attributes() {
		if (isset($_POST['form_subpage_' . get_class($this) . '_attributes_addVoicemailObjectClass'])) {
			$this->attributes['objectClass'][] = 'AsteriskVoiceMail';
			return array();
		}
		// remove extension
		elseif (isset($_POST['form_subpage_' . get_class($this) . '_attributes_remVoicemailObjectClass'])) {
			$this->attributes['objectClass'] = array_delete(array('AsteriskVoiceMail'), $this->attributes['objectClass']);
			$attrKeys = array_keys($this->attributes);
			for ($k = 0; $k < sizeof($attrKeys); $k++) {
				if (strpos($attrKeys[$k], 'AstVoicemail') > -1) {
					unset($this->attributes[$attrKeys[$k]]);
				}
			}
			if (isset($this->attributes['AstContext'])) {
				unset($this->attributes['AstContext']);
			}
			return array();
		}
		$errors = array();
		if (!in_array('AsteriskVoiceMail', $this->attributes['objectClass'])) {
			return array();
		}
		$this->attributes['AstVoicemailMailbox'] = array();
		$this->attributes['AstVoicemailFullname'] = array();
		$this->attributes['AstVoicemailEmail'] = array();
		$this->attributes['AstVoicemailPager'] = array();
		$this->attributes['AstVoicemailOptions'] = array();
		$this->attributes['AstVoicemailContext'] = array();

		if (isset($_POST['AstContext'])) {
			$this->attributes['AstContext'][0] = $_POST['AstContext'];
			if($this->attributes['AstContext'][0] == '') {
				$errors[] = $this->messages['AstContext'][0];
			}
			elseif (!get_preg($this->attributes['AstContext'][0], 'realname')) {
				$errors[] = $this->messages['AstContext'][1];
			}
		}
		if (isset($_POST['AstVoicemailMailbox'])) {
			$this->attributes['AstVoicemailMailbox'][0] = $_POST['AstVoicemailMailbox'];
			// check if caller ID is empty
			if($this->attributes['AstVoicemailMailbox'][0] == '') {
				$errors[] = $this->messages['AstVoicemailMailbox'][0];
			}
			// check format
			else if (!get_preg($this->attributes['AstVoicemailMailbox'][0], 'username')) {
				$errors[] = $this->messages['AstVoicemailMailbox'][1];
			}
			// check for duplicate Voicemail ID
			else if (!isset($this->orig['AstVoicemailMailbox'][0]) || (($this->orig['AstVoicemailMailbox'][0] != $this->attributes['AstVoicemailMailbox'][0]))) {
				$entries = searchLDAPByAttribute('AstVoicemailMailbox', $this->attributes['AstVoicemailMailbox'][0], 'AstVoicemailMailbox', array('dn'), array('user'));
				if (sizeof($entries) > 0) {
					$errors[] = $this->messages['AstVoicemailMailbox'][2];
				}
			}
		}
		if (isset($_POST['AstVoicemailFullname'])) {
			$this->attributes['AstVoicemailFullname'][0] = $_POST['AstVoicemailFullname'];
			if($this->attributes['AstVoicemailFullname'][0] != '' && !get_preg($this->attributes['AstVoicemailFullname'][0], 'realname')) {
				$errors[] = $this->messages['AstVoicemailFullname'][0];
			}
		}
		if (isset($_POST['AstVoicemailEmail'])) {
			$this->attributes['AstVoicemailEmail'][0] = $_POST['AstVoicemailEmail'];
			if($this->attributes['AstVoicemailEmail'][0] != '' && !get_preg($this->attributes['AstVoicemailEmail'][0], 'email')) {
				$errors[] = $this->messages['AstVoicemailEmail'][0];
			}
		}
		if (isset($_POST['AstVoicemailPager'])) {
			$this->attributes['AstVoicemailPager'][0] = $_POST['AstVoicemailPager'];
			if($this->attributes['AstVoicemailPager'][0] != '' && !get_preg($this->attributes['AstVoicemailPager'][0], 'telephone')) {
				$errors[] = $this->messages['AstVoicemailPager'][0];
			}
		}
		if (isset($_POST['AstVoicemailOptions'])) {
			$this->attributes['AstVoicemailOptions'][0] = $_POST['AstVoicemailOptions'];
		}
		if (isset($_POST['AstVoicemailContext'])) {
			$this->attributes['AstVoicemailContext'][0] = $_POST['AstVoicemailContext'];
			if($this->attributes['AstVoicemailContext'][0] != '' && !get_preg($this->attributes['AstVoicemailContext'][0], 'realname')) {
				$errors[] = $this->messages['AstVoicemailContext'][0];
			}
		}
		return $errors;
	}

	/**
	 * Returns a list of PDF entries
	 */
	function get_pdfEntries() {
		$return = array();
		$return[get_class($this) . '_AstVoicemailMailbox'] = array('<block><key>' . _('Mailbox') . '</key><value>' . $this->attributes['AstVoicemailMailbox'][0] . '</value></block>');
		$return[get_class($this) . '_AstContext'] = array('<block><key>' . _('Account context') . '</key><value>' . $this->attributes['AstContext'][0] . '</value></block>');
		$return[get_class($this) . '_AstVoicemailFullname'] = array('<block><key>' . _('Full name') . '</key><value>' . $this->attributes['AstVoicemailFullname'][0] . '</value></block>');
		$return[get_class($this) . '_AstVoicemailContext'] = array('<block><key>' . _('Voicemail context') . '</key><value>' . $this->attributes['AstVoicemailContext'][0] . '</value></block>');
		$return[get_class($this) . '_AstVoicemailPager'] = array('<block><key>' . _('Pager') . '</key><value>' . $this->attributes['AstVoicemailPager'][0] . '</value></block>');
		$return[get_class($this) . '_AstVoicemailEmail'] = array('<block><key>' . _('Email address') . '</key><value>' . $this->attributes['AstVoicemailEmail'][0] . '</value></block>');
		$return[get_class($this) . '_AstVoicemailOptions'] = array('<block><key>' . _('Options') . '</key><value>' . $this->attributes['AstVoicemailOptions'][0] . '</value></block>');
		return $return;
	}

	/**
	 * In this function the LDAP account is built up.
	 *
	 * @param array $rawAccounts list of hash arrays (name => value) from user input
	 * @param array $partialAccounts list of hash arrays (name => value) which are later added to LDAP
	 * @param array $ids list of IDs for column position (e.g. "posixAccount_uid" => 5)
	 * @param array $selectedModules list of selected account modules
	 * @return array list of error messages if any
	 */
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules) {
		$messages = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array("AsteriskVoiceMail", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "AsteriskVoiceMail";
			
			if (!in_array('asteriskAccount', $selectedModules)) {
				//add context
				if ($rawAccounts[$i][$ids['asteriskAccount_AstContext']] == "") {
					// default value
					$partialAccounts[$i]['AstContext'] = 'default';
				}
				elseif (get_preg($rawAccounts[$i][$ids['asteriskVoicemail_AstContext']], 'realname')) {
					$partialAccounts[$i]['AstContext'] = $rawAccounts[$i][$ids['asteriskAccount_AstContext']];
				}
				else {
					$errMsg = $this->messages['AstContext'][2];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			
			// add account caller id
			if (get_preg($rawAccounts[$i][$ids['asteriskVoicemail_AstVoicemailMailbox']], 'realname')) {
				$partialAccounts[$i]['AstVoicemailMailbox'] = $rawAccounts[$i][$ids['asteriskVoicemail_AstVoicemailMailbox']];
			}
			else {
				$errMsg = $this->messages['AstVoicemailMailbox'][1];
				array_push($errMsg, array($i));
				$messages[] = $errMsg;
			}

			if (get_preg($rawAccounts[$i][$ids['asteriskVoicemail_AstVoicemailEmail']], 'email')) {
				$partialAccounts[$i]['AstVoicemailEmail'] = $rawAccounts[$i][$ids['asteriskVoicemail_AstVoicemailEmail']];
			}
			else {
				$errMsg = $this->messages['AstVoicemailEmail'][1];
				array_push($errMsg, array($i));
				$messages[] = $errMsg;
			}

			if (get_preg($rawAccounts[$i][$ids['asteriskVoicemail_AstVoicemailPager']], 'telephone')) {
				$partialAccounts[$i]['AstVoicemailPager'] = $rawAccounts[$i][$ids['asteriskVoicemail_AstVoicemailPager']];
			}
			else {
				$errMsg = $this->messages['AstVoicemailPager'][1];
				array_push($errMsg, array($i));
				$messages[] = $errMsg;
			}

			if ($rawAccounts[$i][$ids['asteriskVoicemail_AstVoicemailOptions']]!= "") {
				$partialAccounts[$i]['AstVoicemailOptions'] = $rawAccounts[$i][$ids['asteriskVoicemail_AstVoicemailOptions']];
			}
			
			if (($rawAccounts[$i][$ids['asteriskVoicemail_AstVoicemailFullname']] != "") && (get_preg($rawAccounts[$i][$ids['asteriskVoicemail_AstVoicemailFullname']], 'realname')) ) {
				$partialAccounts[$i]['AstVoicemailFullname'] = $rawAccounts[$i][$ids['asteriskVoicemail_AstVoicemailFullname']];
			}
			else {
				$errMsg = $this->messages['AstVoicemailFullname'][0];
				array_push($errMsg, array($i));
				$messages[] = $errMsg;
			}

			if (($rawAccounts[$i][$ids['asteriskVoicemail_AstVoicemailContext']] != "") && (get_preg($rawAccounts[$i][$ids['asteriskVoicemail_AstVoicemailContext']], 'realname')) ) {
				$partialAccounts[$i]['AstVoicemailContext'] = $rawAccounts[$i][$ids['asteriskVoicemail_AstVoicemailContext']];
			}
			else {
				$errMsg = $this->messages['AstVoicemailContext'][1];
				array_push($errMsg, array($i));
				$messages[] = $errMsg;
			}

			$partialAccounts[$i]['AstVoicemailPassword'] = $rawAccounts[$i][$ids['asteriskVoicemail_AstVoicemailPassword']];
		}
		return $messages;
	}

	/**
	 * This functions returns true if all needed settings are done.
	 *
	 * @return boolean true if LDAP operation can be done
	 */
	function module_complete() {
		if (in_array('AsteriskVoiceMail', $this->attributes['objectClass'])) {
			if(!isset($this->attributes['AstVoicemailPassword'][0]) || $this->attributes['AstVoicemailPassword'][0] == "" ) {
				return false;
			}
		}		
		return true;
	}

	/**
	 * This method specifies if a module manages password attributes.
	 * @see passwordService::managesPasswordAttributes
	 *
	 * @return boolean true if this module manages password attributes
	 */
	public function managesPasswordAttributes() {
		if (!in_array('AsteriskVoiceMail', $this->attributes['objectClass'])) {
			return false;
		}
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
		$this->attributes['AstVoicemailPassword'][0] = $password;
		return array();
	}
	
}

?>
