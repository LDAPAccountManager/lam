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

include_once ("../../lib/config.inc");
include_once("../../lib/ldap.inc");
include_once("../../lib/pdf.inc");
include_once("../../lib/account.inc");

// used to display status messages
include_once ("../../lib/status.inc");

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

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
		array_shift($info); // delete count entry
		for ($i = 0; $i < sizeof($info); $i++) {
			$trans_primary_hash[$info[$i]['gidnumber'][0]] = $info[$i]['cn'][0];
		}
		$_SESSION['trans_primary_hash'] = $trans_primary_hash;
	}
}


$usr_units = $_SESSION['usr_units'];

// check if button was pressed and if we have to add/delete a user or create a PDF
if ($_POST['new_user'] || $_POST['del_user'] || $_POST['pdf_user'] || $_POST['pdf_all']){
	// add new user
	if ($_POST['new_user']){
		metaRefresh("../account/useredit.php?type=user");
		exit;
	}
	// delete user(s)
	elseif ($_POST['del_user']){
		// search for checkboxes
		while ($entry = @array_pop($_POST)) {
			if (eregi("^uid=.*$", $entry)) $users[] = $entry;
		}
		$_SESSION['delete_dn'] = $users;
		if (sizeof($users) > 0) {
			metaRefresh("../delete.php?type=user");
			exit;
		}
	}
	// PDF for selected users
	elseif ($_POST['pdf_user']){
		// search for checkboxes
		while ($entry = @array_pop($_POST)) {
			if (eregi("^uid=.*$", $entry)) $users[] = $entry;
		}
		$list = array();
		// load users from LDAP
		for ($i = 0; $i < sizeof($users); $i++) {
			$list[$i] = loaduser($users[$i]);
			$list[$i]->unix_password = "";
			$list[$i]->smb_password = "";
		}
		if (sizeof($list) > 0) {
			if ($_SESSION['config']->get_scriptServer()) $list = getquotas($list);
			createUserPDF($list);
			exit;
		}
	}
	// PDF for all users
	elseif ($_POST['pdf_all']){
		$list = array();
		for ($i = 0; $i < sizeof($_SESSION['userlist']); $i++) {
			$list[$i] = loaduser($_SESSION['userlist'][$i]['dn']);
			$list[$i]->unix_password = "";
			$list[$i]->smb_password = "";
		}
		if (sizeof($list) > 0) {
			if ($_SESSION['config']->get_scriptServer()) $list = getquotas($list);
			createUserPDF($list);
			exit;
		}
	}
}

echo $_SESSION['header'];

echo "<title>listusers</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head><body>\n";
echo "<script src=\"../../lib/functions.js\" type=\"text/javascript\" language=\"javascript\"></script>\n";

$page = $_GET["page"];
if (!$page) $page = 1;

// take maximum count of user entries shown on one page out of session
if ($_SESSION["config"]->get_MaxListEntries() <= 0) {
	$max_pageentrys = 10;	// default setting, if not yet set
}
else $max_pageentrys = $_SESSION["config"]->get_MaxListEntries();

// generate attribute-description table
$attr_array = array();	// list of LDAP attributes to show
$desc_array = array();	// list of descriptions for the attributes
$attr_string = $_SESSION["config"]->get_userlistAttributes();
$temp_array = explode(";", $attr_string);
$hash_table = $_SESSION["ldap"]->attributeUserArray();

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

$sortattrib = $_GET["sortattrib"];
if (!$sortattrib)
     $sortattrib = strtolower($attr_array[0]);

// check search suffix
if ($_POST['usr_suffix']) $usr_suffix = $_POST['usr_suffix'];  // new suffix selected via combobox
elseif ($_SESSION['usr_suffix']) $usr_suffix = $_SESSION['usr_suffix'];  // old suffix from session
else $usr_suffix = $_SESSION["config"]->get_UserSuffix();  // default suffix


// generate search filter for sort links
$searchfilter = "";
for ($k = 0; $k < sizeof($desc_array); $k++) {
	if (eregi("^([0-9a-z_\\*\\+\\-])+$", $_POST["filter" . strtolower($attr_array[$k])])) {
		$searchfilter = $searchfilter . "&filter" .
		  strtolower($attr_array[$k]) . "=".
		  $_POST["filter" . strtolower($attr_array[$k])];
	}
}

// configure search filter
// Unix/Samba3 users have the attribute "posixAccount" and do not end with "$"
$filter = "(&(objectClass=posixAccount) (!(uid=*$))";
for ($k = 0; $k < sizeof($desc_array); $k++) {
  if (eregi("^([0-9a-z_\\*\\+\\-])+$", $_POST["filter" . strtolower($attr_array[$k])]))
    $filter = $filter . "(" . strtolower($attr_array[$k]) . "=" .
      $_POST["filter" . strtolower($attr_array[$k])] . ")";
  else
    $_POST["filter" . strtolower($attr_array[$k])] = "";
}
$filter = $filter . ")";

// read entries only from ldap server if not yet stored in session or if refresh
// button is pressed or if filter is applied
if ($_SESSION["userlist"] && $_GET["norefresh"]) {
	usort ($_SESSION["userlist"], "cmp_array");
	$userinfo = $_SESSION["userlist"];
}
else {
	$attrs = $attr_array;
	$sr = @ldap_search($_SESSION["ldap"]->server(), $usr_suffix, $filter, $attrs);
	if (ldap_errno($_SESSION["ldap"]->server()) == 4) {
		StatusMessage("WARN", _("LDAP sizelimit exceeded, not all entries are shown."), "See README.openldap to solve this problem.");
	}
	if ($sr) {
		$userinfo = ldap_get_entries ($_SESSION["ldap"]->server, $sr);
		ldap_free_result ($sr);
		if ($userinfo["count"] == 0) StatusMessage("WARN", "", _("No Users found!"));
			// delete first array entry which is "count"
			array_shift($userinfo);
			usort ($userinfo, "cmp_array");
			$_SESSION["userlist"] = $userinfo;
		}
	else {
		$_SESSION['userlist'] = array();
		$userinfo = array();
		StatusMessage("ERROR",
			_("LDAP Search failed! Please check your preferences."),
			_("No Users found!"));
	}
}

$user_count = sizeof ($_SESSION["userlist"]);

echo ("<form action=\"listusers.php\" method=\"post\">\n");

// display table only if users exist in LDAP
if ($user_count != 0) {

  // create navigation bar on top of user table
  draw_navigation_bar ($user_count);

  echo ("<br />");
}

  // print user table header
  echo "<table rules=\"all\" class=\"userlist\" width=\"100%\">\n";


  echo "<tr class=\"userlist-head\"><th width=22 height=34></th><th></th>\n";
  // table header
  for ($k = 0; $k < sizeof ($desc_array); $k++) {
    if ($sortattrib == strtolower($attr_array[$k]))
      echo "<th class=\"userlist-activecolumn\">\n";
    else
      echo "<th>\n";
    echo "<a class=\"userlist\" href=\"listusers.php?norefresh=1&amp;sortattrib=" .
      strtolower($attr_array[$k]) . $searchfilter . "&amp;trans_primary=" . $trans_primary . "\">" .
      $desc_array[$k] . "</a></th>\n";
  }
  echo "</tr>\n";

  echo "<tr class=\"userlist\"><th width=22 height=34></th>\n<th>\n";
  echo "<input type=\"submit\" name=\"apply_filter\" value=\"" . _("Filter") . "\">\n";
  echo "</th>\n";

  // print input boxes for filters
  for ($k = 0; $k < sizeof ($desc_array); $k++) {
    echo "<th>\n";
    echo ("<input type=\"text\" size=15 name=\"filter" . strtolower ($attr_array[$k]) .
	  "\" value=\"" . $_POST["filter" . strtolower($attr_array[$k])] . "\">\n");
    echo "</th>\n";
  }
  echo "</tr>\n";

if ($user_count != 0) {
	// translate GIDs and resort array if selected
	if ($trans_primary == "on") {
		// translate GIDs
		for ($i = 0; $i < sizeof($userinfo); $i++) {
			if ($trans_primary_hash[$userinfo[$i]['gidnumber'][0]]) {
				$userinfo[$i]['gidnumber'][0] = $trans_primary_hash[$userinfo[$i]['gidnumber'][0]];
			}
		}
		// resort if needed
		if ($sortattrib == "gidnumber") {
			usort ($userinfo, "cmp_array");
		}
	}
	// print user list
	$userinfo = array_slice ($userinfo, ($page - 1) * $max_pageentrys, $max_pageentrys);
	for ($i = 0; $i < sizeof ($userinfo); $i++) { // ignore last entry in array which is "count"
		echo("<tr class=\"userlist\"\nonMouseOver=\"user_over(this, '" . $userinfo[$i]["dn"] . "')\"\n" .
			"onMouseOut=\"user_out(this, '" . $userinfo[$i]["dn"] . "')\"\n" .
			"onClick=\"user_click(this, '" . $userinfo[$i]["dn"] . "')\"\n" .
			"onDblClick=\"parent.frames[1].location.href='../account/useredit.php?type=user&amp;DN=" . $userinfo[$i]["dn"] . "'\">\n");
		// checkboxes if selectall = "yes"
		if ($_GET['selectall'] == "yes") {
			echo "<td height=22 align=\"center\">\n<input onClick=\"user_click(this, '" . $userinfo[$i]["dn"] . "')\" type=\"checkbox\" name=\"" .
				$userinfo[$i]["dn"] . "\" value=\"" . $userinfo[$i]["dn"] . "\" checked>\n</td>\n";
		}
		else {
			echo "<td height=22 align=\"center\">\n<input onClick=\"user_click(this, '" . $userinfo[$i]["dn"] . "')\" type=\"checkbox\" name=\"" .
				$userinfo[$i]["dn"] . "\" value=\"" . $userinfo[$i]["dn"] . "\">\n</td>\n";
		}
		echo ("<td align='center'>\n<a href=\"../account/useredit.php?type=user&amp;DN='" . $userinfo[$i]["dn"] . "'\">" .
			_("Edit") . "</a>\n</td>\n");
		for ($k = 0; $k < sizeof($attr_array); $k++) {
			echo ("<td>\n");
			// print attribute values
			if (sizeof($userinfo[$i][strtolower($attr_array[$k])]) > 0) {
				if (is_array($userinfo[$i][strtolower($attr_array[$k])])) {
					// delete first array entry which is "count"
					array_shift($userinfo[$i][strtolower($attr_array[$k])]);
					// sort array
					sort($userinfo[$i][strtolower($attr_array[$k])]);
					// print all attribute entries seperated by "; "
					echo utf8_decode(implode("; ", $userinfo[$i][strtolower($attr_array[$k])])) . "\n";
				}
				else echo utf8_decode($userinfo[$i][strtolower($attr_array[$k])]) . "\n";
			}
		echo ("</td>\n");
		}
	echo("</tr>\n");
	}
	// display select all link
	$colspan = sizeof($attr_array) + 1;
	echo "<tr class=\"userlist\">\n";
	echo "<td align=\"center\"><img src=\"../../graphics/select.png\" alt=\"select all\"></td>\n";
	echo "<td colspan=$colspan>&nbsp;<a href=\"listusers.php?norefresh=1&amp;page=" . $page . "&amp;sortattrib=" . $sortattrib .
		$searchfilter . "&amp;trans_primary=" . $trans_primary . "&amp;selectall=yes\">" .
		"<font color=\"black\"><b>" . _("Select all") . "</b></font></a></td>\n";
	echo "</tr>\n";
}
echo ("</table>\n");

echo ("<br>");
if ($user_count != 0) {
  draw_navigation_bar ($user_count);
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
echo ("<input type=\"submit\" name=\"new_user\" value=\"" . _("New user") . "\">\n");
if ($user_count != 0) {
	echo ("<input type=\"submit\" name=\"del_user\" value=\"" . _("Delete user(s)") . "\">\n");
	echo ("<br><br><br>\n");
	echo "<fieldset><legend><b>PDF</b></legend>\n";
	echo ("<input type=\"submit\" name=\"pdf_user\" value=\"" . _("Create PDF for selected user(s)") . "\">\n");
	echo "&nbsp;";
	echo ("<input type=\"submit\" name=\"pdf_all\" value=\"" . _("Create PDF for all users") . "\">\n");
	echo "</fieldset>";
}

echo ("<p>&nbsp;</p>\n");

echo ("</form>\n");
echo "</body></html>\n";

/**
 * @brief draws a navigation bar to switch between pages
 *
 *
 * @return void
 */
function draw_navigation_bar ($user_count) {
	global $max_pageentrys;
	global $page;
	global $sortattrib;
	global $searchfilter;
	global $trans_primary;

	echo ("<table class=\"userlist-navbar\" width=\"100%\" border=\"0\"\n");
	echo ("<tr>\n");
	echo ("<td class=\"userlist-navbar\">\n<input type=\"submit\" name=\"refresh\" value=\"" . _("Refresh") . "\">\n&nbsp;&nbsp;");
	if ($page != 1)
		echo ("<a class=\"userlist\" href=\"listusers.php?norefresh=1&amp;page=" .
			($page - 1) . "&amp;sortattrib=" . $sortattrib . $searchfilter . "&amp;trans_primary=" . $trans_primary . "\">&lt;=</a>\n");
	else echo ("&lt;=");
	echo ("&nbsp;");

	if ($page < ($user_count / $max_pageentrys))
		echo ("<a class=\"userlist\" href=\"listusers.php?norefresh=1&amp;page=" .
			($page + 1) . "&amp;sortattrib=" . $sortattrib . $searchfilter . "&amp;trans_primary=" . $trans_primary . "\">=&gt;</a>\n");
	else echo ("=&gt;");
	echo ("</td>\n");
	echo ("<td class=\"userlist-navbartext\">\n");
	echo "&nbsp;" . $user_count . " " .  _("User(s) found") . "\n";
	echo ("</td>\n");


	echo ("<td class=\"userlist-activepage\" align=\"right\">");
	for ($i = 0; $i < ($user_count / $max_pageentrys); $i++) {
		if ($i == $page - 1) echo ("&nbsp;" . ($i + 1));
		else echo ("&nbsp;<a class=\"userlist\" href=\"listusers.php?norefresh=1&amp;page=" .
				($i + 1) . "&amp;sortattrib=" . $sortattrib . $searchfilter . "&amp;trans_primary=" . $trans_primary .
				"\">" . ($i + 1) . "</a>\n");
	}
	echo ("</td></tr>\n</table>\n");
}


// compare function used for usort-method
// rows are sorted with the first attribute entry of the sort column
// if objects have attributes with multiple values the others are ignored
function cmp_array($a, $b) {
	// sortattrib specifies the sort column
	global $sortattrib;
	global $attr_array;
	// sort by first attribute with name $sortattrib
	if (!$sortattrib) $sortattrib = strtolower($attr_array[0]);
	if ($sortattrib != "dn") {
		// sort by first column if no attribute is given
		if ($a[$sortattrib][0] == $b[$sortattrib][0]) return 0;
		else if ($a[$sortattrib][0] == max($a[$sortattrib][0], $b[$sortattrib][0])) return 1;
		else return -1;
	}
	else {
		if ($a[$sortattrib] == $b[$sortattrib]) return 0;
		else if ($a[$sortattrib] == max($a[$sortattrib], $b[$sortattrib])) return 1;
		else return -1;
	}
}

// save variables to session
$_SESSION['usr_units'] = $usr_units;
$_SESSION['usr_suffix'] = $usr_suffix;

?>
