<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2005 - 2013  Roland Gruber

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
	* 
	* @see baseModule::get_metaData()
	*/
	function get_metaData() {
		$return = array();
		// icon
		$return['icon'] = 'keyBig.png';
		// manages host accounts
		$return["account_types"] = array("user");
		// alias name
		$return["alias"] = _("SSH public key");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('ldapPublicKey');
		// managed attributes
		$return['attributes'] = array('sshPublicKey');
		// help Entries
		$return['help'] = array(
			'key' => array(
				"Headline" => _("SSH public key"), 'attr' => 'sshPublicKey',
				"Text" => _("Please enter your public SSH key.")
			),
			'keyList' => array(
				"Headline" => _("SSH public key"), 'attr' => 'sshPublicKey',
				"Text" => _("Please a comma separated list of your public SSH keys.")
			)
		);
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'ldapPublicKey_sshPublicKey',
				'description' => _('SSH public key'),
				'help' => 'keyList',
				'example' => _('ssh-dss 234234 user@host')
			)
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'sshPublicKey' => _('SSH public keys')
		);
		// self service field settings
		$return['selfServiceFieldSettings'] = array(
			'sshPublicKey' => _('SSH public keys'),
		);
		$return['selfServiceReadOnlyFields'] = array('sshPublicKey');
		return $return;
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		$return = new htmlTable();
		$keyCount = 0;
		// list current keys
		if (isset($this->attributes['sshPublicKey'])) {
			$keyCount = sizeof($this->attributes['sshPublicKey']);
			for ($i = 0; $i < sizeof($this->attributes['sshPublicKey']); $i++) {
				$return->addElement(new htmlOutputText(_('SSH public key')));
				$sshInput = new htmlInputField('sshPublicKey' . $i, $this->attributes['sshPublicKey'][$i]);
				$sshInput->setFieldSize(50);
				$sshInput->setFieldMaxLength(4096);
				$return->addElement($sshInput);
				$return->addElement(new htmlButton('delKey' . $i, 'del.png', true));
				$return->addElement(new htmlHelpLink('key'), true);
			}
		}
		// input box for new key
		$return->addElement(new htmlOutputText(_('New SSH public key')));
		$sshNewKey = new htmlInputField('sshPublicKey');
		$sshNewKey->setFieldSize(50);
		$sshNewKey->setFieldMaxLength(4096);
		$return->addElement($sshNewKey);
		$return->addElement(new htmlButton('addKey', 'add.png', true));
		$return->addElement(new htmlHelpLink('key'));
		$return->addElement(new htmlHiddenInput('key_number', $keyCount));
		return $return;
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_attributes() {
		$this->attributes['sshPublicKey'] = array();
		// check old keys
		if (isset($_POST['key_number'])) {
			for ($i = 0; $i < $_POST['key_number']; $i++) {
				if (isset($_POST['delKey' . $i])) continue;
				if (isset($_POST['sshPublicKey' . $i]) && ($_POST['sshPublicKey' . $i] != "")) {
					$this->attributes['sshPublicKey'][] = $_POST['sshPublicKey' . $i];
				}
			}
		}
		// check new key
		if (isset($_POST['sshPublicKey']) && ($_POST['sshPublicKey'] != "")) {
			$this->attributes['sshPublicKey'][] = $_POST['sshPublicKey'];
		}
		$this->attributes['sshPublicKey'] = array_unique($this->attributes['sshPublicKey']);
		return array();
	}

	/**
	* In this function the LDAP account is built up.
	*
	* @param array $rawAccounts list of hash arrays (name => value) from user input
	* @param array $ids list of IDs for column position (e.g. "posixAccount_uid" => 5)
	* @param array $partialAccounts list of hash arrays (name => value) which are later added to LDAP
	* @param array $selectedModules list of selected account modules
	* @return array list of error messages if any
	*/
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules) {
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
			$return['ldapPublicKey_sshPublicKey'][0] = '<block><key>' . _('SSH public keys') . '</key><tr><td align=\"L\">' . $this->attributes['sshPublicKey'][0] . '</td></tr></block>';
			for ($i = 1; $i < sizeof($this->attributes['sshPublicKey']); $i++) {
				$return['ldapPublicKey_sshPublicKey'][] = '<block><tr><td align=\"L\">' . $this->attributes['sshPublicKey'][$i] . '</td></tr></block>';
			}
		}
		return $return;
	}

	/**
	 * Returns the meta HTML code for each input field.
	 * format: array(<field1> => array(<META HTML>), ...)
	 * It is not possible to display help links.
	 *
	 * @param array $fields list of active fields
	 * @param array $attributes attributes of LDAP account
	 * @param boolean $passwordChangeOnly indicates that the user is only allowed to change his password and no LDAP content is readable
	 * @param array $readOnlyFields list of read-only fields
	 * @return array list of meta HTML elements (field name => htmlTableRow)
	 */
	function getSelfServiceOptions($fields, $attributes, $passwordChangeOnly, $readOnlyFields) {
		$return = array();
		if ($passwordChangeOnly) {
			return $return; // no fields as long no LDAP content can be read
		}
		if (in_array('sshPublicKey', $fields)) {
			$sshPublicKeys = array();
			if (isset($attributes['sshPublicKey'][0])) {
				$sshPublicKeys = $attributes['sshPublicKey'];
			}
			$sshPublicKeyField = new htmlInputTextarea('ldapPublicKey_sshPublicKey', implode("\r\n", $sshPublicKeys), 100, 4);
			if (in_array('sshPublicKey', $readOnlyFields)) {
				$text = '';
				for ($i = 0; $i < sizeof($sshPublicKeys); $i++) {
					if ($i > 0) {
						$text .= '<br>';
					}
					$text .= htmlspecialchars($sshPublicKeys[$i]);
				}
				$sshPublicKeyField = new htmlOutputText($text, false);
			}
			$label = new htmlOutputText(_('SSH public keys'));
			$label->alignment = htmlElement::ALIGN_TOP;
			$return['sshPublicKey'] = new htmlTableRow(array(
				$label, $sshPublicKeyField
			));
		}
		return $return;
	}

	/**
	 * Checks if all input values are correct and returns the LDAP attributes which should be changed.
	 * <br>Return values:
	 * <br>messages: array of parameters to create status messages
	 * <br>add: array of attributes to add
	 * <br>del: array of attributes to remove
	 * <br>mod: array of attributes to modify
	 * <br>info: array of values with informational value (e.g. to be used later by pre/postModify actions)
	 * 
	 * Calling this method does not require the existence of an enclosing {@link accountContainer}.
	 *
	 * @param string $fields input fields
	 * @param array $attributes LDAP attributes
	 * @param boolean $passwordChangeOnly indicates that the user is only allowed to change his password and no LDAP content is readable
	 * @param array $readOnlyFields list of read-only fields
	 * @return array messages and attributes (array('messages' => array(), 'add' => array('mail' => array('test@test.com')), 'del' => array(), 'mod' => array(), 'info' => array()))
	 */
	public function checkSelfServiceOptions($fields, $attributes, $passwordChangeOnly, $readOnlyFields) {
		$return = array('messages' => array(), 'add' => array(), 'del' => array(), 'mod' => array(), 'info' => array());
		if ($passwordChangeOnly) {
			return $return; // skip processing if only a password change is done
		}
		if (in_array('sshPublicKey', $fields)) {
			$newKeys = explode("\r\n", trim($_POST['ldapPublicKey_sshPublicKey']));
			$count = sizeof($newKeys);
			for ($i = 0; $i < $count; $i++) {
				if (trim($newKeys[$i]) == '') {
					unset($newKeys[$i]);
				}
			}
			$newKeys = array_values(array_unique($newKeys));
			$oldKeys = array();
			if (isset($attributes['sshPublicKey'][0])) {
				$oldKeys = $attributes['sshPublicKey'];
			}
			$update = false;
			if (sizeof($newKeys) != sizeof($oldKeys)) {
				$update = true;
			}
			else {
				for ($i = 0; $i < sizeof($newKeys); $i++) {
					if (!in_array($newKeys[$i], $oldKeys)) {
						$update = true;
						break;
					}
				}
			}
			if ($update) {
				if (sizeof($oldKeys) == 0) {
					$return['add']['sshPublicKey'] = $newKeys;
				}
				elseif (sizeof($newKeys) == 0) {
					$return['del']['sshPublicKey'] = $newKeys;
				}
				else {
					$return['mod']['sshPublicKey'] = $newKeys;
				}
			}
		}
		return $return;
	}

}


?>
