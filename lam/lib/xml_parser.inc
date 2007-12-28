<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003 - 2006  Michael Duergner

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
 * LDAP Account Manager xmlParser class
 * 
 * @author Michael Duergner
 * @package PDF
 */

/**
 * 
 * 
 * @author Michael Duergner
 * @package PDF
 */
class xmlParser {
	
	/**
	 * 
	 */
	private $xmlParser;
	
	/**
	 * Constructor
	 */
	function __construct() {
		$this->xmlParser = xml_parser_create();
		xml_set_object($this->xmlParser,$this);
		xml_parser_set_option($this->xmlParser, XML_OPTION_CASE_FOLDING, 1);
		xml_parser_set_option($this->xmlParser, XML_OPTION_SKIP_WHITE, 1);
	}
	
	/**
	 * 
	 * 
	 * @param string 
	 * 
	 * @return array 
	 */
	function parse($filename) {
		if(file_exists($filename)) {
			$xmlStructure = array();
			$xmlIndex = array();
			xml_parse_into_struct($this->xmlParser,implode("\n",file($filename)),$xmlStructure,$xmlIndex);
			return array($xmlStructure,$xmlIndex);
		}
		return array();
	}
}
?>