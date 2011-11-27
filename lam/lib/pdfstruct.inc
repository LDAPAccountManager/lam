<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Michael Duergner
                       2011  Roland Gruber

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
 * 
 * @return array $scope All available PDF structure definitions for the submitted account
 * scope. Each entry is a string being the filename that may be passed to the 
 * createModulePDF() function as second argument.
 */
function getPDFStructureDefinitions($scope = "user") {
	$return = array();
	$path = dirname(__FILE__) . '/../config/pdf/';
	if(is_dir($path)) {
		$dirHandle = opendir($path);
		while($file = readdir($dirHandle)) {
			$struct_file = explode('.',$file);
			if(!is_dir($path.$file) && ($file != '.') && ($file != '..') && ($struct_file[1] == $scope) && ($struct_file[2] == 'xml')) {
				array_push($return, $struct_file[0]);
			}
		}
		sort($return);
	}
 	return $return;
}

/**
 * This function is used to get pdf structure from xml file.
 * Used in createModulePDF.
 *
 * @param string $scope The account scope for wich the PDF structure should be returned.
 * @param string $pdf_structure Structure name of selected scope wich should be returned.
 * 
 * @return array PDF structure
 */
function loadPDFStructureDefinitions($scope='user',$pdf_structure='default') {
	$parser = new xmlParser();
	$file = dirname(__FILE__) . '/../config/pdf/' . $pdf_structure . '.' . $scope . '.xml';
	$xml = $parser->parse($file);
	$border = array();
	$structure = array();
	$complete_page_definitions = array('filename' => 'printLogo.jpg', 'headline' => 'LDAP Account Manager');
	if($xml[0][$xml[1]['PDF'][0]]['attributes']['TYPE'] == $scope) {
		$border['start'] = $xml[1]['PDF'][0];
		$page_definitions = $xml[0][$xml[1]['PDF'][0]]['attributes'];
		foreach($page_definitions as $key => $value) {
			$complete_page_definitions[strtolower($key)] = $value;
			unset($page_definitions[$key]);
		}
		$border['end'] = $xml[1]['PDF'][1];
	}
	$structure = array_slice($xml[0],$border['start'] + 1,$border['end'] - ($border['start'] + 1));
	return array('structure' => $structure, 'page_definitions' => $complete_page_definitions);
}


/**
 * Saves PDF structure definitions to XML file in format: <name>.<scope>.xml
 * 
 * @param string $scope account type
 * @param string $definition Name of definition
 * @return string "no perms" if access denied or "ok".
 */
function savePDFStructureDefinitions($scope,$definition) {
	if (!preg_match('/[a-zA-Z0-9\-\_]+/',$definition)) return 'no perms';
	if (!preg_match('/[a-zA-Z]+/',$scope)) return 'no perms';
	$struct_file = dirname(__FILE__) . '/../config/pdf/' . $definition . '.' . $scope . '.xml';
	if(!is_writable(dirname(__FILE__) . '/../config/pdf/')) {
		return 'no perms';
	}
	else {
		$handle = @fopen($struct_file,'w');
		if (!$handle) return 'no perms';
		$pdf_attributes = '';
		foreach($_SESSION['currentPageDefinitions'] as $key => $value) {
			if($key != 'type') {
				$pdf_attributes .= ' ' . $key . '="' . $value . '"';
			}
		}
		$file = '<pdf type="' . $scope . "\"" . $pdf_attributes . ">\n";
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
 * @param string $scope account type
 * @param string $definition Name of definition to delete
 * 
 * @return boolean True if file was deleted or false if a problem occured.
 */
function deletePDFStructureDefinition($scope, $definition) {
	if (!preg_match('/[a-zA-Z0-9\-\_]+/',$definition)) return false;
	if (!preg_match('/[a-zA-Z]+/',$scope)) return false;
	$file = dirname(__FILE__) . '/../config/pdf/' . $definition . '.' . $scope . '.xml';
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
	$dirPath = dirname(__FILE__) . '/../config/pdf/logos/';
	$dirHandle = opendir($dirPath);
	while($file = readdir($dirHandle)) {
		if(!is_dir($file) && $file != '.' && $file != '..' && preg_match('/\\.(jpg|png)$/',$file)) {
			$infos = getimagesize($dirPath . $file);
			if($infos[0] <= 2000 && $infos[1] <= 300) {
				array_push($return, array('filename' => $file, 'infos' => $infos));
			}
		}
	}
	sort($return);
	return $return;
}
?>