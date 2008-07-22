<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2004 - 2006  Roland Gruber

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
* This file includes functions to control lamdaemon.
*
* @author Tilo Lutz
* @author Roland Gruber
* @author Thomas Manninger
*
* @package modules
*/

/**
* Sends commands to lamdaemon script.
*
* @param array $commands List of command lines
* @param string $server remote server
* @return array Output of lamdaemon
*
*/
function lamdaemon($commands, $server) {
	// use new PHP SSH mechanismn
	if (function_exists("ssh2_connect")) {
		return lamdaemonSSH($commands, $server);
	}

	// get username and password of the current lam-admin
	$ldap_q = $_SESSION['ldap']->decrypt_login();

	$userstring = implode ("\n", $commands);
	$output_array = array();
	$towrite = escapeshellarg($server)." ".escapeshellarg($_SESSION['config']->get_scriptPath())." - -";
	$descriptorspec = array(
		0 => array("pipe", "r"), // stdin
		1 => array("pipe", "w"), // stout
		2 => array("file", "/dev/null", "a") // sterr
		);
	$process = proc_open(escapeshellarg($_SESSION['lampath']."lib/lamdaemonOld.pl")." ".$towrite,
		$descriptorspec,
		$pipes);
	if (is_resource($process)) {
		/* perl-script is running
		* $pipes[0] is writeable handle to child stdin
		* $pipes[1] is readable handle to child stdout
		* any error is send to /dev/null
		*/
		// user+passwd
		fwrite($pipes[0], $ldap_q[0] . "\n");
		fwrite($pipes[0], $ldap_q[1] . "\n");
		// Write to stdin
		fwrite($pipes[0], $userstring);
		}
	fclose($pipes[0]);
	while (!feof($pipes[1])) {
		$output = fgets($pipes[1], 1024);
		if ($output!='') $output_array[] = $output;
		}
	fclose($pipes[1]);
	proc_close($process);
	if (sizeof($output_array) > 0) {
		return $output_array;
	}
	else {
		return false;
	}
}

/**
* Sends commands to lamdaemon script via PHP SSH functions.
*
* @param array $commands List of command lines
* @param string $server remote server
* @return array Output of lamdaemon
*
*/
function lamdaemonSSH($commands, $server) {
	$commands = implode("\n", $commands) . "\n";
	// get username and password of the current lam-admin
	$credentials = $_SESSION['ldap']->decrypt_login();
	$serverNameParts = explode(",", $server);
	if (sizeof($serverNameParts) > 1) {
		$handle = @ssh2_connect($serverNameParts[0], $serverNameParts[1]);
	}
	else {
		$handle = @ssh2_connect($server);
	}
	if ($handle) {
		$sr = @ldap_read($_SESSION['ldap']->server(), $credentials[0], "objectClass=posixAccount", array('uid'));
		if (!$sr) {
			$return = array("ERROR," . _("Your LAM admin user must be a valid Unix account to work with lamdaemon!") . ",");
			return $return;
		}
		$entry = @ldap_get_entries($_SESSION['ldap']->server(), $sr);
		if (!isset($entry[0]['uid'][0])) {
			$return = array("ERROR," . _("Your LAM admin user must be a valid Unix account to work with lamdaemon!") . ",");
			return $return;
		}
		$userName = $entry[0]['uid'][0];
		if (@ssh2_auth_password($handle, $userName, $credentials[1])) {
			$shell = ssh2_exec($handle, "sudo " . $_SESSION['config']->get_scriptPath());
			fwrite($shell, $commands);
			$return = array();
			$time = time() + (sizeof($commands) * 30);
			while (sizeof($return) < sizeof($commands)) {
				if ($time < time()) {
					$return = array("ERROR," . _("Timeout while executing lamdaemon commands!") . ",");
					return $return;
				}
				usleep(100);
				$read = split("\n", trim(fread($shell, 100000)));
				if ((sizeof($read) == 1) && (!isset($read[0]) || ($read[0] == ""))) continue;
				for ($i = 0; $i < sizeof($read); $i++) {
					$return[] = $read[$i];
				}
			}
			return $return;
		}
		else {
			$return = array("ERROR," . _('Unable to connect to remote server!') . "," . $server);
			return $return;
		}
	}
	else {
		$return = array("ERROR," . _('Unable to connect to remote server!') . "," . $server);
		return $return;
	}
}

?>
