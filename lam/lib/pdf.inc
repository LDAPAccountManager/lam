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
/** space between title and first field */
define('LAMPDF_SPACE_TITLE_FIELD', 2);
/** space between field label and value */
define('LAMPDF_SPACE_LABEL_VALUE', 1);
/** line width */
define('LAMPDF_Y_MAX', 297 - (2 * LAMPDF_MARGIN));
/** line width */
define('LAMPDF_LINEWIDTH', 190);
/** line height */
define('LAMPDF_LINEHEIGHT', 5);
/** width of a label */
define('LAMPDF_LABELWIDTH', 50);
/** width of an image */
define('LAMPDF_IMAGE_WIDTH', 40);
/** height of logo image */
define('LAMPDF_LOGO_HEIGHT', 15);
/** font size */
define('LAMPDF_FONT_SIZE', 7);
/** font size for bigger text */
define('LAMPDF_FONT_SIZE_BIG', 10);
/** font size for page title text */
define('LAMPDF_FONT_SIZE_TITLE', 20);
/** font location */
define('K_PATH_FONTS', __DIR__ . '/3rdParty/fonts');

/** access to PDF configuration files */
include_once(__DIR__ . '/pdfstruct.inc');
/** composer */
include_once(__DIR__ . '/3rdParty/composer/autoload.php');
/** image functions */
include_once __DIR__ . '/imageutils.inc';

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
function createModulePDF(array $accounts, string $pdf_structure, string $font, bool $returnAsString = false): string {
	$account_type = $accounts[0]->get_type();
	$pdfStructurePersistenceManager = new PdfStructurePersistenceManager();
	$structure = $pdfStructurePersistenceManager->readPdfStructure($_SESSION['config']->getName(),
		$account_type->getId(), $pdf_structure);
	// get the list of PDF keys
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
 * @param accountContainer[] $accounts A numbered array containing all accounts for the PDF page.
 * The entries of the array must be AccountContainer objects.
 * @param string[] $pdfKeys list of elements in PDF
 * @param ConfiguredType $account_type account type
 * @param string $font font to use
 * @param bool $returnAsString returns the PDF output as a string value instead of writing it to a file
 * @return string PDF file name
 */
function createPdf(PDFStructure $structure, array $accounts, array $pdfKeys, ConfiguredType $account_type, string $font, bool $returnAsString): string {
	if (!in_array($font, getPdfFonts())) {
		logNewMessage(LOG_ERR, 'Invalid font specified: ' . $font);
	}
	$pdf = new Tcpdf();
	$pdf->setTitle($structure->getTitle());
	$pdf->setCreator("LDAP Account Manager");
	$pdf->setAuthor("LDAP Account Manager");

	// Loop over each account and add a new page in the PDF file for it
	foreach ($accounts as $account) {

		// Start a new page for each account
		$pageData = $pdf->addPage();
		$extraTopMargin = 0;
		addPageHeader($pdf, $structure, $font, $pageData, $extraTopMargin);

		// Get PDF entries for the current account
		$entries = $account->get_pdfEntries($pdfKeys, $account_type->getId());

		// Now create the PDF file according to the structure with the submitted values
		foreach ($structure->getSections() as $section) {
			if ($section instanceof PDFTextSection) {
				changeFont($pdf, $font);
				$info_string = str_replace("\r", "", $section->getText());
				$pdf->addTextCell(
					txt: trim($info_string),
					posx: LAMPDF_MARGIN,
					posy: getYWithoutOverflow($pdf->getLastBBox()['y'] + LAMPDF_SPACE_SECTION + $extraTopMargin, $pdf),
					width: LAMPDF_LINEWIDTH,
					halign: 'L',
					drawcell: false);
				$extraTopMargin = 0;
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
				changeFont($pdf, $font, LAMPDF_FONT_SIZE_BIG);
				$pdf->addTextCell(
					txt: trim($section_headline),
					posx: LAMPDF_MARGIN,
					posy: getYWithoutOverflow($pdf->getLastBBox()['y'] + LAMPDF_SPACE_SECTION + $extraTopMargin, $pdf),
					width: LAMPDF_LINEWIDTH,
					halign: 'L',
					drawcell: false);
				$extraTopMargin = LAMPDF_SPACE_TITLE_FIELD;
				// entries
				foreach ($section->getEntries() as $entry) {
					// skip non-existent entries
					if (!isset($entries[$entry->getKey()])) {
						continue;
					}
					$valueEntries = $entries[$entry->getKey()];
					// loop over all rows of this entry (most of the time this will be just one)
					foreach ($valueEntries as $valueEntry) {
						if ($valueEntry instanceof PDFLabelValue) {
							printLabelValue($pdf, $valueEntry, $font, $extraTopMargin);
							$extraTopMargin = 0;
						}
						elseif ($valueEntry instanceof PDFTable) {
							printTable($pdf, $valueEntry, $font);
							$extraTopMargin = 0;
						}
						elseif ($valueEntry instanceof PDFImage) {
							printImage($pdf, $valueEntry, $pageData);
						}
					}
				}
				$extraTopMargin = 0;
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
 * Adds the page header.
 *
 * @param Tcpdf $pdf PDF object
 * @param PDFStructure $structure PDF structure
 * @param string $fontName font name
 * @param array<string, string|int|bool> $pageData page data
 * @param int $extraTopMargin y-position on PDF page
 */
function addPageHeader(Tcpdf $pdf, PDFStructure $structure, string $fontName, array $pageData, int &$extraTopMargin): void {
	// folding marks
	if ($structure->getFoldingMarks() === PDFStructure::FOLDING_STANDARD) {
		$lineStyle = [
			'lineWidth' => 0.2,
		];
		$foldingMarks = [97, 202];
		foreach ($foldingMarks as $mark) {
			$marksOut = $pdf->graph->getLine(0, $mark, 3, $mark, $lineStyle);
			$pdf->page->addContent($marksOut);
		}
	}
	// title
	changeFont($pdf, $fontName, LAMPDF_FONT_SIZE_TITLE);
	$pdf->addTextCell(
		txt: replaceSpecialCharacters($structure->getTitle()),
		posx: LAMPDF_MARGIN,
		posy: LAMPDF_MARGIN,
		width: LAMPDF_LINEWIDTH,
		halign: 'R',
		drawcell: false);
	// logo
	$logoFile = $structure->getLogo();
	if (!empty($logoFile) && ($logoFile !== PDFStructure::NO_LOGO)) {
		$pdfStructurePersistenceManager = new PdfStructurePersistenceManager();
		$logoBinary = $pdfStructurePersistenceManager->getPdfLogoBinary($_SESSION['config']->getName(), $logoFile);
		if (!empty($logoBinary)) {
			$imageConverter = ImageManipulationFactory::getImageManipulator($logoBinary);
			$imageConverter->convertToJpeg();
			$imageId = $pdf->image->add('@' . $imageConverter->getImageData(), 200);
			$width = $imageConverter->getWidth() / ($imageConverter->getHeight() / LAMPDF_LOGO_HEIGHT);
			$imageOut = $pdf->image->getSetImage(
				$imageId,
				LAMPDF_MARGIN,
				LAMPDF_MARGIN,
				$width,
				LAMPDF_LOGO_HEIGHT,
				$pageData['height']);
			$pdf->page->addContent($imageOut);
			$extraTopMargin = LAMPDF_MARGIN;
		}
		else {
			logNewMessage(LOG_ERR, 'Unable to read PDF logo ' . $logoFile);
		}
	}
}

/**
 * Changes the current font in the PDF.
 *
 * @param Tcpdf $pdf PDF
 * @param string $fontName font name
 * @param float $fontSize size
 * @param string $style style (e.g. 'B')
 */
function changeFont(Tcpdf $pdf, string $fontName, float $fontSize = LAMPDF_FONT_SIZE, string $style = ''): void {
	$font = $pdf->font->insert($pdf->pon, $fontName, $style, $fontSize);
	$pdf->page->addContent($font['out']);
}

/**
 * Creates a section headline.
 *
 * @param PDFEntry $entry content entry
 *
 * @return string headline
 */
function getSectionHeadline(PDFEntry $entry): string {
	return $entry->getHeadline() ?? '';
}

/**
 * Prints a PDFLabelValue entry.
 *
 * @param Tcpdf $pdf PDF
 * @param PDFLabelValue $valueEntry entry
 * @param string $fontName font name
 * @param int $extraTopMargin extra top margin
 */
function printLabelValue(Tcpdf $pdf, PDFLabelValue $valueEntry, string $fontName, int $extraTopMargin): void {
	changeFont($pdf, $fontName);
	$posY = getYWithoutOverflow($pdf->getLastBBox()['y'] + LAMPDF_SPACE_FIELD + $extraTopMargin, $pdf);
	$pdf->addTextCell(
		txt: replaceSpecialCharacters($valueEntry->getLabel()) . ':',
		posx: LAMPDF_MARGIN,
		posy: $posY,
		width: LAMPDF_LABELWIDTH,
		halign: 'R',
		drawcell: false);
	$pdf->addTextCell(
		txt: replaceSpecialCharacters($valueEntry->getValue()),
		posx: LAMPDF_MARGIN + LAMPDF_LABELWIDTH + LAMPDF_SPACE_LABEL_VALUE,
		posy: $posY,
		width: LAMPDF_LINEWIDTH - LAMPDF_LABELWIDTH - LAMPDF_SPACE_LABEL_VALUE,
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
function printTable(Tcpdf $pdf, PDFTable $table, string $fontName): void {
	$headline = $table->getHeadline();
	$extraTopMargin = LAMPDF_SPACE_TITLE_FIELD;
	if (!empty($headline)) {
		changeFont($pdf, $fontName);
		$pdf->addTextCell(
			txt: replaceSpecialCharacters($headline) . ':',
			posx: LAMPDF_MARGIN,
			posy: getYWithoutOverflow($pdf->getLastBBox()['y'] + LAMPDF_SPACE_FIELD + $extraTopMargin, $pdf),
			width: LAMPDF_LINEWIDTH,
			halign: 'L',
			drawcell: false);
		$extraTopMargin = LAMPDF_SPACE_TITLE_FIELD;
	}
	changeFont($pdf, $fontName);
	foreach ($table->rows as $row) {
		$posY = getYWithoutOverflow($pdf->getLastBBox()['y'] + LAMPDF_SPACE_FIELD + $extraTopMargin, $pdf);
		$posX = LAMPDF_MARGIN;
		foreach ($row->cells as $cell) {
			$width = $cell->width ?? LAMPDF_LINEWIDTH;
			if (!empty($width) && (str_contains($width, '%'))) {
				$width = ceil(LAMPDF_LINEWIDTH * ((int) substr($width, 0, -1)) / 100);
			}
			if ($cell->bold) {
				changeFont($pdf, $fontName, LAMPDF_FONT_SIZE, 'B');
			}
			$pdf->addTextCell(
				txt: replaceSpecialCharacters($cell->content),
				posx: $posX,
				posy: $posY,
				width: $width,
				height: LAMPDF_FONT_SIZE,
				halign: $cell->align,
				drawcell: false,
				fit: 'F');
			changeFont($pdf, $fontName);
			$posX += $width;
		}
		$extraTopMargin = 0;
	}
}

/**
 * Prints an image entry.
 *
 * @param Tcpdf $pdf PDF
 * @param PDFImage $imageEntry entry
 * @param array<string, string|int|bool> $pageData page data
 */
function printImage(Tcpdf $pdf, PDFImage $imageEntry, array $pageData): void {
	$imageManipulator = ImageManipulationFactory::getImageManipulator($imageEntry->getImageData());
	$imageManipulator->thumbnail(200, 400);
	$imageId = $pdf->image->add('@' . $imageManipulator->getImageData(), 200);
	$height = $imageManipulator->getHeight() / ($imageManipulator->getWidth() / LAMPDF_IMAGE_WIDTH);
	$out = $pdf->image->getSetImage(
		$imageId,
		LAMPDF_MARGIN + LAMPDF_LINEWIDTH - LAMPDF_IMAGE_WIDTH,
		getYWithoutOverflow($pdf->getLastBBox()['y'], $pdf),
		LAMPDF_IMAGE_WIDTH,
		$height,
		$pageData['height']);
	$pdf->page->addContent($out);
}

/**
 * Returns the y-position to use for new cells
 *
 * @param float|int $posY calculated y-position of cell
 * @return float position to use
 */
function getYWithoutOverflow(float|int $posY, Tcpdf $pdf): float {
	if ($posY > LAMPDF_Y_MAX) {
		$pdf->addPage();
		return LAMPDF_MARGIN;
	}
	return $posY;
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
	public string $content;
	/** text alignment */
	public string $align;
	/** cell width (e.g. "20%") */
	public ?string $width;
	/** bold text */
	public bool $bold;

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
		$this->align = $align ?? self::ALIGN_LEFT;
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
