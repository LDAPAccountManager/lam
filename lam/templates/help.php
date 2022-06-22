<?php
namespace LAM\HELP;
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Michael Duergner
                2008 - 2022  Roland Gruber

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

use ScopeAndModuleValidation;

/**
 * LDAP Account Manager help page.
 *
 * @author Michael Duergner
 * @author Roland Gruber
 * @package Help
 */

/** LDAP connection */
include_once(__DIR__ . "/../lib/ldap.inc");

/** configuration */
include_once(__DIR__ . "/../lib/config.inc");

/** self service functions */
include_once(__DIR__ . "/../lib/selfService.inc");
if (!empty($_GET['selfService']) && ($_GET['selfService'] === '1')) {
	session_name('SELFSERVICE');
}

if (strtolower(session_module_name()) == 'files') {
	session_save_path("../sess");
}
lam_start_session();

/** status messages */
include_once(__DIR__ . "/../lib/status.inc");

setlanguage();

/** help data */
include_once(__DIR__ . "/../help/help.inc"); // Include help/help.inc which provides $helpArray where the help pages are stored


/**
 * Print HTML header of the help page.
 */
function echoHTMLHead(): void {
	echo $_SESSION['header'];
	$title = "LDAP Account Manager Help";
	printHeaderContents($title, '..');
	?>
		</head>
		<body>
	<?php
	// include all JavaScript files
	printJsIncludes('..');
}

/**
 * Print HTML footer of the help page.
 */
function echoHTMLFoot(): void {
	?>
		</body>
	</html>
	<?php
}

/**
 * Print help site for a specific help number.
 *
 * @param array<mixed> $helpEntry the help entry that is to be displayed.
 */
function displayHelp(array $helpEntry): void {
	echoHTMLHead();
	echo "<h1 class=\"help\">" . $helpEntry['Headline'] . "</h1>\n";
	$format = "<p class=\"help\">" . $helpEntry['Text'] . "</p>\n";
	if (isset($helpEntry['attr'])) {
		$format .= '<br><hr>' . _('Technical name') . ': <i>' . $helpEntry['attr'] . '</i>';
	}
	echo $format;
	if(isset($helpEntry['SeeAlso']) && is_array($helpEntry['SeeAlso'])) {
		echo '<p class="help">' . _('See also') . ': <a class="helpSeeAlso" href="' . $helpEntry['SeeAlso']['link'] . '">' . $helpEntry['SeeAlso']['text'] . '</a></p>';
	}
	echoHTMLFoot();
}

/* If no help number was submitted print error message */
if (!isset($_GET['HelpNumber'])) {
	$errorMessage = "Sorry no help number submitted.";
	echoHTMLHead();
	statusMessage("ERROR", "", $errorMessage);
	echoHTMLFoot();
	exit;
}

$helpEntry = array();

// module help
if (isset($_GET['module']) && !($_GET['module'] == 'main') && !($_GET['module'] == '')) {
	include_once(__DIR__ . "/../lib/modules.inc");
	$moduleName = $_GET['module'];
	if (!ScopeAndModuleValidation::isValidModuleName($moduleName)) {
	    logNewMessage(LOG_ERR, 'Invalid module name: ' . $moduleName);
	    die();
    }
	if (!empty($_GET['scope'])) {
	    $scope = $_GET['scope'];
	    if (!ScopeAndModuleValidation::isValidScopeName($scope)) {
		    logNewMessage(LOG_ERR, 'Invalid scope name: ' . $scope);
		    die();
        }
		$helpEntry = getHelp($moduleName, $_GET['HelpNumber'], $scope);
	}
	if (!$helpEntry) {
		$variables = array(htmlspecialchars($_GET['HelpNumber']), htmlspecialchars($moduleName));
		$errorMessage = _("Sorry the help id '%s' is not available for the module '%s'.");
		echoHTMLHead();
		statusMessage("ERROR", "", $errorMessage, $variables);
		echoHTMLFoot();
		exit;
	}
}
// help entry in help.inc
else {
	/* If submitted help number is not in help/help.inc print error message */
	if (!array_key_exists($_GET['HelpNumber'], $helpArray)) {
		$variables = array(htmlspecialchars($_GET['HelpNumber']));
		$errorMessage = _("Sorry this help number ({bold}%s{endbold}) is not available.");
		echoHTMLHead();
		statusMessage("ERROR", "", $errorMessage, $variables);
		echoHTMLFoot();
		exit;
	}
	else {
		$helpEntry = $helpArray[$_GET['HelpNumber']];
	}
}

displayHelp($helpEntry);
