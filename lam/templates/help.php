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


  LDAP Account Manager checking login datas.
*/

session_save_path("../sess"); // Set session save path
@session_start(); // Start LDAP Account Manager session

include_once("../lib/status.inc");

/* Read help/help.txt, build $helpArray and add $helpArray to session
Return true if file exists, false else */
function readHelpFile()
{
	global $helpArray;
	$helpFile = "../help/help.txt";
	if(is_file($helpFile))
	{
		$file = fopen($helpFile, "r");
		$i = 1;
		while(!feof($file))
		{
			$line = trim(fgets($file,8192));
			if(substr($line,0,1) == "#")
			{
				echo "continue";
				continue;
			}
			if($i == 1)
			{
				$helpNumber = $line;
				$i++;
			}
			elseif($i == 2)
			{
				$helpArray[$helpNumber]['Headline'] = $line;
				$i++;
			}
			elseif($i == 3)
			{
				$helpArray[$helpNumber]['Text'] = $line;
				$i++;
			}
			elseif($i == 4)
			{
				$helpArray[$helpNumber]['SeeAlso'] = $line;
				$i = 1;
			}
		}
		session_register("helpArray");
		return true;
	}
	return false;
}

/* Test if $helpArray is in session, if yes load it, if no try readHelpFile. If that fails define error message and return false, true else. */
function getHelpArray()
{
	global $helpArray, $errorMessage;
	if(session_is_registered('helpArray'))
	{session_register("helpArray");

		$helpArray = $_SESSION['helpArray'];
		return true;
	}
	elseif(readHelpFile())
	{
		return true;
	}
	else
	{
		$errorMessage = _("Couldn't read {bold}help/help.txt{endbold}. No topics available.");
		return false;
	}
}

/* Print HTML head */
function echoHTMLHead()
{
?>
<html>
	<head>
		<title>LDAP Account Manager Help Center</title>
		<link rel="stylesheet" type="text/css" href="../style/layout.css">
	</head>
	<body>
<?
}

/* Print HTML foot */
function echoHTMLFoot()
{
?>
	</body>
</html>
<?
}

/* Print help site */
function displayHelp($helpNumber)
{
	global $helpArray, $errorMessage;
	$loadArray = getHelpArray();
	/* If no help number was submitted print error message */
	if($helpNumber == "")
	{
		$errorMessage = _("Sorry no help number submitted.");
		echoHTMLHead();
		statusMessage("ERROR","",$errorMessage);
		echoHTMLFoot();
	}
	/* If submitted help number was not submitted print error message */
	elseif($loadArray && !array_key_exists($helpNumber,$helpArray))
	{
		$errorMessage = _("Sorry this help number ({bold}" . $helpNumber . "{endbold}) is not available.");
		echoHTMLHead();
		statusMessage("ERROR","",$errorMessage);
		echoHTMLFoot();
	}
	/* Print help site if getHelpArray was successful */
	elseif($loadArray)
	{
		echoHTMLHead();
		echo "		<h1 class=\"help\">" . $helpArray[$helpNumber]['Headline'] . "</h1>\n";
		echo "		<p class=\"help\">" . $helpArray[$helpNumber]['Text'] . "</p>\n";
		echo "		<p class=\"help\">See also: " . $helpArray[$helpNumber]['SeeAlso'] . "</p>\n";
		echoHTMLFoot();
	}
	/* Print the error messages of errors that occured before */
	else
	{
		echoHTMLHead();
		statusMessage("ERROR","",$errorMessage);
		echoHTMLFoot();
	}
}

displayHelp($_GET['HelpNumber']);

?>
