<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2005 - 2015  Roland Gruber

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
* Manages SSH public keys.
*
* @package modules
* @author Roland Gruber
* @author Andrew Dibble <adibble@mobitv.com>
*/

/**
* Manages SSH public keys.
*
* @package modules
*/
class ldapPublicKey extends baseModule {

	/** session variable for existing keys in self service */
	const SESS_KEY_LIST = 'ldapPublicKey_keyList';

	/**
	* Returns true if this module can manage accounts of the current type, otherwise false.
	*
	* @return boolean true if module fits
	*/
	public function can_manage() {
		return in_array($this->get_scope(), array('user'));
	}

	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	*
	* @see baseModule::get_metaData()
	*/
	function get_metaData() {
		$return = array();
		// icon
		$return['icon'] = 'keyBig.png';
		// alias name
		$return["alias"] = _("SSH public key");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('ldapPublicKey');
		// managed attributes
		$return['attributes'] = array('sshPublicKey');
		// help Entries
		$return['help'] = array(
			'sshPublicKey' => array(
				"Headline" => _("SSH public key"), 'attr' => 'sshPublicKey',
				"Text" => _("Please enter your public SSH key.")
			),
			'keyList' => array(
				"Headline" => _("SSH public key"), 'attr' => 'sshPublicKey',
				"Text" => _("Please a comma separated list of your public SSH keys.")
			),
			'upload' => array(
				"Headline" => _("File upload"), 'attr' => 'sshPublicKey',
				"Text" => _("Upload a file with one or more keys. Each line contains one key.")
			),
		);
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'ldapPublicKey_sshPublicKey',
				'description' => _('SSH public key'),
				'help' => 'keyList',
				'example' => _('ssh-dss 234234 user@host')
			)
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'sshPublicKey' => _('SSH public keys')
		);
		// self service field settings
		$return['selfServiceFieldSettings'] = array(
			'sshPublicKey' => _('SSH public keys'),
		);
		$return['selfServiceReadOnlyFields'] = array('sshPublicKey');
		return $return;
	}

	/**
	* This function fills the message array.
	**/
	function load_Messages() {
		$this->messages['file'][0] = array('ERROR', _('No file selected.'));
		$this->messages['file'][1] = array('ERROR', _('The uploaded file seems to be no valid public key.'));
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 *
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		$return = new htmlTable();
		$this->addMultiValueInputTextField($return, 'sshPublicKey', _('SSH public key'), false, '16384', false, null, '50');
		// file upload
		$return->addElement(new htmlSpacer(null, '20px'), true);
		$return->addElement(new htmlOutputText(_('Upload file')));
		$uploadGroup = new htmlGroup();
		$uploadGroup->addElement(new htmlInputFileUpload('sshPublicKeyFile'));
		$uploadGroup->addElement(new htmlSpacer('1px', null));
		$uploadGroup->addElement(new htmlButton('sshPublicKeyFileSubmit', _('Upload')));
		$uploadGroup->addElement(new htmlSpacer('5px', null));
		$uploadGroup->addElement(new htmlHelpLink('upload'));
		$return->addElement($uploadGroup);
		return $return;
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_attributes() {
		$messages = array();
		$this->processMultiValueInputTextField('sshPublicKey', $messages);
		// file upload
		if (isset($_POST['sshPublicKeyFileSubmit'])) {
			if ($_FILES['sshPublicKeyFile'] && ($_FILES['sshPublicKeyFile']['size'] > 0)) {
				$handle = fopen($_FILES['sshPublicKeyFile']['tmp_name'], "r");
				$data = fread($handle, 10000000);
				fclose($handle);
				$isOk = true;
				if (is_executable('/usr/bin/ssh-keygen')) {
					$out = array();
					$retVal = 0;
					exec('/usr/bin/ssh-keygen -l -f ' . escapeshellarg($_FILES['sshPublicKeyFile']['tmp_name']), $out, $retVal);
					if ($retVal !== 0) {
						$messages[] = $this->messages['file'][1];
						$isOk = false;
					}
				}
				if ($isOk) {
					$data = str_replace("\r\n", "\n", $data);
					$data = str_replace("\r", "\n", $data);
					$lines = explode("\n", $data);
					foreach ($lines as $line) {
						if (!empty($line) && !(strpos($line, '#') === 0)) {
							$this->attributes['sshPublicKey'][] = $line;
						}
					}
				}
			}
			else {
				$messages[] = $this->messages['file'][0];
			}
		}
		$this->attributes['sshPublicKey'] = array_values(array_unique($this->attributes['sshPublicKey']));
		return $messages;
	}

	/**
	* In this function the LDAP account is built up.
	*
	* @param array $rawAccounts list of hash arrays (name => value) from user input
	* @param array $ids list of IDs for column position (e.g. "posixAccount_uid" => 5)
	* @param array $partialAccounts list of hash arrays (name => value) which are later added to LDAP
	* @param array $selectedModules list of selected account modules
	* @return array list of error messages if any
	*/
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules) {
		$messages = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array("ldapPublicKey", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "ldapPublicKey";
			// add keys
			if ($rawAccounts[$i][$ids['ldapPublicKey_sshPublicKey']] != "") {
				$keys = explode(',', $rawAccounts[$i][$ids['ldapPublicKey_sshPublicKey']]);
				// check format
				for ($m = 0; $m < sizeof($keys); $m++) {
					$partialAccounts[$i]['sshPublicKey'][] = $keys[$m];
				}
			}
		}
		return $messages;
	}

	/**
	 * Returns a list of possible PDF entries for this account.
	 *
	 * @param array $pdfKeys list of PDF keys that are included in document
	 * @return list of PDF entries (array(<PDF key> => <PDF lines>))
	 */
	function get_pdfEntries($pdfKeys) {
		$return = array();
		if (sizeof($this->attributes['sshPublicKey']) > 0) {
			$pdfTable = new PDFTable(_('SSH public keys'));
			for ($i = 0; $i < sizeof($this->attributes['sshPublicKey']); $i++) {
				$pdfRow = new PDFTableRow();
				$pdfRow->cells[] = new PDFTableCell($this->attributes['sshPublicKey'][$i]);
				$pdfTable->rows[] = $pdfRow;
			}
			$this->addPDFTable($return, 'sshPublicKey', $pdfTable);
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
	function getSelfServiceOptions($fields, $attributes, $passwordChangeOnly, $readOnlyFields) {
		$return = array();
		if ($passwordChangeOnly) {
			return $return; // no fields as long no LDAP content can be read
		}
		if (in_array('sshPublicKey', $fields)) {
			$sshPublicKeys = array();
			if (isset($attributes['sshPublicKey'][0])) {
				$sshPublicKeys = $attributes['sshPublicKey'];
			}
			$_SESSION[self::SESS_KEY_LIST] = $sshPublicKeys;
			$keyTable = new htmlTable();
			// JavaScript functions
			$keyTable->addElement($this->getSelfServiceKeysJSBlock(), true);
			// input fields
			$keyTable->addElement(new htmlDiv('sshPublicKeyDiv', $this->getSelfServiceKeys()), true);
			// upload status
			$uploadStatus = new htmlDiv('ldapPublicKey_upload_status_key', new htmlOutputText(''));
			$uploadStatus->setCSSClasses(array('qq-upload-list'));
			$keyTable->addElement($uploadStatus, true);
			$keyLabel = new htmlOutputText($this->getSelfServiceLabel('sshPublicKey', _('SSH public keys')));
			$row = new htmlResponsiveRow();
			$row->addLabel($keyLabel);
			$row->addField($keyTable);
			$return['sshPublicKey'] = $row;
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
				$keyInput = new htmlInputField('sshPublicKey_' . $i, $keys[$i]);
				$keyInput->setFieldMaxLength(16384);
				$group->addElement($keyInput);
				$delLink = new htmlLink('', '#', '../../graphics/del.png');
				$delLink->setTitle(_('Delete'));
				$delLink->setOnClick('ldapPublicKeyDeleteKey(' . $i . ', ' . sizeof($keys) . ');return false;');
				$group->addElement($delLink);
				if ($i == (sizeof($keys) - 1)) {
					$addLink = new htmlLink('', '#', '../../graphics/add.png');
					$addLink->setTitle(_('Add'));
					$addLink->setOnClick('ldapPublicKeyAddKey(' . sizeof($keys) . ');return false;');
					$group->addElement($addLink);
				}
				$content->add($group, 12, 12, 12, 'nowrap');
			}
		}
		else {
			$addLink = new htmlLink('', '#', '../../graphics/add.png');
			$addLink->setTitle(_('Add'));
			$addLink->setOnClick('ldapPublicKeyAddKey(' . sizeof($keys) . ');return false;');
			$content->add($addLink, 12);
		}
		// upload button
		$uploadButtons = new htmlGroup();
		$uploadButtons->addElement(new htmlDiv('ldapPublicKeyKeyUploadId', new htmlOutputText('')), true);
		$keyUpload = new htmlJavaScript('ldapPublicKeyUploadKey(\'ldapPublicKeyKeyUploadId\', ' . sizeof($keys) . ');');
		$uploadButtons->addElement($keyUpload);
		$content->add($uploadButtons, 12);
		return $content;
	}

	/**
	 * Returns the Java Script functions to manage the keys.
	 *
	 * @return htmlJavaScript JS block
	 */
	private static function getSelfServiceKeysJSBlock() {
		$content = '
			function ldapPublicKeyDeleteKey(id, count) {
				var actionJSON = {
					"action": "deleteKey",
					"id": id
				};
				for (c = 0; c < count; c++) {
					actionJSON["sshPublicKey_" + c] = jQuery(\'#sshPublicKey_\' + c).val();
				}
				jQuery.post(\'../misc/ajax.php?selfservice=1&module=ldapPublicKey&scope=user'
					. '&' . getSecurityTokenName() . '=' . getSecurityTokenValue()
					. '\', {jsonInput: actionJSON}, function(data) {ldapPublicKeyDeleteKeyHandleReply(data);}, \'json\');
			}

			function ldapPublicKeyDeleteKeyHandleReply(data) {
				if (data.errorsOccured == "false") {
					jQuery(\'#sshPublicKeyDiv\').html(data.html);
				}
				else {
					alert(data.errormessage);
				}
			}

			function ldapPublicKeyAddKey(count) {
				var actionJSON = {
					"action": "addKey"
				};
				for (c = 0; c < count; c++) {
					actionJSON["sshPublicKey_" + c] = jQuery(\'#sshPublicKey_\' + c).val();
				}
				jQuery.post(\'../misc/ajax.php?selfservice=1&module=ldapPublicKey&scope=user'
					. '&' . getSecurityTokenName() . '=' . getSecurityTokenValue()
					. '\', {jsonInput: actionJSON}, function(data) {ldapPublicKeyAddKeyHandleReply(data);}, \'json\');
			}

			function ldapPublicKeyAddKeyHandleReply(data) {
				if (data.errorsOccured == "false") {
					jQuery(\'#sshPublicKeyDiv\').html(data.html);
				}
				else {
					alert(data.errormessage);
				}
			}

			function ldapPublicKeyUploadKey(elementID, count) {
				var uploadStatus = document.getElementById(\'ldapPublicKey_upload_status_key\');
				var parameters = {
					action: \'ajaxKeyUpload\'
				};
				for (c = 0; c < count; c++) {
					parameters["sshPublicKey_" + c] = jQuery(\'#sshPublicKey_\' + c).val();
				}
				var uploader = new qq.FineUploader({
					element: document.getElementById(elementID),
					listElement: uploadStatus,
					request: {
						endpoint: \'../misc/ajax.php?selfservice=1&module=ldapPublicKey&scope=user'
						. '&' . getSecurityTokenName() . '=' . getSecurityTokenValue() . '\',
						forceMultipart: true,
						paramsInBody: true,
						params: parameters
					},
					multiple: false,
					callbacks: {
						onComplete: function(id, fileName, data) {
							if (data.success) {
								if (data.html) {
									jQuery(\'#sshPublicKeyDiv\').html(data.html);
								}
							}
							else {
								alert(data.error);
							}
						}
					}
				});
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
	 * @return array messages and attributes (array('messages' => array(), 'add' => array('mail' => array('test@test.com')), 'del' => array(), 'mod' => array(), 'info' => array()))
	 */
	public function checkSelfServiceOptions($fields, $attributes, $passwordChangeOnly, $readOnlyFields) {
		$return = array('messages' => array(), 'add' => array(), 'del' => array(), 'mod' => array(), 'info' => array());
		if ($passwordChangeOnly) {
			return $return; // skip processing if only a password change is done
		}
		if (in_array('sshPublicKey', $fields)) {
			$newKeys = array();
			$counter = 0;
			while (isset($_POST['sshPublicKey_' . $counter])) {
				$newKeys[] = $_POST['sshPublicKey_' . $counter];
				$counter++;
			}
			$count = sizeof($newKeys);
			for ($i = 0; $i < $count; $i++) {
				if (trim($newKeys[$i]) == '') {
					unset($newKeys[$i]);
				}
			}
			$newKeys = array_values(array_unique($newKeys));
			$oldKeys = array();
			if (isset($attributes['sshPublicKey'][0])) {
				$oldKeys = $attributes['sshPublicKey'];
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
					$return['add']['sshPublicKey'] = $newKeys;
				}
				elseif (sizeof($newKeys) == 0) {
					$return['del']['sshPublicKey'] = $newKeys;
				}
				else {
					$return['mod']['sshPublicKey'] = $newKeys;
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
		// AJAX uploads are non-JSON
		if (isset($_GET['action']) && ($_GET['action'] == 'ajaxKeyUpload')) {
			$this->ajaxUpload();
			return;
		}
		$jsonInput = $_POST['jsonInput'];
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
				$tabindex = 999;
				parseHtml(null, $contentElement, array(), true, $tabindex, $this->get_scope());
				$content = ob_get_contents();
				ob_end_clean();
				$jsonReturn = array(
					'errorsOccured' => 'false',
					'html' => $content,
				);
			}
		}
		echo json_encode($jsonReturn);
	}

	/**
	 * Handles an AJAX file upload and prints the JSON result.
	 */
	private function ajaxUpload() {
		$x = $_GET;
		$y = $_FILES;
		$result = array('success' => true);
		if (!isset($_FILES['qqfile']) || ($_FILES['qqfile']['size'] < 10)) {
			$result = array('error' => _('No file received.'));
		}
		else {
			$handle = fopen($_FILES['qqfile']['tmp_name'], "r");
			$data = fread($handle, 100000000);
			fclose($handle);
			if (is_executable('/usr/bin/ssh-keygen')) {
				$out = array();
				$retVal = 0;
				exec('/usr/bin/ssh-keygen -l -f ' . escapeshellarg($_FILES['qqfile']['tmp_name']), $out, $retVal);
				if ($retVal !== 0) {
					$result = array('error' => $this->messages['file'][1][1]);
					echo json_encode($result);
					return;
				}
			}
			$data = str_replace("\r\n", "\n", $data);
			$data = str_replace("\r", "\n", $data);
			$lines = explode("\n", $data);
			foreach ($lines as $line) {
				if (!empty($line) && !(strpos($line, '#') === 0)) {
					$_SESSION[self::SESS_KEY_LIST][] = $line;
				}
			}
			$_SESSION[self::SESS_KEY_LIST] = array_values(array_unique($_SESSION[self::SESS_KEY_LIST]));
			ob_start();
			$contentElement = $this->getSelfServiceKeys();
			ob_end_clean();
			ob_start();
			$tabindex = 999;
			parseHtml(null, $contentElement, array(), true, $tabindex, $this->get_scope());
			$content = ob_get_contents();
			ob_end_clean();
			$result['html'] = $content;
		}
		echo json_encode($result);
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
		$tabindex = 999;
		parseHtml(null, $contentElement, array(), true, $tabindex, $this->get_scope());
		$content = ob_get_contents();
		ob_end_clean();
		return array(
			'errorsOccured' => 'false',
			'html' => $content,
		);
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
		return array('errorsOccured' => 'true', 'errormessage' => $message);
	}

}


?>
