<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003 - 2006  Roland Gruber

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

/** Used to get type information. */
include_once("types.inc");
/** Used to get PDF information. */
include_once("pdfstruct.inc");
/** Used to create PDF files. */
include_once("pdf.inc");


/**
 * Generates the list view.
 *
 * @package lists
 * @author Roland Gruber
 *
 */
class lamList {

	/** Account type */
	var $type;

	/** current page number */
	var $page = 1;

	/** list of LDAP attributes */
	var $attrArray = array();

	/** list of attribute descriptions */
	var $descArray = array();

	/** maximum count of entries per page */
	var $maxPageEntries = 10;

	/** sort column name */
	var $sortColumn;

	/** LDAP suffix */
	var $suffix;

	/** refresh page switch */
	var $refresh = true;

	/** LDAP entries */
	var $entries;

	/** filter string to include in URL */
	var $filterText;

	/** list of possible LDAP suffixes(organizational units) */
	var $possibleSuffixes;

	/** list of account specific labels */
	var $labels;

	/**
	 * Constructor
	 *
	 * @param string $type account type
	 * @return lamList list object
	 */
	function lamList($type) {
		$this->type = $type;
		$this->labels = array(
			'nav' => _("%s object(s) found"),
			'error_noneFound' => _("No objects found!"),
			'newEntry' => _("New object"),
			'deleteEntry' => _("Delete object"),
			'createPDF' => _("Create PDF for selected object(s)"),
			'createPDFAll' => _("Create PDF for all objects"));
	}

	/**
	 * Prints the HTML code to display the list view.
	 */
	function showPage() {
		// do POST actions
		$this->listDoPost();
		// get some parameters
		$this->listGetParams();
		// print HTML head
		$this->listPrintHeader();
		// refresh data if needed
		if ($this->refresh) $this->listRefreshData();
		// filter entries
		$filteredEntries = $this->listFilterAccounts();
		// sort rows by sort column
		if ($filteredEntries) {
			$filteredEntries = $this->listSort($filteredEntries);
		}
		// show form
		echo ("<form action=\"list.php?type=" . $this->type . "&amp;norefresh=true\" method=\"post\">\n");
		// draw account list if accounts were found
		if (sizeof($filteredEntries) > 0) {
			// buttons
			$this->listPrintButtons(false);
			echo ("<hr style=\"background-color: #999999;\">\n");
			// navigation bar
			$this->listDrawNavigationBar(sizeof($filteredEntries));
			echo ("<br>\n");
			// account table head
			$this->listPrintTableHeader();
			// account table body
			$this->listPrintTableBody($filteredEntries);
			echo ("<br>\n");
			// navigation bar
			$this->listDrawNavigationBar(sizeof($filteredEntries));
			echo ("<br>\n");
			// other options
			$this->listPrintAdditionalOptions();
			// PDF bar
			$this->listPrintPDFButtons();
		}
		else {
			// buttons
			$this->listPrintButtons(true);
			echo ("<hr>\n");
			// account table head
			$this->listPrintTableHeader();
			echo "</table><br>\n";
			// other options
			$this->listPrintAdditionalOptions();
		}
		echo ("</form>\n");
		echo "</body></html>\n";
	}

	/**
	* Builds the regular expressions from the filter values.
	*
	* @return array filter data array($attribute => array('regex' => $reg, 'original' => $orig))
	* $reg is the regular expression to use, $orig the user's unmodified input string
	*/
	function listBuildFilter() {
		$filter = array();
		// build filter array
		for ($i = 0; $i < sizeof($this->attrArray); $i++) {
			$foundFilter = null;
			if (isset($_GET["filter" . strtolower($this->attrArray[$i])])) {
				$foundFilter = $_GET["filter" . strtolower($this->attrArray[$i])];
			}
			if (isset($_POST["filter" . strtolower($this->attrArray[$i])])) {
				$foundFilter = $_POST["filter" . strtolower($this->attrArray[$i])];
			}
			if (isset($foundFilter) && eregi('^([0-9a-z _\\*\\$\\.-])+$', $foundFilter)) {
				$filter[$this->attrArray[$i]]['original'] = $foundFilter;
				$filter[$this->attrArray[$i]]['regex'] = $foundFilter;
				// replace special characters
				$filter[$this->attrArray[$i]]['regex'] = str_replace('.', '\\.', $filter[$this->attrArray[$i]]['regex']);
				$filter[$this->attrArray[$i]]['regex'] = str_replace("*", "(.)*", $filter[$this->attrArray[$i]]['regex']);
				$filter[$this->attrArray[$i]]['regex'] = str_replace('$', '[$]', $filter[$this->attrArray[$i]]['regex']);
				// add string begin and end
				$filter[$this->attrArray[$i]]['regex'] = "^" . $filter[$this->attrArray[$i]]['regex'] . "$";
			}
		}
		// save filter string
		$filterAttributes = array_keys($filter);
		$searchFilter = array();
		for ($i = 0; $i < sizeof($filterAttributes); $i++) {
			$searchFilter[] = "filter" . $filterAttributes[$i] . "=" . $filter[$filterAttributes[$i]]['original'];
		}
		if (sizeof($searchFilter) > 0) {
			$searchFilter = "&amp;" . implode("&amp;", $searchFilter);
		}
		else {
			$searchFilter = "";
		}
		$this->filterText = $searchFilter;
		return $filter;
	}


	/**
	* Removes all entries which do not fit to the filter.
	*
	* @return array filtered list of accounts
	*/
	function listFilterAccounts() {
		$entries = array();
		$filter = $this->listBuildFilter();
		$attributes = array_keys($filter);
		for ($r = 0; $r < sizeof($this->entries); $r++) {
			$skip = false;
			for ($a = 0; $a < sizeof($attributes); $a++) {
				// check if filter fits
				$found = false;
				for ($i = 0; $i < sizeof($this->entries[$r][$attributes[$a]]); $i++) {
					if (eregi($filter[$attributes[$a]]['regex'], $this->entries[$r][$attributes[$a]][$i])) {
						$found = true;
						break;
					}
				}
				if (!$found) {
					$skip = true;
					break;
				}
			}
			if (!$skip) {
				$entries[] = &$this->entries[$r];
			}
		}
		if (sizeof($entries) == 0) StatusMessage("WARN", $this->labels['error_noneFound']);
		return $entries;
	}


	/**
	* Sorts an account list by a given attribute
	*
	* @param array $info the account list
	* @return array sorted account list
	*/
	function listSort(&$info) {
		if (!is_array($this->attrArray)) return $info;
		if (!is_string($this->sortColumn)) return $info;
		// sort and return account list
		usort($info, array($this, "cmp_array"));
		return $info;
	}


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
	function cmp_array(&$a, &$b) {
		// sort specifies the sort column
		$sort = $this->sortColumn;
		$attr_array = $this->attrArray;
		// sort by first column if no attribute is given
		if (!$sort) $sort = strtolower($attr_array[0]);
		if ($sort != "dn") {
			// sort by first attribute with name $sort
			return strnatcasecmp($a[$sort][0], $b[$sort][0]);
		}
		else {
			return strnatcasecmp($a[$sort], $b[$sort]);
		}
	}

	/**
	* Draws a navigation bar to switch between pages
	*
	* @param integer $count number of account entries
	*/
	function listDrawNavigationBar($count) {

		echo("<table class=\"" . $this->type . "nav\" width=\"100%\" border=\"0\">\n");
		echo("<tr>\n");
		echo("<td><input type=\"submit\" name=\"refresh\" value=\"" . _("Refresh") . "\">&nbsp;&nbsp;");
		if ($this->page != 1) {
			echo("<a href=\"list.php?type=" . $this->type . "&amp;norefresh=true&amp;page=" . ($this->page - 1) . "&amp;sort=" . $this->sortColumn . $this->filterText . "\">&lt;=</a>\n");
		}
		else {
			echo("&lt;=");
		}
		echo("&nbsp;");

		if ($this->page < ($count / $this->maxPageEntries)) {
			echo("<a href=\"list.php?type=" . $this->type . "&amp;norefresh=true&amp;page=" . ($this->page + 1) . "&amp;sort=" . $this->sortColumn . $this->filterText . "\">=&gt;</a>\n");
		}
		else {
			echo("=&gt;</td>");
		}

		echo("<td class=\"" . $this->type . "nav-text\">");
		echo"&nbsp;";
		printf($this->labels['nav'], $count);
		echo("</td>");

		echo("<td class=\"" . $this->type . "nav-activepage\" align=\"right\">");
		for ($i = 0; $i < ($count / $this->maxPageEntries); $i++) {
			if ($i == $this->page - 1) {
				echo("&nbsp;" . ($i + 1));
			}
			else {
				echo("&nbsp;<a href=\"list.php?type=" . $this->type . "&amp;norefresh=true&amp;page=" . ($i + 1) . "&amp;sort=" . $this->sortColumn . "\">" . ($i + 1) . "</a>\n");
			}
		}
		echo("</td></tr></table>\n");
	}

	/**
	* Prints the attribute and filter row at the account table head
	*/
	function listPrintTableHeader() {
		// print table header
		echo "<table rules=\"all\" class=\"" . $this->type . "list\" width=\"100%\">\n";
		echo "<tr class=\"" . $this->type . "list-head\">\n<th width=22 height=34></th>\n<th></th>\n";
		// table header
		for ($k = 0; $k < sizeof($this->descArray); $k++) {
			if (strtolower($this->attrArray[$k]) == $this->sortColumn) {
				echo "<th class=\"" . $this->type . "list-sort\"><a href=\"list.php?type=" . $this->type . "&amp;".
					"sort=" . strtolower($this->attrArray[$k]) . $this->filterText . "&amp;norefresh=y" . "\">" . $this->descArray[$k] . "</a></th>\n";
			}
			else echo "<th><a href=\"list.php?type=" . $this->type . "&amp;".
				"sort=" . strtolower($this->attrArray[$k]) . $this->filterText . "&amp;norefresh=y" . "\">" . $this->descArray[$k] . "</a></th>\n";
		}
		echo "</tr>\n";

		// print filter row
		echo "<tr align=\"center\" class=\"" . $this->type . "list\">\n";
		echo "<td width=22 height=34>";
			// help link
			echo "<a href=\"../help.php?HelpNumber=250\" target=\"lamhelp\">";
			echo "<img src=\"../../graphics/help.png\" alt=\"" . _('Help') . "\" title=\"" . _('Help') . "\">";
			echo "</a>\n";
		echo "</td>\n";
		echo "<td>";
		echo "<input type=\"submit\" name=\"apply_filter\" value=\"" . _("Filter") . "\">";
		echo "</td>\n";
		// print input boxes for filters
		for ($k = 0; $k < sizeof ($this->descArray); $k++) {
			$value = "";
			if (isset($_GET["filter" . strtolower($this->attrArray[$k])])) {
				$value = " value=\"" . $_GET["filter" . strtolower($this->attrArray[$k])] . "\"";
			}
			if (isset($_POST["filter" . strtolower($this->attrArray[$k])])) {
				$value = " value=\"" . $_POST["filter" . strtolower($this->attrArray[$k])] . "\"";
			}
			echo "<td>";
			echo ("<input type=\"text\" size=15 name=\"filter" . strtolower ($this->attrArray[$k]) ."\"" . $value . ">");
			echo "</td>\n";
		}
		echo "</tr>\n";
	}

	/**
	* Prints the entry list
	*
	* @param array $info entries
	*/
	function listPrintTableBody(&$info) {
		// calculate which rows to show
		$table_begin = ($this->page - 1) * $this->maxPageEntries;
		if (($this->page * $this->maxPageEntries) > sizeof($info)) $table_end = sizeof($info);
		else $table_end = ($this->page * $this->maxPageEntries);
		// print account list
		for ($i = $table_begin; $i < $table_end; $i++) {
			echo("<tr class=\"" . $this->type . "list\" onMouseOver=\"list_over(this, '" . $info[$i]['LAM_ID'] . "', '" . $this->type . "')\"\n" .
				" onMouseOut=\"list_out(this, '" . $info[$i]['LAM_ID'] . "', '" . $this->type . "')\"\n" .
				" onClick=\"list_click(this, '" . $info[$i]['LAM_ID'] . "', '" . $this->type . "')\"\n" .
				" onDblClick=\"parent.frames[1].location.href='../account/edit.php?type=" . $this->type . "&amp;DN=" . $info[$i]['dn'] . "'\">\n");
			if (isset($_GET['selectall'])) {
				echo " <td height=22 align=\"center\"><input onClick=\"list_click(this, '" . $info[$i]['LAM_ID'] . "', '" . $this->type . "')\"" .
					" type=\"checkbox\" checked name=\"" . $info[$i]['LAM_ID'] . "\"></td>\n";
			}
			else {
				echo " <td height=22 align=\"center\"><input onClick=\"list_click(this, '" . $info[$i]['LAM_ID'] . "', '" . $this->type . "')\"" .
					" type=\"checkbox\" name=\"" . $info[$i]['LAM_ID'] . "\"></td>\n";
			}
			echo " <td align='center'>";
				$this->listPrintToolLinks($info[$i]);
			echo "</td>\n";
			for ($k = 0; $k < sizeof($this->attrArray); $k++) {
				echo ("<td>");
				$attrName = strtolower($this->attrArray[$k]);
				$this->listPrintTableCellContent($info[$i], $attrName);
				echo ("</td>\n");
			}
			echo("</tr>\n");
		}
		// display select all link
		$colspan = sizeof($this->attrArray) + 1;
		echo "<tr class=\"" . $this->type . "list\">\n";
		echo "<td align=\"center\"><img src=\"../../graphics/select.png\" alt=\"select all\"></td>\n";
		echo "<td colspan=$colspan>&nbsp;<a href=\"list.php?type=" . $this->type . "&amp;norefresh=y&amp;page=" . $this->page .
			"&amp;sort=" . $this->sortColumn . $this->filterText . "&amp;selectall=yes\">" .
			"<font color=\"black\"><b>" . _("Select all") . "</b></font></a></td>\n";
		echo "</tr>\n";
		echo ("</table>");
	}
	
	/**
	 * Prints the tool image links (e.g. edit and delete) for each account.
	 * 
	 * $account array LDAP attributes
	 */
	function listPrintToolLinks($account) {
		// edit image
		echo "<a href=\"../account/edit.php?type=" . $this->type . "&amp;DN='" . $account['dn'] . "'\">";
		echo "<img src=\"../../graphics/edit.png\" alt=\"" . _("Edit") . "\" title=\"" . _("Edit") . "\">";
		echo "</a>";
		// delete image
		echo "<a href=\"deletelink.php?type=" . $this->type . "&amp;DN='" . $account['dn'] . "'\">";
		echo "<img src=\"../../graphics/delete.png\" alt=\"" . _("Delete") . "\" title=\"" . _("Delete") . "\">";
		echo "</a>";
	}
	
	/**
	 * Prints the content of a cell in the account list for a given LDAP entry and attribute.
	 *
	 * @param array $entry LDAP attributes
	 * @param string $attribute attribute name
	 */
	function listPrintTableCellContent(&$entry, &$attribute) {
		// print all attribute entries seperated by "; "
		if (isset($entry[$attribute]) && sizeof($entry[$attribute]) > 0) {
			// delete "count" entry
			unset($entry[$attribute]['count']);
			if (is_array($entry[$attribute])) {
				// sort array
				sort($entry[$attribute]);
				echo htmlspecialchars(implode("; ", $entry[$attribute]), ENT_QUOTES, "UTF-8");
			}
			else {
				echo htmlspecialchars($entry[$attribute], ENT_QUOTES, "UTF-8");
			}
		}		
	}

	/**
	* Manages all POST actions (e.g. button pressed) for the account lists.
	*/
	function listDoPost() {
		// check if button was pressed and if we have to add/delete an account
		if (isset($_POST['new']) || isset($_POST['del']) || isset($_POST['pdf']) || isset($_POST['pdf_all'])){
			// add new account
			if (isset($_POST['new'])){
				metaRefresh("../account/edit.php?type=" . $this->type);
				exit;
			}
			// delete account(s)
			elseif (isset($_POST['del'])){
				// search for checkboxes
				$accounts = array_keys($_POST, "on");
				// skip trans_primary box
				for ($i = 0; $i < sizeof($accounts); $i++) {
					if ($accounts[$i] === 'trans_primary') {
						unset($accounts[$i]);
						$accounts = array_values($accounts);
						break;
					}
				}
				// build DN list
				$_SESSION['delete_dn'] = array();
				for ($i = 0; $i < sizeof($accounts); $i++) {
					$_SESSION['delete_dn'][] = $this->entries[$accounts[$i]]['dn'];
				}
				if (sizeof($accounts) > 0) {
					metaRefresh("../delete.php?type=" . $this->type);
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
					if (!isset($this->entries[$accounts[$i]]['dn'])) continue;
					$_SESSION["accountPDF-$i"] = new accountContainer($this->type, "accountPDF-$i");
					$_SESSION["accountPDF-$i"]->load_account($this->entries[$accounts[$i]]['dn']);
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
				for ($i = 0; $i < sizeof($this->entries); $i++) {
					$_SESSION["accountPDF-$i"] = new accountContainer($this->type, "accountPDF-$i");
					$_SESSION["accountPDF-$i"]->load_account($this->entries[$i]['dn']);
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
	* Prints a combobox with possible sub-DNs.
	*/
	function listShowOUSelection() {
		if (sizeof($this->possibleSuffixes) > 1) {
			echo ("<b>" . _("Suffix") . ": </b>");
			echo ("<select class=\"" . $this->type . "\" size=1 name=\"suffix\" onchange=\"listOUchanged()\">\n");
			for ($i = 0; $i < sizeof($this->possibleSuffixes); $i++) {
				if ($this->suffix == $this->possibleSuffixes[$i]) {
					echo ("<option selected>" . $this->possibleSuffixes[$i] . "</option>\n");
				}
				else echo("<option>" . $this->possibleSuffixes[$i] . "</option>\n");
			}
			echo ("</select>\n");
			echo ("<input class=\"" . $this->type . "\" type=\"submit\" name=\"refresh\" value=\"" . _("Change suffix") . "\">");
		}
	}

	/**
	 * Prints the create, delete and PDF buttons.
	 *
	 * @param boolean $createOnly true if only the create button should be displayed
	 */
	function listPrintButtons($createOnly) {
		echo "<table border=0 width=\"100%\">\n";
		echo "<tr>\n";
		echo "<td align=\"left\">\n";
		// add/delete/PDF buttons
		echo ("<input class=\"" . $this->type . "\" type=\"submit\" name=\"new\" value=\"" . $this->labels['newEntry'] . "\">\n");
		if (!$createOnly) {
			echo ("<input class=\"" . $this->type . "\" type=\"submit\" name=\"del\" value=\"" . $this->labels['deleteEntry'] . "\">\n");
		}
		echo "</td>\n";
		echo "<td align=\"right\">\n";
			$this->listShowOUSelection();
		echo "</td>\n";
		echo "</tr>\n";
		echo "</table>\n";
	}
	
	/**
	 * Prints the PDF button bar.
	 */
	function listPrintPDFButtons() {
		echo "<fieldset class=\"" . $this->type . "edit\"><legend><b>PDF</b></legend>\n";
		echo ("<b>" . _('PDF structure') . ":</b>&nbsp;&nbsp;<select name=\"pdf_structure\">\n");
		$pdf_structures = getPDFStructureDefinitions($this->type);
		foreach($pdf_structures as $pdf_structure) {
			echo "<option " . (($pdf_structure == 'default') ? " selected" : "") . ">" . $pdf_structure . "</option>";
		}
		echo "</select>&nbsp;&nbsp;&nbsp;&nbsp;\n";
		echo ("<input type=\"submit\" name=\"pdf\" value=\"" . $this->labels['createPDF'] . "\">\n");
		echo "&nbsp;";
		echo ("<input type=\"submit\" name=\"pdf_all\" value=\"" . $this->labels['createPDFAll'] . "\">\n");
		echo "</fieldset>";
	}

	/**
	 * Prints the HTML head.
	 */
	function listPrintHeader() {
		echo $_SESSION['header'];
		echo "<title>Account list</title>\n";
		echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
		echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/type_" . $this->type . ".css\">\n";
		echo "</head><body>\n";
		$this->listPrintJavaScript();
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
		
		// OU selection changed
		echo "function listOUchanged() {\n";
			echo "selectOU = document.getElementsByName('suffix')[0];\n";
			echo "location.href='list.php?type=" . $this->type . "&suffix=' + selectOU.options[selectOU.selectedIndex].value;\n";
		echo "}\n";
		
		echo "//-->\n";
		echo "</script>\n";
	}

	/**
	* Returns an hash array containing with all attributes to be shown and their descriptions.
	* Format: array(attribute => description)
	*
	* @return array attribute list
	*/
	function listGetAttributeDescriptionList() {
		$ret = array();
		$attr_string = $_SESSION["config"]->get_listAttributes($this->type);
		$temp_array = explode(";", $attr_string);
		$hash_table = getListAttributeDescriptions($this->type);
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

	/**
	 * Sets some internal parameters.
	 */
	function listGetParams() {
		// get current page
		if (isset($_GET["page"])) $this->page = $_GET["page"];
		else $this->page = 1;
		// generate attribute-description table
		$temp_array = $this->listGetAttributeDescriptionList();
		$this->attrArray = array_keys($temp_array);	// list of LDAP attributes to show
		$this->descArray = array_values($temp_array);	// list of descriptions for the attributes
		// get maximum count of entries shown on one page
		if ($_SESSION["config"]->get_MaxListEntries() <= 0)
			$this->maxPageEntries = 10;	// default setting, if not yet set
		else
			$this->maxPageEntries = $_SESSION["config"]->get_MaxListEntries();
		// get sorting column
		if (isset($_GET["sort"])) $this->sortColumn = $_GET["sort"];
		else $this->sortColumn = strtolower($this->attrArray[0]);
		// check search suffix
		if (isset($_POST['suffix'])) $this->suffix = $_POST['suffix'];  // new suffix selected via combobox
		elseif (isset($_GET['suffix'])) $this->suffix = $_GET['suffix'];  // new suffix selected via combobox
		elseif (!$this->suffix) $this->suffix = $_SESSION["config"]->get_Suffix($this->type);  // default suffix
		// check if LDAP data should be refreshed
		$this->refresh = true;
		if (isset($_GET['norefresh'])) $this->refresh = false;
		if (isset($_POST['refresh'])) $this->refresh = true;
	}

	/**
	 * Rereads the entries from LDAP.
	 */
	function listRefreshData() {
		// configure search filter
		$module_filter = get_ldap_filter($this->type);  // basic filter is provided by modules
		$filter = "(&" . $module_filter  . ")";
		$attrs = $this->attrArray;
		$sr = @ldap_search($_SESSION["ldap"]->server(), $this->suffix, $filter, $attrs);
		if (ldap_errno($_SESSION["ldap"]->server()) == 4) {
			StatusMessage("WARN", _("LDAP sizelimit exceeded, not all entries are shown."), _("See README.openldap.txt to solve this problem."));
		}
		if ($sr) {
			$info = ldap_get_entries($_SESSION["ldap"]->server, $sr);
			ldap_free_result($sr);
			// delete first array entry which is "count"
			unset($info['count']);
			// save position in original $info
			for ($i = 0; $i < sizeof($info); $i++) {
				$info[$i]['LAM_ID'] = $i;
				if (isset($info[$i]['count'])) unset($info[$i]['count']);
			}
			// save results
			$this->entries = $info;
		}
		else {
			$this->entries = array();
			StatusMessage("ERROR", _("LDAP Search failed! Please check your preferences."));
		}
		// generate list of possible suffixes
		$this->possibleSuffixes = $_SESSION['ldap']->search_units($_SESSION["config"]->get_Suffix($this->type));
	}
	
	/**
	 * Prints additional option fields for specific object types.
	 */
	function listPrintAdditionalOptions() {
		// may be used by subclasses
	}

}

?>