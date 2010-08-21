<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2006 - 2010  Roland Gruber

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

setlanguage();

include '../main_header.php';

echo "<h1>" . _("Lamdaemon test") . "</h1>\n";

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
	lamRunLamdaemonTestSuite($_POST['server'], $serverTitles[$_POST['server']] , isset($_POST['checkQuotas']));
}
else if ((sizeof($servers) > 0) && isset($servers[0]) && ($servers[0] != '')) {
	echo "<form action=\"lamdaemonTest.php\" method=\"post\">\n";
	echo "<fieldset class=\"useredit\"><legend><b>" . _("Lamdaemon test") . "</b></legend><br>\n";
	echo "<table>\n";
	echo "<tr>\n";
		echo "<td>\n";
			echo _("Server");
		echo "</td>\n";
		echo "<td>\n";
			echo "<select name=\"server\">\n";
				for ($i = 0; $i < sizeof($servers); $i++) {
					$servers[$i] = explode(":", $servers[$i]);
					$serverName = $servers[$i][0];
					$title = $serverName;
					if (isset($servers[$i][1])) {
						$title = $servers[$i][1] . " (" . $serverName . ")";
					}
					echo "<option value=\"$serverName\">$title</option>\n";
				}
			echo "</select>\n";
		echo "</td>\n";
	echo "</tr>\n";
	echo "<tr>\n";
		echo "<td>\n";
			echo _("Check quotas");
		echo "</td>\n";
		echo "<td>\n";
			echo "<input type=\"checkbox\" name=\"checkQuotas\">\n";
		echo "</td>\n";
	echo "</tr>\n";
	echo "</table>\n";
	
	echo "<br>";
	
	echo "<input type=\"submit\" name=\"runTest\" value=\"" . _("Ok") . "\">\n";
	echo "</fieldset>\n";
	echo "</form>\n";
}
else {
	StatusMessage("ERROR", _('No lamdaemon server set, please update your LAM configuration settings.'));
}


include '../main_footer.php';


/**
 * Runs a test case of lamdaemon.
 *
 * @param string $command test command
 * @param boolean $stopTest specifies if test should be run
 * @param connection $handle SSH connection
 * @param string $testText describing text
 * @return boolean true, if errors occured
 */
function lamTestLamdaemon($command, $stopTest, $handle, $testText) {
	$okImage = "<img width=16 height=16 src=\"../../graphics/pass.png\" alt=\"\">\n";
	$failImage = "<img width=16 height=16 src=\"../../graphics/fail.png\" alt=\"\">\n";
	// run lamdaemon and get user quotas
	if (!$stopTest) {
		echo "<tr class=\"userlist-bright\">\n<td nowrap>" . $testText . "&nbsp;&nbsp;</td>\n";
		flush();
		$lamdaemonOk = false;
		$output = $handle->exec("sudo " . $_SESSION['config']->get_scriptPath() . ' ' . escapeshellarg($command));
		if ((stripos(strtolower($output), "error") === false) && ((strpos($output, 'INFO,') === 0) || (strpos($output, 'QUOTA_ENTRY') === 0))) {
			$lamdaemonOk = true;
		}
		if ($lamdaemonOk) {
			echo "<td nowrap>" . $okImage . "&nbsp;&nbsp;</td>";
			echo "<td>" . _("Lamdaemon successfully run.") . "</td>";
		}
		else {
			echo "<td nowrap>" . $failImage . "&nbsp;&nbsp;</td>\n";
			echo "<td>\n";
			if (!(strpos($output, 'ERROR,') === 0) && !(strpos($output, 'WARN,') === 0)) {
				// error messages from console (e.g. sudo)
				StatusMessage('ERROR', $output);
			}
			else {
				// error messages from lamdaemon
				call_user_func_array('StatusMessage', explode(",", $output));
			}
			echo "</td>\n";
			$stopTest = true;
		}
		echo "</tr>\n";
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
 */
function lamRunLamdaemonTestSuite($serverName, $serverTitle, $testQuota) {
	$SPLIT_DELIMITER = "###x##y##x###";
	$okImage = "<img width=16 height=16 src=\"../../graphics/pass.png\" alt=\"\">\n";
	$failImage = "<img width=16 height=16 src=\"../../graphics/fail.png\" alt=\"\">\n";
	
	echo "<table class=\"userlist\" rules=\"none\">\n";

	flush();
	$stopTest = false;

	echo "<tr class=\"userlist-bright\">\n<td colspan=\"3\" align=\"center\"><b>$serverTitle</b>\n</td>\n</tr>";

	// check script server and path
	echo "<tr class=\"userlist-bright\">\n<td nowrap>" . _("Lamdaemon server and path") . "&nbsp;&nbsp;</td>\n";
	if (!isset($serverName) || (strlen($serverName) < 3)) {
		echo "<td>" . $failImage . "</td>\n";
		echo "<td>" . _("No lamdaemon server set, please update your LAM configuration settings.") . "</td>";
	}
	elseif (($_SESSION['config']->get_scriptPath() == null) || (strlen($_SESSION['config']->get_scriptPath()) < 10)) {
		echo "<td nowrap>" . $failImage . "&nbsp;&nbsp;</td>\n";
		echo "<td>" . _("No lamdaemon path set, please update your LAM configuration settings.") . "</td>";
		$stopTest = true;
	}
	else {
		echo "<td nowrap>" . $okImage . "&nbsp;&nbsp;</td>\n";
		echo "<td>" . sprintf(_("Using %s as lamdaemon remote server."), $serverName) . "</td>";
	}
	echo "</tr>\n";

	flush();

	// check Unix account of LAM admin
	if (!$stopTest) {
		echo "<tr class=\"userlist-bright\">\n<td nowrap>" . _("Unix account") . "&nbsp;&nbsp;</td>\n";
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
			echo "<td nowrap>" . $okImage . "&nbsp;&nbsp;</td>\n";
			echo "<td>" . sprintf(_("Using %s to connect to remote server."), $userName) . "</td>";
		}
		else {
			echo "<td nowrap>" . $failImage . "&nbsp;&nbsp;</td>\n";
			echo "<td>" . sprintf(_("Your LAM admin user (%s) must be a valid Unix account to work with lamdaemon!"), $credentials[0]) . "</td>";
			$stopTest = true;
		}
		echo "</tr>\n";
	}

	flush();

	// check SSH login
	if (!$stopTest) {
		echo "<tr class=\"userlist-bright\">\n<td nowrap>" . _("SSH connection") . "&nbsp;&nbsp;</td>\n";
		flush();
		$sshOk = false;
		$handle = lamTestConnectSSH($serverName);
		if ($handle) {
			if ($handle->login($userName, $credentials[1])) {
				$sshOk = true;
			}
		}
		if ($sshOk) {
			echo "<td nowrap>" . $okImage . "&nbsp;&nbsp;</td>";
			echo "<td>" . _("SSH connection could be established.") . "</td>";
		}
		else {
			echo "<td nowrap>" . $failImage . "&nbsp;&nbsp;</td>\n";
			echo "<td>" . _("Unable to connect to remote server!") . "</td>";
			$stopTest = true;
		}
		echo "</tr>\n";
	}

	flush();
	
	$stopTest = lamTestLamdaemon("+" . $SPLIT_DELIMITER . "test" . $SPLIT_DELIMITER . "basic", $stopTest, $handle, _("Execute lamdaemon"));
	$handle = lamTestConnectSSH($serverName);
	@$handle->login($userName, $credentials[1]);
	$stopTest = lamTestLamdaemon("+" . $SPLIT_DELIMITER . "test" . $SPLIT_DELIMITER . "nss" . $SPLIT_DELIMITER . "$userName", $stopTest, $handle, _("Lamdaemon: check NSS LDAP"));
	if ($testQuota) {
		$handle = lamTestConnectSSH($serverName);
		@$handle->login($userName, $credentials[1]);
		$stopTest = lamTestLamdaemon("+" . $SPLIT_DELIMITER . "test" . $SPLIT_DELIMITER . "quota", $stopTest, $handle, _("Lamdaemon: Quota module installed"));
		$handle = lamTestConnectSSH($serverName);
		@$handle->login($userName, $credentials[1]);
		$stopTest = lamTestLamdaemon("+" . $SPLIT_DELIMITER . "quota" . $SPLIT_DELIMITER . "get" . $SPLIT_DELIMITER . "user", $stopTest, $handle, _("Lamdaemon: read quotas"));
	}

	echo "</table><br>\n";
	
	echo "<h2>" . _("Lamdaemon test finished.") . "</h2>\n";
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
