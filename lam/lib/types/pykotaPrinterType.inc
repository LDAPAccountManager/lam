<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2013 - 2017  Roland Gruber

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
* The account type for PyKota printers.
*
* @package types
* @author Roland Gruber
*/

/**
* The account type for PyKota printers.
*
* @package types
*/
class pykotaPrinterType extends baseType {

	/**
	 * Constructs a new printer type object.
	 *
	 * @param ConfiguredType $type configuration
	 */
	public function __construct($type) {
		parent::__construct($type);
		$this->LABEL_CREATE_ANOTHER_ACCOUNT = _('Create another printer');
		$this->LABEL_BACK_TO_ACCOUNT_LIST = _('Back to printer list');
	}

	/**
	* Returns the alias name of this account type.
	*
	* @return string alias name
	*/
	function getAlias() {
		return _("Printers");
	}

	/**
	* Returns the description of this account type.
	*
	* @return string description
	*/
	function getDescription() {
		return _("PyKota printers");
	}

	/**
	* Returns the class name for the list object.
	*
	* @return string class name
	*/
	function getListClassName() {
		return "lamPykotaPrinterTypeList";
	}

	/**
	* Returns the default attribute list for this account type.
	*
	* @return string attribute list
	*/
	function getDefaultListAttributes() {
		return "#cn;#description;#pykotaPricePerPage;#pykotaPricePerJob;#pykotaMaxJobSize";
	}

	/**
	* Returns a list of attributes which have a translated description.
	* This is used for the head row in the list view.
	*
	* @return array list of descriptions
	*/
	function getListAttributeDescriptions() {
		return array (
			"cn" => _('Printer name'),
			"description" => _('Description'),
			'pykotaPricePerPage' => _('Price per page'),
			'pykotaPricePerJob' => _('Price per job'),
			'pykotaMaxJobSize' => _('Maximum job size'),
		);
	}

	/**
	 * Returns the the title text for the title bar on the new/edit page.
	 *
	 * @param accountContainer $container account container
	 * @return String title text
	 */
	public function getTitleBarTitle($container) {
		// get attributes
		$attributes = array();
		if ($container->getAccountModule('pykotaPrinter') != null) {
			$attributes = $container->getAccountModule('pykotaPrinter')->getAttributes();
		}
		// check if cn is set
		if (isset($attributes['cn'][0])) {
			return htmlspecialchars($attributes['cn'][0]);
		}
		// show new publication label
		if ($container->isNewAccount) {
			return _("New printer");
		}
		// fall back to default
		return parent::getTitleBarTitle($container);
	}

	/**
	 * Returns the the subtitle text for the title bar on the new/edit page.
	 *
	 * @param accountContainer $container account container
	 * @return String title text
	 */
	public function getTitleBarSubtitle($container) {
		// get attributes
		$attributes = array();
		if ($container->getAccountModule('pykotaPrinter') != null) {
			$attributes = $container->getAccountModule('pykotaPrinter')->getAttributes();
		}
		// check if description is set
		if (isset($attributes['description'][0])) {
			return htmlspecialchars($attributes['description'][0]);
		}
		// fall back to default
		return parent::getTitleBarSubtitle($container);
	}

	/**
	 * {@inheritDoc}
	 * @see baseType::getIcon()
	 */
	public function getIcon() {
		return 'printer.png';
	}

}


/**
 * Generates the list view.
 *
 * @package lists
 * @author Roland Gruber
 *
 */
class lamPykotaPrinterTypeList extends lamList {

	/**
	 * Constructor
	 *
	 * @param string $type account type
	 * @return lamList list object
	 */
	function __construct($type) {
		parent::__construct($type);
		$this->labels = array(
			'nav' => _("Printer count: %s"),
			'error_noneFound' => _("No printers found!"),
			'newEntry' => _("New printer"),
			'deleteEntry' => _("Delete selected printers"));
	}

}


?>