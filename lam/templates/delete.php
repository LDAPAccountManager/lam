<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Tilo Lutz

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


  LDAP Account Manager Delete user, hosts or groups
*/


// include all needed files
include_once('../lib/account.inc'); // File with all account-funtions
include_once('../lib/config.inc'); // File with configure-functions
include_once('../lib/profiles.inc'); // functions to load and save profiles
include_once('../lib/status.inc'); // Return error-message
include_once('../lib/pdf.inc'); // Return a pdf-file
include_once('../lib/ldap.inc'); // LDAP-functions

/* We have to include all modules
* before start session
* *** fixme I would prefer loading them dynamic but
* i don't know how to to this
*/
$dir = opendir('../lib/modules');
while ($entry = readdir($dir))
	if (is_file('../lib/modules/'.$entry)) include_once ('../lib/modules/'.$entry);

// Start session
session_save_path('../sess');
@session_start();

// Redirect to startpage if user is not loged in
if (!isset($_SESSION['loggedIn'])) {
	metaRefresh("login.php");
	exit;
	}

// Set correct language, codepages, ....
setlanguage();

if (!isset($_SESSION['cache'])) {
	$_SESSION['cache'] = new cache();
	}
if ($_GET['type']) {
	// Create account list
	foreach ($_SESSION['delete_dn'] as $dn) {
		$start = strpos ($dn, "=")+1;
		$end = strpos ($dn, ",");
		$users[] = substr($dn, $start, $end-$start);
		}

	//load account
	$_SESSION['account'] = new accountContainer($_GET['type'], 'account');
	$_SESSION['account']->load_account($_SESSION['delete_dn'][0]);
	// Show HTML Page
	echo $_SESSION['header'];
	echo "<title>";
	echo _("Delete Account");
	echo "</title>\n";
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\">\n";
	echo "</head><body>\n";
	echo "<form action=\"delete.php\" method=\"post\">\n";
	echo "<fieldset class=\"".$_GET['type']."edit-dark\"><legend class=\"".$_GET['type']."edit-bright\"><b>";
		echo _('Please confirm:');
		echo "</b></legend>\n";
	echo "<input name=\"type\" type=\"hidden\" value=\"" . $_GET['type'] . "\">\n";
	echo _("Do you really want to remove the following accounts?");
	echo "<br>\n";
	echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
	for ($i=0; $i<count($users); $i++) {
		echo "<tr>\n";
		echo "<td>" . _("Account name:") . " $users[$i]</td>\n";
		echo "<td>" . _('DN') . " " . $_SESSION['delete_dn'][$i] . "</td>\n";
		echo "</tr>\n";
		}
	echo "</table>\n";
	echo "<br>\n";
	// Print delete rows from modules
	echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
	$modules = array_keys($_SESSION['account']->module);
	for ($i=0; $i<count($modules); $i++) {
		$_SESSION['account']->module[$modules[$i]]->display_html_delete($_POST);
		}
	echo "</table>\n";
	echo "<br>\n";
	echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
	echo "<td><input name=\"delete\" type=\"submit\" value=\"" . _('Delete') . "\"></td>\n";
	echo "<td><input name=\"cancel\" type=\"submit\" value=\"" . _('Cancel') . "\"></td>\n";
	echo "</table>\n";
	echo "</fieldset>\n";
	echo "</form>\n";
	echo "</body>\n";
	echo "</html>\n";
	}

if ($_POST['cancel']) {
	if (isset($_SESSION['delete_dn'])) unset($_SESSION['delete_dn']);
	metaRefresh("lists/list" . $_POST['type'] . "s.php");
	}

if ($_POST['delete']) {
	// Show HTML Page
	echo $_SESSION['header'];
	echo "<title>";
	echo _("Delete Account");
	echo "</title>\n";
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\">\n";
	echo "</head><body>\n";
	echo "<form action=\"delete.php\" method=\"post\">\n";
	echo "<input name=\"type\" type=\"hidden\" value=\"" . $_POST['type'] . "\">\n";
	echo "<fieldset class=\"".$_POST['type']."edit-dark\"><legend class=\"".$_POST['type']."edit-bright\"><b>";
		echo _('Deleting. Please stand by ...');
		echo "</b></legend>\n";

	echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
	// Delete dns
	for ($m=0; $m<count($_SESSION['delete_dn']); $m++) {
		// Set to true if an real error has happened
		$stopprocessing = false;
		// First load DN.
		$_SESSION['account']->load_account($_SESSION['delete_dn'][$m]);
		// get commands and changes of each attribute
		$module = array_keys ($_SESSION['account']->module);
		$attributes = array();
		$errors = array();
		// load attributes
		foreach ($module as $singlemodule) {
			// load changes
			$temp = $_SESSION['account']->module[$singlemodule]->delete_attributes($_POST);
			if (is_array($temp)) {
				// merge changes
				$DNs = array_keys($temp);
				// *** fixme don't include references
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
					if ($singleerror[0] = 'ERROR') $stopprocessing = true;
					}
				}
			if (!$stopprocessing) {
				// modify attributes
				if (isset($attributes[$DNs[$i]]['modify']) && !$stopprocessing) {
					$success = @ldap_mod_replace($_SESSION[$_SESSION['account']->ldap]->server(), $DNs[$i], $attributes[$DNs[$i]]['modify']);
					if (!$success) {
						$errors[] = array ('ERROR', 'LDAP', sprintf(_('Was unable to modify attribtues from dn: %s. This is possible a bug. Please check your ldap logs and send a bug report if it is a possible bug.'), $DNs[$i]));
						$stopprocessing = true;
						}
					else
						$_SESSION['cache']->update_cache($DNs[$i], 'modify', $attributes[$DNs[$i]]['modify']);
					}
				// add attributes
				if (isset($attributes[$DNs[$i]]['add']) && !$stopprocessing) {
					$success = @ldap_mod_add($_SESSION[$_SESSION['account']->ldap]->server(), $DNs[$i], $attributes[$DNs[$i]]['add']);
					if (!$success) {
						$errors[] = array ('ERROR', 'LDAP', sprintf(_('Was unable to add attribtues to dn: %s. This is possible a bug. Please check your ldap logs and send a bug report if it is a possible bug.'), $DNs[$i]));
						$stopprocessing = true;
						}
					else
						$_SESSION['cache']->update_cache($DNs[$i], 'add', $attributes[$DNs[$i]]['add']);
					}
				// removce attributes
				if (isset($attributes[$DNs[$i]]['remove']) && !$stopprocessing) {
					$success = @ldap_mod_del($_SESSION[$_SESSION['account']->ldap]->server(), $DNs[$i], $attributes[$DNs[$i]]['remove']);
					if (!$success) {
						$errors[] = array ('ERROR', 'LDAP', sprintf(_('Was unable to remove attribtues from dn: %s. This is possible a bug. Please check your ldap logs and send a bug report if it is a possible bug.'), $DNs[$i]));
						$stopprocessing = true;
						}
					else
						$_SESSION['cache']->update_cache($DNs[$i], 'remove', $attributes[$DNs[$i]]['remove']);
					}
				}
			}
		if (!$stopprocessing) {
			foreach ($attributes as $DN) {
				if (is_array($DN['lamdaemon']['command'])) $result = $_SESSION['account']->lamdaemon($DN['lamdaemon']['command']);
				// Error somewhere in lamdaemon
				foreach ($result as $singleresult) {
					if (is_array($singleresult)) {
						if ($singleresult[0] = 'ERROR') $stopprocessing = true;
						$temparray[0] = $singleresult[0];
						$temparray[1] = _($singleresult[1]);
						$temparray[2] = _($singleresult[2]);
						}
					}
				}
			}
		if (!$stopprocessing) {
			$success = @ldap_delete($_SESSION[$_SESSION['account']->ldap]->server(), $_SESSION['delete_dn'][$m]);
			if (!$success) $errors[] = array ('ERROR', 'LDAP', sprintf(_('Was unable to remove attribtues from dn: %s. This is possible a bug. Please check your ldap logs and send a bug report if it is a possible bug.'), $DNs[$i]));
			else
				$_SESSION['cache']->update_cache($_SESSION['delete_dn'][$m], 'delete_dn');
			}
		if (!$stopprocessing) {
			echo "<tr>\n";
			echo "<td>" . sprintf(_('Deleted DN: %s'), $_SESSION['delete_dn'][$m]) . "</td>\n";
			echo "</tr>\n";
			foreach ($errors as $error) StatusMessage($error[0], $error[1], $error[2]);
			}
		else {
			echo "<tr>\n";
			echo "<td>" . sprintf(_('Error while deleting DN: %s'), $_SESSION['delete_dn'][$m]) . "</td>\n";
			echo "</tr>\n";
			foreach ($errors as $error) StatusMessage($error[0], $error[1], $error[2]);
			}
		}
	echo "</table>\n";
	echo "<br>\n";
	echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
	echo "<td><input name=\"cancel\" type=\"submit\" value=\"" . _('Back to list') . "\"></td>\n";
	echo "</table>\n";
	echo "</fieldset>\n";
	echo "</form>\n";
	echo "</body>\n";
	echo "</html>\n";

	}

?>
