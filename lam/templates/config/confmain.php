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
if (isset($_POST['back']) || isset($_POST['submitconf']) || isset($_POST['editmodules'])){
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
		$_SESSION['conf_sufftree'] = $_POST['sufftree'];
		$_SESSION['conf_usrlstattr'] = $_POST['usrlstattr'];
		$_SESSION['conf_grplstattr'] = $_POST['grplstattr'];
		$_SESSION['conf_hstlstattr'] = $_POST['hstlstattr'];
		$_SESSION['conf_maxlistentries'] = $_POST['maxlistentries'];
		$_SESSION['conf_lang'] = $_POST['lang'];
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
if (isset($_POST['passwd'])) $passwd = $_POST['passwd'];
if (isset($_GET["modulesback"])) $passwd = $_SESSION['conf_passwd'];

// check if password was entered
// if not: load login page
if (! $passwd) {
	$message = _("No password was entered!");
	/** go back to login if password is empty */
	require('conflogin.php');
	exit;
}

$filename = $_POST['filename'];
if (isset($_GET["modulesback"])) $filename = $_SESSION['conf_filename'];
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
	$conf->set_Suffix('user', $_SESSION['conf_suffusers']);
	$conf->set_Suffix('group', $_SESSION['conf_suffgroups']);
	$conf->set_Suffix('host', $_SESSION['conf_suffhosts']);
	$conf->set_Suffix('domain', $_SESSION['conf_suffdomains']);
	$conf->set_Suffix('tree', $_SESSION['conf_sufftree']);
	$conf->set_listAttributes($_SESSION['conf_usrlstattr'], 'user');
	$conf->set_listAttributes($_SESSION['conf_grplstattr'], 'group');
	$conf->set_listAttributes($_SESSION['conf_hstlstattr'], 'host');
	$conf->set_MaxListEntries($_SESSION['conf_maxlistentries']);
	$conf->set_defaultLanguage($_SESSION['conf_lang']);
	$conf->set_scriptpath($_SESSION['conf_scriptpath']);
	$conf->set_scriptserver($_SESSION['conf_scriptserver']);
	// check if modules were edited
	if ($_GET["moduleschanged"] == "true") {
		$conf->set_AccountModules($_SESSION['conf_usermodules'], 'user');
		$conf->set_AccountModules($_SESSION['conf_groupmodules'], 'group');
		$conf->set_AccountModules($_SESSION['conf_hostmodules'], 'host');
	}
}

// index for tab order
$tabindex = 1;

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
	"<input tabindex=\"$tabindex\" size=50 type=\"text\" name=\"serverurl\" value=\"" . $conf->get_ServerURL() . "\">".
	"</td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=201\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
$tabindex++;

// new line
echo ("<tr><td colspan=3>&nbsp</td></tr>");

// user suffix
echo ("<tr><td align=\"right\"><b>".
	_("UserSuffix") . ": </b></td>".
	"<td><input tabindex=\"$tabindex\" size=50 type=\"text\" name=\"suffusers\" value=\"" . $conf->get_Suffix('user') . "\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=202\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
$tabindex++;
// group suffix
echo ("<tr><td align=\"right\"><b>".
	_("GroupSuffix") . ": </b></td>".
	"<td><input tabindex=\"$tabindex\" size=50 type=\"text\" name=\"suffgroups\" value=\"" . $conf->get_Suffix('group') . "\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=202\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
$tabindex++;
// host suffix
echo ("<tr><td align=\"right\"><b>".
	_("HostSuffix") . ": </b></td>".
	"<td><input tabindex=\"$tabindex\" size=50 type=\"text\" name=\"suffhosts\" value=\"" . $conf->get_Suffix('host') . "\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=202\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
$tabindex++;
// domain suffix
echo ("<tr><td align=\"right\"><b>".
	_("DomainSuffix") . " **: </b></td>".
	"<td><input tabindex=\"$tabindex\" size=50 type=\"text\" name=\"suffdomains\" value=\"" . $conf->get_Suffix('domain') . "\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=202\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
$tabindex++;
// tree suffix
echo ("<tr><td align=\"right\"><b>".
	_("TreeSuffix") . ": </b></td>".
	"<td><input tabindex=\"$tabindex\" size=50 type=\"text\" name=\"sufftree\" value=\"" . $conf->get_Suffix('tree') . "\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=203\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
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
echo ("<td><a href=\"../help.php?HelpNumber=214\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");

echo ("</table>");
echo ("</fieldset>");

echo ("<p></p>");

echo ("<fieldset><legend><b>" . _("Account modules") . "</b></legend>");
echo ("<table border=0>");

// Account modules
echo "<tr><td><b>" . _("User modules") . ": </b>" . implode(", ", $conf->get_AccountModules('user')) . "</td></tr>\n";
echo "<tr><td><b>" . _("Group modules") . ": </b>" . implode(", ", $conf->get_AccountModules('group')) . "</td></tr>\n";
echo "<tr><td><b>" . _("Host modules") . ": </b>" . implode(", ", $conf->get_AccountModules('host')) . "</td></tr>\n";
echo "<tr><td>&nbsp;</td></tr>\n";
echo "<tr><td><input tabindex=\"$tabindex\" type=\"submit\" name=\"editmodules\" value=\"" . _("Edit modules") . "\">&nbsp;&nbsp;" .
	"<a href=\"../help.php?HelpNumber=217\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n";
$tabindex++;

echo ("</table>");
echo ("</fieldset>");

echo ("<p></p>");


// module settings

// get list of scopes of modules
$scopes = array();
$mods = $conf->get_AccountModules('user');
for ($i = 0; $i < sizeof($mods); $i++) $scopes[$mods[$i]][] = 'user';
$mods = $conf->get_AccountModules('group');
for ($i = 0; $i < sizeof($mods); $i++) $scopes[$mods[$i]][] = 'group';
$mods = $conf->get_AccountModules('host');
for ($i = 0; $i < sizeof($mods); $i++) $scopes[$mods[$i]][] = 'host';

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


echo ("<fieldset><legend><b>" . _("LDAP List settings") . "</b></legend>\n");
echo ("<table border=0>\n");

// user list attributes
echo ("<tr><td align=\"right\"><b>".
	_("Attributes in User List") . " *:</b></td>".
	"<td><input tabindex=\"$tabindex\" size=50 type=\"text\" name=\"usrlstattr\" value=\"" . $conf->get_listAttributes('user') . "\"></td>");
echo ("<td><a href=\"../help.php?HelpNumber=206\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
$tabindex++;
// group list attributes
echo ("<tr><td align=\"right\"><b>".
	_("Attributes in Group List") . " *:</b></td>".
	"<td><input tabindex=\"$tabindex\" size=50 type=\"text\" name=\"grplstattr\" value=\"" . $conf->get_listAttributes('group') . "\"></td>");
echo ("<td><a href=\"../help.php?HelpNumber=206\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
$tabindex++;
// host list attributes
echo ("<tr><td align=\"right\"><b>".
	_("Attributes in Host List") . " **:</b></td>".
	"<td><input tabindex=\"$tabindex\" size=50 type=\"text\" name=\"hstlstattr\" value=\"" . $conf->get_listAttributes('host') . "\"></td>");
echo ("<td><a href=\"../help.php?HelpNumber=206\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
$tabindex++;

echo ("<tr><td colspan=3>&nbsp</td></tr>\n");

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
echo ("<td><a href=\"../help.php?HelpNumber=209\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");

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
echo ("<td><a href=\"../help.php?HelpNumber=211\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
echo ("<tr><td align=\"right\"><b>".
	_("Path to external script") . ": </b></td>".
	"<td><input tabindex=\"$tabindex\" size=50 type=\"text\" name=\"scriptpath\" value=\"" . $conf->get_scriptPath() . "\"></td>\n");
$tabindex++;
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
	"<td><input tabindex=\"$tabindex\" size=50 type=\"text\" name=\"admins\" value=\"" . $conf->get_Adminstring() . "\"></td>\n");
echo ("<td><a href=\"../help.php?HelpNumber=207\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
$tabindex++;

echo ("<tr><td colspan=3>&nbsp;</td></tr>\n");

// new password
echo ("<tr><td align=\"right\"><font color=\"red\"><b>".
	_("New Password") . ": </b></font></td>".
	"<td align=\"left\"><input tabindex=\"$tabindex\" type=\"password\" name=\"passwd1\"></td>\n");
$tabindex++;
echo ("<td rowspan=2><a href=\"../help.php?HelpNumber=212\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
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

// modules
echo ("<p><input type=\"hidden\" name=\"usermodules\" value=\"" . implode(",", $conf->get_AccountModules('user')) . "\"></p>\n");
echo ("<p><input type=\"hidden\" name=\"groupmodules\" value=\"" . implode(",", $conf->get_AccountModules('group')) . "\"></p>\n");
echo ("<p><input type=\"hidden\" name=\"hostmodules\" value=\"" . implode(",", $conf->get_AccountModules('host')) . "\"></p>\n");

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
			echo "Unrecognized type" . ": " . $values['kind'] . "\n";
			break;
	}
}


?>

