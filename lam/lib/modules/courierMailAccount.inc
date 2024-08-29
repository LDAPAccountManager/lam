<?php

use LAM\TYPES\ConfiguredType;
use LAM\TYPES\TypeManager;

/*
 * This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
 * Copyright (C) 2017 Lars Althof
 *               2017 - 2024 Roland Gruber
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/**
 * Courier mail extension for users.
 *
 * @package modules
 *
 * @author Lars Althof
 * @author Roland Gruber
 */

/**
 * Courier mail extension for users.
 *
 * @package modules
 *
 * @author Lars Althof
 * @author Roland Gruber
 */
class courierMailAccount extends baseModule {

	/**
	 * These attributes will be ignored by default if a new account is copied from an existing one.
	 */
	private const ATTRIBUTES_TO_IGNORE_ON_COPY = ['mailbox'];

	/**
	 * Creates a new courierMailAccount object.
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
		return $this->get_scope() === 'user';
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_metaData() {
		$return = [];
		// icon
		$return['icon'] = 'mail.svg';
		// alias name
		$return["alias"] = _("Courier");
		// module dependencies
		$return['dependencies'] = [
			'depends' => [
				'inetOrgPerson'
			],
			'conflicts' => []
		];
		// managed object classes
		$return['objectClasses'] = [
			'courierMailAccount'
		];
		// managed attributes
		$return['attributes'] = [
			'mailbox', 'mailhost', 'sharedgroup', 'disableimap',
			'disablepop3', 'disableshared', 'disablewebmail', 'quota'
		];
		// help entries
		$return['help'] = [
			'disableimap' => [
				"Headline" => _("Disable IMAP access"),
				"attr" => 'disableimap',
				"Text" => _("Setting this will disallow the user from logging in using IMAP.")
			],
			'disablepop3' => [
				"Headline" => _("Disable POP3 access"),
				"attr" => 'disablepop3',
				"Text" => _("Setting this will disallow the user from logging in using POP3.")
			],
			'disablewebmail' => [
				"Headline" => _("Disable webmail access"),
				"attr" => 'disablewebmail',
				"Text" => _("Setting this will disallow the user from logging in using webmail.")
			],
			'disableshared' => [
				"Headline" => _("Disable shared folder access"),
				"attr" => 'disableshared',
				"Text" => _("Setting this will make shared folders invisible to this user.")
			],
			'mailbox' => [
				"Headline" => _("Mailbox"),
				"attr" => 'mailbox',
				"Text" => _("Path to the user's mailbox. Usually relative to the user's home folder.")
			],
			'mailhost' => [
				"Headline" => _("Mailbox home server"),
				"attr" => 'mailhost',
				"Text" => _("Name of the host that stores the user's email. Must be a FQDN.")
			],
			'quota' => [
				"Headline" => _("Mailbox quota"),
				"attr" => 'quota',
				"Text" => _("The amount of storage available to the user.")
			],
			'quotaUpload' => [
				"Headline" => _("Mailbox quota"),
				"attr" => 'quota',
				"Text" => _("The amount of storage available to the user.") . ' ' . _('Please enter the amount in MB, e.g. 1000 means 1000MB of quota.')
			],
			'autoAdd' => [
				"Headline" => _("Automatically add this extension"),
				"Text" => _("This will enable the extension automatically if this profile is loaded.")
			],
			'homeDirectory' => [
				"Headline" => _("Home directory"),
				"attr" => 'homeDirectory',
				"Text" => _("The system home directory of the user. In a virtual user setup this is typically '/' for all users.")
			],
		];
		// profile options
		$profileContainer = new htmlResponsiveRow();
		$profileContainer->add(new htmlResponsiveInputCheckbox('courierMailAccount_addExt', false, _('Automatically add this extension'), 'autoAdd'));
		$return['profile_options'] = $profileContainer;
		// PDF fields
		$return['PDF_fields'] = [
			'mailbox' => _('Mailbox'),
			'mailhost' => _('Mailbox home server'),
			'quota' => _('Mailbox quota'),
			'disableimap' => _('Disable IMAP access'),
			'disablepop3' => _('Disable POP3 access'),
			'disablewebmail' => _('Disable webmail access'),
			'disableshared' => _('Disable shared folder access')
		];
		// upload fields
		$return['upload_columns'] = [
			[
				'name' => 'courierMailAccount_mailbox',
				'description' => _('Mailbox'),
				'help' => 'mailbox',
			],
			[
				'name' => 'courierMailAccount_mailhost',
				'description' => _('Mailbox home server'),
				'help' => 'mailhost',
			],
			[
				'name' => 'courierMailAccount_quota',
				'description' => _('Mailbox quota'),
				'help' => 'quotaUpload',
				'example' => '1000'
			],
			[
				'name' => 'courierMailAccount_disableimap',
				'description' => _('Disable IMAP access'),
				'help' => 'disableimap',
				'values' => 'true, false',
				'default' => 'false',
			],
			[
				'name' => 'courierMailAccount_disablepop3',
				'description' => _('Disable POP3 access'),
				'help' => 'disablepop3',
				'values' => 'true, false',
				'default' => 'false',
			],
			[
				'name' => 'courierMailAccount_disablewebmail',
				'description' => _('Disable webmail access'),
				'help' => 'disablewebmail',
				'values' => 'true, false',
				'default' => 'false',
			],
			[
				'name' => 'courierMailAccount_disableshared',
				'description' => _('Disable shared folder access'),
				'help' => 'disableshared',
				'values' => 'true, false',
				'default' => 'false',
			],
		];
		return $return;
	}

	/**
	 * This function fills the error message array with messages
	 */
	public function load_Messages() {
		$this->messages['mailhost'][0] = ['ERROR', _('The mail host is not valid.')]; // third array value is set dynamically
		$this->messages['mailhost'][1] = ['ERROR', _('Account %s:') . ' courierMailAccount_mailhost', _('The mail host is not valid.')];
		$this->messages['quota'][0] = ['ERROR', _('The quota must be entered as a floating point number.')]; // third array value is set dynamically
		$this->messages['quota'][1] = ['ERROR', _('Account %s:') . ' courierMailAccount_quota', _('Please enter a number.')];
		$this->messages['homeDirectory'][0] = ['ERROR', _('Homedirectory contains invalid characters.')]; // third array value is set dynamically
		$this->messages['homeDirectory'][1] = ['ERROR', _('Account %s:') . ' courierMailAccount_homeDirectory', _('Homedirectory contains invalid characters.')];
		$this->messages['boolean'][0] = ['ERROR', _('Account %s:') . ' courierMailAccount_%s', _('This value can only be "true" or "false".')];
	}

	/**
	 * {@inheritDoc}
	 */
	public function loadAttributesFromAccountCopy(array $ldapAttributes, array $attributesToIgnore = []): void {
		$attributesToIgnore = array_merge(baseModule::ATTRIBUTES_TO_IGNORE_ON_COPY_DEFAULT, self::ATTRIBUTES_TO_IGNORE_ON_COPY);
		parent::loadAttributesFromAccountCopy($ldapAttributes, $attributesToIgnore);
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::getManagedAttributes()
	 */
	public function getManagedAttributes($typeId) {
		$attrs = parent::getManagedAttributes($typeId);
		$typeManager = new TypeManager();
		$type = $typeManager->getConfiguredType($typeId);
		if ($type == null) {
			return $attrs;
		}
		$modules = $type->getModules();
		if ($this->isUnixDisabled($modules)) {
			$attrs[] = 'homeDirectory';
		}
		return $attrs;
	}

	/**
	 * {@inheritDoc}
	 */
	public function display_html_attributes() {
		$return = new htmlResponsiveRow();
		if ($this->isExtensionEnabled()) {
			$this->getAccountContainer()->replaceWildcardsInArray(['homeDirectory', 'mailbox'], $this->attributes);
			if ($this->isUnixDisabled($this->getAccountContainer()->get_type()->getModules())) {
				// home directory
				$this->addSimpleInputTextField($return, 'homeDirectory', _('Home directory'), true);
			}
			// show current mailBox
			$this->addSimpleInputTextField($return, 'mailbox', _('Mailbox folder'));
			// mail host input
			$this->addSimpleInputTextField($return, 'mailhost', _('Mailbox host'));
			// quota with selection for unit of measure
			$unitIndex = 2;
			$value = 0;
			if (isset($this->attributes['quota'][0]) && (strlen($this->attributes['quota'][0]) > 0)) {
				$unitIndex = floor(log(substr($this->attributes['quota'][0], 0, -1), 1000));
				$value = round((float) (substr($this->attributes['quota'][0], 0, -1)) / pow(1000, $unitIndex), 2);
			}
			$return->addLabel(new htmlOutputText(_('Mailbox quota')));
			$tempTable = new htmlTable();
			$boxInput = new htmlInputField('quotaValue', $value);
			$boxInput->setFieldSize(5);
			$boxInput->setFieldMaxLength(5);
			$tempTable->addElement($boxInput);
			$units = [
				'B', 'kB', 'MB', 'GB',
				'TB', 'PB', 'EB'
			];
			$selected = [
				$units[$unitIndex]
			];
			$unitSelect = new htmlSelect('quotaUnit', $units, $selected);
			$unitSelect->setSortElements(false);
			$tempTable->addElement($unitSelect);
			$tempTable->addElement(new htmlHelpLink('quota'));
			$return->addField($tempTable);
			// check boxes for account access control
			$disableimap = isset($this->attributes['disableimap'][0]) && ($this->attributes['disableimap'][0] != 0);
			$return->add(new htmlResponsiveInputCheckbox('disableimap', $disableimap, _('Disable IMAP access'), 'disableimap'));
			$disablepop3 = isset($this->attributes['disablepop3'][0]) && ($this->attributes['disablepop3'][0] != 0);
			$return->add(new htmlResponsiveInputCheckbox('disablepop3', $disablepop3, _('Disable POP3 access'), 'disablepop3'));
			$disablewebmail = isset($this->attributes['disablewebmail'][0]) && ($this->attributes['disablewebmail'][0] != 0);
			$return->add(new htmlResponsiveInputCheckbox('disablewebmail', $disablewebmail, _('Disable webmail access'), 'disablewebmail'));
			$disableshared = isset($this->attributes['disableshared'][0]) && ($this->attributes['disableshared'][0] != 0);
			$return->add(new htmlResponsiveInputCheckbox('disableshared', $disableshared, _('Disable shared folder access'), 'disableshared'));
			// remove extension
			$return->addVerticalSpacer('2rem');
			$remButton = new htmlButton('remObjectClass', _('Remove Courier mail extension'));
			$remButton->setCSSClasses(['lam-danger']);
			$return->add($remButton, 12, 12, 12, 'text-center');
		}
		else {
			$return->add(new htmlButton('addObjectClass', _('Add Courier mail extension')));
		}
		return $return;
	}

	/**
	 * {@inheritdoc}
	 */
	public function process_attributes() {
		// add extension
		if (isset($_POST['addObjectClass'])) {
			$this->attributes['objectClass'][] = 'courierMailAccount';
			return [];
		}
		// remove extension
		elseif (isset($_POST['remObjectClass'])) {
			$this->attributes['objectClass'] = array_delete([
				'courierMailAccount'
			], $this->attributes['objectClass']);
			$attrNames = $this->getManagedAttributes($this->getAccountContainer()->get_type()->getId());
			foreach ($attrNames as $attrName) {
				if (isset($this->attributes[$attrName])) {
					unset($this->attributes[$attrName]);
				}
			}
			return [];
		}
		// skip processing if extension is not active
		if (!$this->isExtensionEnabled()) {
			return [];
		}
		$this->getAccountContainer()->replaceWildcardsInPOST(['homeDirectory', 'mailbox']);
		$errors = [];
		// check new mailbox
		$this->attributes['mailbox'][0] = $_POST['mailbox'];
		// quota
		$units = [
			'B', 'kB', 'MB',
			'GB', 'TB', 'PB', 'EB'
		];
		// check new quota
		if (isset($_POST['quotaValue'])) {
			if (is_numeric($_POST['quotaValue']) && in_array($_POST['quotaUnit'], $units)) {
				if ($_POST['quotaValue'] > 0) {
					$power = array_search($_POST['quotaUnit'], $units);
					$this->attributes['quota'][0] = $_POST['quotaValue'] * pow(1000, $power) . "S";
				}
				elseif (isset($this->attributes['quota'])) {
					unset($this->attributes['quota']);
				}
			}
			else {
				$message = $this->messages['quota'][0];
				$message[] = $_POST['quotaValue'];
				$errors[] = $message;
			}
		}
		elseif (isset($this->attributes['quota'])) {
			unset($this->attributes['quota']);
		}
		// mail host
		if (!empty($_POST['mailhost'])) {
			// must be a valid host name
			$this->attributes['mailhost'][0] = $_POST['mailhost'];
			if (!get_preg($this->attributes['mailhost'][0], 'hostname')) {
				$message = $this->messages['mailhost'][0];
				$message[] = $_POST['mailhost'];
				$errors[] = $message;
			}
		}
		elseif (isset($this->attributes['mailhost'])) {
			unset($this->attributes['mailhost']);
		}
		// disable IMAP
		$this->attributes['disableimap'][0] = 0;
		if (isset($_POST['disableimap'])) {
			$this->attributes['disableimap'][0] = 1;
		}
		// disable POP3
		$this->attributes['disablepop3'][0] = 0;
		if (isset($_POST['disablepop3'])) {
			$this->attributes['disablepop3'][0] = 1;
		}
		// disable Webmail
		$this->attributes['disablewebmail'][0] = 0;
		if (isset($_POST['disablewebmail'])) {
			$this->attributes['disablewebmail'][0] = 1;
		}
		// disable shared folder
		$this->attributes['disableshared'][0] = 0;
		if (isset($_POST['disableshared'])) {
			$this->attributes['disableshared'][0] = 1;
		}
		// home directory
		if ($this->isUnixDisabled($this->getAccountContainer()->get_type()->getModules())) {
			// check new home directory
			$this->attributes['homeDirectory'][0] = $_POST['homeDirectory'];
			// check for valid path
			if (!get_preg($this->attributes['homeDirectory'][0], 'filePath')) {
				$message = $this->messages['homeDirectory'][0];
				$message[] = $this->attributes['homeDirectory'][0];
				$errors[] = $message;
			}
		}
		return $errors;
	}

	/**
	 * {@inheritdoc}
	 */
	public function module_ready() {
		$attrs = $this->getAccountContainer()->getAccountModule('inetOrgPerson')->getAttributes();
		return isset($attrs['mail'][0]);
	}

	/**
	 * Returns if the Courier extension is enabled.
	 *
	 * @return boolean Courier extension is active
	 */
	private function isExtensionEnabled() {
		return in_array('courierMailAccount', $this->attributes['objectClass']);
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfFields()
	 */
	public function get_pdfFields($typeId) {
		$fields = parent::get_pdfFields($typeId);
		$typeManager = new TypeManager();
		$modules = $typeManager->getConfiguredType($typeId)->getModules();
		if ($this->isUnixDisabled($modules)) {
			$fields['homeDirectory'] = _('Home directory');
		}
		return $fields;
	}

	/**
	 * {@inheritdoc}
	 */
	public function get_pdfEntries($pdfKeys, $typeId) {
		$return = [];
		$this->addSimplePDFField($return, 'mailbox', _('Mailbox'));
		$this->addSimplePDFField($return, 'mailhost', _('Mailbox home server'));
		$this->addSimplePDFField($return, 'homeDirectory', _('Home directory'));
		$disableOptions = [
			'disableimap' => _('Disable IMAP access'),
			'disablepop3' => _('Disable POP3 access'),
			'disablewebmail' => _('Disable webmail access'),
			'disableshared' => _('Disable shared folder access')
		];
		foreach ($disableOptions as $attrName => $label) {
			$value = _('No');
			if (isset($this->attributes[$attrName][0]) && ($this->attributes[$attrName][0] == '1')) {
				$value = _('Yes');
			}
			$this->addPDFKeyValue($return, $attrName, $label, $value);
		}
		// quota
		$unitIndex = 2;
		$value = 0;
		if (isset($this->attributes['quota'][0]) && (strlen($this->attributes['quota'][0]) > 0)) {
			$unitIndex = floor(log(substr($this->attributes['quota'][0], 0, -1), 1000));
			$value = round((float) (substr($this->attributes['quota'][0], 0, -1)) / pow(1000, $unitIndex), 2);
		}
		$units = [
			'B', 'kB', 'MB', 'GB',
			'TB', 'PB', 'EB'
		];
		$this->addPDFKeyValue($return, 'quota', _('Mailbox quota'), $value . $units[$unitIndex]);
		return $return;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::getManagedAttributes()
	 */
	function get_uploadColumns($selectedModules, &$type) {
		$return = parent::get_uploadColumns($selectedModules, $type);
		// home directory
		if ($this->isUnixDisabled($selectedModules)) {
			$return[] = [
				'name' => 'courierMailAccount_homeDirectory',
				'description' => _('Home directory'),
				'help' => 'homeDirectory',
				'example' => _('/home/smiller'),
				'required' => true
			];
		}
		return $return;
	}

	/**
	 * {@inheritdoc}
	 */
	public function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
		$messages = [];
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array("courierMailAccount", $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = "courierMailAccount";
			}
			// home directory
			if ($this->isUnixDisabled($selectedModules)) {
				if (!get_preg($rawAccounts[$i][$ids['courierMailAccount_homeDirectory']], 'homeDirectory')) {
					$message = $this->messages['homeDirectory'][1];
					$message[] = [$i];
					$messages[] = $message;
				}
				else {
					$partialAccounts[$i]['homeDirectory'] = $rawAccounts[$i][$ids['courierMailAccount_homeDirectory']];
				}
			}
			// mailbox
			if (!empty($rawAccounts[$i][$ids['courierMailAccount_mailbox']])) {
				$partialAccounts[$i]['mailbox'] = $rawAccounts[$i][$ids['courierMailAccount_mailbox']];
			}
			// add mailhost
			if (!empty($rawAccounts[$i][$ids['courierMailAccount_mailhost']])) {
				if (!get_preg($rawAccounts[$i][$ids['courierMailAccount_mailhost']], 'hostname')) {
					$message = $this->messages['mailhost'][1];
					$message[] = [$i];
					$messages[] = $message;
				}
				else {
					$partialAccounts[$i]['mailhost'] = $rawAccounts[$i][$ids['courierMailAccount_mailhost']];
				}
			}
			// mail quota
			if (!empty($rawAccounts[$i][$ids['courierMailAccount_quota']])) {
				if (is_numeric($rawAccounts[$i][$ids['courierMailAccount_quota']])) {
					$partialAccounts[$i]['quota'] = ($rawAccounts[$i][$ids['courierMailAccount_quota']] * 1000000) . "S";
				}
				else {
					$message = $this->messages['quota'][1];
					$message[] = [$i];
					$messages[] = $message;
				}
			}
			// boolean flags
			$flags = [
				'disableimap', 'disablepop3',
				'disablewebmail', 'disableshared'
			];
			$flagValues = ['true', 'false'];
			foreach ($flags as $flag) {
				$partialAccounts[$i][$flag] = 0;
				if (!empty($rawAccounts[$i][$ids['courierMailAccount_' . $flag]])) {
					$flagValue = $rawAccounts[$i][$ids['courierMailAccount_' . $flag]];
					if (!in_array($flagValue, $flagValues)) {
						$message = $this->messages['boolean'][0];
						$message[] = [$i, $flag];
						$messages[] = $message;
					}
					elseif ($flagValue == 'true') {
						$partialAccounts[$i][$flag] = 1;
					}
				}
			}
		}
		return $messages;
	}

	/**
	 * {@inheritdoc}
	 */
	public function load_profile($profile) {
		// profile mappings in meta data
		parent::load_profile($profile);
		// add extension
		if (isset($profile['courierMailAccount_addExt'][0])
			&& ($profile['courierMailAccount_addExt'][0] == "true")
			&& !in_array('courierMailAccount', $this->attributes['objectClass'])) {
			$this->attributes['objectClass'][] = 'courierMailAccount';
		}
	}

	/**
	 * Returns if this account is virtual only or also a unix account
	 *
	 * @param string[] modules
	 * @return boolean only virtual account
	 */
	private function isUnixDisabled($modules) {
		return !in_array('posixAccount', $modules);
	}

	/**
	 * @inheritDoc
	 */
	public function getListAttributeDescriptions(ConfiguredType $type): array {
		return [
			'mailbox' => _('Mailbox'),
			'mailhost' => _('Mailbox home server'),
			'quota' => _('Mailbox quota'),
			'disableimap' => _('Disable IMAP access'),
			'disablepop3' => _('Disable POP3 access'),
			'disablewebmail' => _('Disable webmail access'),
			'disableshared' => _('Disable shared folder access')
		];
	}

}
