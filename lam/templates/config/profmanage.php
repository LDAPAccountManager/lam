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
use LAMException;
use ServerProfilePersistenceManager;

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
if (isFileBasedSession()) {
	session_save_path("../../sess");
}
lam_start_session();

setlanguage();


$cfg = new LAMCfgMain();
$serverProfilePersistenceManager = new ServerProfilePersistenceManager();
$files = array();
try {
	$files = $serverProfilePersistenceManager->getProfiles();
}
catch (LAMException $e) {
	logNewMessage(LOG_ERR, 'Unable to read server profiles: ' . $e->getTitle());
}

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
			try {
				$serverProfilePersistenceManager->createProfileFromTemplate($_POST['addprofile'], $_POST['addTemplate'], $_POST['addpassword']);
				$_SESSION['conf_isAuthenticated'] = $_POST['addprofile'];
				$_SESSION['conf_config'] = $serverProfilePersistenceManager->loadProfile($_POST['addprofile']);
				$_SESSION['conf_messages'][] = array('INFO', _("Created new profile."), $_POST['addprofile']);
				metaRefresh('confmain.php');
				exit;
			} catch (LAMException $e) {
			    $error = $e->getTitle();
			}
		}
		else {
			$error = _("Profile passwords are different or empty!");
		}
	}
	// rename profile
	elseif ($_POST['action'] == "rename") {
        try {
            $serverProfilePersistenceManager->renameProfile($_POST['oldfilename'], $_POST['renfilename']);
            // reread profile list
            $files = $serverProfilePersistenceManager->getProfiles();
            $msg = _("Renamed profile.");
        } catch (LAMException $e) {
            logNewMessage(LOG_ERR, 'Unable to read server profiles: ' . $e->getTitle());
            $error = $e->getTitle();
        }
	}
	// delete profile
	elseif ($_POST['action'] == "delete") {
        try {
            $serverProfilePersistenceManager->deleteProfile($_POST['delfilename']);
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
            $files = $serverProfilePersistenceManager->getProfiles();
	        $msg = _("Profile deleted.");
        } catch (LAMException $e) {
            $error = _("Unable to delete profile!");
            logNewMessage(LOG_ERR, 'Unable to delete server profile: ' . $e->getTitle());
        }
	}
	// set new profile password
	elseif ($_POST['action'] == "setpass") {
		if (preg_match("/^[a-z0-9_-]+$/i", $_POST['setprofile'])) {
			if ($_POST['setpassword'] && $_POST['setpassword2'] && ($_POST['setpassword'] == $_POST['setpassword2'])) {
				try {
					$config = $serverProfilePersistenceManager->loadProfile($_POST['setprofile']);
					$config->set_Passwd($_POST['setpassword']);
					$serverProfilePersistenceManager->saveProfile($config, $_POST['setprofile']);
					$msg = _("New password set successfully.");
				} catch (LAMException $e) {
				    $error = $e->getTitle();
				}
				$config = null;
			}
			else {
			    $error = _("Profile passwords are different or empty!");
			}
		}
		else {
		    $error = _("Profile name is invalid!");
		}
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
		else {
		    $error = _("Profile name is invalid!");
		}
	}
}


echo $_SESSION['header'];
printHeaderContents(_("Profile management"), '../..');
?>
	</head>
	<body>
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
$box->add($newProfileInput);
$profileNewPwd1 = new htmlResponsiveInputField(_("Profile password"), 'addpassword');
$profileNewPwd1->setIsPassword(true);
$box->add($profileNewPwd1);
$profileNewPwd2 = new htmlResponsiveInputField(_("Reenter password"), 'addpassword2');
$profileNewPwd2->setIsPassword(true);
$profileNewPwd2->setSameValueFieldID('addpassword');
$box->add($profileNewPwd2);
$existing = array();
foreach ($files as $file) {
	$existing[$file] = $file;
}
$builtIn = array();
foreach ($serverProfilePersistenceManager->getConfigTemplates() as $file) {
	$builtIn[$file] = $file . '.sample';
}
$templates = array(
	_('Built-in templates') => $builtIn,
	_('Existing server profiles') => $existing,
);
$addTemplateSelect = new htmlResponsiveSelect('addTemplate', $templates, array('unix.sample'), _('Template'), '267');
$addTemplateSelect->setContainsOptgroups(true);
$addTemplateSelect->setHasDescriptiveElements(true);
$box->add($addTemplateSelect, 12);
$box->addVerticalSpacer('0.5rem');
$newProfileButton = new htmlButton('btnAddProfile', _('Add'));
$newProfileButton->setCSSClasses(array('lam-primary'));
$newProfileButton->setOnClick("document.getElementById('action').value = 'add';"
    . "window.lam.dialog.requestPasswordAndSendForm('" . _("Add profile") . "', '" .
	_('Ok') . "', '" . _('Cancel') . "', '" . _('Master password') . "', 'passwd', 'profileForm');");
$box->addLabel($newProfileButton);
$box->add(new htmlOutputText(''), 0, 6);

// rename profile
$box->add(new htmlSubTitle(_("Rename profile")));
$box->add(new htmlResponsiveSelect('oldfilename', $files, array(), _('Profile name'), '231'), 12);
$oldProfileInput = new htmlResponsiveInputField(_('New profile name'), 'renfilename');
$box->add($oldProfileInput);
$box->addVerticalSpacer('0.5rem');
$renameProfileButton = new htmlButton('btnRenameProfile', _('Rename'));
$renameProfileButton->setCSSClasses(array('lam-secondary'));
$renameProfileButton->setOnClick("document.getElementById('action').value = 'rename';" .
    "window.lam.dialog.requestPasswordAndSendForm('" . _("Rename profile") . "', '" .
	_('Ok') . "', '" . _('Cancel') . "', '" . _('Master password') . "', 'passwd', 'profileForm');");
$box->addLabel($renameProfileButton);
$box->add(new htmlOutputText(''), 0, 6);

// delete profile
$box->add(new htmlSubTitle(_("Delete profile")), 12);
$box->add(new htmlResponsiveSelect('delfilename', $files, array(), _('Profile name'), '232'), 12);
$box->addVerticalSpacer('0.5rem');
$deleteProfileButton = new htmlButton('btnDeleteProfile', _('Delete'));
$deleteProfileButton->setCSSClasses(array('lam-danger'));
$deleteProfileButton->setOnClick("document.getElementById('action').value = 'delete';" .
    "window.lam.dialog.requestPasswordAndSendForm('" . _("Delete profile") . "', '" .
	_('Ok') . "', '" . _('Cancel') . "', '" . _('Master password') . "', 'passwd', 'profileForm');");
$box->addLabel($deleteProfileButton);
$box->add(new htmlOutputText(''), 0, 6);

// set password
$box->add(new htmlSubTitle(_("Set profile password")), 12);
$box->add(new htmlResponsiveSelect('setprofile', $files, array(), _('Profile name'), '233'), 12);
$profileSetPwd1 = new htmlResponsiveInputField(_("Profile password"), 'setpassword');
$profileSetPwd1->setIsPassword(true);
$box->add($profileSetPwd1);
$profileSetPwd2 = new htmlResponsiveInputField(_("Reenter password"), 'setpassword2');
$profileSetPwd2->setIsPassword(true);
$profileSetPwd2->setSameValueFieldID('setpassword');
$box->add($profileSetPwd2);
$box->addVerticalSpacer('0.5rem');
$setPasswordProfileButton = new htmlButton('btnSetPasswordProfile', _('Set profile password'));
$setPasswordProfileButton->setCSSClasses(array('lam-secondary'));
$setPasswordProfileButton->setOnClick("document.getElementById('action').value = 'setpass';" .
    "window.lam.dialog.requestPasswordAndSendForm('" . _("Set profile password") . "', '" .
	_('Ok') . "', '" . _('Cancel') . "', '" . _('Master password') . "', 'passwd', 'profileForm');");
$box->addLabel($setPasswordProfileButton, 12);
$box->add(new htmlOutputText(''), 0, 6);


// set default profile
$conf = new LAMCfgMain();
$defaultProfile = $conf->default;
$box->add(new htmlSubTitle(_("Change default profile")));
$box->add(new htmlResponsiveSelect('defaultfilename', $files, array($defaultProfile), _('Profile name'), '234'), 12);
$box->addVerticalSpacer('0.5rem');
$defaultProfileButton = new htmlButton('btnDefaultProfile', _('Ok'));
$defaultProfileButton->setCSSClasses(array('lam-secondary'));
$defaultProfileButton->setOnClick("document.getElementById('action').value = 'setdefault';" .
    "window.lam.dialog.requestPasswordAndSendForm('" . _("Change default profile") . "', '" .
	_('Ok') . "', '" . _('Cancel') . "', '" . _('Master password') . "', 'passwd', 'profileForm');");
$box->addLabel($defaultProfileButton);
$box->add(new htmlOutputText(''), 0, 6);

$boxDiv = new htmlDiv(null, $box);
$boxDiv->setCSSClasses(array('ui-corner-all', 'roundedShadowBox', 'limitWidth'));
$row->add($boxDiv);

$row->add(new htmlHiddenInput('action', 'none'));

$row->addVerticalSpacer('2rem');
$backLink = new htmlLink(_("Back to profile login"), 'conflogin.php');
$row->add($backLink, 12, 12, 12, 'text-left');

parseHtml('', new htmlDiv(null, $row, array('centeredTable')), array(), false, $tabindex, 'user');

?>
		</form>
		<p><br></p>

	</body>
</html>

