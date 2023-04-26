<?php
namespace LAM\CONFIG;

use htmlJavaScript;
use htmlResponsiveTable;
use LAM\LOGIN\WEBAUTHN\WebauthnManager;
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
use LAMException;
use PDO;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2023  Roland Gruber

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
if (isFileBasedSession()) {
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

if (isset($_SESSION['header'])) {
	echo $_SESSION['header'];
}
printHeaderContents(_("Edit general settings"), '../..');
?>
</head>
<body>
<div id="lam-topnav" class="lam-header">
    <div class="lam-header-left lam-menu-stay">
        <a href="https://www.ldap-account-manager.org/" target="new_window">
            <img class="align-middle" width="24" height="24" alt="help" src="../../graphics/logo24.png">
            <span class="hide-on-mobile">
                        <?php
						echo getLAMVersionText();
						?>
                    </span>
        </a>
    </div>
	<?php
	if (is_dir(dirname(__FILE__) . '/../../docs/manual')) {
		?>
        <a class="lam-header-right lam-menu-icon hide-on-tablet" href="javascript:void(0);" class="icon" onclick="window.lam.topmenu.toggle();">
            <img class="align-middle" width="16" height="16" alt="menu" src="../../graphics/menu.svg">
            <span class="padding0">&nbsp;</span>
        </a>
        <a class="lam-header-right lam-menu-entry" target="_blank" href="../../docs/manual/index.html">
            <span class="padding0"><?php echo _("Help") ?></span>
        </a>
		<?php
	}
	?>
</div>
<br>
<?php
// include all JavaScript files
printJsIncludes('../..');

$errors = array();
$messages = array();
// check if submit button was pressed
if (isset($_POST['submitFormData'])) {
    if (extension_loaded('PDO')) {
	    // set database
	    $cfg->configDatabaseType = $_POST['configDatabaseType'];
	    $cfg->configDatabaseServer = $_POST['configDatabaseServer'];
	    $cfg->configDatabasePort = $_POST['configDatabasePort'];
	    $cfg->configDatabaseName = $_POST['configDatabaseName'];
	    $cfg->configDatabaseUser = $_POST['configDatabaseUser'];
	    $cfg->configDatabasePassword = $_POST['configDatabasePassword'];
	    if ($cfg->configDatabaseType === LAMCfgMain::DATABASE_MYSQL) {
		    if (empty($cfg->configDatabaseServer) || !get_preg($cfg->configDatabaseServer, 'hostname')) {
			    $errors[] = _('Please enter a valid database host name.');
		    }
		    if (empty($cfg->configDatabaseName)) {
			    $errors[] = _('Please enter a valid database name.');
		    }
		    if (empty($cfg->configDatabaseUser)) {
			    $errors[] = _('Please enter a valid database user.');
		    }
		    if (empty($cfg->configDatabasePassword)) {
			    $errors[] = _('Please enter a valid database password.');
		    }
	    }
    }
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
		$cfg->licenseWarningType = $_POST['licenseWarningType'];
		$cfg->licenseEmailFrom = $_POST['licenseEmailFrom'];
		$cfg->licenseEmailTo = $_POST['licenseEmailTo'];
		if ((($cfg->licenseWarningType === LAMCfgMain::LICENSE_WARNING_EMAIL) || ($cfg->licenseWarningType === LAMCfgMain::LICENSE_WARNING_ALL))
            && !get_preg($cfg->licenseEmailFrom, 'email')) {
		    $errors[] = _('Licence') . ': ' . _('From address') . ' - ' . _('Please enter a valid email address!');
        }
		if (($cfg->licenseWarningType === LAMCfgMain::LICENSE_WARNING_EMAIL) || ($cfg->licenseWarningType === LAMCfgMain::LICENSE_WARNING_ALL)) {
		    $toEmails = preg_split('/;[ ]*/', $cfg->licenseEmailTo);
		    if ($toEmails !== false) {
				foreach ($toEmails as $toEmail) {
					if (!get_preg($toEmail, 'email')) {
						$errors[] = _('Licence') . ': ' . _('TO address') . ' - ' . _('Please enter a valid email address!');
						break;
					}
				}
            }
		}
	}
	// set session timeout
	$cfg->sessionTimeout = $_POST['sessionTimeout'];
	// set hide login error details
	$cfg->hideLoginErrorDetails = (isset($_POST['hideLoginErrorDetails']) && ($_POST['hideLoginErrorDetails'] === 'on')) ? 'true' : 'false';
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
			if (!preg_match($ipRegex, $allowedHostsList[$i]) || (strlen($allowedHostsList[$i]) > 45)) {
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
			$allowedHostsSelfServiceList = explode("\r\n", $allowedHostsSelfService);
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
			$allowedHostsSelfServiceList = array_unique($allowedHostsSelfServiceList);
			$allowedHostsSelfService = implode(",", $allowedHostsSelfServiceList);
		} else {
			$allowedHostsSelfService = "";
		}
		$cfg->allowedHostsSelfService = $allowedHostsSelfService;
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
		}
		else {
			$handle = fopen($_FILES['sslCaCert']['tmp_name'], "r");
			if ($handle === false) {
				$errors[] = _('Unable to create temporary file.');
			}
			else {
				$data = fread($handle, 10000000);
				if ($data === false) {
					$errors[] = _('Unable to create temporary file.');
				}
				else {
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
		$cfg->mailUser = $_POST['mailUser'];
		$cfg->mailPassword = $_POST['mailPassword'];
		$cfg->mailEncryption = $_POST['mailEncryption'];
		$cfg->mailServer = $_POST['mailServer'];
		if (!empty($cfg->mailServer) && !get_preg($cfg->mailServer, 'hostAndPort')) {
            $errors[] = _('Please enter the mail server with host name and port.');
        }
	}
	$cfg->errorReporting = $_POST['errorReporting'];
	// save settings
	if (isset($_POST['submit'])) {
		$cfg->save();
		if (sizeof($errors) == 0) {
			$scriptTag = new htmlJavaScript('window.lam.dialog.showSuccessMessageAndRedirect("' . _("Your settings were successfully saved.") . '", "", "' . _('Ok') . '", "../login.php")');
			$tabIndex = 0;
			parseHtml(null, $scriptTag, array(), false, $tabIndex, null);
			echo '</body></html>';
			exit();
		}
	}
}

?>
<form class="text-center" enctype="multipart/form-data" action="mainmanage.php" method="post" novalidate="novalidate">

	<?php
	$tabindex = 1;

	$row = new htmlResponsiveRow();
	$row->add(new htmlTitle(_('General settings')), 12);

	// print messages
	foreach ($errors as $error) {
		$row->add(new htmlStatusMessage("ERROR", $error), 12);
	}
	foreach ($messages as $message) {
		$row->add(new htmlStatusMessage("INFO", $message), 12);
	}

	// check if config file is writable
	if (!$cfg->isWritable()) {
		$row->add(new htmlStatusMessage('WARN', _('The config file is not writable.'), _('Your changes cannot be saved until you make the file writable for the webserver user.')), 12);
	}

	// database
	if (extension_loaded('PDO')) {
		$row->add(new htmlSubTitle(_('Configuration storage')), 12);
		$storageProviders = array(
			_('Local file system') => LAMCfgMain::DATABASE_FILE_SYSTEM
		);
		if (in_array('mysql', PDO::getAvailableDrivers())) {
			$storageProviders['MySQL'] = LAMCfgMain::DATABASE_MYSQL;
		}
		$storageProviderSelect = new htmlResponsiveSelect('configDatabaseType', $storageProviders, array($cfg->configDatabaseType), _('Database type'), '293');
		$storageProviderSelect->setHasDescriptiveElements(true);
		$dbRowsToShow = array(
		    LAMCfgMain::DATABASE_FILE_SYSTEM => array(),
            LAMCfgMain::DATABASE_MYSQL => array('configDatabaseServer', 'configDatabasePort', 'configDatabaseName', 'configDatabaseUser', 'configDatabasePassword')
        );
		$storageProviderSelect->setTableRowsToShow($dbRowsToShow);
		$dbRowsToHide = array(
			LAMCfgMain::DATABASE_FILE_SYSTEM => array('configDatabaseServer', 'configDatabasePort', 'configDatabaseName', 'configDatabaseUser', 'configDatabasePassword'),
			LAMCfgMain::DATABASE_MYSQL => array()
        );
		$storageProviderSelect->setTableRowsToHide($dbRowsToHide);
		$row->add($storageProviderSelect, 12);
		$dbHost = new htmlResponsiveInputField(_('Database host'), 'configDatabaseServer', $cfg->configDatabaseServer, '273');
		$dbHost->setRequired(true);
		$row->add($dbHost, 12);
		$dbPort = new htmlResponsiveInputField(_('Database port'), 'configDatabasePort', $cfg->configDatabasePort, '274');
		$row->add($dbPort, 12);
		$dbName = new htmlResponsiveInputField(_('Database name'), 'configDatabaseName', $cfg->configDatabaseName, '276');
		$dbName->setRequired(true);
		$row->add($dbName, 12);
		$dbUser = new htmlResponsiveInputField(_('Database user'), 'configDatabaseUser', $cfg->configDatabaseUser, '275');
		$dbUser->setRequired(true);
		$row->add($dbUser, 12);
		$dbPassword = new htmlResponsiveInputField(_('Database password'), 'configDatabasePassword', deobfuscateText($cfg->configDatabasePassword), '275');
		$dbPassword->setRequired(true);
		$dbPassword->setIsPassword(true);
		$row->add($dbPassword, 12);
    }

	// license
	if (isLAMProVersion()) {
		$row->add(new htmlSubTitle(_('Licence')), 12);
		$row->add(new htmlResponsiveInputTextarea('license', implode("\n", $cfg->getLicenseLines()), '30', '10', _('Licence'), '287'), 12);
		$warningOptions = array(
	        _('Screen') => LAMCfgMain::LICENSE_WARNING_SCREEN,
			_('Email') => LAMCfgMain::LICENSE_WARNING_EMAIL,
			_('Both') => LAMCfgMain::LICENSE_WARNING_ALL,
			_('None') => LAMCfgMain::LICENSE_WARNING_NONE,
        );
		$warningTypeSelect = new htmlResponsiveSelect('licenseWarningType', $warningOptions, array($cfg->getLicenseWarningType()), _('Expiration warning'), '288');
		$warningTypeSelect->setHasDescriptiveElements(true);
		$warningTypeSelect->setSortElements(false);
		$warningTypeSelect->setTableRowsToHide(array(
			LAMCfgMain::LICENSE_WARNING_SCREEN => array('licenseEmailFrom', 'licenseEmailTo'),
			LAMCfgMain::LICENSE_WARNING_NONE => array('licenseEmailFrom', 'licenseEmailTo'),
        ));
		$warningTypeSelect->setTableRowsToShow(array(
			LAMCfgMain::LICENSE_WARNING_EMAIL => array('licenseEmailFrom', 'licenseEmailTo'),
			LAMCfgMain::LICENSE_WARNING_ALL => array('licenseEmailFrom', 'licenseEmailTo'),
		));
		$row->add($warningTypeSelect, 12);
		$licenseFrom = new htmlResponsiveInputField(_('From address'), 'licenseEmailFrom', $cfg->licenseEmailFrom, '289');
		$licenseFrom->setRequired(true);
		$row->add($licenseFrom, 12);
		$licenseTo = new htmlResponsiveInputField(_('TO address'), 'licenseEmailTo', $cfg->licenseEmailTo, '290');
		$licenseTo->setRequired(true);
		$row->add($licenseTo, 12);

		$row->add(new htmlSpacer(null, '1rem'), true);
	}

	// security settings
	$row->add(new htmlSubTitle(_("Security settings")), 12);
	$options = array(5, 10, 20, 30, 60, 90, 120, 240);
	$row->add(new htmlResponsiveSelect('sessionTimeout', $options, array($cfg->sessionTimeout), _("Session timeout"), '238'));
	$hideLoginErrorDetails = ($cfg->hideLoginErrorDetails === 'true');
	$row->add(new htmlResponsiveInputCheckbox('hideLoginErrorDetails', $hideLoginErrorDetails, _('Hide LDAP details on failed login'), '257'));
	$row->add(new htmlResponsiveInputTextarea('allowedHosts', implode("\n", explode(",", $cfg->allowedHosts)), '30', '7', _("Allowed hosts"), '241'));
	if (isLAMProVersion()) {
		$row->add(new htmlResponsiveInputTextarea('allowedHostsSelfService', implode("\n", explode(",", $cfg->allowedHostsSelfService)), '30', '7', _("Allowed hosts (self service)"), '241'));
	}
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
		$sslDownloadBtn = new htmlLink('', '../../tmp/' . $sslFileName, '../../graphics/save.svg');
		$sslDownloadBtn->setTargetWindow('_blank');
		$sslDownloadBtn->setTitle(_('Download CA certificates'));
		$sslDownloadBtn->setCSSClasses(array('icon'));
		$sslDelSaveGroup->addElement($sslDownloadBtn);
		$sslDeleteBtn = new htmlButton('sslCaCertDelete', 'del.svg', true);
		$sslDeleteBtn->setTitle(_('Delete all CA certificates'));
		$sslDelSaveGroup->addElement($sslDeleteBtn);
	}
	$sslDelSaveGroup->addElement(new htmlHelpLink('204'));
	$row->addField($sslDelSaveGroup);
	$row->addLabel(new htmlInputFileUpload('sslCaCert'));
	$sslUploadBtn = new htmlButton('sslCaCertUpload', _('Upload'));
	$sslUploadBtn->setTitle(_('Upload CA certificate in DER/PEM format.'));
	$row->addField($sslUploadBtn);
	if (function_exists('stream_socket_client') && function_exists('stream_context_get_params')) {
		$sslImportServerUrl = !empty($_POST['serverurl']) ? $_POST['serverurl'] : 'ldaps://';
		$serverUrlUpload = new htmlInputField('serverurl', $sslImportServerUrl);
		$row->addLabel($serverUrlUpload);
		$sslImportBtn = new htmlButton('sslCaCertImport', _('Import from server'));
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
			if (get_preg($validTo, 'digit')) {
			    $date = DateTime::createFromFormat('U', $validTo, new DateTimeZone('UTC'));
			    if ($date !== false) {
					$validTo = $date->format('Y-m-d');
                }
            }
			$cn = isset($sslCerts[$i]['subject']['CN']) ? $sslCerts[$i]['subject']['CN'] : '';
			$delBtn = new htmlButton('deleteCert_' . $i, 'del.svg', true);
			$certsData[] = array(
				new htmlOutputText($cn),
				new htmlDiv(null, new htmlOutputText($validTo), array('nowrap')),
				new htmlOutputText($serial),
				$delBtn
			);
		}
		$certsTable = new htmlResponsiveTable($certsTitles, $certsData);
		$certsTable->setCSSClasses(array('text-left'));
		$row->add($certsTable, 12);
	}

	// password policy
	$row->add(new htmlSubTitle(_("Password policy")), 12);
	$optionsPwdLength = array();
	for ($i = 0; $i <= 50; $i++) {
		$optionsPwdLength[] = $i;
	}
	$options4 = array(0, 1, 2, 3, 4);
	$row->add(new htmlResponsiveSelect('passwordMinLength', $optionsPwdLength, array($cfg->passwordMinLength), _('Minimum password length'), '242'), 12);
	$row->addVerticalSpacer('1rem');
	$row->add(new htmlResponsiveSelect('passwordMinLower', $optionsPwdLength, array($cfg->passwordMinLower), _('Minimum lowercase characters'), '242'), 12);
	$row->add(new htmlResponsiveSelect('passwordMinUpper', $optionsPwdLength, array($cfg->passwordMinUpper), _('Minimum uppercase characters'), '242'), 12);
	$row->add(new htmlResponsiveSelect('passwordMinNumeric', $optionsPwdLength, array($cfg->passwordMinNumeric), _('Minimum numeric characters'), '242'), 12);
	$row->add(new htmlResponsiveSelect('passwordMinSymbol', $optionsPwdLength, array($cfg->passwordMinSymbol), _('Minimum symbolic characters'), '242'), 12);
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
	$row->add($errorLogSelect);

	// mail options
	if (isLAMProVersion()) {
		$row->add(new htmlSubTitle(_('Mail options')), 12);
		$mailServer = new htmlResponsiveInputField(_("Mail server"), 'mailServer', $cfg->mailServer, '253');
		$row->add($mailServer);
		$mailUser = new htmlResponsiveInputField(_("User name"), 'mailUser', $cfg->mailUser, '254');
		$row->add($mailUser);
		$mailPassword = new htmlResponsiveInputField(_("Password"), 'mailPassword', $cfg->mailPassword, '255');
		$mailPassword->setIsPassword(true);
		$row->add($mailPassword);
		$mailEncryptionOptions = array(
	        'TLS' => LAMCfgMain::SMTP_TLS,
			'SSL' => LAMCfgMain::SMTP_SSL,
			_('None') => LAMCfgMain::SMTP_NONE,
        );
		$selectedMailEncryption = empty($cfg->mailEncryption) ? LAMCfgMain::SMTP_TLS : $cfg->mailEncryption;
		$mailEncryptionSelect = new htmlResponsiveSelect('mailEncryption', $mailEncryptionOptions, array($selectedMailEncryption), _('Encryption protocol'), '256');
		$mailEncryptionSelect->setHasDescriptiveElements(true);
		$row->add($mailEncryptionSelect);
		addSecurityTokenToSession(false);
		$mailTestButton = new htmlButton('testSmtp', _('Test settings'));
		$mailTestButton->setOnClick("window.lam.smtp.test(event, '" . getSecurityTokenName()
            . "', '" . getSecurityTokenValue() . "', '" . _('Ok') . "')");
		$row->addLabel(new htmlOutputText("&nbsp;", false));
		$row->addField($mailTestButton);
	}

	// webauthn management
	if (extension_loaded('PDO')
		&& in_array('sqlite', \PDO::getAvailableDrivers())) {
		include_once __DIR__ . '/../../lib/webauthn.inc';
		$webAuthnManager = new WebauthnManager();
		try {
			$database = $webAuthnManager->getDatabase();
			if ($database->hasRegisteredCredentials()) {
				$row->add(new htmlSubTitle(_('WebAuthn devices')));
				$webauthnSearchField = new htmlResponsiveInputField(_('User DN'), 'webauthn_searchTerm', null, '252');
				$row->add($webauthnSearchField, 12);
				$row->addVerticalSpacer('0.5rem');
				$row->add(new htmlButton('webauthn_search', _('Search')), 12, 12, 12, 'text-center');
				$resultDiv = new htmlDiv('webauthn_results', new htmlOutputText(''), array('lam-webauthn-results', 'text-left'));
				addSecurityTokenToSession(false);
				$resultDiv->addDataAttribute('sec_token_value', getSecurityTokenValue());
				$row->add($resultDiv);
				$confirmationDiv = new htmlDiv('webauthnDeleteConfirm', new htmlOutputText(_('Do you really want to remove this device?')), array('hidden'));
				$row->add($confirmationDiv);
			}
		}
		catch (LAMException $e) {
		    logNewMessage(LOG_ERR, 'Webauthn error: ' . $e->getTitle() . ' ' . $e->getMessage());
		    $row->add(new htmlStatusMessage('ERROR', $e->getTitle()));
        }
	}

	// change master password
	$row->add(new htmlSubTitle(_("Change master password")));
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
		$saveButton = new htmlButton('submit', _("Save"));
		$saveButton->setCSSClasses(array('lam-primary'));
		$buttonTable->addElement($saveButton);
		$buttonTable->addElement(new htmlSpacer('0.5rem', null));
		$buttonTable->addElement(new htmlButton('cancel', _("Cancel")));
		$row->add($buttonTable, 12);
		$row->add(new htmlHiddenInput('submitFormData', '1'), 12);
	}

	$box = new htmlDiv(null, $row);
	$box->setCSSClasses(array('roundedShadowBox'));
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

