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

function StatusMessage($MessageTyp, $MessageHeadline, $MessageText)
{
	/* Setting CSS-StyleSheet class depending on the $MessageTyp and rewriting $MessageTyp with a readable string. */
	if($MessageTyp == "INFO")
	{
		$class = "class=\"statusInfo\"";
		$MessageTyp = _("Information");
	}
	elseif($MessageTyp == "WARN")
	{
		$class = "class=\"statusWarn\"";
		$MessageTyp = _("Warning");
	}
	elseif($MessageTyp == "ERROR")
	{
		$class = "class=\"statusError\"";
		$MessageTyp = _("Error");
	}
	/* Set output-message, when none or false $MessageTyp is submitted. */
	else
	{
		$class = "class=\"statusError\"";
		$MessageTyp = _("LAM Internal Error");
		$MessageHeadline = _("Invalid/Missing Message type");
		$MessageText = _("Please report this error to the Bug-Tracker at {link=http://lam.sf.net}LDAP Account Manager Development Team{endlink}. The error number is {bold}0001:Invalid/Missing Message type.{endbold} Thank you.");
	}

	$MessageHeadline = parseMessageString($MessageHeadline);
	$MessageText = parseMessageString($MessageText);

	$MessageTyp = "<h1 $class>$MessageTyp</h1>"; // Format $MessageTyp
	$MessageHeadline = "<h2 $class>$MessageHeadline</h2>"; // Format $MessageHeadline
	$MessageText = "<p $class>$MessageText</p>"; // Format $MessageText
	echo "<div $class><br>" . $MessageTyp.$MessageHeadline.$MessageText . "<br></div>"; // Writing status message
}

/* Use the three replace functions on the submitted Text. */
function parseMessageString($MessageString)
{
	return linkText(colorText(boldText($MessageString)));
}

/* Replace {bold} and {endbold} with <b> and </b> HTML-Tags. */
function boldText($text)
{
	$pattern = "/\{bold\}([^{]*)\{endbold\}/"; // Regular expression matching {bold}[Text]{endbold}
	$replace = "<b class\"status\">\\1</b>"; // Replace pattern
	return preg_replace($pattern,$replace,$text);
}

/* Replace {color=#[HEX-Value]} or {color=[HEX-Value]} and {endcolor} with <font color="#[HEX-Value]"> and </font> HTML-Tags. */
function colorText($text)
{
	$pattern = "/\{color=#?([0-9,a-f,A-F]{6})\}([^{]*)\{endcolor\}/"; // Regular expression matching {color=#[HEX-Value]}[Text]{endcolor} or {color=[HEX-Value]}[Text]{endcolor}
	$replace = "<font color=\"#\\1\">\\2</font>"; // Replace pattern
	return preg_replace($pattern,$replace,$text);
}

/* Replace {link=[Link-Target]} and {endlink} with <a href="[Link-Target]" target="_blank"> and </a> HTML-Tags. */
function linkText($text)
{
	$pattern = "/\{link=([^}]*)\}([^{]*)\{endlink\}/"; // Regular expression matching {link=[Link-Target]}[Text]{endlink}
	$replace = "<a href=\"\\1\" target=\"_blank\">\\2</a>"; //Replace pattern
	return preg_replace($pattern,$replace,$text);
}
?>
