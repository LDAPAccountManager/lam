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


  LDAP Account Manager display help pages.
*/
include_once("../lib/ldap.inc");
include_once("../lib/config.inc");

session_save_path("../sess"); // Set session save path
@session_start(); // Start LDAP Account Manager session

include_once("../lib/status.inc"); // Include lib/status.php which provides statusMessage()
include_once("../help/help.inc"); // Include help/help.inc which provides $helpArray where the help pages are stored

/* Print HTML head */
function echoHTMLHead()
{
setlanguage();
echo $_SESSION['header'];
?>
<html>
	<head>
		<title>LDAP Account Manager Help Center</title>
		<link rel="stylesheet" type="text/css" href="../style/layout.css">
	</head>
	<body>
<?php
}

/* Print HTML foot */
function echoHTMLFoot()
{
?>
	</body>
</html>
<?php
}

/* Print help site */
function displayHelp($helpNumber)
{
	global $helpArray;
	/* If no help number was submitted print error message */
	if($helpNumber == "")
	{
		$errorMessage = _("Sorry no help number submitted.");
		echoHTMLHead();
		statusMessage("ERROR","",$errorMessage);
		echoHTMLFoot();
	}
	/* If submitted help number is not in help/help.inc print error message */
	elseif(!array_key_exists($helpNumber,$helpArray))
	{
		$reference = "({bold}" . $helpNumber . "{endbold})";
		$errorMessage = _("Sorry this help number $reference is not available.");
		echoHTMLHead();
		statusMessage("ERROR","",$errorMessage);
		echoHTMLFoot();
	}
	/* Print help site out of $helpArray */
	elseif($helpArray[$helpNumber]["ext"] == "FALSE")
	{
		echoHTMLHead();
		echo "		<h1 class=\"help\">" . $helpArray[$helpNumber]['Headline'] . "</h1>\n";
		echo "		<p class=\"help\">" . $helpArray[$helpNumber]['Text'] . "</p>\n";
		if($helpArray[$helpNumber]["SeeAlso"] <> "")
		{
			echo "		<p class=\"help\">See also: " . $helpArray[$helpNumber]['SeeAlso'] . "</p>\n";
		}
		echoHTMLFoot();
	}
	/* Load external help page */
	elseif($helpArray[$helpNumber]["ext"] == "TRUE")
	{
		echoHTMLHead();
		include_once("../help/" . $helpArray[$helpNumber]["Link"]);
		echoHTMLFoot();
	}
	/* Print empty page in all other cases */
	else
	{
		echoHTMLHead();
		echoHTMLFoot();
	}
}

displayHelp($_GET['HelpNumber']);

?>
