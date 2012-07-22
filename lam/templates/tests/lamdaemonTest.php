<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2006 - 2012  Roland Gruber

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
* Tests the lamdaemon script.
*
* @author Roland Gruber
* @author Thomas Manninger
* @package tools
*/

/** security functions */
include_once("../../lib/security.inc");
/** access to configuration options */
include_once("../../lib/config.inc");

// start session
startSecureSession();

// die if no write access
if (!checkIfWriteAccessIsAllowed()) die();

checkIfToolIsActive('toolTests');

setlanguage();

include '../main_header.php';
echo "<div class=\"userlist-bright smallPaddingContent\">\n";
echo "<form action=\"lamdaemonTest.php\" method=\"post\">\n";

$container = new htmlTable();
$container->addElement(new htmlTitle(_("Lamdaemon test")), true);

$servers = explode(";", $_SESSION['config']->get_scriptServers());
$serverIDs = array();
$serverTitles = array();
for ($i = 0; $i < sizeof($servers); $i++) {
	$serverParts = explode(":", $servers[$i]);
	$serverName = $serverParts[0];
	$title = $serverName;
	if (isset($serverParts[1])) {
		$title = $serverParts[1] . " (" . $serverName . ")";
	}
	$serverIDs[] = $serverName;
	$serverTitles[$serverName] = $title;
}

if (isset($_POST['runTest'])) {
	lamRunLamdaemonTestSuite($_POST['server'], $serverTitles[$_POST['server']] , isset($_POST['checkQuotas']), $container);
}
else if ((sizeof($servers) > 0) && isset($servers[0]) && ($servers[0] != '')) {
	$container->addElement(new htmlOutputText(_("Server")));
	$serverOptions = array();
	for ($i = 0; $i < sizeof($servers); $i++) {
		$servers[$i] = explode(":", $servers[$i]);
		$serverName = $servers[$i][0];
		$title = $serverName;
		if (isset($servers[$i][1])) {
			$title = $servers[$i][1] . " (" . $serverName . ")";
		}
		$serverOptions[$title] = $serverName;
	}
	$serverSelect = new htmlSelect('server', $serverOptions);
	$serverSelect->setHasDescriptiveElements(true);
	$container->addElement($serverSelect, true);
	
	$container->addElement(new htmlOutputText(_("Check quotas")));
	$container->addElement(new htmlInputCheckbox('checkQuotas', false), true);
	
	$container->addElement(new htmlSpacer(null, '10px'), true);
	
	$okButton = new htmlButton('runTest', _("Ok"));
	$okButton->colspan = 2;
	$container->addElement($okButton);
}
else {
	$container->addElement(new htmlStatusMessage("ERROR", _('No lamdaemon server set, please update your LAM configuration settings.')));
}

$tabindex = 1;
parseHtml(null, $container, array(), false, $tabindex, 'user');

echo "</form>\n";
echo "</div>\n";
include '../main_footer.php';


/**
 * Runs a test case of lamdaemon.
 *
 * @param string $command test command
 * @param boolean $stopTest specifies if test should be run
 * @param connection $handle SSH connection
 * @param string $testText describing text
 * @param htmlTable $container container for HTML output
 * @return boolean true, if errors occured
 */
function lamTestLamdaemon($command, $stopTest, $handle, $testText, $container) {
	$okImage = "../../graphics/pass.png";
	$failImage = "../../graphics/fail.png";
	$spacer = new htmlSpacer('10px', null);
	// run lamdaemon and get user quotas
	if (!$stopTest) {
		$container->addElement(new htmlOutputText($testText));
		$container->addElement($spacer);
		flush();
		$lamdaemonOk = false;
		$output = $handle->exec("sudo " . $_SESSION['config']->get_scriptPath() . ' ' . escapeshellarg($command));
		if ((stripos(strtolower($output), "error") === false) && ((strpos($output, 'INFO,') === 0) || (strpos($output, 'QUOTA_ENTRY') === 0))) {
			$lamdaemonOk = true;
		}
		if ($lamdaemonOk) {
			$container->addElement(new htmlImage($okImage));
			$container->addElement($spacer);
			$container->addElement(new htmlOutputText(_("Lamdaemon successfully run.")), true);
		}
		else {
			$container->addElement(new htmlImage($failImage));
			$container->addElement($spacer);
			if (!(strpos($output, 'ERROR,') === 0) && !(strpos($output, 'WARN,') === 0)) {
				// error messages from console (e.g. sudo)
				$container->addElement(new htmlStatusMessage('ERROR', $output), true);
			}
			else {
				// error messages from lamdaemon
				$parts = explode(",", $output);
				if (sizeof($parts) == 2) {
					$container->addElement(new htmlStatusMessage($parts[0], $parts[1]), true);
				}
				elseif (sizeof($parts) == 3) {
					$container->addElement(new htmlStatusMessage($parts[0], $parts[1], $parts[2]), true);
				}
				else {
					$container->addElement(new htmlOutputText($output), true);
				}
			}
			$stopTest = true;
		}
	}
	flush();
	return $stopTest;
}

/**
 * Runs all tests for a given server.
 *
 * @param String $serverName server ID
 * @param String $serverTitle server name
 * @param boolean $testQuota true, if Quotas should be checked
 * @param htmlTable $container container for HTML output
 */
function lamRunLamdaemonTestSuite($serverName, $serverTitle, $testQuota, $container) {
	$SPLIT_DELIMITER = "###x##y##x###";
	$LAMDAEMON_PROTOCOL_VERSION = '2';
	$okImage = "../../graphics/pass.png";
	$failImage = "../../graphics/fail.png";
	
	flush();
	$stopTest = false;
	$spacer = new htmlSpacer('10px', null);

	$container->addElement(new htmlSubTitle($serverTitle), true);

	// check script server and path
	$container->addElement(new htmlOutputText(_("Lamdaemon server and path")));
	$container->addElement(new htmlSpacer('10px', null));
	if (!isset($serverName) || (strlen($serverName) < 3)) {
		$container->addElement(new htmlImage($failImage));
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText(_("No lamdaemon server set, please update your LAM configuration settings.")), true);
	}
	elseif (($_SESSION['config']->get_scriptPath() == null) || (strlen($_SESSION['config']->get_scriptPath()) < 10)) {
		$container->addElement(new htmlImage($failImage));
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText(_("No lamdaemon path set, please update your LAM configuration settings.")), true);
		$stopTest = true;
	}
	else {
		$container->addElement(new htmlImage($okImage));
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText(sprintf(_("Using %s as lamdaemon remote server."), $serverName)), true);
	}

	flush();

	// check Unix account of LAM admin
	if (!$stopTest) {
		$container->addElement(new htmlOutputText(_("Unix account")));
		$container->addElement($spacer);
		$credentials = $_SESSION['ldap']->decrypt_login();
		$unixOk = false;
		$sr = @ldap_read($_SESSION['ldap']->server(), $credentials[0], "objectClass=posixAccount", array('uid'));
		if ($sr) {
			$entry = @ldap_get_entries($_SESSION['ldap']->server(), $sr);
			$userName = $entry[0]['uid'][0];
			if ($userName) {
				$unixOk = true;
			}
		}
		if ($unixOk) {
			$container->addElement(new htmlImage($okImage));
			$container->addElement($spacer);
			$container->addElement(new htmlOutputText(sprintf(_("Using %s to connect to remote server."), $userName)), true);
		}
		else {
			$container->addElement(new htmlImage($failImage));
			$container->addElement($spacer);
			$container->addElement(new htmlOutputText(sprintf(_("Your LAM admin user (%s) must be a valid Unix account to work with lamdaemon!"), $credentials[0])), true);
			$stopTest = true;
		}
	}

	flush();

	// check SSH login
	if (!$stopTest) {
		$container->addElement(new htmlOutputText(_("SSH connection")));
		$container->addElement($spacer);
		flush();
		$sshOk = false;
		$handle = lamTestConnectSSH($serverName);
		if ($handle) {
			if ($handle->login($userName, $credentials[1])) {
				$sshOk = true;
			}
		}
		if ($sshOk) {
			$container->addElement(new htmlImage($okImage));
			$container->addElement($spacer);
			$container->addElement(new htmlOutputText(_("SSH connection could be established.")), true);
		}
		else {
			$container->addElement(new htmlImage($failImage));
			$container->addElement($spacer);
			$container->addElement(new htmlOutputText(_("Unable to connect to remote server!")), true);
			$stopTest = true;
		}
	}

	flush();
	
	if (!$stopTest) {
		$stopTest = lamTestLamdaemon("+" . $SPLIT_DELIMITER . "test" . $SPLIT_DELIMITER . "basic", $stopTest, $handle, _("Execute lamdaemon"), $container);
	}
	
	if (!$stopTest) {
		$stopTest = lamTestLamdaemon("+" . $SPLIT_DELIMITER . "test" . $SPLIT_DELIMITER . "version" . $SPLIT_DELIMITER . $LAMDAEMON_PROTOCOL_VERSION, $stopTest, $handle, _("Lamdaemon version"), $container);
	}
	
	if (!$stopTest) {
		$handle = lamTestConnectSSH($serverName);
		@$handle->login($userName, $credentials[1]);
		$stopTest = lamTestLamdaemon("+" . $SPLIT_DELIMITER . "test" . $SPLIT_DELIMITER . "nss" . $SPLIT_DELIMITER . "$userName", $stopTest, $handle, _("Lamdaemon: check NSS LDAP"), $container);
		if (!$stopTest && $testQuota) {
			$handle = lamTestConnectSSH($serverName);
			@$handle->login($userName, $credentials[1]);
			$stopTest = lamTestLamdaemon("+" . $SPLIT_DELIMITER . "test" . $SPLIT_DELIMITER . "quota", $stopTest, $handle, _("Lamdaemon: Quota module installed"), $container);
			$handle = lamTestConnectSSH($serverName);
			@$handle->login($userName, $credentials[1]);
			$stopTest = lamTestLamdaemon("+" . $SPLIT_DELIMITER . "quota" . $SPLIT_DELIMITER . "get" . $SPLIT_DELIMITER . "user", $stopTest, $handle, _("Lamdaemon: read quotas"), $container);
		}
	}

	$container->addElement(new htmlSpacer(null, '10px'), true);
	$endMessage = new htmlOutputText(_("Lamdaemon test finished."));
	$endMessage->colspan = 5;
	$container->addElement($endMessage);
}

/**
 * Connects to the given SSH server.
 *
 * @param String $server server name (e.g. localhost or localhost,1234)
 * @return object handle
 */
function lamTestConnectSSH($server) {
	// add phpseclib to include path
	set_include_path(get_include_path() . PATH_SEPARATOR . dirname(__FILE__) . '/../../lib/3rdParty/phpseclib');
	include_once('Net/SSH2.php');
	$serverNameParts = explode(",", $server);
	if (sizeof($serverNameParts) > 1) {
		return @new Net_SSH2($serverNameParts[0], $serverNameParts[1]);
	}
	else {
		return @new Net_SSH2($server);
	}
}

?>
