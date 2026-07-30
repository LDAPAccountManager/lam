<?php

namespace LAM\INIT;

use htmlButton;
use htmlJavaScript;
use htmlOutputText;
use htmlResponsiveInputField;
use htmlResponsiveRow;
use htmlStatusMessage;
use htmlTitle;
use LAMCfgMain;
use LAMException;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2025  Roland Gruber

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
* Password dialog for initial configuration.
*
* @author Roland Gruber
* @package main
*/

/** security functions */
include_once __DIR__ . "/../lib/security.inc";
/** access to configuration settings */
include_once __DIR__ . "/../lib/config.inc";
/** status messages */
include_once __DIR__ . "/../lib/status.inc";

// set session save path
if (isFileBasedSession()) {
	session_save_path(__DIR__ . '/../sess');
}

lam_start_session();

$cfgMain = new LAMCfgMain();
if ($cfgMain->hasPasswordSet()) {
	logNewMessage(LOG_ERR, 'Invalid attempt to set initial config password');
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
	// check password strength
	$pwdPolicyResult = isValidConfigurationPassword($password1);
	if (!$pwdPolicyResult) {
		$message = new htmlStatusMessage('ERROR', _('Please enter at least 8 characters including letters, a number and a symbol.'));
		printContent($message);
		exit();
	}
	// set new password
	$cfgMain->setPassword($password1);
	try {
		$cfgMain->save();
		metaRefresh('config/mainlogin.php');
		exit();
	}
	catch (LAMException $e) {
		$message = new htmlStatusMessage('ERROR', $e->getTitle(), $e->getMessage());
	}
}

printContent($message);

/**
 * Displays the content area
 *
 * @param htmlStatusMessage|null $message status message
 * @param bool $showPasswordInputs show password input fields
 */
function printContent(?htmlStatusMessage $message = null, bool $showPasswordInputs = true): void {
	echo '
		<!DOCTYPE html>
		<html><head>
		<meta name="robots" content="noindex, nofollow">
		<meta http-equiv="content-type" content="text/html; charset=UTF-8">
		<meta http-equiv="pragma" content="no-cache">
		<meta http-equiv="cache-control" content="no-cache">
	';
	printHeaderContents('LDAP Account Manager', '..');
	echo '</head>
	<body>';
	printJsIncludes('..');

	echo '<div class="smallPaddingContent">';
	echo "<form action=\"setInitialPassword.php\" method=\"post\">\n";
	addSecurityTokenToSession();
	$container = new htmlResponsiveRow();
	if ($message !== null) {
		$container->addVerticalSpacer('1rem');
		$container->add($message);
	}
	$container->addVerticalSpacer('2rem');
	if ($showPasswordInputs) {
		$container->add(new htmlTitle(_('Initial configuration')));
		$container->addVerticalSpacer('3rem');
		$container->add(new htmlOutputText(_("Please enter your new LAM main configuration password.")), 12, 12, 12, 'text-center');
		$container->addVerticalSpacer('2rem');
		$pwdInput1 = new htmlResponsiveInputField(_('New password'), 'password1', '');
		$pwdInput1->setIsPassword(true, true, true);
		$container->add($pwdInput1);
		$pwdInput2 = new htmlResponsiveInputField(_('Repeat password'), 'password2', '');
		$pwdInput2->setIsPassword(true);
		$pwdInput2->setSameValueFieldID('password1');
		$container->add($pwdInput2);
		$container->addVerticalSpacer('1rem');
		$container->add(new htmlButton('changePassword', _("Submit")), 12, 12, 12, 'text-center');
		addSecurityTokenToMetaHTML($container);
		$container->add(new htmlJavaScript('checkFieldsHaveSameValues("password1", "password2");'));
	}

	parseHtml(null, $container, [], false);

	echo '</form><br>
	</div>
	<br><br>
    </body>
    </html>';
}
