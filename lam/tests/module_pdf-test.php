#!/usr/bin/php -q
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


  LDAP Account Manager module PDF test
*/

include_once('../lib/pdf.inc');

//print_r(getStructure());
//print_r(processLine("<block><key>Test key</key><value><b><i>Test value</i></b></value></block>"));
//print_r(processLine("<block><p><b>Test p</b></p><br /></block>"));
	
	$entries = array("Surname" => array("<block><key>Test key</key><value>Test value</value></block>"),"Given name" => array("<block><p><b>Test p</b></p></block>"),"User quotas" => array("<block><key>User quotas</key><td width=\"30\" align=\"L\"><b>Mountpoint</b></td><td width=\"30\" align=\"L\"><b>Soft block</b></td><td width=\"30\" align=\"L\"><b>Soft inode</b></td><td width=\"30\" align=\"L\"><b>Hard block</b></td><td width=\"30\" align=\"L\"><b>Hard inode</b></td></block>","<block><td width=\"30\" align=\"L\">/usr</td><td width=\"30\" align=\"L\">10</td><td width=\"30\" align=\"L\">100</td><td width=\"30\" align=\"L\">15</td><td width=\"30\" align=\"L\">150</td></block>"));
	$structure = getStructure(array("User"));
	$structure = $structure['User'];

	$pdf = new LamPDF("User");
	
	// Loop over each account and add a new page in the PDF file for it 
		// Start a new page for each account
		$pdf->AddPage();
		
		// Get PDF entries for the current account
		//$entries = $account->get_pdfEntries($account_type);
		
		// Now create the PDF file acording to the structure with the submitted values 
		foreach($structure as $entry) {
			// We have a new section to start
			$name = $entry['attributes']['NAME'];
			if($entry['tag'] == "SECTION" && $entry['type'] == "open") {
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
				
			}
			// We have to include an entry from the account
			elseif($entry['tag'] == "ENTRY") {
				// Get current entry
				$entry = $entries[$name];
				
				// Loop over all rows of this entry (most of the time this will be just one)
				if($entry != null) {
				foreach($entry as $line) {
					// Substitue XML syntax with valid FPDF methods
					$methods = processLine($line);
					// Call every method
					foreach($methods as $method) {
						call_user_method_array	($method[0],$pdf,$method[1]);
					}
				}
				$key = false;
				}
			}
		}
		$pdf->Close();
		$pdf->Output('/home/md/workspace/lam/tests/test.pdf','F');
?>