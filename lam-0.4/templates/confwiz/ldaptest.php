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


  Configuration wizard - server settings second part
*/

include_once('../../lib/config.inc');
include_once('../../lib/ldap.inc');
include_once('../../lib/status.inc');

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

// check master password
$cfg = new CfgMain();
if ($cfg->password != $_SESSION['confwiz_masterpwd']) {
	require("../config/conflogin.php");
	exit;
}

// check if user clicked cancel button
if ($_POST['cancel']) {
	@unlink("../../config/" . $_SESSION['confwiz_config']->file . ".conf");
	metarefresh('../config/conflogin.php');
}

// check if all suffixes exist
$conf = $_SESSION['confwiz_config'];
$new_suffs = array();
if ($conf->get_UserSuffix() && ($conf->get_UserSuffix() != "")) {
	$info = @ldap_search($_SESSION['confwiz_ldap']->server, $conf->get_UserSuffix(), "", array());
	$res = @ldap_get_entries($_SESSION['confwiz_ldap']->server, $info);
	if (!$res && !in_array($conf->get_UserSuffix(), $new_suffs)) $new_suffs[] = $conf->get_UserSuffix();
}
if ($conf->get_GroupSuffix() && ($conf->get_GroupSuffix() != "")) {
	$info = @ldap_search($_SESSION['confwiz_ldap']->server, $conf->get_GroupSuffix(), "", array());
	$res = @ldap_get_entries($_SESSION['confwiz_ldap']->server, $info);
	if (!$res && !in_array($conf->get_GroupSuffix(), $new_suffs)) $new_suffs[] = $conf->get_GroupSuffix();
}
if ($conf->get_HostSuffix() && ($conf->get_HostSuffix() != "")) {
	$info = @ldap_search($_SESSION['confwiz_ldap']->server, $conf->get_HostSuffix(), "", array());
	$res = @ldap_get_entries($_SESSION['confwiz_ldap']->server, $info);
	if (!$res && !in_array($conf->get_HostSuffix(), $new_suffs)) $new_suffs[] = $conf->get_HostSuffix();
}
if ($conf->is_samba3() && $conf->get_DomainSuffix() && ($conf->get_DomainSuffix() != "")) {
	$info = @ldap_search($_SESSION['confwiz_ldap']->server, $conf->get_DomainSuffix(), "", array());
	$res = @ldap_get_entries($_SESSION['confwiz_ldap']->server, $info);
	if (!$res && !in_array($conf->get_DomainSuffix(), $new_suffs)) $new_suffs[] = $conf->get_DomainSuffix();
}

if (sizeof($new_suffs) > 0) {
	// check if user wanted to create suffixes
	if ($_POST['createsuff']) {
		$fail = array();
		$errors = array();
		// add entries
		for ($i = 0; $i < sizeof($new_suffs); $i++) {
			// check if entry is already present
			$info = @ldap_search($_SESSION['confwiz_ldap']->server, $new_suffs[$i], "", array());
			$res = @ldap_get_entries($_SESSION['confwiz_ldap']->server, $info);
			if ($res) continue;
			$suff = $new_suffs[$i];
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
				if (!@ldap_add($_SESSION['confwiz_ldap']->server(), $dn, $attr)) {
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
				if (!@ldap_add($_SESSION['confwiz_ldap']->server(), $dn, $attr)) {
					// check if we have to add parent entries
					if (ldap_errno($_SESSION['confwiz_ldap']->server()) == 32) {
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
							$info = @ldap_search($_SESSION['confwiz_ldap']->server, $subsuffs[$k], "", array());
							$res = @ldap_get_entries($_SESSION['confwiz_ldap']->server, $info);
							if (!$res) {
								$suffarray = explode(",", $subsuffs[$k]);
								$headarray = explode("=", $suffarray[0]);
								if ($headarray[0] == "ou") {  // add ou entry
									$attr = array();
									$attr['objectClass'] = 'organizationalunit';
									$attr['ou'] = $headarray[1];
									$dn = $subsuffs[$k];
									if (!@ldap_add($_SESSION['confwiz_ldap']->server(), $dn, $attr)) {
										$fail[] = $suff;
										$error[] = ldap_error($_SESSION['confwiz_ldap']->server());
										break;
									}
								}
								else {  // add root entry
									$attr = array();
									$attr['objectClass'] = 'organization';
									$attr[$headarray[0]] = $headarray[1];
									$dn = $subsuffs[$k];
									if (!@ldap_add($_SESSION['confwiz_ldap']->server(), $dn, $attr)) {
										$fail[] = $suff;
										$error[] = ldap_error($_SESSION['confwiz_ldap']->server());
										break;
									}
								}
							}
						}
					}
					else {
						$fail[] = $suff;
						$error[] = ldap_error($_SESSION['confwiz_ldap']->server());
					}
				}
			}
		}
		// show errors
		if (sizeof($fail) > 0) {
			echo $_SESSION['header'];
			echo "<title>";
					echo _("Configuration wizard");
			echo "</title>\n";
			echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
			echo "</head><body>\n";
				echo "<p align=\"center\"><a href=\"http://lam.sf.net\" target=\"_blank\">\n";
				echo "<img src=\"../../graphics/banner.jpg\" border=1 alt=\"LDAP Account Manager\"></a>\n";
				echo "</p>\n";
				echo "<hr>\n";
				echo "<p>&nbsp;</p>\n";
				// print failed suffixes
				for ($i = 0; $i < sizeof($fail); $i++) {
					StatusMessage("ERROR", _("Failed to create entry!") . "<br>" . $error[$i], $fail[$i]);
				}
				echo "<p>&nbsp;</p>\n";
				echo "<p><br><br><a href=\"server2.php\">" . _("Back to server settings") . "</a></p>\n";
			echo "</body></html>\n";
			exit;
		}
	}
	// show needed suffixes
	else {
		echo $_SESSION['header'];
		echo "<title>";
				echo _("Configuration wizard");
		echo "</title>\n";
		echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
		echo "</head><body>\n";
			echo "<p align=\"center\"><a href=\"http://lam.sf.net\" target=\"_blank\">\n";
			echo "<img src=\"../../graphics/banner.jpg\" border=1 alt=\"LDAP Account Manager\"></a>\n";
			echo "</p>\n";
			echo "<hr>\n";
			echo "<p>&nbsp;</p>\n";
			echo "<p><font color=\"red\"><b>" . _("The following suffix(es) are missing in LDAP. LAM will create them for you.") . "</b></font></p>\n";
			echo "<p>&nbsp;</p>\n";
			// print missing suffixes
			for ($i = 0; $i < sizeof($new_suffs); $i++) {
				echo "<p><b>" . $new_suffs[$i] . "</b></p>\n";
			}
			echo "<p>&nbsp;</p>\n";
			echo "<form action=\"ldaptest.php\" method=\"post\">\n";
			echo "<input type=\"submit\" name=\"createsuff\" value=\"" . _("Create") . "\">";
			echo "<input type=\"submit\" name=\"cancel\" value=\"" . _("Cancel") . "\">";
			echo "</form>\n";
		echo "</body></html>\n";
		exit;
	}
}

// check if domain object is present
if ($_SESSION['confwiz_config']->is_samba3()) {
		// get list of domains
		$domlist = $_SESSION['confwiz_ldap']->search_domains($_SESSION['confwiz_config']->get_domainSuffix());
	if (sizeof($domlist) < 1) {
		if ($_POST['createdom']) {
			// check input
			$suffix = $_SESSION['confwiz_config']->get_DomainSuffix();
			$server = $_SESSION['confwiz_ldap']->server;
			$filter = "(|(sambasid=" . $_POST['dom_SID'] . ")(sambadomainname=" . $_POST['dom_name'] . "))";
			$sr = @ldap_search($server, $suffix, $filter, array());
			$info = @ldap_get_entries($_SESSION["confwiz_ldap"]->server, $sr);
			$errors = array();
			// check for existing domains
			if ($info["count"] > 0) {
				$errors[] = _("This Samba 3 domain is already present!");
			}
			// check domain name
			if (!eregi("^[a-z0-9_\\-]+$", $_POST['dom_name'])) {
				$errors[] = _("Domain name is invalid!");
			}
			// check SID
			if (!eregi("^S-[0-9]-[0-9]-[0-9]{2,2}-[0-9]*-[0-9]*-[0-9]*$", $_POST['dom_SID'])) {
				$errors[] = _("Samba 3 domain SID is invalid!");
			}
			// check numbers
			if ($_POST['dom_nextRID'] && !is_numeric($_POST['dom_nextRID'])) {
				$errors[] = _("Next RID is not a number!");
			}
			if ($_POST['dom_nextUserRID'] && !is_numeric($_POST['dom_nextUserRID'])) {
				$errors[] = _("Next user RID is not a number!");
				}
			if ($_POST['dom_nextGroupRID'] && !is_numeric($_POST['dom_nextGroupRID'])) {
				$errors[] = _("Next group RID is not a number!");
			}
			if (!is_numeric($_POST['dom_RIDbase'])) {
				$errors[] = _("Algorithmic RID base is not a number!");
			}
			// try to create domain if no error occured
			if (sizeof($errors) < 1) {
				$DN = "sambaDomainName" . "=" . $_POST['dom_name'] . "," . $_POST['dom_suffix'];
				$attr = array();
				$attr['objectclass'] = "sambaDomain";
				$attr['sambaDomainName'] = $_POST['dom_name'];
				$attr['sambaSID'] = $_POST['dom_SID'];
				if ($_POST['dom_nextRID']) $attr['sambaNextRid'] = $_POST['dom_nextRID'];
				if ($_POST['dom_nextGroupRID']) $attr['sambaNextGroupRid'] = $_POST['dom_nextGroupRID'];
				if ($_POST['dom_nextUserRID']) $attr['sambaNextUserRid'] = $_POST['dom_nextUserRID'];
				$attr['sambaAlgorithmicRidBase'] = $_POST['dom_RIDbase'];
				// write to LDAP
				if (! @ldap_add($_SESSION['confwiz_ldap']->server(), $DN, $attr)) {
					$errors[] = _("Failed to add domain!") . "\n<br>" .  ldap_error($_SESSION['confwiz_ldap']->server());
				}
				else {
					// remember domain SID
					$_SESSION["confwiz_domainsid"] = $_POST['dom_SID'];
				}
			}
			// show error messages
			if (sizeof($errors) > 1) {
				echo $_SESSION['header'];
				echo "<title>";
						echo _("Configuration wizard");
				echo "</title>\n";
				echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
				echo "</head><body>\n";
					echo "<p align=\"center\"><a href=\"http://lam.sf.net\" target=\"_blank\">\n";
					echo "<img src=\"../../graphics/banner.jpg\" border=1 alt=\"LDAP Account Manager\"></a>\n";
					echo "</p>\n";
					echo "<hr>\n";
					echo "<p>&nbsp;</p>\n";
					// print errors
					for ($i = 0; $i < sizeof($errors); $i++) {
						StatusMessage("ERROR", $errors[$i], "");
					}
					echo "<p>&nbsp;</p>\n";
					echo "<p><br><br><a href=\"server2.php\">" . _("Back to server settings") . "</a></p>\n";
				echo "</body></html>\n";
				exit;
			}
		}
		else {
			// get possible suffixes
			$domsuff = $_SESSION['confwiz_ldap']->search_units($_SESSION['confwiz_config']->get_domainSuffix());
			echo $_SESSION['header'];
			echo "<title>";
					echo _("Configuration wizard");
			echo "</title>\n";
			echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
			echo "</head><body>\n";
				echo "<p align=\"center\"><a href=\"http://lam.sf.net\" target=\"_blank\">\n";
				echo "<img src=\"../../graphics/banner.jpg\" border=1 alt=\"LDAP Account Manager\"></a>\n";
				echo "</p>\n";
				echo "<hr>\n";
				echo "<p>&nbsp;</p>\n";
				echo "<p>". _("No domains found, please create one.") . "</p>\n";
				echo "<p>&nbsp;</p>\n";
				echo "<form action=\"ldaptest.php\" method=\"post\">\n";
				echo "<fieldset class=\"domedit\">\n";
					echo "<legend class=\"domedit\"><b>" . _("Domain Settings") . "</b></legend>\n";
					echo "<table border=0>\n";
						// domain name
						echo "<tr>\n";
							echo "<td>\n";
									echo "<b>" . _("Domain name") . ":</b>\n";
							echo "</td>\n";
							echo "<td>\n";
								echo "<input type=\"text\" name=\"dom_name\">\n";
							echo "</td>\n";
							echo ("<td><a href=\"../help.php?HelpNumber=651\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
						echo "</tr>\n";
						echo "<tr>\n";
							echo "<td>&nbsp;</td><td>&nbsp;</td>\n";
						echo "</tr>\n";
						// domain SID
						echo "<tr>\n";
							echo "<td>\n";
							echo "<b>" . _("Domain SID") . ": </b>\n";
							echo "</td>\n";
							echo "<td>\n";
								echo "<input type=\"text\" size=\"50\" name=\"dom_SID\">\n";
							echo "</td>\n";
							echo ("<td><a href=\"../help.php?HelpNumber=653\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
						echo "</tr>\n";
						echo "<tr>\n";
							echo "<td>&nbsp;</td><td>&nbsp;</td>\n";
						echo "</tr>\n";
						// next RID
						echo "<tr>\n";
							echo "<td><b>" . _("Next RID") . " " . _("(optional)") . ": </b></td>\n";
							echo "<td>\n";
								echo "<input type=\"text\" name=\"dom_nextRID\">\n";
							echo "</td>\n";
							echo ("<td><a href=\"../help.php?HelpNumber=654\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
						echo "</tr>\n";
						// next user RID
						echo "<tr>\n";
							echo "<td><b>" . _("Next User RID") . " " . _("(optional)") . ": </b></td>\n";
							echo "<td>\n";
								echo "<input type=\"text\" name=\"dom_nextUserRID\">\n";
							echo "</td>\n";
							echo ("<td><a href=\"../help.php?HelpNumber=655\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
						echo "</tr>\n";
						// next group RID
						echo "<tr>\n";
							echo "<td><b>" . _("Next Group RID") . " " . _("(optional)") . ": </b></td>\n";
							echo "<td>\n";
								echo "<input type=\"text\" name=\"dom_nextGroupRID\">\n";
							echo "</td>\n";
							echo ("<td><a href=\"../help.php?HelpNumber=656\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
						echo "</tr>\n";
						echo "<tr>\n";
							echo "<td>&nbsp;</td><td>&nbsp;</td>\n";
						echo "</tr>\n";
						// algorithmic RID base
						echo "<tr>\n";
							echo "<td><b>" . _("Algorithmic RID Base") . ": </b></td>\n";
							echo "<td>\n";
								echo "<input type=\"text\" name=\"dom_RIDbase\" value=\"1000\">\n";
							echo "</td>\n";
							echo ("<td><a href=\"../help.php?HelpNumber=657\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
						echo "</tr>\n";
						echo "<tr>\n";
							echo "<td>&nbsp;</td><td>&nbsp;</td>\n";
						echo "</tr>\n";
						// domain suffix
						echo "<tr>\n";
							echo "<td>\n";
								echo "<b>" . _("Suffix") . ": </b>\n";
							echo "</td>\n";
							echo "<td>\n";
								echo "<select name=\"dom_suffix\">\n";
								for ($i = 0; $i < sizeof($domsuff); $i++) {
									echo "<option>" . $domsuff[$i] . "</option>\n";
								}
								echo "</select>";
							echo "</td>\n";
							echo ("<td><a href=\"../help.php?HelpNumber=652\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
						echo "</tr>\n";
						echo "<tr>\n";
							echo "<td>&nbsp;</td><td>&nbsp;</td>\n";
						echo "</tr>\n";
					echo "</table>\n";
				echo "</fieldset>\n";
				echo "<p>&nbsp;</p>\n";
				echo "<input type=\"submit\" name=\"createdom\" value=\"" . _("Create") . "\">";
				echo "<input type=\"submit\" name=\"cancel\" value=\"" . _("Cancel") . "\">";
				echo "</form>\n";
			echo "</body></html>\n";
			exit;
		}
	}
	else {
		// remember domain SID
		$_SESSION["confwiz_domainsid"] = $domlist[0]->SID;
	}
}


// check if essential default Samba groups are present
if ($_SESSION['confwiz_config']->is_samba3() && !$_POST['creategroups'] && !$_POST['ignoregroups']) {
	$d512 = $d513 = $d514 = false;
	$suffix = $_SESSION['confwiz_config']->get_groupSuffix();
	$domSID = $_SESSION['confwiz_domainsid'];
	$filter = "(objectclass=sambagroupmapping)";
	$server = $_SESSION['confwiz_ldap']->server;
	$sr = @ldap_search($server, $suffix, $filter, array("sambaSID"));
	if ($sr) {
		$info = @ldap_get_entries($_SESSION["confwiz_ldap"]->server, $sr);
		if ($info) {
			// check SIDs
			array_shift($info);
			for ($i = 0; $i < sizeof($info); $i++) {
				if ($info[$i]['sambasid']['0'] == $domSID . "-512") {
					$d512 = true;
				}
				elseif ($info[$i]['sambasid']['0'] == $domSID . "-513") {
					$d513 = true;
				}
				elseif ($info[$i]['sambasid']['0'] == $domSID . "-514") {
					$d514 = true;
				}
			}
		}
	}
	// make a list of missing groups
	$missing_groups = array();
	if (!$d512)  {
		$temp = array();
		$temp['sambasid'] = $domSID . "-512";
		$temp['displayname'] = "Domain Admins";
		$temp['cn'] = "domainadmins";
		$missing_groups[] = $temp;
	}
	if (!$d513)  {
		$temp = array();
		$temp['sambasid'] = $domSID . "-513";
		$temp['displayname'] = "Domain Users";
		$temp['cn'] = "domainusers";
		$missing_groups[] = $temp;
	}
	if (!$d514)  {
		$temp = array();
		$temp['sambasid'] = $domSID . "-514";
		$temp['displayname'] = "Domain Guests";
		$temp['cn'] = "domainguests";
		$missing_groups[] = $temp;
	}
	$_SESSION['conwiz_missing_groups'] = $missing_groups;
	if (sizeof($missing_groups) > 0) {
		// show user a list of missing groups
		echo $_SESSION['header'];
		echo "<title>";
				echo _("Configuration wizard");
		echo "</title>\n";
		echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
		echo "</head><body>\n";
			echo "<p align=\"center\"><a href=\"http://lam.sf.net\" target=\"_blank\">\n";
			echo "<img src=\"../../graphics/banner.jpg\" border=1 alt=\"LDAP Account Manager\"></a>\n";
			echo "</p>\n";
			echo "<hr>\n";
			echo "<p>&nbsp;</p>\n";
			echo "<p>" . _("LAM detected that one or more essential Samba groups are missing. They are listed below.") .
			" " . _("LAM can create them for you or you have to create them manually later.") . "</p>\n";
			echo "<p>&nbsp;</p>\n";
			for ($i = 0; $i < sizeof($missing_groups); $i++) {
				echo "<p><b>" . _("Windows group name") . ": </b>" . $missing_groups[$i]['displayname'] . "<p>\n";
				echo "<p><b>" . _("Unix group name") . ": </b>" . $missing_groups[$i]['cn'] . "<p>\n";
				echo "<p><b>" . _("Group SID") . ": </b>" . $missing_groups[$i]['sambasid'] . "<p>\n";
				echo "<p>&nbsp;</p>\n";
				echo "<p>&nbsp;</p>\n";
			}
		echo "<form action=\"ldaptest.php\" method=\"post\">\n";
		echo "<input type=\"submit\" name=\"creategroups\" value=\"" . _("Create") . "\">";
		echo "<input type=\"submit\" name=\"ignoregroups\" value=\"" . _("Ignore") . "\">";
		echo "<input type=\"submit\" name=\"cancel\" value=\"" . _("Cancel") . "\">";
		echo "</form>\n";
		echo "</body></html>\n";
		exit;
	}
}

// create needed Samab groups
if ($_SESSION['confwiz_config']->is_samba3() && $_POST['creategroups']) {
	$suffix = $_SESSION['confwiz_config']->get_groupSuffix();
	$domSID = $_SESSION['confwiz_domainsid'];
	$filter = "(objectclass=posixgroup)";
	$server = $_SESSION['confwiz_ldap']->server;
	$sr = @ldap_search($server, $suffix, $filter, array("gidnumber"));
	if ($sr) {
		$info = @ldap_get_entries($_SESSION["confwiz_ldap"]->server, $sr);
		if ($info) {
			array_shift($info);
			// create list of GID numbers
			$gid_numbers = array();
			for ($i = 0; $i < sizeof($info); $i++) {
				// ignore GIDs that are out of range
				if ($info[$i]['gidnumber'][0] <= $_SESSION['confwiz_config']->get_maxGID()) {
					if ($info[$i]['gidnumber'][0] >= $_SESSION['confwiz_config']->get_minGID()) {
						$gid_numbers[] = $info[$i]['gidnumber'][0];
					}
				}
			}
			// if no GIDs are used add (minGID -1)
			if (sizeof($gid_numbers) < 1) $gid_numbers[] = $_SESSION['confwiz_config']->get_minGID() - 1;
			sort($gid_numbers);
			$missing_groups = $_SESSION['conwiz_missing_groups'];
			$errors = array();
			// check if free GID numbers exist
			if ($gid_numbers[sizeof($gid_numbers) - 1] < $_SESSION['confwiz_config']->get_maxGID() - 3) {
				$gidnumber = $gid_numbers[sizeof($gid_numbers) - 1];
				for ($i = 0; $i < sizeof($missing_groups); $i++) {
					$gidnumber++;
					$attributes = array();
					$attributes['objectclass'][] = 'posixGroup';
					$attributes['objectclass'][] = 'sambaGroupMapping';
					$attributes['sambaGroupType'] = 2;
					$attributes['gidnumber'] = $gidnumber;
					$attributes['sambaSID'] = $missing_groups[$i]['sambasid'];
					$attributes['description'] = $missing_groups[$i]['displayname'];
					$attributes['displayname'] = $missing_groups[$i]['displayname'];
					$attributes['cn'] = $missing_groups[$i]['cn'];
					$dn = 'cn=' . $attributes['cn'] . ',' . $_SESSION['confwiz_config']->get_groupSuffix();
					if (!ldap_add($_SESSION['confwiz_ldap']->server(), $dn, $attributes)) {
						$errors[] = 'Unable to create group:' . " " . $missing_groups[$i]['cn'];
					}
				}
			}
			// not enough free GIDs
			else {
				$errors[] = 'There are not enough free GID numbers in the GID range!';
			}
			if (sizeof($errors) < 1) {
				metarefresh('final.php');
			}
			else {
				echo $_SESSION['header'];
				echo "<title>";
						echo _("Configuration wizard");
				echo "</title>\n";
				echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
				echo "</head><body>\n";
					echo "<p align=\"center\"><a href=\"http://lam.sf.net\" target=\"_blank\">\n";
					echo "<img src=\"../../graphics/banner.jpg\" border=1 alt=\"LDAP Account Manager\"></a>\n";
					echo "</p>\n";
					echo "<hr>\n";
					echo "<p>&nbsp;</p>\n";
					// print errors
					for ($i = 0; $i < sizeof($errors); $i++) {
						StatusMessage("ERROR", $errors[$i], '');
					}
					echo "<p>&nbsp;</p>\n";
					echo "<p><br><br><a href=\"server2.php\">" . _("Back to server settings") . "</a></p>\n";
				echo "</body></html>\n";
				exit;
			}
		}
	}
	exit;
}

// if nothing is missing go to last page
metarefresh('final.php');

?>
