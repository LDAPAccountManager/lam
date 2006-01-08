<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003-04  Roland Gruber

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


/**
* Main page of configuration
*
* @package configuration
* @author Roland Gruber
*/


/** Access to config functions */
include_once ("../../lib/config.inc");

/** access to module settings */
include_once ("../../lib/modules.inc");

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

// check if button was pressed and if we have to save the setting or go back to login
if (isset($_POST['back']) || isset($_POST['submitconf']) || isset($_POST['editmodules']) || isset($_POST['edittypes'])){
	// save settings
	if ($_POST['submitconf'] || $_POST['editmodules'] || $_POST['edittypes']){
		// save HTTP-POST variables in session
		$_SESSION['conf_passwd'] = $_POST['passwd'];
		$_SESSION['conf_passwd1'] = $_POST['passwd1'];
		$_SESSION['conf_passwd2'] = $_POST['passwd2'];
		$_SESSION['conf_serverurl'] = $_POST['serverurl'];
		$_SESSION['conf_cachetimeout'] = $_POST['cachetimeout'];
		$_SESSION['conf_admins'] = $_POST['admins'];
		$_SESSION['conf_sufftree'] = $_POST['sufftree'];
		$_SESSION['conf_maxlistentries'] = $_POST['maxlistentries'];
		$_SESSION['conf_lang'] = $_POST['lang'];
		$_SESSION['conf_scriptpath'] = $_POST['scriptpath'];
		$_SESSION['conf_scriptserver'] = $_POST['scriptserver'];
		$_SESSION['conf_filename'] = $_POST['filename'];
		$modSettings = array_keys($_SESSION['config_types']);
		for ($i = 0; $i < sizeof($modSettings); $i++) $_SESSION['config_moduleSettings'][$modSettings[$i]] = $_POST[$modSettings[$i]];
	}
	// go to final page
	if ($_POST['submitconf']){
		metaRefresh("confsave.php");
	}
	// go to modules page
	elseif ($_POST['editmodules']){
		metaRefresh("confmodules.php");
	}
	// go to types page
	elseif ($_POST['edittypes']){
		metaRefresh("conftypes.php");
	}
	// back to login
	else if ($_POST['back']){
		metaRefresh("../login.php");
	}
	exit;
}

// get password if register_globals is off
if (isset($_POST['passwd'])) $passwd = $_POST['passwd'];
if (isset($_GET["modulesback"]) || isset($_GET["typesback"])) $passwd = $_SESSION['conf_passwd'];

// check if password was entered
// if not: load login page
if (! $passwd) {
	$message = _("No password was entered!");
	/** go back to login if password is empty */
	require('conflogin.php');
	exit;
}

$filename = $_POST['filename'];
if (isset($_GET["modulesback"]) || isset($_GET["typesback"])) $filename = $_SESSION['conf_filename'];
$conf = new Config($filename);

// check if password is valid
// if not: load login page
if (!(($conf->get_Passwd()) == $passwd)) {
	$message = _("The password is invalid! Please try again.");
	/** go back to login if password is invalid */
	require('conflogin.php');
	exit;
}

// check if user comes from modules page
if (isset($_GET["modulesback"])) {
	// load config values from session
	$conf->set_ServerURL($_SESSION['conf_serverurl']);
	$conf->set_cacheTimeout($_SESSION['conf_cachetimeout']);
	$conf->set_Adminstring($_SESSION['conf_admins']);
	$conf->set_Suffix('tree', $_SESSION['conf_sufftree']);
	$conf->set_MaxListEntries($_SESSION['conf_maxlistentries']);
	$conf->set_defaultLanguage($_SESSION['conf_lang']);
	$conf->set_scriptpath($_SESSION['conf_scriptpath']);
	$conf->set_scriptserver($_SESSION['conf_scriptserver']);
}

// check if user comes from types page
if (isset($_GET["typesback"])) {
	// check if a new account type was added
	if (isset($_GET["typeschanged"])) {
		metaRefresh("confmodules.php");
		exit;		
	}
}

// type information
if (!isset($_SESSION['conf_accountTypes'])) $_SESSION['conf_accountTypes'] = $conf->get_ActiveTypes();
if (!isset($_SESSION['conf_accountTypesOld'])) $_SESSION['conf_accountTypesOld'] = $conf->get_ActiveTypes();
if (!isset($_SESSION['conf_typeSettings'])) $_SESSION['conf_typeSettings'] = $conf->get_typeSettings();

// index for tab order
$tabindex = 1;
$tabindexLink = 1000;

echo $_SESSION['header'];

echo ("<title>" . _("LDAP Account Manager Configuration") . "</title>\n");
echo ("<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n");
echo ("</head>\n");
echo ("<body>\n");
echo ("<p align=\"center\"><a href=\"http://lam.sourceforge.net\" target=\"new_window\">".
	"<img src=\"../../graphics/banner.jpg\" border=1 alt=\"LDAP Account Manager\"></a></p>\n<hr>\n<p></p>\n");

// display formular
echo ("<form action=\"confmain.php\" method=\"post\">\n");

echo ("<fieldset><legend><b>" . _("Server settings") . "</b></legend>");
echo ("<table border=0>");
// serverURL
echo ("<tr><td align=\"right\"><b>" . _("Server address") . " *: </b></td>".
	"<td align=\"left\">".
	"<input tabindex=\"$tabindex\" size=50 type=\"text\" name=\"serverurl\" value=\"" . $conf->get_ServerURL() . "\">".
	"</td>\n");
echo "<td>";
echo "<a href=\"../help.php?HelpNumber=201\" target=\"lamhelp\">";
echo "<img src=\"../../graphics/help.png\" alt=\"" . _('Help') . "\" title=\"" . _('Help') . "\">";
echo "</a>\n";
echo "</td></tr>\n";
$tabindex++;

// new line
echo ("<tr><td colspan=3>&nbsp</td></tr>");

// tree suffix
echo ("<tr><td align=\"right\"><b>".
	_("Tree suffix") . ": </b></td>".
	"<td><input tabindex=\"$tabindex\" size=50 type=\"text\" name=\"sufftree\" value=\"" . $conf->get_Suffix('tree') . "\"></td>\n");
echo "<td>";
echo "<a href=\"../help.php?HelpNumber=203\" target=\"lamhelp\">";
echo "<img src=\"../../graphics/help.png\" alt=\"" . _('Help') . "\" title=\"" . _('Help') . "\">";
echo "</a>\n";
echo "</td></tr>\n";
$tabindex++;

// new line
echo ("<tr><td colspan=3>&nbsp</td></tr>");

// LDAP cache timeout
echo ("<tr><td align=\"right\"><b>".
	_("Cache timeout") . ": </b></td>".
	"<td><select tabindex=\"$tabindex\" name=\"cachetimeout\">\n<option selected>".$conf->get_cacheTimeout()."</option>\n");
if ($conf->get_cacheTimeout() != 0) echo("<option>0</option>\n");
if ($conf->get_cacheTimeout() != 1) echo("<option>1</option>\n");
if ($conf->get_cacheTimeout() != 2) echo("<option>2</option>\n");
if ($conf->get_cacheTimeout() != 5) echo("<option>5</option>\n");
if ($conf->get_cacheTimeout() != 10) echo("<option>10</option>\n");
if ($conf->get_cacheTimeout() != 15) echo("<option>15</option>\n");
echo ("</select></td>\n");
$tabindex++;
echo "<td>";
echo "<a href=\"../help.php?HelpNumber=214\" target=\"lamhelp\">";
echo "<img src=\"../../graphics/help.png\" alt=\"" . _('Help') . "\" title=\"" . _('Help') . "\">";
echo "</a>\n";
echo "</td></tr>\n";

echo ("</table>");
echo ("</fieldset>");

echo ("<p></p>");

echo ("<fieldset><legend><b>" . _("Account types and modules") . "</b></legend>");

// Account modules
$types = $_SESSION['conf_accountTypes'];
for ($i = 0; $i < sizeof($types); $i++) {
	echo "<b>" . getTypeAlias($types[$i]) . ": </b>" . implode(", ", $conf->get_AccountModules($types[$i])) . "<br>\n";
}
echo "<br>\n";
echo "<input tabindex=\"$tabindex\" type=\"submit\" name=\"edittypes\" value=\"" . _("Edit account types") . "\">&nbsp;&nbsp;";
$tabindex++;
echo "<input tabindex=\"$tabindex\" type=\"submit\" name=\"editmodules\" value=\"" . _("Edit modules") . "\">&nbsp;&nbsp;";
echo "<a href=\"../help.php?HelpNumber=217\" target=\"lamhelp\">";
echo "<img src=\"../../graphics/help.png\" alt=\"" . _('Help') . "\" title=\"" . _('Help') . "\">";
echo "</a>\n";
$tabindex++;

echo ("</fieldset>");

echo ("<p></p>");


// module settings

// get list of scopes of modules
$scopes = array();
for ($m = 0; $m < sizeof($types); $m++) {
	$mods = $conf->get_AccountModules($types[$m]);
	for ($i = 0; $i < sizeof($mods); $i++) $scopes[$mods[$i]][] = $types[$m];
}

// get module options
$options = getConfigOptions($scopes);
// get current setting
$old_options = $conf->get_moduleSettings();
// get module descriptions
$moduleDescriptions = getConfigDescriptions();

// save scopes
$_SESSION['config_scopes'] = $scopes;

// display module boxes
$modules = array_keys($options);
$_SESSION['config_types'] = array();
for ($i = 0; $i < sizeof($modules); $i++) {
	if (sizeof($options[$modules[$i]]) < 1) continue;
	echo "<fieldset>\n";
	echo "<legend><b>" . $moduleDescriptions['legend'][$modules[$i]] . "</b></legend>\n";
	$configTypes = parseHtml($modules[$i], $options[$modules[$i]], $old_options, true, $tabindex, $tabindexLink, 'config');
	$_SESSION['config_types'] = array_merge($configTypes, $_SESSION['config_types']);
	echo "</fieldset>\n";
	echo "<br>";
}


echo ("<fieldset><legend><b>" . _("List settings") . "</b></legend>\n");
echo ("<table border=0>\n");

// maximum list entries
echo ("<tr><td align=\"right\"><b>".
	_("Maximum list entries") . " : </b></td>".
	"<td><select tabindex=\"$tabindex\" name=\"maxlistentries\">\n<option selected>".$conf->get_MaxListEntries()."</option>\n");
if ($conf->get_MaxListEntries() != 10) echo("<option>10</option>\n");
if ($conf->get_MaxListEntries() != 20) echo("<option>20</option>\n");
if ($conf->get_MaxListEntries() != 30) echo("<option>30</option>\n");
if ($conf->get_MaxListEntries() != 50) echo("<option>50</option>\n");
if ($conf->get_MaxListEntries() != 75) echo("<option>75</option>\n");
if ($conf->get_MaxListEntries() != 100) echo("<option>100</option>\n");
echo ("</select></td>\n");
$tabindex++;
echo "<td>";
echo "<a href=\"../help.php?HelpNumber=208\" target=\"lamhelp\">";
echo "<img src=\"../../graphics/help.png\" alt=\"" . _('Help') . "\" title=\"" . _('Help') . "\">";
echo "</a>\n";
echo "</td></tr>\n";

echo ("</table>\n");
echo ("</fieldset>\n");
echo ("<p></p>\n");

echo ("<fieldset><legend><b>" . _("Language settings") . "</b></legend>\n");
echo ("<table border=0>\n");

// language
echo ("<tr>");
echo ("<td><b>" . _("Default language") . ":</b></td><td>\n");
// read available languages
$languagefile = "../../config/language";
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
echo ("<select tabindex=\"$tabindex\" name=\"lang\">");
for ($i = 0; $i < sizeof($languages); $i++) {
	$entry = explode(":", $languages[$i]);
	if ($conf->get_defaultLanguage() != $languages[$i]) echo("<option value=\"" . $languages[$i] . "\">" . $entry[2] . "</option>\n");
	else echo("<option selected value=\"" . $languages[$i] . "\">" . $entry[2] . "</option>\n");
}
echo ("</select>\n");
$tabindex++;
}
else
{
	echo _("Unable to load available languages. Setting English as default language. For further instructions please contact the Admin of this site.");
}
echo ("</td>\n");
echo "<td>";
echo "<a href=\"../help.php?HelpNumber=209\" target=\"lamhelp\">";
echo "<img src=\"../../graphics/help.png\" alt=\"" . _('Help') . "\" title=\"" . _('Help') . "\">";
echo "</a>\n";
echo "</td></tr>\n";

echo ("</table>\n");
echo ("</fieldset>\n");

echo ("<p></p>\n");

// script settings
echo ("<fieldset><legend><b>" . _("Script settings") . "</b></legend>\n");
echo ("<table border=0>\n");

echo ("<tr><td align=\"right\"><b>".
	_("Server of external script") . ": </b></td>".
	"<td><input tabindex=\"$tabindex\" size=50 type=\"text\" name=\"scriptserver\" value=\"" . $conf->get_scriptServer() . "\"></td>\n");
$tabindex++;
echo "<td>";
echo "<a href=\"../help.php?HelpNumber=211\" target=\"lamhelp\">";
echo "<img src=\"../../graphics/help.png\" alt=\"" . _('Help') . "\" title=\"" . _('Help') . "\">";
echo "</a>\n";
echo "</td></tr>\n";
echo ("<tr><td align=\"right\"><b>".
	_("Path to external script") . ": </b></td>".
	"<td><input tabindex=\"$tabindex\" size=50 type=\"text\" name=\"scriptpath\" value=\"" . $conf->get_scriptPath() . "\"></td>\n");
$tabindex++;
echo "<td>";
echo "<a href=\"../help.php?HelpNumber=210\" target=\"lamhelp\">";
echo "<img src=\"../../graphics/help.png\" alt=\"" . _('Help') . "\" title=\"" . _('Help') . "\">";
echo "</a>\n";
echo "</td></tr>\n";

echo ("</table>\n");
echo ("</fieldset>\n");

echo ("<p></p>\n");

// security setings
echo ("<fieldset><legend><b>" . _("Security settings") . "</b></legend>\n");
echo ("<table border=0>\n");
// admin list
echo ("<tr><td align=\"right\"><b>".
	_("List of valid users") . " *: </b></td>".
	"<td><input tabindex=\"$tabindex\" size=50 type=\"text\" name=\"admins\" value=\"" . $conf->get_Adminstring() . "\"></td>\n");
echo "<td>";
echo "<a href=\"../help.php?HelpNumber=207\" target=\"lamhelp\">";
echo "<img src=\"../../graphics/help.png\" alt=\"" . _('Help') . "\" title=\"" . _('Help') . "\">";
echo "</a>\n";
echo "</td></tr>\n";
$tabindex++;

echo ("<tr><td colspan=3>&nbsp;</td></tr>\n");

// new password
echo ("<tr><td align=\"right\"><font color=\"red\"><b>".
	_("New Password") . ": </b></font></td>".
	"<td align=\"left\"><input tabindex=\"$tabindex\" type=\"password\" name=\"passwd1\"></td>\n");
$tabindex++;
echo "<td rowspan=2>";
echo "<a href=\"../help.php?HelpNumber=212\" target=\"lamhelp\">";
echo "<img src=\"../../graphics/help.png\" alt=\"" . _('Help') . "\" title=\"" . _('Help') . "\">";
echo "</a>\n";
echo "</td></tr>\n";
// reenter password
echo ("<tr><td align=\"right\"><font color=\"red\"><b>".
	_("Reenter Password") . ": </b></font></td>".
	"<td align=\"left\"><input tabindex=\"$tabindex\" type=\"password\" name=\"passwd2\"></td></tr>\n");
$tabindex++;
echo ("</table>\n");
echo ("</fieldset>\n");
echo ("<p></p>\n");


// buttons
echo ("<table border=0>\n");

echo "<tr>";
echo "<td align=\"left\"><pre>";
	echo "<input tabindex=\"$tabindex\" type=\"submit\" name=\"submitconf\" value=\"" . _("Submit") . "\">";
	$tabindex++;
	echo "<input tabindex=\"$tabindex\" type=\"reset\" name=\"resetconf\" value=\"" . _("Reset") . "\">";
	$tabindex++;
	echo "<input tabindex=\"$tabindex\" type=\"submit\" name=\"back\" value=\"" . _("Abort") . "\"\n";
	$tabindex++;

echo ("></pre></td></tr>\n");

echo ("</table>\n");

echo ("<p></p>");

echo ("<p>* = ". _("required") . "</p>");
echo ("<p>** = ". _("required for Samba 3 accounts") . "</p>");

// password for configuration
echo ("<p><input type=\"hidden\" name=\"passwd\" value=\"" . $passwd . "\"></p>\n");

// config file
echo ("<p><input type=\"hidden\" name=\"filename\" value=\"" . $filename . "\"></p>\n");

echo ("</form>\n");
echo ("</body>\n");
echo ("</html>\n");


?>

