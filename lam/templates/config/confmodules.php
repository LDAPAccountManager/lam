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


// user modules
$selected_users_temp = $_SESSION['conf_usermodules'];
$available_users = array();
$available_users = getAvailableModules('user');
$selected_users = array();
// only use available modules as selected
for ($i = 0; $i < sizeof($selected_users_temp); $i++) {
	if (in_array($selected_users_temp[$i], $available_users)) $selected_users[] = $selected_users_temp[$i];
}
$no_conflicts_user = true;
$no_depends_user = true;
$no_missing_basemodule_user = true;

// remove modules from selection
if ($_POST['user_selected'] && ($_POST['user_remove'])) {
	$new_selected_users = array();
	for ($i = 0; $i < sizeof($selected_users); $i++) {
		if (! in_array($selected_users[$i], $_POST['user_selected'])) $new_selected_users[] = $selected_users[$i];
	}
	$selected_users = $new_selected_users;
	$_SESSION['conf_usermodules'] = $selected_users;
}

// add modules to selection
elseif ($_POST['user_available'] && ($_POST['user_add'])) {
	$new_selected_users = $selected_users;
	for ($i = 0; $i < sizeof($_POST['user_available']); $i++) {
		if (! in_array($_POST['user_available'][$i], $selected_users)) $new_selected_users[] = $_POST['user_available'][$i];
	}
	$selected_users = $new_selected_users;
	$_SESSION['conf_usermodules'] = $selected_users;
}


// group modules
$selected_groups_temp = $_SESSION['conf_groupmodules'];
$available_groups = array();
$available_groups = getAvailableModules('group');
$selected_groups = array();
// only use available modules as selected
for ($i = 0; $i < sizeof($selected_groups_temp); $i++) {
	if (in_array($selected_groups_temp[$i], $available_groups)) $selected_groups[] = $selected_groups_temp[$i];
}
$no_conflicts_group = true;
$no_depends_group = true;
$no_missing_basemodule_group = true;

// remove modules from selection
if ($_POST['group_selected'] && ($_POST['group_remove'])) {
	$new_selected_groups = array();
	for ($i = 0; $i < sizeof($selected_groups); $i++) {
		if (! in_array($selected_groups[$i], $_POST['group_selected'])) $new_selected_groups[] = $selected_groups[$i];
	}
	$selected_groups = $new_selected_groups;
	$_SESSION['conf_groupmodules'] = $selected_groups;
}

// add modules to selection
elseif ($_POST['group_available'] && ($_POST['group_add'])) {
	$new_selected_groups = $selected_groups;
	for ($i = 0; $i < sizeof($_POST['group_available']); $i++) {
		if (! in_array($_POST['group_available'][$i], $selected_groups)) $new_selected_groups[] = $_POST['group_available'][$i];
	}
	$selected_groups = $new_selected_groups;
	$_SESSION['conf_groupmodules'] = $selected_groups;
}


// host modules
$selected_hosts_temp = $_SESSION['conf_hostmodules'];
$available_hosts = array();
$available_hosts = getAvailableModules('host');
$selected_hosts = array();
// only use available modules as selected
for ($i = 0; $i < sizeof($selected_hosts_temp); $i++) {
	if (in_array($selected_hosts_temp[$i], $available_hosts)) $selected_hosts[] = $selected_hosts_temp[$i];
}
$no_conflicts_host = true;
$no_depends_host = true;
$no_missing_basemodule_host = true;

// remove modules from selection
if ($_POST['host_selected'] && ($_POST['host_remove'])) {
	$new_selected_hosts = array();
	for ($i = 0; $i < sizeof($selected_hosts); $i++) {
		if (! in_array($selected_hosts[$i], $_POST['host_selected'])) $new_selected_hosts[] = $selected_hosts[$i];
	}
	$selected_hosts = $new_selected_hosts;
	$_SESSION['conf_hostmodules'] = $selected_hosts;
}

// add modules to selection
elseif ($_POST['host_available'] && ($_POST['host_add'])) {
	$new_selected_hosts = $selected_hosts;
	for ($i = 0; $i < sizeof($_POST['host_available']); $i++) {
		if (! in_array($_POST['host_available'][$i], $selected_hosts)) $new_selected_hosts[] = $_POST['host_available'][$i];
	}
	$selected_hosts = $new_selected_hosts;
	$_SESSION['conf_hostmodules'] = $selected_hosts;
}


// show user modules
echo "<fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\"><b>" . _("User modules") . "</b></legend>\n";
echo "<table border=0 width=\"100%\">\n";
	// select boxes
	echo "<tr>\n";
		echo "<td width=\"5%\"></td>\n";
		echo "<td width=\"40%\">\n";
			echo "<fieldset class=\"useredit-bright\">\n";
				echo "<legend class=\"useredit-bright\">" . _("Selected user modules") . "</legend>\n";
				echo "<select class=\"useredit-bright\" name=\"user_selected[]\" size=5 multiple>\n";
					for ($i = 0; $i < sizeof($selected_users); $i++) {
						if (in_array($selected_users[$i], $available_users)) {  // selected modules must be available
							if (is_base_module($selected_users[$i], "user")) {  // mark base modules
								echo "<option value=\"" . $selected_users[$i] . "\">";
								echo $selected_users[$i] . "(" . getModuleAlias($selected_users[$i], "user") .  ")(" . _("base module") . ")";
								echo "</option>\n";
							}
							else {
								echo "<option value=\"" . $selected_users[$i] . "\">";
								echo $selected_users[$i] . "(" . getModuleAlias($selected_users[$i], "user") .  ")";
								echo "</option>\n";
							}
						}
					}
				echo "</select>\n";
			echo "</fieldset>\n";
		echo "</td>\n";
		echo "<td width=\"10%\" align=\"center\">\n";
			echo "<p>";
				echo "<input type=submit value=\"&lt;=\" name=\"user_add\">";
				echo "<br>";
				echo "<input type=submit value=\"=&gt;\" name=\"user_remove\">";
			echo "</p>\n";
		echo "</td>\n";
		echo "<td width=\"40%\">\n";
			echo "<fieldset class=\"useredit-bright\">\n";
				echo "<legend class=\"useredit-bright\">" . _("Available user modules") . "</legend>\n";
				echo "<select class=\"useredit-bright\" name=\"user_available[]\" size=5 multiple>\n";
					for ($i = 0; $i < sizeof($available_users); $i++) {
						if (! in_array($available_users[$i], $selected_users)) {  // display non-selected modules
							if (is_base_module($available_users[$i], "user")) {  // mark base modules
								echo "<option value=\"" . $available_users[$i] . "\">";
								echo $available_users[$i] . "(" . getModuleAlias($available_users[$i], "user") .  ")(" . _("base module") . ")";
								echo "</option>\n";
							}
							else {
								echo "<option value=\"" . $available_users[$i] . "\">";
								echo $available_users[$i] . "(" . getModuleAlias($available_users[$i], "user") .  ")";
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
$user_depends = check_module_depends($selected_users, getModulesDependencies('user'));
if ($user_depends != false) {
	$no_depends_user = false;
	echo "<p>\n";
		for ($i = 0; $i < sizeof($user_depends); $i++) {
			echo "<font color=\"red\"><b>" . _("Unsolved dependency:") . " </b>" . $user_depends[$i][0] . " (" .
				$user_depends[$i][1] . ")" . "</font><br>\n";
		}
	echo "<p>\n";
}

// check conflicts
$user_conflicts = check_module_conflicts($selected_users, getModulesDependencies('user'));
if ($user_conflicts != false) {
	$no_conflicts_user = false;
	echo "<p>\n";
		for ($i = 0; $i < sizeof($user_conflicts); $i++) {
			echo "<font color=\"red\"><b>" . _("Conflicting module:") . " </b>" . $user_conflicts[$i][0] . " (" .
				$user_conflicts[$i][1] . ")" . "</font><br>\n";
		}
	echo "<p>\n";
}

// check for base module
$found = false;
for ($i = 0; $i < sizeof($selected_users); $i++) {
	if (is_base_module($selected_users[$i], "user")) {
		$found = true;
		break;
	}
}
if (! $found) {
	$no_missing_basemodule_user = false;
	echo "<p>\n";
			echo "<font color=\"red\"><b>" . _("No base module selected!") . "</b></font><br>\n";
	echo "<p>\n";
}

echo "</fieldset>\n";

echo "<p></p>\n";


// show group modules
echo "<fieldset class=\"groupedit-bright\"><legend class=\"groupedit-bright\"><b>" . _("Group modules") . "</b></legend>\n";
echo "<table border=0 width=\"100%\">\n";
	// select boxes
	echo "<tr>\n";
		echo "<td width=\"5%\"></td>\n";
		echo "<td width=\"40%\">\n";
			echo "<fieldset class=\"groupedit-bright\">\n";
				echo "<legend class=\"groupedit-bright\">" . _("Selected group modules") . "</legend>\n";
				echo "<select class=\"groupedit-bright\" name=\"group_selected[]\" size=5 multiple>\n";
					for ($i = 0; $i < sizeof($selected_groups); $i++) {
						if (in_array($selected_groups[$i], $available_groups)) {  // selected modules must be available
							if (is_base_module($selected_groups[$i], "group")) {  // mark base modules
								echo "<option value=\"" . $selected_groups[$i] . "\">";
								echo $selected_groups[$i] . "(" . getModuleAlias($selected_groups[$i], "group") .  ")(" . _("base module") . ")";
								echo "</option>\n";
							}
							else {
								echo "<option value=\"" . $selected_groups[$i] . "\">";
								echo $selected_groups[$i] . "(" . getModuleAlias($selected_groups[$i], "group") .  ")";
								echo "</option>\n";
							}
						}
					}
				echo "</select>\n";
			echo "</fieldset>\n";
		echo "</td>\n";
		echo "<td width=\"10%\" align=\"center\">\n";
			echo "<p>";
				echo "<input type=submit value=\"&lt;=\" name=\"group_add\">";
				echo "<br>";
				echo "<input type=submit value=\"=&gt;\" name=\"group_remove\">";
			echo "</p>\n";
		echo "</td>\n";
		echo "<td width=\"40%\">\n";
			echo "<fieldset class=\"groupedit-bright\">\n";
				echo "<legend class=\"groupedit-bright\">" . _("Available group modules") . "</legend>\n";
				echo "<select class=\"groupedit-bright\" name=\"group_available[]\" size=5 multiple>\n";
					for ($i = 0; $i < sizeof($available_groups); $i++) {
						if (! in_array($available_groups[$i], $selected_groups)) {  // display non-selected modules
							if (is_base_module($available_groups[$i], "group")) {  // mark base modules
								echo "<option value=\"" . $available_groups[$i] . "\">";
								echo $available_groups[$i] . "(" . getModuleAlias($available_groups[$i], "group") .  ")(" . _("base module") . ")";
								echo "</option>\n";
							}
							else {
								echo "<option value=\"" . $available_groups[$i] . "\">";
								echo $available_groups[$i] . "(" . getModuleAlias($available_groups[$i], "group") .  ")";
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
$group_depends = check_module_depends($selected_groups, getModulesDependencies('group'));
if ($group_depends != false) {
	$no_depends_group = false;
	echo "<p>\n";
		for ($i = 0; $i < sizeof($group_depends); $i++) {
			echo "<font color=\"red\"><b>" . _("Unsolved dependency:") . " </b>" . $group_depends[$i][0] . " (" .
				$group_depends[$i][1] . ")" . "</font><br>\n";
		}
	echo "<p>\n";
}

// check conflicts
$group_conflicts = check_module_conflicts($selected_groups, getModulesDependencies('group'));
if ($group_conflicts != false) {
	$no_conflicts_group = false;
	echo "<p>\n";
		for ($i = 0; $i < sizeof($group_conflicts); $i++) {
			echo "<font color=\"red\"><b>" . _("Conflicting module:") . " </b>" . $group_conflicts[$i][0] . " (" .
				$group_conflicts[$i][1] . ")" . "</font><br>\n";
		}
	echo "<p>\n";
}

// check for base module
$found = false;
for ($i = 0; $i < sizeof($selected_groups); $i++) {
	if (is_base_module($selected_groups[$i], "group")) {
		$found = true;
		break;
	}
}
if (! $found) {
	$no_missing_basemodule_group = false;
	echo "<p>\n";
			echo "<font color=\"red\"><b>" . _("No base module selected!") . "</b></font><br>\n";
	echo "<p>\n";
}

echo "</fieldset>\n";

echo "<p></p>\n";


// show host modules
echo "<fieldset class=\"hostedit-bright\"><legend class=\"hostedit-bright\"><b>" . _("Host modules") . "</b></legend>\n";
echo "<table border=0 width=\"100%\">\n";
	// select boxes
	echo "<tr>\n";
		echo "<td width=\"5%\"></td>\n";
		echo "<td width=\"40%\">\n";
			echo "<fieldset class=\"hostedit-bright\">\n";
				echo "<legend class=\"hostedit-bright\">" . _("Selected host modules") . "</legend>\n";
				echo "<select class=\"hostedit-bright\" name=\"host_selected[]\" size=5 multiple>\n";
					for ($i = 0; $i < sizeof($selected_hosts); $i++) {
						if (in_array($selected_hosts[$i], $available_hosts)) {  // selected modules must be available
							if (is_base_module($selected_hosts[$i], "host")) {  // mark base modules
								echo "<option value=\"" . $selected_hosts[$i] . "\">";
								echo $selected_hosts[$i] . "(" . getModuleAlias($selected_hosts[$i], "host") .  ")(" . _("base module") . ")";
								echo "</option>\n";
							}
							else {
								echo "<option value=\"" . $selected_hosts[$i] . "\">";
								echo $selected_hosts[$i] . "(" . getModuleAlias($selected_hosts[$i], "host") .  ")";
								echo "</option>\n";
							}
						}
					}
				echo "</select>\n";
			echo "</fieldset>\n";
		echo "</td>\n";
		echo "<td width=\"10%\" align=\"center\">\n";
			echo "<p>";
				echo "<input type=submit value=\"&lt;=\" name=\"host_add\">";
				echo "<br>";
				echo "<input type=submit value=\"=&gt;\" name=\"host_remove\">";
			echo "</p>\n";
		echo "</td>\n";
		echo "<td width=\"40%\">\n";
			echo "<fieldset class=\"hostedit-bright\">\n";
				echo "<legend class=\"hostedit-bright\">" . _("Available host modules") . "</legend>\n";
				echo "<select class=\"hostedit-bright\" name=\"host_available[]\" size=5 multiple>\n";
					for ($i = 0; $i < sizeof($available_hosts); $i++) {
						if (! in_array($available_hosts[$i], $selected_hosts)) {  // display non-selected modules
							if (is_base_module($available_hosts[$i], "host")) {  // mark base modules
								echo "<option value=\"" . $available_hosts[$i] . "\">";
								echo $available_hosts[$i] . "(" . getModuleAlias($available_hosts[$i], "host") .  ")(" . _("base module") . ")";
								echo "</option>\n";
							}
							else {
								echo "<option value=\"" . $available_hosts[$i] . "\">";
								echo $available_hosts[$i] . "(" . getModuleAlias($available_hosts[$i], "host") .  ")";
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
$host_depends = check_module_depends($selected_hosts, getModulesDependencies('host'));
if ($host_depends != false) {
	$no_depends_host = false;
	echo "<p>\n";
		for ($i = 0; $i < sizeof($host_depends); $i++) {
			echo "<font color=\"red\"><b>" . _("Unsolved dependency:") . " </b>" . $host_depends[$i][0] . " (" .
				$host_depends[$i][1] . ")" . "</font><br>\n";
		}
	echo "<p>\n";
}

// check conflicts
$host_conflicts = check_module_conflicts($selected_hosts, getModulesDependencies('host'));
if ($host_conflicts != false) {
	$no_conflicts_host = false;
	echo "<p>\n";
		for ($i = 0; $i < sizeof($host_conflicts); $i++) {
			echo "<font color=\"red\"><b>" . _("Conflicting module:") . " </b>" . $host_conflicts[$i][0] . " (" .
				$host_conflicts[$i][1] . ")" . "</font><br>\n";
		}
	echo "<p>\n";
}

// check for base module
$found = false;
for ($i = 0; $i < sizeof($selected_hosts); $i++) {
	if (is_base_module($selected_hosts[$i], "host")) {
		$found = true;
		break;
	}
}
if (! $found) {
	$no_missing_basemodule_host = false;
	echo "<p>\n";
			echo "<font color=\"red\"><b>" . _("No base module selected!") . "</b></font><br>\n";
	echo "<p>\n";
}

echo "</fieldset>\n";


// submit buttons
echo "<p>\n";
	// disable button if there are conflicts/depends
	if ($no_conflicts_user && $no_depends_user && $no_missing_basemodule_user &&
		$no_conflicts_group && $no_depends_group && $no_missing_basemodule_group &&
		$no_conflicts_host && $no_depends_host && $no_missing_basemodule_host) {
		echo "<input type=\"submit\" value=\"" . _("Submit") . "\" name=\"submit\">\n";
	}
	else {
		echo "<input type=\"submit\" value=\"" . _("Submit") . "\" name=\"submit\" disabled>\n";
	}
	echo "&nbsp;";
	echo "<input type=\"submit\" value=\"" . _("Abort") . "\" name=\"abort\">\n";
echo "</p>\n";

echo "</form>\n";
echo "</body>\n";
echo "</html>\n";

?>




