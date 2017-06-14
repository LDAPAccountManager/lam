<?php
/*
 * This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
 * Copyright (C) 2017 Lars Althof
 *               2017 Roland Gruber
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
	 * {@inheritdoc}
	 */
	public function can_manage() {
		return in_array($this->get_scope(), array('mailAlias'));
	}

	/**
	 * {@inheritdoc}
	 */
	function get_metaData() {
		$return = array();
		// icon
		$return['icon'] = 'courierMail.png';
		// alias name
		$return["alias"] = _("Courier");
		// LDAP filter
		$return["ldap_filter"] = array(
			'or' => "(objectClass=courierMailAlias)"
		);
		// RDN attribute
		$return["RDN"] = array(
			"mail" => "low"
		);
		// module dependencies
		$return['dependencies'] = array(
			'depends' => array('nisMailAlias'),
			'conflicts' => array()
		);
		// managed object classes
		$return['objectClasses'] = array('CourierMailAlias');
		// managed attributes
		$return['attributes'] = array('mail', 'maildrop', 'mailsource', 'description');
		// help Entries
		$return['help'] = array(
			'mail' => array(
				"Headline" => _("Email address"),
				"attr" => 'mail',
				"Text" => _("This is the email address of the alias.")
			),
			'maildrop' => array(
				"Headline" => _("Recipient address"),
				"attr" => 'maildrop',
				"Text" => _("This is the recipient address of the mail alias. There can be more than one.")
			),
			'mailsource' => array(
				"Headline" => _("Mail source"),
				"attr" => 'mailsource',
				"Text" => _("The mail source can be 'local' or 'estmp'.")
			),
			'description' => array(
				"Headline" => _("Description"),
				"attr" => 'description',
				"Text" => _('This is an optional description for this entry.')
			)
		);
		$return['PDF_fields'] = array(
			'mail' => _('Email address'),
			'maildrop' => _('Recipient address'),
			'mailsource' => _('Mail source'),
			'description' => _('Description')
		);
		$return['upload_columns'] = array(
			array(
				'name' => 'courierMailAlias_mail',
				'description' => _('Email address'),
				'help' => 'mail',
				'example' => _('group@company.com'),
				'required' => true
			),
			array(
				'name' => 'courierMailAlias_maildrop',
				'description' => _('Recipient address'),
				'help' => 'maildrop',
				'example' => _('group1@company.com,group2@company.com'),
				'required' => true
			),
			array(
				'name' => 'courierMailAlias_mailsource',
				'description' => _('Mail source'),
				'help' => 'mailsource',
				'values' => 'esmtp, local'
			),
			array(
				'name' => 'courierMailAlias_description',
				'description' => _('Description'),
				'help' => 'description',
			),
		);
		return $return;
	}

	/**
	 * {@inheritdoc}
	 */
	function load_Messages() {
		$this->messages['maildrop'][0] = array('ERROR', _('Recipient address'), _('Please enter a valid email address!'));
		$this->messages['maildrop'][1] = array('ERROR', _('Account %s:') . ' courierMailAlias_maildrop', _('Please enter a valid email address!'));
		$this->messages['mailsource'][0] = array('ERROR', _('Account %s:') . ' courierMailAlias_maildrop', _('Please enter a valid mail source.'));
		$this->messages['mail'][0] = array('ERROR', _('Email address'), _('Please enter a valid email address!'));
		$this->messages['mail'][1] = array('ERROR', _('Account %s:') . ' courierMailAlias_mail', _('Please enter a valid email address!'));
	}

	/**
	 * {@inheritdoc}
	 */
	function display_html_attributes() {
		$return = new htmlTable();
		$mail = (!empty($this->attributes['mail'][0])) ? $this->attributes['mail'][0] : '';
		$boxInput = new htmlTableExtendedInputField (_('Email address'), 'mail', $mail, 'mail', true);
		$boxInput->setFieldSize(40);
		$boxInput->setFieldMaxLength(40);
		$return->addElement($boxInput, true);
		$this->addMultiValueInputTextField($return, 'maildrop', _('Recipient address'), true);
		$return->addElement(new htmlOutputText(_('Mail source')));
		$selectedSource = (!empty($this->attributes['mailsource'][0])) ? $this->attributes['mailsource'][0] : "-";
		$return->addElement(new htmlSelect('mailsource', array('-', 'esmtp', 'local'), array($selectedSource)));
		$return->addElement(new htmlHelpLink('mailsource'), true);
		$description = (!empty($this->attributes['description'][0])) ? $this->attributes['description'][0] : '';
		$boxInput = new htmlTableExtendedInputField(_('Description'), 'description', $description, 'description');
		$boxInput->setFieldSize(40);
		$boxInput->setFieldMaxLength(100);
		$return->addElement($boxInput, true);
		return $return;
	}

	/**
	 * {@inheritdoc}
	 */
	function process_attributes() {
		$errors = array();
		$this->attributes['mail'][0] = $_POST['mail'];
		if (empty($this->attributes['mail'][0]) || !get_preg($this->attributes['mail'][0], 'email')) {
			$errors[] = $this->messages['mail'][0];
		}
		$this->processMultiValueInputTextField('maildrop', $errors, 'mailLocalAddress');
		if (empty($this->attributes['maildrop'])) {
			$errors[] = $this->messages['maildrop'][0];
		}
		$this->attributes['mailsource'] = array();
		if ($_POST['mailsource'] != "-") {
			$this->attributes['mailsource'][0] = $_POST['mailsource'];
		} elseif (isset($this->attributes['mailsource'])) {
			unset($this->attributes['mailsource']);
		}
		$this->attributes['description'][0] = $_POST['description'];
		return $errors;
	}

	/**
	 * {@inheritdoc}
	 */
	function get_pdfEntries($pdfKeys, $typeId) {
		$return = array();
		$this->addSimplePDFField($return, 'mail', _('Email address'));
		$this->addSimplePDFField($return, 'maildrop', _('Recipient address'));
		$this->addSimplePDFField($return, 'mailsource', _('Mail source'));
		$this->addSimplePDFField($return, 'description', _('Description'));
		return $return;
	}

	/**
	 * {@inheritdoc}
	 */
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
		$messages = array();
		$possibleSources = array('esmtp', 'local');
		for( $i = 0; $i < sizeof($rawAccounts); $i++) {
			if (!in_array( "courierMailAlias", $partialAccounts[$i]['objectClass'] )) {
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
					array_push($error, array($i));
					$messages[] = $error;
				}
			}
		}
		return $messages;
	}

}

?>