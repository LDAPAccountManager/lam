<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Michael Duergner

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


  LDAP Account Manager status messages.
*/

function StatusMessage($MessageTyp,$MessageHeadline,$MessageText,$MessageVariables = array()) {
	/* Setting CSS-StyleSheet class depending on the $MessageTyp and rewriting $MessageTyp with a readable string. */
	if($MessageTyp == "INFO") {
		$class = "class=\"statusInfo\"";
	}
	elseif($MessageTyp == "WARN") {
		$class = "class=\"statusWarn\"";
	}
	elseif($MessageTyp == "ERROR") {
		$class = "class=\"statusError\"";
	}
	/* Set output-message, when none or false $MessageTyp is submitted. */
	else {
		$class = "class=\"statusError\"";
		$MessageTyp = _("LAM Internal Error");
		$MessageHeadline = _("Invalid/Missing Message type");
		$MessageText = _("Please report this error to the Bug-Tracker at {link=http://lam.sf.net}LDAP Account Manager Development Team{endlink}. The error number is {bold}0001:Invalid/Missing Message type.{endbold} Thank you.");
	}

	$MessageHeadline = parseMessageString($MessageHeadline);
	$MessageText = parseMessageString($MessageText);

	if (is_file("../graphics/error.png")) {
		$MessageTyp = "<img src=\"../graphics/" . strtolower($MessageTyp) . ".png\" alt=\"" . $MessageTyp . "\" width=\"50\" height=\"50\">";
	}
	else {
		$MessageTyp = "<img src=\"../../graphics/" . strtolower($MessageTyp) . ".png\" alt=\"" . $MessageTyp . "\" width=\"50\" height=\"50\">";
	}

	$MessageHeadline = "<h2 " . $class . ">" . $MessageHeadline . "</h2>"; // Format $MessageHeadline
	$MessageText = "<p " . $class . ">" . $MessageText . "</p>"; // Format $MessageText
	$format = "<div " . $class . ">\n<br>\n<table>\n<tr>\n<td>" . $MessageTyp . "</td>\n<td>" . $MessageHeadline . $MessageText . "</td>\n</tr>\n</table>\n<br>\n</div>\n";
	for($i=0;$i<count($MessageVariables);$i++) {
		$variables .= "," . $MessageVariables[$i];
	}
	$command = 'printf("' . addslashes($format) . '"' . $variables . ');';
	call_user_func_array('printf',array($format,$MessageVariables));
	//eval($command);
}

/* Use the three replace functions on the submitted Text. */
function parseMessageString($MessageString) {
	return linkText(colorText(boldText($MessageString)));
}

/* Replace {bold} and {endbold} with <b> and </b> HTML-Tags. */
function boldText($text) {
	$pattern = "/\{bold\}([^{]*)\{endbold\}/"; // Regular expression matching {bold}[Text]{endbold}
	$replace = "<b class=\"status\">\\1</b>"; // Replace pattern
	return preg_replace($pattern,$replace,$text);
}

/* Replace {color=#[HEX-Value]} or {color=[HEX-Value]} and {endcolor} with <font color="#[HEX-Value]"> and </font> HTML-Tags. */
function colorText($text) {
	$pattern = "/\{color=#?([0-9,a-f,A-F]{6})\}([^{]*)\{endcolor\}/"; // Regular expression matching {color=#[HEX-Value]}[Text]{endcolor} or {color=[HEX-Value]}[Text]{endcolor}
	$replace = "<font color=\"#\\1\">\\2</font>"; // Replace pattern
	return preg_replace($pattern,$replace,$text);
}

/* Replace {link=[Link-Target]} and {endlink} with <a href="[Link-Target]" target="_blank"> and </a> HTML-Tags. */
function linkText($text) {
	$pattern = "/\{link=([^}]*)\}([^{]*)\{endlink\}/"; // Regular expression matching {link=[Link-Target]}[Text]{endlink}
	$replace = "<a href=\"\\1\" target=\"_blank\">\\2</a>"; //Replace pattern
	return preg_replace($pattern,$replace,$text);
}
?>
