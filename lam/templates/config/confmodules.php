<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2004  Roland Gruber

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
* confmodules lets the user select the account modules
*
* @package configuration
* @author Roland Gruber
*/


/** Access to config functions */
include_once ('../../lib/config.inc');
/** Access to module lists */
include_once ('../../lib/modules.inc');

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

$conf = new Config($_SESSION['conf_filename']);

$passwd = $_SESSION['conf_passwd'];
// check if password is correct
// if not: load login page
if ($passwd != $conf->get_Passwd()) {
	/** go back to login if password is invalid */
	require('conflogin.php');
	exit;
}

// user pressed submit/abort button
if ($_POST['submit']) {
	//selection ok, back to other settings
	metarefresh('confmain.php?modulesback=true&amp;moduleschanged=true');
}
elseif ($_POST['abort']) {
	// no changes
	metarefresh('confmain.php?modulesback=true');
}

echo $_SESSION['header'];

echo "<title>" . _("LDAP Account Manager Configuration") . "</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head><body>\n";

echo ("<p align=\"center\"><a href=\"http://lam.sf.net\" target=\"new_window\">".
	"<img src=\"../../graphics/banner.jpg\" border=1 alt=\"LDAP Account Manager\"></a></p><hr><br>\n");

echo ("<form action=\"confmodules.php\" method=\"post\">\n");
echo "<p align=\"center\"><big><b>" . _("Module selection") . "</b></big><br><br></p>";


$account_list = array(
	array('user', _('User modules')),
	array('group', _('Group modules')),
	array('host', _('Host modules'))
);

$allDependenciesOk  = true;

for ($i = 0; $i < sizeof($account_list); $i++) {
	$ret = config_showAccountModules($account_list[$i][0], $account_list[$i][1]);
	if (!$ret) {
		$allDependenciesOk = false;
	}
}


// submit buttons
echo "<p>\n";
	// disable button if there are conflicts/depends
	if ($allDependenciesOk) {
		echo "<input type=\"submit\" value=\"" . _("Submit") . "\" name=\"submit\">\n";
	}
	else {
		echo "<input type=\"submit\" value=\"" . _("Submit") . "\" name=\"submit\" disabled>\n";
	}
	echo "&nbsp;";
	echo "<input type=\"submit\" value=\"" . _("Abort") . "\" name=\"abort\">\n";
echo "</p>\n";

echo "<p><br><br>\n";
echo "(*) <a href=\"../help.php?HelpNumber=237\">" . _("Base module") . "</a>";
echo "</p>\n";

echo "</form>\n";
echo "</body>\n";
echo "</html>\n";


/**
* Displays the module selection boxes and checks if dependencies are fulfilled.
*
* @param string $scope account type
* @param string $title title for module selection (e.g. "User modules")
* @return boolean true if all dependencies are ok
*/
function config_showAccountModules($scope, $title) {
	// account modules
	$selected_temp = $_SESSION['conf_' . $scope . 'modules'];
	$available = array();
	$available = getAvailableModules($scope);
	$selected = array();
	// only use available modules as selected
	for ($i = 0; $i < sizeof($selected_temp); $i++) {
		if (in_array($selected_temp[$i], $available)) $selected[] = $selected_temp[$i];
	}
	$no_conflicts = true;
	$no_depends = true;
	$no_missing_basemodule = true;
	
	// remove modules from selection
	if ($_POST[$scope . '_selected'] && ($_POST[$scope . '_remove'])) {
		$new_selected = array();
		for ($i = 0; $i < sizeof($selected); $i++) {
			if (! in_array($selected[$i], $_POST[$scope . '_selected'])) $new_selected[] = $selected[$i];
		}
		$selected = $new_selected;
		$_SESSION['conf_' . $scope . 'modules'] = $selected;
	}
	
	// add modules to selection
	elseif ($_POST[$scope . '_available'] && ($_POST[$scope . '_add'])) {
		$new_selected = $selected;
		for ($i = 0; $i < sizeof($_POST[$scope . '_available']); $i++) {
			if (! in_array($_POST[$scope . '_available'][$i], $selected)) $new_selected[] = $_POST[$scope . '_available'][$i];
		}
		$selected = $new_selected;
		$_SESSION['conf_' . $scope . 'modules'] = $selected;
	}
	
	// show account modules
	echo "<fieldset class=\"" . $scope . "edit-bright\"><legend class=\"" . $scope . "edit-bright\"><b>" . $title . "</b></legend>\n";
	echo "<table border=0 width=\"100%\">\n";
		// select boxes
		echo "<tr>\n";
			echo "<td width=\"5%\"></td>\n";
			echo "<td width=\"40%\">\n";
				echo "<fieldset class=\"" . $scope . "edit-bright\">\n";
					echo "<legend class=\"" . $scope . "edit-bright\">" . _("Selected modules") . "</legend>\n";
					echo "<select class=\"" . $scope . "edit-bright\" name=\"" . $scope . "_selected[]\" size=5 multiple>\n";
						for ($i = 0; $i < sizeof($selected); $i++) {
							if (in_array($selected[$i], $available)) {  // selected modules must be available
								if (is_base_module($selected[$i], $scope)) {  // mark base modules
									echo "<option value=\"" . $selected[$i] . "\">";
									echo $selected[$i] . "(" . getModuleAlias($selected[$i], $scope) .  ")(*)";
									echo "</option>\n";
								}
								else {
									echo "<option value=\"" . $selected[$i] . "\">";
									echo $selected[$i] . "(" . getModuleAlias($selected[$i], $scope) .  ")";
									echo "</option>\n";
								}
							}
						}
					echo "</select>\n";
				echo "</fieldset>\n";
			echo "</td>\n";
			echo "<td width=\"10%\" align=\"center\">\n";
				echo "<p>";
					echo "<input type=submit value=\"&lt;=\" name=\"" . $scope . "_add\">";
					echo "<br>";
					echo "<input type=submit value=\"=&gt;\" name=\"" . $scope . "_remove\">";
				echo "</p>\n";
			echo "</td>\n";
			echo "<td width=\"40%\">\n";
				echo "<fieldset class=\"" . $scope . "edit-bright\">\n";
					echo "<legend class=\"" . $scope . "edit-bright\">" . _("Available modules") . "</legend>\n";
					echo "<select class=\"" . $scope . "edit-bright\" name=\"" . $scope . "_available[]\" size=5 multiple>\n";
						for ($i = 0; $i < sizeof($available); $i++) {
							if (! in_array($available[$i], $selected)) {  // display non-selected modules
								if (is_base_module($available[$i], $scope)) {  // mark base modules
									echo "<option value=\"" . $available[$i] . "\">";
									echo $available[$i] . "(" . getModuleAlias($available[$i], $scope) .  ")(*)";
									echo "</option>\n";
								}
								else {
									echo "<option value=\"" . $available[$i] . "\">";
									echo $available[$i] . "(" . getModuleAlias($available[$i], $scope) .  ")";
									echo "</option>\n";
								}
							}
						}
					echo "</select>\n";
				echo "</fieldset>\n";
			echo "</td>\n";
			echo "<td width=\"5%\"></td>\n";
		echo "</tr>\n";
	echo "</table>\n";
	
	// check dependencies
	$depends = check_module_depends($selected, getModulesDependencies($scope));
	if ($depends != false) {
		$no_depends = false;
		echo "<p>\n";
			for ($i = 0; $i < sizeof($depends); $i++) {
				echo "<font color=\"red\"><b>" . _("Unsolved dependency:") . " </b>" . $depends[$i][0] . " (" .
					$depends[$i][1] . ")" . "</font><br>\n";
			}
		echo "<p>\n";
	}
	
	// check conflicts
	$conflicts = check_module_conflicts($selected, getModulesDependencies($scope));
	if ($conflicts != false) {
		$no_conflicts = false;
		echo "<p>\n";
			for ($i = 0; $i < sizeof($conflicts); $i++) {
				echo "<font color=\"red\"><b>" . _("Conflicting module:") . " </b>" . $conflicts[$i][0] . " (" .
					$conflicts[$i][1] . ")" . "</font><br>\n";
			}
		echo "<p>\n";
	}
	
	// check for base module
	$baseCount = 0;
	for ($i = 0; $i < sizeof($selected); $i++) {
		if (is_base_module($selected[$i], $scope)) {
			$baseCount++;
		}
	}
	if ($baseCount != 1) {
		$no_missing_basemodule = false;
		echo "<p>\n";
				echo "<font color=\"red\"><b>" . _("No or more than one base module selected!") . "</b></font><br>\n";
		echo "<p>\n";
	}
	
	echo "</fieldset>\n";
	
	echo "<p></p>\n";
	
	return ($no_conflicts & $no_depends & $no_missing_basemodule);
	
}


?>




