<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003 - 2004  Roland Gruber

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
* This file includes functions to manage the list views.
*
* @package lists
* @author Roland Gruber
*/

/**
* Builds the regular expressions from the filter values.
*
* @param array $post HTTP Post values
* @param array $attributes list of displayed attributes
* @return array filter data array($attribute => array('regex' => $reg, 'original' => $orig))
* $reg is the regular expression to use, $orig the user's unmodified input string
*/
function listBuildFilter($post, $attributes) {
	$filter = array();
	for ($i = 0; $i < sizeof($attributes); $i++) {
		if (eregi('^([0-9a-z_\\*\\$])+$', $_POST["filter" . strtolower($attributes[$i])])) {
			$filter[$attributes[$i]]['original'] = $_POST["filter" . strtolower($attributes[$i])];
			$filter[$attributes[$i]]['regex'] = $_POST["filter" . strtolower($attributes[$i])];
			// replace special characters
			$filter[$attributes[$i]]['regex'] = str_replace("*", "(.)*", $filter[$attributes[$i]]['regex']);
			$filter[$attributes[$i]]['regex'] = str_replace('$', '[$]', $filter[$attributes[$i]]['regex']);
			// add string begin and end
			$filter[$attributes[$i]]['regex'] = "^" . $filter[$attributes[$i]]['regex'] . "$";
		}
	}
	return $filter;
}


/**
* Removes all entries which do not fit to the filter.
*
* @param array $entries list of accounts
* @param array $filter attribute filter
* @return array filtered list of accounts
*/
function listFilterAccounts($entries, $filter) {
	$attributes = array_keys($filter);
	for ($r = 0; $r < sizeof($entries); $r++) {
		for ($a = 0; $a < sizeof($attributes); $a++) {
			// check if filter fits
			$found = false;
			for ($i = 0; $i < sizeof($entries[$r][$attributes[$a]]); $i++) {
				if (eregi($filter[$attributes[$a]]['regex'], $entries[$r][$attributes[$a]][$i])) {
					$found = true;
					break;
				}
			}
			if (!$found) {
				// remove account and reindex array
				unset($entries[$r]);
				$entries = array_values($entries);
				$r--;
				break;
			}
		}
	}
	return $entries;
}


/**
* Sorts an account list by a given attribute
*
* @param string $sort the attribute by which to sort
* @param array $attr_array array of displayed attributes
* @param array $info the account list
* @return array sorted account list
*/
function listSort($sort, $attr_array, $info) {

	/**
	* Compare function used for usort-method
	*
	* Rows are sorted with the first attribute entry of the sort column.
	* If objects have attributes with multiple values only the first is used for sorting.
	*
	* @param array $a first row which is compared
	* @param array $b second row which is compared
	* @return integer 0 if both are equal, 1 if $a is greater, -1 if $b is greater
	*/
	function cmp_array($a, $b) {
		// sort specifies the sort column
		global $sort;
		global $attr_array;
		// sort by first column if no attribute is given
		if (!$sort) $sort = strtolower($attr_array[0]);
		if ($sort != "dn") {
			// sort by first attribute with name $sort
			if ($a[$sort][0] == $b[$sort][0]) return 0;
			elseif ($a[$sort][0] == max($a[$sort][0], $b[$sort][0])) return 1;
			else return -1;
		}
		else {
			if ($a[$sort] == $b[$sort]) return 0;
			elseif ($a[$sort] == max($a[$sort], $b[$sort])) return 1;
			else return -1;
		}
	}

	if (!is_array($attr_array)) return $info;
	if (!is_string($sort)) return $info;
	// sort and return account list
	usort($info, "cmp_array");
	return $info;
}


/**
* Draws a navigation bar to switch between pages
*
* @param integer $count number of account entries
* @param integer $max_page_entries maximum number of account per page
* @param integer $page current page number
* @param string $sort sort attribute
* @param string $searchFilter LDAP search filter
* @param string $scope account type (user/group/host/domain)
* @param string $text string including the number of accounts
*/
function listDrawNavigationBar($count, $max_page_entries, $page, $sort, $searchFilter, $scope, $text) {

	echo("<table class=\"" . $scope . "nav\" width=\"100%\" border=\"0\">\n");
	echo("<tr>\n");
	echo("<td><input type=\"submit\" name=\"refresh\" value=\"" . _("Refresh") . "\">&nbsp;&nbsp;");
	if ($page != 1) {
		echo("<a href=\"list" . $scope . "s.php?norefresh=true&amp;page=" . ($page - 1) . "&amp;sort=" . $sort . $searchFilter . "\">&lt;=</a>\n");
	}
	else {
		echo("&lt;=");
	}
	echo("&nbsp;");

	if ($page < ($count / $max_page_entries)) {
		echo("<a href=\"list" . $scope . "s.php?norefresh=true&amp;page=" . ($page + 1) . "&amp;sort=" . $sort . $searchFilter . "\">=&gt;</a>\n");
	}
	else {
		echo("=&gt;</td>");
	}

	echo("<td class=\"" . $scope . "nav-text\">");
	echo"&nbsp;";
	printf($text, $count);
	echo("</td>");

	echo("<td class=\"" . $scope . "nav-activepage\" align=\"right\">");
	for ($i = 0; $i < ($count / $max_page_entries); $i++) {
		if ($i == $page - 1) {
			echo("&nbsp;" . ($i + 1));
		}
		else {
			echo("&nbsp;<a href=\"list" . $scope . "s.php?norefresh=true&amp;page=" . ($i + 1) . "&amp;sort=" . $sort . "\">" . ($i + 1) . "</a>\n");
		}
	}
	echo("</td></tr></table>\n");
}

/**
* Prints the attribute and filter row at the account table head
*
* @param string $scope account type (user, group, host)
* @param string $searchFilter search filter for hyperlinks
* @param array $desc_array list of attribute descriptions
* @param array $attr_array list of attribute names
* @param array $_POST HTTP-POST values
* @param string $sort sort attribute
*/
function listPrintTableHeader($scope, $searchFilter, $desc_array, $attr_array, $_POST, $sort) {
	// print table header
	echo "<table rules=\"all\" class=\"" . $scope . "list\" width=\"100%\">\n";
	echo "<tr class=\"" . $scope . "list-head\">\n<th width=22 height=34></th>\n<th></th>\n";
	// table header
	for ($k = 0; $k < sizeof($desc_array); $k++) {
		if (strtolower($attr_array[$k]) == $sort) {
			echo "<th class=\"" . $scope . "list-sort\"><a href=\"list" . $scope . "s.php?".
				"sort=" . strtolower($attr_array[$k]) . $searchFilter . "&amp;norefresh=y" . "\">" . $desc_array[$k] . "</a></th>\n";
		}
		else echo "<th><a href=\"list" . $scope . "s.php?".
			"sort=" . strtolower($attr_array[$k]) . $searchFilter . "&amp;norefresh=y" . "\">" . $desc_array[$k] . "</a></th>\n";
	}
	echo "</tr>\n";
	
	// print filter row
	echo "<tr align=\"center\" class=\"" . $scope . "list\">\n<td width=22 height=34></td>\n<td>";
	echo "<input type=\"submit\" name=\"apply_filter\" value=\"" . _("Filter") . "\">";
	echo "</td>\n";
	// print input boxes for filters
	for ($k = 0; $k < sizeof ($desc_array); $k++) {
		echo "<td>";
		echo ("<input type=\"text\" size=15 name=\"filter" . strtolower ($attr_array[$k]) .
		"\" value=\"" . $_POST["filter" . strtolower($attr_array[$k])] . "\"" .
		"title=\"" . _("Here you can input small filters (e.g. 'value' or 'v*').") . "\"" .
		">");
		echo "</td>\n";
	}
	echo "</tr>\n";
}

/**
* Manages all POST actions (e.g. button pressed) for the account lists.
*
* @param string $scope account type
*/
function listDoPost($scope) {
	// check if button was pressed and if we have to add/delete an account
	if ($_POST['new'] || $_POST['del'] || $_POST['pdf'] || $_POST['pdf_all']){
		// add new account
		if ($_POST['new']){
			metaRefresh("../account/edit.php?type=" . $scope);
			exit;
		}
		// delete account(s)
		elseif ($_POST['del']){
			// search for checkboxes
			$accounts = array_keys($_POST, "on");
			$_SESSION['delete_dn'] = $accounts;
			if (sizeof($accounts) > 0) {
				metaRefresh("../delete.php?type=" . $scope);
				exit;
			}
		}
		// PDF for selected accounts
		elseif ($_POST['pdf']){
			$pdf_structure = $_POST['pdf_structure'];
			// search for checkboxes
			$accounts = array_keys($_POST, "on");
			$list = array();
			// load accounts from LDAP
			for ($i = 0; $i < sizeof($accounts); $i++) {
				$_SESSION["accountPDF-$i"] = new accountContainer($scope, "accountPDF-$i");
				$_SESSION["accountPDF-$i"]->load_account($accounts[$i]);
				$list[$i] = $_SESSION["accountPDF-$i"];
			}
			if (sizeof($list) > 0) {
				createModulePDF($list,$pdf_structure);
				exit;
			}
		}
		// PDF for all accounts
		elseif ($_POST['pdf_all']){
			$list = array();
			for ($i = 0; $i < sizeof($_SESSION[$scope . 'info']); $i++) {
				$_SESSION["accountPDF-$i"] = new accountContainer($scope, "accountPDF-$i");
				$_SESSION["accountPDF-$i"]->load_account($_SESSION[$scope . 'info'][$i]['dn']);
				$list[$i] = $_SESSION["accountPDF-$i"];
			}
			if (sizeof($list) > 0) {
				createModulePDF($list,$_POST['pdf_structure']);
				exit;
			}
		}
	}
}


/**
* Returns the LDAP attribute names and their description for the user list
*
* @return array list of LDAP attributes and descriptions
*/
function listGetAttributeUserArray() {
	return array (
		"uid" => _("User ID"),
		"uidnumber" => _("UID number"),
		"gidnumber" => _("GID number"),
		"cn" => _("Username"),
		"host" => _("Allowed hosts"),
		"givenname" => _("First name"),
		"sn" => _("Last name"),
		"homedirectory" => _("Home directory"),
		"loginshell" => _("Login shell"),
		"mail" => _("E-Mail"),
		"gecos" => _("Description")
		);
}

/**
* Returns the LDAP attribute names and their description for the group list
*
* @return array list of LDAP attributes and descriptions
*/
function listGetAttributeGroupArray() {
	return array (
		"cn" => _("Group name"),
		"gidnumber" => _("GID number"),
		"memberuid" => _("Group members"),
		"member" => _("Group member DNs"),
		"description" => _("Group description")
		);
}

/**
* Returns the LDAP attribute names and their description for the host list
*
* @return array list of LDAP attributes and descriptions
*/
function listGetAttributeHostArray() {
	return array (
		"uid" => _("Host username"),
		"cn" => _("Host name"),
		"rid" => _("RID (Windows UID)"),
		"description" => _("Host description"),
		"uidnumber" => _("UID number"),
		"gidnumber" => _("GID number")
		);
}


?>