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

  Manages Samba 3 domain entries.

*/
include_once ("../lib/config.inc");
include_once ("../lib/ldap.inc");

// start session
session_save_path("../sess");
@session_start();

setlanguage();


// add/edit domain
if (($_GET['action'] == "edit") || ($_GET['action'] == "new")) {
	// get list of domains
	$domlist = $_SESSION['ldap']->search_domains($_SESSION['config']->get_domainSuffix());
	// get possible suffixes
	$domsuff = $_SESSION['ldap']->search_units($_SESSION['config']->get_domainSuffix());
	if ($_GET['action'] == "edit") {
		// remove "\'"
		$_GET['DN'] = str_replace("\\'", "", $_GET['DN']);
		// load attributes from domain
		for ($i = 0; $i < sizeof($domlist); $i++) {
			if ($domlist[$i]->dn == $_GET['DN']) {
				$domain = $domlist[$i];
				break;
			}
		}
		// get suffix
		$tmp_arr = explode(",", $domain->dn);
		array_shift($tmp_arr);
		$domain_suffix = implode(",", $tmp_arr);
	}
	else {
		$domain = new samba3domain();
		$domain_suffix = $_SESSION['config']->get_domainSuffix();
	}
	// display page
	echo $_SESSION['header'];
	echo "<html>\n";
	echo "<head>\n";
	echo "<title>Domain Management</title>\n";
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\">\n";
	echo "</head>\n";
	echo "<body>\n";
	// print message, if needed
	if ($_SESSION['domain_message']) StatusMessage("INFO", $_SESSION['domain_message'], "");
		// print fieldset
		echo "<form action=\"domain.php\" method=\"post\">\n";
		echo "<p>&nbsp;</p>\n";
		echo "<fieldset>\n";
			echo "<legend><b>" . _("Domain Settings") . "</b></legend>\n";
			echo "<table border=0>\n";
				echo "<tr>\n";
					echo "<td>\n";
						// domain name
							echo "<b>" . _("Domain name") . ":</b>\n";
					echo "</td>\n";
					echo "<td>\n";
						if ($_GET['action'] == "edit") {
							echo $domain->name . "\n";
						}
						else echo "<input type=\"text\" name=\"dom_name\">\n";
					echo "</td>\n";
					echo ("<td><a href=\"help.php?HelpNumber=651\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
				echo "</tr>\n";
				echo "<tr>\n";
					echo "<td>&nbsp;</td><td>&nbsp;</td>\n";
				echo "</tr>\n";
				echo "<tr>\n";
					echo "<td>\n";
						// domain suffix
						echo "<b>" . _("Suffix") . ": </b>\n";
					echo "</td>\n";
					echo "<td>\n";
						echo "<select name=\"dom_suffix\">\n";
						for ($i = 0; $i < sizeof($domsuff); $i++) {
							if ($domsuff[$i] == $domain_suffix) echo "<option selected>" . $domain_suffix . "</option>\n";
							else echo "<option>" . $domsuff[$i] . "</option>\n";
						}
						echo "</select>";
					echo "</td>\n";
					echo ("<td><a href=\"help.php?HelpNumber=652\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
				echo "</tr>\n";
				echo "<tr>\n";
					echo "<td>&nbsp;</td><td>&nbsp;</td>\n";
				echo "</tr>\n";
				echo "<tr>\n";
					echo "<td>\n";
					// domain SID
					echo "<b>" . _("Domain SID") . ": </b>\n";
					echo "</td>\n";
					echo "<td>\n";
						if ($_GET['action'] == "edit") {
							echo $domain->SID . "\n";
						}
						else echo "<input type=\"text\" size=\"50\" name=\"dom_SID\">\n";
					echo "</td>\n";
					echo ("<td><a href=\"help.php?HelpNumber=653\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
				echo "</tr>\n";
				echo "<tr>\n";
					echo "<td>&nbsp;</td><td>&nbsp;</td>\n";
				echo "</tr>\n";
				echo "<tr>\n";
					// next RID
					echo "<td><b>" . _("Next RID") . ": </b></td>\n";
					echo "<td>\n";
						echo "<input type=\"text\" name=\"dom_nextRID\" value=\"" . $domain->nextRID . "\">\n";
					echo "</td>\n";
					echo ("<td><a href=\"help.php?HelpNumber=654\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
				echo "</tr>\n";
				echo "<tr>\n";
					// next user RID
					echo "<td><b>" . _("Next User RID") . ": </b></td>\n";
					echo "<td>\n";
						echo "<input type=\"text\" name=\"dom_nextUserRID\" value=\"" . $domain->nextUserRID . "\">\n";
					echo "</td>\n";
					echo ("<td><a href=\"help.php?HelpNumber=655\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
				echo "</tr>\n";
				echo "<tr>\n";
					// next group RID
					echo "<td><b>" . _("Next Group RID") . ": </b></td>\n";
					echo "<td>\n";
						echo "<input type=\"text\" name=\"dom_nextGroupRID\" value=\"" . $domain->nextGroupRID . "\">\n";
					echo "</td>\n";
					echo ("<td><a href=\"help.php?HelpNumber=656\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
				echo "</tr>\n";
				echo "<tr>\n";
					echo "<td>&nbsp;</td><td>&nbsp;</td>\n";
				echo "</tr>\n";
				echo "<tr>\n";
					// algorithmic RID base
					echo "<td><b>" . _("Algorithmic RID Base") . ": </b></td>\n";
					echo "<td>\n";
						if ($_GET['action'] == "edit") echo $domain->RIDbase . "\n";
						else echo "<input type=\"text\" name=\"dom_RIDbase\" value=\"" . $domain->RIDbase . "\">\n";
					echo "</td>\n";
					echo ("<td><a href=\"help.php?HelpNumber=657\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
				echo "</tr>\n";
			echo "</table>\n";
		echo "</fieldset>\n";
		// post DN and old RID values
		echo "<input type=\"hidden\" name=\"dom_DN\" value=\"" . $domain->dn . "\">";
		echo "<input type=\"hidden\" name=\"dom_oldnextRID\" value=\"" . $domain->nextRID . "\">";
		echo "<input type=\"hidden\" name=\"dom_oldnextUserRID\" value=\"" . $domain->nextUserRID . "\">";
		echo "<input type=\"hidden\" name=\"dom_oldnextGroupRID\" value=\"" . $domain->nextGroupRID . "\">";
		// edit or add operation
		if ($_GET['action'] == "edit") echo "<input type=\"hidden\" name=\"edit\" value=\"yes\">";
		else echo "<input type=\"hidden\" name=\"add\" value=\"yes\">";
		echo "<p>&nbsp;</p>\n";
		echo "<p>\n";
			echo "<input type=\"submit\" name=\"sub_save\" value=\"" . _("Submit") . "\">\n";
			echo "<input type=\"reset\" value=\"" . _("Reset") . "\">\n";
			echo "<input type=\"submit\" name=\"sub_back\" value=\"" . _("Cancel") . "\">\n";
		echo "</p>\n";
		echo "</form>\n";
	echo "</body>\n";
	echo "</html>\n";
}


// delete domain, ask if sure
elseif ($_GET['action'] == "delete") {
	// remove "\'" and make array
	$DNs = str_replace("\\'", "", $_GET['DN']);
	$DNs = explode(";", $DNs);
	// display page
	echo "<html>\n";
	echo "<head>\n";
	echo "<title>Domain Management</title>\n";
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\">\n";
	echo "</head>\n";
	echo "<body>\n";
		echo "<p>&nbsp;</p>\n";
		echo "<p><b><font color=\"red\">" . _("Delete domain(s)?") . "</font></b></p>\n";
		echo "<p>&nbsp;</p>\n";
		for ($i = 0; $i < sizeof($DNs); $i++) {
			echo "<p><b>" . $DNs[$i] . "</b></p>\n";
		}
	echo "<p>&nbsp;</p>\n";
	echo "<form action=\"domain.php\" method=\"post\">\n";
		echo "<input type=\"hidden\" name=\"delDN\" value=\"" . implode(";", $DNs) . "\">\n";
		echo "<input type=\"submit\" name=\"sub_delete\" value=\"" . _("Delete") . "\">\n";
		echo "<input type=\"submit\" name=\"sub_back\" value=\"" . _("Cancel") . "\">\n";
	echo "</form>";
	echo "</body>\n";
	echo "</html>\n";
}


// save domain
elseif ($_POST['sub_save']) {
	echo "<html>\n";
	echo "<head>\n";
	echo "<title>Domain Management</title>\n";
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\">\n";
	echo "</head>\n";
	echo "<body>\n";
	// check input
	if ($_POST['add'] && !eregi("^[a-z0-9_\\-]+$", $_POST['dom_name'])) StatusMessage("ERROR", "", _("Domain name is invalid!"));
	elseif ($_POST['add'] && !eregi("^S-[0-9]-[0-9]-[0-9]{2,2}-[0-9]*-[0-9]*-[0-9]*$", $_POST['dom_SID'])) {
		StatusMessage("ERROR", "", _("Samba 3 domain SID is invalid!"));
	}
	elseif ($_POST['dom_nextRID'] && !is_numeric($_POST['dom_nextRID'])) StatusMessage("ERROR", "", _("Next RID is not a number!"));
	elseif ($_POST['dom_nextUserRID'] && !is_numeric($_POST['dom_nextUserRID'])) StatusMessage("ERROR", "", _("Next user RID is not a number!"));
	elseif ($_POST['dom_nextGroupRID'] && !is_numeric($_POST['dom_nextGroupRID'])) StatusMessage("ERROR", "", _("Next group RID is not a number!"));
	elseif ($_POST['add'] && !is_numeric($_POST['dom_RIDbase'])) StatusMessage("ERROR", "", _("Algorithmic RID base is not a number!"));
	// edit entry
	elseif ($_POST['edit'] == "yes") {
		$success = true;
		// change attributes
		$attr = array();
		if ($_POST['dom_nextRID'] != $_POST['dom_oldnextRID']) $attr['sambaNextRid'] = $_POST['dom_nextRID'];
		if ($_POST['dom_nextUserRID'] != $_POST['dom_oldnextUserRID']) $attr['sambaNextUserRid'] = $_POST['dom_nextUserRID'];
		if ($_POST['dom_nextGroupRID'] != $_POST['dom_oldnextGroupRID']) $attr['sambaNextGroupRid'] = $_POST['dom_nextGroupRID'];
		if (sizeof($attr) > 0) $success = ldap_modify($_SESSION['ldap']->server(), $_POST['dom_DN'], $attr);
		// change suffix
		$RDN = explode(",", $_POST['dom_DN']);
		$RDN = $RDN[0];
		$newDN = $RDN . "," . $_POST['dom_suffix'];
		if ($_POST['dom_DN'] != $newDN) {
			$success = ldap_rename($_SESSION['ldap']->server(), $_POST['dom_DN'], $RDN, $_POST['dom_suffix'], true);
		}
		if ($success) StatusMessage("INFO", "Domain has been modified.", $DN);
		else StatusMessage("ERROR", "", "Failed to modify domain!");
	}
	// add entry
	else {
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
		if (ldap_add($_SESSION['ldap']->server(), $DN, $attr)) {
			StatusMessage("INFO", "Domain has been created.", $DN);
		}
		else StatusMessage("ERROR", "", "Failed to add domain!");
	}
	echo "<p>&nbsp;</p>\n";
	echo "<p><a href=\"lists/listdomains.php\">" . _("Back to domain list") . "</a></p>\n";
	echo "</body>\n";
	echo "</html>\n";
}


// back to list
elseif ($_POST['sub_back']) {
	metaRefresh("lists/listdomains.php");
}


// delete domain, user was sure
elseif ($_POST['sub_delete']) {
	$DNs = explode(";", $_POST['delDN']);
	// display page
	echo "<html>\n";
	echo "<head>\n";
	echo "<title>Domain Management</title>\n";
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\">\n";
	echo "</head>\n";
	echo "<body>\n";
	// delete DNs
	for ($i = 0; $i < sizeof($DNs); $i++) {
		if (ldap_delete($_SESSION['ldap']->server(), $DNs[$i])) StatusMessage("INFO", "Domain deleted successfully.", $DNs[$i]);
		else StatusMessage("ERROR", "Unable to delete domain!", $DNs[$i]);
	}
	echo "<p>&nbsp;</p>\n";
	echo "<p><a href=\"lists/listdomains.php\">" . _("Back to domain list") . "</a></p>\n";
	echo "</body>\n";
	echo "</html>\n";
}

?>
