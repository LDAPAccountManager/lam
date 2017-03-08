<?php
namespace LAM\LOGIN;
use \LAM\LIB\TWO_FACTOR\TwoFactorProviderService;
use \htmlResponsiveRow;
use \htmlGroup;
use \htmlOutputText;
use \htmlSpacer;
use \htmlSelect;
use \htmlInputField;
use \htmlButton;
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2017  Roland Gruber

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
$ldap = $_SESSION['ldap'];
$credentials = $ldap->decrypt_login();
$password = $credentials[1];
$user = $_SESSION['user2factor'];
if (get_preg($user, 'dn')) {
	$user = extractRDNValue($user);
}

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
		unset($_SESSION['user2factor']);
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

if (isset($_POST['submit'])) {
	$twoFactorInput = $_POST['2factor'];
	$serial = $_POST['serial'];
	if (empty($twoFactorInput) || !in_array($serial, $serials)) {
		$errorMessage = _(sprintf('Please enter "%s".', $twoFactorLabel));
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
			unset($_SESSION['user2factor']);
			metaRefresh("main.php");
			die();
		}
		else {
			$errorMessage = _(sprintf('Verification failed.', $twoFactorLabel));
		}
	}
}

?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html class="no-js">
<head>
<meta http-equiv="content-type" content="text/html; charset=UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<meta http-equiv="pragma" content="no-cache">
		<meta http-equiv="cache-control" content="no-cache">
	<title><?php echo _("Login"); ?></title>
	<link rel="stylesheet" type="text/css" href="../style/responsive/105_normalize.css">
	<link rel="stylesheet" type="text/css" href="../style/responsive/110_foundation.css">
	<?php
		// include all CSS files
		$cssDirName = dirname(__FILE__) . '/../style';
		$cssDir = dir($cssDirName);
		$cssFiles = array();
		$cssEntry = $cssDir->read();
		while ($cssEntry !== false) {
			if (substr($cssEntry, strlen($cssEntry) - 4, 4) == '.css') {
				$cssFiles[] = $cssEntry;
			}
			$cssEntry = $cssDir->read();
		}
		sort($cssFiles);
		foreach ($cssFiles as $cssEntry) {
			echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/" . $cssEntry . "\">\n";
		}
		if (isset($profile->additionalCSS) && ($profile->additionalCSS != '')) {
			$CSSlinks = explode("\n", $profile->additionalCSS);
			for ($i = 0; $i < sizeof($CSSlinks); $i++) {
				$CSSlinks[$i] = trim($CSSlinks[$i]);
				if ($CSSlinks[$i] == '') {
					continue;
				}
				echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"" . $CSSlinks[$i] . "\">\n";
			}
		}
	?>
</head>
<body class="admin">
<?php

// include all JavaScript files
$jsDirName = dirname(__FILE__) . '/lib';
$jsDir = dir($jsDirName);
$jsFiles = array();
while ($jsEntry = $jsDir->read()) {
	if (substr($jsEntry, strlen($jsEntry) - 3, 3) != '.js') continue;
	$jsFiles[] = $jsEntry;
}
sort($jsFiles);
foreach ($jsFiles as $jsEntry) {
	echo "<script type=\"text/javascript\" src=\"lib/" . $jsEntry . "\"></script>\n";
}
?>

	<script type="text/javascript" src="lib/extra/responsive/200_modernizr.js"></script>
	<script type="text/javascript" src="lib/extra/responsive/250_foundation.js"></script>
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

	<form enctype="multipart/form-data" action="login2Factor.php" method="post" autocomplete="off">
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
	$row->add(new htmlSpacer('1em', '1em'), 12);
	$submit = new htmlButton('submit', _("Submit"));
	$submit->setCSSClasses(array('fullwidth'));
	$row->add($submit, 12, 12, 12, 'fullwidth');
	$row->add(new htmlSpacer('0.5em', '0.5em'), 12);
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
		$(document).foundation();
		myElement = document.getElementsByName('2factor')[0];
		myElement.focus();
	</script>
</body>
</html>
