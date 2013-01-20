<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2013  Roland Gruber

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
	
	/** tabindex for GUI elements */
	protected $tabindex = 1;

	/** ID for list size config option */
	const LIST_SIZE_OPTION_NAME = "L_SIZE";
	
	/** prefix for virtual (non-LDAP) attributes */
	const VIRTUAL_ATTRIBUTE_PREFIX = 'lam_virtual_';
	
	/**
	 * Constructor
	 *
	 * @param string $type account type
	 * @return lamList list object
	 */
	public function __construct($type) {
		$this->type = $type;
		$this->labels = array(
			'nav' => _("Object count: %s"),
			'error_noneFound' => _("No objects found!"),
			'newEntry' => _("New object"),
			'deleteEntry' => _("Delete selected objects"));
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
		$this->tabindex = 1;
		// do POST actions
		$postFragment = $this->listDoPost();
		// get some parameters
		$this->listGetParams();
		// print HTML head
		$this->listPrintHeader();
		// print messages when redirected from other pages
		$this->listPrintRedirectMessages();
		// refresh data if needed
		if ($this->refresh) {
			$this->listBuildFilter();
			$this->listRefreshData();
		}
		// sort rows by sort column
		if (isset($this->entries)) {
			$this->listSort($this->entries);
		}
		// insert HTML fragment from listDoPost
		echo $postFragment;
		// config dialog
		$this->listPrintConfigurationPage();
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
			echo ("<br>\n");
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
		if (!isset($_POST['clear_filter'])) {
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
					if (preg_match('/^([\p{L}\p{N} _\\*\\$\\.@-])+$/iu', $foundFilter)) { // \p{L} matches any Unicode letter
						$filterParam .= "&amp;filter" . $this->attrArray[$i] . '=' . $foundFilter;
						$LDAPFilterPart .= '(' . $this->attrArray[$i] . '=' . $foundFilter . ')';
					}
					else {
						StatusMessage('ERROR', _('Please enter a valid filter. Only letters, numbers and " _*$.@-" are allowed.'), htmlspecialchars($foundFilter));
					}
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
	*/
	protected function listSort(&$info) {
		if (!is_array($this->attrArray)) return;
		if (!is_string($this->sortColumn)) return;
		// sort and return account list
		usort($info, array($this, "cmp_array"));
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
		echo("<table width=\"100%\" border=\"0\">\n");
		echo("<tr>\n");
		echo("<td align=\"left\">");
		printf($this->labels['nav'], $count);
		echo("</td>");

		if ($count > $this->maxPageEntries) {
			echo("<td class=\"activepage\" align=\"right\">");
			if ($this->page != 1) {
				echo("<a title=\"" . _('Jump to first page') . "\" href=\"list.php?type=" . $this->type . "&amp;norefresh=true&amp;page=1" .
					"&amp;sort=" . $this->sortColumn . "&amp;sortdirection=" . $this->sortDirection . $this->filterText . "\">" .
					"<img height=16 width=16 class=\"align-middle\" alt=\"\" src=\"../../graphics/go-first.png\"></a>\n");
			}
			if ($this->page > 10) {
				echo("<a title=\"" . _('Jump 10 pages backward') . "\" href=\"list.php?type=" . $this->type . "&amp;norefresh=true&amp;page=" . ($this->page - 10) .
					"&amp;sort=" . $this->sortColumn . "&amp;sortdirection=" . $this->sortDirection . $this->filterText . "\">" .
					"<img height=16 width=16 class=\"align-middle\" alt=\"\" src=\"../../graphics/go-previous.png\"></a>\n");
			}
			for ($i = $this->page - 6; $i < ($this->page + 5); $i++) {
				if ($i >= ($count / $this->maxPageEntries)) {
					break;
				}
				elseif ($i < 0) {
					continue;
				}
				if ($i == $this->page - 1) {
					echo '&nbsp;' . ($i + 1) . '&nbsp;';
				}
				else {
					echo "&nbsp;<a href=\"list.php?type=" . $this->type . "&amp;norefresh=true&amp;page=" . ($i + 1) .
					"&amp;sort=" . $this->sortColumn . "&amp;sortdirection=" . $this->sortDirection . $this->filterText . "\">" . ($i + 1) . "</a>\n";
				}
			}
			if ($this->page < (($count / $this->maxPageEntries) - 10)) {
				echo("<a title=\"" . _('Jump 10 pages forward') . "\" href=\"list.php?type=" . $this->type . "&amp;norefresh=true&amp;page=" . ($this->page + 10) .
					"&amp;sort=" . $this->sortColumn . "&amp;sortdirection=" . $this->sortDirection . $this->filterText . "\">" .
					"<img height=16 width=16 class=\"align-middle\" alt=\"\" src=\"../../graphics/go-next.png\"></a>\n");
			}
			if ($this->page < ($count / $this->maxPageEntries)) {
				echo("<a title=\"" . _('Jump to last page') . "\" href=\"list.php?type=" . $this->type . "&amp;norefresh=true&amp;page=" . ceil(($count / $this->maxPageEntries)) .
					"&amp;sort=" . $this->sortColumn . "&amp;sortdirection=" . $this->sortDirection . $this->filterText . "\">" .
					"<img height=16 width=16 class=\"align-middle\" alt=\"\" src=\"../../graphics/go-last.png\"></a>\n");
			}
			echo "</td>";
		}
		echo "</tr></table>\n";
	}

	/**
	* Prints the attribute and filter row at the account table head
	*/
	protected function listPrintTableHeader() {
		// print table header
		echo "<table id=\"accountTable\" frame=\"box\" rules=\"none\" class=\"" . $this->type . "-border accountlist ui-corner-all\" width=\"100%\"><thead>\n";
		echo "<tr class=\"" . $this->type . "-dark\">\n<th width=22 height=34></th>\n<th></th>\n";
		// table header
		for ($k = 0; $k < sizeof($this->descArray); $k++) {
			if (strtolower($this->attrArray[$k]) == $this->sortColumn) {
				$sortImage = "sort_asc.png";
				if ($this->sortDirection < 0) {
					$sortImage = "sort_desc.png";
				}
				echo "<th align=\"left\"><a href=\"list.php?type=" . $this->type . "&amp;".
					"sort=" . strtolower($this->attrArray[$k]) . $this->filterText . "&amp;norefresh=y" . "\">" . $this->descArray[$k] .
					"&nbsp;<img height=16 width=16 style=\"vertical-align: middle;\" src=\"../../graphics/$sortImage\" alt=\"sort direction\"></a></th>\n";
			}
			else echo "<th align=\"left\"><a href=\"list.php?type=" . $this->type . "&amp;".
				"sort=" . strtolower($this->attrArray[$k]) . $this->filterText . "&amp;norefresh=y" . "\">" . $this->descArray[$k] . "</a></th>\n";
		}
		echo "</tr>\n";

		// print filter row
		echo "<tr align=\"center\" class=\"" . $this->type . "-bright\">\n";
		echo "<td width=22 height=34>";
			printHelpLink(getHelp('', '250'), '250');
		echo "</td>\n";
		echo "<td>";
		$filterGroup = new htmlGroup();
		$filterButton = new htmlButton('apply_filter', 'filter.png', true);
		$filterButton->setTitle(_("Filter"));
		$filterGroup->addElement($filterButton);
		if (isset($this->filterPart) && ($this->filterPart != '')) {
			$filterGroup->addElement(new htmlSpacer('1px', null));
			$clearFilterButton = new htmlButton('clear_filter', 'clearFilter.png', true);
			$clearFilterButton->setTitle(_('Clear filter'));
			$filterGroup->addElement($clearFilterButton);
		}
		parseHtml(null, $filterGroup, array(), false, $this->tabindex, $this->type);
		echo "</td>\n";
		// print input boxes for filters
		for ($k = 0; $k < sizeof ($this->descArray); $k++) {
			echo "<td align=\"left\">";
			if ($this->canBeFiltered($this->attrArray[$k])) {
				$value = "";
				if (!isset($_POST['clear_filter'])) {
					if (isset($_GET["filter" . strtolower($this->attrArray[$k])])) {
						$value = $_GET["filter" . strtolower($this->attrArray[$k])];
					}
					if (isset($_POST["filter" . strtolower($this->attrArray[$k])])) {
						$value = $_POST["filter" . strtolower($this->attrArray[$k])];
					}
				}
				$filterInput = new htmlInputField('filter' . strtolower($this->attrArray[$k]), $value);
				$filterInput->setCSSClasses(array($this->type . '-dark'));
				$filterInput->setFieldSize('15');
				$filterInput->setOnKeyPress("SubmitForm('apply_filter', event);");
				parseHtml(null, $filterInput, array(), false, $this->tabindex, $this->type);
			}
			echo "</td>\n";
		}
		echo "</tr></thead>\n";
	}
	
	/**
	 * Returns if the given attribute can be filtered.
	 * If filtering is not possible then no filter box will be displayed.
	 * By default all attributes can be filtered.
	 * 
	 * @param String $attr attribute name
	 * @return boolean filtering possible
	 */
	protected function canBeFiltered($attr) {
		return true;
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
				$classes = ' ' . $this->type . '-bright';
			}
			else {
				$classes = ' ' . $this->type . '-dark';
			}
			echo("<tr class=\"$classes\"" .
				" onClick=\"list_click('" . $rowID . "')\"\n" .
				" onDblClick=\"top.location.href='../account/edit.php?type=" . $this->type . "&amp;DN=" . rawurlencode($info[$i]['dn']) . "'\">\n");
			echo " <td align=\"center\"><input class=\"accountBoxUnchecked\" onClick=\"list_click('" . $rowID . "')\"" .
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
		echo "<tr class=\"" . $this->type . "-bright\">\n";
		echo "<td align=\"center\"><a href=\"#\" onClick=\"list_switchAccountSelection();\"><img height=16 width=16 src=\"../../graphics/select.png\" alt=\"select all\"></a></td>\n";
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
		$toolCount = 0;
		$group = new htmlGroup();
		// edit link
		$editLink = new htmlLink('', "../account/edit.php?type=" . $this->type . "&DN='" . rawurlencode($account['dn']) . "'", '../../graphics/edit.png');
		$editLink->setTitle(_("Edit"));
		$group->addElement($editLink);
		$toolCount++;
		// delete link
		if (checkIfWriteAccessIsAllowed()) {
			$deleteLink = new htmlLink('', "deletelink.php?type=" . $this->type . "&DN='" . rawurlencode($account['dn']) . "'", '../../graphics/delete.png');
			$deleteLink->setTitle(_("Delete"));
			$group->addElement($deleteLink);
			$toolCount++;
		}
		// PDF button
		$pdfButton = new htmlButton("createPDF_" . $id, 'pdf.png', true);
		$pdfButton->setTitle(_('Create PDF file'));
		$group->addElement($pdfButton);
		$toolCount++;
		// additional tools
		$tools = $this->getAdditionalTools();
		for ($i = 0; $i < sizeof($tools); $i++) {
			$toolLink = new htmlLink('', $tools[$i]->getLinkTarget() . "?type=" . $this->type . "&DN='" . rawurlencode($account['dn']) . "'", '../../graphics/' . $tools[$i]->getImage());
			$toolLink->setTitle($tools[$i]->getName());
			$group->addElement($toolLink);
			$toolCount++;
		}
		$width = ($toolCount * 20) + 20;
		echo "<td align='center' style=\"white-space: nowrap; width: ${width}px;\">";
		parseHtml(null, $group, array(), false, $this->tabindex, $this->type);
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
	* 
	* @return String HTML fragment to insert into beginning of account list
	*/
	protected function listDoPost() {
		// check if button was pressed and if we have to add/delete an account or call file upload
		if (isset($_POST['new']) || isset($_POST['del']) || isset($_POST['fileUpload'])){
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
			// file upload
			elseif (isset($_POST['fileUpload'])){
				metaRefresh("../masscreate.php?type=" . $this->type);
				exit;
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
			$filename = '';
			// create for clicked account
			if ($option == 'DN') {
				$_SESSION["accountPDF"] = new accountContainer($this->type, "accountPDF");
				$_SESSION["accountPDF"]->load_account(base64_decode($_POST['clickedAccount']));
				$filename = createModulePDF(array($_SESSION["accountPDF"]),$pdfStruct);
				unset($_SESSION["accountPDF"]);
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
					$filename = createModulePDF($list,$pdfStruct);
					for ($i = 0; $i < sizeof($accounts); $i++) {
						unset($_SESSION["accountPDF-$i"]);
					}
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
					$filename = createModulePDF($list,$pdfStruct);
					for ($i = 0; $i < sizeof($this->entries); $i++) {
						// clean session
						unset($_SESSION["accountPDF-$i"]);
					}
				}
			}
			elseif ($option == 'SESSION') {
				$filename = createModulePDF(array($_SESSION[$_POST['PDFSessionID']]),$pdfStruct);
				unset($_SESSION[$_POST['PDFSessionID']]);
			}
			if ($filename != '') {
				return "<script type=\"text/javascript\">window.open('" . $filename . "', '_blank');</script>";
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
		return '';
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
		// get possible PDF structures
		$pdf_structures = getPDFStructureDefinitions($this->type);
		
		$this->listPrintHeader();

		echo "<div class=\"ui-tabs-nav " . $this->type . "-bright\">";
		echo "<div class=\"smallPaddingContent\">\n";
		echo "<form action=\"list.php?type=" . $this->type . "&amp;norefresh=true\" method=\"post\">\n";
		
		$container = new htmlTable();
		$container->addElement(new htmlSubTitle(_('Create PDF file')), true);
		
		$container->addElement(new htmlTableExtendedSelect('pdf_structure', $pdf_structures, array('default'), _('PDF structure'), '405'), true);
		
		$container->addElement(new htmlSpacer(null, '5px'), true);
		$container->addElement(new htmlOutputText(_('Create for')));
		// check if account object is already in session
		if ($sessionObject != null) {
			$container->addElement(new htmlOutputText($sessionObject->finalDN));
			$container->addElement(new htmlHiddenInput('createFor', 'SESSION'));
			$container->addElement(new htmlHiddenInput('PDFSessionID', $PDFSessionID), true);
		}
		else {
			$radioOptions = array(
				getAbstractDN(base64_decode($id)) => 'DN',
				sprintf(_('All selected accounts (%s)'), sizeof($selAccounts)) => 'SELECTED',
				sprintf(_('All accounts (%s)'), sizeof($this->entries)) => 'ALL'
			);
			$container->addElement(new htmlRadio('createFor', $radioOptions, 'DN'), true);
		}
		
		$container->addElement(new htmlSpacer(null, '10px'), true);
		$buttonContainer = new htmlTable();
		$buttonContainer->colspan = 3;
		$buttonContainer->addElement(new htmlButton('createPDFok', _('Ok')));
		$buttonContainer->addElement(new htmlButton('createPDFCancel', _('Cancel')));
		$container->addElement($buttonContainer, true);
		// hidden inputs for selected accounts
		for ($i = 0; $i < sizeof($selAccounts); $i++) {
			$container->addElement(new htmlHiddenInput($selAccounts[$i], 'on'));
		}
		$container->addElement(new htmlHiddenInput('clickedAccount', $id));
		
		parseHtml(null, $container, array(), false, $this->tabindex, $this->type);
		
		$this->listPrintFooter();
	}

	/**
	* Prints a combobox with possible sub-DNs.
	* 
	* @return htmlGroup OU selection (may be empty)
	*/
	protected function listShowOUSelection() {
		$group = new htmlGroup();
		if (sizeof($this->possibleSuffixes) > 1) {
			$suffixList = array();
			for ($i = 0; $i < sizeof($this->possibleSuffixes); $i++) {
				$suffixList[getAbstractDN($this->possibleSuffixes[$i])] = $this->possibleSuffixes[$i];
			}
			$suffixSelect = new htmlSelect('suffix', $suffixList, array($this->suffix));
			$suffixSelect->setOnchangeEvent("listOUchanged('" . $this->type . "', this)");
			$suffixSelect->setRightToLeftTextDirection(true);
			$suffixSelect->setSortElements(false);
			$suffixSelect->setHasDescriptiveElements(true);
			$group->addElement($suffixSelect);
			$group->addElement(new htmlSpacer('5px', null));
		}
		return $group;
	}

	/**
	 * Prints the create and delete buttons.
	 *
	 * @param boolean $createOnly true if only the create button should be displayed
	 * @param int $tabindex HTML tabindex counter
	 */
	protected function listPrintButtons($createOnly) {
		$table = new htmlTable('100%');
		$left = new htmlGroup();
		// button part
		$left->alignment = htmlElement::ALIGN_LEFT;
		if (checkIfWriteAccessIsAllowed()) {
			// add/delete buttons
			$newButton = new htmlButton('new', $this->labels['newEntry']);
			$newButton->setIconClass('createButton');
			$left->addElement($newButton);
			if (!$createOnly) {
				$left->addElement(new htmlSpacer('1px', null));
				$delButton = new htmlButton('del', $this->labels['deleteEntry']);
				$delButton->setIconClass('deleteButton');
				$left->addElement($delButton);
			}
			$type = new $this->type();
			$toolSettings = $_SESSION['config']->getToolSettings();
			if ($type->supportsFileUpload() && !(isset($toolSettings['tool_hide_toolFileUpload']) && ($toolSettings['tool_hide_toolFileUpload'] == 'true'))) {
				$left->addElement(new htmlSpacer('20px', null));
				$uploadButton = new htmlButton('fileUpload', _('File upload'));
				$uploadButton->setIconClass('upButton');
				$left->addElement($uploadButton);
			}
		}
		
		// OU selection and settings
		$right = new htmlGroup();
		$right->alignment = htmlElement::ALIGN_RIGHT;
		$right->addElement($this->listShowOUSelection());
		$refreshButton = new htmlButton('refresh', 'refresh.png', true);
		$refreshButton->setTitle(_("Refresh"));
		$right->addElement($refreshButton);
		$right->addElement(new htmlSpacer('1px', null));
		$settingsLink = new htmlLink('', '#', '../../graphics/tools.png');
		$settingsLink->setOnClick('listShowSettingsDialog(\'' . _('Change list settings') . '\', \'' . _('Ok') . '\', \'' . _('Cancel') . '\');');
		$settingsLink->setTitle(_('Change settings'));
		$right->addElement($settingsLink);
		
		$this->addExtraInputElementsToTopArea($left, $right);
		$table->addElement($left);
		$table->addElement($right);
		parseHtml(null, $table, array(), false, $this->tabindex, $this->type);
	}
	
	/**
	 * Can be used by subclasses to add e.g. additional buttons to the top area.
	 * 
	 * @param htmlGroup $left left part
	 * @param htmlGroup $right right part
	 */
	protected function addExtraInputElementsToTopArea(&$left, &$right) {
		// only used by subclasses
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
	protected function listGetAttributeDescriptionList() {
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
		if (isset($_POST['refresh']) || isset($_POST['apply_filter']) || isset($_POST['clear_filter'])) {
			$this->refresh = true;
		}
	}

	/**
	 * Rereads the entries from LDAP.
	 */
	protected function listRefreshData() {
		// check suffix
		if (!$this->suffix) {
			$this->suffix = $_SESSION["config"]->get_Suffix($this->type);  // default suffix
		}
		// configure search filter
		$module_filter = get_ldap_filter($this->type);  // basic filter is provided by modules
		$filter = "(&" . $module_filter . $this->filterPart  . ")";
		$attrs = $this->attrArray;
		// remove virtual attributes from list
		for ($i = 0; $i < sizeof($attrs); $i++) {
			if (strpos($attrs[$i], self::VIRTUAL_ATTRIBUTE_PREFIX) === 0) {
				unset($attrs[$i]);
			}
		}
		$attrs = array_values($attrs);
		// include additional attributes
		$additionalAttrs = $this->getAdditionalLDAPAttributesToRead();
		for ($i = 0; $i < sizeof($additionalAttrs); $i++) {
			if (!in_array_ignore_case($additionalAttrs[$i], $attrs)) {
				$attrs[] = $additionalAttrs[$i];
			}
		}
		$this->entries = searchLDAP($this->suffix, $filter, $attrs);
		$lastError = getLastLDAPError();
		if ($lastError != null) {
			call_user_func_array('StatusMessage', $lastError);
		}
		// generate list of possible suffixes
		$typeObj = new $this->type();
		$this->possibleSuffixes = $typeObj->getSuffixList();
	}
	
	/**
	 * Forces a refresh of the LDAP data.
	 * Function must be called before $this->refresh option is checked to load new LDAP data (e.g. in listGetParams).
	 */
	protected function forceRefresh() {
		$this->refresh = true;
		if (isset($_GET['norefresh'])) {
			unset($_GET['norefresh']);
		}
	}
	
	/**
	 * Returns a list of additional LDAP attributes that should be read.
	 * This can be used to show additional data even if the user selected other attributes to show in the list.
	 * 
	 * @return array additional attribute names
	 */
	protected function getAdditionalLDAPAttributesToRead() {
		return array();
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
		$listSizeOption = new lamSelectListOption(_("Maximum list entries"), array(10, 20, 30, 50, 75, 100, 500, 1000), self::LIST_SIZE_OPTION_NAME);
		$listSizeOption->setHelpID('208');
		$listSizeOption->setValue($this->maxPageEntries);
		return array($listSizeOption);
	}

	/**
	 * Prints the list configuration page.
	 */
	protected function listPrintConfigurationPage() {
		echo "<div id=\"settingsDialog\" class=\"hidden\">\n";
		echo "<form id=\"settingsDialogForm\" action=\"list.php?type=" . $this->type . "&amp;norefresh=true\" method=\"post\">\n";
		echo '<table width="100%"><tr><td>';
		
		$configContainer = new htmlTable();
		for ($i = 0; $i < sizeof($this->configOptions); $i++) {
			$configContainer->mergeTableElements($this->configOptions[$i]->getMetaHTML());
		}
		$configContainer->addElement(new htmlHiddenInput('saveConfigOptions', 'ok'));		
		
		parseHtml('', $configContainer, array(), false, $this->tabindex, $this->type);

		echo "</td></tr></table>\n";
		echo '</form>';
		echo "</div>\n";
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

	/**
	 * Prints messages when another page (e.g. delete/upload) redirects to the list view.
	 */
	protected function listPrintRedirectMessages() {
		if (isset($_GET['deleteAllOk'])) {
			StatusMessage('INFO', _('Deletion was successful.'));
		}
		elseif (isset($_GET['uploadAllOk'])) {
			StatusMessage('INFO', _("Upload has finished"));
			if (isset($_SESSION['mass_pdf']['file'])) {
				StatusMessage('INFO', sprintf(_('You can download your PDF files {link=%s}{color=#d2131a}here{endcolor}{endlink}.'), '../' . $_SESSION['mass_pdf']['file']));
			}
		}
		if (isset($_SESSION['listRedirectMessages'])) {
			for ($i = 0; $i < sizeof($_SESSION['listRedirectMessages']); $i++) {
				call_user_func_array('StatusMessage', $_SESSION['listRedirectMessages'][$i]);
			}
			unset($_SESSION['listRedirectMessages']);
		}
	}

}

/**
 * Represents a tool which can be included in the account lists.
 *
 * @package lists
 * @author Roland Gruber
 */
class lamListTool {
	
	/** tool name */
	private $name;
	/** tool image */
	private $image;
	/** link target */
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
	
	/** unique ID */
	private $ID;
	/** option value */
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
	 * @return htmlTable meta HTML
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
	
	/** option name */
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
	 * @return htmlTable meta HTML
	 */
	public function getMetaHTML() {
		$return = new htmlTable();
		$return->addElement(new htmlTableExtendedInputCheckbox($this->getID(), $this->isSelected(), $this->name));
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
	
	/** option name */
	private $name;
	/** possible select options */
	private $options;
	/** help ID */
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
	 * @return htmlTable meta HTML
	 */
	public function getMetaHTML() {
		$return = new htmlTable();
		$return->addElement(new htmlTableExtendedSelect($this->getID(), $this->options, array($this->getValue()), $this->name, $this->helpID));
		return $return;
	}
	
}

?>