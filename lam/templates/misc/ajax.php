<?php
namespace LAM\AJAX;
use htmlResponsiveTable;
use htmlSpacer;
use htmlStatusMessage;
use \LAM\TOOLS\IMPORT_EXPORT\Importer;
use \LAM\TOOLS\IMPORT_EXPORT\Exporter;
use LAM\TOOLS\TREEVIEW\TreeView;
use LAM\TOOLS\TREEVIEW\TreeViewTool;
use \LAM\TYPES\TypeManager;
use \htmlResponsiveRow;
use \htmlLink;
use \htmlOutputText;
use \htmlButton;
use \LAM\LOGIN\WEBAUTHN\WebauthnManager;
use \LAMCfgMain;
use LAMException;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2011 - 2023  Roland Gruber

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
* Manages all AJAX requests.
*
* @author Roland Gruber
* @package tools
*/

/** security functions */
include_once(__DIR__ . "/../../lib/security.inc");
/** LDIF import */
include_once(__DIR__ . "/../../lib/import.inc");
/** schema for tree view */
include_once __DIR__ . "/../../lib/schema.inc";
/** tree view tool */
include_once __DIR__ . "/../../lib/tools.inc";
include_once __DIR__ . "/../../lib/tools/treeview.inc";

// start session
if (isset($_GET['selfservice'])) {
	// self service uses a different session name
	session_name('SELFSERVICE');
}

// return standard JSON response if session expired
if (startSecureSession(false, true) === false) {
	Ajax::setHeader();
	echo json_encode(['sessionExpired' => "true"]);
	die();
}

setlanguage();

$ajax = new Ajax();
$ajax->handleRequest();

/**
 * Manages all AJAX requests.
 */
class Ajax {

	/**
	 * Manages an AJAX request.
	 */
	public function handleRequest(): void {
		static::setHeader();
		// check token
		validateSecurityToken();
		$isSelfService = isset($_GET['selfservice']);
		if (isset($_GET['module']) && isset($_GET['scope']) && in_array($_GET['module'], getAvailableModules($_GET['scope']))) {
			enforceUserIsLoggedIn();
			if (isset($_GET['useContainer']) && ($_GET['useContainer'] == '1')) {
				$sessionKey  = htmlspecialchars((string) $_GET['editKey']);
				if (!isset($_SESSION[$sessionKey])) {
					logNewMessage(LOG_ERR, 'Unable to find account container');
					die();
				}
				$module = $_SESSION[$sessionKey]->getAccountModule($_GET['module']);
				$module->handleAjaxRequest();
			}
			else {
				$module = new $_GET['module']($_GET['scope']);
				$module->handleAjaxRequest();
			}
			die();
		}
		if (!isset($_GET['function'])) {
			die();
		}
		$function = $_GET['function'];

		if (($function === 'passwordStrengthCheck') && isset($_POST['jsonInput'])) {
			$this->checkPasswordStrength(json_decode((string) $_POST['jsonInput'], true, 512, JSON_THROW_ON_ERROR));
			die();
		}
		if ($function === 'webauthn') {
			enforceUserIsLoggedIn(false);
			$this->manageWebauthn($isSelfService);
			die();
		}
		if ($function === 'webauthnDevices') {
			$this->enforceUserIsLoggedInToMainConfiguration();
			$this->manageWebauthnDevices();
			die();
		}
		if ($function === 'testSmtp') {
			$this->enforceUserIsLoggedInToMainConfiguration();
			$this->testSmtpConnection();
			die();
		}
		enforceUserIsLoggedIn();
		if (($function === 'passwordChange') && isset($_POST['jsonInput'])) {
			self::managePasswordChange(json_decode((string) $_POST['jsonInput'], true, 512, JSON_THROW_ON_ERROR));
		}
		elseif ($function === 'import') {
			include_once('../../lib/import.inc');
			$importer = new Importer();
			ob_start();
			$jsonOut = $importer->doImport();
			ob_end_clean();
			echo $jsonOut;
		}
		elseif ($function === 'export') {
			include_once('../../lib/export.inc');
			$attributes = $_POST['attributes'];
			$baseDn = $_POST['baseDn'];
			$ending = $_POST['ending'];
			$filter = $_POST['filter'];
			$format = $_POST['format'];
			$includeSystem = ($_POST['includeSystem'] === 'true');
			$saveAsFile = ($_POST['saveAsFile'] === 'true');
			$searchScope = $_POST['searchScope'];
			$exporter = new Exporter($baseDn, $searchScope, $filter, $attributes, $includeSystem, $saveAsFile, $format, $ending);
			ob_start();
			$jsonOut = $exporter->doExport();
			ob_end_clean();
			echo $jsonOut;
		}
		elseif ($function === 'upload') {
			include_once('../../lib/upload.inc');
			$typeManager = new \LAM\TYPES\TypeManager();
			$uploader = new \LAM\UPLOAD\Uploader($typeManager->getConfiguredType($_GET['typeId']));
			ob_start();
			$jsonOut = $uploader->doUpload();
			ob_end_clean();
			echo $jsonOut;
		}
		elseif ($function === 'dnselection') {
			ob_start();
			$jsonOut = $this->dnSelection();
			ob_end_clean();
			echo $jsonOut;
		}
		elseif ($function === 'webauthnOwnDevices') {
			$this->manageWebauthnOwnDevices();
		}
		elseif ($function === 'treeview') {
			include_once(__DIR__ . "/../../lib/treeview.inc");
			$treeView = new TreeView();
			ob_start();
			$jsonOut = $treeView->answerAjaxCall();
			ob_end_clean();
			echo $jsonOut;
		}
		elseif ($function === 'checkPassword') {
			$this->checkPassword();
		}
	}

	/**
	 * Sets JSON HTTP header.
	 */
	public static function setHeader(): void {
		if (!headers_sent()) {
			header('Content-Type: application/json; charset=utf-8');
		}
	}

	/**
	 * Manages a password change request on the edit account page.
	 *
	 * @param array<mixed> $input input parameters
	 */
	private static function managePasswordChange(array $input): void {
		$sessionKey  = htmlspecialchars((string) $_GET['editKey']);
		$return = $_SESSION[$sessionKey]->setNewPassword($input);
		echo json_encode($return, JSON_THROW_ON_ERROR);
	}

	/**
	 * Checks if a password is accepted by LAM's password policy.
	 *
	 * @param array<mixed> $input input parameters
	 */
	private function checkPasswordStrength(array $input): void {
		$password = $input['password'];
		$result = checkPasswordStrength($password, null, null);
		echo json_encode(["result" => $result], JSON_THROW_ON_ERROR);
	}

	/**
	 * Manages webauthn requests.
	 *
	 * @param bool $isSelfService request is from self service
	 */
	private function manageWebauthn($isSelfService): void {
		include_once __DIR__ . '/../../lib/webauthn.inc';
		if ($isSelfService) {
			$userDN = lamDecrypt($_SESSION['selfService_clientDN'], 'SelfService');
		}
		else {
			$userDN = $_SESSION['ldap']->getUserName();
		}
		$webauthnManager = new WebauthnManager();
		$isRegistered = $webauthnManager->isRegistered($userDN);
		if (!$isRegistered) {
			$registrationObject = $webauthnManager->getRegistrationObject($userDN, $isSelfService);
			$_SESSION['webauthn_registration'] = json_encode($registrationObject, JSON_THROW_ON_ERROR);
			echo json_encode(
				[
					'action' => 'register',
					'registration' => $registrationObject
				],
				JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE | JSON_THROW_ON_ERROR
			);
		}
		else {
			$authenticationObject = $webauthnManager->getAuthenticationObject($userDN, $isSelfService);
			$_SESSION['webauthn_authentication'] = json_encode($authenticationObject, JSON_THROW_ON_ERROR);
			echo json_encode(
				[
					'action' => 'authenticate',
					'authentication' => $authenticationObject
				],
				JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE | JSON_THROW_ON_ERROR
			);
		}
		die();
	}

	/**
	 * Webauthn device management.
	 */
	private function manageWebauthnDevices(): void {
		$action = $_POST['action'];
		if ($action === 'search') {
			$searchTerm = $_POST['searchTerm'];
			if (!empty($searchTerm)) {
				$this->manageWebauthnDevicesSearch($searchTerm);
			}
		}
		elseif ($action === 'delete') {
			$dn = $_POST['dn'];
			$credentialId = $_POST['credentialId'];
			if (!empty($dn) && !empty($credentialId)) {
				$this->manageWebauthnDevicesDelete($dn, $credentialId);
			}
		}
	}

	/**
	 * Searches for webauthn devices and prints the results as html.
	 *
	 * @param string $searchTerm search term
	 */
	private function manageWebauthnDevicesSearch($searchTerm): void {
		include_once __DIR__ . '/../../lib/webauthn.inc';
		$webAuthnManager = new WebauthnManager();
		$database = $webAuthnManager->getDatabase();
		$results = $database->searchDevices('%' . $searchTerm . '%');
		$row = new htmlResponsiveRow();
		$row->addVerticalSpacer('0.5rem');
		if (empty($results)) {
			$row->add(new htmlStatusMessage('INFO', _('No devices found.')), 12);
		}
		else {
			$titles = [
				_('User'),
				_('Name'),
				_('Registration'),
				_('Last use'),
				_('Delete')
			];
			$data = [];
			$id = 0;
			foreach ($results as $result) {
				$delButton = new htmlButton('deleteDevice' . $id, 'del.svg', true);
				$delButton->addDataAttribute('credential', $result['credentialId']);
				$delButton->addDataAttribute('dn', $result['dn']);
				$delButton->addDataAttribute('dialogtitle', _('Remove device'));
				$delButton->addDataAttribute('oktext', _('Ok'));
				$delButton->addDataAttribute('canceltext', _('Cancel'));
				$delButton->setCSSClasses(['webauthn-delete']);
				$name = !empty($result['name']) ? $result['name'] : '';
				$data[] = [
					new htmlOutputText($result['dn']),
					new htmlOutputText($name),
					new htmlOutputText(date('Y-m-d H:i:s', $result['registrationTime'])),
					new htmlOutputText(date('Y-m-d H:i:s', $result['lastUseTime'])),
					$delButton
				];
				$id++;
			}
			$table = new htmlResponsiveTable($titles, $data);
			$row->add($table, 12);
		}
		$row->addVerticalSpacer('2rem');
		ob_start();
		$row->generateHTML(null, [], [], false, null);
		$content = ob_get_contents();
		ob_end_clean();
		echo json_encode(['content' => $content], JSON_THROW_ON_ERROR);
	}

	/**
	 * Deletes a webauthn device.
	 *
	 * @param string $dn user DN
	 * @param string $credentialId base64 encoded credential id
	 */
	private function manageWebauthnDevicesDelete($dn, $credentialId): void {
		include_once __DIR__ . '/../../lib/webauthn.inc';
		$webAuthnManager = new WebauthnManager();
		$database = $webAuthnManager->getDatabase();
		$success = $database->deleteDevice($dn, $credentialId);
		if ($success) {
			$message = new htmlStatusMessage('INFO', _('The device was deleted.'));
		}
		else {
			$message = new htmlStatusMessage('ERROR', _('The device was not found.'));
		}
		$row = new htmlResponsiveRow();
		$row->addVerticalSpacer('0.5rem');
		$row->add($message, 12);
		$row->addVerticalSpacer('2rem');
		ob_start();
		$row->generateHTML(null, [], [], true, null);
		$content = ob_get_contents();
		ob_end_clean();
		echo json_encode(['content' => $content], JSON_THROW_ON_ERROR);
	}

	/**
	 * Updates a webauthn device name.
	 *
	 * @param string $dn user DN
	 * @param string $credentialId base64 encoded credential id
	 * @param string $name name
	 */
	private function manageWebauthnDevicesUpdateName($dn, $credentialId, $name): void {
		include_once __DIR__ . '/../../lib/webauthn.inc';
		$webAuthnManager = new WebauthnManager();
		$database = $webAuthnManager->getDatabase();
		$success = $database->updateDeviceName($dn, $credentialId, $name);
		if ($success) {
			logNewMessage(LOG_DEBUG, 'Changed name of ' . $dn . ' ' . $credentialId . ' to ' . $name);
		}
		else {
			logNewMessage(LOG_ERR, 'Unable to change name of ' . $dn . ' ' . $credentialId . ' to ' . $name);
		}
		echo json_encode([]);
	}

	/**
	 * Manages requests to setup user's own webauthn devices.
	 */
	private function manageWebauthnOwnDevices(): void {
		$action = $_POST['action'];
		$dn = $_POST['dn'];
		$sessionDn = $_SESSION['ldap']->getUserName();
		if ($sessionDn !== $dn) {
			logNewMessage(LOG_ERR, 'WebAuthn delete canceled, DN does not match.');
			die();
		}
		if ($action === 'delete') {
			$credentialId = $_POST['credentialId'];
			$this->manageWebauthnDevicesDelete($sessionDn, $credentialId);
		}
		elseif ($action === 'setName') {
			$credentialId = $_POST['credentialId'];
			$name = $_POST['name'];
			$this->manageWebauthnDevicesUpdateName($sessionDn, $credentialId, $name);
		}
	}

	/**
	 * Handles DN selection fields.
	 *
	 * @return string JSON output
	 */
	private function dnSelection(): string {
		$dn = trim((string) $_POST['dn']);
		if (empty($dn) || !get_preg($dn, 'dn')) {
			$dnList = $this->getDefaultDns();
		}
		else {
			$dnList = $this->getSubDns($dn);
		}
		$html = $this->buildDnSelectionHtml($dnList, $dn);
		return json_encode(['dialogData' => $html], JSON_THROW_ON_ERROR);
	}

	/**
	 * Returns a list of default DNs from account types + tree suffix.
	 *
	 * @return string[] default DNs
	 */
	private function getDefaultDns() {
		$typeManager = new TypeManager();
		$baseDnList = [];
		foreach ($typeManager->getConfiguredTypes() as $type) {
			$suffix = $type->getSuffix();
			if (!empty($suffix)) {
				$baseDnList[] = $suffix;
			}
		}
		$treeSuffixes = TreeViewTool::getRootDns();
		foreach ($treeSuffixes as $treeSuffix) {
			$baseDnList[] = $treeSuffix;
		}
		$baseDnList = array_unique($baseDnList);
		usort($baseDnList, 'compareDN');
		return $baseDnList;
	}

	/**
	 * Returns the HTML to build the DN selection list.
	 *
	 * @param string[] $dnList DN list
	 * @param string $currentDn current DN
	 * @return string HTML code
	 */
	private function buildDnSelectionHtml($dnList, $currentDn): string {
		$fieldId = trim((string) $_POST['fieldId']);
		$mainRow = new htmlResponsiveRow();
		$onclickUp = 'window.lam.html.updateDnSelection(this, \''
				. htmlspecialchars($fieldId) . '\', \'' . getSecurityTokenName() . '\', \''
				. getSecurityTokenValue() . '\')';
		if (!empty($currentDn)) {
			$row = new htmlResponsiveRow();
			$row->addDataAttribute('dn', $currentDn);
			$text = new htmlOutputText($currentDn);
			$text->setIsBold(true);
			$row->add($text, 12, 9);
			$row->setCSSClasses(['text-right']);
			$buttonId = base64_encode($currentDn);
			$buttonId = str_replace('=', '', $buttonId);
			$button = new htmlButton($buttonId, _('Ok'));
			$button->setOnClick('window.lam.html.selectDn(this, \'' . htmlspecialchars($fieldId) . '\')');
			$row->add($button, 12, 3, 3, 'text-left');
			$mainRow->add($row);
			$mainRow->addVerticalSpacer('1rem');
			// back up
			$row = new htmlResponsiveRow();
			$row->addDataAttribute('dn', extractDNSuffix($currentDn));
			$text = new htmlLink('..', '#');
			$text->setCSSClasses(['bold']);
			$text->setOnClick($onclickUp);
			$row->add($text, 12, 9);
			$row->setCSSClasses(['text-right']);
			$row->add(new htmlSpacer('16px'), 12, 3);
			$mainRow->add($row);
			$mainRow->addVerticalSpacer('2rem');
		}
		foreach ($dnList as $dn) {
			$row = new htmlResponsiveRow();
			$row->addDataAttribute('dn', $dn);
			$link = new htmlLink($dn, '#');
			$link->setOnClick($onclickUp);
			$row->add($link, 12, 9);
			$row->setCSSClasses(['text-right']);
			$buttonId = base64_encode($dn);
			$buttonId = str_replace('=', '', $buttonId);
			$button = new htmlButton($buttonId, _('Ok'));
			$button->setOnClick('window.lam.html.selectDn(this, \'' . htmlspecialchars($fieldId) . '\')');
			$row->add($button, 12, 3, 3, 'text-left');
			$mainRow->add($row, 12);
		}
		ob_start();
		parseHtml(null, $mainRow, [], false, 'user');
		$out = ob_get_contents();
		ob_end_clean();
		if ($out === false) {
			return '';
		}
		return $out;
	}

	/**
	 * Returns the sub DNs of given DN.
	 *
	 * @param string $dn DN
	 * @return string[] sub DNs
	 */
	private function getSubDns($dn) {
		$dnEntries = ldapListDN($dn);
		$dnList = [];
		foreach ($dnEntries as $entry) {
			$dnList[] = $entry['dn'];
		}
		usort($dnList, 'compareDN');
		return $dnList;
	}

	/**
	 * Checks if the user entered the configuration master password.
	 * Dies if password is not set.
	 */
	private function enforceUserIsLoggedInToMainConfiguration(): void {
		if (!isset($_SESSION['cfgMain'])) {
			$cfg = new LAMCfgMain();
		}
		else {
			$cfg = $_SESSION['cfgMain'];
		}
		if (isset($_SESSION["mainconf_password"]) && ($cfg->checkPassword($_SESSION["mainconf_password"]))) {
			return;
		}
		die();
	}

	/**
	 * Checks if the given password matches the hash value.
	 */
	private function checkPassword() : void {
		$hashValue = $_POST['hashValue'];
		$checkValue = $_POST['checkValue'];
		$hashPart = preg_replace('/^\\{([A-Z0-9-]+)\\}[!]?/', '', $hashValue);
		$matches = checkPasswordHash(getHashType($hashValue), $hashPart, $checkValue);
		$resultRow = new htmlResponsiveRow();
		if ($matches) {
			$text = new htmlOutputText(_('Password matches'));
			$text->setCSSClasses(['text-center', 'display-as-block', 'text-ok']);
		}
		else {
			$text = new htmlOutputText(_('Password does not match'));
			$text->setCSSClasses(['text-center', 'display-as-block', 'text-error']);
		}
		$resultRow->add($text);
		ob_start();
		parseHtml(null, $resultRow, [], false, 'user');
		$out = ob_get_contents();
		ob_end_clean();
		$result = ['resultHtml' => $out];
		echo json_encode($result, JSON_THROW_ON_ERROR);
	}

	/**
	 * Checks if the SMTP settings in main config are valid.
	 */
	private function testSmtpConnection(): void {
		$server = $_POST['server'];
		$user = $_POST['user'];
		$password = $_POST['password'];
		$encryption = $_POST['encryption'];
		if (empty($server)) {
			$result = ['info' => _('Local SMTP server cannot be tested.')];
			echo json_encode($result, JSON_THROW_ON_ERROR);
			return;
		}
		try {
			testSmtpConnection($server, $user, $password, $encryption);
			$result = ['info' => _('Connection to SMTP server was successful.')];
			echo json_encode($result, JSON_THROW_ON_ERROR);
		}
		catch (LAMException $e) {
			$result = ['error' => _('Unable to connect to SMTP server.'), 'details' => $e->getMessage()];
			echo json_encode($result, JSON_THROW_ON_ERROR);
		}
	}

}
