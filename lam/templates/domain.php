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

echo ("<?xml version=\"1.0\" encoding=\"ISO-8859-15\"?>\n");
echo ("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n");


// add/edit domain
if (($_GET['action'] == "edit") || ($_GET['action'] == "new")) {
	// get list of domains
	$domlist = $_SESSION['ldap']->search_domains($_SESSION['config']->get_domainSuffix());
	// get possible suffixes
	$domsuff = $_SESSION['ldap']->search_units($_SESSION['config']->get_domainSuffix());
	if ($_GET['action'] == "edit") {
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
	echo "<html>\n";
	echo "<head>\n";
	echo "<title>Domain Management</title>\n";
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\">\n";
	echo "</head>\n";
	echo "<body>\n";
	// print message, if needed
	if ($_SESSION['domain_message']) echo "<p><font color=\"red\"><b>" . $_SESSION['domain_message'] . "</b></font></p><p>&nbsp;</p>";
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
				echo "</tr>\n";
				echo "<tr>\n";
					// next user RID
					echo "<td><b>" . _("Next User RID") . ": </b></td>\n";
					echo "<td>\n";
						echo "<input type=\"text\" name=\"dom_nextUserRID\" value=\"" . $domain->nextUserRID . "\">\n";
					echo "</td>\n";
				echo "</tr>\n";
				echo "<tr>\n";
					// next RID
					echo "<td><b>" . _("Next Group RID") . ": </b></td>\n";
					echo "<td>\n";
						echo "<input type=\"text\" name=\"dom_nextGroupRID\" value=\"" . $domain->nextGroupRID . "\">\n";
					echo "</td>\n";
				echo "</tr>\n";
				echo "<tr>\n";
					echo "<td>&nbsp;</td><td>&nbsp;</td>\n";
				echo "</tr>\n";
				echo "<tr>\n";
					// next RID
					echo "<td><b>" . _("Algorithmic RID Base") . ": </b></td>\n";
					echo "<td>\n";
						echo "<input type=\"text\" name=\"dom_RIDbase\" value=\"" . $domain->RIDbase . "\">\n";
					echo "</td>\n";
				echo "</tr>\n";
			echo "</table>\n";
		echo "</fieldset>\n";
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
}
// save domain
elseif ($_POST['sub_save']) {
}
// back to list
elseif ($_POST['sub_back']) {
}
// delete domain, user was sure
elseif ($_POST['sub_delete']) {
}

?>
