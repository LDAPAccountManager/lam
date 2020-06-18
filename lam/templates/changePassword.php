<?php

namespace LAM\INIT;

use htmlButton;
use htmlOutputText;
use htmlResponsiveInputField;
use htmlResponsiveRow;
use htmlStatusMessage;

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
* Password change dialog for expired passwords.
*
* @author Roland Gruber
* @package main
*/

/** security functions */
include_once(__DIR__ . "/../lib/security.inc");
/** access to configuration settings */
include_once(__DIR__ . "/../lib/config.inc");
/** LDAP access */
include_once(__DIR__ . "/../lib/ldap.inc");
/** status messages */
include_once(__DIR__ . "/../lib/status.inc");

// start session
startSecureSession();
enforceUserIsLoggedIn();

if (!checkIfWriteAccessIsAllowed()) {
	die();
}

setlanguage();

if (!empty($_POST)) {
	validateSecurityToken();
}

$message = null;

// check if user already pressed button
if (isset($_POST['changePassword'])) {
	// check new password
	$password1 = $_POST['password1'];
	$password2 = $_POST['password2'];
	if ($password1 == '') {
		$message = new htmlStatusMessage('ERROR', _('No password was entered!'));
		printContent($message);
		exit();
	}
	// check if passwords match
	if ($password1 != $password2) {
		$message = new htmlStatusMessage('ERROR', _('Passwords are different!'));
		printContent($message);
		exit();
	}
	// check passsword strength
	$userDn = $_SESSION['ldap']->getUserName();
	$additionalAttrs = array();
	$rdnAttr = extractRDNAttribute($userDn);
	$userName = null;
	if (($rdnAttr === 'uid') || ($rdnAttr === 'uid')) {
		$userName = extractRDNValue($userDn);
	}
	$pwdPolicyResult = checkPasswordStrength($password1, $userName, $additionalAttrs);
	if ($pwdPolicyResult !== true) {
		$message = new htmlStatusMessage('ERROR', $pwdPolicyResult);
		printContent($message);
		exit();
	}
	// set new password
	$modifyResult = @ldap_exop_passwd($_SESSION['ldap']->server(), $userDn, $_SESSION['ldap']->getPassword(), $password1);
	if ($modifyResult === true) {
		$_SESSION['ldap']->encrypt_login($userDn, $password1);
		$message = new htmlStatusMessage('INFO', _('Password changed.'));
		printContent($message, false);
		exit();
	}
	else {
		$message = new htmlStatusMessage('ERROR', _('Unable to set password'), getExtendedLDAPErrorMessage($_SESSION['ldap']->server()));
		printContent($message);
		exit();
	}
}

printContent($message);

/**
 * Displays the content area
 *
 * @param htmlStatusMessage $message status message
 * @param bool $showPasswordInputs show password input fields
 */
function printContent($message = null, $showPasswordInputs = true) {
	include __DIR__ . '/../lib/adminHeader.inc';
	echo '<div class="user-bright smallPaddingContent">';
	echo "<form action=\"changePassword.php\" method=\"post\">\n";
	$container = new htmlResponsiveRow();
	if ($message !== null) {
		$container->addVerticalSpacer('1rem');
		$container->add($message, 12);
	}
	$container->addVerticalSpacer('2rem');
	if ($showPasswordInputs) {
		$container->add(new htmlOutputText(_("It seems your password expired. You can set a new one here.")), 12, 12, 12, 'text-center');
		$container->addVerticalSpacer('2rem');
		$pwdInput1 = new htmlResponsiveInputField(_('New password'), 'password1', '');
		$pwdInput1->setIsPassword(true, true, true);
		$container->add($pwdInput1, 12);
		$pwdInput2 = new htmlResponsiveInputField(_('Repeat password'), 'password2', '');
		$pwdInput2->setIsPassword(true);
		$pwdInput2->setSameValueFieldID('password1');
		$container->add($pwdInput2, 12);
		$container->addVerticalSpacer('1rem');
		$container->add(new htmlButton('changePassword', _("Submit")), 12, 12, 12, 'text-center');
		addSecurityTokenToMetaHTML($container);
	}

	$tabindex = 1;
	parseHtml(null, $container, array(), false, $tabindex, 'user');

	echo "</form><br>\n";
	echo "</div>\n";
	include __DIR__ . '/../lib/adminFooter.inc';
}
