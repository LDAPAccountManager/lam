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
* This is an editor for organizational units.
*
* @author Roland Gruber
* @package tools
*/

/** security functions */
include_once("../lib/security.inc");
/** access to configuration data */
include_once("../lib/config.inc");
/** access LDAP server */
include_once("../lib/ldap.inc");
/** used to print status messages */
include_once("../lib/status.inc");

// start session
startSecureSession();

// die if no write access
if (!checkIfWriteAccessIsAllowed()) die();

setlanguage();

$types = $_SESSION['config']->get_ActiveTypes();

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
			if ($found == null) {
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
		$ret = @ldap_delete($_SESSION['ldap']->server(), $_POST['deletename']);
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
		$sr = @ldap_list($_SESSION['ldap']->server(), $_POST['deleteableOU'], "ObjectClass=*", array(""));
		$info = @ldap_get_entries($_SESSION['ldap']->server(), $sr);
		if ($sr && $info['count'] == 0) {
			// print header
			include 'main_header.php';
			echo '<div class="userlist-bright smallPaddingContent">';
			echo "<form action=\"ou_edit.php\" method=\"post\">\n";
			$tabindex = 1;
			$container = new htmlTable();
			$label = new htmlOutputText(_("Do you really want to delete this OU?"));
			$label->colspan = 5;
			$container->addElement($label, true);
			$container->addElement(new htmlSpacer(null, '10px'), true);
			$dnLabel = new htmlOutputText(getAbstractDN($_POST['deleteableOU']));
			$dnLabel->colspan = 5;
			$container->addElement($dnLabel, true);
			$container->addElement(new htmlSpacer(null, '10px'), true);
			$container->addElement(new htmlButton('sure', _("Delete")));			
			$container->addElement(new htmlButton('abort', _("Cancel")));
			$container->addElement(new htmlHiddenInput('deleteOU', 'submit'));		
			$container->addElement(new htmlHiddenInput('deletename', $_POST['deleteableOU']));		
			parseHtml(null, $container, array(), false, $tabindex, 'user');
			echo "</form>";
			echo '</div>';
			include 'main_footer.php';
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
	include 'main_header.php';
	echo '<div class="userlist-bright smallPaddingContent">';
	echo ("<form action=\"ou_edit.php\" method=\"post\">\n");

	$tabindex = 1;
	$container = new htmlTable();
	$container->addElement(new htmlSubTitle(_("OU editor")), true);
	if (isset($error)) {
		$msg = new htmlStatusMessage("ERROR", "", $error);
		$msg->colspan = 5;
		$container->addElement($msg, true);
	}
	elseif (isset($message)) {
		$msg = new htmlStatusMessage("INFO", "", $message);
		$msg->colspan = 5;
		$container->addElement($msg, true);
	}
	
	$types = array();
	$typeList = $_SESSION['config']->get_ActiveTypes();
	for ($i = 0; $i < sizeof($typeList); $i++) {
		$types[$typeList[$i]] = getTypeAlias($typeList[$i]);
	}
	natcasesort($types);
	$options = array();
	foreach ($types as $name => $title) {
		$elements = array();
		$units = searchLDAPByAttribute(null, null, 'organizationalunit', array('dn'), array($name));
		for ($u = 0; $u < sizeof($units); $u++) {
			$elements[getAbstractDN($units[$u]['dn'])] = $units[$u]['dn'];
		}
		$options[$title] = $elements;
	}
	// new OU
	$container->addElement(new htmlOutputText(_("New organisational unit")));
	$parentOUSelect = new htmlSelect('parentOU', $options, array());
	$parentOUSelect->setContainsOptgroups(true);
	$parentOUSelect->setHasDescriptiveElements(true);
	$parentOUSelect->setRightToLeftTextDirection(true);
	$parentOUSelect->setSortElements(false);
	$container->addElement($parentOUSelect);
	$container->addElement(new htmlInputField('newOU'));
	$container->addElement(new htmlButton('createOU', _("Ok")));
	$container->addElement(new htmlHelpLink('601'), true);
	
	$container->addElement(new htmlSpacer(null, '10px'), true);

	// delete OU
	$container->addElement(new htmlOutputText(_("Delete organisational unit")));
	$deleteableOUSelect = new htmlSelect('deleteableOU', $options, array());
	$deleteableOUSelect->setContainsOptgroups(true);
	$deleteableOUSelect->setHasDescriptiveElements(true);
	$deleteableOUSelect->setRightToLeftTextDirection(true);
	$deleteableOUSelect->setSortElements(false);
	$container->addElement($deleteableOUSelect);
	$container->addElement(new htmlOutputText(''));
	$container->addElement(new htmlButton('deleteOU', _("Ok")));
	$container->addElement(new htmlHelpLink('602'), true);
	
	parseHtml(null, $container, array(), false, $tabindex, 'user');
	echo ("</form>\n");
	echo '</div>';
	include 'main_footer.php';
}
