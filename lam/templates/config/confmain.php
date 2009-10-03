<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003 - 2009  Roland Gruber

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
if (strtolower(session_module_name()) == 'files') {
	session_save_path("../../sess");
}
@session_start();

setlanguage();

// get password
if (isset($_POST['passwd'])) $passwd = $_POST['passwd'];

// check if password was entered
// if not: load login page
if (!isset($passwd) && !isset($_SESSION['conf_isAuthenticated'])) {
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
if ((!isset($_SESSION['conf_isAuthenticated']) || !($_SESSION['conf_isAuthenticated'] === $conf->getName())) && !$conf->check_Passwd($passwd)) {
	$sessionKeys = array_keys($_SESSION);
	for ($i = 0; $i < sizeof($sessionKeys); $i++) {
		if (substr($sessionKeys[$i], 0, 5) == "conf_") unset($_SESSION[$sessionKeys[$i]]);
	}
	$_SESSION['conf_message'] = _("The password is invalid! Please try again.");
	/** go back to login if password is invalid */
	require('conflogin.php');
	exit;
}
$_SESSION['conf_isAuthenticated'] = $conf->getName();

// check if user canceled editing
if (isset($_POST['cancelSettings'])) {
	metaRefresh("../login.php");
	exit;
}

$errorsToDisplay = array();

// check if button was pressed and if we have to save the settings or go to another tab
if (isset($_POST['saveSettings']) || isset($_POST['editmodules'])
	|| isset($_POST['edittypes']) || isset($_POST['generalSettingsButton'])
	|| isset($_POST['moduleSettings'])) {
	$errorsToDisplay = checkInput();
	if (sizeof($errorsToDisplay) == 0) {
		// go to final page
		if (isset($_POST['saveSettings'])) {
			metaRefresh("confsave.php");
			exit;
		}
		// go to modules page
		elseif (isset($_POST['editmodules'])) {
			metaRefresh("confmodules.php");
			exit;
		}
		// go to types page
		elseif (isset($_POST['edittypes'])) {
			metaRefresh("conftypes.php");
			exit;
		}
		// go to module settings page
		elseif (isset($_POST['moduleSettings'])) {
			metaRefresh("moduleSettings.php");
			exit;
		}
	}
}


// index for tab order
$tabindex = 1;

echo $_SESSION['header'];

echo ("<title>" . _("LDAP Account Manager Configuration") . "</title>\n");
echo ("<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n");
echo "<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"../../graphics/favicon.ico\">\n";
echo ("</head>\n");
echo ("<body onload=\"configLoginMethodChanged()\">\n");
echo "<script type=\"text/javascript\" src=\"../wz_tooltip.js\"></script>\n";
echo "<script type=\"text/javascript\" src=\"config.js\"></script>\n";
echo ("<p align=\"center\"><a href=\"http://www.ldap-account-manager.org/\" target=\"new_window\">".
	"<img src=\"../../graphics/banner.jpg\" border=1 alt=\"LDAP Account Manager\"></a></p>\n<hr>\n<p>&nbsp;</p>\n");

if (!$conf->isWritable()) {
	StatusMessage('WARN', 'The config file is not writable.', 'Your changes cannot be saved until you make the file writable for the webserver user.');
	echo "<br>";
}

// display error messages
if (sizeof($errorsToDisplay) > 0) {
	for ($i = 0; $i < sizeof($errorsToDisplay); $i++) {
		call_user_func_array('StatusMessage', $errorsToDisplay[$i]);
	}
	echo "<br>";
}

// display formular
echo ("<form action=\"confmain.php\" method=\"post\">\n");
echo "<table border=0 width=\"100%\" style=\"border-collapse: collapse;\">\n";
echo "<tr valign=\"top\"><td style=\"border-bottom: 1px solid;padding:0px;\" colspan=2>";
// show tabs
echo "<table width=\"100%\" border=0 style=\"border-collapse: collapse;\">";
echo "<tr>\n";
	$buttonSpace = '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;';
	// general settings
	echo "<td style=\"padding-bottom:0px;padding-right:5px;padding-left:5px;padding-top:10px;\">\n";
	echo "<table class=\"settingsTab\" width=\"100%\">\n";
	echo "<tr><td class=\"settingsActiveTab\" onclick=\"document.getElementsByName('generalSettingsButton')[0].click();\"";
	echo " align=\"center\">\n";
	$buttonStyle = 'background-image: url(../../graphics/bigTools.png);';
	echo "<input style=\"" . $buttonStyle . "\" name=\"generalSettingsButton\" type=\"submit\" value=\"" . $buttonSpace . _('General settings') . "\"";
	echo ">\n";
	echo "</td></tr></table>\n";
	echo '</td>';
	// account types
	echo "<td style=\"padding-bottom:0px;padding-right:5px;padding-left:5px;padding-top:10px;\">\n";
	echo "<table class=\"settingsTab\" width=\"100%\">\n";
	echo "<tr><td onclick=\"document.getElementsByName('edittypes')[0].click();\"";
	echo " align=\"center\">\n";
	$buttonStyle = 'background-image: url(../../graphics/gear.png);';
	echo "<input style=\"" . $buttonStyle . "\" name=\"edittypes\" type=\"submit\" value=\"" . $buttonSpace . _('Account types') . "\"";
	echo ">\n";
	echo "</td></tr></table>\n";
	echo '</td>';
	// module selection
	echo "<td style=\"padding-bottom:0px;padding-right:5px;padding-left:5px;padding-top:10px;\">\n";
	echo "<table class=\"settingsTab\" width=\"100%\">\n";
	echo "<tr><td onclick=\"document.getElementsByName('editmodules')[0].click();\"";
	echo " align=\"center\">\n";
	$buttonStyle = 'background-image: url(../../graphics/modules.png);';
	echo "<input style=\"" . $buttonStyle . "\" name=\"editmodules\" type=\"submit\" value=\"" . $buttonSpace . _('Modules') . "\"";
	echo ">\n";
	echo "</td></tr></table>\n";
	echo '</td>';
	// module settings
	echo "<td style=\"padding-bottom:0px;padding-right:5px;padding-left:5px;padding-top:10px;\">\n";
	echo "<table class=\"settingsTab\" width=\"100%\">\n";
	echo "<tr><td onclick=\"document.getElementsByName('moduleSettings')[0].click();\"";
	echo " align=\"center\">\n";
	$buttonStyle = 'background-image: url(../../graphics/moduleSettings.png);';
	echo "<input style=\"" . $buttonStyle . "\" name=\"moduleSettings\" type=\"submit\" value=\"" . $buttonSpace . _('Module settings') . "\"";
	echo ">\n";
	echo "</td></tr></table>\n";
	echo '</td>';
	echo "<td width=\"100%\">&nbsp;</td>";
	// spacer
	echo "<td width=\"100%\">&nbsp;</td>";
	// save button
	echo "<td style=\"padding-bottom:0px;padding-right:5px;padding-left:5px;padding-top:10px;\">\n";
	echo "<table class=\"settingsTab\" width=\"100%\">\n";
	echo "<tr><td onclick=\"document.getElementsByName('saveSettings')[0].click();\"";
	echo " align=\"center\">\n";
	$buttonStyle = 'background-image: url(../../graphics/pass.png);';
	echo "<input style=\"" . $buttonStyle . "\" name=\"saveSettings\" type=\"submit\" value=\"" . $buttonSpace . _('Save') . "\"";
	echo ">\n";
	echo "</td></tr></table>\n";
	echo '</td>';
	// cancel button
	echo "<td style=\"padding-bottom:0px;padding-right:5px;padding-left:5px;padding-top:10px;\">\n";
	echo "<table class=\"settingsTab\" width=\"100%\">\n";
	echo "<tr><td onclick=\"document.getElementsByName('cancelSettings')[0].click();\"";
	echo " align=\"center\">\n";
	$buttonStyle = 'background-image: url(../../graphics/fail.png);';
	echo "<input style=\"" . $buttonStyle . "\" name=\"cancelSettings\" type=\"submit\" value=\"" . $buttonSpace . _('Cancel') . "\"";
	echo ">\n";
	echo "</td></tr></table>\n";
	echo '</td>';
	echo "</tr></table>\n";		
// end tabs
echo "</td></tr>\n";

echo "<tr><td><br><br>\n";
echo ("<fieldset><legend><img align=\"middle\" src=\"../../graphics/profiles.png\" alt=\"profiles.png\"> <b>" . _("Server settings") . "</b></legend><br>\n");
echo ("<table border=0>");
// serverURL
echo ("<tr><td align=\"right\"><b>" . _("Server address") . " *: </b></td>".
	"<td align=\"left\">".
	"<input tabindex=\"$tabindex\" size=50 type=\"text\" name=\"serverurl\" value=\"" . $conf->get_ServerURL() . "\">".
	"</td>\n");
echo "<td>";
printHelpLink(getHelp('', '201'), '201');
echo "</td></tr>\n";
$tabindex++;
// use TLS
echo "<tr><td align=\"right\"><b>" . _("Activate TLS") . ": </b></td>\n";
echo "<td align=\"left\">\n";
echo "<select tabindex=\"$tabindex\" size=1 name=\"useTLS\">";
$useTLS = $conf->getUseTLS();
if (isset($useTLS) && ($useTLS == 'yes')) {
	echo "<option value=\"yes\" selected>" . _("yes") . "</option>";
	echo "<option value=\"no\">" . _("no") . "</option>";
}
else {
	echo "<option value=\"yes\">" . _("yes") . "</option>";
	echo "<option value=\"no\" selected>" . _("no") . "</option>";
}
echo "</select>\n";
echo "</td>\n";
echo "<td>";
printHelpLink(getHelp('', '201'), '201');
echo "</td></tr>\n";
$tabindex++;

// new line
echo ("<tr><td colspan=3>&nbsp;</td></tr>");

// tree suffix
echo ("<tr><td align=\"right\"><b>".
	_("Tree suffix") . ": </b></td>".
	"<td><input tabindex=\"$tabindex\" size=50 type=\"text\" name=\"sufftree\" value=\"" . $conf->get_Suffix('tree') . "\"></td>\n");
echo "<td>";
printHelpLink(getHelp('', '203'), '203');
echo "</td></tr>\n";
$tabindex++;

// new line
echo ("<tr><td colspan=3>&nbsp;</td></tr>");

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
printHelpLink(getHelp('', '214'), '214');
echo "</td></tr>\n";

// access level is only visible in Pro version
if (isLAMProVersion()) {
	// new line
	echo ("<tr><td colspan=3>&nbsp;</td></tr>");
	
	// access level
	echo ("<tr><td align=\"right\"><b>".
		_("Access level") . ": </b></td>".
		"<td><select tabindex=\"$tabindex\" name=\"accessLevel\">\n");
	if ($conf->getAccessLevel() == LAMConfig::ACCESS_ALL) {
		echo("<option selected value=" . LAMConfig::ACCESS_ALL . ">" . _('Write access') . "</option>\n");
	}
	else {
		echo("<option value=" . LAMConfig::ACCESS_ALL . ">" . _('Write access') . "</option>\n");
	}
	if ($conf->getAccessLevel() == LAMConfig::ACCESS_PASSWORD_CHANGE) {
		echo("<option selected value=" . LAMConfig::ACCESS_PASSWORD_CHANGE . ">" . _('Change passwords') . "</option>\n");
	}
	else {
		echo("<option value=" . LAMConfig::ACCESS_PASSWORD_CHANGE . ">" . _('Change passwords') . "</option>\n");
	}
	if ($conf->getAccessLevel() == LAMConfig::ACCESS_READ_ONLY) {
		echo("<option selected value=" . LAMConfig::ACCESS_READ_ONLY . ">" . _('Read only') . "</option>\n");
	}
	else {
		echo("<option value=" . LAMConfig::ACCESS_READ_ONLY . ">" . _('Read only') . "</option>\n");
	}
	echo ("</select></td>\n");
	$tabindex++;
	echo "<td>";
	printHelpLink(getHelp('', '215'), '215');
	echo "</td></tr>\n";
}

echo ("</table>");
echo ("</fieldset>");

echo ("<br>");

echo ("<fieldset><legend><img align=\"middle\" src=\"../../graphics/language.png\" alt=\"language.png\"> <b>" . _("Language settings") . "</b></legend><br>\n");
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
printHelpLink(getHelp('', '209'), '209');
echo "</td></tr>\n";

echo ("</table>\n");
echo ("</fieldset>\n");

echo ("<br>\n");

// script settings
echo ("<fieldset><legend><img align=\"middle\" src=\"../../graphics/lamdaemon.png\" alt=\"lamdaemon.png\"> <b>" . _("Script settings") . "</b></legend><br>\n");
echo ("<table border=0>\n");

echo ("<tr><td align=\"right\"><b>".
	_("Server list") . ": </b></td>".
	"<td><input tabindex=\"$tabindex\" size=50 type=\"text\" name=\"scriptservers\" value=\"" . $conf->get_scriptServers(false) . "\"></td>\n");
$tabindex++;
echo "<td>";
printHelpLink(getHelp('', '218'), '218');
echo "</td></tr>\n";
echo ("<tr><td align=\"right\"><b>".
	_("Path to external script") . ": </b></td>".
	"<td><input tabindex=\"$tabindex\" size=50 type=\"text\" name=\"scriptpath\" value=\"" . $conf->get_scriptPath() . "\"></td>\n");
$tabindex++;
echo "<td>";
printHelpLink(getHelp('', '210'), '210');
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
printHelpLink(getHelp('', '219'), '219');
echo "</td></tr>\n";

echo ("</table>\n");
echo ("</fieldset>\n");

echo ("<br>\n");

// security setings
echo ("<fieldset><legend><img align=\"middle\" src=\"../../graphics/security.png\" alt=\"security.png\"> <b>" . _("Security settings") . "</b></legend><br>\n");
echo ("<table border=0>\n");
// login method
echo ("<tr><td align=\"right\"><b>".
	_("Login method") . ": </b></td>".
	"<td><select tabindex=\"$tabindex\" name=\"loginMethod\" onchange=\"configLoginMethodChanged()\">\n");
if ($conf->getLoginMethod() == LAMConfig::LOGIN_LIST) {
	echo("<option selected value=" . LAMConfig::LOGIN_LIST . ">" . _('Fixed list') . "</option>\n");
}
else {
	echo("<option value=" . LAMConfig::LOGIN_LIST . ">" . _('Fixed list') . "</option>\n");
}
if ($conf->getLoginMethod() == LAMConfig::LOGIN_SEARCH) {
	echo("<option selected value=" . LAMConfig::LOGIN_SEARCH . ">" . _('LDAP search') . "</option>\n");
}
else {
	echo("<option value=" . LAMConfig::LOGIN_SEARCH . ">" . _('LDAP search') . "</option>\n");
}
echo ("</select></td>\n");
$tabindex++;
echo "<td>";
printHelpLink(getHelp('', '220'), '220');
echo "</td></tr>\n";
// admin list
$adminText = implode("\n", explode(";", $conf->get_Adminstring()));
echo "<tr id=\"trAdminList\"><td align=\"right\">\n";
echo "<b>".
	_("List of valid users") . " *: </b></td>".
	"<td><textarea tabindex=\"$tabindex\" name=\"admins\" cols=75 rows=3>" . $adminText . "</textarea></td>\n";
echo "<td>";
printHelpLink(getHelp('', '207'), '207');
echo "</td></tr>\n";
$tabindex++;
// login search suffix
echo "<tr id=\"trLoginSearchSuffix\"><td align=\"right\">\n";
echo "<b>".
	_("LDAP suffix") . " *: </b></td>".
	"<td><input type=\"text\" tabindex=\"$tabindex\" name=\"loginSearchSuffix\" value=\"" . $conf->getLoginSearchSuffix() . "\"  size=50></td>\n";
echo "<td>";
printHelpLink(getHelp('', '221'), '221');
echo "</td></tr>\n";
$tabindex++;
// login search filter
echo "<tr id=\"trLoginSearchFilter\"><td align=\"right\">\n";
echo "<b>".
	_("LDAP filter") . " *: </b></td>".
	"<td><input type=\"text\" tabindex=\"$tabindex\" name=\"loginSearchFilter\" value=\"" . $conf->getLoginSearchFilter() . "\"  size=50></td>\n";
echo "<td>";
printHelpLink(getHelp('', '221'), '221');
echo "</td></tr>\n";
$tabindex++;

echo ("<tr><td colspan=3>&nbsp;</td></tr>\n");

// new password
echo ("<tr><td align=\"right\"><font color=\"red\"><b>".
	_("New password") . ": </b></font></td>".
	"<td align=\"left\"><input tabindex=\"$tabindex\" type=\"password\" name=\"passwd1\"></td>\n");
$tabindex++;
echo "<td rowspan=2>";
printHelpLink(getHelp('', '212'), '212');
echo "</td></tr>\n";
// reenter password
echo ("<tr><td align=\"right\"><font color=\"red\"><b>".
	_("Reenter password") . ": </b></font></td>".
	"<td align=\"left\"><input tabindex=\"$tabindex\" type=\"password\" name=\"passwd2\"></td></tr>\n");
$tabindex++;
echo ("</table>\n");
echo ("</fieldset>\n");

echo ("<p>* = ". _("required") . "</p>");
echo ("<p>&nbsp;</p>\n");

echo '</td></tr></table>';
echo ("</form>\n");
echo ("</body>\n");
echo ("</html>\n");


/**
 * Checks user input and saves the entered settings.
 *
 * @return array list of errors
 */
function checkInput() {
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
	$conf->setUseTLS($_POST['useTLS']);
	if (!$conf->set_cacheTimeout($_POST['cachetimeout'])) {
		$errors[] = array("ERROR", _("Cache timeout is invalid!"));
	}
	if (isLAMProVersion()) {
		$conf->setAccessLevel($_POST['accessLevel']);
	}
	$adminText = $_POST['admins'];
	$adminText = explode("\n", $adminText);
	$adminTextNew = array();
	for ($i = 0; $i < sizeof($adminText); $i++) {
		if (trim($adminText[$i]) == "") continue;
		$adminTextNew[] = trim($adminText[$i]);
	}
	$conf->setLoginMethod($_POST['loginMethod']);
	$conf->setLoginSearchFilter($_POST['loginSearchFilter']);
	$conf->setLoginSearchSuffix($_POST['loginSearchSuffix']);
	if (!$conf->set_Adminstring(implode(";", $adminTextNew))) {
		$errors[] = array("ERROR", _("List of admin users is empty or invalid!"));
	}
	if (!$conf->set_Suffix("tree", $_POST['sufftree'])) {
		$errors[] = array("ERROR", _("TreeSuffix is invalid!"));
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
	if (isset($_POST['chmod_owr']) && ($_POST['chmod_owr'] == 'on')) $chmodOwner += 4;
	if (isset($_POST['chmod_oww']) && ($_POST['chmod_oww'] == 'on')) $chmodOwner += 2;
	if (isset($_POST['chmod_owe']) && ($_POST['chmod_owe'] == 'on')) $chmodOwner += 1;
	if (isset($_POST['chmod_grr']) && ($_POST['chmod_grr'] == 'on')) $chmodGroup += 4;
	if (isset($_POST['chmod_grw']) && ($_POST['chmod_grw'] == 'on')) $chmodGroup += 2;
	if (isset($_POST['chmod_gre']) && ($_POST['chmod_gre'] == 'on')) $chmodGroup += 1;
	if (isset($_POST['chmod_otr']) && ($_POST['chmod_otr'] == 'on')) $chmodOther += 4;
	if (isset($_POST['chmod_otw']) && ($_POST['chmod_otw'] == 'on')) $chmodOther += 2;
	if (isset($_POST['chmod_ote']) && ($_POST['chmod_ote'] == 'on')) $chmodOther += 1;
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

	return $errors;
}

?>

