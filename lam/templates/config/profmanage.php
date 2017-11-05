<?php
namespace LAM\CONFIG;
use \LAMCfgMain;
use \LAMConfig;
use \htmlStatusMessage;
use \htmlResponsiveRow;
use \htmlTitle;
use \htmlSubTitle;
use \htmlResponsiveInputField;
use \htmlResponsiveSelect;
use \htmlButton;
use \htmlOutputText;
use \htmlHiddenInput;
use \htmlDiv;
use \htmlLink;
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2017  Roland Gruber

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
$files = getConfigProfiles();

// check if submit button was pressed
if (isset($_POST['action'])) {
	// check master password
	if (!$cfg->checkPassword($_POST['passwd'])) {
		$error = _("Master password is wrong!");
	}
	// add new profile
	elseif ($_POST['action'] == "add") {
		// check profile password
		if ($_POST['addpassword'] && $_POST['addpassword2'] && ($_POST['addpassword'] == $_POST['addpassword2'])) {
			$result = createConfigProfile($_POST['addprofile'], $_POST['addpassword'], $_POST['addTemplate']);
			if ($result === true) {
				$_SESSION['conf_isAuthenticated'] = $_POST['addprofile'];
				$_SESSION['conf_config'] = new LAMConfig($_POST['addprofile']);
				$_SESSION['conf_messages'][] = array('INFO', _("Created new profile."), $_POST['addprofile']);
				metaRefresh('confmain.php');
				exit;
			}
			else {
				$error = $result;
			}
		}
		else {
			$error = _("Profile passwords are different or empty!");
		}
	}
	// rename profile
	elseif ($_POST['action'] == "rename") {
		if (preg_match("/^[a-z0-9_-]+$/i", $_POST['oldfilename']) && preg_match("/^[a-z0-9_-]+$/i", $_POST['renfilename']) && !in_array($_POST['renfilename'], getConfigProfiles())) {
			if (rename("../../config/" . $_POST['oldfilename'] . ".conf", "../../config/" . $_POST['renfilename'] . ".conf")) {
			    // rename pdf and profiles folder
			    rename("../../config/profiles/" . $_POST['oldfilename'], "../../config/profiles/" . $_POST['renfilename']);
			    rename("../../config/pdf/" . $_POST['oldfilename'], "../../config/pdf/" . $_POST['renfilename']);
				// rename sqlite database if any
				if (file_exists("../../config/" . $_POST['oldfilename'] . ".sqlite")) {
					rename("../../config/" . $_POST['oldfilename'] . ".sqlite", "../../config/" . $_POST['renfilename'] . ".sqlite");
				}
				$msg = _("Renamed profile.");
			}
			else $error = _("Could not rename file!");
			// update default profile setting if needed
			if ($cfg->default == $_POST['oldfilename']) {
				$cfg->default = $_POST['renfilename'];
				$cfg->save();
			}
			// reread profile list
			$files = getConfigProfiles();
		}
		else $error = _("Profile name is invalid!");
	}
	// delete profile
	elseif ($_POST['action'] == "delete") {
		if (deleteConfigProfile($_POST['delfilename']) == null) {
			$msg = _("Profile deleted.");
			// update default profile setting if needed
			if ($cfg->default == $_POST['delfilename']) {
				$filesNew = array_delete(array($_POST['delfilename']), $files);
				if (sizeof($filesNew) > 0) {
					sort($filesNew);
					$cfg->default = $filesNew[0];
					$cfg->save();
				}
			}
			// reread profile list
			$files = getConfigProfiles();
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
printHeaderContents(_("Profile management"), '../..');
?>
	</head>
	<body class="admin">
		<table border=0 width="100%" class="lamHeader ui-corner-all">
			<tr>
				<td align="left" height="30">
					<a class="lamLogo" href="http://www.ldap-account-manager.org/" target="new_window">LDAP Account Manager</a>
				</td>
			</tr>
		</table>

<?php
// include all JavaScript files
printJsIncludes('../..');

?>
		<br>
		<!-- form for adding/renaming/deleting profiles -->
		<form id="profileForm" name="profileForm" action="profmanage.php" method="post">
<?php
$tabindex = 1;

$row = new htmlResponsiveRow();

// print messages
if (isset($error)) {
	$row->add(new htmlStatusMessage('ERROR', $error), 12);
	$row->addVerticalSpacer('1rem');
}
if (isset($msg)) {
	$row->add(new htmlStatusMessage('INFO', $msg), 12);
	$row->addVerticalSpacer('1rem');
}

$box = new htmlResponsiveRow();
$box->add(new htmlTitle(_("Profile management")), 12);

// new profile
$box->add(new htmlSubTitle(_("Add profile")), 12);
$newProfileInput = new htmlResponsiveInputField(_("Profile name"), 'addprofile', null, '230');
$box->add($newProfileInput, 12);
$profileNewPwd1 = new htmlResponsiveInputField(_("Profile password"), 'addpassword');
$profileNewPwd1->setIsPassword(true);
$box->add($profileNewPwd1, 12);
$profileNewPwd2 = new htmlResponsiveInputField(_("Reenter password"), 'addpassword2');
$profileNewPwd2->setIsPassword(true);
$profileNewPwd2->setSameValueFieldID('addpassword');
$box->add($profileNewPwd2, 12);
$existing = array();
foreach ($files as $file) {
	$existing[$file] = $file . '.conf';
}
$builtIn = array();
foreach (getConfigTemplates() as $file) {
	$builtIn[$file] = $file . '.conf.sample';
}
$templates = array(
	_('Built-in templates') => $builtIn,
	_('Existing server profiles') => $existing,
);
$addTemplateSelect = new htmlResponsiveSelect('addTemplate', $templates, array('unix.conf.sample'), _('Template'), '267');
$addTemplateSelect->setContainsOptgroups(true);
$addTemplateSelect->setHasDescriptiveElements(true);
$box->add($addTemplateSelect, 12);
$box->addVerticalSpacer('0.5rem');
$newProfileButton = new htmlButton('btnAddProfile', _('Add'));
$newProfileButton->setOnClick("jQuery('#action').val('add');showConfirmationDialog('" . _("Add profile") . "', '" .
	_('Ok') . "', '" . _('Cancel') . "', 'passwordDialogDiv', 'profileForm', null); document.getElementById('passwd').focus();");
$box->addLabel($newProfileButton);
$box->add(new htmlOutputText(''), 0, 6);

// rename profile
$box->add(new htmlSubTitle(_("Rename profile")), 12);
$box->add(new htmlResponsiveSelect('oldfilename', $files, array(), _('Profile name'), '231'), 12);
$oldProfileInput = new htmlResponsiveInputField(_('New profile name'), 'renfilename');
$box->add($oldProfileInput, 12);
$box->addVerticalSpacer('0.5rem');
$renameProfileButton = new htmlButton('btnRenameProfile', _('Rename'));
$renameProfileButton->setOnClick("jQuery('#action').val('rename');showConfirmationDialog('" . _("Rename profile") . "', '" .
	_('Ok') . "', '" . _('Cancel') . "', 'passwordDialogDiv', 'profileForm', null); document.getElementById('passwd').focus();");
$box->addLabel($renameProfileButton);
$box->add(new htmlOutputText(''), 0, 6);

// delete profile
$box->add(new htmlSubTitle(_("Delete profile")), 12);
$box->add(new htmlResponsiveSelect('delfilename', $files, array(), _('Profile name'), '232'), 12);
$box->addVerticalSpacer('0.5rem');
$deleteProfileButton = new htmlButton('btnDeleteProfile', _('Delete'));
$deleteProfileButton->setOnClick("jQuery('#action').val('delete');showConfirmationDialog('" . _("Delete profile") . "', '" .
	_('Ok') . "', '" . _('Cancel') . "', 'passwordDialogDiv', 'profileForm', null); document.getElementById('passwd').focus();");
$box->addLabel($deleteProfileButton);
$box->add(new htmlOutputText(''), 0, 6);

// set password
$box->add(new htmlSubTitle(_("Set profile password")), 12);
$box->add(new htmlResponsiveSelect('setprofile', $files, array(), _('Profile name'), '233'), 12);
$profileSetPwd1 = new htmlResponsiveInputField(_("Profile password"), 'setpassword');
$profileSetPwd1->setIsPassword(true);
$box->add($profileSetPwd1, 12);
$profileSetPwd2 = new htmlResponsiveInputField(_("Reenter password"), 'setpassword2');
$profileSetPwd2->setIsPassword(true);
$profileSetPwd2->setSameValueFieldID('setpassword');
$box->add($profileSetPwd2, 12);
$box->addVerticalSpacer('0.5rem');
$setPasswordProfileButton = new htmlButton('btnSetPasswordProfile', _('Set profile password'));
$setPasswordProfileButton->setOnClick("jQuery('#action').val('setpass');showConfirmationDialog('" . _("Set profile password") . "', '" .
		_('Ok') . "', '" . _('Cancel') . "', 'passwordDialogDiv', 'profileForm', null); document.getElementById('passwd').focus();");
$box->addLabel($setPasswordProfileButton, 12);
$box->add(new htmlOutputText(''), 0, 6);


// set default profile
$conf = new LAMCfgMain();
$defaultprofile = $conf->default;
$box->add(new htmlSubTitle(_("Change default profile")), 12);
$box->add(new htmlResponsiveSelect('defaultfilename', $files, array($defaultprofile), _('Profile name'), '234'), 12);
$box->addVerticalSpacer('0.5rem');
$defaultProfileButton = new htmlButton('btnDefaultProfile', _('Ok'));
$defaultProfileButton->setOnClick("jQuery('#action').val('setdefault');showConfirmationDialog('" . _("Change default profile") . "', '" .
	_('Ok') . "', '" . _('Cancel') . "', 'passwordDialogDiv', 'profileForm', null); document.getElementById('passwd').focus();");
$box->addLabel($defaultProfileButton);
$box->add(new htmlOutputText(''), 0, 6);

$boxDiv = new htmlDiv(null, $box);
$boxDiv->setCSSClasses(array('ui-corner-all', 'roundedShadowBox', 'limitWidth'));
$row->add($boxDiv, 12);

$row->add(new htmlHiddenInput('action', 'none'), 12);

// dialog
$dialogDivContent = new htmlResponsiveRow();
$masterPassword = new htmlResponsiveInputField(_("Master password"), 'passwd', '', '236');
$masterPassword->setIsPassword(true);
$dialogDivContent->add($masterPassword, 12);
$dialogDiv = new htmlDiv('passwordDialogDiv', $dialogDivContent);
$dialogDiv->setCSSClasses(array('hidden'));
$row->add($dialogDiv, 12);

$row->addVerticalSpacer('2rem');
$backLink = new htmlLink(_("Back to profile login"), 'conflogin.php', '../../graphics/undo.png');
$row->add($backLink, 12, 12, 12, 'text-left');

parseHtml('', new htmlDiv(null, $row, array('centeredTable')), array(), false, $tabindex, 'user');

?>
		</form>
		<p><br></p>

	</body>
</html>

