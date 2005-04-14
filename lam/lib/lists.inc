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
* @param array $attributes list of displayed attributes
* @return array filter data array($attribute => array('regex' => $reg, 'original' => $orig))
* $reg is the regular expression to use, $orig the user's unmodified input string
*/
function listBuildFilter($attributes) {
	$filter = array();
	for ($i = 0; $i < sizeof($attributes); $i++) {
		if (isset($_POST["filter" . strtolower($attributes[$i])]) && eregi('^([0-9a-z_\\*\\$])+$', $_POST["filter" . strtolower($attributes[$i])])) {
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
		$value = "";
		if (isset($_POST["filter" . strtolower($attr_array[$k])])) {
			$value = " value=\"" . $_POST["filter" . strtolower($attr_array[$k])] . "\"";
		}
		echo "<td>";
		echo ("<input type=\"text\" size=15 name=\"filter" . strtolower ($attr_array[$k]) ."\"" .
		$value . "title=\"" . _("Here you can input small filters (e.g. 'value' or 'v*').") . "\">");
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
	if (isset($_POST['new']) || isset($_POST['del']) || isset($_POST['pdf']) || isset($_POST['pdf_all'])){
		// add new account
		if (isset($_POST['new'])){
			metaRefresh("../account/edit.php?type=" . $scope);
			exit;
		}
		// delete account(s)
		elseif (isset($_POST['del'])){
			// search for checkboxes
			$accounts = array_keys($_POST, "on");
			$_SESSION['delete_dn'] = array();
			for ($i = 0; $i < sizeof($accounts); $i++) {
				$_SESSION['delete_dn'][] = $_SESSION[$scope . 'info'][$accounts[$i]]['dn'];
			}
			if (sizeof($accounts) > 0) {
				metaRefresh("../delete.php?type=" . $scope);
				exit;
			}
		}
		// PDF for selected accounts
		elseif (isset($_POST['pdf'])){
			$pdf_structure = $_POST['pdf_structure'];
			// search for checkboxes
			$accounts = array_keys($_POST, "on");
			$list = array();
			// load accounts from LDAP
			for ($i = 0; $i < sizeof($accounts); $i++) {
				$_SESSION["accountPDF-$i"] = new accountContainer($scope, "accountPDF-$i");
				$_SESSION["accountPDF-$i"]->load_account($_SESSION[$scope . 'info'][$accounts[$i]]['dn']);
				$list[$i] = $_SESSION["accountPDF-$i"];
			}
			if (sizeof($list) > 0) {
				createModulePDF($list,$pdf_structure);
				exit;
			}
		}
		// PDF for all accounts
		elseif (isset($_POST['pdf_all'])){
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
* @param string $scope account type
* @return array list of LDAP attributes and descriptions
*/
function listGetAttributeArray($scope) {
	switch ($scope) {
		case 'user':
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
			break;
		case 'group':
			return array (
				"cn" => _("Group name"),
				"gidnumber" => _("GID number"),
				"memberuid" => _("Group members"),
				"member" => _("Group member DNs"),
				"description" => _("Group description")
				);
			break;
		case 'host':
			return array (
				"uid" => _("Host username"),
				"cn" => _("Host name"),
				"rid" => _("RID (Windows UID)"),
				"description" => _("Host description"),
				"uidnumber" => _("UID number"),
				"gidnumber" => _("GID number")
				);
			break;
		default:
			return array();
			break;
	}
}

/**
* Prints a combobox with possible sub-DNs.
*
* @param array $units list of OUs
* @param string $suffix current LDAP suffix
*/
function listShowOUSelection($units, $suffix) {
	if (sizeof($units) > 1) {
		echo ("<p align=\"left\">\n");
		echo ("<b>" . _("Suffix") . ": </b>");
		echo ("<select size=1 name=\"suffix\">\n");
		for ($i = 0; $i < sizeof($units); $i++) {
			if ($suffix == $units[$i]) echo ("<option selected>" . $units[$i] . "</option>\n");
			else echo("<option>" . $units[$i] . "</option>\n");
		}
		echo ("</select>\n");
		echo ("<input type=\"submit\" name=\"refresh\" value=\"" . _("Change suffix") . "\">");
		echo ("</p>\n");
	}
}

/**
* Prints JavaScript code needed for mouse-over effects.
*/
function listPrintJavaScript() {
	echo "<script type=\"text/javascript\" language=\"javascript\">\n";
	echo "<!--\n";
	// mouseOver function
	echo "function list_over(list, box, scope) {\n";
		echo "cbox = document.getElementsByName(box)[0];\n";
		echo "if (cbox.checked == false) list.setAttribute('class', scope + 'list-over', 0);\n";
	echo "}";
	
	// mouseOut function
	echo "function list_out(list, box, scope) {\n";
		echo "cbox = document.getElementsByName(box)[0];\n";
		echo "if (cbox.checked == false) list.setAttribute('class', scope + 'list', 0);\n";
	echo "}\n";
	
	// onClick function
	echo "function list_click(list, box, scope) {\n";
		echo "cbox = document.getElementsByName(box)[0];\n";
		echo "if (cbox.checked == true) {\n";
			echo "cbox.checked = false;\n";
			echo "list.setAttribute('class', scope + 'list-over', 0);\n";
		echo "}\n";
		echo "else {\n";
			echo "cbox.checked = true;\n";
			echo "list.setAttribute('class', scope + 'list-checked', 0);\n";
		echo "}\n";
	echo "}\n";
	echo "//-->\n";
	echo "</script>\n";
}

/**
* Returns an hash array containing with all attributes to be shown and their descriptions.
* Format: array(attribute => description)
*
* @param string $scope account type
* @return array attribute list
*/
function listGetAttributeDescriptionList($scope) {
	$ret = array();
	$attr_string = $_SESSION["config"]->get_listAttributes($scope);
	$temp_array = explode(";", $attr_string);
	$hash_table = listGetAttributeArray($scope);
	// generate column attributes and descriptions
	for ($i = 0; $i < sizeof($temp_array); $i++) {
		// if value is predifined, look up description in hash_table
		if (substr($temp_array[$i],0,1) == "#") {
			$attr = strtolower(substr($temp_array[$i],1));
			if (isset($hash_table[$attr])) {
				$ret[$attr] = $hash_table[$attr];
			}
			else {
				$ret[$attr] = $attr;
			}
		}
		// if not predefined, the attribute is seperated by a ":" from description
		else {
			$attr = explode(":", $temp_array[$i]);
			if (isset($attr[1])) {
				$ret[$attr[0]] = $attr[1];
			}
			else {
				$ret[$attr[0]] = $attr[0];
			}
		}
	}
	return $ret;
}


?>