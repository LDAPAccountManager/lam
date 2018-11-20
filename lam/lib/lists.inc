<?php
use LAM\TYPES\ConfiguredType;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2018  Roland Gruber

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

	/** entries to show */
	protected $entries;

	/** entries from LDAP */
	protected $ldapEntries;

	/** sort mapping for entries array(original index => sorted index) */
	protected $sortMapping;

	/** list of filters (attribute name => filter input) */
	protected $filters = array();

	/** list of possible LDAP suffixes(organizational units) */
	protected $possibleSuffixes;

	/** list of account specific labels */
	protected $labels;

	/** configuration options */
	private $configOptions;

	/** tabindex for GUI elements */
	protected $tabindex = 1;

	/** defines if the server side filter changed */
	protected $serverSideFilterChanged;

	/** ID for list size config option */
	const LIST_SIZE_OPTION_NAME = "L_SIZE";

	/** prefix for virtual (non-LDAP) attributes */
	const VIRTUAL_ATTRIBUTE_PREFIX = 'lam_virtual_';

	/**
	 * List of attributes to filter on server side.
	 *
	 * @var string[]
	 */
	protected $serverSideFilterAttributes = array(
		'cn', 'commonname', 'uid', 'memberuid', 'description',
		'sn', 'surname', 'gn', 'givenname', 'company', 'mail'
	);

	/**
	 * Constructor
	 *
	 * @param LAM\TYPES\ConfiguredType $type account type
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
			if (isset($_COOKIE["ListOptions_" . $this->type->getId()])) {
				$cookieValue = $_COOKIE["ListOptions_" . $this->type->getId()];
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
		// update filter
		$this->listBuildFilter();
		// get some parameters
		$this->listGetParams();
		// print HTML head
		$this->printHeader();
		// print messages when redirected from other pages
		$this->listPrintRedirectMessages();
		// refresh data if needed
		if ($this->refresh) {
			$this->listRefreshData();
		}
		// local filtering
		$this->applyLocalFilters();
		// sort rows by sort column
		if (isset($this->entries)) {
			$this->listCreateSortMapping($this->entries);
		}
		// insert HTML fragment from listDoPost
		echo $postFragment;
		// config dialog
		$this->listPrintConfigurationPage();
		// show form
		echo "<div class=\"ui-tabs-panel ui-widget-content ui-corner-bottom\">";
		echo "<div id=\"listTabContentArea\">\n";
		echo ("<form action=\"list.php?type=" . $this->type->getId() . "&amp;norefresh=true\" method=\"post\">\n");
		// draw account list if accounts were found
		if (sizeof($this->entries) > 0) {
			// buttons
			$this->listPrintButtons(false);
			echo ("<br>\n");
			// navigation bar
			$this->listDrawNavigationBar(sizeof($this->entries));
			echo ("<br>\n");
			$this->printAccountTable($this->entries);
		}
		else {
			// buttons
			$this->listPrintButtons(true);
			echo ("<br>\n");
			// navigation bar
			$this->listDrawNavigationBar(sizeof($this->entries));
			echo ("<br>\n");
			$accounts = array();
			$this->printAccountTable($accounts);
			echo "</table><br>\n";
		}
		$this->printFooter();
	}

	/**
	* Builds the regular expressions from the filter values.
	*/
	protected function listBuildFilter() {
		if (isset($_GET['accountEditBack'])) {
			return;
		}
		$oldFilter = $this->filters;
		$this->serverSideFilterChanged = false;
		$this->filters = array();
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
					if (preg_match('/^([\p{L}\p{N} _\\*\\$\\.:@-])+$/iu', $foundFilter)) { // \p{L} matches any Unicode letter
						$this->filters[strtolower($this->attrArray[$i])] = $foundFilter;
					}
					else {
						StatusMessage('ERROR', _('Please enter a valid filter. Only letters, numbers and " _*$.@-" are allowed.'), htmlspecialchars($foundFilter));
					}
				}
			}
		}
		$filterAttrs = array_merge(array_keys($oldFilter), array_keys($this->filters));
		$filterAttrs = array_unique($filterAttrs);
		foreach ($filterAttrs as $attrName) {
			if (!$this->isAttributeFilteredByServer($attrName)) {
				continue;
			}
			if (!isset($oldFilter[$attrName]) || !isset($this->filters[$attrName]) || ($oldFilter[$attrName] != $this->filters[$attrName])) {
				$this->serverSideFilterChanged = true;
				break;
			}
		}
	}

	/**
	* Determines the sort mapping and stores it in $this->sortMapping.
	* The sort mapping is used to display the right rows when the account table is created.
	*
	* @param array $info the account list
	*/
	protected function listCreateSortMapping(&$info) {
		if (!is_array($this->attrArray)) return;
		if (!is_string($this->sortColumn)) return;
		$toSort = array();
		$col = $this->sortColumn;
		$size = sizeof($info);
		if ($this->sortColumn != 'dn') {
			for ($i = 0; $i < $size; $i++) {
				// sort by first attribute with name $sort
				$toSort[] = &$info[$i][$col][0];
			}
		}
		else {
			for ($i = 0; $i < $size; $i++) {
				$toSort[] = &$info[$i][$col];
			}
		}
		natcasesort($toSort);
		$sortResult = array();
		if ($this->sortDirection == 1) {
			foreach ($toSort as $orig => $val) {
				$sortResult[] = $orig;
			}
		}
		else {
			$counter = sizeof($toSort);
			foreach ($toSort as $orig => $val) {
				$counter--;
				$sortResult[$counter] = $orig;
			}
		}
		$this->sortMapping = &$sortResult;
	}

	/**
	* Draws a navigation bar to switch between pages
	*
	* @param integer $count number of account entries
	*/
	protected function listDrawNavigationBar($count) {
		$filter = $this->getFilterAsTextForURL();
		$row = new htmlResponsiveRow();
		$row->setCSSClasses(array('maxrow'));
		$countLabel = new htmlOutputText(sprintf($this->labels['nav'], $count));
		$row->add($countLabel, 12, 6, 6);
		$navGroup = new htmlGroup();
		if ($count > $this->maxPageEntries) {
			if ($this->page != 1) {
				$linkHref = "list.php?type=" . $this->type->getId() . "&norefresh=true&page=1" .
						"&sort=" . $this->sortColumn . "&sortdirection=" . $this->sortDirection . $filter;
				$link = new htmlLink(null, $linkHref, '../../graphics/go-first.png');
				$link->setTitle(_('Jump to first page'));
				$link->setCSSClasses(array('margin5'));
				$navGroup->addElement($link);
			}
			if ($this->page > 11) {
				$linkHref = "list.php?type=" . $this->type->getId() . "&norefresh=true&page=" . ($this->page - 10) .
						"&sort=" . $this->sortColumn . "&sortdirection=" . $this->sortDirection . $filter;
				$link = new htmlLink(null, $linkHref, '../../graphics/go-previous.png');
				$link->setTitle(_('Jump 10 pages backward'));
				$link->setCSSClasses(array('margin5'));
				$navGroup->addElement($link);
			}
			$pageCount =  ceil($count / $this->maxPageEntries);
			for ($i = $this->page - 6; $i < ($this->page + 5); $i++) {
				if ($i >= $pageCount) {
					break;
				}
				elseif ($i < 0) {
					continue;
				}
				if ($i == $this->page - 1) {
					$url = "list.php?type=" . $this->type->getId() . "&norefresh=true" .
							"&sort=" . $this->sortColumn . "&sortdirection=" . $this->sortDirection . $filter;
					$navInput = new htmlInputField('listNavPage', ($i + 1));
					$navInput->setMinimumAndMaximumNumber(1, $pageCount);
					$navInput->setCSSClasses(array('listPageInput'));
					$navInput->setOnKeyPress('listPageNumberKeyPress(\'' . $url . '\', event);');
					$navGroup->addElement($navInput);
				}
				else {
					$linkHref = "list.php?type=" . $this->type->getId() . "&norefresh=true&page=" . ($i + 1) .
					"&sort=" . $this->sortColumn . "&sortdirection=" . $this->sortDirection . $filter;
					$link = new htmlLink(($i + 1), $linkHref);
					$link->setCSSClasses(array('margin5'));
					$navGroup->addElement($link);
				}
			}
			if ($this->page < ($pageCount - 10)) {
				$linkHref = "list.php?type=" . $this->type->getId() . "&norefresh=true&page=" . ($this->page + 10) .
						"&sort=" . $this->sortColumn . "&sortdirection=" . $this->sortDirection . $filter;
				$link = new htmlLink(null, $linkHref, '../../graphics/go-next.png');
				$link->setTitle(_('Jump 10 pages forward'));
				$link->setCSSClasses(array('margin5'));
				$navGroup->addElement($link);
			}
			if ($this->page < $pageCount) {
				$linkHref = "list.php?type=" . $this->type->getId() . "&norefresh=true&page=" . $pageCount .
						"&sort=" . $this->sortColumn . "&sortdirection=" . $this->sortDirection . $filter;
				$link = new htmlLink(null, $linkHref, '../../graphics/go-last.png');
				$link->setTitle(_('Jump to last page'));
				$link->setCSSClasses(array('margin5'));
				$navGroup->addElement($link);
			}
		}
		$row->add($navGroup, 12, 6, 6, 'responsiveLabel');
		parseHtml(null, $row, array(), false, $this->tabindex, $this->type->getScope());
	}

	/**
	 * Returns the filter as text to be used as URL parameter.
	 *
	 * @return String filter text
	 */
	protected function getFilterAsTextForURL() {
		$text = '';
		foreach ($this->filters as $attr => $filter) {
			$text .= "&filter" . strtolower($attr) . '=' . $filter;
		}
		return $text;
	}

	/**
	* Prints the entry list
	*
	* @param array $info entries
	*/
	private function printAccountTable(&$info) {
		$scope = $this->type->getScope();
		$titles = $this->descArray;
		array_unshift($titles, _('Actions'));
		$data = array();
		$data[] = $this->getSortingElements();
		$data[] = $this->getFilterElements();
		$onClickEvents = array();
		$onDoubleClickEvents = array();
		$this->addDataElements($data, $info, $onClickEvents, $onDoubleClickEvents);

		$table = new htmlResponsiveTable($titles, $data);
		$table->setRowClasses($scope . '-dark', $scope . '-bright');
		$table->setCSSClasses(array($scope . '-border accountlist'));
		$table->setOnClickEvents($onClickEvents);
		$table->setOnDoubleClickEvents($onDoubleClickEvents);

		parseHtml(null, $table, array(), false, $this->tabindex, $scope);
	}

	/**
	 * Returns the elements to show in sorting row.
	 *
	 * @return htmlElement[] elements
	 */
	private function getSortingElements() {
		$filter = $this->getFilterAsTextForURL();
		$sortElements = array(new htmlOutputText(_('Sort sequence')));
		foreach ($this->attrArray as $attributeName) {
			$link = "list.php?type=" . $this->type->getId() . "&".
				"sort=" . strtolower($attributeName) . $filter . "&norefresh=y";
			$buttons = new htmlGroup();
			if (strtolower($attributeName) == $this->sortColumn) {
				if ($this->sortDirection < 0) {
					$buttons->addElement(new htmlLink(null, $link . '&sortdirection=1', '../../graphics/downarrows.png'));
					$buttons->addElement(new htmlLink(null, $link . '&sortdirection=-1', '../../graphics/uparrows-black.png'));
				}
				else {
					$buttons->addElement(new htmlLink(null, $link . '&sortdirection=1', '../../graphics/downarrows-black.png'));
					$buttons->addElement(new htmlLink(null, $link . '&sortdirection=-1', '../../graphics/uparrows.png'));
				}
			}
			else {
				$buttons->addElement(new htmlLink(null, $link . '&sortdirection=1', '../../graphics/downarrows.png'));
				$buttons->addElement(new htmlLink(null, $link . '&sortdirection=-1', '../../graphics/uparrows.png'));
			}
			$sortElements[] = $buttons;
		}
		return $sortElements;
	}

	/**
	 * Returns the elements to show in filter row.
	 *
	 * @return htmlElement[] elements
	 */
	private function getFilterElements() {
		$actionElement = new htmlGroup();
		$selectAll = new htmlInputCheckbox('tableSelectAll', false);
		$selectAll->setCSSClasses(array('align-middle'));
		$selectAll->setOnClick('list_switchAccountSelection();');
		$actionElement->addElement($selectAll);
		$actionElement->addElement(new htmlSpacer('1rem', null));
		$actionElement->addElement(new htmlOutputText(_('Filter')));
		$filterButton = new htmlButton('apply_filter', 'filter.png', true);
		$filterButton->setTitle(_("Here you can input simple filter expressions (e.g. 'value' or 'v*'). The filter is case-insensitive."));
		$actionElement->addElement($filterButton);
		if (sizeof($this->filters) > 0) {
			$clearFilterButton = new htmlButton('clear_filter', 'clearFilter.png', true);
			$clearFilterButton->setTitle(_('Clear filter'));
			$actionElement->addElement($clearFilterButton);
		}

		$filterElements = array(new htmlDiv(null, $actionElement, array('nowrap')));
		$clearFilter = isset($_POST['clear_filter']);
		foreach ($this->attrArray as $attributeName) {
			$attributeName = strtolower($attributeName);
			if ($this->canBeFiltered($attributeName)) {
				$value = "";
				if (!$clearFilter && isset($this->filters[$attributeName])) {
					$value = $this->filters[$attributeName];
				}
				$filterInput = new htmlInputField('filter' . $attributeName, $value, null);
				$filterInput->setCSSClasses(array($this->type->getScope() . '-bright'));
				$filterInput->setOnKeyPress("SubmitForm('apply_filter', event);");
				$filterElements[] = $filterInput;
			}
			else {
				$filterElements[] = new htmlOutputText('');
			}
		}
		return $filterElements;
	}

	/**
	 * Adds the LDAP data elements to the given array.
	 *
	 * @param array $data data for responsible table
	 * @param array $info entries
	 * @param array $onClickEvents row number => code
	 * @param array $onDoubleClickEvents row number => code
	 */
	private function addDataElements(&$data, &$info, &$onClickEvents, &$onDoubleClickEvents) {
		// calculate which rows to show
		$table_begin = ($this->page - 1) * $this->maxPageEntries;
		if (($this->page * $this->maxPageEntries) > sizeof($info)) $table_end = sizeof($info);
		else $table_end = ($this->page * $this->maxPageEntries);
		// get sort mapping
		$sortMapping = &$this->sortMapping;
		if (empty($sortMapping)) {
			$sortMapping = array();
			$infoSize = sizeof($info);
			for ($i = 0; $i < $infoSize; $i++) {
				$sortMapping[$i] = $i;
			}
		}
		// print account list
		for ($i = $table_begin; $i < $table_end; $i++) {
			$row = array();
			$index = $sortMapping[$i];
			$rowID = base64_encode($info[$index]['dn']);
			$actionElement = new htmlGroup();
			$checkbox = new htmlInputCheckbox($rowID, false);
			$checkbox->setOnClick("list_click('" . $rowID . "');");
			$checkbox->setCSSClasses(array('accountBoxUnchecked align-middle'));
			$actionElement->addElement($checkbox);
			$actionElement->addElement(new htmlSpacer('0.5rem', null));
			$onClickEvents[$i + 2] = "list_click('" . $rowID . "');";
			$onDoubleClickEvents[$i + 2] = "top.location.href='../account/edit.php?type=" . $this->type->getId() . "&amp;DN=" . rawurlencode($info[$index]['dn']) . "';";
			$this->addToolLinks($info[$index], $rowID, $actionElement);
			$row[] = new htmlDiv(null, $actionElement, array('nowrap'));
			foreach ($this->attrArray as $attributeName) {
				$attributeName = strtolower($attributeName);
				$row[] = $this->getTableCellContent($info[$index], $attributeName);
			}
			$data[] = $row;
		}
	}

	/**
	 * Adds the tool image links (e.g. edit and delete) for each account.
	 *
	 * @param array $account LDAP attributes
	 * @param String $id account ID
	 * @param htmlGroup $element location where to add tools
	 */
	private function addToolLinks($account, $id, &$element) {
		$toolCount = 0;
		// edit link
		$editLink = new htmlLink('', "../account/edit.php?type=" . $this->type->getId() . "&DN='" . rawurlencode($account['dn']) . "'", '../../graphics/edit.png');
		$editLink->setTitle(_("Edit"));
		$element->addElement($editLink);
		$toolCount++;
		// delete link
		if (checkIfWriteAccessIsAllowed($this->type->getId()) && checkIfDeleteEntriesIsAllowed($this->type->getId())) {
			$deleteLink = new htmlLink('', "deletelink.php?type=" . $this->type->getId() . "&DN='" . rawurlencode($account['dn']) . "'", '../../graphics/delete.png');
			$deleteLink->setTitle(_("Delete"));
			$element->addElement($deleteLink);
			$toolCount++;
		}
		// PDF button
		$pdfButton = new htmlButton("createPDF_" . $id, 'pdf.png', true);
		$pdfButton->setTitle(_('Create PDF file'));
		$element->addElement($pdfButton);
		$toolCount++;
		// additional tools
		$tools = $this->getAdditionalTools();
		for ($i = 0; $i < sizeof($tools); $i++) {
			$toolLink = new htmlLink('', $tools[$i]->getLinkTarget() . "?type=" . $this->type->getId() . "&DN='" . rawurlencode($account['dn']) . "'", '../../graphics/' . $tools[$i]->getImage());
			$toolLink->setTitle($tools[$i]->getName());
			$element->addElement($toolLink);
			$toolCount++;
		}
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
	 * Returns the content of a cell in the account list for a given LDAP entry and attribute.
	 *
	 * @param array $entry LDAP attributes
	 * @param string $attribute attribute name
	 * @return htmlElement content
	 */
	protected function getTableCellContent(&$entry, &$attribute) {
		// print all attribute entries seperated by "; "
		if (isset($entry[$attribute]) && sizeof($entry[$attribute]) > 0) {
			if (is_array($entry[$attribute])) {
				if (($attribute == 'entryexpiretimestamp') && !empty($entry[$attribute][0])) {
					return new htmlOutputText(formatLDAPTimestamp($entry[$attribute][0]));
				}
				else {
					// sort array
					sort($entry[$attribute]);
					return new htmlOutputText(implode("; ", $entry[$attribute]));
				}
			}
			else {
				return new htmlOutputText($entry[$attribute]);
			}
		}
	}

	/**
	* Manages all POST actions (e.g. button pressed) for the account lists.
	*
	* @return String HTML fragment to insert into beginning of account list
	*/
	protected function listDoPost() {
		if (!empty($_POST)) {
			validateSecurityToken();
		}
		// check if button was pressed and if we have to add/delete an account or call file upload
		if (isset($_POST['new']) || isset($_POST['del']) || isset($_POST['fileUpload'])){
			if (!checkIfWriteAccessIsAllowed($this->type->getId())) {
				die();
			}
			// add new account
			if (isset($_POST['new']) && checkIfNewEntriesAreAllowed($this->type->getId())){
				metaRefresh("../account/edit.php?type=" . $this->type->getId() . "&amp;suffix=" . $this->suffix);
				exit;
			}
			// delete account(s)
			elseif (isset($_POST['del']) && checkIfDeleteEntriesIsAllowed($this->type->getId())){
				// search for checkboxes
				$accounts = array_keys($_POST, "on");
				// build DN list
				$_SESSION['delete_dn'] = array();
				for ($i = 0; $i < sizeof($accounts); $i++) {
					if ($accounts[$i] == 'tableSelectAll') {
						continue;
					}
					$_SESSION['delete_dn'][] = base64_decode($accounts[$i]);
				}
				if (sizeof($accounts) > 0) {
					metaRefresh("../delete.php?type=" . $this->type->getId());
					exit;
				}
			}
			// file upload
			elseif (isset($_POST['fileUpload']) && checkIfNewEntriesAreAllowed($this->type->getId())){
				metaRefresh("../upload/masscreate.php?type=" . $this->type->getId());
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
			$pdfFont = $_POST['pdf_font'];
			$option = $_POST['createFor'];
			$filename = '';
			// create for clicked account
			if ($option == 'DN') {
				$_SESSION["accountPDF"] = new accountContainer($this->type, "accountPDF");
				$_SESSION["accountPDF"]->load_account(base64_decode($_POST['clickedAccount']));
				$filename = \LAM\PDF\createModulePDF(array($_SESSION["accountPDF"]), $pdfStruct, $pdfFont);
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
					$filename = \LAM\PDF\createModulePDF($list, $pdfStruct, $pdfFont);
					for ($i = 0; $i < sizeof($accounts); $i++) {
						unset($_SESSION["accountPDF-$i"]);
					}
				}
			}
			// create for all accounts
			elseif ($option == 'ALL') {
				$list = array();
				$entriesCount = sizeof($this->entries);
				for ($i = 0; $i < $entriesCount; $i++) {
					$_SESSION["accountPDF-$i"] = new accountContainer($this->type, "accountPDF-$i");
					$_SESSION["accountPDF-$i"]->load_account($this->entries[$i]['dn']);
					$list[$i] = $_SESSION["accountPDF-$i"];
				}
				if (sizeof($list) > 0) {
					$filename = \LAM\PDF\createModulePDF($list, $pdfStruct, $pdfFont);
					for ($i = 0; $i < $entriesCount; $i++) {
						// clean session
						unset($_SESSION["accountPDF-$i"]);
					}
				}
			}
			elseif ($option == 'SESSION') {
				$filename = \LAM\PDF\createModulePDF(array($_SESSION[$_POST['PDFSessionID']]), $pdfStruct, $pdfFont);
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
				setcookie("ListOptions_" . $this->type->getId(), $cookieValue, time()+60*60*24*365, "/", null, null, true);
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
		$pdf_structures = \LAM\PDF\getPDFStructures($this->type->getId());

		$this->printHeader();

		echo "<div class=\"ui-tabs-nav " . $this->type->getScope() . "-bright\">";
		echo "<div class=\"smallPaddingContent\">\n";
		$refresh = '&amp;norefresh=true';
		if (isset($_GET['refresh']) && ($_GET['refresh'] == 'true')) {
			$refresh = '&amp;refresh=true';
		}
		echo "<form action=\"list.php?type=" . $this->type->getId() . $refresh . "\" method=\"post\">\n";

		$container = new htmlResponsiveRow();
		$container->add(new htmlSubTitle(_('Create PDF file')), 12);

		$container->add(new htmlResponsiveSelect('pdf_structure', $pdf_structures, array('default'), _('PDF structure'), '405'), 12);
		$fonts = \LAM\PDF\getPdfFonts();
		$fontSelection = new htmlResponsiveSelect('pdf_font', $fonts, array(), _('Font'), '411');
		$fontSelection->setCSSClasses(array('lam-save-selection'));
		$fontSelection->setHasDescriptiveElements(true);
		$fontSelection->setSortElements(false);
		$container->add($fontSelection, 12);

		$container->addVerticalSpacer('1rem');
		$container->addLabel(new htmlOutputText(_('Create for')));
		// check if account object is already in session
		if ($sessionObject != null) {
			$container->addField(new htmlOutputText($sessionObject->finalDN));
			$container->add(new htmlHiddenInput('createFor', 'SESSION'), 0);
			$container->add(new htmlHiddenInput('PDFSessionID', $PDFSessionID), 0);
		}
		else {
			$radioOptions = array(
				getAbstractDN(base64_decode($id)) => 'DN',
				sprintf(_('All selected accounts (%s)'), sizeof($selAccounts)) => 'SELECTED',
				sprintf(_('All accounts (%s)'), sizeof($this->entries)) => 'ALL'
			);
			$container->addField(new htmlRadio('createFor', $radioOptions, 'DN'));
		}

		$container->addVerticalSpacer('2rem');
		$container->addLabel(new htmlOutputText('&nbsp;', false));
		$buttonContainer = new htmlGroup();
		$buttonContainer->addElement(new htmlButton('createPDFok', _('Ok')));
		$buttonContainer->addElement(new htmlSpacer('0.5rem', null));
		$buttonContainer->addElement(new htmlButton('createPDFCancel', _('Cancel')));
		$container->addField($buttonContainer);
		// hidden inputs for selected accounts
		for ($i = 0; $i < sizeof($selAccounts); $i++) {
			$container->add(new htmlHiddenInput($selAccounts[$i], 'on'), 0);
		}
		$container->add(new htmlHiddenInput('clickedAccount', $id), 0);
		addSecurityTokenToMetaHTML($container);

		parseHtml(null, $container, array(), false, $this->tabindex, $this->type->getScope());

		$this->printFooter();
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
			$suffixSelect->setOnchangeEvent("listOUchanged('" . $this->type->getId() . "', this)");
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
		$row = new htmlResponsiveRow();
		$row->setCSSClasses(array('maxrow'));
		$left = new htmlGroup();
		// button part
		if (checkIfWriteAccessIsAllowed($this->type->getId())) {
			// add button
			if (checkIfNewEntriesAreAllowed($this->type->getId())) {
				$newButton = new htmlButton('new', $this->labels['newEntry']);
				$newButton->setIconClass('createButton');
				$newButton->setCSSClasses(array('fullwidth-mobile-only'));
				$left->addElement($newButton);
			}
			// delete button
			if (!$createOnly && checkIfDeleteEntriesIsAllowed($this->type->getId())) {
				$delButton = new htmlButton('del', $this->labels['deleteEntry']);
				$delButton->setIconClass('deleteButton');
				$delButton->setCSSClasses(array('fullwidth-mobile-only'));
				$left->addElement($delButton);
			}
			$toolSettings = $_SESSION['config']->getToolSettings();
			if ($this->type->getBaseType()->supportsFileUpload() && checkIfNewEntriesAreAllowed($this->type->getId())
					&& !(isset($toolSettings['tool_hide_toolFileUpload']) && ($toolSettings['tool_hide_toolFileUpload'] == 'true'))) {
				$uploadButton = new htmlButton('fileUpload', _('File upload'));
				$uploadButton->setIconClass('upButton');
				$uploadButton->setCSSClasses(array('fullwidth-mobile-only'));
				$left->addElement($uploadButton);
			}
		}

		// OU selection and settings
		$right = new htmlResponsiveRow();
		$right->add($this->listShowOUSelection(), 12, 12, 10);
		$rightButtonGroup = new htmlGroup();
		$refreshButton = new htmlButton('refresh', 'refresh.png', true);
		$refreshButton->setTitle(_("Refresh"));
		$rightButtonGroup->addElement($refreshButton);
		$settingsLink = new htmlLink('', '#', '../../graphics/tools.png');
		$settingsLink->setOnClick('listShowSettingsDialog(\'' . _('Change list settings') . '\', \'' . _('Ok') . '\', \'' . _('Cancel') . '\');');
		$settingsLink->setTitle(_('Change settings'));
		$rightButtonGroup->addElement($settingsLink);
		$right->add($rightButtonGroup, 12, 12, 2);

		$this->addExtraInputElementsToTopArea($left, $right);
		$row->add($left, 12, 6, 6, 'text-left');
		$row->add($right, 12, 6, 6, 'text-right');
		parseHtml(null, $row, array(), false, $this->tabindex, $this->type->getScope());
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
	 * Prints the header part of the page.
	 */
	private function printHeader() {
		include 'adminHeader.inc';
		$this->printHeaderContent();
	}

	/**
	 * Prints any extra HTML for the header part.
	 */
	protected function printHeaderContent() {
		// implemented by child classes if needed
	}

	/**
	 * Prints the footer area of the page.
	 */
	private function printFooter() {
		$this->printFooterContent();
		include 'adminFooter.inc';
	}

	/**
	 * Prints any extra HTML for the footer part.
	 */
	protected function printFooterContent() {
		?>
		<input type="hidden" name="<?php echo getSecurityTokenName(); ?>" value="<?php echo getSecurityTokenValue(); ?>">
			</form></div></div>
			<script type="text/javascript">
				jQuery(document).ready(function() {
					jQuery('#tab_<?php echo $this->type->getId(); ?>').addClass('ui-tabs-active');
					jQuery('#tab_<?php echo $this->type->getId(); ?>').addClass('ui-state-active');
				});
			</script>
		<?php
	}

	/**
	* Returns an hash array containing with all attributes to be shown and their descriptions.
	* Format: array(attribute => description)
	*
	* @return array attribute list
	*/
	protected function listGetAttributeDescriptionList() {
		$attrs = $this->type->getAttributes();
		$ret = array();
		foreach ($attrs as $attr) {
			$ret[$attr->getAttributeName()] = $attr->getAlias();
		}
		return $ret;
	}

	/**
	 * Sets some internal parameters.
	 */
	protected function listGetParams() {
		if (isset($_GET['accountEditBack'])) {
			$this->refresh = true;
			return;
		}
		// check if only PDF should be shown
		if (isset($_GET['printPDF'])) {
			$this->showPDFPage(null);
			exit();
		}
		// get current page
		if (!empty($_GET["page"])) $this->page = $_GET["page"];
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
			$this->sortDirection = htmlspecialchars($_GET['sortdirection']);
		}
		// check search suffix
		if (isset($_POST['suffix'])) {
			// new suffix selected via combobox
			$this->suffix = $_POST['suffix'];
		}
		elseif (isset($_GET['suffix'])) {
			// new suffix selected via combobox
			$this->suffix = $_GET['suffix'];
		}
		elseif (!$this->suffix) {
			// default suffix
			$this->suffix = $this->type->getSuffix();
		}
		// check if LDAP data should be refreshed
		$this->refresh = true;
		if (isset($_GET['norefresh'])) {
			$this->refresh = false;
		}
		if (isset($_POST['refresh'])) {
			$this->refresh = true;
		}
		if (isset($_POST['apply_filter']) || isset($_POST['clear_filter'])) {
			if ($this->serverSideFilterChanged) {
				$this->refresh = true;
			}
		}
	}

	/**
	 * Rereads the entries from LDAP.
	 */
	protected function listRefreshData() {
		// check suffix
		if (!$this->suffix) {
			$this->suffix = $this->type->getSuffix();  // default suffix
		}
		// configure search filter
		$module_filter = get_ldap_filter($this->type->getId());  // basic filter is provided by modules
		$filter = "(&" . $module_filter . $this->buildLDAPAttributeFilter()  . ")";
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
		$this->ldapEntries = searchLDAP($this->suffix, $filter, $attrs);
		$this->entries = array();
		foreach ($this->ldapEntries as $index => &$attrs) {
			$this->entries[$index] = &$attrs;
		}
		$lastError = getLastLDAPError();
		if ($lastError != null) {
			call_user_func_array('StatusMessage', $lastError);
		}
		// generate list of possible suffixes
		$this->possibleSuffixes = $this->type->getSuffixList();
	}

	/**
	 * Builds the LDAP filter based on the filter entries in the GUI.
	 *
	 * @return String LDAP filter
	 */
	protected function buildLDAPAttributeFilter() {
		$text = '';
		foreach ($this->filters as $attr => $filter) {
			if (!$this->isAttributeFilteredByServer($attr)) {
				continue;
			}
			$text .= '(' . $attr . '=' . $filter . ')';
		}
		return $text;
	}

	/**
	 * Specifies if the given attribute name is used for server side filtering (LDAP filter string).
	 *
	 * @param string $attrName attribute name
	 * @return bool filter server side
	 */
	protected function isAttributeFilteredByServer($attrName) {
		return in_array(strtolower($attrName), $this->serverSideFilterAttributes);
	}

	/**
	 * Applies any local filters for attributes that cannot be filtered server side.
	 */
	protected function applyLocalFilters() {
		$this->entries = array();
		foreach ($this->ldapEntries as $index => &$data) {
			$this->entries[$index] = &$data;
		}
		$toFilter = array();
		foreach ($this->filters as $filterAttribute => $filterValue) {
			if ($this->isAttributeFilteredByServer($filterAttribute) || ($filterValue === '')) {
				continue;
			}
			foreach ($this->entries as $index => &$data) {
				if (in_array($index, $toFilter)) {
					continue;
				}
				$regex = str_replace(array('*'), array('.*'), $filterValue);
				$regex = '/^' . $regex . '$/i';
				if (!$this->isFilterMatching($data, $filterAttribute, $regex)) {
					$toFilter[] = $index;
				}
			}
		}
		foreach ($toFilter as $index) {
			unset($this->entries[$index]);
		}
		$this->entries = array_values($this->entries);
	}

	/**
	 * Checks if the given LDAP data matches the filter.
	 *
	 * @param array $data LDAP attributes
	 * @param string $filterAttribute filter attribute name
	 * @param string $regex filter attribute regex
	 */
	protected function isFilterMatching(&$data, $filterAttribute, $regex) {
		if (!isset($data[$filterAttribute])) {
			return false;
		}
		foreach ($data[$filterAttribute] as $value) {
			if (preg_match($regex, $value)) {
				return true;
			}
		}
		return false;
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
		echo "<div id=\"settingsDialog\" class=\"hidden dialog-content\">\n";
		echo "<form id=\"settingsDialogForm\" action=\"list.php?type=" . $this->type->getId() . "&amp;norefresh=true\" method=\"post\">\n";

		$configContainer = new htmlResponsiveRow();
		for ($i = 0; $i < sizeof($this->configOptions); $i++) {
			$configContainer->add($this->configOptions[$i]->getMetaHTML(), 12);
		}
		$configContainer->add(new htmlHiddenInput('saveConfigOptions', 'ok'), 12);
		addSecurityTokenToMetaHTML($configContainer);

		parseHtml('', $configContainer, array(), false, $this->tabindex, $this->type->getScope());

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
				StatusMessage('INFO', sprintf(_('You can download your PDF files {link=%s}{color=#d2131a}here{endcolor}{endlink}.'), $_SESSION['mass_pdf']['file']));
			}
		}
		elseif (isset($_GET['accountEditInvalidID'])) {
			StatusMessage('WARN', _('Please do not edit multiple accounts in parallel in multiple browser tabs.'));
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
	 * @return htmlResponsiveRow meta HTML
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
	 * {@inheritDoc}
	 * @see lamListOption::getMetaHTML()
	 */
	public function getMetaHTML() {
		$return = new htmlResponsiveRow();
		$return->add(new htmlResponsiveInputCheckbox($this->getID(), $this->isSelected(), $this->name), 12);
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
	 * {@inheritDoc}
	 * @see lamListOption::getMetaHTML()
	 */
	public function getMetaHTML() {
		$return = new htmlResponsiveRow();
		$return->add(new htmlResponsiveSelect($this->getID(), $this->options, array($this->getValue()), $this->name, $this->helpID), 12);
		return $return;
	}

}

?>