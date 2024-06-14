<?php

use \LAM\PDF\PDFTable;
use \LAM\PDF\PDFTableCell;
use \LAM\PDF\PDFTableRow;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2018 - 2024  Roland Gruber

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
 * Manages YubiKey keys.
 *
 * @package modules
 * @author Roland Gruber
 */

/**
 * Manages YubiKey keys.
 *
 * @package modules
 */
class yubiKeyUser extends baseModule {

	/** session variable for existing keys in self service */
	const SESS_KEY_LIST = 'yubiKeyUser_keyList';

	/**
	 * Creates a new yubiKeyUser object.
	 *
	 * @param string $scope account type (user, group, host)
	 */
	public function __construct($scope) {
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
	 *
	 * @see baseModule::get_metaData()
	 */
	public function get_metaData() {
		$return = [];
		// icon
		$return['icon'] = 'yubikey.png';
		// alias name
		$return["alias"] = "YubiKey";
		// module dependencies
		$return['dependencies'] = ['depends' => [], 'conflicts' => []];
		// managed object classes
		$objectClass = $this->getObjectClassName();
		if (!empty($objectClass)) {
			$return['objectClasses'] = [$objectClass];
		}
		// managed attributes
		$attributeName = $this->getAttributeName();
		$return['attributes'] = [$attributeName];
		// help Entries
		$return['help'] = [
			$attributeName => [
				"Headline" => _("YubiKey ids"), 'attr' => $attributeName,
				"Text" => _("The YubiKey id are the first 12 letters of the key output. Simply touch your YubiKey in an empty field to set it.")
			],
			'keyList' => [
				"Headline" => _("YubiKey ids"), 'attr' => $attributeName,
				"Text" => _("Please a comma separated list of your YubiKey ids.")
			],
			'objectClass' => [
				"Headline" => _("Object class"),
				"Text" => _("Please enter the object class that should be used for YubiKey (e.g. 'yubiKeyUser').")
			],
			'attributeName' => [
				"Headline" => _("Attribute name"),
				"Text" => _("Please enter the attribute name that should be used for YubiKey (e.g. 'yubiKeyId').")
			],
			'whereToFind' => [
				"Headline" => _("YubiKey ids"),
				"Text" => _("The YubiKey id are the first 12 letters of the key output. Simply touch your YubiKey in an empty field to set it.")
			],
		];
		// upload fields
		$return['upload_columns'] = [
			[
				'name' => 'yubiKeyUser_yubiKeyId',
				'description' => _('YubiKey ids'),
				'help' => 'keyList',
				'example' => 'abcd1234, vwyxz12345'
			]
		];
		// available PDF fields
		$return['PDF_fields'] = [
			'yubiKeyId' => _('YubiKey ids')
		];
		// self service field settings
		$return['selfServiceFieldSettings'] = [
			'yubiKeyId' => _('YubiKey ids'),
		];
		$return['selfServiceReadOnlyFields'] = ['yubiKeyId'];
		return $return;
	}

	/**
	 * This function fills the message array.
	 **/
	protected function load_Messages() {
		$this->messages['yubiKeyId'][0] = ['ERROR', _('Please enter a valid key id.')];
		$this->messages['yubiKeyId'][1] = ['ERROR', _('Account %s:') . ' yubiKeyUser_yubiKeyId', _('Please enter a valid key id.')];
	}

	/**
	 * {@inheritDoc}
	 */
	public function loadAttributesFromAccountCopy(array $ldapAttributes, array $attributesToIgnore = []): void {
		$attributesToIgnore = array_merge(baseModule::ATTRIBUTES_TO_IGNORE_ON_COPY_DEFAULT, [$this->getAttributeName()]);
		parent::loadAttributesFromAccountCopy($ldapAttributes, $attributesToIgnore);
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 *
	 * @return htmlElement HTML meta data
	 */
	public function display_html_attributes() {
		$return = new htmlResponsiveRow();
		$objectClass = $this->getObjectClassName();
		$attributeName = $this->getAttributeName();
		if (empty($attributeName)) {
			$message = new htmlStatusMessage('ERROR', _('Invalid configuration detected. Please edit your server profile (module settings) and fill all required fields.'));
			$return->add($message, 12);
			return $return;
		}
		if (empty($objectClass) || in_array($objectClass, $this->attributes['objectClass'])) {
			$htmlIds = [];
			$this->addMultiValueInputTextField($return, $attributeName, _('YubiKey ids'), false, '12', false, null, null, $htmlIds, 'lam-prevent-enter');
			if (!empty($objectClass)) {
				$return->addVerticalSpacer('2rem');
				$remButton = new htmlButton('remObjectClass', _('Remove YubiKey extension'));
				$remButton->setCSSClasses(['lam-danger']);
				$return->add($remButton, 12, 12, 12, 'text-center');
			}
		}
		else {
			$return->add(new htmlButton('addObjectClass', _('Add YubiKey extension')), 12);
		}
		return $return;
	}

	/**
	 * Processes user input of the primary module page.
	 * It checks if all input values are correct and updates the associated LDAP attributes.
	 *
	 * @return array list of info/error messages
	 */
	public function process_attributes() {
		$messages = [];
		$objectClass = $this->getObjectClassName();
		$attributeName = $this->getAttributeName();
		// add extension
		if (isset($_POST['addObjectClass'])) {
			$this->attributes['objectClass'][] = $objectClass;
			return [];
		}
		// remove extension
		elseif (isset($_POST['remObjectClass'])) {
			$this->attributes['objectClass'] = array_delete([$objectClass], $this->attributes['objectClass']);
			if (!empty($this->attributes[$attributeName])) {
				unset($this->attributes[$attributeName]);
			}
			return [];
		}
		// skip processing if extension is not active
		if (!empty($objectClass) && !in_array($objectClass, $this->attributes['objectClass'])) {
			return [];
		}
		$this->processMultiValueInputTextField($attributeName, $messages, 'ascii');
		$this->attributes[$attributeName] = array_values(array_unique($this->attributes[$attributeName]));
		return $messages;
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
	public function save_attributes() {
		$objectClass = $this->getObjectClassName();
		if (!empty($objectClass) && !in_array($objectClass, $this->attributes['objectClass']) && !in_array($objectClass, $this->orig['objectClass'])) {
			// skip saving if the extension was not added/modified
			return [];
		}
		return parent::save_attributes();
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::build_uploadAccounts()
	 */
	public function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
		$objectClass = $this->getObjectClassName();
		$attributeName = $this->getAttributeName();
		$messages = [];
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!empty($objectClass) && !in_array($objectClass, $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = $objectClass;
			}
			// add keys
			$message = $this->messages['yubiKeyId'][1];
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'yubiKeyUser_yubiKeyId', $attributeName, 'ascii', $message, $messages, '/,[ ]*/');
		}
		return $messages;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	public function get_pdfEntries($pdfKeys, $typeId) {
		$return = [];
		$attributeName = $this->getAttributeName();
		if (!empty($this->attributes[$attributeName])) {
			$pdfTable = new PDFTable(_('YubiKey ids'));
			for ($i = 0; $i < sizeof($this->attributes[$attributeName]); $i++) {
				$pdfRow = new PDFTableRow();
				$pdfRow->cells[] = new PDFTableCell($this->attributes[$attributeName][$i]);
				$pdfTable->rows[] = $pdfRow;
			}
			$this->addPDFTable($return, 'yubiKeyId', $pdfTable);
		}
		return $return;
	}

	/**
	 * Returns the meta HTML code for each input field.
	 * format: array(<field1> => array(<META HTML>), ...)
	 * It is not possible to display help links.
	 *
	 * @param array $fields list of active fields
	 * @param array $attributes attributes of LDAP account
	 * @param boolean $passwordChangeOnly indicates that the user is only allowed to change his password and no LDAP content is readable
	 * @param array $readOnlyFields list of read-only fields
	 * @return array list of meta HTML elements (field name => htmlResponsiveRow)
	 */
	public function getSelfServiceOptions($fields, $attributes, $passwordChangeOnly, $readOnlyFields) {
		$return = [];
		if ($passwordChangeOnly) {
			return $return; // no fields as long no LDAP content can be read
		}
		$attributes = array_change_key_case($attributes, CASE_LOWER);
		$objectClass = strtolower($this->getObjectClassName());
		if (!empty($objectClass) && !in_array_ignore_case($objectClass, $attributes['objectclass'])) {
			return $return;
		}
		if (in_array('yubiKeyId', $fields)) {
			$attributeName = strtolower($this->getAttributeName());
			$keyIds = [];
			if (isset($attributes[$attributeName][0])) {
				$keyIds = $attributes[$attributeName];
			}
			$_SESSION[self::SESS_KEY_LIST] = $keyIds;
			$row = new htmlResponsiveRow();
			// JavaScript functions
			$row->add($this->getSelfServiceKeysJSBlock(), 0);
			// input fields
			$keyLabel = new htmlOutputText($this->getSelfServiceLabel('yubiKeyId', _('YubiKey ids')));
			$row->addLabel($keyLabel);
			$row->addField(new htmlDiv('yubiKeyIdDiv', $this->getSelfServiceKeys()));
			$return['yubiKeyId'] = $row;
		}
		return $return;
	}

	/**
	 * Returns the meta HTML code to display the key area.
	 * This also includes the file upload.
	 *
	 * @return htmlTable key content
	 */
	private function getSelfServiceKeys() {
		$keys = $_SESSION[self::SESS_KEY_LIST];
		$content = new htmlResponsiveRow();
		if (sizeof($keys) > 0) {
			for ($i = 0; $i < sizeof($keys); $i++) {
				$group = new htmlGroup();
				$keyInput = new htmlInputField('yubiKeyId_' . $i, $keys[$i]);
				$keyInput->setFieldMaxLength(12);
				$keyInput->setCSSClasses(['lam-prevent-enter']);
				$keyInput->setAccessibilityLabel($this->getSelfServiceLabel('yubiKeyId', _('YubiKey ids')));
				$group->addElement($keyInput);
				$delLink = new htmlLink('', '#', '../../graphics/del.svg');
				$delLink->setTitle(_('Delete'));
				$delLink->setOnClick('yubiKeyIdDeleteKey(' . $i . ', ' . sizeof($keys) . ');return false;');
				$delLink->setCSSClasses(['icon']);
				$group->addElement($delLink);
				$content->add($group, 12, 12, 12, 'nowrap');
			}
		}
		$addGroup = new htmlGroup();
		$addLink = new htmlLink(_('Add'), '#', '../../graphics/add.svg', true);
		$addLink->setTitle(_('Add'));
		$addLink->setOnClick('yubiKeyIdAddKey(' . sizeof($keys) . ');return false;');
		$addLink->setCSSClasses(['icon']);
		$addGroup->addElement($addLink);
		$addGroup->addElement(new htmlSpacer('0.5rem', null));
		$addGroup->addElement(new htmlHelpLink('whereToFind', 'yubiKeyUser'));
		$content->add($addGroup, 12);
		return $content;
	}

	/**
	 * Returns the Java Script functions to manage the keys.
	 *
	 * @return htmlJavaScript JS block
	 */
	private static function getSelfServiceKeysJSBlock() {
		$content = '
			function yubiKeyIdDeleteKey(id, count) {
				var actionJSON = {
					"action": "deleteKey",
					"id": id
				};
				for (c = 0; c < count; c++) {
					actionJSON["yubiKeyId_" + c] = document.getElementById(\'yubiKeyId_\' + c).value;
				}
				let data = new FormData();
				data.append("' . getSecurityTokenName() . '", "' . getSecurityTokenValue() . '");
				data.append("jsonInput", JSON.stringify(actionJSON));
				fetch(\'../misc/ajax.php?selfservice=1&module=yubiKeyUser&scope=user\', {
					method: "POST",
					body: data
				})
				.then(async response => {
					const jsonData = await response.json();
					yubiKeyIdDeleteKeyHandleReply(jsonData);
				});
			}

			function yubiKeyIdDeleteKeyHandleReply(data) {
				if (data.errorsOccurred == "false") {
					document.getElementById(\'yubiKeyIdDiv\').innerHTML = data.html;
				}
				else {
					window.lam.dialog.showError(data.errormessage, null, "' . _('Ok') . '");
				}
			}

			function yubiKeyIdAddKey(count) {
				var actionJSON = {
					"action": "addKey"
				};
				for (c = 0; c < count; c++) {
					actionJSON["yubiKeyId_" + c] = document.getElementById(\'yubiKeyId_\' + c).value;
				}
				let data = new FormData();
				data.append("' . getSecurityTokenName() . '", "' . getSecurityTokenValue() . '");
				data.append("jsonInput", JSON.stringify(actionJSON));
				fetch(\'../misc/ajax.php?selfservice=1&module=yubiKeyUser&scope=user\', {
					method: "POST",
					body: data
				})
				.then(async response => {
					const jsonData = await response.json();
					yubiKeyIdAddKeyHandleReply(jsonData);
				});
			}

			function yubiKeyIdAddKeyHandleReply(data) {
				if (data.errorsOccurred == "false") {
					document.getElementById(\'yubiKeyIdDiv\').innerHTML = data.html;
					window.lam.html.preventEnter();
				}
				else {
					window.lam.dialog.showError(data.errormessage, null, "' . _('Ok') . '");
				}
			}

			';
		return new htmlJavaScript($content);
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
	public function checkSelfServiceOptions($fields, $attributes, $passwordChangeOnly, $readOnlyFields) {
		$return = ['messages' => [], 'add' => [], 'del' => [], 'mod' => [], 'info' => []];
		if ($passwordChangeOnly) {
			return $return; // skip processing if only a password change is done
		}
		$attributes = array_change_key_case($attributes, CASE_LOWER);
		$objectClass = strtolower($this->getObjectClassName());
		if (!in_array_ignore_case($objectClass, $attributes['objectclass'])) {
			return $return;
		}
		if (in_array('yubiKeyId', $fields)) {
			$attributeName = strtolower($this->getAttributeName());
			$newKeys = [];
			$counter = 0;
			while (isset($_POST['yubiKeyId_' . $counter])) {
				$newKeys[] = $_POST['yubiKeyId_' . $counter];
				$counter++;
			}
			$count = sizeof($newKeys);
			for ($i = 0; $i < $count; $i++) {
				if (trim($newKeys[$i]) == '') {
					unset($newKeys[$i]);
				}
				elseif (!get_preg($newKeys[$i], 'ascii')) {
					$return['messages'][] = ['ERROR', _('Please enter a valid key id.'), htmlspecialchars($newKeys[$i])];
				}
			}
			$newKeys = array_values(array_unique($newKeys));
			$oldKeys = [];
			if (isset($attributes[$attributeName][0])) {
				$oldKeys = $attributes[$attributeName];
			}
			$update = false;
			if (sizeof($newKeys) != sizeof($oldKeys)) {
				$update = true;
			}
			else {
				for ($i = 0; $i < sizeof($newKeys); $i++) {
					if (!in_array($newKeys[$i], $oldKeys)) {
						$update = true;
						break;
					}
				}
			}
			if ($update) {
				if (sizeof($oldKeys) == 0) {
					$return['add'][$attributeName] = $newKeys;
				}
				elseif (sizeof($newKeys) == 0) {
					$return['del'][$attributeName] = $newKeys;
				}
				else {
					$return['mod'][$attributeName] = $newKeys;
				}
			}
		}
		return $return;
	}

	/**
	 * Manages AJAX requests.
	 * This function may be called with or without an account container.
	 */
	public function handleAjaxRequest() {
		$jsonInput = json_decode($_POST['jsonInput'], true);
		$jsonReturn = self::invalidAjaxRequest();
		if (isset($jsonInput['action'])) {
			if ($jsonInput['action'] == 'deleteKey') {
				$jsonReturn = $this->ajaxDeleteSelfServiceKey($jsonInput);
			}
			elseif ($jsonInput['action'] == 'addKey') {
				$_SESSION[self::SESS_KEY_LIST][] = '';
				ob_start();
				$contentElement = $this->getSelfServiceKeys();
				ob_end_clean();
				ob_start();
				parseHtml(null, $contentElement, [], true, $this->get_scope());
				$content = ob_get_contents();
				ob_end_clean();
				$jsonReturn = [
					'errorsOccurred' => 'false',
					'html' => $content,
				];
			}
		}
		echo json_encode($jsonReturn);
	}

	/**
	 * Manages the deletion of a key.
	 *
	 * @param array $data JSON data
	 */
	private function ajaxDeleteSelfServiceKey($data) {
		if (!isset($data['id'])) {
			return self::invalidAjaxRequest();
		}
		$index = $data['id'];
		if (array_key_exists($index, $_SESSION[self::SESS_KEY_LIST])) {
			unset($_SESSION[self::SESS_KEY_LIST][$index]);
			$_SESSION[self::SESS_KEY_LIST] = array_values($_SESSION[self::SESS_KEY_LIST]);
		}
		ob_start();
		$contentElement = $this->getSelfServiceKeys();
		ob_end_clean();
		ob_start();
		parseHtml(null, $contentElement, [], true, $this->get_scope());
		$content = ob_get_contents();
		ob_end_clean();
		return [
			'errorsOccurred' => 'false',
			'html' => $content,
		];
	}

	/**
	 * Invalid AJAX request received.
	 *
	 * @param String $message error message
	 */
	public static function invalidAjaxRequest($message = null) {
		if ($message == null) {
			$message = _('Invalid request');
		}
		return ['errorsOccurred' => 'true', 'errormessage' => $message];
	}

	/**
	 * Returns the object class used for YubiKey.
	 *
	 * @return string object class
	 */
	private function getObjectClassName() {
		if (!empty($this->moduleSettings['yubiKeyUser_objectClass'][0])) {
			return $this->moduleSettings['yubiKeyUser_objectClass'][0];
		}
		elseif (!empty($this->selfServiceSettings->moduleSettings['yubiKeyUser_objectClass'][0])) {
			return $this->selfServiceSettings->moduleSettings['yubiKeyUser_objectClass'][0];
		}
		return null;
	}

	/**
	 * Returns the attribute name used for YubiKey.
	 *
	 * @return string attribute name
	 */
	private function getAttributeName() {
		if (!empty($this->moduleSettings['yubiKeyUser_attributeName'][0])) {
			return $this->moduleSettings['yubiKeyUser_attributeName'][0];
		}
		elseif (!empty($this->selfServiceSettings->moduleSettings['yubiKeyUser_attributeName'][0])) {
			return $this->selfServiceSettings->moduleSettings['yubiKeyUser_attributeName'][0];
		}
		return null;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_configOptions()
	 */
	public function get_configOptions($scopes, $allScopes) {
		$configContainer = new htmlResponsiveRow();
		$configContainer->add(new htmlResponsiveInputField(_('Object class'), 'yubiKeyUser_objectClass', null, 'objectClass'), 12);
		$configContainer->add(new htmlResponsiveInputField(_('Attribute name'), 'yubiKeyUser_attributeName', 'yubiKeyId', 'attributeName', true), 12);
		return $configContainer;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::getSelfServiceSettings()
	 */
	public function getSelfServiceSettings($profile) {
		$container = new htmlResponsiveRow();
		$container->add(new htmlResponsiveInputField(_('Object class'), 'yubiKeyUser_objectClass', null, ['objectClass', 'yubiKeyUser']), 12);
		$container->add(new htmlResponsiveInputField(_('Attribute name'), 'yubiKeyUser_attributeName', 'yubiKeyId', ['attributeName', 'yubiKeyUser']), 12);
		return $container;
	}

}
