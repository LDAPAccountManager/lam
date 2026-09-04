<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2026  Roland Gruber

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
include_once __DIR__ . '/../../lib/config.inc';
/** Used to print status messages */
include_once __DIR__ . '/../../lib/status.inc';
if (isLAMProVersion()) {
	include_once __DIR__ . "/../../lib/env.inc";
}

// start session
if (isFileBasedSession()) {
	session_save_path(__DIR__ . '/../../sess');
}
lam_start_session();
session_regenerate_id(true);

setlanguage();

// remove settings from session
if (isset($_SESSION["mainconf_password"])) {
	unset($_SESSION["mainconf_password"]);
}
if (isset($_SESSION['cfgMain'])) {
	unset($_SESSION['cfgMain']);
}
$cfgMain = new LAMCfgMain();
// check if user entered a password
if (isset($_POST['passwd'])) {
	if ($cfgMain->checkPassword($_POST['passwd'])) {
		$_SESSION["mainconf_password"] = $_POST['passwd'];
		metaRefresh("mainmanage.php");
		exit();
	}
	$message = _("The password is invalid! Please try again.");
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
            <img class="align-middle" width="24" height="24" alt="help" src="../../graphics/logo.svg">
            <span class="hide-on-mobile">
                        <?php
						echo getLAMVersionText();
						?>
                    </span>
        </a>
    </div>
	<?php
	if (is_dir(__DIR__ . '/../../docs/manual')) {
		?>
        <button class="lam-header-right lam-menu-icon hide-on-tablet icon"
           onclick="window.lam.topmenu.toggle();">
            <img class="align-middle" width="16" height="16" alt="menu" src="../../graphics/menu.svg">
            <span class="padding0">&nbsp;</span>
        </button>
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
    <?php
    $content = new htmlResponsiveRow();
	$content->setCSSClasses(['limitWidth']);
    $box = new htmlResponsiveRow();
    $box->setCSSClasses(['padding1', 'roundedShadowBox']);

	$box->add(new htmlOutputText(_("Please enter the master password to change the general preferences:")));
    // print message if login was incorrect or no config profiles are present
    if (isset($message)) {
		$box->addVerticalSpacer('1rem');
		$box->add(new htmlStatusMessage('ERROR', $message));
    }
	$box->addVerticalSpacer('1rem');
    // password input
    $label = new htmlOutputText(_('Master password'));
    $passwordGroup = new htmlGroup();
    $passwordField = new htmlInputField('passwd');
    $passwordField->setFieldSize(15);
    $passwordField->setIsPassword(true);
    $passwordField->setCSSClasses(['lam-initial-focus']);
    $passwordGroup->addElement($passwordField);
    $passwordGroup->addElement(new htmlHelpLink('236'));
    $passwordDiv = new htmlDiv(null, $passwordGroup);
    $passwordDiv->setCSSClasses(['nowrap']);
	$box->addLabel($label);
	$box->addField($passwordDiv);
    // button
	$box->addVerticalSpacer('1rem');
    $okButton = new htmlButton('submit', _("Ok"));
    $okButton->setCSSClasses(['lam-primary']);
	$box->add($okButton);
    $content->add($box);

    $content->addVerticalSpacer('1rem');
	$content->add(new htmlLink(_("Back to login"), '../login.php'), 12, null, null, 'text-left');
	$content->addVerticalSpacer('2rem');

    $form = new htmlForm('loginform', 'mainlogin.php', $content);
    $form->setCSSClasses(['text-center']);
    parseHtml(null, $form, [], false);
    ?>

</body>
</html>
