<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2004 - 2026  Roland Gruber

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
 * Provides NIS mail alias management.
 *
 * @package modules
 * @author Roland Gruber
 */

use LAM\TYPES\ConfiguredType;

/**
 * Provides NIS mail alias management.
 *
 * @package modules
 */
class nisMailAlias extends baseModule {

	/** @var string[]|null mail cache */
	private ?array $cachedMailList = null;
	/** @var string[]|null $cachedUserList user cache */
	private ?array $cachedUserList = null;
	/** display limit */
	private const DISPLAY_LIMIT = 50;

	/**
	 * {@inheritDoc}
	 */
	public function can_manage(): bool {
		return $this->get_scope() === 'mailAlias';
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_metaData(): array {
		$return = [];
		// icon
		$return['icon'] = 'mail.svg';
		// base module
		$return["is_base"] = true;
		// LDAP filter
		$return["ldap_filter"] = ['or' => "(objectClass=nisMailAlias)"];
		// alias name
		$return["alias"] = _("Mail aliases");
		// RDN attribute
		$return["RDN"] = ["cn" => "normal"];
		// module dependencies
		$return['dependencies'] = ['depends' => [], 'conflicts' => []];
		// managed object classes
		$return['objectClasses'] = ['nisMailAlias'];
		// managed attributes
		$return['attributes'] = ['cn', 'rfc822MailMember'];
		// help Entries
		$return['help'] = [
			'alias' => [
				"Headline" => _("Alias name"), 'attr' => 'cn',
				"Text" => _("Mails to this name are forwarded to the recipients.")
			],
			'recipient' => [
				"Headline" => _("Recipients"), 'attr' => 'rfc822MailMember',
				"Text" => _("Please enter the recipients for this alias.")
			],
			'recipientList' => [
				"Headline" => _("Recipient list"), 'attr' => 'rfc822MailMember',
				"Text" => _("This is a comma separated list of recipients.")
			],
			'filter' => [
				"Headline" => _("Filter"),
				"Text" => _("Here you can enter a filter value. Only entries which contain the filter text will be shown.")
			],
		];
		// upload fields
		$return['upload_columns'] = [
			[
				'name' => 'nisMailAlias_alias',
				'description' => _('Alias name'),
				'help' => 'alias',
				'example' => 'root',
				'required' => true
			],
			[
				'name' => 'nisMailAlias_recipients',
				'description' => _('Recipient list'),
				'help' => 'recipientList',
				'example' => _('smiller')
			]
		];
		// available PDF fields
		$return['PDF_fields'] = [
			'alias' => _('Alias name'),
			'recipients' => _('Recipient list')
		];
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	protected function load_Messages(): void {
		$this->messages['alias'][0] = ['ERROR', _('Alias is empty or invalid!')];  // third array value is set dynamically
		$this->messages['alias'][1] = ['ERROR', _('Account %s:') . ' nisMailAlias_alias', _('Alias is empty or invalid!')];
		$this->messages['recipient'][0] = ['ERROR', _('Recipient is invalid!')];  // third array value is set dynamically
		$this->messages['recipient'][1] = ['ERROR', _('Account %s:') . ' nisMailAlias_recipient', _('Recipient is invalid!')];
	}

	/**
	 * {@inheritDoc}
	 */
	public function save_attributes(): array {
		// skip saving if account is based on another structural object class
		if (!$this->getAccountContainer()->isNewAccount && !in_array('nisMailAlias', $this->getAccountContainer()->attributes_orig['objectClass'])) {
			return [];
		}
		return parent::save_attributes();
	}

	/**
	 * {@inheritDoc}
	 */
	public function display_html_attributes(): htmlElement|array {
		$return = new htmlResponsiveRow();
		// alias name
		$alias = '';
		if (isset($this->attributes['cn'][0])) {
			$alias = $this->attributes['cn'][0];
		}
		$nameInput = new htmlResponsiveInputField(_('Alias name'), 'cn', $alias, 'alias');
		$nameInput->setRequired(true);
		$nameInput->setCSSClasses(['maxwidth20']);
		$return->add($nameInput);
		// list current recipients
		$mailList = $this->getMailList();
		$userList = $this->getUserList();
		$autoList = [];
		if ((count($userList) + count($mailList)) < 300) {
			$autoList = array_merge($userList, $mailList);
		}
		$recipientCount = 0;
		if (isset($this->attributes['rfc822MailMember'])) {
			natcasesort($this->attributes['rfc822MailMember']);
			$this->attributes['rfc822MailMember'] = array_values($this->attributes['rfc822MailMember']);
			$recipientCount = count($this->attributes['rfc822MailMember']);
			if ($recipientCount < self::DISPLAY_LIMIT) {
				for ($i = 0; $i < $recipientCount; $i++) {
					if ($i === 0) {
						if ($recipientCount === 1) {
							$return->addLabel(new htmlOutputText(_('Recipient')));
						}
						else {
							$return->addLabel(new htmlOutputText(_('Recipients')));
						}
					}
					else {
						$return->addLabel(new htmlOutputText('&nbsp;', false));
					}
					$mailField = new htmlInputField('rfc822MailMember' . $i, $this->attributes['rfc822MailMember'][$i]);
					if ($autoList !== []) {
						$mailField->enableAutocompletion($autoList);
					}
					$mailField->setCSSClasses(['maxwidth20']);
					$mailGroup = new htmlGroup();
					$mailGroup->addElement($mailField);
					$mailGroup->addElement(new htmlAccountPageButton(static::class, 'selectMail', 'recipient' . $i, 'mail.svg', true, _('Select mail')));
					$mailGroup->addElement(new htmlAccountPageButton(static::class, 'selectUser', 'recipient' . $i, 'user.svg', true, _('Select user')));
					$mailGroup->addElement(new htmlButton('delRec' . $i, 'del.svg', true));
					$mailGroup->addElement(new htmlHelpLink('recipient'));
					$return->addField($mailGroup);
				}
			}
			else {
				$memberText = implode("\r\n", $this->attributes['rfc822MailMember']);
				$return->add(new htmlResponsiveInputTextarea('rfc822MailMember', $memberText, 50, 30, _('Recipients'), 'recipient'));
			}
		}
		if ($recipientCount < self::DISPLAY_LIMIT) {
			// input box for new recipient
			$return->addLabel(new htmlOutputText(_('New recipient')));
			$newMailField = new htmlInputField('rfc822MailMember');
			$newMailField->setOnKeyPress('SubmitForm(\'addRec\', event);');
			if ($autoList !== []) {
				$newMailField->enableAutocompletion($autoList);
			}
			$newMailField->setCSSClasses(['maxwidth20']);
			$newGroup = new htmlGroup();
			$newGroup->addElement($newMailField);
			$newGroup->addElement(new htmlAccountPageButton(static::class, 'selectMail', 'recipientNew', 'mail.svg', true, _('Select mail')));
			$newGroup->addElement(new htmlAccountPageButton(static::class, 'selectUser', 'recipientNew', 'user.svg', true, _('Select user')));
			$newGroup->addElement(new htmlButton('addRec', 'add.svg', true));
			$newGroup->addElement(new htmlHelpLink('recipient'));
			$newGroup->addElement(new htmlHiddenInput('rec_number', (string) $recipientCount));
			$return->addField($newGroup);
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function process_attributes(): array {
		$errors = [];
		$this->attributes['cn'] = [];
		$recipientCount = empty($this->attributes['rfc822MailMember']) ? 0 : count($this->attributes['rfc822MailMember']);
		$this->attributes['rfc822MailMember'] = [];
		// check alias name
		$this->attributes['cn'][] = $_POST['cn'];
		if (!isset($_POST['cn']) || ($_POST['cn'] == "") || !get_preg($_POST['cn'], 'nis_alias')) {
			$message = $this->messages['alias'][0];
			$message[] = $_POST['cn'];
			$errors[] = $message;
		}
		if ($recipientCount < self::DISPLAY_LIMIT) {
			// check old recipients
			if (isset($_POST['rec_number'])) {
				for ($i = 0; $i < $_POST['rec_number']; $i++) {
					if (isset($_POST['delRec' . $i])) {
						continue;
					}
					if (isset($_POST['rfc822MailMember' . $i]) && ($_POST['rfc822MailMember' . $i] != "")) {
						// check if address has correct format
						if (!get_preg($_POST['rfc822MailMember' . $i], 'nis_recipient') && !get_preg($_POST['rfc822MailMember' . $i], 'email')) {
							$message = $this->messages['recipient'][0];
							$message[] = $_POST['rfc822MailMember' . $i];
							$errors[] = $message;
						}
						$this->attributes['rfc822MailMember'][] = $_POST['rfc822MailMember' . $i];
					}
				}
			}
			// check new recipient
			if (isset($_POST['rfc822MailMember']) && ($_POST['rfc822MailMember'] != "")) {
				// check if recipient has correct format
				if (get_preg($_POST['rfc822MailMember'], 'nis_recipient') || get_preg($_POST['rfc822MailMember'], 'email')) {
					$this->attributes['rfc822MailMember'][] = $_POST['rfc822MailMember'];
				}
				else {
					$message = $this->messages['recipient'][0];
					$message[] = $_POST['rfc822MailMember'];
					$errors[] = $message;
				}
			}
		}
		else {
			$recipients = explode("\r\n", $_POST['rfc822MailMember']);
			foreach ($recipients as $recipient) {
				$recipient = trim($recipient);
				if (empty($recipient)) {
					continue;
				}
				$this->attributes['rfc822MailMember'][] = $recipient;
				// check if recipient has correct format
				if (!get_preg($recipient, 'nis_recipient') && !get_preg($recipient, 'email')) {
					$message = $this->messages['recipient'][0];
					$message[] = htmlspecialchars($recipient);
					$errors[] = $message;
				}
			}
		}
		$this->attributes['rfc822MailMember'] = array_unique($this->attributes['rfc822MailMember']);
		return $errors;
	}

	/**
	 * Displays the mail selection.
	 *
	 * @return htmlElement meta HTML code
	 */
	public function display_html_selectMail(): htmlElement {
		return $this->display_html_select(true);
	}

	/**
	 * Processes user input of the host/user selection page.
	 * It checks if all input values are correct and updates the associated LDAP attributes.
	 *
	 * @return array<string|string[]>[] list of info/error messages
	 */
	public function process_selectMail(): array {
		return $this->process_select();
	}

	/**
	 * Displays the user selection.
	 *
	 * @return htmlElement meta HTML code
	 */
	public function display_html_selectUser(): htmlElement {
		return $this->display_html_select(false);
	}

	/**
	 * Processes user input of the host/user selection page.
	 * It checks if all input values are correct and updates the associated LDAP attributes.
	 *
	 * @return array<string|string[]>[] list of info/error messages
	 */
	public function process_selectUser(): array {
		return $this->process_select();
	}

	/**
	 * Displays the user/mail selection.
	 *
	 * @param boolean $isMail mail selection (user selection if false)
	 * @return htmlElement meta HTML code
	 */
	public function display_html_select($isMail): htmlElement {
		if ($isMail) {
			$regex = 'email';
			$options = $this->getMailList();
			$suffix = 'Mail';
			$label = _('Email');
		}
		else {
			$regex = 'username';
			$options = $this->getUserList();
			$suffix = 'User';
			$label = _('User');
		}
		$return = new htmlResponsiveRow();
		$postKeys = array_keys($_POST);
		$position = 'New';
		for ($i = 0; $i < count($postKeys); $i++) {
			if (str_starts_with($postKeys[$i], 'form_subpage_' . static::class . '_select' . $suffix . '_recipient')) {
				$position = substr($postKeys[$i], strlen('form_subpage_' . static::class . '_select' . $suffix . '_recipient'));
				break;
			}
		}
		// load list with all mail addresses
		$count = count($options);
		for ($i = 0; $i < $count; $i++) {
			if (!get_preg($options[$i], $regex)) {
				unset($options[$i]);
			}
		}
		$options = array_values($options);
		$return->addLabel(new htmlOutputText(_('Filter')));
		$filterGroup = new htmlGroup();
		$filterInput = new htmlInputField('filter', null);
		$filterInput->setCSSClasses(['maxwidth10']);
		$filterInput->filterSelectBox('selectBox');
		$filterGroup->addElement($filterInput);
		$filterGroup->addElement(new htmlHelpLink('filter'));
		$return->addField($filterGroup);
		$return->addLabel(new htmlOutputText($label));
		$mailSelect = new htmlSelect('selectBox', $options, [], 15);
		$mailSelect->setMultiSelect($position === 'New');
		$mailSelect->colspan = 5;
		$return->addField($mailSelect);
		$return->addVerticalSpacer('2rem');
		$return->addLabel(new htmlAccountPageButton(static::class, 'attributes', 'select', _('Ok')));
		$return->addField(new htmlAccountPageButton(static::class, 'attributes', 'back', _('Cancel')));
		$return->add(new htmlHiddenInput('position', $position));
		return $return;
	}

	/**
	 * Processes user input of the host/user selection page.
	 * It checks if all input values are correct and updates the associated LDAP attributes.
	 *
	 * @return array<string|string[]>[] list of info/error messages
	 */
	public function process_select(): array {
		if (isset($_POST['form_subpage_' . static::class . '_attributes_select'])) {
			if (!isset($this->attributes['rfc822MailMember'])) {
				$this->attributes['rfc822MailMember'] = [];
			}
			$position = $_POST['position'];
			if ($position == 'New') {
				$this->attributes['rfc822MailMember'] = array_merge($this->attributes['rfc822MailMember'], $_POST['selectBox']);
			}
			else {
				$this->attributes['rfc822MailMember'][$_POST['position']] = $_POST['selectBox'];
			}
			return [];
		}
		return [];
	}

	/**
	 * {@inheritDoc}
	 */
	public function getButtonStatus(): string {
		if (!$this->getAccountContainer()->isNewAccount) {
			// check if account is based on our object class
			$objectClasses = $this->getAccountContainer()->attributes_orig['objectClass'];
			if (is_array($objectClasses) && !in_array('nisMailAlias', $objectClasses)) {
				return "disabled";
			}
		}
		return "enabled";
	}

	/**
	 * {@inheritDoc}
	 */
	public function build_uploadAccounts(array $rawAccounts, array $ids, array &$partialAccounts, array $selectedModules, ConfiguredType &$type): array {
		$messages = [];
		for ($i = 0; $i < count($rawAccounts); $i++) {
			// add object class
			if (!in_array("nisMailAlias", $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = "nisMailAlias";
			}
			// add alias name
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'nisMailAlias_alias', 'cn',
				'nis_alias', $this->messages['alias'][1], $messages);
			// add recipients
			if ($rawAccounts[$i][$ids['nisMailAlias_recipients']] != "") {
				$aliases = explode(',', $rawAccounts[$i][$ids['nisMailAlias_recipients']]);
				// check format
				for ($a = 0; $a < count($aliases); $a++) {
					if (get_preg($aliases[$a], 'nis_recipient') || get_preg($aliases[$a], 'email')) {
						$partialAccounts[$i]['rfc822MailMember'][] = $aliases[$a];
					}
					else {
						$errMsg = $this->messages['recipient'][1];
						$errMsg[] = [$i];
						$messages[] = $errMsg;
					}
				}
			}
		}
		return $messages;
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_pdfEntries(array $pdfKeys, string $typeId): array {
		$return = [];
		$this->addSimplePDFField($return, 'alias', _('Alias name'), 'cn');
		$this->addSimplePDFField($return, 'recipients', _('Recipient list'), 'rfc822MailMember');
		return $return;
	}

	/**
	 * Returns a list of existing email addresses.
	 *
	 * @return string[] email addresses
	 */
	private function getMailList(): array {
		if ($this->cachedMailList !== null) {
			return $this->cachedMailList;
		}
		$this->cachedMailList = [];
		$entries = searchLDAPByAttribute('mail', '*', null, ['mail'], ['user']);
		foreach ($entries as $entry) {
			$this->cachedMailList[] = $entry['mail'][0];
		}
		$this->cachedMailList = array_values(array_unique($this->cachedMailList));
		return $this->cachedMailList;
	}

	/**
	 * Returns a list of existing usernames.
	 *
	 * @return string[] usernames
	 */
	private function getUserList(): array {
		if ($this->cachedUserList !== null) {
			return $this->cachedUserList;
		}
		$this->cachedUserList = [];
		$result = searchLDAPByFilter('(|(objectClass=posixAccount)(objectClass=inetOrgPerson))', ['uid'], ['user']);
		$count = count($result);
		for ($i = 0; $i < $count; $i++) {
			if (empty($result[$i]['uid'][0])) {
				unset($result[$i]);
				continue;
			}
			$this->cachedUserList[] = $result[$i]['uid'][0];
		}
		$this->cachedUserList = array_values(array_unique($this->cachedUserList));
		return $this->cachedUserList;
	}

	/**
	 * @inheritDoc
	 */
	public function getListAttributeDescriptions(ConfiguredType $type): array {
		return [
			"cn" => _("Alias name"),
			"rfc822mailmember" => _("Recipient list")
		];
	}

}
