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

define('FPDF_FONTPATH', 'font/');
include_once("fpdf.php");


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
		$iv = base64_decode($_COOKIE['IV']);
		$key = base64_decode($_COOKIE['Key']);
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
		$pdfFile->setFont("arial","B",12);
		$pdfFile->Write(5,"- " . _("User Information") . ":");
		$pdfFile->Ln(6);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,$info_string,0,1,"L",0);
		$pdfFile->Ln(9);

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
			$account->unix_password = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($account->unix_password), MCRYPT_MODE_ECB, $iv);
			$account->unix_password = str_replace(chr(00), '', $account->unix_password);
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
		if($account->smb_password_no == "1") {
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
			$account->smb_password = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($account->smb_password), MCRYPT_MODE_ECB, $iv);
			$account->smb_password = str_replace(chr(00), '', $account->smb_password);
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
		$pdfFile->Cell(50,5,$account->smb_home,0,1,"L",0);
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
	// Print page header
	function header() {
		$imageFile = substr(__FILE__,0,strlen(__FILE__)- 11) . "graphics/printLogo.jpg";
		$this->Image($imageFile,10,10,50,20,"JPG");
		$this->SetFont("arial","B",22);
		$this->Cell(170,5,"LDAP Account Manager",0,1,"R",0);
		$this->Ln(3);
		$this->SetFont("times","",14);
		$this->Cell(170,5,"- " . _("User information page") . " -",0,0,"R",0);
		$this->SetLineWidth(0.8);
		$this->Line(10,40,200,40);
		$this->Line(10,42,200,42);
		$this->SetY(60);
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


// lamHostPDF class || For defining own a Header and Footer
class lamHostPDF extends FPDF {
	// Print page header
	function header() {
		$imageFile = substr(__FILE__,0,strlen(__FILE__)- 11) . "graphics/printLogo.jpg";
		$this->Image($imageFile,10,10,50,20,"JPG");
		$this->SetFont("arial","B",22);
		$this->Cell(170,5,"LDAP Account Manager",0,1,"R",0);
		$this->Ln(3);
		$this->SetFont("times","",14);
		$this->Cell(170,5,"- " . _("Samba-Host information page") . " -",0,0,"R",0);
		$this->SetLineWidth(0.8);
		$this->Line(10,35,200,35);
		$this->Line(10,37,200,37);
		$this->SetY(45);
	}

	// Print page footer
	function footer() {
		$this->SetLineWidth(0.8);
		$this->Line(10,280,200,280);
		$this->Line(10,282,200,282);
		$this->SetY(285);
		$this->SetFont("arial","B",12);
		$this->Cell(0,5,_("This document was automatically created by LDAP Account Manager"),0,0,"C",0);
		$this->SetFont("arial","",12);
		$this->Cell(0,5,$this->PageNo(),0,1,"R",0);
	}
}


// lamGroupPDF class || For defining own Header and Footer
class lamGroupPDF extends FPDF {
	// Print page header
	function header() {
		$imageFile = substr(__FILE__,0,strlen(__FILE__)- 11) . "graphics/printLogo.jpg";
		$this->Image($imageFile,10,10,50,20,"JPG");
		$this->SetFont("arial","B",22);
		$this->Cell(170,5,"LDAP Account Manager",0,1,"R",0);
		$this->Ln(3);
		$this->SetFont("times","",14);
		$this->Cell(170,5,"- " . _("Group information page") . " -",0,0,"R",0);
		$this->SetLineWidth(0.8);
		$this->Line(10,35,200,35);
		$this->Line(10,37,200,37);
		$this->SetY(45);
	}

	// Print page footer
	function footer() {
		$this->SetLineWidth(0.8);
		$this->Line(10,280,200,280);
		$this->Line(10,282,200,282);
		$this->SetY(285);
		$this->SetFont("arial","B",12);
		$this->Cell(0,5,_("This document was automatically created by LDAP Account Manager"),0,0,"C",0);
		$this->SetFont("arial","",12);
		$this->Cell(0,5,$this->PageNo(),0,1,"R",0);
	}
}

