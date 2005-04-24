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
* @package profiles
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
	$old_options = loadAccountProfile($_GET['edit'], $type);
}

// display formular
echo ("<form action=\"profilecreate.php?type=$type\" method=\"post\">\n");

// suffix box
// get root suffix
$rootsuffix = $_SESSION['config']->get_Suffix($type);
// get subsuffixes
$suffixes = array();
foreach ($_SESSION['ldap']->search_units($rootsuffix) as $suffix) {
	$suffixes[] = $suffix;
}
if (sizeof($suffixes) > 0) {
echo "<fieldset class=\"" . $type . "edit\">\n<legend><b>" . _("LDAP suffix") . "</b></legend>\n";
	echo _("LDAP suffix") . ":&nbsp;&nbsp;";
	echo "<select tabindex=\"1\">";
	for ($i = 0; $i < sizeof($suffixes); $i++) echo "<option>" . $suffixes[$i] . "</option>\n";
	echo "</select>\n";
	// help link
	echo "&nbsp;<a href=\"../help.php?HelpNumber=361\" target=\"lamhelp\">";
	echo "<img src=\"../../graphics/help.png\" alt=\"" . _('Help') . "\" title=\"" . _('Help') . "\">";
	echo "</a>\n";
echo "</fieldset>\n<br>\n";
}

// index for tab order (1 is LDAP suffix)
$tabindex = 2;
$tabindexLink = 1000;	// links are at the end

// display module options
$modules = array_keys($options);
for ($m = 0; $m < sizeof($modules); $m++) {
	// ignore modules without options
	if (sizeof($options[$modules[$m]]) < 1) continue;
	echo "<fieldset class=\"" . $type . "edit\">\n";
		echo "<legend><b>" . getModuleAlias($modules[$m], $type) . "</b></legend>\n";
	$profileTypes = parseHtml($modules[$m], $options[$modules[$m]], $old_options, true, $tabindex, $tabindexLink, $type);
	$_SESSION['profile_types'] = array_merge($profileTypes, $_SESSION['profile_types']);
	echo "</fieldset>\n";
	echo "<br>";
}

// profile name and submit/abort buttons
echo ("<b>" . _("Profile name") . ":</b> \n");
$tabindex++;
echo ("<input tabindex=\"$tabindex\" type=\"text\" name=\"profname\" value=\"" . $_GET['edit'] . "\">\n");
// help link
echo "<a href=\"../help.php?HelpNumber=360\" target=\"lamhelp\">";
echo "<img src=\"../../graphics/help.png\" alt=\"" . _('Help') . "\" title=\"" . _('Help') . "\">";
echo "</a><br><br>\n";
$tabindex++;
echo ("<input tabindex=\"$tabindex\" type=\"submit\" name=\"submit\" value=\"" . _("Save") . "\">\n");
$tabindex++;
echo ("<input tabindex=\"$tabindex\" type=\"reset\" name=\"reset\" value=\"" . _("Reset") . "\">\n");
$tabindex++;
echo ("<input tabindex=\"$tabindex\" type=\"submit\" name=\"abort\" value=\"" . _("Abort") . "\">\n");
echo "<input type=\"hidden\" name=\"accounttype\" value=\"$type\">\n";

echo ("</form></body></html>\n");

?>
