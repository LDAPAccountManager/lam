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
	if($MessageTyp == "INFO")
	{
		$class = "class=\"status_info\"";
		$MessageTyp = _("Information");
	}
	elseif($MessageTyp == "WARN")
	{
		$class = "class=\"status_warn\"";
		$MessageTyp = _("Warning");
	}
	elseif($MessageTyp == "ERROR")
	{
		$class = "class=\"status_error\"";
		$MessageTyp = _("Error");
	}
	else
	{
		$class = "class=\"status_error\"";
		$MessageTyp = _("LAM Internal Error");
		$MessageHeadline = _("Invalid/Missing Message Typ");
		$MessageText = _("Please report this error to the {link=mailto:lam-devel@sourceforge.net}LDAP Account Manager Development Team{endlink}. The error number is {bold}0001:Invalid/Missing Message Typ.{endbold} Thank you.");
	}

	$MessageHeadline = parseMessageText($MessageHeadline);
	$MessageText = parseMessageText($MessageText);

	$MessageTyp = "<h1 $class>$MessageTyp</h1>";
	$MessageHeadline = "<h2 $class>$MessageHeadline</h2>";
	$MessageText = "<p $class>$MessageText</p>";
	echo "<div $class><br>" . $MessageTyp.$MessageHeadline.$MessageText . "<br></div>";
}

function parseMessageText($MessageText)
{
	$return = linkText(colorText(boldText($MessageText)));
	return $return;
}

function boldText($text)
{
	$pattern = "/\{bold\}([^{]*)\{endbold\}/";
	$replace = "<b class\"status\">\\1</b>";
	$return = preg_replace($pattern,$replace,$text);
	return $return;
}

function colorText($text)
{
	$pattern = "/\{color=([0-9,a,b,c,d,e,f,A,B,C,D,F]{6})\}([^{]*)\{endcolor\}/";
	$replace = "<font color=\"#\\1\">\\2</font>";
	$return = preg_replace($pattern,$replace,$text);
	return $return;
}

function linkText($text)
{
	$pattern = "/\{link=([^}]*)\}([^{]*)\{endlink\}/";
	$replace = "<a href=\"\\1\" target=\"_blank\">\\2</a>";
	$return = preg_replace($pattern,$replace,$text);
	return $return;
}

?>