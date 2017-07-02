<?php
namespace LAM\PDF;
/*
  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2004  Michael Duergner
                2003 - 2017  Roland Gruber

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
 * LDAP Account Manager PDF printing library. It consists of lamPDF class,
 * the createModulePDF() function that may be called by other pages
 * and furthermore some helper functions.
 *
 * @author Michael Duergner
 * @author Roland Gruber
 * @package PDF
 */

/** line width */
define('LAMPDF_LINEWIDTH', 190);
/** line height */
define('LAMPDF_LINEHEIGHT', 5);
/** width of a label */
define('LAMPDF_LABELWIDTH', 50);
/** font size */
define('LAMPDF_FONT_SIZE', 7);
/** font size for bigger text */
define('LAMPDF_FONT_SIZE_BIG', 10);

/** access to PDF configuration files */
include_once('pdfstruct.inc');

/**
 * This function creates the PDF output of one or more accounts.
 *
 * @param array $accounts A numbered array containing all accounts the PDF page should
 * be created for. The entries of the array must be AccountContainer objects.
 * @param string $pdf_structure The filename of the structure definition that should be used
 * to create the PDF page. If not submitted the 'default.user' structure definition
 * for the appropriate account type.
 * @param $returnAsString returns the PDF output as String value instead of writing it to a file
 * @return String PDF file name
 */
function createModulePDF($accounts, $pdf_structure="default", $returnAsString = false) {

	/** PDF generator class */
	include_once("fpdf-lam.inc");
	/** Unicode support for FPDF */
	include_once("ufpdf.php");
	/** LAM PDF generator class */
	include_once("lamPDF.inc");

	$account_type = $accounts[0]->get_type();
	// Get PDF structure from xml file
	$reader = new PDFStructureReader();
	$structure = $reader->read($account_type->getId(), $pdf_structure);
	// get list of PDF keys
	$pdfKeys = array();
	foreach($structure->getSections() as $section) {
		if (!$section instanceof PDFEntrySection) {
			continue;
		}
		if ($section->isAttributeTitle()) {
			$pdfKeys[] = $section->getPdfKey();
		}
		foreach ($section->getEntries() as $entry) {
			$pdfKeys[] = $entry->getKey();
		}
	}
	$pdfKeys = array_unique($pdfKeys);

	// The decimal separator must be a dot in order to write pdf-files
	setlocale(LC_NUMERIC, "C");

	$fontName = "Dejavu";

	// Create a new PDF file acording to the account type
	$pdf = new \lamPDF($structure, $fontName);

	// Loop over each account and add a new page in the PDF file for it
	foreach($accounts as $account) {

		// Start a new page for each account
		$pdf->AddPage();

		// Get PDF entries for the current account
		$entries = $account->get_pdfEntries($pdfKeys, $account_type->getId());

		// Now create the PDF file acording to the structure with the submitted values
		foreach ($structure->getSections() as $section) {
			if ($section instanceof PDFTextSection) {
				$pdf->setFont($fontName, "", LAMPDF_FONT_SIZE);
				$info_string = str_replace("\r", "", $section->getText());
				$info_string = explode("\n", $info_string);
				foreach ($info_string as $text) {
					$pdf->MultiCell(0, LAMPDF_LINEHEIGHT, trim($text), 0, "L", 0);
					$pdf->Ln(0);
				}
				$pdf->Ln(LAMPDF_LINEHEIGHT * 2);
			}
			elseif ($section instanceof PDFEntrySection) {
				// section headline
				if($section->isAttributeTitle()) {
					$section_headline = getSectionHeadline($entries[$section->getPdfKey()][0]);
				}
				else {
					$section_headline = $section->getTitle();
				}
				$pdf->setFont($fontName, "B", LAMPDF_FONT_SIZE_BIG);
				$pdf->Write(0,$section_headline . ":");
				$pdf->Ln(LAMPDF_LINEHEIGHT);
				// entries
				foreach ($section->getEntries() as $entry) {
					// skip non-existent entries
					if (isset($entries[$entry->getKey()])) {
						// Get current entry
						$valueEntries = $entries[$entry->getKey()];
						// Print entry only when module sumitted values for it
						if(is_array($valueEntries)) {
							// Loop over all rows of this entry (most of the time this will be just one)
							foreach($valueEntries as $valueEntry) {
								if ($valueEntry instanceof PDFLabelValue) {
									printLabelValue($pdf, $valueEntry, $fontName);
								}
								else if ($valueEntry instanceof PDFTable) {
									printTable($pdf, $valueEntry, $fontName);
								}
							}
						}
					}
				}
				// section end
				$pdf->Ln(LAMPDF_LINEHEIGHT * 2);
			}
		}
	}
	// Close PDF
	$pdf->Close();
	if (!$returnAsString) {
		// use timestamp and random number from ldap.inc as filename so it should be unique.
		$filename = '../../tmp/' . getRandomNumber() . time() .'.pdf';
		// Save PDF
		$pdf->Output($filename);
		chmod($filename, 0640);
		// return PDF file name
		return $filename;
	}
	else {
		return $pdf->Output('', 'S');
	}
}

/**
 * Creates a section headline.
 *
 * @param PDFEntry $entry content entry
 *
 * @return string headline
 */
function getSectionHeadline($entry) {
	return $entry->getHeadline();
}

/**
 * Prints a PDFLabelValue entry.
 *
 * @param lamPDF $pdf PDF
 * @param PDFLabelValue $valueEntry entry
 * @param string $fontName font name
 */
function printLabelValue(&$pdf, $valueEntry, $fontName) {
	$pdf->SetFont($fontName, 'B', LAMPDF_FONT_SIZE);
	$pdf->Cell(LAMPDF_LABELWIDTH, LAMPDF_LINEHEIGHT, $valueEntry->getLabel() . ':', 0, 0, 'R', 0);
	$pdf->SetFont($fontName, '', LAMPDF_FONT_SIZE);
	$pdf->MultiCell(0, LAMPDF_LINEHEIGHT, $valueEntry->getValue(), 0, 'L', 0);
	$pdf->Ln(0);
}

/**
 * Prints a PDFTable entry.
 *
 * @param lamPDF $pdf PDF
 * @param PDFTable $table entry
 * @param string $fontName font name
 */
function printTable(&$pdf, $table, $fontName) {
	$headline = $table->getHeadline();
	if (!empty($headline)) {
		$pdf->SetFont($fontName, 'B', LAMPDF_FONT_SIZE);
		$pdf->Cell(LAMPDF_LABELWIDTH, LAMPDF_LINEHEIGHT, $headline . ':', 0 , 0, 'L', 0);
		$pdf->Ln(LAMPDF_LINEHEIGHT);
	}
	foreach ($table->rows as $row) {
		foreach ($row->cells as $cell) {
			$width = $cell->width;
			if (!empty($width) && (strpos($width, '%') !== false)) {
				$width = ceil(LAMPDF_LINEWIDTH * substr($width, 0, -1) / 100);
			}
			if ($cell->bold) {
				$pdf->SetFont($fontName, 'B', LAMPDF_FONT_SIZE);
			}
			$pdf->Cell($width, LAMPDF_LINEHEIGHT, $cell->content, 0, 0, $cell->align, 0);
			if ($cell->bold) {
				$pdf->SetFont($fontName, '', LAMPDF_FONT_SIZE);
			}
		}
		$pdf->Ln(LAMPDF_LINEHEIGHT);
	}
	$pdf->Ln(LAMPDF_LINEHEIGHT);
}

/**
 * Common interface for all PDF entries.
 *
 * @package PDF
 * @author Roland Gruber
 */
interface PDFEntry {

	/**
	 * Returns the head line of the entry.
	 *
	 * @return string label
	 */
	public function getHeadline();

}

/**
 * Represents a table for PDF export.
 *
 * @package PDF
 * @author Roland Gruber
 */
class PDFTable implements PDFEntry {

	/** optional label of table */
	private $label = '';
	/** list of PDFTableRow elements */
	public $rows = array();

	/**
	 * Constructor
	 *
	 * @param String $label label
	 */
	public function __construct($label = null) {
		$this->label = $label;
	}

	/**
	 * {@inheritDoc}
	 * @see PDFEntry::getHeadline()
	 */
	public function getHeadline() {
		return $this->label;
	}

}

/**
 * Represents a table row for PDF export.
 *
 * @package PDF
 * @author Roland Gruber
 */
class PDFTableRow {

	/** list of PDFTableCell */
	public $cells = array();

}

/**
 * Represents a table cell for PDF export.
 *
 * @package PDF
 * @author Roland Gruber
 */
class PDFTableCell {

	const ALIGN_LEFT = 'L';
	const ALIGN_RIGHT = 'R';
	const ALIGN_CENTER = 'C';

	/** content text of cell */
	public $content = '';
	/** text alignment */
	public $align = self::ALIGN_LEFT;
	/** cell width (e.g. "20%") */
	public $width = null;
	/** bold text */
	public $bold = false;

	/**
	 * Constructor.
	 *
	 * @param String $content cell content
	 * @param String $width width (e.g. "20%")
	 * @param String $align cell alignment (default: left)
	 * @param boolean $bold print in bold
	 */
	public function __construct($content, $width = null, $align = null, $bold = false) {
		$this->content = empty($content) ? ' ' : $content;
		$this->align = ($align == null) ? self::ALIGN_LEFT : $align;
		$this->width = $width;
		$this->bold = $bold;
	}

}

/**
 * Simple PDF object to print label value entries.
 *
 * @package PDF
 * @author Roland Gruber
 */
class PDFLabelValue implements PDFEntry {

	private $label = '';

	private $value = '';

	/**
	 * Constructor
	 *
	 * @param string $label label
	 * @param string $value value
	 */
	public function __construct($label, $value) {
		$this->label = $label;
		$this->value = $value;
	}

	/**
	 * Returns the label.
	 *
	 * @return string $label label
	 */
	public function getLabel() {
		return $this->label;
	}

	/**
	 * Returns the value.
	 *
	 * @return string $value value
	 */
	public function getValue() {
		return $this->value;
	}

	/**
	 * {@inheritDoc}
	 * @see PDFEntry::getHeadline()
	 */
	public function getHeadline() {
		return $this->value;
	}

}
