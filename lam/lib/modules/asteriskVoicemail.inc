<?php
/*

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2010         Pavel Pozdnyak
                2010 - 2026  Roland Gruber

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
	public function __construct($scope) {
		// call parent constructor
		parent::__construct($scope);
		$this->autoAddObjectClasses = false;
	}

	/**
	 * {@inheritDoc}
	 */
	public function can_manage(): bool {
		return $this->get_scope() === 'user';
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_metaData(): array {
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
		// self-service
		$return['selfServiceFieldSettings'] = [
			'syncAsteriskVoicemailPassword' => _('Sync Asterisk password with Unix password'),
		];
		// self-service: fields that cannot be relabeled
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
		$profileContainer->add(new htmlResponsiveInputField(_('Options'), 'asteriskVoicemail_AstVoicemailOptions', null, 'AstVoicemailOptions'));
		$profileContainer->add(new htmlResponsiveInputField(_('Voicemail context'), 'asteriskVoicemail_AstVoicemailContext', null, 'AstVoicemailContext'));
		$profileContainer->add(new htmlResponsiveInputField(_('Account context'), 'asteriskVoicemail_AstContext', null, 'AstContext'));
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
	 * {@inheritDoc}
	 */
	protected function load_Messages(): void {
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
		parent::loadAttributesFromAccountCopy($ldapAttributes, array_merge(baseModule::ATTRIBUTES_TO_IGNORE_ON_COPY_DEFAULT, self::ATTRIBUTES_TO_IGNORE_ON_COPY));
	}

	/**
	 * {@inheritDoc}
	 */
	public function display_html_attributes(): htmlElement|array {
		$return = new htmlResponsiveRow();
		if (in_array('AsteriskVoiceMail', $this->attributes['objectClass'])) {
			if (!isset($this->attributes['AstVoicemailPassword'])) {
				$message = new htmlStatusMessage($this->messages['AstVoicemailPassword'][0][0], $this->messages['AstVoicemailPassword'][0][1]);
				$return->add($message);
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
			$return->add(new htmlButton('addVoicemailObjectClass', _('Add Asterisk voicemail extension')));
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function process_attributes(): array {
		if (isset($_POST['addVoicemailObjectClass'])) {
			$this->attributes['objectClass'][] = 'AsteriskVoiceMail';
			return [];
		}
		// remove extension
		elseif (isset($_POST['remVoicemailObjectClass'])) {
			$this->attributes['objectClass'] = array_delete(['AsteriskVoiceMail'], $this->attributes['objectClass']);
			$attrKeys = array_keys($this->attributes);
			for ($k = 0; $k < count($attrKeys); $k++) {
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
			elseif (!get_preg($this->attributes['AstVoicemailMailbox'][0], 'username')) {
				$errors[] = $this->messages['AstVoicemailMailbox'][1];
			}
			// check for duplicate Voicemail ID
			elseif (!isset($this->orig['AstVoicemailMailbox'][0]) || ($this->orig['AstVoicemailMailbox'][0] != $this->attributes['AstVoicemailMailbox'][0])) {
				$entries = searchLDAPByAttribute('AstVoicemailMailbox', $this->attributes['AstVoicemailMailbox'][0], 'AstVoicemailMailbox', ['dn'], ['user']);
				if (count($entries) > 0) {
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
	 */
	public function get_pdfEntries(array $pdfKeys, string $typeId): array {
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
	 */
	public function build_uploadAccounts(array $rawAccounts, array $ids, array &$partialAccounts, array $selectedModules, ConfiguredType &$type): array {
		$messages = [];
		for ($i = 0; $i < count($rawAccounts); $i++) {
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
	 * {@inheritDoc}
	 */
	public function module_complete(): bool {
		return (!in_array('AsteriskVoiceMail', $this->attributes['objectClass'])
			|| !empty($this->attributes['AstVoicemailPassword'][0]));
	}

	/**
	 * {@inheritDoc}
	 */
	public function save_attributes(): array {
		if (!in_array('AsteriskVoiceMail', $this->attributes['objectClass']) && !in_array('AsteriskVoiceMail', $this->orig['objectClass'])) {
			// skip saving if the extension was not added/modified
			return [];
		}
		return $this->getAccountContainer()->save_module_attributes($this->attributes, $this->orig);
	}

	/**
	 * {@inheritDoc}
	 */
	public function managesPasswordAttributes(): bool {
		return in_array('AsteriskVoiceMail', $this->attributes['objectClass']);
	}

	/**
	 * {@inheritDoc}
	 */
	public function supportsForcePasswordChange(): bool {
		return false;
	}

	/**
	 * {@inheritDoc}
	 */
	public function passwordChangeRequested(string $password, array $modules, bool $forcePasswordChange): array {
		if (!in_array(static::class, $modules)) {
			return [];
		}
		$this->attributes['AstVoicemailPassword'][0] = $password;
		return [];
	}

	/**
	 * {@inheritDoc}
	 */
	public function checkSelfServiceOptions(array $fields, array $attributes, bool $passwordChangeOnly, array $readOnlyFields): array {
		$return = ['messages' => [], 'add' => [], 'del' => [], 'mod' => [], 'info' => []];
		if (!isset($attributes['objectClass']) || !in_array_ignore_case('AsteriskVoiceMail', $attributes['objectClass'])) {
			return $return;
		}
		if (isset($_POST['posixAccount_password']) && ($_POST['posixAccount_password'] != '')) {
			if ($_POST['posixAccount_password'] != $_POST['posixAccount_password2']) {
				return $return;
			}
			elseif (!get_preg($_POST['posixAccount_password'], 'password')) {
				return $return;
			}
			elseif (in_array('syncAsteriskVoicemailPassword', $fields)) {
				// sync password
				$return['mod']['AstVoicemailPassword'][0] = $_POST['posixAccount_password'];
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
	public function getPasswordQuickChangeOptions(bool $forcePasswordChangeByDefault): array {
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
