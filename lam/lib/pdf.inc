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


  LDAP Account Manager lamPDF class and some pdf functions
*/

define('FPDF_FONTPATH', 'font/');
include_once("fpdf.php");

function createUserPDF($accounts) {
// Check if all submitted accounts are user accounts
	for($i=0;$i<count($accounts);$i++) {
		$account = $accounts[$i];
		if($account->type <> "user") {
			return 2; // 2 means non user account submitted for createUserPDF()
		}
	}
// Start PDF file
	$pdfFile = new lamPDF();
	$pdfFile->Open();
// Write some information into the PDF file
	$pdfFile->setFont("arial","",12);
	$pdfFile->setTitle("LDAP Account Manager");
	$pdfFile->setSubject(_("User information page"));
	$pdfFile->setAuthor("LDAP Account Manager Devel-Team -Michael Dürgner-");
	$pdfFile->setCreator("LDAP Account Manager (pdf.inc)");
// Loop for every sumbitted account and print its values on a extra page
	for($i=0;$i<count($accounts);$i++) {
		$account = $accounts[$i];
		$iv = base64_decode($_COOKIE['IV']);
		$key = base64_decode($_COOKIE['Key']);
		$pdfFile->addPage();
// Print General settings
		$pdfFile->setFont("arial","B",12);
		$pdfFile->Write(5,"- " . _("General User Settings") . ":");
		$pdfFile->Ln(6);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,_("Surname") . ":",0,0,"R",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,$account->general_surname,0,1,"L",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,_("Given name") . ":",0,0,"R",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,$account->general_givenname,0,1,"L",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,_("Username") . ":",0,0,"R",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,$account->general_username,0,1,"L",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,_("Home Directory") . ":",0,0,"R",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,$account->general_homedir,0,1,"L",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,_("Group(s)") . ":",0,0,"R",0);
		$text = $account->general_group;
		$groups = $account->general_groupadd;
		for($j=0;$j<count($groups);$j++) {
			$text .= ", " . $groups[$j];
		}
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,$text,0,1,"L",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,_("Login Shell(s)") . ":",0,0,"R",0);
		$text = "";
		$shells = $account->general_shell;
		for($j=0;$j<count($shells);$j++) {
			if($j == 0 || count($shells) == 1) {
				$text = $shells[$j];
			}
			else {
				$text .= ", " . $shells[$j];
			}
		}
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,$text,0,1,"L",0);
		$pdfFile->Ln(9);
		$pdfFile->setFont("arial","B",12);
// Print Unix settings
		$pdfFile->Write(5,"- " . _("Unix User Settings") . ":");
		$pdfFile->Ln(6);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,_("Unix password") . ":",0,0,"R",0);
		if($account->unix_password_no == "1") {
			$pdfFile->setFont("times","B",10);
			$pdfFile->Cell(50,5,_("Unix password disabled!"),0,1,"L",0);
		}
		else {
			$account->unix_password = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($account->unix_password), MCRYPT_MODE_ECB, $iv);
			$account->unix_password = str_replace(chr(00), '', $account->unix_password);
			$pdfFile->setFont("times","B",10);
			$pdfFile->Cell(50,5,$account->unix_password,0,1,"L",0);
		}
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,_("Password expire") . ":",0,0,"R",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,$account->unix_pwdexpire,0,1,"L",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,_("Login at host(s)") . ":",0,0,"R",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,$account->unix_host,0,1,"L",0);
		$pdfFile->Ln(9);
		$pdfFile->setFont("arial","B",12);
// Print Samba settings
		$pdfFile->Write(5,"- " . _("Samba User Settings") . ":");
		$pdfFile->Ln(6);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,_("Samba password") . ":",0,0,"R",0);
		if($account->smb_password_no == "1") {
			$pdfFile->setFont("times","B",10);
			$pdfFile->Cell(50,5,_("Samba password disabled!"),0,1,"L",0);
		}
		elseif($account->smb_useunixpwd == "1") {
			$pdfFile->setFont("times","B",10);
			$pdfFile->Cell(50,5,_("Samba password set to unix password."),0,1,"L",0);
		}
		else {
			$account->smb_password = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($account->smb_password), MCRYPT_MODE_ECB, $iv);
			$account->smb_password = str_replace(chr(00), '', $account->smb_password);
			$pdfFile->setFont("times","B",10);
			$pdfFile->Cell(50,5,$account->smb_password,0,1,"L",0);
		}
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,_("Home drive") . ":",0,0,"R",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,$account->smb_homedrive,0,1,"L",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,_("Script path") . ":",0,0,"R",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,$account->smb_scriptPath,0,1,"L",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,_("Profile path") . ":",0,0,"R",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,$account->smb_profilePath,0,1,"L",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,_("Login at workstation(s)") . ":",0,0,"R",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,$account->smb_smbuserworkstations,0,1,"L",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,_("Samba home directory") . "::",0,0,"R",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,$account->smb_home,0,1,"L",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,_("Samba Domain") . ":",0,0,"R",0);
		$pdfFile->setFont("times","B",10);
		if($_SESSION['config']->get_samba3() == "yes") {
			$account->smb_domain = $account->smb_domain->name;
		}
		$pdfFile->Cell(50,5,$account->smb_domain,0,1,"L",0);
		$pdfFile->Ln(9);
		$pdfFile->setFont("arial","B",12);
// Print Quota settings
		$pdfFile->Write(5,"- " . _("Quota Settings") . ":");
		$pdfFile->Ln(6);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,_("User quota(s)") . ":",0,0,"R",0);
		$pdfFile->setFont("times","B",10);
		$quotas = $account->quota;
		if(count($quota)>0) {
			$pdfFile->Cell(20,5,_("Mountpoint"),0,0,"L",0);
			$pdfFile->Cell(20,5,_("Soft block"),0,0,"L",0);
			$pdfFile->Cell(20,5,_("Soft inode"),0,0,"L",0);
			$pdfFile->Cell(20,5,_("Hard block"),0,0,"L",0);
			$pdfFile->Cell(20,5,_("Hard inode"),0,1,"L",0);
			for($j=0;$j<count($quotas);$j++) {
				$pdfFile->Cell(50,5,"",0,0,"R",0);
				$pdfFile->Cell(20,5,$quotas[$j][0],0,0,"L",0);
				$pdfFile->Cell(20,5,$quotas[$j][2],0,0,"L",0);
				$pdfFile->Cell(20,5,$quotas[$j][3],0,0,"L",0);
				$pdfFile->Cell(20,5,$quotas[$j][6],0,0,"L",0);
				$pdfFile->Cell(20,5,$quotas[$j][7],0,1,"L",0);
			}
		}
		else {
			$pdfFile->Cell(50,5,_("No user quotas defined or no quota support by filesystem."),0,1,"L",0);
		}
		$pdfFile->Ln(9);
		$pdfFile->setFont("arial","B",12);
// Print Personal settings
		$pdfFile->Write(5,"- " . _("Personal User Settings") . ":");
		$pdfFile->Ln(6);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,_("Title") . ":",0,0,"R",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,$account->personal_title,0,1,"L",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,_("E-Mail") . "::",0,0,"R",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,$account->personal_mail,0,1,"L",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,_("Phone") . ":",0,0,"R",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,$account->personal_telephoneNumber,0,1,"L",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,_("Cellular phone") . ":",0,0,"R",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,$account->personal_mobileTelephoneNumber,0,1,"L",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,_("Facsimile") . ":",0,0,"R",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,$account->personal_facsimileTelephoneNumber,0,1,"L",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,_("Street") . ":",0,0,"R",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,$account->personal_street,0,1,"L",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,_("Postal code") . "::",0,0,"R",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,$account->personal_postalCode,0,1,"L",0);
		$pdfFile->setFont("times","",10);
		$pdfFile->Cell(50,5,_("Postal address") . ":",0,0,"R",0);
		$pdfFile->setFont("times","B",10);
		$pdfFile->Cell(50,5,$account->personal_postalAddress,0,1,"L",0);
	}
// Close document and send it to the browser
	$pdfFile->Close();
	$pdfFile->Output();
	return 0; // 0 means everything successful; page(s) printed
}

// lamPDF class || For defining own a Header and Footer
class lamPDF extends FPDF {
	// Print page header
	function header() {
		$this->Image("../../graphics/printLogo.jpg",10,10,50,20,"JPG");
		$this->SetFont("arial","B",22);
		$this->Cell(170,5,"LDAP Account Manager",0,1,"R",0);
		$this->Ln(3);
		$this->SetFont("times","",14);
		$this->Cell(170,5,"-" . _("User information page") . "-",0,0,"R",0);
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
		$this->Cell(0,5,_("This document was automatically created by LDAP Account Manager") . " (lib/pdf.inc)",0,0,"C",0);
	}
}
