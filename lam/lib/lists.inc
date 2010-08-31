<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2010  Roland Gruber

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
	
	/** LDAP filter part which was entered by user via filter boxes */
	protected $filterPart = '';

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
		if ($this->refresh) {
			$this->listBuildFilter();
			$this->listRefreshData();
		}
		// sort rows by sort column
		if (isset($this->entries)) {
			$this->entries = $this->listSort($this->entries);
		}
		// show form
		echo "<div class=\"ui-tabs-panel ui-widget-content ui-corner-bottom\">";
		echo "<div id=\"listTabContentArea\">\n";
		echo ("<form action=\"list.php?type=" . $this->type . "&amp;norefresh=true\" method=\"post\">\n");
		// draw account list if accounts were found
		if (sizeof($this->entries) > 0) {
			// buttons
			$this->listPrintButtons(false);
			echo ("<br>\n");
			// navigation bar
			$this->listDrawNavigationBar(sizeof($this->entries));
			echo ("<br>\n");
			echo "<div id=\"listScrollArea\" style=\"overflow: auto; padding: 1px;\">";
			// account table head
			$this->listPrintTableHeader();
			// account table body
			$this->listPrintTableBody($this->entries);
			echo "</div>";
		}
		else {
			// buttons
			$this->listPrintButtons(true);
			echo ("<hr style=\"background-color: #999999;\">\n");
			// navigation bar
			$this->listDrawNavigationBar(sizeof($this->entries));
			echo ("<br>\n");
			// account table head
			$this->listPrintTableHeader();
			echo "</table><br>\n";
		}
		$this->listPrintFooter();
	}

	/**
	* Builds the regular expressions from the filter values.
	*/
	protected function listBuildFilter() {
		$filter = array();
		$filterParam = "";
		$LDAPFilterPart = "";
		// build filter array
		for ($i = 0; $i < sizeof($this->attrArray); $i++) {
			$foundFilter = null;
			if (isset($_GET["filter" . strtolower($this->attrArray[$i])])) {
				$foundFilter = $_GET["filter" . strtolower($this->attrArray[$i])];
			}
			if (isset($_POST["filter" . strtolower($this->attrArray[$i])])) {
				$foundFilter = $_POST["filter" . strtolower($this->attrArray[$i])];
			}
			if (isset($foundFilter) && ($foundFilter != '')) {
				if (preg_match('/^([\p{L}\p{N} _\\*\\$\\.-])+$/iu', $foundFilter)) { // \p{L} matches any Unicode letter
					$filterParam .= "&amp;filter" . $this->attrArray[$i] . '=' . $foundFilter;
					$LDAPFilterPart .= '(' . $this->attrArray[$i] . '=' . $foundFilter . ')';
				}
				else {
					StatusMessage('ERROR', _('Please enter a valid filter. Only letters, numbers and " _*$.-" are allowed.'), htmlspecialchars($foundFilter));
				}
			}
		}
		$this->filterText = $filterParam;
		$this->filterPart = $LDAPFilterPart;
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
		if ($this->sortColumn != "dn") {
			// sort by first attribute with name $sort
			return @strnatcasecmp($a[$this->sortColumn][0], $b[$this->sortColumn][0]) * $this->sortDirection;
		}
		else {
			return strnatcasecmp($a[$this->sortColumn], $b[$this->sortColumn]) * $this->sortDirection;
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
		echo("<td width=\"33%\"><input type=\"submit\" name=\"refresh\" value=\"" . _("Refresh") . "\">&nbsp;&nbsp;");
		echo("</td>");

		echo("<td width=\"33%\" align=\"center\" class=\"" . $this->type . "nav-text\">");
		echo"&nbsp;";
		printf($this->labels['nav'], $count);
		echo("</td>");

		echo("<td width=\"33%\" class=\"" . $this->type . "nav-activepage\" align=\"right\">");
		if ($this->page != 1) {
			echo("<a title=\"" . _('Jump to first page') . "\" href=\"list.php?type=" . $this->type . "&amp;norefresh=true&amp;page=1" .
				"&amp;sort=" . $this->sortColumn . "&amp;sortdirection=" . $this->sortDirection . $this->filterText . "\">" .
				"<img class=\"align-middle\" alt=\"\" src=\"../../graphics/go-first.png\"></a>\n");
		}
		if ($this->page > 10) {
			echo("<a title=\"" . _('Jump 10 pages backward') . "\" href=\"list.php?type=" . $this->type . "&amp;norefresh=true&amp;page=" . ($this->page - 10) .
				"&amp;sort=" . $this->sortColumn . "&amp;sortdirection=" . $this->sortDirection . $this->filterText . "\">" .
				"<img class=\"align-middle\" alt=\"\" src=\"../../graphics/go-previous.png\"></a>\n");
		}
		for ($i = $this->page - 5; $i < ($this->page + 4); $i++) {
			if ($i >= ($count / $this->maxPageEntries)) {
				break;
			}
			elseif ($i < 0) {
				continue;
			}
			if ($i == $this->page - 1) {
				echo("&nbsp;" . ($i + 1));
			}
			else {
				echo("&nbsp;<a href=\"list.php?type=" . $this->type . "&amp;norefresh=true&amp;page=" . ($i + 1) .
				"&amp;sort=" . $this->sortColumn . "&amp;sortdirection=" . $this->sortDirection . $this->filterText . "\">" . ($i + 1) . "</a>\n");
			}
		}
		if ($this->page < (($count / $this->maxPageEntries) - 10)) {
			echo("<a title=\"" . _('Jump 10 pages forward') . "\" href=\"list.php?type=" . $this->type . "&amp;norefresh=true&amp;page=" . ($this->page + 10) .
				"&amp;sort=" . $this->sortColumn . "&amp;sortdirection=" . $this->sortDirection . $this->filterText . "\">" .
				"<img class=\"align-middle\" alt=\"\" src=\"../../graphics/go-next.png\"></a>\n");
		}
		if ($this->page < ($count / $this->maxPageEntries)) {
			echo("<a title=\"" . _('Jump to last page') . "\" href=\"list.php?type=" . $this->type . "&amp;norefresh=true&amp;page=" . round(($count / $this->maxPageEntries)) .
				"&amp;sort=" . $this->sortColumn . "&amp;sortdirection=" . $this->sortDirection . $this->filterText . "\">" .
				"<img class=\"align-middle\" alt=\"\" src=\"../../graphics/go-last.png\"></a>\n");
		}
		echo("</td></tr></table>\n");
	}

	/**
	* Prints the attribute and filter row at the account table head
	*/
	protected function listPrintTableHeader() {
		// print table header
		echo "<table id=\"accountTable\" frame=\"box\" rules=\"none\" class=\"" . $this->type . "list\" width=\"100%\"><thead>\n";
		echo "<tr class=\"" . $this->type . "list-dark\">\n<th width=22 height=34></th>\n<th></th>\n";
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
		echo "<tr align=\"center\" class=\"" . $this->type . "list-bright\">\n";
		echo "<td width=22 height=34>";
			printHelpLink(getHelp('', '250'), '250');
		echo "</td>\n";
		echo "<td>";
		echo "<button class=\"smallPadding\" id=\"filterButton\" name=\"apply_filter\">" . _("Filter") . "</button>";
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
		echo "</tr></thead>\n";
	}

	/**
	* Prints the entry list
	*
	* @param array $info entries
	*/
	protected function listPrintTableBody(&$info) {
		echo "<tbody>\n";
		// calculate which rows to show
		$table_begin = ($this->page - 1) * $this->maxPageEntries;
		if (($this->page * $this->maxPageEntries) > sizeof($info)) $table_end = sizeof($info);
		else $table_end = ($this->page * $this->maxPageEntries);
		// print account list
		for ($i = $table_begin; $i < $table_end; $i++) {
			$rowID = base64_encode($info[$i]['dn']);
			if ((($i - $table_begin) % 2) == 1) {
				$classes = ' ' . $this->type . 'list-bright';
			}
			else {
				$classes = ' ' . $this->type . 'list-dark';
			}
			echo("<tr class=\"$classes\" onMouseOver=\"list_over(this, '" . $rowID . "', '" . $this->type . "')\"\n" .
				" onMouseOut=\"list_out(this, '" . $rowID . "', '" . $this->type . "')\"\n" .
				" onClick=\"list_click(this, '" . $rowID . "', '" . $this->type . "')\"\n" .
				" onDblClick=\"top.location.href='../account/edit.php?type=" . $this->type . "&amp;DN=" . rawurlencode($info[$i]['dn']) . "'\">\n");
			echo " <td align=\"center\"><input class=\"accountBoxUnchecked\" onClick=\"list_click(this, '" . $rowID . "', '" . $this->type . "')\"" .
				" type=\"checkbox\" name=\"" . $rowID . "\"></td>\n";
			$this->listPrintToolLinks($info[$i], $rowID);
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
		echo "<tr class=\"" . $this->type . "list-bright\">\n";
		echo "<td align=\"center\"><a href=\"#\" onClick=\"list_switchAccountSelection();\"><img src=\"../../graphics/select.png\" alt=\"select all\"></a></td>\n";
		echo "<td colspan=$colspan>&nbsp;<a href=\"#\" onClick=\"list_switchAccountSelection();\">" .
			"<font color=\"black\">" . _("Select all") . "</font></a></td>\n";
		echo "</tr>\n";
		echo "</tbody>\n";
		echo "</table>\n";
	}
	
	/**
	 * Prints the tool image links (e.g. edit and delete) for each account.
	 * 
	 * @param array $account LDAP attributes
	 * @param String $id account ID
	 */
	private function listPrintToolLinks($account, $id) {
		$output = '';
		$toolCount = 0;
		// edit image
		$output .= "<a href=\"../account/edit.php?type=" . $this->type . "&amp;DN='" . rawurlencode($account['dn']) . "'\">";
		$output .= "<img src=\"../../graphics/edit.png\" alt=\"" . _("Edit") . "\" title=\"" . _("Edit") . "\">";
		$output .= "</a>\n ";
		$toolCount++;
		// delete image
		if (checkIfWriteAccessIsAllowed()) {
			$output .= "<a href=\"deletelink.php?type=" . $this->type . "&amp;DN='" . rawurlencode($account['dn']) . "'\">";
			$output .= "<img src=\"../../graphics/delete.png\" alt=\"" . _("Delete") . "\" title=\"" . _("Delete") . "\">";
			$output .= "</a>\n ";
			$toolCount++;
		}
		// pdf image
		$pdfButtonStyle = "background-image: url(../../graphics/pdf.png);background-position: -1px -1px;background-repeat: no-repeat;width:20px;height:20px;background-color:transparent;border-style:none;";
		$output .= "<input type=\"submit\" style=\"$pdfButtonStyle\" name=\"createPDF_" . $id . "\" value=\" \" title=\"" . _('Create PDF file') . "\">\n ";
		$toolCount++;
		// additional tools
		$tools = $this->getAdditionalTools();
		for ($i = 0; $i < sizeof($tools); $i++) {
			$output .= "<a href=\"" . $tools[$i]->getLinkTarget() . "?type=" . $this->type . "&amp;DN='" . rawurlencode($account['dn']) . "'\">";
			$output .= "<img src=\"../../graphics/" . $tools[$i]->getImage() . "\" alt=\"" . $tools[$i]->getName() . "\" title=\"" . $tools[$i]->getName() . "\">";
			$output .= "</a>\n ";
			$toolCount++;
		}
		$width = ($toolCount * 20) + 20;
		echo "<td align='center' style=\"white-space: nowrap; width: ${width}px\">";
		echo $output;
		echo "</td>\n";
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
				// build DN list
				$_SESSION['delete_dn'] = array();
				for ($i = 0; $i < sizeof($accounts); $i++) {
					$_SESSION['delete_dn'][] = base64_decode($accounts[$i]);
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
				if (sizeof($parts) == 2) {
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
				$_SESSION["accountPDF"]->load_account(base64_decode($_POST['clickedAccount']));
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
					$_SESSION["accountPDF-$i"] = new accountContainer($this->type, "accountPDF-$i");
					$_SESSION["accountPDF-$i"]->load_account(base64_decode($accounts[$i]));
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
			elseif ($option == 'SESSION') {
				createModulePDF(array($_SESSION[$_POST['PDFSessionID']]),$pdfStruct);
				unset($_SESSION[$_GET['PDFSessionID']]);
				exit;
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
		$sessionObject = null;
		$PDFSessionID = null;
		if (($id == null) && isset($_GET['PDFSessionID'])) {
			$PDFSessionID = $_GET['PDFSessionID'];
			$sessionObject = $_SESSION[$PDFSessionID];
		}
		// search for checkboxes
		$selAccounts = array_keys($_POST, "on");
		if (!in_array($id, $selAccounts)) {
			$selAccounts[] = $id;
		}

		$this->listPrintHeader();

		echo "<br>\n";
		echo "<form target=\"_blank\" action=\"list.php?type=" . $this->type . "&amp;norefresh=true\" method=\"post\">\n";
		echo "<div class=\"ui-tabs-nav\">";
		
		echo '<fieldset class="' . $this->type . 'edit"><legend>' . _('Create PDF file') . '</legend><br>';
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
					// check if account object is already in session
					if ($sessionObject != null) {
						echo $sessionObject->finalDN;
						echo "<input type=\"hidden\" name=\"createFor\" value=\"SESSION\">\n";
						echo "<input type=\"hidden\" name=\"PDFSessionID\" value=\"$PDFSessionID\">\n";
					}
					else {
						echo '<select name="createFor">';
						echo '<option value="DN">' . base64_decode($id) . '</option>';
						echo '<option value="SELECTED">' . sprintf(_('All selected accounts (%s)'), sizeof($selAccounts)) . '</option>';
						echo '<option value="ALL">' . sprintf(_('All accounts (%s)'), sizeof($this->entries)) . '</option>';
						echo '</select>';
					}
				echo '</td>';
			echo '</tr>';
			echo '<tr>';
				echo '<td colspan=2>&nbsp;</td>';
			echo '</tr>';
			echo '<tr>';
				echo '<td colspan=2>';
					echo '<input type="submit" name="createPDFok" value="' . _('Ok') . '"> ';
					echo '&nbsp;<a href="list.php?type=' . $this->type . '&amp;norefresh=true">' . _('Cancel') . '</a>';
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
			echo ("<select class=\"" . $this->type . " rightToLeftText\" size=1 name=\"suffix\" onchange=\"listOUchanged('" . $this->type . "', this)\">\n");
			for ($i = 0; $i < sizeof($this->possibleSuffixes); $i++) {
				if ($this->suffix == $this->possibleSuffixes[$i]) {
					echo ("<option value=\"" . $this->possibleSuffixes[$i] . "\" selected>" . getAbstractDN($this->possibleSuffixes[$i]) . "</option>\n");
				}
				else echo("<option value=\"" . $this->possibleSuffixes[$i] . "\">" . getAbstractDN($this->possibleSuffixes[$i]) . "</option>\n");
			}
			echo ("</select>\n");
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
		include '../main_header.php';
	}
	
	/**
	 * Prints the HTML footer.
	 */
	protected function listPrintFooter() {
		?>
			</form></div></div>
			<script type="text/javascript">
				jQuery(document).ready(function() {
					jQuery('#tab_<?php echo $this->type; ?>').addClass('ui-tabs-selected');
					jQuery('#tab_<?php echo $this->type; ?>').addClass('ui-state-active');
					window.onload = listResizeITabContentDiv;
					window.onresize = listResizeITabContentDiv;
					jQuery('#filterButton').button();
				});
			</script>
		<?php
		include '../main_footer.php';
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
		// check if only PDF should be shown
		if (isset($_GET['printPDF'])) {
			$this->showPDFPage(null);
			exit();
		}
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
		if (isset($_POST['refresh']) || isset($_POST['apply_filter'])) {
			$this->refresh = true;
		}
	}

	/**
	 * Rereads the entries from LDAP.
	 */
	protected function listRefreshData() {
		// configure search filter
		$module_filter = get_ldap_filter($this->type);  // basic filter is provided by modules
		$filter = "(&" . $module_filter . $this->filterPart  . ")";
		$attrs = $this->attrArray;
		$entries = searchLDAP($this->suffix, $filter, $attrs);
		$lastError = getLastLDAPError();
		if ($lastError != null) {
			call_user_func_array('StatusMessage', $lastError);
		}
		$this->entries = $entries;
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
		echo "<div class=\"ui-tabs-nav\">";
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