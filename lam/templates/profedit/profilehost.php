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

// start session
session_save_path("../../sess");
@session_start();

// check if user is logged in, if not go to login
if (!$_SESSION['ldap'] || !$_SESSION['ldap']->server()) {
	echo("<meta http-equiv=\"refresh\" content=\"0; URL=../login.php\">\n");
	exit;
}

// print header
echo ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
echo ("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n");
echo ("<html><head>\n<title></title>\n<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n</head><body><br>\n");

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
echo ("<td><a href=\"../help.php?HelpNumber=370\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
echo ("</tr>\n");

// empty row
echo ("<tr><td>&nbsp</td><td>&nbsp</td><td>&nbsp</td></tr>\n");

// domain
echo ("<tr>\n");
echo ("<td align=\"right\"><b>" . _("Domain") . ": </b></td>\n");
echo ("<td><input type=\"text\" value=\"" . $acct->smb_domain . "\" name=\"smb_domain\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=371\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
echo ("</tr>\n");

echo ("</table>\n");
echo ("</fieldset>\n");


echo ("<br><br>\n");

// profile name and submit/abort buttons
echo ("<table border=0>\n");
echo ("<tr>\n");
echo ("<td><b>" . _("Profile Name") . ":</b></td>\n");
echo ("<td><input type=\"text\" name=\"profname\" value=\"" . $_GET['edit'] . "\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=380\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
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
