<?php
/*
	$Id$

	This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
	Copyright (C) 2003 - 2006  Tilo Lutz
	Copyright (C) 2007 - 2009  Roland Gruber

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
include_once("../lib/security.inc");
/** account functions */
include_once('../lib/account.inc');
/** current configuration options */
include_once('../lib/config.inc');
/** message displaying */
include_once('../lib/status.inc');
/** LDAP connection */
include_once('../lib/ldap.inc');
/** lamdaemon interface */
include_once('../lib/lamdaemon.inc');
/** module interface */
include_once('../lib/modules.inc');

// Start session
startSecureSession();

if (!checkIfWriteAccessIsAllowed()) {
	die();
}

// Redirect to startpage if user is not loged in
if (!isset($_SESSION['loggedIn']) || ($_SESSION['loggedIn'] !== true)) {
	metaRefresh("login.php");
	exit;
}

// Set correct language, codepages, ....
setlanguage();

if (!isset($_SESSION['cache'])) {
	$_SESSION['cache'] = new cache();
}
if (isset($_GET['type']) && isset($_SESSION['delete_dn'])) {
	// Create account list
	foreach ($_SESSION['delete_dn'] as $dn) {
		$start = strpos ($dn, "=")+1;
		$end = strpos ($dn, ",");
		$users[] = substr($dn, $start, $end-$start);
	}

	//load account
	$_SESSION['account'] = new accountContainer($_GET['type'], 'account');
	// Show HTML Page
	echo $_SESSION['header'];
	echo "<title>LDAP Account Manager</title>\n";
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\">\n";
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/type_" . $_GET['type'] . ".css\">\n";
	echo "</head><body>\n";
	echo "<script type=\"text/javascript\" src=\"wz_tooltip.js\"></script>\n";
	echo "<form action=\"delete.php\" method=\"post\">\n";
	echo "<fieldset class=\"".$_GET['type']."edit\"><legend><b>";
	echo _('Please confirm:');
	echo "</b></legend><br>\n";
	echo "<input name=\"type\" type=\"hidden\" value=\"" . $_GET['type'] . "\">\n";
	echo "<b>" . _("Do you really want to remove the following accounts?") . "</b>";
	echo "<br><br>\n";
	echo "<table border=0>\n";
	for ($i=0; $i<count($users); $i++) {
		echo "<tr>\n";
		echo "<td><b>" . _("Account name:") . "</b> $users[$i]</td>\n";
		echo "<td>&nbsp;&nbsp;<b>" . _('DN') . ":</b> " . $_SESSION['delete_dn'][$i] . "</td>\n";
		$childCount = getChildCount($_SESSION['delete_dn'][$i]);
		if ($childCount > 0) {
			echo "<td>&nbsp;&nbsp;<b>" . _('Number of child entries') . ":</b> " . $childCount . "</td>\n";
		}
		echo "</tr>\n";
	}
	echo "</table>\n";
	echo "<br>\n";
	// Print delete rows from modules
	echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
	$modules = $_SESSION['config']->get_AccountModules($_GET['type']);
	$values = array();
	$tabindex = 100;
	foreach ($modules as $module) {
		$module = new $module($_GET['type']);
		parseHtml(get_class($module), $module->display_html_delete(), $values, true, $tabindex, $_GET['type']);
	}
	echo "</table>\n";
	echo "<br>\n";
	echo "<input name=\"delete\" type=\"submit\" value=\"" . _('Delete') . "\">&nbsp;\n";
	echo "<input name=\"cancel\" type=\"submit\" value=\"" . _('Cancel') . "\">\n";
	echo "</fieldset>\n";
	echo "</form>\n";
	echo "</body>\n";
	echo "</html>\n";
}

if ($_POST['cancel']) {
	if (isset($_SESSION['delete_dn'])) unset($_SESSION['delete_dn']);
	metaRefresh("lists/list.php?type=" . $_POST['type']);
}

if ($_POST['delete']) {
	// Show HTML Page
	echo $_SESSION['header'];
	echo "<title>LDAP Account Manager</title>\n";
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\">\n";
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/type_" . $_POST['type'] . ".css\">\n";
	echo "</head><body>\n";
	echo "<form action=\"delete.php\" method=\"post\">\n";
	echo "<input name=\"type\" type=\"hidden\" value=\"" . $_POST['type'] . "\">\n";
	echo "<fieldset class=\"".$_POST['type']."edit\"><legend><b>";
	echo _('Deleting. Please stand by ...');
	echo "</b></legend><br>\n";

	// Delete dns
	for ($m=0; $m<count($_SESSION['delete_dn']); $m++) {
		// Set to true if an real error has happened
		$stopprocessing = false;
		// First load DN.
		$_SESSION['account']->load_account($_SESSION['delete_dn'][$m]);
		// get commands and changes of each attribute
		$moduleNames = array_keys($_SESSION['account']->getAccountModules());
		$modules = $_SESSION['account']->getAccountModules();
		$attributes = array();
		$errors = array();
		// predelete actions
		if (!$stopprocessing) {
			foreach ($moduleNames as $singlemodule) {
				$success = $modules[$singlemodule]->preDeleteActions();
				if (!$success) {
					$stopprocessing = true;
					break;
				}
			}
		}
		if (!$stopprocessing) {
			// load attributes
			foreach ($moduleNames as $singlemodule) {
				// load changes
				$temp = $modules[$singlemodule]->delete_attributes();
				if (is_array($temp)) {
					// merge changes
					$DNs = array_keys($temp);
					$attributes = array_merge_recursive($temp, $attributes);
					for ($i=0; $i<count($DNs); $i++) {
						$ops = array_keys($temp[$DNs[$i]]);
						for ($j=0; $j<count($ops); $j++) {
							$attrs = array_keys($temp[$DNs[$i]][$ops[$j]]);
							for ($k=0; $k<count($attrs); $k++)
							$attributes[$DNs[$i]][$ops[$j]][$attrs[$k]] = array_unique($attributes[$DNs[$i]][$ops[$j]][$attrs[$k]]);
						}
					}
				}
			}
			$DNs = array_keys($attributes);
			for ($i=0; $i<count($DNs); $i++) {
				if (isset($attributes[$DNs[$i]]['errors'])) {
					foreach ($attributes[$DNs[$i]]['errors'] as $singleerror) {
						$errors[] = $singleerror;
						if ($singleerror[0] == 'ERROR') $stopprocessing = true;
					}
				}
				if (!$stopprocessing) {
					// modify attributes
					if (isset($attributes[$DNs[$i]]['modify']) && !$stopprocessing) {
						$success = @ldap_mod_replace($_SESSION['ldap']->server(), $DNs[$i], $attributes[$DNs[$i]]['modify']);
						if (!$success) {
							$errors[] = array ('ERROR', sprintf(_('Was unable to modify attribtues from DN: %s.'), $DNs[$i]), ldap_error($_SESSION['ldap']->server()));
							$stopprocessing = true;
						}
					}
					// add attributes
					if (isset($attributes[$DNs[$i]]['add']) && !$stopprocessing) {
						$success = @ldap_mod_add($_SESSION['ldap']->server(), $DNs[$i], $attributes[$DNs[$i]]['add']);
						if (!$success) {
							$errors[] = array ('ERROR', sprintf(_('Was unable to add attribtues to DN: %s.'), $DNs[$i]), ldap_error($_SESSION['ldap']->server()));
							$stopprocessing = true;
						}
					}
					// removce attributes
					if (isset($attributes[$DNs[$i]]['remove']) && !$stopprocessing) {
						$success = @ldap_mod_del($_SESSION['ldap']->server(), $DNs[$i], $attributes[$DNs[$i]]['remove']);
						if (!$success) {
							$errors[] = array ('ERROR', sprintf(_('Was unable to remove attribtues from DN: %s.'), $DNs[$i]), ldap_error($_SESSION['ldap']->server()));
							$stopprocessing = true;
						}
					}
				}
			}
		}
		if (!$stopprocessing) {
			$errors = deleteDN($_SESSION['delete_dn'][$m]);
			if (sizeof($errors) > 0) $stopprocessing = true;
		}
		// post delete actions
		if (!$stopprocessing) {
			foreach ($moduleNames as $singlemodule) {
				$modules[$singlemodule]->postDeleteActions();
			}
		}		
		if (!$stopprocessing) {
			echo sprintf(_('Deleted DN: %s'), $_SESSION['delete_dn'][$m]) . "<br>\n";
			foreach ($errors as $error) StatusMessage($error[0], $error[1], $error[2]);
			echo "<br>\n";
		}
		else {
			echo sprintf(_('Error while deleting DN: %s'), $_SESSION['delete_dn'][$m]) . "<br>\n";
			foreach ($errors as $error) StatusMessage($error[0], $error[1], $error[2]);
			echo "<br>\n";
		}
	}
	$_SESSION['cache']->refresh_cache(true);
	echo "<br>\n";
	echo "<br><input name=\"cancel\" type=\"submit\" value=\"" . _('Back to list') . "\">\n";
	echo "</fieldset>\n";
	echo "</form>\n";
	echo "</body>\n";
	echo "</html>\n";

}

/**
* Returns the number of child entries of a DN.
*
* @param string $dn DN of parent
* @return interger number of childs
*/
function getChildCount($dn) {
	$return = 0;
	$sr = @ldap_search($_SESSION['ldap']->server(), escapeDN($dn), 'objectClass=*', array('dn'), 0, 0, 0, LDAP_DEREF_ALWAYS);
	if ($sr) {
		$entries = ldap_get_entries($_SESSION['ldap']->server(), $sr);
		$return = $entries['count'] - 1;
	}
	return $return;
}

/**
* Deletes a DN and all child entries.
*
* @param string $dn DN to delete
* @return array error messages
*/
function deleteDN($dn) {
	$errors = array();
	$sr = @ldap_list($_SESSION['ldap']->server(), $dn, 'objectClass=*', array('dn'), 0);
	if ($sr) {
		$entries = ldap_get_entries($_SESSION['ldap']->server(), $sr);
		for ($i = 0; $i < $entries['count']; $i++) {
			// delete recursively
			$subErrors = deleteDN($entries[$i]['dn']);
			for ($e = 0; $e < sizeof($subErrors); $e++) $errors[] = $subErrors[$e];
		}
	}
	else {
		$errors[] = array ('ERROR', sprintf(_('Was unable to delete DN: %s.'), $dn), ldap_error($_SESSION['ldap']->server()));
	}
	// delete parent DN
	$success = @ldap_delete($_SESSION['ldap']->server(), $dn);
	$ldapUser = $_SESSION['ldap']->decrypt_login();
	$ldapUser = $ldapUser[0];
	if (!$success) {
		logNewMessage(LOG_ERR, '[' . $ldapUser .'] Unable to delete DN: ' . $dn . ' (' . ldap_err2str(ldap_errno($_SESSION['ldap']->server())) . ').');
		$errors[] = array ('ERROR', sprintf(_('Was unable to delete DN: %s.'), $dn), ldap_error($_SESSION['ldap']->server()));
	}
	else {
		logNewMessage(LOG_NOTICE, '[' . $ldapUser .'] Deleted DN: ' . $dn);
	}
	return $errors;
}

?>
