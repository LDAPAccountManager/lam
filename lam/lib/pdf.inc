<?
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Tilo Lutz

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


  LDAP Account Manager functions used by account.php
*/

// Include PDF-Class
define('FPDF_FONTPATH', 'font/');
include_once('fpdf.php');

function createpdf($accounts) { // Will output a pdf-file with accounts
	$pdf= new FPDF();
	$pdf->Open();
	$pdf->SetAuthor('Tilo Lutz');
	$pdf->SetCreator('pdf.inc, LDAP Accunt Manager');
	for ( $i=0; $i<sizeof($accounts); $i++ ) { // Loop vor every account
		$pdf->AddPage();
		$pdf->SetFont('Arial', 'B', 16);
		$pdf->Cell(100,10,_('Hello World!'),0,1,'C');
		}

	$pdf->Output(); // Output PDF-File
	}


?>
