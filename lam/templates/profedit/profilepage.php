<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2010  Roland Gruber

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
* Manages creating/changing of profiles.
*
* @package profiles
* @author Roland Gruber
*/

/** security functions */
include_once("../../lib/security.inc");
/** helper functions for profiles */
include_once("../../lib/profiles.inc");
/** access to LDAP server */
include_once("../../lib/ldap.inc");
/** access to configuration options */
include_once("../../lib/config.inc");
/** access to account modules */
include_once("../../lib/modules.inc");
/** Used to display status messages */
include_once("../../lib/status.inc");

// start session
startSecureSession();

// die if no write access
if (!checkIfWriteAccessIsAllowed()) die();

setlanguage();

// check if user is logged in, if not go to login
if (!$_SESSION['ldap'] || !$_SESSION['ldap']->server()) {
	metaRefresh("../login.php");
	exit;
}

// copy type and profile name from POST to GET
if (isset($_POST['profname'])) $_GET['edit'] = $_POST['profname'];
if (isset($_POST['accounttype'])) $_GET['type'] = $_POST['accounttype'];

// abort button was pressed
// back to profile editor
if (isset($_POST['abort'])) {
	metaRefresh("profilemain.php");
	exit;
}

$errors = array();

// save button was presed
if (isset($_POST['save'])) {
	// create option array to check and save
	$options = array();
	$opt_keys = array_keys($_SESSION['profile_types']);
	foreach ($opt_keys as $element) {
		// text fields
		if ($_SESSION['profile_types'][$element] == "text") {
			$options[$element] = array($_POST[$element]);
		}
		// checkboxes
		elseif ($_SESSION['profile_types'][$element] == "checkbox") {
			if (isset($_POST[$element]) && ($_POST[$element] == "on")) $options[$element] = array('true');
			else $options[$element] = array('false');
		}
		// dropdownbox
		elseif ($_SESSION['profile_types'][$element] == "select") {
			$options[$element] = array($_POST[$element]);
		}
		// multiselect
		elseif ($_SESSION['profile_types'][$element] == "multiselect") {
			if (isset($_POST[$element])) $options[$element] = $_POST[$element];  // value is already an array
			else $options[$element] = array();
		}
	}
	
	// remove double slashes if magic quotes are on
	if (get_magic_quotes_gpc() == 1) {
		foreach ($opt_keys as $element) {
			if (isset($options[$element][0]) && is_string($options[$element][0])) $options[$element][0] = stripslashes($options[$element][0]);
		}
	}
	
	// check options
	$errors = checkProfileOptions($_POST['accounttype'], $options);
	if (sizeof($errors) == 0) {  // input data is valid, save profile
		// save profile
		if (saveAccountProfile($options, $_POST['profname'], $_POST['accounttype'])) {
			metaRefresh('profilemain.php?savedSuccessfully=' . $_POST['profname']);
			exit();
		}
		else {
			$errors[] = array("ERROR", _("Unable to save profile!"), $_POST['profname']);
		}
	}
}

// print header
include '../main_header.php';

// print error messages if any
if (sizeof($errors) > 0) {
	echo "<br>\n";
	for ($i = 0; $i < sizeof($errors); $i++) {
		call_user_func_array('StatusMessage', $errors[$i]);
	}
}
	
// empty list of attribute types
$_SESSION['profile_types'] = array();

// check if account type is valid
$type = $_GET['type'];

// get module options
$options = getProfileOptions($type);

// load old profile or POST values if needed
$old_options = array();
if (isset($_POST['save'])) {
	$postKeys = array_keys($_POST);
	for ($i = 0; $i < sizeof($postKeys); $i++) {
		if (!is_array($_POST[$postKeys[$i]])) {
			if (get_magic_quotes_gpc() == 1) {
				$old_options[$postKeys[$i]] = array(stripslashes($_POST[$postKeys[$i]]));
			}
			else {
				$old_options[$postKeys[$i]] = array($_POST[$postKeys[$i]]);
			}
		}
		else {
			$old_options[$postKeys[$i]] = $_POST[$postKeys[$i]];
		}
	}
}
elseif (isset($_GET['edit'])) {
	$old_options = loadAccountProfile($_GET['edit'], $type);
}

// display formular
echo ("<form action=\"profilepage.php?type=$type\" method=\"post\">\n");

$profName = '';
if (isset($_GET['edit'])) {
	$profName = $_GET['edit'];
}

$tabindex = 1;

$container = new htmlTable();
$container->addElement(new htmlTitle(_("Profile editor")), true);

// general options
$dnContent = new htmlTable();
$dnContent->addElement(new htmlTableExtendedInputField(_("Profile name") . '*', 'profname', $profName, '360'), true);
$dnContent->addElement(new htmlSpacer(null, '10px'), true);
// suffix box
// get root suffix
$rootsuffix = $_SESSION['config']->get_Suffix($type);
// get subsuffixes
$suffixes = array();
$typeObj = new $type();
$possibleSuffixes = $typeObj->getSuffixList();
foreach ($possibleSuffixes as $suffix) {
	$suffixes[getAbstractDN($suffix)] = $suffix;
}
$selectedSuffix = array();
if (isset($old_options['ldap_suffix'][0])) {
	$selectedSuffix[] = $old_options['ldap_suffix'][0];
}
$suffixSelect = new htmlTableExtendedSelect('ldap_suffix', $suffixes, $selectedSuffix, _("LDAP suffix"), '361');
$suffixSelect->setHasDescriptiveElements(true);
$suffixSelect->setSortElements(false);
$suffixSelect->setRightToLeftTextDirection(true);
$dnContent->addElement($suffixSelect, true);
// RDNs
$rdns = getRDNAttributes($type);
$selectedRDN = array();
if (isset($old_options['ldap_rdn'][0])) {
	$selectedRDN[] = $old_options['ldap_rdn'][0];
}
$dnContent->addElement(new htmlTableExtendedSelect('ldap_rdn', $rdns, $selectedRDN, _("RDN identifier"), '301'), true);

$container->addElement(new htmlFieldset($dnContent, _("General settings"), '../../graphics/logo32.png'), true);
$container->addElement(new htmlSpacer(null, '15px'), true);

$_SESSION['profile_types'] = parseHtml(null, $container, $old_options, false, $tabindex, $type);

// display module options
$modules = array_keys($options);
for ($m = 0; $m < sizeof($modules); $m++) {
	// ignore modules without options
	if (sizeof($options[$modules[$m]]) < 1) continue;
	$module = new $modules[$m]($type);
	$icon = $module->getIcon();
	if ($icon != null) {
		$icon = '../../graphics/' . $icon;
	}
	$container = new htmlTable();
	$container->addElement(new htmlFieldset($options[$modules[$m]], getModuleAlias($modules[$m], $type), $icon), true);
	$container->addElement(new htmlSpacer(null, '15px'), true);
	$_SESSION['profile_types'] = array_merge($_SESSION['profile_types'], parseHtml($modules[$m], $container, $old_options, false, $tabindex, $type));
}

// profile name and submit/abort buttons
$buttonTable = new htmlTable();
$saveButton = new htmlButton('save', _('Save'));
$saveButton->setIconClass('saveButton');
$buttonTable->addElement($saveButton);
$cancelButton = new htmlButton('abort', _('Cancel'));
$cancelButton->setIconClass('cancelButton');
$buttonTable->addElement($cancelButton);
$buttonTable->addElement(new htmlHiddenInput('accounttype', $type));

$_SESSION['profile_types'] = array_merge($_SESSION['profile_types'], parseHtml(null, $buttonTable, $old_options, false, $tabindex, $type));

?>
<script type="text/javascript">
	jQuery(document).ready(function() {
		var maxWidth = 0;
		jQuery('fieldset').each(function() {
			if (jQuery(this).width() > maxWidth) {
				maxWidth = jQuery(this).width();
			};
		});
		jQuery('fieldset').each(function() {
			jQuery(this).css({'width': maxWidth});
		});
	});
</script>
<?php
echo ("</form>\n");
include '../main_footer.php';

?>
