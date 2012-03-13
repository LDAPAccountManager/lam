<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2012  Roland Gruber

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
* Configuration profile management.
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
	session_save_path("../../sess");
}
@session_start();

setlanguage();


$cfg = new LAMCfgMain();
// check if submit button was pressed
if (isset($_POST['action'])) {
	// check master password
	if (!$cfg->checkPassword($_POST['passwd'])) {
		$error = _("Master password is wrong!");
	}
	// add new profile
	elseif ($_POST['action'] == "add") {
		if (preg_match("/^[a-z0-9_-]+$/i", $_POST['addprofile']) && !in_array($_POST['addprofile'], getConfigProfiles())) {
			// check profile password
			if ($_POST['addpassword'] && $_POST['addpassword2'] && ($_POST['addpassword'] == $_POST['addpassword2'])) {
				// check if lam.conf_sample exists
				if (!is_file("../../config/lam.conf_sample")) {
					$error = "The file config/lam.conf_sample was not found. Please restore it.";				
				}
				else {
					// create new profile file
					@copy("../../config/lam.conf_sample", "../../config/" . $_POST['addprofile'] . ".conf");
					@chmod ("../../config/" . $_POST['addprofile'] . ".conf", 0600);
					$file = is_file("../../config/" . $_POST['addprofile'] . ".conf");
					if ($file) {
						// load as config and write new password
						$conf = new LAMConfig($_POST['addprofile']);
						$conf->set_Passwd($_POST['addpassword']);
						$conf->save();
						$_SESSION['conf_isAuthenticated'] = $_POST['addprofile'];
						$_SESSION['conf_config'] = $conf;
						$_SESSION['conf_messages'][] = array('INFO', _("Created new profile."), $_POST['addprofile']);
						metaRefresh('confmain.php');
						exit;
					}
					else {
						$error = _("Unable to create new profile!");
					}
				}
			}
			else $error = _("Profile passwords are different or empty!");
		}
		else $error = _("Profile name is invalid!");
	}
	// rename profile
	elseif ($_POST['action'] == "rename") {
		if (preg_match("/^[a-z0-9_-]+$/i", $_POST['oldfilename']) && preg_match("/^[a-z0-9_-]+$/i", $_POST['renfilename']) && !in_array($_POST['renfilename'], getConfigProfiles())) {
			if (rename("../../config/" . $_POST['oldfilename'] . ".conf", "../../config/" . $_POST['renfilename'] . ".conf")) {
				$msg = _("Renamed profile.");
			}
			else $error = _("Could not rename file!");
			// update default profile setting if needed
			if ($cfg->default == $_POST['oldfilename']) {
				$cfg->default = $_POST['renfilename'];
				$cfg->save();
			}
		}
		else $error = _("Profile name is invalid!");
	}
	// delete profile
	elseif ($_POST['action'] == "delete") {
		if (preg_match("/^[a-z0-9_-]+$/i", $_POST['delfilename']) && @unlink("../../config/" . $_POST['delfilename'] . ".conf")) {
			$msg = _("Profile deleted.");
		}
		else $error = _("Unable to delete profile!");
	}
	// set new profile password
	elseif ($_POST['action'] == "setpass") {
		if (preg_match("/^[a-z0-9_-]+$/i", $_POST['setprofile'])) {
			if ($_POST['setpassword'] && $_POST['setpassword2'] && ($_POST['setpassword'] == $_POST['setpassword2'])) {
				$config = new LAMConfig($_POST['setprofile']);
				$config->set_Passwd($_POST['setpassword']);
				$config->save();
				$config = null;
				$msg = _("New password set successfully.");
			}
			else $error = _("Profile passwords are different or empty!");
		}
		else $error = _("Profile name is invalid!");
	}
	// set default profile
	elseif ($_POST['action'] == "setdefault") {
		if (preg_match("/^[a-z0-9_-]+$/i", $_POST['defaultfilename'])) {
			$configMain = new LAMCfgMain();
			$configMain->default = $_POST['defaultfilename'];
			$configMain->save();
			$configMain = null;
			$msg = _("New default profile set successfully.");
		}
		else $error = _("Profile name is invalid!");
	}
}


echo $_SESSION['header'];

?>

		<title>
			<?php
				echo _("Profile management");
			?>
		</title>
	<?php 
		// include all CSS files
		$cssDirName = dirname(__FILE__) . '/../../style';
		$cssDir = dir($cssDirName);
		while ($cssEntry = $cssDir->read()) {
			if (substr($cssEntry, strlen($cssEntry) - 4, 4) != '.css') continue;
			echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/" . $cssEntry . "\">\n";
		}
	?>
		<link rel="shortcut icon" type="image/x-icon" href="../../graphics/favicon.ico">
	</head>
	<body>
		<table border=0 width="100%" class="lamHeader ui-corner-all">
			<tr>
				<td align="left" height="30">
					<a class="lamHeader" href="http://www.ldap-account-manager.org/" target="new_window">&nbsp;<img src="../../graphics/logo32.png" width=24 height=24 class="align-middle" alt="LDAP Account Manager">&nbsp;&nbsp;LDAP Account Manager</a>
				</td>
				<td align="right" height=20>
					<a href="conflogin.php"><IMG alt="configuration" src="../../graphics/undo.png">&nbsp;<?php echo _("Back to profile login") ?></a>
				</td>
			</tr>
		</table>
		<br>

<?php
// include all JavaScript files
$jsDirName = dirname(__FILE__) . '/../lib';
$jsDir = dir($jsDirName);
$jsFiles = array();
while ($jsEntry = $jsDir->read()) {
	if (substr($jsEntry, strlen($jsEntry) - 3, 3) != '.js') continue;
	$jsFiles[] = $jsEntry;
}
sort($jsFiles);
foreach ($jsFiles as $jsEntry) {
	echo "<script type=\"text/javascript\" src=\"../lib/" . $jsEntry . "\"></script>\n";
}

// print messages
if (isset($error) || isset($msg)) {
	if (isset($error)) {
		StatusMessage("ERROR", $error);
	}
	if (isset($msg)) {
		StatusMessage("INFO", $msg);
	}
}

// check if config.cfg is valid
if (!isset($cfg->default)) {
	StatusMessage("ERROR", _("Please set up your master configuration file (config/config.cfg) first!"), "");
	echo "</body>\n</html>\n";
	die();
}

?>

		<br>
		<!-- form for adding/renaming/deleting profiles -->
		<form id="profileForm" name="profileForm" action="profmanage.php" method="post">
		<input type="hidden" name="action" id="action" value="none">
		<div id="passwordDialogDiv" class="hidden">
			<?PHP echo _("Master password"); ?>
			<input type="password" name="passwd">
			<?PHP
				printHelpLink(getHelp('', '236'), '236');
			?>
		</div>
		<div class="filled ui-corner-all">
<?php
$files = getConfigProfiles();

$topicSpacer = new htmlSpacer(null, '20px');

$tabindex = 1;
$container = new htmlTable();

$container->addElement(new htmlTitle(_("Profile management")), true);

// new profile
$container->addElement(new htmlSubTitle(_("Add profile")), true);
$newProfileInput = new htmlTableExtendedInputField(_("Profile name"), 'addprofile', null, '230');
$newProfileInput->setFieldSize(15);
$container->addElement($newProfileInput, true);
$profileNewPwd1 = new htmlTableExtendedInputField(_("Profile password"), 'addpassword');
$profileNewPwd1->setIsPassword(true);
$profileNewPwd1->setFieldSize(15);
$container->addElement($profileNewPwd1, true);
$profileNewPwd2 = new htmlTableExtendedInputField(_("Reenter password"), 'addpassword2');
$profileNewPwd2->setIsPassword(true);
$profileNewPwd2->setFieldSize(15);
$container->addElement($profileNewPwd2, true);
$newProfileButton = new htmlButton('btnAddProfile', _('Add'));
$newProfileButton->setOnClick("jQuery('#action').val('add');showConfirmationDialog('" . _("Add profile") . "', '" . 
	_('Ok') . "', '" . _('Cancel') . "', 'passwordDialogDiv', 'profileForm');");
$container->addElement($newProfileButton, true);
$container->addElement($topicSpacer, true);

// rename profile
$container->addElement(new htmlSubTitle(_("Rename profile")), true);
$container->addElement(new htmlTableExtendedSelect('oldfilename', $files, array(), _('Profile name'), '231'), true);
$oldProfileInput = new htmlTableExtendedInputField(_('New profile name'), 'renfilename');
$oldProfileInput->setFieldSize(15);
$container->addElement($oldProfileInput, true);
$renameProfileButton = new htmlButton('btnRenameProfile', _('Rename'));
$renameProfileButton->setOnClick("jQuery('#action').val('rename');showConfirmationDialog('" . _("Rename profile") . "', '" . 
	_('Ok') . "', '" . _('Cancel') . "', 'passwordDialogDiv', 'profileForm');");
$container->addElement($renameProfileButton, true);
$container->addElement($topicSpacer, true);

// delete profile
$container->addElement(new htmlSubTitle(_("Delete profile")), true);
$container->addElement(new htmlTableExtendedSelect('delfilename', $files, array(), _('Profile name'), '232'), true);
$deleteProfileButton = new htmlButton('btnDeleteProfile', _('Delete'));
$deleteProfileButton->setOnClick("jQuery('#action').val('delete');showConfirmationDialog('" . _("Delete profile") . "', '" . 
	_('Ok') . "', '" . _('Cancel') . "', 'passwordDialogDiv', 'profileForm');");
$container->addElement($deleteProfileButton, true);
$container->addElement($topicSpacer, true);

// set password
$container->addElement(new htmlSubTitle(_("Set profile password")), true);
$container->addElement(new htmlTableExtendedSelect('setprofile', $files, array(), _('Profile name'), '233'), true);
$profileSetPwd1 = new htmlTableExtendedInputField(_("Profile password"), 'setpassword');
$profileSetPwd1->setIsPassword(true);
$profileSetPwd1->setFieldSize(15);
$container->addElement($profileSetPwd1, true);
$profileSetPwd2 = new htmlTableExtendedInputField(_("Reenter password"), 'setpassword2');
$profileSetPwd2->setIsPassword(true);
$profileSetPwd2->setFieldSize(15);
$container->addElement($profileSetPwd2, true);
$setPasswordProfileButton = new htmlButton('btnSetPasswordProfile', _('Set profile password'));
$setPasswordProfileButton->setOnClick("jQuery('#action').val('setpass');showConfirmationDialog('" . _("Set profile password") . "', '" . 
	_('Ok') . "', '" . _('Cancel') . "', 'passwordDialogDiv', 'profileForm');");
$container->addElement($setPasswordProfileButton, true);
$container->addElement($topicSpacer, true);

// set default profile
$conf = new LAMCfgMain();
$defaultprofile = $conf->default;
$container->addElement(new htmlSubTitle(_("Change default profile")), true);
$container->addElement(new htmlTableExtendedSelect('defaultfilename', $files, array($defaultprofile), _('Profile name'), '234'), true);
$defaultProfileButton = new htmlButton('btnDefaultProfile', _('Ok'));
$defaultProfileButton->setOnClick("jQuery('#action').val('setdefault');showConfirmationDialog('" . _("Change default profile") . "', '" . 
	_('Ok') . "', '" . _('Cancel') . "', 'passwordDialogDiv', 'profileForm');");
$container->addElement($defaultProfileButton, true);
$container->addElement($topicSpacer, true);

parseHtml('', $container, array(), false, $tabindex, 'user');

?>
		</div>
		</form>
		<p><br></p>

	</body>
</html>

