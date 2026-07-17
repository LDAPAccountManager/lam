<?php

namespace LAM\PDF;

use accountContainer;
use Com\Tecnick\Pdf\Tcpdf;
use LAM\ImageUtils\ImageManipulationFactory;
use LAM\TYPES\ConfiguredType;
use LAMException;
use LamTemporaryFilesManager;

/*
  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2004  Michael Duergner
                2003 - 2026  Roland Gruber

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

/** margin */
define('LAMPDF_MARGIN', 10);
/** space between sections */
define('LAMPDF_SPACE_SECTION', 15);
/** space between fields */
define('LAMPDF_SPACE_FIELD', 5);
/** line width */
define('LAMPDF_LINEWIDTH', 190);
/** line height */
define('LAMPDF_LINEHEIGHT', 5);
/** width of a label */
define('LAMPDF_LABELWIDTH', 50);
/** width of an image */
define('LAMPDF_IMAGE_WIDTH', 40);
/** font size */
define('LAMPDF_FONT_SIZE', 7);
/** font size for bigger text */
define('LAMPDF_FONT_SIZE_BIG', 10);

/** access to PDF configuration files */
include_once(__DIR__ . '/pdfstruct.inc');

/**
 * This function creates the PDF output of one or more accounts.
 *
 * @param accountContainer[] $accounts A numbered array containing all accounts the PDF page should
 * be created for. The entries of the array must be AccountContainer objects.
 * @param string $pdf_structure The filename of the structure definition that should be used
 * to create the PDF page. If not submitted the 'default.user' structure definition
 * for the appropriate account type.
 * @param string $font font to use (e.g. DejaVu)
 * @param bool $returnAsString returns the PDF output as String value instead of writing it to a file
 * @return string PDF file name
 * @throws LAMException error creating PDF
 */
function createModulePDF($accounts, $pdf_structure, $font, $returnAsString = false): string {
	$account_type = $accounts[0]->get_type();
	// Get PDF structure from xml file
	$pdfStructurePersistenceManager = new PdfStructurePersistenceManager();
	$structure = $pdfStructurePersistenceManager->readPdfStructure($_SESSION['config']->getName(),
		$account_type->getId(), $pdf_structure);
	// get list of PDF keys
	$pdfKeys = [];
	foreach ($structure->getSections() as $section) {
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
	return createPdf($structure, $accounts, $pdfKeys, $account_type, $font, $returnAsString);
}

/**
 * This function creates the PDF output of one or more accounts using TCPDF.
 *
 * @param PDFStructure $structure PDF structure
 * @param accountContainer[] $accounts A numbered array containing all accounts the PDF page should
 * be created for. The entries of the array must be AccountContainer objects.
 * @param string[] $pdfKeys list of elements in PDF
 * @param ConfiguredType $account_type account type
 * @param string $font font to use
 * @param boolean $returnAsString returns the PDF output as String value instead of writing it to a file
 * @return string PDF file name
 */
function createPdf($structure, $accounts, $pdfKeys, $account_type, $font, $returnAsString): string {
	if (!in_array($font, getPdfFonts())) {
		logNewMessage(LOG_ERR, 'Invalid font specified: ' . $font);
	}
	include_once(__DIR__ . '/3rdParty/composer/autoload.php');
	define('K_PATH_FONTS', __DIR__ . '/3rdParty/composer/tecnickcom/tc-lib-pdf-font/target/fonts');
	$pdf = new Tcpdf();
	$pdf->setTitle($structure->getTitle());
	$pdf->setCreator("LDAP Account Manager");
	$pdf->setAuthor("LDAP Account Manager");
	$normalFont = $pdf->font->insert($pdf->pon, $font, '', LAMPDF_FONT_SIZE);
	$normalFontBold = $pdf->font->insert($pdf->pon, $font, 'B', LAMPDF_FONT_SIZE);
	$bigFontBold = $pdf->font->insert($pdf->pon, $font, 'B', LAMPDF_FONT_SIZE_BIG);

	// Loop over each account and add a new page in the PDF file for it
	foreach ($accounts as $account) {

		// Start a new page for each account
		$pageData = $pdf->addPage();

		// Get PDF entries for the current account
		$entries = $account->get_pdfEntries($pdfKeys, $account_type->getId());

		// Now create the PDF file according to the structure with the submitted values
		foreach ($structure->getSections() as $section) {
			if ($section instanceof PDFTextSection) {
				$pdf->page->addContent($normalFont['out']);
				$info_string = str_replace("\r", "", $section->getText());
				$pdf->addTextCell(
					txt: trim($info_string),
					posx: LAMPDF_MARGIN,
					posy: $pdf->getLastBBox()['y'] + LAMPDF_SPACE_SECTION,
					width: LAMPDF_LINEWIDTH,
					halign: 'L',
					drawcell: false);
			}
			elseif ($section instanceof PDFEntrySection) {
				// section headline
				if ($section->isAttributeTitle()) {
					if (empty($entries[$section->getPdfKey()][0])) {
						continue;
					}
					$section_headline = getSectionHeadline($entries[$section->getPdfKey()][0]);
				}
				else {
					$section_headline = $section->getTitle();
				}
				$pdf->page->addContent($bigFontBold['out']);
				$pdf->addTextCell(
					txt: trim($section_headline),
					posx: LAMPDF_MARGIN,
					posy: $pdf->getLastBBox()['y'] + LAMPDF_SPACE_SECTION,
					width: LAMPDF_LINEWIDTH,
					halign: 'L',
					drawcell: false);
				// entries
				foreach ($section->getEntries() as $entry) {
					// skip non-existent entries
					if (isset($entries[$entry->getKey()])) {
						// Get current entry
						$valueEntries = $entries[$entry->getKey()];
						// Loop over all rows of this entry (most of the time this will be just one)
						foreach ($valueEntries as $valueEntry) {
							if ($valueEntry instanceof PDFLabelValue) {
								printLabelValue($pdf, $valueEntry, $normalFontBold, $normalFont);
							}
							elseif ($valueEntry instanceof PDFTable) {
								printTable($pdf, $valueEntry, $font);
							}
							elseif ($valueEntry instanceof PDFImage) {
								printImage($pdf, $valueEntry, $pageData);
							}
						}
					}
				}
			}
		}
	}

	$out = $pdf->getOutPDFString();
	if (!$returnAsString) {
		$tempFilesManager = new LamTemporaryFilesManager();
		$fileName = $tempFilesManager->registerTemporaryFile('.pdf');
		// Save PDF
		$handle = $tempFilesManager->openTemporaryFileForWrite($fileName);
		fwrite($handle, $out);
		fclose($handle);
		// return PDF file name
		return $fileName;
	}
	else {
		return $out;
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
 * @param Tcpdf $pdf PDF
 * @param PDFLabelValue $valueEntry entry
 * @param string $fontName font name
 */
function printLabelValue($pdf, PDFLabelValue $valueEntry, array $fontLabel, array $fontValue): void {
	$pdf->page->addContent($fontLabel['out']);
	$posY = $pdf->getLastBBox()['y'] + LAMPDF_SPACE_FIELD;
	$pdf->addTextCell(
		txt: replaceSpecialCharacters($valueEntry->getLabel()) . ':',
		posx: LAMPDF_MARGIN,
		posy: $posY,
		width: LAMPDF_LABELWIDTH,
		halign: 'R',
		drawcell: false);
	$pdf->page->addContent($fontValue['out']);
	$pdf->addTextCell(
		txt: replaceSpecialCharacters($valueEntry->getValue()),
		posx: (LAMPDF_MARGIN * 2) + LAMPDF_LABELWIDTH,
		posy: $posY,
		width: LAMPDF_LINEWIDTH - LAMPDF_LABELWIDTH - LAMPDF_MARGIN,
		halign: 'L',
		drawcell: false);
}

/**
 * Prints a PDFTable entry.
 *
 * @param Tcpdf $pdf PDF
 * @param PDFTable $table entry
 * @param string $fontName font name
 */
function printTable(&$pdf, $table, $fontName): void {
	return;
	$headline = $table->getHeadline();
	if (!empty($headline)) {
		$pdf->SetFont($fontName, 'B', LAMPDF_FONT_SIZE);
		$pdf->Cell(LAMPDF_LABELWIDTH, LAMPDF_LINEHEIGHT, replaceSpecialCharacters($headline) . ':',
			0, 0, 'L');
		$pdf->SetFont($fontName, '', LAMPDF_FONT_SIZE);
		$pdf->Ln(LAMPDF_LINEHEIGHT);
	}
	foreach ($table->rows as $row) {
		foreach ($row->cells as $cell) {
			$width = $cell->width;
			if (!empty($width) && (str_contains($width, '%'))) {
				$width = ceil(LAMPDF_LINEWIDTH * ((int) substr($width, 0, -1)) / 100);
			}
			if ($cell->bold) {
				$pdf->SetFont($fontName, 'B', LAMPDF_FONT_SIZE);
			}
			$pdf->Cell($width, LAMPDF_LINEHEIGHT, replaceSpecialCharacters($cell->content), 0,
				0, $cell->align, false, '', 1);
			if ($cell->bold) {
				$pdf->SetFont($fontName, '', LAMPDF_FONT_SIZE);
			}
		}
		$pdf->Ln(LAMPDF_LINEHEIGHT);
	}
	$pdf->Ln(LAMPDF_LINEHEIGHT);
}

/**
 * Prints an image entry.
 *
 * @param Tcpdf $pdf PDF
 * @param PDFImage $imageEntry entry
 */
function printImage($pdf, $imageEntry, $pageData): void {
	include_once __DIR__ . '/imageutils.inc';
	$imageManipulator = ImageManipulationFactory::getImageManipulator($imageEntry->getImageData());
	$imageManipulator->thumbnail(200, 400);
	$imageId = $pdf->image->add('@' . $imageManipulator->getImageData(), 200);
	$height = $imageManipulator->getHeight() / ($imageManipulator->getWidth() / LAMPDF_IMAGE_WIDTH);
	$out = $pdf->image->getSetImage(
		$imageId,
		LAMPDF_LINEWIDTH - LAMPDF_IMAGE_WIDTH,
		$pdf->getLastBBox()['y'], LAMPDF_IMAGE_WIDTH, $height, $pageData['height']);
	$pdf->page->addContent($out);
	$imageManipulator = null;
}

/**
 * Replaces special characters that cannot be printed in PDF.
 * Can be removed once tc-lib-pdf is used.
 *
 * @param string|null $text input text
 * @return string output text
 */
function replaceSpecialCharacters(?string $text): string {
	if (empty($text)) {
		return $text;
	}
	return str_replace('❭', '>', $text);
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
	 * @return string|null label
	 */
	public function getHeadline(): ?string;

}

/**
 * Represents a table for PDF export.
 *
 * @package PDF
 * @author Roland Gruber
 */
class PDFTable implements PDFEntry {

	/** optional label of table */
	private ?string $label;
	/** @var PDFTableRow[] $rows list of PDFTableRow elements */
	public array $rows = [];

	/**
	 * Constructor
	 *
	 * @param string|null $label label
	 */
	public function __construct(?string $label = null) {
		$this->label = $label;
	}

	/**
	 * {@inheritDoc}
	 * @see PDFEntry::getHeadline()
	 */
	public function getHeadline(): ?string {
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

	/** @var PDFTableCell[] $cells list of PDFTableCell */
	public array $cells = [];

}

/**
 * Represents a table cell for PDF export.
 *
 * @package PDF
 * @author Roland Gruber
 */
class PDFTableCell {

	public const ALIGN_LEFT = 'L';
	public const ALIGN_RIGHT = 'R';
	public const ALIGN_CENTER = 'C';

	/** content text of cell */
	public string $content = '';
	/** text alignment */
	public string $align = self::ALIGN_LEFT;
	/** cell width (e.g. "20%") */
	public ?string $width;
	/** bold text */
	public bool $bold = false;

	/**
	 * Constructor.
	 *
	 * @param string|null $content cell content
	 * @param string|null $width width (e.g. "20%")
	 * @param string|null $align cell alignment (default: left)
	 * @param boolean $bold print in bold
	 */
	public function __construct(?string $content, ?string $width = null, ?string $align = null, bool $bold = false) {
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

	private string $label;

	private string $value;

	/**
	 * Constructor
	 *
	 * @param string $label label
	 * @param string $value value
	 */
	public function __construct(string $label, string $value) {
		$this->label = $label;
		$this->value = $value;
	}

	/**
	 * Returns the label.
	 *
	 * @return string $label label
	 */
	public function getLabel(): string {
		return $this->label;
	}

	/**
	 * Returns the value.
	 *
	 * @return string $value value
	 */
	public function getValue(): string {
		return $this->value;
	}

	/**
	 * {@inheritDoc}
	 * @see PDFEntry::getHeadline()
	 */
	public function getHeadline(): ?string {
		return $this->value;
	}

}

/**
 * Adds an image to the PDF file.
 *
 * @package PDF
 * @author Roland Gruber
 */
class PDFImage implements PDFEntry {

	private string $binaryData;

	/**
	 * Constructor
	 *
	 * @param string $binaryData image data
	 */
	public function __construct(string $binaryData) {
		$this->binaryData = $binaryData;
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\PDF\PDFEntry::getHeadline()
	 */
	public function getHeadline(): ?string {
		return '';
	}

	/**
	 * Returns the image data.
	 *
	 * @return string image data
	 */
	public function getImageData(): string {
		return $this->binaryData;
	}

}
