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
	* Creates a new mitKerberos object.
	*
	* @param string $scope account type (user, group, host)
	*/
	function __construct($scope) {
		// call parent constructor
		parent::__construct($scope);
		$this->autoAddObjectClasses = false;
	}

	/**
	* Returns true if this module can manage accounts of the current type, otherwise false.
	*
	* @return boolean true if module fits
	*/
	public function can_manage() {
		return in_array($this->get_scope(), array('user', 'group'));
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
		$profileContainer->addElement(new htmlTableExtendedInputCheckbox('inetLocalMailRecipient_addExt', false, _('Automatically add this extension'), 'autoAdd'), true);
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
	* Loads the values of an account profile into internal variables.
	*
	* @param array $profile hash array with profile values (identifier => value)
	*/
	function load_profile($profile) {
		// profile mappings in meta data
		parent::load_profile($profile);
		// add extension
		if (isset($profile['inetLocalMailRecipient_addExt'][0]) && ($profile['inetLocalMailRecipient_addExt'][0] == "true")) {
			if (!in_array('inetLocalMailRecipient', $this->attributes['objectClass'])) {
				$this->attributes['objectClass'][] = 'inetLocalMailRecipient';
			}
		}
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 *
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		$return = new htmlTable();
		if (in_array('inetLocalMailRecipient', $this->attributes['objectClass'])) {
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

			$return->addElement(new htmlSpacer(null, '10px'), true);
			$deleteButton = new htmlButton('remObjectClass', _('Remove mail routing extension'));
			$deleteButton->colspan = 3;
			$return->addElement($deleteButton);
		}
		else {
			$return->addElement(new htmlButton('addObjectClass', _('Add mail routing extension')));
		}
		return $return;
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_attributes() {
		if (isset($_POST['addObjectClass'])) {
			$this->attributes['objectClass'][] = 'inetLocalMailRecipient';
			return array();
		}
		elseif (isset($_POST['remObjectClass'])) {
			$this->attributes['objectClass'] = array_delete(array('inetLocalMailRecipient'), $this->attributes['objectClass']);
			for ($i = 0; $i < sizeof($this->meta['attributes']); $i++) {
				if (isset($this->attributes[$this->meta['attributes'][$i]])) {
					unset($this->attributes[$this->meta['attributes'][$i]]);
				}
			}
			return array();
		}
		if (!in_array('inetLocalMailRecipient', $this->attributes['objectClass'])) {
			return array();
		}
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
				$data = searchLDAPByAttribute('mailLocalAddress', $_POST['localAdr'], 'inetLocalMailRecipient', array('dn'), array($this->get_scope()));
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
	* Returns a list of modifications which have to be made to the LDAP account.
	*
	* @return array list of modifications
	* <br>This function returns an array with 3 entries:
	* <br>array( DN1 ('add' => array($attr), 'remove' => array($attr), 'modify' => array($attr)), DN2 .... )
	* <br>DN is the DN to change. It may be possible to change several DNs (e.g. create a new user and add him to some groups via attribute memberUid)
	* <br>"add" are attributes which have to be added to LDAP entry
	* <br>"remove" are attributes which have to be removed from LDAP entry
	* <br>"modify" are attributes which have to been modified in LDAP entry
	* <br>"info" are values with informational value (e.g. to be used later by pre/postModify actions)
	*/
	function save_attributes() {
		if (!in_array('inetLocalMailRecipient', $this->attributes['objectClass']) && !in_array('inetLocalMailRecipient', $this->orig['objectClass'])) {
			// skip saving if the extension was not added/modified
			return array();
		}
		return parent::save_attributes();
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::build_uploadAccounts()
	 */
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
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
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	function get_pdfEntries($pdfKeys, $typeId) {
		$return = array();
		$this->addSimplePDFField($return, 'routingAdr', _('Routing address'), 'mailRoutingAddress');
		$this->addSimplePDFField($return, 'localAdr', _('Local address list'), 'mailLocalAddress');
		$this->addSimplePDFField($return, 'host', _('Mail server'), 'mailHost');
		return $return;
	}

}


?>
