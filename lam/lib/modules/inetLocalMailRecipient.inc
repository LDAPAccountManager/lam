<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2004 - 2015  Roland Gruber

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
* Provides mail routing for user accounts.
*
* @package modules
* @author Roland Gruber
*/

/**
* Provides mail routing for users.
*
* @package modules
*/
class inetLocalMailRecipient extends baseModule {

	/**
	* Returns true if this module can manage accounts of the current type, otherwise false.
	*
	* @return boolean true if module fits
	*/
	public function can_manage() {
		return in_array($this->get_scope(), array('user'));
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
		$return['icon'] = 'mailBig.png';
		// alias name
		$return["alias"] = _("Mail routing");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('inetLocalMailRecipient');
		// managed attributes
		$return['attributes'] = array('mailLocalAddress', 'mailHost', 'mailRoutingAddress');
		// help Entries
		$return['help'] = array(
			'mailRoutingAddress' => array(
				"Headline" => _("Routing address"), 'attr' => 'mailRoutingAddress',
				"Text" => _("This is the target email address for the user's mails.")
			),
			'localAdr' => array(
				"Headline" => _("Local address"), 'attr' => 'mailLocalAddress',
				"Text" => _("This is one of the users public email addresses.")
			),
			'localAdrList' => array(
				"Headline" => _("Local address list"), 'attr' => 'mailLocalAddress',
				"Text" => _("This is a comma separated list of the users public email addresses.")
			),
			'mailHost' => array(
				"Headline" => _("Mail server"), 'attr' => 'mailHost',
				"Text" => _("This is the mail server for the user.")
			));
		// profile options
		$profileContainer = new htmlTable();
		$profileContainer->addElement(new htmlTableExtendedInputField(_('Mail server'), 'inetLocalMailRecipient_host', null, 'mailHost'));
		$return['profile_options'] = $profileContainer;
		// profile checks
		$return['profile_checks']['inetLocalMailRecipient_host'] = array(
			'type' => 'ext_preg',
			'regex' => 'DNSname',
			'error_message' => $this->messages['mailHost'][0]);
		// profile mappings
		$return['profile_mappings'] = array(
			'inetLocalMailRecipient_host' => 'mailHost'
		);
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'inetLocalMailRecipient_routingAdr',
				'description' => _('Routing address'),
				'help' => 'mailRoutingAddress',
				'example' => _('smiller@otherdomain.org')
			),
			array(
				'name' => 'inetLocalMailRecipient_localAdr',
				'description' => _('Local address list'),
				'help' => 'localAdrList',
				'example' => _('smiller@yourdomain.org')
			),
			array(
				'name' => 'inetLocalMailRecipient_server',
				'description' => _('Mail server'),
				'help' => 'mailHost',
				'example' => _('mail.yourdomain.org')
			)
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'routingAdr' => _('Routing address'),
			'localAdr' => _('Local address list'),
			'host' => _('Mail server')
		);
		return $return;
	}

	/**
	* This function fills the error message array with messages
	*/
	function load_Messages() {
		$this->messages['mailRoutingAddress'][0] = array('ERROR', 'Routing address is invalid!');  // third array value is set dynamically
		$this->messages['mailRoutingAddress'][1] = array('ERROR', _('Account %s:') . ' inetLocalMailRecipient_routingAdr', 'Routing address is invalid!');
		$this->messages['localAdr'][0] = array('ERROR', 'Local address is invalid!');  // third array value is set dynamically
		$this->messages['localAdr'][1] = array('ERROR', _('Account %s:') . ' inetLocalMailRecipient_localAdr', 'Local address is invalid!');
		$this->messages['mailHost'][0] = array('ERROR', 'Mail server is invalid!');  // third array value is set dynamically
		$this->messages['mailHost'][1] = array('ERROR', _('Account %s:') . ' inetLocalMailRecipient_server', 'Mail server is invalid!');
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 *
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		$return = new htmlTable();
		// mail routing address
		$this->addSimpleInputTextField($return, 'mailRoutingAddress', _('Routing address'));
		// mail server
		$this->addSimpleInputTextField($return, 'mailHost', _('Mail server'));
		// list current local addresses
		$localAdresses = array();
		if (isset($this->attributes['mailLocalAddress'])) $localAdresses = $this->attributes['mailLocalAddress'];
		for ($i = 0; $i < sizeof($localAdresses); $i++) {
			if ($i == 0) {
				$return->addElement(new htmlOutputText(_('Local address')));
			}
			else {
				$return->addElement(new htmlOutputText(''));
			}
			$return->addElement(new htmlInputField('localAdr' . $i, $localAdresses[$i]));
			$return->addElement(new htmlButton('delAdr' . $i, 'del.png', true));
			if ($i == 0) {
				$return->addElement(new htmlHelpLink('localAdr'));
			}
			$return->addNewLine();
		}
		// input box for new local addresses
		$return->addElement(new htmlOutputText(_('New local address')));
		$return->addElement(new htmlInputField('localAdr', ''));
		$return->addElement(new htmlButton('addAdr', 'add.png', true));
		$return->addElement(new htmlHelpLink('localAdr'));
		$return->addElement(new htmlHiddenInput('adr_number', sizeof($localAdresses)));
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
		$this->attributes['mailRoutingAddress'] = array();
		$this->attributes['mailLocalAddress'] = array();
		$this->attributes['mailHost'] = array();
		// check routing address
		if (isset($_POST['mailRoutingAddress']) && ($_POST['mailRoutingAddress'] != "")) {
			// check if address has correct format
			if (get_preg($_POST['mailRoutingAddress'], 'email')) {
				$this->attributes['mailRoutingAddress'][0] = $_POST['mailRoutingAddress'];
			}
			else {
					$message = $this->messages['mailRoutingAddress'][0];
					$message[] = $_POST['mailRoutingAddress'];
					$errors[] = $message;
			}
		}
		// check mail server
		if (isset($_POST['mailHost']) && ($_POST['mailHost'] != "")) {
			// check if address has correct format
			if (get_preg($_POST['mailHost'], 'DNSname')) {
				$this->attributes['mailHost'][0] = $_POST['mailHost'];
			}
			else {
					$message = $this->messages['mailHost'][0];
					$message[] = $_POST['mailHost'];
					$errors[] = $message;
			}
		}
		// check old local addresses
		if (isset($_POST['adr_number'])) {
			for ($i = 0; $i < $_POST['adr_number']; $i++) {
				if (isset($_POST['delAdr' . $i])) continue;
				if (isset($_POST['localAdr' . $i]) && ($_POST['localAdr' . $i] != "")) {
					// check if address has correct format
					if (!get_preg($_POST['localAdr' . $i], 'mailLocalAddress')) {
						$message = $this->messages['localAdr'][0];
						$message[] = $_POST['localAdr' . $i];
						$errors[] = $message;
					}
					$this->attributes['mailLocalAddress'][] = $_POST['localAdr' . $i];
				}
			}
		}
		// check new local address
		if (isset($_POST['localAdr']) && ($_POST['localAdr'] != "")) {
			// check if address has correct format
			if (get_preg($_POST['localAdr'], 'mailLocalAddress')) {
				// check if new address is not already in database
				$data = searchLDAPByAttribute('mailLocalAddress', $_POST['localAdr'], 'inetLocalMailRecipient', array('dn'), array('user'));
				if (sizeof($data) > 0) {
					$errors[] = array('WARN', _('This mail address is already in use:') . " " . $_POST['localAdr'], $data[0]['dn']);
				}
				$this->attributes['mailLocalAddress'][] = $_POST['localAdr'];
			}
			else {
					$message = $this->messages['localAdr'][0];
					$message[] = $_POST['localAdr'];
					$errors[] = $message;
			}
		}
		$this->attributes['mailLocalAddress'] = array_unique($this->attributes['mailLocalAddress']);
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
			if (!in_array("inetLocalMailRecipient", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "inetLocalMailRecipient";
			// add local addresses
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'inetLocalMailRecipient_localAdr', 'mailLocalAddress', 'mailLocalAddress', $this->messages['localAdr'][1], $messages, '/,[ ]*/');
			// add routing address
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'inetLocalMailRecipient_routingAdr', 'mailRoutingAddress',
				'email', $this->messages['mailRoutingAddress'][1], $messages);
			// add mail server
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'inetLocalMailRecipient_server', 'mailHost',
				'DNSname', $this->messages['mailHost'][1], $messages);
		}
		return $messages;
	}

	/**
	 * Returns a list of possible PDF entries for this account.
	 *
	 * @param array $pdfKeys list of PDF keys that are included in document
	 * @return list of PDF entries (array(<PDF key> => <PDF lines>))
	 */
	function get_pdfEntries($pdfKeys) {
		$return = array();
		$this->addSimplePDFField($return, 'routingAdr', _('Routing address'), 'mailRoutingAddress');
		$this->addSimplePDFField($return, 'localAdr', _('Local address list'), 'mailLocalAddress');
		$this->addSimplePDFField($return, 'host', _('Mail server'), 'mailHost');
		return $return;
	}

}


?>
