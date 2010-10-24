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
* Creates main suffixes if they are missing.
*
* @author Roland Gruber
* @package main
*/

/** security functions */
include_once("../lib/security.inc");
/** access to configuration settings */
include_once("../lib/config.inc");
/** LDAP access */
include_once("../lib/ldap.inc");
/** status messages */
include_once("../lib/status.inc");

// start session
startSecureSession();

if (!checkIfWriteAccessIsAllowed()) {
	die();
}

setlanguage();

// check if user already pressed button
if (isset($_POST['add_suff']) || isset($_POST['cancel'])) {
	if (isset($_POST['add_suff'])) {
		$fail = array();
		$errors = array();
		$new_suff = $_POST['new_suff'];
		$new_suff = str_replace("\\", "", $new_suff);
		$new_suff = str_replace("'", "", $new_suff);
		$new_suff = explode(";", $new_suff);
		// add entries
		for ($i = 0; $i < sizeof($new_suff); $i++) {
			// check if entry is already present
			$info = @ldap_read($_SESSION['ldap']->server(), escapeDN($new_suff[$i]), "objectclass=*", array('dn'), 0, 0, 0, LDAP_DEREF_NEVER);
			$res = @ldap_get_entries($_SESSION['ldap']->server(), $info);
			if ($res) continue;
			$suff = $new_suff[$i];
			// generate DN and attributes
			$tmp = explode(",", $suff);
			$name = explode("=", $tmp[0]);
			array_shift($tmp);
			$end = implode(",", $tmp);
			if ($name[0] != "ou") {  // add root entry
				$attr = array();
				$attr[$name[0]] = $name[1];
				$attr['objectClass'] = 'organization';
				$dn = $suff;
				if (!@ldap_add($_SESSION['ldap']->server(), $dn, $attr)) {
					$fail[] = $suff;
					continue;
				}
			}
			else {  // add organizational unit
				$name = $name[1];
				$attr = array();
				$attr['objectClass'] = "organizationalunit";
				$attr['ou'] = $name;
				$dn = $suff;
				if (!@ldap_add($_SESSION['ldap']->server(), $dn, $attr)) {
					// check if we have to add parent entries
					if (ldap_errno($_SESSION['ldap']->server()) == 32) {
						$temp = explode(",", $suff);
						$subsuffs = array();
						// make list of subsuffixes
						for ($k = 0; $k < sizeof($temp); $k++) {
							$part = explode("=", $temp[$k]);
							if ($part[0] == "ou") $subsuffs[] = implode(",", array_slice($temp, $k));
							else {
								$subsuffs[] = implode(",", array_slice($temp, $k));
								break;
							}
						}
						// create missing entries
						for ($k = sizeof($subsuffs) - 1; $k >= 0; $k--) {
							// check if subsuffix is present
							$info = @ldap_read($_SESSION['ldap']->server(), escapeDN($subsuffs[$k]), "objectclass=*", array('dn'), 0, 0, 0, LDAP_DEREF_NEVER);
							$res = @ldap_get_entries($_SESSION['ldap']->server(), $info);
							if (!$res) {
								$suffarray = explode(",", $subsuffs[$k]);
								$headarray = explode("=", $suffarray[0]);
								if ($headarray[0] == "ou") {  // add ou entry
									$attr = array();
									$attr['objectClass'] = 'organizationalunit';
									$attr['ou'] = $headarray[1];
									$dn = $subsuffs[$k];
									if (!@ldap_add($_SESSION['ldap']->server(), $dn, $attr)) {
										$fail[] = $suff;
										$error[] = ldap_error($_SESSION['ldap']->server());
										break;
									}
								}
								else {  // add root entry
									$attr = array();
									$attr['objectClass'][] = 'organization';
									$attr[$headarray[0]] = $headarray[1];
									if ($headarray[0] == "dc") {
										$attr['o'] = $headarray[1];
										$attr['objectClass'][] = 'dcObject';
									}
									$dn = $subsuffs[$k];
									if (!@ldap_add($_SESSION['ldap']->server(), $dn, $attr)) {
										$fail[] = $suff;
										$error[] = ldap_error($_SESSION['ldap']->server());
										break;
									}
								}
							}
						}
					}
					else {
						$fail[] = $suff;
						$error[] = ldap_error($_SESSION['ldap']->server());
					}
				}
			}
		}
	}
	include 'main_header.php';
	// print error/success messages
	if (isset($_POST['add_suff'])) {
		if (sizeof($fail) > 0) {
			// print error messages
			for ($i = 0; $i < sizeof($fail); $i++) {
				StatusMessage("ERROR", _("Failed to create entry!") . "<br>" . $error[$i], $fail[$i]);
			}
			include 'main_footer.php';
		}
		else {
			// print success message
			StatusMessage("INFO", "", _("All changes were successful."));
			include 'main_footer.php';
		}
	}
	else {
		// no suffixes were created
		StatusMessage("INFO", "", _("No changes were made."));
		include 'main_footer.php';
	}
	exit;
}

// first show of page
$new_suff = $_GET['suffs'];
$new_suff = str_replace("\\", "", $new_suff);
$new_suff = str_replace("'", "", $new_suff);
$new_suff = explode(";", $new_suff);

include 'main_header.php';
	echo '<div class="userlist-bright smallPaddingContent">';
	echo "<form action=\"initsuff.php\" method=\"post\">\n";
	$container = new htmlTable();
	$container->addElement(new htmlOutputText(_("The following suffix(es) are missing in LDAP. LAM can create them for you.")), true);
	$container->addElement(new htmlSpacer(null, '10px'), true);
	// print missing suffixes
	for ($i = 0; $i < sizeof($new_suff); $i++) {
		$container->addElement(new htmlOutputText($new_suff[$i]), true);
	}
	$container->addElement(new htmlSpacer(null, '10px'), true);

	$buttonContainer = new htmlTable();
	$buttonContainer->addElement(new htmlButton('add_suff', _("Create")));
	$buttonContainer->addElement(new htmlButton('cancel', _("Cancel")));
	$buttonContainer->addElement(new htmlHiddenInput('new_suff', implode(";", $new_suff)));
	$container->addElement($buttonContainer);
	
	$tabindex = 1;
	parseHtml(null, $container, array(), false, $tabindex, 'user');
	
	echo "</form><br>\n";
	echo "</div>\n";
include 'main_footer.php';
?>
