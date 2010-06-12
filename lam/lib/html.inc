<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2010  Roland Gruber

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
interface htmlElement {
	
	const ALIGN_TOP = 0;
	const ALIGN_LEFT = 1;
	const ALIGN_RIGHT = 2;
	const ALIGN_BOTTOM = 3;

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
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope);

}

/**
 * Structures elements using a table.
 *
 * @package metaHTML
 */
class htmlTable implements htmlElement {

	/** table header */
	const header = "<table>\n<tr>\n";
	/** table footer */
	const footer = "</tr>\n</table>\n";
	/** new line */
	const newLine = "</tr><tr>\n";
	
	private $elements = array();

	/**
	 * Adds an element to the table. The element may be a htmlElement object or a simple String.
	 *
	 * @param mixed $element htmlElement object or a simple String
	 * @param boolean $newLine adds a new line after the element (optional, default false)
	 */
	public function addElement($element, $newLine = false) {
		if ($element instanceof htmlElement) {
			$this->elements[] = "<td>\n";
			$this->elements[] = $element;
			$this->elements[] = "</td>\n";
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
		$this->elements[] = htmlTable::newLine;
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
		echo htmlTable::header;
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
		echo htmlTable::footer;
		return $return;
	}

}

/**
 * An extended input field that combines label, input field and help.
 * 
 * @package metaHTML
 */
class htmlTableExtendedInputField implements htmlElement {

	/** Descriptive label */
	private $label;
	/** unique field name */
	private $fieldName;
	/** field value */
	private $fieldValue;
	/** field size (default 30) */
	private $fieldSize = 30;
	/** field max length (default 255) */
	private $fieldMaxLength = 255;
	/** help ID */
	private $helpID;
	/** required field */
	private $required = false;
	
	/**
	 * Constructor
	 *
	 * @param String $label descriptive label
	 * @param String $fieldName unique field name
	 * @param String $fieldValue value of input field (optional)
	 * @param String $helpID help ID (optional)
	 */
	function __construct($label, $fieldName, $fieldValue = null, $helpID = null) {
		$this->label = htmlspecialchars($label);
		$this->fieldName = htmlspecialchars($fieldName);
		$this->fieldValue = htmlspecialchars($fieldValue);
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
		if (isset($values[$this->fieldName])) {
			$this->fieldValue = $values[$this->fieldName][0];
		}
		// print label text
		echo $this->label;
		if ($this->required) {
			echo '*';
		}
		echo "\n</td>\n<td>\n";
		// print input field
		$name = ' name="' . $this->fieldName . '"';
		$value = '';
		if ($this->fieldValue != null) {
			$value = ' value="' . $this->fieldValue . '"';
		}
		$maxLength = '';
		if ($this->fieldMaxLength != null) {
			$maxLength = ' maxlength="' . $this->fieldMaxLength . '"';
		}
		$size = ' size="' . $this->fieldSize . '"';
		$fieldTabIndex = ' tabindex="' . $tabindex . '"';
		$tabindex++;
		echo '<input type="text"' . $name . $value . $maxLength . $size . $fieldTabIndex . '>';
		// print help link
		if ($this->helpID != null) {
			echo "\n</td>\n<td>\n";
			$helpLink = new htmlHelpLink($this->helpID);
			$helpLink->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		}
		return array($this->fieldName => 'text');
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
	 * Specifies if this input field must be filled.
	 * 
	 * @param boolean $required required or not
	 */
	public function setRequired($required) {
		$this->required = $required;
	}

}

/**
 * Renders a help link.
 * 
 * @package metaHTML
 */
class htmlHelpLink implements htmlElement {
	
	/** help ID */
	private $helpID;
	
	/**
	 * Constructor
	 *
	 * @param String $helpID help ID
	 */
	function __construct($helpID) {
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
class htmlButton implements htmlElement {
	
	/** button name */
	protected $name;
	/** button text or image */
	protected $value;
	/** image button or text button */
	protected $isImageButton;
	
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
		$value = '';
		$style = '';
		$class = '';
		$name = ' name="' . $this->name . '"';
		// image button
		if ($this->isImageButton) {
			$value = ' value=" "';
			$class = ' class="smallImageButton"';
			$style = ' style="background-image: url(../../graphics/' . $this->value . ');"';
		}
		// text button
		else {
			if ($this->value != null) {
				$value = ' value="' . $this->value . '"';
			}
		}
		echo '<input type="submit"' . $name . $value . $style . $class . '>';
		return array($this->name => 'submit');
	}
	
}

/**
 * Prints a button for the account pages.
 *
 * @package metaHTML
 */
class htmlAccountPageButton extends htmlButton implements htmlElement {
	
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
class htmlSelect implements htmlElement {
	
	/** name of select field */
	private $name;
	/** size */
	private $size;
	/** allows multi-selection */
	private $multiSelect = false;
	/** elements */
	private $elements;
	/** selected elements */
	private $selectedElements;
	/** descriptive elements */
	private $hasDescriptiveElements = false;
	/** sorting enabled */
	private $sortElements = true;
	
	/**
	 * Constructor.
	 *
	 * @param String $name element name
	 * @param array $elements list of elememts
	 * @param array $selectedElements list of selected elements
	 * @param int $size size (optional, default = 1)
	 */
	function __construct($name, $elements, $selectedElements, $size = 1) {
		$this->name = htmlspecialchars($name);
		$this->elements = $elements;
		$this->selectedElements = $selectedElements;
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
		if (isset($values[$this->name])) {
			$this->selectedElements = $values[$this->name];
		}
		$multi = '';
		if ($this->multiSelect) {
			$multi = ' multiple';
			$this->name = $this->name . '[]';
		}
		$name = ' name="' . $this->name . '"';
		$size = ' size="' . $this->size . '"';
		echo '<select' . $name . $size . $multi . ' tabindex="' . $tabindex . "\">\n";
		$tabindex++;
		// sorting
		if ($this->sortElements) {
			if ($this->hasDescriptiveElements) {
				$labels = array_keys($this->elements);
				natcasesort($labels);
				$newElements = array();
				foreach ($labels as $label) {
					$newElements[$label] = $this->elements[$label];
				}
				$this->elements = $newElements;
			}
			else {
				natcasesort($this->elements);
			}
		}
		foreach ($this->elements as $key => $value) {
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
		echo "</select>\n";
		if ($this->multiSelect) {
			return array($this->name => 'multiselect');
		}
		else {
			return array($this->name => 'select');
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
		echo $this->label;
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
 * Prints the text and escapes contained HTML code by default.
 * 
 * @package metaHTML
 */
class htmlOutputText implements htmlElement {

	/** the text to print */
	private $string;
	/** specifies if HTML code should be escaped */
	private $escapeHTML;

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
		if ($this->escapeHTML) {
			echo htmlspecialchars($this->string);
		}
		else {
			echo $this->string;
		}
		return array();
	}

}

/**
 * Prints the HTML code for a checkbox.
 * 
 * @package metaHTML
 */
class htmlInputCheckbox implements htmlElement {
	
	/** unique name of input element */
	private $name;
	/** value */
	private $checked;
	
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
		echo '<input type="checkbox" name="' . $this->name . '"' . $tabindexValue . $checked . '>';
		return array($this->name => 'checkbox');
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
		if ($this->labelFirst) {
			echo $this->label;
			echo "\n</td>\n<td>\n";
			$return = parent::generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		}
		else {
			$return = parent::generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
			echo "\n</td>\n<td>\n";
			echo $this->label;
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
 * Prints the HTML code for a textarea.
 * 
 * @package metaHTML
 */
class htmlInputTextarea implements htmlElement {
	
	/** unique name of input element */
	private $name;
	/** value */
	private $value;
	/** column count */
	private $colCount;
	/** row count */
	private $rowCount;
	
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
		$this->colCount = $colCount;
		$this->rowCount = $rowCount;
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
			$this->value = implode("\r\n", $values[$this->name]);
		}
		$colCount = ' cols="' . $this->colCount . '"';
		$rowCount = ' rows="' . $this->rowCount . '"';
		$tabindexValue = ' tabindex="' . $tabindex . '"';
		$tabindex++;
		echo '<textarea name="' . $this->name . '"' . $tabindexValue . $colCount . $rowCount . '>' . $this->value . '</textarea>';
		return array($this->name => 'textarea');
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
	function __construct($name, $value, $colCount, $rowCount, $label, $helpID) {
		parent::__construct($name, $value, $colCount, $rowCount);
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
		echo $this->label;
		if ($this->required) {
			echo '*';
		}
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
class htmlImage implements htmlElement {
	
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

?>