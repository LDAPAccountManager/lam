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

setlanguage();


/* Print HTML head */
function echoHTMLHead()
{
echo $_SESSION['header'];
?>
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
function displayHelp($helpEntry,$helpVariables) {
	/* Load external help page */
	if($helpEntry["ext"] == "TRUE")
	{
		echoHTMLHead();
		include_once("../help/" . $helpEntry["Link"]);
		echoHTMLFoot();
	}
	/* Print help site out of $helpEntry */
	else
	{
		echoHTMLHead();
		echo "		<h1 class=\"help\">" . $helpEntry['Headline'] . "</h1>\n";
		$format = "		<p class=\"help\">" . $helpEntry['Text'] . "</p>\n";
		array_unshift($helpVariables,$format);
		call_user_func_array("printf",$helpVariables);
		if(is_array($helpArray['SeeAlso'])) {
			while($current = current($helpEntry["SeeAlso"]))
			{
				echo '		<p class="help">' . (( isset($current['link'])) ? '<a class="helpSeeAlso" href="' . $current['link'] . '">' : '') . _('See also') . ': ' . $current['text'] . (( isset($current['link'])) ? '</a>' : '') . '</p>\n';
				next($helpEntry["SeeAlso"]);
			}
		}
		echoHTMLFoot();
	}
}

/* If no help number was submitted print error message */
if(!isset($_GET['item']))
{
	$errorMessage = _("Sorry no help number submitted.");
	echoHTMLHead();
	statusMessage("ERROR","",$errorMessage);
	echoHTMLFoot();
	exit;
}

$helpEntry = array();

if(isset($_GET['module'])) {
	include_once("../lib/modules.inc");
	$helpEntry = getHelp($_GET['module'],$_GET['item'],$_GET['scope']);
	//$helpEntry = $_SESSION['account']->getHelp($_GET['module'],$_GET['item']);
	if(!$helpEntry) {
		$variables = array();
		array_push($variables,$_GET['item']);
		array_push($variables,$_GET['module']);
		$errorMessage = _("Sorry this help id ({bold}%s{endbold}) is not available for this module ({bold}%s{endbold}).");
		echoHTMLHead();
		statusMessage("ERROR","",$errorMessage,$variables);
		echoHTMLFoot();
		exit;
	}
}
else {
	/* If submitted help number is not in help/help.inc print error message */
	if(!array_key_exists($_GET['HelpNumber'],$helpArray))
	{
		$variables = array();
		array_push($variables,$_GET['HelpNumber']);
		$errorMessage = _("Sorry this help number ({bold}%d{endbold}) is not available.");
		echoHTMLHead();
		statusMessage("ERROR","",$errorMessage,$variables);
		echoHTMLFoot();
		exit;
	}
	else {
		$helpEntry = $helpArray[$_GET['HelpNumber']];
	}
}

$i = 1;
$moreVariables = true;
$helpVariables = array();
while($moreVariables) {
	if(isset($_GET['var' . $i])) {
		array_push($helpVariables,$_GET['var' . $i]);
		$i++;
	}
	else {
		$moreVariables = false;
	}
}

displayHelp($helpEntry,$helpVariables);

?>