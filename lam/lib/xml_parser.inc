<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Michael Drgner

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
 * @author Michael Dürgner
 * @version 0.5
 * @package PDF
 * @copyright Copyright (C) 2003-2004 Michael Dürgner
 * @license GPL
 */

/**
 * 
 * 
 * @author Michael Dürgner
 * @version 0.5
 * @package PDF
 */
class xmlParser {
	
	/**
	 * 
	 */
	var $xmlParser;
	
	/**
	 * 
	 */
	function xmlParser() {
		$this->xmlParser = xml_parser_create();
		xml_set_object($this->xmlParser,&$this);
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
			xml_parse_into_struct($this->xmlParser,implode("\n",file($filename)),$xmlStructure,$xmlIndex);
			return array($xmlStructure,$xmlIndex);
		}
	}
}
?>