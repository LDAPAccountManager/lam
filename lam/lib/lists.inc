<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003 - 2009  Roland Gruber

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
	protected $type;

	/** current page number */
	protected $page = 1;

	/** list of LDAP attributes */
	protected $attrArray = array();

	/** list of attribute descriptions */
	protected $descArray = array();

	/** maximum count of entries per page */
	protected $maxPageEntries = 30;

	/** sort column name */
	protected $sortColumn;
	
	/** sort direction: 1 for ascending, -1 for descending */
	protected $sortDirection = 1;

	/** LDAP suffix */
	protected $suffix;

	/** refresh page switch */
	protected $refresh = true;

	/** LDAP entries */
	protected $entries;

	/** filter string to include in URL */
	protected $filterText;

	/** list of possible LDAP suffixes(organizational units) */
	protected $possibleSuffixes;

	/** list of account specific labels */
	protected $labels;
	
	/** configuration options */
	private $configOptions;

	/** ID for list size config option */
	const LIST_SIZE_OPTION_NAME = "L_SIZE";
	
	/**
	 * Constructor
	 *
	 * @param string $type account type
	 * @return lamList list object
	 */
	public function __construct($type) {
		$this->type = $type;
		$this->labels = array(
			'nav' => _("%s object(s) found"),
			'error_noneFound' => _("No objects found!"),
			'newEntry' => _("New object"),
			'deleteEntry' => _("Delete object"));
		$this->configOptions = $this->listGetAllConfigOptions();
		$this->listReadOptionsFromCookie();
	}
	
	/**
	 * Reads the list options from the cookie value.
	 */
	private function listReadOptionsFromCookie() {
		if (sizeof($this->configOptions) > 0) {
			if (isset($_COOKIE["ListOptions_" . $this->type])) {
				$cookieValue = $_COOKIE["ListOptions_" . $this->type];
				$valueParts = explode(";", $cookieValue);
				$values = array();
				for ($i = 0; $i < sizeof($valueParts); $i++) {
					$key_value = explode('=', $valueParts[$i]);
					if (sizeof($key_value) == 2) {
						$values[$key_value[0]] = $key_value[1];
					}
				}
				for ($i = 0; $i < sizeof($this->configOptions); $i++) {
					if (isset($values[$this->configOptions[$i]->getID()])) {
						$this->configOptions[$i]->setValue($values[$this->configOptions[$i]->getID()]);
					}
				}
				// notify subclasses
				$this->listConfigurationChanged();
			}
		}
	}

	/**
	 * Prints the HTML code to display the list view.
	 */
	public function showPage() {
		if (isset($_GET['openConfig'])) {
			$this->listPrintConfigurationPage();
			return;
		}
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
			echo ("<hr style=\"background-color: #999999;\">\n");
			// buttons
			$this->listPrintButtons(false);
			echo ("<br>\n");
		}
		else {
			// buttons
			$this->listPrintButtons(true);
			echo ("<hr style=\"background-color: #999999;\">\n");
			// navigation bar
			$this->listDrawNavigationBar(sizeof($filteredEntries));
			echo ("<br>\n");
			// account table head
			$this->listPrintTableHeader();
			echo "</table><br>\n";
		}
		$this->listPrintFooter();
	}

	/**
	* Builds the regular expressions from the filter values.
	*
	* @return array filter data array($attribute => array('regex' => $reg, 'original' => $orig))
	* $reg is the regular expression to use, $orig the user's unmodified input string
	*/
	protected function listBuildFilter() {
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
	protected function listFilterAccounts() {
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
	protected function listSort(&$info) {
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
	protected function cmp_array(&$a, &$b) {
		// sort specifies the sort column
		$sort = $this->sortColumn;
		// sort by first column if no attribute is given
		if (!$sort) $sort = strtolower($this->attrArray[0]);
		if ($sort != "dn") {
			// sort by first attribute with name $sort
			return @strnatcasecmp($a[$sort][0], $b[$sort][0]) * $this->sortDirection;
		}
		else {
			return strnatcasecmp($a[$sort], $b[$sort]) * $this->sortDirection;
		}
	}

	/**
	* Draws a navigation bar to switch between pages
	*
	* @param integer $count number of account entries
	*/
	protected function listDrawNavigationBar($count) {

		echo("<table class=\"" . $this->type . "nav\" width=\"100%\" border=\"0\">\n");
		echo("<tr>\n");
		echo("<td><input type=\"submit\" name=\"refresh\" value=\"" . _("Refresh") . "\">&nbsp;&nbsp;");
		if ($this->page != 1) {
			echo("<a href=\"list.php?type=" . $this->type . "&amp;norefresh=true&amp;page=" . ($this->page - 1) .
				"&amp;sort=" . $this->sortColumn . "&amp;sortdirection=" . $this->sortDirection . $this->filterText . "\">" .
				"<img style=\"vertical-align: middle;\" src=\"../../graphics/back.gif\" alt=\"back\"></a>\n");
		}

		if ($this->page < ($count / $this->maxPageEntries)) {
			echo("<a href=\"list.php?type=" . $this->type . "&amp;norefresh=true&amp;page=" . ($this->page + 1) .
				"&amp;sort=" . $this->sortColumn . "&amp;sortdirection=" . $this->sortDirection . $this->filterText . "\">" .
				"<img style=\"vertical-align: middle;\" src=\"../../graphics/forward.gif\" alt=\"forward\"></a>\n");
		}
		echo("</td>");

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
				echo("&nbsp;<a href=\"list.php?type=" . $this->type . "&amp;norefresh=true&amp;page=" . ($i + 1) .
				"&amp;sort=" . $this->sortColumn . "&amp;sortdirection=" . $this->sortDirection . $this->filterText . "\">" . ($i + 1) . "</a>\n");
			}
		}
		echo("</td></tr></table>\n");
	}

	/**
	* Prints the attribute and filter row at the account table head
	*/
	protected function listPrintTableHeader() {
		// print table header
		echo "<table rules=\"all\" class=\"" . $this->type . "list\" width=\"100%\">\n";
		echo "<tr class=\"" . $this->type . "list-head\">\n<th width=22 height=34></th>\n<th></th>\n";
		// table header
		for ($k = 0; $k < sizeof($this->descArray); $k++) {
			if (strtolower($this->attrArray[$k]) == $this->sortColumn) {
				$sortImage = "sort_asc.png";
				if ($this->sortDirection < 0) {
					$sortImage = "sort_desc.png";
				}
				echo "<th class=\"" . $this->type . "list-sort\"><a href=\"list.php?type=" . $this->type . "&amp;".
					"sort=" . strtolower($this->attrArray[$k]) . $this->filterText . "&amp;norefresh=y" . "\">" . $this->descArray[$k] .
					"&nbsp;<img style=\"vertical-align: middle;\" src=\"../../graphics/$sortImage\" alt=\"sort direction\"></a></th>\n";
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
			echo ("<input type=\"text\" size=15 name=\"filter" . strtolower ($this->attrArray[$k]) ."\"" . $value . " onkeypress=\"SubmitForm('apply_filter', event)\">");
			echo "</td>\n";
		}
		echo "</tr>\n";
	}

	/**
	* Prints the entry list
	*
	* @param array $info entries
	*/
	protected function listPrintTableBody(&$info) {
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
				echo " <td align=\"center\"><input onClick=\"list_click(this, '" . $info[$i]['LAM_ID'] . "', '" . $this->type . "')\"" .
					" type=\"checkbox\" checked name=\"" . $info[$i]['LAM_ID'] . "\"></td>\n";
			}
			else {
				echo " <td align=\"center\"><input onClick=\"list_click(this, '" . $info[$i]['LAM_ID'] . "', '" . $this->type . "')\"" .
					" type=\"checkbox\" name=\"" . $info[$i]['LAM_ID'] . "\"></td>\n";
			}
			echo " <td align='center'>";
				$this->listPrintToolLinks($info[$i], $info[$i]['LAM_ID']);
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
	 * @param array $account LDAP attributes
	 * @param String $id account ID
	 */
	private function listPrintToolLinks($account, $id) {
		// edit image
		echo "<a href=\"../account/edit.php?type=" . $this->type . "&amp;DN='" . $account['dn'] . "'\">";
		echo "<img src=\"../../graphics/edit.png\" alt=\"" . _("Edit") . "\" title=\"" . _("Edit") . "\">";
		echo "</a>\n ";
		// delete image
		if (checkIfWriteAccessIsAllowed()) {
			echo "<a href=\"deletelink.php?type=" . $this->type . "&amp;DN='" . $account['dn'] . "'\">";
			echo "<img src=\"../../graphics/delete.png\" alt=\"" . _("Delete") . "\" title=\"" . _("Delete") . "\">";
			echo "</a>\n ";
		}
		// pdf image
		echo "<input type=\"image\" style=\"background:transparent;\" name=\"createPDF_" . $id . "\" src=\"../../graphics/pdf.png\" title=\"" . _('Create PDF file') . "\">\n ";
		// additional tools
		$tools = $this->getAdditionalTools();
		for ($i = 0; $i < sizeof($tools); $i++) {
			echo "<a href=\"" . $tools[$i]->getLinkTarget() . "?type=" . $this->type . "&amp;DN='" . $account['dn'] . "'\">";
			echo "<img src=\"../../graphics/" . $tools[$i]->getImage() . "\" alt=\"" . $tools[$i]->getName() . "\" title=\"" . $tools[$i]->getName() . "\">";
			echo "</a>\n ";
		}
	}
	
	/**
	 * Prints the content of a cell in the account list for a given LDAP entry and attribute.
	 *
	 * @param array $entry LDAP attributes
	 * @param string $attribute attribute name
	 */
	protected function listPrintTableCellContent(&$entry, &$attribute) {
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
	protected function listDoPost() {
		// check if button was pressed and if we have to add/delete an account
		if (isset($_POST['new']) || isset($_POST['del'])){
			if (!checkIfWriteAccessIsAllowed()) {
				die();
			}
			// add new account
			if (isset($_POST['new'])){
				metaRefresh("../account/edit.php?type=" . $this->type . "&amp;suffix=" . $this->suffix);
				exit;
			}
			// delete account(s)
			elseif (isset($_POST['del'])){
				// search for checkboxes
				$accounts = array_keys($_POST, "on");
				// skip option boxes
				$change = false;
				for ($i = 0; $i < sizeof($accounts); $i++) {
					if (!is_numeric($accounts[$i])) {
						unset($accounts[$i]);
						$change = true;
					}
				}
				if ($change) {
					$accounts = array_values($accounts);
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
		}
		// PDF button
		foreach ($_POST as $key => $value) {
			if (strpos($key, 'createPDF_') > -1) {
				$parts = explode("_", $key);
				if (sizeof($parts) == 3) {
					$this->showPDFPage($parts[1]);
					exit;
				}
			}
		}
		// PDF creation Ok
		if (isset($_POST['createPDFok'])) {
			$pdfStruct = $_POST['pdf_structure'];
			$option = $_POST['createFor'];
			// create for clicked account
			if ($option == 'DN') {
				$_SESSION["accountPDF"] = new accountContainer($this->type, "accountPDF");
				$_SESSION["accountPDF"]->load_account($this->entries[$_POST['clickedAccount']]['dn']);
				createModulePDF(array($_SESSION["accountPDF"]),$pdfStruct);
				unset($_SESSION["accountPDF"]);
				exit;
			}
			// create for all selected accounts
			elseif ($option == 'SELECTED') {
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
					createModulePDF($list,$pdfStruct);
					for ($i = 0; $i < sizeof($accounts); $i++) {
						unset($_SESSION["accountPDF-$i"]);
					}
					exit;
				}
			}
			// create for all accounts
			elseif ($option == 'ALL') {
				$list = array();
				for ($i = 0; $i < sizeof($this->entries); $i++) {
					$_SESSION["accountPDF-$i"] = new accountContainer($this->type, "accountPDF-$i");
					$_SESSION["accountPDF-$i"]->load_account($this->entries[$i]['dn']);
					$list[$i] = $_SESSION["accountPDF-$i"];
				}
				if (sizeof($list) > 0) {
					createModulePDF($list,$pdfStruct);
					for ($i = 0; $i < sizeof($this->entries); $i++) {
						// clean session
						unset($_SESSION["accountPDF-$i"]);
					}
					exit;
				}
			}
		}
		// check if back from configuration page
		if (sizeof($this->configOptions) > 0) {
			if (isset($_POST['saveConfigOptions'])) {
				$cookieValue = '';
				for ($i = 0; $i < sizeof($this->configOptions); $i++) {
					$this->configOptions[$i]->fillFromPostData();
					$cookieValue .= $this->configOptions[$i]->getID() . "=" . $this->configOptions[$i]->getValue() . ';';
				}
				// save options as cookie for one year
				setcookie("ListOptions_" . $this->type, $cookieValue, time()+60*60*24*365, "/");
				// notify subclasses
				$this->listConfigurationChanged();
			}
		}
	}
	
	/**
	 * Shows the page where the user may select the PDF options.
	 *
	 * @param String $id account ID
	 */
	private function showPDFPage($id) {
		// search for checkboxes
		$selAccounts = array_keys($_POST, "on");
		if (!in_array($id, $selAccounts)) {
			$selAccounts[] = $id;
		}

		$this->listPrintHeader();

		echo '<h1 align="center">' . _('Create PDF file') . "</h1>\n";
		echo "<form action=\"list.php?type=" . $this->type . "&amp;norefresh=true\" method=\"post\">\n";

		echo '<fieldset class="' . $this->type . 'edit"><br>';
		echo '<table class="' . $this->type . 'edit">';
			echo '<tr>';
				echo '<td>';
					echo _('PDF structure') . " ";
				echo '</td>';
				echo '<td>';
					echo "<select name=\"pdf_structure\">\n";
					$pdf_structures = getPDFStructureDefinitions($this->type);
					foreach($pdf_structures as $pdf_structure) {
						echo "<option " . (($pdf_structure == 'default') ? " selected" : "") . ">" . $pdf_structure . "</option>";
					}
					echo "</select>\n";
				echo '</td>';
			echo '</tr>';
			echo '<tr>';
				echo '<td>';
					echo _('Create for') . " ";
				echo '</td>';
				echo '<td>';
					echo '<select name="createFor">';
					echo '<option value="DN">' . $this->entries[$id]['dn'] . '</option>';
					echo '<option value="SELECTED">' . sprintf(_('All selected accounts (%s)'), sizeof($selAccounts)) . '</option>';
					echo '<option value="ALL">' . sprintf(_('All accounts (%s)'), sizeof($this->entries)) . '</option>';
					echo '</select>';
				echo '</td>';
			echo '</tr>';
			echo '<tr>';
				echo '<td colspan=2>&nbsp;</td>';
			echo '</tr>';
			echo '<tr>';
				echo '<td colspan=2>';
					echo '<input type="submit" name="createPDFok" value="' . _('Ok') . '"> ';
					echo '<input type="submit" name="createPDFcancel" value="' . _('Cancel') . '">';
				echo '</td>';
			echo '</tr>';
		echo '</table>';
		echo '</fieldset>';
		
		// hiden inputs for selected accounts
		for ($i = 0; $i < sizeof($selAccounts); $i++) {
			echo '<input type="hidden" name="' . $selAccounts[$i] . '" value="on">';
		}
		echo '<input type="hidden" name="clickedAccount" value="' . $id . '">';
		
		$this->listPrintFooter();
	}

	/**
	* Prints a combobox with possible sub-DNs.
	*/
	protected function listShowOUSelection() {
		if (sizeof($this->possibleSuffixes) > 1) {
			echo ("<b>" . _("Suffix") . ": </b>");
			echo ("<select class=\"" . $this->type . "\" size=1 name=\"suffix\" onchange=\"listOUchanged('" . $this->type . "')\">\n");
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
	 * Prints the create and delete buttons.
	 *
	 * @param boolean $createOnly true if only the create button should be displayed
	 */
	protected function listPrintButtons($createOnly) {
		echo "<table border=0 width=\"100%\">\n";
		echo "<tr>\n";
		echo "<td align=\"left\">\n";
		if (checkIfWriteAccessIsAllowed()) {
			// add/delete buttons
			echo ("<input class=\"" . $this->type . "\" type=\"submit\" name=\"new\" value=\"" . $this->labels['newEntry'] . "\">\n");
			if (!$createOnly) {
				echo ("<input class=\"" . $this->type . "\" type=\"submit\" name=\"del\" value=\"" . $this->labels['deleteEntry'] . "\">\n");
			}
			echo "&nbsp;&nbsp;&nbsp;";
		}
		$this->listShowOUSelection();
		echo "</td>\n";
		echo "<td align=\"right\">\n";
		echo '<a href="list.php?type=' . $this->type . '&amp;openConfig=1">';
			echo '<img src="../../graphics/tools.png" alt="' . _('Change settings') . '" title="' . _('Change settings') . '">';
			echo '&nbsp;' . _('Change settings');
		echo '</a>';
		echo "</td>\n";
		echo "</tr>\n";
		echo "</table>\n";
	}
	
	/**
	 * Prints the HTML header.
	 */
	protected function listPrintHeader() {
		echo $_SESSION['header'];
		echo "<title>Account list</title>\n";
		echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
		echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/type_" . $this->type . ".css\">\n";
		echo "</head><body>\n";
		echo "<script type=\"text/javascript\" src=\"list.js\"></script>\n";
		echo "<script type=\"text/javascript\" src=\"../wz_tooltip.js\"></script>\n";
	}
	
	/**
	 * Prints the HTML footer.
	 */
	protected function listPrintFooter() {	
		echo ("</form>\n");
		echo "</body></html>\n";		
	}

	/**
	* Returns an hash array containing with all attributes to be shown and their descriptions.
	* Format: array(attribute => description)
	*
	* @return array attribute list
	*/
	private function listGetAttributeDescriptionList() {
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
	protected function listGetParams() {
		// get current page
		if (isset($_GET["page"])) $this->page = $_GET["page"];
		else $this->page = 1;
		// generate attribute-description table
		$temp_array = $this->listGetAttributeDescriptionList();
		$this->attrArray = array_keys($temp_array);	// list of LDAP attributes to show
		$this->descArray = array_values($temp_array);	// list of descriptions for the attributes
		// get sorting column
		if (isset($_GET["sort"])) {
			if ($_GET["sort"] == $this->sortColumn) {
				$this->sortDirection = -$this->sortDirection;
			}
			else {
				$this->sortColumn = $_GET["sort"];
				$this->sortDirection = 1;
			}
		}
		else {
			$this->sortColumn = strtolower($this->attrArray[0]);
			$this->sortDirection = 1;
		}
		// get sort order
		if (isset($_GET['sortdirection'])) {
			$this->sortDirection = $_GET['sortdirection'];
		}
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
	protected function listRefreshData() {
		// configure search filter
		$module_filter = get_ldap_filter($this->type);  // basic filter is provided by modules
		$filter = "(&" . $module_filter  . ")";
		$attrs = $this->attrArray;
		$sr = @ldap_search($_SESSION["ldap"]->server(), escapeDN($this->suffix), $filter, $attrs);
		if (ldap_errno($_SESSION["ldap"]->server()) == 4) {
			StatusMessage("WARN", _("LDAP sizelimit exceeded, not all entries are shown."), _("See README.openldap.txt to solve this problem."));
		}
		if ($sr) {
			$info = ldap_get_entries($_SESSION["ldap"]->server(), $sr);
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
			StatusMessage("ERROR", _("LDAP search failed! Please check your preferences."));
		}
		// generate list of possible suffixes
		$this->possibleSuffixes = $_SESSION['ldap']->search_units($_SESSION["config"]->get_Suffix($this->type));
	}
	
	/**
	 * Returns a list of lamListTool objects to display next to the edit/delete buttons.
	 *
	 * @return lamListTool[] tools
	 */
	protected function getAdditionalTools() {
		return array();
	}
	
	/**
	 * Returns a list of possible configuration options.
	 *
	 * @return array list of lamListOption objects
	 */
	protected function listGetAllConfigOptions() {
		$listSizeOption = new lamSelectListOption(_("Maximum list entries"), array(10, 20, 30, 50, 75, 100), self::LIST_SIZE_OPTION_NAME);
		return array($listSizeOption);
	}

	/**
	 * Prints the list configuration page.
	 */
	protected function listPrintConfigurationPage() {
		$this->listPrintHeader();
		
		echo '<h1 align="center">' . _('Change list settings') . "</h1>\n";
		
		echo "<form action=\"list.php?type=" . $this->type . "&amp;norefresh=true\" method=\"post\">\n";
		echo '<fieldset class="' . $this->type . 'edit"><br>';
		echo "<table class=\"" . $this->type . "edit\" width=\"100%\">\n";
		echo "<tr class=\"" . $this->type . "list\"><td>\n";

		$tabindex = 0;
		for ($i = 0; $i < sizeof($this->configOptions); $i++) {
			parseHtml('', $this->configOptions[$i]->getMetaHTML(), array(), true, $tabindex, $this->type);
		}

		echo "<br>";
		echo "<input type=\"submit\" name=\"saveConfigOptions\" value=\"" . _('Ok') . "\">\n";
		echo "<input type=\"submit\" name=\"cancelConfigOptions\" value=\"" . _('Cancel') . "\">\n";
		echo "</td></tr></table>\n";
		echo "</fieldset>\n";
		
		$this->listPrintFooter();		
	}
	
	/**
	 * Returns the configuration option with the given ID.
	 *
	 * @param String $ID ID
	 */
	protected function listGetConfigOptionByID($ID) {
		for ($i = 0; $i < sizeof($this->configOptions); $i++) {
			if ($this->configOptions[$i]->getID() === $ID) {
				return $this->configOptions[$i];
			}
		}
		return null;
	}
	
	/**
	 * Called when the configuration options changed.
	 */
	protected function listConfigurationChanged() {
		$sizeOption = $this->listGetConfigOptionByID(self::LIST_SIZE_OPTION_NAME);
		if ($sizeOption->getValue() != null) {
			$this->maxPageEntries = $sizeOption->getValue();
		}
		return;
	}

}

/**
 * Represents a tool which can be included in the account lists.
 *
 * @package lists
 * @author Roland Gruber
 */
class lamListTool {
	
	private $name;
	private $image;
	private $target;
	
	/**
	 * Constructor
	 *
	 * @param String $name tool name
	 * @param String $image image file
	 * @param String $target target page
	 * @return lamListTool tool object
	 */
	public function __construct($name, $image, $target) {
		$this->name = $name;
		$this->image = $image;
		$this->target = $target;
	}

	/**
	 * Returns the name of the tool image.
	 * The image is returned without path (e.g. mytool.png). All images must reside in the graphics folder.
	 *
	 * @return String image name
	 */
	public function getImage() {
		return $this->image;
	}

	/**
	 * Returns the tool name.
	 * This is used for the tool tip.
	 *
	 * @return String name
	 */
	public function getName() {
		return $this->name;
	}

	/**
	 * Returns the PHP file (relative to 'templates/lists') which will be the target for this tool.
	 * The target page will be opened with two GET parameters: DN and type (e.g. user)
	 * 
	 * @return String page file (e.g. 'mytool.php')
	 */
	public function getLinkTarget() {
		return $this->target;
	}

}

/**
 * Represents a list configuration option.
 *
 * @package lists
 * @author Roland Gruber
 */
abstract class lamListOption {
	
	private $ID;
	
	private $value;
	
	/**
	 * Creates a new config option.
	 *
	 * @param String $ID unique ID
	 * @return lamConfigOption config option
	 */
	public function __construct($ID) {
		$this->ID = $ID;
	}
	
	/**
	 * Returns the option ID.
	 *
	 * @return String ID
	 */
	public function getID() {
		return $this->ID;
	}
	
	/**
	 * Fills the config option from POST data.
	 * 
	 * @return array list of StatusMessages (array(<type>, <head line>, <body>))
	 */
	public abstract function fillFromPostData();
	
	/**
	 * Returns the option value. The value must not contain "=" and ";".
	 *
	 * @return String value
	 */
	public function getValue() {
		return $this->value;
	}
	
	/**
	 * Sets the config option value. The value must not contain "=" and ";".
	 *
	 * @param String $value
	 */
	public function setValue($value) {
		if ((strpos($value, '=') > -1) || (strpos($value, ';') > -1)) {
			user_error("Invalid value for list option: " . $value, E_ERROR);
		}
		$this->value = $value;
	}
	
	/**
	 * Returns the meta HTML data to display this option.
	 * 
	 * @return array meta HTML
	 */
	public abstract function getMetaHTML();
	
}

/**
 * Boolean option for list configuration.
 *
 * @package lists
 * @author Roland Gruber
 */
class lamBooleanListOption extends lamListOption {
	
	private $name;
	
	/**
	 * Creates a new boolean option.
	 *
	 * @param String $name name to show on config page
	 * @param String $ID unique ID
	 * @return lamBooleanListOption config option
	 */
	public function __construct($name, $ID) {
		parent::__construct($ID);
		$this->name = $name;
	}
	
	/**
	 * Returns if this option is selected.
	 *
	 * @return boolean true, if selected
	 */
	public function isSelected() {
		return ($this->getValue() === "1");
	}
	
	/**
	 * Fills the config option from POST data.
	 * 
	 * @return array list of StatusMessages (array(<type>, <head line>, <body>))
	 */
	public function fillFromPostData() {
		if (isset($_POST[$this->getID()])) {
			$this->setValue("1");
		}
		else {
			$this->setValue("0");
		}
	}

	/**
	 * Returns the meta HTML data to display this option.
	 * 
	 * @return array meta HTML
	 */
	public function getMetaHTML() {
		$return = array();
		$return[] = array(
			array('kind' => 'input', 'name' => $this->getID(), 'type' => 'checkbox', 'checked' => $this->isSelected()),
			array('kind' => 'text', 'text' => $this->name)
		);
		return $return;
	}

}

/**
 * Boolean option for list configuration.
 *
 * @package lists
 * @author Roland Gruber
 */
class lamSelectListOption extends lamListOption {
	
	private $name;
	private $options;
	private $helpID;
	
	/**
	 * Creates a new selection list option.
	 *
	 * @param String $name name to show on config page
	 * @param array $options list of possible values
	 * @param String $ID unique ID
	 * @return lamBooleanListOption config option
	 */
	public function __construct($name, $options, $ID) {
		parent::__construct($ID);
		$this->name = $name;
		$this->options = $options;
	}
	
	/**
	 * Sets the help ID.
	 *
	 * @param Strign $id help ID
	 */
	public function setHelpID($id) {
		$this->helpID = $id;
	}
	
	/**
	 * Fills the config option from POST data.
	 * 
	 * @return array list of StatusMessages (array(<type>, <head line>, <body>))
	 */
	public function fillFromPostData() {
		if (isset($_POST[$this->getID()])) {
			$this->setValue($_POST[$this->getID()]);
		}
		else {
			$this->setValue(null);
		}
	}

	/**
	 * Returns the meta HTML data to display this option.
	 * 
	 * @return array meta HTML
	 */
	public function getMetaHTML() {
		$return = array();
		$return[] = array(
			array('kind' => 'text', 'text' => $this->name),
			array('kind' => 'select', 'name' => $this->getID(), 'options' => $this->options, 'options_selected' => array($this->getValue())),
			array('kind' => 'help', 'value' => '208')
		);
		return $return;
	}

}

?>