<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Roland Gruber

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

  Creates main suffixes if they are missing.

*/

include_once ("../lib/config.inc");
include_once ("../lib/ldap.inc");
include_once ("../lib/status.inc");

// start session
session_save_path("../sess");
@session_start();

setlanguage();

// check if user already pressed button
if ($_POST['add_suff'] || $_POST['cancel']) {
	if ($_POST['add_suff']) {
		$fail = array();
		$errors = array();
		$new_suff = $_POST['new_suff'];
		$new_suff = str_replace("\\", "", $new_suff);
		$new_suff = str_replace("'", "", $new_suff);
		$new_suff = explode(";", $new_suff);
		// add entries
		for ($i = 0; $i < sizeof($new_suff); $i++) {
			// check if entry is already present
			$info = @ldap_search($_SESSION['ldap']->server, $new_suff[$i], "", array());
			$res = @ldap_get_entries($_SESSION['ldap']->server, $info);
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
							$info = @ldap_search($_SESSION['ldap']->server, $subsuffs[$k], "", array());
							$res = @ldap_get_entries($_SESSION['ldap']->server, $info);
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
	echo $_SESSION['header'];
	echo "<title>initsuff</title>\n";
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\">\n";
	echo "</head>\n<body>\n";
	// print error/success messages
	if ($_POST['add_suff']) {
		if (sizeof($fail) > 0) {
			// print error messages
			for ($i = 0; $i < sizeof($fail); $i++) {
				StatusMessage("ERROR", _("Failed to create entry!") . "<br>" . $error[$i], $fail[$i]);
			}
			echo "<p>&nbsp;</p>\n";
			echo "<a href=\"lists/listusers.php\">" . _("User list") . "</a>\n";
			echo "</body></html>\n";
		}
		else {
			// print success message
			StatusMessage("INFO", "", _("All changes were successful."));
			if ($_SESSION['config']->is_samba3()) {
				$doms = $_SESSION['ldap']->search_domains($_SESSION['config']->get_domainSuffix());
				echo "<p>&nbsp;</p>\n";
				if (sizeof($doms) == 0) {
					echo "<a href=\"domain.php?action=new\">" . _("No domains found, please create one.") . "</a>\n";
				}
				else {
					echo "<a href=\"lists/listusers.php\">" . _("User list") . "</a>\n";
				}
				echo "</body></html>\n";
			}
			else {
				echo "<p>&nbsp;</p>\n";
				echo "<a href=\"lists/listusers.php\">" . _("User list") . "</a>\n";
				echo "</body></html>\n";
			}
		}
	}
	else {
		// no suffixes were created
		StatusMessage("INFO", "", _("No changes were made."));
		echo "<p>&nbsp;</p>\n";
		echo "<a href=\"lists/listusers.php\">" . _("User list") . "</a>\n";
		echo "</body></html>\n";
	}
	exit;
}

// first show of page
$new_suff = $_GET['suffs'];
$new_suff = str_replace("\\", "", $new_suff);
$new_suff = str_replace("'", "", $new_suff);
$new_suff = explode(";", $new_suff);

echo $_SESSION['header'];
echo "<title>initsuff</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\">\n";
echo "</head><body>\n";
	echo "<p>&nbsp;</p>\n";
	echo "<p><font color=\"red\"><b>" . _("The following suffix(es) are missing in LDAP. LAM can create them for you.") . "</b></font></p>\n";
	echo "<p>&nbsp;</p>\n";
	// print missing suffixes
	for ($i = 0; $i < sizeof($new_suff); $i++) {
		echo "<p><b>" . $new_suff[$i] . "</b></p>\n";
	}
	echo "<p>&nbsp;</p>\n";
	echo "<form action=\"initsuff.php\" method=\"post\">\n";
	echo "<input type=\"hidden\" name=\"new_suff\" value=\"" . implode(";", $new_suff) . "\">\n";
	echo "<input type=\"submit\" name=\"add_suff\" value=\"" . _("Create") . "\">";
	echo "<input type=\"submit\" name=\"cancel\" value=\"" . _("Cancel") . "\">";
	echo "</form>\n";
echo "</body></html>\n";
?>
