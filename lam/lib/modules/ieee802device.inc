<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2004 - 2016  Roland Gruber

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
class ieee802device extends baseModule {

	/**
	* Returns true if this module can manage accounts of the current type, otherwise false.
	*
	* @return boolean true if module fits
	*/
	public function can_manage() {
		return in_array($this->get_scope(), array('host'));
	}

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
			'macAddress' => array(
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
		$this->messages['macAddress'][0] = array('ERROR', 'MAC address is invalid!');  // third array value is set dynamically
		$this->messages['macAddress'][1] = array('ERROR', _('Account %s:') . ' ieee802Device_mac', 'MAC address is invalid!');
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 *
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		$return = new htmlTable();
		$this->addMultiValueInputTextField($return, 'macAddress', _('MAC address'), false, 17);
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
		$this->processMultiValueInputTextField('macAddress', $errors, 'macAddress');
		// check if MAC is already assigned
		if (!empty($this->attributes['macAddress'])) {
			foreach ($this->attributes['macAddress'] as $macAddress) {
				if (!empty($this->orig['macAddress']) && in_array($macAddress, $this->orig['macAddress'])) {
					// check only new MAC addresses
					continue;
				}
				$result = searchLDAPByAttribute('macAddress', $macAddress, 'ieee802Device', array('dn'), array('host'));
				if (!empty($result)) {
					$errors[] = array('WARN', _('This MAC address is already in use.'), getAbstractDN($result[0]['dn']));
				}
			}
		}
		return $errors;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::build_uploadAccounts()
	 */
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
		$messages = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array("ieee802Device", $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = "ieee802Device";
			}
			// add MACs
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'ieee802Device_mac', 'macAddress', 'macAddress', $this->messages['macAddress'][1], $messages, '/,[ ]*/');
		}
		return $messages;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	function get_pdfEntries($pdfKeys, $typeId) {
		$return = array();
		$this->addSimplePDFField($return, 'macAddress', _('MAC addresses'));
		return $return;
	}

}


?>
