<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Tilo Lutz
                2005 - 2026  Roland Gruber

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
 * Manages the object class "account" for users and hosts.
 *
 * @package modules
 *
 * @author Tilo Lutz
 * @author Roland Gruber
 * @author Michael Duergner
 */

use LAM\TYPES\ConfiguredType;

/**
 * Manages the object class "account" for users and hosts.
 *
 * @package modules
 */
class account extends baseModule {

	/**
	 * {@inheritDoc}
	 */
	public function can_manage(): bool {
		return in_array($this->get_scope(), ["host", "user"]);
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_metaData(): array {
		$return = [];
		// icon
		$return['icon'] = 'user.svg';
		// alias name
		$return["alias"] = _('Account');
		// this is a base module
		$return["is_base"] = true;
		// LDAP filter
		$return["ldap_filter"] = ['or' => "(objectClass=account)"];
		// RDN attribute
		$return["RDN"] = ["uid" => "low"];
		// module dependencies
		$return['dependencies'] = ['depends' => [], 'conflicts' => []];
		// managed object classes
		$return['objectClasses'] = ['account'];
		// LDAP aliases
		$return['LDAPaliases'] = ['userid' => 'uid'];
		// managed attributes
		$return['attributes'] = ['uid', 'description'];
		// available PDF fields
		$return['PDF_fields'] = [
			'description' => _('Description')
		];
		// help Entries
		$return['help'] = [
			'uidHost' => [
				"Headline" => _("Host name"), 'attr' => 'uid',
				"Text" => _("Host name of the host which should be created. Valid characters are: a-z,A-Z,0-9, .-_$. Host names are always ending with $. If last character is not $ it will be added. If host name is already used host name will be expanded with a number. The next free number will be used.")
			],
			'descriptionHost' => [
				"Headline" => _("Description"), 'attr' => 'description',
				"Text" => _("Host description. If left empty host name will be used.")
			],
			'uidUser' => [
				"Headline" => _("User name"), 'attr' => 'uid',
				"Text" => _("User name of the user who should be created. Valid characters are: a-z,A-Z,0-9, @.-_.")
			],
			'descriptionUser' => [
				"Headline" => _("Description"), 'attr' => 'description',
				"Text" => _("User description. If left empty user name will be used.")
			]
		];
		// upload columns
		$descriptionHelpId = ($this->get_scope() === 'user') ? 'descriptionUser' : 'descriptionHost';
		$return['upload_columns'][] = [
			'name' => 'account_description',
			'description' => _('Description'),
			'help' => $descriptionHelpId,
			'example' => ''
		];
		return $return;
	}

	/**
	 * Returns if the Unix module is active for the current account type.
	 *
	 * @return bool Unix active
	 */
	private function isUnixActive() {
		if ($this->getAccountContainer() != null) {
			return $this->getAccountContainer()->getAccountModule('posixAccount') != null;
		}
		return false;
	}

	/**
	 * {@inheritDoc}
	 */
	protected function load_Messages(): void {
		$this->messages['uid'][0] = ['ERROR', _('User name'), _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !')];
		$this->messages['uid'][1] = ['ERROR', _('Account %s:') . ' posixAccount_userName', _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !')];
		$this->messages['uid'][2] = ['WARN', _('User name'), _('You are using capital letters. This can cause problems because Windows is not case-sensitive.')];
		$this->messages['uid'][3] = ['ERROR', _('User name'), _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !')];
	}

	/**
	 * {@inheritDoc}
	 */
	public function module_complete(): bool {
		return $this->isUnixActive() || !empty($this->attributes['uid'][0]);
	}

	/**
	 * {@inheritDoc}
	 */
	public function getButtonStatus(): string {
		if (!$this->getAccountContainer()->isNewAccount) {
			// check if account is based on our object class
			$objectClasses = $this->getAccountContainer()->attributes_orig['objectClass'];
			if (is_array($objectClasses) && !in_array('account', $objectClasses)) {
				return "disabled";
			}
		}
		return "enabled";
	}

	/**
	 * {@inheritDoc}
	 */
	public function save_attributes(): array {
		// skip saving if account is based on another structural object class
		if (!$this->getAccountContainer()->isNewAccount && !in_array('account', $this->getAccountContainer()->attributes_orig['objectClass'])) {
			return [];
		}
		return $this->getAccountContainer()->save_module_attributes($this->attributes, $this->orig);
	}

	/**
	 * {@inheritDoc}
	 */
	public function process_attributes(): array {
		$errors = [];
		// Load attributes
		$this->attributes['description'][0] = $_POST['description'];
		// user name if no posixAccount
		if (!$this->isUnixActive()) {
			$this->attributes['uid'][0] = $_POST['uid'];
			if (!get_preg($this->attributes['uid'][0], '!upper')) {
				$errors[] = $this->messages['uid'][2];
			}
			if (!get_preg($this->attributes['uid'][0], 'username')) {
				$errors[] = $this->messages['uid'][3];
			}
		}
		return $errors;
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 *
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		$container = new htmlResponsiveRow();
		// user name if no posixAccount
		if (!$this->isUnixActive()) {
			$title = _('User name');
			if ($this->get_scope() == 'host') {
				$title = _('Host name');
			}
			$uidInput = $this->addSimpleInputTextField($container, 'uid', $title, true);
			$uidInput->setFieldMaxLength(100);
			$uidHelpId = ($this->get_scope() === 'user') ? 'uidUser' : 'uidHost';
			$uidInput->setHelpId($uidHelpId);
		}
		// description
		$descriptionInput = $this->addSimpleInputTextField($container, 'description', _('Description'));
		$descriptionHelpId = ($this->get_scope() === 'user') ? 'descriptionUser' : 'descriptionHost';
		$descriptionInput->setHelpId($descriptionHelpId);
		return $container;
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_pdfFields(string $typeId): array {
		$fields = parent::get_pdfFields($typeId);
		if (!$this->isUnixActive()) {
			$fields['uid'] = _('User name');
		}
		return $fields;
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_pdfEntries(array $pdfKeys, string $typeId): array {
		$return = [];
		$this->addSimplePDFField($return, 'description', _('Description'));
		$this->addSimplePDFField($return, 'uid', _('User name'));
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_uploadColumns(array $selectedModules, ConfiguredType $type): array {
		$return = parent::get_uploadColumns($selectedModules, $type);
		if (!in_array('posixAccount', $selectedModules)) {
			$uidHelpId = ($this->get_scope() === 'user') ? 'uidUser' : 'uidHost';
			$return[] = [
				'name' => 'account_uid',
				'description' => _('User name'),
				'help' => $uidHelpId,
				'required' => true
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
			// add object class
			if (!in_array("account", $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = "account";
			}
			// description
			if (isset($rawAccounts[$i][$ids['account_description']]) && ($rawAccounts[$i][$ids['account_description']] !== '')) {
				$partialAccounts[$i]['description'] = $rawAccounts[$i][$ids['account_description']];
			}
			elseif (isset($ids['account_uid']) && isset($rawAccounts[$i][$ids['account_uid']])) {
				$partialAccounts[$i]['description'] = $rawAccounts[$i][$ids['account_uid']];
			}
			elseif (isset($partialAccounts[$i]['uid'])) {
				$partialAccounts[$i]['description'] = $partialAccounts[$i]['uid'];
			}
			if (!in_array('posixAccount', $selectedModules)) {
				// user name
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'account_uid', 'uid', 'username', $this->messages['uid'][1], $messages);
			}
		}
		return $messages;
	}

	/**
	 * @inheritDoc
	 */
	public function getListAttributeDescriptions(ConfiguredType $type): array {
		return [
			"description" => _("Description"),
			'uid' => _("User name")
		];
	}

}
