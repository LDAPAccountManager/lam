<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2019  Roland Gruber

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
include_once(__DIR__ . "/../lib/security.inc");
/** access to configuration settings */
include_once(__DIR__ . "/../lib/config.inc");
/** LDAP access */
include_once(__DIR__ . "/../lib/ldap.inc");
/** status messages */
include_once(__DIR__ . "/../lib/status.inc");

// start session
startSecureSession();
enforceUserIsLoggedIn();

if (!checkIfWriteAccessIsAllowed()) {
	die();
}

setlanguage();

if (!empty($_POST)) {
	validateSecurityToken();
}

// check if user already pressed button
if (isset($_POST['add_suff']) || isset($_POST['cancel'])) {
	if (isset($_POST['add_suff'])) {
		$failedDNs = array();
		$newSuffixes = $_POST['new_suff'];
		$newSuffixes = str_replace("\\", "", $newSuffixes);
		$newSuffixes = str_replace("'", "", $newSuffixes);
		$newSuffixes = explode(";", $newSuffixes);
		// add entries
		foreach ($newSuffixes as $newSuffix) {
			// check if entry is already present
			$info = @ldap_read($_SESSION['ldap']->server(), escapeDN($newSuffix), "objectclass=*", array('dn'), 0, 0, 0, LDAP_DEREF_NEVER);
			$res = false;
			if ($info !== false) {
				$res = ldap_get_entries($_SESSION['ldap']->server(), $info);
			}
			if ($res) {
				continue;
			}
			$suff = $newSuffix;
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
					$failedDNs[$suff] = ldap_error($_SESSION['ldap']->server());
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
						$dnParts = explode(",", $suff);
						$subsuffs = array();
						// make list of subsuffixes
						$dnPartsCount = sizeof($dnParts);
						for ($k = 0; $k < $dnPartsCount; $k++) {
							$part = explode("=", $dnParts[$k]);
							if ($part[0] == "ou") {
								$subsuffs[] = implode(",", array_slice($dnParts, $k));
							}
							else {
								$subsuffs[] = implode(",", array_slice($dnParts, $k));
								break;
							}
						}
						// create missing entries
						$subsuffCount = sizeof($subsuffs);
						for ($k = $subsuffCount - 1; $k >= 0; $k--) {
							// check if subsuffix is present
							$info = @ldap_read($_SESSION['ldap']->server(), escapeDN($subsuffs[$k]), "objectclass=*", array('dn'), 0, 0, 0, LDAP_DEREF_NEVER);
							$res = false;
							if ($info !== false) {
								$res = ldap_get_entries($_SESSION['ldap']->server(), $info);
							}
							if (!$res) {
								$suffarray = explode(",", $subsuffs[$k]);
								$headarray = explode("=", $suffarray[0]);
								if ($headarray[0] == "ou") {  // add ou entry
									$attr = array();
									$attr['objectClass'] = 'organizationalunit';
									$attr['ou'] = $headarray[1];
									$dn = $subsuffs[$k];
									if (!@ldap_add($_SESSION['ldap']->server(), $dn, $attr)) {
										$failedDNs[$suff] = ldap_error($_SESSION['ldap']->server());
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
										$failedDNs[$suff] = ldap_error($_SESSION['ldap']->server());
										break;
									}
								}
							}
						}
					}
					else {
						$failedDNs[$suff] = ldap_error($_SESSION['ldap']->server());
					}
				}
			}
		}
	}
	include '../lib/adminHeader.inc';
	// print error/success messages
	if (isset($_POST['add_suff'])) {
		if (sizeof($failedDNs) > 0) {
			// print error messages
			foreach ($failedDNs as $suffix => $error) {
				StatusMessage("ERROR", _("Failed to create entry!") . "<br>" . htmlspecialchars($error), htmlspecialchars($suffix));
			}
			include '../lib/adminFooter.inc';
		}
		else {
			// print success message
			StatusMessage("INFO", "", _("All changes were successful."));
			include '../lib/adminFooter.inc';
		}
	}
	else {
		// no suffixes were created
		StatusMessage("INFO", "", _("No changes were made."));
		include '../lib/adminFooter.inc';
	}
	exit;
}

// first show of page
$newSuffixes = $_GET['suffs'];
$newSuffixes = str_replace("\\", "", $newSuffixes);
$newSuffixes = str_replace("'", "", $newSuffixes);
$newSuffixes = explode(";", $newSuffixes);

include __DIR__ . '/../lib/adminHeader.inc';
	echo '<div class="user-bright smallPaddingContent">';
	echo "<form action=\"initsuff.php\" method=\"post\">\n";
	$container = new htmlResponsiveRow();
	$container->add(new htmlOutputText(_("The following suffixes are missing in LDAP. LAM can create them for you.")), 12);
	$container->add(new htmlOutputText(_("You can setup the LDAP suffixes for all account types in your LAM server profile on tab \"Account types\".")), 12);
	$container->addVerticalSpacer('1rem');
	// print missing suffixes
	foreach ($newSuffixes as $newSuffix) {
		$container->add(new htmlOutputText($newSuffix), 12);
	}
	$container->addVerticalSpacer('2rem');

	$buttonContainer = new htmlGroup();
	$buttonContainer->addElement(new htmlButton('add_suff', _("Create")));
	$buttonContainer->addElement(new htmlButton('cancel', _("Cancel")));
	$buttonContainer->addElement(new htmlHiddenInput('new_suff', implode(";", $newSuffixes)));
	$container->add($buttonContainer, 12);
	addSecurityTokenToMetaHTML($container);

	$tabindex = 1;
	parseHtml(null, $container, array(), false, $tabindex, 'user');

	echo "</form><br>\n";
	echo "</div>\n";
include __DIR__ . '/../lib/adminFooter.inc';
?>
