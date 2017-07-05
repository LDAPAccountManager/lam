<?php
use LAM\TYPES\ConfiguredType;

/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2005 - 2017  Roland Gruber

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
* This is the parent class of all account types.
*
* @package types
* @author Roland Gruber
*/

/**
* This is the parent class of all account types.
*
* @package types
*/
class baseType {

	/** label to create another account */
	public $LABEL_CREATE_ANOTHER_ACCOUNT;
	/** label to return to account list */
	public $LABEL_BACK_TO_ACCOUNT_LIST;

	/** type configuration */
	private $type;

	/**
	 * Creates a new type object.
	 *
	 * @param ConfiguredType $type configuration
	 */
	public function __construct($type) {
		$this->type = $type;
		$this->LABEL_CREATE_ANOTHER_ACCOUNT = _('Create another account');
		$this->LABEL_BACK_TO_ACCOUNT_LIST = _('Back to account list');
	}

	/**
	* Returns the alias name of this account type.
	* This function must be overwritten by the child classes.
	*
	* @return string alias name
	*/
	public function getAlias() {
		return "baseType";
	}

	/**
	* Returns the description of this account type.
	* This function must be overwritten by the child classes.
	*
	* @return string description
	*/
	public function getDescription() {
		return "base type";
	}

	/**
	* Returns the class name for the list object.
	*
	* @return string class name
	*/
	public function getListClassName() {
		return "lamList";
	}

	/**
	* Returns the default attribute list for this account type.
	* This function must be overwritten by the child classes.
	*
	* @return string attribute list
	*/
	public function getDefaultListAttributes() {
		return "dn:DN,objectClass:Object classes";
	}

	/**
	* Returns a list of attributes which have a translated description.
	* This is used for the head row in the list view.
	*
	* @return array list of descriptions
	*/
	public function getListAttributeDescriptions() {
		return array();
	}

	/**
	 * Returns if entries of this type may be created via file upload.
	 *
	 * @return boolean true, if file upload is supported
	 */
	public function supportsFileUpload() {
		return true;
	}

	/**
	 * Returns the the title text for the title bar on the new/edit page.
	 *
	 * @param accountContainer $container account container
	 * @return String title text
	 */
	public function getTitleBarTitle($container) {
		if ($container->dn_orig == null) {
			return null;
		}
		return htmlspecialchars(getAbstractDN($container->dn_orig));
	}

	/**
	 * Returns the the title text for the title bar on the new/edit page.
	 *
	 * @param accountContainer $container account container
	 * @return String title text
	 */
	public function getTitleBarSubtitle($container) {
		return null;
	}

	/**
	 * Returns the LDAP filter to find the possible suffixes for this account type.
	 *
	 * @return string LDAP filter
	 */
	public function getSuffixFilter() {
		return "(|(objectClass=organizationalunit)(objectClass=country)(objectClass=organization)(objectClass=krbRealmContainer)(objectClass=container))";
	}

	/**
	 * This function is called after the edit page is processed and before the page content is generated.
	 * This can be used to run custom handlers after each page processing.
	 *
	 * @param accountContainer $container account container
	 */
	public function runEditPagePostAction(&$container) {

	}

	/**
	* Returns a list of configuration options.
	*
	* The field names are used as keywords to load and save settings.
	* We recommend to use the type name as prefix for them (e.g. user_someSetting) to avoid naming conflicts.
	*
	* @return mixed htmlElement or array of htmlElement
	*
	* @see htmlElement
	*/
	public function get_configOptions() {
		return null;
	}

	/**
	* Checks input values of config settings.
	* <br>
	* If the input data is invalid the return value is an array that contains subarrays to build StatusMessages ('message type', 'message head', 'message text').
	* <br>If no errors occured the function returns an empty array.
	*
	* @param array $options hash array (option name => value) that contains the input. The option values are all arrays containing one or more elements.
	* @return array list of error messages
	*/
	public function check_configOptions(&$options) {
		return array();
	}

	/**
	 * Returns if this account type supports multiple configurations.
	 *
	 * @return boolean multiple configs supported
	 */
	public function supportsMultipleConfigs() {
		return true;
	}

	/**
	 * Returns the type configuration.
	 *
	 * @return ConfiguredType type configuration
	 */
	protected function getType() {
		return $this->type;
	}

	/**
	 * Returns the file name of the type icon.
	 * It needs to be 16x16px and located in graphics folder.
	 * By default this is "{type name}.png"
	 *
	 * @return string file name
	 */
	public function getIcon() {
		return get_class($this) . '.png';
	}

	/**
	 * Returns the scope name.
	 *
	 * @return string scope
	 */
	public function getScope() {
		return get_class($this);
	}

}

?>