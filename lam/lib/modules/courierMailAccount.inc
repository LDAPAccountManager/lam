<?php
use \LAM\TYPES\TypeManager;
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
		return in_array($this->get_scope(), array(
			'user'
		));
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_metaData() {
		$return = array();
		// icon
		$return['icon'] = 'courierMail.png';
		// alias name
		$return["alias"] = _("Courier");
		// module dependencies
		$return['dependencies'] = array(
			'depends' => array(
				'inetOrgPerson'
			),
			'conflicts' => array()
		);
		// managed object classes
		$return['objectClasses'] = array(
			'courierMailAccount'
		);
		// managed attributes
		$return['attributes'] = array(
			'mailbox', 'mailhost', 'sharedgroup', 'disableimap',
			'disablepop3', 'disableshared', 'disablewebmail', 'quota'
		);
		// help entries
		$return['help'] = array(
			'disableimap' => array(
				"Headline" => _("Disable IMAP access"),
				"attr" => 'disableimap',
				"Text" => _("Setting this will disallow the user from logging in using IMAP.")
			),
			'disablepop3' => array(
				"Headline" => _("Disable POP3 access"),
				"attr" => 'disablepop3',
				"Text" => _("Setting this will disallow the user from logging in using POP3.")
			),
			'disablewebmail' => array(
				"Headline" => _("Disable webmail access"),
				"attr" => 'disablewebmail',
				"Text" => _("Setting this will disallow the user from logging in using SQwebmail.")
			),
			'disableshared' => array(
				"Headline" => _("Disable shared folder access"),
				"attr" => 'disableshared',
				"Text" => _("Setting this will make shared folders invisible to this user.")
			),
			'mailbox' => array(
				"Headline" => _("User mailbox"),
				"attr" => 'mailbox',
				"Text" => _("Path to the user's mailbox. Usually relative to the user's home folder. In a mixed setup with users that are both local unix users and virtual Courier mail users it can be beneficial to configure this to be the Courier mail home directory.")
			),
			'mailhost' => array(
				"Headline" => _("User mail host"),
				"attr" => 'mailhost',
				"Text" => _("Name of the host that stores the user's email. Must be a FQDN.")
			),
			'quota' => array(
				"Headline" => _("User mail quota"),
				"attr" => 'quota',
				"Text" => _("The amont of storage available to the user.")
			),
			'autoAdd' => array(
				"Headline" => _("Automatically add this extension"),
				"Text" => _("Automatically add this extension when creating new users.")
			),
			'unixUserType' => array(
				"Headline" => _("How are local Unix users handled"),
				"Text" => _("This choice depends on how you wish to run your system. If you choose 'local' then it will be assumed that Courier mail is set up to use the homedirectory attribute as the root path for maildirs and the mailbox attribute for the name of the maildir folder if different from './MAILDIR'. If you choose 'virtual' then it will be assumed that Courier mail is set up to ignore the 'homedirectory' attribute and instead use the 'mailbox' attribute as the root path for maildirs, and the maildir directory is 'MAILDIR' below this directory.")
			),
			'virtMailRoot' => array(
				"Headline" => _("The root path for virtual mail users"),
				"Text" => _("This path will be used as the mail root for virtual users.")
			),
			'homeDirectory' => array(
				"Headline" => _("User home directory"),
				"attr" => 'homeDirectory',
				"Text" => _("The system home directory of the user. In a virtual user setup this is typically the root for all users.")
			),
			'clearPassword' => array(
				"Headline" => _("User's password in clear text"),
				"attr" => 'clearPassword',
				"Text" => _("This attribute contains the user's password in clear text. It is not recommended to use.")
			),
			'description' => array(
				"Headline" => _("Description of the user"),
				"attr" => 'description',
				"Text" => _("A description of the user.")
			),
			'gecos' => array(
				"Headline" => _("GECOS field"),
				"attr" => 'gecos',
				"Text" => _("")
			),
			'gidNumber' => array(
				"Headline" => _("Group number"),
				"attr" => 'gidNumber',
				"Text" => _("The number of the operating system group this user belongs to.")
			),
			'loginShell' => array(
				"Headline" => _("User unix login shell"),
				"attr" => 'loginShell',
				"Text" => _("The shell used by this user")
			),
			'uid' => array(
				"Headline" => _("User id"),
				"attr" => 'uid',
				"Text" => _("The id of the user in the operating system.")
			),
			'uidNumber' => array(
				"Headline" => _("User id number"),
				"attr" => 'uidnumber',
				"Text" => _("The operating system number for this user.")
			),
			'userPassword' => array(
				"Headline" => _("User's encrypted password"),
				"attr" => 'userPassword',
				"Text" => _("The encrypted password for this user.")
			)
		);
		// profile options
		$profileContainer = new htmlTable();
		$profileContainer->addElement(new htmlTableExtendedInputCheckbox('courierMailAccount_addExt', false, _('Automatically add this extension'), 'autoAdd'), true);
		$return['profile_options'] = $profileContainer;
		// PDF fields
		$return['PDF_fields'] = array(
			'mailbox' => _('User mailbox'),
			'mailhost' => _('User mail host'),
			'quota' => _('User mail quota'),
			'disableimap' => _('Disable IMAP access'),
			'disablepop3' => _('Disable POP3 access'),
			'disablewebmail' => _('Disable webmail access'),
			'disableshared' => _('Disable shared folder access')
		);
		return $return;
	}

	/**
	 * This function fills the error message array with messages
	 */
	public function load_Messages() {
		$this->messages['mailhost'][0] = array('ERROR', _('The mail host is not valid.')); // third array value is set dynamically
		$this->messages['quota'][0] = array('ERROR', _('The quota must be entered as a floating point number.')); // third array value is set dynamically
		$this->messages['quota'][1] = array('ERROR', _('The quota must be entered as one of four formats: 1000, 1000S, 1000k, 1000kB.')); // third array value is set dynamically
		$this->messages['homeDirectory'][0] = array('ERROR', _('Homedirectory contains invalid characters.')); // third array value is set dynamically
		$this->messages['homeDirectory'][1] = array('ERROR', _('Account %s:') . ' courierMailAccount_homeDirectory', _('Homedirectory contains invalid characters.'));
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::getManagedAttributes()
	 */
	public function getManagedAttributes($typeId) {
		$attrs = parent::getManagedAttributes($typeId);
		$typeManager = new TypeManager();
		$modules = $typeManager->getConfiguredType($typeId)->getModules();
		if ($this->isUnixDisabled($modules)) {
			$attrs[] = 'homeDirectory';
		}
		return $attrs;
	}

	/**
	 * {@inheritDoc}
	 */
	public function display_html_attributes() {
		$return = new htmlTable();
		if ($this->isExtensionEnabled()) {
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
			$return->addElement(new htmlOutputText(_('Mail quota')));
			$tempTable = new htmlTable();
			$boxInput = new htmlInputField('quotaValue', $value);
			$boxInput->setFieldSize(5);
			$boxInput->setFieldMaxLength(5);
			$tempTable->addElement($boxInput);
			$units = array(
				'B', 'kB', 'MB', 'GB',
				'TB', 'PB', 'EB'
			);
			$selected = array(
				$units[$unitIndex]
			);
			$unitSelect = new htmlSelect('quotaUnit', $units, $selected);
			$unitSelect->setSortElements(false);
			$tempTable->addElement($unitSelect);
			$return->addElement($tempTable);
			$return->addElement(new htmlHelpLink('quota'), true);
			// check boxes for account access control
			$disableimap = isset($this->attributes['disableimap'][0]) && ($this->attributes['disableimap'][0] != 0);
			$return->addElement(new htmlTableExtendedInputCheckbox('disableimap', $disableimap, _('Disable IMAP use'), 'disableimap'), true);
			$disablepop3 = isset($this->attributes['disablepop3'][0]) && ($this->attributes['disablepop3'][0] != 0);
			$return->addElement(new htmlTableExtendedInputCheckbox('disablepop3', $disablepop3, _('Disable POP3 use'), 'disablepop3'), true);
			$disablewebmail = isset($this->attributes['disablewebmail'][0]) && ($this->attributes['disablewebmail'][0] != 0);
			$return->addElement(new htmlTableExtendedInputCheckbox('disablewebmail', $disablewebmail, _('Disable Webmail use'), 'disablewebmail'), true);
			$disableshared = isset($this->attributes['disableshared'][0]) && ($this->attributes['disableshared'][0] != 0);
			$return->addElement(new htmlTableExtendedInputCheckbox('disableshared', $disableshared, _('Disable Shared Folder use'), 'disableshared'), true);
			// remove extension
			$return->addElement(new htmlSpacer(null, '20px'), true);
			$remButton = new htmlButton('remObjectClass', _('Remove Courier mail extension'));
			$remButton->colspan = 3;
			$return->addElement($remButton);
		}
		else {
			$return->addElement(new htmlButton('addObjectClass', _('Add Courier mail extension')));
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
			return array();
		}
		// remove extension
		elseif (isset($_POST['remObjectClass'])) {
			$this->attributes['objectClass'] = array_delete(array(
				'courierMailAccount'
			), $this->attributes['objectClass']);
			$attrNames = $this->getManagedAttributes($this->getAccountContainer()->get_type()->getId());
			foreach ($attrNames as $attrName) {
				if (isset($this->attributes[$attrName])) {
					unset($this->attributes[$attrName]);
				}
			}
			return array();
		}
		// skip processing if extension is not active
		if (!$this->isExtensionEnabled()) {
			return array();
		}
		$this->getAccountContainer()->replaceWildcardsInPOST(array('homeDirectory', 'mailbox'));
		$errors = array();
		// check new mailbox
		$this->attributes['mailbox'][0] = $_POST['mailbox'];
		// quota
		$units = array(
			'B', 'kB', 'MB',
			'GB', 'TB', 'PB', 'EB'
		);
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
	public function get_pdfEntries() {
		$return = array();
		$this->addSimplePDFField($return, 'mailbox', _('User mailbox'));
		$this->addSimplePDFField($return, 'mailhost', _('User mail host'));
		$this->addSimplePDFField($return, 'homeDirectory', _('Home directory'));
		$disableOptions = array(
			'disableimap' => _('IMAP access disabled'),
			'disablepop3' => _('POP3 access disabled'),
			'disablewebmail' => _('Webmail access disabled'),
			'disableshared' => _('Shared folder access disabled')
		);
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
		$units = array(
			'B', 'kB', 'MB', 'GB',
			'TB', 'PB', 'EB'
		);
		$this->addPDFKeyValue($return, 'quota', _('User mail quota'), $value . $units[$unitIndex]);
		return $return;
	}

	/**
	 * {@inheritdoc}
	 */
	public function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules) {
		$messages = array();
		$unitsLong = array(
			'B', 'kB', 'MB', 'GB',
			'TB', 'PB', 'EB'
		);
		$unitsShort = array(
			'', 'k', 'M', 'G',
			'T', 'P', 'E'
		);
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {

			// add object class

			if (!in_array("courierMailAccount", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "courierMailAccount";

			// add mailbox

			if ($rawAccounts[$i][$ids['courierMailAccount_mailbox']] != "") {
				if (!get_preg($rawAccounts[$i][$ids['courierMailAccount_mailbox']], 'filePath')) {
					$message = $this->messages['mailbox'][0];
					$message[] = $_POST['mailbox'];
					$errors[] = $message;
				}
				else
					$partialAccounts[$i]['mailbox'][] = $rawAccounts[$i][$ids['courierMailAccount_mailbox']];
			}

			// add mailhost

			if ($rawAccounts[$i][$ids['courierMailAccount_mailhost']] != "") {
				if (!get_preg($rawAccounts[$i][$ids['courierMailAccount_mailhost']], 'hostname')) {
					$message = $this->messages['mailhost'][0];
					$message[] = $_POST['mailhost'];
					$errors[] = $message;
				}
				else
					$partialAccounts[$i]['mailhost'][] = $rawAccounts[$i][$ids['courierMailAccount_mailhost']];
			}

			// add mail quota. This works if the quota is one of four formats: 1000, 1000S, 1000k, 1000kB

			if ($rawAccounts[$i][$ids['courierMailAccount_quota']] != "") {

				// 1st format: 1000

				if (is_numeric($rawAccounts[$i][$ids['courierMailAccount_quota']]))
					$partialAccounts[$i]['quota'][] = $rawAccounts[$i][$ids['courierMailAccount_quota']] . "S";

					// Is it one of 2nd or 3rd formats: 1000S, 1000k

				elseif (is_numeric(mb_substr($rawAccounts[$i][$ids['courierMailAccount_quota']], 0, -1))) {

					// 2nd format: 1000S

					if (mb_substr($rawAccounts[$i][$ids['courierMailAccount_quota']], -1) == "S")
						$partialAccounts[$i]['quota'][] = $rawAccounts[$i][$ids['courierMailAccount_quota']] . "S";

						// 3rd format: 1000k

					elseif (in_array(mb_substr($rawAccounts[$i][$ids['courierMailAccount_quota']], -1), $unitsShort)) {
						$power = array_search(mb_substr($rawAccounts[$i][$ids['courierMailAccount_quota']], -1), $unitsShort);
						$partialAccounts[$i]['quota'][] = mb_substr($rawAccounts[$i][$ids['courierMailAccount_quota']], 0, -1) * pow(1000, $power) . "S";
					}
				}

				// 4th format: 1000kB

				elseif (is_numeric(mb_substr($rawAccounts[$i][$ids['courierMailAccount_quota']], 0, -2)) && in_array(mb_substr($rawAccounts[$i][$ids['courierMailAccount_quota']], -2), $unitsLong)) {
					$power = array_search(mb_substr($rawAccounts[$i][$ids['courierMailAccount_quota']], -1), $unitsShort);
					$partialAccounts[$i]['quota'][] = mb_substr($rawAccounts[$i][$ids['courierMailAccount_quota']], 0, -1) * pow(1000, $power) . "S";
				}
				else {
					$errMsg = $this->messages['quota'][1];
					array_push($errMsg, array(
						$i
					));
					$messages[] = $errMsg;
				}
			}

			if ($rawAccounts[$i][$ids['courierMailAccount_disableimap']] != "") {
				if (($rawAccounts[$i][$ids['courierMailAccount_disableimap']]) == "true" || ($rawAccounts[$i][$ids['courierMailAccount_disableimap']] == "1")) $partialAccounts[$i]['disableimap'][] = 1;
			}

			if ($rawAccounts[$i][$ids['courierMailAccount_disablepop3']] != "") {
				if (($rawAccounts[$i][$ids['courierMailAccount_disablepop3']]) == "true" || ($rawAccounts[$i][$ids['courierMailAccount_disablepop3']] == "1")) $partialAccounts[$i]['disablepop3'][] = 1;
			}

			if ($rawAccounts[$i][$ids['courierMailAccount_disablewebmail']] != "") {
				if (($rawAccounts[$i][$ids['courierMailAccount_disablewebmail']]) == "true" || ($rawAccounts[$i][$ids['courierMailAccount_disablewebmail']] == "1")) $partialAccounts[$i]['disablewebmail'][] = 1;
			}

			if ($rawAccounts[$i][$ids['courierMailAccount_disableshared']] != "") {
				if (($rawAccounts[$i][$ids['courierMailAccount_disableshared']]) == "true" || ($rawAccounts[$i][$ids['courierMailAccount_disableshared']] == "1")) $partialAccounts[$i]['disableshared'][] = 1;
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
		if (isset($profile['courierMailAccount_addExt'][0]) && ($profile['courierMailAccount_addExt'][0] == "true")) {
			if (!in_array('courierMailAccount', $this->attributes['objectClass'])) {
				$this->attributes['objectClass'][] = 'courierMailAccount';
			}
		}
	}

	/**
	 * Returns if this account is virtual only or also a unix account
	 *
	 * @param string[] modules
	 * @return boolean only virtual account
	 */
	private function isUnixDisabled($modules) {
		if (in_array('posixAccount', $modules)) {
			return false;
		}
		else {
			return true;
		}
	}

}

?>