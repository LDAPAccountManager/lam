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
			else if ($a[$sort][0] == max($a[$sort][0], $b[$sort][0])) return 1;
			else return -1;
		}
		else {
			if ($a[$sort] == $b[$sort]) return 0;
			else if ($a[$sort] == max($a[$sort], $b[$sort])) return 1;
			else return -1;
		}
	}

	// sort and return account list
	usort($info, "cmp_array");
	return $info;
}


/**
* @brief draws a navigation bar to switch between pages
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
		echo("<a href=\"list" . $scope . "s.php?page=" . ($page - 1) . "&amp;sort=" . $sort . $searchFilter . "\">&lt;=</a>\n");
	}
	else {
		echo("&lt;=");
	}
	echo("&nbsp;");

	if ($page < ($count / $max_page_entries)) {
		echo("<a href=\"list" . $scope . "s.php?page=" . ($page + 1) . "&amp;sort=" . $sort . $searchFilter . "\">=&gt;</a>\n");
	}
	else {
		echo("=&gt;</td>");
	}

	echo("<td class=\"" . $scope . "nav-text\">");
	//echo"&nbsp;" . $count . " " .  _("Group(s) found");
	echo"&nbsp;";
	printf($text, $count);
	echo("</td>");

	echo("<td class=\"" . $scope . "nav-activepage\" align=\"right\">");
	for ($i = 0; $i < ($count / $max_page_entries); $i++) {
		if ($i == $page - 1) {
			echo("&nbsp;" . ($i + 1));
		}
		else {
			echo("&nbsp;<a href=\"list" . $scope . "s.php?page=" . ($i + 1) . "&amp;sort=" . $sort . "\">" . ($i + 1) . "</a>\n");
		}
	}
	echo("</td></tr></table>\n");
}



?>