<?php
namespace LAM\CONFIG;
use htmlJavaScript;
use \htmlStatusMessage;
use LAMException;
use ServerProfilePersistenceManager;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2009 - 2023  Roland Gruber

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
* End page of configuration.
*
* @package configuration
* @author Roland Gruber
*/


/** Access to config functions */
include_once(__DIR__ . "/../../lib/config.inc");
/** access to module settings */
include_once(__DIR__ . "/../../lib/modules.inc");
/** common functions */
include_once __DIR__ . '/../../lib/configPages.inc';

// start session
if (isFileBasedSession()) {
	session_save_path("../../sess");
}
lam_start_session();

setlanguage();

// check if password was entered
// if not: load login page
if (!isset($_SESSION['conf_isAuthenticated']) || ($_SESSION['conf_config']->getName() !== $_SESSION['conf_isAuthenticated'])) {
	$_SESSION['conf_message'] = new htmlStatusMessage('ERROR', _("No password was entered!"));
	/** go back to login if password is empty */
	require('conflogin.php');
	exit;
}

$conf = &$_SESSION['conf_config'];
$confName = $_SESSION['conf_isAuthenticated'];

echo $_SESSION['header'];
printHeaderContents(_("LDAP Account Manager Configuration"), '../..');
echo "<body>\n";
// include all JavaScript files
printJsIncludes('../..');
printConfigurationPageHeaderBar($conf);

$serverProfilePersistenceManager = new ServerProfilePersistenceManager();
try {
	$serverProfilePersistenceManager->saveProfile($conf, $confName);
	$scriptTag = new htmlJavaScript('window.lam.dialog.showSuccessMessageAndRedirect("' . _("Your settings were successfully saved.") . '", "' . htmlspecialchars($confName) . '", "' . _('Ok') . '", "../login.php")');
	parseHtml(null, $scriptTag, array(), false, null);
}
catch (LAMException $e) {
	$scriptTag = new htmlJavaScript('window.lam.dialog.showErrorMessageAndRedirect("' . htmlspecialchars($e->getTitle()) . '", "' . htmlspecialchars($e->getMessage()) . '", "' . _('Ok') . '", "../login.php")');
	parseHtml(null, $scriptTag, array(), false, null);
}
finally {
	// remove settings from session
	$sessionKeys = array_keys($_SESSION);
	for ($i = 0; $i < sizeof($sessionKeys); $i++) {
		if (substr($sessionKeys[$i], 0, 5) == "conf_") {
			unset($_SESSION[$sessionKeys[$i]]);
		}
	}
}

?>
</body>
</html>
