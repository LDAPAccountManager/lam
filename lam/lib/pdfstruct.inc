<?php
namespace LAM\PDF;

use \htmlStatusMessage;
use \LAMException;
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Michael Duergner
                2011 - 2017  Roland Gruber

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
include_once("config.inc");

/** LDAP object */
include_once("ldap.inc");

/**
 * This function will return all available PDF structure definitions for the submitted
 * account type.
 *
 * @param string $typeId the account type
 * @param string $profile server profile name
 *
 * @return array All available PDF structure definitions for the submitted account
 * scope. Each entry is a string being the filename that may be passed to the
 * createModulePDF() function as second argument.
 */
function getPDFStructures($typeId, $profile = null) {
	$return = array();
	if (!preg_match('/[a-zA-Z]+/', $typeId)) {
		return null;
	}
	if (!isset($profile)) {
		$profile = $_SESSION['config']->getName();
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
 *
 * @return boolean True if file was deleted or false if a problem occured.
 */
function deletePDFStructure($typeId, $name) {
	if (!isValidPDFStructureName($name) || !preg_match('/[a-zA-Z]+/',$typeId)) {
		return false;
	}
	$file = dirname(__FILE__) . '/../config/pdf/' . $_SESSION['config']->getName() . '/' . $name . '.' . $typeId . '.xml';
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
 * @return array list of logo files
 */
function getAvailableLogos() {
	$return = array();
	$dirPath = dirname(__FILE__) . '/../config/pdf/' . $_SESSION['config']->getName() . '/logos/';
	$dirHandle = opendir($dirPath);
	while($file = readdir($dirHandle)) {
		if(!is_dir($file) && $file != '.' && $file != '..' && preg_match('/\\.(jpg|png)$/i',$file)) {
			$infos = getimagesize($dirPath . $file);
			if($infos[0] <= 2000 && $infos[1] <= 300) {
				array_push($return, array('filename' => $file, 'infos' => $infos));
			}
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
 * @throws Exception
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
	$basePath = dirname(__FILE__) . '/../config/pdf/';
	$templatePath = dirname(__FILE__) . '/../config/templates/pdf/';
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
 * @return StatusMessage status message to display
 */
function uploadPDFLogo($file, $name) {
	if (!preg_match('/[a-zA-Z0-9_-]+\\.(png)|(jpg)/i', $name)) {
		return new htmlStatusMessage('ERROR', _('Unable to upload logo file.'), _('The file name must end with ".png" or ".jpg".'));
	}
	$infos = getimagesize($file);
	if ($infos[0] <= 2000 && $infos[1] <= 300) {
		$dirPath = dirname(__FILE__) . '/../config/pdf/' . $_SESSION['config']->getName() . '/logos/';
		$success = copy($file, $dirPath . '/' . $name);
		if ($success) {
			return new htmlStatusMessage('INFO', _('Uploaded logo file.'), $name);
		}
		else {
			return new htmlStatusMessage('ERROR', _('Unable to upload logo file.'), $name);
		}
	}
	return new htmlStatusMessage('ERROR', _('Unable to upload logo file.'), _('The file must not exeed 2000x300px.'));
}

/**
 * Deletes a PDF logo file.
 *
 * @param String $name file name
 * @return StatusMessage status message to display
 */
function deletePDFLogo($name) {
	// check if valid file
	$found = false;
	$logos = getAvailableLogos();
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
	foreach ($activeTypes as $type) {
		$structures = getPDFStructures($type->getId());
		foreach ($structures as $structure) {
			$data = loadPDFStructure($type->getId(), $structure);
			if ($data['page_definitions']['filename'] == $name) {
				return new htmlStatusMessage('ERROR', _('Unable to delete logo file.'),
					sprintf(_('Logo is still in use by PDF structure "%s" in account type "%s".'), $structure, $type->getAlias()));
			}
		}
	}
	// delete file
	$dirPath = dirname(__FILE__) . '/../config/pdf/' . $_SESSION['config']->getName() . '/logos/';
	$success = @unlink($dirPath . '/' . $name);
	if ($success) {
		return new htmlStatusMessage('INFO', _('Logo file deleted.'), $name);
	}
	else {
		return new htmlStatusMessage('ERROR', _('Unable to delete logo file.'), $name);
	}
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
 */
function installPDFTemplates() {
	$templatePath = dirname(__FILE__) . '/../config/templates/pdf';
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
	$basePath = dirname(__FILE__) . '/../config/pdf/' . $_SESSION['config']->getName();
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
	$templatePath = dirname(__FILE__) . '/../config/templates/pdf/logos';
	$templateDir = @dir($templatePath);
	if ($templateDir) {
		$entry = $templateDir->read();
		while ($entry){
			$path = $basePath . '/logos/' . $entry;
			if ((strpos($entry, '.') !== 1) && !is_file($path)) {
				$template = $templatePath . '/' . $entry;
				logNewMessage(LOG_DEBUG, 'Copy template ' . $template . ' to ' . $path);
				@copy($template, $path);
			}
			$entry = $templateDir->read();
		}
	}
}

/**
 * Reads a PDF structure.
 *
 * @author Roland Gruber
 */
class PDFStructureReader {

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
		return dirname(__FILE__) . '/../config/pdf/' . $_SESSION['config']->getName() . '/' . $name . '.' . $typeId . '.xml';
	}

	/**
	 * Reads a PDF structure file.
	 *
	 * @param string $file file name
	 * @return PDFStructure structure
	 */
	private function readPDFFile($file) {
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
			if ($xml->nodeType === \XMLReader::SIGNIFICANT_WHITESPACE) {
				continue;
			}
			elseif (($xml->name === 'pdf') && ($xml->nodeType == \XMLReader::END_ELEMENT)) {
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
				logNewMessage(LOG_ERR, 'Unexpected tag name: ' . $xml->name);
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
			elseif ($xml->nodeType === \XMLReader::END_ELEMENT) {
				continue;
			}
			elseif ($xml->nodeType === \XMLReader::SIGNIFICANT_WHITESPACE) {
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

	/**
	 * Writes the PDF structure to disk.
	 *
	 * @param string $typeId type ID
	 * @param string $name structure name
	 * @param PDFStructure $structure structure
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
	 */
	protected function getFileName($typeId, $name) {
		if (!isValidPDFStructureName($name) || !preg_match('/[a-zA-Z]+/', $typeId)) {
			throw new \LAMException(_('PDF structure name not valid'),
					_('The name for that PDF-structure you submitted is not valid. A valid name must consist of the following characters: \'a-z\',\'A-Z\',\'0-9\',\'_\',\'-\'.'));
		}
		if(!is_writable(dirname(__FILE__) . '/../config/pdf/' . $_SESSION['config']->getName())) {
			throw new \LAMException(_('Could not save PDF structure, access denied.'));
		}
		return dirname(__FILE__) . '/../config/pdf/' . $_SESSION['config']->getName() . '/' . $name . '.' . $typeId . '.xml';
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
	private $entries;

	/**
	 * Constructor
	 *
	 * @param string $title title
	 */
	public function __construct($title) {
		$this->title = $title;
	}

	/**
	 * Returns if the title is an attribute value.
	 *
	 * @return boolean is attribute
	 */
	public function isAttributeTitle() {
		return boolval(preg_match('/^_([a-zA-Z0-9_-])+$/', $this->title));
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

?>
