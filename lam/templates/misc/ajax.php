<?php
namespace LAM\AJAX;
use \LAM\TOOLS\IMPORT_EXPORT\Importer;
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2011 - 2018  Roland Gruber

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
include_once("../../lib/security.inc");
/** LDIF import */
include_once("../../lib/import.inc");

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

		if (isset($_GET['module']) && isset($_GET['scope']) && in_array($_GET['module'], getAvailableModules($_GET['scope']))) {
			enforceUserIsLoggedIn();
			if (isset($_GET['useContainer']) && ($_GET['useContainer'] == '1')) {
				if (!isset($_SESSION['account'])) die();
				$module = $_SESSION['account']->getAccountModule($_GET['module']);
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
		elseif ($function === 'upload') {
			include_once('../../lib/upload.inc');
			$typeManager = new \LAM\TYPES\TypeManager();
			$uploader = new \LAM\UPLOAD\Uploader($typeManager->getConfiguredType($_GET['typeId']));
			ob_start();
			$jsonOut = $uploader->doUpload();
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
		$return = $_SESSION['account']->setNewPassword($input);
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

}


?>
