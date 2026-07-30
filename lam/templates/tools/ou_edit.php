<?php

namespace LAM\TOOLS\OU_EDIT;

use \htmlSpacer;
use \htmlOutputText;
use \htmlButton;
use \htmlHiddenInput;
use \htmlTitle;
use \htmlSubTitle;
use \htmlStatusMessage;
use \htmlResponsiveRow;
use \htmlResponsiveSelect;
use \htmlResponsiveInputField;
use \htmlGroup;
use LAM\TYPES\TypeManager;

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
 * This is an editor for organizational units.
 *
 * @author Roland Gruber
 * @package tools
 */

/** security functions */
include_once __DIR__ . "/../../lib/security.inc";
/** access to configuration data */
include_once __DIR__ . "/../../lib/config.inc";
/** access LDAP server */
include_once __DIR__ . "/../../lib/ldap.inc";
/** used to print status messages */
include_once __DIR__ . "/../../lib/status.inc";

// start session
startSecureSession();
enforceUserIsLoggedIn();

// die if no write access
if (!checkIfWriteAccessIsAllowed()) {
	die();
}

checkIfToolIsActive('toolOUEditor');

setlanguage();

if (!empty($_POST)) {
	validateSecurityToken();
}

$error = null;
$message = null;

$optionsToDelete = [];
$optionsToInsert = [];

/**
 * Refreshes the possible OUs.
 *
 * @param array<string, array<string, string>> $optionsToInsert OUs that can be used for adding
 * @param array<string, array<string, string>> $optionsToDelete OUs that can be deleted
 */
function refreshOus(array &$optionsToInsert, array &$optionsToDelete): void {
	$typeManager = new TypeManager();
	$typeList = $typeManager->getConfiguredTypes();
	$types = [];
	foreach ($typeList as $type) {
		if ($type->isHidden() || !checkIfWriteAccessIsAllowed($type->getId())) {
			continue;
		}
		$types[$type->getId()] = $type->getAlias();
	}
	natcasesort($types);
	foreach ($types as $typeId => $title) {
		$type = $typeManager->getConfiguredType($typeId);
		if ($type === null) {
			continue;
		}
		$elements = [];
		$units = searchLDAP($type->getSuffix(), '(|(objectclass=organizationalunit)(objectclass=organization))', ['dn']);
		foreach ($units as $unit) {
			if (is_string($unit['dn'])) {
				$elements[getAbstractDN($unit['dn'])] = $unit['dn'];
			}
		}
		if (!empty($elements)) {
			$optionsToDelete[$title] = $elements;
			uasort($optionsToDelete[$title], compareDn(...));
		}
		$optionsToInsert[$title] = $elements;
		if (empty($optionsToInsert[$title])) {
			$optionsToInsert[$title] = [getAbstractDN($type->getSuffix()) => $type->getSuffix()];
		}
		uasort($optionsToInsert[$title], compareDn(...));
	}
}

refreshOus($optionsToInsert, $optionsToDelete);

// check if deletion was canceled
if (isset($_POST['abort'])) {
	display_main(null, null, $optionsToInsert, $optionsToDelete);
	exit;
}

// check if the submit button was pressed
if (isset($_POST['createOU']) || isset($_POST['deleteOU'])) {
	$validDeletableDns = flattenArray($optionsToDelete);
	// new ou
	if (isset($_POST['createOU'])) {
		// create ou if valid
		$validParentDns = flattenArray($optionsToInsert);
		if (preg_match("/^[a-z0-9 _\\-]+$/i", $_POST['newOU']) && in_array_ignore_case($_POST['parentOU'], $validParentDns)) {
			// check if ou already exists
			$new_dn = "ou=" . ldap_escape($_POST['newOU'], '', LDAP_ESCAPE_DN) . "," . $_POST['parentOU'];
			$found = ldapGetDN($new_dn);
			if ($found === null) {
				// add new ou
				$ou['objectClass'][] = "organizationalunit";
				$ou['ou'][] = $_POST['newOU'];
				$ret = ldapAddNewEntry($_SESSION['ldap']->server(), $new_dn, $ou);
				if ($ret) {
					$message = _("New OU created successfully.");
					refreshOus($optionsToInsert, $optionsToDelete);
				}
				else {
					$error = _("Unable to create new OU!");
				}
			}
			else {
				$error = _("OU already exists!");
			}
		}
		// show errormessage if ou is invalid
		else {
			$error = _("OU is invalid!") . "<br>" . htmlspecialchars($_POST['newOU']);
		}
	}
	// delete ou, user was sure
	elseif (isset($_POST['deleteOU']) && isset($_POST['sure']) && in_array_ignore_case($_POST['deletename'], $validDeletableDns)) {
		$ret = ldapDeleteEntry($_SESSION['ldap']->server(), $_POST['deletename']);
		if ($ret) {
			$message = _("OU deleted successfully.");
			refreshOus($optionsToInsert, $optionsToDelete);
		}
		else {
			$error = _("Unable to delete OU!");
		}
	}
	// ask if the user is sure to delete
	elseif (isset($_POST['deleteOU']) && in_array_ignore_case($_POST['deleteableOU'], $validDeletableDns)) {
		// check for subentries
		$sr = ldap_list($_SESSION['ldap']->server(), $_POST['deleteableOU'], "(objectClass=*)", [""]);
		if ($sr === false) {
			$error = _("OU is not empty or invalid!");
		}
		else {
			$info = ldap_get_entries($_SESSION['ldap']->server(), $sr);
			if (($info !== false) && ($info['count'] === 0)) {
				// print header
				include_once __DIR__ . '/../../lib/adminHeader.inc';
				echo '<div class="smallPaddingContent">';
				echo "<form action=\"ou_edit.php\" method=\"post\">\n";
				$container = new htmlResponsiveRow();
				$label = new htmlOutputText(_("Do you really want to delete this OU?"));
				$label->colspan = 5;
				$container->add($label, 12);
				$container->addVerticalSpacer('1rem');
				$dnLabel = new htmlOutputText(getAbstractDN($_POST['deleteableOU']));
				$dnLabel->colspan = 5;
				$container->add($dnLabel, 12);
				$container->addVerticalSpacer('1rem');
				$buttonGroup = new htmlGroup();
				$deleteButton = new htmlButton('sure', _("Delete"));
				$deleteButton->setCSSClasses(['lam-danger']);
				$buttonGroup->addElement($deleteButton);
				$buttonGroup->addElement(new htmlSpacer('0.5rem', null));
				$buttonGroup->addElement(new htmlButton('abort', _("Cancel")));
				$container->add($buttonGroup, 12);
				$container->add(new htmlHiddenInput('deleteOU', 'submit'), 12);
				$container->add(new htmlHiddenInput('deletename', $_POST['deleteableOU']), 12);
				addSecurityTokenToMetaHTML($container);
				parseHtml(null, $container, [], false);
				echo "</form>";
				echo '</div>';
				include_once __DIR__ . '/../../lib/adminFooter.inc';
				exit();
			}
			else {
				$error = _("OU is not empty or invalid!");
			}
		}
	}
}

display_main($message, $error, $optionsToInsert, $optionsToDelete);

/**
 * Displays the main page of the OU editor
 *
 * @param string|null $message info message
 * @param string|null $error error message
 * @param array<string, array<string, string>> $optionsToInsert options where new OU can be inserted
 * @param array<string, array<string, string>> $optionsToDelete OUs that can be deleted
 */
function display_main(?string $message, ?string $error, array $optionsToInsert, array $optionsToDelete): void {
	// display main page
	include_once __DIR__ . '/../../lib/adminHeader.inc';
	echo '<div class="smallPaddingContent">';
	echo "<form action=\"ou_edit.php\" method=\"post\">\n";

	$container = new htmlResponsiveRow();
	$container->add(new htmlTitle(_("OU editor")), 12);
	if ($error !== null) {
		$msg = new htmlStatusMessage("ERROR", "", $error);
		$msg->colspan = 5;
		$container->add($msg, 12);
	}
	elseif ($message !== null) {
		$msg = new htmlStatusMessage("INFO", "", $message);
		$msg->colspan = 5;
		$container->add($msg, 12);
	}

	if (!empty($optionsToInsert)) {
		// new OU
		$container->add(new htmlSubTitle(_("New organisational unit")));
		$parentOUSelect = new htmlResponsiveSelect('parentOU', $optionsToInsert, [], _('Parent DN'), '601');
		$parentOUSelect->setContainsOptgroups(true);
		$parentOUSelect->setHasDescriptiveElements(true);
		$parentOUSelect->setRightToLeftTextDirection(true);
		$parentOUSelect->setSortElements(false);
		$container->add($parentOUSelect);
		$container->add(new htmlResponsiveInputField(_('Name'), 'newOU'));
		$container->addLabel(new htmlOutputText('&nbsp;', false));
		$container->addField(new htmlButton('createOU', _("Ok")));
		$container->addVerticalSpacer('2rem');
	}
	if (!empty($optionsToDelete)) {
		// delete OU
		$container->add(new htmlSubTitle(_("Delete organisational unit")));
		$deleteableOUSelect = new htmlResponsiveSelect('deleteableOU', $optionsToDelete, [], _('Organisational unit'), '602');
		$deleteableOUSelect->setContainsOptgroups(true);
		$deleteableOUSelect->setHasDescriptiveElements(true);
		$deleteableOUSelect->setRightToLeftTextDirection(true);
		$deleteableOUSelect->setSortElements(false);
		$container->add($deleteableOUSelect);
		$container->addLabel(new htmlOutputText('&nbsp;', false));
		$container->addField(new htmlButton('deleteOU', _("Ok")));
	}

	addSecurityTokenToMetaHTML($container);
	parseHtml(null, $container, [], false);
	echo "</form>\n";
	echo '</div>';
	include_once __DIR__ . '/../../lib/adminFooter.inc';
}
