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
if ($_POST['back'] || $_POST['submitconf'] || $_POST['editmodules']){
	// save settings
	if ($_POST['submitconf'] || $_POST['editmodules']){
		// save HTTP-POST variables in session
		$_SESSION['conf_passwd'] = $_POST['passwd'];
		$_SESSION['conf_passwd1'] = $_POST['passwd1'];
		$_SESSION['conf_passwd2'] = $_POST['passwd2'];
		$_SESSION['conf_serverurl'] = $_POST['serverurl'];
		$_SESSION['conf_cachetimeout'] = $_POST['cachetimeout'];
		$_SESSION['conf_admins'] = $_POST['admins'];
		$_SESSION['conf_suffusers'] = $_POST['suffusers'];
		$_SESSION['conf_suffgroups'] = $_POST['suffgroups'];
		$_SESSION['conf_suffhosts'] = $_POST['suffhosts'];
		$_SESSION['conf_suffdomains'] = $_POST['suffdomains'];
		$_SESSION['conf_minUID'] = $_POST['minUID'];
		$_SESSION['conf_maxUID'] = $_POST['maxUID'];
		$_SESSION['conf_minGID'] = $_POST['minGID'];
		$_SESSION['conf_maxGID'] = $_POST['maxGID'];
		$_SESSION['conf_minMach'] = $_POST['minMach'];
		$_SESSION['conf_maxMach'] = $_POST['maxMach'];
		$_SESSION['conf_usrlstattr'] = $_POST['usrlstattr'];
		$_SESSION['conf_grplstattr'] = $_POST['grplstattr'];
		$_SESSION['conf_hstlstattr'] = $_POST['hstlstattr'];
		$_SESSION['conf_maxlistentries'] = $_POST['maxlistentries'];
		$_SESSION['conf_lang'] = $_POST['lang'];
		$_SESSION['conf_pwdhash'] = $_POST['pwdhash'];
		$_SESSION['conf_scriptpath'] = $_POST['scriptpath'];
		$_SESSION['conf_scriptserver'] = $_POST['scriptserver'];
		$_SESSION['conf_usermodules'] = explode(",", $_POST['usermodules']);
		$_SESSION['conf_groupmodules'] = explode(",", $_POST['groupmodules']);
		$_SESSION['conf_hostmodules'] = explode(",", $_POST['hostmodules']);
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
	// back to login
	else if ($_POST['back']){
		metaRefresh("../login.php");
	}
	exit;
}

// get password if register_globals is off
if ($_POST['passwd']) $passwd = $_POST['passwd'];
if ($_GET["modulesback"] == "true") $passwd = $_SESSION['conf_passwd'];

// check if password was entered
// if not: load login page
if (! $passwd) {
	$message = _("No password was entered!");
	/** go back to login if password is empty */
	require('conflogin.php');
	exit;
}

$filename = $_POST['filename'];
if ($_GET["modulesback"] == "true") $filename = $_SESSION['conf_filename'];
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
if ($_GET["modulesback"] == "true") {
	// load config values from session
	$conf->set_ServerURL($_SESSION['conf_serverurl']);
	$conf->set_cacheTimeout($_SESSION['conf_cachetimeout']);
	$conf->set_Adminstring($_SESSION['conf_admins']);
	$conf->set_UserSuffix($_SESSION['conf_suffusers']);
	$conf->set_GroupSuffix($_SESSION['conf_suffgroups']);
	$conf->set_HostSuffix($_SESSION['conf_suffhosts']);
	$conf->set_DomainSuffix($_SESSION['conf_suffdomains']);
	$conf->set_minUID($_SESSION['conf_minUID']);
	$conf->set_maxUID($_SESSION['conf_maxUID']);
	$conf->set_minGID($_SESSION['conf_minGID']);
	$conf->set_maxGID($_SESSION['conf_maxGID']);
	$conf->set_minMachine($_SESSION['conf_minMach']);
	$conf->set_maxMachine($_SESSION['conf_maxMach']);
	$conf->set_userlistAttributes($_SESSION['conf_usrlstattr']);
	$conf->set_grouplistAttributes($_SESSION['conf_grplstattr']);
	$conf->set_hostlistAttributes($_SESSION['conf_hstlstattr']);
	$conf->set_MaxListEntries($_SESSION['conf_maxlistentries']);
	$conf->set_defaultLanguage($_SESSION['conf_lang']);
	$conf->set_scriptpath($_SESSION['conf_scriptpath']);
	$conf->set_scriptserver($_SESSION['conf_scriptserver']);
	$conf->set_pwdhash($_SESSION['conf_pwdhash']);
	// check if modules were edited
	if ($_GET["moduleschanged"] == "true") {
		$conf->set_UserModules($_SESSION['conf_usermodules']);
		$conf->set_GroupModules($_SESSION['conf_groupmodules']);
		$conf->set_HostModules($_SESSION['conf_hostmodules']);
	}
}

echo $_SESSION['header'];

echo ("<title>" . _("LDAP Account Manager Configuration") . "</title>\n");
echo ("<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n");
echo ("</head>\n");
echo ("<body>\n");
echo ("<p align=\"center\"><a href=\"http://lam.sf.net\" target=\"new_window\">".
	"<img src=\"../../graphics/banner.jpg\" border=1 alt=\"LDAP Account Manager\"></a></p>\n<hr>\n<p></p>\n");

// display formular
echo ("<form action=\"confmain.php\" method=\"post\">\n");

echo ("<fieldset><legend><b>" . _("Server settings") . "</b></legend>");
echo ("<table border=0>");
// serverURL
echo ("<tr><td align=\"right\"><b>" . _("Server address") . " *: </b></td>".
	"<td align=\"left\">".
	"<input size=50 type=\"text\" name=\"serverurl\" value=\"" . $conf->get_ServerURL() . "\">".
	"</td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=201\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");

// new line
echo ("<tr><td colspan=3>&nbsp</td></tr>");

// user suffix
echo ("<tr><td align=\"right\"><b>".
	_("UserSuffix") . " *: </b></td>".
	"<td><input size=50 type=\"text\" name=\"suffusers\" value=\"" . $conf->get_UserSuffix() . "\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=202\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
// group suffix
echo ("<tr><td align=\"right\"><b>".
	_("GroupSuffix") . " *: </b></td>".
	"<td><input size=50 type=\"text\" name=\"suffgroups\" value=\"" . $conf->get_GroupSuffix() . "\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=202\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
// host suffix
echo ("<tr><td align=\"right\"><b>".
	_("HostSuffix") . " **: </b></td>".
	"<td><input size=50 type=\"text\" name=\"suffhosts\" value=\"" . $conf->get_HostSuffix() . "\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=202\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
// domain suffix
echo ("<tr><td align=\"right\"><b>".
	_("DomainSuffix") . " ***: </b></td>".
	"<td><input size=50 type=\"text\" name=\"suffdomains\" value=\"" . $conf->get_DomainSuffix() . "\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=202\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");

// new line
echo ("<tr><td colspan=3>&nbsp</td></tr>");

// LDAP password hash type
echo ("<tr><td align=\"right\"><b>".
	_("Password hash type") . " : </b></td>".
	"<td><select name=\"pwdhash\">\n<option selected>" . $conf->get_pwdhash() . "</option>\n");
if ($conf->get_pwdhash() != "CRYPT") echo("<option>CRYPT</option>\n");
if ($conf->get_pwdhash() != "SHA") echo("<option>SHA</option>\n");
if ($conf->get_pwdhash() != "SSHA") echo("<option>SSHA</option>\n");
if ($conf->get_pwdhash() != "MD5") echo("<option>MD5</option>\n");
if ($conf->get_pwdhash() != "SMD5") echo("<option>SMD5</option>\n");
if ($conf->get_pwdhash() != "PLAIN") echo("<option>PLAIN</option>\n");
echo ("</select></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=215\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");

// new line
echo ("<tr><td colspan=3>&nbsp</td></tr>");

// LDAP cache timeout
echo ("<tr><td align=\"right\"><b>".
	_("Cache timeout") . ": </b></td>".
	"<td><select name=\"cachetimeout\">\n<option selected>".$conf->get_cacheTimeout()."</option>\n");
if ($conf->get_cacheTimeout() != 0) echo("<option>0</option>\n");
if ($conf->get_cacheTimeout() != 1) echo("<option>1</option>\n");
if ($conf->get_cacheTimeout() != 2) echo("<option>2</option>\n");
if ($conf->get_cacheTimeout() != 5) echo("<option>5</option>\n");
if ($conf->get_cacheTimeout() != 10) echo("<option>10</option>\n");
if ($conf->get_cacheTimeout() != 15) echo("<option>15</option>\n");
echo ("</select></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=214\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");

echo ("</table>");
echo ("</fieldset>");

echo ("<p></p>");

echo ("<fieldset><legend><b>" . _("Account modules") . "</b></legend>");
echo ("<table border=0>");

// Account modules
echo "<tr><td><b>" . _("User modules") . ": </b>" . implode(", ", $conf->get_UserModules()) . "</td></tr>\n";
echo "<tr><td><b>" . _("Group modules") . ": </b>" . implode(", ", $conf->get_GroupModules()) . "</td></tr>\n";
echo "<tr><td><b>" . _("Host modules") . ": </b>" . implode(", ", $conf->get_HostModules()) . "</td></tr>\n";
echo "<tr><td>&nbsp;</td></tr>\n";
echo "<tr><td><input type=\"submit\" name=\"editmodules\" value=\"" . _("Edit modules") . "\">&nbsp;&nbsp;" .
	"<a href=\"../help.php?HelpNumber=217\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n";

echo ("</table>");
echo ("</fieldset>");

echo ("<p></p>");


// module settings

// get list of scopes of modules
$scopes = array();
$mods = $conf->get_UserModules();
for ($i = 0; $i < sizeof($mods); $i++) $scopes[$mods[$i]][] = 'user';
$mods = $conf->get_GroupModules();
for ($i = 0; $i < sizeof($mods); $i++) $scopes[$mods[$i]][] = 'group';
$mods = $conf->get_HostModules();
for ($i = 0; $i < sizeof($mods); $i++) $scopes[$mods[$i]][] = 'host';

// get module options
$options = getConfigOptions($scopes);
// get current setting
$old_options = $conf->get_moduleSettings();
// get module descriptions
$moduleDescriptions = getConfigDescriptions();

// save scopes
$_SESSION['config_scopes'] = $scopes;

// index for tab order (1 is LDAP suffix)
$tabindex = 2;

// display module boxes
$modules = array_keys($options);
for ($m = 0; $m < sizeof($modules); $m++) {
	// ignore empty values
	if (!is_array($options[$modules[$m]]) || (sizeof($options[$modules[$m]]) < 1)) continue;
	echo "<fieldset>\n";
		echo "<legend><b>" . $moduleDescriptions['legend'][$modules[$m]] . "</b></legend>\n";
		echo "<table>\n";
		for ($l = 0; $l < sizeof($options[$modules[$m]]); $l++) {  // option lines
			echo "<tr>\n";
			for ($o = 0; $o < sizeof($options[$modules[$m]][$l]); $o++) {  // line parts
				echo "<td";
				if (isset($options[$modules[$m]][$l][$o]['align'])) echo " align=\"" . $options[$modules[$m]][$l][$o]['align'] . "\"";
				if (isset($options[$modules[$m]][$l][$o]['colspan'])) echo " colspan=\"" . $options[$modules[$m]][$l][$o]['colspan'] . "\"";
				if (isset($options[$modules[$m]][$l][$o]['rowspan'])) echo " rowspan=\"" . $options[$modules[$m]][$l][$o]['rowspan'] . "\"";
				echo ">";
				print_option($options[$modules[$m]][$l][$o], $modules[$m], $old_options, $tabindex);
				echo "</td>\n";
			}
			echo "</tr>\n";
		}
		echo "</table>\n";
	echo "</fieldset>\n";
	echo "<br>";
}


echo ("<fieldset><legend><b>" . _("Ranges") . "</b></legend>");
echo ("<table border=0>");

// minUID
echo ("<tr><td align=\"right\"><b>".
	_("Minimum UID number") . " *: </b>".
	"<input size=6 type=\"text\" name=\"minUID\" value=\"" . $conf->get_minUID() . "\"></td>\n");
echo "<td>&nbsp;&nbsp;&nbsp;</td>\n";
// maxUID
echo ("<td align=\"right\"><b>" . _("Maximum UID number") . " *: </b>".
	"<input size=6 type=\"text\" name=\"maxUID\" value=\"" . $conf->get_maxUID() . "\"></td>\n");
// UID text
echo ("<td><a href=\"../help.php?HelpNumber=203\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
// minGID
echo ("<tr><td align=\"right\"><b>".
	_("Minimum GID number") . " *: </b>".
	"<input size=6 type=\"text\" name=\"minGID\" value=\"" . $conf->get_minGID() . "\"></td>\n");
echo "<td>&nbsp;&nbsp;&nbsp;</td>\n";
// maxGID
echo ("<td align=\"right\"><b>" . _("Maximum GID number")." *: </b>".
	"<input size=6 type=\"text\" name=\"maxGID\" value=\"" . $conf->get_maxGID() . "\"></td>\n");
// GID text
echo ("<td><a href=\"../help.php?HelpNumber=204\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
// minMach
echo ("<tr><td align=\"right\"><b>".
	_("Minimum Machine number") . " **: </b>".
	"<input size=6 type=\"text\" name=\"minMach\" value=\"" . $conf->get_minMachine() . "\"></td>\n");
echo "<td>&nbsp;&nbsp;&nbsp;</td>\n";
// maxMach
echo ("<td align=\"right\"><b>" . _("Maximum Machine number") . " **: </b>".
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
	_("Attributes in User List") . " *:</b></td>".
	"<td><input size=50 type=\"text\" name=\"usrlstattr\" value=\"" . $conf->get_userlistAttributes() . "\"></td>");
echo ("<td><a href=\"../help.php?HelpNumber=206\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
// group list attributes
echo ("<tr><td align=\"right\"><b>".
	_("Attributes in Group List") . " *:</b></td>".
	"<td><input size=50 type=\"text\" name=\"grplstattr\" value=\"" . $conf->get_grouplistAttributes() . "\"></td>");
echo ("<td><a href=\"../help.php?HelpNumber=206\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
// host list attributes
echo ("<tr><td align=\"right\"><b>".
	_("Attributes in Host List") . " **:</b></td>".
	"<td><input size=50 type=\"text\" name=\"hstlstattr\" value=\"" . $conf->get_hostlistAttributes() . "\"></td>");
echo ("<td><a href=\"../help.php?HelpNumber=206\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");

echo ("<tr><td colspan=3>&nbsp</td></tr>\n");

// maximum list entries
echo ("<tr><td align=\"right\"><b>".
	_("Maximum list entries") . " : </b></td>".
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
echo ("<select name=\"lang\">");
for ($i = 0; $i < sizeof($languages); $i++) {
	$entry = explode(":", $languages[$i]);
	if ($conf->get_defaultLanguage() != $languages[$i]) echo("<option value=\"" . $languages[$i] . "\">" . $entry[2] . "</option>\n");
	else echo("<option selected value=\"" . $languages[$i] . "\">" . $entry[2] . "</option>\n");
}
echo ("</select>\n");
}
else
{
	echo _("Unable to load available languages. Setting English as default language. For further instructions please contact the Admin of this site.");
}
echo ("</td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=209\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");

echo ("</table>\n");
echo ("</fieldset>\n");

echo ("<p></p>\n");

// script settings
echo ("<fieldset><legend><b>" . _("Script settings") . "</b></legend>\n");
echo ("<table border=0>\n");

echo ("<tr><td align=\"right\"><b>".
	_("Server of external script") . ": </b></td>".
	"<td><input size=50 type=\"text\" name=\"scriptserver\" value=\"" . $conf->get_scriptServer() . "\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=211\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
echo ("<tr><td align=\"right\"><b>".
	_("Path to external script") . ": </b></td>".
	"<td><input size=50 type=\"text\" name=\"scriptpath\" value=\"" . $conf->get_scriptPath() . "\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=210\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");

echo ("</table>\n");
echo ("</fieldset>\n");

echo ("<p></p>\n");

// security setings
echo ("<fieldset><legend><b>" . _("Security settings") . "</b></legend>\n");
echo ("<table border=0>\n");
// admin list
echo ("<tr><td align=\"right\"><b>".
	_("List of valid users") . " *: </b></td>".
	"<td><input size=50 type=\"text\" name=\"admins\" value=\"" . $conf->get_Adminstring() . "\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=207\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");

echo ("<tr><td colspan=3>&nbsp;</td></tr>\n");

// new password
echo ("<tr><td align=\"right\"><font color=\"red\"><b>".
	_("New Password") . ": </b></font></td>".
	"<td align=\"left\"><input type=\"password\" name=\"passwd1\"></td>\n");
echo ("<td rowspan=2><a href=\"../help.php?HelpNumber=212\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
// reenter password
echo ("<tr><td align=\"right\"><font color=\"red\"><b>".
	_("Reenter Password") . ": </b></font></td>".
	"<td align=\"left\"><input type=\"password\" name=\"passwd2\"></td></tr>\n");
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

echo ("<p>* = ". _("required") . "</p>");
echo ("<p>** = ". _("required for Samba accounts") . "</p>");
echo ("<p>*** = ". _("required for Samba 3 accounts") . "</p>");

// password for configuration
echo ("<p><input type=\"hidden\" name=\"passwd\" value=\"" . $passwd . "\"></p>\n");

// config file
echo ("<p><input type=\"hidden\" name=\"filename\" value=\"" . $filename . "\"></p>\n");

// modules
echo ("<p><input type=\"hidden\" name=\"usermodules\" value=\"" . implode(",", $conf->get_UserModules()) . "\"></p>\n");
echo ("<p><input type=\"hidden\" name=\"groupmodules\" value=\"" . implode(",", $conf->get_GroupModules()) . "\"></p>\n");
echo ("<p><input type=\"hidden\" name=\"hostmodules\" value=\"" . implode(",", $conf->get_HostModules()) . "\"></p>\n");

echo ("</form>\n");
echo ("</body>\n");
echo ("</html>\n");



/**
* prints out the row of a section table including the option name, values and help
*
* @param array $values an array formated as module option
* @param string $module_name the name of the module the options belong to
* @param array $old_options a hash array with the values from the loaded profile
* @param integer $tabindex current value for tabulator order
*/
function print_option($values, $modulename, $old_options, &$tabindex) {
	switch ($values['kind']) {
		// text value
		case 'text':
			echo $values['text'] . "\n";
			break;
		// help link
		case 'help':
			echo "<a href=../help.php?module=$modulename&amp;HelpNumber=" . $values['value'] . ">" . _('Help') . "</a>\n";
			break;
		// input field
		case 'input':
			if (($values['type'] == 'text') || ($values['type'] == 'checkbox')) {
				if ($values['type'] == 'text') {
					$output = "<input tabindex=\"$tabindex\" type=\"text\" name=\"" . $values['name'] . "\"";
					if ($values['size']) $output .= " size=\"" . $values['size'] . "\"";
					if ($values['maxlength']) $output .= " maxlength=\"" . $values['maxlength'] . "\"";
					if (isset($old_options[$values['name']])) $output .= " value=\"" . $old_options[$values['name']][0] . "\"";
					elseif ($values['value']) $output .= " value=\"" . $values['value'] . "\"";
					if ($values['disabled']) $output .= " disabled";
					$output .= ">\n";
					echo $output;
					$_SESSION['config_types'][$values['name']] = "text";
				}
				elseif ($values['type'] == 'checkbox') {
					$output = "<input tabindex=\"$tabindex\" type=\"checkbox\" name=\"" . $values['name'] . "\"";
					if ($values['size']) $output .= " size=\"" . $values['size'] . "\"";
					if ($values['maxlength']) $output .= " maxlength=\"" . $values['maxlength'] . "\"";
					if ($values['disabled']) $output .= " disabled";
					if (isset($old_options[$values['name']]) && ($old_options[$values['name']][0] == 'true')) $output .= " checked";
					elseif ($values['checked']) $output .= " checked";
					$output .= ">\n";
					echo $output;
					$_SESSION['config_types'][$values['name']] = "checkbox";
				}
				$tabindex++;
			}
			break;
		// select box
		case 'select':
			if (! is_numeric($values['size'])) $values['size'] = 1;// correct size if needed
			if ($values['multiple']) {
				echo "<select tabindex=\"$tabindex\" name=\"" . $values['name'] . "[]\" size=\"" . $values['size'] . "\" multiple>\n";
				$_SESSION['config_types'][$values['name']] = "multiselect";
			}
			else {
				echo "<select tabindex=\"$tabindex\" name=\"" . $values['name'] . "\" size=\"" . $values['size'] . "\">\n";
				$_SESSION['config_types'][$values['name']] = "select";
			}
			// option values
			for ($i = 0; $i < sizeof($values['options']); $i++) {
				// use values from old profile if given
				if (isset($old_options[$values['name']])) {
					if (in_array($values['options'][$i], $old_options[$values['name']])) {
						echo "<option selected>" . $values['options'][$i] . "</option>\n";
					}
					else {
						echo "<option>" . $values['options'][$i] . "</option>\n";
					}
				}
				// use default values if not in profile
				else {
					if (is_array($values['options_selected']) && in_array($values['options'][$i], $values['options_selected'])) {
						echo "<option selected>" . $values['options'][$i] . "</option>\n";
					}
					else {
						echo "<option>" . $values['options'][$i] . "</option>\n";
					}
				}
			}
			echo "</select>\n";
			$tabindex++;
			break;
		// subtable
		case 'table':
			echo "<table>\n";
			for ($l = 0; $l < sizeof($values['value']); $l++) {  // option lines
				echo "<tr>\n";
				for ($o = 0; $o < sizeof($values['value'][$l]); $o++) {  // line parts
					echo "<td>";
					print_option($values['value'][$l][$o], $values['value'], $old_options, $tabindex);
					echo "</td>\n";
				}
				echo "</tr>\n";
			}
			echo "</table>\n";
		break;
		// print error message for invalid types
		default:
			echo _("Unrecognized type") . ": " . $values['kind'] . "\n";
			break;
	}
}


?>

