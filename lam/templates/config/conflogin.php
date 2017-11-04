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
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2016  Roland Gruber

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
if (strtolower(session_module_name()) == 'files') {
	session_save_path(dirname(__FILE__) . '/../../sess');
}
session_set_cookie_params(0, '/', null, null, true);
session_start();
session_regenerate_id(true);

setlanguage();

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

$files = getConfigProfiles();
printHeaderContents(_("Login"), '../..');

if (sizeof($files) < 1) {
	$message = new htmlStatusMessage('INFO', _("No server profiles found. Please create one."));
}
$tabindex = 1;
?>
	</head>
	<body class="admin">
		<?php
			printJsIncludes('../..');
			// set focus on password field
			?>
			<script type="text/javascript" language="javascript">
			<!--
			window.onload = function() {
				loginField = document.getElementsByName('passwd')[0];
				loginField.focus();
			}
			jQuery(document).ready(function() {
				jQuery('#submitButton').button();
			});
			//-->
			</script>
		<table border=0 width="100%" class="lamHeader ui-corner-all">
			<tr>
				<td align="left" height="30">
					<a class="lamLogo" href="http://www.ldap-account-manager.org/" target="new_window">LDAP Account Manager</a>
				</td>
			</tr>
		</table>
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
			$box->add($passwordInput, 12);
			$box->addVerticalSpacer('1rem');
			$button = new htmlButton('submit', _("Ok"));
			$box->addLabel($button);
			$box->add(new htmlOutputText(''), 0, 6);
			$box->addVerticalSpacer('1.5rem');
			$box->add(new htmlHorizontalLine(), 12);
			$box->addVerticalSpacer('1.5rem');
		}
		$box->add(new htmlLink(_("Manage server profiles"), 'profmanage.php'), 12, 12, 12, 'text-center');

		$boxDiv = new htmlDiv(null, $box);
		$boxDiv->setCSSClasses(array('ui-corner-all', 'roundedShadowBox', 'limitWidth'));
		$row->add($boxDiv, 12);

		// back link
		$row->addVerticalSpacer('2rem');
		$backLink = new htmlLink(_("Back to login"), '../login.php', '../../graphics/undo.png');
		$row->add($backLink, 12, 12, 12, 'text-left');

		parseHtml(null, $row, array(), false, $tabindex, 'user');

		?>
		</form>

		<p><br><br></p>


	</body>
</html>
