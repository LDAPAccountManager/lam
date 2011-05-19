<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2009 - 2011  Roland Gruber

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
* This file includes functions to manage LAM tools.
*
* @package tools
* @author Roland Gruber
*/

/**
 * Returns the tools which are available for LAM.
 *
 * @return array list of LAMtool classes
 */
function getTools() {
	$toolsDirName = dirname(__FILE__) . '/tools';
	$toolsDir = dir($toolsDirName);
	$entry = $toolsDir->read();
	// include all files in the tools directory
	while ($entry) {
		if ((substr($entry, strlen($entry) - 4, 4) == '.inc') && is_file($toolsDirName . '/'.$entry)) {
			include_once($toolsDirName . '/'.$entry);
		}
		$entry = $toolsDir->read();
	}
	// find tools classes
	$classList = get_declared_classes();
	$return = array();
	for ($i = 0; $i < sizeof($classList); $i++) {
		if (in_array('LAMTool', class_implements($classList[$i]))) {
			$return[] = $classList[$i];
		}
	}
	return $return;
}

/**
 * Represents a tool.
 * LAM will scan lib/tools/*.inc for classes which implement this interface. This alows to
 * dynamically plugin additional tools. There will be an entry on the tools page inside LAM
 * for each found class (if it matches the security level).
 * A LAMTool only specifies name, description and location of a tool. The tool functionality
 * is provided by the tool's target page.
 *
 * @author Roland Gruber
 * @package tools
 */
interface LAMTool {
	
	/**
	 * Returns the name of the tool.
	 * 
	 * @return string name
	 */
	 function getName();
	
	/**
	 * returns a description text for the tool.
	 * 
	 * @return string description
	 */
	function getDescription();
	
	/**
	 * Returns a link to the tool page (relative to templates/).
	 * 
	 * @return string link
	 */
	function getLink();
	
	/** 
	 * Returns if the tool requires write access to LDAP.
	 * 
	 * @return boolean true if write access is needed
	 */
	function getRequiresWriteAccess();
	
	/**
	 * Returns if the tool requires password change rights.
	 * 
	 * @return boolean true if password change rights are needed
	 */
	function getRequiresPasswordChangeRights();
	
	/**
	 * Returns the link to the tool image (relative to graphics/)
	 *
	 * @return string image URL
	 */
	function getImageLink();
	
	/**
	 * Returns the prefered position of this tool on the tools page.
	 * The position may be between 0 and 1000. 0 is the top position.
	 *
	 * @return int prefered position
	 */
	function getPosition();
	
	/**
	 * Returns a list of sub tools or an empty array.
	 * 
	 * @return array list of subtools (LAMTool)
	 */
	function getSubTools();
	
	/**
	 * Returns if the tool is visible in the menu.
	 *
	 * @return boolean visible
	 */
	function isVisible();
	
}

/**
 * Represents a subtool.
 *
 * @author Roland Gruber
 * @package tools
 */
class LAMSubTool {
	
	/** visible tool name */
	public $name;
	/** tool description */
	public $description;
	/** tool link (relative to templates/) */
	public $link;
	/** image URL (relative to graphics/) */
	public $image;
	
}

?>