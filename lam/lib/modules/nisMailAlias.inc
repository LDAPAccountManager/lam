<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2004 - 2006  Roland Gruber

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
* Provides NIS mail alias management.
*
* @package modules
* @author Roland Gruber
*/

/**
* Provides NIS mail alias management.
*
* @package modules
*/
class nisMailAlias extends baseModule {

	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	*/
	function get_metaData() {
		$return = array();
		// manages host accounts
		$return["account_types"] = array("mailAlias");
		// base module
		$return["is_base"] = true;
		// LDAP filter
		$return["ldap_filter"] = array('or' => "(objectClass=nisMailAlias)");
		// alias name
		$return["alias"] = _("Mail aliases");
		// RDN attribute
		$return["RDN"] = array("cn" => "normal");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('nisMailAlias');
		// managed attributes
		$return['attributes'] = array('cn', 'rfc822MailMember');
		// help Entries
		$return['help'] = array(
			'alias' => array(
				"Headline" => _("Alias name"),
				"Text" => _("Mails to this name are forwarded to the recipients.")
			),
			'recipient' => array(
				"Headline" => _("Recipient"),
				"Text" => _("This is one recipient for this alias.")
			),
			'recipientList' => array(
				"Headline" => _("Recipient list"),
				"Text" => _("This is a comma separated list of recipients.")
			));
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'nisMailAlias_alias',
				'description' => _('Alias name'),
				'help' => 'alias',
				'example' => 'root',
				'required' => true
			),
			array(
				'name' => 'nisMailAlias_recipients',
				'description' => _('Recipient list'),
				'help' => 'recipientList',
				'example' => _('smiller')
			)
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'alias', 'recipients'
		);
		return $return;
	}

	/**
	* This function fills the error message array with messages
	*/
	function load_Messages() {
		$this->messages['alias'][0] = array('ERROR', 'Alias is empty or invalid!');  // third array value is set dynamically
		$this->messages['alias'][1] = array('ERROR', _('Account %s:') . ' nisMailAlias_alias', 'Alias is empty or invalid!');
		$this->messages['recipient'][0] = array('ERROR', 'Recipient is invalid!');  // third array value is set dynamically
		$this->messages['recipient'][1] = array('ERROR', _('Account %s:') . ' nisMailAlias_recipient', 'Recipient is invalid!');
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
		// skip saving if account is based on another structural object class
		if (!$_SESSION[$this->base]->isNewAccount && !in_array('nisMailAlias', $_SESSION[$this->base]->attributes_orig['objectClass'])) {
			return array();
		}
		return parent::save_attributes();
	}

	/**
	* This function will create the meta HTML code to show a page with all attributes.
	*/
	function display_html_attributes() {
		$return = array();
		// alias name
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Alias name')),
			1 => array('kind' => 'input', 'name' => 'cn', 'type' => 'text', 'size' => '20', 'value' => $this->attributes['cn'][0]),
			2 => array('kind' => 'help', 'value' => 'alias'));
		// list current recipients
		for ($i = 0; $i < sizeof($this->attributes['rfc822MailMember']); $i++) {
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Recipient')),
				1 => array('kind' => 'input', 'name' => 'rfc822MailMember' . $i, 'type' => 'text', 'size' => '17', 'value' => $this->attributes['rfc822MailMember'][$i]),
				2 => array('kind' => 'input', 'type' => 'submit', 'name' => 'delRec' . $i, 'value' => _("Remove")),
				3 => array('kind' => 'help', 'value' => 'recipient'));
		}
		// input box for new recipient
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('New recipient')),
			1 => array('kind' => 'input', 'name' => 'rfc822MailMember', 'type' => 'text', 'size' => '17', 'value' => ''),
			2 => array('kind' => 'input', 'type' => 'submit', 'name' => 'addRec', 'value' => _("Add")),
			3 => array('kind' => 'help', 'value' => 'recipient'),
			4 => array('kind' => 'input', 'type' => 'hidden', 'value' => sizeof($this->attributes['rfc822MailMember']), 'name' => 'rec_number'));
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
		$this->attributes['cn'] = array();
		$this->attributes['rfc822MailMember'] = array();
		// check alias name
		if (isset($_POST['cn']) && ($_POST['cn'] != "") && get_preg($_POST['cn'], 'nis_alias')) {
			$this->attributes['cn'][] = $_POST['cn'];
		}
		else {
				$message = $this->messages['alias'][0];
				$message[] = $_POST['cn'];
				$errors[] = $message;
		}
		// check old recipients
		if (isset($_POST['rec_number'])) {
			for ($i = 0; $i < $_POST['rec_number']; $i++) {
				if (isset($_POST['delRec' . $i])) continue;
				if (isset($_POST['rfc822MailMember' . $i]) && ($_POST['rfc822MailMember' . $i] != "")) {
					// check if address has correct format
					if (!get_preg($_POST['rfc822MailMember' . $i], 'nis_recipient')) {
						$message = $this->messages['recipient'][0];
						$message[] = $_POST['rfc822MailMember' . $i];
						$errors[] = $message;
					}
					$this->attributes['rfc822MailMember'][] = $_POST['rfc822MailMember' . $i];
				}
			}
		}
		// check new recipient
		if (isset($_POST['rfc822MailMember']) && ($_POST['rfc822MailMember'] != "")) {
			// check if address has correct format
			if (get_preg($_POST['rfc822MailMember'], 'nis_recipient')) {
				$this->attributes['rfc822MailMember'][] = $_POST['rfc822MailMember'];
			}
			else {
					$message = $this->messages['recipient'][0];
					$message[] = $_POST['rfc822MailMember'];
					$errors[] = $message;
			}
		}
		$this->attributes['rfc822MailMember'] = array_unique($this->attributes['rfc822MailMember']);
		return $errors;
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
			if (is_array($objectClasses) && !in_array('nisMailAlias', $objectClasses)) {
				return "disabled";
			}
		}
		return "enabled";
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
			if (!in_array("nisMailAlias", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "nisMailAlias";
			// add alias name
			// check format
			if (get_preg($rawAccounts[$i][$ids['nisMailAlias_alias']], 'nis_alias')) {
				$partialAccounts[$i]['cn'][] = $rawAccounts[$i][$ids['nisMailAlias_alias']];
			}
			else {
				$errMsg = $this->messages['alias'][1];
				array_push($errMsg, array($i));
				$messages[] = $errMsg;
			}
			// add recipients
			if ($rawAccounts[$i][$ids['nisMailAlias_recipients']] != "") {
				$aliases = explode(',', $rawAccounts[$i][$ids['nisMailAlias_recipients']]);
				// check format
				for ($a = 0; $a < sizeof($aliases); $a++) {
					if (get_preg($aliases[$a], 'nis_recipient')) {
						$partialAccounts[$i]['rfc822MailMember'][] = $aliases[$a];
					}
					else {
						$errMsg = $this->messages['recipient'][1];
						array_push($errMsg, array($i));
						$messages[] = $errMsg;
					}
				}
			}
		}
		return $messages;
	}

	/**
	* Returns the PDF entries for this module.
	*
	* @return array list of possible PDF entries
	*/
	function get_pdfEntries() {
		$return = array();
		if (isset($this->attributes['cn'][0])) {
			$return['nisMailAlias_alias'][0] = '<block><key>' . _('Alias name') . '</key><value>' . $this->attributes['cn'][0] . '</value></block>';
		}
		if (sizeof($this->attributes['rfc822MailMember']) > 0) {
			$return['nisMailAlias_recipients'][0] = '<block><key>' . _('Recipient list') . '</key><value>' . implode(', ', $this->attributes['rfc822MailMember']) . '</value></block>';
		}
		return $return;
	}

}


?>
