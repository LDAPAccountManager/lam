<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2004 - 2010  Roland Gruber

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
* @param array $command command to execute
* @param string $server remote server
* @return array Output of lamdaemon
*
*/
function lamdaemon($command, $server) {
	// remove the following line to restore SSH via PHP SSH2
	return lamdaemonSeclib($command, $server);
	if (!function_exists('ssh2_connect')) {
		return array('ERROR,' . _('This module requires the PHP ssh2 extension.'));
	}
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
		$sr = @ldap_read($_SESSION['ldap']->server(), $credentials[0], "objectClass=posixAccount", array('uid'), 0, 0, 0, LDAP_DEREF_NEVER);
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
			$shell = ssh2_exec($handle, "sudo " . $_SESSION['config']->get_scriptPath() . ' ' . escapeshellarg($command));
			$return = array();
			$time = time() + 30;
			while (sizeof($return) < 1) {
				if ($time < time()) {
					$return = array("ERROR," . _("Timeout while executing lamdaemon commands!") . ",");
					return $return;
				}
				usleep(100);
				$read = explode("\n", trim(fread($shell, 100000)));
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

/**
* Sends commands to lamdaemon script.
*
* @param array $command command to execute
* @param string $server remote server
* @return array Output of lamdaemon
*
*/
function lamdaemonSeclib($command, $server) {
	if ($server == '') {
		return array();
	}
	// add phpseclib to include path
	set_include_path(get_include_path() . PATH_SEPARATOR . dirname(__FILE__) . '/3rdParty/phpseclib');
	include_once('Net/SSH2.php');
	// get username and password of the current lam-admin
	$credentials = $_SESSION['ldap']->decrypt_login();
	$serverNameParts = explode(",", $server);
	if (sizeof($serverNameParts) > 1) {
		$handle = new Net_SSH2($serverNameParts[0], $serverNameParts[1]);
	}
	else {
		$handle = new Net_SSH2($server);
	}
	if ($handle) {
		$sr = @ldap_read($_SESSION['ldap']->server(), $credentials[0], "objectClass=posixAccount", array('uid'), 0, 0, 0, LDAP_DEREF_NEVER);
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
		if ($handle->login($userName, $credentials[1])) {
			$output = $handle->exec("sudo " . $_SESSION['config']->get_scriptPath() . ' ' . escapeshellarg($command));
			$return = array($output);
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
