<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2004  Roland Gruber

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
	
	/** used for account pages, true if input data is correct */
	var $inputCorrect = true;

	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	*/
	function get_metaData() {
		$return = array();
		// manages host accounts
		$return["account_types"] = array("group");
		// base module
		$return["is_base"] = true;
		// LDAP filter
		$return["ldap_filter"] = array('or' => "(objectClass=nisMailAlias)");
		// alias name
		$return["alias"] = _("Mail aliases");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
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
	* This function loads all needed attributes into the object.
	*
	* @param array $attr an array as it is retured from ldap_get_attributes
	*/
	function load_attributes($attr) {
		$this->attributes['objectClass'] = array();
		$this->attributes['rfc822MailMember'] = array();
		$this->attributes['cn'] = array();
		$this->orig['objectClass'] = array();
		$this->orig['rfc822MailMember'] = array();
		$this->orig['cn'] = array();
		if (isset($attr['objectClass'])) {
			$this->attributes['objectClass'] = $attr['objectClass'];
			$this->orig['objectClass'] = $attr['objectClass'];
		}
		if (isset($attr['cn'])) {
			$this->attributes['cn'] = $attr['cn'];
			$this->orig['cn'] = $attr['cn'];
		}
		if (isset($attr['rfc822MailMember'])) {
			$this->attributes['rfc822MailMember'] = $attr['rfc822MailMember'];
			$this->orig['rfc822MailMember'] = $attr['rfc822MailMember'];
		}
		// add object class if needed
		if (! in_array('nisMailAlias', $this->orig['objectClass'])) {
			$this->attributes['objectClass'][] = 'nisMailAlias';
		}
		return 0;
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
		return $_SESSION[$this->base]->save_module_attributes($this->attributes, $this->orig);
	}

	/**
	* This function returns a list of all account pages in this module.
	*/
	function pages() {
		return array('attributes');
	}

	/**
	* This function will create the meta HTML code to show a page with all attributes.
	*
	* @param array $post HTTP-POST values
	*/
	function display_html_attributes(&$post) {
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
	* Write variables into object and do some regex checks
	*
	* @param array $post HTTP-POST values
	*/
	function process_attributes(&$post) {
		$this->triggered_messages = array();
		$this->attributes['cn'] = array();
		$this->attributes['rfc822MailMember'] = array();
		// check alias name
		if (isset($post['cn']) && ($post['cn'] != "") && get_preg($post['cn'], 'nis_alias')) {
			$this->attributes['cn'][] = $post['cn'];
		}
		else {
				$message = $this->messages['alias'][0];
				$message[] = $post['cn'];
				$this->triggered_messages[] = array($message);
		}
		// check old recipients
		if (isset($post['rec_number'])) {
			for ($i = 0; $i < $post['rec_number']; $i++) {
				if (isset($post['delRec' . $i])) continue;
				if (isset($post['rfc822MailMember' . $i]) && ($post['rfc822MailMember' . $i] != "")) {
					// check if address has correct format
					if (!get_preg($post['rfc822MailMember' . $i], 'nis_recipient')) {
						$message = $this->messages['recipient'][0];
						$message[] = $post['rfc822MailMember' . $i];
						$this->triggered_messages[] = array($message);
					}
					$this->attributes['rfc822MailMember'][] = $post['rfc822MailMember' . $i];
				}
			}
		}
		// check new recipient
		if (isset($post['rfc822MailMember']) && ($post['rfc822MailMember'] != "")) {
			// check if address has correct format
			if (get_preg($post['rfc822MailMember'], 'nis_recipient')) {
				$this->attributes['rfc822MailMember'][] = $post['rfc822MailMember'];
			}
			else {
					$message = $this->messages['recipient'][0];
					$message[] = $post['rfc822MailMember'];
					$this->triggered_messages[] = array($message);
			}
		}
		$this->attributes['rfc822MailMember'] = array_unique($this->attributes['rfc822MailMember']);
		if (sizeof($this->triggered_messages) > 0) {
			$this->inputCorrect = false;
			return $this->triggered_messages;
		}
		else {
			$this->inputCorrect = true;
			return 0;
		}
	}

	/**
	* This function returns true if all needed settings are done.
	*/
	function module_complete() {
		return $this->inputCorrect;
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
	* Returns a list of PDF entries
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
