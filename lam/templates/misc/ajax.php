<?php
namespace LAM\AJAX;
use \LAM\TOOLS\IMPORT_EXPORT\Importer;
use \LAM\TOOLS\IMPORT_EXPORT\Exporter;
use \LAM\TYPES\TypeManager;
use \htmlResponsiveRow;
use \htmlLink;
use \htmlOutputText;
use \htmlButton;
use function LAM\LOGIN\WEBAUTHN\getRegistrationObject;
use function LAM\LOGIN\WEBAUTHN\isRegistered;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2011 - 2019  Roland Gruber

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

// start session
if (isset($_GET['selfservice'])) {
	// self service uses a different session name
	session_name('SELFSERVICE');
}

// return standard JSON response if session expired
if (startSecureSession(false, true) === false) {
	echo json_encode(array(
		'sessionExpired' => "true"
	));
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
	public function handleRequest() {
		$this->setHeader();
		// check token
		validateSecurityToken();
		$isSelfService = isset($_GET['selfservice']);
		if (isset($_GET['module']) && isset($_GET['scope']) && in_array($_GET['module'], getAvailableModules($_GET['scope']))) {
			enforceUserIsLoggedIn();
			if (isset($_GET['useContainer']) && ($_GET['useContainer'] == '1')) {
				$sessionKey  = htmlspecialchars($_GET['editKey']);
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
		}
		if (!isset($_GET['function'])) {
			die();
		}
		$function = $_GET['function'];
		if (!isset($_POST['jsonInput'])) {
			die();
		}

		$jsonInput = $_POST['jsonInput'];
		if ($function == 'passwordStrengthCheck') {
			$this->checkPasswordStrength($jsonInput);
			die();
		}
		if ($function === 'webauthn') {
			enforceUserIsLoggedIn(false);
			$this->manageWebauthn($isSelfService);
			die();
		}
		enforceUserIsLoggedIn();
		if ($function == 'passwordChange') {
			$this->managePasswordChange($jsonInput);
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
	}

	/**
	 * Sets JSON HTTP header.
	 */
	private static function setHeader() {
		if (!headers_sent()) {
			header('Content-Type: application/json; charset=utf-8');
		}
	}

	/**
	 * Manages a password change request on the edit account page.
	 *
	 * @param array $input input parameters
	 */
	private static function managePasswordChange($input) {
		$sessionKey  = htmlspecialchars($_GET['editKey']);
		$return = $_SESSION[$sessionKey]->setNewPassword($input);
		echo json_encode($return);
	}

	/**
	 * Checks if a password is accepted by LAM's password policy.
	 *
	 * @param array $input input parameters
	 */
	private function checkPasswordStrength($input) {
		$password = $input['password'];
		$result = checkPasswordStrength($password, null, null);
		echo json_encode(array("result" => $result));
	}

	/**
	 * Manages webauthn requests.
	 *
	 * @param bool $isSelfService request is from self service
	 */
	private function manageWebauthn($isSelfService) {
		include_once __DIR__ . '/../../lib/3rdParty/composer/autoload.php';
		include_once __DIR__ . '/../../lib/webauthn.inc';
		$userDN = $_SESSION['ldap']->getUserName();
		$isRegistered = isRegistered($userDN);
		if (!$isRegistered) {
			$registrationObject = getRegistrationObject($userDN);
			echo json_encode(
				array(
					'action' => 'register',
					'registration' => $registrationObject
				),
				JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE
			);
			die();
		}
	}

	/**
	 * Handles DN selection fields.
	 *
	 * @return string JSON output
	 */
	private function dnSelection() {
		$dn = trim($_POST['dn']);
		if (empty($dn) || !get_preg($dn, 'dn')) {
			$dnList = $this->getDefaultDns();
			$dn = null;
		}
		else {
			$dnList = $this->getSubDns($dn);
		}
		$html = $this->buildDnSelectionHtml($dnList, $dn);
		return json_encode(array('dialogData' => $html));
	}

	/**
	 * Returns a list of default DNs from account types + tree suffix.
	 *
	 * @return string[] default DNs
	 */
	private function getDefaultDns() {
		$typeManager = new TypeManager();
		$baseDnList = array();
		foreach ($typeManager->getConfiguredTypes() as $type) {
			$suffix = $type->getSuffix();
			if (!empty($suffix)) {
				$baseDnList[] = $suffix;
			}
		}
		$treeSuffix = $_SESSION['config']->get_Suffix('tree');
		if (!empty($treeSuffix)) {
			$baseDnList[] = $suffix;
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
	 */
	private function buildDnSelectionHtml($dnList, $currentDn) {
		$fieldId = trim($_POST['fieldId']);
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
			$row->setCSSClasses(array('text-right'));
			$buttonId = base64_encode($currentDn);
			$buttonId = str_replace('=', '', $buttonId);
			$button = new htmlButton($buttonId, _('Ok'));
			$button->setIconClass('okButton');
			$button->setOnClick('window.lam.html.selectDn(this, \'' . htmlspecialchars($fieldId) . '\')');
			$row->add($button, 12, 3);
			$mainRow->add($row, 12);
			// back up
			$row = new htmlResponsiveRow();
			$row->addDataAttribute('dn', extractDNSuffix($currentDn));
			$text = new htmlLink('..', '#');
			$text->setCSSClasses(array('bold'));
			$text->setOnClick($onclickUp);
			$row->add($text, 12, 9);
			$row->setCSSClasses(array('text-right'));
			$buttonId = base64_encode('..');
			$buttonId = str_replace('=', '', $buttonId);
			$button = new htmlButton($buttonId, _('Up'));
			$button->setIconClass('upButton');
			$button->setOnClick($onclickUp);
			$row->add($button, 12, 3);
			$mainRow->add($row, 12);
		}
		foreach ($dnList as $dn) {
			$row = new htmlResponsiveRow();
			$row->addDataAttribute('dn', $dn);
			$link = new htmlLink($dn, '#');
			$link->setOnClick($onclickUp);
			$row->add($link, 12, 9);
			$row->setCSSClasses(array('text-right'));
			$buttonId = base64_encode($dn);
			$buttonId = str_replace('=', '', $buttonId);
			$button = new htmlButton($buttonId, _('Ok'));
			$button->setIconClass('okButton');
			$button->setOnClick('window.lam.html.selectDn(this, \'' . htmlspecialchars($fieldId) . '\')');
			$row->add($button, 12, 3);
			$mainRow->add($row, 12);
		}
		$tabindex = 1000;
		ob_start();
		parseHtml(null, $mainRow, array(), false, $tabindex, 'user');
		$out = ob_get_contents();
		ob_end_clean();
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
		$dnList = array();
		foreach ($dnEntries as $entry) {
			$dnList[] = $entry['dn'];
		}
		usort($dnList, 'compareDN');
		return $dnList;
	}

}


?>
