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
include_once("../../lib/config.inc");
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
				echo "<td>";
				print_option($options[$modules[$m]][$l][$o], $modules[$m], $old_options);
				echo "</td>\n";
			}
			echo "</tr>\n";
		}
		echo "</table>\n";
	echo "</fieldset>\n";
}

// profile name and submit/abort buttons
echo "<p>&nbsp;</p>";
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

// prints out the row of a table including the option name, values and help
// $values: An array formated as module option
// $module_name: The name of the module the options belong to
// $old_options: A hash array with the values from the loaded profile
function print_option($values, $modulename, $old_options) {
	switch ($values['kind']) {
		// text value
		case 'text':
			echo $values['text'] . "\n";
			break;
		// help link
		case 'help':
			echo "<a href=../help.php?module=$modulename&amp;module=" . $values['value'] . ">" . _('Help') . "</a>\n";
			break;
		// input field
		case 'input':
			if (($values['type'] == 'text') || ($values['type'] == 'checkbox')) {
				if ($values['type'] == 'text') {
					$output = "<input type=\"text\" name=\"" . $values['name'] . "\"";
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
					$output = "<input type=\"checkbox\" name=\"" . $values['name'] . "\"";
					if ($values['size']) $output .= " size=\"" . $values['size'] . "\"";
					if ($values['maxlength']) $output .= " maxlength=\"" . $values['maxlength'] . "\"";
					if ($values['disabled']) $output .= " disabled";
					if (isset($old_options[$values['name']]) && ($old_options[$values['name']][0] == true)) $output .= " checked";
					elseif ($values['checked']) $output .= " checked";
					$output .= ">\n";
					echo $output;
					$_SESSION['profile_types'][$values['name']] = "checkbox";
				}
			}
			break;
		// select box
		case 'select':
			if ($values['multiple']) {
				echo "<select name=\"" . $values['name'] . "[]\" size=\"" . $values['size'] . "\" multiple>\n";
				$_SESSION['profile_types'][$values['name']] = "multiselect";
			}
			else {
				echo "<select name=\"" . $values['name'] . "\" size=\"" . $values['size'] . "\">\n";
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
					if (in_array($values['options'][$i], $values['options_selected'])) {
						echo "<option selected>" . $values['options'][$i] . "</option>\n";
					}
					else {
						echo "<option>" . $values['options'][$i] . "</option>\n";
					}
				}
			}
			echo "</select>\n";
			break;
		// print error message for invalid types
		default:
			echo _("Unrecognized type") . ": " . $values['kind'] . "\n";
			break;
	}
}

?>
