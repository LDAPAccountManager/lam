<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2010 - 2026  Roland Gruber

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
 * Interface between modules and other parts of LAM.
 *
 * @package metaHTML
 * @author Roland Gruber
 */

/**
 * Returns the marker for required values.
 *
 * @return string HTML code for required marker
 */
function htmlGetRequiredMarker(): string {
	return '<span class="lam-required" title="' . _('required') . '">*</span>';
}

/**
 * Returns the marker for required values.
 *
 * @return htmlSpan HTML code for required marker
 */
function htmlGetRequiredMarkerElement(): htmlSpan {
	$span = new htmlSpan(new htmlOutputText('*'), ['lam-required']);
	$span->setTitle(_('required'));
	return $span;
}

/**
 * Represents a HTML element.
 * This is used to build HTML code by using objects.
 *
 * @package metaHTML
 */
abstract class htmlElement {

	/** align to top */
	public const ALIGN_TOP = 0;
	/** align to left */
	public const ALIGN_LEFT = 1;
	/** align to right */
	public const ALIGN_RIGHT = 2;
	/** align to bottom */
	public const ALIGN_BOTTOM = 3;
	/** align to center */
	public const ALIGN_CENTER = 4;

	/** alignment when inside a table */
	public ?int $alignment = null;
	/** colspan if inside a table */
	public ?int $colspan = null;
	/** rowspan if inside a table */
	public ?int $rowspan = null;
	/** @var string[] $cssClasses CSS classes */
	protected ?array $cssClasses = [];
	/** @var string[] $tableCellCssClasses table cell CSS classes */
	protected ?array $tableCellCssClasses = [];
	/** @var array<string, string> $dataAttributes data attributes */
	private array $dataAttributes = [];
	/** accessibility label */
	private ?string $accessibilityLabel = null;

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string|null $module Name of account module
	 * @param htmlElement[] $input List of meta-HTML elements
	 * @param array<string, string[]> $values List of values which override the defaults in $input (name => value)
	 * @param bool $restricted If true then no buttons will be displayed
	 * @param string|null $scope account type
	 * @return array<string, string> List of input field names and their type (name => type)
	 */
	abstract public function generateHTML($module, $input, $values, $restricted, $scope): array;

	/**
	 * Returns the HTML attributes for the alignment.
	 *
	 * @return string alignment HTML attributes (e.g. align="right" valign="top")
	 */
	public function getAlignmentString(): string {
		$align = '';
		if ($this->alignment !== null) {
			switch ($this->alignment) {
				case htmlElement::ALIGN_BOTTOM:
					$align = 'valign="bottom"';
					break;
				case htmlElement::ALIGN_TOP:
					$align = 'valign="top"';
					break;
				case htmlElement::ALIGN_LEFT:
					$align = 'align="left"';
					break;
				case htmlElement::ALIGN_RIGHT:
					$align = 'align="right"';
					break;
				case htmlElement::ALIGN_CENTER:
					$align = 'align="center"';
					break;
			}
		}
		return $align;
	}

	/**
	 * Returns the HTML attribute for the colspan.
	 *
	 * @return string colspan HTML attribute (e.g. colspan=3)
	 */
	public function getColspanString(): string {
		if ($this->colspan === null) {
			return '';
		}
		else {
			return 'colspan="' . $this->colspan . '"';
		}
	}

	/**
	 * Returns the HTML attribute for the rowspan.
	 *
	 * @return string rowspan HTML attribute (e.g. rowspan=3)
	 */
	public function getRowspanString(): string {
		if ($this->rowspan === null) {
			return '';
		}
		else {
			return 'rowspan="' . $this->rowspan . '"';
		}
	}

	/**
	 * Returns the CSS classes of this element.
	 *
	 * @return string[]|null $classes CSS class names
	 */
	public function getCSSClasses(): ?array {
		return $this->cssClasses;
	}

	/**
	 * Adds CSS classes to this element.
	 *
	 * @param string[]|null $classes CSS class names
	 */
	public function setCSSClasses(?array $classes): void {
		$this->cssClasses = $classes;
	}

	/**
	 * Adds CSS classes to the surrounding table cell for this element.
	 *
	 * @param string[]|null $classes CSS class names
	 */
	public function setTableCellCSSClasses(?array $classes): void {
		$this->tableCellCssClasses = $classes;
	}

	/**
	 * Returns the CSS classes of the surrounding table cell for this element.
	 *
	 * @return string[]|null CSS classes
	 */
	public function getTableCellCSSClasses(): ?array {
		return $this->tableCellCssClasses;
	}

	/**
	 * Adds a data attribute.
	 *
	 * @param string $key attribute name (without "data-")
	 * @param string $value attribute value
	 */
	public function addDataAttribute(string $key, string $value): void {
		$this->dataAttributes[$key] = $value;
	}

	/**
	 * Returns the data attributes as rendered string.
	 *
	 * @return string data attributes
	 */
	protected function getDataAttributesAsString(): string {
		$result = '';
		foreach ($this->dataAttributes as $key => $value) {
			$result .= ' data-' . htmlspecialchars($key) . '="' . htmlspecialchars($value) . '"';
		}
		return $result;
	}

	/**
	 * Sets the accessibility label.
	 *
	 * @param string|null $accessibilityLabel label
	 */
	public function setAccessibilityLabel(?string $accessibilityLabel): void {
		$this->accessibilityLabel = $accessibilityLabel;
	}

	/**
	 * Returns the markup for the accessibility data.
	 *
	 * @return string markup to be included in HTML tag (starting with space)
	 */
	public function getAccessibilityMarkup(): string {
		if (empty($this->accessibilityLabel)) {
			return '';
		}
		return ' aria-label="' . htmlspecialchars($this->accessibilityLabel) . '"';
	}

}

/**
 * Structures elements using a table.
 *
 * @package metaHTML
 */
class htmlTable extends htmlElement {

	/** table footer */
	private const FOOTER = "</table>\n";
	/** new line */
	private const NEWLINE = "</tr><tr>\n";

	/** @var array<string|htmlElement> $elements list of subelements */
	private array $elements = [];
	/** specifies if currently a row is open */
	private bool $rowOpen = false;
	/** table width */
	private ?string $width;
	/** HTML ID */
	private ?string $id;

	/**
	 * Constructor
	 *
	 * @param string|null $width table width (e.g. 100%)
	 * @param string|null $id id
	 * @see htmlElement
	 */
	public function __construct(?string $width = null, ?string $id = null) {
		$this->width = $width;
		$this->id = $id;
	}

	/**
	 * Adds an element to the table.
	 *
	 * @param htmlElement $element htmlElement object or a simple string
	 * @param bool $newLine adds a new line after the element (optional, default false)
	 * @param bool $isTableHeadElement specifies if this is a head or body element (default: body)
	 */
	public function addElement(htmlElement $element, bool $newLine = false, bool $isTableHeadElement = false): void {
		// check if a row needs to be opened
		if (!$this->rowOpen) {
			$this->elements[] = "<tr>\n";
			$this->rowOpen = true;
		}
		// check if alignment option was given
		$align = $element->getAlignmentString();
		$colspan = $element->getColspanString();
		$rowspan = $element->getRowspanString();
		$css = '';
		if (count($element->getTableCellCSSClasses()) > 0) {
			$css = 'class="' . implode(' ', $element->getTableCellCSSClasses()) . '"';
		}
		$tagName = 'td';
		if ($isTableHeadElement) {
			$tagName = 'th';
		}
		$this->elements[] = "<$tagName $align $colspan $rowspan $css>\n";
		$this->elements[] = $element;
		$this->elements[] = "</$tagName>\n";
		if ($newLine) {
			$this->addNewLine();
		}
	}

	/**
	 * Adds another line to the table.
	 */
	public function addNewLine(): void {
		$this->elements[] = $this->rowOpen ? self::NEWLINE : "<tr>\n";
	}

	/**
	 * Adds an htmlSpacer with the given width.
	 *
	 * @param string $width width (e.g. 10px)
	 */
	public function addSpace(string $width): void {
		$this->addElement(new htmlSpacer($width, null));
	}

	/**
	 * Adds an htmlSpacer with the given height and ends the row.
	 *
	 * @param string $height height (e.g. 10px)
	 */
	public function addVerticalSpace(string $height): void {
		$this->addElement(new htmlSpacer(null, $height), true);
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		$return = [];
		$width = '';
		if ($this->width != null) {
			$width = ' width="' . htmlspecialchars($this->width) . '"';
		}
		$id = '';
		if (!empty($this->id)) {
			$id = ' id="' . $this->id . '"';
		}
		$classAttr = '';
		if (count($this->cssClasses) > 0) {
			$classAttr = ' class="' . implode(' ', $this->cssClasses) . '"';
		}
		echo "<table" . $width . $id . $classAttr . ">\n";
		// print all contained elements
		for ($i = 0; $i < count($this->elements); $i++) {
			// print htmlElement objects
			if ($this->elements[$i] instanceof htmlElement) {
				$fields = $this->elements[$i]->generateHTML($module, $input, $values, $restricted, $scope);
				$return = array_merge($return, $fields);
			}
			elseif ($i !== count($this->elements) - 1 || ($this->elements[$i] != self::NEWLINE)) {
				echo $this->elements[$i];
			}
		}
		if ($this->rowOpen) {
			echo "</tr>\n";
		}
		echo self::FOOTER;
		return $return;
	}

	/**
	 * Merges the content of another htmlTable object into this table.
	 *
	 * @param htmlTable $table table to get elements
	 */
	public function mergeTableElements(htmlTable $table): void {
		// remove obsolete new lines at the end
		if ($table->elements[count($table->elements) - 1] == self::NEWLINE) {
			unset($table->elements[count($table->elements) - 1]);
		}
		// close last row of other table if needed
		if ($table->rowOpen) {
			$table->elements[] = "</tr>\n";
		}
		// close last own row if needed
		if ($this->rowOpen) {
			if ($this->elements[count($this->elements) - 1] == self::NEWLINE) {
				unset($this->elements[count($this->elements) - 1]);
			}
			else {
				$this->elements[] = "</tr>\n";
			}
			$this->rowOpen = false;
		}
		$this->elements = array_merge($this->elements, $table->elements);
	}

}

/**
 * Table component for client-side controlled data tables.
 */
class htmlDataTable extends htmlElement {

	public const DATA_AJAX_URL = 'ajaxurl';
	public const DATA_TOKEN_NAME = 'tokenname';
	public const DATA_TOKEN_VALUE = 'tokenvalue';
	public const DATA_ACTION = 'action';
	public const DATA_OK_TEXT = 'oktext';
	public const SORT_ASC = 'asc';
	public const SORT_DESC = 'desc';

	private string $id;
	private mixed $height;
	/** @var htmlDataTableColumn[] $columns columns */
	private array $columns;
	private string $searchFieldId = '';
	private string $onRowClick = '';
	private ?string $initialSortColumn = null;
	private ?string $initialSortOrder = null;

	/**
	 * Constructor
	 *
	 * @param string $id table ID
	 * @param htmlDataTableColumn[] $columns columns
	 * @param int|string $height table height
	 */
	public function __construct(string $id, array $columns, mixed $height = 300) {
		$this->id = htmlspecialchars($id);
		$this->height = $height;
		$this->columns = $columns;
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		echo '<link href="../../style/tabulator/tabulator.css" rel="stylesheet">';
		echo '<script type="text/javascript" src="../../templates/lib/extra/tabulator/tabulator.js"></script>';
		echo '<div id="' . $this->id . '" class="lam-data-table" ' . $this->getDataAttributesAsString() . '></div>';
		$columnOptions = [];
		foreach ($this->columns as $column) {
			$headerFilter = $column->filterable ? ', headerFilter: "input"' : '';
			$columnOptions[] = '{
				title: "' . $column->label . '",
				field: "' . $column->name . '",
				headerTooltip: "' . $column->label . '",
				formatter: "' . $column->getFormatter() . '",
				visible: ' . ($column->visible ? 'true' : 'false') . ',
				formatterParams: ' . json_encode((object) $column->getFormatterOptions()) . '
				' . $headerFilter . '
			}';
		}
		$columnsCode = implode(', ', $columnOptions);
		$searchFilterCode = '';
		if (!empty($this->searchFieldId)) {
			$searchFilterCode = 'window.lam.datatable.addSearchField("' . $this->id . '", "' . $this->searchFieldId . '");';
		}
		$onRowClickCode = '';
		if (!empty($this->onRowClick)) {
			$onRowClickCode = 'window.lam.datatable.tables["' . $this->id . '"].on("rowClick", ' . $this->onRowClick . ');';
		}
		$initialSortingCode = '';
		if (!empty($this->initialSortColumn)) {
			$initialSortingCode = 'initialSort: [{column: "' . $this->initialSortColumn . '", dir: "' . $this->initialSortOrder . '"}],';
		}
		echo '<script type="text/javascript">
			window.lam.datatable.init("' . $this->id . '", new Tabulator("#' . $this->id . '", {
                height: "' . $this->height . '",
                layout: "fitColumns",
                pagination:"local",
				paginationSize:50,
				paginationSizeSelector:[50, 100, 250, 500],
				columns: [' . $columnsCode . '],
				data: [],
				locale: "en-gb",
				' . $initialSortingCode . '
				langs: {
                    "en-gb": {
                        "pagination": {
 			               "page_size": "",
			                "page_title": "",
            			    "first": "↤",
							"first_title": "",
							"last": "↦",
							"last_title": "",
							"prev": "←",
							"prev_title": "",
							"next": "→",
							"next_title": ""                            
                        }
                    }
				}
			}));
			' . $searchFilterCode . '
			' . $onRowClickCode . '
		</script>';
		return [];
	}

	/**
	 * Sets the search field.
	 *
	 * @param string $searchFieldId ID of field
	 */
	public function setSearchField(string $searchFieldId): void {
		$this->searchFieldId = $searchFieldId;
	}

	/**
	 * Sets the row click handler.
	 *
	 * @param string $callback callback code
	 */
	public function setOnRowClick(string $callback): void {
		$this->onRowClick = $callback;
	}

	/**
	 * Sets the initial sort order.
	 *
	 * @param string $columnName column name
	 * @param string $sortOrder htmlDataTable::SORT_ASC|htmlDataTable::SORT_DESC
	 */
	public function setInitialSorting(string $columnName, string $sortOrder): void {
		if (in_array($sortOrder, [self::SORT_ASC, self::SORT_DESC])) {
			$this->initialSortColumn = $columnName;
			$this->initialSortOrder = $sortOrder;
		}
	}

}

/**
 * Column for data table.
 */
class htmlDataTableColumn {

	public const TYPE_TEXT = 'text';
	public const TYPE_EMAIL = 'email';
	public const TYPE_TELEPHONE = 'telephone';
	public const TYPE_IMAGE = 'image';
	public const TYPE_HTML = 'html';

	public string $label;
	public string $name;
	public bool $filterable;
	public string $formattingType;
	public bool $visible;

	/**
	 * @param string $label column label text
	 * @param string $name column ID
	 * @param bool $filterable can be filtered
	 * @param string $formattingType formatting type
	 * @param bool $visible column is visible
	 */
	public function __construct(string $label, string $name, bool $filterable = true, string $formattingType = self::TYPE_TEXT, bool $visible = true) {
		$this->label = htmlspecialchars($label);
		$this->name = htmlspecialchars($name);
		$this->filterable = $filterable;
		$this->formattingType = $formattingType;
		$this->visible = $visible;
	}

	/**
	 * Returns the Tabulator formatter name.
	 *
	 * @return string name
	 */
	public function getFormatter(): string {
		return match ($this->formattingType) {
			self::TYPE_EMAIL, self::TYPE_TELEPHONE => 'link',
			self::TYPE_IMAGE => 'image',
			self::TYPE_HTML => 'html',
			default => 'textarea',
		};
	}

	/**
	 * Returns the Tabulator formatter options.
	 *
	 * @return array<string, string> options
	 */
	public function getFormatterOptions(): array {
		return match ($this->formattingType) {
			self::TYPE_EMAIL => ['urlPrefix' => 'mailto:', 'target' => '_blank'],
			self::TYPE_TELEPHONE => ['urlPrefix' => 'tel:', 'target' => '_blank'],
			default => [],
		};
	}

}

/**
 * A standard input field.
 *
 * @package metaHTML
 */
class htmlInputField extends htmlElement {

	/** unique field name */
	protected string $fieldName;
	/** field value */
	protected ?string $fieldValue = '';
	/** field size (default 30) */
	protected ?int $fieldSize = 30;
	/** field max length (default 1000) */
	protected ?int $fieldMaxLength = 1000;
	/** on keypress event */
	protected ?string $onKeyPress = null;
	/** on keyupp event */
	protected ?string $onKeyUp = null;
	/** oninput event */
	protected ?string $onInput = null;
	/** password field */
	protected bool $isPassword = false;
	/** check password strength */
	protected bool $checkPasswordStrength = false;
	/** disables browser autofilling of password fields */
	protected bool $disableAutoFill = false;
	/** enabled or disabled */
	protected bool $isEnabled = true;
	/** indicates that the value should be saved in obfuscated form */
	protected bool $obfuscate = false;
	/** indicates that this field should not automatically be saved in the self-service or server profile */
	protected bool $transient = false;
	/** required field */
	protected bool $required = false;
	/** enable autocomplete */
	protected bool $autocomplete = false;
	/** @var string[] $autocompleteValues autocompletion suggestions */
	protected array $autocompleteValues = [];
	/** show calendar */
	protected bool $showCalendar = false;
	/** show DN selection */
	protected bool $showDnSelection = false;
	/** calendar format */
	protected string $calendarFormat = '';
	/** calendar with time */
	protected bool $calendarFormatWithTime = false;
	/** calendar with seconds */
	protected bool $calendarFormatWithSeconds = false;
	/** title attribute */
	protected ?string $title = null;
	/** field ID that needs to have same value (e.g. password field) */
	protected ?string $sameValueFieldID = null;
	/** marks the input field as auto trimmed (remove spaces at start/end) */
	protected bool $autoTrim = true;
	/** minimum value */
	protected ?int $minValue = null;
	/** maximum value */
	protected ?int $maxValue = null;
	/** step for numeric values */
	protected ?int $stepValue = null;
	/** id */
	protected ?string $id = null;
	/** input type */
	protected string $type = 'text';
	/** validation pattern */
	protected ?string $validationPattern = null;

	/**
	 * Constructor
	 *
	 * @param string $fieldName unique field name
	 * @param string|null $fieldValue value of input field (optional)
	 * @param int|null $fieldSize input field length (default 30)
	 */
	public function __construct(string $fieldName, ?string $fieldValue = null, ?int $fieldSize = null) {
		if (isObfuscatedText($fieldValue)) {
			$fieldValue = deobfuscateText($fieldValue);
		}
		$this->fieldName = htmlspecialchars($fieldName);
		if ($fieldValue !== null) {
			$this->fieldValue = htmlspecialchars($fieldValue);
		}
		if ($fieldSize !== null) {
			$this->fieldSize = $fieldSize;
		}
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		if ($this->isAutoTrim()) {
			$this->cssClasses[] = 'lam-autotrim';
		}
		if (isset($values[$this->fieldName])) {
			if (isObfuscatedText($values[$this->fieldName][0])) {
				$this->fieldValue = htmlspecialchars(deobfuscateText($values[$this->fieldName][0]));
			}
			else {
				$this->fieldValue = htmlspecialchars($values[$this->fieldName][0]);
			}
		}
		$min = '';
		if ($this->minValue !== null) {
			$min = ' min="' . $this->minValue . '"';
		}
		$max = '';
		if ($this->maxValue !== null) {
			$max = ' max="' . $this->maxValue . '"';
		}
		$step = '';
		if ($this->stepValue !== null) {
			$step = ' step="' . $this->stepValue . '"';
		}
		$validationPattern = '';
		if ($this->validationPattern !== null) {
			$validationPattern = ' pattern="' . $this->validationPattern . '"';
		}
		// print input field
		$required = '';
		if ($this->required) {
			$required = ' required';
		}
		$class = ' class="' . implode(' ', $this->cssClasses) . '"';
		$name = ' name="' . $this->fieldName . '"';
		$idValue = $this->id ?? $this->fieldName;
		$id = ' id="' . $idValue . '"';
		$value = '';
		if ($this->fieldValue !== null) {
			$value = ' value="' . $this->fieldValue . '"';
		}
		$maxLength = '';
		if ($this->fieldMaxLength !== null) {
			$maxLength = ' maxlength="' . $this->fieldMaxLength . '"';
		}
		$size = '';
		if ($this->fieldSize !== null) {
			$size = ' size="' . $this->fieldSize . '"';
		}
		$inputType = $this->type;
		if ($this->isPassword) {
			$inputType = 'password';
		}
		elseif (($this->minValue !== null) || ($this->maxValue !== null)) {
			$inputType = 'number';
		}
		$disabled = '';
		if (!$this->isEnabled) {
			$disabled = ' disabled';
		}
		$onKeyPress = '';
		if ($this->onKeyPress != null) {
			$onKeyPress = ' onkeypress="' . $this->onKeyPress . '"';
		}
		$onKeyUp = '';
		if ($this->onKeyUp != null) {
			$onKeyUp = ' onkeyup="' . $this->onKeyUp . '"';
		}
		$onInput = '';
		if ($this->onInput != null) {
			$onInput = ' oninput="' . $this->onInput . '"';
		}
		$title = '';
		if (!empty($this->title)) {
			$title = ' title="' . $this->title . '"';
		}
		$autoCompleteVal = '';
		if ($this->disableAutoFill) {
			$autoCompleteVal = ' autocomplete="new-password"';
		}
		$autoCompleteData = '';
		if ($this->autocomplete && !empty($idValue)) {
			if (($this->fieldValue !== null) && !in_array($this->fieldValue, $this->autocompleteValues)) {
				array_unshift($this->autocompleteValues, $this->fieldValue);
			}
			echo '<datalist id="datalist_' . $idValue . '" autocomplete="off">';
			foreach ($this->autocompleteValues as $autocompleteValue) {
				echo '<option value="' . $autocompleteValue . '">';
			}
			echo '</datalist>';
			$autoCompleteData = ' list="datalist_' . $idValue . '"';
		}
		if ($this->showDnSelection) {
			echo '<span class="nowrap">';
		}
		if ($this->checkPasswordStrength) {
			echo '<div id="' . $idValue . '-pwd-strength-issues" class="lam-password-strength"></div>';
		}
		echo '<input type="' . $inputType . '"' . $class . $name . $id . $value . $maxLength . $required
			. $min . $max . $step . $validationPattern . $size . $onKeyPress . $onKeyUp
			. $onInput . $title . $disabled . $this->getAccessibilityMarkup()
			. $this->getDataAttributesAsString() . $autoCompleteVal . $autoCompleteData . '>';
		if ($this->showDnSelection) {
			echo '<img class="align-middle" src="../../graphics/search-color.svg"
					   width="16" height="16" title="' . _('Choose entry') . '"
					   onclick="window.lam.html.showDnSelection(\'' . $this->fieldName . '\', \'' . _('Choose entry') . '\'
					   			, \'' . _('Ok') . '\', \'' . _('Cancel') . '\', \'' . getSecurityTokenName() . '\'
					   			, \'' . getSecurityTokenValue() . '\');">';
			echo '</span>';
		}
		// calendar
		if ($this->showCalendar) {
			$locale = 'en';
			if (!empty($_SESSION['language'])) {
				$sessionLanguage = $_SESSION['language'];
				$sessionLanguage = str_replace('.utf8', '', $sessionLanguage);
				if ($sessionLanguage === 'zh_TW') {
					$locale = 'zh_tw';
				}
				else {
					$sessionLanguageParts = explode('_', $sessionLanguage);
					$locale = $sessionLanguageParts[0];
				}
			}
			$calendarWithTime = $this->calendarFormatWithTime ? "enableTime: true, " : "";
			$calendarWithSeconds = $this->calendarFormatWithSeconds ? "enableSeconds: true, " : "";
			echo '<script type="text/javascript">
				flatpickr("#' . $this->fieldName . '", {
					dateFormat: "' . $this->calendarFormat . '",
					hourIncrement: 1,
					minuteIncrement: 1,
					time_24hr: true,
					' . $calendarWithTime . $calendarWithSeconds . '
					locale: "' . $locale . '"
				});
				</script>
			';
		}
		// check value against reference field
		if ($this->sameValueFieldID != null) {
			echo '<script type="text/javascript">
					checkFieldsHaveSameValues("' . $this->fieldName . '", "' . $this->sameValueFieldID . '");
				</script>
			';
		}
		if ($this->checkPasswordStrength) {
			$query = '?admin=1';
			if (isSelfService()) {
				$query = '?selfservice=1';
			}
			$ajaxPath = "../templates/misc/ajax.php";
			if (is_file("../../templates/misc/ajax.php")) {
				$ajaxPath = "../../templates/misc/ajax.php";
			}
			elseif (is_file("../../../templates/misc/ajax.php")) {
				$ajaxPath = "../../../templates/misc/ajax.php";
			}
			$ajaxPath .= $query;
			echo '<script type="text/javascript">
					checkPasswordStrength("' . $this->fieldName . '", "' . $ajaxPath . '", "' . getSecurityTokenName() . '", "' . getSecurityTokenValue() . '");
				</script>
			';
		}
		if ($this->transient) {
			return [];
		}
		if ($this->obfuscate) {
			return [$this->fieldName => 'text_obfuscated'];
		}
		else {
			return [$this->fieldName => 'text'];
		}
	}

	/**
	 * Sets the maximum field length.
	 *
	 * @param int|null $fieldMaxLength length
	 */
	public function setFieldMaxLength(?int $fieldMaxLength): void {
		$this->fieldMaxLength = $fieldMaxLength;
	}

	/**
	 * Sets the field size (default is 30).
	 *
	 * @param int|null $fieldSize size
	 */
	public function setFieldSize(?int $fieldSize): void {
		$this->fieldSize = $fieldSize;
	}

	/**
	 * Specifies if this is a password field.
	 *
	 * @param bool $isPassword password field
	 * @param bool $checkStrength check if matches password policy (default: false)
	 * @param bool $disableAutoFill prevent autofilling by browser
	 */
	public function setIsPassword(bool $isPassword, bool $checkStrength = false, bool $disableAutoFill = false): void {
		$this->isPassword = $isPassword;
		$this->checkPasswordStrength = $checkStrength;
		$this->disableAutoFill = $disableAutoFill;
	}

	/**
	 * Specifies if this component is enabled and accepts user modification.
	 *
	 * @param bool $isEnabled enabled if true
	 */
	public function setIsEnabled(bool $isEnabled): void {
		$this->isEnabled = $isEnabled;
	}

	/**
	 * Specifies if the value should be saved in obfuscated form (e.g. self-service profile).
	 *
	 * @param bool $obfuscate obfuscate value
	 */
	public function setObfuscate(bool $obfuscate): void {
		$this->obfuscate = $obfuscate;
	}

	/**
	 * Specifies that the value should not be automatically saved when used in self-service or server profile (default: false).
	 *
	 * @param bool $transient transient field
	 */
	public function setTransient(bool $transient): void {
		$this->transient = $transient;
	}

	/**
	 * Specifies if the input field is required.
	 *
	 * @param bool $required required
	 */
	public function setRequired(bool $required): void {
		$this->required = $required;
	}

	/**
	 * Sets the field as number field with minimum and maximum values.
	 *
	 * @param int $minimum minimum
	 * @param int|null $maximum maximum
	 * @param int $step step value
	 */
	public function setMinimumAndMaximumNumber(int $minimum = 0, ?int $maximum = null, int $step = 1): void {
		$this->minValue = $minimum;
		$this->maxValue = $maximum;
		$this->stepValue = $step;
	}

	/**
	 * Enables autocompletion for this input field.
	 *
	 * @param string[] $values list of values to suggest
	 */
	public function enableAutocompletion(array $values): void {
		$this->autocomplete = true;
		$this->autocompleteValues = $values;
	}

	/**
	 * Sets the JavaScript for the onKeyPress event.
	 *
	 * @param string $onKeyPress JavaScript code
	 */
	public function setOnKeyPress(string $onKeyPress): void {
		$this->onKeyPress = $onKeyPress;
	}

	/**
	 * Sets the JavaScript for the onKeyUp event.
	 *
	 * @param string $onKeyUp JavaScript code
	 */
	public function setOnKeyUp(string $onKeyUp): void {
		$this->onKeyUp = $onKeyUp;
	}

	/**
	 * Sets the JavaScript for the onInput event.
	 *
	 * @param string $onInput JavaScript code
	 */
	public function setOnInput(string $onInput): void {
		$this->onInput = $onInput;
	}

	/**
	 * Shows a calendar when the field is selected.
	 *
	 * @param string $format calendar format (e.g. "Y-m-d" or "Y-m-d H:i:S")
	 * @param bool $withTime activate time selection (default: false)
	 * @param bool $withSeconds show seconds (default: false)
	 */
	public function showCalendar(string $format, bool $withTime = false, bool $withSeconds = false): void {
		$this->showCalendar = true;
		$this->calendarFormatWithTime = $withTime;
		$this->calendarFormatWithSeconds = $withSeconds;
		$format = str_replace('yy', 'Y', $format);
		$format = str_replace('dd', 'd', $format);
		$format = str_replace('mm', 'm', $format);
		$this->calendarFormat = str_replace('\\', '\\\\', $format);
	}

	/**
	 * Shows a DN selection next to input field.
	 */
	public function showDnSelection(): void {
		$this->showDnSelection = true;
	}

	/**
	 * Sets the title for the input field.
	 *
	 * @param string $title title value
	 */
	public function setTitle(string $title): void {
		$this->title = htmlspecialchars($title);
	}

	/**
	 * Specifies the ID of a second field that must have the same value as this field.
	 * This field is marked red if different or green if equal.
	 *
	 * @param string $sameValueFieldID ID of reference field
	 */
	public function setSameValueFieldID(string $sameValueFieldID): void {
		$this->sameValueFieldID = $sameValueFieldID;
	}

	/**
	 * Turns this field into a live filter for a select box.
	 * Cannot be used together with setOnKeyUp().
	 *
	 * @param string $name select box name
	 */
	public function filterSelectBox(string $name): void {
		$this->setOnKeyUp('window.lam.filterSelect.activate(\'' . $this->fieldName . '\', \'' . $name . '\', event);');
	}

	/**
	 * Returns if the field content should be auto-trimmed (remove spaces at start/end).
	 *
	 * @return bool auto-trim input
	 */
	protected function isAutoTrim(): bool {
		return $this->autoTrim && !$this->isPassword;
	}

	public function disableAutoTrim(): void {
		$this->autoTrim = false;
	}

	/**
	 * Sets the element ID.
	 *
	 * @param string $id id
	 */
	public function setId(string $id): void {
		$this->id = $id;
	}

	/**
	 * Sets the input type.
	 *
	 * @param string $type type (e.g. number)
	 */
	public function setType(string $type): void {
		$this->type = $type;
	}

	/**
	 * Sets the validation pattern.
	 *
	 * @param string $pattern pattern (e.g. "[0-9]{3}")
	 */
	public function setValidationPattern(string $pattern): void {
		$this->validationPattern = $pattern;
	}

	/**
	 * Sets the step value for number fields.
	 *
	 * @param int $step step value (e.g. 1)
	 */
	public function setStepValue(int $step): void {
		$this->stepValue = $step;
	}

}

/**
 * Renders a help link.
 *
 * @package metaHTML
 */
class htmlHelpLink extends htmlElement {

	/** help ID */
	private string $helpID;
	/** module name if it should be forced */
	private ?string $module;
	/** account type if it should be forced */
	private ?string $scope;

	/**
	 * Constructor
	 *
	 * @param string $helpID help ID
	 * @param string|null $module module name (optional, only if value from generateHTML() should be overwritten)
	 * @param string|null $scope account type (e.g. user) (optional, only if value from generateHTML() should be overwritten)
	 */
	public function __construct(string $helpID, ?string $module = null, ?string $scope = null) {
		$this->helpID = $helpID;
		$this->module = $module;
		$this->scope = $scope;
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		// overwrite module and scope if needed
		if ($this->module != null) {
			$module = $this->module;
		}
		// print link
		$helpEntry = getHelp($module, $this->helpID);
		if (empty($helpEntry)) {
			return [];
		}
		printHelpLink($helpEntry, $this->helpID, $module, $this->cssClasses);
		return [];
	}

}

/**
 * Simple button.
 *
 * @package metaHTML
 */
class htmlButton extends htmlElement {

	/** button name */
	protected string $name;
	/** button text or image */
	protected string $value;
	/** image button or text button */
	protected bool $isImageButton;
	/** title */
	private ?string $title = null;
	/** enabled or disabled */
	private bool $isEnabled = true;
	/** onclick event */
	private ?string $onClick = null;
	/** button type (default: "submit" if no onClick and "button" with onClick) */
	private ?string $type = null;
	/** disable form validation */
	private bool $noFormValidation = false;

	/**
	 * Constructor.
	 *
	 * @param string $name button name
	 * @param string $value button text or image (16x16px, relative to graphics folder)
	 * @param bool $isImageButton image or text button (default text)
	 */
	public function __construct(string $name, string $value, bool $isImageButton = false) {
		$this->name = htmlspecialchars($name);
		$this->value = htmlspecialchars($value);
		$this->isImageButton = $isImageButton;
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		if ($restricted) {
			// no buttons in restricted mode
			logNewMessage(LOG_ERR, 'Meta HTML: Requested button in restricted mode.');
			return [];
		}
		$style = '';
		$classList = $this->cssClasses;
		$class = '';
		$title = '';
		$name = ' name="' . $this->name . '"';
		// image button
		if ($this->isImageButton) {
			$classList[] = 'smallImageButton';
			$classList[] = 'align-middle';
			$style = ' style="background-image: url(../../graphics/' . $this->value . ');"';
		}
		// button with text and icon
		else {
			$classList[] = 'margin5';
		}
		$class = ' class="' . implode(' ', $classList) . '"';
		if ($this->title != null) {
			$title = ' title="' . $this->title . '"';
		}
		$disabled = '';
		if (!$this->isEnabled) {
			$disabled = ' disabled';
		}
		$noFormValidation = '';
		if ($this->noFormValidation) {
			$noFormValidation = ' formnovalidate="formnovalidate"';
		}
		$type = $this->type === null ? ' type="submit"' : ' type="' . $this->type . '"';
		$onClick = '';
		if ($this->onClick !== null) {
			if ($this->type === null) {
				$type = ' type="button"';
			}
			$onClick = ' onclick="' . $this->onClick . '"';
		}
		$id = ' id="btn_' . preg_replace('/[^a-zA-Z0-9_-]/', '', $this->name) . '"';
		if ($this->isImageButton) {
			echo '<input type="submit" ' . $id . ' value=" "' . $name . $onClick . $style . $class
				. $title . $disabled . $noFormValidation . $this->getDataAttributesAsString() . '>';
		}
		else {
			echo '<button' . $id . $name . $type . $onClick . $style . $class . $title . $disabled
				. $noFormValidation . $this->getDataAttributesAsString() . '>' . $this->value . '</button>';
		}
		return [$this->name => 'submit'];
	}

	/**
	 * Sets the button title (tooltip).
	 *
	 * @param string|null $title title
	 */
	public function setTitle(?string $title): void {
		if ($title !== null) {
			$this->title = htmlspecialchars($title);
		}
	}

	/**
	 * Specifies if this component is enabled and accepts user modification.
	 *
	 * @param bool $isEnabled enabled if true
	 */
	public function setIsEnabled(bool $isEnabled): void {
		$this->isEnabled = $isEnabled;
	}

	/**
	 * Sets the onclick event code.
	 * This makes this button a simple button that does not submit a form.
	 *
	 * @param string $onClick JS code
	 */
	public function setOnClick(string $onClick): void {
		$this->onClick = $onClick;
	}

	/**
	 * Allows to override the default button type ("submit" if no onClick and "button" with onClick).
	 *
	 * @param string $type type (e.g. button)
	 */
	public function setType(string $type): void {
		$this->type = $type;
	}

	/**
	 * Disables form validation (e.g. for cancel buttons).
	 */
	public function disableFormValidation(): void {
		$this->noFormValidation = true;
	}

}

/**
 * Prints a button for the account pages.
 *
 * @package metaHTML
 */
class htmlAccountPageButton extends htmlButton {

	/**
	 * Constructor
	 *
	 * @param string $targetModule module name which renders next page
	 * @param string $targetPage name of next page
	 * @param string $identifier identifier for button
	 * @param string $value button text or image (16x16px, relative to graphics folder)
	 * @param bool $isImageButton image or text button (default text)
	 * @param string|null $title title to show
	 */
	public function __construct(string $targetModule, string $targetPage, string $identifier, string $value, bool $isImageButton = false, ?string $title = null) {
		parent::__construct('form_subpage_' . $targetModule . '_' . $targetPage . '_' . $identifier, $value, $isImageButton);
		if ($title !== null) {
			$this->setTitle($title);
		}
	}

}

/**
 * Represents a select box.
 *
 * @package metaHTML
 */
class htmlSelect extends htmlElement {

	/** name of select field */
	protected string $name;
	/** size */
	private string $size;
	/** allows multi-selection */
	private bool $multiSelect = false;
	/** @var string[]|array<string, string>|array<string, string[]|array<string, string>> $elements elements */
	private array $elements;
	/** @var string[] $selectedElements selected elements */
	private array $selectedElements = [];
	/** descriptive elements */
	private bool $hasDescriptiveElements = false;
	/** contains optgroups */
	private bool $containsOptgroups = false;
	/** sorting enabled */
	private bool $sortElements = true;
	/** right to left text direction */
	private bool $rightToLeftTextDirection = false;
	/** enabled or disabled */
	private bool $isEnabled = true;
	/** width of input element */
	private string $width = '';
	/** transform select boxes with one element to text */
	private bool $transformSingleSelect = true;
	/** onchange event */
	private ?string $onchangeEvent = null;
	/** indicates that this field should not automatically be saved in the self-service or server profile */
	private bool $transient = false;
	/** @var array<string, string[]> $tableRowsToHide list of enclosing table rows to hide when checked */
	protected array $tableRowsToHide = [];
	/** @var array<string, string[]> $tableRowsToShow list of enclosing table rows to show when checked */
	protected array $tableRowsToShow = [];
	/** dynamic scrolling */
	private bool $dynamicScrolling = false;
	/** @var array<string, string> $optionCssClasses CSS classes for values */
	private array $optionCssClasses = [];

	/**
	 * Constructor.
	 *
	 * <br>Examples:
	 * <br>
	 * <br>$select = new htmlSelect('myName', array('value1', 'value2'), array('value1'));
	 * <br>
	 * <br>$select = new htmlSelect('myName', array('label1' => 'value1', 'label2' => 'value2'), array('value1'));
	 * <br>$select->setHasDescriptiveElements(true);
	 * <br>
	 * <br>$select = new htmlSelect('myName', array('optgroupLabel' => array('value1', 'value2')), array('value1'));
	 * <br>$select->setHasDescriptiveElements(true);
	 * <br>$select->setContainsOptgroups(true);
	 *
	 * @param string $name element name
	 * @param string[]|array<string, string>|array<string, string[]|array<string, string>> $elements list of elements array(label => value) or array(value1, value2) or array('optgroup' => array(...))
	 * @param string[] $selectedElements list of selected elements (optional, default none)
	 * @param int $size size (optional, default = 1)
	 */
	public function __construct(string $name, array $elements, array $selectedElements = [], int $size = 1) {
		$this->name = htmlspecialchars($name);
		$this->elements = $elements;
		if ($selectedElements != null) {
			$this->selectedElements = $selectedElements;
		}
		$this->size = htmlspecialchars((string) $size);
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		if (isset($values[$this->name])) {
			$this->selectedElements = $values[$this->name];
		}
		$multi = '';
		$name = ' name="' . $this->name . '" id="' . $this->name . '"';
		if ($this->multiSelect) {
			$multi = ' multiple';
			$name = ' name="' . $this->name . '[]" id="' . $this->name . '"';
		}
		$size = ' size="' . $this->size . '"';
		$classList = $this->cssClasses;
		if ($this->rightToLeftTextDirection) {
			$classList[] = 'rightToLeftText';
		}
		$class = ' class="' . implode(' ', $classList) . '"';
		$disabled = '';
		if (!$this->isEnabled) {
			$disabled = ' disabled';
		}
		$style = '';
		if ($this->width !== '') {
			$style = ' style="width: ' . $this->width . '"';
		}
		$onchange = '';
		if ($this->onchangeEvent != null) {
			$onchange = $this->onchangeEvent;
		}
		if (($this->tableRowsToHide != null) || ($this->tableRowsToShow != null)) {
			$this->printCodeForShowHideTableRows($onchange);
		}
		if ($onchange !== '') {
			$onchange = ' onchange="' . $onchange . '"';
		}
		// hide select boxes that contain less than 2 elements
		if ((count($this->elements) < 2) && !$this->multiSelect && !$this->containsOptgroups && $this->transformSingleSelect) {
			echo '<div class="hidden">';
		}
		// print select box
		echo '<select' . $this->getDataAttributesAsString() . $this->getAccessibilityMarkup() . $class . $style
			. $name . $size . $multi . $disabled . $onchange . ">\n";
		if ($this->containsOptgroups) {
			foreach ($this->elements as $label => $elements) {
				if (count($elements) > 0) {
					echo '<optgroup label="' . $label . '">';
					$this->printOptionsHTML($elements);
					echo '</optgroup>';
				}
			}
		}
		else {
			$this->printOptionsHTML($this->elements);
		}
		echo "</select>\n";
		// if select box has only one element then show it as text
		if ((count($this->elements) < 2) && !$this->multiSelect && !$this->containsOptgroups && $this->transformSingleSelect) {
			echo '</div>';
			if (count($this->elements) === 1) {
				echo '&nbsp;';
				if ($this->hasDescriptiveElements) {
					$keys = array_keys($this->elements);
					echo $keys[0];
				}
				else {
					echo $this->elements[0];
				}
			}
		}
		if ($this->transient) {
			return [];
		}
		if ($this->multiSelect) {
			return [$this->name => 'multiselect'];
		}
		else {
			return [$this->name => 'select'];
		}
	}

	/**
	 * Prints the HTML code of the option tags.
	 *
	 * @param array<string, string> $elements list of options
	 */
	private function printOptionsHTML(array $elements): void {
		if ($this->dynamicScrolling) {
			echo "<option value=\"#\">#</option>\n";
			return;
		}
		// sorting
		if ($this->sortElements) {
			if ($this->hasDescriptiveElements) {
				$labels = array_keys($elements);
				natcasesort($labels);
				$newElements = [];
				foreach ($labels as $label) {
					$newElements[$label] = $elements[$label];
				}
				$elements = $newElements;
			}
			else {
				natcasesort($elements);
			}
		}
		foreach ($elements as $key => $value) {
			$selected = '';
			$optionClass = '';
			if (isset($this->optionCssClasses[$value])) {
				$optionClass = 'class="' . $this->optionCssClasses[$value] . '"';
			}
			if (in_array($value, $this->selectedElements) || (empty($this->selectedElements) && empty($value))) {
				$selected = ' selected';
			}
			if ($this->hasDescriptiveElements) {
				echo "<option value=\"" . htmlspecialchars($value) . "\"$selected $optionClass>" . htmlspecialchars($key) . "</option>\n";
			}
			else {
				echo "<option$selected $optionClass>" . htmlspecialchars($value) . "</option>\n";
			}
		}
	}

	/**
	 * Specifies if the elements are just a simple list or an associative array (default: simple list).
	 *
	 * @param bool $hasDescriptiveElements activates descriptive elements
	 */
	public function setHasDescriptiveElements(bool $hasDescriptiveElements): void {
		$this->hasDescriptiveElements = $hasDescriptiveElements;
	}

	/**
	 * Specifies if the elements are divided into optgroups.
	 * In this case the provided options are an array where the key is the optgroup label and the value is an array containing the options for the optgroup.
	 *
	 * @param bool $containsOptgroups activates optgroups
	 */
	public function setContainsOptgroups(bool $containsOptgroups): void {
		$this->containsOptgroups = $containsOptgroups;
	}

	/**
	 * Specifies if multi-selection is enabled (default: disabled).
	 *
	 * @param bool $multiSelect allows multi-selection
	 */
	public function setMultiSelect(bool $multiSelect): void {
		$this->multiSelect = $multiSelect;
	}

	/**
	 * Specifies if the elements should be sorted (default: sort).
	 *
	 * @param bool $sortElements sort elements
	 */
	public function setSortElements(bool $sortElements): void {
		$this->sortElements = $sortElements;
	}

	/**
	 * Specifies if the text direction should be set to right to left.
	 *
	 * @param bool $rightToLeftTextDirection if true use right to left direction
	 */
	public function setRightToLeftTextDirection(bool $rightToLeftTextDirection): void {
		$this->rightToLeftTextDirection = $rightToLeftTextDirection;
	}

	/**
	 * Specifies if this component is enabled and accepts user modification.
	 *
	 * @param bool $isEnabled enabled if true
	 */
	public function setIsEnabled(bool $isEnabled): void {
		$this->isEnabled = $isEnabled;
	}

	/**
	 * Specifies the width of this selection box.
	 *
	 * @param string $width width (e.g. 20em)
	 */
	public function setWidth(string $width): void {
		$this->width = htmlspecialchars($width);
	}

	/**
	 * Specifies if select boxes that contain only a single element should be transformed to a simple text field.
	 *
	 * @param bool $transformSingleSelect transform single options to text
	 */
	public function setTransformSingleSelect(bool $transformSingleSelect): void {
		$this->transformSingleSelect = $transformSingleSelect;
	}

	/**
	 * Sets the JavaScript code for the onchange event.
	 *
	 * @param string $onchangeEvent onchange event code (e.g. myfunction();)
	 */
	public function setOnchangeEvent(string $onchangeEvent): void {
		$this->onchangeEvent = htmlspecialchars($onchangeEvent);
	}

	/**
	 * Specifies that the value should not be automatically saved when used in self-service or server profile (default: false).
	 *
	 * @param bool $transient transient field
	 */
	public function setTransient(bool $transient): void {
		$this->transient = $transient;
	}

	/**
	 * This will hide the given table rows when the select is changed to the specified value.
	 * The given IDs can be of any e.g. input element. Starting from this element
	 * the first parent "<tr>" element will be used to show/hide.
	 * <br>
	 * <br>
	 * <br>Example: <tr><td><input type="checkbox" id="mycheckbox"></td></tr>
	 * <br> Using "mycheckbox" will use this "tr" to hide/show.
	 * <br>
	 * <br> Example for $tableRowsToHide:
	 * <br> array('yes' => array('option1', 'option2'), 'no' => array('option3'))
	 *
	 * @param array<string, string[]> $tableRowsToHide array of select value => array of IDs of child elements to hide
	 */
	public function setTableRowsToHide(array $tableRowsToHide): void {
		$this->tableRowsToHide = $tableRowsToHide;
	}

	/**
	 * This will show the given table rows when the select is changed to the specified value.
	 * The given IDs can be of any e.g. input element. Starting from this element
	 * the first parent "<tr>" element will be used to show/hide.
	 * <br>
	 * <br>
	 * <br>Example: <tr><td><input type="checkbox" id="mycheckbox"></td></tr>
	 * <br> Using "mycheckbox" will use this "tr" to hide/show.
	 * <br>
	 * <br> Example for $tableRowsToShow:
	 * <br> array('yes' => array('option1', 'option2'), 'no' => array('option3'))
	 *
	 * @param array<string, string[]> $tableRowsToShow array of select value => array of IDs of child elements to show
	 */
	public function setTableRowsToShow(array $tableRowsToShow): void {
		$this->tableRowsToShow = $tableRowsToShow;
	}

	/**
	 * Creates the JavaScript code to hide/show table rows based on the select value.
	 *
	 * @param string $onChange onChange code
	 */
	private function printCodeForShowHideTableRows(string &$onChange): void {
		if ((count($this->tableRowsToHide) === 0) && (count($this->tableRowsToShow) === 0)) {
			return;
		}
		$values = [];
		if (!empty($this->tableRowsToHide)) {
			$values = array_merge($values, array_keys($this->tableRowsToHide));
		}
		if (!empty($this->tableRowsToShow)) {
			$values = array_merge($values, array_keys($this->tableRowsToShow));
		}
		$values = array_unique($values);
		$selector = $this->getShowHideSelector();
		// build JavaScript to show/hide depending on fields
		foreach ($values as $val) {
			// build onChange listener
			$onChange .= 'if (document.getElementById(\'' . $this->name . '\').value == \'' . $val . '\') {';
			if (isset($this->tableRowsToShow[$val])) {
				for ($i = 0; $i < count($this->tableRowsToShow[$val]); $i++) {
					$onChange .= 'document.getElementById(\'' . $this->tableRowsToShow[$val][$i] . '\')?.closest(\'' . $selector . '\').classList.remove(\'hidden\');';
				}
			}
			if (isset($this->tableRowsToHide[$val])) {
				for ($i = 0; $i < count($this->tableRowsToHide[$val]); $i++) {
					$onChange .= 'document.getElementById(\'' . $this->tableRowsToHide[$val][$i] . '\')?.closest(\'' . $selector . '\').classList.add(\'hidden\');';
				}
			}
			$onChange .= '};';
		}
		// build script to set initial state
		$script = '<script type="text/javascript">window.lam.utility.documentReady(function() {' . "\n";
		if (isset($this->tableRowsToShow[$this->selectedElements[0]])) {
			for ($i = 0; $i < count($this->tableRowsToShow[$this->selectedElements[0]]); $i++) {
				$classType = 'remove';
				$script .= 'document.getElementById(\'' . $this->tableRowsToShow[$this->selectedElements[0]][$i] . '\')?.closest(\'' . $selector . '\').classList.' . $classType . '(\'hidden\');' . "\n";
			}
		}
		if (isset($this->tableRowsToHide[$this->selectedElements[0]])) {
			for ($i = 0; $i < count($this->tableRowsToHide[$this->selectedElements[0]]); $i++) {
				$classType = 'add';
				$script .= 'document.getElementById(\'' . $this->tableRowsToHide[$this->selectedElements[0]][$i] . '\')?.closest(\'' . $selector . '\').classList.' . $classType . '(\'hidden\');' . "\n";
			}
		}
		$script .= '});</script>';
		echo $script;
	}

	/**
	 * Returns the selector to use to find the show/hide elements.
	 *
	 * @return string selector
	 */
	protected function getShowHideSelector(): string {
		return 'tr';
	}

	/**
	 * Enable dynamic scrolling. This limits the number of select options to 10000 by dynamically adding/removing options.
	 * This will not be enabled when optgroups are used or the option size is less than 10000.
	 */
	public function enableDynamicScrolling(): void {
		// not possible for optgroups and smaller option lists
		if ((count($this->elements) < 10000) || $this->containsOptgroups) {
			return;
		}
		$this->dynamicScrolling = true;
		$elementData = [];
		$counter = 0;
		foreach ($this->elements as $key => $value) {
			$elementData[] = [
				'label' => $key,
				'value' => $value,
				'selected' => in_array($value, $this->selectedElements),
				'index' => $counter];
			$counter++;
		}
		$this->addDataAttribute('dynamic-options', json_encode($elementData));
		$this->cssClasses[] = 'lam-dynamicOptions';
	}

	/**
	 * Sets CSS classes for option values.
	 *
	 * @param array<string, string> $optionCssClasses array('option value' => 'CSS class(es)')
	 */
	public function setOptionCssClasses(array $optionCssClasses): void {
		$this->optionCssClasses = $optionCssClasses;
	}

}


/**
 * Represents a radio selection.
 *
 * @package metaHTML
 */
class htmlRadio extends htmlElement {

	/** name of select field */
	private string $name;
	/** @var array<string, string> $elements elements */
	private array $elements;
	/** selected element */
	private ?string $selectedElement = null;
	/** enabled or disabled */
	private bool $isEnabled = true;
	/** on change code */
	private ?string $onchangeEvent = null;
	/** @var array<string, string[]> $tableRowsToHide list of enclosing table rows to hide when checked */
	protected array $tableRowsToHide = [];
	/** @var array<string, string[]> $tableRowsToShow list of enclosing table rows to show when checked */
	protected array $tableRowsToShow = [];

	/**
	 * Constructor.
	 *
	 * <br>Examples:
	 * <br>
	 * <br>$radio = new htmlRadio('myName', array('label1' => 'value1', 'label2' => 'value2'), array('value1'));
	 *
	 * @param string $name element name
	 * @param array<string, string> $elements list of elements array(label => value)
	 * @param string|null $selectedElement value of selected element (optional, default none)
	 */
	public function __construct(string $name, array $elements, ?string $selectedElement = null) {
		$this->name = htmlspecialchars($name);
		$this->elements = $elements;
		if ($selectedElement != null) {
			$this->selectedElement = $selectedElement;
		}
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		if (isset($values[$this->name][0])) {
			$this->selectedElement = $values[$this->name][0];
		}
		$name = ' name="' . $this->name . '"';
		$disabled = '';
		if (!$this->isEnabled) {
			$disabled = ' disabled';
		}
		if (($this->tableRowsToHide != null) || ($this->tableRowsToShow != null)) {
			$this->printInitialState();
		}
		// print radio list
		$counter = 0;
		foreach ($this->elements as $label => $value) {
			$showHideOnchange = '';
			if (($this->tableRowsToHide != null) || ($this->tableRowsToShow != null)) {
				$showHideOnchange = $this->getOnchangeCodeForShowHideTableRows($counter);
			}
			$onchange = '';
			if ($this->onchangeEvent != null) {
				$onchange = ' onchange="' . $this->onchangeEvent . '"';
			}
			elseif (!empty($showHideOnchange)) {
				$onchange = ' onchange="' . $showHideOnchange . '"';
			}
			echo '<div class="nowrap">';
			$selected = '';
			if ($value == $this->selectedElement) {
				$selected = ' checked';
			}
			echo '<input type="radio" id="' . $this->name . $counter . '"' . $name . $disabled . $selected . $onchange . ' value="' . $value . '"> ';
			echo '<label for="' . $this->name . $counter . '">' . $label . '</label>';
			echo '</div>';
			$counter++;
		}
		return [$this->name => 'select'];
	}

	/**
	 * Specifies if this component is enabled and accepts user modification.
	 *
	 * @param bool $isEnabled enabled if true
	 */
	public function setIsEnabled(bool $isEnabled): void {
		$this->isEnabled = $isEnabled;
	}

	/**
	 * Sets the JavaScript code for the onchange event.
	 *
	 * @param string $onchangeEvent onchange event code (e.g. myfunction();)
	 */
	public function setOnchangeEvent(string $onchangeEvent): void {
		$this->onchangeEvent = htmlspecialchars($onchangeEvent);
	}

	/**
	 * Returns the selector to use to find the show/hide elements.
	 *
	 * @return string selector
	 */
	protected function getShowHideSelector(): string {
		return 'tr';
	}

	/**
	 * Creates the JavaScript code to hide/show table rows based on the select value.
	 *
	 * @param int $counter index
	 * @return string onChange code
	 */
	private function getOnchangeCodeForShowHideTableRows(int $counter): string {
		$onChange = '';
		if ((count($this->tableRowsToHide) === 0) && (count($this->tableRowsToShow) === 0)) {
			return $onChange;
		}
		$values = [];
		if (!empty($this->tableRowsToHide)) {
			$values = array_merge($values, array_keys($this->tableRowsToHide));
		}
		if (!empty($this->tableRowsToShow)) {
			$values = array_merge($values, array_keys($this->tableRowsToShow));
		}
		$values = array_unique($values);
		$selector = $this->getShowHideSelector();
		// build Java script to show/hide depending fields
		foreach ($values as $val) {
			// build onChange listener
			$onChange .= 'if (document.getElementById(\'' . $this->name . $counter . '\').value == \'' . $val . '\') {';
			if (isset($this->tableRowsToShow[$val])) {
				for ($i = 0; $i < count($this->tableRowsToShow[$val]); $i++) {
					$onChange .= 'document.getElementById(\'' . $this->tableRowsToShow[$val][$i] . '\')?.closest(\'' . $selector . '\').classList.remove(\'hidden\');';
				}
			}
			if (isset($this->tableRowsToHide[$val])) {
				for ($i = 0; $i < count($this->tableRowsToHide[$val]); $i++) {
					$onChange .= 'document.getElementById(\'' . $this->tableRowsToHide[$val][$i] . '\')?.closest(\'' . $selector . '\').classList.add(\'hidden\');';
				}
			}
			$onChange .= '};';
		}
		return $onChange;
	}

	private function printInitialState(): void {
		$selector = $this->getShowHideSelector();
		// build script to set initial state
		$script = '<script type="text/javascript">window.lam.utility.documentReady(function() {' . "\n";
		if (isset($this->tableRowsToShow[$this->selectedElement])) {
			for ($i = 0; $i < count($this->tableRowsToShow[$this->selectedElement]); $i++) {
				$script .= 'document.getElementById(\'' . $this->tableRowsToShow[$this->selectedElement][$i] . '\')?.closest(\'' . $selector . '\').classList.remove(\'hidden\');' . "\n";
			}
		}
		if (isset($this->tableRowsToHide[$this->selectedElement])) {
			for ($i = 0; $i < count($this->tableRowsToHide[$this->selectedElement]); $i++) {
				$script .= 'document.getElementById(\'' . $this->tableRowsToHide[$this->selectedElement][$i] . '\')?.closest(\'' . $selector . '\').classList.add(\'hidden\');' . "\n";
			}
		}
		$script .= '});</script>';
		echo $script;
	}

	/**
	 * This will hide the given table rows when the radio is changed to the specified value.
	 * The given IDs can be of any e.g. input element. Starting from this element
	 * the first parent "<tr>" element will be used to show/hide.
	 * <br>
	 * <br>
	 * <br> Example for $tableRowsToHide:
	 * <br> array('val1' => array('option1', 'option2'), 'val2' => array('option3'))
	 *
	 * @param array<string, string[]> $tableRowsToHide array of select value => array of IDs of child elements to hide
	 */
	public function setTableRowsToHide(array $tableRowsToHide): void {
		$this->tableRowsToHide = $tableRowsToHide;
	}

	/**
	 * This will show the given table rows when the radio is changed to the specified value.
	 * The given IDs can be of any e.g. input element. Starting from this element
	 * the first parent "<tr>" element will be used to show/hide.
	 * <br>
	 * <br>
	 * <br> Example for $tableRowsToShow:
	 * <br> array('val1' => array('option1', 'option2'), 'val2' => array('option3'))
	 *
	 * @param array<string, string[]> $tableRowsToShow array of select value => array of IDs of child elements to show
	 */
	public function setTableRowsToShow(array $tableRowsToShow): void {
		$this->tableRowsToShow = $tableRowsToShow;
	}

}

/**
 * Prints the text and escapes contained HTML code by default.
 *
 * @package metaHTML
 */
class htmlOutputText extends htmlElement {

	/** the text to print */
	private string $string;
	/** specifies if HTML code should be escaped */
	private bool $escapeHTML;
	/** bold text */
	private bool $isBold = false;
	/** mark as required */
	private bool $markAsRequired;
	/** no wrap */
	private bool $noWrap = false;
	/** preformatted */
	private bool $isPreformatted = false;
	/** title */
	private ?string $title = null;

	/**
	 * Constructor.
	 *
	 * @param string $string output text
	 * @param bool $escapeHTML escape HTML code (default yes)
	 * @param bool $markAsRequired mark text like a required field
	 */
	public function __construct(string $string, bool $escapeHTML = true, bool $markAsRequired = false) {
		$this->string = $string;
		$this->escapeHTML = $escapeHTML;
		$this->markAsRequired = $markAsRequired;
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		$cssClasses = empty($this->cssClasses) ? '' : 'class="' . implode(' ', $this->cssClasses) . '"';
		if (!empty($this->title)) {
			echo '<span title="' . $this->title . '">';
		}
		if ($this->noWrap) {
			echo "<div class=\"nowrap\">";
		}
		if ($this->isBold) {
			echo "<b>";
		}
		if ($this->isPreformatted) {
			echo "<pre $cssClasses>";
		}
		elseif (!empty($cssClasses)) {
			echo "<span $cssClasses>";
		}
		if ($this->escapeHTML) {
			echo htmlspecialchars($this->string);
		}
		else {
			echo $this->string;
		}
		if ($this->markAsRequired) {
			echo htmlGetRequiredMarker();
		}
		if ($this->isPreformatted) {
			echo "</pre>";
		}
		elseif (!empty($cssClasses)) {
			echo "</span>";
		}
		if ($this->isBold) {
			echo "</b>";
		}
		if ($this->noWrap) {
			echo "</div>";
		}
		if (!empty($this->title)) {
			echo "</span>";
		}
		return [];
	}

	/**
	 * Specifies if the whole text should be printed in bold.
	 *
	 * @param bool $isBold bold text
	 */
	public function setIsBold(bool $isBold): void {
		$this->isBold = $isBold;
	}

	/**
	 * Adds a marker that indicates a required field.
	 *
	 * @param bool $markAsRequired add marker
	 */
	public function setMarkAsRequired(bool $markAsRequired): void {
		$this->markAsRequired = $markAsRequired;
	}

	/**
	 * Specifies if word wrap is allowed for this text.
	 *
	 * @param bool $noWrap no wrapping if set to true (default false)
	 */
	public function setNoWrap(bool $noWrap): void {
		$this->noWrap = $noWrap;
	}

	/**
	 * Sets if the text is preformatted.
	 *
	 * @param bool $preformatted is preformatted (default true)
	 */
	public function setPreformatted(bool $preformatted = true): void {
		$this->isPreformatted = $preformatted;
	}

	/**
	 * Sets a title for this text.
	 *
	 * @param string|null $title title
	 */
	public function setTitle(?string $title): void {
		$this->title = $title;
	}

}

/**
 * Prints the HTML code for a checkbox.
 *
 * @package metaHTML
 */
class htmlInputCheckbox extends htmlElement {

	/** unique name of input element */
	protected string $name;
	/** value */
	protected bool $checked;
	/** enabled or disabled */
	protected bool $isEnabled = true;
	/** @var string[] $tableRowsToHide list of enclosing table rows to hide when checked */
	protected array $tableRowsToHide = [];
	/** @var string[] $tableRowsToShow list of enclosing table rows to show when checked */
	protected array $tableRowsToShow = [];
	/** indicates that this field should not automatically be saved in the self-service or server profile */
	private bool $transient = false;
	/** @var string[] $elementsToEnable list of input elements to enable when checked */
	protected array $elementsToEnable = [];
	/** @var string[] $elementsToDisable list of input elements to disable when checked */
	protected array $elementsToDisable = [];
	/** onclick event code */
	private ?string $onClick = null;
	/** title */
	private ?string $title = null;


	/**
	 * Constructor.
	 *
	 * @param string $name unique name
	 * @param bool $checked checked
	 */
	public function __construct(string $name, bool $checked) {
		$this->name = htmlspecialchars($name);
		$this->checked = $checked;
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		if (isset($values[$this->name])) {
			$this->checked = ($values[$this->name][0] == 'true');
		}
		$checked = '';
		if ($this->checked) {
			$checked = ' checked';
		}
		$title = '';
		if ($this->title !== null) {
			$title = ' title="' . $this->title . '"';
		}
		$disabled = '';
		if (!$this->isEnabled) {
			$disabled = ' disabled';
		}
		$classes = ' ';
		if (!empty($this->cssClasses)) {
			$classes = ' class="' . implode(' ', $this->cssClasses) . '"';
		}
		// build Java script to show/hide depending fields
		$onChange = '';
		$script = '';
		$selector = $this->getShowHideSelector();
		if ((count($this->tableRowsToShow) > 0) || (count($this->tableRowsToHide) > 0)) {
			// build onChange listener
			$onChange .= 'if (document.querySelector(\'#' . $this->name . ':checked\') !== null) {';
			for ($i = 0; $i < count($this->tableRowsToShow); $i++) {
				$onChange .= 'document.getElementById(\'' . $this->tableRowsToShow[$i] . '\')?.closest(\'' . $selector . '\').classList.remove(\'hidden\');';
			}
			for ($i = 0; $i < count($this->tableRowsToHide); $i++) {
				$onChange .= 'document.getElementById(\'' . $this->tableRowsToHide[$i] . '\')?.closest(\'' . $selector . '\').classList.add(\'hidden\');';
			}
			$onChange .= '}';
			$onChange .= 'else {';
			for ($i = 0; $i < count($this->tableRowsToShow); $i++) {
				$onChange .= 'document.getElementById(\'' . $this->tableRowsToShow[$i] . '\')?.closest(\'' . $selector . '\').classList.add(\'hidden\');';
			}
			for ($i = 0; $i < count($this->tableRowsToHide); $i++) {
				$onChange .= 'document.getElementById(\'' . $this->tableRowsToHide[$i] . '\')?.closest(\'' . $selector . '\').classList.remove(\'hidden\');';
			}
			$onChange .= '};';
			// build script to set initial state
			$script .= '<script type="text/javascript">window.lam.utility.documentReady(function() {';
			for ($i = 0; $i < count($this->tableRowsToShow); $i++) {
				$classType = 'add';
				if ($this->checked) {
					$classType = 'remove';
				}
				$script .= 'document.getElementById(\'' . $this->tableRowsToShow[$i] . '\')?.closest(\'' . $selector . '\').classList.' . $classType . '(\'hidden\');';
			}
			for ($i = 0; $i < count($this->tableRowsToHide); $i++) {
				$classType = 'remove';
				if ($this->checked) {
					$classType = 'add';
				}
				$script .= 'document.getElementById(\'' . $this->tableRowsToHide[$i] . '\')?.closest(\'' . $selector . '\').classList.' . $classType . '(\'hidden\');';
			}
			$script .= '});</script>';
		}
		// build Java script to enable/disable elements
		if ((count($this->elementsToEnable) > 0) || (count($this->elementsToDisable) > 0)) {
			// build onChange listener
			$onChange .= 'if (document.querySelector(\'#' . $this->name . ':checked\') !== null) {';
			for ($i = 0; $i < count($this->elementsToEnable); $i++) {
				$onChange .= 'document.getElementById(\'' . $this->elementsToEnable[$i] . '\').disabled = false;';
			}
			for ($i = 0; $i < count($this->elementsToDisable); $i++) {
				$onChange .= 'document.getElementById(\'' . $this->elementsToDisable[$i] . '\').disabled = true;';
			}
			$onChange .= '}';
			$onChange .= 'else {';
			for ($i = 0; $i < count($this->elementsToEnable); $i++) {
				$onChange .= 'document.getElementById(\'' . $this->elementsToEnable[$i] . '\').disabled = true;';
			}
			for ($i = 0; $i < count($this->elementsToDisable); $i++) {
				$onChange .= 'document.getElementById(\'' . $this->elementsToDisable[$i] . '\').disabled = false;';
			}
			$onChange .= '};';
			// build script to set initial state
			$script .= '<script type="text/javascript">window.lam.utility.documentReady(function() {';
			for ($i = 0; $i < count($this->elementsToEnable); $i++) {
				$classType = 'true';
				if ($this->checked) {
					$classType = 'false';
				}
				$script .= 'document.getElementById(\'' . $this->elementsToEnable[$i] . '\').disabled = ' . $classType . ';';
			}
			for ($i = 0; $i < count($this->elementsToDisable); $i++) {
				$classType = 'false';
				if ($this->checked) {
					$classType = 'true';
				}
				$script .= 'document.getElementById(\'' . $this->elementsToDisable[$i] . '\').disabled = ' . $classType . ';';
			}
			$script .= '});</script>';
		}
		if (!empty($onChange)) {
			$onChange = ' onChange="' . $onChange . '"';
		}
		$onClick = '';
		if (!empty($this->onClick)) {
			$onClick = ' onclick="' . $this->onClick . '"';
		}
		echo '<input type="checkbox" id="' . $this->name . '" name="' . $this->name . '"' . $classes
			. $this->getAccessibilityMarkup() . $this->getDataAttributesAsString() . $onChange . $onClick . $title . $checked . $disabled . '>';
		echo $script;
		if ($this->transient) {
			return [];
		}
		return [$this->name => 'checkbox'];
	}

	/**
	 * Specifies if this component is enabled and accepts user modification.
	 *
	 * @param bool $isEnabled enabled if true
	 */
	public function setIsEnabled(bool $isEnabled): void {
		$this->isEnabled = $isEnabled;
	}

	/**
	 * This will hide the given table rows when the checkbox is checked.
	 * The given IDs can be of any e.g. input element. Starting from this element
	 * the first parent "<tr>" element will be used to show/hide.
	 * <br>
	 * <br>
	 * <br>Example: <tr><td><input type="checkbox" id="mycheckbox"></td></tr>
	 * <br> Using "mycheckbox" will use this "tr" to hide/show.
	 *
	 * @param string[] $tableRowsToHide IDs of child elements to hide
	 */
	public function setTableRowsToHide(array $tableRowsToHide): void {
		$this->tableRowsToHide = $tableRowsToHide;
	}

	/**
	 * This will show the given table rows when the checkbox is checked.
	 * The given IDs can be of any e.g. input element. Starting from this element
	 * the first parent "<tr>" element will be used to show/hide.
	 * <br>
	 * <br>
	 * <br>Example: <tr><td><input type="checkbox" id="mycheckbox"></td></tr>
	 * <br> Using "mycheckbox" will use this "tr" to hide/show.
	 *
	 * @param string[] $tableRowsToShow IDs of child elements to show
	 */
	public function setTableRowsToShow(array $tableRowsToShow): void {
		$this->tableRowsToShow = $tableRowsToShow;
	}

	/**
	 * Specifies that the value should not be automatically saved when used in self-service or server profile (default: false).
	 *
	 * @param bool $transient transient field
	 */
	public function setTransient(bool $transient): void {
		$this->transient = $transient;
	}

	/**
	 * This will disable the given input elements when the checkbox is checked.
	 * The given IDs can be of any input element (e.g. select, checkbox, ...).
	 *
	 * @param string[] $elements IDs of elements to disable
	 */
	public function setElementsToDisable(array $elements): void {
		$this->elementsToDisable = $elements;
	}

	/**
	 * This will enable the given input elements when the checkbox is checked.
	 * The given IDs can be of any input element (e.g. select, checkbox, ...).
	 *
	 * @param string[] $elements IDs of elements to enable
	 */
	public function setElementsToEnable(array $elements): void {
		$this->elementsToEnable = $elements;
	}

	/**
	 * Returns the CSS selector to use to find show/hide elements.
	 */
	protected function getShowHideSelector(): string {
		return 'tr';
	}

	/**
	 * Sets the onclick code.
	 *
	 * @param string $code JS code
	 */
	public function setOnClick(string $code): void {
		$this->onClick = $code;
	}

	/**
	 * Returns the checkbox name.
	 *
	 * @return string name
	 */
	protected function getName(): string {
		return $this->name;
	}

	/**
	 * Sets the title.
	 *
	 * @param string|null $title title
	 */
	public function setTitle(?string $title): void {
		$this->title = $title;
	}

}

/**
 * Prints the HTML code for a file upload field.
 *
 * @package metaHTML
 */
class htmlInputFileUpload extends htmlElement {

	/** unique name of input element */
	private string $name;
	/** enabled or disabled */
	private bool $isEnabled = true;
	/** required */
	protected bool $required = false;
	/** onchange event */
	protected string $onChange = '';

	/**
	 * Constructor.
	 *
	 * @param string $name unique name
	 */
	public function __construct(string $name) {
		$this->name = htmlspecialchars($name);
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		$disabled = '';
		if (!$this->isEnabled) {
			$disabled = ' disabled';
		}
		$required = '';
		if ($this->required) {
			$required = ' required="required"';
		}
		$onChange = '';
		if (!empty($this->onChange)) {
			$onChange = ' onchange="' . $this->onChange . '"';
		}
		$classValue = '';
		if (!empty($this->cssClasses)) {
			$classValue = ' class="' . implode(' ', $this->cssClasses) . '"';
		}
		echo '<input type="file" id="' . $this->name . '" name="' . $this->name . '"' . $classValue .
			$onChange . $disabled . $required . $this->getDataAttributesAsString() . '>';
		return [$this->name => 'file'];
	}

	/**
	 * Specifies if this component is enabled and accepts user modification.
	 *
	 * @param bool $isEnabled enabled if true
	 */
	public function setIsEnabled(bool $isEnabled): void {
		$this->isEnabled = $isEnabled;
	}

	/**
	 * Sets the field required.
	 *
	 * @param bool $required field is required (default: true)
	 */
	public function setRequired(bool $required = true): void {
		$this->required = $required;
	}

	/**
	 * Sets the onChange event.
	 *
	 * @param string $onChange onChange code
	 */
	public function setOnChange(string $onChange): void {
		$this->onChange = $onChange;
	}

}

/**
 * Prints the HTML code for a textarea.
 *
 * @package metaHTML
 */
class htmlInputTextarea extends htmlElement {

	/** unique name of input element */
	protected string $name;
	/** value */
	private string $value;
	/** column count */
	private string $colCount;
	/** row count */
	private string $rowCount;
	/** enabled or disabled */
	private bool $isEnabled = true;
	/** specifies if LAM should display this field with a WYSIWYG editor */
	protected bool $richEdit = false;
	/** required field */
	protected bool $required = false;

	/**
	 * Constructor.
	 *
	 * @param string $name unique name
	 * @param string $value value
	 * @param int $colCount number of characters per line
	 * @param int $rowCount number of rows
	 */
	public function __construct(string $name, string $value, int $colCount, int $rowCount) {
		$this->name = htmlspecialchars($name);
		$this->value = htmlspecialchars($value);
		$this->colCount = htmlspecialchars((string) $colCount);
		$this->rowCount = htmlspecialchars((string) $rowCount);
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		if (isset($values[$this->name])) {
			$this->value = htmlspecialchars(implode("\r\n", $values[$this->name]));
		}
		$colCount = ($this->colCount != null) ? ' cols="' . $this->colCount . '"' : '';
		$rowCount = ($this->rowCount != null) ? ' rows="' . $this->rowCount . '"' : '';
		$disabled = '';
		if (!$this->isEnabled) {
			$disabled = ' disabled';
		}
		$classList = $this->cssClasses;
		if ($this->richEdit) {
			$classList[] = 'lam-rich-edit';
		}
		$required = '';
		if ($this->required) {
			$required = ' required';
		}
		$classes = ' class="' . implode(' ', $classList) . '"';
		echo '<textarea name="' . $this->name . '" id="' . $this->name . '"' . $classes . $colCount .
			$rowCount . $this->getDataAttributesAsString() . $this->getAccessibilityMarkup() . $required . $disabled . '>' .
			$this->value . '</textarea>';
		return [$this->name => 'textarea'];
	}

	/**
	 * Specifies if this component is enabled and accepts user modification.
	 *
	 * @param bool $isEnabled enabled if true
	 */
	public function setIsEnabled(bool $isEnabled): void {
		$this->isEnabled = $isEnabled;
	}

	/**
	 * Specifies if the textarea should be displayed with a WYSIWYG editor.
	 * <br>This requires that the page which displays the textarea also includes the JS for the rich editing.
	 * <br>Rich editing is disabled by default.
	 *
	 * @param bool $richEdit rich edit or standard
	 */
	public function setIsRichEdit(bool $richEdit): void {
		$this->richEdit = $richEdit;
	}

	/**
	 * Specifies if the input field is required.
	 *
	 * @param bool $required required
	 */
	public function setRequired(bool $required): void {
		$this->required = $required;
	}

}

/**
 * Prints the HTML code for a color picker field.
 *
 * @package metaHTML
 */
class htmlInputColorPicker extends htmlElement {

	/** unique name of input element */
	private string $name;
	/** color value */
	private string $color;
	/** enabled or disabled */
	private bool $isEnabled = true;

	/**
	 * Constructor.
	 *
	 * @param string $name unique name
	 * @param string $colorValue color value (e.g. #000000)
	 */
	public function __construct(string $name, string $colorValue = '#000000') {
		$this->name = htmlspecialchars($name);
		$this->color = htmlspecialchars($colorValue);
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		$disabled = '';
		if (!$this->isEnabled) {
			$disabled = ' disabled';
		}
		echo '<input type="color" value="' . $this->color . '" id="' . $this->name . '" name="' . $this->name . '"' . $disabled . '>';
		return [$this->name => 'file'];
	}

	/**
	 * Specifies if this component is enabled and accepts user modification.
	 *
	 * @param bool $isEnabled enabled if true
	 */
	public function setIsEnabled(bool $isEnabled): void {
		$this->isEnabled = $isEnabled;
	}

}

/**
 * Color picker with descriptive label and help link.
 *
 * @package metaHTML
 */
class htmlResponsiveInputColorPicker extends htmlInputColorPicker {

	/** descriptive label */
	private string $label;
	/** help ID */
	private ?string $helpID = null;
	/** help module */
	private ?string $helpModule = null;
	/** render HTML of parent class */
	private bool $renderParentHtml = false;

	/**
	 * Constructor.
	 *
	 * @param string $name unique name
	 * @param string $colorValue color value (e.g. #000000)
	 * @param string $label descriptive label
	 * @param string[]|string|null $helpID help ID
	 */
	public function __construct($name, $colorValue, string $label, array|string|null $helpID = null) {
		parent::__construct($name, $colorValue);
		$this->label = htmlspecialchars($label);
		if (is_string($helpID)) {
			$this->helpID = $helpID;
		}
		elseif (is_array($helpID)) {
			$this->helpID = $helpID[0];
			$this->helpModule = $helpID[1];
		}
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		if ($this->renderParentHtml) {
			return parent::generateHTML($module, $input, $values, $restricted, $scope);
		}
		// HTML of parent class is rendered on second call (done by htmlResponsiveRow)
		$this->renderParentHtml = true;
		$row = new htmlResponsiveRow();
		// label text
		$labelGroup = new htmlGroup();
		$labelGroup->addElement(new htmlOutputText($this->label));
		if (!empty($this->helpID)) {
			$helpLinkLabel = new htmlHelpLink($this->helpID, $this->helpModule);
			$helpLinkLabel->setCSSClasses(['hide-on-tablet', 'margin-left5']);
			$labelGroup->addElement($helpLinkLabel);
		}
		$row->add($labelGroup, 12, 6, 6, 'responsiveLabel');
		// input field
		$fieldGroup = new htmlGroup();
		$fieldGroup->addElement($this);
		if (!empty($this->helpID)) {
			$helpLinkField = new htmlHelpLink($this->helpID, $this->helpModule);
			$helpLinkField->setCSSClasses(['hide-on-mobile']);
			$fieldGroup->addElement($helpLinkField);
		}
		$row->add($fieldGroup, 12, 6, 6, 'responsiveField nowrap');
		return $row->generateHTML($module, $input, $values, $restricted, $scope);
	}

}

/**
 * Prints the HTML code for an image.
 *
 * @package metaHTML
 */
class htmlImage extends htmlElement {

	/** ID value */
	private ?string $id = null;
	/** path to image */
	private string $path;
	/** width */
	private ?string $width;
	/** height */
	private ?string $height;
	/** alt text */
	private string $alt;
	/** title */
	private ?string $title;
	/** onClick event */
	private ?string $onClick;
	/** enable cropping */
	private bool $crop = false;
	/** enable lightbox */
	private bool $lightbox = false;
	/** help popup title */
	private ?string $helpTitle = null;
	/** help popup content */
	private ?htmlElement $helpContent = null;

	/**
	 * Constructor.
	 *
	 * @param string $path image location
	 * @param string|null $width image width (optional, default original size)
	 * @param string|null $height image height (optional, default original size)
	 * @param string $alt alt text (optional)
	 * @param string|null $title title text (optional)
	 * @param string|null $onClick onClick code (optional)
	 */
	public function __construct(string $path, ?string $width = null, ?string $height = null, string $alt = ' ', ?string $title = null, ?string $onClick = null) {
		$this->path = htmlspecialchars($path);
		$this->width = $width;
		$this->height = $height;
		$this->alt = htmlspecialchars($alt);
		$this->title = $title;
		$this->onClick = $onClick;
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		$idValue = '';
		if ($this->id != null) {
			$idValue = ' id="' . $this->id . '"';
		}
		$path = ' src="' . $this->path . '"';
		$width = '';
		if ($this->width != null) {
			$width = ' width="' . $this->width . '"';
		}
		$height = '';
		if ($this->height != null) {
			$height = ' height="' . $this->height . '"';
		}
		$alt = ' alt="' . $this->alt . '"';
		$title = '';
		if (!empty($this->title)) {
			$title = ' title="' . $this->title . '"';
		}
		if ($this->crop) {
			$this->cssClasses[] = 'cropperjsImage';
		}
		if ($this->lightbox) {
			$this->cssClasses[] = 'lam-lightbox';
			$this->addDataAttribute('lightbox-label-close', _('Close'));
		}
		$classes = '';
		if (!empty($this->cssClasses)) {
			$classes = 'class="' . implode(' ', $this->cssClasses) . '"';
		}
		$onClick = '';
		if ($this->onClick != null) {
			$onClick = ' onclick="' . $this->onClick . '"';
		}
		$helpTitleValue = '';
		$helpContentValue = '';
		if ($this->helpTitle !== null) {
			$helpTitleValue = ' helptitle="' . $this->helpTitle . '"';
			ob_start();
			$this->helpContent->generateHTML($module, $input, $values, $restricted, $scope);
			$helpContentString = ob_get_contents();
			ob_end_clean();
			$helpContentValue = ' helpdata="' . htmlspecialchars($helpContentString) . '"';
		}
		echo '<img' . $idValue . $path . $width . $height . $alt . $title . $classes . $onClick . $helpTitleValue . $helpContentValue . $this->getDataAttributesAsString() . ">";
		if ($this->lightbox) {
			echo '</a>';
		}
		if ($this->crop) {
			echo '<script type="text/javascript">
					window.lam.html.initCropping();
					</script>';
			echo '<input id="croppingDataX" type="hidden" name="croppingDataX" value="0"/>';
			echo '<input id="croppingDataY" type="hidden" name="croppingDataY" value="0"/>';
			echo '<input id="croppingDataWidth" type="hidden" name="croppingDataWidth" value="0"/>';
			echo '<input id="croppingDataHeight" type="hidden" name="croppingDataHeight" value="0"/>';
			echo '<input id="croppingDataSelectionWidth" type="hidden" name="croppingDataSelectionWidth" value="0"/>';
			echo '<input id="croppingDataSelectionHeight" type="hidden" name="croppingDataSelectionHeight" value="0"/>';
		}
		return [];
	}

	/**
	 * Enables cropping feature.
	 * This will display a cropping box on the image. The cropping data
	 * can be found in POST data (croppingDataX, croppingDataY, croppingDataWidth, croppingDataHeight).
	 */
	public function enableCropping(): void {
		$this->crop = true;
	}

	/**
	 * Enables lightbox feature.
	 */
	public function enableLightbox(): void {
		$this->lightbox = true;
	}

	/**
	 * Activates the help popup on hover.
	 *
	 * @param string $title title
	 * @param htmlElement $content help content
	 */
	public function setHelpData(string $title, htmlElement $content): void {
		$this->helpTitle = htmlspecialchars($title);
		$this->helpContent = $content;
	}

	/**
	 * Sets the onClick event code.
	 *
	 * @param string $code JS code
	 */
	public function setOnClick(string $code): void {
		$this->onClick = $code;
	}

	/**
	 * Sets the ID of the image.
	 *
	 * @param string|null $id ID
	 */
	public function setId(?string $id): void {
		$this->id = $id;
	}

}

/**
 * Adds an empty space with given width and height.
 *
 * @package metaHTML
 */
class htmlSpacer extends htmlElement {

	/** width of spacer in px */
	private ?string $width = null;
	/** height of spacer in px */
	private ?string $height = null;

	/**
	 * Constructor.
	 *
	 * @param string|null $width width (e.g. 10px)
	 * @param string|null $height height (e.g. 10px)
	 * @param string[]|null $cssClasses CSS classes
	 */
	public function __construct(?string $width, ?string $height = null, ?array $cssClasses = []) {
		if ($width !== null) {
			$this->width = htmlspecialchars($width);
		}
		if ($height !== null) {
			$this->height = htmlspecialchars($height);
		}
		if (!empty($cssClasses)) {
			$this->setCSSClasses($cssClasses);
		}
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		$width = '';
		if ($this->width !== null) {
			$width = 'width: ' . $this->width . ';';
		}
		$height = '';
		if ($this->height !== null) {
			$height = 'height: ' . $this->height . ';';
		}
		$classes = '';
		if (!empty($this->cssClasses)) {
			$classes = 'class="' . implode(' ', $this->cssClasses) . '"';
		}
		echo "<div $classes style=\"$width $height display: inline-block;\"></div>\n";
		return [];
	}

}

/**
 * Prints a status message (e.g. error message).
 *
 * @package metaHTML
 */
class htmlStatusMessage extends htmlElement {

	/** message type (e.g. ERROR) */
	private string $type;
	/** message title */
	private ?string $title;
	/** message text */
	private ?string $text;
	/** @var array<string|string[]>|null $params message parameters */
	private ?array $params;

	/**
	 * Constructor.
	 *
	 * @param string $type message type (e.g. ERROR)
	 * @param string|null $title message title
	 * @param string|null $text message (optional)
	 * @param array<string|string[]>|null $params additional message parameters
	 */
	public function __construct(string $type, ?string $title, ?string $text = null, ?array $params = null) {
		$this->type = $type;
		$this->title = $title;
		$this->text = $text;
		$this->params = $params;
	}

	/**
	 * Constructor with parameter array.
	 *
	 * @param array<string|string[]> $params parameters in same order as normal constructor
	 * @return htmlStatusMessage message
	 */
	public static function fromParamArray(array $params): htmlStatusMessage {
		if (count($params) < 2) {
			throw new BadMethodCallException("Invalid parameter count");
		}

		$count = count($params);
		return match ($count) {
			2 => new htmlStatusMessage($params[0], $params[1]),
			3 => new htmlStatusMessage($params[0], $params[1], $params[2]),
			4 => new htmlStatusMessage($params[0], $params[1], $params[2], $params[3]),
			default => throw new BadMethodCallException("Invalid parameter count"),
		};
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		if (!empty($this->cssClasses)) {
			echo '<div class="' . implode(' ', $this->cssClasses) . '">';
		}
		StatusMessage($this->type, $this->title, $this->text, $this->params);
		if (!empty($this->cssClasses)) {
			echo '</div>';
		}
		return [];
	}

	/**
	 * Returns the message type.
	 *
	 * @return string type
	 */
	public function getType(): string {
		return $this->type;
	}

}

/**
 * Generates a title line. This is used for page titles.
 *
 * @package metaHTML
 */
class htmlTitle extends htmlElement {

	/** descriptive label */
	private string $label;

	/**
	 * Constructor.
	 *
	 * @param string $label label
	 */
	public function __construct(string $label) {
		$this->label = htmlspecialchars($label);
		// the title should not end at a table cell
		$this->colspan = 100;
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		echo "<div class=\"title\">\n";
		echo "<h2 class=\"titleText\">\n";
		echo $this->label;
		echo "</h2>\n";
		echo "</div>\n";
		return [];
	}

}

/**
 * Generates a subtitle line. This is used to group multiple fields.
 *
 * @package metaHTML
 */
class htmlSubTitle extends htmlElement {

	public const H3 = 'h3';
	public const H4 = 'h4';
	public const H5 = 'h5';

	/** descriptive label */
	private string $label;
	/** optional image */
	private ?string $image = null;
	/** optional ID for this element (e.g. to use for JavaScript) */
	private ?string $id = null;
	/** show large icon */
	private bool $largeIcon;
	/** @var string|string[]|null $helpId help ID */
	private string|array|null $helpId = null;
	/** headline type (e.g. h3) */
	private string $type;

	/**
	 * Constructor.
	 *
	 * @param string $label label
	 * @param string|null $image optional image
	 * @param string|null $id optional ID for this element (e.g. to use for JavaScript)
	 * @param bool $largeIcon show large (32x32px) icon instead of small one (16x16px)
	 * @param string $type headline type
	 */
	public function __construct(string $label, ?string $image = null, ?string $id = null, bool $largeIcon = false, string $type = self::H3) {
		$this->label = htmlspecialchars($label);
		if ($image !== null) {
			$this->image = htmlspecialchars($image);
		}
		if ($id !== null) {
			$this->id = htmlspecialchars($id);
		}
		// the title should not end at a table cell
		$this->colspan = 100;
		$this->largeIcon = $largeIcon;
		$this->type = htmlspecialchars($type);
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		$idValue = '';
		if ($this->id != null) {
			$idValue = ' id="' . $this->id . '"';
		}
		echo "<div $idValue class=\"subTitle\">\n";
		echo "<" . $this->type ." class=\"subTitleText\">\n";
		if ($this->image != null) {
			$size = $this->largeIcon ? 32 : 16;
			echo '<img height=' . $size . ' width=' . $size . ' src="' . $this->image . '" alt="' . $this->label . '">&nbsp;';
		}
		echo $this->label;
		if ($this->helpId !== null) {
			$spacer = new htmlSpacer('0.5rem', null);
			$spacer->generateHTML($module, $input, $values, $restricted, $scope);
			$helpLink = new htmlHelpLink($this->helpId);
			$helpLink->generateHTML($module, $input, $values, $restricted, $scope);
		}
		echo "</" . $this->type . ">\n";
		echo "</div>\n";
		return [];
	}

	/**
	 * Sets an additional help id.
	 *
	 * @param string|string[] $helpId
	 */
	public function setHelpId(array|string $helpId): void {
		$this->helpId = $helpId;
	}

}

/**
 * Generates a hidden input field.
 *
 * @package metaHTML
 */
class htmlHiddenInput extends htmlElement {

	/** field name */
	private string $name;
	/** field value */
	private string $value = '';

	/**
	 * Constructor.
	 *
	 * @param string $name input name
	 * @param string|null $value input value
	 */
	public function __construct(string $name, ?string $value = '') {
		$this->name = htmlspecialchars($name);
		if ($value !== null) {
			$this->value = htmlspecialchars($value);
		}
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		echo '<input type="hidden" name="' . $this->name . '" id="' . $this->name . '" value="' . $this->value . '">';
		return [$this->name => 'hidden'];
	}

}

/**
 * Generates a link.
 * The link can include an optional image in front of the link text.
 *
 * @package metaHTML
 */
class htmlLink extends htmlElement {

	/** link text */
	private ?string $text = null;
	/** link target */
	protected ?string $target = null;
	/** optional image */
	private ?string $image = null;
	/** title */
	private ?string $title = null;
	/** target window */
	private ?string $targetWindow = null;
	/** onClick event */
	private ?string $onClick = null;
	/** onMouseOver event */
	private ?string $onMouseOver = null;
	/** link id */
	private ?string $id = null;

	/**
	 * Constructor.
	 *
	 * @param string|null $text label
	 * @param string|null $target target URL
	 * @param string|null $image URL of optional image
	 */
	public function __construct(?string $text, ?string $target, ?string $image = null) {
		if ($text !== null) {
			$this->text = htmlspecialchars($text);
		}
		if ($target !== null) {
			$this->target = htmlspecialchars($target);
		}
		if ($image !== null) {
			$this->image = htmlspecialchars($image);
		}
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		$text = $this->getContent();
		$image = '';
		if ($this->image != null) {
			$image = '<img class="align-middle" src="' . $this->image . '" alt="' . $this->getAlt() . '">';
			if (!empty($text)) {
				$image .= '&nbsp;';
			}
		}
		$title = '';
		if ($this->title != null) {
			$title = ' title="' . $this->title . '"';
		}
		$targetWindow = '';
		if ($this->targetWindow != null) {
			$targetWindow = ' target="' . $this->targetWindow . '"';
		}
		$onClick = '';
		if ($this->onClick != null) {
			$onClick = ' onclick="' . $this->onClick . '"';
		}
		$onMouseOver = '';
		if ($this->onMouseOver != null) {
			$onMouseOver = ' onmouseover="' . $this->onMouseOver . '"';
		}
		$idAttr = '';
		if (!empty($this->id)) {
			$idAttr = ' id="' . $this->id . '"';
		}
		$classAttr = '';
		if (count($this->cssClasses) > 0) {
			$classAttr = ' class="' . implode(' ', $this->cssClasses) . '"';
		}
		echo '<a href="' . $this->target . '"' . $idAttr . $classAttr . $title . $targetWindow . $onClick . $onMouseOver . $this->getDataAttributesAsString() . '>' . $image . $text . '</a>';
		return [];
	}

	/**
	 * Returns the value for the alt attribute.
	 *
	 * @return string|null alt value
	 */
	protected function getAlt(): ?string {
		if (!empty($this->text)) {
			return $this->text;
		}
		return $this->title;
	}

	/**
	 * Returns the value for the link content.
	 *
	 * @return string|null content
	 */
	protected function getContent(): ?string {
		return $this->text;
	}

	/**
	 * Sets the link title.
	 *
	 * @param string $title title
	 */
	public function setTitle(string $title): void {
		$this->title = htmlspecialchars($title);
	}

	/**
	 * Sets the target window (e.g. _blank).
	 *
	 * @param string $window target window (e.g. _blank)
	 */
	public function setTargetWindow(string $window): void {
		$this->targetWindow = htmlspecialchars($window);
	}

	/**
	 * Sets the onClick event.
	 *
	 * @param string $event JavaScript code
	 */
	public function setOnClick(string $event): void {
		$this->onClick = htmlspecialchars($event);
	}

	/**
	 * Sets the onMouseOver event.
	 *
	 * @param string $event JavaScript code
	 */
	public function setOnMouseOver(string $event): void {
		$this->onMouseOver = htmlspecialchars($event);
	}

	/**
	 * Sets the element id.
	 *
	 * @param string $id unique id
	 */
	public function setId(string $id): void {
		$this->id = $id;
	}

}

/**
 * Generates a link around a htmlElement.
 *
 * @package metaHTML
 */
class htmlContentLink extends htmlLink {

	private htmlElement $content;
	private string $contentText = '';

	/**
	 * Constructor
	 *
	 * @param htmlElement $content content to link
	 * @param string $target link target
	 */
	public function __construct(htmlElement $content, string $target) {
		$this->content = $content;
		$this->target = htmlspecialchars($target);
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		ob_start();
		parseHtml($module, $this->content, $values, $restricted, $scope);
		$this->contentText = ob_get_contents();
		ob_end_clean();
		return parent::generateHTML($module, $input, $values, $restricted, $scope);
	}

	/**
	 * Returns the value for the alt attribute.
	 *
	 * @return string alt value
	 */
	protected function getAlt(): string {
		return '';
	}

	/**
	 * Returns the value for the link content.
	 *
	 * @return string content
	 */
	protected function getContent(): string {
		return $this->contentText;
	}

}

/**
 * Groups multiple htmlElements.
 * This is useful if multiple elements should be included in a single table cell.
 * The HTML code of the subelements is printed in the order they were added. No additional code is added.
 *
 * @package metaHTML
 */
class htmlGroup extends htmlElement {

	/** @var htmlElement[] $subelements elements */
	private array $subelements = [];

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		$return = [];
		for ($i = 0; $i < count($this->subelements); $i++) {
			$return = array_merge($return, $this->subelements[$i]->generateHTML($module, $input, $values, $restricted, $scope));
		}
		return $return;
	}

	/**
	 * Adds a subelement.
	 *
	 * @param htmlElement $sub subelement
	 */
	public function addElement(htmlElement $sub): void {
		$this->subelements[] = $sub;
	}

}

/**
 * Prints a horizontal line.
 *
 * @package metaHTML
 */
class htmlHorizontalLine extends htmlElement {

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		$return = [];
		echo "<hr>";
		return $return;
	}

}

/**
 * Creates a simple DIV element.
 *
 * @package metaHTML
 */
class htmlDiv extends htmlElement {

	/** unique ID */
	private ?string $id = null;
	/** htmlElement that generates inner content */
	private htmlElement $content;

	/**
	 * Constructor.
	 *
	 * @param string|null $id unique ID
	 * @param htmlElement $content inner content
	 * @param string[]|null $cssClasses CSS classes
	 */
	public function __construct(?string $id, htmlElement $content, ?array $cssClasses = null) {
		if ($id !== null) {
			$this->id = htmlspecialchars($id);
		}
		$this->content = $content;
		$this->cssClasses = $cssClasses;
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		$idValue = '';
		if ($this->id != null) {
			$idValue = ' id="' . $this->id . '"';
		}
		$classesValue = '';
		if (($this->cssClasses !== null) && (count($this->cssClasses) > 0)) {
			$classesValue = ' class="' . implode(' ', $this->cssClasses) . '"';
		}
		echo '<div' . $idValue . $classesValue . $this->getDataAttributesAsString() . '>';
		$return = $this->content->generateHTML($module, $input, $values, $restricted, $scope);
		echo '</div>';
		return $return;
	}
}

/**
 * Creates a simple SPAN element.
 *
 * @package metaHTML
 */
class htmlSpan extends htmlElement {

	/** htmlElement that generates inner content */
	private htmlElement $content;
	/** onclick handler */
	private ?string $onclick = null;
	/** title */
	private ?string $title = null;

	/**
	 * Constructor.
	 *
	 * @param htmlElement $content inner content
	 * @param string[]|null $cssClasses CSS classes
	 */
	public function __construct(htmlElement $content, ?array $cssClasses = null) {
		$this->content = $content;
		$this->cssClasses = $cssClasses;
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		$classesValue = '';
		if (($this->cssClasses !== null) && (count($this->cssClasses) > 0)) {
			$classesValue = ' class="' . implode(' ', $this->cssClasses) . '"';
		}
		$onclickHandler = '';
		if (!empty($this->onclick)) {
			$onclickHandler = ' onclick="' . $this->onclick . '"';
		}
		$titleCode = '';
		if ($this->title !== null) {
			$titleCode = ' title="' . $this->title . '"';
		}
		echo '<span' . $classesValue . $titleCode . $onclickHandler . '>';
		$return = $this->content->generateHTML($module, $input, $values, $restricted, $scope);
		echo '</span>';
		return $return;
	}

	/**
	 * Sets the onclick event.
	 *
	 * @param string $event event handler code
	 */
	public function setOnclick(string $event): void {
		$this->onclick = $event;
	}

	/**
	 * Sets the title.
	 *
	 * @param string $title title
	 */
	public function setTitle(string $title): void {
		$this->title = htmlspecialchars($title);
	}

}

/**
 * Creates a JavaScript element.
 *
 * @package metaHTML
 */
class htmlJavaScript extends htmlElement {

	/** htmlElement that generates inner content */
	private string $content;

	/**
	 * Constructor.
	 *
	 * @param string $content script
	 */
	public function __construct(string $content) {
		$this->content = $content;
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		$return = [];
		echo '<script type="text/javascript">';
		echo $this->content;
		echo '</script>';
		return $return;
	}

}

/**
 * Creates a iframe element.
 *
 * @package metaHTML
 */
class htmlIframe extends htmlElement {

	/** HTML id */
	private ?string $id;

	/**
	 * Constructor.
	 *
	 * @param string|null $id HTML id
	 */
	public function __construct(?string $id = null) {
		$this->id = $id;
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		$return = [];
		$idAttr = '';
		if (!empty($this->id)) {
			$idAttr = ' id="' . $this->id . '"';
		}
		echo '<iframe ' . $idAttr . $this->getDataAttributesAsString() . '>';
		echo '</iframe>';
		return $return;
	}

}

/**
 * Creates a Script element to integrate external JavaScript files.
 *
 * @package metaHTML
 */
class htmlScript extends htmlElement {

	/** src value */
	private string $src;
	/** is async */
	private bool $async;
	/** execute after page is parsed */
	private bool $defer;

	/**
	 * Constructor.
	 *
	 * @param string $src script path
	 * @param boolean $isAsync script will be executed while the page continues the parsing (default true)
	 * @param boolean $isDeferred script is executed when the page has finished parsing (default true)
	 */
	public function __construct(string $src, bool $isAsync = true, bool $isDeferred = true) {
		$this->src = $src;
		$this->async = $isAsync;
		$this->defer = $isDeferred;
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		$return = [];
		$async = $this->async ? ' async' : '';
		$defer = $this->defer ? ' defer="defer"' : '';
		echo '<script src="' . $this->src . '"' . $async . $defer . '>';
		echo '</script>';
		return $return;
	}

}

/**
 * Creates a link element to integrate external CSS files.
 *
 * @package metaHTML
 */
class htmlLinkCss extends htmlElement {

	/** src value */
	private string $src;

	/**
	 * Constructor.
	 *
	 * @param string $src script path
	 */
	public function __construct(string $src) {
		$this->src = $src;
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		$return = [];
		echo '<link href="' . $this->src . '" type="text/css" rel="stylesheet"/>';
		return $return;
	}

}

/**
 * Creates a list of elements that can be sorted by the user via drag'n'drop.
 *
 * @package metaHTML
 */
class htmlSortableList extends htmlElement {

	/** @var string[]|htmlElement[] $elements list of elements */
	private array $elements;
	/** HTML ID */
	private string $id;
	/** on update event */
	private ?string $onUpdate = null;

	/**
	 * Constructor.
	 *
	 * @param string[]|htmlElement[] $elements list of elements as text (HTML special chars must be escaped already) or htmlElement
	 * @param string $id HTML ID
	 */
	public function __construct(array $elements, string $id) {
		$this->elements = $elements;
		$this->id = htmlspecialchars($id);
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		if (count($this->elements) === 0) {
			return [];
		}
		$return = [];
		$classesValue = '';
		if (!empty($this->cssClasses)) {
			$classesValue = ' class="' . implode(' ', $this->cssClasses) . '"';
		}
		echo '<ul' . $classesValue . ' id="' . $this->id . '">';
		for ($i = 0; $i < count($this->elements); $i++) {
			$element = $this->elements[$i];
			echo '<li data-position-orig="' . $i . '">';
			echo '<span class="lam-sortable-list-icon"></span>';
			if ($element instanceof htmlElement) {
				parseHtml($module, $element, $values, $restricted, $scope);
			}
			else {
				echo $element;
			}
			echo '</li>';
		}
		echo '</ul>';
		$onUpdate = '';
		if ($this->onUpdate !== null) {
			$onUpdate = 'onUpdate: ' . $this->onUpdate;
		}
		$scriptContent = '
			Sortable.create(
				document.getElementById("' . $this->id . '"),
				{
					' . $onUpdate . '
				}
			);';
		$script = new htmlJavaScript($scriptContent);
		$script->generateHTML($module, $input, $values, $restricted, $scope);
		return $return;
	}

	/**
	 * Sets the JS code that is executed when the element order was changed.
	 *
	 * @param string $onUpdate JS code
	 */
	public function setOnUpdate(string $onUpdate): void {
		$this->onUpdate = $onUpdate;
	}

}

/**
 * Creates a list of content elements in accordion style.
 * HTML special characters must be escaped before providing to htmlAccordion.
 */
class htmlAccordion extends htmlElement {

	private string $id;
	/** @var array<string, htmlElement> $elements elements */
	private array $elements;
	/** @var int|false $openInitial initially opened tab */
	private mixed $openInitial;
	private bool $saveState = false;
	private bool $allowResorting = false;

	/**
	 * Constructor.
	 *
	 * @param string $id HTML ID
	 * @param array<string, htmlElement> $elements list of content elements array('title' => htmlElement)
	 * @param int|false $openInitial index of element that is initially opened (default: 0), set to 'false' to close all
	 */
	public function __construct(string $id, array $elements, mixed $openInitial = 0) {
		$this->id = $id;
		$this->elements = $elements;
		$this->openInitial = $openInitial;
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		$result = [];
		if ($this->openInitial !== false) {
			$this->addDataAttribute('openinitial', (string) $this->openInitial);
		}
		$cssClasses = $this->getCSSClasses();
		$cssClasses[] = 'lam-accordion-container';
		if ($this->saveState) {
			$this->addDataAttribute('savestate', 'true');
		}
		echo '<div id="' . $this->id . '" class="' . implode(' ', $cssClasses) . '"' . $this->getDataAttributesAsString() . '>';
		$contentIndex = 0;
		foreach ($this->elements as $label => $content) {
			echo '<button id="' . $this->id . '_' . $contentIndex . '" class="lam-accordion-button" onclick="return false;" data-index="' . $contentIndex . '">';
			echo $label;
			echo '</button>';
			echo '<div class="lam-accordion-content" data-index="' . $contentIndex . '">';
			echo '<div class="lam-accordion-content-inner">';
			$result = array_merge($result, $content->generateHTML($module, $input, $values, $restricted, $scope));
			echo '</div>';
			echo '</div>';
			$contentIndex++;
		}
		$hiddenIndexId = $this->id . "_index";
		echo '<input type="hidden" name="' . $hiddenIndexId . '" id="' . $hiddenIndexId . '" value="false">';
		echo '</div>';
		if ($this->allowResorting) {
			$onResorting = 'onUpdate: function() {
				window.lam.html.updateAccordionSorting("' . $this->id . '");
			}';
			$scriptContent = '
			Sortable.create(
				document.getElementById("' . $this->id . '"),
				{
					' . $onResorting . '
				}
			);';
			$resortingGroup = new htmlGroup();
			$script = new htmlJavaScript($scriptContent);
			$resortingGroup->addElement($script);
			$resortingGroup->addElement(new htmlHiddenInput($this->id . '_sorting', ''));
			$resortingGroup->generateHTML($module, $input, $values, $restricted, $scope);
		}
		return $result;
	}

	/**
	 * Saves the state of open/closed items over page reloads.
	 *
	 * @param bool $saveState save state
	 */
	public function saveState(bool $saveState = true): void {
		$this->saveState = $saveState;
	}

	/**
	 * Allows the user to resort the accordion elements.
	 */
	public function allowResorting(): void {
		$this->allowResorting = true;
	}

}

/**
 * Responsive row with 12 column layout.
 */
class htmlResponsiveRow extends htmlElement {

	/** @var htmlResponsiveCell[] $cells cells */
	private array $cells = [];
	/** HTML ID */
	private ?string $id = null;
	private ?string $onClick = null;

	/**
	 * Creates a new responsive row.
	 *
	 * @param htmlElement $label label element if this is a simple label+field row
	 * @param htmlElement $field field element if this is a simple label+field row
	 */
	public function __construct($label = null, $field = null) {
		if ($label != null) {
			$this->addLabel($label);
		}
		if ($field != null) {
			$this->addField($field);
		}
	}

	/**
	 * Sets the HTML id.
	 *
	 * @param string|null $id ID
	 */
	public function setId(?string $id): void {
		$this->id = $id;
	}

	/**
	 * Adds a responsive cell at the end of the row.
	 *
	 * @param htmlResponsiveCell $cell cell
	 */
	public function addCell(htmlResponsiveCell $cell): void {
		$this->cells[] = $cell;
	}

	/**
	 * Adds a responsive cell at the beginning of the row.
	 *
	 * @param htmlResponsiveCell $cell cell
	 */
	public function addCellAtTheBeginning(htmlResponsiveCell $cell): void {
		array_unshift($this->cells, $cell);
	}

	/**
	 * Adds a cell with the given content and column counts.
	 *
	 * @param htmlElement $content content inside cell
	 * @param int $numMobile number of columns for mobile view
	 * @param int|null $numTablet number of columns for tablet view (set to mobile if null)
	 * @param int|null $numDesktop number of columns for desktop view (set to tablet if null)
	 * @param string $classes CSS classes separated by space
	 */
	public function add(htmlElement $content, int $numMobile = 12, ?int $numTablet = null, ?int $numDesktop = null, string $classes = ''): void {
		$tabletCols = $numTablet ?? $numMobile;
		$desktopCols = $numDesktop ?? $tabletCols;
		$this->addCell(new htmlResponsiveCell($content, $numMobile, $tabletCols, $desktopCols, $classes));
	}

	/**
	 * Adds a cell at the beginning with the given content and column counts.
	 *
	 * @param htmlElement $content content inside cell
	 * @param int $numMobile number of columns for mobile view
	 * @param int|null $numTablet number of columns for tablet view (set to mobile if null)
	 * @param int|null $numDesktop number of columns for desktop view (set to tablet if null)
	 * @param string $classes CSS classes separated by space
	 */
	public function addAtTheBeginning(htmlElement $content, int $numMobile = 12, ?int $numTablet = null, ?int $numDesktop = null, string $classes = ''): void {
		$tabletCols = $numTablet ?? $numMobile;
		$desktopCols = $numDesktop ?? $tabletCols;
		$this->addCellAtTheBeginning(new htmlResponsiveCell($content, $numMobile, $tabletCols, $desktopCols, $classes));
	}

	/**
	 * Adds the content as a typical label with 12/6/6 columns and CSS class "responsiveLabel".
	 *
	 * @param htmlElement $content label
	 * @param string $cssClasses additional CSS classes
	 */
	public function addLabel(htmlElement $content, string $cssClasses = ''): void {
		$this->add($content, 12, 6, 6, 'responsiveLabel nowrap ' . $cssClasses);
	}

	/**
	 * Adds the content as a typical field with 12/6/6 columns and CSS class "responsiveField".
	 *
	 * @param htmlElement $content field
	 * @param string $cssClasses CSS class names separated by space
	 */
	public function addField(htmlElement $content, string $cssClasses = ''): void {
		$this->add($content, 12, 6, 6, 'responsiveField ' . $cssClasses);
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		$return = [];
		$cssClasses = implode(' ', $this->cssClasses);
		$idParam = '';
		if ($this->id !== null) {
			$idParam = ' id="' . $this->id . '"';
		}
		$onClickCode = '';
		if (!empty($this->onClick)) {
			$onClickCode = ' onclick="' . $this->onClick . '"';
		}
		echo '<div class="row ' . $cssClasses . '"' . $this->getDataAttributesAsString() . $idParam . $onClickCode . '>';
		foreach ($this->cells as $cell) {
			$return = array_merge($return, $cell->generateHTML($module, $input, $values, $restricted, $scope));
		}
		echo '</div>';
		return $return;
	}

	/**
	 * Adds a vertical spacer with 12 columns.
	 *
	 * @param string $space space in px or rem
	 */
	public function addVerticalSpacer(string $space): void {
		$this->add(new htmlSpacer(null, $space));
	}

	/**
	 * Returns the current number of cells.
	 *
	 * @return int cell count
	 */
	public function getCellCount(): int {
		return count($this->cells);
	}

	public function setOnClick(string $code): void {
		$this->onClick = $code;
	}

}

/**
 * Responsive cell inside htmlResponsiveRow with 12 column layout.
 */
class htmlResponsiveCell extends htmlElement {

	private htmlElement $content;
	private int $mobile;
	private ?int $tablet;
	private ?int $desktop;
	private string $classes;

	/**
	 * Constructs a cell inside a responsive row with 12 columns.
	 *
	 * @param htmlElement $content content inside cell
	 * @param int $numMobile number of columns for mobile view
	 * @param int|null $numTablet number of columns for tablet view
	 * @param int|null $numDesktop number of columns for desktop view
	 * @param string $classes CSS classes separated by space
	 */
	public function __construct(htmlElement $content, int $numMobile, ?int $numTablet, ?int $numDesktop, string $classes = '') {
		$this->content = $content;
		$this->mobile = $numMobile;
		$this->tablet = $numTablet;
		$this->desktop = $numDesktop;
		$this->classes = $classes;
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		$clMobile = ($this->mobile > 0) ? 'small-' . $this->mobile : 'hide-for-small-only';
		$clTablet = ($this->tablet > 0) ? 'medium-' . $this->tablet : 'hide-for-medium-only';
		$clDesktop = ($this->desktop > 0) ? 'large-' . $this->desktop : 'hide-for-large-only';

		echo '<div class="' . $clMobile . ' ' . $clTablet . ' ' . $clDesktop . ' columns ' . $this->classes . '">';
		$return = $this->content->generateHTML($module, $input, $values, $restricted, $scope);
		echo '</div>';
		return $return;
	}

}

/**
 * A responsive input field that combines label, input field and help.
 *
 * @package metaHTML
 */
class htmlResponsiveInputField extends htmlInputField {

	/** Descriptive label */
	private string $label;
	/** help ID */
	private ?string $helpID = null;
	/** help module name */
	private ?string $helpModule = null;
	/** render HTML of parent class */
	private bool $renderParentHtml = false;
	/** short label */
	private bool $shortLabel = false;

	/**
	 * Constructor
	 *
	 * @param string $label descriptive label
	 * @param string $fieldName unique field name
	 * @param string $fieldValue value of input field (optional)
	 * @param string|string[]|null $helpID help ID or array of help ID + module name (optional)
	 * @param bool $required field is required
	 */
	public function __construct($label, $fieldName, $fieldValue = null, $helpID = null, $required = false) {
		parent::__construct($fieldName, $fieldValue);
		$this->label = $label;
		if (is_string($helpID)) {
			$this->helpID = $helpID;
		}
		elseif (is_array($helpID)) {
			$this->helpID = $helpID[0];
			$this->helpModule = $helpID[1];
		}
		$this->fieldSize = null;
		$this->required = $required;
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		if ($this->renderParentHtml) {
			return parent::generateHTML($module, $input, $values, $restricted, $scope);
		}
		// HTML of parent class is rendered on second call (done by htmlResponsiveRow)
		$this->renderParentHtml = true;
		$row = new htmlResponsiveRow();
		// label text
		$labelGroup = new htmlGroup();
		$idValue = $this->id ?? $this->fieldName;
		$labelGroup->addElement(new htmlLabel($idValue, $this->label));
		if ($this->required) {
			$labelGroup->addElement(htmlGetRequiredMarkerElement());
		}
		if (!empty($this->helpID)) {
			$helpLinkLabel = new htmlHelpLink($this->helpID, $this->helpModule);
			$helpLinkLabel->setCSSClasses(['hide-on-tablet', 'margin-left5']);
			$labelGroup->addElement($helpLinkLabel);
		}
		$tabletDesktopLabelColumns = $this->shortLabel ? 4 : 6;
		$row->add($labelGroup, 12, $tabletDesktopLabelColumns, $tabletDesktopLabelColumns, 'responsiveLabel');
		// input field
		$fieldGroup = new htmlGroup();
		$fieldGroup->addElement($this);
		if (!empty($this->helpID)) {
			$helpLinkField = new htmlHelpLink($this->helpID, $this->helpModule);
			$helpLinkField->setCSSClasses(['hide-on-mobile']);
			$fieldGroup->addElement($helpLinkField);
		}
		$tabletDesktopFieldColumns = $this->shortLabel ? 8 : 6;
		$row->add($fieldGroup, 12, $tabletDesktopFieldColumns, $tabletDesktopFieldColumns, 'responsiveField nowrap');
		return $row->generateHTML($module, $input, $values, $restricted, $scope);
	}

	/**
	 * Use a short label (4 columns instead of 6) for tablet/desktop.
	 */
	public function setShortLabel(): void {
		$this->shortLabel = true;
	}

	/**
	 * Sets the help ID.
	 *
	 * @param string|null $helpID help ID
	 */
	public function setHelpId(?string $helpID): void {
		$this->helpID = $helpID;
	}

	/**
	 * Sets the help module.
	 *
	 * @param string|null $helpModule help module name
	 */
	public function setHelpModule(?string $helpModule): void {
		$this->helpModule = $helpModule;
	}

}

/**
 * File upload with descriptive label and help link.
 *
 * @package metaHTML
 */
class htmlResponsiveInputFileUpload extends htmlInputFileUpload {

	/** descriptive label */
	private string $label;
	/** help ID */
	private ?string $helpID = null;
	/** help module */
	private ?string $helpModule = null;
	/** render HTML of parent class */
	private bool $renderParentHtml = false;

	/**
	 * Constructor.
	 *
	 * @param string $name unique name
	 * @param string $label descriptive label
	 * @param string|string[]|null $helpID help ID
	 * @param bool $required required field
	 */
	public function __construct($name, string $label, $helpID = null, $required = false) {
		parent::__construct($name);
		$this->label = htmlspecialchars($label);
		if (is_string($helpID)) {
			$this->helpID = $helpID;
		}
		elseif (is_array($helpID)) {
			$this->helpID = $helpID[0];
			$this->helpModule = $helpID[1];
		}
		$this->required = $required;
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		if ($this->renderParentHtml) {
			return parent::generateHTML($module, $input, $values, $restricted, $scope);
		}
		// HTML of parent class is rendered on second call (done by htmlResponsiveRow)
		$this->renderParentHtml = true;
		$row = new htmlResponsiveRow();
		// label text
		$labelGroup = new htmlGroup();
		$labelGroup->addElement(new htmlOutputText($this->label));
		if ($this->required) {
			$labelGroup->addElement(htmlGetRequiredMarkerElement());
		}
		if (!empty($this->helpID)) {
			$helpLinkLabel = new htmlHelpLink($this->helpID, $this->helpModule);
			$helpLinkLabel->setCSSClasses(['hide-on-tablet', 'margin-left5']);
			$labelGroup->addElement($helpLinkLabel);
		}
		$row->add($labelGroup, 12, 6, 6, 'responsiveLabel');
		// input field
		$fieldGroup = new htmlGroup();
		$fieldGroup->addElement($this);
		if (!empty($this->helpID)) {
			$helpLinkField = new htmlHelpLink($this->helpID, $this->helpModule);
			$helpLinkField->setCSSClasses(['hide-on-mobile']);
			$fieldGroup->addElement($helpLinkField);
		}
		$row->add($fieldGroup, 12, 6, 6, 'responsiveField nowrap');
		return $row->generateHTML($module, $input, $values, $restricted, $scope);
	}

}

/**
 * Responsive text area with label and help link.
 *
 * @package metaHTML
 */
class htmlResponsiveInputTextarea extends htmlInputTextarea {

	/** descriptive label */
	private string $label;
	/** help ID */
	private ?string $helpID = null;
	/** help module */
	private ?string $helpModule = null;
	/** render HTML of parent class */
	private bool $renderParentHtml = false;
	/** short label */
	private bool $shortLabel = false;

	/**
	 * Constructor.
	 *
	 * @param string $name unique name
	 * @param string $value value
	 * @param int $colCount number of characters per line
	 * @param int $rowCount number of rows
	 * @param string $label descriptive label
	 * @param string|string[] $helpID help ID
	 */
	public function __construct($name, $value, $colCount, $rowCount, string $label, $helpID = null) {
		parent::__construct($name, $value, $colCount, $rowCount);
		$this->label = htmlspecialchars($label);
		if (is_string($helpID)) {
			$this->helpID = $helpID;
		}
		elseif (is_array($helpID)) {
			$this->helpID = $helpID[0];
			$this->helpModule = $helpID[1];
		}
		$this->alignment = htmlElement::ALIGN_TOP;
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		if ($this->renderParentHtml) {
			return parent::generateHTML($module, $input, $values, $restricted, $scope);
		}
		// HTML of parent class is rendered on second call (done by htmlResponsiveRow)
		$this->renderParentHtml = true;
		$row = new htmlResponsiveRow();
		// label text
		$labelGroup = new htmlGroup();
		$labelGroup->addElement(new htmlLabel($this->name, $this->label));
		if ($this->required) {
			$labelGroup->addElement(htmlGetRequiredMarkerElement());
		}
		if (!empty($this->helpID)) {
			$helpLinkLabel = new htmlHelpLink($this->helpID, $this->helpModule);
			$helpCssClasses = ['margin-left5'];
			if (!$this->richEdit) {
				$helpCssClasses[] = 'hide-on-tablet';
			}
			$helpLinkLabel->setCSSClasses($helpCssClasses);
			$labelGroup->addElement($helpLinkLabel);
		}
		$tabletDesktopLabelColumns = $this->shortLabel ? 4 : 6;
		$row->add($labelGroup, 12, $tabletDesktopLabelColumns, $tabletDesktopLabelColumns, 'responsiveLabel');
		// input field
		$fieldGroup = new htmlGroup();
		$fieldGroup->addElement($this);
		if (!empty($this->helpID) && !$this->richEdit) {
			$helpLink = new htmlHelpLink($this->helpID, $this->helpModule);
			$helpLink->setCSSClasses(['align-top', 'hide-on-mobile']);
			$fieldGroup->addElement($helpLink);
		}
		$tabletDesktopFieldColumns = $this->shortLabel ? 8 : 6;
		$row->add($fieldGroup, 12, $tabletDesktopFieldColumns, $tabletDesktopFieldColumns, 'responsiveField nowrap');
		return $row->generateHTML($module, $input, $values, $restricted, $scope);
	}

	/**
	 * Use a short label (4 columns instead of 6) for tablet/desktop.
	 */
	public function setShortLabel(): void {
		$this->shortLabel = true;
	}

	/**
	 * Sets the help ID.
	 *
	 * @param string|null $helpID help ID
	 */
	public function setHelpId(?string $helpID): void {
		$this->helpID = $helpID;
	}

	/**
	 * Sets the help module.
	 *
	 * @param string|null $helpModule help module name
	 */
	public function setHelpModule(?string $helpModule): void {
		$this->helpModule = $helpModule;
	}

}

/**
 * Responsive select with label and help link.
 *
 * @package metaHTML
 */
class htmlResponsiveSelect extends htmlSelect {

	/** descriptive label */
	private string $label;
	/** help ID */
	private ?string $helpID = null;
	/** help module name */
	private ?string $helpModule = null;
	/** render HTML of parent class */
	private bool $renderParentHtml = false;
	/** short label */
	private bool $shortLabel = false;

	/**
	 * Constructor.
	 *
	 * @param string $name element name
	 * @param string[]|array<string, string>|array<string, string[]|array<string, string>> $elements list of elements
	 * @param string[] $selectedElements list of selected elements
	 * @param string $label descriptive label
	 * @param string|string[] $helpID help ID or array of help ID + module name (optional)
	 * @param int $size size (optional, default = 1)
	 */
	public function __construct($name, $elements, $selectedElements, string $label, $helpID = null, $size = 1) {
		parent::__construct($name, $elements, $selectedElements, $size);
		$this->label = $label;
		if (is_string($helpID)) {
			$this->helpID = $helpID;
		}
		elseif (is_array($helpID)) {
			$this->helpID = $helpID[0];
			$this->helpModule = $helpID[1];
		}
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		if ($this->renderParentHtml) {
			return parent::generateHTML($module, $input, $values, $restricted, $scope);
		}
		// HTML of parent class is rendered on second call (done by htmlResponsiveRow)
		$this->renderParentHtml = true;
		$row = new htmlResponsiveRow();
		// label text
		$labelGroup = new htmlGroup();
		$labelGroup->addElement(new htmlLabel($this->name, $this->label));
		if (!empty($this->helpID)) {
			$helpLinkLabel = new htmlHelpLink($this->helpID, $this->helpModule);
			$helpLinkLabel->setCSSClasses(['hide-on-tablet', 'margin-left5']);
			$labelGroup->addElement($helpLinkLabel);
		}
		$tabletDesktopLabelColumns = $this->shortLabel ? 4 : 6;
		$row->add($labelGroup, 12, $tabletDesktopLabelColumns, $tabletDesktopLabelColumns, 'responsiveLabel');
		// input field
		$fieldGroup = new htmlGroup();
		$fieldGroup->addElement($this);
		if (!empty($this->helpID)) {
			$helpLink = new htmlHelpLink($this->helpID, $this->helpModule);
			$helpLink->setCSSClasses(['hide-on-mobile']);
			$fieldGroup->addElement($helpLink);
		}
		$tabletDesktopFieldColumns = $this->shortLabel ? 8 : 6;
		$row->add($fieldGroup, 12, $tabletDesktopFieldColumns, $tabletDesktopFieldColumns, 'responsiveField nowrap');
		return $row->generateHTML($module, $input, $values, $restricted, $scope);
	}

	/**
	 * {@inheritDoc}
	 * @see htmlSelect::getShowHideSelector()
	 */
	protected function getShowHideSelector(): string {
		return '.row';
	}

	/**
	 * Use a short label (4 columns instead of 6) for tablet/desktop.
	 */
	public function setShortLabel(): void {
		$this->shortLabel = true;
	}

}

/**
 * Responsive select with label and help link.
 *
 * @package metaHTML
 */
class htmlResponsiveRadio extends htmlRadio {

	/** descriptive label */
	private string $label;
	/** help ID */
	private ?string $helpID;
	/** render HTML of parent class */
	private bool $renderParentHtml = false;

	/**
	 * Constructor.
	 *
	 * @param string $label descriptive label
	 * @param string $name element name
	 * @param array<string, string> $elements list of elements
	 * @param string|null $selectedElement list of selected elements
	 * @param string|null $helpID help ID (optional, default none)
	 */
	public function __construct(string $label, $name, $elements, ?string $selectedElement = null, ?string $helpID = null) {
		parent::__construct($name, $elements, $selectedElement);
		$this->label = htmlspecialchars($label);
		$this->helpID = $helpID;
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		if ($this->renderParentHtml) {
			return parent::generateHTML($module, $input, $values, $restricted, $scope);
		}
		// HTML of parent class is rendered on second call (done by htmlResponsiveRow)
		$this->renderParentHtml = true;
		$row = new htmlResponsiveRow();
		// label text
		$labelGroup = new htmlGroup();
		$labelGroup->addElement(new htmlOutputText($this->label));
		if (!empty($this->helpID)) {
			$helpLinkLabel = new htmlHelpLink($this->helpID);
			$helpLinkLabel->setCSSClasses(['hide-on-tablet', 'margin-left5']);
			$labelGroup->addElement($helpLinkLabel);
		}
		$row->add($labelGroup, 12, 6, 6, 'responsiveLabel');
		// input field
		$fieldGroup = new htmlGroup();
		$fieldGroup->addElement(new htmlDiv(null, $this, ['float-left']));
		if (!empty($this->helpID)) {
			$helpLink = new htmlHelpLink($this->helpID);
			$helpLink->setCSSClasses(['hide-on-mobile']);
			$fieldGroup->addElement(new htmlDiv(null, $helpLink));
		}
		$row->add($fieldGroup, 12, 6, 6, 'responsiveField nowrap');
		return $row->generateHTML($module, $input, $values, $restricted, $scope);
	}

	/**
	 * Returns the selector to use to find the show/hide elements.
	 *
	 * @return string selector
	 */
	protected function getShowHideSelector(): string {
		return '.row';
	}

}

/**
 * Responsive checkbox with descriptive label and help link.
 *
 * @package metaHTML
 */
class htmlResponsiveInputCheckbox extends htmlInputCheckbox {

	/** descriptive label */
	private string $label;
	/** help ID */
	private ?string $helpID = null;
	/** help module name */
	private ?string $helpModule = null;
	/** render HTML of parent class */
	private bool $renderParentHtml = false;
	/** long label */
	private bool $longLabel;
	/** label after checkbox */
	private bool $labelAfterCheckbox = false;
	/** short label */
	private bool $shortLabel = false;

	/**
	 * Constructor.
	 *
	 * @param string $name unique name
	 * @param bool $checked checked
	 * @param string $label descriptive label
	 * @param string|string[] $helpID help ID or array of help ID + module name (optional)
	 * @param bool $longLabel more space for label (default: false)
	 */
	public function __construct($name, $checked, string $label, $helpID = null, $longLabel = false) {
		parent::__construct($name, $checked);
		$this->label = $label;
		if (is_string($helpID)) {
			$this->helpID = $helpID;
		}
		elseif (is_array($helpID)) {
			$this->helpID = $helpID[0];
			$this->helpModule = $helpID[1];
		}
		$this->longLabel = $longLabel;
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		if ($this->renderParentHtml) {
			return parent::generateHTML($module, $input, $values, $restricted, $scope);
		}
		// HTML of parent class is rendered on second call (done by htmlResponsiveRow)
		$this->renderParentHtml = true;
		$row = new htmlResponsiveRow();
		$tabletColumnsLabel = 6;
		$tabletColumnsBox = 6;
		$mobileColumnsLabel = 10;
		$mobileColumnsBox = 2;
		if ($this->longLabel) {
			$tabletColumnsLabel = 10;
			$tabletColumnsBox = 2;
		}
		if ($this->shortLabel) {
			$tabletColumnsLabel = 4;
			$tabletColumnsBox = 8;
		}
		// label text
		$text = new htmlLabel($this->getName(), $this->label);
		$text->setCSSClasses($this->cssClasses);
		// input field
		$fieldGroup = new htmlGroup();
		$fieldGroup->addElement($this);
		if (!empty($this->helpID)) {
			$helpLink = new htmlHelpLink($this->helpID, $this->helpModule);
			$helpLink->setCSSClasses(['margin-left5 align-unset-img']);
			$fieldGroup->addElement($helpLink);
		}
		if ($this->labelAfterCheckbox) {
			$row->add($fieldGroup, $mobileColumnsBox, $tabletColumnsBox, $tabletColumnsBox, 'responsiveLabel nowrap');
			$row->add($text, $mobileColumnsLabel, $tabletColumnsLabel, $tabletColumnsLabel, 'responsiveField');
		}
		else {
			$row->add($text, $mobileColumnsLabel, $tabletColumnsLabel, $tabletColumnsLabel, 'responsiveLabel');
			$row->add($fieldGroup, $mobileColumnsBox, $tabletColumnsBox, $tabletColumnsBox, 'responsiveField nowrap');
		}
		return $row->generateHTML($module, $input, $values, $restricted, $scope);
	}

	/**
	 * {@inheritDoc}
	 * @see htmlInputCheckbox::getShowHideSelector()
	 */
	protected function getShowHideSelector(): string {
		return '.row';
	}

	/**
	 * Sets if the label should be shown after the checkbox instead before it.
	 *
	 * @param bool $labelAfterCheckbox show label after box
	 */
	public function setLabelAfterCheckbox(bool $labelAfterCheckbox = true): void {
		$this->labelAfterCheckbox = $labelAfterCheckbox;
	}

	/**
	 * Use a short label (4 columns instead of 6) for tablet/desktop.
	 */
	public function setShortLabel(): void {
		$this->shortLabel = true;
	}

}

/**
 * Responsive table.
 *
 * @author roland Gruber
 */
class htmlResponsiveTable extends htmlElement {

	/** @var string[] $titles row titles */
	private array $titles;

	/** @var htmlElement[][] $data data rows */
	private array $data;

	/** @var string[] $widths widths of the columns */
	private array $widths = [];

	/** @var int[] $highlighted highlighted rows */
	private array $highlighted = [];
	/** @var array<int, string> $onClick onclick code (row number => code) */
	private array $onClick = [];
	/** @var array<int, string> $onDoubleClick ondoubleclick code (row number => code) */
	private array $onDoubleClick = [];

	/**
	 * Creates the table.
	 *
	 * @param string[] $titles row titles
	 * @param htmlElement[][] $data data rows
	 * @param int[]|null $highlighted list of row numbers that should be highlighted (starting at 0)
	 */
	public function __construct(array $titles, array $data, ?array $highlighted = null) {
		$this->titles = $titles;
		$this->data = $data;
		if (!empty($highlighted)) {
			$this->highlighted = $highlighted;
		}
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		$return = [];
		$classes = $this->cssClasses;
		$classes[] = 'responsive-table';
		echo '<table class="' . implode(' ', $classes) . '">';
		echo '<thead>';
		$headClass = empty($this->cssOddRow) ? '' : ' class="' . $this->cssOddRow . '"';
		echo '<tr ' . $headClass . '>';
		$counter = 0;
		foreach ($this->titles as $title) {
			$width = '';
			if (isset($this->widths[$counter])) {
				$width = 'width="' . $this->widths[$counter] . '"';
			}
			echo '<th ' . $width . '>' . htmlspecialchars($title) . '</th>';
			$counter++;
		}
		echo '</tr>';
		echo '</thead>';
		echo '<tbody>';
		$titleCount = count($this->titles);
		$counter = 0;
		foreach ($this->data as $row) {
			$cssClass = '';
			$cssClasses = [];
			if (in_array($counter, $this->highlighted)) {
				$cssClasses[] = 'highlighted';
			}
			if (!empty($cssClasses)) {
				$cssClass = ' class="' . implode(' ', $cssClasses) . '"';
			}
			$onClick = '';
			if (!empty($this->onClick[$counter])) {
				$onClick = ' onclick="' . $this->onClick[$counter] . '"';
			}
			$onDoubleClick = '';
			if (!empty($this->onDoubleClick[$counter])) {
				$onDoubleClick = ' ondblclick="' . $this->onDoubleClick[$counter] . '"';
			}
			echo '<tr ' . $cssClass . $onClick . $onDoubleClick . '>';
			for ($i = 0; $i < $titleCount; $i++) {
				echo '<td data-label="' . $this->titles[$i] . '">';
				$ids = parseHtml($module, $row[$i], $values, $restricted, $scope);
				$return = array_merge($return, $ids);
				echo '</td>';
			}
			echo '</tr>';
			$counter++;
		}
		echo '</tbody>';
		echo '</table>';
		return $return;
	}

	/**
	 * Sets the width of each column.
	 *
	 * @param string[] $widths widths
	 */
	public function setWidths(array $widths): void {
		$this->widths = $widths;
	}

	/**
	 * Sets the onclick code for the rows.
	 *
	 * @param array<int, string> $calls row number => code
	 */
	public function setOnClickEvents(array $calls): void {
		$this->onClick = $calls;
	}

	/**
	 * Sets the ondoubleclick code for the rows.
	 *
	 * @param array<int, string> $calls row number => code
	 */
	public function setOnDoubleClickEvents(array $calls): void {
		$this->onDoubleClick = $calls;
	}

}

/**
 * Renders a canvas.
 *
 * @author Roland Gruber
 */
class htmlCanvas extends htmlElement {

	private string $id;

	/**
	 * Constructor
	 *
	 * @param string $id html id
	 */
	public function __construct(string $id) {
		$this->id = $id;
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		$classesValue = '';
		if (!empty($this->cssClasses)) {
			$classesValue = ' class="' . implode(' ', $this->cssClasses) . '"';
		}
		echo '<canvas id="' . $this->id . '" ' . $classesValue . '>';
		echo '</canvas>';
		return [];
	}
}

/**
 * Renders a video.
 *
 * @author Roland Gruber
 */
class htmlVideo extends htmlElement {

	private string $id;

	/**
	 * Constructor
	 *
	 * @param string $id html id
	 */
	public function __construct(string $id) {
		$this->id = $id;
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		$classesValue = '';
		if (!empty($this->cssClasses)) {
			$classesValue = ' class="' . implode(' ', $this->cssClasses) . '"';
		}
		echo '<video id="' . $this->id . '" ' . $classesValue . '>';
		echo '</video>';
		return [];
	}
}

/**
 * Creates a form element for POST.
 *
 * @package metaHTML
 */
class htmlForm extends htmlElement {

	/** form name */
	private string $name;
	/** submit target */
	private string $action;
	/** inner content */
	private htmlElement $content;

	/**
	 * Constructor.
	 *
	 * @param string $name name
	 * @param string $action target URL
	 */
	public function __construct(string $name, string $action, htmlElement $content) {
		$this->name = $name;
		$this->action = $action;
		$this->content = $content;
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		$return = [];
		echo '<form name="' . $this->name . '" action="' . $this->action . '" method="post" enctype="multipart/form-data">';
		if ($this->content != null) {
			$return = $this->content->generateHTML($module, $input, $values, $restricted, $scope);
		}
		echo '</form>';
		return $return;
	}

}

/**
 * Represents a (un)ordered list.
 */
class htmlList extends htmlElement {

	/** @var htmlElement[] $elements list elements */
	private array $elements;

	private bool $isOrdered;

	private ?string $id;

	/**
	 * Constructor.
	 *
	 * @param htmlElement[] $elements elements to render
	 * @param string|null $id HTML unique id
	 * @param bool|null $isOrdered is an ordered list
	 */
	public function __construct(array $elements, ?string $id = null, ?bool $isOrdered = false) {
		$this->elements = $elements;
		$this->id = $id;
		$this->isOrdered = $isOrdered;
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		$parentTag = $this->isOrdered ? 'ol' : 'ul';
		$idValue = empty($this->id) ? '' : ' id="' . $this->id . '"';
		$classesValue = '';
		if (!empty($this->cssClasses)) {
			$classesValue = ' class="' . implode(' ', $this->cssClasses) . '"';
		}
		echo '<' . $parentTag . $idValue . $classesValue . '>';
		$return = [];
		foreach ($this->elements as $element) {
			echo '<li>';
			$return = array_merge($return, $element->generateHTML($module, $input, $values, $restricted, $scope));
			echo '</li>';
		}
		echo '</' . $parentTag . '>';
		return $return;
	}

}

/**
 * Represents a label.
 */
class htmlLabel extends htmlElement {

	private string $for;

	private string $text;

	/** mark as required */
	private bool $markAsRequired = false;

	/**
	 * Constructor.
	 *
	 * @param string $for target element
	 * @param string $text label text
	 */
	public function __construct(string $for, string $text) {
		$this->for = htmlspecialchars($for);
		$this->text = htmlspecialchars($text);
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		$classesValue = '';
		if (!empty($this->cssClasses)) {
			$classesValue = ' class="' . implode(' ', $this->cssClasses) . '"';
		}
		echo '<label for="' . $this->for . '"' . $classesValue . '>';
		echo $this->text;
		echo '</label>';
		if ($this->markAsRequired) {
			echo htmlGetRequiredMarker();
		}
		return [];
	}

	/**
	 * Adds a marker that indicates a required field.
	 *
	 * @param bool $markAsRequired add marker
	 */
	public function setMarkAsRequired(bool $markAsRequired = true): void {
		$this->markAsRequired = $markAsRequired;
	}

}

/**
 * Represents a progress bar.
 */
class htmlProgressbar extends htmlElement {

	private ?string $id = null;
	private int $progress;

	/**
	 * Constructor.
	 *
	 * @param ?string $id HTML id
	 * @param int $progress initial progress
	 */
	public function __construct(?string $id = null, int $progress = 0) {
		if ($id !== null) {
			$this->id = htmlspecialchars($id);
		}
		$this->progress = $progress;
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		$extraClasses = '';
		if (!empty($this->cssClasses)) {
			$extraClasses = implode(' ', $this->cssClasses);
		}
		$idValue = '';
		if ($this->id !== null) {
			$idValue = ' id="' . $this->id . '"';
		}
		echo '<div ' . $idValue . ' class="lam-progressbar ' . $extraClasses . '">';
		echo '<div class="lam-progressbar-bar" style="width: ' . $this->progress . '%">';
		echo '</div>';
		echo '</div>';
		return [];
	}

}

/**
 * Tab container.
 */
class htmlTabContainer extends htmlElement {

	/**
	 * @var htmlTab[] $tabs tabs
	 */
	private array $tabs;

	private int $activeTabIndex = 1;

	/**
	 * Adds a new tab.
	 *
	 * @param htmlTab $tab tab
	 * @param bool $isActive is active tab
	 */
	public function addTab(htmlTab $tab, bool $isActive = false): void {
		$this->tabs[] = $tab;
		if ($isActive) {
			$this->activeTabIndex = count($this->tabs);
		}
	}

	/**
	 * {@inheritDoc}
	 */
	public function generateHTML($module, $input, $values, $restricted, $scope): array {
		echo '<div id="tabs" class="lam-tab-container">
			<ul class="lam-tab-navigation">';
		foreach ($this->tabs as $tabIndex => $tab) {
			$tabNumber = $tabIndex + 1;
			$tabActiveClass = ($this->activeTabIndex === $tabNumber) ? 'lam-tab-active' : '';
			$tabImage = ($tab->image !== null) ? '<img alt="" src="../../graphics/' . $tab->image . '">' : '';
			echo '<li data-tabid="' . $tabNumber . '" class="lam-tab ' . $tabActiveClass . '">
		            <a href="#tab' . $tabNumber . '" class="lam-tab-anchor">';
			echo $tabImage;
			$labelClass = ($tab->image !== null) ? 'hide-on-mobile' : '';
			echo '<span class="' . $labelClass . '"> ' . $tab->label . ' </span>
					</a>
				</li>';
		}
		echo '</ul>';
		$return = [];
		foreach ($this->tabs as $tabIndex => $tab) {
			$tabNumber = $tabIndex + 1;
			$tabActiveClass = ($this->activeTabIndex === $tabNumber) ? 'lam-tab-active' : '';
			echo '<div id="tab' . $tabNumber . '" class="lam-tab-content ' . $tabActiveClass . '" data-tabid="' . $tabNumber . '">';
			$data = parseHtml($module, $tab->content, $values, $restricted, $scope);
			$return = array_merge($return, $data);
			echo '</div>';
		}
		return $return;
	}

}

/**
 * Tab inside htmlTabContainer.
 */
class htmlTab {

	public string $label;

	public ?string $image;

	public htmlElement $content;

	/**
	 * Constructor.
	 *
	 * @param htmlElement $content tab content
	 * @param string $label tab label text
	 * @param string|null $image tab image
	 */
	public function __construct(htmlElement $content, string $label = '', ?string $image = null) {
		$this->content = $content;
		$this->image = $image;
		$this->label = htmlspecialchars($label);
	}

}
