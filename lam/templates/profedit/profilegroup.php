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

// load quota list
if ($_SESSION['config']->get_scriptPath()) {
	$tempacc = new account();
	$tempacc->type = "group";
	$acct_q = getquotas(array($tempacc));
}

// print header
echo $_SESSION['header'];
echo "<title></title>\n<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head><body><br>\n";

$acct = new Account();

// check if profile should be edited
if ($_GET['edit']) {
	$acct = loadGroupProfile($_GET['edit']);
}

// display formular
echo ("<form action=\"profilecreate.php?type=group\" method=\"post\">\n");

if ($_SESSION['config']->is_samba3()) {
	// Samba part
	echo ("<fieldset><legend><b>" . _("Samba") . "</b></legend>\n");
	echo ("<table border=0>\n");

	// domain
	echo ("<tr>\n");
	echo ("<td align=\"right\"><b>" . _("Domain") . ": </b></td>\n");
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
	echo ("<td><a href=\"../help.php?HelpNumber=438\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
	echo ("</tr>\n");

	echo ("</table>\n");
	echo ("</fieldset>\n");
}


// Quota settings if script is given
if ($_SESSION['config']->get_scriptPath()) {
	echo ("<br>");
	echo "<fieldset><legend><b>"._('Quota properties')."</b></legend>\n";
	echo "<table border=0>\n";
	// description line
	echo "<tr>\n";
	echo "<td align=\"center\"><b>" . _('Mountpoint') . "</b>&nbsp;&nbsp;</td>\n";
	echo "<td align=\"center\"><b>" . _('Soft block limit') . "</b>&nbsp;&nbsp;</td>\n";
	echo "<td align=\"center\"><b>" . _('Hard block limit') . "</b>&nbsp;&nbsp;</td>\n";
	echo "<td align=\"center\"><b>" . _('Soft inode limit') . "</b>&nbsp;&nbsp;</td>\n";
	echo "<td align=\"center\"><b>" . _('Hard inode limit') . "</b>&nbsp;&nbsp;</td>\n";
	echo "</tr>\n";
	// help line
	echo "<tr><td align=\"center\"><a href=\"../help.php?HelpNumber=439\" target=\"lamhelp\">"._('Help').'</a></td>'."\n".
		"<td align=\"center\"><a href=\"../help.php?HelpNumber=441\" target=\"lamhelp\">"._('Help').'</a></td>'."\n".
		"<td align=\"center\"><a href=\"../help.php?HelpNumber=442\" target=\"lamhelp\">"._('Help').'</a></td>'."\n".
		"<td align=\"center\"><a href=\"../help.php?HelpNumber=445\" target=\"lamhelp\">"._('Help').'</a></td>'."\n".
		"<td align=\"center\"><a href=\"../help.php?HelpNumber=446\" target=\"lamhelp\">"._('Help').'</a></td>'."\n".
		'</tr>'."\n";
	// quota settings
	for ($i = 0; $i < (sizeof($acct_q[0]->quota)); $i++) {
		// load values from profile
		for ($k = 0; $k < sizeof($acct->quota); $k++) {
			// check for equal mountpoints
			if ($acct->quota[$k][0] == $acct_q[0]->quota[$i][0]) {
				$acct_q[0]->quota[$i][2] = $acct->quota[$i][2];
				$acct_q[0]->quota[$i][3] = $acct->quota[$i][3];
				$acct_q[0]->quota[$i][6] = $acct->quota[$i][6];
				$acct_q[0]->quota[$i][7] = $acct->quota[$i][7];
			}
		}
		echo "<tr>\n";
		echo '<td>' . $acct_q[0]->quota[$i][0] . "<input type=\"hidden\" name=\"f_quota_" . $i . "_0\" value=\"" . $acct_q[0]->quota[$i][0] . "\"></td>\n"; // mountpoint
		echo '<td align="center"><input name="f_quota_' . $i . '_2" type="text" size="12" maxlength="20" value="' . $acct_q[0]->quota[$i][2] . "\"></td>\n"; // blocks soft limit
		echo '<td align="center"><input name="f_quota_' . $i . '_3" type="text" size="12" maxlength="20" value="' . $acct_q[0]->quota[$i][3] . "\"></td>\n"; // blocks hard limit
		echo '<td align="center"><input name="f_quota_' . $i . '_6" type="text" size="12" maxlength="20" value="' . $acct_q[0]->quota[$i][6] . "\"></td>\n"; // inodes soft limit
		echo '<td align="center"><input name="f_quota_' . $i . '_7" type="text" size="12" maxlength="20" value="' . $acct_q[0]->quota[$i][7] . "\"></td>\n"; // inodes hard limit
		echo "</tr>\n";
	}
	echo "</table>\n";
	// save number of mountpoints
	echo "<input type=\"hidden\" name=\"quotacount\" value=\"" . (sizeof($acct_q[0]->quota)) . "\">\n";
	echo "</fieldset>\n";
}

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
