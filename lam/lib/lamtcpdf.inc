<?php

namespace LAM\PDF;

use LAM\ImageUtils\ImageManipulationFactory;
use TCPDF;

/*
  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2017 - 2024  Roland Gruber

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
 * LDAP Account Manager PDF printing library for TCPDF.
 *
 * @author Roland Gruber
 * @package PDF
 */

/** PDF generator class */
include_once(__DIR__ . "/3rdParty/tcpdf/tcpdf.php");
/** image manipulation */
include_once(__DIR__ . "/imageutils.inc");

class LAMTCPDF extends TCPDF {

	/**
	 * @var PDFStructure structure
	 */
	private $structure;

	/**
	 * @var string font name
	 */
	private $fontName;

	/**
	 * Constructor.
	 *
	 * @param PDFStructure $structure PDF structure
	 * @param string $fontName font name
	 */
	public function __construct($structure, $fontName) {
		TCPDF::__construct('P', 'mm', 'A4', true, 'UTF-8', false);
		$this->structure = $structure;
		$this->fontName = $fontName;
		$this->SetAutoPageBreak(true, PDF_MARGIN_BOTTOM);
		$this->SetMargins(10, 35, 10);
		// generic PDF information
		$this->setTitle($this->structure->getTitle());
		$this->setCreator("LDAP Account Manager");
		$this->setAuthor("LDAP Account Manager");
		// footer
		$this->setPrintFooter(true);
	}

	/**
	 * {@inheritDoc}
	 * @see TCPDF::Header()
	 */
	public function Header() {
		$logoFile = $this->structure->getLogo();
		$pdfStructurePersistenceManager = new PdfStructurePersistenceManager();
		if (!empty($logoFile) && ($logoFile !== 'none')) {
			$logoBinary = $pdfStructurePersistenceManager->getPdfLogoBinary($_SESSION['config']->getName(), $logoFile);
			if (!empty($logoBinary)) {
				$imageConverter = ImageManipulationFactory::getImageManipulator($logoBinary);
				$imageConverter->convertToJpeg();
				$this->Image('@' . $imageConverter->getImageData(), 10, 10, '', 15, 'JPG', '', 'T', false, 300, '', false, false, 0, false, false, false);
			}
			else {
				logNewMessage(LOG_ERR, 'Unable to read PDF logo ' . $logoFile);
				$this->SetY(10);
			}
		}
		else {
			$this->SetY(10);
		}
		$this->SetFont($this->fontName, '', 20);
		$this->Cell(0, 15, $this->structure->getTitle(), 0, true, 'R', 0, '', 0, false, 'M', 'M');
		//set folding marks
		if ($this->structure->getFoldingMarks() == PDFStructure::FOLDING_STANDARD) {
			$this->SetLineWidth(0.2);
			$foldingMarks = [97, 202];
			foreach ($foldingMarks as $mark) {
				$this->Line(0, $mark, 5, $mark);
			}
		}
	}

	/**
	 * {@inheritDoc}
	 * @see TCPDF::Footer()
	 */
	public function Footer() {
		$this->SetY(-15);
		$this->SetFont($this->fontName, '', LAMPDF_FONT_SIZE);
		$footerText = _("This document was automatically created by LDAP Account Manager") . ' (' . getFormattedTime('Y-m-d H:i:s T') . ')';
		$this->Cell(0, 10, $footerText, 0, false, 'C', 0, '', 0, false, 'T', 'M');
	}

}
