<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Michael Drgner

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


  LDAP Account Manager lamPDF class and some pdf functions
*/

define('FPDF_FONTPATH','font/');
define('LAMPDF_LINEWIDTH',190);
include_once("fpdf.php");
include_once('xml_parser.inc');

$key = false;
$line_width = LAMPDF_LINEWIDTH;

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

function createModulePDF($accounts,$pdf_structure="default.xml") {
	
	global $key;
	
	// Get account type from account container if none was specified or
	// if it is different to the submitted or previous stored
	if($account_type == "" || $account_type != $accounts[0]->get_type()) {
		$account_type = $accounts[0]->get_type();
	}
	
	// Get PDF structure from xml file
	$structure = getStructure($account_type,$pdf_structure);
	
	// The decimal separator must be a dot in order to write pdf-files
	setlocale(LC_NUMERIC, "C");
	
	// Create a new PDF file acording to the account type
	$pdf = new LamPDF($account_type);
	
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
					$section_headline = '- ' . _($name);
				}
				$pdf->setFont("arial","B",12);
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
				$pdf->setFont("times","",10);
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
						$methods = processLine($line);
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

function getStructure($scope,$pdf_structure) {
	$parser = new xmlParser();
	$xml = $parser->parse($_SESSION['lampath'] . '/config/pdf/' . $scope . '/' . $pdf_structure);
	
	$border = array();
	$structure = array();
	$pdf_entries = $xml[1]['PDF'];
	while(($index = current($pdf_entries)) != null) {
		$border['start'] = $index;
		next($pdf_entries);
		$border['end'] = current($pdf_entries);
		next($pdf_entries);
	}
	return array_slice($xml[0],$border['start'] + 1,$border['end'] - ($border['start'] + 1));
}

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

function processLine($line,$first_td = true) {
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
		$return[] = array('setFont',array('times',$format[0],10));
		$return[] = array('Cell',array(50,5,$format[1] . ':',0,0,'R',0));
		$return[] = array('setFont',array('times','',10));
		return array_merge($return,processLine($matches[1] . $matches[3],false));
	}
	elseif(preg_match($value_pattern,$line,$matches)) {
		$format = processFormatTags($matches[2],'');
		$return[] = array('setFont',array('times',$format[0],10));
		$return[] = array('Cell',array(50,5,$format[1],0,0,'L',0));
		$return[] = array('setFont',array('times','',10));
		return array_merge($return,processLine($matches[1] . $matches[3]));
	}
	elseif(preg_match($p_pattern,$line,$matches)) {
		$format = processFormatTags($matches[2],'');
		$return[] = array('setFont',array('times',$format[0],10));
		$return[] = array('Write',array(5,$format[1]));
		$return[] = array('setFont',array('times','',10));
		return array_merge($return,processLine($matches[1] . $matches[3]));
	}
	elseif(preg_match($td_pattern,$line,$matches)) {
		if($first_td && $key) {
			$first_td = !$first_td;
			$return[] = array('Cell',array(50,5,'',0,0,'L',0));
		}
		$format = processFormatTags($matches[3],'');
		$attrs = processAttributes($matches[2],array('width' => $line_width,'height' => 5,'align' => 'L'));
		$return[] = array('setFont',array('times',$format[0],10));
		$return[] = array('Cell',array($attrs['width'],$attrs['height'],$format[1],0,0,$attrs['align'],0));
		$return[] = array('setFont',array('times','',10));
		return array_merge($return,processLine($matches[1] . $matches[4],$first_td));
	}
	elseif(preg_match($br_pattern,$line,$matches)) {
		return array(array('Ln',array(5)));
	}
	elseif(preg_match($block_pattern,$line,$matches)) {
		$line_width = LAMPDF_LINEWIDTH;
		return array(array('Ln',array(5)));
	}
}

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


// lamPDF class || For defining own a Header and Footer
class lamPDF extends FPDF {
	
	function lamPDF($account_type = "User") {
		// Call constructor of superclass
		$this->FPDF('P','mm','A4');
		
		// Decide which PDF file type we shall use
		switch($account_type) {
			case "user":
				$subject = _("User information page");
				break;
			case "group":
				$subject = _("Group information page");
				break;
			case "host":
				$subject = _("Samba-Host information page");
				break;
		}
		
		// Open PDF file and write some basic information
		$this->Open();
		$this->setFont("arial","",12);
		$this->setTitle("LDAP Account Manager");
		$this->setSubject($subject);
		$this->setAuthor("LDAP Account Manager Devel-Team -Michael Duergner-");
		$this->setCreator("LDAP Account Manager (pdf.inc)");
	}
	
	// Print page header
	function header() {
		$imageFile = substr(__FILE__,0,strlen(__FILE__)- 11) . "graphics/printLogo.jpg";
		$this->Image($imageFile,10,10,50,20,"JPG");
		$this->SetFont("arial","B",22);
		$this->Cell(170,5,"LDAP Account Manager",0,1,"R",0);
		$this->Ln(3);
		$this->SetFont("times","",14);
		$this->Cell(170,5,"- " . $this->subject . " -",0,0,"R",0);
		$this->SetLineWidth(0.8);
		$this->Line(10,40,200,40);
		$this->Line(10,42,200,42);
		$this->SetY(50);
	}

	// Print page footer
	function footer() {
		$this->SetLineWidth(0.8);
		$this->Line(10,280,200,280);
		$this->Line(10,282,200,282);
		$this->SetY(285);
		$this->Cell(0,5,_("This document was automatically created by LDAP Account Manager"),0,0,"C",0);
	}
}