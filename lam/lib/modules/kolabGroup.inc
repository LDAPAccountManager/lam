<?php

use LAM\TYPES\ConfiguredType;
use LAM\TYPES\TypeManager;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2013 - 2026  Roland Gruber

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
 * Manages Kolab group accounts.
 *
 * @package modules
 * @author Roland Gruber
 */

/**
 * Manages Kolab group accounts.
 *
 * @package modules
 */
class kolabGroup extends baseModule {

	/** @var string[]|null $mailCache cache for mail attribute */
	private ?array $mailCache = null;

	/**
	 * Creates a new kolabGroup object.
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
		return $this->get_scope() === 'group';
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_metaData(): array {
		$return = [];
		// icon
		$return['icon'] = 'kolab.png';
		// alias name
		$return["alias"] = _("Kolab");
		// module dependencies
		$return['dependencies'] = ['depends' => [], 'conflicts' => []];
		// managed object classes
		$return['objectClasses'] = ['kolabGroupOfUniqueNames'];
		// managed attributes
		$return['attributes'] = ['kolabAllowSMTPRecipient', 'kolabAllowSMTPSender'];
		// help Entries
		$return['help'] = [
			'mail' => [
				"Headline" => _("Email address"), 'attr' => 'mail',
				"Text" => _("The list's email address.")
			],
			'mailList' => [
				"Headline" => _("Email address"), 'attr' => 'mail',
				"Text" => _("The list's email address.") . ' ' . _("Multiple values are separated by semicolon.")
			],
			'kolabAllowSMTPRecipient' => [
				"Headline" => _('Allowed recipients'), 'attr' => 'kolabAllowSMTPRecipient',
				"Text" => _('Describes the allowed or disallowed SMTP recipient addresses for mail sent by this account (e.g. "domain.tld" or "-user@domain.tld").')
			],
			'kolabAllowSMTPRecipientList' => [
				"Headline" => _('Allowed recipients'), 'attr' => 'kolabAllowSMTPRecipient',
				"Text" => _('Describes the allowed or disallowed SMTP recipient addresses for mail sent by this account (e.g. "domain.tld" or "-user@domain.tld").')
					. ' ' . _("Multiple values are separated by semicolon.")
			],
			'kolabAllowSMTPSender' => [
				"Headline" => _('Allowed senders'), 'attr' => 'kolabAllowSMTPSender',
				"Text" => _('Describes the allowed or disallowed SMTP addresses sending mail to this account (e.g. "domain.tld" or "-user@domain.tld").')
			],
			'kolabAllowSMTPSenderList' => [
				"Headline" => _('Allowed senders'), 'attr' => 'kolabAllowSMTPSender',
				"Text" => _('Describes the allowed or disallowed SMTP addresses sending mail to this account (e.g. "domain.tld" or "-user@domain.tld").')
					. ' ' . _("Multiple values are separated by semicolon.")
			],
			'autoAdd' => [
				"Headline" => _("Automatically add this extension"),
				"Text" => _("This will enable the extension automatically if this profile is loaded.")
			],
			'addExtension' => [
				"Headline" => _("Add Kolab extension"),
				"Text" => _("Adds this Kolab extension.")
			],
		];
		// profile options
		$profileContainer = new htmlResponsiveRow();
		$profileContainer->add(new htmlResponsiveInputCheckbox('kolabGroup_addExt', false, _('Automatically add this extension'), 'autoAdd'));
		$return['profile_options'] = $profileContainer;
		// upload fields
		$return['upload_columns'] = [
			[
				'name' => 'kolabGroup_addExtension',
				'description' => _('Add Kolab extension'),
				'help' => 'addExtension',
				'example' => 'false',
				'default' => 'false',
				'values' => 'true, false'
			],
			[
				'name' => 'kolabGroup_kolabAllowSMTPRecipient',
				'description' => _('Allowed recipients'),
				'help' => 'kolabAllowSMTPRecipientList',
				'example' => '.com; -.net',
			],
			[
				'name' => 'kolabGroup_kolabAllowSMTPSender',
				'description' => _('Allowed senders'),
				'help' => 'kolabAllowSMTPSenderList',
				'example' => '.com; -.net',
			],
		];
		// available PDF fields
		$return['PDF_fields'] = [
			'kolabAllowSMTPRecipient' => _('Allowed recipients'),
			'kolabAllowSMTPSender' => _('Allowed senders'),
		];
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	protected function load_Messages(): void {
		$this->messages['mail'][0] = ['ERROR', _('Email address'), _('Please enter a valid email address!')];
		$this->messages['mail'][1] = ['ERROR', _('Account %s:') . ' kolabGroup_mail', _('Please enter a valid email address!')];
		$this->messages['mail'][2] = ['ERROR', _('Email address'), _('Email address already exists.')];
		$this->messages['mail'][3] = ['ERROR', _('Account %s:') . ' kolabGroup_mail', _('Email address already exists.')];
		$this->messages['kolabAllowSMTPRecipient'][0] = ['ERROR', _('Allowed recipients'), _('Please enter a valid recipient expression.')];
		$this->messages['kolabAllowSMTPRecipient'][1] = ['ERROR', _('Account %s:') . ' kolabGroup_kolabAllowSMTPRecipient', _('Please enter a valid recipient expression.')];
		$this->messages['kolabAllowSMTPSender'][0] = ['ERROR', _('Allowed senders'), _('Please enter a valid sender expression.')];
		$this->messages['kolabAllowSMTPSender'][1] = ['ERROR', _('Account %s:') . ' kolabGroup_kolabAllowSMTPSender', _('Please enter a valid sender expression.')];
	}

	/**
	 * {@inheritDoc}
	 */
	public function display_html_attributes(): htmlElement|array {
		$container = new htmlResponsiveRow();
		if (isset($this->attributes['objectClass']) && in_array('kolabGroupOfUniqueNames', $this->attributes['objectClass'])) {
			// mail
			if ($this->manageMail($this->getAccountContainer()->get_type()->getModules())) {
				$this->addSimpleInputTextField($container, 'mail', _('Email address'), true);
			}
			// allowed recipients
			$this->addMultiValueInputTextField($container, 'kolabAllowSMTPRecipient', _('Allowed recipients'));
			// allowed senders
			$this->addMultiValueInputTextField($container, 'kolabAllowSMTPSender', _('Allowed senders'));
		}
		else {
			// add button
			$container->add(new htmlButton('addObjectClass', _('Add Kolab extension')));
		}
		return $container;
	}

	/**
	 * {@inheritDoc}
	 */
	public function process_attributes(): array {
		$errors = [];
		if (isset($_POST['addObjectClass'])) {
			$this->attributes['objectClass'][] = 'kolabGroupOfUniqueNames';
			return $errors;
		}
		if (isset($_POST['remObjectClass'])) {
			$this->attributes['objectClass'] = array_delete(['kolabGroupOfUniqueNames'], $this->attributes['objectClass']);
			$attrs = $this->meta['attributes'];
			foreach ($attrs as $name) {
				if (isset($this->attributes[$name])) {
					unset($this->attributes[$name]);
				}
			}
			return $errors;
		}
		// skip processing if object class is not set
		if (!isset($this->attributes['objectClass']) || !in_array('kolabGroupOfUniqueNames', $this->attributes['objectClass'])) {
			return $errors;
		}
		// mail
		if ($this->manageMail($this->getAccountContainer()->get_type()->getModules())) {
			if (!empty($_POST['mail'])) {
				$this->attributes['mail'][0] = $_POST['mail'];
				// check format
				if (!get_preg($_POST['mail'], 'email')) {
					$errors[] = $this->messages['mail'][0];
				}
				// check if unique
				if ($this->getAccountContainer()->isNewAccount || (!empty($this->orig['mail'][0]) && ($this->orig['mail'][0] != $this->attributes['mail'][0]))) {
					$this->loadMailCache();
					if (in_array_ignore_case($_POST['mail'], $this->mailCache)) {
						$errors[] = $this->messages['mail'][2];
					}
				}
			}
			elseif (isset($this->attributes['mail'])) {
				unset($this->attributes['mail']);
			}
		}
		// allowed recipients
		$this->processMultiValueInputTextField('kolabAllowSMTPRecipient', $errors, 'kolabEmailPrefix');
		// allowed senders
		$this->processMultiValueInputTextField('kolabAllowSMTPSender', $errors, 'kolabEmailPrefix');
		return $errors;
	}

	/**
	 * {@inheritDoc}
	 */
	public function save_attributes(): array {
		if (!in_array('kolabGroupOfUniqueNames', $this->attributes['objectClass']) && !in_array('kolabGroupOfUniqueNames', $this->orig['objectClass'])) {
			// skip saving if the extension was not added/modified
			return [];
		}
		return parent::save_attributes();
	}

	/**
	 * {@inheritDoc}
	 */
	public function load_profile(array $profile): void {
		// profile mappings in meta data
		parent::load_profile($profile);
		// add extension
		if (isset($profile['kolabGroup_addExt'][0])
			&& ($profile['kolabGroup_addExt'][0] == "true")
			&& !in_array('kolabGroupOfUniqueNames', $this->attributes['objectClass'])) {
			$this->attributes['objectClass'][] = 'kolabGroupOfUniqueNames';
		}
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_uploadColumns(array $selectedModules, ConfiguredType $type): array {
		$return = parent::get_uploadColumns($selectedModules, $type);
		if ($this->manageMail($selectedModules)) {
			$return[] = [
				'name' => 'kolabGroup_mail',
				'description' => _('Email address'),
				'help' => 'mailList',
				'example' => 'list@company.com',
				'required' => true,
				'unique' => true,
			];
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function build_uploadAccounts(array $rawAccounts, array $ids, array &$partialAccounts, array $selectedModules, ConfiguredType &$type): array {
		$messages = [];
		for ($i = 0; $i < count($rawAccounts); $i++) {
			// check if extension should be added
			if (isset($rawAccounts[$i][$ids['kolabGroup_addExtension']]) && ($rawAccounts[$i][$ids['kolabGroup_addExtension']] != 'true')) {
				continue;
			}
			// add object classes
			if (!in_array('kolabGroupOfUniqueNames', $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = 'kolabGroupOfUniqueNames';
			}
			// mail
			if ($this->manageMail($selectedModules) && !empty($rawAccounts[$i][$ids['kolabGroup_mail']])) {
				if (get_preg($rawAccounts[$i][$ids['kolabGroup_mail']], 'email')) {
					$this->loadMailCache();
					if (!in_array_ignore_case(trim($rawAccounts[$i][$ids['kolabGroup_mail']]), $this->mailCache)) {
						$partialAccounts[$i]['mail'] = trim($rawAccounts[$i][$ids['kolabGroup_mail']]);
					}
					else {
						$errMsg = $this->messages['mail'][3];
						$errMsg[] = [$i];
						$messages[] = $errMsg;
					}
				}
				else {
					$errMsg = $this->messages['mail'][1];
					$errMsg[] = [$i];
					$messages[] = $errMsg;
				}
			}
			// allowed recipients
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'kolabGroup_kolabAllowSMTPRecipient', 'kolabAllowSMTPRecipient', 'kolabEmailPrefix', $this->messages['kolabAllowSMTPRecipient'][1], $messages, '/;[ ]*/');
			// allowed senders
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'kolabGroup_kolabAllowSMTPSender', 'kolabAllowSMTPSender', 'kolabEmailPrefix', $this->messages['kolabAllowSMTPSender'][1], $messages, '/;[ ]*/');
		}
		return $messages;
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_pdfFields(string $typeId): array {
		$fields = parent::get_pdfFields($typeId);
		$typeManager = new TypeManager();
		$modules = $typeManager->getConfiguredType($typeId)->getModules();
		if ($this->manageMail($modules)) {
			$fields['mail'] = _('Email address');
		}
		return $fields;
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_pdfEntries(array $pdfKeys, string $typeId): array {
		$return = [];
		$this->addSimplePDFField($return, 'mail', _('Email address'));
		$this->addSimplePDFField($return, 'kolabAllowSMTPRecipient', _('Allowed recipients'));
		$this->addSimplePDFField($return, 'kolabAllowSMTPSender', _('Allowed senders'));
		return $return;
	}

	/**
	 * Returns if the mail attribute should be managed.
	 *
	 * @param string[] $modules account modules
	 * @return boolean manage mail attribute
	 */
	private function manageMail($modules) {
		return !in_array('qmailGroup', $modules);
	}

	/**
	 * Loads the list of email addresses into the cache.
	 */
	private function loadMailCache(): void {
		if ($this->mailCache !== null) {
			return;
		}
		$results = searchLDAPByFilter('(mail=*)', ['mail'], [$this->get_scope()]);
		$this->mailCache = [];
		foreach ($results as $result) {
			if (isset($result['mail'][0])) {
				$this->mailCache[] = $result['mail'][0];
			}
		}
	}

	/**
	 * {@inheritDoc}
	 */
	public function getManagedAttributes(string $typeId): array {
		$attrs = parent::getManagedAttributes($typeId);
		$typeManager = new TypeManager();
		if ($this->manageMail($typeManager->getConfiguredType($typeId)->getModules())) {
			$attrs[] = 'mail';
		}
		return $attrs;
	}

}



