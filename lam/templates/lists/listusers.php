<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Leonhard Walchshäusl

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

// used to display status messages
include_once ("../../lib/status.inc");

// start session
session_save_path("../../sess");
@session_start();

// copy HTTP-GET variables to HTTP-POST
$_POST = $_POST + $_GET;

// check if button was pressed and if we have to add/delete a user
if ($_POST['new_user'] || $_POST['del_user']){
  // add new user
  if ($_POST['new_user']){
    echo("<meta http-equiv=\"refresh\" content=\"0; URL=../account.php?type=user\">");
    exit;
  }
  // delete user(s)
  if ($_POST['del_user']){
    // search for checkboxes
    $users = array_keys($_POST, "on");
    $userstr = implode(";", $users);
    echo("<meta http-equiv=\"refresh\" content=\"0; URL=../delete.php?type=user&DN='$userstr'\">");
  }
  exit;
}

echo "<html><head><title>listusers</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head><body>\n";
echo "<script src=\"../../lib/functions.js\" type=\"text/javascript\" language=\"javascript\"></script>\n";

// generate attribute-description table
$attr_array;	// list of LDAP attributes to show
$desc_array;	// list of descriptions for the attributes
$attr_string = $_SESSION["config"]->get_userlistAttributes();
$temp_array = explode(";", $attr_string);
$hash_table = $_SESSION["ldap"]->attributeUserArray();
$page = $_GET["page"];
if (!$page)
     $page = 1;

// take maximum count of user entries shown on one page out of session
if ($_SESSION["config"]->get_MaxListEntries() <= 0)
     $max_pageentrys = 10;	// default setting, if not yet set
     else
     $max_pageentrys = $_SESSION["config"]->get_MaxListEntries();


for ($i = 0; $i < sizeof($temp_array); $i++) {
  // if value is predifined, look up description in hash_table
  if (substr($temp_array[$i],0,1) == "#") {
    $attr = substr($temp_array[$i],1);
    $attr_array[$i] = $attr;
    $desc_array[] = $hash_table[$attr];
  }
  // if not predefined, the attribute is seperated by a ":" from description
  else {
    $attr = explode(":", $temp_array[$i]);
    $attr_array[$i] = $attr[0];
    $desc_array[$i] = $attr[1];
  }
}

$sortattrib = $_GET["sortattrib"];
if (!$sortattrib)
     $sortattrib = strtolower($attr_array[0]);


// generate search filter for sort links
$searchfilter = "";
for ($k = 0; $k < sizeof($desc_array); $k++) {
	if ($_POST["filter" . strtolower($attr_array[$k])]) {
		$searchfilter = $searchfilter . "&filter" . 
		  strtolower($attr_array[$k]) . "=".
		  $_POST["filter" . strtolower($attr_array[$k])];
	}
}

// configure search filter
// Users have the attribute "*"
$filter = "(&(&(|(objectClass=posixAccount) (objectClass=sambaAccount)) (!(uid=*$)))";
for ($k = 0; $k < sizeof($desc_array); $k++) {
  if ($_POST["filter" . strtolower($attr_array[$k])])
    $filter = $filter . "(" . strtolower($attr_array[$k]) . "=" .
      $_POST["filter" . strtolower($attr_array[$k])] . ")";
  else
    $_POST["filter" . strtolower($attr_array[$k])] = "";
}
$filter = $filter . ")";

// read entries only from ldap server if not yet stored in session or if refresh
// button is pressed or if filter is applied
if ($_SESSION["userlist"] && $_GET["norefresh"]) {
  if ($_GET["sort"] == 1)
    usort ($_SESSION["userlist"], "cmp_array");
  $userinfo = $_SESSION["userlist"];
} else {
  $attrs = $attr_array;
  $sr = @ldap_search($_SESSION["ldap"]->server(),
		     $_SESSION["config"]->get_UserSuffix(),
		     $filter, $attrs);
  if ($sr) {
    $userinfo = ldap_get_entries ($_SESSION["ldap"]->server, $sr);
    ldap_free_result ($sr);
    if ($userinfo["count"] == 0)
      StatusMessage ("WARN", "",
		     _("No User(s) found with applied search filter <") .
		     $filter . ">");

    // delete first array entry which is "count"
    array_shift($userinfo);
    $_SESSION["userlist"] = $userinfo;
  }
else 
  StatusMessage("ERROR", 
		_("LDAP Search failed! Please check your preferences."), 
		_("No Groups found!"));
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


  echo "<tr class=\"userlist_head\"><th width=22 height=34></th><th></th>\n";
  // table header
  for ($k = 0; $k < sizeof ($desc_array); $k++) {
    if ($sortattrib == strtolower($attr_array[$k]))
      echo "<th class=\"userlist_activecolumn\">\n";
    else
      echo "<th>\n";
    echo "<a class=\"userlist\" href=\"listusers.php?norefresh=1&sort=1&sortattrib=" . 
      strtolower($attr_array[$k]) . $searchfilter . "\">" . 
      $desc_array[$k] . "</a></th>\n";
  }
  echo "</tr>\n";

  echo "<tr class=\"test\"><th width=22 height=34></th><th>\n";
  echo "<input type=\"submit\" name=\"apply_filter\" value=\"" . _("Apply") . "\">";
  echo "</th>\n";

  // print input boxes for filters
  for ($k = 0; $k < sizeof ($desc_array); $k++) {
    echo "<th>";
    echo ("<input type=\"text\" name=\"filter" . strtolower ($attr_array[$k]) . 
	  "\" value=\"" . $_POST["filter" . strtolower($attr_array[$k])] . "\">");
    echo "</th>";
  }
  echo "</tr>\n";

if ($user_count != 0) {
  // print user list
  $userinfo = array_slice ($userinfo, ($page - 1) * $max_pageentrys, 
			   $max_pageentrys); 
  for ($i = 0; $i < sizeof ($userinfo); $i++) { // ignore last entry in array which is "count"
    echo("<tr class=\"userlist\" onMouseOver=\"user_over(this, '" . $userinfo[$i]["dn"] . "')\"" .
	 " onMouseOut=\"user_out(this, '" . $userinfo[$i]["dn"] . "')\"" .
	 " onClick=\"user_click(this, '" . $userinfo[$i]["dn"] . "')\"" .
	 " onDblClick=parent.frames[1].location.href=\"../account.php?type=user&DN='" . $userinfo[$i]["dn"] . "'\">" .
	 " <td height=22><input onClick=\"user_click(this, '" . $userinfo[$i]["dn"] . "')\" type=\"checkbox\" name=\"" . $userinfo[$i]["dn"] . "\"></td>" .
	 " <td align='center'><a href=\"../account.php?type=user&DN='" . $userinfo[$i]["dn"] . "'\">" . _("Edit") . "</a></td>\n");
    for ($k = 0; $k < sizeof($attr_array); $k++) {
      echo ("<td>\n");
      // print all attribute entries seperated by "; "
      if (sizeof($userinfo[$i][strtolower($attr_array[$k])]) > 0) {
	// delete first array entry which is "count"
	array_shift($userinfo[$i][strtolower($attr_array[$k])]);
	// print all other attributes
	echo implode("; ", $userinfo[$i][strtolower($attr_array[$k])]);
      }
    }
    echo ("</td>");
  }
  echo("</tr>\n");
}
echo ("</table>");

echo ("<br />");
if ($user_count != 0) {
  draw_navigation_bar ($user_count);
  echo ("<br />");
}


echo ("<table align=\"left\" border=\"0\">");
echo ("<tr><td align=\"left\"><input type=\"submit\" name=\"new_user\" value=\"" . _("New User") . "\"></td>");

if ($user_count != 0) {
  echo ("<td align=\"left\"><input type=\"submit\" name=\"del_user\" value=\"" . _("Delete User(s)") . "\"></td></tr>");
}
echo ("</table>\n");
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

  echo ("<table class=\"userlist_navbar\" width=\"100%\" border=\"0\"\n");
  echo ("<tr>");
  echo ("<td class=\"userlist_navbar\"><input type=\"submit\" name=\"refresh\" value=\"" . _("Refresh") . "\">&nbsp;&nbsp;");
  if ($page != 1)
    echo ("<a class=\"userlist\" href=\"listusers.php?norefresh=1&page=" . 
	  ($page - 1) . "&sortattrib=" . $sortattrib . 
	  $searchfilter . "\">&lt;=</a>");
  else
    echo ("&lt;=");
  echo ("&nbsp;");

  if ($page < ($user_count / $max_pageentrys))
    echo ("<a class=\"userlist\" href=\"listusers.php?norefresh=1&page=" . 
	  ($page + 1) . "&sortattrib=" . $sortattrib . $searchfilter . "\">=&gt;</a>");
  else
    echo ("=&gt;");
  echo ("</td>");
  echo ("<td class=\"userlist_navbartext\">");
  echo "&nbsp;" . $user_count . " " .  _("Users found");
  echo ("</td>");


  echo ("<td class=\"userlist_activepage\" align=\"right\">");
  for ($i = 0; $i < ($user_count / $max_pageentrys); $i++) {
    if ($i == $page - 1)
      echo ("&nbsp;" . ($i + 1));
    else
      echo ("&nbsp;<a class=\"userlist\" href=\"listusers.php?norefresh=1&page=" . 
	    ($i + 1) . 
	    "&sortattrib=" . $sortattrib . $searchfilter . 
	    "\">" . ($i + 1) . "</a>");
  }
  echo ("</td></tr></table>");
}


// compare function used for usort-method
// rows are sorted with the first attribute entry of the sort column
// if objects have attributes with multiple values the others are ignored
function cmp_array($a, $b) {
  // sortattrib specifies the sort column
  global $sortattrib;
  global $attr_array;
  // sort by first attribute with name $sortattrib
  if ($a[$sortattrib][0] == $b[$sortattrib][0]) return 0;
  else if ($a[$sortattrib][0] == 
	   max($a[$sortattrib][0], $b[$sortattrib][0])) return 1;
  else return -1;
}

?>
