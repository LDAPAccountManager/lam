<?php
namespace LAM\TOOLS\WEBAUTHN;
use \htmlButton;
use htmlDiv;
use htmlGroup;
use \htmlOutputText;
use \htmlResponsiveRow;
use \htmlResponsiveTable;
use \htmlStatusMessage;
use \htmlTitle;
use \LAM\LOGIN\WEBAUTHN\PublicKeyCredentialSourceRepositorySQLite;

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

checkIfToolIsActive('toolWebauthn');

setlanguage();

include __DIR__ . '/../../lib/adminHeader.inc';
echo '<div class="user-bright smallPaddingContent">';
echo "<form action=\"webauthn.php\" method=\"post\">\n";
$tabindex = 1;
$container = new htmlResponsiveRow();

$container->add(new htmlTitle(_("Webauthn devices")), 12);

$userDn = $_SESSION['ldap']->getUserName();
$database = new PublicKeyCredentialSourceRepositorySQLite();
$results = $database->searchDevices($userDn);
$container->addVerticalSpacer('0.5rem');
$buttonGroup = new htmlGroup();
$reloadButton = new htmlButton('reload', _('Reload'));
$reloadButton->setIconClass('refreshButton');
$buttonGroup->addElement($reloadButton);
$container->add($buttonGroup, 12);
$container->addVerticalSpacer('2rem');
if (empty($results)) {
	$container->add(new htmlStatusMessage('INFO', _('No devices found.')), 12);
}
else {
	$titles = array(
		_('Registration'),
		_('Last use'),
		_('Delete')
	);
	$data = array();
	$id = 0;
	foreach ($results as $result) {
		$delButton = new htmlButton('deleteDevice' . $id, 'delete.png', true);
		$delButton->addDataAttribute('credential', $result['credentialId']);
		$delButton->addDataAttribute('dn', $result['dn']);
		$delButton->addDataAttribute('dialogtitle', _('Remove device'));
		$delButton->addDataAttribute('oktext', _('Ok'));
		$delButton->addDataAttribute('canceltext', _('Cancel'));
		$delButton->setOnClick('window.lam.webauthn.removeOwnDevice(event);');
		$data[] = array(
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


parseHtml(null, $container, array(), false, $tabindex, 'user');

echo '</form>';
echo '</div>';
include __DIR__ . '/../../lib/adminFooter.inc';

?>
