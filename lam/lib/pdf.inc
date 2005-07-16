<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Michael Dürgner

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
 * the createModulePDF() and getAvailablePDFStructure() functions that may
 * be called by other pages and furthermore some helper functions.
 * 
 * @author Michael Dürgner
 * @package PDF
 * @copyright Copyright (C) 2003-2004 Michael Dürgner
 * @license GPL
 */

/**
 * 
 */
define('LAMPDF_LINEWIDTH',190);

/**
 * 
 */
include_once("fpdf.php");

/**
 * 
 */
include_once("ufpdf.php");

/**
 * 
 */
include_once('xml_parser.inc');

$key = false;
$line_width = LAMPDF_LINEWIDTH;

/**
 * This function will return all available PDF structure definitions for the submitted
 * account scope.
 * 
 * @param string The account scope the PDF structure definitions should be
 * returned.
 * 
 * @return array All available PDF structure definitions for the submitted account
 * scope. Each entry is a string being the filename that may be passed to the 
 * createModulePDF() function as second argument.
 */
function getAvailablePDFStructures($scope='user') {
	$return = array();
	$dirHandle = opendir($_SESSION['lampath'] . '/config/pdf/' . $scope . '/');
	while($file = readdir($dirHandle)) {
		if(!is_dir($file) && $file != '.' && $file != '..') {
			array_push($return,$file);
		}
	}
	sort($return);
	return $return;
}

/**
 * This function creates the PDF output of one or more accounts. At the moment
 * this function can create a PDF page for user, group and host accounts. But
 * this is not limited by the function itself but by the account types that are
 * allowed in LAM and the exsisting PDF structure definitions. 
 * 
 * @param array A numbered array containing all accounts the PDF page should
 * be created for. The entries of the array must be AccountContainer objects.
 * @param string The filename of the structure definition that should be used
 * to create the PDF page. If not submitted the 'default.xml' structure definition
 * for the appropriate account type.
 */
function createModulePDF($accounts,$pdf_structure="default.xml") {
	
	global $key;
	
	// Get account type from account container if none was specified or
	// if it is different to the submitted or previous stored
	if($account_type == "" || $account_type != $accounts[0]->get_type()) {
		$account_type = $accounts[0]->get_type();
	}
	
	// Get PDF structure from xml file
	$load = getStructure($account_type,$pdf_structure);
	$structure = $load['structure'];
	
	// The decimal separator must be a dot in order to write pdf-files
	setlocale(LC_NUMERIC, "C");
	
	$fontName = "BitstreamVeraSans-Roman"; // TODO: load font name from XML file
	
	// Create a new PDF file acording to the account type
	$pdf = new LamPDF($account_type,$load['page_definitions'],$fontName);
	
	// Loop over each account and add a new page in the PDF file for it 
	foreach($accounts as $account) {
		
		// Start a new page for each account
		$pdf->AddPage();
		
		// Get PDF entries for the current account
		$entries = $account->get_pdfEntries($account_type);
		
		// Now create the PDF file acording to the structure with the submitted values 
		foreach($structure as $entry) {
			// We have a new section to start
			if($entry['tag'] == "SECTION" && $entry['type'] == "open") {
				$name = $entry['attributes']['NAME'];
				if(preg_match("/^\_[a-zA-Z\_]+/",$name)) {
					$section_headline = getSectionHeadline($entries[substr($name,1)][0]);
				}
				else {
					$section_headline = $name;
				}
				$pdf->setFont($fontName,"B",12);
				$pdf->Write(5,$section_headline . ":");
				$pdf->Ln(6);
			}
			// We have a section to end
			elseif($entry['tag'] == "SECTION" && $entry['type'] == "close") {
				$pdf->Ln(9);
			}
			// We have to include a static text.
			elseif($entry['tag'] == "TEXT") {
				// Load PDF text from structure array
				$info_string = $entry['value'];
				// Get all allowed vairables from account-object
				$values = get_object_vars($account);
				$values = array_keys($values);
				// Replace $varstring in string with variable
				foreach ($values as $value) {
					// replace string
					if (is_string($account->$value)) {
						$info_string = str_replace('$'.$value, $account->$value, $info_string);
					}
					// replace object
					elseif (is_object($account->$value)) {
						$values2 = get_object_vars($account->$value);
						$values2 = array_keys($values2);
						foreach ($values2 as $value2) {
							$info_string = str_replace('$'.$value.'->'.$value2, $account->$value->$value2, $info_string);
							}
						}
					// replace array
					elseif (is_array($account->$value)) {
						foreach ($account->$value as $sub_array2) {
							$sub_array .= $sub_array2.", ";
						}
						$sub_array = substr($sub_array, 0, -2);
						$info_string = str_replace('$'.$value, $sub_array, $info_string);
					}
				}
				// Split string in array
				$info_array = explode("\n", $info_string);
				// Set font for text
				$pdf->setFont($fontName,"",10);
				// Print each entry in array (each line in the text) as new line
				foreach ($info_array as $info) {
					$pdf->Cell(50,5,$info,0,1,"L",0);
				}
				// Print linebreak afterwards
				$pdf->Ln(6);
			}
			// We have to include an entry from the account
			elseif($entry['tag'] == "ENTRY") {
				// Get name of current entry
				$name = $entry['attributes']['NAME'];
				
				// Get current entry
				$value_entry = $entries[$name];
				
				// Print entry only when module sumitted values for it
				if(is_array($value_entry)) {
					// Loop over all rows of this entry (most of the time this will be just one)
					foreach($value_entry as $line) {
						// Substitue XML syntax with valid FPDF methods
						$methods = processLine($line,true,$fontName);
						// Call every method
						foreach($methods as $method) {
							call_user_func_array(array(&$pdf,$method[0]),$method[1]);
						}
					}
				}
				$key = false;
			}
		}
	}
	
	// Close PDF
	$pdf->Close();
	// Get relative url path
	$fullpath = realpath('.');
	$subdirs = explode('/', str_replace($_SESSION['lampath'], '', $fullpath));
	for ($i=0; $i<count($subdirs); $i++ ) $filename .= '../';
	// use timestamp and random number from ldap.inc as filename so it should be unique.
	$filename .= 'tmp/' . $_SESSION['ldap']->rand . time() .'.pdf';
	// Save PDF
	$pdf->Output($filename);
	// Output meta refresh to pdf-file
	metaRefresh($filename);
	// Return relative path of pdf-file
	return $filename;
}

/**
 * 
 * 
 * @param string 
 * @param string
 * 
 * @return array
 */
function getStructure($scope,$pdf_structure) {
	$parser = new xmlParser();
	$xml = $parser->parse($_SESSION['lampath'] . '/config/pdf/' . $scope . '/' . $pdf_structure);
	
	$border = array();
	$structure = array();
	$complete_page_definitions = array('filename' => 'printLogo.jpg', 'headline' => 'LDAP Account Manager', 'margin-top' => '10.0', 'margin-bottom' => '20.0', 'margin-left' => '10.0', 'margin-right' => '10.0');
	if($xml[0][$xml[1]['PDF'][0]]['attributes']['TYPE'] == $scope) {
		$border['start'] = $xml[1]['PDF'][0];
		$page_definitions = $xml[0][$xml[1]['PDF'][0]]['attributes'];
		foreach($page_definitions as $key => $value) {
			$complete_page_definitions[strtolower($key)] = $value;
			unset($page_definitions[$key]);
		}
		$border['end'] = $xml[1]['PDF'][1];
	}
	$structure = array_slice($xml[0],$border['start'] + 1,$border['end'] - ($border['start'] + 1));
	return array('structure' => $structure, 'page_definitions' => $complete_page_definitions);
}

/**
 * 
 * 
 * @param string
 * 
 * @return string 
 */
function getSectionHeadline($line) {
	$headline_pattern = '/<block>.*<value>(.*)<\/value><\/block>/';
	if(preg_match($headline_pattern,$line,$matches)) {
		$valueStyle = processFormatTags($matches[1],'');
		return $valueStyle[1];
	}
	else {
		return '';
	}
}

/**
 * 
 * 
 * @param string 
 * @param boolean
 * 
 * @return array
 */
function processLine($line,$first_td = true, $fontName) {
	global $key, $line_width;
	
	// PCRE matching <block> tag
	$block_pattern = '/<block><\/block>/';
	// PCRE matching a <key> tag
	$key_pattern = '/(<block>)<key>(.+)<\/key>(.*<\/block>)/';
	// PCRE matching a <value> tag
	// !!FIXME!! value must contain at least one character
	$value_pattern = '/(<block>.*)<value>(.*)<\/value>(<\/block>)/';
	// PCRE matching a <td> tag
	$td_pattern = '/(<block>.*?)<td(.*?)>(.+?)<\/td>(.*<\/block>)/';
	// PCRE matching a <p> tag
	$p_pattern = '/(<block>.*)<p>(.+)<\/p>(.*<\/block>)/';
	// PCRE matching a <br> tag
	$br_pattern = '/<br \/>/';
	
	$return = array();
	if(preg_match($key_pattern,$line,$matches)) {
		$key = true;
		$line_width = $line_width - 50;
		$format = processFormatTags($matches[2],'B');
		$return[] = array('setFont',array($fontName,$format[0],9));
		$return[] = array('Cell',array(50,5,$format[1] . ':',0,0,'R',0));
		$return[] = array('setFont',array($fontName,'',9));
		return array_merge($return,processLine($matches[1] . $matches[3],false,$fontName));
	}
	elseif(preg_match($value_pattern,$line,$matches)) {
		$format = processFormatTags($matches[2],'');
		$return[] = array('setFont',array($fontName,$format[0],9));
		$return[] = array('Cell',array(50,5,$format[1],0,0,'L',0));
		$return[] = array('setFont',array($fontName,'',9));
		return array_merge($return,processLine($matches[1] . $matches[3],true,$fontName));
	}
	elseif(preg_match($p_pattern,$line,$matches)) {
		$format = processFormatTags($matches[2],'');
		$return[] = array('setFont',array($fontName,$format[0],9));
		$return[] = array('Write',array(5,$format[1]));
		$return[] = array('setFont',array($fontName,'',9));
		return array_merge($return,processLine($matches[1] . $matches[3],true,$fontName));
	}
	elseif(preg_match($td_pattern,$line,$matches)) {
		if($first_td && $key) {
			$first_td = !$first_td;
			$return[] = array('Cell',array(50,5,'',0,0,'L',0));
		}
		$format = processFormatTags($matches[3],'');
		$attrs = processAttributes($matches[2],array('width' => $line_width,'height' => 5,'align' => 'L'));
		$return[] = array('setFont',array($fontName,$format[0],9));
		$return[] = array('Cell',array($attrs['width'],$attrs['height'],$format[1],0,0,$attrs['align'],0));
		$return[] = array('setFont',array($fontName,'',9));
		return array_merge($return,processLine($matches[1] . $matches[4],$first_td,$fontName));
	}
	elseif(preg_match($br_pattern,$line,$matches)) {
		return array(array('Ln',array(5)));
	}
	elseif(preg_match($block_pattern,$line,$matches)) {
		$line_width = LAMPDF_LINEWIDTH;
		return array(array('Ln',array(5)));
	}
}

/**
 * 
 * 
 * @param string 
 * @param string
 * 
 * @return array 
 */
function processFormatTags($line,$style) {
	// PCRE matching a <i> tag
	$i_pattern = '/(.*)<i>(.+)<\/i>(.*)/';
	// PCRE matching a <b> tag
	$b_pattern = '/(.*)<b>(.+)<\/b>(.*)/';
	// PCRE matching a <u> tag
	$u_pattern = '/(.*)<u>(.+)<\/u>(.*)/';
	// Replacement pattern when one of the above pattern matched
	$replace = "\$1\$2\$3";
	
	if(preg_match($i_pattern,$line,$matches)) {
		$style .= "I";
		$line = preg_replace($i_pattern,$replace,$line);
	}
	if(preg_match($b_pattern,$line,$matches)) {
		$style .= "B";
		$line = preg_replace($b_pattern,$replace,$line);
	}
	if(preg_match($u_pattern,$line,$matches)) {
		$style .= "U";
		$line = preg_replace($u_pattern,$replace,$line);
	}
	return array($style,$line);
}

/**
 * 
 * 
 * @param string 
 * @param array
 * 
 * @return array
 */
function processAttributes($attrs,$return = array()) {
	global $line_width;
	
	// PCRE matching width attribute
	$width_pattern = '/(.*)width\=\"(\\d+)(\%?)\"(.*)/';
	// PCRE matching height attribute
	$height_pattern = '/(.*)height\=\"(\\d+)\"(.*)/';
	// PCRE matching align attribute
	$align_pattern = '/(.*)align\=\"(L|R|C)\"(.*)/';
	
	// Remove leading and trailing whitespaces
	$attrs = trim($attrs);
	
	if(preg_match($width_pattern,$attrs,$matches)) {
		if($matches[3] == '%') {
			$return['width'] = ceil($line_width * $matches[2] / 100);
		}
		else {
			$return['width'] = ceil($matches[2]);
		}
		return processAttributes($matches[1] . $matches[4],$return);
	}
	elseif(preg_match($height_pattern,$attrs,$matches)) {
		$return['height'] = $matches[2];
		return processAttributes($matches[1] . $matches[3],$return);
	}
	elseif(preg_match($align_pattern,$attrs,$matches)) {
		$return['align'] = $matches[2];
		return processAttributes($matches[1] . $matches[3],$return);
	}
	else {
		return $return;
	}
}


/**
 * Creates a LAM information page in PDF format.
 * 
 * @author Michael Dürgner
 * @package PDF
 */
class lamPDF extends UFPDF {
	
	/**
	 * format settings for page layout
	 */
	var $page_definitions;
	
	/**
	* current active font name
	*/
	var $fontName;
	
	/**
	* list of supported fonts
	* format: <font name> => array(<file for default font style>, <file for bold>, <italic>, <bold italic>)
	*/
	var $fontList = array(
		'BitstreamVeraSans-Roman' => array('vera.php', 'verab.php', 'verabi.php', 'verai.php')
	);

	/**
	 * 
	 * 
	 * @param string 
	 * @param array
	 */
	function lamPDF($account_type = "User",$page_definitions = array(),$fontName) {
		$this->fontName = $fontName;
		define('FPDF_FONTPATH', $_SESSION['lampath'] . "lib/" . 'font/');
		// Call constructor of superclass
		$this->FPDF('P','mm','A4');
		
		$this->page_definitions = $page_definitions;
		
		// Decide which PDF file type we shall use
		switch($account_type) {
			case "user":
				$subject = _("User information page");
				break;
			case "group":
				$subject = _("Group information page");
				break;
			case "host":
				$subject = _("Host information page");
				break;
		}
		
		// Open PDF file and write some basic information
		$this->Open();
		$this->AddFont($this->fontName, '', $this->fontList[$this->fontName][0]);
		$this->AddFont($this->fontName, 'B', $this->fontList[$this->fontName][1]);
		$this->AddFont($this->fontName, 'I', $this->fontList[$this->fontName][2]);
		$this->AddFont($this->fontName, 'BI', $this->fontList[$this->fontName][3]);
		$this->setFont($this->fontName,"",12);
		$this->setTitle($this->page_definitions['headline']);
		$this->setSubject($subject);
		$this->setAuthor("LDAP Account Manager Devel-Team -Michael Duergner-");
		$this->setCreator("LDAP Account Manager (pdf.inc)");
		$this->setMargins($this->page_definitions['margin-left'],$this->page_definitions['margin-top'],$this->page_definitions['margin-right']);
		$this->setAutoPageBreak(true,$this->page_definitions['margin-bottom']);
	}
	
	/**
	 * 
	 */
	function header() {
		if($this->page_definitions['filename'] != 'none') {
			$imageFile = substr(__FILE__,0,strlen(__FILE__)- 11) . "config/pdf/logos/" . $this->page_definitions['filename'];
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
		$this->SetFont($this->fontName,"B",22);
		$this->Cell(170,5,$this->page_definitions['headline'],0,1,"R",0);
		$this->Ln(3);
		$this->SetFont($this->fontName,"",14);
		$this->Cell(170,5,"- " . $this->subject . " -",0,0,"R",0);
		$this->SetLineWidth(0.8);
		$this->Line(10,$this->page_definitions['margin-top'] + 30,200,$this->page_definitions['margin-top'] + 30);
		$this->Line(10,$this->page_definitions['margin-top'] + 32,200,$this->page_definitions['margin-top'] + 32);
		$this->SetY(50);
	}

	/**
	 * 
	 */
	function footer() {
		$this->SetLineWidth(0.8);
		$this->Line(10,280,200,280);
		$this->Line(10,282,200,282);
		$this->SetY(285);
		$this->Cell(0,5,_("This document was automatically created by LDAP Account Manager"),0,0,"C",0);
	}
}