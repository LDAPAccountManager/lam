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
	$pdf_entries = $xml[1]['PDF'];
	$border[$current] = array();
	while(($index = current($pdf_entries)) != null) {
		if($xml[0][$index]['attributes']['TYPE'] == $scope) {
			$border['start'] = $index;
			next($pdf_entries);
			$border['end'] = current($pdf_entries);
		}
		next($pdf_entries);
	}
	$structure = array_slice($xml[0],$border['start'] + 1,$border['end'] - ($border['start'] + 1));
	return $structure;
}

function savePDFStructureDefinitions($scope,$definition) {
	$handle = fopen($_SESSION['lampath'] . 'config/pdf/' . $scope . '/' . $definition,'w');
	$file = "<pdf type=\"" . $scope . "\">\n";
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
?>