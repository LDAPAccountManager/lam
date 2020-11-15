<?php
namespace LAM\LOGIN;
use htmlStatusMessage;
use \LAM\LIB\TWO_FACTOR\TwoFactorProviderService;
use \htmlResponsiveRow;
use \htmlGroup;
use \htmlOutputText;
use \htmlSpacer;
use \htmlSelect;
use \htmlInputField;
use \htmlButton;
use LAMException;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2017 - 2020  Roland Gruber

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
* This page redirects to the correct start page after checking 2nd factor.
*
* @package main
* @author Roland Gruber
*/

/** config object */
include_once '../lib/config.inc';

// start session
startSecureSession();

setlanguage();

$config = $_SESSION['config'];
$password = $_SESSION['ldap']->getPassword();
$user = $_SESSION['ldap']->getUserName();

// get serials
try {
	$service = new TwoFactorProviderService($config);
	$provider = $service->getProvider();
	$serials = $provider->getSerials($user, $password);
}
catch (\Exception $e) {
	logNewMessage(LOG_ERR, 'Unable to get 2-factor serials for ' . $user . ' ' . $e->getMessage());
	metaRefresh("login.php?2factor=error");
	die();
}

$twoFactorLabelConfig = $config->getTwoFactorAuthenticationLabel();
$twoFactorLabel = empty($twoFactorLabelConfig) ? _('PIN+Token') : $twoFactorLabelConfig;

if (sizeof($serials) == 0) {
	if ($config->getTwoFactorAuthenticationOptional()) {
		unset($_SESSION['2factorRequired']);
		metaRefresh("main.php");
		die();
	}
	else {
		metaRefresh("login.php?2factor=noToken");
		die();
	}
}

if (isset($_POST['logout'])) {
	// destroy session
	session_destroy();
	unset($_SESSION);
	// redirect to login page
	metaRefresh("login.php");
	exit();
}

if (isset($_POST['submit']) || isset($_POST['sig_response']) || isset($_POST['codeVerifier'])) {
	$twoFactorInput = isset($_POST['2factor']) ? $_POST['2factor'] : null;
	$serial = isset($_POST['serial']) ? $_POST['serial'] : null;
	if (!$provider->hasCustomInputForm() && (empty($twoFactorInput) || !in_array($serial, $serials))) {
		$errorMessage = sprintf(_('Please enter "%s".'), $twoFactorLabel);
	}
	else {
		$twoFactorValid = false;
		try {
			$twoFactorValid = $provider->verify2ndFactor($user, $password, $serial, $twoFactorInput);
		}
		catch (\Exception $e) {
			logNewMessage(LOG_WARNING, '2-factor verification failed: ' . $e->getMessage());
		}
		if ($twoFactorValid) {
			unset($_SESSION['2factorRequired']);
			metaRefresh("main.php");
			die();
		}
		else {
			$errorMessage = _('Verification failed.');
		}
	}
}

echo $_SESSION['header'];
printHeaderContents(_("Login"), '..');
?>
</head>
<body class="admin">
<?php

// include all JavaScript files
printJsIncludes('..');
?>

	<table border=0 width="100%" class="lamHeader ui-corner-all">
		<tr>
			<td align="left" height="30">
				<a class="lamLogo" href="http://www.ldap-account-manager.org/" target="new_window">LDAP Account Manager</a>
			</td>
		<td align="right" height=20>
		</td>
		</tr>
	</table>

	<br><br>

	<form id="2faform" enctype="multipart/form-data" action="login2Factor.php" method="post" autocomplete="off">
<?php
echo $config->getTwoFactorAuthenticationCaption();

?>
	<div class="centeredTable">
	<div class="roundedShadowBox limitWidth">
<?php

	$group = new htmlGroup();
	$row = new htmlResponsiveRow();
	// error
	if (!empty($errorMessage)) {
		$row->add(new \htmlStatusMessage('ERROR', $errorMessage), 12);
		$row->add(new htmlSpacer('1em', '1em'), 12);
	}

	if (!$provider->hasCustomInputForm()) {
		// serial
		$row->add(new htmlOutputText(_('Serial number')), 12, 12, 12, 'text-left');
		$serialSelect = new htmlSelect('serial', $serials);
		$row->add($serialSelect, 12);
		// token
		$row->add(new htmlOutputText($twoFactorLabel), 12, 12, 12, 'text-left');
		$twoFactorInput = new htmlInputField('2factor', '');
		$twoFactorInput->setFieldSize(null);
		$twoFactorInput->setIsPassword(true);
		$row->add($twoFactorInput, 12);
	}
	else {
	    try {
		    $provider->addCustomInput($row, $user);
	    }
	    catch (LAMException $e) {
	        logNewMessage(LOG_ERR, 'Error rendering 2FA form. ' . $e->getTitle());
	        $row->add(new htmlStatusMessage('ERROR', _('Unable to start 2-factor authentication.')), 12);
        }
	}

	// buttons
	$row->add(new htmlSpacer('1em', '1em'), 12);
	if ($provider->isShowSubmitButton()) {
		$submit = new htmlButton('submit', _("Submit"));
		$submit->setCSSClasses(array('fullwidth'));
		$row->add($submit, 12, 12, 12, 'fullwidth');
		$row->add(new htmlSpacer('0.5em', '0.5em'), 12);
	}
	$logout = new htmlButton('logout', _("Cancel"));
	$logout->setCSSClasses(array('fullwidth'));
	$row->add($logout, 12);
	$group->addElement($row);

	$tabindex = 1;
	addSecurityTokenToMetaHTML($group);
	parseHtml(null, $group, array(), false, $tabindex, 'user');

?>
	</div>
	</div>
	</form>
	<br><br>

	<script type="text/javascript">
		myElement = document.getElementsByName('2factor')[0];
		if (myElement) {
			myElement.focus();
		}
	</script>
</body>
</html>
