<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2005 - 2006  Roland Gruber

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
				'example' => _('ssh-dss 234234 user@host')
			)
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'sshPublicKey'
		);
		return $return;
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return array HTML meta data
	 */
	function display_html_attributes() {
		$return = array();
		// list current keys
		for ($i = 0; $i < sizeof($this->attributes['sshPublicKey']); $i++) {
			$return[] = array(
				array('kind' => 'text', 'text' => _('SSH public key')),
				array('kind' => 'input', 'name' => 'sshPublicKey' . $i, 'type' => 'text', 'size' => '50', 'maxlength' => '2048', 'value' => $this->attributes['sshPublicKey'][$i]),
				array('kind' => 'input', 'type' => 'submit', 'name' => 'delKey' . $i, 'value' => _("Remove")),
				array('kind' => 'help', 'value' => 'key'));
		}
		// input box for new key
		$return[] = array(
			array('kind' => 'text', 'text' => _('New SSH public key')),
			array('kind' => 'input', 'name' => 'sshPublicKey', 'type' => 'text', 'size' => '50', 'maxlength' => '2048', 'value' => ''),
			array('kind' => 'input', 'type' => 'submit', 'name' => 'addKey', 'value' => _("Add")),
			array('kind' => 'help', 'value' => 'key'),
			array('kind' => 'input', 'type' => 'hidden', 'value' => sizeof($this->attributes['sshPublicKey']), 'name' => 'key_number'));
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
	* @param array $partialAccounts list of hash arrays (name => value) which are later added to LDAP
	* @param array $ids list of IDs for column position (e.g. "posixAccount_uid" => 5)
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
			$return['ldapPublicKey_sshPublicKey'][0] = '<block><key>' . _('SSH public key(s)') . '</key><tr><td align=\"L\">' . $this->attributes['sshPublicKey'][0] . '</td></tr></block>';
			for ($i = 1; $i < sizeof($this->attributes['sshPublicKey']); $i++) {
				$return['ldapPublicKey_sshPublicKey'][] = '<block><tr><td align=\"L\">' . $this->attributes['sshPublicKey'][$i] . '</td></tr></block>';
			}
		}
		return $return;
	}

}


?>
