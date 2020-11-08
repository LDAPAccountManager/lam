<?php
namespace LAM\TOOLS\WEBAUTHN;
use \htmlButton;
use \htmlDiv;
use \htmlGroup;
use htmlHiddenInput;
use htmlInputField;
use \htmlOutputText;
use \htmlResponsiveRow;
use \htmlResponsiveTable;
use \htmlSpacer;
use \htmlStatusMessage;
use \htmlTitle;
use \LAM\LOGIN\WEBAUTHN\PublicKeyCredentialSourceRepositorySQLite;
use LAM\LOGIN\WEBAUTHN\WebauthnManager;
use Webauthn\PublicKeyCredentialCreationOptions;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2020  Roland Gruber

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
* Allows webauthn device management.
*
* @author Roland Gruber
* @package tools
*/

/** security functions */
include_once(__DIR__ . "/../../lib/security.inc");
/** access to configuration options */
include_once(__DIR__ . "/../../lib/config.inc");
/** webauthn */
include_once __DIR__ . '/../../lib/webauthn.inc';

// start session
startSecureSession();
enforceUserIsLoggedIn();
validateSecurityToken();

checkIfToolIsActive('toolWebauthn');

setlanguage();

include __DIR__ . '/../../lib/adminHeader.inc';
echo '<div class="user-bright smallPaddingContent">';
echo "<form id='webauthnform' action=\"webauthn.php\" method=\"post\">\n";
$tabindex = 1;
$container = new htmlResponsiveRow();

$container->add(new htmlTitle(_("WebAuthn devices")), 12);

$webauthnManager = new WebauthnManager();

$userDn = $_SESSION['ldap']->getUserName();
$database = new PublicKeyCredentialSourceRepositorySQLite();
showRemoveMessage($container);
addNewDevice($container, $webauthnManager);
$container->addVerticalSpacer('0.5rem');
$container->add(new htmlHiddenInput('registrationData', ''), 12);
$errorMessageDiv = new htmlDiv('generic-webauthn-error', new htmlOutputText(''));
$errorMessageDiv->addDataAttribute('button', _('Ok'));
$errorMessageDiv->addDataAttribute('title', _('WebAuthn failed'));
$container->add($errorMessageDiv, 12);
$buttonGroup = new htmlGroup();
$registerButton = new htmlButton('register', _('Register new device'));
$registerButton->addDataAttribute('dn', $userDn);
$registerButton->addDataAttribute('sec_token_value', getSecurityTokenValue());
$registerButton->addDataAttribute('sec_token_name', getSecurityTokenName());
$registration = $webauthnManager->getRegistrationObject($userDn, false);
$registrationJson = json_encode($registration);
$_SESSION['webauthn_registration'] = $registrationJson;
$registerButton->addDataAttribute('publickey', $registrationJson);
$registerButton->setIconClass('createButton');
$registerButton->setOnClick('window.lam.webauthn.registerOwnDevice(event, false);');
$buttonGroup->addElement($registerButton);
$container->add($buttonGroup, 12);
$container->addVerticalSpacer('2rem');
$results = $database->searchDevices($userDn);
if (empty($results)) {
	$container->add(new htmlStatusMessage('INFO', _('No devices found.')), 12);
}
else {
	$titles = array(
		_('Name'),
		_('Save'),
		_('Registration'),
		_('Last use'),
		_('Delete')
	);
	$data = array();
	$id = 0;
	foreach ($results as $result) {
		$credentialId = $result['credentialId'];
		$delButton = new htmlButton('deleteDevice' . $id, 'delete.png', true);
		$delButton->addDataAttribute('credential', $credentialId);
		$delButton->addDataAttribute('dn', $result['dn']);
		$delButton->addDataAttribute('dialogtitle', _('Remove device'));
		$delButton->addDataAttribute('oktext', _('Ok'));
		$delButton->addDataAttribute('canceltext', _('Cancel'));
		$delButton->setOnClick('window.lam.webauthn.removeOwnDevice(event, false);');
		$saveButton = new htmlButton('saveDevice' . $id, 'save.png', true);
		$saveButton->addDataAttribute('credential', $credentialId);
		$saveButton->addDataAttribute('dn', $result['dn']);
		$saveButton->addDataAttribute('nameelement', 'deviceName_' . $id);
		$saveButton->setOnClick('window.lam.webauthn.updateOwnDeviceName(event, false);');
		$nameField = new htmlInputField('deviceName_' . $id, $result['name']);
		$nameFieldClasses = array('maxwidth20');
		if (!empty($_GET['updated']) && ($_GET['updated'] === $credentialId)) {
			$nameFieldClasses[] = 'markPass';
		}
		$nameField->setCSSClasses($nameFieldClasses);
		$data[] = array(
			$nameField,
			$saveButton,
			new htmlOutputText(date('Y-m-d H:i:s', $result['registrationTime'])),
			new htmlOutputText(date('Y-m-d H:i:s', $result['lastUseTime'])),
			$delButton
		);
		$id++;
	}
	$table = new htmlResponsiveTable($titles, $data);
	$tableDiv = new htmlDiv('webauthn_results', $table);
	$tableDiv->addDataAttribute('sec_token_value', getSecurityTokenValue());
	$container->add($tableDiv, 12);
}
$container->addVerticalSpacer('2rem');

$confirmationDiv = new htmlDiv('webauthnDeleteConfirm', new htmlOutputText(_('Do you really want to remove this device?')), array('hidden'));
$container->add($confirmationDiv, 12);

addSecurityTokenToMetaHTML($container);

parseHtml(null, $container, array(), false, $tabindex, 'user');

echo '</form>';
echo '</div>';
include __DIR__ . '/../../lib/adminFooter.inc';

/**
 * Checks if a new device should be registered and adds it.
 *
 * @param htmlResponsiveRow $container row
 * @param WebauthnManager $webauthnManager webauthn manager
 */
function addNewDevice($container, $webauthnManager) {
	if (empty($_POST['registrationData'])) {
		return;
	}
	$registrationData = base64_decode($_POST['registrationData']);
	$registrationObject = PublicKeyCredentialCreationOptions::createFromString($_SESSION['webauthn_registration']);
	$success = $webauthnManager->storeNewRegistration($registrationObject, $registrationData);
	if ($success) {
		$container->add(new htmlStatusMessage('INFO', _('The device was registered.')), 12);
	}
	else {
		$container->add(new htmlStatusMessage('ERROR', _('The device failed to register.')), 12);
	}
}

/**
 * Shows the message if a device was removed.
 *
 * @param htmlResponsiveRow $container row
 */
function showRemoveMessage($container) {
	if (!empty($_POST['removed']) && ($_POST['removed'] === 'true')) {
		$container->add(new htmlStatusMessage('INFO', _('The device was deleted.')), 12);
	}
}
