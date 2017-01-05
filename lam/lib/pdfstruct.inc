<?php
namespace LAM\PDF;

use \htmlStatusMessage;
use \LAMException;
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Michael Duergner
                2011 - 2016  Roland Gruber

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
 * account scope.
 *
 * @param string $scope The account scope the PDF structure definitions should be
 * returned.
 * @param string $profile server profile name
 *
 * @return array $scope All available PDF structure definitions for the submitted account
 * scope. Each entry is a string being the filename that may be passed to the
 * createModulePDF() function as second argument.
 */
function getPDFStructures($scope = "user", $profile = null) {
	$return = array();

	if (!isset($profile)) {
		$profile = $_SESSION['config']->getName();
	}
	$path = dirname(__FILE__) . '/../config/pdf/' . $profile;
	if(is_dir($path)) {
		$dirHandle = opendir($path);
		while($file = readdir($dirHandle)) {
			$struct_file = explode('.',$file);
			if(!is_dir($path.$file) && ($file != '.') && ($file != '..') && (sizeof($struct_file) == 3) && ($struct_file[1] == $scope) && ($struct_file[2] == 'xml')) {
				array_push($return, $struct_file[0]);
			}
		}
		sort($return);
	}
 	return $return;
}

/**
 * This function is used to get the PDF structure from XML file.
 *
 * @param string $typeId the account type
 * @param string $name structure name
 *
 * @return array PDF structure
 */
function loadPDFStructure($typeId, $name='default') {
	if (!isValidPDFStructureName($name) || !preg_match('/[a-zA-Z]+/', $typeId)) {
		return null;
	}
	$parser = new xmlParser();
	$file = dirname(__FILE__) . '/../config/pdf/' . $_SESSION['config']->getName() . '/' . $name . '.' . $typeId . '.xml';
	$xml = $parser->parse($file);
	$border = array();
	$structure = array();
	$complete_page_definitions = array('filename' => 'printLogo.jpg', 'headline' => 'LDAP Account Manager');
	if (!empty($xml)) {
		$border['start'] = $xml[1]['PDF'][0];
		$page_definitions = $xml[0][$xml[1]['PDF'][0]]['attributes'];
		foreach($page_definitions as $key => $value) {
			$complete_page_definitions[strtolower($key)] = $value;
			unset($page_definitions[$key]);
		}
		$border['end'] = $xml[1]['PDF'][1];
		$structure = array_slice($xml[0],$border['start'] + 1,$border['end'] - ($border['start'] + 1));
	}
	return array('structure' => $structure, 'page_definitions' => $complete_page_definitions);
}


/**
 * Saves PDF structure to XML file in format: <name>.<typeId>.xml
 *
 * @param string $typeId account type
 * @param string $name name of structure
 * @return string "no perms" if access denied or "ok".
 */
function savePDFStructure($typeId, $name) {
	if (!isValidPDFStructureName($name) || !preg_match('/[a-zA-Z]+/', $typeId)) {
		return 'no perms';
	}
	$struct_file = dirname(__FILE__) . '/../config/pdf/' . $_SESSION['config']->getName() . '/' . $name . '.' . $typeId . '.xml';
	if(!is_writable(dirname(__FILE__) . '/../config/pdf/' . $_SESSION['config']->getName())) {
		return 'no perms';
	}
	else {
		$handle = @fopen($struct_file,'w');
		if (!$handle) return 'no perms';
		$pdf_attributes = '';
		foreach($_SESSION['currentPageDefinitions'] as $key => $value) {
			$pdf_attributes .= ' ' . $key . '="' . $value . '"';
		}
		$file = '<pdf' . $pdf_attributes . ">\n";
		foreach($_SESSION['currentPDFStructure'] as $entry) {
			$ident = '';
			for($i=0;$i<$entry['level'] -1;$i++) {
				$ident .= "\t";
			}
			$attributes = '';
			if(isset($entry['attributes']) && is_array($entry['attributes'])) {
				foreach($entry['attributes'] as $key => $value) {
					$attributes .= ' ' . strtolower($key) . '="' . $value . '"';
				}
			}
			if($entry['type'] == 'open') {
				$file .= $ident . '<' . strtolower($entry['tag']) . $attributes . ">\n";
			}
			elseif($entry['type'] == 'close') {
				$file .= $ident . '</' . strtolower($entry['tag']) . ">\n";
			}
			elseif($entry['type'] == 'complete') {
				if(isset($entry['value'])) {
					$file .= $ident . '<' . strtolower($entry['tag']) . $attributes . '>' . $entry['value'] . '</' . strtolower($entry['tag']) . ">\n";
				}
				else {
					$file .= $ident . '<' . strtolower($entry['tag']) . $attributes . " />\n";
				}
			}
		}
		$file .= "</pdf>";
		fwrite($handle,$file);
		fclose($handle);
		return 'ok';
	}
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
	$activeTypes = $_SESSION['config']->get_ActiveTypes();
	foreach ($activeTypes as $type) {
		$structures = getPDFStructures($type);
		foreach ($structures as $structure) {
			$data = loadPDFStructure($type, $structure);
			if ($data['page_definitions']['filename'] == $name) {
				return new htmlStatusMessage('ERROR', _('Unable to delete logo file.'),
					sprintf(_('Logo is still in use by PDF structure "%s" in account type "%s".'), $structure, \LAM\TYPES\getTypeAlias($type)));
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
	return preg_match('/[a-zA-Z0-9\-\_]+/',$name) === 1;
}

?>
