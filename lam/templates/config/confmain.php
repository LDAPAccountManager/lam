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




*/

include_once ("../../lib/config.inc");

// start session
session_save_path("../../sess");
session_start();

// check if button was pressed and if we have to save the setting or go back to login
if ($_POST['back'] || $_POST['submitconf']){
	// save settings
	if ($_POST['submitconf']){
		// save HTTP-POST variables in session
		// get data if register_globals is off
		if ($_POST['passwd']) $passwd = $_POST['passwd'];
		if ($_POST['passwd1']) $passwd1 = $_POST['passwd1'];
		if ($_POST['passwd2']) $passwd2 = $_POST['passwd2'];
		if ($_POST['serverurl']) $serverurl = $_POST['serverurl'];
		if ($_POST['admins']) $admins = $_POST['admins'];
		if ($_POST['suffusers']) $suffusers = $_POST['suffusers'];
		if ($_POST['suffgroups']) $suffgroups = $_POST['suffgroups'];
		if ($_POST['suffhosts']) $suffhosts = $_POST['suffhosts'];
		if ($_POST['minUID']) $minUID = $_POST['minUID'];
		if ($_POST['maxUID']) $maxUID = $_POST['maxUID'];
		if ($_POST['minGID']) $minGID = $_POST['minGID'];
		if ($_POST['maxGID']) $maxGID = $_POST['maxGID'];
		if ($_POST['minMach']) $minMach = $_POST['minMach'];
		if ($_POST['maxMach']) $maxMach = $_POST['maxMach'];
		if ($_POST['usrlstattr']) $usrlstattr = $_POST['usrlstattr'];
		if ($_POST['grplstattr']) $grplstattr = $_POST['grplstattr'];
		if ($_POST['hstlstattr']) $hstlstattr = $_POST['hstlstattr'];
		if ($_POST['maxlistentries']) $maxlistentries = $_POST['maxlistentries'];
		if ($_POST['language']) $language = $_POST['language'];
		if ($_POST['scriptpath']) $scriptpath = $_POST['scriptpath'];
		if ($_POST['samba3']) $samba3 = $_POST['samba3'];
		if ($_POST['domainSID']) $domainSID = $_POST['domainSID'];
		if ($_POST['scriptpath']) $scriptpath = $_POST['scriptpath'];
		else $scriptpath = "";
		if ($_POST['scriptserver']) $scriptserver = $_POST['scriptserver'];
		else $scriptserver = "";
		session_register('passwd', 'passwd1', 'passwd2', 'serverurl', 'admins', 'suffusers',
			'suffgroups', 'suffhosts', 'minUID', 'maxUID', 'minGID', 'maxGID', 'minMach',
			'maxMach', 'usrlstattr', 'grplstattr', 'hstlstattr', 'maxlistentries', 'language',
			'scriptpath', 'scriptserver', 'samba3', 'domainSID');
		echo("<meta http-equiv=\"refresh\" content=\"0; URL=confsave.php\">");
	}
	// back to login
	else if ($_POST['back']){
		echo("<meta http-equiv=\"refresh\" content=\"0; URL=../login.php\">");
		}
	exit;
}

// get password if register_globals is off
if ($_POST['passwd']) $passwd = $_POST['passwd'];

// check if password was entered
// if not: load login page
if (! $passwd) {
	$message = _("No password was entered!");
	require('conflogin.php');
	exit;
}

// check if password is valid
// if not: load login page
include_once ('../../lib/config.inc');
$conf = new Config();
if (!(($conf->get_Passwd()) == $passwd)) {
	$message = _("The password is invalid! Please try again.");
	require('conflogin.php');
	exit;
}

echo ("<?xml version=\"1.0\" encoding=\"ISO-8859-15\"?>\n");
echo ("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n");

echo ("<html>\n");
echo ("<head>\n");
echo ("<title>" . _("LDAP Account Manager Configuration") . "</title>\n");
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo ("</head>\n");
echo ("<body>\n");
echo ("<p align=\"center\"><a href=\"http://lam.sf.net\" target=\"new_window\">".
	"<img src=\"../../graphics/banner.jpg\" border=1 alt=\"LDAP Account Manager\"></a></p>\n<hr>\n<p></p>\n");

// display formular
echo ("<form action=\"confmain.php\" method=\"post\">\n");

echo ("<fieldset><legend><b>" . _("Server settings") . "</b></legend>");
echo ("<table border=0>");
// serverURL
echo ("<tr><td align=\"right\"><b>" . _("Server Address") . ": </b></td>".
	"<td align=\"left\">".
	"<input size=50 type=\"text\" name=\"serverurl\" value=\"" . $conf->get_ServerURL() . "\">".
	"</td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=201\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");

// new line
echo ("<tr><td colspan=3>&nbsp</td></tr>");

// user suffix
echo ("<tr><td align=\"right\"><b>".
	_("UserSuffix") . ": </b></td>".
	"<td><input size=50 type=\"text\" name=\"suffusers\" value=\"" . $conf->get_UserSuffix() . "\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=202\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
// group suffix
echo ("<tr><td align=\"right\"><b>".
	_("GroupSuffix") . ": </b></td>".
	"<td><input size=50 type=\"text\" name=\"suffgroups\" value=\"" . $conf->get_GroupSuffix() . "\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=202\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
// host suffix
echo ("<tr><td align=\"right\"><b>".
	_("HostSuffix") . ": </b></td>".
	"<td><input size=50 type=\"text\" name=\"suffhosts\" value=\"" . $conf->get_HostSuffix() . "\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=202\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");

echo ("</table>");
echo ("</fieldset>");
echo ("<p></p>");

echo ("<fieldset><legend><b>" . _("Samba settings") . "</b></legend>");
echo ("<table border=0>");

// Samba version
echo ("<tr><td align=\"right\"><b>".
	_("Samba 3.x schema") . ": </b></td><td><select name=\"samba3\">\n");
if ($conf->get_samba3() == "yes") echo ("<option>yes</option><option>no</option></select></td>");
else echo ("<option>no</option><option>yes</option></select></td>");
echo ("<td><a href=\"../help.php?HelpNumber=213\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");

// new line
echo ("<tr><td colspan=3>&nbsp</td></tr>");

// Samba domain SID
echo ("<tr><td align=\"right\"><b>".
	_("Domain SID (Samba 3 only)") . ": </b></td>".
	"<td><input size=50 type=\"text\" name=\"domainSID\" value=\"" . $conf->get_domainSID() . "\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=214\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");

echo ("</table>");
echo ("</fieldset>");
echo ("<p></p>");

echo ("<fieldset><legend><b>" . _("Ranges") . "</b></legend>");
echo ("<table border=0>");

// minUID
echo ("<tr><td align=\"right\"><b>".
	_("Minimum UID number") . ": </b>".
	"<input size=6 type=\"text\" name=\"minUID\" value=\"" . $conf->get_minUID() . "\"></td>\n");
// maxUID
echo ("<td align=\"right\"><b>" . _("Maximum UID number") . ": </b>".
	"<input size=6 type=\"text\" name=\"maxUID\" value=\"" . $conf->get_maxUID() . "\"></td>\n");
// UID text
echo ("<td><a href=\"../help.php?HelpNumber=203\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
// minGID
echo ("<tr><td align=\"right\"><b>".
	_("Minimum GID number") . ": </b>".
	"<input size=6 type=\"text\" name=\"minGID\" value=\"" . $conf->get_minGID() . "\"></td>\n");
// maxGID
echo ("<td align=\"right\"><b>" . _("Maximum GID number").": </b>".
	"<input size=6 type=\"text\" name=\"maxGID\" value=\"" . $conf->get_maxGID() . "\"></td>\n");
// GID text
echo ("<td><a href=\"../help.php?HelpNumber=204\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
// minMach
echo ("<tr><td align=\"right\"><b>".
	_("Minimum Machine number") . ": </b>".
	"<input size=6 type=\"text\" name=\"minMach\" value=\"" . $conf->get_minMachine() . "\"></td>\n");
// maxMach
echo ("<td align=\"right\"><b>" . _("Maximum Machine number") . ": </b>".
	"<input size=6 type=\"text\" name=\"maxMach\" value=\"" . $conf->get_maxMachine() . "\"></td>\n");
// Machine text
echo ("<td><a href=\"../help.php?HelpNumber=205\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");

echo ("</table>\n");
echo ("</fieldset>\n");
echo ("<p></p>\n");

echo ("<fieldset><legend><b>" . _("LDAP List settings") . "</b></legend>\n");
echo ("<table border=0>\n");

// user list attributes
echo ("<tr><td align=\"right\"><b>".
	_("Attributes in User List:") . "</b></td>".
	"<td><input size=50 type=\"text\" name=\"usrlstattr\" value=\"" . $conf->get_userlistAttributes() . "\"></td>");
echo ("<td><a href=\"../help.php?HelpNumber=206\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
// user list attributes
echo ("<tr><td align=\"right\"><b>".
	_("Attributes in Group List:") . "</b></td>".
	"<td><input size=50 type=\"text\" name=\"grplstattr\" value=\"" . $conf->get_grouplistAttributes() . "\"></td>");
echo ("<td><a href=\"../help.php?HelpNumber=206\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
// user list attributes
echo ("<tr><td align=\"right\"><b>".
	_("Attributes in Host List:") . "</b></td>".
	"<td><input size=50 type=\"text\" name=\"hstlstattr\" value=\"" . $conf->get_hostlistAttributes() . "\"></td>");
echo ("<td><a href=\"../help.php?HelpNumber=206\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");

echo ("<tr><td colspan=3>&nbsp</td></tr>\n");

// maximum list entries
echo ("<tr><td align=\"right\"><b>".
	_("Maximum list entries") . ": </b></td>".
	"<td><select name=\"maxlistentries\">\n<option selected>".$conf->get_MaxListEntries()."</option>\n");
if ($conf->get_MaxListEntries() != 10) echo("<option>10</option>\n");
if ($conf->get_MaxListEntries() != 20) echo("<option>20</option>\n");
if ($conf->get_MaxListEntries() != 30) echo("<option>30</option>\n");
if ($conf->get_MaxListEntries() != 50) echo("<option>50</option>\n");
if ($conf->get_MaxListEntries() != 75) echo("<option>75</option>\n");
if ($conf->get_MaxListEntries() != 100) echo("<option>100</option>\n");
echo ("</select></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=208\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");

echo ("</table>\n");
echo ("</fieldset>\n");
echo ("<p></p>\n");

echo ("<fieldset><legend><b>" . _("Language settings") . "</b></legend>\n");
echo ("<table border=0>\n");

// language
echo ("<tr>");
echo ("<td><b>" . _("Default Language") . "</b></td><td>\n");
// read available languages
$languagefile = "../../config/language.conf";
if(is_file($languagefile))
{
	$file = fopen($languagefile, "r");
	$i = 0;
	while(!feof($file))
	{
		$line = fgets($file, 1024);
		if($line == "\n" || $line[0] == "#" || $line == "") continue; // ignore comment and empty lines
		$languages[$i] = chop($line);
		$i++;
	}
	fclose($file);
// generate language list
echo ("<select name=\"language\">");
for ($i = 0; $i < sizeof($languages); $i++) {
	$entry = explode(":", $languages[$i]);
	if ($_SESSION['config']->get_defaultLanguage() != $languages[$i]) echo("<option value=\"" . $languages[$i] . "\">" . $entry[2] . "</option>\n");
	else echo("<option selected value=\"" . $languages[$i] . "\">" . $entry[2] . "</option>\n");
}
echo ("</select>\n");
}
else
{
	echo _("Unable to load available languages. For further instructions please contact the Admin of this site.\n");
}
echo ("</td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=209\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");

echo ("</table>\n");
echo ("</fieldset>\n");
echo ("<p></p>\n");

echo ("<fieldset><legend><b>" . _("Script settings") . "</b></legend>\n");
echo ("<table border=0>\n");

// script settings
echo ("<tr><td align=\"right\"><b>".
	_("Path to external script") . ": </b></td>".
	"<td><input size=50 type=\"text\" name=\"scriptpath\" value=\"" . $conf->get_scriptPath() . "\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=210\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
echo ("<tr><td align=\"right\"><b>".
	_("Server of external script") . ": </b></td>".
	"<td><input size=50 type=\"text\" name=\"scriptserver\" value=\"" . $conf->get_scriptServer() . "\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=211\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");

echo ("</table>\n");
echo ("</fieldset>\n");
echo ("<p></p>\n");


echo ("<fieldset><legend><b>" . _("Security settings") . "</b></legend>\n");
echo ("<table border=0>\n");
// admin list
echo ("<tr><td align=\"right\"><b>".
	_("List of valid users") . ": </b></td>".
	"<td colspan=2><input size=50 type=\"text\" name=\"admins\" value=\"" . $conf->get_Adminstring() . "\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=207\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
echo ("</table>\n");

echo ("<p></p>\n");

echo ("<table border=0>\n");
// new password
echo ("<tr><td bgcolor=\"red\" align=\"right\"><b>".
	_("New Password") . ": </b></td>".
	"<td bgcolor=\"red\" align=\"left\"><input type=\"password\" name=\"pass1\"></td>\n");
echo ("<td rowspan=2><a href=\"../help.php?HelpNumber=212\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
// reenter password
echo ("<tr><td bgcolor=\"red\" align=\"right\"><b>".
	_("Reenter Password") . ": </b></td>".
	"<td bgcolor=\"red\" align=\"left\"><input type=\"password\" name=\"pass2\"></td></tr>\n");
echo ("</table>\n");
echo ("</fieldset>\n");
echo ("<p></p>\n");


// buttons
echo ("<table border=0>\n");

echo ("<tr><td align=\"left\"><pre>".
	"<input type=\"submit\" name=\"submitconf\" value=\"" . _("Submit") . "\">".
	"<input type=\"reset\" name=\"resetconf\" value=\"" . _("Reset") . "\">".
	"<input type=\"submit\" name=\"back\" value=\"" . _("Abort") . "\"\n");

echo ("></pre></td></tr>\n");

echo ("</table>\n");

echo ("<p></p>");

// password for configuration
echo ("<p><input type=\"hidden\" name=\"passwd\" value=\"" . $passwd . "\"></p>\n");

echo ("</form>\n");
echo ("</body>\n");
echo ("</html>\n");

?>

