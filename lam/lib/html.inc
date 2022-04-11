<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2010 - 2022  Roland Gruber

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
	$span = new htmlSpan(new htmlOutputText('*'), array('lam-required'));
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
	const ALIGN_TOP = 0;
	/** align to left */
	const ALIGN_LEFT = 1;
	/** align to right */
	const ALIGN_RIGHT = 2;
	/** align to bottom */
	const ALIGN_BOTTOM = 3;
	/** align to center */
	const ALIGN_CENTER = 4;

	/** validation rule to allow only numbers ([0-9]+) */
	const VALIDATE_NUMERIC = 'numeric';
	/** validation rule to allow positive/negative numbers ([-]?[0-9]+) */
	const VALIDATE_NUMERIC_WITH_NEGATIVE = 'numericWithNegative';

	/** alignment when inside a table */
	public $alignment = null;
	/** colspan if inside a table */
	public $colspan = null;
	/** rowspan if inside a table */
	public $rowspan = null;
	/** CSS classes */
	protected $cssClasses = array();
	/** table cell CSS classes */
	protected $tableCellCssClasses = array();
	/** data attributes */
	private $dataAttributes = array();

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	abstract function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope);

	/**
	 * Returns the HTML attributes for the alignment.
	 *
	 * @return String alignment HTML attributes (e.g. align="right" valign="top")
	 */
	public function getAlignmentString() {
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
	 * @return String colspan HTML attribute (e.g. colspan=3)
	 */
	public function getColspanString() {
		if ($this->colspan == null) {
			return '';
		}
		else return 'colspan="' . $this->colspan . '"';
	}

	/**
	 * Returns the HTML attribute for the rowspan.
	 *
	 * @return String rowspan HTML attribute (e.g. rowspan=3)
	 */
	public function getRowspanString() {
		if ($this->rowspan == null) {
			return '';
		}
		else return 'rowspan="' . $this->rowspan . '"';
	}

	/**
	 * Returns the CSS classes of this element.
	 *
	 * @return array $classes CSS class names
	 */
	public function getCSSClasses() {
		return $this->cssClasses;
	}

	/**
	 * Adds CSS classes to this element.
	 *
	 * @param array $classes CSS class names
	 */
	public function setCSSClasses($classes) {
		$this->cssClasses = $classes;
	}

	/**
	 * Adds CSS classes to the surrounding table cell for this element.
	 *
	 * @param array $classes CSS class names
	 */
	public function setTableCellCSSClasses($classes) {
		$this->tableCellCssClasses = $classes;
	}

	/**
	 * Returns the CSS classes of the surrounding table cell for this element.
	 *
	 * @return array CSS classes
	 */
	public function getTableCellCSSClasses() {
		return $this->tableCellCssClasses;
	}

	/**
	 * Adds a data attribute.
	 *
	 * @param string $key attribute name (without "data-")
	 * @param string $value attribute value
	 */
	public function addDataAttribute($key, $value) {
		$this->dataAttributes[$key] = $value;
	}

	/**
	 * Returns the data attributes as rendered string.
	 *
	 * @return string data attributes
	 */
	protected function getDataAttributesAsString() {
		$result = '';
		foreach ($this->dataAttributes as $key => $value) {
			$result .= ' data-' . htmlspecialchars($key) . '="' . htmlspecialchars($value) . '"';
		}
		return $result;
	}

}

/**
 * Structures elements using a table.
 *
 * @package metaHTML
 */
class htmlTable extends htmlElement {

	/** table footer */
	const FOOTER = "</table>\n";
	/** new line */
	const NEWLINE = "</tr><tr>\n";

	/** list of subelements */
	private $elements = array();
	/** specifies if currently a row is open */
	private $rowOpen = false;
	/** table width */
	private $width = null;
	/** HTML ID */
	private $id = null;

	/**
	 * Constructor
	 *
	 * @param String $width table width (e.g. 100%)
	 * @see htmlElement
	 */
	function __construct($width = null, $id = null) {
		$this->width = $width;
		$this->id = $id;
	}

	/**
	 * Adds an element to the table. The element may be a htmlElement object or a simple String.
	 *
	 * @param mixed $element htmlElement object or a simple String
	 * @param boolean $newLine adds a new line after the element (optional, default false)
	 * @param boolean $isTableHeadElement specifies if this is a head or body element (default: body)
	 */
	public function addElement($element, $newLine = false, $isTableHeadElement = false) {
		// add row element
		if ($element instanceof htmlTableRow) {
			// check if a row needs to be closed
			if ($this->rowOpen) {
				$this->elements[] = "</tr>\n";
				$this->rowOpen = false;
			}
			$this->elements[] = $element;
		}
		// add cell element
		elseif ($element instanceof htmlElement) {
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
			if (sizeof($element->getTableCellCSSClasses()) > 0) {
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
		else {
			StatusMessage('ERROR', 'Invalid element', print_r($element, true));
		}
	}

	/**
	 * Adds another line to the table.
	 */
	public function addNewLine() {
		if (!$this->rowOpen) {
			$this->elements[] = "<tr>\n";
		}
		else {
			$this->elements[] = htmlTable::NEWLINE;
		}
	}

	/**
	 * Adds an htmlSpacer with the given width.
	 *
	 * @param String $width width (e.g. 10px)
	 */
	public function addSpace($width) {
		$this->addElement(new htmlSpacer($width, null));
	}

	/**
	 * Adds an htmlSpacer with the given height and ends the row.
	 *
	 * @param String $height height (e.g. 10px)
	 */
	public function addVerticalSpace($height) {
		$this->addElement(new htmlSpacer(null, $height), true);
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	public function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		$return = array();
		$width = '';
		if ($this->width != null) {
			$width = ' width="' . htmlspecialchars($this->width) . '"';
		}
		$id = '';
		if (!empty($this->id)) {
			$id = ' id="' . $this->id . '"';
		}
		$classAttr = '';
		if (sizeof($this->cssClasses) > 0) {
			$classAttr = ' class="' . implode(' ', $this->cssClasses) . '"';
		}
		echo "<table" . $width . $id . $classAttr . ">\n";
		// print all contained elements
		for ($i = 0; $i < sizeof($this->elements); $i++) {
			// print htmlElement objects
			if ($this->elements[$i] instanceof htmlElement) {
				$fields = $this->elements[$i]->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
				$return = array_merge($return, $fields);
			}
			// print simple Strings
			else {
				if ($i != (sizeof($this->elements) - 1) || !($this->elements[$i] == htmlTable::NEWLINE) ) {
					echo $this->elements[$i];
				}
			}
		}
		if ($this->rowOpen) {
			echo "</tr>\n";
		}
		echo htmlTable::FOOTER;
		return $return;
	}

	/**
	 * Merges the content of another htmlTable object into this table.
	 *
	 * @param $table table to get elements
	 */
	public function mergeTableElements($table) {
		if (is_null($table) || !($table instanceof htmlTable)) {
			return;
		}
		// remove obsolete new lines at the end
		if ($table->elements[sizeof($table->elements) - 1] == htmlTable::NEWLINE) {
			unset($table->elements[sizeof($table->elements) - 1]);
		}
		// close last row of other table if needed
		if ($table->rowOpen) {
			$table->elements[] = "</tr>\n";
		}
		// close last own row if needed
		if ($this->rowOpen) {
			if ($this->elements[sizeof($this->elements) - 1] == htmlTable::NEWLINE) {
				unset($this->elements[sizeof($this->elements) - 1]);
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
 * A row inside a htmlTable.
 *
 * @see htmlTable
 * @package metaHTML
 */
class htmlTableRow extends htmlElement {

	/** table cells */
	private $cells;

	/**
	 * Constructor
	 *
	 * @param array $cells list of htmlElements
	 * @see htmlElement
	 */
	public function __construct($cells) {
		$this->cells = $cells;
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	public function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		$types = array();
		$classAttr = '';
		if (sizeof($this->cssClasses) > 0) {
			$classAttr = ' class="' . implode(' ', $this->cssClasses) . '"';
		}
		echo "<tr" . $classAttr . ">\n";
			for ($i = 0; $i < sizeof($this->cells); $i++) {
				// check if alignment option was given
				$align = $this->cells[$i]->getAlignmentString();
				$colspan = $this->cells[$i]->getColspanString();
				$rowspan = $this->cells[$i]->getRowspanString();
				echo "<td $align $colspan $rowspan>\n";
				$types = array_merge($types, $this->cells[$i]->generateHTML($module, $input, $values, $restricted, $tabindex, $scope));
				echo "</td>\n";
			}
		echo "</tr>";
		return $types;
	}

}

/**
 * A standard input field.
 *
 * @package metaHTML
 */
class htmlInputField extends htmlElement {

	/** unique field name */
	protected $fieldName;
	/** field value */
	protected $fieldValue = '';
	/** field size (default 30) */
	protected $fieldSize = 30;
	/** field max length (default 1000) */
	protected $fieldMaxLength = 1000;
	/** on keypress event */
	protected $onKeyPress = null;
	/** on keyupp event */
	protected $onKeyUp = null;
	/** oninput event */
	protected $onInput = null;
	/** password field */
	protected $isPassword = false;
	/** check password strength */
	protected $checkPasswordStrength = false;
	/** disables browser autofilling of password fields */
	protected $disableAutoFill = false;
	/** enabled or disabled */
	protected $isEnabled = true;
	/** indicates that the value should be saved in obfuscated form */
	protected $obfuscate = false;
	/** indicates that this field should not automatically be saved in the self service or server profile */
	protected $transient = false;
	/** required field */
	protected $required = false;
	/** validation rule */
	protected $validationRule = null;
	/** enable autocomplete */
	protected $autocomplete = false;
	/** autocompletion suggestions */
	protected $autocompleteValues = array();
	/** autocomplete start at this input length */
	protected $autocompleteMinLength = 1;
	/** show calendar */
	protected $showCalendar = false;
	/** show DN selection */
	protected $showDnSelection = false;
	/** calendar format */
	protected $calendarFormat = '';
	/** calendar with time */
	protected $calendarFormatWithTime = false;
	/** calendar with seconds */
	protected $calendarFormatWithSeconds = false;
	/** title attribute */
	protected $title = null;
	/** field ID that needs to have same value (e.g. password field) */
	protected $sameValueFieldID = null;
	/** marks the input field as auto trimmed (remove spaces at start/end) */
	protected $autoTrim = true;
	/** minimum value */
	protected $minValue = null;
	/** maximum value */
	protected $maxValue = null;
	/** id */
	protected $id = null;

	/**
	 * Constructor
	 *
	 * @param String $fieldName unique field name
	 * @param String $fieldValue value of input field (optional)
	 * @param String $fieldSize input field length (default 30)
	 */
	function __construct($fieldName, $fieldValue = null, $fieldSize = null) {
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
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		$this->cssClasses[] = 'ui-corner-all';
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
		$validators = array();
		if ($this->required) {
			$validators[] = 'required';
		}
		if ($this->validationRule != null) {
			$validators[] = 'custom[' . $this->validationRule . ']';
		}
		$min = '';
		if ($this->minValue !== null) {
			$min = ' min="' . $this->minValue . '"';
		}
		$max = '';
		if ($this->maxValue !== null) {
			$max = ' max="' . $this->maxValue . '"';
		}
		// print input field
		if ($this->autocomplete) {
			$this->cssClasses[] = 'lam-autocomplete';
		}
		if (sizeof($validators) > 0) {
			$class = ' class="validate[' . implode(',', $validators) . '] ' . implode(' ', $this->cssClasses) . '"';
		}
		else {
			$class = ' class="' . implode(' ', $this->cssClasses) . '"';
		}
		$name = ' name="' . $this->fieldName . '"';
		$idValue = ($this->id !== null) ? $this->id : $this->fieldName;
		$id = ' id="' . $idValue . '"';
		$value = '';
		if ($this->fieldValue != null) {
			$value = ' value="' . $this->fieldValue . '"';
		}
		$maxLength = '';
		if ($this->fieldMaxLength != null) {
			$maxLength = ' maxlength="' . $this->fieldMaxLength . '"';
		}
		$size = '';
		if ($this->fieldSize != null) {
			$size = ' size="' . $this->fieldSize . '"';
		}
		$fieldTabIndex = ' tabindex="' . $tabindex . '"';
		$tabindex++;
		$inputType = 'text';
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
		if ($this->autocomplete) {
			$autoCompleteData = ' data-autocomplete="' . base64_encode(json_encode($this->autocompleteValues)) . '"';
			$autoCompleteData .= ' data-autocomplete-minLength="' . $this->autocompleteMinLength . '"';
		}
		if ($this->showDnSelection) {
			echo '<span class="nowrap">';
		}
		echo '<input type="' . $inputType . '"' . $class . $name . $id . $value . $maxLength
			. $min . $max . $size . $fieldTabIndex . $onKeyPress . $onKeyUp . $onInput . $title . $disabled
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
			$sessionLanguage = $_SESSION['language'];
			$sessionLanguage = str_replace('.utf8', '', $sessionLanguage);
			if ($sessionLanguage === 'zh_TW') {
				$locale = 'zh_tw';
			}
			else {
				$sessionLanguageParts = explode('_', $sessionLanguage);
				$locale = $sessionLanguageParts[0];
			}
			$calendarWithTime = $this->calendarFormatWithTime ? "enableTime: true, " : "";
			$calendarWithSeconds = $this->calendarFormatWithSeconds ? "enableSeconds: true, " : "";
			echo '<script type="text/javascript">
				jQuery(function() {
					$("#' . $this->fieldName . '").flatpickr({
						dateFormat: "' . $this->calendarFormat . '",
						hourIncrement: 1,
						minuteIncrement: 1,
						time_24hr: true,
						' . $calendarWithTime . $calendarWithSeconds . '
						locale: "' . $locale . '"
					});
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
			$ajaxPath .=  $query;
			echo '<script type="text/javascript">
					checkPasswordStrength("' . $this->fieldName . '", "' . $ajaxPath . '", "' . getSecurityTokenName() . '", "' . getSecurityTokenValue() . '");
				</script>
			';
		}
		if ($this->transient) {
			return array();
		}
		if ($this->obfuscate) {
			return array($this->fieldName => 'text_obfuscated');
		}
		else {
			return array($this->fieldName => 'text');
		}
	}

	/**
	 * Sets the maximum field length.
	 *
	 * @param int $fieldMaxLength length
	 */
	public function setFieldMaxLength($fieldMaxLength) {
		$this->fieldMaxLength = $fieldMaxLength;
	}

	/**
	 * Sets the field size (default is 30).
	 *
	 * @param int $fieldSize size
	 */
	public function setFieldSize($fieldSize) {
		$this->fieldSize = $fieldSize;
	}

	/**
	 * Specifies if this is a password field.
	 *
	 * @param boolean $isPassword password field
	 * @param boolean $checkStrength check if matches password policy (default: false)
	 * @param boolean $disableAutoFill prevent autofilling by browser
	 */
	public function setIsPassword($isPassword, $checkStrength = false, $disableAutoFill = false) {
		$this->isPassword = $isPassword;
		$this->checkPasswordStrength = $checkStrength;
		$this->disableAutoFill = $disableAutoFill;
	}

	/**
	 * Specifies if this component is enabled and accepts user modification.
	 *
	 * @param boolean $isEnabled enabled if true
	 */
	public function setIsEnabled($isEnabled) {
		$this->isEnabled = $isEnabled;
	}

	/**
	 * Specifies if the value should be saved in obfuscated form (e.g. self service profile).
	 *
	 * @param boolean $obfuscate obfuscate value
	 */
	public function setObfuscate($obfuscate) {
		$this->obfuscate = $obfuscate;
	}

	/**
	 * Specifies that the value should not be automatically saved when used in self service or server profile (default: false).
	 *
	 * @param boolean $transient transient field
	 */
	public function setTransient($transient) {
		$this->transient = $transient;
	}

	/**
	 * Specifies if the input field is required.
	 *
	 * @param boolean $required required
	 */
	public function setRequired($required) {
		$this->required = $required;
	}

	/**
	 * Specifies the validation rule (e.g. htmlElement::VALIDATE_NUMERIC) for this field.
	 * This rule is checked on client side when the input field looses focus.
	 *
	 * @param boolean $rule rule name
	 */
	public function setValidationRule($rule) {
		$this->validationRule = $rule;
	}

	/**
	 * Sets the field as number field with minimum and maximum values.
	 *
	 * @param integer $minimum minimum
	 * @param integer $maximum maximum
	 */
	public function setMinimumAndMaximumNumber($minimum, $maximum = null) {
		$this->minValue = $minimum;
		$this->maxValue = $maximum;
	}

	/**
	 * Enables autocompletion for this input field.
	 *
	 * @param array $values list of values to suggest
	 * @param int $minLength autocompletion starts after this number of characters entered (default 1; 0 means immediate start)
	 */
	public function enableAutocompletion($values, $minLength = 1) {
		$this->autocomplete = true;
		$this->autocompleteValues = $values;
		$this->autocompleteMinLength = $minLength;
	}

	/**
	 * Sets the JavaScript for the onKeyPress event.
	 *
	 * @param String $onKeyPress JavaScript code
	 */
	public function setOnKeyPress($onKeyPress) {
		$this->onKeyPress = $onKeyPress;
	}

	/**
	 * Sets the JavaScript for the onKeyUp event.
	 *
	 * @param String $onKeyUp JavaScript code
	 */
	public function setOnKeyUp($onKeyUp) {
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
	 * @param String $format calendar format (e.g. Y-m-d)
	 * @param bool $withTime activate time selection
	 * @param bool $withSeconds show seconds
	 */
	public function showCalendar($format, $withTime = false, $withSeconds = false) {
		$this->showCalendar = true;
		$this->calendarFormatWithTime = $withTime;
		$this->calendarFormatWithSeconds = $withSeconds;
		$format = str_replace('yy', 'Y', $format);
		$format = str_replace('dd', 'd', $format);
		$format = str_replace('mm', 'm', $format);
		$this->calendarFormat = $format;
	}

	/**
	 * Shows a DN selection next to input field.
	 */
	public function showDnSelection() {
		$this->showDnSelection = true;
	}

	/**
	 * Sets the title for the input field.
	 *
	 * @param String $title title value
	 */
	public function setTitle($title) {
		$this->title = htmlspecialchars($title);
	}

	/**
	 * Specifies the ID of a second field that must have the same value as this field.
	 * This field is marked red if different or green if equal.
	 *
	 * @param String $sameValueFieldID ID of reference field
	 */
	public function setSameValueFieldID($sameValueFieldID) {
		$this->sameValueFieldID = $sameValueFieldID;
	}

	/**
	 * Turns this field into a live filter for a select box.
	 * Cannot be used together with setOnKeyUp().
	 *
	 * @param String $name select box name
	 */
	public function filterSelectBox($name) {
		$this->setOnKeyUp('window.lam.filterSelect.activate(\'' . $this->fieldName . '\', \'' . $name . '\', event);');
	}

	/**
	 * Returns if the field content should be auto-trimmed (remove spaces at start/end).
	 *
	 * @return boolean auto-trim input
	 */
	protected function isAutoTrim() {
		return $this->autoTrim && !$this->isPassword;
	}

	public function disableAutoTrim() {
		$this->autoTrim = false;
	}

	/**
	 * Sets the element ID.
	 *
	 * @param string $id id
	 */
	public function setId(string $id) {
		$this->id = $id;
	}

}

/**
 * An extended input field that combines label, input field and help.
 *
 * @package metaHTML
 */
class htmlTableExtendedInputField extends htmlInputField {

	/** Descriptive label */
	private $label;
	/** help ID */
	private $helpID;

	/**
	 * Constructor
	 *
	 * @param String $label descriptive label
	 * @param String $fieldName unique field name
	 * @param String $fieldValue value of input field (optional)
	 * @param String $helpID help ID (optional)
	 * @param boolean $required input required
	 */
	function __construct($label, $fieldName, $fieldValue = null, $helpID = null, $required = false) {
		parent::__construct($fieldName, $fieldValue);
		$this->label = htmlspecialchars($label);
		$this->helpID = $helpID;
		$this->required = $required;
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		// print label text
		echo '<div class="nowrap">';
		echo $this->label;
		if ($this->required) {
			echo htmlGetRequiredMarker();
		}
		echo '</div>';
		echo "\n</td>\n<td>\n";
		// print input field
		$return = parent::generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		// print help link
		if ($this->helpID != null) {
			echo "\n</td>\n<td>\n";
			$helpLink = new htmlHelpLink($this->helpID);
			$helpLink->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		}
		return $return;
	}

}

/**
 * Renders a help link.
 *
 * @package metaHTML
 */
class htmlHelpLink extends htmlElement {

	/** help ID */
	private $helpID;
	/** module name if it should be forced */
	private $module;
	/** account type if it should be forced */
	private $scope;

	/**
	 * Constructor
	 *
	 * @param String $helpID help ID
	 * @param String $module module name (optional, only if value from generateHTML() should be overwritten)
	 * @param String $scope account type (e.g. user) (optional, only if value from generateHTML() should be overwritten)
	 */
	function __construct($helpID, $module = null, $scope = null) {
		$this->helpID = $helpID;
		$this->module = $module;
		$this->scope = $scope;
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		// overwrite module and scope if needed
		if ($this->module != null) {
			$module = $this->module;
		}
		if ($this->scope != null) {
			$scope = $this->scope;
		}
		// print link
		$helpEntry = getHelp($module, $this->helpID, $scope);
		if (empty($helpEntry)) {
			return array();
		}
		printHelpLink($helpEntry, $this->helpID, $module, $scope, $this->cssClasses);
		return array();
	}

}

/**
 * Simple button.
 *
 * @package metaHTML
 */
class htmlButton extends htmlElement {

	/** button name */
	protected $name;
	/** button text or image */
	protected $value;
	/** image button or text button */
	protected $isImageButton;
	/** title */
	private $title = null;
	/** enabled or disabled */
	private $isEnabled = true;
	/** icon class (CSS) for buttons with icon + text */
	private $iconClass = null;
	/** onclick event */
	private $onClick = null;
	/** button type (default: "submit" if no onClick and "button" with onClick) */
	private $type = null;

	/**
	 * Constructor.
	 *
	 * @param String $name button name
	 * @param String $value button text or image (16x16px, relative to graphics folder)
	 * @param String $isImageButton image or text button (default text)
	 */
	function __construct($name, $value, $isImageButton = false) {
		$this->name = htmlspecialchars($name);
		$this->value = htmlspecialchars($value);
		$this->isImageButton = $isImageButton;
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		if ($restricted) {
			// no buttons in restricted mode
			logNewMessage(LOG_ERR, 'Meta HTML: Requested button in restricted mode.');
			return array();
		}
		$fieldTabIndex = ' tabindex="' . $tabindex . '"';
		$tabindex++;
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
		// text button
		elseif ($this->iconClass == null) {
			$classList[] = 'smallPadding';
		}
		// button with text and icon
		else {
			$classList[] = 'margin5';
		}
		if (sizeof($classList) > 0) {
			$class = ' class="' . implode(' ', $classList) . '"';
		}
		if ($this->title != null) {
			$title = ' title="' . $this->title . '"';
		}
		$disabled = '';
		if (!$this->isEnabled) {
			$disabled = ' disabled';
		}
		if ($this->type == null) {
			$type = ' type="submit"';
		}
		else {
			$type = ' type="' . $this->type . '"';
		}
		$onClick = '';
		if ($this->onClick != null) {
			if ($this->type == null) {
				$type = ' type="button"';
			}
			$onClick = ' onclick="' . $this->onClick . '"';
		}
		$id = ' id="btn_' . preg_replace('/[^a-zA-Z0-9_-]/', '', $this->name) . '"';
		if ($this->isImageButton) {
			echo '<input type="submit" ' . $id . ' value=" "' . $name . $onClick . $fieldTabIndex . $style . $class . $title . $disabled . $this->getDataAttributesAsString() . '>';
		}
		else {
			echo '<button' . $id . $name . $fieldTabIndex . $type . $onClick . $style . $class . $title . $disabled . $this->getDataAttributesAsString() . '>' . $this->value . '</button>';
			// text buttons get JQuery style
			if ($this->iconClass != null) {
				$icon = '{ icons: { primary: \'' . $this->iconClass . '\' } }';
				echo '<script type="text/javascript">';
				echo "jQuery('#btn_" . $this->name . "').button(" . $icon . ");";
				echo '</script>';
			}
		}
		return array($this->name => 'submit');
	}

	/**
	 * Sets the button title (tooltip).
	 *
	 * @param String $title title
	 */
	public function setTitle($title) {
		if ($title !== null) {
			$this->title = htmlspecialchars($title);
		}
	}

	/**
	 * Specifies if this component is enabled and accepts user modification.
	 *
	 * @param boolean $isEnabled enabled if true
	 */
	public function setIsEnabled($isEnabled) {
		$this->isEnabled = $isEnabled;
	}

	/**
	 * Sets an additional icon for a text button.
	 * The icon class is a CSS class that specifies the icon image (e.g. "deleteButton" in layout.css).
	 *
	 * @param String $iconClass icon class
	 */
	public function setIconClass($iconClass) {
		$this->iconClass = htmlspecialchars($iconClass);
	}

	/**
	 * Sets the onclick event code.
	 * This makes this button a simple button that does not submit a form.
	 *
	 * @param String $onClick JS code
	 */
	public function setOnClick($onClick) {
		$this->onClick = $onClick;
	}

	/**
	 * Allows to override the default button type ("submit" if no onClick and "button" with onClick).
	 */
	public function setType($type) {
		$this->type = $type;
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
	 * @param String $targetModule module name which renders next page
	 * @param String $targetPage name of next page
	 * @param String $identifier identifier for button
	 * @param String $value button text or image (16x16px, relative to graphics folder)
	 * @param String $isImageButton image or text button (default text)
	 * @param String $title title to show
	 */
	function __construct($targetModule, $targetPage, $identifier, $value, $isImageButton = false, $title = null) {
		$this->name = htmlspecialchars('form_subpage_' . $targetModule . '_' . $targetPage . '_' . $identifier);
		$this->value = $value;
		$this->isImageButton = $isImageButton;
		if ($title != null) {
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
	private $name;
	/** size */
	private $size;
	/** allows multi-selection */
	private $multiSelect = false;
	/** elements */
	private $elements;
	/** selected elements */
	private $selectedElements = array();
	/** descriptive elements */
	private $hasDescriptiveElements = false;
	/** contains optgroups */
	private $containsOptgroups = false;
	/** sorting enabled */
	private $sortElements = true;
	/** right to left text direction */
	private $rightToLeftTextDirection = false;
	/** enabled or disabled */
	private $isEnabled = true;
	/** width of input element */
	private $width = '';
	/** transform select boxes with one element to text */
	private $transformSingleSelect = true;
	/** onchange event */
	private $onchangeEvent = null;
	/** indicates that this field should not automatically be saved in the self service or server profile */
	private $transient = false;
	/** list of enclosing table rows to hide when checked */
	protected $tableRowsToHide = array();
	/** list of enclosing table rows to show when checked */
	protected $tableRowsToShow = array();
	/** dynamic scrolling */
	private $dynamicScrolling = false;
	/** CSS classes for values */
	private $optionCssClasses = array();

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
	 * @param String $name element name
	 * @param array $elements list of elements array(label => value) or array(value1, value2) or array('optgroup' => array(...))
	 * @param array $selectedElements list of selected elements (optional, default none)
	 * @param int $size size (optional, default = 1)
	 */
	function __construct($name, $elements, $selectedElements = array(), $size = 1) {
		$this->name = htmlspecialchars($name);
		$this->elements = $elements;
		if ($selectedElements != null) {
			$this->selectedElements = $selectedElements;
		}
		$this->size = htmlspecialchars($size);
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		$this->cssClasses[] = 'ui-corner-all';
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
		$class = '';
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
		if ($this->width != '') {
			$style = ' style="width: ' . $this->width . '"';
		}
		$onchange = '';
		if ($this->onchangeEvent != null) {
			$onchange = $this->onchangeEvent;
		}
		if (($this->tableRowsToHide != null) || ($this->tableRowsToShow != null)) {
			$this->printCodeForShowHideTableRows($onchange);
		}
		if ($onchange != '') {
			$onchange = ' onchange="' . $onchange . '"';
		}
		// hide select boxes that contain less than 2 elements
		if ((sizeof($this->elements) < 2) && !$this->multiSelect && !$this->containsOptgroups && $this->transformSingleSelect) {
			echo '<div class="hidden">';
		}
		// print select box
		echo '<select' . $this->getDataAttributesAsString() . $class . $style
				. $name . $size . $multi . $disabled . $onchange
				. ' tabindex="' . $tabindex . "\">\n";
		$tabindex++;
		if ($this->containsOptgroups) {
			foreach ($this->elements as $label => $elements) {
				if (sizeof($elements) > 0) {
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
		if ((sizeof($this->elements) < 2) && !$this->multiSelect && !$this->containsOptgroups && $this->transformSingleSelect) {
			echo '</div>';
			if (sizeof($this->elements) == 1) {
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
			return array();
		}
		if ($this->multiSelect) {
			return array($this->name => 'multiselect');
		}
		else {
			return array($this->name => 'select');
		}
	}

	/**
	 * Prints the HTML code of the option tags.
	 *
	 * @param array $elements list of options
	 */
	private function printOptionsHTML($elements) {
		if ($this->dynamicScrolling) {
			echo "<option value=\"#\">#</option>\n";
			return;
		}
		// sorting
		if ($this->sortElements) {
			if ($this->hasDescriptiveElements) {
				$labels = array_keys($elements);
				natcasesort($labels);
				$newElements = array();
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
			if ($this->hasDescriptiveElements) {
				if (in_array($value, $this->selectedElements) || (empty($this->selectedElements) && empty($value))) {
					$selected = ' selected';
				}
				echo "<option value=\"" . htmlspecialchars($value) . "\"$selected $optionClass>" . htmlspecialchars($key) . "</option>\n";
			}
			else {
				if (in_array($value, $this->selectedElements) || (empty($this->selectedElements) && empty($value))) {
					$selected = ' selected';
				}
				echo "<option$selected $optionClass>" . htmlspecialchars($value) . "</option>\n";
			}
		}
	}

	/**
	 * Specifies if the elements are just a simple list or an assoziative array (default: simple list).
	 *
	 * @param boolean $hasDescriptiveElements activates descriptive elements
	 */
	public function setHasDescriptiveElements($hasDescriptiveElements) {
		$this->hasDescriptiveElements = $hasDescriptiveElements;
	}

	/**
	 * Specifies if the elements are divided into optgroups.
	 * In this case the provided options are an array where the key is the optgroup label and the value is an array containing the options for the optgroup.
	 *
	 * @param boolean $containsOptgroups activates optgroups
	 */
	public function setContainsOptgroups($containsOptgroups) {
		$this->containsOptgroups = $containsOptgroups;
	}

	/**
	 * Specifies if multi-selection is enabled (default: disabled).
	 *
	 * @param boolean $multiSelect allows multi-selection
	 */
	public function setMultiSelect($multiSelect) {
		$this->multiSelect = $multiSelect;
	}

	/**
	 * Specifies if the elements should be sorted (default: sort).
	 *
	 * @param boolean $sortElements sort elements
	 */
	public function setSortElements($sortElements) {
		$this->sortElements = $sortElements;
	}

	/**
	 * Specifies if the text direction should be set to right to left.
	 *
	 * @param boolean $rightToLeftTextDirection if true use right to left direction
	 */
	public function setRightToLeftTextDirection($rightToLeftTextDirection) {
		$this->rightToLeftTextDirection = $rightToLeftTextDirection;
	}

	/**
	 * Specifies if this component is enabled and accepts user modification.
	 *
	 * @param boolean $isEnabled enabled if true
	 */
	public function setIsEnabled($isEnabled) {
		$this->isEnabled = $isEnabled;
	}

	/**
	 * Specifies the width of this selection box.
	 *
	 * @param String $width width (e.g. 20em)
	 */
	public function setWidth($width) {
		$this->width = htmlspecialchars($width);
	}

	/**
	 * Specifies if select boxes that contain only a single element should be transformed to a simple text field.
	 *
	 * @param boolean $transformSingleSelect transform single options to text
	 */
	public function setTransformSingleSelect($transformSingleSelect) {
		$this->transformSingleSelect = $transformSingleSelect;
	}

	/**
	 * Sets the JavaScript code for the onchange event.
	 *
	 * @param String $onchangeEvent onchange event code (e.g. myfunction();)
	 */
	public function setOnchangeEvent($onchangeEvent) {
		$this->onchangeEvent = htmlspecialchars($onchangeEvent);
	}

	/**
	 * Specifies that the value should not be automatically saved when used in self service or server profile (default: false).
	 *
	 * @param boolean $transient transient field
	 */
	public function setTransient($transient) {
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
	 * @param array $tableRowsToHide array of select value => array of IDs of child elements to hide
	 */
	public function setTableRowsToHide($tableRowsToHide) {
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
	 * @param array $tableRowsToShow array of select value => array of IDs of child elements to show
	 */
	public function setTableRowsToShow($tableRowsToShow) {
		$this->tableRowsToShow = $tableRowsToShow;
	}

	/**
	 * Creates the JavaScript code to hide/show table rows based on the select value.
	 *
	 * @param String $onChange onChange code
	 */
	private function printCodeForShowHideTableRows(&$onChange) {
		if ((sizeof($this->tableRowsToHide) == 0) && (sizeof($this->tableRowsToShow) == 0)) {
			return;
		}
		$values = array();
		if (!empty($this->tableRowsToHide)) {
			$values = array_merge($values, array_keys($this->tableRowsToHide));
		}
		if (!empty($this->tableRowsToShow)) {
			$values = array_merge($values, array_keys($this->tableRowsToShow));
		}
		$selector = $this->getShowHideSelector();
		// build Java script to show/hide depending fields
		foreach ($values as $val) {
			// build onChange listener
			$onChange .= 'if (jQuery(\'#' . $this->name . '\').val() == \'' . $val . '\') {';
			if (isset($this->tableRowsToShow[$val])) {
				for ($i = 0; $i < sizeof($this->tableRowsToShow[$val]); $i++) {
					$onChange .= 'jQuery(\'#' . $this->tableRowsToShow[$val][$i] . '\').closest(\'' . $selector . '\').removeClass(\'hidden\');';
				}
			}
			if (isset($this->tableRowsToHide[$val])) {
				for ($i = 0; $i < sizeof($this->tableRowsToHide[$val]); $i++) {
					$onChange .= 'jQuery(\'#' . $this->tableRowsToHide[$val][$i] . '\').closest(\'' . $selector . '\').addClass(\'hidden\');';
				}
			}
			$onChange .= '};';
		}
		// build script to set initial state
		$script = '<script type="text/javascript">jQuery(document).ready(function() {' . "\n";
		if (isset($this->tableRowsToShow[$this->selectedElements[0]])) {
			for ($i = 0; $i < sizeof($this->tableRowsToShow[$this->selectedElements[0]]); $i++) {
				$classType = 'removeClass';
				$script .= 'jQuery(\'#' . $this->tableRowsToShow[$this->selectedElements[0]][$i] . '\').closest(\'' . $selector . '\').' . $classType . '(\'hidden\');' . "\n";
			}
		}
		if (isset($this->tableRowsToHide[$this->selectedElements[0]])) {
			for ($i = 0; $i < sizeof($this->tableRowsToHide[$this->selectedElements[0]]); $i++) {
				$classType = 'addClass';
				$script .= 'jQuery(\'#' . $this->tableRowsToHide[$this->selectedElements[0]][$i] . '\').closest(\'' . $selector . '\').' . $classType . '(\'hidden\');' . "\n";
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
	protected function getShowHideSelector() {
		return 'tr';
	}

	/**
	 * Enable dynamic scrolling. This limits the number of select options to 10000 by dynamically adding/removing options.
	 * This will not be enabled when optgroups are used or the option size is less than 10000.
	 */
	public function enableDynamicScrolling() {
		// not possible for optgroups and smaller option lists
		if ((sizeof($this->elements) < 10000) || $this->containsOptgroups) {
			return;
		}
		$this->dynamicScrolling = true;
		$elementData = array();
		foreach ($this->elements as $key => $value) {
			$elementData[] = array('label' => $key, 'value' => $value);
		}
		$this->addDataAttribute('dynamic-options', json_encode($elementData));
		$this->cssClasses[] = 'lam-dynamicOptions';
	}

	/**
	 * Sets CSS classes for option values.
	 *
	 * @param array $optionCssClasses array('option value' => 'CSS class(es)')
	 */
	public function setOptionCssClasses(array $optionCssClasses): void {
		$this->optionCssClasses = $optionCssClasses;
	}

}

/**
 * Select with label and help link.
 *
 * @package metaHTML
 */
class htmlTableExtendedSelect extends htmlSelect {

	/** descriptive label */
	private $label;
	/** help ID */
	private $helpID;

	/**
	 * Constructor.
	 *
	 * @param String $name element name
	 * @param array $elements list of elememts
	 * @param array $selectedElements list of selected elements
	 * @param String $label descriptive label
	 * @param String $helpID help ID (optional, default none)
	 * @param int $size size (optional, default = 1)
	 */
	function __construct($name, $elements, $selectedElements, $label, $helpID = null, $size = 1) {
		parent::__construct($name, $elements, $selectedElements, $size);
		$this->label = htmlspecialchars($label);
		$this->helpID = $helpID;
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		echo '<div class="nowrap">';
		echo $this->label;
		echo '</div>';
		echo "\n</td>\n<td>\n";
		$return = parent::generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		// print help link
		if ($this->helpID != null) {
			echo "\n</td>\n<td>\n";
			$helpLink = new htmlHelpLink($this->helpID);
			$helpLink->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		}
		return $return;
	}

}

/**
 * Represents a radio selection.
 *
 * @package metaHTML
 */
class htmlRadio extends htmlElement {

	/** name of select field */
	private $name;
	/** elements */
	private $elements;
	/** selected element */
	private $selectedElement = null;
	/** enabled or disabled */
	private $isEnabled = true;
	/** on change code */
	private $onchangeEvent = null;
	/** list of enclosing table rows to hide when checked */
	protected $tableRowsToHide = array();
	/** list of enclosing table rows to show when checked */
	protected $tableRowsToShow = array();

	/**
	 * Constructor.
	 *
	 * <br>Examples:
	 * <br>
	 * <br>$radio = new htmlRadio('myName', array('label1' => 'value1', 'label2' => 'value2'), array('value1'));
	 *
	 * @param String $name element name
	 * @param array $elements list of elements array(label => value)
	 * @param String $selectedElement value of selected element (optional, default none)
	 */
	function __construct($name, $elements, $selectedElement = null) {
		$this->name = htmlspecialchars($name);
		$this->elements = $elements;
		if ($selectedElement != null) {
			$this->selectedElement = $selectedElement;
		}
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
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
			$onClick = 'onClick="
				jQuery(\'input[name=' . $this->name . ']\').prop(\'checked\', false);
				jQuery(\'#' . $this->name . $counter . '\').prop(\'checked\', true);
				jQuery(\'#' . $this->name . $counter . '\').trigger(\'change\');
				"';
			if ($this->isEnabled === false) {
				$onClick = '';
			}
			echo '<div class="nowrap" ' . $onClick . '>';
			$selected = '';
			if ($value == $this->selectedElement) {
				$selected = ' checked';
			}
			echo '<input type="radio" id="' . $this->name . $counter . '"' . $name . $disabled . $selected . $onchange . ' value="' . $value . '" tabindex="' . $tabindex . '"> ' . $label;
			echo '</div>';
			$tabindex++;
			$counter++;
		}
		return array($this->name => 'select');
	}

	/**
	 * Specifies if this component is enabled and accepts user modification.
	 *
	 * @param boolean $isEnabled enabled if true
	 */
	public function setIsEnabled($isEnabled) {
		$this->isEnabled = $isEnabled;
	}

	/**
	 * Sets the JavaScript code for the onchange event.
	 *
	 * @param String $onchangeEvent onchange event code (e.g. myfunction();)
	 */
	public function setOnchangeEvent($onchangeEvent) {
		$this->onchangeEvent = htmlspecialchars($onchangeEvent);
	}

	/**
	 * Returns the selector to use to find the show/hide elements.
	 *
	 * @return string selector
	 */
	protected function getShowHideSelector() {
		return 'tr';
	}

	/**
	 * Creates the JavaScript code to hide/show table rows based on the select value.
	 *
	 * @param int $counter index
	 * @return String onChange code
	 */
	private function getOnchangeCodeForShowHideTableRows($counter) {
		$onChange = '';
		if ((sizeof($this->tableRowsToHide) == 0) && (sizeof($this->tableRowsToShow) == 0)) {
			return;
		}
		$values = array();
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
			$onChange .= 'if (jQuery(\'#' . $this->name . $counter . '\').val() == \'' . $val . '\') {';
			if (isset($this->tableRowsToShow[$val])) {
				for ($i = 0; $i < sizeof($this->tableRowsToShow[$val]); $i++) {
					$onChange .= 'jQuery(\'#' . $this->tableRowsToShow[$val][$i] . '\').closest(\'' . $selector . '\').removeClass(\'hidden\');';
				}
			}
			if (isset($this->tableRowsToHide[$val])) {
				for ($i = 0; $i < sizeof($this->tableRowsToHide[$val]); $i++) {
					$onChange .= 'jQuery(\'#' . $this->tableRowsToHide[$val][$i] . '\').closest(\'' . $selector . '\').addClass(\'hidden\');';
				}
			}
			$onChange .= '};';
		}
		return $onChange;
	}

	private function printInitialState() {
		$selector = $this->getShowHideSelector();
		// build script to set initial state
		$script = '<script type="text/javascript">jQuery(document).ready(function() {' . "\n";
		if (isset($this->tableRowsToShow[$this->selectedElement])) {
			for ($i = 0; $i < sizeof($this->tableRowsToShow[$this->selectedElement]); $i++) {
				$classType = 'removeClass';
				$script .= 'jQuery(\'#' . $this->tableRowsToShow[$this->selectedElement][$i] . '\').closest(\'' . $selector . '\').' . $classType . '(\'hidden\');' . "\n";
			}
		}
		if (isset($this->tableRowsToHide[$this->selectedElement])) {
			for ($i = 0; $i < sizeof($this->tableRowsToHide[$this->selectedElement]); $i++) {
				$classType = 'addClass';
				$script .= 'jQuery(\'#' . $this->tableRowsToHide[$this->selectedElement][$i] . '\').closest(\'' . $selector . '\').' . $classType . '(\'hidden\');' . "\n";
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
	 * @param array $tableRowsToHide array of select value => array of IDs of child elements to hide
	 */
	public function setTableRowsToHide($tableRowsToHide) {
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
	 * @param array $tableRowsToShow array of select value => array of IDs of child elements to show
	 */
	public function setTableRowsToShow($tableRowsToShow) {
		$this->tableRowsToShow = $tableRowsToShow;
	}

}

/**
 * Radio list with descriptive label and help link.
 *
 * @package metaHTML
 */
class htmlTableExtendedRadio extends htmlRadio {

	/** descriptive label */
	private $label;
	/** help ID */
	private $helpID;

	/**
	 * Constructor.
	 *
	 * @param String $label descriptive label
	 * @param String $name element name
	 * @param array $elements list of elements array(label => value)
	 * @param String $selectedElement value of selected element (optional, default none)
	 * @param String $helpID help ID
	 */
	function __construct($label, $name, $elements, $selectedElement = null, $helpID = null) {
		parent::__construct($name, $elements, $selectedElement);
		$this->label = htmlspecialchars($label);
		$this->helpID = $helpID;
		$this->alignment = htmlElement::ALIGN_TOP;
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		if ($this->label != null) {
			echo '<div class="nowrap">';
			echo $this->label;
			echo '</div>';
			echo "\n</td>\n<td>\n";
		}
		$return = parent::generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		// print help link
		if ($this->helpID != null) {
			echo "\n</td>\n<td valign=\"top\">\n";
			$helpLink = new htmlHelpLink($this->helpID);
			$helpLink->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		}
		return $return;
	}

}

/**
 * Prints the text and escapes contained HTML code by default.
 *
 * @package metaHTML
 */
class htmlOutputText extends htmlElement {

	/** the text to print */
	private $string;
	/** specifies if HTML code should be escaped */
	private $escapeHTML;
	/** bold text */
	private $isBold = false;
	/** mark as required */
	private $markAsRequired = false;
	/** no wrap */
	private $noWrap = false;
	/** preformatted */
	private $isPreformatted = false;
	/** title */
	private $title = null;

	/**
	 * Constructor.
	 *
	 * @param String $string output text
	 * @param boolean $escapeHTML escape HTML code (default yes)
	 * @param boolean $markAsRequired mark text like a required field
	 */
	function __construct($string, $escapeHTML = true, $markAsRequired = false) {
		$this->string = $string;
		$this->escapeHTML = $escapeHTML;
		$this->markAsRequired = $markAsRequired;
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
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
		return array();
	}

	/**
	 * Specifies if the whole text should be printed in bold.
	 *
	 * @param boolean $isBold bold text
	 */
	public function setIsBold($isBold) {
		$this->isBold = $isBold;
	}

	/**
	 * Adds a marker that indicates a required field.
	 *
	 * @param boolean $markAsRequired add marker
	 */
	public function setMarkAsRequired($markAsRequired) {
		$this->markAsRequired = $markAsRequired;
	}

	/**
	 * Specifies if word wrap is allowed for this text.
	 *
	 * @param boolean $noWrap no wrapping if set to true (default false)
	 */
	public function setNoWrap($noWrap) {
		$this->noWrap = $noWrap;
	}

	/**
	 * Sets if the text is preformatted.
	 *
	 * @param boolean $preformatted is preformatted (default true)
	 */
	public function setPreformatted($preformatted = true) {
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
	protected $name;
	/** value */
	protected $checked;
	/** enabled or disabled */
	protected $isEnabled = true;
	/** list of enclosing table rows to hide when checked */
	protected $tableRowsToHide = array();
	/** list of enclosing table rows to show when checked */
	protected $tableRowsToShow = array();
	/** indicates that this field should not automatically be saved in the self service or server profile */
	private $transient = false;
	/** list of input elements to enable when checked */
	protected $elementsToEnable = array();
	/** list of input elements to disable when checked */
	protected $elementsToDisable = array();
	/** onclick event code */
	private $onClick;


	/**
	 * Constructor.
	 *
	 * @param String $name unique name
	 * @param boolean $checked checked
	 */
	function __construct($name, $checked) {
		$this->name = htmlspecialchars($name);
		$this->checked = $checked;
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		if (isset($values[$this->name])) {
			if ($values[$this->name][0] == 'true') {
				$this->checked = true;
			}
			else {
				$this->checked = false;
			}
		}
		$checked = '';
		if ($this->checked) {
			$checked = ' checked';
		}
		$tabindexValue = ' tabindex="' . $tabindex . '"';
		$tabindex++;
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
		if ((sizeof($this->tableRowsToShow) > 0) || (sizeof($this->tableRowsToHide) > 0)) {
			// build onChange listener
			$onChange .= 'if (jQuery(\'#' . $this->name . ':checked\').val() !== undefined) {';
			for ($i = 0; $i < sizeof($this->tableRowsToShow); $i++) {
				$onChange .= 'jQuery(\'#' . $this->tableRowsToShow[$i] . '\').closest(\'' . $selector . '\').removeClass(\'hidden\');';
			}
			for ($i = 0; $i < sizeof($this->tableRowsToHide); $i++) {
				$onChange .= 'jQuery(\'#' . $this->tableRowsToHide[$i] . '\').closest(\'' . $selector . '\').addClass(\'hidden\');';
			}
			$onChange .= '}';
			$onChange .= 'else {';
			for ($i = 0; $i < sizeof($this->tableRowsToShow); $i++) {
				$onChange .= 'jQuery(\'#' . $this->tableRowsToShow[$i] . '\').closest(\'' . $selector . '\').addClass(\'hidden\');';
			}
			for ($i = 0; $i < sizeof($this->tableRowsToHide); $i++) {
				$onChange .= 'jQuery(\'#' . $this->tableRowsToHide[$i] . '\').closest(\'' . $selector . '\').removeClass(\'hidden\');';
			}
			$onChange .= '};';
			// build script to set initial state
			$script = '<script type="text/javascript">jQuery(document).ready(function() {';
			for ($i = 0; $i < sizeof($this->tableRowsToShow); $i++) {
				$classType = 'addClass';
				if ($this->checked) {
					$classType = 'removeClass';
				}
				$script .= 'jQuery(\'#' . $this->tableRowsToShow[$i] . '\').closest(\'' . $selector . '\').' . $classType . '(\'hidden\');';
			}
			for ($i = 0; $i < sizeof($this->tableRowsToHide); $i++) {
				$classType = 'removeClass';
				if ($this->checked) {
					$classType = 'addClass';
				}
				$script .= 'jQuery(\'#' . $this->tableRowsToHide[$i] . '\').closest(\'' . $selector . '\').' . $classType . '(\'hidden\');';
			}
			$script .= '});</script>';
		}
		// build Java script to enable/disable elements
		if ((sizeof($this->elementsToEnable) > 0) || (sizeof($this->elementsToDisable) > 0)) {
			// build onChange listener
			$onChange .= 'if (jQuery(\'#' . $this->name . ':checked\').val() !== undefined) {';
			for ($i = 0; $i < sizeof($this->elementsToEnable); $i++) {
				$onChange .= 'jQuery(\'#' . $this->elementsToEnable[$i] . '\').prop(\'disabled\', false);';
			}
			for ($i = 0; $i < sizeof($this->elementsToDisable); $i++) {
				$onChange .= 'jQuery(\'#' . $this->elementsToDisable[$i] . '\').prop(\'disabled\', true);';
			}
			$onChange .= '}';
			$onChange .= 'else {';
			for ($i = 0; $i < sizeof($this->elementsToEnable); $i++) {
				$onChange .= 'jQuery(\'#' . $this->elementsToEnable[$i] . '\').prop(\'disabled\', true);';
			}
			for ($i = 0; $i < sizeof($this->elementsToDisable); $i++) {
				$onChange .= 'jQuery(\'#' . $this->elementsToDisable[$i] . '\').prop(\'disabled\', false);';
			}
			$onChange .= '};';
			// build script to set initial state
			$script = '<script type="text/javascript">jQuery(document).ready(function() {';
			for ($i = 0; $i < sizeof($this->elementsToEnable); $i++) {
				$classType = 'true';
				if ($this->checked) {
					$classType = 'false';
				}
				$script .= 'jQuery(\'#' . $this->elementsToEnable[$i] . '\').prop(\'disabled\', ' . $classType . ');';
			}
			for ($i = 0; $i < sizeof($this->elementsToDisable); $i++) {
				$classType = 'false';
				if ($this->checked) {
					$classType = 'true';
				}
				$script .= 'jQuery(\'#' . $this->elementsToDisable[$i] . '\').prop(\'disabled\', ' . $classType . ');';
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
		echo '<input type="checkbox" id="' . $this->name . '" name="' . $this->name . '"' . $classes . $tabindexValue . $onChange . $onClick . $checked . $disabled . '>';
		echo $script;
		if ($this->transient) {
			return array();
		}
		return array($this->name => 'checkbox');
	}

	/**
	 * Specifies if this component is enabled and accepts user modification.
	 *
	 * @param boolean $isEnabled enabled if true
	 */
	public function setIsEnabled($isEnabled) {
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
	 * @param array $tableRowsToHide IDs of child elements to hide
	 */
	public function setTableRowsToHide($tableRowsToHide) {
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
	 * @param array $tableRowsToShow IDs of child elements to show
	 */
	public function setTableRowsToShow($tableRowsToShow) {
		$this->tableRowsToShow = $tableRowsToShow;
	}

	/**
	 * Specifies that the value should not be automatically saved when used in self service or server profile (default: false).
	 *
	 * @param boolean $transient transient field
	 */
	public function setTransient($transient) {
		$this->transient = $transient;
	}

	/**
	 * This will disable the given input elements when the checkbox is checked.
	 * The given IDs can be of any input element (e.g. select, checkbox, ...).
	 *
	 * @param array $elements IDs of elements to disable
	 */
	public function setElementsToDisable($elements) {
		$this->elementsToDisable = $elements;
	}

	/**
	 * This will enable the given input elements when the checkbox is checked.
	 * The given IDs can be of any input element (e.g. select, checkbox, ...).
	 *
	 * @param array $elements IDs of elements to enable
	 */
	public function setElementsToEnable($elements) {
		$this->elementsToEnable = $elements;
	}

	/**
	 * Returns the CSS selector to use to find show/hide elements.
	 */
	protected function getShowHideSelector() {
		return 'tr';
	}

	/**
	 * Sets the onclick code.
	 *
	 * @param string $code JS code
	 */
	public function setOnClick($code) {
		$this->onClick = $code;
	}

}

/**
 * Checkbox with descriptive label and help link.
 *
 * @package metaHTML
 */
class htmlTableExtendedInputCheckbox extends htmlInputCheckbox {

	/** descriptive label */
	private $label;
	/** help ID */
	private $helpID;
	/** specifies if label is printed before the checkbox */
	private $labelFirst;

	/**
	 * Constructor.
	 *
	 * @param String $name unique name
	 * @param boolean $checked checked
	 * @param String $label descriptive label
	 * @param String $helpID help ID
	 * @param boolean $labelFirst specifies if the label is at the beginning or at the end (optional, default beginning)
	 */
	function __construct($name, $checked, $label, $helpID = null, $labelFirst = true) {
		parent::__construct($name, $checked);
		$this->label = htmlspecialchars($label);
		$this->helpID = $helpID;
		$this->labelFirst = $labelFirst;
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		$onClick = 'onClick="jQuery(\'#' . $this->name . '\').prop(\'checked\',!jQuery(\'#' . $this->name . '\').prop(\'checked\')); jQuery(\'#' . $this->name . '\').change();"';
		if ($this->labelFirst) {
			echo '<div class="nowrap" ' . $onClick . '>';
			echo $this->label;
			echo '</div>';
			echo "\n</td>\n<td>\n";
			$return = parent::generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		}
		else {
			$return = parent::generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
			echo "\n</td>\n<td>\n";
			echo '<div class="nowrap" ' . $onClick . '>';
			echo $this->label;
			echo '</div>';
		}
		// print help link
		if ($this->helpID != null) {
			echo "\n</td>\n<td>\n";
			$helpLink = new htmlHelpLink($this->helpID);
			$helpLink->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		}
		return $return;
	}

}

/**
 * Checkbox with descriptive label and help link.
 *
 * @package metaHTML
 */
class htmlLabeledInputCheckbox extends htmlInputCheckbox {

	/** descriptive label */
	private $label;
	/** help ID */
	private $helpID;
	/** specifies if label is printed before the checkbox */
	private $labelFirst;

	/**
	 * Constructor.
	 *
	 * @param String $name unique name
	 * @param boolean $checked checked
	 * @param String $label descriptive label
	 * @param String $helpID help ID
	 * @param boolean $labelFirst specifies if the label is at the beginning or at the end (optional, default beginning)
	 */
	function __construct($name, $checked, $label, $helpID = null, $labelFirst = true) {
		parent::__construct($name, $checked);
		$this->label = htmlspecialchars($label);
		$this->helpID = $helpID;
		$this->labelFirst = $labelFirst;
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		$onClick = 'onClick="jQuery(\'#' . $this->name . '\').prop(\'checked\',!jQuery(\'#' . $this->name . '\').prop(\'checked\')); jQuery(\'#' . $this->name . '\').change();"';
		if ($this->labelFirst) {
			echo '<span class="nowrap" ' . $onClick . '>';
			echo $this->label;
			echo '</span>';
			$return = parent::generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		}
		else {
			$return = parent::generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
			echo '<span class="nowrap" ' . $onClick . '>';
			echo $this->label;
			echo '</span>';
		}
		// print help link
		if ($this->helpID != null) {
			$helpLink = new htmlHelpLink($this->helpID);
			$helpLink->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		}
		return $return;
	}

}

/**
 * Prints the HTML code for a file upload field.
 *
 * @package metaHTML
 */
class htmlInputFileUpload extends htmlElement {

	/** unique name of input element */
	private $name;
	/** enabled or disabled */
	private $isEnabled = true;

	/**
	 * Constructor.
	 *
	 * @param String $name unique name
	 */
	function __construct($name) {
		$this->name = htmlspecialchars($name);
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		$tabindexValue = ' tabindex="' . $tabindex . '"';
		$tabindex++;
		$disabled = '';
		if (!$this->isEnabled) {
			$disabled = ' disabled';
		}
		$classValue = '';
		if (!empty($this->cssClasses)) {
			$classValue = ' class="' . implode(' ', $this->cssClasses) . '"';
		}
		echo '<input type="file" id="' . $this->name . '" name="' . $this->name . '"' . $tabindexValue . $classValue .
			$disabled . $this->getDataAttributesAsString() . '>';
		return array($this->name => 'file');
	}

	/**
	 * Specifies if this component is enabled and accepts user modification.
	 *
	 * @param boolean $isEnabled enabled if true
	 */
	public function setIsEnabled($isEnabled) {
		$this->isEnabled = $isEnabled;
	}

}

/**
 * File upload with descriptive label and help link.
 *
 * @package metaHTML
 */
class htmlTableExtendedInputFileUpload extends htmlInputFileUpload {

	/** descriptive label */
	private $label;
	/** help ID */
	private $helpID;

	/**
	 * Constructor.
	 *
	 * @param String $name unique name
	 * @param String $label descriptive label
	 * @param String $helpID help ID
	 */
	function __construct($name, $label, $helpID = null) {
		parent::__construct($name);
		$this->label = htmlspecialchars($label);
		$this->helpID = $helpID;
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		echo '<div class="nowrap">';
		echo $this->label;
		echo '</div>';
		echo "\n</td>\n<td>\n";
		$return = parent::generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		// print help link
		if ($this->helpID != null) {
			echo "\n</td>\n<td>\n";
			$helpLink = new htmlHelpLink($this->helpID);
			$helpLink->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		}
		return $return;
	}

}

/**
 * Prints the HTML code for a textarea.
 *
 * @package metaHTML
 */
class htmlInputTextarea extends htmlElement {

	/** unique name of input element */
	private $name;
	/** value */
	private $value;
	/** column count */
	private $colCount;
	/** row count */
	private $rowCount;
	/** enabled or disabled */
	private $isEnabled = true;
	/** specifies if LAM should display this field with a WYSIWYG editor */
	protected $richEdit = false;
	/** required field */
	protected $required = false;

	/**
	 * Constructor.
	 *
	 * @param String $name unique name
	 * @param String $value value
	 * @param int $colCount number of characters per line
	 * @param int $rowCount number of rows
	 */
	function __construct($name, $value, $colCount, $rowCount) {
		$this->name = htmlspecialchars($name);
		$this->value = htmlspecialchars($value);
		$this->colCount = htmlspecialchars($colCount);
		$this->rowCount = htmlspecialchars($rowCount);
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		$this->cssClasses[] = 'ui-corner-all';
		if (isset($values[$this->name])) {
			$this->value = implode("\r\n", $values[$this->name]);
		}
		$colCount = ($this->colCount != null) ? ' cols="' . $this->colCount . '"' : '';
		$rowCount = ($this->rowCount != null) ? ' rows="' . $this->rowCount . '"' : '';
		$tabindexValue = ' tabindex="' . $tabindex . '"';
		$tabindex++;
		$disabled = '';
		if (!$this->isEnabled) {
			$disabled = ' disabled';
		}
		$classList = $this->cssClasses;
		$classes = '';
		if ($this->richEdit) {
			$classList[] = 'ckeditor';
		}
		$validators = array();
		if ($this->required) {
			$validators[] = 'required';
		}
		if (!empty($validators)) {
			$classList[] = 'validate[' . implode(',', $validators) . ']';
		}
		$classes = ' class="' . implode(' ', $classList) . '"';
		echo '<textarea name="' . $this->name . '" id="' . $this->name . '"' . $tabindexValue . $classes . $colCount .
			$rowCount . $this->getDataAttributesAsString() . $disabled . '>' . $this->value . '</textarea>';
		return array($this->name => 'textarea');
	}

	/**
	 * Specifies if this component is enabled and accepts user modification.
	 *
	 * @param boolean $isEnabled enabled if true
	 */
	public function setIsEnabled($isEnabled) {
		$this->isEnabled = $isEnabled;
	}

	/**
	 * Specifies if the textarea should be displayed with a WYSIWYG editor.
	 * <br>This requires that the page which displays the textarea also includes the ckeditor JS.
	 * <br>Rich editing is disabled by default.
	 *
	 * @param boolean $richEdit rich edit or standard
	 */
	public function setIsRichEdit($richEdit) {
		$this->richEdit = $richEdit;
	}

	/**
	 * Specifies if the input field is required.
	 *
	 * @param boolean $required required
	 */
	public function setRequired($required) {
		$this->required = $required;
	}

}

/**
 * Text area with label and help link.
 *
 * @package metaHTML
 */
class htmlTableExtendedInputTextarea extends htmlInputTextarea {

	/** descriptive label */
	private $label;
	/** help ID */
	private $helpID;

	/**
	 * Constructor.
	 *
	 * @param String $name unique name
	 * @param String $value value
	 * @param int $colCount number of characters per line
	 * @param int $rowCount number of rows
	 * @param String $label descriptive label
	 * @param String $helpID help ID
	 */
	function __construct($name, $value, $colCount, $rowCount, $label, $helpID = null) {
		parent::__construct($name, $value, $colCount, $rowCount);
		$this->label = htmlspecialchars($label);
		$this->helpID = $helpID;
		$this->alignment = htmlElement::ALIGN_TOP;
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		echo '<div class="nowrap">';
		echo $this->label;
		if ($this->required) {
			echo htmlGetRequiredMarker();
		}
		echo '</div>';
		echo "\n</td>\n<td>\n";
		$return = parent::generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		// print help link
		if ($this->helpID != null) {
			echo "\n</td>\n<td valign=\"top\">\n";
			$helpLink = new htmlHelpLink($this->helpID);
			$helpLink->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		}
		return $return;
	}

}

/**
 * Prints the HTML code for a color picker field.
 *
 * @package metaHTML
 */
class htmlInputColorPicker extends htmlElement {

	/** unique name of input element */
	private $name;
	/** color value */
	private $color;
	/** enabled or disabled */
	private $isEnabled = true;

	/**
	 * Constructor.
	 *
	 * @param string $name unique name
	 * @param string $colorValue color value (e.g. #000000)
	 */
	public function __construct($name, $colorValue = '#000000') {
		$this->name = htmlspecialchars($name);
		$this->color = htmlspecialchars($colorValue);
	}

	/**
	 * {@inheritDoc}
	 * @see htmlElement::generateHTML()
	 */
	public function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		$tabindexValue = ' tabindex="' . $tabindex . '"';
		$tabindex++;
		$disabled = '';
		if (!$this->isEnabled) {
			$disabled = ' disabled';
		}
		echo '<input type="color" value="' . $this->color . '" id="' . $this->name . '" name="' . $this->name . '"' . $tabindexValue . $disabled . '>';
		return array($this->name => 'file');
	}

	/**
	 * Specifies if this component is enabled and accepts user modification.
	 *
	 * @param boolean $isEnabled enabled if true
	 */
	public function setIsEnabled($isEnabled) {
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
	private $label;
	/** help ID */
	private $helpID;
	/** help module */
	private $helpModule = null;
	/** render HTML of parent class */
	private $renderParentHtml = false;

	/**
	 * Constructor.
	 *
	 * @param string $name unique name
	 * @param string $colorValue color value (e.g. #000000)
	 * @param string $label descriptive label
	 * @param string $helpID help ID
	 */
	public function __construct($name, $colorValue, $label, $helpID = null) {
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
	 * @see htmlInputColorPicker::generateHTML()
	 */
	public function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		if ($this->renderParentHtml) {
			return parent::generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		}
		// HTML of parent class is rendered on second call (done by htmlResponsiveRow)
		$this->renderParentHtml = true;
		$row = new htmlResponsiveRow();
		// label text
		$labelGroup = new htmlGroup();
		$labelGroup->addElement(new htmlOutputText($this->label));
		if (!empty($this->helpID)) {
			$helpLinkLabel = new htmlHelpLink($this->helpID, $this->helpModule);
			$helpLinkLabel->setCSSClasses(array('hide-on-tablet', 'margin-left5'));
			$labelGroup->addElement($helpLinkLabel);
		}
		$row->add($labelGroup, 12, 6, 6, 'responsiveLabel');
		// input field
		$fieldGroup = new htmlGroup();
		$fieldGroup->addElement($this);
		if (!empty($this->helpID)) {
			$helpLinkField = new htmlHelpLink($this->helpID, $this->helpModule);
			$helpLinkField->setCSSClasses(array('hide-on-mobile'));
			$fieldGroup->addElement($helpLinkField);
		}
		$row->add($fieldGroup, 12, 6, 6, 'responsiveField nowrap');
		return $row->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
	}

}

/**
 * Prints the HTML code for an image.
 *
 * @package metaHTML
 */
class htmlImage extends htmlElement {

	/** path to image */
	private $path;
	/** width */
	private $width;
	/** height */
	private $height;
	/** alt text */
	private $alt;
	/** title */
	private $title;
	/** onClick event */
	private $onClick = null;
	/** enable cropping */
	private $crop = false;
	/** enable lightbox */
	private $lightbox = false;
	/** @var string help popup title */
	private $helpTitle = null;
	/** @var htmlElement help popup content	*/
	private $helpContent = null;

	/**
	 * Constructor.
	 *
	 * @param String $path image location
	 * @param int $width image width (optional, default original size)
	 * @param int $height image height (optional, default original size)
	 * @param String $alt alt text (optional)
	 * @param String $onClick onClick code (optional)
	 */
	public function __construct($path, $width = null, $height = null, $alt = ' ', $title = null, $onClick = null) {
		$this->path = htmlspecialchars($path);
		$this->width = $width;
		$this->height = $height;
		$this->alt = htmlspecialchars($alt);
		$this->title = $title;
		$this->onClick = $onClick;
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	public function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
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
		$classes = '';
		if (!empty($this->cssClasses)) {
			$classes = 'class="' . implode(' ', $this->cssClasses) . '"';
		}
		$onClick = '';
		if ($this->onClick != null) {
			$onClick = ' onclick="' . $this->onClick . '"';
		}
		if ($this->lightbox) {
			echo '<a href="' . $this->path . '" class="lam-lightbox">';
		}
		$helpTitleValue = '';
		$helpContentValue = '';
		if ($this->helpTitle !== null) {
			$helpTitleValue = ' helptitle="' . $this->helpTitle . '"';
			ob_start();
			$this->helpContent->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
			$helpContentString = ob_get_contents();
			ob_end_clean();
			$helpContentValue = ' helpdata="' . htmlspecialchars($helpContentString) . '"';
		}
		echo '<img' . $path . $width . $height . $alt . $title . $classes . $onClick . $helpTitleValue . $helpContentValue . $this->getDataAttributesAsString() . ">";
		if ($this->lightbox) {
			echo '</a>';
		}
		if ($this->crop) {
			$cropJsPath = 'templates/lib/extra/cropperjs/cropper.js';
			if (is_file('../../templates/login.php')) {
				$cropJsPath = '../../' . $cropJsPath;
			}
			else {
				$cropJsPath = '../' . $cropJsPath;
			}
			echo '<script type="text/javascript" src="' . $cropJsPath . '"></script>';
			echo '<script type="text/javascript">
					var image = jQuery(\'.cropperjsImage\')[0];
					var cropper = new Cropper(image, {
						viewMode: 1,
						movable: false,
						zoomable: false,
						crop: function(event) {
							jQuery(\'#croppingDataX\').val(event.detail.x);
							jQuery(\'#croppingDataY\').val(event.detail.y);
							jQuery(\'#croppingDataWidth\').val(event.detail.width);
							jQuery(\'#croppingDataHeight\').val(event.detail.height);
						}
					});
					</script>';
			echo '<input id="croppingDataX" type="hidden" name="croppingDataX" value="0"/>';
			echo '<input id="croppingDataY" type="hidden" name="croppingDataY" value="0"/>';
			echo '<input id="croppingDataWidth" type="hidden" name="croppingDataWidth" value="0"/>';
			echo '<input id="croppingDataHeight" type="hidden" name="croppingDataHeight" value="0"/>';
		}
		return array();
	}

	/**
	 * Enables cropping feature.
	 * This will display a cropping box on the image. The cropping data
	 * can be found in POST data (croppingDataX, croppingDataY, croppingDataWidth, croppingDataHeight).
	 */
	public function enableCropping() {
		$this->crop = true;
	}

	/**
	 * Enables lightbox feature.
	 */
	public function enableLightbox() {
		$this->lightbox = true;
	}

	/**
	 * Activates the help popup on hover.
	 *
	 * @param string $title title
	 * @param htmlElement $content help content
	 */
	public function setHelpData(string $title, htmlElement $content) {
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

}

/**
 * Adds an empty space with given width and height.
 *
 * @package metaHTML
 */
class htmlSpacer extends htmlElement {

	/** width of spacer in px */
	private $width = null;
	/** height of spacer in px */
	private $height = null;

	/**
	 * Constructor.
	 *
	 * @param String $width width (e.g. 10px)
	 * @param String $height height (e.g. 10px)
	 */
	function __construct($width, $height = null) {
		if ($width !== null) {
			$this->width = htmlspecialchars($width);
		}
		if ($height !== null) {
			$this->height = htmlspecialchars($height);
		}
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		$width = '';
		if ($this->width !== null) {
			$width = 'width: ' . $this->width . ';';
		}
		$height = '';
		if ($this->height !== null) {
			$height = 'height: ' . $this->height . ';';
		}
		echo "<div style=\"$width $height display: inline-block;\"></div>\n";
		return array();
	}

}

/**
 * Prints a status message (e.g. error message).
 *
 * @package metaHTML
 */
class htmlStatusMessage extends htmlElement {

	/** message type (e.g. ERROR) */
	private $type;
	/** message title */
	private $title;
	/** message text */
	private $text;
	/** message parameters */
	private $params;

	/**
	 * Constructor.
	 *
	 * @param String $type message type (e.g. ERROR)
	 * @param String $title message title
	 * @param String $text message (optional)
	 * @param array $params additional message parameters
	 */
	public function __construct($type, $title, $text = null, $params = null) {
		$this->type = $type;
		$this->title = $title;
		$this->text = $text;
		$this->params = $params;
	}

	/**
	 * Constructor with parameter array.
	 *
	 * @param array $params parameters in same order as normal constructor
	 * @return htmlStatusMessage
	 */
	public static function fromParamArray($params) {
		if (sizeof($params) < 2) {
			throw new BadMethodCallException("Invalid parameter count");
		}

		$count = count($params);
		switch ($count) {
			case 2:
				return new htmlStatusMessage($params[0], $params[1]);
				break;
			case 3:
				return new htmlStatusMessage($params[0], $params[1], $params[2]);
				break;
			case 4:
				return new htmlStatusMessage($params[0], $params[1], $params[2], $params[3]);
				break;
			default:
				throw new BadMethodCallException("Invalid parameter count");
		}
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	public function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		if (!empty($this->cssClasses)) {
			echo '<div class="' . implode(' ', $this->cssClasses) . '">';
		}
		StatusMessage($this->type, $this->title, $this->text, $this->params);
		if (!empty($this->cssClasses)) {
			echo '</div>';
		}
		return array();
	}

	/**
	 * Returns the message type.
	 *
	 * @return String type
	 */
	public function getType() {
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
	private $label = null;

	/**
	 * Constructor.
	 *
	 * @param String $label label
	 */
	function __construct($label) {
		$this->label = htmlspecialchars($label);
		// the title should not end at a table cell
		$this->colspan = 100;
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		echo "<div class=\"title\">\n";
		echo "<h2 class=\"titleText\">\n";
		echo $this->label;
		echo "</h2>\n";
		echo "</div>\n";
		return array();
	}

}

/**
 * Generates a subtitle line. This is used to group multiple fields.
 *
 * @package metaHTML
 */
class htmlSubTitle extends htmlElement {

	/** descriptive label */
	private $label = null;
	/** optional image */
	private $image = null;
	/** optional ID for this element (e.g. to use for JavaScript) */
	private $id = null;
	/** show large icon */
	private $largeIcon = false;
	/** help ID */
	private $helpId = null;

	/**
	 * Constructor.
	 *
	 * @param String $label label
	 * @param String $image optional image
	 * @param String $id optional ID for this element (e.g. to use for JavaScript)
	 * @param bool $largeIcon show large (32x32px) icon instead of small one (16x16px)
	 */
	public function __construct($label, $image = null, $id = null, $largeIcon = false) {
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
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	public function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		$idValue = '';
		if ($this->id != null) {
			$idValue = ' id="' . $this->id . '"';
		}
		echo "<div $idValue class=\"subTitle\">\n";
		echo "<h4 class=\"subTitleText\">\n";
		if ($this->image != null) {
			$size = $this->largeIcon ? 32 : 16;
			echo '<img height=' . $size . ' width=' . $size . ' src="' . $this->image . '" alt="' . $this->label . '">&nbsp;';
		}
		echo $this->label;
		if ($this->helpId !== null) {
			$spacer = new htmlSpacer('0.5rem', null);
			$spacer->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
			$helpLink = new htmlHelpLink($this->helpId);
			$helpLink->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		}
		echo "</h4>\n";
		echo "</div>\n";
		return array();
	}

	/**
	 * Sets an additional help id.
	 *
	 * @param string|array $helpId
	 */
	public function setHelpId($helpId) {
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
	private $name = null;
	/** field value */
	private $value = null;

	/**
	 * Constructor.
	 *
	 * @param String $name input name
	 * @param String $value input value
	 */
	function __construct($name, $value) {
		$this->name = htmlspecialchars($name);
		$this->value = htmlspecialchars($value);
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		echo '<input type="hidden" name="' . $this->name . '" id="' . $this->name . '" value="' . $this->value . '">';
		return array($this->name => 'hidden');
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
	private $text = null;
	/** link target */
	protected $target = null;
	/** optional image */
	private $image = null;
	/** title */
	private $title = null;
	/** target window */
	private $targetWindow = null;
	/** onClick event */
	private $onClick = null;
	/** onMouseOver event */
	private $onMouseOver = null;
	/** link id */
	private $id;

	/**
	 * Constructor.
	 *
	 * @param String $text label
	 * @param String $target target URL
	 * @param String $image URL of optional image
	 */
	function __construct($text, $target, $image = null) {
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
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
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
			$id = !empty($this->id) ? $this->id : 'a_' . preg_replace('/[^a-zA-Z0-9_]+/', '_', $this->target);
			$idAttr = ' id="' . $id . '"';
		}
		$classAttr = '';
		if (sizeof($this->cssClasses) > 0) {
			$classAttr = ' class="' . implode(' ', $this->cssClasses) . '"';
		}
		echo '<a href="' . $this->target . '"' . $idAttr . $classAttr . $title . $targetWindow . $onClick . $onMouseOver . $this->getDataAttributesAsString() . '>' . $image . $text . '</a>';
		return array();
	}

	/**
	 * Returns the value for the alt attribute.
	 *
	 * @return string alt value
	 */
	protected function getAlt() {
		return $this->text;
	}

	/**
	 * Returns the value for the link content.
	 *
	 * @return string content
	 */
	protected function getContent() {
		return $this->text;
	}

	/**
	 * Sets the link title.
	 *
	 * @param String $title title
	 */
	public function setTitle($title) {
		$this->title = htmlspecialchars($title);
	}

	/**
	 * Sets the target window (e.g. _blank).
	 *
	 * @param String $window target window (e.g. _blank)
	 */
	public function setTargetWindow($window) {
		$this->targetWindow = htmlspecialchars($window);
	}

	/**
	 * Sets the onClick event.
	 *
	 * @param String $event JavaScript code
	 */
	public function setOnClick($event) {
		$this->onClick = htmlspecialchars($event);
	}

	/**
	 * Sets the onMouseOver event.
	 *
	 * @param String $event JavaScript code
	 */
	public function setOnMouseOver($event) {
		$this->onMouseOver = htmlspecialchars($event);
	}

	/**
	 * Sets the element id.
	 *
	 * @param string $id unique id
	 */
	public function setId($id) {
		$this->id = $id;
	}

}

/**
 * Generates a link around a htmlElement.
 *
 * @package metaHTML
 */
class htmlContentLink extends htmlLink {

	private $content = null;
	private $contentText = '';

	/**
	 * Constructor
	 *
	 * @param htmlElement $content content to link
	 * @param String $target link target
	 * @param boolean $highlightOnHover highlight content on hover
	 */
	function __construct($content, $target) {
		$this->content = $content;
		$this->target = htmlspecialchars($target);
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		ob_start();
		parseHtml($module, $this->content, $values, $restricted, $tabindex, $scope);
		$this->contentText = ob_get_contents();
		ob_end_clean();
		return parent::generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
	}

	/**
	 * Returns the value for the alt attribute.
	 *
	 * @return string alt value
	 */
	protected function getAlt() {
		return '';
	}

	/**
	 * Returns the value for the link content.
	 *
	 * @return string content
	 */
	protected function getContent() {
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

	/** link text */
	private $subelements = array();

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		$return = array();
		for ($i = 0; $i < sizeof($this->subelements); $i++) {
			$return = array_merge($return, $this->subelements[$i]->generateHTML($module, $input, $values, $restricted, $tabindex, $scope));
		}
		return $return;
	}

	/**
	 * Adds a subelement.
	 *
	 * @param htmlElement $sub subelement
	 */
	public function addElement($sub) {
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
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		$return = array();
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
	private $id = null;
	/** htmlElement that generates inner content */
	private $content = null;

	/**
	 * Constructor.
	 *
	 * @param String $id unique ID
	 * @param htmlElement $content inner content
	 * @param array $classes CSS classes
	 * @param string[] $cssClasses CSS classes
	 */
	function __construct($id, $content, $cssClasses = null) {
		if ($id !== null) {
			$this->id = htmlspecialchars($id);
		}
		$this->content = $content;
		$this->cssClasses = $cssClasses;
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		$return = array();
		$idValue = '';
		if ($this->id != null) {
			$idValue = ' id="' . $this->id . '"';
		}
		$classesValue = '';
		if (($this->cssClasses != null) && (sizeof($this->cssClasses) > 0)) {
			$classesValue = ' class="' . implode(' ', $this->cssClasses) . '"';
		}
		echo '<div' . $idValue . $classesValue . $this->getDataAttributesAsString() . '>';
		if ($this->content != null) {
			$return = $this->content->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		}
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
	private $content = null;
	/** onclick handler */
	private $onclick = null;
	/** title */
	private $title = null;

	/**
	 * Constructor.
	 *
	 * @param htmlElement $content inner content
	 * @param array $classes CSS classes
	 * @param string[] $cssClasses CSS classes
	 */
	function __construct($content, $cssClasses = null) {
		$this->content = $content;
		$this->cssClasses = $cssClasses;
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		$return = array();
		$classesValue = '';
		if (($this->cssClasses != null) && (sizeof($this->cssClasses) > 0)) {
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
		if ($this->content != null) {
			$return = $this->content->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		}
		echo '</span>';
		return $return;
	}

	/**
	 * Sets the onclick event.
	 *
	 * @param string $event event handler code
	 */
	public function setOnclick($event) {
		$this->onclick = $event;
	}

	/**
	 * Sets the title.
	 *
	 * @param string|null $title title
	 */
	public function setTitle(?string $title): void {
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
	private $content = null;

	/**
	 * Constructor.
	 *
	 * @param String $content script
	 */
	function __construct($content) {
		$this->content = $content;
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		$return = array();
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
	private $id = null;

	/**
	 * Constructor.
	 *
	 * @param String $content script
	 */
	function __construct($id = null) {
		$this->id = $id;
	}

	/**
	 * {@inheritDoc}
	 * @see htmlElement::generateHTML()
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		$return = array();
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
	private $src = null;
	/** is async */
	private $async = false;
	/** execute after page is parsed */
	private $defer = false;

	/**
	 * Constructor.
	 *
	 * @param String $src script path
	 * @param boolean $isAsync script will be executed while the page continues the parsing (default true)
	 * @param boolean $isDeferred script is executed when the page has finished parsing (default true)
	 */
	function __construct($src, $isAsync = true, $isDeferred = true) {
		$this->src = $src;
		$this->async = $isAsync;
		$this->defer = $isDeferred;
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		$return = array();
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
	private $src = null;

	/**
	 * Constructor.
	 *
	 * @param String $src script path
	 */
	function __construct($src) {
		$this->src = $src;
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		$return = array();
		echo '<link href="' . $this->src . '" type="text/css" rel="stylesheet"/>';
		return $return;
	}

}

/**
 * Sets all given elements to the same width.
 *
 * @package metaHTML
 */
class htmlEqualWidth extends htmlElement {

	/** list of element IDs */
	private $elements = array();

	/**
	 * Constructor.
	 *
	 * @param array $elements list of element IDs
	 */
	function __construct($elements) {
		foreach ($elements as $element) {
			$this->elements[] = htmlspecialchars($element);
		}
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		if (sizeof($this->elements) == 0) {
			return array();
		}
		$return = array();
		$listContent = "'#" . $this->elements[0] . "'";
		for ($i = 1; $i < sizeof($this->elements); $i++) {
			$listContent .= ", '#" . $this->elements[$i] . "'";
		}
		echo '<script type="text/javascript">';
		echo ' jQuery(document).ready(function() {';
		echo '  var equalWidthElements = new Array(' . $listContent . ');';
		echo '  equalWidth(equalWidthElements);';
		echo ' });';
		echo '</script>';
		return $return;
	}

}

/**
 * Sets all given elements to the same height.
 *
 * @package metaHTML
 */
class htmlEqualHeight extends htmlElement {

	/** list of element IDs */
	private $elements = array();

	/**
	 * Constructor.
	 *
	 * @param array $elements list of element IDs
	 */
	function __construct($elements) {
		foreach ($elements as $element) {
			$this->elements[] = htmlspecialchars($element);
		}
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		if (sizeof($this->elements) == 0) {
			return array();
		}
		$return = array();
		$listContent = "'#" . $this->elements[0] . "'";
		for ($i = 1; $i < sizeof($this->elements); $i++) {
			$listContent .= ", '#" . $this->elements[$i] . "'";
		}
		echo '<script type="text/javascript">';
		echo ' jQuery(document).ready(function() {';
		echo '  var equalHeightElements = new Array(' . $listContent . ');';
		echo '  equalHeight(equalHeightElements);';
		echo ' });';
		echo '</script>';
		return $return;
	}

}

/**
 * Creates a list of elements that can be sorted by the user via drag'n'drop.
 *
 * @package metaHTML
 */
class htmlSortableList extends htmlElement {

	/** list of elements */
	private $elements = array();
	/** HTML ID */
	private $id = '';
	/** element width */
	private $elementWidth = '';
	/** on update event */
	private $onUpdate = null;

	/**
	 * Constructor.
	 *
	 * @param array $elements list of elements as text (HTML special chars must be escaped already) or htmlElement
	 * @param String HTML ID
	 * @param String $elementWidth width of elements (default 250px)
	 */
	function __construct($elements, $id, $elementWidth='250px') {
		$this->elements = $elements;
		$this->id = htmlspecialchars($id);
		$this->elementWidth = $elementWidth;
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		if (sizeof($this->elements) == 0) {
			return array();
		}
		$return = array();
		echo '<ul style="width:' . $this->elementWidth . ';" class="sortableList" id="' . $this->id . '">';
		foreach ($this->elements as $element) {
			echo '<li class="ui-state-default"><span class="ui-icon ui-icon-arrowthick-2-n-s"></span>';
			if ($element instanceof htmlElement) {
				parseHtml($module, $element, $values, $restricted, $tabindex, $scope);
			}
			else {
				echo $element;
			}
			echo '</li>';
		}
		echo '</ul>';
		$onUpdate = '';
		if ($this->onUpdate != null) {
			$onUpdate = '{
				update: function(event, ui) {' . $this->onUpdate . '},
				start:  function(event, ui) {
				            var posOrig = ui.item.index();
            				ui.item.data(\'posOrig\', posOrig);
        				}
			}';
		}
		$scriptContent = '
			jQuery(function() {
				$("#' . $this->id . '").sortable(' . $onUpdate . ');
				$("#' . $this->id . '").disableSelection();
			});';
		$script = new htmlJavaScript($scriptContent);
		$script->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		return $return;
	}

	/**
	 * Sets the JS code that is executed when the element order was changed.
	 * The code can access the variables event and ui. See JQueryUI docs for details.
	 * ui.item.data('posOrig') will contain the original position of the moved element.
	 *
	 * @param String $onUpdate JS code
	 */
	public function setOnUpdate($onUpdate) {
		$this->onUpdate = $onUpdate;
	}

}

/**
 * Creates a list of content elements in accordion style.
 * HTML special characters must be escaped before providing to htmlAccordion.
 */
class htmlAccordion extends htmlElement {

	private $id = null;
	private $elements = null;
	private $openInitial = '1';
	private $collapsible = false;

	/**
	 * Constructor.
	 *
	 * @param String $id HTML ID
	 * @param array $elements list of content elements array('title' => htmlElement)
	 * @param String $openInitial index of element that is initially opened (default: 0), set to 'false' to close all
	 * @param boolean $collapsible specifies if all elements may be closed at the same time (default: false, true if $openInitial is false)
	 */
	function __construct($id, $elements, $openInitial = '0', $collapsible = false) {
		$this->id = $id;
		$this->elements = $elements;
		$this->openInitial = $openInitial;
		if (($openInitial === 'false') || ($openInitial === false)) {
			$this->collapsible = true;
		}
		else {
			$this->collapsible = $collapsible;
		}
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		$result = array();
		$collapsible = 'false';
		if ($this->collapsible) {
			$collapsible = 'true';
		}
		$active = 'false';
		if ($this->openInitial !== false) {
			$active = $this->openInitial;
		}
		echo '<div id="' . $this->id . '">';
		foreach ($this->elements as $label => $content) {
			echo '<h3>' . $label . '</h3>';
			echo '<div>';
			$result = array_merge($result, $content->generateHTML($module, $input, $values, $restricted, $tabindex, $scope));
			echo '</div>';
		}
		echo '</div>';
		$hiddenIndexId = $this->id . "_index";
		echo '<input type="hidden" name="' . $hiddenIndexId . '" id="' . $hiddenIndexId . '" value="' . $active . '">';
		$script = 'jQuery(function() {
						$( "#' . $this->id . '" ).accordion({
						collapsible: ' . $collapsible . ',
						active: ' . $active . ',
						activate: function( event, ui ) {
									var newOption = jQuery("#' . $this->id . '").accordion( "option", "active" );
									jQuery("#' . $this->id . '_index").val(newOption);
								}
						});
					});';
		$js = new htmlJavaScript($script);
		$js->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		return $result;
	}

}

/**
 * Creates a Google reCAPTCHA element.
 *
 * @package metaHTML
 */
class htmlReCAPTCHA extends htmlElement {

	/** site key */
	private $key = null;

	/**
	 * Constructor.
	 *
	 * @param String $key site key
	 */
	function __construct($key) {
		$this->key = htmlspecialchars($key);
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		$script = new htmlScript('https://www.google.com/recaptcha/api.js');
		$script->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		echo '<div class="g-recaptcha" data-sitekey="' . $this->key . '"></div>';
		return array();
	}
}

/**
 * Responsive row with 12 column layout.
 */
class htmlResponsiveRow extends htmlElement {

	/** @var htmlResponsiveCell[] cells */
	private $cells = array();
	/** HTML ID */
	private $id = null;

	/**
	 * Creates a new responsive row.
	 *
	 * @param htmlElement $label label element if this is a simple label+field row
	 * @param htmlElement $field field element if this is a simple label+field row
	 */
	public function __construct($label = null, $field = null) {
		$this->cells = array();
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
	 * @param string $id ID
	 */
	public function setId($id) {
		$this->id = $id;
	}

	/**
	 * Adds a responsive cell to the row.
	 *
	 * @param htmlResponsiveCell $cell cell
	 */
	public function addCell($cell) {
		$this->cells[] = $cell;
	}

	/**
	 * Adds a cell with the given content and column counts.
	 *
	 * @param htmlElement $content content inside cell
	 * @param int $numMobile number of columns for mobile view
	 * @param int $numTablet number of columns for tablet view (set to mobile if null)
	 * @param int $numDesktop number of columns for desktop view (set to tablet if null)
	 * @param String $classes CSS classes separated by space
	 */
	public function add($content, $numMobile = 12, $numTablet = null, $numDesktop = null, $classes = '') {
		$tabletCols = ($numTablet === null) ? $numMobile : $numTablet;
		$desktopCols = ($numDesktop === null) ? $tabletCols : $numDesktop;
		$this->addCell(new htmlResponsiveCell($content, $numMobile, $tabletCols, $desktopCols, $classes));
	}

	/**
	 * Adds the content as a typical label with 12/6/6 columns and CSS class "responsiveLabel".
	 *
	 * @param htmlElement $content label
	 * @param string $cssClasses additional CSS classes
	 */
	public function addLabel($content, $cssClasses = '') {
		$this->add($content, 12, 6, 6, 'responsiveLabel nowrap ' . $cssClasses);
	}

	/**
	 * Adds the content as a typical field with 12/6/6 columns and CSS class "responsiveField".
	 *
	 * @param htmlElement $content field
	 * @param string $cssClasses CSS class names separated by space
	 */
	public function addField($content, $cssClasses = '') {
		$this->add($content, 12, 6, 6, 'responsiveField ' . $cssClasses);
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	public function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		$return = array();
		$cssClasses = implode(' ', $this->cssClasses);
		$idParam = '';
		if ($this->id !== null) {
			$idParam = ' id="' . $this->id . '"';
		}
		echo '<div class="row ' . $cssClasses . '"' .  $this->getDataAttributesAsString() . $idParam . '>';
		foreach ($this->cells as $cell) {
			$return = array_merge($return, $cell->generateHTML($module, $input, $values, $restricted, $tabindex, $scope));
		}
		echo '</div>';
		return $return;
	}

	/**
	 * Adds a vertical spacer with 12 columns.
	 *
	 * @param string $space space in px or rem
	 */
	public function addVerticalSpacer($space) {
		$this->add(new htmlSpacer(null,	$space), 12);
	}

	/**
	 * Returns the current number of cells.
	 *
	 * @return int cell count
	 */
	public function getCellCount(): int {
		return sizeof($this->cells);
	}

}

/**
 * Responsive cell inside htmlResponsiveRow with 12 column layout.
 */
class htmlResponsiveCell extends htmlElement {

	private $content = null;
	private $mobile = null;
	private $tablet = null;
	private $desktop = null;
	private $classes = '';

	/**
	 * Constructs a cell inside a responsive row with 12 columns.
	 *
	 * @param htmlElement $content content inside cell
	 * @param int $numMobile number of columns for mobile view
	 * @param int $numTablet number of columns for tablet view
	 * @param int $numDesktop number of columns for desktop view
	 * @param String $classes CSS classes separated by space
	 */
	public function __construct($content, $numMobile, $numTablet, $numDesktop, $classes = '') {
		$this->content = $content;
		$this->mobile = $numMobile;
		$this->tablet = $numTablet;
		$this->desktop = $numDesktop;
		$this->classes = $classes;
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	public function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		$clMobile = ($this->mobile > 0) ? 'small-' . $this->mobile : 'hide-for-small-only';
		$clTablet = ($this->tablet > 0) ? 'medium-' . $this->tablet : 'hide-for-medium-only';
		$clDesktop = ($this->desktop > 0) ? 'large-' . $this->desktop : 'hide-for-large-only';

		echo '<div class="' . $clMobile . ' ' . $clTablet . ' ' . $clDesktop . ' columns ' . $this->classes . '">';
		$return = $this->content->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
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
	private $label;
	/** help ID */
	private $helpID;
	/** help module name */
	private $helpModule = null;
	/** render HTML of parent class */
	private $renderParentHtml = false;
	/** short label */
	private $shortLabel = false;
	
	/**
	 * Constructor
	 *
	 * @param String $label descriptive label
	 * @param String $fieldName unique field name
	 * @param String $fieldValue value of input field (optional)
	 * @param String|array $helpID help ID or array of help ID + module name (optional)
	 * @param bool $required field is required
	 */
	function __construct($label, $fieldName, $fieldValue = null, $helpID = null, $required = false) {
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
	 * @see htmlInputField::generateHTML()
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		if ($this->renderParentHtml) {
			return parent::generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
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
			$helpLinkLabel->setCSSClasses(array('hide-on-tablet', 'margin-left5'));
			$labelGroup->addElement($helpLinkLabel);
		}
		$tabletDesktopLabelColumns = $this->shortLabel ? 4 : 6;
		$row->add($labelGroup, 12, $tabletDesktopLabelColumns, $tabletDesktopLabelColumns, 'responsiveLabel');
		// input field
		$fieldGroup = new htmlGroup();
		$fieldGroup->addElement($this);
		if (!empty($this->helpID)) {
			$helpLinkField = new htmlHelpLink($this->helpID, $this->helpModule);
			$helpLinkField->setCSSClasses(array('hide-on-mobile'));
			$fieldGroup->addElement($helpLinkField);
		}
		$tabletDesktopFieldColumns = $this->shortLabel ? 8 : 6;
		$row->add($fieldGroup, 12, $tabletDesktopFieldColumns, $tabletDesktopFieldColumns, 'responsiveField nowrap');
		return $row->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
	}

	/**
	 * Use a short label (4 columns instead of 6) for tablet/desktop.
	 */
	public function setShortLabel() {
		$this->shortLabel = true;
	}

}

/**
 * File upload with descriptive label and help link.
 *
 * @package metaHTML
 */
class htmlResponsiveInputFileUpload extends htmlInputFileUpload {

	/** descriptive label */
	private $label;
	/** help ID */
	private $helpID;
	/** help module */
	private $helpModule = null;
	/** render HTML of parent class */
	private $renderParentHtml = false;
	/** required */
	private $required = false;
	
	/**
	 * Constructor.
	 *
	 * @param String $name unique name
	 * @param String $label descriptive label
	 * @param String $helpID help ID
	 * @param string $required required field
	 */
	function __construct($name, $label, $helpID = null, $required = false) {
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
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		if ($this->renderParentHtml) {
			return parent::generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
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
			$helpLinkLabel->setCSSClasses(array('hide-on-tablet', 'margin-left5'));
			$labelGroup->addElement($helpLinkLabel);
		}
		$row->add($labelGroup, 12, 6, 6, 'responsiveLabel');
		// input field
		$fieldGroup = new htmlGroup();
		$fieldGroup->addElement($this);
		if (!empty($this->helpID)) {
			$helpLinkField = new htmlHelpLink($this->helpID, $this->helpModule);
			$helpLinkField->setCSSClasses(array('hide-on-mobile'));
			$fieldGroup->addElement($helpLinkField);
		}
		$row->add($fieldGroup, 12, 6, 6, 'responsiveField nowrap');
		return $row->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
	}

	/**
	 * Sets the field required.
	 *
	 * @param bool $required field is required (default: true)
	 */
	public function setRequired($required = true) {
		$this->required = $required;
	}

}

/**
 * Responsive text area with label and help link.
 *
 * @package metaHTML
 */
class htmlResponsiveInputTextarea extends htmlInputTextarea {

	/** descriptive label */
	private $label;
	/** help ID */
	private $helpID;
	/** help module */
	private $helpModule = null;
	/** render HTML of parent class */
	private $renderParentHtml = false;

	/**
	 * Constructor.
	 *
	 * @param String $name unique name
	 * @param String $value value
	 * @param int $colCount number of characters per line
	 * @param int $rowCount number of rows
	 * @param String $label descriptive label
	 * @param String|array $helpID help ID
	 */
	function __construct($name, $value, $colCount, $rowCount, $label, $helpID = null) {
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
	 * @see htmlInputField::generateHTML()
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		if ($this->renderParentHtml) {
			return parent::generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
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
			$helpCssClasses = array('margin-left5');
			if (!$this->richEdit) {
				$helpCssClasses[] = 'hide-on-tablet';
			}
			$helpLinkLabel->setCSSClasses($helpCssClasses);
			$labelGroup->addElement($helpLinkLabel);
		}
		$row->add($labelGroup, 12, 6, 6, 'responsiveLabel');
		// input field
		$fieldGroup = new htmlGroup();
		$fieldGroup->addElement($this);
		if (!empty($this->helpID) && !$this->richEdit) {
			$helpLink = new htmlHelpLink($this->helpID, $this->helpModule);
			$helpLink->setCSSClasses(array('align-top', 'hide-on-mobile'));
			$fieldGroup->addElement($helpLink);
		}
		$row->add($fieldGroup, 12, 6, 6, 'responsiveField nowrap');
		return $row->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
	}

}

/**
 * Responsive select with label and help link.
 *
 * @package metaHTML
 */
class htmlResponsiveSelect extends htmlSelect {

	/** descriptive label */
	private $label;
	/** help ID */
	private $helpID;
	/** help module name */
	private $helpModule = null;
	/** render HTML of parent class */
	private $renderParentHtml = false;
	/** short label */
	private $shortLabel = false;

	/**
	 * Constructor.
	 *
	 * @param String $name element name
	 * @param array $elements list of elememts
	 * @param array $selectedElements list of selected elements
	 * @param String $label descriptive label
	 * @param String|array $helpID help ID or array of help ID + module name (optional)
	 * @param int $size size (optional, default = 1)
	 */
	function __construct($name, $elements, $selectedElements, $label, $helpID = null, $size = 1) {
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
	 * @see htmlInputField::generateHTML()
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		if ($this->renderParentHtml) {
			return parent::generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		}
		// HTML of parent class is rendered on second call (done by htmlResponsiveRow)
		$this->renderParentHtml = true;
		$row = new htmlResponsiveRow();
		// label text
		$labelGroup = new htmlGroup();
		$labelGroup->addElement(new htmlOutputText($this->label));
		if (!empty($this->helpID)) {
			$helpLinkLabel = new htmlHelpLink($this->helpID, $this->helpModule);
			$helpLinkLabel->setCSSClasses(array('hide-on-tablet', 'margin-left5'));
			$labelGroup->addElement($helpLinkLabel);
		}
		$tabletDesktopLabelColumns = $this->shortLabel ? 4 : 6;
		$row->add($labelGroup, 12, $tabletDesktopLabelColumns, $tabletDesktopLabelColumns, 'responsiveLabel');
		// input field
		$fieldGroup = new htmlGroup();
		$fieldGroup->addElement($this);
		if (!empty($this->helpID)) {
			$helpLink = new htmlHelpLink($this->helpID, $this->helpModule);
			$helpLink->setCSSClasses(array('hide-on-mobile'));
			$fieldGroup->addElement($helpLink);
		}
		$tabletDesktopFieldColumns = $this->shortLabel ? 8 : 6;
		$row->add($fieldGroup, 12, $tabletDesktopFieldColumns, $tabletDesktopFieldColumns, 'responsiveField nowrap');
		return $row->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
	}

	/**
	 * {@inheritDoc}
	 * @see htmlSelect::getShowHideSelector()
	 */
	protected function getShowHideSelector() {
		return '.row';
	}

	/**
	 * Use a short label (4 columns instead of 6) for tablet/desktop.
	 */
	public function setShortLabel() {
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
	private $label;
	/** help ID */
	private $helpID;
	/** render HTML of parent class */
	private $renderParentHtml = false;

	/**
	 * Constructor.
	 *
	 * @param String $name element name
	 * @param array $elements list of elememts
	 * @param array $selectedElements list of selected elements
	 * @param String $label descriptive label
	 * @param String $helpID help ID (optional, default none)
	 * @param int $size size (optional, default = 1)
	 */
	function __construct($label, $name, $elements, $selectedElement = null, $helpID = null) {
		parent::__construct($name, $elements, $selectedElement);
		$this->label = htmlspecialchars($label);
		$this->helpID = $helpID;
	}

	/**
	 * {@inheritDoc}
	 * @see htmlInputField::generateHTML()
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		if ($this->renderParentHtml) {
			return parent::generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		}
		// HTML of parent class is rendered on second call (done by htmlResponsiveRow)
		$this->renderParentHtml = true;
		$row = new htmlResponsiveRow();
		// label text
		$labelGroup = new htmlGroup();
		$labelGroup->addElement(new htmlOutputText($this->label));
		if (!empty($this->helpID)) {
			$helpLinkLabel = new htmlHelpLink($this->helpID);
			$helpLinkLabel->setCSSClasses(array('hide-on-tablet', 'margin-left5'));
			$labelGroup->addElement($helpLinkLabel);
		}
		$row->add($labelGroup, 12, 6, 6, 'responsiveLabel');
		// input field
		$fieldGroup = new htmlGroup();
		$fieldGroup->addElement(new htmlDiv(null, $this, array('float-left')));
		if (!empty($this->helpID)) {
			$helpLink = new htmlHelpLink($this->helpID);
			$helpLink->setCSSClasses(array('hide-on-mobile'));
			$fieldGroup->addElement(new htmlDiv(null, $helpLink));
		}
		$row->add($fieldGroup, 12, 6, 6, 'responsiveField nowrap');
		return $row->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
	}

	/**
	 * Returns the selector to use to find the show/hide elements.
	 *
	 * @return string selector
	 */
	protected function getShowHideSelector() {
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
	private $label;
	/** help ID */
	private $helpID;
	/** help module name */
	private $helpModule = null;
	/** render HTML of parent class */
	private $renderParentHtml = false;
	/** long label */
	private $longLabel = false;
	/** label after checkbox */
	private $labelAfterCheckbox = false;
	/** short label */
	private $shortLabel = false;
	
	/**
	 * Constructor.
	 *
	 * @param String $name unique name
	 * @param boolean $checked checked
	 * @param String $label descriptive label
	 * @param String|array $helpID help ID or array of help ID + module name (optional)
	 * @param bool $longLabel more space for label (default: false)
	 */
	function __construct($name, $checked, $label, $helpID = null, $longLabel = false) {
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
	 * @see htmlInputField::generateHTML()
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		if ($this->renderParentHtml) {
			return parent::generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
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
		$text = new htmlSpan(new htmlOutputText($this->label));
		$text->setCSSClasses($this->cssClasses);
		$text->setOnclick('jQuery(\'#' . $this->name . '\').prop(\'checked\',!jQuery(\'#' . $this->name . '\').prop(\'checked\')); jQuery(\'#' . $this->name . '\').change();');
		// input field
		$fieldGroup = new htmlGroup();
		$fieldGroup->addElement($this);
		if (!empty($this->helpID)) {
			$helpLink = new htmlHelpLink($this->helpID, $this->helpModule);
			$helpLink->setCSSClasses(array('margin-left5 align-unset-img'));
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
		return $row->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
	}

	/**
	 * {@inheritDoc}
	 * @see htmlInputCheckbox::getShowHideSelector()
	 */
	protected function getShowHideSelector() {
		return '.row';
	}

	/**
	 * Sets if the label should be shown after the checkbox instead before it.
	 *
	 * @param bool $labelAfterCheckbox show label after box
	 */
	public function setLabelAfterCheckbox($labelAfterCheckbox = true) {
		$this->labelAfterCheckbox = $labelAfterCheckbox;
	}

	/**
	 * Use a short label (4 columns instead of 6) for tablet/desktop.
	 */
	public function setShortLabel() {
		$this->shortLabel = true;
	}

}

/**
 * Responsive table.
 *
 * @author roland Gruber
 */
class htmlResponsiveTable extends htmlElement {

	/** @var string[] row titles */
	private $titles;

	/** htmlElement[][] data rows */
	private $data;

	/** widthes of the columns */
	private $widths = array();

	/** highlighted rows */
	private $highlighted = array();
	/** CSS class for odd row numbers */
	private $cssOddRow;
	/** CSS class for even row numbers */
	private $cssEvenRow;
	/** onclick code (row number => code) */
	private $onClick = array();
	/** ondoubleclick code (row number => code) */
	private $onDoubleClick = array();

	/**
	 * Creates the table.
	 *
	 * @param string[] $titles row titles
	 * @param htmlElement[][] $data data rows
	 * @param int[] $highlighted list of row numbers that should be highlighted (starting at 0)
	 */
	public function __construct($titles, $data, $highlighted = null) {
		$this->titles = $titles;
		$this->data = $data;
		if (!empty($highlighted) && is_array($highlighted)) {
			$this->highlighted = $highlighted;
		}
	}

	/**
	 * {@inheritDoc}
	 * @see htmlElement::generateHTML()
	 */
	public function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		$return = array();
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
		$titleCount = sizeof($this->titles);
		$counter = 0;
		foreach ($this->data as $row) {
			$cssClass = '';
			$cssClasses = array();
			if (in_array($counter, $this->highlighted)) {
				$cssClasses[] = 'highlighted';
			}
			if (!empty($this->cssEvenRow) && ($counter % 2 === 0)) {
				$cssClasses[] = $this->cssEvenRow;
			}
			if (!empty($this->cssOddRow) && ($counter % 2 === 1)) {
				$cssClasses[] = $this->cssOddRow;
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
				$ids = parseHtml($module, $row[$i], $values, $restricted, $tabindex, $scope);
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
	public function setWidths($widths) {
		$this->widths = $widths;
	}

	/**
	 * Sets the CSS classes for odd and even rows.
	 * The title row counts as row number -1.
	 *
	 * @param string $oddClass class for odd rows
	 * @param string $evenClass class for even rows
	 */
	public function setRowClasses($oddClass, $evenClass) {
		$this->cssOddRow = $oddClass;
		$this->cssEvenRow = $evenClass;
	}

	/**
	 * Sets the onclick code for the rows.
	 *
	 * @param array $calls row number => code
	 */
	public function setOnClickEvents($calls) {
		$this->onClick = $calls;
	}

	/**
	 * Sets the ondoubleclick code for the rows.
	 *
	 * @param array $calls row number => code
	 */
	public function setOnDoubleClickEvents($calls) {
		$this->onDoubleClick = $calls;
	}

}

/**
 * Renders a canvas.
 *
 * @author Roland Gruber
 */
class htmlCanvas extends htmlElement {

	private $id;

	/**
	 * Constructor
	 *
	 * @param string $id html id
	 */
	public function __construct($id) {
		$this->id = $id;
	}

	/**
	 * @inheritDoc
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		$classesValue = '';
		if (!empty($this->cssClasses)) {
			$classesValue = ' class="' . implode(' ', $this->cssClasses) . '"';
		}
		echo '<canvas id="' . $this->id . '" ' . $classesValue . '>';
		echo '</canvas>';
		return array();
	}
}

/**
 * Renders a video.
 *
 * @author Roland Gruber
 */
class htmlVideo extends htmlElement {

	private $id;

	/**
	 * Constructor
	 *
	 * @param string $id html id
	 */
	public function __construct($id) {
		$this->id = $id;
	}

	/**
	 * @inheritDoc
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		$classesValue = '';
		if (!empty($this->cssClasses)) {
			$classesValue = ' class="' . implode(' ', $this->cssClasses) . '"';
		}
		echo '<video id="' . $this->id . '" ' . $classesValue . '>';
		echo '</video>';
		return array();
	}
}

/**
 * Creates a form element for POST.
 *
 * @package metaHTML
 */
class htmlForm extends htmlElement {

	/** form name */
	private $name = null;
	/** submit target */
	private $action = null;
	/** inner content */
	private $content = null;

	/**
	 * Constructor.
	 *
	 * @param string $name name
	 * @param string $action target URL
	 */
	function __construct(string $name, string $action, htmlElement $content) {
		$this->name = $name;
		$this->action = $action;
		$this->content = $content;
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		$return = array();
		echo '<form name="' . $this->name . '" action="' . $this->action . '" method="post" enctype="multipart/form-data">';
		if ($this->content != null) {
			$return = $this->content->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		}
		echo '</form>';
		return $return;
	}

}

