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

/**
* Manages creating/changing of profiles.
*
* @package Profiles
* @author Roland Gruber
*/

/** helper functions for profiles */
include_once("../../lib/profiles.inc");
/** access to LDAP server */
include_once("../../lib/ldap.inc");
/** access to configuration options */
include_once("../../lib/config.inc");
/** access to account modules */
include_once("../../lib/modules.inc");

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

// check if user is logged in, if not go to login
if (!$_SESSION['ldap'] || !$_SESSION['ldap']->server()) {
	metaRefresh("../login.php");
	exit;
}

// empty list of attribute types
$_SESSION['profile_types'] = array();

// print header
echo $_SESSION['header'];
echo "<title></title>\n<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head><body><br>\n";

// check if account type is valid
$type = $_GET['type'];
if (!(($type == 'user') || ($type == 'group') || ($type == 'host'))) meta_refresh('profilemain.php');

// get module options
$options = getProfileOptions($type);

// load old profile if needed
$old_options = array();
if ($_GET['edit']) {
	if ($type == "user") $old_options = loadUserProfile($_GET['edit']);
	else if ($type == "group") $old_options = loadGroupProfile($_GET['edit']);
	else if ($type == "host") $old_options = loadHostProfile($_GET['edit']);
}

// display formular
echo ("<form action=\"profilecreate.php?type=$type\" method=\"post\">\n");

// suffix box
// get root suffix
$rootsuffix = call_user_func(array($_SESSION['config'], 'get_' . ucfirst($type) . 'Suffix'));
// get subsuffixes
$suffixes = array();
foreach ($_SESSION['ldap']->search_units($rootsuffix) as $suffix) {
	$suffixes[] = $suffix;
}
if (sizeof($suffixes) > 0) {
echo "<fieldset>\n<legend><b>" . _("LDAP suffix") . "</b></legend>\n";
	echo _("LDAP suffix") . ":&nbsp;&nbsp;";
	echo "<select tabindex=\"1\">";
	for ($i = 0; $i < sizeof($suffixes); $i++) echo "<option>" . $suffixes[$i] . "</option>\n";
	echo "</select>\n";
	echo "&nbsp;&nbsp;<a href=../help.php?HelpNumber=TODO>" . _('Help') . "</a>\n";
echo "</fieldset>\n<br>\n";
}

// index for tab order (1 is LDAP suffix)
$tabindex = 2;

// display module options
$modules = array_keys($options);
for ($m = 0; $m < sizeof($modules); $m++) {
	// ignore modules without options
	if (sizeof($options[$modules[$m]]) < 1) continue;
	echo "<fieldset>\n";
		echo "<legend><b>" . getModuleAlias($modules[$m], $type) . "</b></legend>\n";
		echo "<table>\n";
		for ($l = 0; $l < sizeof($options[$modules[$m]]); $l++) {  // option lines
			echo "<tr>\n";
			for ($o = 0; $o < sizeof($options[$modules[$m]][$l]); $o++) {  // line parts
				echo "<td";
				if (isset($options[$modules[$m]][$l][$o]['align'])) echo " align=\"" . $options[$modules[$m]][$l][$o]['align'] . "\"";
				if (isset($options[$modules[$m]][$l][$o]['colspan'])) echo " colspan=\"" . $options[$modules[$m]][$l][$o]['colspan'] . "\"";
				if (isset($options[$modules[$m]][$l][$o]['rowspan'])) echo " rowspan=\"" . $options[$modules[$m]][$l][$o]['rowspan'] . "\"";
				echo ">";
				print_option($options[$modules[$m]][$l][$o], $modules[$m], $old_options);
				echo "</td>\n";
			}
			echo "</tr>\n";
		}
		echo "</table>\n";
	echo "</fieldset>\n";
	echo "<br>";
}

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
echo "<input type=\"hidden\" name=\"accounttype\" value=\"$type\">\n";

echo ("</form></body></html>\n");

/**
* prints out the row of a section table including the option name, values and help
*
* @param array $values an array formated as module option
* @param string $module_name the name of the module the options belong to
* @param array $old_options a hash array with the values from the loaded profile
*/
function print_option($values, $modulename, $old_options) {
	global $tabindex;
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
					$_SESSION['profile_types'][$values['name']] = "text";
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
					$_SESSION['profile_types'][$values['name']] = "checkbox";
				}
				$tabindex++;
			}
			break;
		// select box
		case 'select':
			if (! is_numeric($values['size'])) $values['size'] = 1;// correct size if needed
			if ($values['multiple']) {
				echo "<select tabindex=\"$tabindex\" name=\"" . $values['name'] . "[]\" size=\"" . $values['size'] . "\" multiple>\n";
				$_SESSION['profile_types'][$values['name']] = "multiselect";
			}
			else {
				echo "<select tabindex=\"$tabindex\" name=\"" . $values['name'] . "\" size=\"" . $values['size'] . "\">\n";
				$_SESSION['profile_types'][$values['name']] = "select";
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
					print_option($values['value'][$l][$o], $values['value'], $old_options);
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
