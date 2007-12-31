<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2006 - 2007  Roland Gruber

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

echo $_SESSION['header'];


echo "<title></title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/type_user.css\">\n";
echo "</head>";

echo "<body>\n";

echo "<h1 align=\"center\">" . _("Lamdaemon test") . "</h1>\n";

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
	$okImage = "<img src=\"../../graphics/pass.png\" alt=\"\">\n";
	$failImage = "<img src=\"../../graphics/fail.png\" alt=\"\">\n";
	// run lamdaemon and get user quotas
	if (!$stopTest) {
		echo "<tr class=\"userlist\">\n<td nowrap>" . $testText . "&nbsp;&nbsp;</td>\n";
		flush();
		$lamdaemonOk = false;
		$errorMessage = "";
		$shell = ssh2_exec($handle, "sudo " . $_SESSION['config']->get_scriptPath());
		$stderr = ssh2_fetch_stream($shell, SSH2_STREAM_STDERR);
		fwrite($shell, $command);
		$return = array();
		$time = time() + 20;
		while (sizeof($return) < 1) {
			if ($time < time()) {
				$lamdaemonOk = false;
				$return[] = "ERROR," . _("Timeout while executing lamdaemon commands!");
				break;
			}
			usleep(100);
			$read = split("\n", trim(fread($shell, 100000)));
			if ((sizeof($read) == 1) && (!isset($read[0]) || ($read[0] == ""))) continue;
			for ($i = 0; $i < sizeof($read); $i++) {
				$return[] = $read[$i];
			}
		}
		$errOut = @fread($stderr, 100000);
		if ((strpos(strtolower($errOut), "sudoers") !== false) || (strpos(strtolower($errOut), "sorry") !== false)) {
			$return[] = "ERROR," . _("Sudo is not setup correctly!") . "," . htmlspecialchars(str_replace(",", " ", $errOut));
		}
		elseif (strlen($errOut) > 0) {
			$return[] = "ERROR," . _("Unknown error") . "," . htmlspecialchars(str_replace(",", " ", $errOut));
		}
		if ((sizeof($return) == 1) && (strpos(strtolower($return[0]), "error") === false)) {
			$lamdaemonOk = true;
		}
		if ($lamdaemonOk) {
			echo "<td>" . $okImage . "</td>";
			echo "<td>" . _("Lamdaemon successfully run.") . "</td>";
		}
		else {
			echo "<td>" . $failImage . "&nbsp;&nbsp;</td>\n";
			echo "<td>\n";
			for ($i = 0; $i < sizeof($return); $i++) {
				call_user_func_array('StatusMessage', split(",", $return[$i]));
			}
			echo "</td>\n";
			$stopTest = true;
		}
		echo "</tr>\n";
	}
	flush();
	return $stopTest;
}

$okImage = "<img src=\"../../graphics/pass.png\" alt=\"\">\n";
$failImage = "<img src=\"../../graphics/fail.png\" alt=\"\">\n";

$servers = explode(";", $_SESSION['config']->get_scriptServers());
for ($i = 0; $i < sizeof($servers); $i++) {
	$servers[$i] = explode(":", $servers[$i]);
	$serverName = $servers[$i][0];
	$title = $serverName;
	$serverDisplayName = $servers[$i][0];
	if (isset($servers[$i][1])) {
		$serverDisplayName = $servers[$i][1];
		$title = $serverDisplayName . " (" . $serverName . ")";
	}

	echo "<table class=\"userlist\" rules=\"none\" width=\"750\">\n";

	flush();
	$stopTest = false;

	echo "<tr class=\"userlist\">\n<td colspan=\"3\" align=\"center\"><b>$title</b>\n</td>\n</tr>";

	// check script server and path
	echo "<tr class=\"userlist\">\n<td nowrap>" . _("Lamdaemon server and path") . "&nbsp;&nbsp;</td>\n";
	if (!isset($serverName) || (strlen($serverName) < 3)) {
		echo "<td>" . $failImage . "</td>\n";
		echo "<td>" . _("No lamdaemon server set, please update your LAM configuration settings.") . "</td>";
	}
	elseif (($_SESSION['config']->get_scriptPath() == null) || (strlen($_SESSION['config']->get_scriptPath()) < 10)) {
		echo "<td>" . $failImage . "&nbsp;&nbsp;</td>\n";
		echo "<td>" . _("No lamdaemon path set, please update your LAM configuration settings.") . "</td>";
		$stopTest = true;
	}
	else {
		echo "<td>" . $okImage . "&nbsp;&nbsp;</td>\n";
		echo "<td>" . sprintf(_("Using %s as lamdaemon remote server."), $serverName) . "</td>";
	}
	echo "</tr>\n";

	flush();

	// check Unix account of LAM admin
	if (!$stopTest) {
		echo "<tr class=\"userlist\">\n<td nowrap>" . _("Unix account") . "&nbsp;&nbsp;</td>\n";
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
			echo "<td>" . $okImage . "</td>\n";
			echo "<td>" . sprintf(_("Using %s to connect to remote server."), $userName) . "</td>";
		}
		else {
			echo "<td>" . $failImage . "&nbsp;&nbsp;</td>\n";
			echo "<td>" . sprintf(_("Your LAM admin user (%s) must be a valid Unix account to work with lamdaemon!"), $credentials[0]) . "</td>";
			$stopTest = true;
		}
		echo "</tr>\n";
	}

	flush();

	// check SSH2 function
	if (!$stopTest) {
		echo "<tr class=\"userlist\">\n<td nowrap>" . _("SSH2 module") . "&nbsp;&nbsp;</td>\n";
		if (function_exists("ssh2_connect")) {
			echo "<td>" . $okImage . "</td>";
			echo "<td>" . _("SSH2 module is installed.") . "</td>";
		}
		else {
			echo "<td>" . $failImage . "&nbsp;&nbsp;</td>\n";
			echo "<td>" . _("Please install the SSH2 module for PHP and activate it in your php.ini!") . "</td>";
			$stopTest = true;
		}
		echo "</tr>\n";
	}

	flush();

	// check SSH login
	if (!$stopTest) {
		echo "<tr class=\"userlist\">\n<td nowrap>" . _("SSH connection") . "&nbsp;&nbsp;</td>\n";
		flush();
		$sshOk = false;
		$handle = @ssh2_connect($serverName);
		if ($handle) {
			if (@ssh2_auth_password($handle, $userName, $credentials[1])) {
				$sshOk = true;
			}
		}
		if ($sshOk) {
			echo "<td>" . $okImage . "</td>";
			echo "<td>" . _("SSH connection could be established.") . "</td>";
		}
		else {
			echo "<td>" . $failImage . "&nbsp;&nbsp;</td>\n";
			echo "<td>" . _("Unable to connect to remote server!") . "</td>";
			$stopTest = true;
		}
		echo "</tr>\n";
	}

	flush();

	$stopTest = lamTestLamdaemon("+ test basic\n", $stopTest, $handle, _("Execute lamdaemon"));
	$stopTest = lamTestLamdaemon("+ test quota\n", $stopTest, $handle, _("Lamdaemon: Quota module installed"));
	$stopTest = lamTestLamdaemon("+ quota get user\n", $stopTest, $handle, _("Lamdaemon: read quotas"));

	echo "</table><br>\n";
}

echo "<h2>" . _("Lamdaemon test finished.") . "</h2>\n";

echo "</body>\n";
echo "</html>\n";

?>
