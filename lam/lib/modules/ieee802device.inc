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
* Provides MAC addresses for hosts.
*
* @package modules
* @author Roland Gruber
*/

/**
* Provides MAC addresses for hosts.
*
* @package modules
*/
class ieee802Device extends baseModule {
	
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
		$return["account_types"] = array("host");
		// alias name
		$return["alias"] = _("MAC address");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// help Entries
		$return['help'] = array(
			'mac' => array(
				"Headline" => _("MAC address"),
				"Text" => _("This is the MAC address of the network card of the device (e.g. 00:01:02:DE:EF:18).")
			),
			'macList' => array(
				"Headline" => _("MAC address list"),
				"Text" => _("This is a comma separated list of MAC addresses.")
			));
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'ieee802Device_mac',
				'description' => _('MAC address'),
				'help' => 'macList',
				'example' => '00:01:02:DE:EF:18'
			)
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'macAddress'
		);
		return $return;
	}

	/**
	* This function fills the error message array with messages
	*/
	function load_Messages() {
		$this->messages['mac'][0] = array('ERROR', 'MAC address is invalid!');  // third array value is set dynamically
		$this->messages['mac'][1] = array('ERROR', _('Account %s:') . ' ieee802Device_mac', 'MAC address is invalid!');
	}
	
	/**
	* This function loads all needed attributes into the object.
	*
	* @param array $attr an array as it is retured from ldap_get_attributes
	*/
	function load_attributes($attr) {
		$this->attributes['objectClass'] = array();
		$this->attributes['macAddress'] = array();
		$this->orig['objectClass'] = array();
		$this->orig['macAddress'] = array();
		if (isset($attr['objectClass'])) {
			unset($attr['objectClass']['count']);
			$this->attributes['objectClass'] = $attr['objectClass'];
			$this->orig['objectClass'] = $attr['objectClass'];
		}
		if (isset($attr['macAddress'])) {
			unset($attr['macAddress']['count']);
			$this->attributes['macAddress'] = $attr['macAddress'];
			$this->orig['macAddress'] = $attr['macAddress'];
		}
		// add object class if needed
		if (! in_array('ieee802Device', $this->orig['objectClass'])) {
			$this->attributes['objectClass'][] = 'ieee802Device';
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
		// list current MACs
		for ($i = 0; $i < sizeof($this->attributes['macAddress']); $i++) {
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('MAC address')),
				1 => array('kind' => 'input', 'name' => 'macAddress' . $i, 'type' => 'text', 'size' => '17', 'maxlength' => '17', 'value' => $this->attributes['macAddress'][$i]),
				2 => array('kind' => 'input', 'type' => 'submit', 'name' => 'delMAC' . $i, 'value' => _("Remove")),
				3 => array('kind' => 'help', 'value' => 'mac'));
		}
		// input box for new MAC
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('New MAC address')),
			1 => array('kind' => 'input', 'name' => 'macAddress', 'type' => 'text', 'size' => '17', 'maxlength' => '17', 'value' => ''),
			2 => array('kind' => 'input', 'type' => 'submit', 'name' => 'addMAC', 'value' => _("Add")),
			3 => array('kind' => 'help', 'value' => 'mac'),
			4 => array('kind' => 'input', 'type' => 'hidden', 'value' => sizeof($this->attributes['macAddress']), 'name' => 'mac_number'));
		return $return;
	}

	/**
	* Write variables into object and do some regex checks
	*
	* @param array $post HTTP-POST values
	*/
	function process_attributes(&$post) {
		$this->triggered_messages = array();
		$this->attributes['macAddress'] = array();
		// check old MACs
		if (isset($post['mac_number'])) {
			for ($i = 0; $i < $post['mac_number']; $i++) {
				if (isset($post['delMAC' . $i])) continue;
				if (isset($post['macAddress' . $i]) && ($post['macAddress' . $i] != "")) {
					// check if address has correct format
					if (!get_preg($post['macAddress' . $i], 'macAddress')) {
						$message = $this->messages['mac'][0];
						$message[] = $post['macAddress' . $i];
						$this->triggered_messages[] = array($message);
					}
					$this->attributes['macAddress'][] = $post['macAddress' . $i];
				}
			}
		}
		// check new MAC
		if (isset($post['macAddress']) && ($post['macAddress'] != "")) {
			// check if address has correct format
			if (get_preg($post['macAddress'], 'macAddress')) {
				$this->attributes['macAddress'][] = $post['macAddress'];
			}
			else {
					$message = $this->messages['mac'][0];
					$message[] = $post['macAddress'];
					$this->triggered_messages[] = array($message);
			}
		}
		$this->attributes['macAddress'] = array_unique($this->attributes['macAddress']);
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
	* Returns true if all settings on module page are correct.
	*/
	function module_ready() {
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
			if (!in_array("ieee802Device", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "ieee802Device";
			// add MACs
			if ($rawAccounts[$i][$ids['ieee802Device_mac']] != "") {
				$macs = explode(',', $rawAccounts[$i][$ids['ieee802Device_mac']]);
				// check format
				for ($m = 0; $m < sizeof($macs); $m++) {
					if (get_preg($macs[$m], 'macAddress')) {
						$partialAccounts[$i]['macAddress'][] = $macs[$m];
					}
					else {
						$errMsg = $this->messages['mac'][1];
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
		if (sizeof($this->attributes['macAddress']) > 0) {
			$return['ieee802Device_macAddress'] = '<block><key>' . _('MAC address list') . '</key><value>' . implode(', ', $this->attributes['macAddress']) . '</value></block>';
		}
		return $return;
	}

}


?>
