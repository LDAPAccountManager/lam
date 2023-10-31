<?php
namespace LAM\PDF;

use LAMException;
use PHPUnit\Framework\TestCase;

/*
  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2017 - 2023  Roland Gruber

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

include_once __DIR__ . '/../../lib/pdfstruct.inc';

/**
 * Tests classes in pdfstruct.inc.
 *
 * @author Roland Gruber
 *
 */
class PdfStructTest extends TestCase {

	/**
	 * Reads the sample structure.
	 * @throws LAMException error occurred
	 */
	public function testRead() {
		$file = $this->getTestFileName('test.xml');
		$fileHandle = fopen($file, "r");
		$originalXML = fread($fileHandle, 1_000_000);
		fclose($fileHandle);
		$reader = new PDFStructureReader();
		$structure = $reader->read($originalXML);

		$this->assertEquals('printLogo.jpg', $structure->getLogo());
		$this->assertEquals('User information', $structure->getTitle());
		$this->assertEquals(PDFStructure::FOLDING_STANDARD, $structure->getFoldingMarks());
		$sections = $structure->getSections();
		$this->assertEquals(4, sizeof($sections));
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
		// check fourth section
		$this->assertInstanceOf(PDFEntrySection::class, $sections[3]);
		$this->assertFalse($sections[3]->isAttributeTitle());
		$this->assertEquals('No entries', $sections[3]->getTitle());
		$entries = $sections[3]->getEntries();
		$this->assertEquals(0, sizeof($entries));
	}

	/**
	 * Returns the full path to the given file name.
	 *
	 * @param string $file file name
	 * @return string file name
	 */
	private function getTestFileName($file): string {
		return dirname(__DIR__) . '/resources/pdf/' . $file;
	}

	/**
	 * Tests if the output is the same as the original PDF.
	 * @throws LAMException error occurred
	 */
	public function testWrite() {
		$file = $this->getTestFileName('writer.xml');
		// read input XML
		$fileHandle = fopen($file, "r");
		$originalXML = fread($fileHandle, 1_000_000);
		fclose($fileHandle);
		// read structure
		$reader = new PDFStructureReader();
		$structure = $reader->read($originalXML);
		// create writer and get output XML
		$writer = new PDFStructureWriter();
		$xml = $writer->getXML($structure);
		// compare
		$this->assertEquals($originalXML, $xml);
	}

	/**
	 * Tests PDFTextSection
	 */
	public function testExportPDFTextSection() {
		$section = new PDFTextSection('sometext');

		$data = $section->export();

		$this->assertEquals('sometext', $data);
	}

	/**
	 * Tests PDFEntrySection
	 */
	public function testExportPDFEntrySection() {
		$section = new PDFEntrySection('mytitle');
		$section->setEntries([new PDFSectionEntry('key1'), new PDFSectionEntry('key2')]);

		$data = $section->export();

		$expected = [
			'title' => 'mytitle',
			'entries' => ['key1', 'key2']
		];

		$this->assertEquals($expected, $data);
	}

	/**
	 * Tests PDFStructure
	 */
	public function testExportPDFStructure() {
		$structure = new PDFStructure();
		$structure->setFoldingMarks(PDFStructure::FOLDING_STANDARD);
		$structure->setLogo('somelogo');
		$structure->setTitle('mytitle');
		$entrySection = new PDFEntrySection('sometitle');
		$entrySection->setEntries([new PDFSectionEntry('key1')]);
		$structure->setSections([
			new PDFTextSection('sometext'),
			$entrySection
		]);

		$data = $structure->export();

		$expected = [
			'title' => 'mytitle',
			'foldingMarks' => PDFStructure::FOLDING_STANDARD,
			'logo' => 'somelogo',
			'sections' => [
				[
					'type' => 'text',
					'data' => 'sometext'
				],
				[
					'type' => 'entry',
					'data' => [
						'title' => 'sometitle',
						'entries' => ['key1']
					]
				]
			]
		];

		$this->assertEquals($expected, $data);
	}

	/**
	 * Tests import in PDFEntrySection.
	 */
	public function testImportPDFEntrySection() {
		$data = [
			'title' => 'mytitle',
			'entries' => ['e1', 'e2']
		];

		$section = new PDFEntrySection(null);
		$section->import($data);

		$this->assertEquals('mytitle', $section->getTitle());
		$entries = $section->getEntries();
		$this->assertEquals(2, sizeof($entries));
		$this->assertEquals('e1', ($entries[0]->getKey()));
		$this->assertEquals('e2', ($entries[1]->getKey()));
	}

	/**
	 * Tests the import in PDFStructure.
	 */
	public function testImportPDFStructure() {
		$data = [
			'title' => 'mytitle',
			'foldingMarks' => PDFStructure::FOLDING_STANDARD,
			'logo' => 'logo',
			'sections' => [
				[
					'type' => 'text',
					'data' => 'textvalue'
				],
				[
					'type' => 'entry',
					'data' => [
						'title' => 'etitle',
						'entries' => ['e1', 'e2']
					]
				],
			]
		];

		$structure = new PDFStructure();
		$structure->import($data);

		$this->assertEquals('mytitle', $structure->getTitle());
		$this->assertEquals(PDFStructure::FOLDING_STANDARD, $structure->getFoldingMarks());
		$this->assertEquals('logo', $structure->getLogo());
		$sections = $structure->getSections();
		$this->assertEquals(2, sizeof($sections));
		$this->assertTrue($sections[0] instanceof PDFTextSection);
		$this->assertEquals('textvalue', $sections[0]->getText());
		$this->assertTrue($sections[1] instanceof PDFEntrySection);
		$entries = $sections[1]->getEntries();
		$this->assertEquals(2, sizeof($entries));
		$this->assertEquals('e1', $entries[0]->getKey());
		$this->assertEquals('e2', $entries[1]->getKey());
	}

}
