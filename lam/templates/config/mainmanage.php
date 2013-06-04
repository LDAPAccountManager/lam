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

$cfg = new LAMCfgMain();

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
// check if submit button was pressed
if (isset($_POST['submit'])) {
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
	// save settings
	$cfg->save();
	if (sizeof($errors) == 0) {
		metaRefresh('../login.php?confMainSavedOk=1');
		exit();
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
		<form action="mainmanage.php" method="post">

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

// check if config file is writable
if (!$cfg->isWritable()) {
	$container->addElement(new htmlStatusMessage('WARN', 'The config file is not writable.', 'Your changes cannot be saved until you make the file writable for the webserver user.'), true);
}
$container->addElement(new htmlSpacer(null, '20px'), true);

// security settings
$securityTable = new htmlTable();
$options = array(5, 10, 20, 30, 60, 90, 120, 240);
$securityTable->addElement(new htmlTableExtendedSelect('sessionTimeout', $options, array($cfg->sessionTimeout), _("Session timeout"), '238'), true);
$securityTable->addElement(new htmlTableExtendedInputTextarea('allowedHosts', implode("\n", explode(",", $cfg->allowedHosts)), '30', '7', _("Allowed hosts"), '241'), true);
$securityField = new htmlFieldset($securityTable, _("Security settings"));
$container->addElement($securityField, true);
$container->addElement(new htmlSpacer(null, '10px'), true);

// password policy
$policyTable = new htmlTable();
$options20 = array(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20);
$options4 = array(0, 1, 2, 3, 4);
$policyTable->addElement(new htmlTableExtendedSelect('passwordMinLength', $options20, array($cfg->passwordMinLength), _('Minimum password length'), '242'), true);
$policyTable->addElement(new htmlTableExtendedSelect('passwordMinLower', $options20, array($cfg->passwordMinLower), _('Minimum lowercase characters'), '242'), true);
$policyTable->addElement(new htmlTableExtendedSelect('passwordMinUpper', $options20, array($cfg->passwordMinUpper), _('Minimum uppercase characters'), '242'), true);
$policyTable->addElement(new htmlTableExtendedSelect('passwordMinNumeric', $options20, array($cfg->passwordMinNumeric), _('Minimum numeric characters'), '242'), true);
$policyTable->addElement(new htmlTableExtendedSelect('passwordMinSymbol', $options20, array($cfg->passwordMinSymbol), _('Minimum symbolic characters'), '242'), true);
$policyTable->addElement(new htmlTableExtendedSelect('passwordMinClasses', $options4, array($cfg->passwordMinClasses), _('Minimum character classes'), '242'), true);
$policyField = new htmlFieldset($policyTable, _("Password policy"));
$container->addElement($policyField, true);
$container->addElement(new htmlSpacer(null, '10px'), true);

// logging
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
$loggingField = new htmlFieldset($loggingTable, _("Logging"));
$container->addElement($loggingField, true);
$container->addElement(new htmlSpacer(null, '10px'), true);

// change master password
$passwordTable = new htmlTable();
$pwd1 = new htmlTableExtendedInputField(_("New master password"), 'masterpassword', '', '235');
$pwd1->setIsPassword(true);
$passwordTable->addElement($pwd1, true);
$pwd2 = new htmlTableExtendedInputField(_("Reenter password"), 'masterpassword2', '');
$pwd2->setIsPassword(true);
$passwordTable->addElement($pwd2, true);
$passwordField = new htmlFieldset($passwordTable, _("Change master password"));
$container->addElement($passwordField, true);
$container->addElement(new htmlSpacer(null, '20px'), true);

// buttons
if ($cfg->isWritable()) {
	$buttonTable = new htmlTable();
	$buttonTable->addElement(new htmlButton('submit', _("Ok")));
	$buttonTable->addElement(new htmlButton('cancel', _("Cancel")));
	$container->addElement($buttonTable);
}

$tabindex = 1;
parseHtml(null, $container, array(), false, $tabindex, 'user');
?>

		</form>
		<p><br></p>

	</body>
</html>

