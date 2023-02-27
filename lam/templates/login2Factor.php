<?php
namespace LAM\LOGIN;
use Exception;
use htmlJavaScript;
use htmlResponsiveInputCheckbox;
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
  Copyright (C) 2017 - 2023  Roland Gruber

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
$tabIndex = 1;

// get serials
try {
	$service = new TwoFactorProviderService($config);
	$provider = $service->getProvider();
	if ($provider->supportsToRememberDevice() && $service->isValidRememberedDevice($user)) {
		unset($_SESSION['2factorRequired']);
		metaRefresh("main.php");
		die();
	}
	$serials = $provider->getSerials($user, $password);
}
catch (Exception $e) {
	logNewMessage(LOG_ERR, 'Unable to get 2-factor serials for ' . $user . ' ' . $e->getMessage());
	printHeader();
	$scriptTag = new htmlJavaScript('window.lam.dialog.showErrorMessageAndRedirect("' . _("Unable to start 2-factor authentication.") . '", "", "' . _('Ok') . '", "login.php")');
	parseHtml(null, $scriptTag, array(), false, $tabIndex, null);
	printFooter();
	die();
}

$twoFactorLabelConfig = $config->getTwoFactorAuthenticationLabel();
$twoFactorLabel = empty($twoFactorLabelConfig) ? _('PIN+Token') : $twoFactorLabelConfig;

if (isset($_POST['logout'])) {
	// destroy session
	session_destroy();
	unset($_SESSION);
	// redirect to login page
	metaRefresh("login.php");
	exit();
}

if (empty($serials) && $config->getTwoFactorAuthenticationOptional()) {
    unset($_SESSION['2factorRequired']);
    metaRefresh("main.php");
    die();
}

if (empty($serials)) {
    printHeader();
	$scriptTag = new htmlJavaScript('window.lam.dialog.showErrorMessageAndRedirect("' . _("Unable to start 2-factor authentication because no tokens were found.") . '", "", "' . _('Ok') . '", "login.php")');
	parseHtml(null, $scriptTag, array(), false, $tabIndex, null);
	printFooter();
	die();
}

if (isset($_POST['submit']) || isset($_POST['sig_response'])
    || isset($_POST['codeVerifier']) || isset($_GET['code'])
    || (isset($_GET['session_state']) && isset($_GET['redirect_uri']))) {
	$twoFactorInput = isset($_POST['2factor']) ? $_POST['2factor'] : null;
	$serial = isset($_POST['serial']) ? $_POST['serial'] : null;
	if (!$provider->hasCustomInputForm() && (empty($twoFactorInput) || !in_array($serial, $serials))) {
		$errorMessage = sprintf(_('Please enter "%s".'), $twoFactorLabel);
		header("HTTP/1.1 403 Forbidden");
	}
	else {
		$twoFactorValid = false;
		try {
			$twoFactorValid = $provider->verify2ndFactor($user, $password, $serial, $twoFactorInput);
			if ($twoFactorValid
                && $provider->supportsToRememberDevice()
				&& isset($_POST['rememberDevice'])
				&& ($_POST['rememberDevice'] === 'on')) {
                $service->rememberDevice($user);
			}
		}
		catch (Exception $e) {
			logNewMessage(LOG_WARNING, '2-factor verification failed: ' . $e->getMessage());
			header("HTTP/1.1 403 Forbidden");
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

/**
 * Prints the page header.
 */
function printHeader(): void {
	echo $_SESSION['header'];
	printHeaderContents(_("Login"), '..');
    echo '</head><body>';
    // include all JavaScript files
	printJsIncludes('..');
	?>
    <div id="lam-topnav" class="lam-header">
        <div class="lam-header-left lam-menu-stay">
            <a href="https://www.ldap-account-manager.org/" target="new_window">
                <img class="align-middle" width="24" height="24" alt="help" src="../graphics/logo24.png">
                <span class="hide-on-mobile">
                            <?php
							echo getLAMVersionText();
							?>
                        </span>
            </a>
        </div>
    </div>
    <?php
}

/**
 * Prints the page footer.
 */
function printFooter(): void {
    echo '</body></html>';
}

printHeader();


?>
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
	$row->add(new htmlSpacer('1em', '1em'));
    if ($provider->supportsToRememberDevice()) {
        $remember = new htmlResponsiveInputCheckbox('rememberDevice', false, _('Remember device'), '560');
        $remember->setCSSClasses(array('lam-save-selection'));
        $row->add($remember);
        $row->add(new htmlSpacer('0.5em', '0.5em'));
    }
	if ($provider->isShowSubmitButton()) {
		$submit = new htmlButton('submit', _("Submit"));
		$submit->setCSSClasses(array('fullwidth'));
		$row->add($submit, 12, 12, 12, 'fullwidth');
		$row->add(new htmlSpacer('0.5em', '0.5em'));
	}
	$logout = new htmlButton('logout', _("Cancel"));
	$logout->setCSSClasses(array('fullwidth'));
	$row->add($logout);
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

<?php
printFooter();
