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

  Manages creating/changing of profiles.

*/

include_once("../../lib/profiles.inc");
include_once("../../lib/ldap.inc");
include_once("../../lib/account.inc");
include_once("../../lib/config.inc");

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

// check if user is logged in, if not go to login
if (!$_SESSION['ldap'] || !$_SESSION['ldap']->server()) {
	metaRefresh("../login.php");
	exit;
}

// print header
echo $_SESSION['header'];
echo "<html><head>\n<title></title>\n<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "<meta http-equiv=\"pragma\" content=\"no-cache\">\n";
echo "<meta http-equiv=\"cache-control\" content=\"no-cache\">\n";
echo "</head><body><br>\n";

$acct = new Account();

// check if profile should be edited
if ($_GET['edit']) {
	$acct = loadHostProfile($_GET['edit']);
}

// search available groups
$groups = findgroups();

// display formular
echo ("<form action=\"profilecreate.php?type=host\" method=\"post\">\n");


// Unix part
echo ("<fieldset><legend><b>" . _("Host attributes") . "</b></legend>\n");
echo ("<table border=0>\n");

// primary group
echo ("<tr>\n");
echo ("<td align=\"right\"><b>" . _("Primary group") . ": </b></td>\n");
echo ("<td><select name=\"general_group\">\n");
for ($i = 0; $i < sizeof($groups); $i++) {
	if ($acct->general_group == $groups[$i]) echo ("<option selected>" . $groups[$i] . "</option>\n");
	else echo ("<option>" . $groups[$i] . "</option>\n");
}
echo ("</select></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=412\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
echo ("</tr>\n");

// empty row
echo ("<tr><td>&nbsp</td><td>&nbsp</td><td>&nbsp</td></tr>\n");

// domain
echo ("<tr>\n");
echo ("<td align=\"right\"><b>" . _("Domain") . ": </b></td>\n");
if ($_SESSION['config']->is_samba3()) {
	echo "<td><select name=\"smb_domain\">\n";
	$doms = $_SESSION['ldap']->search_domains($_SESSION['config']->get_DomainSuffix());
	for ($i = 0; $i < sizeof($doms); $i++) {
		if (strtolower($acct->smb_domain->name) == strtolower($doms[$i]->name)) {
			echo ("<option selected value=\"" . $acct->smb_domain->dn . "\">" . $acct->smb_domain->name . "</option>\n");
		}
		else {
			echo ("<option value=\"" . $doms[$i]->dn . "\">" . $doms[$i]->name . "</option>\n");
		}
	}
	echo "</select></td>\n";
}
else {
	echo ("<td><input type=\"text\" value=\"" . $acct->smb_domain . "\" name=\"smb_domain\"></td>\n");
}
echo ("<td><a href=\"../help.php?HelpNumber=460\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
echo ("</tr>\n");

echo ("</table>\n");
echo ("</fieldset>\n");


echo ("<br><br>\n");

// profile name and submit/abort buttons
echo ("<table border=0>\n");
echo ("<tr>\n");
echo ("<td><b>" . _("Profile name") . ":</b></td>\n");
echo ("<td><input type=\"text\" name=\"profname\" value=\"" . $_GET['edit'] . "\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=360\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
echo ("</tr>\n");
echo ("<tr>\n");
echo ("<td colspan=2>&nbsp</td>");
echo ("</tr>\n");
echo ("<tr>\n");
echo ("<td><input type=\"submit\" name=\"submit\" value=\"" . _("Save") . "\"></td>\n");
echo ("<td><input type=\"reset\" name=\"reset\" value=\"" . _("Reset") . "\">\n");
echo ("<input type=\"submit\" name=\"abort\" value=\"" . _("Abort") . "\"></td>\n");
echo ("<td>&nbsp</td>");
echo ("</tr>\n");
echo ("</table>\n");

echo ("</form></body></html>\n");



?>
