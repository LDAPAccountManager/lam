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

*/

include_once("config.inc");
include_once("ldap.inc");

function getPDFStructureDefinitions($scope = "user") {
	$return = array();
	$dirHandle = opendir($_SESSION['lampath'] . '/config/pdf/' . $scope . '/');
	while($file = readdir($dirHandle)) {
		if(!is_dir($file) && $file != '.' && $file != '..' && $file != 'default.xml') {
			array_push($return, substr($file,0,strlen($file)-4));
		}
	}
	sort($return);
	return $return;
}

function loadPDFStructureDefinitions($scope='user', $definition='default.xml') {
	$parser = new xmlParser();
	$xml = $parser->parse($_SESSION['lampath'] . 'config/pdf/' . $scope . '/' . $definition);
	
	$border = array();
	$structure = array();
	$border[$current] = array();
	$page_definitions = array();
	if($xml[0][$xml[1]['PDF'][0]]['attributes']['TYPE'] == $scope) {
		$border['start'] = $xml[1]['PDF'][0];
		$page_definitions = $xml[0][$xml[1]['PDF'][0]]['attributes'];
		foreach($page_definitions as $key => $value) {
			$page_definitions[strtolower($key)] = $value;
			unset($page_definitions[$key]);
		}
		$border['end'] = $xml[1]['PDF'][1];
	}
	$structure = array_slice($xml[0],$border['start'] + 1,$border['end'] - ($border['start'] + 1));
	return array('structure' => $structure, 'page_definitions' => $page_definitions);
}

function savePDFStructureDefinitions($scope,$definition) {
	$handle = fopen($_SESSION['lampath'] . 'config/pdf/' . $scope . '/' . $definition,'w');
	$pdf_attributes = '';
	foreach($_SESSION['currentPageDefinitions'] as $key => $value) {
		if($key != 'TYPE') {
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
		if(is_array($entry['attributes'])) {
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
}

function deletePDFStructureDefinition($scope,$definition) {
	return unlink($_SESSION['lampath'] . 'config/pdf/' . $scope . '/' . $definition . '.xml');
}

function getAvailableLogos() {
	$return = array();
	$dirPath = $_SESSION['lampath'] . '/config/pdf/logos/';
	$dirHandle = opendir($dirPath);
	while($file = readdir($dirHandle)) {
		if(!is_dir($file) && $file != '.' && $file != '..' && preg_match('/\\.(jpg|png)$/',$file)) {
			$infos = getimagesize($dirPath . $file);
			if($infos[0] <= 400 && $infos[1] <= 60) {
				array_push($return, array('filename' => $file, 'infos' => $infos));
			}
		}
	}
	sort($return);
	return $return;
}
?>