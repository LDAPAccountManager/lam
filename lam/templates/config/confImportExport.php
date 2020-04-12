<?php
namespace LAM\CONFIG;
use htmlButton;
use htmlOutputText;
use htmlResponsiveInputField;
use htmlResponsiveRow;
use htmlStatusMessage;
use htmlSubTitle;
use LAM\PERSISTENCE\ConfigDataExporter;
use LAMCfgMain;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2020  Roland Gruber

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
* Import and export functions for LAM configuration.
*
* @package configuration
* @author Roland Gruber
*/


/** Access to persistence functions */
include_once('../../lib/persistence.inc');

// start session
if (strtolower(session_module_name()) == 'files') {
	session_save_path("../../sess");
}
lam_start_session();

setlanguage();

if (!isset($_SESSION['cfgMain'])) {
	$cfg = new LAMCfgMain();
	$_SESSION['cfgMain'] = $cfg;
}
$cfg = &$_SESSION['cfgMain'];

// export
if (isset($_POST['exportConfig']) && $cfg->checkPassword($_SESSION["mainconf_password"])) {
	$exporter = new ConfigDataExporter();
	if (!headers_sent()) {
		header('Content-Type: application/json; charset=utf-8');
		header('Content-disposition: attachment; filename=lam-config.json');
	}
	echo $exporter->exportAsJson();
	exit;
}

echo $_SESSION['header'];
printHeaderContents(_("Import and export configuration"), '../..');

?>
	</head>
	<body class="admin">
    <?php
        // include all JavaScript files
        printJsIncludes('../..');
    ?>
    <table class="lamTop ui-corner-all">
        <tr>
            <td align="left">
                <a class="lamLogo" href="http://www.ldap-account-manager.org/" target="new_window">
				    <?php echo getLAMVersionText(); ?>
                </a>
            </td>
        </tr>
    </table>
    <form action="confImportExport.php" method="post" autocomplete="off">
    <br><br>
    <?php

    // check if user is logged in
    if (!isset($_POST['submitLogin']) && !isset($_SESSION["mainconf_password"])) {
        showLoginDialog();
        exit();
    }

    // check login
    if (isset($_POST['submitLogin'])) {
	    if (!checkLogin($cfg)) {
		    exit();
	    }
    }

    displayImportExport($cfg);

    /**
     * Shows the login dialog for the configuration master password.
     *
     * @param htmlStatusMessage $message message to show if any error occured
     */
	function showLoginDialog($message = null) {
    	$tabindex = 0;
		$content = new htmlResponsiveRow();
		$loginContent = new htmlResponsiveRow();
		$loginContent->setCSSClasses(array('maxrow fullwidth roundedShadowBox spacing5'));
		if ($message !== null) {
		    $loginContent->add($message, 12);
        }
		$pwdInput = new htmlResponsiveInputField(_("Master password"), 'password', '', '236');
		$pwdInput->setIsPassword(true);
		$loginContent->add($pwdInput, 12);
		$loginContent->addLabel(new htmlOutputText('&nbsp;', false));
		$loginContent->addField(new htmlButton('submitLogin', _("Ok")));

		$content->add($loginContent, 12);

		parseHtml(null, $content, array(), false, $tabindex, null);
	}

    /**
     * Checks the login password.
     *
     * @param LAMCfgMain $cfg main config
     * @return bool login ok
     */
	function checkLogin($cfg) {
        $password = $_POST['password'];
        if ($cfg->checkPassword($password)) {
	        $_SESSION["mainconf_password"] = $password;
            return true;
        }
        showLoginDialog(new htmlStatusMessage('ERROR', _('The password is invalid! Please try again.')));
        return false;
    }

    /**
     * Displays the import/export functions.
     *
     * @param LAMCfgMain $cfg main config
     */
    function displayImportExport($cfg) {
	    $tabindex = 0;
	    $content = new htmlResponsiveRow();

	    $content->add(new htmlSubTitle(_('Export')), 12);
	    $content->add(new htmlButton('exportConfig', _('Export')), 12);

	    $content->add(new htmlSubTitle(_('Import')), 12);

	    parseHtml(null, $content, array(), false, $tabindex, null);
    }

	?>
    </form>
	</body>
</html>
