<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2013  Roland Gruber

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

// start session
if (strtolower(session_module_name()) == 'files') {
	session_save_path("../../sess");
}
@session_start();

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
	// remove double slashes if magic quotes are on
	if (get_magic_quotes_gpc() == 1) {
		$postKeys = array_keys($_POST);
		for ($i = 0; $i < sizeof($postKeys); $i++) {
			if (is_string($_POST[$postKeys[$i]])) $_POST[$postKeys[$i]] = stripslashes($_POST[$postKeys[$i]]);
		}
	}
	// set master password
	if (isset($_POST['masterpassword']) && ($_POST['masterpassword'] != "")) {
		if ($_POST['masterpassword'] && $_POST['masterpassword2'] && ($_POST['masterpassword'] == $_POST['masterpassword2'])) {
			$cfg->setPassword($_POST['masterpassword']);
			$msg = _("New master password set successfully.");
			unset($_SESSION["mainconf_password"]);
		}
		else $errors[] = _("Master passwords are different or empty!");
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
	}
	else $allowedHosts = "";
	$cfg->allowedHosts = $allowedHosts;
	// set log level
	$cfg->logLevel = $_POST['logLevel'];
	// set log destination
	if ($_POST['logDestination'] == "none") $cfg->logDestination = "NONE";
	elseif ($_POST['logDestination'] == "syslog") $cfg->logDestination = "SYSLOG";
	else {
		if (isset($_POST['logFile']) && ($_POST['logFile'] != "") && preg_match("/^[a-z0-9\\/\\\\:\\._-]+$/i", $_POST['logFile'])) {
			$cfg->logDestination = $_POST['logFile'];
		}
		else $errors[] = _("The log file is empty or contains invalid characters! Valid characters are: a-z, A-Z, 0-9, /, \\, ., :, _ and -.");
	}
	// password policies
	$cfg->passwordMinLength = $_POST['passwordMinLength'];
	$cfg->passwordMinLower = $_POST['passwordMinLower'];
	$cfg->passwordMinUpper = $_POST['passwordMinUpper'];
	$cfg->passwordMinNumeric = $_POST['passwordMinNumeric'];
	$cfg->passwordMinSymbol = $_POST['passwordMinSymbol'];
	$cfg->passwordMinClasses = $_POST['passwordMinClasses'];
	if (isset($_POST['sslCaCertUpload'])) {
		if (!isset($_FILES['sslCaCert']) || ($_FILES['sslCaCert']['size'] == 0)) {
			$errors[] = _('No file selected.');
		}
		else {
			$handle = fopen($_FILES['sslCaCert']['tmp_name'], "r");
			$data = fread($handle, 10000000);
			fclose($handle);
			$sslReturn = $cfg->uploadSSLCaCert($data);
			if ($sslReturn !== true) {
				$errors[] = $sslReturn;
			}
			else {
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
		if (preg_match('/^([a-zA-Z0-9_\\.-]+)(:([0-9]+))?$/', $_POST['serverurl'], $matches)) {
			$port = '636';
			if (isset($matches[3]) && !empty($matches[3])) {
				$port = $matches[3];
			}
			$pemResult = getLDAPSSLCertificate($matches[1], $port);
			if ($pemResult !== false) {
				$messages[] = _('Imported certificate from server.');
				$messages[] = _('You might need to restart your webserver for changes to take effect.');
				$cfg->uploadSSLCaCert($pemResult);
			}
			else {
				$errors[] = _('Unable to import server certificate. Please use the upload function.');
			}
		}
		else {
			$errors[] = _('Invalid server name. Please enter "server" or "server:port".');
		}
	}
	foreach ($_POST as $key => $value) {
		if (strpos($key, 'deleteCert_') === 0) {
			$index = substr($key, strlen('deleteCert_'));
			$cfg->deleteSSLCaCert($index);
		}
	}
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

?>

		<title>
			<?php
				echo _("Edit general settings");
			?>
		</title>
	<?php 
		// include all CSS files
		$cssDirName = dirname(__FILE__) . '/../../style';
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
			echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/" . $cssEntry . "\">\n";
		}
	?>
		<link rel="shortcut icon" type="image/x-icon" href="../../graphics/favicon.ico">
	</head>
	<body>
		<table border=0 width="100%" class="lamHeader ui-corner-all">
			<tr>
				<td align="left" height="30">
					<a class="lamHeader" href="http://www.ldap-account-manager.org/" target="new_window">&nbsp;<img src="../../graphics/logo32.png" width=24 height=24 class="align-middle" alt="LDAP Account Manager">&nbsp;&nbsp;LDAP Account Manager</a>
				</td>
				<td align="right" height=20>
					<a href="../login.php"><IMG alt="configuration" src="../../graphics/undo.png">&nbsp;<?php echo _("Back to login") ?></a>
				</td>
			</tr>
		</table>
		<br>
		<!-- form for adding/renaming/deleting profiles -->
		<form enctype="multipart/form-data" action="mainmanage.php" method="post">

<?php
// include all JavaScript files
$jsDirName = dirname(__FILE__) . '/../lib';
$jsDir = dir($jsDirName);
$jsFiles = array();
while ($jsEntry = $jsDir->read()) {
	if (substr($jsEntry, strlen($jsEntry) - 3, 3) != '.js') continue;
	$jsFiles[] = $jsEntry;
}
sort($jsFiles);
foreach ($jsFiles as $jsEntry) {
	echo "<script type=\"text/javascript\" src=\"../lib/" . $jsEntry . "\"></script>\n";
}

$container = new htmlTable();

// print messages
for ($i = 0; $i < sizeof($errors); $i++) {
	$container->addElement(new htmlStatusMessage("ERROR", $errors[$i]), true);
}
for ($i = 0; $i < sizeof($messages); $i++) {
	$container->addElement(new htmlStatusMessage("INFO", $messages[$i]), true);
}

// check if config file is writable
if (!$cfg->isWritable()) {
	$container->addElement(new htmlStatusMessage('WARN', 'The config file is not writable.', 'Your changes cannot be saved until you make the file writable for the webserver user.'), true);
}

// security settings
$container->addElement(new htmlSubTitle(_("Security settings")), true);
$securityTable = new htmlTable();
$options = array(5, 10, 20, 30, 60, 90, 120, 240);
$securityTable->addElement(new htmlTableExtendedSelect('sessionTimeout', $options, array($cfg->sessionTimeout), _("Session timeout"), '238'), true);
$securityTable->addElement(new htmlTableExtendedInputTextarea('allowedHosts', implode("\n", explode(",", $cfg->allowedHosts)), '30', '7', _("Allowed hosts"), '241'), true);
// SSL certificate
$securityTable->addElement(new htmlOutputText(_('SSL certificates')));
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
$securityTable->addElement($sslDelSaveGroup);
$securityTable->addElement(new htmlHelpLink('204'));
$securityTable->addElement(new htmlSpacer('250px', null), true);
$securityTable->addElement(new htmlOutputText(''));
$sslButtonTable = new htmlTable();
$sslButtonTable->colspan = 3;
$sslButtonTable->addElement(new htmlInputFileUpload('sslCaCert'));
$sslUploadBtn = new htmlButton('sslCaCertUpload', _('Upload'));
$sslUploadBtn->setIconClass('upButton');
$sslUploadBtn->setTitle(_('Upload CA certificate in DER/PEM format.'));
$sslButtonTable->addElement($sslUploadBtn, true);
if (function_exists('stream_socket_client')) {
	$sslImportGroup = new htmlGroup();
	$sslImportGroup->addElement(new htmlOutputText('ldaps://'));
	$sslImportServerUrl = !empty($_POST['serverurl']) ? $_POST['serverurl'] :  '';
	$sslImportGroup->addElement(new htmlInputField('serverurl'));
	$sslButtonTable->addElement($sslImportGroup);
	$sslImportBtn = new htmlButton('sslCaCertImport', _('Import from server'));
	$sslImportBtn->setIconClass('downButton');
	$sslImportBtn->setTitle(_('Imports the certificate directly from your LDAP server.'));
	$sslImportBtn->setCSSClasses(array('nowrap'));
	$sslButtonTable->addElement($sslImportBtn);
	$sslButtonTable->addElement(new htmlEqualWidth(array('btn_sslCaCertUpload', 'btn_sslCaCertImport')));
}
$securityTable->addElement($sslButtonTable, true);
$sslCerts = $cfg->getSSLCaCertificates();
if (sizeof($sslCerts) > 0) {
	$certTable = new htmlTable();
	$certTable->colspan = 3;
	$certSpace = new htmlSpacer('5px', null);
	$certTable->addElement(new htmlOutputText(''));
	$certTable->addElement(new htmlOutputText(_('Serial number')));
	$certTable->addElement($certSpace);
	$certTable->addElement(new htmlOutputText(_('Valid to')));
	$certTable->addElement($certSpace);
	$certTable->addElement(new htmlOutputText(_('Common name')), true);
	for ($i = 0; $i < sizeof($sslCerts); $i++) {
		$serial = isset($sslCerts[$i]['serialNumber']) ? $sslCerts[$i]['serialNumber'] : '';
		$validTo = isset($sslCerts[$i]['validTo_time_t']) ? $sslCerts[$i]['validTo_time_t'] : '';
		$cn = isset($sslCerts[$i]['subject']['CN']) ? $sslCerts[$i]['subject']['CN'] : '';
		$certTable->addElement(new htmlButton('deleteCert_' . $i, 'delete.png', true));
		$certTable->addElement(new htmlOutputText($serial));
		$certTable->addElement($certSpace);
		$certTable->addElement(new htmlOutputText(formatSSLTimestamp($validTo)));
		$certTable->addElement($certSpace);
		$certTable->addElement(new htmlOutputText($cn), true);
	}
	$securityTable->addElement(new htmlOutputText(''));
	$securityTable->addElement($certTable, true);
}
$container->addElement($securityTable, true);

$container->addElement(new htmlSpacer(null, '10px'), true);

// password policy
$container->addElement(new htmlSubTitle(_("Password policy")), true);
$policyTable = new htmlTable();
$options20 = array(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20);
$options4 = array(0, 1, 2, 3, 4);
$policyTable->addElement(new htmlTableExtendedSelect('passwordMinLength', $options20, array($cfg->passwordMinLength), _('Minimum password length'), '242'), true);
$policyTable->addElement(new htmlTableExtendedSelect('passwordMinLower', $options20, array($cfg->passwordMinLower), _('Minimum lowercase characters'), '242'), true);
$policyTable->addElement(new htmlTableExtendedSelect('passwordMinUpper', $options20, array($cfg->passwordMinUpper), _('Minimum uppercase characters'), '242'), true);
$policyTable->addElement(new htmlTableExtendedSelect('passwordMinNumeric', $options20, array($cfg->passwordMinNumeric), _('Minimum numeric characters'), '242'), true);
$policyTable->addElement(new htmlTableExtendedSelect('passwordMinSymbol', $options20, array($cfg->passwordMinSymbol), _('Minimum symbolic characters'), '242'), true);
$policyTable->addElement(new htmlTableExtendedSelect('passwordMinClasses', $options4, array($cfg->passwordMinClasses), _('Minimum character classes'), '242'), true);
$container->addElement($policyTable, true);
$container->addElement(new htmlSpacer(null, '10px'), true);

// logging
$container->addElement(new htmlSubTitle(_("Logging")), true);
$loggingTable = new htmlTable();
$levelOptions = array(_("Debug") => LOG_DEBUG, _("Notice") => LOG_NOTICE, _("Warning") => LOG_WARNING, _("Error") => LOG_ERR);
$levelSelect = new htmlTableExtendedSelect('logLevel', $levelOptions, array($cfg->logLevel), _("Log level"), '239');
$levelSelect->setHasDescriptiveElements(true);
$loggingTable->addElement($levelSelect, true);
$destinationOptions = array(_("No logging") => "none", _("System logging") => "syslog", _("File") => 'file');
$destinationSelected = 'file';
$destinationPath = $cfg->logDestination;
if ($cfg->logDestination == 'NONE') {
	$destinationSelected = 'none';
	$destinationPath = '';
}
elseif ($cfg->logDestination == 'SYSLOG') {
	$destinationSelected = 'syslog';
	$destinationPath = '';
}
$loggingTable->addElement(new htmlTableExtendedRadio(_("Log destination"), 'logDestination', $destinationOptions, $destinationSelected, '240'), true);
$loggingTable->addElement(new htmlOutputText(''));
$loggingTable->addElement(new htmlInputField('logFile', $destinationPath), true);
$container->addElement($loggingTable, true);
$container->addElement(new htmlSpacer(null, '10px'), true);

// change master password
$container->addElement(new htmlSubTitle(_("Change master password")), true);
$passwordTable = new htmlTable();
$pwd1 = new htmlTableExtendedInputField(_("New master password"), 'masterpassword', '', '235');
$pwd1->setIsPassword(true);
$passwordTable->addElement($pwd1, true);
$pwd2 = new htmlTableExtendedInputField(_("Reenter password"), 'masterpassword2', '');
$pwd2->setIsPassword(true);
$passwordTable->addElement($pwd2, true);
$container->addElement($passwordTable, true);
$container->addElement(new htmlSpacer(null, '20px'), true);

// buttons
if ($cfg->isWritable()) {
	$buttonTable = new htmlTable();
	$buttonTable->addElement(new htmlButton('submit', _("Ok")));
	$buttonTable->addElement(new htmlButton('cancel', _("Cancel")));
	$container->addElement($buttonTable, true);
}

$container->addElement(new htmlHiddenInput('submitFormData', '1'), true);

$tabindex = 1;
$globalFieldset = new htmlFieldset($container, _('General settings'));
parseHtml(null, $globalFieldset, array(), false, $tabindex, 'user');

/**
 * Formats an LDAP time string (e.g. from createTimestamp).
 * 
 * @param String $time LDAP time value
 * @return String formated time
 */
function formatSSLTimestamp($time) {
	$matches = array();
	if (!empty($time)) {
		return date('d.m.Y', $time);
	}
	return '';
}



?>

		</form>
		<p><br></p>

	</body>
</html>

