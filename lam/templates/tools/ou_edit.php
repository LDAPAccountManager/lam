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
* This is an editor for organizational units.
*
* @author Roland Gruber
* @package tools
*/

/** security functions */
include_once(__DIR__ . "/../../lib/security.inc");
/** access to configuration data */
include_once(__DIR__ . "/../../lib/config.inc");
/** access LDAP server */
include_once(__DIR__ . "/../../lib/ldap.inc");
/** used to print status messages */
include_once(__DIR__ . "/../../lib/status.inc");

// start session
startSecureSession();
enforceUserIsLoggedIn();

// die if no write access
if (!checkIfWriteAccessIsAllowed()) die();

checkIfToolIsActive('toolOUEditor');

setlanguage();

if (!empty($_POST)) {
	validateSecurityToken();
}

// check if deletion was canceled
if (isset($_POST['abort'])) {
	display_main(null, null);
	exit;
}

$error = null;
$message = null;

// check if submit button was pressed
if (isset($_POST['createOU']) || isset($_POST['deleteOU'])) {
	// new ou
	if (isset($_POST['createOU'])) {
		// create ou if valid
		if (preg_match("/^[a-z0-9 _\\-]+$/i", $_POST['newOU'])) {
			// check if ou already exists
			$new_dn = "ou=" . $_POST['newOU'] . "," . $_POST['parentOU'];
			$found = ldapGetDN($new_dn);
			if ($found === null) {
				// add new ou
				$ou = array();
				$ou['objectClass'] = "organizationalunit";
				$ou['ou'] = $_POST['newOU'];
				$ret = @ldap_add($_SESSION['ldap']->server(), $new_dn, $ou);
				if ($ret) {
					$message = _("New OU created successfully.");
				}
				else {
					$error = _("Unable to create new OU!");
				}
			}
			else $error = _("OU already exists!");
		}
		// show errormessage if ou is invalid
		else {
			$error = _("OU is invalid!") . "<br>" . htmlspecialchars($_POST['newOU']);
		}
	}
	// delete ou, user was sure
	elseif (isset($_POST['deleteOU']) && isset($_POST['sure'])) {
		$ret = ldap_delete($_SESSION['ldap']->server(), $_POST['deletename']);
		if ($ret) {
			$message = _("OU deleted successfully.");
		}
		else {
			$error = _("Unable to delete OU!");
		}
	}
	// ask if user is sure to delete
	elseif (isset($_POST['deleteOU'])) {
		// check for sub entries
		$sr = ldap_list($_SESSION['ldap']->server(), $_POST['deleteableOU'], "ObjectClass=*", array(""));
		$info = ldap_get_entries($_SESSION['ldap']->server(), $sr);
		if ($sr && $info['count'] == 0) {
			// print header
			include '../../lib/adminHeader.inc';
			echo '<div class="smallPaddingContent">';
			echo "<form action=\"ou_edit.php\" method=\"post\">\n";
			$tabindex = 1;
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
			$buttonGroup->addElement(new htmlButton('sure', _("Delete")));
			$buttonGroup->addElement(new htmlSpacer('0.5rem', null));
			$buttonGroup->addElement(new htmlButton('abort', _("Cancel")));
			$container->add($buttonGroup, 12);
			$container->add(new htmlHiddenInput('deleteOU', 'submit'), 12);
			$container->add(new htmlHiddenInput('deletename', $_POST['deleteableOU']), 12);
			addSecurityTokenToMetaHTML($container);
			parseHtml(null, $container, array(), false, $tabindex, 'user');
			echo "</form>";
			echo '</div>';
			include '../../lib/adminFooter.inc';
			exit();
		}
		else {
			$error = _("OU is not empty or invalid!");
		}
	}
}

display_main($message, $error);

/**
 * Displays the main page of the OU editor
 *
 * @param String $message info message
 * @param String $error error message
 */
function display_main($message, $error) {
	// display main page
	include __DIR__ . '/../../lib/adminHeader.inc';
	echo '<div class="smallPaddingContent">';
	echo "<form action=\"ou_edit.php\" method=\"post\">\n";

	$tabindex = 1;
	$container = new htmlResponsiveRow();
	$container->add(new htmlTitle(_("OU editor")), 12);
	if (isset($error)) {
		$msg = new htmlStatusMessage("ERROR", "", $error);
		$msg->colspan = 5;
		$container->add($msg, 12);
	}
	elseif (isset($message)) {
		$msg = new htmlStatusMessage("INFO", "", $message);
		$msg->colspan = 5;
		$container->add($msg, 12);
	}

	$typeManager = new \LAM\TYPES\TypeManager();
	$typeList = $typeManager->getConfiguredTypes();
	$types = array();
	foreach ($typeList as $type) {
		if ($type->isHidden() || !checkIfWriteAccessIsAllowed($type->getId())) {
			continue;
		}
		$types[$type->getId()] = $type->getAlias();
	}
	natcasesort($types);
	$options = array();
	foreach ($types as $typeId => $title) {
		$type = $typeManager->getConfiguredType($typeId);
		$elements = array();
		$units = searchLDAP($type->getSuffix(), '(objectclass=organizationalunit)', array('dn'));
		foreach ($units as $unit) {
			$elements[getAbstractDN($unit['dn'])] = $unit['dn'];
		}
		$options[$title] = $elements;
	}

	if (!empty($options)) {
		// new OU
		$container->add(new htmlSubTitle(_("New organisational unit")), 12);
		$parentOUSelect = new htmlResponsiveSelect('parentOU', $options, array(), _('Parent DN'), '601');
		$parentOUSelect->setContainsOptgroups(true);
		$parentOUSelect->setHasDescriptiveElements(true);
		$parentOUSelect->setRightToLeftTextDirection(true);
		$parentOUSelect->setSortElements(false);
		$container->add($parentOUSelect, 12);
		$container->add(new htmlResponsiveInputField(_('Name'), 'newOU'), 12);
		$container->addLabel(new htmlOutputText('&nbsp;', false));
		$container->addField(new htmlButton('createOU', _("Ok")));
		$container->addVerticalSpacer('2rem');

		// delete OU
		$container->add(new htmlSubTitle(_("Delete organisational unit")), 12);
		$deleteableOUSelect = new htmlResponsiveSelect('deleteableOU', $options, array(), _('Organisational unit'), '602');
		$deleteableOUSelect->setContainsOptgroups(true);
		$deleteableOUSelect->setHasDescriptiveElements(true);
		$deleteableOUSelect->setRightToLeftTextDirection(true);
		$deleteableOUSelect->setSortElements(false);
		$container->add($deleteableOUSelect, 12);
		$container->addLabel(new htmlOutputText('&nbsp;', false));
		$container->addField(new htmlButton('deleteOU', _("Ok")));
	}

	addSecurityTokenToMetaHTML($container);
	parseHtml(null, $container, array(), false, $tabindex, 'user');
	echo "</form>\n";
	echo '</div>';
	include __DIR__ . '/../../lib/adminFooter.inc';
}
