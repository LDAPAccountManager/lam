<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2006  Roland Gruber

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
* @package tools
*/

/** security functions */
include_once("../../lib/security.inc");
/** access to configuration options */
include_once("../../lib/config.inc");

// start session
startSecureSession();

setlanguage();

echo $_SESSION['header'];


echo "<title></title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/type_user.css\">\n";
echo "</head>";

echo "<body>\n";

echo "<h1 align=\"center\">" . _("Lamdaemon test") . "</h1>\n";

echo "<table class=\"userlist\" rules=\"none\">\n";

flush();
$stopTest = false;

// check script server and path
echo "<tr class=\"userlist\">\n<td>" . _("Lamdaemon server and path") . "&nbsp;&nbsp;</td>\n";
if (!isset($_SESSION['config']->scriptServer) || (strlen($_SESSION['config']->scriptServer) < 3)) {
	echo "<td>" . _("Error") . "</td>\n";
	echo "<td bgcolor=\"red\">" . _("No lamdaemon server set, please update your LAM configuration settings.") . "</td>";
}
elseif (!isset($_SESSION['config']->scriptPath) || (strlen($_SESSION['config']->scriptPath) < 10)) {
	echo "<td>" . _("Error") . "&nbsp;&nbsp;</td>\n";
	echo "<td bgcolor=\"red\">" . _("No lamdaemon path set, please update your LAM configuration settings.") . "</td>";
	$stopTest = true;
}
else {
	echo "<td bgcolor=\"green\">" . _("Ok") . "&nbsp;&nbsp;</td>\n";
	echo "<td bgcolor=\"green\">" . sprintf(_("Using %s as lamdaemon remote server."), $_SESSION['config']->scriptServer) . "</td>";
}
echo "</tr>\n";

flush();

// check Unix account of LAM admin
if (!$stopTest) {
	echo "<tr class=\"userlist\">\n<td>" . _("Unix account") . "&nbsp;&nbsp;</td>\n";
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
		echo "<td bgcolor=\"green\">" . _("Ok") . "</td>\n";
		echo "<td bgcolor=\"green\">" . sprintf(_("Using %s to connect to remote server."), $userName) . "</td>";
	}
	else {
		echo "<td>" . _("Error") . "&nbsp;&nbsp;</td>\n";
		echo "<td bgcolor=\"red\">" . _("Your LAM admin user must be a valid Unix account to work with lamdaemon!") . "</td>";
		$stopTest = true;
	}
	echo "</tr>\n";
}

flush();

// check SSH2 function
if (!$stopTest) {
	echo "<tr class=\"userlist\">\n<td>" . _("SSH2 module") . "&nbsp;&nbsp;</td>\n";
	if (function_exists("ssh2_connect")) {
		echo "<td bgcolor=\"green\">" . _("Ok") . "</td>";
		echo "<td bgcolor=\"green\">" . _("SSH2 module is installed.") . "</td>";
	}
	else {
		echo "<td>" . _("Error") . "&nbsp;&nbsp;</td>\n";
		echo "<td bgcolor=\"red\">" . _("Please install the SSH2 module for PHP and activate it in your php.ini!") . "</td>";
		$stopTest = true;		
	}
	echo "</tr>\n";
}

flush();

// check SSH login
if (!$stopTest) {
	echo "<tr class=\"userlist\">\n<td>" . _("SSH connection") . "&nbsp;&nbsp;</td>\n";
	flush();
	$sshOk = false;
	$handle = @ssh2_connect($_SESSION['config']->scriptServer);
	if ($handle) {
		if (@ssh2_auth_password($handle, $userName, $credentials[1])) {
			$sshOk = true;
		}
	}
	if ($sshOk) {
		echo "<td bgcolor=\"green\">" . _("Ok") . "</td>";
		echo "<td bgcolor=\"green\">" . _("SSH connection could be established.") . "</td>";
	}
	else {
		echo "<td bgcolor=\"red\">" . _("Error") . "&nbsp;&nbsp;</td>\n";
		echo "<td bgcolor=\"red\">" . _("Unable to connect to remote server!") . "</td>";
		$stopTest = true;		
	}
	echo "</tr>\n";
}

flush();

// run lamdaemon and get user quotas
if (!$stopTest) {
	echo "<tr class=\"userlist\">\n<td>" . _("Execute lamdaemon") . "&nbsp;&nbsp;</td>\n";
	flush();
	$lamdaemonOk = false;
	$errorMessage = "";
	$shell = ssh2_exec($handle, "sudo " . $_SESSION['config']->scriptPath);
	$stderr = ssh2_fetch_stream($shell, SSH2_STREAM_STDERR);
	fwrite($shell, "+ quota get user\n");
	$return = array();
	$time = time() + 20;
	while (sizeof($return) < 1) {
		if ($time < time()) {
			$lamdaemonOk = false;
			$errorMessage = _("Timeout while executing lamdaemon commands!");
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
	if ((stripos($errOut, "sudoers") !== false) || (stripos($errOut, "sorry") !== false)) {
		$return[] = "ERROR," . _("Sudo is not setup correctly!") . "," . str_replace(",", " ", $errOut);
	}
	if ((sizeof($return) == 1) && (stripos($return[0], "error") === false)) {
		$lamdaemonOk = true;
	}
	if ($lamdaemonOk) {
		echo "<td bgcolor=\"green\">" . _("Ok") . "</td>";
		echo "<td bgcolor=\"green\">" . _("Lamdaemon successfully run.") . "</td>";
	}
	else {
		echo "<td bgcolor=\"red\">" . _("Error") . "&nbsp;&nbsp;</td>\n";
		echo "<td bgcolor=\"red\">\n";
		for ($i = 0; $i < sizeof($return); $i++) {
			call_user_func_array('StatusMessage', split(",", $return[$i]));
		}
		echo "</td>\n";
		$stopTest = true;		
	}
	echo "</tr>\n";
}

echo "</table>\n";

echo "<h2>" . _("Lamdaemon test finished.") . "</h2>\n";

echo "</body>\n";
echo "</html>\n";

?>
