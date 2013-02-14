<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2010 - 2013  Roland Gruber

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

}

/**
 * Structures elements using a table.
 *
 * @package metaHTML
 */
class htmlTable extends htmlElement {

	/** table footer */
	const footer = "</table>\n";
	/** new line */
	const newLine = "</tr><tr>\n";
	
	/** list of subelements */
	private $elements = array();
	/** specifies if currently a row is open */
	private $rowOpen = false;
	/** table width */
	private $width = null;
	
	/**
	 * Constructor
	 * 
	 * @param String $width table width (e.g. 100%)
	 * @see htmlElement
	 */
	function __construct($width = null) {
		$this->width = $width;
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
			$this->elements[] = htmlTable::newLine;
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
		$return = array();
		$width = '';
		if ($this->width != null) {
			$width = ' width="' . htmlspecialchars($this->width) . '"';
		}
		$classAttr = '';
		if (sizeof($this->cssClasses) > 0) {
			$classAttr = ' class="' . implode(' ', $this->cssClasses) . '"';
		}
		echo "<table" . $width . $classAttr . ">\n";
		// print all contained elements
		for ($i = 0; $i < sizeof($this->elements); $i++) {
			// print htmlElement objects
			if ($this->elements[$i] instanceof htmlElement) {
				$fields = $this->elements[$i]->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
				$return = array_merge($return, $fields);
			}
			// print simple Strings
			else {
				if ($i != (sizeof($this->elements) - 1) || !($this->elements[$i] == htmlTable::newLine) ) {
					echo $this->elements[$i];
				}
			}
		}
		if ($this->rowOpen) {
			echo "</tr>\n";
		}
		echo htmlTable::footer;
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
		if ($table->elements[sizeof($table->elements) - 1] == htmlTable::newLine) {
			unset($table->elements[sizeof($table->elements) - 1]);
		}
		// close last row of other table if needed
		if ($table->rowOpen) {
			$table->elements[] = "</tr>\n";
		}
		// close last own row if needed
		if ($this->rowOpen) {
			if ($this->elements[sizeof($this->elements) - 1] == htmlTable::newLine) {
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
	function __construct($cells) {
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
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
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
	protected $fieldValue;
	/** field size (default 30) */
	protected $fieldSize = 30;
	/** field max length (default 255) */
	protected $fieldMaxLength = 255;
	/** on keypress event */
	protected $onKeyPress = null;
	/** password field */
	protected $isPassword = false;
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
	/** multiple values in one field */
	protected $autocompleteMultiValue = false;
	/** separator expression for multiple values in one field */
	protected $autocompleteMultiValueSeparatorExp = null;
	/** separator for multiple values in one field */
	protected $autocompleteMultiValueSeparator = null;
	/** autocompletion suggestions */
	protected $autocompleteValues = array();
	/** autocomplete start at this input length */
	protected $autocompleteMinLength = 1;
	/** show calendar */
	protected $showCalendar = false;
	/** calendar format */
	protected $calendarFormat = '';
	
	/**
	 * Constructor
	 *
	 * @param String $fieldName unique field name
	 * @param String $fieldValue value of input field (optional)
	 * @param String $fieldSize maximum input length
	 */
	function __construct($fieldName, $fieldValue = null, $fieldSize = null) {
		if (isObfuscatedText($fieldValue)) {
			$fieldValue = deobfuscateText($fieldValue);
		}
		$this->fieldName = htmlspecialchars($fieldName);
		$this->fieldValue = htmlspecialchars($fieldValue);
		if ($fieldSize != null) {
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
		if (isset($values[$this->fieldName])) {
			if (isObfuscatedText($values[$this->fieldName][0])) {
				$this->fieldValue = deobfuscateText($values[$this->fieldName][0]);
			}
			else {
				$this->fieldValue = $values[$this->fieldName][0];
			}
		}
		$validators = array();
		if ($this->required) {
			$validators[] = 'required';
		}
		if ($this->validationRule != null) {
			$validators[] = 'custom[' . $this->validationRule . ']';
		}
		// print input field
		$class = '';
		if (sizeof($validators) > 0) {
			$class = ' class="validate[' . implode(',', $validators) . '] ' . implode(' ', $this->cssClasses) . '"';
		}
		else {
			$class = ' class="' . implode(' ', $this->cssClasses) . '"';
		}
		$name = ' name="' . $this->fieldName . '"';
		$id = ' id="' . $this->fieldName . '"';
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
		$disabled = '';
		if (!$this->isEnabled) {
			$disabled = ' disabled';
		}
		$onKeyPress = '';
		if ($this->onKeyPress != null) {
			$onKeyPress = ' onkeypress="' . $this->onKeyPress . '"';
		}
		echo '<input type="' . $inputType . '"' . $class . $name . $id . $value . $maxLength . $size . $fieldTabIndex . $onKeyPress . $disabled . '>';
		// autocompletion
		if ($this->autocomplete) {
			echo "<script type=\"text/javascript\">\n";
				echo 'jQuery(function() {';
				echo 'var availableTags' . $this->fieldName . ' = [' . implode(',', $this->autocompleteValues) . '];';
				if (!$this->autocompleteMultiValue) {
					echo 'jQuery( "#' . $this->fieldName . '" ).autocomplete({ source: availableTags' . $this->fieldName . ', minLength: ' . $this->autocompleteMinLength . '});';
				}
				else {
					echo 'function split' . $this->fieldName . '(val) {return val.split( /' . $this->autocompleteMultiValueSeparatorExp . '/ );}';
					echo 'function extractLast' . $this->fieldName . '(term) {return split' . $this->fieldName . '(term).pop();}';
					echo 'jQuery( "#' . $this->fieldName . '" ).bind("keydown", function(event) {
									if (event.keyCode === jQuery.ui.keyCode.TAB && jQuery(this).data("autocomplete").menu.active ) {
										event.preventDefault();
									}
							})';
					echo '.autocomplete({
						source: function(request, response) {response(jQuery.ui.autocomplete.filter(
									availableTags' . $this->fieldName . ', extractLast' . $this->fieldName . '(request.term)));
								},
						minLength: ' . $this->autocompleteMinLength . ',
						focus: function() {return false;},
						select: function(event,ui) {
							var terms = split' . $this->fieldName . '(this.value);
							terms.pop();
							terms.push(ui.item.value);
							terms.push("");
							this.value = terms.join("' . $this->autocompleteMultiValueSeparator . '");return false;
						}
						});';
				}
				echo '});';
			echo "</script\n>";
		}
		if ($this->showCalendar) {
			echo '<script type="text/javascript">
				jQuery(function() {
					$("#' . $this->fieldName . '").datepicker({ dateFormat: "' . $this->calendarFormat . '"});
				});
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
	 * Sets the field size.
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
	 */
	public function setIsPassword($isPassword) {
		$this->isPassword = $isPassword;
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
	 * Enables autocompletion for this input field.
	 * 
	 * @param array $values list of values to suggest
	 * @param int $minLength autocompletion starts after this number of caracters entered (default 1; 0 means immediate start)
	 * @param boolean $multiValue allow multiple autocompletion values in the same fields
	 * @param String $multiSeparator separator expression if multiple autocompletion values are allowed (default ",\s*")
	 * @param String $multiSeparator separator for two values (default ", ")
	 */
	public function enableAutocompletion($values, $minLength = 1, $multiValue = false, $multiSeparatorExp = ',\s*', $multiSeparator = ', ') {
		for ($i = 0; $i < sizeof($values); $i++) {
			$values[$i] = '"' . htmlspecialchars($values[$i]) . '"';
		}
		$this->autocomplete = true;
		$this->autocompleteValues = $values;
		$this->autocompleteMinLength = $minLength;
		$this->autocompleteMultiValue = $multiValue;
		$this->autocompleteMultiValueSeparatorExp = $multiSeparatorExp;
		$this->autocompleteMultiValueSeparator = $multiSeparator;
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
	 * Shows a calendar when the field is selected.
	 * 
	 * @param String $format calendar format (e.g. yyyy-mm-dd)
	 */
	public function showCalendar($format) {
		$this->showCalendar = true;
		$this->calendarFormat = $format;
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
	 */
	function __construct($label, $fieldName, $fieldValue = null, $helpID = null) {
		parent::__construct($fieldName, $fieldValue);
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
		// print label text
		echo '<div class="nowrap">';
		echo $this->label;
		if ($this->required) {
			$graphicsPath = "../../graphics";
			if (is_dir("../graphics")) $graphicsPath = "../graphics";
			echo '<img src="' . $graphicsPath . '/required.png" alt="required" width=16 height=16 title="' . _('required') . '">';
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
		// overwrite module and scop if needed
		if ($this->module != null) {
			$module = $this->module;
		}
		if ($this->scope != null) {
			$scope = $this->scope;
		}
		// print link
		$helpEntry = getHelp($module, $this->helpID, $scope);
		printHelpLink($helpEntry, $this->helpID, $module, $scope);
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
			$style = ' style="background-image: url(../../graphics/' . $this->value . '); background-color: transparent;"';
		}
		// text button
		elseif ($this->iconClass == null) {
			$classList[] = 'smallPadding';
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
		$type = ' type="submit"';
		$onClick = '';
		if ($this->onClick != null) {
			$type = ' type="button"';
			$onClick = ' onclick="' . $this->onClick . '"';
		}
		$id = ' id="btn_' . preg_replace('/[^a-zA-Z0-9_-]/', '', $this->name) . '"';
		if ($this->isImageButton) {
			echo '<input type="submit" ' . $id . ' value=" "' . $name . $fieldTabIndex . $style . $class . $title . $disabled . '>';
		}
		else {
			echo '<button' . $id . $name . $fieldTabIndex . $type . $onClick . $style . $class . $title . $disabled . '>' . $this->value . '</button>';
			// text buttons get JQuery style
			$icon = '';
			if ($this->iconClass != null) {
				$icon = '{ icons: { primary: \'' . $this->iconClass . '\' } }';
			}
			echo '<script type="text/javascript">';
			echo ' jQuery(document).ready(function() {';
			echo "jQuery('#btn_" . $this->name . "').button(" . $icon . ");";
			echo '});';
			echo '</script>';
		}
		return array($this->name => 'submit');
	}
	
	/**
	 * Sets the button title (tooltip).
	 * 
	 * @param String $title title
	 */
	public function setTitle($title) {
		$this->title = htmlspecialchars($title);
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
	 */
	function __construct($targetModule, $targetPage, $identifier, $value, $isImageButton = false) {
		$this->name = htmlspecialchars('form_subpage_' . $targetModule . '_' . $targetPage . '_' . $identifier);
		$this->value = $value;
		$this->isImageButton = $isImageButton;
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
			$onchange = ' onchange="' . $this->onchangeEvent . '"';
		}
		// hide select boxes that contain less than 2 elements
		if ((sizeof($this->elements) < 2) && !$this->multiSelect && $this->transformSingleSelect) {
			echo '<div class="hidden">';
		}
		// print select box
		echo '<select' . $class . $style . $name . $size . $multi . $disabled . $onchange . ' tabindex="' . $tabindex . "\">\n";
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
		if ((sizeof($this->elements) == 1) && !$this->multiSelect && $this->transformSingleSelect) {
			echo '</div>';
			if ($this->hasDescriptiveElements) {
				$keys = array_keys($this->elements);
				echo $keys[0];
			}
			else {
				echo $this->elements[0];
			}
			echo '&nbsp;';
		}
		elseif (sizeof($this->elements) == 0) {
			echo '</div>';
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
			if ($this->hasDescriptiveElements) {
				if (in_array($value, $this->selectedElements)) {
					$selected = ' selected';
				}
				echo "<option value=\"" . htmlspecialchars($value) . "\"$selected>" . htmlspecialchars($key) . "</option>\n";
			}
			else {
				if (in_array($value, $this->selectedElements)) {
					$selected = ' selected';
				}
				echo "<option$selected>" . htmlspecialchars($value) . "</option>\n";
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
	 * Specifies if the elemets should be sorted (default: sort).
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
		$onchange = '';
		if ($this->onchangeEvent != null) {
			$onchange = ' onchange="' . $this->onchangeEvent . '"';
		}
		// print radio list
		$counter = 0;
		foreach ($this->elements as $label => $value) {
			$onClick = 'onClick="
				jQuery(\'input[name=' . $this->name . ']\').attr(\'checked\', false);
				jQuery(\'#' . $this->name . $counter . '\').attr(\'checked\', true);
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

	/**
	 * Constructor.
	 * 
	 * @param String $string output text
	 * @param boolean $escapeHTML escape HTML code (default yes)
	 */
	function __construct($string, $escapeHTML = true) {
		$this->string = $string;
		$this->escapeHTML = $escapeHTML;
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
		if ($this->noWrap) {
			echo "<div class=\"nowrap\">";
		}
		if ($this->isBold) {
			echo "<b>";
		}
		if ($this->escapeHTML) {
			echo htmlspecialchars($this->string);
		}
		else {
			echo $this->string;
		}
		if ($this->markAsRequired) {
			$graphicsPath = "../../graphics";
			if (is_dir("../graphics")) $graphicsPath = "../graphics";
			echo '<img src="' . $graphicsPath . '/required.png" alt="required" width=16 height=16 title="' . _('required') . '">';
		}
		if ($this->isBold) {
			echo "</b>";
		}
		if ($this->noWrap) {
			echo "</div>";
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
		// build Java script to show/hide depending fields
		$onChange = '';
		$script = '';
		if ((sizeof($this->tableRowsToShow) > 0) || (sizeof($this->tableRowsToHide) > 0)) {
			// build onChange listener
			$onChange = ' onChange="';
			$onChange .= 'if (jQuery(\'#' . $this->name . ':checked\').val() !== undefined) {';
			for ($i = 0; $i < sizeof($this->tableRowsToShow); $i++) {
				$onChange .= 'jQuery(\'#' . $this->tableRowsToShow[$i] . '\').closest(\'tr\').removeClass(\'hidden\');';
			}
			for ($i = 0; $i < sizeof($this->tableRowsToHide); $i++) {
				$onChange .= 'jQuery(\'#' . $this->tableRowsToHide[$i] . '\').closest(\'tr\').addClass(\'hidden\');';
			}
			$onChange .= '}';
			$onChange .= 'else {';
			for ($i = 0; $i < sizeof($this->tableRowsToShow); $i++) {
				$onChange .= 'jQuery(\'#' . $this->tableRowsToShow[$i] . '\').closest(\'tr\').addClass(\'hidden\');';
			}
			for ($i = 0; $i < sizeof($this->tableRowsToHide); $i++) {
				$onChange .= 'jQuery(\'#' . $this->tableRowsToHide[$i] . '\').closest(\'tr\').removeClass(\'hidden\');';
			}
			$onChange .= '};';
			$onChange .= '"';
			// build script to set initial state
			$script = '<script type="text/javascript">jQuery(document).ready(function() {';
			for ($i = 0; $i < sizeof($this->tableRowsToShow); $i++) {
				$classType = 'addClass';
				if ($this->checked) {
					$classType = 'removeClass';
				}
				$script .= 'jQuery(\'#' . $this->tableRowsToShow[$i] . '\').closest(\'tr\').' . $classType . '(\'hidden\');';
			}
			for ($i = 0; $i < sizeof($this->tableRowsToHide); $i++) {
				$classType = 'removeClass';
				if ($this->checked) {
					$classType = 'addClass';
				}
				$script .= 'jQuery(\'#' . $this->tableRowsToHide[$i] . '\').closest(\'tr\').' . $classType . '(\'hidden\');';
			}
			$script .= '});</script>';
		}
		echo '<input type="checkbox" id="' . $this->name . '" name="' . $this->name . '"' . $tabindexValue . $onChange . $checked . $disabled . '>';
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
		$onClick = 'onClick="jQuery(\'#' . $this->name . '\').attr(\'checked\',!jQuery(\'#' . $this->name . '\').attr(\'checked\')); jQuery(\'#' . $this->name . '\').change();"';
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
		echo '<input type="file" name="' . $this->name . '"' . $tabindexValue . $disabled . '>';
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
	/** specifies if LAM should display this field whith a WYSIWYG editor */
	private $richEdit = false;
	
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
		$colCount = ' cols="' . $this->colCount . '"';
		$rowCount = ' rows="' . $this->rowCount . '"';
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
		$classes = ' class="' . implode(' ', $classList) . '"';
		echo '<textarea name="' . $this->name . '" id="' . $this->name . '"' . $tabindexValue . $classes . $colCount . $rowCount . $disabled . '>' . $this->value . '</textarea>';
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
	 * Specifies if the textarea should be displayed whith a WYSIWYG editor.
	 * <br>This requires that the page which displays the textarea also includes the ckeditor JS.
	 * <br>Rich editing is disabled by default.
	 * 
	 * @param boolean $richEdit rich edit or standard
	 */
	public function setIsRichEdit($richEdit) {
		$this->richEdit = $richEdit;
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
	/** required field */
	private $required = false;
	
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
			$graphicsPath = "../../graphics";
			if (is_dir("../graphics")) $graphicsPath = "../graphics";
			echo '<img src="' . $graphicsPath . '/required.png" alt="required" width=16 height=16 title="' . _('required') . '">';
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
	
	/**
	 * Specifies if this input field must be filled.
	 * 
	 * @param boolean $required required or not
	 */
	public function setRequired($required) {
		$this->required = $required;
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
	
	/**
	 * Constructor.
	 * 
	 * @param String $path image location
	 * @param int $width image width (optional, default original size) 
	 * @param int $height image height (optional, default original size)
	 * @param String $alt alt text (optional)
	 */
	function __construct($path, $width = null, $height = null, $alt = ' ') {
		$this->path = htmlspecialchars($path);
		$this->width = $width;
		$this->height = $height;
		$this->alt = htmlspecialchars($alt);
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
		echo '<img' . $path . $width . $height . $alt . ">\n";
		return array();
	}

}

/**
 * Adds an empty space with given width and height.
 *
 * @package metaHTML
 */
class htmlSpacer extends htmlElement {
	
	/** width of spacer in px */
	private $width;
	/** height of spacer in px */
	private $height;
	
	/**
	 * Constructor.
	 *
	 * @param String $width width (e.g. 10px)
	 * @param String $height height (e.g. 10px)
	 */
	function __construct($width, $height) {
		$this->width = htmlspecialchars($width);
		$this->height = htmlspecialchars($height);
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
		if ($this->width != null) {
			$width = 'width: ' . $this->width . ';';
		}
		$height = '';
		if ($this->height != null) {
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
	function __construct($type, $title, $text = null, $params = null) {
		$this->type = $type;
		$this->title = $title;
		$this->text = $text;
		$this->params = $params;
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
		StatusMessage($this->type, $this->title, $this->text, $this->params);
		return array();
	}

}
	
/**
 * Generates a fieldset.
 * 
 * @package metaHTML
 */
class htmlFieldset extends htmlElement {

	/** fieldset content */
	private $content;
	/** descriptive label */
	private $label = null;
	/** label image */
	private $labelImage = null;
	
	/**
	 * Constructor.
	 * 
	 * @param htmlElement $content content to display inside fieldset
	 * @param String $label label
	 * @param String $labelImage image to put before label 
	 */
	function __construct($content, $label = null, $labelImage = null) {
		$this->content = $content;
		$this->label = htmlspecialchars($label);
		$this->labelImage = htmlspecialchars($labelImage);
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
		$class = 'ui-corner-all';
		if ($scope != null) {
			$class .= ' ' . $scope . '-border ' . $scope . '-bright';
		}
		echo "<fieldset class=\"$class\">\n";
		// generate legend
		if (($this->label != null) || ($this->labelImage != null)) {
			echo "<legend>";
			if ($this->labelImage != null) {
				echo "<img align=\"middle\" src=\"" . $this->labelImage . "\" alt=\"\"> ";
			}
			if ($this->label != null) {
				echo $this->label;
			}
			echo "</legend>\n";
		}
		$return = $this->content->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		echo "</fieldset>\n";
		return $return;
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
	
	/**
	 * Constructor.
	 * 
	 * @param String $label label
	 * @param String $image optional image
	 * @param String $id optional ID for this element (e.g. to use for JavaScript)
	 */
	function __construct($label, $image = null, $id = null) {
		$this->label = htmlspecialchars($label);
		$this->image = htmlspecialchars($image);
		$this->id = htmlspecialchars($id);
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
		$idValue = '';
		if ($this->id != null) {
			$idValue = ' id="' . $this->id . '"';
		}
		echo "<div $idValue class=\"subTitle\">\n";
		echo "<h4 class=\"subTitleText\">\n";
		if ($this->image != null) {
			echo '<img src="' . $this->image . '" alt="' . $this->label . '">&nbsp;';
		}
		echo $this->label;
		echo "</h4>\n";
		echo "</div>\n";
		return array();
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
	private $target = null;
	/** optional image */
	private $image = null;
	/** title */
	private $title = null;
	/** target window */
	private $targetWindow = null;
	/** onClick event */
	private $onClick = null;
	/** show as button */
	private $showAsButton = false;
	
	/**
	 * Constructor.
	 * 
	 * @param String $text label
	 * @param String $target target URL
	 * @param String $image URL of optional image
	 * @param boolean $showAsButton shows this like as a button
	 */
	function __construct($text, $target, $image = null, $showAsButton = false) {
		$this->text = htmlspecialchars($text);
		$this->target = htmlspecialchars($target);
		$this->image = htmlspecialchars($image);
		$this->showAsButton = $showAsButton;
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
		$image = '';
		if ($this->image != null) {
			$image = '<img class="align-middle" src="' . $this->image . '" alt="' . $this->text . '">&nbsp;';
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
		$idAttr = '';
		if ($this->showAsButton) {
			$id = 'a_' . preg_replace('/[^a-zA-Z0-9_]+/', '_', $this->target);
			$idAttr = ' id="' . $id . '"';
		}
		$classAttr = '';
		if (sizeof($this->cssClasses) > 0) {
			$classAttr = ' class="' . implode(' ', $this->cssClasses) . '"';
		}
		echo '<a href="' . $this->target . '"' . $idAttr . $classAttr . $title . $targetWindow . $onClick . '>' . $image . $this->text . '</a>';
		if ($this->showAsButton) {
			echo '<script type="text/javascript">';
			echo ' jQuery(document).ready(function() {';
			echo "jQuery('#" . $id . "').button();";
			echo '});';
			echo '</script>';
		}
		return array();
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
	 */
	function __construct($id, $content) {
		$this->id = htmlspecialchars($id);
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
		$idValue = '';
		if ($this->id != null) {
			$idValue = ' id="' . $this->id . '"';
		}
		$classesValue = '';
		if (($this->cssClasses != null) && (sizeof($this->cssClasses) > 0)) {
			$classesValue = ' class="' . implode(' ', $this->cssClasses) . '"';
		}
		echo '<div' . $idValue . $classesValue . '>';
		if ($this->content != null) {
			$return = $this->content->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		}
		echo '</div>';
		return $return;
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
	 * @param array $elements list of element IDs (HTML special chars must be escaped already)
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
			echo $element;
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

?>
