<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Roland Gruber, Leonhard Walchshäusl
  Copyright (C) 2004  Roland Gruber

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more detaexils.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/


/**
* This page displays a list of all users.
*
* @package lists
* @author Roland Gruber
*/


/** Access to configuration options */
include_once ("../../lib/config.inc");
/** Access to LDAP connection */
include_once ("../../lib/ldap.inc");
/** Used to print status messages */
include_once ("../../lib/status.inc");
/** Used to create PDF files */
include_once("../../lib/pdf.inc");
/** Access to account modules */
include_once("../../lib/modules.inc");
/** Basic list functions */
include_once("../../lib/lists.inc");


// start session
session_save_path("../../sess");
@session_start();

setlanguage();

$scope = 'user';

// copy HTTP-GET variables to HTTP-POST
$_POST = $_POST + $_GET;

// check if primary group should be translated
if ($_POST['trans_primary'] == "on") $trans_primary = "on";
else $trans_primary = "off";
$trans_primary_hash = $_SESSION['trans_primary_hash'];
// generate hash table for group translation
if ($trans_primary == "on" && !$_GET["norefresh"]) {
	$trans_primary_hash = array();
	$suffix = $_SESSION['config']->get_groupSuffix();
	$filter = "objectClass=posixGroup";
	$attrs = array("cn", "gidNumber");
	$sr = @ldap_search($_SESSION["ldap"]->server(), $suffix, $filter, $attrs);
	if ($sr) {
		$info = @ldap_get_entries($_SESSION["ldap"]->server(), $sr);
		unset($info['count']); // delete count entry
		for ($i = 0; $i < sizeof($info); $i++) {
			$trans_primary_hash[$info[$i]['gidnumber'][0]] = $info[$i]['cn'][0];
		}
		$_SESSION['trans_primary_hash'] = $trans_primary_hash;
	}
}


$info = $_SESSION[$scope . 'info'];
$usr_units = $_SESSION['usr_units'];

listDoPost($scope);

echo $_SESSION['header'];

echo "<title>listusers</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head><body>\n";
echo "<script src=\"../../lib/functions.js\" type=\"text/javascript\" language=\"javascript\"></script>\n";

$page = $_GET["page"];
if (!$page) $page = 1;

// take maximum count of user entries shown on one page out of session
if ($_SESSION["config"]->get_MaxListEntries() <= 0) {
	$max_page_entries = 10;	// default setting, if not yet set
}
else $max_page_entries = $_SESSION["config"]->get_MaxListEntries();

// generate attribute-description table
$attr_array = array();	// list of LDAP attributes to show
$desc_array = array();	// list of descriptions for the attributes
$attr_string = $_SESSION["config"]->get_userlistAttributes();
$temp_array = explode(";", $attr_string);
$hash_table = listGetAttributeUserArray();

// generate column attributes and descriptions
for ($i = 0; $i < sizeof($temp_array); $i++) {
  // if value is predifined, look up description in hash_table
  if (substr($temp_array[$i],0,1) == "#") {
    $attr = strtolower(substr($temp_array[$i],1));
    $attr_array[$i] = $attr;
    if ($hash_table[$attr]) $desc_array[] = strtoupper($hash_table[$attr]);
	else $desc_array[] = strtoupper($attr);
  }
  // if not predefined, the attribute is seperated by a ":" from description
  else {
    $attr = explode(":", $temp_array[$i]);
    $attr_array[$i] = $attr[0];
    if ($attr[1]) $desc_array[$i] = strtoupper($attr[1]);
	else $desc_array[$i] = strtoupper($attr[0]);
  }
}

$sort = $_GET["sort"];
if (!$sort)
     $sort = strtolower($attr_array[0]);

// check search suffix
if ($_POST['usr_suffix']) $usr_suffix = $_POST['usr_suffix'];  // new suffix selected via combobox
elseif ($_SESSION['usr_suffix']) $usr_suffix = $_SESSION['usr_suffix'];  // old suffix from session
else $usr_suffix = $_SESSION["config"]->get_UserSuffix();  // default suffix


// configure search filter for LDAP
$module_filter = get_ldap_filter("user");  // basic filter is provided by modules
$filter = "(&" . $module_filter . ")";

$refresh = true;
if ($_GET['norefresh']) $refresh = false;
if ($_POST['refresh']) $refresh = true;

if ($refresh) {
	$attrs = $attr_array;
	$sr = @ldap_search($_SESSION["ldap"]->server(), $usr_suffix, $filter, $attrs);
	if (ldap_errno($_SESSION["ldap"]->server()) == 4) {
		StatusMessage("WARN", _("LDAP sizelimit exceeded, not all entries are shown."), _("See README.openldap.txt to solve this problem."));
	}
	if ($sr) {
		$info = ldap_get_entries ($_SESSION["ldap"]->server, $sr);
		ldap_free_result ($sr);
		// delete first array entry which is "count"
		unset($info['count']);
		// save results
		$_SESSION[$scope . 'info'] = $info;
	}
	else {
		$_SESSION[$scope . 'info'] = array();
		$info = array();
		StatusMessage("ERROR",
			_("LDAP Search failed! Please check your preferences."),
			_("No users found!"));
	}
}

$filter = listBuildFilter($_POST, $attr_array);
$info = listFilterAccounts($info, $filter);
if (sizeof($info) == 0) StatusMessage("WARN", "", _("No users found!"));
// sort rows by sort column ($sort)
if ($info) {
	$info = listSort($sort, $attr_array, $info);
	$_SESSION[$scope . 'info'] = $info;
}

// build filter URL
$searchFilter = array();
$filterAttributes = array_keys($filter);
for ($i = 0; $i < sizeof($filterAttributes); $i++) {
	$searchFilter[] = "filter" . $filterAttributes[$i] . "=" . $filter[$filterAttributes[$i]]['original'];
}
if (sizeof($searchFilter) > 0) {
	$searchFilter = "&amp;" . implode("&amp;", $searchFilter);
}
else {
	$searchFilter = "";
}

$user_count = sizeof($info);

echo ("<form action=\"listusers.php?norefresh=true\" method=\"post\">\n");

// display table only if users exist in LDAP
if ($user_count != 0) {
	// create navigation bar on top of user table
	listDrawNavigationBar($user_count, $max_page_entries, $page, $sort,
		$searchFilter . "&amp;trans_primary=" . $trans_primary, "user", _("%s user(s) found"));
	echo ("<br />");
}

// account table head
listPrintTableHeader("user", $searchFilter . "&amp;trans_primary=" . $trans_primary, $desc_array, $attr_array, $_POST, $sort);

// calculate which rows to show
$table_begin = ($page - 1) * $max_page_entries;
if (($page * $max_page_entries) > sizeof($info)) $table_end = sizeof($info);
else $table_end = ($page * $max_page_entries);

if ($user_count != 0) {
	// translate GIDs and resort array if selected
	if ($trans_primary == "on") {
		// translate GIDs
		for ($i = 0; $i < sizeof($info); $i++) {
			if ($trans_primary_hash[$info[$i]['gidnumber'][0]]) {
				$info[$i]['gidnumber'][0] = $trans_primary_hash[$info[$i]['gidnumber'][0]];
			}
		}
		// resort if needed
		if ($sort == "gidnumber") {
			$info = listSort($sort, $attr_array, $info);
			$_SESSION[$scope . 'info'] = $info;
		}
	}
	// print user list
	for ($i = $table_begin; $i < $table_end; $i++) {
		echo("<tr class=\"userlist\"\nonMouseOver=\"user_over(this, '" . $i . "')\"\n" .
			"onMouseOut=\"user_out(this, '" . $i . "')\"\n" .
			"onClick=\"user_click(this, '" . $i . "')\"\n" .
			"onDblClick=\"parent.frames[1].location.href='../account/edit.php?type=user&amp;DN=" . $info[$i]['dn'] . "'\">\n");
		// checkboxes if selectall = "yes"
		if ($_GET['selectall'] == "yes") {
			echo "<td height=22 align=\"center\">\n<input onClick=\"user_click(this, '" . $i . "')\" type=\"checkbox\" name=\"" .
				$i . "\" checked>\n</td>\n";
		}
		else {
			echo "<td height=22 align=\"center\">\n<input onClick=\"user_click(this, '" . $i . "')\" type=\"checkbox\" name=\"" .
				$i . "\">\n</td>\n";
		}
		echo ("<td align='center'>\n<a href=\"../account/edit.php?type=user&amp;DN='" . $info[$i]['dn'] . "'\">" .
			_("Edit") . "</a>\n</td>\n");
		for ($k = 0; $k < sizeof($attr_array); $k++) {
			echo ("<td>\n");
			// print attribute values
			if (sizeof($info[$i][strtolower($attr_array[$k])]) > 0) {
				if (is_array($info[$i][strtolower($attr_array[$k])])) {
					// delete first array entry which is "count"
					unset($info[$i][strtolower($attr_array[$k])]['count']);
					// sort array
					sort($info[$i][strtolower($attr_array[$k])]);
					// print all attribute entries seperated by "; "
					echo implode("; ", $info[$i][strtolower($attr_array[$k])]) . "\n";
				}
				else echo $info[$i][strtolower($attr_array[$k])] . "\n";
			}
		echo ("</td>\n");
		}
	echo("</tr>\n");
	}
	// display select all link
	$colspan = sizeof($attr_array) + 1;
	echo "<tr class=\"userlist\">\n";
	echo "<td align=\"center\"><img src=\"../../graphics/select.png\" alt=\"select all\"></td>\n";
	echo "<td colspan=$colspan>&nbsp;<a href=\"listusers.php?norefresh=1&amp;page=" . $page . "&amp;sort=" . $sort .
		$searchFilter . "&amp;trans_primary=" . $trans_primary . "&amp;selectall=yes\">" .
		"<font color=\"black\"><b>" . _("Select all") . "</b></font></a></td>\n";
	echo "</tr>\n";
}
echo ("</table>\n");

echo ("<br>");
if ($user_count != 0) {
	listDrawNavigationBar($user_count, $max_page_entries, $page, $sort,
		$searchFilter . "&amp;trans_primary=" . $trans_primary, "user", _("%s user(s) found"));
	echo ("<br>");
}

if (! $_GET['norefresh']) {
	// generate list of possible suffixes
	$usr_units = $_SESSION['ldap']->search_units($_SESSION["config"]->get_UserSuffix());
}

// print combobox with possible sub-DNs
if (sizeof($usr_units) > 1) {
	echo ("<p align=\"left\">\n");
	echo ("<b>" . _("Suffix") . ": </b>");
	echo ("<select size=1 name=\"usr_suffix\">\n");
	for ($i = 0; $i < sizeof($usr_units); $i++) {
		if ($usr_suffix == $usr_units[$i]) echo ("<option selected>" . $usr_units[$i] . "</option>\n");
		else echo("<option>" . $usr_units[$i] . "</option>\n");
	}
	echo ("</select>\n");
	echo ("<input type=\"submit\" name=\"refresh\" value=\"" . _("Change Suffix") . "\">");
	echo ("</p>\n");
}

// show translate GID to group name box if there is a column with gidnumber
if (in_array("gidnumber", $attr_array)) {
	echo "<p align=\"left\">\n";
	echo "<b>" . _("Translate GID number to group name") . ": </b>";
	if ($trans_primary == "on") {
		echo "<input type=\"checkbox\" name=\"trans_primary\" checked>";
	}
	else echo "<input type=\"checkbox\" name=\"trans_primary\">";
	echo ("&nbsp;&nbsp;<input type=\"submit\" name=\"apply\" value=\"" . _("Apply") . "\">");
	echo "</p>\n";
}

echo ("<p>&nbsp;</p>\n");

// new/delete/PDF buttons
echo ("<input type=\"submit\" name=\"new\" value=\"" . _("New user") . "\">\n");
if ($user_count != 0) {
	echo ("<input type=\"submit\" name=\"del\" value=\"" . _("Delete user(s)") . "\">\n");
	echo ("<br><br><br>\n");
	echo "<fieldset><legend><b>PDF</b></legend>\n";
	echo ("<b>" . _('PDF structure') . ":</b>&nbsp;&nbsp;<select name=\"pdf_structure\">\n");
	$pdf_structures = getAvailablePDFStructures('user');
	foreach($pdf_structures as $pdf_structure) {
		echo "<option value=\"" . $pdf_structure . "\"" . (($pdf_structure == 'default.xml') ? " selected" : "") . ">" . substr($pdf_structure,0,strlen($pdf_structure)-4) . "</option>";
	}
	echo "</select>&nbsp;&nbsp;&nbsp;&nbsp;\n";
	echo ("<input type=\"submit\" name=\"pdf\" value=\"" . _("Create PDF for selected user(s)") . "\">\n");
	echo "&nbsp;";
	echo ("<input type=\"submit\" name=\"pdf_all\" value=\"" . _("Create PDF for all users") . "\">\n");
	echo "</fieldset>";
}

echo ("<p>&nbsp;</p>\n");

echo ("</form>\n");
echo "</body></html>\n";


// save variables to session
$_SESSION['usr_units'] = $usr_units;
$_SESSION['usr_suffix'] = $usr_suffix;

?>
