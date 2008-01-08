<?PHP
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003 - 2006  Michael Duergner

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
 * @package PDF
 */

/**
 * Creates a LAM information page in PDF format.
 * 
 * @package PDF
 * @author Michael Duergner
 */
class lamPDF extends UFPDF {
	
	/**
	 * format settings for page layout
	 */
	private $page_definitions;
	
	/**
	* current active font name
	*/
	private $fontName;
	
	/**
	* list of supported fonts
	* format: <font name> => array(<file for default font style>, <file for bold>, <italic>, <bold italic>)
	*/
	private $fontList = array(
		'BitstreamVeraSans-Roman' => array('vera.php', 'verab.php', 'verabi.php', 'verai.php')
	);

	/**
	 * 
	 * 
	 * @param string $account_type
	 * @param array $page_definitions
	 */
	function __construct($page_definitions = array(),$fontName) {
		$this->fontName = $fontName;
		define('FPDF_FONTPATH', $_SESSION['lampath'] . "lib/" . 'font/');
		// Call constructor of superclass
		$this->FPDF('P','mm','A4');
		
		$this->page_definitions = $page_definitions;
		
		// Open PDF file and write some basic information
		$this->Open();
		$this->AddFont($this->fontName, '', $this->fontList[$this->fontName][0]);
		$this->AddFont($this->fontName, 'B', $this->fontList[$this->fontName][1]);
		$this->AddFont($this->fontName, 'I', $this->fontList[$this->fontName][2]);
		$this->AddFont($this->fontName, 'BI', $this->fontList[$this->fontName][3]);
		$this->setFont($this->fontName,"",12);
		$this->setTitle($this->page_definitions['headline']);
		$this->setCreator("LDAP Account Manager");
		$this->setMargins($this->page_definitions['margin-left'],$this->page_definitions['margin-top'],$this->page_definitions['margin-right']);
		$this->setAutoPageBreak(true,$this->page_definitions['margin-bottom']);
	}
	
	/**
	 * 
	 */
	function header() {
		if($this->page_definitions['filename'] != 'none') {
			$imageFile = substr(__FILE__,0,strlen(__FILE__)- 14) . "config/pdf/logos/" . $this->page_definitions['filename'];
			$width = $this->page_definitions['logo-width'];
			$height = $this->page_definitions['logo-height'];
			if($this->page_definitions['logo-max'] == true) {
				if(($width / $height) <= 2.5) {
					$factor = 20 / $height;
					$width = $factor * $width;
					$height = 20;
				}
				else {
					$factor = 50 / $width;
					$height = $factor * $height;
					$width = 50;
				}
			}
			$this->Image($imageFile,10,10,$width,$height,"JPG");
		}
		$this->SetY(18);
		$this->SetFont($this->fontName,"B",18);
		$this->Cell(170,5,$this->page_definitions['headline'],0,1,"R",0);
		$this->Ln(3);
		$this->SetLineWidth(0.4);
		$this->Line(10,$this->page_definitions['margin-top'] + 30,200,$this->page_definitions['margin-top'] + 30);
		$this->Line(10,$this->page_definitions['margin-top'] + 32,200,$this->page_definitions['margin-top'] + 32);
		$this->SetY(50);
	}

	/**
	 * 
	 */
	function footer() {
		$this->SetLineWidth(0.4);
		$this->Line(10,280,200,280);
		$this->Line(10,282,200,282);
		$this->SetY(286);
		$this->SetFont($this->fontName,"",7);
		$this->Cell(0,5,_("This document was automatically created by LDAP Account Manager"),0,0,"C",0);
	}
}

?>