<?php
namespace LAM\DELETE;
use \htmlGroup;
use htmlJavaScript;
use \htmlResponsiveRow;
use \htmlButton;
use \htmlSpacer;
use \htmlHiddenInput;
use \htmlOutputText;
use \htmlStatusMessage;
/*

	This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
	Copyright (C) 2003 - 2006  Tilo Lutz
	Copyright (C) 2007 - 2023  Roland Gruber

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
* Used to delete accounts from LDAP tree.
*
* @author Tilo Lutz
* @author Roland Gruber
* @package main
*/


/** security functions */
include_once(__DIR__ . "/../lib/security.inc");
/** account functions */
include_once(__DIR__ . '/../lib/account.inc');
/** current configuration options */
include_once(__DIR__ . '/../lib/config.inc');
/** message displaying */
include_once(__DIR__ . '/../lib/status.inc');
/** LDAP connection */
include_once(__DIR__ . '/../lib/ldap.inc');
/** remote interface */
include_once(__DIR__ . '/../lib/remote.inc');
/** module interface */
include_once(__DIR__ . '/../lib/modules.inc');

// Start session
startSecureSession();
enforceUserIsLoggedIn();

if (!checkIfWriteAccessIsAllowed()) {
	die();
}

// Redirect to startpage if user is not logged in
if (!isLoggedIn()) {
	metaRefresh("login.php");
	exit;
}

// Set correct language, codepages, ....
setlanguage();

if (!empty($_POST)) {
	validateSecurityToken();
}

$sessionAccountPrefix = 'deleteContainer';
foreach ($_SESSION as $key => $value) {
	if (str_starts_with($key, $sessionAccountPrefix)) {
		unset($_SESSION[$key]);
		logNewMessage(LOG_NOTICE, "del " . $key);
	}
}

$typeManager = new \LAM\TYPES\TypeManager();

if (isset($_POST['type']) && ($typeManager->getConfiguredType($_POST['type']) === null)) {
	logNewMessage(LOG_ERR, 'Invalid type: ' . $_POST['type']);
	die();
}

if (isset($_GET['type']) && isset($_SESSION['delete_dn'])) {
	$typeId = $_GET['type'];
	$type = $typeManager->getConfiguredType($typeId);
	if ($type === null) {
		logNewMessage(LOG_ERR, 'Invalid type id: ' . $typeId);
		die();
	}
	if (!checkIfDeleteEntriesIsAllowed($type->getId()) || !checkIfWriteAccessIsAllowed($type->getId())) {
		logNewMessage(LOG_ERR, 'User tried to delete entries of forbidden type '. $type->getId());
		die();
	}
	// Create account list
    $users = [];
	foreach ($_SESSION['delete_dn'] as $dn) {
		$start = strpos ($dn, "=")+1;
		$end = strpos ($dn, ",");
		$users[] = substr($dn, $start, $end-$start);
	}

	$sessionKey = $sessionAccountPrefix . (new \DateTime('now', getTimeZone()))->getTimestamp() . generateRandomText();
	//load account
	$_SESSION[$sessionKey] = new \accountContainer($type, $sessionKey);
	// Show HTML Page
	include '../lib/adminHeader.inc';
	echo "<div class=\"smallPaddingContent\">";
	echo "<br>\n";
	echo "<form action=\"delete.php\" method=\"post\">\n";
	$container = new htmlResponsiveRow();
	$container->add(new htmlOutputText(_("Do you really want to remove the following accounts?")), 12);
	$container->addVerticalSpacer('2rem');
	$userCount = sizeof($users);
	for ($i = 0; $i < $userCount; $i++) {
		$container->addLabel(new htmlOutputText(_("Account name:")));
		$container->addField(new htmlOutputText($users[$i]));
		$container->addLabel(new htmlOutputText(_('DN') . ':'));
		$container->addField(new htmlOutputText($_SESSION['delete_dn'][$i]));
		$_SESSION[$sessionKey]->load_account($_SESSION['delete_dn'][$i]);
		if (!$_SESSION[$sessionKey]->hasOnlyVirtualChildren()) {
			$childCount = getChildCount($_SESSION['delete_dn'][$i]);
			if ($childCount > 0) {
				$container->addLabel(new htmlOutputText(_('Number of child entries') . ':'));
				$container->addField(new htmlOutputText($childCount));
			}
		}
		$container->addVerticalSpacer('0.5rem');
	}
	addSecurityTokenToMetaHTML($container);
	$container->add(new htmlHiddenInput('type', $type->getId()), 12);
	$container->addVerticalSpacer('1rem');
	parseHtml(null, $container, [], false, $type->getScope());
	// Print delete rows from modules
	$modules = $_SESSION['config']->get_AccountModules($type->getId());
	$values = [];
	foreach ($modules as $module) {
		$module = \moduleCache::getModule($module, $type->getScope());
		parseHtml($module::class, $module->display_html_delete(), $values, true, $type->getScope());
	}
	$buttonContainer = new htmlResponsiveRow();
	$buttonContainer->addVerticalSpacer('1rem');
	$buttonGroup = new htmlGroup();
	$delButton = new htmlButton('delete', _('Delete'));
	$delButton->setCSSClasses(['lam-danger']);
	$buttonGroup->addElement($delButton);
	$buttonGroup->addElement(new htmlSpacer('0.5rem', null));
	$cancelButton = new htmlButton('cancel', _('Cancel'));
	$buttonGroup->addElement($cancelButton);
	$buttonContainer->add($buttonGroup, 12);
	$buttonContainer->addVerticalSpacer('1rem');
	parseHtml(null, $buttonContainer, [], false, $type->getScope());
	echo "</form>\n";
	echo "</div>\n";
	include '../lib/adminFooter.inc';
}

if (isset($_POST['cancel'])) {
	if (isset($_SESSION['delete_dn'])) {
		unset($_SESSION['delete_dn']);
	}
	metaRefresh("lists/list.php?type=" . $_POST['type']);
}
elseif (isset($_POST['cancelAllOk'])) {
	if (isset($_SESSION['delete_dn'])) {
		unset($_SESSION['delete_dn']);
	}
	metaRefresh("lists/list.php?type=" . $_POST['type'] . '&deleteAllOk=1');
}

if (isset($_POST['delete'])) {
	$typeId = $_POST['type'];
	$type = $typeManager->getConfiguredType($typeId);
	if (!checkIfDeleteEntriesIsAllowed($type->getId()) || !checkIfWriteAccessIsAllowed($type->getId())) {
		logNewMessage(LOG_ERR, 'User tried to delete entries of forbidden type '. $type->getId());
		die();
	}
	// Show HTML Page
	include __DIR__ . '/../lib/adminHeader.inc';
	echo "<form action=\"delete.php\" method=\"post\">\n";
	echo "<div class=\"smallPaddingContent\"><br>\n";
	$container = new htmlResponsiveRow();
	addSecurityTokenToMetaHTML($container);
	$container->add(new htmlHiddenInput('type', $type->getId()), 12);

	$sessionKey = $sessionAccountPrefix . (new \DateTime('now', getTimeZone()))->getTimestamp() . generateRandomText();
	$_SESSION[$sessionKey] = new \accountContainer($type, $sessionKey);
	// Delete dns
	$allOk = true;
	$allErrors = [];
	foreach ($_SESSION['delete_dn'] as $deleteDN) {
		// Set to true if an real error has happened
		$stopProcessing = false;
		// First load DN.
		$_SESSION[$sessionKey]->load_account($deleteDN);
		// get commands and changes of each attribute
		$moduleNames = array_keys($_SESSION[$sessionKey]->getAccountModules());
		$modules = $_SESSION[$sessionKey]->getAccountModules();
		$attributes = [];
		$errors = [];
		// predelete actions
        foreach ($moduleNames as $singlemodule) {
            $success = true;
            $messages = $modules[$singlemodule]->preDeleteActions();
            foreach ($messages as $message) {
                $errors[] = $message;
                if ($message[0] === 'ERROR') {
                    $success = false;
                    $allOk = false;
                }
                elseif ($message[0] === 'WARN') {
                    $allOk = false;
                }
            }
            if (!$success) {
                $stopProcessing = true;
                break;
            }
        }
		if (!$stopProcessing) {
			// load attributes
			foreach ($moduleNames as $singlemodule) {
				// load changes
				$temp = $modules[$singlemodule]->delete_attributes();
                // merge changes
                $DNs = array_keys($temp);
                $attributes = array_merge_recursive($temp, $attributes);
                foreach ($DNs as $dn) {
                    $ops = array_keys($temp[$dn]);
                    foreach ($ops as $op) {
                        $attrs = array_keys($temp[$dn][$op]);
                        foreach ($attrs as $attribute) {
                            $attributes[$dn][$op][$attribute] = array_unique($attributes[$dn][$op][$attribute]);
                        }
                    }
                }
			}
			$DNs = array_keys($attributes);
			foreach ($DNs as $dn) {
				if (isset($attributes[$dn]['errors'])) {
					foreach ($attributes[$dn]['errors'] as $singleerror) {
						$errors[] = $singleerror;
						if ($singleerror[0] == 'ERROR') {
							$stopProcessing = true;
							$allOk = false;
						}
					}
				}
				if (!$stopProcessing) {
					// modify attributes
					if (isset($attributes[$dn]['modify'])) {
						$success = ldap_mod_replace($_SESSION['ldap']->server(), $dn, $attributes[$dn]['modify']);
						if (!$success) {
							$errors[] = ['ERROR', sprintf(_('Was unable to modify attributes from DN: %s.'), $dn), getDefaultLDAPErrorString($_SESSION['ldap']->server())];
							$stopProcessing = true;
							$allOk = false;
						}
					}
					// add attributes
					if (isset($attributes[$dn]['add']) && !$stopProcessing) {
						$success = ldap_mod_add($_SESSION['ldap']->server(), $dn, $attributes[$dn]['add']);
						if (!$success) {
							$errors[] = ['ERROR', sprintf(_('Was unable to add attributes to DN: %s.'), $dn), getDefaultLDAPErrorString($_SESSION['ldap']->server())];
							$stopProcessing = true;
							$allOk = false;
						}
					}
					// remove attributes
					if (isset($attributes[$dn]['remove']) && !$stopProcessing) {
						$success = ldap_mod_del($_SESSION['ldap']->server(), $dn, $attributes[$dn]['remove']);
						if (!$success) {
							$errors[] = ['ERROR', sprintf(_('Was unable to remove attributes from DN: %s.'), $dn), getDefaultLDAPErrorString($_SESSION['ldap']->server())];
							$stopProcessing = true;
							$allOk = false;
						}
					}
				}
			}
		}
		if (!$stopProcessing) {
			$recursive = !$_SESSION[$sessionKey]->hasOnlyVirtualChildren();
			$messages = deleteDN($deleteDN, $recursive);
			foreach ($messages as $message) {
				$errors[] = $message;
				if ($message[0] === 'ERROR') {
					$stopProcessing = true;
					$allOk = false;
				}
				elseif ($message[0] === 'WARN') {
					$allOk = false;
                }
			}
		}
		// post delete actions
		if (!$stopProcessing) {
			foreach ($moduleNames as $singlemodule) {
				$messages = $modules[$singlemodule]->postDeleteActions();
				foreach ($messages as $message) {
					$errors[] = $message;
					if (($message[0] === 'ERROR') || ($message[0] === 'WARN')) {
						$allOk = false;
					}
				}
			}
		}
		if (!$stopProcessing) {
			$container->add(new htmlOutputText(sprintf(_('Deleted DN: %s'), $deleteDN)), 12);
			foreach ($errors as $error) {
				$container->add(htmlStatusMessage::fromParamArray($error), 12);
			}
		}
		else {
			$container->add(new htmlOutputText(sprintf(_('Error while deleting DN: %s'), $deleteDN)), 12);
			foreach ($errors as $error) {
				$container->add(htmlStatusMessage::fromParamArray($error), 12);
			}
		}
		$allErrors = [...$allErrors, ...$errors];
	}
	$container->addVerticalSpacer('2rem');
	$buttonName = $allOk ? 'cancelAllOk' : 'cancel';
	$container->add(new htmlButton($buttonName, _('Back to list')), 12);
	$container->addVerticalSpacer('1rem');
	if ($allOk) {
		$_SESSION['listRedirectMessages'] = $allErrors;
        $container->add(new htmlJavaScript('document.getElementById("btn_cancelAllOk").click();'));
	}
	parseHtml(null, $container, [], false, $type->getScope());
	echo "</div>\n";
	echo "</form>\n";
	include __DIR__ . '/../lib/adminFooter.inc';

}

/**
* Returns the number of child entries of a DN.
*
* @param string $dn DN of parent
* @return integer number of children
*/
function getChildCount($dn) {
	$entries = searchLDAP($dn, '(objectClass=*)', ['dn']);
	return (sizeof($entries) - 1);
}
