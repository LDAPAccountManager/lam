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
*
* @package modules
*/

/**
* Sends commands to lamdaemon script.
*
* @param array $commands List of command lines
* @return array Output of lamdaemon
*
*/
function lamdaemon($commands) {
	// get username and password of the current lam-admin
	$ldap_q = $_SESSION['ldap']->decrypt_login();

	$userstring = implode ("\n", $commands);
	$output_array = array();
	if (function_exists('proc_open')) {
		// New Code, requires PHP 4.3
		$towrite = escapeshellarg($_SESSION['config']->scriptServer)." ".escapeshellarg($_SESSION['config']->scriptPath)." - -";
		$descriptorspec = array(
			0 => array("pipe", "r"), // stdin
			1 => array("pipe", "w"), // stout
			2 => array("file", "/dev/null", "a") // sterr
			);
		$process = proc_open(escapeshellarg($_SESSION['lampath']."lib/lamdaemon.pl")." ".$towrite,
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
		}
	else { // PHP 4.3>
		$towrite = escapeshellarg($_SESSION['config']->scriptServer)." ".escapeshellarg($_SESSION['config']->scriptPath)." ".
			escapeshellarg($ldap_q[0]).' '.escapeshellarg($ldap_q[1]);
		$command = escapeshellarg($_SESSION['lampath']."lib/lamdaemon.pl")." ".$towrite;
		$pipe = popen("echo \"$userstring\"|$command" , 'r');
		while(!feof($pipe)) {
			//$output .= fread($pipe, 1024);
			$output = fgets($pipe, 1024);
			if ($output!='') $output_array[] = $output;
			}
		pclose($pipe);
		}
	if (sizeof($output_array) > 0) {
		return $output_array;
	}
	else {
		return false;
	}
}

?>
