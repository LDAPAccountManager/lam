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
echo ("<html><head>\n<title></title>\n<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n</head><body><br>\n");

$acct = new Account();

// get list of login shells
$shelllist =  file('../../config/shells');
$hells = array();
for ($i = 0; $i < sizeof($shelllist); $i++) {
	$shelllist[$i] = chop($shelllist[$i]);
	$shelllist[$i] = trim($shelllist[$i]);
	if ($shelllist[$i] != "") $shells[sizeof($shells)] = $shelllist[$i];
}

// check if profile should be edited
if ($_GET['edit']) {
	$acct = loadUserProfile($_GET['edit']);
}

// search available groups
$groups = findgroups();

// calculate date for unix password expiry
if ($acct->unix_pwdexpire) {
$tstamp = $acct->unix_pwdexpire;
$tdate = date(dmY, $acct->unix_pwdexpire);
$unix_pwdexpire_day = substr($tdate, 0, 2);
$unix_pwdexpire_mon = substr($tdate, 2, 2);
$unix_pwdexpire_yea = substr($tdate, 4, 4);
}

// display formular
echo ("<form action=\"profilecreate.php?type=user\" method=\"post\">\n");


// Unix part
echo ("<fieldset><legend><b>" . _("Unix account") . "</b></legend>\n");
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
echo ("<td><a href=\"../help.php?HelpNumber=406\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
echo ("</tr>\n");

// additional groups
echo ("<tr>\n");
echo ("<td align=\"right\"><b>" . _("Additional groups") . ": </b></td>\n");
echo ("<td><select name=\"general_groupadd[]\" size=5 multiple>\n");
for ($i = 0; $i < sizeof($groups); $i++) {
	if ($acct->general_groupadd && in_array($groups[$i], $acct->general_groupadd)) echo ("<option selected>" . $groups[$i] . "</option>\n");
	else echo ("<option>" . $groups[$i] . "</option>\n");
}
echo ("</select></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=402\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
echo ("</tr>\n");

// empty row
echo ("<tr><td>&nbsp</td><td>&nbsp</td><td>&nbsp</td></tr>\n");

// path to home directory
echo ("<tr>\n");
echo ("<td align=\"right\"><b>" . _("Home Directory") . ": </b></td>\n");
echo ("<td><input type=\"text\" value=\"" . $acct->general_homedir . "\" name=\"general_homedir\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=403\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
echo ("</tr>\n");

// login shell
echo ("<tr>\n");
echo ("<td align=\"right\"><b>" . _("Login shell") . ": </b></td>\n");
echo ("<td><select name=\"general_shell\">\n");
for ($i = 0; $i < sizeof($shells); $i++) {
	if ($shells[$i] == $acct->general_shell) echo ("<option selected>" . $shells[$i] . "</option>\n");
	else echo ("<option>" . $shells[$i] . "</option>\n");
}
echo ("</select></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=405\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
echo ("</tr>\n");

// empty row
echo ("<tr><td>&nbsp</td><td>&nbsp</td><td>&nbsp</td></tr>\n");

// no Unix password
echo ("<tr>\n");
echo ("<td align=\"right\"><b>" . _("Set Unix Password") . ": </b></td>\n");
echo ("<td><select name=\"unix_password_no\">\n");
if ($acct->unix_password_no == "1") echo ("<option selected value=1>"._("no")."</option><option value=0>"._("yes")."</option>\n");
else echo ("<option selected value=0>"._("yes")."</option><option value=1>"._("no")."</option>\n");
echo ("</select></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=426\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
echo ("</tr>\n");

// Unix: password expiry warn
echo ("<tr>\n");
echo ("<td align=\"right\"><b>" . _("Password warning") . ": </b></td>\n");
echo ("<td><input type=\"text\" name=\"unix_pwdwarn\" value=\"" . $acct->unix_pwdwarn . "\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=414\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
echo ("</tr>\n");

// Unix: password expiry
echo ("<tr>\n");
echo ("<td align=\"right\"><b>" . _("Password expiry") . ": </b></td>\n");
echo ("<td><input type=\"text\" name=\"unix_pwdallowlogin\" value=\"" . $acct->unix_pwdallowlogin . "\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=415\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
echo ("</tr>\n");

// maximum password age
echo ("<tr>\n");
echo ("<td align=\"right\"><b>" . _("Maximum password age") . ": </b></td>\n");
echo ("<td><input type=\"text\" name=\"unix_pwdmaxage\" value=\"" . $acct->unix_pwdmaxage . "\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=416\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
echo ("</tr>\n");

// minimum password age
echo ("<tr>\n");
echo ("<td align=\"right\"><b>" . _("Minimum password age") . ": </b></td>\n");
echo ("<td><input type=\"text\" name=\"unix_pwdminage\" value=\"" . $acct->unix_pwdminage . "\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=417\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
echo ("</tr>\n");

// password expire date
echo ("<tr>\n");
echo ("<td align=\"right\"><b>" . _("Account expires on") . ": </b></td>\n");
echo ("<td>\n");
echo ("<select name=\"unix_pwdexpire_day\">\n");
for ( $i=1; $i<=31; $i++ ) {
	if ($unix_pwdexpire_day == $i) echo "<option selected>$i</option>\n";
	else echo "<option>$i</option>\n";
}
echo ("</select>\n");
echo ("<select name=\"unix_pwdexpire_mon\">\n");
for ( $i=1; $i<=12; $i++ ) {
	if ($unix_pwdexpire_mon == $i) echo "<option selected>$i</option>\n";
	else echo "<option>$i</option>\n";
}
echo ("</select>\n");
echo ("<select name=\"unix_pwdexpire_yea\">");
for ( $i=2003; $i<=2030; $i++ ) {
	if ($unix_pwdexpire_yea == $i) echo "<option selected>$i</option>\n";
	else echo "<option>$i</option>\n";
}
echo ("</select></td>");
echo ("<td><a href=\"../help.php?HelpNumber=418\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
echo ("</tr>\n");

// empty row
echo ("<tr><td>&nbsp</td><td>&nbsp</td><td>&nbsp</td></tr>\n");

// unix workstations
echo ("<tr>\n");
echo ("<td align=\"right\"><b>" . _("Unix workstations") . ": </b></td>\n");
echo ("<td><input type=\"text\" name=\"unix_host\" value=\"" . $acct->unix_host . "\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=466\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
echo ("</tr>\n");

// empty row
echo ("<tr><td>&nbsp</td><td>&nbsp</td><td>&nbsp</td></tr>\n");

// deactivate account
echo ("<tr>\n");
echo ("<td align=\"right\"><b>" . _("Account is deactivated") . ": </b></td>\n");
echo ("<td><select name=\"unix_deactivated\">\n");
if ($acct->unix_deactivated == "1") echo ("<option selected value=1>"._("yes")."</option><option value=0>"._("no")."</option>\n");
else echo ("<option selected value=0>"._("no")."</option><option value=1>"._("yes")."</option>\n");
echo ("</select></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=427\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
echo ("</tr>\n");


echo ("</table>\n");
echo ("</fieldset>\n");
echo ("<br>");



// Samba part
echo ("<fieldset><legend><b>" . _("Samba account") . "</b></legend>\n");
echo ("<table border=0>\n");

// no Samba password
echo ("<tr>\n");
echo ("<td align=\"right\"><b>" . _("Set Samba password") . ": </b></td>\n");
echo ("<td><select name=\"smb_password_no\">\n");
if ($acct->smb_password_no == "1") echo ("<option selected value=1>"._("no")."</option><option value=0>"._("yes")."</option>\n");
else echo ("<option selected value=0>"._("yes")."</option><option value=1>"._("no")."</option>\n");
echo ("</select></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=426\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
echo ("</tr>\n");

// use Unix password as Samba password
echo ("<tr>\n");
echo ("<td align=\"right\"><b>" . _("Set Unix password for Samba") . ": </b></td>\n");
echo ("<td><select name=\"smb_useunixpwd\">\n");
if ($acct->smb_useunixpwd == "0") echo ("<option selected value=0>"._("no")."</option><option value=1>"._("yes")."</option>\n");
else echo ("<option selected value=1>"._("yes")."</option><option value=0>"._("no")."</option>\n");
echo ("</select></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=301\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
echo ("</tr>\n");

// password expires
echo ("<tr>\n");
echo ("<td align=\"right\"><b>" . _("Password does not expire") . ": </b></td>\n");
echo ("<td><select name=\"smb_flagsD\">\n");
if ($acct->smb_flagsD == "0") echo ("<option selected value=0>"._("no")."</option><option value=1>"._("yes")."</option>\n");
else echo ("<option selected value=1>"._("yes")."</option><option value=0>"._("no")."</option>\n");
echo ("</select></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=429\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
echo ("</tr>\n");

// empty row
echo ("<tr><td>&nbsp</td><td>&nbsp</td><td>&nbsp</td></tr>\n");

// drive letter for home directory
echo ("<tr>\n");
echo ("<td align=\"right\"><b>" . _("Home drive") . ": </b></td>\n");
echo ("<td><select name=\"smb_homedrive\">\n");
if ($acct->smb_homedrive == "D:") echo "<option selected>D:</option>"; else echo "<option>D:</option>\n";
if ($acct->smb_homedrive == "E:") echo "<option selected>E:</option>"; else echo "<option>E:</option>\n";
if ($acct->smb_homedrive == "F:") echo "<option selected>F:</option>"; else echo "<option>F:</option>\n";
if ($acct->smb_homedrive == "G:") echo "<option selected>G:</option>"; else echo "<option>G:</option>\n";
if ($acct->smb_homedrive == "H:") echo "<option selected>H:</option>"; else echo "<option>H:</option>\n";
if ($acct->smb_homedrive == "I:") echo "<option selected>I:</option>"; else echo "<option>I:</option>\n";
if ($acct->smb_homedrive == "J:") echo "<option selected>J:</option>"; else echo "<option>J:</option>\n";
if ($acct->smb_homedrive == "K:") echo "<option selected>K:</option>"; else echo "<option>K:</option>\n";
if ($acct->smb_homedrive == "L:") echo "<option selected>L:</option>"; else echo "<option>L:</option>\n";
if ($acct->smb_homedrive == "M:") echo "<option selected>M:</option>"; else echo "<option>M:</option>\n";
if ($acct->smb_homedrive == "N:") echo "<option selected>N:</option>"; else echo "<option>N:</option>\n";
if ($acct->smb_homedrive == "O:") echo "<option selected>O:</option>"; else echo "<option>O:</option>\n";
if ($acct->smb_homedrive == "P:") echo "<option selected>P:</option>"; else echo "<option>P:</option>\n";
if ($acct->smb_homedrive == "Q:") echo "<option selected>Q:</option>"; else echo "<option>Q:</option>\n";
if ($acct->smb_homedrive == "R:") echo "<option selected>R:</option>"; else echo "<option>R:</option>\n";
if ($acct->smb_homedrive == "S:") echo "<option selected>S:</option>"; else echo "<option>S:</option>\n";
if ($acct->smb_homedrive == "T:") echo "<option selected>T:</option>"; else echo "<option>T:</option>\n";
if ($acct->smb_homedrive == "U:") echo "<option selected>U:</option>"; else echo "<option>U:</option>\n";
if ($acct->smb_homedrive == "V:") echo "<option selected>V:</option>"; else echo "<option>V:</option>\n";
if ($acct->smb_homedrive == "W:") echo "<option selected>W:</option>"; else echo "<option>W:</option>\n";
if ($acct->smb_homedrive == "X:") echo "<option selected>X:</option>"; else echo "<option>X:</option>\n";
if ($acct->smb_homedrive == "Y:") echo "<option selected>Y:</option>"; else echo "<option>Y:</option>\n";
if ($acct->smb_homedrive == "Z:") echo "<option selected>Z:</option>"; else echo "<option>Z:</option>\n";
echo ("</select></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=433\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
echo ("</tr>\n");

// path to home directory
echo ("<tr>\n");
echo ("<td align=\"right\"><b>" . _("Home path") . ": </b></td>\n");
echo ("<td><input type=\"text\" value=\"" . $acct->smb_smbhome . "\" name=\"smb_smbhome\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=437\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
echo ("</tr>\n");

// path to profile
echo ("<tr>\n");
echo ("<td align=\"right\"><b>" . _("Profile path") . ": </b></td>\n");
echo ("<td><input type=\"text\" value=\"" . $acct->smb_profilePath . "\" name=\"smb_profilepath\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=435\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
echo ("</tr>\n");

// path to logon scripts
echo ("<tr>\n");
echo ("<td align=\"right\"><b>" . _("Script path") . ": </b></td>\n");
echo ("<td><input type=\"text\" value=\"" . $acct->smb_scriptPath . "\" name=\"smb_scriptPath\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=434\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
echo ("</tr>\n");

// empty row
echo ("<tr><td>&nbsp</td><td>&nbsp</td><td>&nbsp</td></tr>\n");

// workstations
echo ("<tr>\n");
echo ("<td align=\"right\"><b>" . _("Workstations") . ": </b></td>\n");
echo ("<td><input type=\"text\" value=\"" . $acct->smb_smbuserworkstations . "\" name=\"smb_smbuserworkstations\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=436\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
echo ("</tr>\n");

// empty row
echo ("<tr><td>&nbsp</td><td>&nbsp</td><td>&nbsp</td></tr>\n");

// domain
echo ("<tr>\n");
echo ("<td align=\"right\"><b>" . _("Domain") . ": </b></td>\n");
if ($_SESSION['config']->get_samba3() == "yes") {
	echo "<td><select name=\"smb_domain\">\n";
	$doms = $_SESSION['ldap']->search_domains($_SESSION['config']->get_DomainSuffix());
	for ($i = 0; $i < sizeof($doms); $i++) {
		if (strtolower($acct->smb_domain->name) == strtolower($doms[$i]->name)) {
			echo ("<option selected value=\"" . $act->smb_domain->dn . "\">" . $acct->smb_domain->name . "</option>\n");
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
echo ("<td><a href=\"../help.php?HelpNumber=438\" target=\"lamhelp\">" . _("Help") . "</a></td>\n");
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
