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
define('LAMPDF_LINEWIDTH',149);
include_once("fpdf.php");
//include_once('include.inc');
include_once('xml_parser.inc');

$key = false;
$line_width = LAMPDF_LINEWIDTH;

function createModulePDF($accounts,$account_type) {
	
	global $key;
	
	// Store PDF structure in Session when called the first time
	if(!isset($_SESSION['pdf_structure'])) {
		$_SESSION['pdf_structure'] = getStructure();
	}
	
	// Get PDF structure from session
	$structure = $_SESSION['pdf_structure'][$account_type];
	
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
				if(preg_match("/^\_[a-z]+/",$name)) {
					$section_headline = $entries[ucwords(substr($name,1))][0];
				}
				else {
					$section_headline = $name;
				}
				$pdf->setFont("arial","B",12);
				$pdf->Write(5,"- " . _($section_headline) . ":");
				$pdf->Ln(6);
			}
			// We have a section to end
			elseif($entry['tag'] == "SECTION" && $entry['type'] == "close") {
				$pdf->Ln(9);
			}
			// We have to include a static text.
			elseif($entry['tag'] == "TEXT") {
				// Decide which individual text to fetch
				if(isset($entry['attributes']['NAME'])) {
					$text_name = $entry['attributes']['NAME'];
				}
				else {
					$text_name = "User";
				}
				// Load string with individual text for PDF from session
				$info_string = $_SESSION['config']->get_pdftext($text_name);
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
				// Get current entry
				$entry = $entries[$name];
				
				// Loop over all rows of this entry (most of the time this will be just one)
				foreach($entry as $line) {
					// Substitue XML syntax with valid FPDF methods
					$methods = processLine($line);
					// Call every method
					foreach($methods as $method) {
						call_user_method_array($pdf,$method[0],$method[1]);
					}
				}
				$key = false;
			}
		}
	}
	
	// Close PDF
	$pdfFile->Close();
	// Get relative url path
	$fullpath = realpath('.');
	$subdirs = explode('/', str_replace($_SESSION['lampath'], '', $fullpath));
	for ($i=0; $i<count($subdirs); $i++ ) $filename .= '../';
	// use timestamp and random number from ldap.inc as filename so it should be unique.
	$filename .= 'tmp/' . $_SESSION['ldap']->rand . time() .'.pdf';
	// Save PDF
	$pdfFile->Output($filename);
	// Output meta refresh to pdf-file
	metaRefresh($filename);
	// Return relative path of pdf-file
	return $filename;
}

function getStructure($account_type = array("User","Group","Host")) {
	$parser = new xmlParser();
	$xml = $parser->parse('../config/pdf-structure.xml');
	$border = array();
	$structure = array();
	while(($current = current($account_type)) != null) {
		$pdf_entries = $xml[1]['PDF'];
		$border[$current] = array();
		while(($index = current($pdf_entries)) != null) {
			if($xml[0][$index]['attributes']['TYPE'] == $current) {
				$border[$current]['start'] = $index;
				next($pdf_entries);
				$border[$current]['end'] = current($pdf_entries);
			}
			next($pdf_entries);
		}
		$structure[$current] = array_slice($xml[0],$border[$current]['start'] + 1,$border[$current]['end'] - ($border[$current]['start'] + 1));
		next($account_type);
	}
	return $structure;
}

function processLine($line,$first_td = true) {
	global $key, $line_width;
	
	// PCRE matching <block> tag
	$block_pattern = '/<block><\/block>/';
	// PCRE matching a <key> tag
	$key_pattern = '/(<block>)<key>(.+)<\/key>(.*<\/block>)/';
	// PCRE matching a <value> tag
	$value_pattern = '/(<block>.*)<value>(.+)<\/value>(<\/block>)/';
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
	$replace = "\${1}\${2}\${3}";
	
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

function createUserPDF($accounts) {
// The decimal separator must be a dot in order to write pdf-files
	setlocale(LC_NUMERIC, "C");
// Start PDF file
	$pdfFile = new lamPDF();
	$pdfFile->Open();
// Write some information into the PDF file
	$pdfFile->setFont("arial","",12);
	$pdfFile->setTitle("LDAP Account Manager");
	$pdfFile->setSubject(_("User information page"));
	$pdfFile->setAuthor("LDAP Account Manager Devel-Team -Michael Duergner-");
	$pdfFile->setCreator("LDAP Account Manager (pdf.inc)");
// Loop for every sumbitted account and print its values on a extra page
	foreach ($accounts as $account) {
		$pdfFile->addPage();
		// Load string with additional information from session
		$info_string = $_SESSION['config']->pdftext;
		// Print individuall text.
		// Get all allowed vairables from account-object
		$values = get_object_vars($account);
		$values = array_keys($values);
		// Replace $varstring in string with variable
		foreach ($values as $value) {
			// replace string
			if (is_string($account->$value)) $info_string = str_replace('$'.$value, $account->$value, $info_string);
			// replace object
			else if (is_object($account->$value)) {
				$values2 = get_object_vars($account->$value);
				$values2 = array_keys($values2);
				foreach ($values2 as $value2) {
					$info_string = str_replace('$'.$value.'->'.$value2, $account->$value->$value2, $info_string);
					}
				}
			// replace array
			else if (is_array($account->$value)) {
				foreach ($account->$value as $sub_array2) $sub_array .= $sub_array2.", ";
				$sub_array = substr($sub_array, 0, -2);
				$info_string = str_replace('$'.$value, $sub_array, $info_string);
				}
			}
		// Split string in array
		$info_array = explode("\n", $info_string);
		$pdfFile->setFont("times","",10);
		foreach ($info_array as $info)
			$pdfFile->Cell(50,5,$info,0,1,"L",0);
		$pdfFile->Ln(6);
// Print Personal settings
		$pdfFile->setFont("arial","B",12);
		$pdfFile->Write(5,"- " . _("Personal User Infos") . ":");
		$pdfFile->Ln(6);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,_("Title") . ":",0,0,"R",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,$account->personal_title,0,1,"L",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,_("Surname") . ":",0,0,"R",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,$account->general_surname,0,1,"L",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,_("Given name") . ":",0,0,"R",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,$account->general_givenname,0,1,"L",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,_("Street") . ":",0,0,"R",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,$account->personal_street,0,1,"L",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,_("Postal code") . ":",0,0,"R",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,$account->personal_postalCode,0,1,"L",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,_("Postal address") . ":",0,0,"R",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,$account->personal_postalAddress,0,1,"L",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,_("E-Mail") . ":",0,0,"R",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,$account->personal_mail,0,1,"L",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,_("Phone") . ":",0,0,"R",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,$account->personal_telephoneNumber,0,1,"L",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,_("Cellular phone") . ":",0,0,"R",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,$account->personal_mobileTelephoneNumber,0,1,"L",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,_("Facsimile") . ":",0,0,"R",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,$account->personal_facsimileTelephoneNumber,0,1,"L",0);
		$pdfFile->setFont("times","",10);
// Print Unix settings
		$pdfFile->Ln(9);
		$pdfFile->setFont("arial","B",12);
		$pdfFile->Write(5,"- " . _("Unix User Settings") . ":");
		$pdfFile->Ln(6);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,_("Username") . ":",0,0,"R",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,$account->general_username,0,1,"L",0);
		if($account->unix_password_no == "1") {
			$pdfFile->setFont("times","B",10);
			$pdfFile->Cell(50,5,_("Unix password") . ":",0,0,"R",0);
			$pdfFile->setFont("times","",10);
			$pdfFile->Cell(50,5,_("Unix password disabled!"),0,1,"L",0);
		}
		elseif($account->unix_password == "") {
		}
		else {
			$account->unix_password = $_SESSION['ldap']->decrypt(base64_decode($account->unix_password));
			$pdfFile->setFont("times","B",10);
			$pdfFile->Cell(50,5,_("Unix password") . ":",0,0,"R",0);
			$pdfFile->setFont("times","",10);
			$pdfFile->Cell(50,5,$account->unix_password,0,1,"L",0);
		}
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,_("Group(s)") . ":",0,0,"R",0);
		$text = $account->general_group;
		$groups = $account->general_groupadd;
		for($j=0;$j<count($groups);$j++) {
			$text .= ", " . $groups[$j];
		}
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,$text,0,1,"L",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,_("Home Directory") . ":",0,0,"R",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,$account->general_homedir,0,1,"L",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,_("Login Shell") . ":",0,0,"R",0);
		$text = "";
		$shells = $account->general_shell;
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,$account->general_shell,0,1,"L",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,_("Password expire") . ":",0,0,"R",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,date('j. F Y' ,$account->unix_pwdexpire),0,1,"L",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,_("Login at host(s)") . ":",0,0,"R",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,$account->unix_host,0,1,"L",0);
// Print Samba settings
		$pdfFile->Ln(9);
		$pdfFile->setFont("arial","B",12);
		$pdfFile->Write(5,"- " . _("Windows User Settings") . ":");
		$pdfFile->Ln(6);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,_("Username") . ":",0,0,"R",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,$account->general_username,0,1,"L",0);
		if($account->smb_flags['N']) {
			$pdfFile->setFont("times","B",10);
			$pdfFile->Cell(50,5,_("Windows password") . ":",0,0,"R",0);
			$pdfFile->setFont("times","",10);
			$pdfFile->Cell(50,5,_("Window password disabled!"),0,1,"L",0);
		}
		elseif($account->smb_useunixpwd == "1") {
			$pdfFile->setFont("times","B",10);
			$pdfFile->Cell(50,5,_("Windows password") . ":",0,0,"R",0);
			$pdfFile->setFont("times","",10);
			$pdfFile->Cell(50,5,_("Windows password set to unix password."),0,1,"L",0);
		}
		elseif($account->smb_password == "") {
		}
		else {
			$account->smb_password = $_SESSION['ldap']->decrypt(base64_decode($account->smb_password));
			$pdfFile->setFont("times","B",10);
			$pdfFile->Cell(50,5,_("Windows password") . ":",0,0,"R",0);
			$pdfFile->setFont("times","",10);
			$pdfFile->Cell(50,5,$account->smb_password,0,1,"L",0);
		}
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,_("Home drive") . ":",0,0,"R",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,$account->smb_homedrive,0,1,"L",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,_("Script path") . ":",0,0,"R",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,$account->smb_scriptPath,0,1,"L",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,_("Profile path") . ":",0,0,"R",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,$account->smb_profilePath,0,1,"L",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,_("Login at workstation(s)") . ":",0,0,"R",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,$account->smb_smbuserworkstations,0,1,"L",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,_("Windows home directory") . ":",0,0,"R",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,$account->smb_smbhome,0,1,"L",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,_("Windows Domain") . ":",0,0,"R",0);
		$pdfFile->setFont("times","",10);
		if($_SESSION['config']->get_samba3() == "yes") $pdfFile->Cell(50,5,$account->smb_domain->name,0,1,"L",0);
		else $pdfFile->Cell(50,5,$account->smb_domain,0,1,"L",0);
// Print Quota settings
		$pdfFile->Ln(9);
		$pdfFile->setFont("arial","B",12);
		$pdfFile->Write(5,"- " . _("Quota Settings") . ":");
		$pdfFile->Ln(6);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,_("User quota(s)") . ":",0,0,"R",0);
		$pdfFile->setFont("times","",10);
		$quotas = $account->quota;
		if(count($quotas)>0) {
			$pdfFile->Cell(30,5,_("Mountpoint"),0,0,"L",0);
			$pdfFile->Cell(30,5,_("Soft block"),0,0,"L",0);
			$pdfFile->Cell(30,5,_("Soft inode"),0,0,"L",0);
			$pdfFile->Cell(30,5,_("Hard block"),0,0,"L",0);
			$pdfFile->Cell(30,5,_("Hard inode"),0,1,"L",0);
			for($j=0;$j<count($quotas);$j++) {
				$pdfFile->Cell(50,5,"",0,0,"R",0);
				$pdfFile->Cell(30,5,$quotas[$j][0],0,0,"L",0);
				$pdfFile->Cell(30,5,$quotas[$j][2],0,0,"L",0);
				$pdfFile->Cell(30,5,$quotas[$j][6],0,0,"L",0);
				$pdfFile->Cell(30,5,$quotas[$j][3],0,0,"L",0);
				$pdfFile->Cell(30,5,$quotas[$j][7],0,1,"L",0);
			}
		}
		else {
			$pdfFile->Cell(50,5,_("No user quotas defined or no quota support by filesystem."),0,1,"L",0);
		}
		$pdfFile->Ln(9);
	}
	// Close PDF
	$pdfFile->Close();
	// Get relative url path
	$fullpath = realpath('.');
	$subdirs = explode('/', str_replace($_SESSION['lampath'], '', $fullpath));
	for ($i=0; $i<count($subdirs); $i++ ) $filename .= '../';
	// use timestamp and random number from ldap.inc as filename so it should be unique.
	$filename .= 'tmp/' . $_SESSION['ldap']->rand . time() .'.pdf';
	// Save PDF
	$pdfFile->Output($filename);
	// Output meta refresh to pdf-file
	metaRefresh($filename);
	// Return relative path of pdf-file
	return $filename;
}

// creates a PDF with host accounts
// $accounts: array of account
function createHostPDF($accounts) {
	// The decimal separator must be a dot in order to write pdf-files
	setlocale(LC_NUMERIC, "C");
	// Start PDF file
	$pdfFile = new lamHostPDF();
	$pdfFile->Open();
	// Write some information into the PDF file
	$pdfFile->setFont("arial","",12);
	$pdfFile->setTitle("LDAP Account Manager");
	$pdfFile->setSubject(_("Samba-Host information page"));
	$pdfFile->setAuthor("LDAP Account Manager Devel-Team -Roland Gruber-");
	$pdfFile->setCreator("LDAP Account Manager (pdf.inc)");
	// print host accounts
	$pdfFile->addPage();
	$k = 0;  // used to count the already printed accounts per page
	for($i = 0; $i < count($accounts); $i++) {
		$k++;
		$account = $accounts[$i];
		$ypos = $pdfFile->GetY();
		$pdfFile->setFont("times","B",16);
		$pdfFile->Write(5,$account->general_username);
		$pdfFile->Ln(8);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Write(5,_("Description") . ": ");
		$pdfFile->setFont("times","",10);
		$pdfFile->Write(5,$account->general_gecos);
		$pdfFile->Ln(5);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Write(5,_("UID") . ": ");
		$pdfFile->setFont("times","",10);
		$pdfFile->Write(5,$account->general_uidNumber);
		$pdfFile->Ln(5);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Write(5,_("Primary group") . ": ");
		$pdfFile->setFont("times","",10);
		$pdfFile->Write(5,$account->general_group);
		$pdfFile->Ln(5);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Write(5,_("Domain") . ": ");
		$pdfFile->setFont("times","",10);
		unset($domain);
		if (is_string($account->smb_domain)) {
			$domain = $account->smb_domain;
		}
		else {
			$domain = $account->smb_domain->name;
		}
		$pdfFile->Write(5,$domain);
		$pdfFile->Ln(5);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Write(5,_("DN") . ": ");
		$pdfFile->setFont("times","",9);
		$pdfFile->Write(5,$account->general_dn);
		// print second column
		if ($accounts[$i+1]) {
			$k++;
			$i++;
			$account = $accounts[$i];
			$pdfFile->SetY($ypos);
			$pdfFile->SetX(115);
			$pdfFile->setFont("times","B",16);
			$pdfFile->Write(5,$account->general_username);
			$pdfFile->Ln(7);
			$pdfFile->SetX(115);
			$pdfFile->setFont("times","B",10);
			$pdfFile->Write(5,_("Description") . ": ");
			$pdfFile->setFont("times","",10);
			$pdfFile->Write(5,$account->general_gecos);
			$pdfFile->Ln(5);
			$pdfFile->SetX(115);
			$pdfFile->setFont("times","B",10);
			$pdfFile->Write(5,_("UID") . ": ");
			$pdfFile->setFont("times","",10);
			$pdfFile->Write(5,$account->general_uidNumber);
			$pdfFile->Ln(5);
			$pdfFile->SetX(115);
			$pdfFile->setFont("times","B",10);
			$pdfFile->Write(5,_("Primary group") . ": ");
			$pdfFile->setFont("times","",10);
			$pdfFile->Write(5,$account->general_group);
			$pdfFile->Ln(5);
			$pdfFile->SetX(115);
			$pdfFile->setFont("times","B",10);
			$pdfFile->Write(5,_("Domain") . ": ");
			$pdfFile->setFont("times","",10);
			unset($domain);
			if (is_string($account->smb_domain)) {
				$domain = $account->smb_domain;
			}
			else {
				$domain = $account->smb_domain->name;
			}
			$pdfFile->Write(5,$domain);
			$pdfFile->Ln(5);
			$pdfFile->SetX(115);
			$pdfFile->setFont("times","B",10);
			$pdfFile->Write(5,_("DN") . ": ");
			$pdfFile->setFont("times","",9);
			$pdfFile->Write(5,$account->general_dn);
		}
		// space between two rows
		if ($i < count($accounts) - 1) {
			$pdfFile->Ln(12);
		}
		// new page after twelve accounts
		if ($k == 12) {
			$k = 0;
			$pdfFile->addPage();
		}
	}

	// Close PDF
	$pdfFile->Close();
	// Get relative url path
	$fullpath = realpath('.');
	$subdirs = explode('/', str_replace($_SESSION['lampath'], '', $fullpath));
	for ($i=0; $i<count($subdirs); $i++ ) $filename .= '../';
	// use timestamp and random number from ldap.inc as filename so it should be unique.
	$filename .= 'tmp/' . $_SESSION['ldap']->rand . time() .'.pdf';
	// Save PDF
	$pdfFile->Output($filename);
	// Output meta refresh to pdf-file
	metaRefresh($filename);
	// Return relative path of pdf-file
	return $filename;
}

// creates a PDF with groups
// $accounts: array of account
function createGroupPDF($accounts) {
	// The decimal separator must be a dot in order to write pdf-files
	setlocale(LC_NUMERIC, "C");
	// Start PDF file
	$pdfFile = new lamGroupPDF();
	$pdfFile->Open();
	// Write some information into the PDF file
	$pdfFile->setFont("arial","",12);
	$pdfFile->setTitle("LDAP Account Manager");
	$pdfFile->setSubject(_("Group information page"));
	$pdfFile->setAuthor("LDAP Account Manager Devel-Team -Roland Gruber-");
	$pdfFile->setCreator("LDAP Account Manager (pdf.inc)");
	// print group accounts
	$pdfFile->addPage();
	for($i = 0; $i < count($accounts); $i++) {
		$account = $accounts[$i];
		$ypos = $pdfFile->GetY();
		if ($ypos > 260) $pdfFile->addPage();
		$pdfFile->setFont("times","B",16);
		$pdfFile->Write(5,$account->general_username);
		$pdfFile->Ln(8);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Write(5,_("Description") . ": ");
		$pdfFile->setFont("times","",10);
		$pdfFile->Write(5,$account->general_gecos);
		$pdfFile->Ln(5);
		if ($account->smb_displayname) {
			$pdfFile->setFont("times","B",10);
			$pdfFile->Write(5,_("Display name") . ": ");
			$pdfFile->setFont("times","",10);
			$pdfFile->Write(5,$account->smb_displayname);
			$pdfFile->Ln(5);
		}
		$pdfFile->setFont("times","B",10);
		$pdfFile->Write(5,_("GID") . ": ");
		$pdfFile->setFont("times","",10);
		$pdfFile->Write(5,$account->general_uidNumber);
		$pdfFile->Ln(5);
		if ($account->domain) {
			$pdfFile->setFont("times","B",10);
			$pdfFile->Write(5,_("Domain") . ": ");
			$pdfFile->setFont("times","",10);
			$pdfFile->Write(5,$account->domain->name);
			$pdfFile->Ln(5);
		}
		$pdfFile->setFont("times","B",10);
		$pdfFile->Write(5,_("DN") . ": ");
		$pdfFile->setFont("times","",9);
		$pdfFile->Write(5,$account->general_dn);
		if (is_array($account->unix_memberUid)) {
			$pdfFile->Ln(5);
			$pdfFile->setFont("times","B",10);
			$pdfFile->Write(5,_("Members") . ": ");
			$pdfFile->setFont("times","",10);
			$pdfFile->Write(5,implode(", ", $account->unix_memberUid));
		}

		// space between two rows
		if ($i < count($accounts) - 1) {
			$pdfFile->Ln(12);
		}
	}

	// Close PDF
	$pdfFile->Close();
	// Get relative url path
	$fullpath = realpath('.');
	$subdirs = explode('/', str_replace($_SESSION['lampath'], '', $fullpath));
	for ($i=0; $i<count($subdirs); $i++ ) $filename .= '../';
	// use timestamp and random number from ldap.inc as filename so it should be unique.
	$filename .= 'tmp/' . $_SESSION['ldap']->rand . time() .'.pdf';
	// Save PDF
	$pdfFile->Output($filename);
	// Output meta refresh to pdf-file
	metaRefresh($filename);
	// Return relative path of pdf-file
	return $filename;
}


// lamPDF class || For defining own a Header and Footer
class lamPDF extends FPDF {
	
	function lamPDF($account_type = "User") {
		// Call constructor of superclass
		$this->FPDF('P','mm','A4');
		
		// Decide which PDF file type we shall use
		switch($account_type) {
			case "User":
				$subject = _("User information page");
				break;
			case "Group":
				$subject = _("Group information page");
				break;
			case "Host":
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