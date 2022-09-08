<?php
namespace LAM\CONFIG;
use \htmlStatusMessage;
use \htmlResponsiveRow;
use \LAMCfgMain;
use \htmlButton;
use \htmlOutputText;
use \htmlLink;
use \htmlDiv;
use \htmlResponsiveSelect;
use \htmlResponsiveInputField;
use \htmlHorizontalLine;
use LAMException;
use ServerProfilePersistenceManager;

/*
  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2021  Roland Gruber

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
* Login page to change the preferences.
*
* @package configuration
* @author Roland Gruber
*/


/** Access to config functions */
include_once('../../lib/config.inc');
/** Used to print status messages */
include_once('../../lib/status.inc');

// start session
if (isFileBasedSession()) {
	session_save_path(dirname(__FILE__) . '/../../sess');
}
lam_start_session();
session_regenerate_id(true);

setlanguage();

$message = null;
// get error message from confmain.php
if (isset($_SESSION['conf_message'])) {
	$message = $_SESSION['conf_message'];
}

// remove settings from session
$sessionKeys = array_keys($_SESSION);
for ($i = 0; $i < sizeof($sessionKeys); $i++) {
	if (substr($sessionKeys[$i], 0, 5) == "conf_") unset($_SESSION[$sessionKeys[$i]]);
}

echo $_SESSION['header'];

$serverProfilePersistenceManager = new ServerProfilePersistenceManager();
$files = array();
try {
	$files = $serverProfilePersistenceManager->getProfiles();
}
catch (LAMException $e) {
	logNewMessage(LOG_ERR, 'Unable to read server profiles: ' . $e->getTitle());
}
printHeaderContents(_("Login"), '../..');

if (sizeof($files) < 1) {
	$message = new htmlStatusMessage('INFO', _("No server profiles found. Please create one."));
}
$tabindex = 1;
?>
	</head>
	<body>
		<?php
			printJsIncludes('../..');
			?>
        <div id="lam-topnav" class="lam-header">
            <div class="lam-header-left lam-menu-stay">
                <a href="https://www.ldap-account-manager.org/" target="new_window">
                    <img class="align-middle" width="24" height="24" alt="help" src="../../graphics/logo24.png">
                    <span class="hide-on-mobile">
                        <?php
                        echo getLAMVersionText();
                        ?>
                    </span>
                </a>
            </div>
			<?php
			if (is_dir(dirname(__FILE__) . '/../../docs/manual')) {
				?>
                <a class="lam-header-right lam-menu-icon hide-on-tablet" href="javascript:void(0);" class="icon" onclick="window.lam.topmenu.toggle();">
                    <img class="align-middle" width="16" height="16" alt="menu" src="../../graphics/menu.svg">
                    <span class="padding0">&nbsp;</span>
                </a>
                <a class="lam-header-right lam-menu-entry" target="_blank" href="../../docs/manual/index.html">
                    <span class="padding0"><?php echo _("Help") ?></span>
                </a>
				<?php
			}
			?>
        </div>
		<br><br>
		<!-- form to change existing profiles -->
		<form action="confmain.php" method="post" autocomplete="off">

		<?php

		$row = new htmlResponsiveRow();

		// message
		if ($message !== null) {
			$row->add($message, 12);
			$row->addVerticalSpacer('2rem');
		}

		$box = new htmlResponsiveRow();
		if (sizeof($files) > 0) {
			$box->add(new htmlOutputText(_("Please enter your password to change the server preferences:")), 12);
			$box->addVerticalSpacer('1.5rem');
			$conf = new LAMCfgMain();
			$selectedProfile = array();
			$profilesExisting = false;
			$profiles = $files;
			if (!empty($_COOKIE["lam_default_profile"]) && in_array($_COOKIE["lam_default_profile"], $files)) {
				$selectedProfile[] = $_COOKIE["lam_default_profile"];
			}
			else {
				$selectedProfile[] = $conf->default;
			}
			$box->add(new htmlResponsiveSelect('filename', $profiles, $selectedProfile, _('Profile name')), 12);
			$passwordInput = new htmlResponsiveInputField(_('Password'), 'passwd', '', '200');
			$passwordInput->setIsPassword(true);
			$passwordInput->setCSSClasses(array('lam-initial-focus'));
			$box->add($passwordInput, 12);
			$box->addVerticalSpacer('1rem');
			$button = new htmlButton('submit', _("Ok"));
			$button->setCSSClasses(array('lam-primary'));
			$box->addLabel($button);
			$box->add(new htmlOutputText(''), 0, 6);
			$box->addVerticalSpacer('1.5rem');
			$box->add(new htmlHorizontalLine(), 12);
			$box->addVerticalSpacer('1.5rem');
		}
		$manageLink = new htmlLink(_("Manage server profiles"), 'profmanage.php');
		$box->add($manageLink, 12, 12, 12, 'text-center');

		$boxDiv = new htmlDiv(null, $box);
		$boxDiv->setCSSClasses(array('ui-corner-all', 'roundedShadowBox', 'limitWidth', 'text-center'));
		$row->add($boxDiv, 12);

		// back link
		$row->addVerticalSpacer('2rem');
		$backLink = new htmlLink(_("Back to login"), '../login.php');
		$row->add($backLink, 12, 12, 12, 'text-left');

		parseHtml(null, new htmlDiv(null, $row, array('centeredTable')), array(), false, $tabindex, 'user');

		?>
		</form>

		<p><br><br></p>


	</body>
</html>
