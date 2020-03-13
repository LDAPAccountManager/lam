<?php
namespace LAM\CONFIG;

use \LAMCfgMain;
use \htmlTable;
use \htmlTitle;
use \htmlStatusMessage;
use \htmlSubTitle;
use \htmlSpacer;
use \htmlOutputText;
use \htmlLink;
use \htmlGroup;
use \htmlButton;
use \htmlHelpLink;
use \htmlInputField;
use \htmlInputFileUpload;
use \DateTime;
use \DateTimeZone;
use \htmlResponsiveRow;
use \htmlResponsiveInputTextarea;
use \htmlResponsiveSelect;
use \htmlResponsiveInputCheckbox;
use \htmlResponsiveInputField;
use \htmlDiv;
use \htmlHiddenInput;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2020  Roland Gruber

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
 * Manages the main configuration options.
 *
 * @package configuration
 * @author Roland Gruber
 */


/** Access to config functions */
include_once('../../lib/config.inc');
/** Used to print status messages */
include_once('../../lib/status.inc');
/** LAM Pro */
include_once('../../lib/selfService.inc');

// start session
if (strtolower(session_module_name()) == 'files') {
	session_save_path("../../sess");
}
lam_start_session();

setlanguage();

if (!isset($_SESSION['cfgMain'])) {
	$cfg = new LAMCfgMain();
	$_SESSION['cfgMain'] = $cfg;
}
$cfg = &$_SESSION['cfgMain'];

// check if user is logged in
if (!isset($_SESSION["mainconf_password"]) || (!$cfg->checkPassword($_SESSION["mainconf_password"]))) {
	require('mainlogin.php');
	exit();
}

if (isset($_POST['cancel'])) {
	// back to login
	metaRefresh('../login.php');
	exit();
}

$errors = array();
$messages = array();
// check if submit button was pressed
if (isset($_POST['submitFormData'])) {
	// set master password
	if (isset($_POST['masterpassword']) && ($_POST['masterpassword'] != "")) {
		if ($_POST['masterpassword'] && $_POST['masterpassword2'] && ($_POST['masterpassword'] == $_POST['masterpassword2'])) {
			$cfg->setPassword($_POST['masterpassword']);
			$msg = _("New master password set successfully.");
			unset($_SESSION["mainconf_password"]);
		} else {
			$errors[] = _("Master passwords are different or empty!");
		}
	}
	// set license
	if (isLAMProVersion()) {
		$licenseLines = explode("\n", $_POST['license']);
		$licenseLines = array_map('trim', $licenseLines);
		$cfg->setLicenseLines($licenseLines);
	}
	// set session timeout
	$cfg->sessionTimeout = $_POST['sessionTimeout'];
	// set allowed hosts
	if (isset($_POST['allowedHosts'])) {
		$allowedHosts = $_POST['allowedHosts'];
		$allowedHostsList = explode("\n", $allowedHosts);
		for ($i = 0; $i < sizeof($allowedHostsList); $i++) {
			$allowedHostsList[$i] = trim($allowedHostsList[$i]);
			// ignore empty lines
			if ($allowedHostsList[$i] == "") {
				unset($allowedHostsList[$i]);
				continue;
			}
			// check each line
			$ipRegex = '/^[0-9a-f\\.:\\*]+$/i';
			if (!preg_match($ipRegex, $allowedHostsList[$i]) || (strlen($allowedHostsList[$i]) > 15)) {
				$errors[] = sprintf(_("The IP address %s is invalid!"), htmlspecialchars(str_replace('%', '%%', $allowedHostsList[$i])));
			}
		}
		$allowedHosts = implode(",", $allowedHostsList);
	} else {
		$allowedHosts = "";
	}
	$cfg->allowedHosts = $allowedHosts;
	// set allowed hosts for self service
	if (isLAMProVersion()) {
		if (isset($_POST['allowedHostsSelfService'])) {
			$allowedHostsSelfService = $_POST['allowedHostsSelfService'];
			$allowedHostsSelfServiceList = explode("\n", $allowedHostsSelfService);
			for ($i = 0; $i < sizeof($allowedHostsSelfServiceList); $i++) {
				$allowedHostsSelfServiceList[$i] = trim($allowedHostsSelfServiceList[$i]);
				// ignore empty lines
				if ($allowedHostsSelfServiceList[$i] == "") {
					unset($allowedHostsSelfServiceList[$i]);
					continue;
				}
				// check each line
				$ipRegex = '/^[0-9a-f\\.:\\*]+$/i';
				if (!preg_match($ipRegex, $allowedHostsSelfServiceList[$i]) || (strlen($allowedHostsSelfServiceList[$i]) > 15)) {
					$errors[] = sprintf(_("The IP address %s is invalid!"), htmlspecialchars(str_replace('%', '%%', $allowedHostsSelfServiceList[$i])));
				}
			}
			$allowedHostsSelfService = implode(",", $allowedHostsSelfServiceList);
		} else {
			$allowedHostsSelfService = "";
		}
		$cfg->allowedHostsSelfService = $allowedHostsSelfService;
	}
	// set session encryption
	if (function_exists('openssl_random_pseudo_bytes')) {
		$encryptSession = 'false';
		if (isset($_POST['encryptSession']) && ($_POST['encryptSession'] == 'on')) {
			$encryptSession = 'true';
		}
		$cfg->encryptSession = $encryptSession;
	}
	// set log level
	$cfg->logLevel = $_POST['logLevel'];
	// set log destination
	if ($_POST['logDestination'] == "none") {
		$cfg->logDestination = "NONE";
	} elseif ($_POST['logDestination'] == "syslog") {
		$cfg->logDestination = "SYSLOG";
	} elseif ($_POST['logDestination'] == "remote") {
		$cfg->logDestination = "REMOTE:" . $_POST['logRemote'];
		$remoteParts = explode(':', $_POST['logRemote']);
		if ((sizeof($remoteParts) !== 2) || !get_preg($remoteParts[0], 'DNSname') || !get_preg($remoteParts[1], 'digit')) {
			$errors[] = _("Please enter a valid remote server in format \"server:port\".");
		}
	} else {
		if (isset($_POST['logFile']) && ($_POST['logFile'] != "") && preg_match("/^[a-z0-9\\/\\\\:\\._-]+$/i", $_POST['logFile'])) {
			$cfg->logDestination = $_POST['logFile'];
		} else {
			$errors[] = _("The log file is empty or contains invalid characters! Valid characters are: a-z, A-Z, 0-9, /, \\, ., :, _ and -.");
		}
	}
	// password policies
	$cfg->passwordMinLength = $_POST['passwordMinLength'];
	$cfg->passwordMinLower = $_POST['passwordMinLower'];
	$cfg->passwordMinUpper = $_POST['passwordMinUpper'];
	$cfg->passwordMinNumeric = $_POST['passwordMinNumeric'];
	$cfg->passwordMinSymbol = $_POST['passwordMinSymbol'];
	$cfg->passwordMinClasses = $_POST['passwordMinClasses'];
	$cfg->checkedRulesCount = $_POST['passwordRulesCount'];
	$cfg->passwordMustNotContain3Chars = isset($_POST['passwordMustNotContain3Chars']) && ($_POST['passwordMustNotContain3Chars'] == 'on') ? 'true' : 'false';
	$cfg->passwordMustNotContainUser = isset($_POST['passwordMustNotContainUser']) && ($_POST['passwordMustNotContainUser'] == 'on') ? 'true' : 'false';
	if (function_exists('curl_init')) {
		$cfg->externalPwdCheckUrl = $_POST['externalPwdCheckUrl'];
		if (!empty($cfg->externalPwdCheckUrl) && (strpos($cfg->externalPwdCheckUrl, '{SHA1PREFIX}') === false)) {
			$errors[] = _('The URL for the external password check is invalid.');
		}
	}
	if (isset($_POST['sslCaCertUpload'])) {
		if (!isset($_FILES['sslCaCert']) || ($_FILES['sslCaCert']['size'] == 0)) {
			$errors[] = _('No file selected.');
		} else {
			$handle = fopen($_FILES['sslCaCert']['tmp_name'], "r");
			$data = fread($handle, 10000000);
			fclose($handle);
			$sslReturn = $cfg->uploadSSLCaCert($data);
			if ($sslReturn !== true) {
				$errors[] = $sslReturn;
			} else {
				$messages[] = _('You might need to restart your webserver for changes to take effect.');
			}
		}
	}
	if (isset($_POST['sslCaCertDelete'])) {
		$cfg->deleteSSLCaCert();
		$messages[] = _('You might need to restart your webserver for changes to take effect.');
	}
	if (isset($_POST['sslCaCertImport'])) {
		$matches = array();
		if (preg_match('/^ldaps:\\/\\/([a-zA-Z0-9_\\.-]+)(:([0-9]+))?$/', $_POST['serverurl'], $matches)) {
			$port = '636';
			if (isset($matches[3]) && !empty($matches[3])) {
				$port = $matches[3];
			}
			$pemResult = getLDAPSSLCertificate($matches[1], $port);
			if ($pemResult !== false) {
				$messages[] = _('Imported certificate from server.');
				$messages[] = _('You might need to restart your webserver for changes to take effect.');
				$cfg->uploadSSLCaCert($pemResult);
			} else {
				$errors[] = _('Unable to import server certificate. Please use the upload function.');
			}
		} else {
			$errors[] = _('Invalid server name. Please enter "server" or "server:port".');
		}
	}
	foreach ($_POST as $key => $value) {
		if (strpos($key, 'deleteCert_') === 0) {
			$index = substr($key, strlen('deleteCert_'));
			$cfg->deleteSSLCaCert($index);
		}
	}
	// mail EOL
	if (isLAMProVersion()) {
		$cfg->mailEOL = $_POST['mailEOL'];
	}
	$cfg->errorReporting = $_POST['errorReporting'];
	// save settings
	if (isset($_POST['submit'])) {
		$cfg->save();
		if (sizeof($errors) == 0) {
			metaRefresh('../login.php?confMainSavedOk=1');
			exit();
		}
	}
}

echo $_SESSION['header'];
printHeaderContents(_("Edit general settings"), '../..');
?>
</head>
<body class="admin">
<table border=0 width="100%" class="lamHeader ui-corner-all">
    <tr>
        <td align="left" height="30">
            <a class="lamLogo" href="http://www.ldap-account-manager.org/" target="new_window">
				<?php echo getLAMVersionText(); ?>
            </a>
        </td>
    </tr>
</table>
<br>
<!-- form for adding/renaming/deleting profiles -->
<form enctype="multipart/form-data" action="mainmanage.php" method="post">

	<?php
	// include all JavaScript files
	printJsIncludes('../..');

	$tabindex = 1;

	$row = new htmlResponsiveRow();
	$row->add(new htmlTitle(_('General settings')), 12);

	// print messages
	for ($i = 0; $i < sizeof($errors); $i++) {
		$row->add(new htmlStatusMessage("ERROR", $errors[$i]), 12);
	}
	for ($i = 0; $i < sizeof($messages); $i++) {
		$row->add(new htmlStatusMessage("INFO", $messages[$i]), 12);
	}

	// check if config file is writable
	if (!$cfg->isWritable()) {
		$row->add(new htmlStatusMessage('WARN', _('The config file is not writable.'), _('Your changes cannot be saved until you make the file writable for the webserver user.')), 12);
	}

	// license
	if (isLAMProVersion()) {
		$row->add(new htmlSubTitle(_('Licence')), 12);
		$row->add(new htmlResponsiveInputTextarea('license', implode("\n", $cfg->getLicenseLines()), null, 10, _('Licence'), '287'), 12);

		$row->add(new htmlSpacer(null, '1rem'), true);
	}

	// security settings
	$row->add(new htmlSubTitle(_("Security settings")), 12);
	$options = array(5, 10, 20, 30, 60, 90, 120, 240);
	$row->add(new htmlResponsiveSelect('sessionTimeout', $options, array($cfg->sessionTimeout), _("Session timeout"), '238'), 12);
	$row->add(new htmlResponsiveInputTextarea('allowedHosts', implode("\n", explode(",", $cfg->allowedHosts)), null, '7', _("Allowed hosts"), '241'), 12);
	if (isLAMProVersion()) {
		$row->add(new htmlResponsiveInputTextarea('allowedHostsSelfService', implode("\n", explode(",", $cfg->allowedHostsSelfService)), null, '7', _("Allowed hosts (self service)"), '241'), 12);
	}
	$encryptSession = ($cfg->encryptSession === 'true');
	$encryptSessionBox = new htmlResponsiveInputCheckbox('encryptSession', $encryptSession, _('Encrypt session'), '245');
	$encryptSessionBox->setIsEnabled(function_exists('openssl_random_pseudo_bytes'));
	$row->add($encryptSessionBox, 12);
	// SSL certificate
	$row->addVerticalSpacer('1rem');
	$row->addLabel(new htmlOutputText(_('SSL certificates')));
	$sslMethod = _('use system certificates');
	$sslFileName = $cfg->getSSLCaCertTempFileName();
	if ($sslFileName != null) {
		$sslMethod = _('use custom CA certificates');
	}
	$sslDelSaveGroup = new htmlGroup();
	$sslDelSaveGroup->addElement(new htmlOutputText($sslMethod));
	$sslDelSaveGroup->addElement(new htmlSpacer('5px', null));
	// delete+download button
	if ($sslFileName != null) {
		$sslDownloadBtn = new htmlLink('', '../../tmp/' . $sslFileName, '../../graphics/save.png');
		$sslDownloadBtn->setTargetWindow('_blank');
		$sslDownloadBtn->setTitle(_('Download CA certificates'));
		$sslDelSaveGroup->addElement($sslDownloadBtn);
		$sslDeleteBtn = new htmlButton('sslCaCertDelete', 'delete.png', true);
		$sslDeleteBtn->setTitle(_('Delete all CA certificates'));
		$sslDelSaveGroup->addElement($sslDeleteBtn);
	}
	$sslDelSaveGroup->addElement(new htmlHelpLink('204'));
	$row->addField($sslDelSaveGroup);
	$row->addLabel(new htmlInputFileUpload('sslCaCert'));
	$sslUploadBtn = new htmlButton('sslCaCertUpload', _('Upload'));
	$sslUploadBtn->setIconClass('upButton');
	$sslUploadBtn->setTitle(_('Upload CA certificate in DER/PEM format.'));
	$row->addField($sslUploadBtn);
	if (function_exists('stream_socket_client') && function_exists('stream_context_get_params')) {
		$sslImportServerUrl = !empty($_POST['serverurl']) ? $_POST['serverurl'] : 'ldaps://';
		$serverUrlUpload = new htmlInputField('serverurl', $sslImportServerUrl);
		$row->addLabel($serverUrlUpload);
		$sslImportBtn = new htmlButton('sslCaCertImport', _('Import from server'));
		$sslImportBtn->setIconClass('downButton');
		$sslImportBtn->setTitle(_('Imports the certificate directly from your LDAP server.'));
		$row->addField($sslImportBtn);
	}

	$sslCerts = $cfg->getSSLCaCertificates();
	if (sizeof($sslCerts) > 0) {
		$certsTitles = array(_('Common name'), _('Valid to'), _('Serial number'), _('Delete'));
		$certsData = array();
		for ($i = 0; $i < sizeof($sslCerts); $i++) {
			$serial = isset($sslCerts[$i]['serialNumber']) ? $sslCerts[$i]['serialNumber'] : '';
			$validTo = isset($sslCerts[$i]['validTo_time_t']) ? $sslCerts[$i]['validTo_time_t'] : '';
			$cn = isset($sslCerts[$i]['subject']['CN']) ? $sslCerts[$i]['subject']['CN'] : '';
			$delBtn = new htmlButton('deleteCert_' . $i, 'delete.png', true);
			$certsData[] = array(
				new htmlOutputText($cn),
				new htmlOutputText($validTo),
				new htmlOutputText($serial),
				$delBtn
			);
		}
		$certsTable = new \htmlResponsiveTable($certsTitles, $certsData);
		$row->add($certsTable, 12);
	}

	// password policy
	$row->add(new htmlSubTitle(_("Password policy")), 12);
	$options20 = array(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20);
	$options4 = array(0, 1, 2, 3, 4);
	$row->add(new htmlResponsiveSelect('passwordMinLength', $options20, array($cfg->passwordMinLength), _('Minimum password length'), '242'), 12);
	$row->addVerticalSpacer('1rem');
	$row->add(new htmlResponsiveSelect('passwordMinLower', $options20, array($cfg->passwordMinLower), _('Minimum lowercase characters'), '242'), 12);
	$row->add(new htmlResponsiveSelect('passwordMinUpper', $options20, array($cfg->passwordMinUpper), _('Minimum uppercase characters'), '242'), 12);
	$row->add(new htmlResponsiveSelect('passwordMinNumeric', $options20, array($cfg->passwordMinNumeric), _('Minimum numeric characters'), '242'), 12);
	$row->add(new htmlResponsiveSelect('passwordMinSymbol', $options20, array($cfg->passwordMinSymbol), _('Minimum symbolic characters'), '242'), 12);
	$row->add(new htmlResponsiveSelect('passwordMinClasses', $options4, array($cfg->passwordMinClasses), _('Minimum character classes'), '242'), 12);
	$row->addVerticalSpacer('1rem');
	$rulesCountOptions = array(_('all') => '-1', '3' => '3', '4' => '4');
	$rulesCountSelect = new htmlResponsiveSelect('passwordRulesCount', $rulesCountOptions, array($cfg->checkedRulesCount), _('Number of rules that must match'), '246');
	$rulesCountSelect->setHasDescriptiveElements(true);
	$row->add($rulesCountSelect, 12);
	$passwordMustNotContainUser = ($cfg->passwordMustNotContainUser === 'true');
	$row->add(new htmlResponsiveInputCheckbox('passwordMustNotContainUser', $passwordMustNotContainUser, _('Password must not contain user name'), '247'), 12);
	$passwordMustNotContain3Chars = ($cfg->passwordMustNotContain3Chars === 'true');
	$row->add(new htmlResponsiveInputCheckbox('passwordMustNotContain3Chars', $passwordMustNotContain3Chars, _('Password must not contain part of user/first/last name'), '248'), 12);
	if (function_exists('curl_init')) {
		$row->addVerticalSpacer('1rem');
		$row->add(new htmlResponsiveInputField(_('External password check'), 'externalPwdCheckUrl', $cfg->externalPwdCheckUrl, '249'), 12);
	}

	// logging
	$row->add(new htmlSubTitle(_("Logging")), 12);
	$levelOptions = array(_("Debug") => LOG_DEBUG, _("Notice") => LOG_NOTICE, _("Warning") => LOG_WARNING, _("Error") => LOG_ERR);
	$levelSelect = new htmlResponsiveSelect('logLevel', $levelOptions, array($cfg->logLevel), _("Log level"), '239');
	$levelSelect->setHasDescriptiveElements(true);
	$row->add($levelSelect, 12);
	$destinationOptions = array(
		_("No logging") => "none",
		_("System logging") => "syslog",
		_("File") => 'file',
		_("Remote") => 'remote',
	);
	$destinationSelected = 'file';
	$destinationPath = $cfg->logDestination;
	$destinationRemote = '';
	if ($cfg->logDestination == 'NONE') {
		$destinationSelected = 'none';
		$destinationPath = '';
	} elseif ($cfg->logDestination == 'SYSLOG') {
		$destinationSelected = 'syslog';
		$destinationPath = '';
	} elseif (strpos($cfg->logDestination, 'REMOTE') === 0) {
		$destinationSelected = 'remote';
		$remoteParts = explode(':', $cfg->logDestination, 2);
		$destinationRemote = empty($remoteParts[1]) ? '' : $remoteParts[1];
		$destinationPath = '';
	}
	$logDestinationSelect = new htmlResponsiveSelect('logDestination', $destinationOptions, array($destinationSelected), _("Log destination"), '240');
	$logDestinationSelect->setTableRowsToHide(array(
		'none' => array('logFile', 'logRemote'),
		'syslog' => array('logFile', 'logRemote'),
		'remote' => array('logFile'),
		'file' => array('logRemote'),
	));
	$logDestinationSelect->setTableRowsToShow(array(
		'file' => array('logFile'),
		'remote' => array('logRemote'),
	));
	$logDestinationSelect->setHasDescriptiveElements(true);
	$row->add($logDestinationSelect, 12);
	$row->add(new htmlResponsiveInputField(_('File'), 'logFile', $destinationPath), 12);
	$row->add(new htmlResponsiveInputField(_('Remote server'), 'logRemote', $destinationRemote, '251'), 12);
	$errorLogOptions = array(
		_('PHP system setting') => LAMCfgMain::ERROR_REPORTING_SYSTEM,
		_('default') => LAMCfgMain::ERROR_REPORTING_DEFAULT,
		_('all') => LAMCfgMain::ERROR_REPORTING_ALL
	);
	$errorLogSelect = new htmlResponsiveSelect('errorReporting', $errorLogOptions, array($cfg->errorReporting), _('PHP error reporting'), '244');
	$errorLogSelect->setHasDescriptiveElements(true);
	$row->add($errorLogSelect, 12);

	// additional options
	if (isLAMProVersion()) {
		$row->add(new htmlSubTitle(_('Additional options')), 12);
		$mailEOLOptions = array(
			_('Default (\r\n)') => 'default',
			_('Non-standard (\n)') => 'unix'
		);
		$mailEOLSelect = new htmlResponsiveSelect('mailEOL', $mailEOLOptions, array($cfg->mailEOL), _('Email format'), '243');
		$mailEOLSelect->setHasDescriptiveElements(true);
		$row->add($mailEOLSelect, 12);
	}
	$row->addVerticalSpacer('3rem');

	// webauthn management
	if ((version_compare(phpversion(), '7.2.0') >= 0)
		&& extension_loaded('PDO')
		&& in_array('sqlite', \PDO::getAvailableDrivers())) {
		include_once __DIR__ . '/../../lib/webauthn.inc';
		$database = new \LAM\LOGIN\WEBAUTHN\PublicKeyCredentialSourceRepositorySQLite();
		if ($database->hasRegisteredCredentials()) {
			$row->add(new htmlSubTitle(_('Webauthn devices')), 12);
			$webauthnSearchField = new htmlResponsiveInputField(_('User DN'), 'webauthn_searchTerm', null, '252');
			$row->add($webauthnSearchField, 12);
			$row->addVerticalSpacer('0.5rem');
			$row->add(new htmlButton('webauthn_search', _('Search')), 12, 12, 12, 'text-center');
			$resultDiv = new htmlDiv('webauthn_results', new htmlOutputText(''), array('lam-webauthn-results'));
			addSecurityTokenToSession(false);
			$resultDiv->addDataAttribute('sec_token_value', getSecurityTokenValue());
			$row->add($resultDiv, 12);
			$confirmationDiv = new htmlDiv('webauthnDeleteConfirm', new htmlOutputText(_('Do you really want to remove this device?')), array('hidden'));
			$row->add($confirmationDiv, 12);
		}
	}

	// change master password
	$row->add(new htmlSubTitle(_("Change master password")), 12);
	$pwd1 = new htmlResponsiveInputField(_("New master password"), 'masterpassword', '', '235');
	$pwd1->setIsPassword(true, false, true);
	$row->add($pwd1, 12);
	$pwd2 = new htmlResponsiveInputField(_("Reenter password"), 'masterpassword2', '');
	$pwd2->setIsPassword(true, false, true);
	$pwd2->setSameValueFieldID('masterpassword');
	$row->add($pwd2, 12);
	$row->addVerticalSpacer('3rem');

	// buttons
	if ($cfg->isWritable()) {
		$buttonTable = new htmlTable();
		$buttonTable->addElement(new htmlButton('submit', _("Ok")));
		$buttonTable->addElement(new htmlSpacer('1rem', null));
		$buttonTable->addElement(new htmlButton('cancel', _("Cancel")));
		$row->add($buttonTable, 12);
		$row->add(new htmlHiddenInput('submitFormData', '1'), 12);
	}

	$box = new htmlDiv(null, $row);
	$box->setCSSClasses(array('ui-corner-all', 'roundedShadowBox'));
	parseHtml(null, $box, array(), false, $tabindex, 'user');


	/**
	 * Formats an LDAP time string (e.g. from createTimestamp).
	 *
	 * @param String $time LDAP time value
	 * @return String formatted time
	 */
	function formatSSLTimestamp($time) {
		if (!empty($time)) {
			$timeZone = 'UTC';
			$sysTimeZone = @date_default_timezone_get();
			if (!empty($sysTimeZone)) {
				$timeZone = $sysTimeZone;
			}
			$date = new DateTime('@' . $time, new DateTimeZone($timeZone));
			return $date->format('d.m.Y');
		}
		return '';
	}


	?>

</form>
<p><br></p>

</body>
</html>

