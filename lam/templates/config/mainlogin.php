<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2022  Roland Gruber

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
* Login page to change the main preferences.
*
* @package configuration
* @author Roland Gruber
*/


/** Access to config functions */
include_once('../../lib/config.inc');
/** Used to print status messages */
include_once('../../lib/status.inc');
if (isLAMProVersion()) {
	include_once(__DIR__ . "/../../lib/env.inc");
}

// start session
if (isFileBasedSession()) {
	session_save_path(dirname(__FILE__) . '/../../sess');
}
lam_start_session();
session_regenerate_id(true);

setlanguage();

// remove settings from session
if (isset($_SESSION["mainconf_password"])) unset($_SESSION["mainconf_password"]);
if (isset($_SESSION['cfgMain'])) {
	unset($_SESSION['cfgMain']);
}
$cfgMain = new LAMCfgMain();
// check if user entered a password
if (isset($_POST['passwd'])) {
	if (isset($_POST['passwd']) && ($cfgMain->checkPassword($_POST['passwd']))) {
		$_SESSION["mainconf_password"] = $_POST['passwd'];
		metaRefresh("mainmanage.php");
		exit();
	}
	else {
		$message = _("The password is invalid! Please try again.");
	}
}

if (isset($_SESSION['header'])) {
	echo $_SESSION['header'];
}
printHeaderContents(_("Login"), '../..');
?>
	</head>
	<body>
		<?php
			// include all JavaScript files
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
		<br>
		<?php
			// check if config file is writable
			if (!$cfgMain->isWritable()) {
				StatusMessage('WARN', _('The config file is not writable.'), _('Your changes cannot be saved until you make the file writable for the webserver user.'));
			}
			if (!empty($_GET['invalidLicense']) && ($_GET['invalidLicense'] == '1')) {
				StatusMessage('WARN', _('Invalid licence'), _('Please setup your licence data.'));
			}
			if (!empty($_GET['invalidLicense']) && ($_GET['invalidLicense'] == '2')) {
				StatusMessage('WARN', _('Expired licence'), _('Please setup your licence data.'));
			}
		?>
		<br>
		<!-- form to change main options -->
		<form action="mainlogin.php" method="post" autocomplete="off">
		<table align="center">
		<tr><td>
		<table align="center" border="0" rules="none" bgcolor="white" class="roundedShadowBox" style="padding: 20px;">
		<tr><td>
		<?php
		$spacer = new htmlSpacer('20px', '20px');
		$group = new htmlGroup();
		$row = new htmlResponsiveRow();
		$row->add(new htmlOutputText(_("Please enter the master password to change the general preferences:")), 12);
		$group->addElement($row);
		// print message if login was incorrect or no config profiles are present
		if (isset($message)) {
		    $messageField = new htmlStatusMessage('ERROR', $message);
		    $row = new htmlResponsiveRow();
		    $row->add($messageField, 12);
		    $group->addElement($spacer);
		    $group->addElement($row);
		}
		$group->addElement($spacer);
		// password input
		$label = new htmlOutputText(_('Master password'));
		$passwordGroup = new htmlGroup();
		$passwordField = new htmlInputField('passwd');
		$passwordField->setFieldSize(15);
		$passwordField->setIsPassword(true);
		$passwordField->setCSSClasses(array('lam-initial-focus'));
		$passwordGroup->addElement($passwordField);
		$passwordGroup->addElement(new htmlHelpLink('236'));
		$passwordDiv = new htmlDiv(null, $passwordGroup);
		$passwordDiv->setCSSClasses(array('nowrap'));
		$row = new htmlResponsiveRow($label, $passwordDiv);
		$group->addElement($row);
		// button
		$group->addElement($spacer);
		$okButton = new htmlButton('submit', _("Ok"));
		$okButton->setCSSClasses(array('lam-primary'));
		$row = new htmlResponsiveRow();
		$row->add($okButton, 12);
		$row->setCSSClasses(array(''));
		$group->addElement($row);

		$div = new htmlDiv(null, $group);
		$div->setCSSClasses(array('centeredTable'));

		$tabindex = 1;
		parseHtml(null, $div, array(), false, $tabindex, 'user');
		?>
		</td></tr>
		</table>
		</td></tr>
		<tr><td align="left">
		<br><a href="../login.php"><?php echo _("Back to login"); ?></a>
		</td></tr>
		</table>
		</form>

		<p><br><br></p>

	</body>
</html>
