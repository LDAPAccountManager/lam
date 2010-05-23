<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2010  Roland Gruber

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
* Interface between modules and other parts of LAM.
*
* @package modules
* @author Roland Gruber
*/

/**
 * Represents a HTML element.
 * This is used to build HTML code by using objects.
 * 
 * @package modules
 */
interface htmlElement {

	/**
	 * Prints the HTML code for this element.
	 * 
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope);

}


?>