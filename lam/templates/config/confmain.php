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
		else $scriptpath = "";
		if ($_POST['scriptserver']) $scriptserver = $_POST['scriptserver'];
		else $scriptserver = "";
		session_register('passwd', 'passwd1', 'passwd2', 'serverurl', 'admins', 'suffusers',
			'suffgroups', 'suffhosts', 'minUID', 'maxUID', 'minGID', 'maxGID', 'minMach',
			'maxMach', 'usrlstattr', 'grplstattr', 'hstlstattr', 'maxlistentries', 'language',
			'scriptpath', 'scriptserver');
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
	require('conflogin.php');
	exit;
}

// check if password is valid
// if not: load login page
include_once ('../../lib/config.inc');
$conf = new Config();
if (!(($conf->get_Passwd()) == $passwd)) {
	require('conflogin.php');
	exit;
}

echo ("<html>\n");
echo ("<head>\n");
echo ("<title>" . _("LDAP Account Manager Configuration") . "</title>\n");
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo ("</head>\n");
echo ("<body>\n");
echo ("<p align=\"center\"><a href=\"http://lam.sf.net\" target=\"new_window\">".
	"<img src=\"../../graphics/banner.jpg\" border=1 alt=\"LDAP Account Manager\"></a></p><hr><br><br>\n");

// display formular
echo ("<form action=\"confmain.php\" method=\"post\">\n");

echo ("<fieldset><legend><b>" . _("Server settings") . "</b></legend>");
echo ("<table align=\"center\" border=\"0\">");
// serverURL
echo ("<tr><td width=\"20%\"><p align=\"right\"><b>" . _("Server Address") . ": </b></p></td>".
	"<td width=\"30%\"><p align=\"left\">".
	"<input size=50 type=\"text\" name=\"serverurl\" value=\"" . $conf->get_ServerURL() . "\">".
	"</td>\n");
echo ("<td width=\"50%\"><p align=\"left\">".
	 _("Server Address of LDAP server (e.g. ldap://localhost:389 or ldaps://localhost:636)").
	 "</p></td></tr>\n");

// new line
echo ("<tr><td>&nbsp</td></tr>");

// user suffix
echo ("<tr><td><p align=\"right\"><b>".
	_("UserSuffix") . ": </b></p></td>".
	"<td><input size=50 type=\"text\" name=\"suffusers\" value=\"" . $conf->get_UserSuffix() . "\"></td>\n");
echo ("<td><p align=\"left\">".
	_("This is the suffix from where to search for users.<br>(e.g. ou=People,dc=yourcompany,dc=com)").
	"</p></td></tr>\n");
// group suffix
echo ("<tr><td><p align=\"right\"><b>".
	_("GroupSuffix") . ": </b></p></td>".
	"<td><input size=50 type=\"text\" name=\"suffgroups\" value=\"" . $conf->get_GroupSuffix() . "\"></td>\n");
echo ("<td><p align=\"left\">".
	_("This is the suffix from where to search for groups.<br>(e.g. ou=group,dc=yourcompany,dc=com)").
	"</p></td></tr>\n");
// host suffix
echo ("<tr><td><p align=\"right\"><b>".
	_("HostSuffix") . ": </b></p></td>".
	"<td><input size=50 type=\"text\" name=\"suffhosts\" value=\"" . $conf->get_HostSuffix() . "\"></td>\n");
echo ("<td><p align=\"left\">".
	_("This is the suffix from where to search for Samba hosts.<br>(e.g. ou=machines,dc=yourcompany,dc=com)").
	"</p></td></tr>\n");

echo ("</table>");
echo ("</fieldset>");
echo ("<br>");

echo ("<fieldset><legend><b>" . _("Ranges") . "</b></legend>");
echo ("<table align=\"center\" border=\"0\">");

// minUID
echo ("<tr><td align=\"right\"><b>".
	_("Minimum UID number") . ": </b></td>".
	"<td align=\"left\"><input size=6 type=\"text\" name=\"minUID\" value=\"" . $conf->get_minUID() . "\">\n");
// maxUID
echo ("&nbsp <b>" . _("Maximum UID number") . ": </b>".
	"<input size=6 type=\"text\" name=\"maxUID\" value=\"" . $conf->get_maxUID() . "\"></td>\n");
// UID text
echo ("<td><p align=\"left\">".
	_("These are the minimum and maximum numbers to use for user IDs").
	"</p></td></tr>\n");
// minGID
echo ("<tr><td align=\"right\"><b>".
	_("Minimum GID number") . ": </b></td>".
	"<td align=\"left\"><input size=6 type=\"text\" name=\"minGID\" value=\"" . $conf->get_minGID() . "\">\n");
// maxGID
echo ("&nbsp <b>" . _("Maximum GID number").": </b>".
	"<input size=6 type=\"text\" name=\"maxGID\" value=\"" . $conf->get_maxGID() . "\"></td>\n");
// GID text
echo ("<td><p align=\"left\">".
	_("These are the minimum and maximum numbers to use for group IDs").
	"</p></td></tr>\n");
// minMach
echo ("<tr><td align=\"right\"><b>".
	_("Minimum Machine number") . ": </b></td>".
	"<td align=\"left\"><input size=6 type=\"text\" name=\"minMach\" value=\"" . $conf->get_minMachine() . "\">\n");
// maxMach
echo ("&nbsp <b>" . _("Maximum Machine number") . ": </b>".
	"<input size=6 type=\"text\" name=\"maxMach\" value=\"" . $conf->get_maxMachine() . "\"></td>\n");
// Machine text
echo ("<td><p align=\"left\">".
	_("These are the minimum and maximum numbers to use for Samba hosts. <br> Do not use the same range as for user IDs.") . "</p></td></tr>\n");

echo ("</table>");
echo ("</fieldset>");
echo ("<br>");

echo ("<fieldset><legend><b>" . _("LDAP List settings") . "</b></legend>");
echo ("<table align=\"center\" border=\"0\">");

// user list attributes
echo ("<tr><td><p align=\"right\"><b>".
	_("Attributes in User List:") . "</b></p></td>".
	"<td><input size=50 type=\"text\" name=\"usrlstattr\" value=\"" . $conf->get_userlistAttributes() . "\"></td>");
// list attributes description
echo ("<td rowspan=3><p>".
	_("This is the list of attributes to show in the lists. The entries can either be predefined values".
	" (e.g. '#cn' or '#gidNumber') or individual ones (e.g. 'cn:Group Name'). The entries are seperated by semicolons.").
	"</p></td></tr>");
// user list attributes
echo ("<tr><td><p align=\"right\"><b>".
	_("Attributes in Group List:") . "</b></p></td>".
	"<td><input size=50 type=\"text\" name=\"grplstattr\" value=\"" . $conf->get_grouplistAttributes() . "\"></td></tr>");
// user list attributes
echo ("<tr><td><p align=\"right\"><b>".
	_("Attributes in Host List:") . "</b></p></td>".
	"<td><input size=50 type=\"text\" name=\"hstlstattr\" value=\"" . $conf->get_hostlistAttributes() . "\"></td></tr>");
// two new lines
echo ("<tr><td>&nbsp</td></tr>");
echo ("<tr><td>&nbsp</td></tr>");
// admin list
echo ("<tr><td><p align=\"right\"><b>".
	_("List of valid users") . ": </b></p></td>".
	"<td><input size=50 type=\"text\" name=\"admins\" value=\"" . $conf->get_Adminstring() . "\"></td>\n");
// admin list description
echo ("<td><p align=\"left\">".
	_("Usernames must be seperated by semicolons")."<br>".
	_("(e.g. cn=admin,dc=yourcompany,dc=com;uid=root,ou=people,dc=yourcompany,dc=com)").
	"</p></td></tr>\n");
// maximum list entries
echo ("<tr><td><p align=\"right\"><b>".
	_("Maximum displayed entries") . ": </b></p></td>".
	"<td><select name=\"maxlistentries\"><option selected>".$conf->get_MaxListEntries()."</option>");
if ($conf->get_MaxListEntries() != 10) echo("<option>10</option>");
if ($conf->get_MaxListEntries() != 20) echo("<option>20</option>");
if ($conf->get_MaxListEntries() != 30) echo("<option>30</option>");
if ($conf->get_MaxListEntries() != 50) echo("<option>50</option>");
if ($conf->get_MaxListEntries() != 75) echo("<option>75</option>");
if ($conf->get_MaxListEntries() != 100) echo("<option>100</option>");
echo ("</select></td>\n");
// maximum list entries description
echo ("<td><p align=\"left\">".
	_("This is the maximum count of entries which are displayed on one page of the user/host/group lists.").
	"</p></td></tr>\n");

echo ("</table>\n");
echo ("</fieldset>\n");
echo ("<br>\n");

echo ("<fieldset><legend><b>" . _("Language settings") . "</b></legend>\n");
echo ("<table align=\"left\" border=\"0\">\n");

// language
echo ("<tr>");
echo ("<td><b>" . _("Default Language") . "</b></td>\n<td>");
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
echo ("</td></tr>\n");

echo ("</table>\n");
echo ("</fieldset>\n");
echo ("<br>\n");

echo ("<fieldset><legend><b>" . _("Script settings") . "</b></legend>\n");
echo ("<table align=\"left\" border=\"0\">\n");

// script settings
echo ("<tr><td><p align=\"right\"><b>".
	_("Path to external script") . ": </b></p></td>".
	"<td><input size=50 type=\"text\" name=\"scriptpath\" value=\"" . $conf->get_scriptPath() . "\"></td></tr>\n");
echo ("<tr><td><p align=\"right\"><b>".
	_("Server of external script") . ": </b></p></td>".
	"<td><input size=50 type=\"text\" name=\"scriptserver\" value=\"" . $conf->get_scriptServer() . "\"></td></tr>\n");

echo ("</table>\n");
echo ("</fieldset>\n");
echo ("<br>\n");


echo ("<fieldset><legend><b>" . _("Security settings") . "</b></legend>\n");
echo ("<table align=\"left\" border=\"0\">\n");
// new password
echo ("<tr><td bgcolor=\"red\" align=\"right\"><b>".
	_("New Password") . ": </b></td>".
	"<td bgcolor=\"red\" align=\"left\"><input type=\"password\" name=\"pass1\"></td></tr>\n");
// reenter password
echo ("<tr><td bgcolor=\"red\" align=\"right\"><b>".
	_("Reenter Password") . ": </b></td>".
	"<td bgcolor=\"red\" align=\"left\"><input type=\"password\" name=\"pass2\"></td></tr>\n");
echo ("</table>\n");
echo ("</fieldset>");
echo ("<br>");


// buttons
echo ("<table align=\"left\" border=\"0\">");

echo ("<tr><td align=\"left\"><pre>".
	"<input type=\"submit\" name=\"submitconf\" value=\"" . _("Submit") . "\">".
	"<input type=\"reset\" name=\"resetconf\" value=\"" . _("Reset") . "\">".
	"<input type=\"submit\" name=\"back\" value=\"" . _("Abort") . "\"\n");

echo ("></pre></td></tr>\n");

echo ("</table>\n");

// password for configuration
echo ("<input type=\"hidden\" name=\"passwd\" value=\"" . $passwd . "\"><br>\n");

echo ("</form>\n");
echo ("</body>\n");
echo ("</html>\n");

?>

