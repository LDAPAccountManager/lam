<?php

use LAM\PDF\PDFTable;
use LAM\PDF\PDFTableCell;
use LAM\PDF\PDFTableRow;
use LAM\TYPES\ConfiguredType;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2018 - 2026  Roland Gruber

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

	/** session variable for existing keys in self-service */
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
		// self-service field settings
		$return['selfServiceFieldSettings'] = [
			'yubiKeyId' => _('YubiKey ids'),
		];
		$return['selfServiceReadOnlyFields'] = ['yubiKeyId'];
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	protected function load_Messages(): void {
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
			$return->add($message);
			return $return;
		}
		if (empty($objectClass) || in_array($objectClass, $this->attributes['objectClass'])) {
			$htmlIds = [];
			$this->addMultiValueInputTextField($return, $attributeName, _('YubiKey ids'), false, 12, false, null, null, $htmlIds, 'lam-prevent-enter');
			if (!empty($objectClass)) {
				$return->addVerticalSpacer('2rem');
				$remButton = new htmlButton('remObjectClass', _('Remove YubiKey extension'));
				$remButton->setCSSClasses(['lam-danger']);
				$return->add($remButton, 12, 12, 12, 'text-center');
			}
		}
		else {
			$return->add(new htmlButton('addObjectClass', _('Add YubiKey extension')));
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function process_attributes(): array {
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
	 * {@inheritDoc}
	 */
	public function save_attributes(): array {
		$objectClass = $this->getObjectClassName();
		if (!empty($objectClass) && !in_array($objectClass, $this->attributes['objectClass']) && !in_array($objectClass, $this->orig['objectClass'])) {
			// skip saving if the extension was not added/modified
			return [];
		}
		return parent::save_attributes();
	}

	/**
	 * {@inheritDoc}
	 */
	public function build_uploadAccounts(array $rawAccounts, array $ids, array &$partialAccounts, array $selectedModules, ConfiguredType &$type): array {
		$objectClass = $this->getObjectClassName();
		$attributeName = $this->getAttributeName();
		$messages = [];
		for ($i = 0; $i < count($rawAccounts); $i++) {
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
	 */
	public function get_pdfEntries(array $pdfKeys, string $typeId): array {
		$return = [];
		$attributeName = $this->getAttributeName();
		if (!empty($this->attributes[$attributeName])) {
			$pdfTable = new PDFTable(_('YubiKey ids'));
			for ($i = 0; $i < count($this->attributes[$attributeName]); $i++) {
				$pdfRow = new PDFTableRow();
				$pdfRow->cells[] = new PDFTableCell($this->attributes[$attributeName][$i]);
				$pdfTable->rows[] = $pdfRow;
			}
			$this->addPDFTable($return, 'yubiKeyId', $pdfTable);
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function getSelfServiceOptions(array $fields, array $attributes, bool $passwordChangeOnly, array $readOnlyFields): array {
		$return = [];
		if ($passwordChangeOnly) {
			return $return; // no fields as long no LDAP content can be read
		}
		$attributes = array_change_key_case($attributes);
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
	 * @return htmlResponsiveRow key content
	 */
	private function getSelfServiceKeys() {
		$keys = $_SESSION[self::SESS_KEY_LIST];
		$content = new htmlResponsiveRow();
		if (count($keys) > 0) {
			for ($i = 0; $i < count($keys); $i++) {
				$group = new htmlGroup();
				$keyInput = new htmlInputField('yubiKeyId_' . $i, $keys[$i]);
				$keyInput->setFieldMaxLength(12);
				$keyInput->setCSSClasses(['lam-prevent-enter']);
				$keyInput->setAccessibilityLabel($this->getSelfServiceLabel('yubiKeyId', _('YubiKey ids')));
				$group->addElement($keyInput);
				$delLink = new htmlLink('', '#', '../../graphics/del.svg');
				$delLink->setTitle(_('Delete'));
				$delLink->setOnClick('yubiKeyIdDeleteKey(' . $i . ', ' . count($keys) . ');return false;');
				$delLink->setCSSClasses(['icon']);
				$group->addElement($delLink);
				$content->add($group, 12, 12, 12, 'nowrap');
			}
		}
		$addGroup = new htmlGroup();
		$addLink = new htmlLink(_('Add'), '#', '../../graphics/add.svg');
		$addLink->setTitle(_('Add'));
		$addLink->setOnClick('yubiKeyIdAddKey(' . count($keys) . ');return false;');
		$addLink->setCSSClasses(['icon']);
		$addGroup->addElement($addLink);
		$addGroup->addElement(new htmlSpacer('0.5rem', null));
		$addGroup->addElement(new htmlHelpLink('whereToFind', 'yubiKeyUser'));
		$content->add($addGroup);
		return $content;
	}

	/**
	 * Returns the Java Script functions to manage the keys.
	 *
	 * @return htmlJavaScript JS block
	 */
	private function getSelfServiceKeysJSBlock() {
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
	 * {@inheritDoc}
	 */
	public function checkSelfServiceOptions(array $fields, array $attributes, bool $passwordChangeOnly, array $readOnlyFields): array {
		$return = ['messages' => [], 'add' => [], 'del' => [], 'mod' => [], 'info' => []];
		if ($passwordChangeOnly) {
			return $return; // skip processing if only a password change is done
		}
		$attributes = array_change_key_case($attributes);
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
			$count = count($newKeys);
			for ($i = 0; $i < $count; $i++) {
				if (trim($newKeys[$i]) === '') {
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
			if (count($newKeys) !== count($oldKeys)) {
				$update = true;
			}
			else {
				for ($i = 0; $i < count($newKeys); $i++) {
					if (!in_array($newKeys[$i], $oldKeys)) {
						$update = true;
						break;
					}
				}
			}
			if ($update) {
				if (count($oldKeys) === 0) {
					$return['add'][$attributeName] = $newKeys;
				}
				elseif (count($newKeys) === 0) {
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
	 * {@inheritDoc}
	 */
	public function handleAjaxRequest(): void {
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
				parseHtml(null, $contentElement, [], true);
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
	 * @param array<string, string> $data JSON data
	 * @return array<string, string> result data
	 */
	private function ajaxDeleteSelfServiceKey(array $data): array {
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
		parseHtml(null, $contentElement, [], true);
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
	 * @return array<string, string> error data
	 */
	public static function invalidAjaxRequest(): array {
		$message = _('Invalid request');
		return ['errorsOccurred' => 'true', 'errormessage' => $message];
	}

	/**
	 * Returns the object class used for YubiKey.
	 *
	 * @return string|null object class
	 */
	private function getObjectClassName(): ?string {
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
	private function getAttributeName(): string {
		if (!empty($this->moduleSettings['yubiKeyUser_attributeName'][0])) {
			return strtolower($this->moduleSettings['yubiKeyUser_attributeName'][0]);
		}
		elseif (!empty($this->selfServiceSettings->moduleSettings['yubiKeyUser_attributeName'][0])) {
			return strtolower($this->selfServiceSettings->moduleSettings['yubiKeyUser_attributeName'][0]);
		}
		return 'yubikeyid';
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_configOptions(array $scopes, array $allScopes): htmlElement|array {
		$configContainer = new htmlResponsiveRow();
		$configContainer->add(new htmlResponsiveInputField(_('Object class'), 'yubiKeyUser_objectClass', null, 'objectClass'));
		$configContainer->add(new htmlResponsiveInputField(_('Attribute name'), 'yubiKeyUser_attributeName', 'yubiKeyId', 'attributeName', true));
		return $configContainer;
	}

	/**
	 * {@inheritDoc}
	 */
	public function getSelfServiceSettings(selfServiceProfile $profile): ?htmlElement {
		$container = new htmlResponsiveRow();
		$container->add(new htmlResponsiveInputField(_('Object class'), 'yubiKeyUser_objectClass', null, ['objectClass', 'yubiKeyUser']));
		$container->add(new htmlResponsiveInputField(_('Attribute name'), 'yubiKeyUser_attributeName', 'yubiKeyId', ['attributeName', 'yubiKeyUser']));
		return $container;
	}

}
