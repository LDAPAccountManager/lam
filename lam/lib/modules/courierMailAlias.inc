<?php
/*
 * This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
 * Copyright (C) 2017 Lars Althof
 *               2017 - 2026 Roland Gruber
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
 * Enable the account for Courier Mail Service Aliases
 *
 * @package modules
 *
 * @author Lars Althof
 * @author Roland Gruber
 */

use LAM\TYPES\ConfiguredType;

/**
 * Enable the account for Courier Mail Service Aliases
 *
 * @package modules
 *
 * @author Lars Althof
 * @author Roland Gruber
 */
class courierMailAlias extends baseModule {

	/**
	 * {@inheritDoc}
	 */
	public function can_manage(): bool {
		return $this->get_scope() === 'mailAlias';
	}

	/**
	 * {@inheritdoc}
	 */
	public function get_metaData(): array {
		$return = [];
		// icon
		$return['icon'] = 'mail.svg';
		// alias name
		$return["alias"] = _("Courier");
		// LDAP filter
		$return["ldap_filter"] = [
			'or' => "(objectClass=courierMailAlias)"
		];
		// RDN attribute
		$return["RDN"] = [
			"mail" => "low"
		];
		// module dependencies
		$return['dependencies'] = [
			'depends' => ['nisMailAlias'],
			'conflicts' => []
		];
		// managed object classes
		$return['objectClasses'] = ['CourierMailAlias'];
		// managed attributes
		$return['attributes'] = ['mail', 'maildrop', 'mailsource', 'description'];
		// help Entries
		$return['help'] = [
			'mail' => [
				"Headline" => _("Email address"),
				"attr" => 'mail',
				"Text" => _("This is the email address of the alias.")
			],
			'maildrop' => [
				"Headline" => _("Recipient address"),
				"attr" => 'maildrop',
				"Text" => _("This is the recipient address of the mail alias. There can be more than one.")
			],
			'mailsource' => [
				"Headline" => _("Mail source"),
				"attr" => 'mailsource',
				"Text" => _("The mail source can be 'local' or 'estmp'.")
			],
			'description' => [
				"Headline" => _("Description"),
				"attr" => 'description',
				"Text" => _('This is an optional description for this entry.')
			]
		];
		$return['PDF_fields'] = [
			'mail' => _('Email address'),
			'maildrop' => _('Recipient address'),
			'mailsource' => _('Mail source'),
			'description' => _('Description')
		];
		$return['upload_columns'] = [
			[
				'name' => 'courierMailAlias_mail',
				'description' => _('Email address'),
				'help' => 'mail',
				'example' => _('group@company.com'),
				'required' => true
			],
			[
				'name' => 'courierMailAlias_maildrop',
				'description' => _('Recipient address'),
				'help' => 'maildrop',
				'example' => _('group1@company.com,group2@company.com'),
				'required' => true
			],
			[
				'name' => 'courierMailAlias_mailsource',
				'description' => _('Mail source'),
				'help' => 'mailsource',
				'values' => 'esmtp, local'
			],
			[
				'name' => 'courierMailAlias_description',
				'description' => _('Description'),
				'help' => 'description',
			],
		];
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	protected function load_Messages(): void {
		$this->messages['maildrop'][0] = ['ERROR', _('Recipient address'), _('Please enter a valid email address!')];
		$this->messages['maildrop'][1] = ['ERROR', _('Account %s:') . ' courierMailAlias_maildrop', _('Please enter a valid email address!')];
		$this->messages['mailsource'][0] = ['ERROR', _('Account %s:') . ' courierMailAlias_maildrop', _('Please enter a valid mail source.')];
		$this->messages['mail'][0] = ['ERROR', _('Email address'), _('Please enter a valid email address!')];
		$this->messages['mail'][1] = ['ERROR', _('Account %s:') . ' courierMailAlias_mail', _('Please enter a valid email address!')];
	}

	/**
	 * {@inheritdoc}
	 */
	function display_html_attributes() {
		$return = new htmlResponsiveRow();
		$this->addSimpleInputTextField($return, 'mail', _('Email address'), true);
		$this->addMultiValueInputTextField($return, 'maildrop', _('Recipient address'), true);
		$return->addVerticalSpacer('0.5rem');
		$selectedSource = (empty($this->attributes['mailsource'][0])) ? "-" : $this->attributes['mailsource'][0];
		$return->add(new htmlResponsiveSelect('mailsource', ['-', 'esmtp', 'local'], [$selectedSource], _('Mail source'), 'mailsource'));
		$description = (empty($this->attributes['description'][0])) ? '' : $this->attributes['description'][0];
		$boxInput = new htmlResponsiveInputField(_('Description'), 'description', $description, 'description');
		$boxInput->setFieldMaxLength(100);
		$return->add($boxInput);
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function process_attributes(): array {
		$errors = [];
		$this->attributes['mail'][0] = $_POST['mail'];
		if (empty($this->attributes['mail'][0]) || !get_preg($this->attributes['mail'][0], 'email')) {
			$errors[] = $this->messages['mail'][0];
		}
		$this->processMultiValueInputTextField('maildrop', $errors, 'mailLocalAddress');
		if (empty($this->attributes['maildrop'])) {
			$errors[] = $this->messages['maildrop'][0];
		}
		$this->attributes['mailsource'] = [];
		if ($_POST['mailsource'] !== "-") {
			$this->attributes['mailsource'][0] = $_POST['mailsource'];
		}
		$this->attributes['description'][0] = $_POST['description'];
		return $errors;
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_pdfEntries(array $pdfKeys, string $typeId): array {
		$return = [];
		$this->addSimplePDFField($return, 'mail', _('Email address'));
		$this->addSimplePDFField($return, 'maildrop', _('Recipient address'));
		$this->addSimplePDFField($return, 'mailsource', _('Mail source'));
		$this->addSimplePDFField($return, 'description', _('Description'));
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function build_uploadAccounts(array $rawAccounts, array $ids, array &$partialAccounts, array $selectedModules, ConfiguredType &$type): array {
		$messages = [];
		$possibleSources = ['esmtp', 'local'];
		for ($i = 0; $i < count($rawAccounts); $i++) {
			if (!in_array("courierMailAlias", $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = "courierMailAlias";
			}
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'courierMailAlias_mail', 'mail', 'email', $this->messages['mail'][1], $messages);
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'courierMailAlias_maildrop', 'maildrop', 'mailLocalAddress', $this->messages['maildrop'][1], $messages, '/,[ ]?/');
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'courierMailAlias_description', 'description');
			if (!empty($rawAccounts[$i][$ids['courierMailAlias_mailsource']])) {
				if (in_array($rawAccounts[$i][$ids['courierMailAlias_mailsource']], $possibleSources)) {
					$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'courierMailAlias_mailsource', 'mailsource');
				}
				else {
					$error = $this->messages['mailsource'][0];
					$error[] = [$i];
					$messages[] = $error;
				}
			}
		}
		return $messages;
	}

	/**
	 * @inheritDoc
	 */
	public function getListAttributeDescriptions(ConfiguredType $type): array {
		return [
			'mail' => _('Email address'),
			'maildrop' => _('Recipient address'),
			'mailsource' => _('Mail source'),
			'description' => _('Description')
		];
	}

	/**
	 * @inheritDoc
	 */
	public function getListRenderFunction(string $attributeName, ConfiguredType $type): ?callable {
		if ($attributeName === 'mail') {
			return function(array $entry, string $attribute): htmlElement {
				$group = new htmlGroup();
				if (isset($entry[$attribute][0]) && ($entry[$attribute][0] != '')) {
					for ($i = 0; $i < count($entry[$attribute]); $i++) {
						if ($i > 0) {
							$group->addElement(new htmlOutputText(", "));
						}
						$group->addElement(new htmlLink($entry[$attribute][$i], "mailto:" . $entry[$attribute][$i]));
					}
				}
				return $group;
			};
		}
		return null;
	}

}
