<?php
namespace LAM\PDF;

use \htmlStatusMessage;
use \LAMException;
use \LAM\ImageUtils\ImageManipulationFactory;
/*
  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Michael Duergner
                2011 - 2021  Roland Gruber

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
 * Functions to manage the PDF structures.
 *
 * @author Michael Duergner
 * @package PDF
 */

/** LAM configuration */
include_once(__DIR__ . "/config.inc");

/**
 * Use as server profile name to manage global templates.
 */
const GLOBAL_PROFILE = '__GLOBAL__';

/** LDAP object */
include_once(__DIR__ . "/ldap.inc");

/**
 * This function will return all available PDF structure definitions for the submitted
 * account type.
 *
 * @param string $typeId the account type
 * @param string $profile server profile name
 *
 * @return string[] All available PDF structure definitions for the submitted account
 * scope. Each entry is a string being the filename that may be passed to the
 * createModulePDF() function as second argument.
 */
function getPDFStructures($typeId, $profile) {
	$return = array();
	if (!preg_match('/[a-zA-Z]+/', $typeId)) {
		return null;
	}
	$path = dirname(__FILE__) . '/../config/pdf/' . $profile;
	if(is_dir($path)) {
		$dirHandle = opendir($path);
		while($file = readdir($dirHandle)) {
			$struct_file = explode('.',$file);
			if(!is_dir($path.$file) && ($file != '.') && ($file != '..') && (sizeof($struct_file) == 3) && ($struct_file[1] == $typeId) && ($struct_file[2] == 'xml')) {
				array_push($return, $struct_file[0]);
			}
		}
		sort($return);
	}
 	return $return;
}

/**
 * Deletes XML file with PDF structure definitions.
 *
 * @param string $typeId account type
 * @param string $name Name of definition to delete
 * @param string $serverProfileName server profile name
 * @return boolean True if file was deleted or false if a problem occurred.
 */
function deletePDFStructure($typeId, $name, $serverProfileName) {
	if (!isValidPDFStructureName($name) || !preg_match('/[a-zA-Z]+/',$typeId)) {
		return false;
	}
	$file = dirname(__FILE__) . '/../config/pdf/' . $serverProfileName . '/' . $name . '.' . $typeId . '.xml';
	if(is_file($file) && is_writable($file)) {
		return unlink($file);
	}
	else {
		return false;
	}

}

/**
 * This function returns an array with all aviliable logo images.
 *
 * @param string $serverProfileName server profile name
 * @return array list of logo files (array('filename' => PATH, 'infos' => array(width, height)))
 */
function getAvailableLogos($serverProfileName) {
	$return = array();
	$dirPath = dirname(__FILE__) . '/../config/pdf/' . $serverProfileName . '/logos/';
	if (!is_dir($dirPath)) {
		logNewMessage(LOG_WARNING, 'Unable to read ' . $dirPath);
		return $return;
	}
	$dirHandle = opendir($dirPath);
	if ($dirHandle === false) {
		logNewMessage(LOG_WARNING, 'Unable to read ' . $dirPath);
		return $return;
	}
	while ($file = readdir($dirHandle)) {
		if (!is_dir($file) && $file != '.' && $file != '..' && preg_match('/\\.(jpg|png)$/i',$file)) {
			include_once dirname(__FILE__) . '/imageutils.inc';
			$imageManipulator = ImageManipulationFactory::getImageManipulatorFromFile($dirPath . $file);
			$infos = array($imageManipulator->getWidth(), $imageManipulator->getHeight());
			$imageManipulator = null;
			array_push($return, array('filename' => $file, 'infos' => $infos));
		}
	}
	sort($return);
	return $return;
}

/**
 * Copies a PDF structure from the given source to target.
 *
 * @param \LAM\TYPES\ConfiguredType $sourceType source type
 * @param string $sourceStructureName structure name
 * @param \LAM\TYPES\ConfiguredType $targetType target type
 * @throws LAMException error during copy
 */
function copyStructure($sourceType, $sourceStructureName, $targetType) {
	if (!isValidPDFStructureName($sourceStructureName)) {
		throw new LAMException(_('Failed to copy'));
	}
	$sourceConfig = $sourceType->getTypeManager()->getConfig()->getName();
	$sourceTypeId = $sourceType->getId();
	$targetConfig = $targetType->getTypeManager()->getConfig()->getName();
	$targetTypeId = $targetType->getId();
	$basePath = dirname(__FILE__) . '/../config/pdf/';
	$src = $basePath . $sourceConfig . '/' . $sourceStructureName . '.' . $sourceTypeId . '.xml';
	$dst = $basePath . $targetConfig . '/' . $sourceStructureName . '.' . $targetTypeId . '.xml';
	if (!@copy($src, $dst)) {
		throw new LAMException(_('Failed to copy'), $sourceConfig . ': ' . $sourceStructureName);
	}
}

/**
 * Copies a PDF structure from the given source to global templates.
 *
 * @param \LAM\TYPES\ConfiguredType $sourceType source type
 * @param string $sourceName structure name
 * @throws Exception
 */
function copyStructureToTemplates($sourceType, $sourceName) {
	if (!isValidPDFStructureName($sourceName)) {
		throw new LAMException(_('Failed to copy'));
	}
	$sourceConfig = $sourceType->getTypeManager()->getConfig()->getName();
	$sourceTypeId = $sourceType->getId();
	$basePath = __DIR__ . '/../config/pdf/';
	$templatePath = __DIR__ . '/../config/templates/pdf/';
	$src = $basePath . $sourceConfig . '/' . $sourceName . '.' . $sourceTypeId . '.xml';
	$dst = $templatePath . $sourceName . '.' . $sourceType->getScope() . '.xml';
	if (!@copy($src, $dst)) {
		throw new LAMException(_('Failed to copy'), $sourceConfig . ': ' . $sourceName);
	}
}

/**
 * Uploads a PDF logo file for the current server profile.
 *
 * @param String $file full path of temporary file
 * @param String $name file name
 * @param string $serverProfileName server profile name
 * @return htmlStatusMessage status message to display
 */
function uploadPDFLogo($file, $name, $serverProfileName) {
	if (!preg_match('/[a-zA-Z0-9_-]+\\.(png)|(jpg)/i', $name)) {
		return new htmlStatusMessage('ERROR', _('Unable to upload logo file.'), _('The file name must end with ".png" or ".jpg".'));
	}
	if ($serverProfileName === GLOBAL_PROFILE) {
		$serverProfileName = '../templates/pdf';
	}
	$dirPath = dirname(__FILE__) . '/../config/pdf/' . $serverProfileName . '/logos/';
	$success = copy($file, $dirPath . '/' . $name);
	if ($success) {
		return new htmlStatusMessage('INFO', _('Uploaded logo file.'), $name);
	}
	else {
		return new htmlStatusMessage('ERROR', _('Unable to upload logo file.'), $name);
	}
}

/**
 * Deletes a PDF logo file.
 *
 * @param String $name file name
 * @param string $serverProfileName server profile name
 * @return StatusMessage status message to display
 */
function deletePDFLogo($name, $serverProfileName) {
	// check if valid file
	$found = false;
	$logos = getAvailableLogos($serverProfileName);
	foreach ($logos as $logo) {
		if ($logo['filename'] === $name) {
			$found = true;
			break;
		}
	}
	if (!$found) {
		return new htmlStatusMessage('ERROR', _('File does not exist.'), htmlspecialchars($name));
	}
	// check if still in use
	$typeManager = new \LAM\TYPES\TypeManager();
	$activeTypes = $typeManager->getConfiguredTypes();
	$reader = new PDFStructureReader($serverProfileName);
	foreach ($activeTypes as $type) {
		$structures = getPDFStructures($type->getId(), $serverProfileName);
		foreach ($structures as $structure) {
			try {
				$data = $reader->read($type->getId(), $structure);
				if ($data->getLogo() == $name) {
					return new htmlStatusMessage('ERROR', _('Unable to delete logo file.'),
						sprintf(_('Logo is still in use by PDF structure "%s" in account type "%s".'), $structure, $type->getAlias()));
				}
			}
			catch (LAMException $e) {
				logNewMessage(LOG_ERR, 'Error reading PDF file ' . $e->getMessage());
			}
		}
	}
	// delete file
	$success = @unlink(getPdfLogoFileName($serverProfileName, $name));
	if ($success) {
		return new htmlStatusMessage('INFO', _('Logo file deleted.'), $name);
	}
	return new htmlStatusMessage('ERROR', _('Unable to delete logo file.'), $name);
}

/**
 * Returns if the give structure name is valid.
 *
 * @param string $name structure name
 * @return boolean is valid
 */
function isValidPDFStructureName($name) {
	return preg_match('/^[a-z0-9\-\_]+$/i',$name) === 1;
}

/**
 * Installs template structures to the current server profile.
 *
 * @param string $confName server profile name
 */
function installPDFTemplates(string $confName) {
	$templatePath = __DIR__ . '/../config/templates/pdf';
	$allTemplates = getPdfTemplateNames();
	$basePath = dirname(__FILE__) . '/../config/pdf/' . $confName;
	if (!file_exists($basePath)) {
		mkdir($basePath, 0700, true);
	}
	$typeManager = new \LAM\TYPES\TypeManager();
	foreach ($typeManager->getConfiguredTypes() as $type) {
		if (empty($allTemplates[$type->getScope()])) {
			continue;
		}
		foreach ($allTemplates[$type->getScope()] as $templateName) {
			$path = $basePath . '/' . $templateName . '.' . $type->getId() . '.xml';
			if (!is_file($path)) {
				$template = $templatePath . '/' . $templateName . '.' . $type->getScope() . '.xml';
				logNewMessage(LOG_DEBUG, 'Copy template ' . $template . ' to ' . $path);
				@copy($template, $path);
			}
		}
	}
	if (!file_exists($basePath . '/logos')) {
		mkdir($basePath . '/logos');
	}
	$logos = getPdfTemplateLogoNames();
	foreach ($logos as $logo) {
		$path = getPdfLogoFileName($confName, $logo);
		$template = $templatePath . '/logos/' . $logo;
		if (!is_file($path)) {
			logNewMessage(LOG_DEBUG, 'Copy template ' . $template . ' to ' . $path);
			@copy($template, $path);
		}
	}
}

/**
 * Returns all PDF template names.
 *
 * @return array names (array('user' => array('default')))
 */
function getPdfTemplateNames() {
	$templatePath = __DIR__ . '/../config/templates/pdf';
	$templateDir = @dir($templatePath);
	$allTemplates = array();
	if ($templateDir) {
		$entry = $templateDir->read();
		while ($entry){
			$parts = explode('.', $entry);
			if ((strlen($entry) > 3) && (sizeof($parts) == 3)) {
				$name = $parts[0];
				$scope = $parts[1];
				$allTemplates[$scope][] = $name;
			}
			$entry = $templateDir->read();
		}
	}
	return $allTemplates;
}

/**
 * Deletes a PDF structure template.
 *
 * @param string $profile name of the profile (without .<scope> extension)
 * @param string $scope account type
 * @throws LAMException error deleting template
 */
function deleteTemplateStructure(string $profile, string $scope) {
	if (!isValidPDFStructureName($profile) || !preg_match("/^[a-z0-9_]+$/i", $scope) || ($profile == 'default')) {
		logNewMessage(LOG_NOTICE, "Invalid PDF profile name: $profile:$scope");
		throw new LAMException(_("Unable to delete PDF structure!"));
	}
	$fileName = __DIR__ . '/../config/templates/pdf/' . $profile . '.' . $scope . '.xml';
	$deleted = @unlink($fileName);
	if (!$deleted) {
		throw new LAMException(_("Unable to delete PDF structure!"));
	}
}

/**
 * Returns all PDF template logo names.
 *
 * @return array names (array('user' => array('default.png')))
 */
function getPdfTemplateLogoNames() {
	$templatePath = __DIR__ . '/../config/templates/pdf/logos';
	$templateDir = @dir($templatePath);
	$logos = array();
	if ($templateDir) {
		$entry = $templateDir->read();
		while ($entry){
			if ((strpos($entry, '.') !== 0) && is_file($templatePath . '/' . $entry)) {
				$logos[] = $entry;
			}
			$entry = $templateDir->read();
		}
	}
	return $logos;
}

/**
 * Returns the binary data of the PDF template logo.
 *
 * @param string $name file name (without path)
 * @return string binary
 */
function getPdfTemplateLogoBinary($name) {
	$templatePath = __DIR__ . '/../config/templates/pdf/logos';
	$fileName = $templatePath . '/' . $name;
	$handle = fopen($fileName, 'r');
	$logoBinary = fread($handle, 100000000);
	fclose($handle);
	return $logoBinary;
}

/**
 * Reads the binary data of a given logo.
 *
 * @param string $confName server profile name
 * @param string $name logo name
 * @return string binary data
 */
function getPdfLogoBinary(string $confName, string $name) : string {
	$fileName = getPdfLogoFileName($confName, $name);
	$handle = fopen($fileName, 'r');
	$logoBinary = fread($handle, 100000000);
	fclose($handle);
	return $logoBinary;
}

/**
 * Returns the file name of a given logo.
 *
 * @param string $confName server profile name
 * @param string $name logo name
 * @return string file name
 */
function getPdfLogoFileName(string $confName, string $name) : string {
	return __DIR__ . '/../config/pdf/' . $confName . '/logos/' . $name;
}

/**
 * Reads a PDF structure.
 *
 * @author Roland Gruber
 */
class PDFStructureReader {

	private $serverProfileName;

	/**
	 * Constructor.
	 *
	 * @param $serverProfileName server profile name
	 */
	public function __construct($serverProfileName) {
		if ($serverProfileName === GLOBAL_PROFILE) {
			$this->serverProfileName = '../templates/pdf';
		}
		else {
			$this->serverProfileName = $serverProfileName;
		}
	}

	/**
	 * Reads a PDF structure.
	 *
	 * @param string $typeId type id
	 * @param string $name structure name
	 * @return PDFStructure structure
	 */
	public function read($typeId, $name) {
		if (!isValidPDFStructureName($name) || !preg_match('/[a-zA-Z]+/', $typeId)) {
			return null;
		}
		$file = $this->getFileName($typeId, $name);
		return $this->readPDFFile($file);
	}

	/**
	 * Returns the file name for the given structure.
	 *
	 * @param string $typeId type id
	 * @param string $name structure name
	 * @return string file name
	 */
	protected function getFileName($typeId, $name) {
		return dirname(__FILE__) . '/../config/pdf/' . $this->serverProfileName . '/' . $name . '.' . $typeId . '.xml';
	}

	/**
	 * Reads a PDF structure file.
	 *
	 * @param string $file file name
	 * @return PDFStructure structure
	 */
	private function readPDFFile($file) {
		logNewMessage(LOG_DEBUG, $file);
		$xml = new \XMLReader();
		$xml->open($file);
		$structure = new PDFStructure();
		// open <pdf>
		@$xml->read();
		if (!$xml->name == 'pdf') {
			logNewMessage(LOG_ERR, 'Unknown tag name: ' . $xml->name);
			throw new \LAMException(_('Unable to read PDF structure.'));
		}
		$structure->setLogo($xml->getAttribute('filename'));
		$structure->setTitle($xml->getAttribute('headline'));
		$structure->setFoldingMarks($xml->getAttribute('foldingmarks'));
		$sections = array();
		while ($xml->read()) {
			if (($xml->nodeType === \XMLReader::SIGNIFICANT_WHITESPACE)
					|| (($xml->name === 'pdf') && ($xml->nodeType == \XMLReader::END_ELEMENT))) {
				continue;
			}
			elseif ($xml->name === 'text') {
				$xml->read();
				$sections[] = new PDFTextSection($xml->value);
				$xml->read();
				if (!$xml->name === 'text') {
					logNewMessage(LOG_ERR, 'Unexpected tag name: ' . $xml->name);
					throw new \LAMException(_('Unable to read PDF structure.'));
				}
			}
			elseif ($xml->name === 'section') {
				$sections[] = $this->readSection($xml);
			}
			else {
				logNewMessage(LOG_ERR, 'Unexpected tag name: ' . $xml->name . ' in ' . $file);
				throw new \LAMException(_('Unable to read PDF structure.'));
			}
		}
		$xml->close();
		$structure->setSections($sections);
		return $structure;
	}

	/**
	 * Reads a single section from XML.
	 *
	 * @param \XMLReader $xml reader
	 */
	private function readSection(&$xml) {
		$section = new PDFEntrySection($xml->getAttribute('name'));
		$entries = array();
		while ($xml->read()) {
			if (($xml->name === 'section') && ($xml->nodeType == \XMLReader::END_ELEMENT)) {
				break;
			}
			elseif (($xml->nodeType === \XMLReader::END_ELEMENT)
					|| ($xml->nodeType === \XMLReader::SIGNIFICANT_WHITESPACE)) {
				continue;
			}
			elseif ($xml->name === 'entry') {
				$entries[] = new PDFSectionEntry($xml->getAttribute('name'));
			}
			elseif (!$xml->name === 'entry') {
				logNewMessage(LOG_ERR, 'Unexpected tag name: ' . $xml->name);
				throw new \LAMException(_('Unable to read PDF structure.'));
			}
		}
		$section->setEntries($entries);
		return $section;
	}

}

/**
 * Writes PDF structures to files.
 *
 * @author Roland Gruber
 */
class PDFStructureWriter {

	private $serverProfileName;

	/**
	 * Constructor.
	 *
	 * @param string $serverProfileName server profile name
	 */
	public function __construct($serverProfileName) {
		if ($serverProfileName === GLOBAL_PROFILE) {
			$this->serverProfileName = '../templates/pdf';
		}
		else {
			$this->serverProfileName = $serverProfileName;
		}
	}

	/**
	 * Writes the PDF structure to disk.
	 *
	 * @param string $typeId type ID
	 * @param string $name structure name
	 * @param PDFStructure $structure structure
	 * @throws LAMException error during write
	 */
	public function write($typeId, $name, $structure) {
		$fileName = $this->getFileName($typeId, $name);
		$xml = $this->getXML($structure);
		$this->writeXML($xml, $fileName);
	}

	/**
	 * Writes the PDF structure to disk.
	 *
	 * @param string $typeId type ID
	 * @param string $name structure name
	 * @return string file name
	 * @throws LAMException file not valid or not writable
	 */
	protected function getFileName($typeId, $name) {
		if (!isValidPDFStructureName($name) || !preg_match('/[a-zA-Z]+/', $typeId)) {
			throw new \LAMException(_('PDF structure name not valid'),
					_('The name for that PDF-structure you submitted is not valid. A valid name must consist of the following characters: \'a-z\',\'A-Z\',\'0-9\',\'_\',\'-\'.'));
		}
		$baseDir = __DIR__ . '/../config/pdf/' . $this->serverProfileName;
		if(!is_writable($baseDir)) {
			throw new \LAMException(sprintf(_('Could not save PDF structure, access denied to %s.'), $baseDir));
		}
		return $baseDir . '/' . $name . '.' . $typeId . '.xml';
	}

	/**
	 * Returns the generated XML.
	 *
	 * @param PDFStructure $structure structure
	 * @return string XML
	 */
	public function getXML($structure) {
		$writer = new \XMLWriter();
		$writer->openMemory();
		$writer->setIndent(true);
		$writer->setIndentString("\t");
		$writer->startElement('pdf');
		$writer->writeAttribute('filename', $structure->getLogo());
		$writer->writeAttribute('headline', $structure->getTitle());
		$writer->writeAttribute('foldingmarks', $structure->getFoldingMarks());
		foreach ($structure->getSections() as $section) {
			if ($section instanceof PDFTextSection) {
				$writer->startElement('text');
				$writer->text($section->getText());
				$writer->endElement();
			}
			else {
				$writer->startElement('section');
				if ($section->isAttributeTitle()) {
					$writer->writeAttribute('name', '_' . $section->getPdfKey());
				}
				else {
					$writer->writeAttribute('name', $section->getTitle());
				}
				foreach ($section->getEntries() as $entry) {
					$writer->startElement('entry');
					$writer->writeAttribute('name', $entry->getKey());
					$writer->endElement();
				}
				$writer->endElement();
			}
		}
		$writer->endElement();
		return $writer->outputMemory();
	}

	/**
	 * Writes the XML to the given file.
	 *
	 * @param string $xml XML
	 * @param string $file file name
	 * @throws LAMException error during write
	 */
	protected function writeXML($xml, $file) {
		$handle = @fopen($file,'w');
		if (!$handle) {
			throw new \LAMException(_('Could not save PDF structure, access denied.'));
		}
		fwrite($handle, $xml);
		fclose($handle);
	}

}

/**
 * PDF structure
 *
 * @author Roland Gruber
 */
class PDFStructure {

	/** no folding marks */
	const FOLDING_NONE = 'no';
	/** standard folding marks */
	const FOLDING_STANDARD = 'standard';

	private $logo = null;

	private $title = 'LDAP Account Manager';

	private $foldingMarks = 'no';

	private $sections = array();

	/**
	 * Returns an array representation of the structure.
	 *
	 * @return array export data
	 */
	public function export() {
		$data = array();
		$data['title'] = $this->title;
		$data['foldingMarks'] = $this->foldingMarks;
		$data['logo'] = $this->logo;
		$data['sections'] = array();
		foreach($this->sections as $section) {
			$type = ($section instanceof PDFTextSection) ? 'text' : 'entry';
			$sectionData = $section->export();
			$data['sections'][] = array(
				'type' => $type,
				'data' => $sectionData
			);
		}
		return $data;
	}

	/**
	 * Imports an array representation of the structure.
	 *
	 * @param array $data import data
	 */
	public function import($data) {
		if (isset($data['title'])) {
			$this->title = $data['title'];
		}
		if (isset($data['foldingMarks'])) {
			$this->foldingMarks = $data['foldingMarks'];
		}
		if (isset($data['logo'])) {
			$this->logo = $data['logo'];
		}
		if (isset($data['sections'])) {
			foreach($data['sections'] as $section) {
				if ($section['type'] === 'text') {
					$this->sections[] = new PDFTextSection($section['data']);
				}
				else {
					$entrySection = new PDFEntrySection(null);
					$entrySection->import($section['data']);
					$this->sections[] = $entrySection;
				}
			}
		}
	}

	/**
	 * Returns the logo file path.
	 *
	 * @return string logo
	 */
	public function getLogo() {
		return $this->logo;
	}

	/**
	 * Sets the logo file path.
	 *
	 * @param string $logo logo
	 */
	public function setLogo($logo) {
		$this->logo = $logo;
	}

	/**
	 * Returns the title.
	 *
	 * @return string title
	 */
	public function getTitle() {
		return $this->title;
	}

	/**
	 * Sets the title.
	 *
	 * @param string $title title
	 */
	public function setTitle($title) {
		$this->title = $title;
	}

	/**
	 * Returns if to print folding marks.
	 *
	 * @return string print folding marks
	 */
	public function getFoldingMarks() {
		return $this->foldingMarks;
	}

	/**
	 * Sets if to print folding marks.
	 *
	 * @param string $foldingMarks print folding marks
	 */
	public function setFoldingMarks($foldingMarks) {
		$this->foldingMarks = $foldingMarks;
	}

	/**
	 * Returns the sections.
	 *
	 * @return PDFTextSection[]|PDFEntrySection[] $sections
	 */
	public function getSections() {
		return $this->sections;
	}

	/**
	 * Sets the sections.
	 *
	 * @param PDFTextSection[]|PDFEntrySection[] $sections sections
	 */
	public function setSections($sections) {
		$this->sections = $sections;
	}

}

/**
 * Section for static text.
 *
 * @author Roland Gruber
 */
class PDFTextSection {

	private $text = '';

	/**
	 * Constructor.
	 *
	 * @param string $text text
	 */
	public function __construct($text) {
		$this->text = $text;
	}

	/**
	 * Exports the section.
	 *
	 * @return string text
	 */
	public function export() {
		return $this->getText();
	}

	/**
	 * Returns the text.
	 *
	 * @return string text
	 */
	public function getText() {
		return $this->text;
	}

}

/**
 * PDF section that contains LDAP data entries.
 *
 * @author Roland Gruber
 */
class PDFEntrySection {

	private $title;
	private $entries = array();

	/**
	 * Constructor
	 *
	 * @param string $title title
	 */
	public function __construct($title) {
		$this->title = $title;
	}

	/**
	 * Exports the section.
	 *
	 * @return array export data
	 */
	public function export() {
		$data = array();
		$data['title'] = $this->title;
		$data['entries'] = array();
		foreach($this->getEntries() as $entry) {
			$data['entries'][] = $entry->getKey();
		}
		return $data;
	}

	/**
	 * Imports the section.
	 *
	 * @param array $data import data
	 */
	public function import($data) {
		if (isset($data['title'])) {
			$this->title = $data['title'];
		}
		if ($data['entries']) {
			foreach($data['entries'] as $entry) {
				$this->entries[] = new PDFSectionEntry($entry);
			}
		}
	}

	/**
	 * Returns if the title is an attribute value.
	 *
	 * @return boolean is attribute
	 */
	public function isAttributeTitle() {
		return (bool) preg_match('/^_([a-zA-Z0-9_-])+$/', $this->title);
	}

	/**
	 * Returns the PDF key name.
	 *
	 * @return string PDF key name
	 */
	public function getPdfKey() {
		return substr($this->title, 1);
	}

	/**
	 * Returns the text title.
	 *
	 * @return string title
	 */
	public function getTitle() {
		return $this->title;
	}

	/**
	 * Sets the title text.
	 *
	 * @param string $title title
	 */
	public function setTitle($title) {
		$this->title = $title;
	}

	/**
	 * Returns the entries.
	 *
	 * @return PDFSectionEntry[] entries
	 */
	public function getEntries() {
		return $this->entries;
	}

	/**
	 * Sets the entries.
	 *
	 * @param PDFSectionEntry[] $entries entries
	 */
	public function setEntries($entries) {
		$this->entries = $entries;
	}

}

/**
 * Single PDF entry.
 *
 * @author Roland Gruber
 */
class PDFSectionEntry {

	private $key;

	/**
	 * Constructor
	 *
	 * @param string $key key
	 */
	public function __construct($key) {
		$this->key = $key;
	}

	/**
	 * Returns the PDF key.
	 *
	 * @return string $key key
	 */
	public function getKey() {
		return $this->key;
	}

}

/**
 * Returns a list of possible fonts.
 *
 * @return array list of fonts (description => font name)
 */
function getPdfFonts() {
	return array(
		'DejaVu' => 'DejaVuSerif',
		_('Chinese Traditional') => 'cid0ct',
		_('Chinese Simplified') => 'cid0cs',
		_('Japanese') => 'cid0jp',
		_('Korean') => 'cid0kr',
	);
}

?>
