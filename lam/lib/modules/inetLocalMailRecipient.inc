<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2004 - 2024  Roland Gruber

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

use LAM\TYPES\ConfiguredType;

/**
 * Provides mail routing for users.
 *
 * @package modules
 */
class inetLocalMailRecipient extends baseModule {

	/**
	 * These attributes will be ignored by default if a new account is copied from an existing one.
	 */
	private const ATTRIBUTES_TO_IGNORE_ON_COPY = ['mailLocalAddress', 'mailRoutingAddress'];

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
		return in_array($this->get_scope(), ['user', 'group']);
	}

	/**
	 * Returns meta data that is interpreted by parent class
	 *
	 * @return array array with meta data
	 *
	 * @see baseModule::get_metaData()
	 */
	function get_metaData() {
		$return = [];
		// icon
		$return['icon'] = 'mail.svg';
		// alias name
		$return["alias"] = _("Mail routing");
		// module dependencies
		$return['dependencies'] = ['depends' => [], 'conflicts' => []];
		// managed object classes
		$return['objectClasses'] = ['inetLocalMailRecipient'];
		// managed attributes
		$return['attributes'] = ['mailLocalAddress', 'mailHost', 'mailRoutingAddress'];
		// help Entries
		$return['help'] = [
			'mailRoutingAddress' => [
				"Headline" => _("Routing address"), 'attr' => 'mailRoutingAddress',
				"Text" => _("This is the target email address for the user's mails.")
			],
			'mailLocalAddress' => [
				"Headline" => _("Local address"), 'attr' => 'mailLocalAddress',
				"Text" => _("This is one of the users public email addresses.")
			],
			'mailLocalAddressList' => [
				"Headline" => _("Local address list"), 'attr' => 'mailLocalAddress',
				"Text" => _("This is a comma separated list of the users public email addresses.")
			],
			'mailHost' => [
				"Headline" => _("Mail server"), 'attr' => 'mailHost',
				"Text" => _("This is the mail server for the user.")
			]];
		// profile options
		$profileContainer = new htmlResponsiveRow();
		$profileContainer->add(new htmlResponsiveInputCheckbox('inetLocalMailRecipient_addExt', false, _('Automatically add this extension'), 'autoAdd'), 12);
		$profileContainer->add(new htmlResponsiveInputField(_('Routing address'), 'inetLocalMailRecipient_mailRoutingAddress', null, 'mailRoutingAddress'), 12);
		$profileContainer->add(new htmlResponsiveInputField(_('Local address'), 'inetLocalMailRecipient_mailLocalAddress', null, 'mailLocalAddress'), 12);
		$profileContainer->add(new htmlResponsiveInputField(_('Mail server'), 'inetLocalMailRecipient_host', null, 'mailHost'), 12);
		$return['profile_options'] = $profileContainer;
		// profile checks
		$return['profile_checks']['inetLocalMailRecipient_host'] = [
			'type' => 'ext_preg',
			'regex' => 'DNSname',
			'error_message' => $this->messages['mailHost'][0]];
		// profile mappings
		$return['profile_mappings'] = [
			'inetLocalMailRecipient_host' => 'mailHost',
			'inetLocalMailRecipient_mailRoutingAddress' => 'mailRoutingAddress',
			'inetLocalMailRecipient_mailLocalAddress' => 'mailLocalAddress',
		];
		// upload fields
		$return['upload_columns'] = [
			[
				'name' => 'inetLocalMailRecipient_routingAdr',
				'description' => _('Routing address'),
				'help' => 'mailRoutingAddress',
				'example' => _('smiller@otherdomain.org')
			],
			[
				'name' => 'inetLocalMailRecipient_mailLocalAddress',
				'description' => _('Local address list'),
				'help' => 'mailLocalAddressList',
				'example' => _('smiller@yourdomain.org')
			],
			[
				'name' => 'inetLocalMailRecipient_server',
				'description' => _('Mail server'),
				'help' => 'mailHost',
				'example' => _('mail.yourdomain.org')
			]
		];
		// available PDF fields
		$return['PDF_fields'] = [
			'routingAdr' => _('Routing address'),
			'mailLocalAddress' => _('Local address list'),
			'host' => _('Mail server')
		];
		// self service
		$return['selfServiceFieldSettings'] = [
			'mailLocalAddress' => _('Local address (read-only)'),
			'mailRoutingAddress' => _('Routing address (read-only)'),
		];
		return $return;
	}

	/**
	 * This function fills the error message array with messages
	 */
	function load_Messages() {
		$this->messages['mailRoutingAddress'][0] = ['ERROR', 'Routing address is invalid!'];  // third array value is set dynamically
		$this->messages['mailRoutingAddress'][1] = ['ERROR', _('Account %s:') . ' inetLocalMailRecipient_routingAdr', 'Routing address is invalid!'];
		$this->messages['mailLocalAddress'][0] = ['ERROR', 'Local address is invalid!'];  // third array value is set dynamically
		$this->messages['mailLocalAddress'][1] = ['ERROR', _('Account %s:') . ' inetLocalMailRecipient_mailLocalAddress', 'Local address is invalid!'];
		$this->messages['mailHost'][0] = ['ERROR', 'Mail server is invalid!'];  // third array value is set dynamically
		$this->messages['mailHost'][1] = ['ERROR', _('Account %s:') . ' inetLocalMailRecipient_server', 'Mail server is invalid!'];
	}

	/**
	 * {@inheritDoc}
	 */
	public function loadAttributesFromAccountCopy(array $ldapAttributes, array $attributesToIgnore = []): void {
		$attributesToIgnore = array_merge(baseModule::ATTRIBUTES_TO_IGNORE_ON_COPY_DEFAULT, self::ATTRIBUTES_TO_IGNORE_ON_COPY);
		parent::loadAttributesFromAccountCopy($ldapAttributes, $attributesToIgnore);
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
		$return = new htmlResponsiveRow();
		if (in_array('inetLocalMailRecipient', $this->attributes['objectClass'])) {
			$this->getAccountContainer()->replaceWildcardsInArray(['mailLocalAddress', 'mailRoutingAddress'], $this->attributes);
			// mail routing address
			$this->addSimpleInputTextField($return, 'mailRoutingAddress', _('Routing address'));
			// mail server
			$this->addSimpleInputTextField($return, 'mailHost', _('Mail server'));
			// list current local addresses
			$this->addMultiValueInputTextField($return, 'mailLocalAddress', _('Local address'));

			$return->addVerticalSpacer('2rem');
			$deleteButton = new htmlButton('remObjectClass', _('Remove mail routing extension'));
			$deleteButton->setCSSClasses(['lam-danger']);
			$return->add($deleteButton, 12, 12, 12, 'text-center');
		}
		else {
			$return->add(new htmlButton('addObjectClass', _('Add mail routing extension')));
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
			return [];
		}
		elseif (isset($_POST['remObjectClass'])) {
			$this->attributes['objectClass'] = array_delete(['inetLocalMailRecipient'], $this->attributes['objectClass']);
			for ($i = 0; $i < sizeof($this->meta['attributes']); $i++) {
				if (isset($this->attributes[$this->meta['attributes'][$i]])) {
					unset($this->attributes[$this->meta['attributes'][$i]]);
				}
			}
			return [];
		}
		if (!in_array('inetLocalMailRecipient', $this->attributes['objectClass'])) {
			return [];
		}
		$errors = [];
		$this->getAccountContainer()->replaceWildcardsInPOST(['mailLocalAddress', 'mailRoutingAddress']);
		$this->attributes['mailRoutingAddress'] = [];
		$this->attributes['mailLocalAddress'] = [];
		$this->attributes['mailHost'] = [];
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
		$this->processMultiValueInputTextField('mailLocalAddress', $errors, 'mailLocalAddress');
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
			return [];
		}
		return parent::save_attributes();
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::build_uploadAccounts()
	 */
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
		$messages = [];
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array("inetLocalMailRecipient", $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = "inetLocalMailRecipient";
			}
			// add local addresses
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'inetLocalMailRecipient_mailLocalAddress', 'mailLocalAddress', 'mailLocalAddress', $this->messages['mailLocalAddress'][1], $messages, '/,[ ]*/');
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
		$return = [];
		$this->addSimplePDFField($return, 'routingAdr', _('Routing address'), 'mailRoutingAddress');
		$this->addSimplePDFField($return, 'mailLocalAddress', _('Local address list'), 'mailLocalAddress');
		$this->addSimplePDFField($return, 'host', _('Mail server'), 'mailHost');
		return $return;
	}

	function getSelfServiceOptions($fields, $attributes, $passwordChangeOnly, $readOnlyFields) {
		if ($passwordChangeOnly) {
			return []; // no processing as long no LDAP content can be read
		}
		$return = [];
		if (in_array('mailLocalAddress', $fields)) {
			$mailLocalAddress = [];
			if (isset($attributes['mailLocalAddress'])) {
				$mailLocalAddress = $attributes['mailLocalAddress'];
			}
			array_map('htmlspecialchars', $mailLocalAddress);
			$row = new htmlResponsiveRow();
			$row->addLabel(new htmlOutputText($this->getSelfServiceLabel('mailLocalAddress', _('Local address'))));
			$row->addField(new htmlOutputText(implode('<br>', $mailLocalAddress), false));
			$return['mailLocalAddress'] = $row;
		}
		if (in_array('mailRoutingAddress', $fields)) {
			$mailRoutingAddress = '';
			if (isset($attributes['mailRoutingAddress'][0])) {
				$mailRoutingAddress = $attributes['mailRoutingAddress'][0];
			}
			$row = new htmlResponsiveRow();
			$row->addLabel(new htmlOutputText($this->getSelfServiceLabel('mailRoutingAddress', _('Routing address'))));
			$row->addField(new htmlOutputText($mailRoutingAddress));
			$return['mailRoutingAddress'] = $row;
		}
		return $return;
	}

	/**
	 * @inheritDoc
	 */
	public function getListAttributeDescriptions(ConfiguredType $type): array {
		return [
			'mailroutingaddress' => _('Routing address'),
			'maillocaladdress' => _('Local address list'),
			'mailhost' => _('Mail server')
		];
	}

}
