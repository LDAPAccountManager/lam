<?php
/*

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2010         Pavel Pozdnyak
                2010 - 2024  Roland Gruber

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
 * Manages the Asterisk extension of user accounts.
 *
 * @package modules
 *
 * @author Pavel Pozdnyak
 * @author Roland Gruber
 */

use LAM\TYPES\ConfiguredType;

/**
 * Manages the Asterisk extension of user accounts.
 *
 * @package modules
 */
class asteriskVoicemail extends baseModule implements passwordService {

	/**
	 * These attributes will be ignored by default if a new account is copied from an existing one.
	 */
	private const ATTRIBUTES_TO_IGNORE_ON_COPY = ['AstVoicemailMailbox', 'AstVoicemailPassword',
		'AstVoicemailEmail', 'AstVoicemailPager', 'AstVoicemailFullname'];

	/**
	 * Creates a new asteriskVoicemail object.
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
		return in_array($this->get_scope(), ['user']);
	}

	/**
	 * Returns meta data that is interpreted by parent class
	 *
	 * @return array array with meta data
	 */
	function get_metaData() {
		$return = [];
		$return["is_base"] = false;
		// alias name
		$return["alias"] = _("Asterisk voicemail");
		// module dependencies
		$return['dependencies'] = ['depends' => ['inetOrgPerson'], 'conflicts' => []];
		// managed object classes
		$return['objectClasses'] = ['AsteriskVoiceMail'];
		// managed attributes
		$return['attributes'] = ['AstContext', 'AstVoicemailMailbox',
			'AstVoicemailPassword', 'AstVoicemailFullname', 'AstVoicemailEmail',
			'AstVoicemailPager', 'AstVoicemailOptions', 'AstVoicemailContext'];
		// icon
		$return['icon'] = 'asterisk.png';
		// self service
		$return['selfServiceFieldSettings'] = [
			'syncAsteriskVoicemailPassword' => _('Sync Asterisk password with Unix password'),
		];
		// self service: fields that cannot be relabeled
		$return['selfServiceNoRelabelFields'] = ['syncAsteriskVoicemailPassword'];
		// help
		$return['help'] = [
			'AstContext' => [
				"Headline" => _("Account context"), 'attr' => 'AstContext',
				"Text" => _("The account context stores information about the dial plan.")
			],
			'AstVoicemailMailbox' => [
				"Headline" => _("Mailbox"), 'attr' => 'AstVoicemailMailbox',
				"Text" => _("Voicemail mailbox for this account.")
			],
			'AstVoicemailPassword' => [
				"Headline" => _("Password"), 'attr' => 'AstVoicemailPassword',
				"Text" => _("Password for voicemail mailbox.")
			],
			'AstVoicemailFullname' => [
				"Headline" => _("Full name"), 'attr' => 'AstVoicemailFullname',
				"Text" => _("Full name for Asterisk voicemail mailbox.")
			],
			'AstVoicemailEmail' => [
				"Headline" => _("Email address"), 'attr' => 'AstVoicemailEmail',
				"Text" => _("Email address for this voicemail account.")
			],
			'AstVoicemailPager' => [
				"Headline" => _("Pager"), 'attr' => 'AstVoicemailPager',
				"Text" => _("Pager number for Asterisk voicemail.")
			],
			'AstVoicemailOptions' => [
				"Headline" => _("Options"), 'attr' => 'AstVoicemailOptions',
				"Text" => _("Options for Asterisk voicemail account (e.g. sendvoicemail=yes).")
			],
			'AstVoicemailContext' => [
				"Headline" => _("Voicemail context"), 'attr' => 'AstVoicemailContext',
				"Text" => _("Asterisk voicemail context.")
			],
		];
		// profile options
		$profileContainer = new htmlResponsiveRow();
		$profileContainer->add(new htmlResponsiveInputField(_('Options'), 'asteriskVoicemail_AstVoicemailOptions', null, 'AstVoicemailOptions'), 12);
		$profileContainer->add(new htmlResponsiveInputField(_('Voicemail context'), 'asteriskVoicemail_AstVoicemailContext', null, 'AstVoicemailContext'), 12);
		$profileContainer->add(new htmlResponsiveInputField(_('Account context'), 'asteriskVoicemail_AstContext', null, 'AstContext'), 12);
		$return['profile_options'] = $profileContainer;
		// profile mappings
		$return['profile_mappings'] = [
			'asteriskVoicemail_AstContext' => 'AstContext',
			'asteriskVoicemail_AstVoicemailContext' => 'AstVoicemailContext',
			'asteriskVoicemail_AstVoicemailOptions' => 'AstVoicemailOptions'
		];
		// available PDF fields
		$return['PDF_fields'] = [
			'AstContext' => _('Account context'),
			'AstVoicemailMailbox' => _('Mailbox'),
			'AstVoicemailFullname' => _('Full name'),
			'AstVoicemailEmail' => _('Email address'),
			'AstVoicemailPager' => _('Pager'),
			'AstVoicemailOptions' => _('Options'),
			'AstVoicemailContext' => _('Voicemail context')
		];
		// upload dependencies
		$return['upload_preDepends'] = ['posixAccount', 'inetOrgPerson'];
		// upload fields
		$return['upload_columns'] = [
			[
				'name' => 'asteriskVoicemail_AstContext',
				'description' => _('Account context'),
				'help' => 'AstContext',
				'example' => 'default',
				'required' => true
			],
			[
				'name' => 'asteriskVoicemail_AstVoicemailMailbox',
				'description' => _('Mailbox'),
				'help' => 'AstVoicemailMailbox',
				'example' => '12345',
				'required' => true
			],
			[
				'name' => 'asteriskVoicemail_AstVoicemailPassword',
				'description' => _('Password'),
				'help' => 'AstVoicemailPassword',
				'example' => _('secret'),
				'required' => true
			],
			[
				'name' => 'asteriskVoicemail_AstVoicemailFullname',
				'description' => _('Full name'),
				'help' => 'AstVoicemailFullname',
				'example' => _('Steve Miller')
			],
			[
				'name' => 'asteriskVoicemail_AstVoicemailEmail',
				'description' => _('Email address'),
				'help' => 'AstVoicemailEmail',
				'example' => _('user@company.com')
			],
			[
				'name' => 'asteriskVoicemail_AstVoicemailPager',
				'description' => _('Pager'),
				'help' => 'AstVoicemailPager',
				'example' => _('123-123-1234')
			],
			[
				'name' => 'asteriskVoicemail_AstVoicemailOptions',
				'description' => _('Options'),
				'help' => 'AstVoicemailOptions',
				'example' => 'sendvoicemail=yes'
			],
			[
				'name' => 'asteriskVoicemail_AstVoicemailContext',
				'description' => _('Voicemail context'),
				'help' => 'AstVoicemailContext',
				'example' => 'default'
			]
		];
		return $return;
	}

	/**
	 * This function fills the error message array with messages
	 */
	function load_Messages() {
		//messages for voicemail
		$this->messages['AstVoicemailMailbox'][0] = ['ERROR', _('Please enter a mailbox.')];
		$this->messages['AstVoicemailMailbox'][1] = ['ERROR', _('The mailbox format is invalid.')];
		$this->messages['AstVoicemailMailbox'][2] = ['ERROR', _('There is already another user with this mailbox name.')];
		$this->messages['AstVoicemailMailbox'][3] = ['ERROR', _('Account %s:') . ' asteriskVoicemail_AstVoicemailMailbox', _('The mailbox format is invalid.')];

		$this->messages['AstContext'][0] = ['ERROR', _('Please enter the account context.')];
		$this->messages['AstContext'][1] = ['ERROR', _('The account context is invalid.')];
		$this->messages['AstContext'][2] = ['ERROR', _('Account %s:') . ' asteriskVoicemail_AstContext', _('The account context is invalid.')];

		$this->messages['AstVoicemailFullname'][0] = ['ERROR', _('The full name is invalid.')];
		$this->messages['AstVoicemailFullname'][1] = ['ERROR', _('Account %s:') . ' asteriskVoicemail_AstVoicemailFullname', _('The full name is invalid.')];
		$this->messages['AstVoicemailEmail'][0] = ['ERROR', _('The email address is invalid.')];
		$this->messages['AstVoicemailEmail'][1] = ['ERROR', _('Account %s:') . ' asteriskVoicemail_AstVoicemailEmail', _('The email address is invalid.')];
		$this->messages['AstVoicemailPager'][0] = ['ERROR', _('The pager number has bad format.')];
		$this->messages['AstVoicemailPager'][1] = ['ERROR', _('Account %s:') . ' asteriskVoicemail_AstVoicemailPager', _('The pager number has bad format.')];
		$this->messages['AstVoicemailOptions'][0] = ['ERROR', _('The options have bad format.')];
		$this->messages['AstVoicemailOptions'][1] = ['ERROR', _('Account %s:') . ' asteriskVoicemail_AstVoicemailOptions', _('The options have bad format.')];
		$this->messages['AstVoicemailContext'][0] = ['ERROR', _('The voicemail context name is invalid.')];
		$this->messages['AstVoicemailContext'][1] = ['ERROR', _('Account %s:') . ' asteriskVoicemail_AstVoicemailContext', _('The voicemail context name is invalid.')];
		$this->messages['AstVoicemailPassword'][0] = ['INFO', _('Please set the voicemail password with "Set password" before saving.')];
	}

	/**
	 * {@inheritDoc}
	 */
	public function loadAttributesFromAccountCopy(array $ldapAttributes, array $attributesToIgnore = []): void {
		$attributesToIgnore = array_merge(baseModule::ATTRIBUTES_TO_IGNORE_ON_COPY_DEFAULT, self::ATTRIBUTES_TO_IGNORE_ON_COPY);
		parent::loadAttributesFromAccountCopy($ldapAttributes, $attributesToIgnore);
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 *
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		$return = new htmlResponsiveRow();
		if (in_array('AsteriskVoiceMail', $this->attributes['objectClass'])) {
			if (!isset($this->attributes['AstVoicemailPassword'])) {
				$message = new htmlStatusMessage($this->messages['AstVoicemailPassword'][0][0], $this->messages['AstVoicemailPassword'][0][1]);
				$return->add($message, 12);
			}
			// mailbox
			$this->addSimpleInputTextField($return, 'AstVoicemailMailbox', _("Mailbox"), true);
			// account context
			$this->addSimpleInputTextField($return, 'AstContext', _("Account context"), true);

			$return->addVerticalSpacer('1rem');

			// full name
			$this->addSimpleInputTextField($return, 'AstVoicemailFullname', _("Full name"));
			// email
			$this->addSimpleInputTextField($return, 'AstVoicemailEmail', _("Email address"));
			// pager
			$this->addSimpleInputTextField($return, 'AstVoicemailPager', _("Pager"));
			// options
			$this->addSimpleInputTextField($return, 'AstVoicemailOptions', _("Options"));
			// voicemail context
			$this->addSimpleInputTextField($return, 'AstVoicemailContext', _("Voicemail context"));
			// remove button
			$return->addVerticalSpacer('2rem');
			$remButton = new htmlButton('remVoicemailObjectClass', _('Remove Asterisk voicemail extension'));
			$remButton->setCSSClasses(['lam-danger']);
			$return->add($remButton, 12, 12, 12, 'text-center');
		}
		else {
			// add button
			$return->add(new htmlButton('addVoicemailObjectClass', _('Add Asterisk voicemail extension')), 12);
		}
		return $return;
	}

	/**
	 * Write variables into object and do some regex checks
	 */
	function process_attributes() {
		if (isset($_POST['addVoicemailObjectClass'])) {
			$this->attributes['objectClass'][] = 'AsteriskVoiceMail';
			return [];
		}
		// remove extension
		elseif (isset($_POST['remVoicemailObjectClass'])) {
			$this->attributes['objectClass'] = array_delete(['AsteriskVoiceMail'], $this->attributes['objectClass']);
			$attrKeys = array_keys($this->attributes);
			for ($k = 0; $k < sizeof($attrKeys); $k++) {
				if (strpos($attrKeys[$k], 'AstVoicemail') > -1) {
					unset($this->attributes[$attrKeys[$k]]);
				}
			}
			if (isset($this->attributes['AstContext'])) {
				unset($this->attributes['AstContext']);
			}
			return [];
		}
		$errors = [];
		if (!in_array('AsteriskVoiceMail', $this->attributes['objectClass'])) {
			return [];
		}
		$this->attributes['AstVoicemailMailbox'] = [];
		$this->attributes['AstVoicemailFullname'] = [];
		$this->attributes['AstVoicemailEmail'] = [];
		$this->attributes['AstVoicemailPager'] = [];
		$this->attributes['AstVoicemailOptions'] = [];
		$this->attributes['AstVoicemailContext'] = [];

		if (isset($_POST['AstContext'])) {
			$this->attributes['AstContext'][0] = $_POST['AstContext'];
			if ($this->attributes['AstContext'][0] == '') {
				$errors[] = $this->messages['AstContext'][0];
			}
			elseif (!get_preg($this->attributes['AstContext'][0], 'realname')) {
				$errors[] = $this->messages['AstContext'][1];
			}
		}
		if (isset($_POST['AstVoicemailMailbox'])) {
			$this->attributes['AstVoicemailMailbox'][0] = $_POST['AstVoicemailMailbox'];
			// check if caller ID is empty
			if ($this->attributes['AstVoicemailMailbox'][0] == '') {
				$errors[] = $this->messages['AstVoicemailMailbox'][0];
			}
			// check format
			else if (!get_preg($this->attributes['AstVoicemailMailbox'][0], 'username')) {
				$errors[] = $this->messages['AstVoicemailMailbox'][1];
			}
			// check for duplicate Voicemail ID
			else if (!isset($this->orig['AstVoicemailMailbox'][0]) || ($this->orig['AstVoicemailMailbox'][0] != $this->attributes['AstVoicemailMailbox'][0])) {
				$entries = searchLDAPByAttribute('AstVoicemailMailbox', $this->attributes['AstVoicemailMailbox'][0], 'AstVoicemailMailbox', ['dn'], ['user']);
				if (sizeof($entries) > 0) {
					$errors[] = $this->messages['AstVoicemailMailbox'][2];
				}
			}
		}
		if (isset($_POST['AstVoicemailFullname'])) {
			$this->attributes['AstVoicemailFullname'][0] = $_POST['AstVoicemailFullname'];
			if ($this->attributes['AstVoicemailFullname'][0] != '' && !get_preg($this->attributes['AstVoicemailFullname'][0], 'realname')) {
				$errors[] = $this->messages['AstVoicemailFullname'][0];
			}
		}
		if (isset($_POST['AstVoicemailEmail'])) {
			$this->attributes['AstVoicemailEmail'][0] = $_POST['AstVoicemailEmail'];
			if ($this->attributes['AstVoicemailEmail'][0] != '' && !get_preg($this->attributes['AstVoicemailEmail'][0], 'email')) {
				$errors[] = $this->messages['AstVoicemailEmail'][0];
			}
		}
		if (isset($_POST['AstVoicemailPager'])) {
			$this->attributes['AstVoicemailPager'][0] = $_POST['AstVoicemailPager'];
			if ($this->attributes['AstVoicemailPager'][0] != '' && !get_preg($this->attributes['AstVoicemailPager'][0], 'telephone')) {
				$errors[] = $this->messages['AstVoicemailPager'][0];
			}
		}
		if (isset($_POST['AstVoicemailOptions'])) {
			$this->attributes['AstVoicemailOptions'][0] = $_POST['AstVoicemailOptions'];
		}
		if (isset($_POST['AstVoicemailContext'])) {
			$this->attributes['AstVoicemailContext'][0] = $_POST['AstVoicemailContext'];
			if ($this->attributes['AstVoicemailContext'][0] != '' && !get_preg($this->attributes['AstVoicemailContext'][0], 'realname')) {
				$errors[] = $this->messages['AstVoicemailContext'][0];
			}
		}
		return $errors;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	function get_pdfEntries($pdfKeys, $typeId) {
		$return = [];
		$this->addSimplePDFField($return, 'AstVoicemailMailbox', _('Mailbox'));
		$this->addSimplePDFField($return, 'AstContext', _('Account context'));
		$this->addSimplePDFField($return, 'AstVoicemailFullname', _('Full name'));
		$this->addSimplePDFField($return, 'AstVoicemailContext', _('Voicemail context'));
		$this->addSimplePDFField($return, 'AstVoicemailPager', _('Pager'));
		$this->addSimplePDFField($return, 'AstVoicemailEmail', _('Email address'));
		$this->addSimplePDFField($return, 'AstVoicemailOptions', _('Options'));
		return $return;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::build_uploadAccounts()
	 */
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
		$messages = [];
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array("AsteriskVoiceMail", $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = "AsteriskVoiceMail";
			}

			//add context
			if (empty($rawAccounts[$i][$ids['asteriskVoicemail_AstContext']])) {
				// default value
				$partialAccounts[$i]['AstContext'] = 'default';
			}
			else {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'asteriskVoicemail_AstContext', 'AstContext',
					'realname', $this->messages['AstContext'][2], $messages);
			}

			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'asteriskVoicemail_AstVoicemailMailbox', 'AstVoicemailMailbox',
				'realname', $this->messages['AstVoicemailMailbox'][3], $messages);

			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'asteriskVoicemail_AstVoicemailEmail', 'AstVoicemailEmail',
				'email', $this->messages['AstVoicemailEmail'][1], $messages);

			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'asteriskVoicemail_AstVoicemailPager', 'AstVoicemailPager',
				'telephone', $this->messages['AstVoicemailPager'][1], $messages);

			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'asteriskVoicemail_AstVoicemailOptions', 'AstVoicemailOptions');

			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'asteriskVoicemail_AstVoicemailFullname', 'AstVoicemailFullname',
				'realname', $this->messages['AstVoicemailFullname'][1], $messages);

			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'asteriskVoicemail_AstVoicemailContext', 'AstVoicemailContext',
				'realname', $this->messages['AstVoicemailContext'][1], $messages);

			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'asteriskVoicemail_AstVoicemailPassword', 'AstVoicemailPassword');
		}
		return $messages;
	}

	/**
	 * This functions returns true if all needed settings are done.
	 *
	 * @return boolean true if LDAP operation can be done
	 */
	function module_complete() {
		if (in_array('AsteriskVoiceMail', $this->attributes['objectClass']) && empty($this->attributes['AstVoicemailPassword'][0])) {
			return false;
		}
		return true;
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
		if (!in_array('AsteriskVoiceMail', $this->attributes['objectClass']) && !in_array('AsteriskVoiceMail', $this->orig['objectClass'])) {
			// skip saving if the extension was not added/modified
			return [];
		}
		return $this->getAccountContainer()->save_module_attributes($this->attributes, $this->orig);
	}

	/**
	 * This method specifies if a module manages password attributes.
	 * @return boolean true if this module manages password attributes
	 * @see passwordService::managesPasswordAttributes
	 *
	 */
	public function managesPasswordAttributes() {
		if (!in_array('AsteriskVoiceMail', $this->attributes['objectClass'])) {
			return false;
		}
		return true;
	}

	/**
	 * Specifies if this module supports to force that a user must change his password on next login.
	 *
	 * @return boolean force password change supported
	 */
	public function supportsForcePasswordChange() {
		return false;
	}

	/**
	 * This function is called whenever the password should be changed. Account modules
	 * must change their password attributes only if the modules list contains their module name.
	 *
	 * @param String $password new password
	 * @param $modules list of modules for which the password should be changed
	 * @param boolean $forcePasswordChange force the user to change his password at next login
	 * @return array list of error messages if any as parameter array for StatusMessage
	 *               e.g. return array(array('ERROR', 'Password change failed.'))
	 * @see passwordService::passwordChangeRequested
	 */
	public function passwordChangeRequested($password, $modules, $forcePasswordChange) {
		if (!in_array(get_class($this), $modules)) {
			return [];
		}
		$this->attributes['AstVoicemailPassword'][0] = $password;
		return [];
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
	 * @return array messages and attributes (array('messages' => [], 'add' => array('mail' => array('test@test.com')), 'del' => [], 'mod' => [], 'info' => []))
	 */
	function checkSelfServiceOptions($fields, $attributes, $passwordChangeOnly, $readOnlyFields) {
		$return = ['messages' => [], 'add' => [], 'del' => [], 'mod' => [], 'info' => []];
		if (!isset($attributes['objectClass']) || !in_array_ignore_case('AsteriskVoiceMail', $attributes['objectClass'])) {
			return $return;
		}
		if (isset($_POST['posixAccount_password']) && ($_POST['posixAccount_password'] != '')) {
			if ($_POST['posixAccount_password'] != $_POST['posixAccount_password2']) {
				return $return;
			}
			else {
				if (!get_preg($_POST['posixAccount_password'], 'password')) {
					return $return;
				}
				else {
					// sync password
					if (in_array('syncAsteriskVoicemailPassword', $fields)) {
						$return['mod']['AstVoicemailPassword'][0] = $_POST['posixAccount_password'];
					}
				}
			}
		}
		return $return;
	}

	/**
	 * @inheritDoc
	 */
	public function supportsPasswordQuickChangePage(): bool {
		return true;
	}

	/**
	 * @inheritDoc
	 */
	public function addPasswordQuickChangeAccountDetails(htmlResponsiveRow $row): void {
		// no details
	}

	/**
	 * @inheritDoc
	 */
	public function getPasswordQuickChangeOptions(): array {
		$options = [];
		if (in_array_ignore_case('AsteriskVoiceMail', $this->attributes['objectClass'])) {
			$options[] = new PasswordQuickChangeOption('syncAsteriskVoiceMail', _('Change Asterisk voicemail password'));
		}
		return $options;
	}

	/**
	 * @inheritDoc
	 */
	public function getPasswordQuickChangeChanges(string $password): array {
		$attrs = [];
		if (isset($_POST['syncAsteriskVoiceMail'])) {
			$attrs['AstVoicemailPassword'][0] = $password;
		}
		return $attrs;
	}

	/**
	 * @inheritDoc
	 */
	public function getPasswordQuickChangePasswordStrengthUserName(): ?string {
		return null;
	}

	/**
	 * @inheritDoc
	 */
	public function getPasswordQuickChangePasswordStrengthAttributes(): array {
		return [];
	}

	/**
	 * @inheritDoc
	 */
	public function getPasswordQuickChangeIsPasswordInHistory(string $password): bool {
		return false;
	}

	/**
	 * @inheritDoc
	 */
	public function getListAttributeDescriptions(ConfiguredType $type): array {
		return [
			'astcontext' => _('Account context'),
			'astvoicemailmailbox' => _('Mailbox'),
			'astvoicemailfullname' => _('Full name'),
			'astvoicemailemail' => _('Email address'),
			'astvoicemailpager' => _('Pager'),
			'astvoicemailoptions' => _('Options'),
			'astvoicemailcontext' => _('Voicemail context')
		];
	}

}
