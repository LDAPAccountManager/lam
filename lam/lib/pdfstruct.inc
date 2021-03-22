<?php
namespace LAM\PDF;

use \htmlStatusMessage;
use LAM\PERSISTENCE\ConfigurationDatabase;
use LAM\TYPES\TypeManager;
use LAMCfgMain;
use LAMConfig;
use \LAMException;
use \LAM\ImageUtils\ImageManipulationFactory;
use PDO;
use ServerProfilePersistenceManager;
use XMLReader;
use XMLWriter;
use function LAM\PERSISTENCE\dbTableExists;

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
 * Manages the persistence of PDF structures.
 *
 * @package LAM\PDF
 */
class PdfStructurePersistenceManager {

	/**
	 * @var PdfStructurePersistenceStrategy
	 */
	private $strategy;

	/**
	 * Constructor
	 */
	public function __construct() {
		$configDb = new ConfigurationDatabase(new LAMCfgMain());
		if ($configDb->useRemoteDb()) {
			// TODO
			$this->strategy = new PdfStructurePersistenceStrategyPdo($configDb->getPdo());
		} else {
			$this->strategy = new PdfStructurePersistenceStrategyFiles();
		}
	}

	/**
	 * Returns the names of existing PDF structure templates.
	 *
	 * @return array scope => names (e.g. array('user' => array('default')))
	 * @throws LAMException error reading templates
	 */
	public function getPdfStructureTemplateNames() : array {
		return $this->strategy->getPdfStructureTemplateNames();
	}

	/**
	 * Deletes an PDF structure template.
	 *
	 * @param string $scope user/group/host
	 * @param string $name PDF structure name
	 * @throws LAMException error deleting template
	 */
	public function deletePdfStructureTemplate(string $scope, string $name) : void {
		if (!$this->isValidPdfStructureName($name) || !TypeManager::isValidTypeId($scope) || ($name == 'default')) {
			logNewMessage(LOG_NOTICE, "Invalid account profile name: $name:$scope");
			throw new LAMException(_("Unable to delete profile!"));
		}
		$this->strategy->deletePdfStructureTemplate($scope, $name);
	}

	/**
	 * Reads a PDF structure template.
	 *
	 * @param string $scope user/group/host
	 * @param string $name structure name
	 * @return PDFStructure PDF structure
	 * @throws LAMException error deleting structure
	 */
	public function readPdfStructureTemplate(string $scope, string $name) : PDFStructure {
		if (!TypeManager::isValidTypeId($scope) || !$this->isValidPDFStructureName($name)) {
			throw new LAMException(_('Unable to read PDF structure.'));
		}
		return $this->strategy->readPdfStructureTemplate($scope, $name);
	}

	/**
	 * Saves the PDF structure.
	 *
	 * @param string $scope user/group/host
	 * @param string $name structure name
	 * @param PDFStructure $structure structure
	 * @throws LAMException error saving structure
	 */
	public function savePdfStructureTemplate(string $scope, string $name, PDFStructure $structure) : void {
		if (!TypeManager::isValidTypeId($scope) || !$this->isValidPDFStructureName($name)) {
			throw new LAMException(_('PDF structure name not valid'));
		}
		$this->strategy->savePdfStructureTemplate($scope, $name, $structure);
	}

	/**
	 * Returns a list of template logo file names.
	 *
	 * @return string[] logo file names
	 */
	public function getPdfTemplateLogoNames() : array {
		return $this->strategy->getPdfTemplateLogoNames();
	}

	/**
	 * Returns the binary data for the given template logo.
	 *
	 * @param string $name file name
	 * @return string binary
	 * @throws LAMException error reading file
	 */
	public function getPdfTemplateLogoBinary(string $name) : string {
		if (!$this->isValidLogoFileName($name)) {
			throw new LAMException(_('Unable to read logo file.'));
		}
		return $this->strategy->getPdfTemplateLogoBinary($name);
	}

	/**
	 * Deletes a logo in global templates.
	 *
	 * @param string $name logo name
	 * @throws LAMException error during deletion
	 */
	public function deletePdfTemplateLogo(string $name) : void {
		if (!$this->isValidLogoFileName($name)) {
			throw new LAMException(_('Unable to delete logo file.'));
		}
		$this->strategy->deletePdfTemplateLogo($name);
	}

	/**
	 * Saves the template logo.
	 *
	 * @param string $name file name
	 * @param string $data binary data
	 * @throws LAMException error during save
	 */
	public function savePdfTemplateLogo(string $name, string $data) : void {
		if (!$this->isValidLogoFileName($name)) {
			throw new LAMException(_('Unable to upload logo file.'));
		}
		$this->strategy->savePdfTemplateLogo($name, $data);
	}

	/**
	 * Returns the list of available PDF logos.
	 *
	 * @param string $confName server profile name
	 * @param bool $readDimensions reads the image dimensions
	 * @return PdfLogo[] logos
	 * @throws LAMException error reading logos
	 */
	public function getPdfLogos(string $confName, bool $readDimensions = false) : array {
		if (!LAMConfig::isValidName($confName)) {
			throw new LAMException(_('Unable to read logos.'));
		}
		$logoNames = $this->strategy->getPdfLogoNames($confName);
		sort($logoNames);
		$result = array();
		if ($readDimensions) {
			include_once __DIR__ . '/imageutils.inc';
		}
		foreach ($logoNames as $logoName) {
			if ($readDimensions) {
				$binary = $this->getPdfLogoBinary($confName, $logoName);
				$imageManipulator = ImageManipulationFactory::getImageManipulator($binary);
				$result[] = new PdfLogo($logoName, $imageManipulator->getHeight(), $imageManipulator->getWidth());
				$imageManipulator = null;
			}
			else {
				$result[] = new PdfLogo($logoName);
			}
		}
		return $result;
	}

	/**
	 * Reads a PDF logo.
	 *
	 * @param string $confName server profile name
	 * @param string $name file name
	 * @return string binary data
	 * @throws LAMException error reading logo
	 */
	public function getPdfLogoBinary(string $confName, string $name) : string {
		if (!LAMConfig::isValidName($confName) || !$this->isValidLogoFileName($name)) {
			throw new LAMException(_('Unable to read logo file.'));
		}
		return $this->strategy->getPdfLogoBinary($confName, $name);
	}

	/**
	 * Deletes a PDF logo.
	 *
	 * @param string $confName server profile name
	 * @param string $name logo name
	 * @throws LAMException error deleting logo
	 */
	public function deletePdfLogo(string $confName, string $name) : void {
		if (!LAMConfig::isValidName($confName) || !$this->isValidLogoFileName($name)) {
			throw new LAMException(_('Unable to delete logo file.'));
		}
		// check if existing
		$found = false;
		$logos = $this->getPdfLogos($confName);
		foreach ($logos as $logo) {
			if ($logo->getName() === $name) {
				$found = true;
				break;
			}
		}
		if (!$found) {
			throw new LAMException(_('File does not exist.'), htmlspecialchars($name));
		}
		// check if still in use
		$typeManager = new TypeManager();
		$activeTypes = $typeManager->getConfiguredTypes();
		foreach ($activeTypes as $type) {
			$structures = $this->getPDFStructures($confName, $type->getId());
			foreach ($structures as $structure) {
				$data = $this->readPdfStructure($confName, $type->getId(), $structure);
				if ($data->getLogo() == $name) {
					throw new LAMException(_('Unable to delete logo file.'),
						sprintf(_('Logo is still in use by PDF structure "%s" in account type "%s".'), $structure, $type->getAlias()));
				}
			}
		}
		$this->strategy->deletePdfLogo($confName, $name);
	}

	/**
	 * Saves a PDF logo.
	 *
	 * @param string $confName server profile name
	 * @param string $name logo name
	 * @param string $data binary
	 * @throws LAMException error saving logo
	 */
	public function savePdfLogo(string $confName, string $name, string $data) : void {
		if (!LAMConfig::isValidName($confName) || !$this->isValidLogoFileName($name)) {
			throw new LAMException(_('Unable to upload logo file.'));
		}
		if (!preg_match('/[a-zA-Z0-9_-]+\\.(png)|(jpg)/i', $name)) {
			throw new LAMException(_('Unable to upload logo file.'), _('The file name must end with ".png" or ".jpg".'));
		}
		$this->strategy->savePdfLogo($confName, $name, $data);
	}

	/**
	 * Returns all available PDF structure definitions for the submitted account type.
	 *
	 * @param string $confName server profile name
	 * @param string $typeId the account type
	 *
	 * @return string[] structure names
	 * @throws LAMException error reading structures
	 */
	public function getPDFStructures(string $confName, string $typeId) : array {
		if (!TypeManager::isValidTypeId($typeId)) {
			throw new LAMException(_('Unable to read PDF structures.'));
		}
		$return = $this->strategy->getPDFStructures($confName, $typeId);
		sort($return);
		return $return;
	}

	/**
	 * Deletes a PDF structure.
	 *
	 * @param string $confName server profile name
	 * @param string $typeId user/group/host
	 * @param string $name structure name
	 * @throws LAMException error deleting structure
	 */
	public function deletePdfStructure(string $confName, string $typeId, string $name) : void {
		if (!LAMConfig::isValidName($confName) || !TypeManager::isValidTypeId($typeId) || !$this->isValidPDFStructureName($name)) {
			throw new LAMException(_('Unable to delete PDF structure!'));
		}
		$this->strategy->deletePdfStructure($confName, $typeId, $name);
	}

	/**
	 * Reads a PDF structure.
	 *
	 * @param string $confName server profile name
	 * @param string $typeId user/group/host
	 * @param string $name structure name
	 * @return PDFStructure PDF structure
	 * @throws LAMException error deleting structure
	 */
	public function readPdfStructure(string $confName, string $typeId, string $name) : PDFStructure {
		if (!LAMConfig::isValidName($confName) || !TypeManager::isValidTypeId($typeId) || !$this->isValidPDFStructureName($name)) {
			throw new LAMException(_('Unable to read PDF structure.'));
		}
		return $this->strategy->readPdfStructure($confName, $typeId, $name);
	}

	/**
	 * Saves the PDF structure.
	 *
	 * @param string $confName server profile name
	 * @param string $typeId user/group/host
	 * @param string $name structure name
	 * @param PDFStructure $structure structure
	 * @throws LAMException error saving structure
	 */
	public function savePdfStructure(string $confName, string $typeId, string $name, PDFStructure $structure) : void {
		if (!LAMConfig::isValidName($confName) || !TypeManager::isValidTypeId($typeId) || !$this->isValidPDFStructureName($name)) {
			throw new LAMException(_('PDF structure name not valid'));
		}
		$this->strategy->savePdfStructure($confName, $typeId, $name, $structure);
	}

	/**
	 * Returns if the give structure name is valid.
	 *
	 * @param string $name structure name
	 * @return boolean is valid
	 */
	private function isValidPDFStructureName(string $name) : bool {
		return preg_match('/^[a-z0-9\-\_]+$/i', $name) === 1;
	}

	/**
	 * Returns if the given logo file name is valid.
	 *
	 * @param string $fileName file name
	 * @return bool valid
	 */
	private function isValidLogoFileName(string $fileName) : bool {
		return preg_match('/[a-zA-Z0-9_-]+\\.(png)|(jpg)/i', $fileName);
	}

	/**
	 * Installs template structures to the given server profile.
	 *
	 * @param string $confName server profile name
	 * @throws LAMException error during installation
	 */
	public function installPDFTemplates(string $confName) {
		if (!LAMConfig::isValidName($confName)) {
			throw new LAMException(_("Profile name is invalid!"));
		}
		$serverProfilesPersistenceManager = new ServerProfilePersistenceManager();
		$config = $serverProfilesPersistenceManager->loadProfile($confName);
		$typeManager = new TypeManager($config);
		$allTemplates = $this->getPdfStructureTemplateNames();
		foreach ($typeManager->getConfiguredTypes() as $type) {
			if (empty($allTemplates[$type->getScope()])) {
				continue;
			}
			$existingStructures = $this->getPDFStructures($confName, $type->getId());
			foreach ($allTemplates[$type->getScope()] as $templateName) {
				if (!in_array($templateName, $existingStructures)) {
					$structure = $this->readPdfStructureTemplate($type->getScope(), $templateName);
					$this->savePdfStructure($confName, $type->getId(), $templateName, $structure);
				}
			}
		}
		$logos = $this->getPdfTemplateLogoNames();
		$existingLogos = $this->getPdfLogos($confName);
		foreach ($logos as $logo) {
			if (!in_array($logo, $existingLogos)) {
				$binary = $this->getPdfTemplateLogoBinary($logo);
				$this->savePdfLogo($confName, $logo, $binary);
			}
		}
	}

}

/**
 * Logo for PDF structures.
 *
 * @package LAM\PDF
 */
class PdfLogo {

	private $name;

	private $height;

	private $width;

	/**
	 * Constructor
	 *
	 * @param string $name file name
	 * @param int $height height
	 * @param int $width width
	 */
	public function __construct(string $name, int $height = 0, int $width = 0) {
		$this->name = $name;
		$this->height = $height;
		$this->width = $width;
	}

	/**
	 * Returns the file name.
	 *
	 * @return string file name
	 */
	public function getName(): string {
		return $this->name;
	}

	/**
	 * Returns the height.
	 *
	 * @return int height
	 */
	public function getHeight(): int {
		return $this->height;
	}

	/**
	 * Returns the width.
	 *
	 * @return int width
	 */
	public function getWidth(): int {
		return $this->width;
	}

}

/**
 * Interface for PDF structure persistence.
 *
 * @package LAM\PDF
 */
interface PdfStructurePersistenceStrategy {

	/**
	 * Returns the names of existing PDF structure templates.
	 *
	 * @return array scope => names (e.g. array('user' => array('default')))
	 * @throws LAMException error reading templates
	 */
	public function getPdfStructureTemplateNames() : array;

	/**
	 * Deletes a PDF structure template,
	 *
	 * @param string $scope user/group/host
	 * @param string $name template name
	 * @throws LAMException error deleting template
	 */
	public function deletePdfStructureTemplate(string $scope, string $name) : void;

	/**
	 * Reads a PDF structure template.
	 *
	 * @param string $scope user/group/host
	 * @param string $name structure name
	 * @return PDFStructure PDF structure
	 * @throws LAMException error deleting structure
	 */
	public function readPdfStructureTemplate(string $scope, string $name) : PDFStructure;

	/**
	 * Saves the PDF structure.
	 *
	 * @param string $scope user/group/host
	 * @param string $name structure name
	 * @param PDFStructure $structure structure
	 * @throws LAMException error saving structure
	 */
	public function savePdfStructureTemplate(string $scope, string $name, PDFStructure $structure) : void;

	/**
	 * Returns a list of template logo file names.
	 *
	 * @return string[] logo file names
	 */
	public function getPdfTemplateLogoNames() : array;

	/**
	 * Returns the binary data for the given template logo.
	 *
	 * @param string $name file name
	 * @return string binary
	 * @throws LAMException error reading file
	 */
	public function getPdfTemplateLogoBinary(string $name) : string;

	/**
	 * Deletes a logo in global templates.
	 *
	 * @param string $name logo name
	 * @throws LAMException error during deletion
	 */
	public function deletePdfTemplateLogo(string $name) : void;

	/**
	 * Saves the template logo.
	 *
	 * @param string $name file name
	 * @param string $data binary data
	 * @throws LAMException error during save
	 */
	public function savePdfTemplateLogo(string $name, string $data) : void;

	/**
	 * Returns the list of available PDF logos.
	 *
	 * @param string $confName server profile name
	 * @return string[] logos
	 * @throws LAMException error reading logos
	 */
	public function getPdfLogoNames(string $confName) : array;

	/**
	 * Reads a PDF logo.
	 *
	 * @param string $confName server profile name
	 * @param string $name file name
	 * @return string binary data
	 * @throws LAMException error reading logo
	 */
	public function getPdfLogoBinary(string $confName, string $name) : string;

	/**
	 * Deletes a PDF logo.
	 *
	 * @param string $confName server profile name
	 * @param string $name logo name
	 * @throws LAMException error deleting logo
	 */
	public function deletePdfLogo(string $confName, string $name) : void;

	/**
	 * Saves a PDF logo.
	 *
	 * @param string $confName server profile name
	 * @param string $name logo name
	 * @param string $data binary
	 * @throws LAMException error saving logo
	 */
	public function savePdfLogo(string $confName, string $name, string $data) : void;

	/**
	 * Returns all available PDF structure definitions for the submitted account type.
	 *
	 * @param string $confName server profile name
	 * @param string $typeId the account type
	 *
	 * @return string[] structure names
	 * @throws LAMException error reading structures
	 */
	public function getPDFStructures(string $confName, string $typeId) : array;

	/**
	 * Deletes a PDF structure.
	 *
	 * @param string $confName server profile name
	 * @param string $typeId user/group/host
	 * @param string $name structure name
	 * @throws LAMException error deleting structure
	 */
	public function deletePdfStructure(string $confName, string $typeId, string $name) : void;

	/**
	 * Reads a PDF structure.
	 *
	 * @param string $confName server profile name
	 * @param string $typeId user/group/host
	 * @param string $name structure name
	 * @return PDFStructure PDF structure
	 * @throws LAMException error deleting structure
	 */
	public function readPdfStructure(string $confName, string $typeId, string $name) : PDFStructure;

	/**
	 * Saves the PDF structure.
	 *
	 * @param string $confName server profile name
	 * @param string $typeId user/group/host
	 * @param string $name structure name
	 * @param PDFStructure $structure structure
	 * @throws LAMException error saving structure
	 */
	public function savePdfStructure(string $confName, string $typeId, string $name, PDFStructure $structure) : void;

}

/**
 * Manages PDF structures on file system.
 *
 * @package LAM\PDF
 */
class PdfStructurePersistenceStrategyFiles implements PdfStructurePersistenceStrategy {

	/**
	 * @inheritDoc
	 */
	public function getPdfStructureTemplateNames(): array {
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
	 * @inheritDoc
	 */
	public function deletePdfStructureTemplate(string $scope, string $name): void {
		$fileName = $this-> getPdfStructureTemplateFileName($scope, $name);
		$deleted = @unlink($fileName);
		if (!$deleted) {
			throw new LAMException(_("Unable to delete PDF structure!"));
		}
	}

	/**
	 * @inheritDoc
	 */
	public function readPdfStructureTemplate(string $scope, string $name): PDFStructure {
		$fileName = $this->getPdfStructureTemplateFileName($scope, $name);
		if (!is_file($fileName) || !is_readable($fileName)) {
			throw new LAMException(_('Unable to read PDF structure.'));
		}
		$handle = fopen($fileName, 'r');
		$xmlData = fread($handle, 100000000);
		fclose($handle);
		$reader = new PDFStructureReader();
		return $reader->read($xmlData);
	}

	/**
	 * @inheritDoc
	 */
	public function savePdfStructureTemplate(string $scope, string $name, PDFStructure $structure): void {
		$fileName = $this->getPdfStructureTemplateFileName($scope, $name);
		$writer = new PDFStructureWriter();
		$xml = $writer->getXML($structure);
		$file = @fopen($fileName, "w");
		if (!$file) {
			logNewMessage(LOG_ERR, 'Unable to write ' . $fileName);
			throw new LAMException(_('Unable to save PDF structure.'));
		}
		fputs($file, $xml);
		fclose($file);
	}

	/**
	 * Returns the file name for a PDF structure.
	 *
	 * @param string $scope user/group/host
	 * @param string $name structure name
	 * @return string file name
	 */
	private function getPdfStructureTemplateFileName(string $scope, string $name) : string {
		return __DIR__ . '/../config/templates/pdf/' . $name . '.' . $scope . '.xml';
	}

	/**
	 * @inheritDoc
	 */
	public function getPdfTemplateLogoNames(): array {
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
	 * @inheritDoc
	 */
	public function getPdfTemplateLogoBinary(string $name): string {
		$fileName = $this->getPdfTemplateLogoFileName($name);
		$handle = fopen($fileName, 'r');
		$logoBinary = fread($handle, 100000000);
		fclose($handle);
		return $logoBinary;
	}

	/**
	 * Returns the file name of a given logo.
	 *
	 * @param string $name logo name
	 * @return string file name
	 */
	private function getPdfTemplateLogoFileName(string $name) : string {
		return __DIR__ . '/../config/templates/pdf/logos/' . $name;
	}

	/**
	 * @inheritDoc
	 */
	public function deletePdfTemplateLogo(string $name): void {
		$fileName = $this->getPdfTemplateLogoFileName($name);
		$deleted = @unlink($fileName);
		if (!$deleted) {
			throw new LAMException(_('Unable to delete logo file.'));
		}
	}

	/**
	 * @inheritDoc
	 */
	public function savePdfTemplateLogo(string $name, string $data): void {
		$fileName = $this->getPdfTemplateLogoFileName($name);
		$file = @fopen($fileName, "w");
		if (!$file) {
			logNewMessage(LOG_ERR, 'Unable to write ' . $fileName);
			throw new LAMException(_('Unable to upload logo file.'));
		}
		fputs($file, $data);
		fclose($file);
	}

	/**
	 * Returns the file name of a given logo.
	 *
	 * @param string $confName server profile name
	 * @param string $name logo name
	 * @return string file name
	 */
	private function getPdfLogoFileName(string $confName, string $name) : string {
		return __DIR__ . '/../config/pdf/' . $confName . '/logos/' . $name;
	}

	/**
	 * @inheritDoc
	 */
	public function getPdfLogoNames(string $confName): array {
		$return = array();
		$dirPath = __DIR__ . '/../config/pdf/' . $confName . '/logos/';
		if (!is_dir($dirPath)) {
			mkdir($dirPath, 0700, true);
		}
		$dirHandle = opendir($dirPath);
		if ($dirHandle === false) {
			throw new LAMException(_('Unable to read logos.'));
		}
		while ($file = readdir($dirHandle)) {
			if (!is_dir($file) && $file != '.' && $file != '..' && preg_match('/\\.(jpg|png)$/i', $file)) {
				$return[] = $file;
			}
		}
		return $return;
	}

	/**
	 * @inheritDoc
	 */
	public function getPdfLogoBinary(string $confName, string $name): string {
		$fileName = $this->getPdfLogoFileName($confName, $name);
		$handle = fopen($fileName, 'r');
		$logoBinary = fread($handle, 100000000);
		fclose($handle);
		return $logoBinary;
	}

	/**
	 * @inheritDoc
	 */
	public function deletePdfLogo(string $confName, string $name): void {
		// delete file
		$success = @unlink($this->getPdfLogoFileName($confName, $name));
		if (!$success) {
			throw new LAMException(_('Unable to delete logo file.'), $name);
		}
	}

	/**
	 * @inheritDoc
	 */
	public function savePdfLogo(string $confName, string $name, string $data): void {
		$fileName = $this->getPdfLogoFileName($confName, $name);
		$basePath = dirname($fileName);
		if (!file_exists($basePath)) {
			mkdir($basePath, 0700, true);
		}
		$file = @fopen($fileName, "w");
		if (!$file) {
			logNewMessage(LOG_ERR, 'Unable to write ' . $fileName);
			throw new LAMException(_('Unable to upload logo file.'));
		}
		fputs($file, $data);
		fclose($file);
	}

	/**
	 * @inheritDoc
	 */
	public function getPDFStructures(string $confName, string $typeId): array {
		$return = array();
		$path = __DIR__ . '/../config/pdf/' . $confName;
		if (is_dir($path)) {
			$dirHandle = opendir($path);
			while($file = readdir($dirHandle)) {
				$struct_file = explode('.', $file);
				if(!is_dir($path . $file)
					&& ($file != '.')
					&& ($file != '..')
					&& (sizeof($struct_file) === 3)
					&& ($struct_file[1] === $typeId)
					&& ($struct_file[2] === 'xml')) {
					array_push($return, $struct_file[0]);
				}
			}
		}
		return $return;
	}

	/**
	 * @inheritDoc
	 */
	public function deletePdfStructure(string $confName, string $typeId, string $name): void {
		$fileName = $this->getPdfStructureFileName($confName, $typeId, $name);
		if (!is_file($fileName) || !is_writable($fileName)) {
			throw new LAMException(_('Unable to delete PDF structure!'));
		}
		$deleteOk = @unlink($fileName);
		if (!$deleteOk) {
			throw new LAMException(_('Unable to delete PDF structure!'));
		}
	}

	/**
	 * @inheritDoc
	 */
	public function readPdfStructure(string $confName, string $typeId, string $name): PDFStructure {
		$fileName = $this->getPdfStructureFileName($confName, $typeId, $name);
		if (!is_file($fileName) || !is_readable($fileName)) {
			throw new LAMException(_('Unable to read PDF structure.'));
		}
		$handle = fopen($fileName, 'r');
		$xmlData = fread($handle, 100000000);
		fclose($handle);
		$reader = new PDFStructureReader();
		return $reader->read($xmlData);
	}

	/**
	 * @inheritDoc
	 */
	public function savePdfStructure(string $confName, string $typeId, string $name, PDFStructure $structure): void {
		$fileName = $this->getPdfStructureFileName($confName, $typeId, $name);
		$basePath = dirname($fileName);
		if (!file_exists($basePath)) {
			mkdir($basePath, 0700, true);
		}
		$writer = new PDFStructureWriter();
		$xml = $writer->getXML($structure);
		$file = @fopen($fileName, "w");
		if (!$file) {
			logNewMessage(LOG_ERR, 'Unable to write ' . $fileName);
			throw new LAMException(_('Unable to save PDF structure.'));
		}
		fputs($file, $xml);
		fclose($file);
	}

	/**
	 * Returns the file name of the structure.
	 *
	 * @param string $confName server profile name
	 * @param string $typeId user/group/host
	 * @param string $name structure name
	 * @return string file name
	 */
	private function getPdfStructureFileName(string $confName, string $typeId, string $name) : string {
		return dirname(__FILE__) . '/../config/pdf/' . $confName . '/' . $name . '.' . $typeId . '.xml';
	}

}

/**
 * Manages PDF structures on file system.
 *
 * @package LAM\PDF
 */
class PdfStructurePersistenceStrategyPdo implements PdfStructurePersistenceStrategy {

	const TABLE_NAME = 'pdf_structures';
	const TABLE_NAME_LOGOS = 'pdf_logos';
	const TABLE_NAME_TEMPLATES = 'pdf_structures_templates';
	const TABLE_NAME_TEMPLATES_LOGOS = 'pdf_logos_templates';

	/**
	 * @var PDO
	 */
	private $pdo;

	/**
	 * Constructor
	 *
	 * @param PDO $pdo PDO
	 */
	public function __construct(PDO $pdo) {
		$this->pdo = $pdo;
		$this->checkSchema();
	}

	/**
	 * Checks if the schema has latest version.
	 */
	private function checkSchema() : void {
		if (!dbTableExists($this->pdo, self::TABLE_NAME)) {
			$this->createInitialSchema();
		}
	}

	/**
	 * Creates the initial schema.
	 */
	public function createInitialSchema() : void {
		logNewMessage(LOG_DEBUG, 'Creating database table ' . self::TABLE_NAME);
		$sql = 'create table ' . self::TABLE_NAME . '('
			. 'position int NOT NULL,'
			. 'confname VARCHAR(300) NOT NULL,'
			. 'typeid VARCHAR(300) NOT NULL,'
			. 'name VARCHAR(300) NOT NULL,'
			. 'data TEXT NOT NULL,'
			. 'PRIMARY KEY(position)'
			. ');';
		$this->pdo->exec($sql);
		logNewMessage(LOG_DEBUG, 'Creating database table ' . self::TABLE_NAME_TEMPLATES);
		$sql = 'create table ' . self::TABLE_NAME_TEMPLATES . '('
			. 'scope VARCHAR(100) NOT NULL,'
			. 'name VARCHAR(300) NOT NULL,'
			. 'data TEXT NOT NULL,'
			. 'PRIMARY KEY(scope,name)'
			. ');';
		$this->pdo->exec($sql);
		logNewMessage(LOG_DEBUG, 'Creating database table ' . self::TABLE_NAME_LOGOS);
		$sql = 'create table ' . self::TABLE_NAME_LOGOS . '('
			. 'confname VARCHAR(300) NOT NULL,'
			. 'name VARCHAR(300) NOT NULL,'
			. 'data BLOB NOT NULL,'
			. 'PRIMARY KEY(confname,name)'
			. ');';
		$this->pdo->exec($sql);
		logNewMessage(LOG_DEBUG, 'Creating database table ' . self::TABLE_NAME_TEMPLATES_LOGOS);
		$sql = 'create table ' . self::TABLE_NAME_TEMPLATES_LOGOS . '('
			. 'name VARCHAR(300) NOT NULL,'
			. 'data BLOB NOT NULL,'
			. 'PRIMARY KEY(name)'
			. ');';
		$this->pdo->exec($sql);
		$sql = 'insert into ' . ConfigurationDatabase::TABLE_SCHEMA_VERSIONS . ' (name, version) VALUES ("pdf_structures", 1);';
		$this->pdo->exec($sql);
	}

	/**
	 * @inheritDoc
	 */
	public function getPdfStructureTemplateNames(): array {
		$statement = $this->pdo->prepare("SELECT scope, name FROM " . self::TABLE_NAME_TEMPLATES);
		$statement->execute();
		$results = $statement->fetchAll();
		$profiles = array();
		foreach ($results as $result) {
			$profiles[$result['scope']][] = $result['name'];
		}
		return $profiles;
	}

	/**
	 * @inheritDoc
	 */
	public function deletePdfStructureTemplate(string $scope, string $name): void {
		// TODO: Implement deletePdfStructureTemplate() method.
	}

	/**
	 * @inheritDoc
	 */
	public function readPdfStructureTemplate(string $scope, string $name): PDFStructure {
		// TODO: Implement readPdfStructureTemplate() method.
	}

	/**
	 * @inheritDoc
	 */
	public function savePdfStructureTemplate(string $scope, string $name, PDFStructure $structure): void {
		// TODO: Implement savePdfStructureTemplate() method.
	}

	/**
	 * @inheritDoc
	 */
	public function getPdfTemplateLogoNames(): array {
		$statement = $this->pdo->prepare("SELECT name FROM " . self::TABLE_NAME_TEMPLATES_LOGOS);
		$statement->execute();
		$results = $statement->fetchAll();
		$logos = array();
		foreach ($results as $result) {
			$logos[] = $result['name'];
		}
		return $logos;
	}

	/**
	 * @inheritDoc
	 */
	public function getPdfTemplateLogoBinary(string $name): string {
		// TODO: Implement getPdfTemplateLogoBinary() method.
	}

	/**
	 * @inheritDoc
	 */
	public function deletePdfTemplateLogo(string $name): void {
		// TODO: Implement deletePdfTemplateLogo() method.
	}

	/**
	 * @inheritDoc
	 */
	public function savePdfTemplateLogo(string $name, string $data): void {
		// TODO: Implement savePdfTemplateLogo() method.
	}

	/**
	 * @inheritDoc
	 */
	public function getPdfLogoNames(string $confName): array {
		$statement = $this->pdo->prepare("SELECT name FROM " . self::TABLE_NAME_LOGOS . ' WHERE confname = ?');
		$statement->execute(array($confName));
		$results = $statement->fetchAll();
		$logos = array();
		foreach ($results as $result) {
			$logos[] = $result['name'];
		}
		return $logos;
	}

	/**
	 * @inheritDoc
	 */
	public function getPdfLogoBinary(string $confName, string $name): string {
		// TODO: Implement getPdfLogoBinary() method.
	}

	/**
	 * @inheritDoc
	 */
	public function deletePdfLogo(string $confName, string $name): void {
		// TODO: Implement deletePdfLogo() method.
	}

	/**
	 * @inheritDoc
	 */
	public function savePdfLogo(string $confName, string $name, string $data): void {
		// TODO: Implement savePdfLogo() method.
	}

	/**
	 * @inheritDoc
	 */
	public function getPDFStructures(string $confName, string $typeId): array {
		$statement = $this->pdo->prepare("SELECT name FROM " . self::TABLE_NAME . ' WHERE confname = ? AND typeid = ?');
		$statement->execute(array($confName, $typeId));
		$results = $statement->fetchAll();
		$names = array();
		foreach ($results as $result) {
			$names[] = $result['name'];
		}
		return $names;
	}

	/**
	 * @inheritDoc
	 */
	public function deletePdfStructure(string $confName, string $typeId, string $name): void {
		$statement = $this->pdo->prepare("DELETE FROM " . self::TABLE_NAME . " WHERE confname = ? AND typeid = ? AND name = ?");
		$statement->execute(array($confName, $typeId, $name));
	}

	/**
	 * @inheritDoc
	 */
	public function readPdfStructure(string $confName, string $typeId, string $name): PDFStructure {
		$statement = $this->pdo->prepare("SELECT data FROM " . self::TABLE_NAME . ' WHERE confname = ? AND typeid = ? AND name = ?');
		$statement->execute(array($confName, $typeId, $name));
		$results = $statement->fetchAll();
		if (empty($results)) {
			throw new LAMException(_('Unable to read PDF structure.'));
		}
		$structure = new PDFStructure();
		$structure->import(json_decode($results[0]['data'], true));
		return $structure;
	}

	/**
	 * @inheritDoc
	 */
	public function savePdfStructure(string $confName, string $typeId, string $name, PDFStructure $structure): void {
		$json = json_encode($structure->export());
		$statement = $this->pdo->prepare("SELECT name FROM " . self::TABLE_NAME . " WHERE confname = ? AND typeid = ? AND name = ?");
		$statement->execute(array($confName, $typeId, $name));
		$results = $statement->fetchAll();
		$isExisting = !empty($results);
		if ($isExisting) {
			$statement = $this->pdo->prepare("UPDATE " . self::TABLE_NAME . " SET data = ? WHERE confname = ? AND typeid = ? AND name = ?");
			$statement->execute(array($json, $confName, $typeId, $name));
		}
		else {
			$positionStatement = $this->pdo->prepare("SELECT MAX(position) AS position FROM " . self::TABLE_NAME);
			$positionStatement->execute();
			$positionResult = $positionStatement->fetchAll();
			$position = $positionResult[0]['position'] + 1;
			$statement = $this->pdo->prepare("INSERT INTO " . self::TABLE_NAME . " (position, confname, typeid, name, data) VALUES (?, ?, ?, ?, ?)");
			$statement->execute(array($position, $confName, $typeId, $name, $json));
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
	 * Reads a PDF structure file.
	 *
	 * @param string $data XML data
	 * @return PDFStructure structure
	 * @throws LAMException error reading structure
	 */
	public function read(string $data) : PDFStructure {
		$xml = new XMLReader();
		$xml->XML($data);
		$structure = new PDFStructure();
		// open <pdf>
		@$xml->read();
		if (!$xml->name == 'pdf') {
			logNewMessage(LOG_ERR, 'Unknown tag name: ' . $xml->name);
			throw new LAMException(_('Unable to read PDF structure.'));
		}
		$structure->setLogo($xml->getAttribute('filename'));
		$structure->setTitle($xml->getAttribute('headline'));
		$structure->setFoldingMarks($xml->getAttribute('foldingmarks'));
		$sections = array();
		while ($xml->read()) {
			if (($xml->nodeType === XMLReader::SIGNIFICANT_WHITESPACE)
					|| (($xml->name === 'pdf') && ($xml->nodeType == XMLReader::END_ELEMENT))) {
				continue;
			}
			elseif ($xml->name === 'text') {
				$xml->read();
				$sections[] = new PDFTextSection($xml->value);
				$xml->read();
				if (!$xml->name === 'text') {
					logNewMessage(LOG_ERR, 'Unexpected tag name: ' . $xml->name);
					throw new LAMException(_('Unable to read PDF structure.'));
				}
			}
			elseif ($xml->name === 'section') {
				$sections[] = $this->readSection($xml);
			}
			else {
				logNewMessage(LOG_ERR, 'Unexpected tag name: ' . $xml->name);
				throw new LAMException(_('Unable to read PDF structure.'));
			}
		}
		$xml->close();
		$structure->setSections($sections);
		return $structure;
	}

	/**
	 * Reads a single section from XML.
	 *
	 * @param XMLReader $xml reader
	 * @return PDFEntrySection section
	 * @throws LAMException unable to parse section
	 */
	private function readSection(XMLReader $xml) : PDFEntrySection {
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
				throw new LAMException(_('Unable to read PDF structure.'));
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
	 * Returns the generated XML.
	 *
	 * @param PDFStructure $structure structure
	 * @return string XML
	 */
	public function getXML(PDFStructure $structure) : string {
		$writer = new XMLWriter();
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
