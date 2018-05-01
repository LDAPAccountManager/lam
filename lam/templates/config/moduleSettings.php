<?php
namespace LAM\CONFIG;
use \moduleCache;
use \htmlSpacer;
use \htmlTable;
use \htmlButton;
use \htmlResponsiveRow;
use \htmlSubTitle;
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2009 - 2018  Roland Gruber

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
* Here the user can edit the module settings.
*
* @package configuration
* @author Roland Gruber
*/


/** Access to config functions */
include_once('../../lib/config.inc');
/** Access to account types */
include_once('../../lib/types.inc');
/** common functions */
include_once '../../lib/configPages.inc';

// start session
if (strtolower(session_module_name()) == 'files') {
	session_save_path("../../sess");
}
lam_start_session();

setlanguage();

// check if config is set
// if not: load login page
if (!isset($_SESSION['conf_config'])) {
	/** go back to login if password is invalid */
	require('conflogin.php');
	exit;
}

// check if user canceled editing
if (isset($_POST['cancelSettings'])) {
	metaRefresh("../login.php");
	exit;
}

$conf = &$_SESSION['conf_config'];

$errorsToDisplay = checkInput();

// check if button was pressed and if we have to save the settings or go to another tab
if (isset($_POST['saveSettings']) || isset($_POST['editmodules'])
	|| isset($_POST['edittypes']) || isset($_POST['generalSettingsButton'])
	|| isset($_POST['moduleSettings']) || isset($_POST['jobs'])) {
	if (sizeof($errorsToDisplay) == 0) {
		// go to final page
		if (isset($_POST['saveSettings'])) {
			metaRefresh("confsave.php");
			exit;
		}
		// go to types page
		elseif (isset($_POST['edittypes'])) {
			metaRefresh("conftypes.php");
			exit;
		}
		// go to modules page
		elseif (isset($_POST['editmodules'])) {
			metaRefresh("confmodules.php");
			exit;
		}
		// go to types page
		elseif (isset($_POST['generalSettingsButton'])) {
			metaRefresh("confmain.php");
			exit;
		}
		// go to jobs page
		elseif (isset($_POST['jobs'])) {
			metaRefresh("jobs.php");
			exit;
		}
	}
}

$allTypes = \LAM\TYPES\getTypes();

echo $_SESSION['header'];
printHeaderContents(_("LDAP Account Manager Configuration"), '../..');
echo "</head><body class=\"admin\">\n";
printJsIncludes('../..');
printConfigurationPageHeaderBar($conf);

// print error messages
for ($i = 0; $i < sizeof($errorsToDisplay); $i++) call_user_func_array('StatusMessage', $errorsToDisplay[$i]);

echo ("<form id=\"inputForm\" action=\"moduleSettings.php\" method=\"post\" autocomplete=\"off\" onSubmit=\"saveScrollPosition('inputForm')\">\n");

printConfigurationPageTabs(ConfigurationPageTab::MODULE_SETTINGS);

?>
<input type="text" name="hiddenPreventAutocomplete" autocomplete="false" class="hidden" value="">
<input type="password" name="hiddenPreventAutocompletePwd1" autocomplete="false" class="hidden" value="">
<input type="password" name="hiddenPreventAutocompletePwd2" autocomplete="false" class="hidden" value="">
<?php


// module settings
$typeManager = new \LAM\TYPES\TypeManager($conf);
$types = $typeManager->getConfiguredTypes();

// get list of scopes of modules
$scopes = array();
foreach ($types as $type) {
	$mods = $conf->get_AccountModules($type->getId());
	for ($i = 0; $i < sizeof($mods); $i++) {
		$scopes[$mods[$i]][] = $type->getId();
	}
}

// get module options
$options = getConfigOptions($scopes);
// get current setting
$old_options = $conf->get_moduleSettings();


// display module boxes
$tabindex = 1;
$modules = array_keys($options);
$_SESSION['conf_types'] = array();
for ($i = 0; $i < sizeof($modules); $i++) {
	if (empty($options[$modules[$i]])) {
		continue;
	}
	$module = moduleCache::getModule($modules[$i], 'none');
	$iconImage = $module->getIcon();
	if ($iconImage != null) {
		if (!(strpos($iconImage, 'http') === 0) && !(strpos($iconImage, '/') === 0)) {
			$iconImage = '../../graphics/' . $iconImage;
		}
	}
	$row = new htmlResponsiveRow();
	$row->add(new htmlSubTitle(getModuleAlias($modules[$i], "none"), $iconImage, null, true), 12);
	if (is_array($options[$modules[$i]])) {
		foreach ($options[$modules[$i]] as $option) {
			$row->add($option, 12);
		}
	}
	else {
		$row->add($options[$modules[$i]], 12);
	}
	$configTypes = parseHtml($modules[$i], $row, $old_options, false, $tabindex, 'none');
	$_SESSION['conf_types'] = array_merge($configTypes, $_SESSION['conf_types']);
	echo "<br>";
}

echo "<input type=\"hidden\" name=\"postAvailable\" value=\"yes\">\n";

echo "</div></div>";

$buttonContainer = new htmlTable();
$buttonContainer->addElement(new htmlSpacer(null, '10px'), true);
$saveButton = new htmlButton('saveSettings', _('Save'));
$saveButton->setIconClass('saveButton');
$buttonContainer->addElement($saveButton);
$cancelButton = new htmlButton('cancelSettings', _('Cancel'));
$cancelButton->setIconClass('cancelButton');
$buttonContainer->addElement($cancelButton, true);
$buttonContainer->addElement(new htmlSpacer(null, '10px'), true);
parseHtml(null, $buttonContainer, array(), false, $tabindex, 'none');

if ((sizeof($errorsToDisplay) == 0) && isset($_POST['scrollPositionTop']) && isset($_POST['scrollPositionLeft'])) {
	// scroll to last position
	echo '<script type="text/javascript">
		jQuery(document).ready(function() {
			jQuery(window).scrollTop(' . $_POST['scrollPositionTop'] . ');
			jQuery(window).scrollLeft('. $_POST['scrollPositionLeft'] . ');
	});
	</script>';
}

echo "</form>\n";
echo "</body>\n";
echo "</html>\n";


/**
 * Checks user input and saves the entered settings.
 *
 * @return array list of errors
 */
function checkInput() {
	if (!isset($_POST['postAvailable'])) {
		return array();
	}
	$conf = &$_SESSION['conf_config'];
	$typeManager = new \LAM\TYPES\TypeManager($conf);
	$types = $typeManager->getConfiguredTypes();

	// check module options
	// create option array to check and save
	$options = extractConfigOptionsFromPOST($_SESSION['conf_types']);

	// get list of scopes of modules
	$scopes = array();
	foreach ($types as $type) {
		$mods = $conf->get_AccountModules($type->getId());
		for ($i = 0; $i < sizeof($mods); $i++) {
			$scopes[$mods[$i]][] = $type->getId();
		}
	}
	// check options
	$errors = checkConfigOptions($scopes, $options);
	$conf->set_moduleSettings($options);
	return $errors;
}

?>




