<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2004  Michael Duergner
                2003 - 2013  Roland Gruber

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

/** PDF line width */
define('LAMPDF_LINEWIDTH',190);

/** XML functions */
include_once('xml_parser.inc');

/** access to PDF configuration files */
include_once('pdfstruct.inc');

$key = false;
$line_width = LAMPDF_LINEWIDTH;

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
	include_once("fpdf.php");
	/** Unicode support for FPDF */
	include_once("ufpdf.php");
	/** LAM PDF generator class */
	include_once("lamPDF.inc");

	global $key;
	
	$account_type = $accounts[0]->get_type();
	// Get PDF structure from xml file
	$load = loadPDFStructureDefinitions($account_type,$pdf_structure);
	$structure = $load['structure'];
	
	// The decimal separator must be a dot in order to write pdf-files
	setlocale(LC_NUMERIC, "C");
	
	$fontName = "BitstreamVeraSans-Roman";
	
	// Create a new PDF file acording to the account type
	$pdf = new lamPDF($load['page_definitions'],$fontName);
	
	// Loop over each account and add a new page in the PDF file for it 
	foreach($accounts as $account) {
		
		// Start a new page for each account
		$pdf->AddPage();
		
		// Get PDF entries for the current account
		$entries = $account->get_pdfEntries();
		
		// Now create the PDF file acording to the structure with the submitted values 
		foreach($structure as $entry) {
			// We have a new section to start
			if($entry['tag'] == "SECTION" && $entry['type'] == "open") {
				$name = $entry['attributes']['NAME'];
				if(preg_match("/^_[a-zA-Z_]+/",$name)) {
					$section_headline = getSectionHeadline($entries[substr($name,1)][0]);
				}
				else {
					$section_headline = $name;
				}
				$pdf->setFont($fontName,"B",10);
				$pdf->Write(0,$section_headline . ":");
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
				// Set font for text
				$pdf->setFont($fontName,"",10);
				$pdf->MultiCell(0,5,$info_string,0,"L",0);
				// Print linebreak afterwards
				$pdf->Ln(6);
			}
			// We have to include an entry from the account
			elseif($entry['tag'] == "ENTRY") {
				// Get name of current entry
				$name = $entry['attributes']['NAME'];
				// skip non-existent entries
				if (isset($entries[$name])) {
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
				}
				$key = false;
			}
		}
	}
	
	// Close PDF
	$pdf->Close();
	if (!$returnAsString) {
		// use timestamp and random number from ldap.inc as filename so it should be unique.
		$filename = '../../tmp/' . $_SESSION['ldap']->new_rand() . time() .'.pdf';
		// Save PDF
		$pdf->Output($filename);
		chmod($filename, 0600);
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
 * @param string $line section name
 * 
 * @return string XML code for headline
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
 * Creates the XML code for an PDF entry.
 * 
 * @param string $line XML code of PDF entry
 * @param boolean $first_td True if this is the first column
 * @param String $fontName font name
 * 
 * @return array XML codes
 */
function processLine($line, $first_td = true, $fontName) {
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
	// PCRE matching <tr> tag
	$tr_pattern = '/<tr><\/tr>/';
	// PCRE matching a <p> tag
	$p_pattern = '/(<block>.*)<p>(.+)<\/p>(.*<\/block>)/';
	// PCRE matching a <br> tag
	$br_pattern = '/<br \/>/';
	
	$return = array();
	if(preg_match($key_pattern,$line,$matches)) {
		$key = true;
		$line_width = $line_width - 50;
		$format = processFormatTags($matches[2],'B');
		$return[] = array('setFont',array($fontName,$format[0],7));
		$return[] = array('Cell',array(50,5,$format[1] . ':',0,0,'R',0));
		$return[] = array('setFont',array($fontName,'',7));
		return array_merge($return,processLine($matches[1] . $matches[3],false,$fontName));
	}
	elseif(preg_match($value_pattern,$line,$matches)) {
		$format = processFormatTags($matches[2],'');
		$return[] = array('setFont',array($fontName,$format[0],7));
		$return[] = array('MultiCell',array(0,5,$format[1],0,'L',0));
		$return[] = array('setFont',array($fontName,'',7));
		return array_merge($return,processLine($matches[1] . $matches[3],true,$fontName));
	}
	elseif(preg_match($p_pattern,$line,$matches)) {
		$format = processFormatTags($matches[2],'');
		$return[] = array('setFont',array($fontName,$format[0],7));
		$return[] = array('Write',array(5,$format[1]));
		$return[] = array('setFont',array($fontName,'',7));
		return array_merge($return,processLine($matches[1] . $matches[3],true,$fontName));
	}
	elseif(preg_match($td_pattern,$line,$matches)) {
		if($first_td && $key) {
			$first_td = !$first_td;
			$return[] = array('Cell',array(50,5,'',0,0,'L',0));
		}
		$format = processFormatTags($matches[3],'');
		$attrs = processAttributes($matches[2],array('width' => $line_width,'height' => 5,'align' => 'L'));
		$return[] = array('setFont',array($fontName,$format[0],7));
		$return[] = array('Cell',array($attrs['width'],$attrs['height'],$format[1],0,0,$attrs['align'],0));
		$return[] = array('setFont',array($fontName,'',7));
		return array_merge($return,processLine($matches[1] . $matches[4],$first_td,$fontName));
	}
	elseif(preg_match($br_pattern,$line,$matches)) {
		return array(array('Ln',array(5)));
	}
	elseif(preg_match($block_pattern,$line,$matches)) {
		$line_width = LAMPDF_LINEWIDTH;
		return array();
	}
	elseif(preg_match($tr_pattern,$line,$matches)) {
		$line_width = LAMPDF_LINEWIDTH;
		return array(array('Ln',array(5)));
	}
}

/**
 * Formats the XML code.
 * 
 * @param string $line XML code of PDF entry
 * @param string $style style commands
 * 
 * @return array XML code
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
 * Processes width, height and alignment attributes.
 * 
 * @param string $attrs attributes
 * @param array $return XML code
 * 
 * @return array XML code
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
