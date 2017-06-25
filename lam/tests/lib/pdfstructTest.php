<?php
use LAM\PDF\PDFTextSection;
use LAM\PDF\PDFEntrySection;
use LAM\PDF\PDFStructureReader;
use LAM\PDF\PDFStructure;

/*
  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2017  Roland Gruber

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

include_once 'lam/lib/pdfstruct.inc';

/**
 * Reads a sample PDF structure.
 *
 * @author Roland Gruber
 *
 */
class ReadStructureTest extends PHPUnit_Framework_TestCase {

	/**
	 * Reads the sample structure.
	 */
	public function testRead() {
		$reader = $this->getMockBuilder('\LAM\PDF\PDFStructureReader')
			->setMethods(array('getFileName'))
			->getMock();
		$reader->method('getFileName')->willReturn($this->getTestFileName('test.xml'));

		$structure = $reader->read('type', 'name');
		$this->assertEquals('printLogo.jpg', $structure->getLogo());
		$this->assertEquals('User information', $structure->getTitle());
		$this->assertEquals(PDFStructure::FOLDING_STANDARD, $structure->getFoldingMarks());
		$sections = $structure->getSections();
		$this->assertEquals(3, sizeof($sections));
		// check first section
		$this->assertInstanceOf(PDFEntrySection::class, $sections[0]);
		$this->assertFalse($sections[0]->isAttributeTitle());
		$this->assertEquals('Personal user information', $sections[0]->getTitle());
		$entries = $sections[0]->getEntries();
		$this->assertEquals(3, sizeof($entries));
		$this->assertEquals('inetOrgPerson_givenName', $entries[0]->getKey());
		$this->assertEquals('inetOrgPerson_sn', $entries[1]->getKey());
		$this->assertEquals('inetOrgPerson_street', $entries[2]->getKey());
		// check text section
		$this->assertInstanceOf(PDFTextSection::class, $sections[1]);
		$this->assertEquals('test text', $sections[1]->getText());
		// check third section
		$this->assertInstanceOf(PDFEntrySection::class, $sections[2]);
		$this->assertTrue($sections[2]->isAttributeTitle());
		$this->assertEquals('posixAccount_uid', $sections[2]->getPdfKey());
		$entries = $sections[2]->getEntries();
		$this->assertEquals(2, sizeof($entries));
		$this->assertEquals('posixAccount_homeDirectory', $entries[0]->getKey());
		$this->assertEquals('posixAccount_loginShell', $entries[1]->getKey());
	}

	/**
	 * Returns the full path to the given file name.
	 *
	 * @param string $file file name
	 */
	private function getTestFileName($file) {
		return dirname(dirname(__FILE__)) . '/resources/pdf/' . $file;
	}

}

?>