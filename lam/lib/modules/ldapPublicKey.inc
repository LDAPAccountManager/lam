<?php
use \LAM\PDF\PDFTable;
use \LAM\PDF\PDFTableCell;
use \LAM\PDF\PDFTableRow;
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2005 - 2017  Roland Gruber

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
	* Creates a new sambaSamAccount object.
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
			'autoAdd' => array(
				"Headline" => _("Automatically add this extension"),
				"Text" => _("This will enable the extension automatically if this profile is loaded.")
			),
		);
		// profile options
		$profileContainer = new htmlTable();
		$profileContainer->addElement(new htmlTableExtendedInputCheckbox('ldapPublicKey_addExt', false, _('Automatically add this extension'), 'autoAdd'), true);
		$return['profile_options'] = $profileContainer;
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
		if (in_array('ldapPublicKey', $this->attributes['objectClass'])) {
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
			$return->addElement($uploadGroup, true);

			$return->addElement(new htmlSpacer(null, '30px'), true);

			$remButton = new htmlButton('remObjectClass', _('Remove SSH public key extension'));
			$remButton->colspan = 3;
			$return->addElement($remButton);
		}
		else {
			$return->addElement(new htmlButton('addObjectClass', _('Add SSH public key extension')));
		}
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
		// add extension
		if (isset($_POST['addObjectClass'])) {
			$this->attributes['objectClass'][] = 'ldapPublicKey';
			return array();
		}
		// remove extension
		elseif (isset($_POST['remObjectClass'])) {
			$this->attributes['objectClass'] = array_delete(array('ldapPublicKey'), $this->attributes['objectClass']);
			if (!empty($this->attributes['sshPublicKey'])) {
				unset($this->attributes['sshPublicKey']);
			}
			return array();
		}
		// skip processing if extension is not active
		if (!in_array('ldapPublicKey', $this->attributes['objectClass'])) {
			return array();
		}
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
	function save_attributes() {
		if (!in_array('ldapPublicKey', $this->attributes['objectClass']) && !in_array('ldapPublicKey', $this->orig['objectClass'])) {
			// skip saving if the extension was not added/modified
			return array();
		}
		return parent::save_attributes();
	}

	/**
	* Loads the values of an account profile into internal variables.
	*
	* @param array $profile hash array with profile values (identifier => value)
	*/
	function load_profile($profile) {
		// profile mappings in meta data
		parent::load_profile($profile);
		// add extension
		if (isset($profile['ldapPublicKey_addExt'][0]) && ($profile['ldapPublicKey_addExt'][0] == "true")) {
			if (!in_array('ldapPublicKey', $this->attributes['objectClass'])) {
				$this->attributes['objectClass'][] = 'ldapPublicKey';
			}
		}
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::build_uploadAccounts()
	 */
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
		$messages = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array("ldapPublicKey", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "ldapPublicKey";
			// add keys
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'ldapPublicKey_sshPublicKey', 'sshPublicKey', null, null, $messages, '/,[ ]*/');
		}
		return $messages;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	function get_pdfEntries($pdfKeys, $typeId) {
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
