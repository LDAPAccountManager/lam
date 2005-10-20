<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2005  Roland Gruber

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
* Manages SSH public keys.
*
* @package modules
* @author Roland Gruber
* @author Andrew Dibble <adibble@mobitv.com>
*/

/**
* Manages SSH public keys.
*
* @package modules
*/
class ldapPublicKey extends baseModule {
	
	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	*/
	function get_metaData() {
		$return = array();
		// manages host accounts
		$return["account_types"] = array("user");
		// alias name
		$return["alias"] = _("SSH public key");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// help Entries
		$return['help'] = array(
			'key' => array(
				"Headline" => _("SSH public key"),
				"Text" => _("Please enter your public SSH key.")
			),
			'keyList' => array(
				"Headline" => _("SSH public key"),
				"Text" => _("Please a comma separated list of your public SSH keys.")
			)
		);
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'ldapPublicKey_sshPublicKey',
				'description' => _('SSH public key'),
				'help' => 'keyList',
				'example' => 'ssh-dss 234234 user@host'
			)
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'sshPublicKey'
		);
		return $return;
	}

	/**
	* This function loads all needed attributes into the object.
	*
	* @param array $attr an array as it is retured from ldap_get_attributes
	*/
	function load_attributes($attr) {
		$this->attributes['objectClass'] = array();
		$this->attributes['sshPublicKey'] = array();
		$this->orig['objectClass'] = array();
		$this->orig['sshPublicKey'] = array();
		if (isset($attr['objectClass'])) {
			$this->attributes['objectClass'] = $attr['objectClass'];
			$this->orig['objectClass'] = $attr['objectClass'];
		}
		if (isset($attr['sshPublicKey'])) {
			$this->attributes['sshPublicKey'] = $attr['sshPublicKey'];
			$this->orig['sshPublicKey'] = $attr['sshPublicKey'];
		}
		// add object class if needed
		if (! in_array('ldapPublicKey', $this->orig['objectClass'])) {
			$this->attributes['objectClass'][] = 'ldapPublicKey';
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
	* This function will create the meta HTML code to show a page with all attributes.
	*
	* @param array $post HTTP-POST values
	*/
	function display_html_attributes(&$post) {
		$return = array();
		// list current keys
		for ($i = 0; $i < sizeof($this->attributes['sshPublicKey']); $i++) {
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('SSH public key')),
				1 => array('kind' => 'input', 'name' => 'sshPublicKey' . $i, 'type' => 'text', 'size' => '100', 'maxlength' => '2048', 'value' => $this->attributes['sshPublicKey'][$i]),
				2 => array('kind' => 'input', 'type' => 'submit', 'name' => 'delKey' . $i, 'value' => _("Remove")),
				3 => array('kind' => 'help', 'value' => 'key'));
		}
		// input box for new key
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('New SSH public key')),
			1 => array('kind' => 'input', 'name' => 'sshPublicKey', 'type' => 'text', 'size' => '100', 'maxlength' => '2048', 'value' => ''),
			2 => array('kind' => 'input', 'type' => 'submit', 'name' => 'addKey', 'value' => _("Add")),
			3 => array('kind' => 'help', 'value' => 'key'),
			4 => array('kind' => 'input', 'type' => 'hidden', 'value' => sizeof($this->attributes['sshPublicKey']), 'name' => 'key_number'));
		return $return;
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @param array $post HTTP-POST values
	* @return array list of info/error messages
	*/
	function process_attributes(&$post) {
		$this->triggered_messages = array();
		$this->attributes['sshPublicKey'] = array();
		// check old keys
		if (isset($post['key_number'])) {
			for ($i = 0; $i < $post['key_number']; $i++) {
				if (isset($post['delKey' . $i])) continue;
				if (isset($post['sshPublicKey' . $i]) && ($post['sshPublicKey' . $i] != "")) {
					$this->attributes['sshPublicKey'][] = $post['sshPublicKey' . $i];
				}
			}
		}
		// check new key
		if (isset($post['sshPublicKey']) && ($post['sshPublicKey'] != "")) {
			$this->attributes['sshPublicKey'][] = $post['sshPublicKey'];
		}
		$this->attributes['sshPublicKey'] = array_unique($this->attributes['sshPublicKey']);
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
			if (!in_array("ldapPublicKey", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "ldapPublicKey";
			// add keys
			if ($rawAccounts[$i][$ids['ldapPublicKey_sshPublicKey']] != "") {
				$keys = explode(',', $rawAccounts[$i][$ids['ldapPublicKey_sshPublicKey']]);
				// check format
				for ($m = 0; $m < sizeof($keys); $m++) {
					$partialAccounts[$i]['sshPublicKey'][] = $keys[$m];
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
		if (sizeof($this->attributes['sshPublicKey']) > 0) {
			$return['ldapPublicKey_sshPublicKey'][0] = '<block><key>' . _('SSH public key(s)') . '</key><tr><td align=\"L\">' . $this->attributes['sshPublicKey'][0] . '</td></tr></block>';
			for ($i = 1; $i < sizeof($this->attributes['sshPublicKey']); $i++) {
				$return['ldapPublicKey_sshPublicKey'][] = '<block><tr><td align=\"L\">' . $this->attributes['sshPublicKey'][$i] . '</td></tr></block>';
			}
		}
		return $return;
	}

}


?>
