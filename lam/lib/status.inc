<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Michael Duergner
                2011 - 2024  Roland Gruber

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
 * LDAP Account Manager status messages.
 *
 * @author Michael Duergner
 * @package lib
 */

/**
 * This function prints a short status message. It can be used to print INFO,
 * WARN and ERROR messages.
 *
 * @param string $MessageTyp The type of the message to be printed. It must be one of
 * the following types: 'INFO', 'WARN' or 'ERROR'.
 * <br> Every other type will lead to an error message indicating an invalid message type.
 * @param string $MessageHeadline The headline of the status message.
 * <br> It may be formatted with special color/link/bold tags.
 * @param string $MessageText The text of the status message.
 * <br> It may be formatted with special color/link/bold tags. This parameter is optional.
 * @param array $MessageVariables The variables that are used to replace the spacers (%s) in the
 * submitted text. This parameter is optional.
 * @param boolean $returnOutput if set to true this function will return the generated HTML code instead of printing it directly (default: false)
 * @return String HTML code if $returnOutput is set to true, otherwise null
 */
function StatusMessage($MessageTyp, $MessageHeadline, $MessageText = '', $MessageVariables = [], $returnOutput = false) {
	/* Setting CSS-StyleSheet class depending on the $MessageTyp and rewriting $MessageTyp with a readable string. */
	if ($MessageTyp == "INFO") {
		$class = "class=\"statusInfo lam-status-message\"";
	}
	elseif ($MessageTyp == "WARN") {
		$class = "class=\"statusWarn lam-status-message\"";
	}
	elseif ($MessageTyp == "ERROR") {
		$class = "class=\"statusError lam-status-message\"";
	}
	/* Set output-message, when none or false $MessageTyp is submitted. */
	else {
		$class = "class=\"statusError lam-status-message\"";
		$MessageHeadline = "Invalid/Missing Message type";
		$MessageText = "Please report this error to the Bug-Tracker at {link=http://www.ldap-account-manager.org/}LDAP Account Manager Development Team{endlink}. Thank you.";
	}

	$MessageHeadline = "<div class=\"statusTitle\">" . $MessageHeadline . "</div>"; // Format $MessageHeadline
	if ($MessageText != '') {
		$MessageText = "<p class=\"statusText\">" . $MessageText . "</p>"; // Format $MessageText
	}
	$format = "<div " . $class . ">\n<table>\n<tr>\n<td>" . $MessageHeadline . $MessageText . "</td>\n</tr>\n</table>\n</div>\n";
	if (is_array($MessageVariables)) {
		if (sizeof($MessageVariables) > 0) {
			array_unshift($MessageVariables, $format);
			$output = call_user_func_array('sprintf', $MessageVariables);
		}
		else {
			$output = $format;
		}
	}
	elseif ($MessageVariables !== null) {
		$output = sprintf($format, $MessageVariables);
	}
	else {
		$output = $format;
	}
	if ($returnOutput) {
		return $output;
	}
	else {
		echo $output;
	}
	return null;
}
