<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003 - 2007  Roland Gruber

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
* @author Thomas Manninger
*/


/** Access to config functions */
include_once("../../lib/config.inc");

/** access to module settings */
include_once("../../lib/modules.inc");

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

// get password
if (isset($_POST['passwd'])) $passwd = $_POST['passwd'];
if (isset($_GET["modulesback"]) || isset($_GET["typesback"])) $passwd = $_SESSION['conf_config']->get_Passwd();

// check if password was entered
// if not: load login page
if (! $passwd) {
	$_SESSION['conf_message'] = _("No password was entered!");
	/** go back to login if password is empty */
	require('conflogin.php');
	exit;
}

if (!isset($_SESSION['conf_config']) && isset($_POST['filename'])) {
	$_SESSION['conf_config'] = new LAMConfig($_POST['filename']);
}
$conf = &$_SESSION['conf_config'];

// check if password is valid
// if not: load login page
if (!(($conf->get_Passwd()) == $passwd)) {
	$sessionKeys = array_keys($_SESSION);
	for ($i = 0; $i < sizeof($sessionKeys); $i++) {
		if (substr($sessionKeys[$i], 0, 5) == "conf_") unset($_SESSION[$sessionKeys[$i]]);
	}
	$_SESSION['conf_message'] = _("The password is invalid! Please try again.");
	/** go back to login if password is invalid */
	require('conflogin.php');
	exit;
}

// check if button was pressed and if we have to save the setting or go back to login
if (isset($_POST['back']) || isset($_POST['submitconf']) || isset($_POST['editmodules']) || isset($_POST['edittypes'])){
	// go to final page
	if ($_POST['submitconf']){
		saveSettings();
	}
	// go to modules page
	elseif ($_POST['editmodules']){
		metaRefresh("confmodules.php");
		exit;
	}
	// go to types page
	elseif ($_POST['edittypes']){
		metaRefresh("conftypes.php");
		exit;
	}
	// back to login
	else if ($_POST['back']){
		metaRefresh("../login.php");
		exit;
	}
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

// display error messages
if (isset($_SESSION['conf_errors'])) {
	for ($i = 0; $i < sizeof($_SESSION['conf_errors']); $i++) {
		call_user_func_array('StatusMessage', $_SESSION['conf_errors'][$i]);
	}
	echo "<br>";
}

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
$types = $conf->get_ActiveTypes();
for ($i = 0; $i < sizeof($types); $i++) {
	$moduleNames = $conf->get_AccountModules($types[$i]);
	for ($m = 0; $m < sizeof($moduleNames); $m++) $moduleNames[$m] = getModuleAlias($moduleNames[$m], $types[$i]);
	echo "<b>" . getTypeAlias($types[$i]) . ": </b>" . implode(", ", $moduleNames) . "<br>\n";
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


// display module boxes
$modules = array_keys($options);
$_SESSION['conf_types'] = array();
for ($i = 0; $i < sizeof($modules); $i++) {
	if (sizeof($options[$modules[$i]]) < 1) continue;
	echo "<fieldset>\n";
	echo "<legend><b>" . getModuleAlias($modules[$i], "none") . "</b></legend>\n";
	$configTypes = parseHtml($modules[$i], $options[$modules[$i]], $old_options, true, $tabindex, $tabindexLink, 'config');
	$_SESSION['conf_types'] = array_merge($configTypes, $_SESSION['conf_types']);
	echo "</fieldset>\n";
	echo "<br>";
}


echo ("<fieldset><legend><b>" . _("List settings") . "</b></legend>\n");
echo ("<table border=0>\n");

// maximum list entries
echo ("<tr><td align=\"right\"><b>".
	_("Maximum list entries") . ": </b></td>".
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
	_("Server list") . ": </b></td>".
	"<td><input tabindex=\"$tabindex\" size=50 type=\"text\" name=\"scriptservers\" value=\"" . $conf->get_scriptServers(false) . "\"></td>\n");
$tabindex++;
echo "<td>";
echo "<a href=\"../help.php?HelpNumber=218\" target=\"lamhelp\">";
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
echo "<tr><td align=\"right\"><b>". _("Rights for the home directory") . ": </b></td>\n";
$owr = "";
$oww = "";
$owe = "";
$grr = "";
$grw = "";
$gre = "";
$otr = "";
$otw = "";
$ote = "";
$chmod = $conf->get_scriptRights();
if (checkChmod("read","owner", $chmod)) $owr = 'checked';
if (checkChmod("write","owner", $chmod)) $oww = 'checked';
if (checkChmod("execute","owner", $chmod)) $owe = 'checked';
if (checkChmod("read","group", $chmod)) $grr = 'checked';
if (checkChmod("write","group", $chmod)) $grw = 'checked';
if (checkChmod("execute","group", $chmod)) $gre = 'checked';
if (checkChmod("read","other", $chmod)) $otr = 'checked';
if (checkChmod("write","other", $chmod)) $otw = 'checked';
if (checkChmod("execute","other", $chmod)) $ote = 'checked';

echo "<td align=\"center\">\n";
	echo "<table width=\"280\"><tr align=\"center\">\n";
	echo "<td width=\"70\"></td><th width=\"70\">" . _("Read") . "</th>\n";
	echo "<th width=\"70\">" . _("Write") . "</th>\n";
	echo "<th width=\"70\">"._("Execute")."</th></tr>\n";
	echo "<tr align=\"center\"><th align=\"left\">"._("Owner")."</th>\n";
	echo "<td><input type=\"checkbox\" name=\"chmod_owr\" " . $owr . "></td>\n";
	echo "<td><input type=\"checkbox\" name=\"chmod_oww\" " . $oww . "></td>\n";
	echo "<td><input type=\"checkbox\" name=\"chmod_owe\" " . $owe . "></td></tr>\n";
	echo "<tr align=\"center\"><th align=\"left\">"._("Group")."</th>\n";
	echo "<td><input type=\"checkbox\" name=\"chmod_grr\" " . $grr . "></td>\n";
	echo "<td><input type=\"checkbox\" name=\"chmod_grw\" " . $grw . "></td>\n";
	echo "<td><input type=\"checkbox\" name=\"chmod_gre\" " . $gre . "></td></tr>\n";
	echo "<tr align=\"center\"><th align=\"left\">"._("Other")."</th>\n";
	echo "<td><input type=\"checkbox\" name=\"chmod_otr\" " . $otr . "></td>\n";
	echo "<td><input type=\"checkbox\" name=\"chmod_otw\" " . $otw . "></td>\n";
	echo "<td><input type=\"checkbox\" name=\"chmod_ote\" " . $ote . "></td>\n";
	echo "</tr></table>";
	$tabindex++;
echo "<td>";
echo "<a href=\"../help.php?HelpNumber=219\" target=\"lamhelp\">";
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
$adminText = implode("\n", explode(";", $conf->get_Adminstring()));
echo ("<tr><td align=\"right\"><b>".
	_("List of valid users") . " *: </b></td>".
	"<td><textarea tabindex=\"$tabindex\" size=50 type=\"text\" name=\"admins\" cols=75 rows=5>" . $adminText . "</textarea></td>\n");
echo "<td>";
echo "<a href=\"../help.php?HelpNumber=207\" target=\"lamhelp\">";
echo "<img src=\"../../graphics/help.png\" alt=\"" . _('Help') . "\" title=\"" . _('Help') . "\">";
echo "</a>\n";
echo "</td></tr>\n";
$tabindex++;

echo ("<tr><td colspan=3>&nbsp;</td></tr>\n");

// new password
echo ("<tr><td align=\"right\"><font color=\"red\"><b>".
	_("New password") . ": </b></font></td>".
	"<td align=\"left\"><input tabindex=\"$tabindex\" type=\"password\" name=\"passwd1\"></td>\n");
$tabindex++;
echo "<td rowspan=2>";
echo "<a href=\"../help.php?HelpNumber=212\" target=\"lamhelp\">";
echo "<img src=\"../../graphics/help.png\" alt=\"" . _('Help') . "\" title=\"" . _('Help') . "\">";
echo "</a>\n";
echo "</td></tr>\n";
// reenter password
echo ("<tr><td align=\"right\"><font color=\"red\"><b>".
	_("Reenter password") . ": </b></font></td>".
	"<td align=\"left\"><input tabindex=\"$tabindex\" type=\"password\" name=\"passwd2\"></td></tr>\n");
$tabindex++;
echo ("</table>\n");
echo ("</fieldset>\n");
echo ("<p></p>\n");


// buttons
echo ("<table border=0>\n");

echo "<tr>";
echo "<td align=\"left\"><pre>";
	echo "<input tabindex=\"$tabindex\" type=\"submit\" name=\"submitconf\" value=\"" . _("Ok") . "\">";
	$tabindex++;
	echo "&nbsp;";
	echo "<input tabindex=\"$tabindex\" type=\"submit\" name=\"back\" value=\"" . _("Cancel") . "\"\n";
	$tabindex++;

echo ("></pre></td></tr>\n");

echo ("</table>\n");

echo ("<p></p>");

echo ("<p>* = ". _("required") . "</p>");
echo ("<p>** = ". _("required for Samba 3 accounts") . "</p>");

// password for configuration
echo ("<p><input type=\"hidden\" name=\"passwd\" value=\"" . $passwd . "\"></p>\n");

echo ("</form>\n");
echo ("</body>\n");
echo ("</html>\n");


/**
 * Saves the entered settings.
 *
 */
function saveSettings() {
	$conf = &$_SESSION['conf_config'];
	$types = $conf->get_ActiveTypes();

	// remove double slashes if magic quotes are on
	if (get_magic_quotes_gpc() == 1) {
		$postKeys = array_keys($_POST);
		for ($i = 0; $i < sizeof($postKeys); $i++) {
			if (is_string($_POST[$postKeys[$i]])) $_POST[$postKeys[$i]] = stripslashes($_POST[$postKeys[$i]]);
		}
	}
	// check new preferences
	$errors = array();
	if (!$conf->set_ServerURL($_POST['serverurl'])) {
		$errors[] = array("ERROR", _("Server address is invalid!"));
	}
	if (!$conf->set_cacheTimeout($_POST['cachetimeout'])) {
		$errors[] = array("ERROR", _("Cache timeout is invalid!"));
	}
	$adminText = $_POST['admins'];
	$adminText = explode("\n", $adminText);
	$adminTextNew = array();
	for ($i = 0; $i < sizeof($adminText); $i++) {
		if (trim($adminText[$i]) == "") continue;
		$adminTextNew[] = trim($adminText[$i]);
	}
	if (!$conf->set_Adminstring(implode(";", $adminTextNew))) {
		$errors[] = array("ERROR", _("List of admin users is empty or invalid!"));
	}
	if (!$conf->set_Suffix("tree", $_POST['sufftree'])) {
		$errors[] = array("ERROR", _("TreeSuffix is invalid!"));
	}
	if (!$conf->set_MaxListEntries($_POST['maxlistentries'])) {
		$errors[] = array("ERROR", _("Max list entries is invalid!"));
	}
	if (!$conf->set_defaultLanguage($_POST['lang'])) {
		$errors[] = array("ERROR", _("Language is not defined!"));
	}
	if (!$conf->set_scriptpath($_POST['scriptpath'])) {
		$errors[] = array("ERROR", _("Script path is invalid!"));
	}
	if (!$conf->set_scriptservers($_POST['scriptservers'])) {
		$errors[] = array("ERROR", _("Script server is invalid!"));
	}
	$chmodOwner = 0;
	$chmodGroup = 0;
	$chmodOther = 0;
	if ($_POST['chmod_owr'] == 'on') $chmodOwner += 4;
	if ($_POST['chmod_oww'] == 'on') $chmodOwner += 2;
	if ($_POST['chmod_owe'] == 'on') $chmodOwner += 1;
	if ($_POST['chmod_grr'] == 'on') $chmodGroup += 4;
	if ($_POST['chmod_grw'] == 'on') $chmodGroup += 2;
	if ($_POST['chmod_gre'] == 'on') $chmodGroup += 1;
	if ($_POST['chmod_otr'] == 'on') $chmodOther += 4;
	if ($_POST['chmod_otw'] == 'on') $chmodOther += 2;
	if ($_POST['chmod_ote'] == 'on') $chmodOther += 1;
	$chmod = $chmodOwner . $chmodGroup . $chmodOther;
	if (!$conf->set_scriptrights($chmod)) {
		$errors[] = array("ERROR", _("Script rights are invalid!"));
	}
	// check if password was changed
	if (isset($_POST['passwd1']) && ($_POST['passwd1'] != '')) {
		if ($_POST['passwd1'] != $_POST['passwd2']) {
			$errors[] = array("ERROR", _("Passwords are different!"));
		}
		else {
			// set new password
			$conf->set_Passwd($_POST['passwd1']);
		}
	}

	// check module options
	// create option array to check and save
	$options = array();
	$opt_keys = array_keys($_SESSION['conf_types']);
	for ($i = 0; $i < sizeof($opt_keys); $i++) {
		$element = $opt_keys[$i];
		// text fields
		if ($_SESSION['conf_types'][$element] == "text") {
			$options[$element] = array($_POST[$element]);
		}
		// checkboxes
		elseif ($_SESSION['conf_types'][$element] == "checkbox") {
			if ($_POST[$element] == "on") $options[$element] = array('true');
			else $options[$element] = array('false');
		}
		// dropdownbox
		elseif ($_SESSION['conf_types'][$element] == "select") {
			$options[$element] = array($_POST[$element]);
		}
		// multiselect
		elseif ($_SESSION['conf_types'][$element] == "multiselect") {
			$options[$element] = $_POST[$element];  // value is already an array
		}
	}

	// get list of scopes of modules
	$scopes = array();
	for ($m = 0; $m < sizeof($types); $m++) {
		$mods = $conf->get_AccountModules($types[$m]);
		for ($i = 0; $i < sizeof($mods); $i++) $scopes[$mods[$i]][] = $types[$m];
	}
	// check options
	$errors = array_merge($errors, checkConfigOptions($scopes, $options));
	// print error messages if any
	if (sizeof($errors) > 0) {
		$_SESSION['conf_errors'] = $errors;
		$conf->set_moduleSettings($options);
	}
	// save settings if no errors occured
	else {
		// page head
		echo $_SESSION['header'];
		echo "<title>" . _("LDAP Account Manager Configuration") . "</title>\n";
		echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
		echo "</head><body>\n";
		echo ("<p align=\"center\"><a href=\"http://lam.sourceforge.net\" target=\"new_window\">".
			"<img src=\"../../graphics/banner.jpg\" border=1 alt=\"LDAP Account Manager\"></a></p><hr><br><br>");
		$conf->set_moduleSettings($options);
		$conf->save();
		echo ("<br><br><br><br><br><a href=\"../login.php\" target=\"_top\">" . _("Back to Login") . "</a>");
		echo("</body></html>");
		// remove settings from session
		$sessionKeys = array_keys($_SESSION);
		for ($i = 0; $i < sizeof($sessionKeys); $i++) {
			if (substr($sessionKeys[$i], 0, 5) == "conf_") unset($_SESSION[$sessionKeys[$i]]);
		}
		exit();
	}
}

?>

