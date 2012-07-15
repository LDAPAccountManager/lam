<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2004 - 2012  Roland Gruber

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
		$return['icon'] = 'network-wired.png';
		// manages host accounts
		$return["account_types"] = array("host");
		// alias name
		$return["alias"] = _("MAC address");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('ieee802Device');
		// managed attributes
		$return['attributes'] = array('macAddress');
		// help Entries
		$return['help'] = array(
			'mac' => array(
				"Headline" => _("MAC address"), 'attr' => 'macAddress',
				"Text" => _("This is the MAC address of the network card of the device (e.g. 00:01:02:DE:EF:18).")
			),
			'macList' => array(
				"Headline" => _("MAC address list"), 'attr' => 'macAddress',
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
			'macAddress' => _('MAC addresses')
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
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		$return = new htmlTable();
		$macCount = 0;
		// list current MACs
		if (isset($this->attributes['macAddress'])) {
			$macCount = sizeof($this->attributes['macAddress']);
			for ($i = 0; $i < sizeof($this->attributes['macAddress']); $i++) {
				$return->addElement(new htmlOutputText(_('MAC address')));
				$macInput = new htmlInputField('macAddress' . $i, $this->attributes['macAddress'][$i]);
				$macInput->setFieldSize(17);
				$macInput->setFieldMaxLength(17);
				$return->addElement($macInput);
				$return->addElement(new htmlButton('delMAC' . $i, 'del.png', true));
				$return->addElement(new htmlHelpLink('mac'), true);
			}
		}
		// input box for new MAC
		$return->addElement(new htmlOutputText(_('New MAC address')));
		$newMacInput = new htmlInputField('macAddress', '');
		$newMacInput->setFieldSize(17);
		$newMacInput->setFieldMaxLength(17);
		$return->addElement($newMacInput);
		$return->addElement(new htmlButton('addMAC', 'add.png', true));
		$return->addElement(new htmlHelpLink('mac'));
		$return->addElement(new htmlHiddenInput('mac_number', $macCount));
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
		$this->attributes['macAddress'] = array();
		// check old MACs
		if (isset($_POST['mac_number'])) {
			for ($i = 0; $i < $_POST['mac_number']; $i++) {
				if (isset($_POST['delMAC' . $i])) continue;
				if (isset($_POST['macAddress' . $i]) && ($_POST['macAddress' . $i] != "")) {
					// check if address has correct format
					if (!get_preg($_POST['macAddress' . $i], 'macAddress')) {
						$message = $this->messages['mac'][0];
						$message[] = $_POST['macAddress' . $i];
						$errors[] = $message;
					}
					$this->attributes['macAddress'][] = $_POST['macAddress' . $i];
				}
			}
		}
		// check new MAC
		if (isset($_POST['macAddress']) && ($_POST['macAddress'] != "")) {
			// check if address has correct format
			if (get_preg($_POST['macAddress'], 'macAddress')) {
				$this->attributes['macAddress'][] = $_POST['macAddress'];
			}
			else {
					$message = $this->messages['mac'][0];
					$message[] = $_POST['macAddress'];
					$errors[] = $message;
			}
		}
		$this->attributes['macAddress'] = array_unique($this->attributes['macAddress']);
		return $errors;
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
	* Returns the PDF entries for this module.
	*
	* @return array list of possible PDF entries
	*/
	function get_pdfEntries() {
		$return = array();
		if (sizeof($this->attributes['macAddress']) > 0) {
			$return['ieee802Device_macAddress'][0] = '<block><key>' . _('MAC addresses') . '</key><value>' . implode(', ', $this->attributes['macAddress']) . '</value></block>';
		}
		return $return;
	}

}


?>
