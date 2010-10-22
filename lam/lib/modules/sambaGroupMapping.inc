<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Tilo Lutz
                2007 - 2010  Roland Gruber

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
* Manages Samba 3 accounts for groups.
*
* @package modules
*
* @author Tilo Lutz
* @author Roland Gruber
* @author Michael Duergner
*/

/**
* Manages the object class "sambaGroupMapping" for groups.
*
* @package modules
*/
class sambaGroupMapping extends baseModule {

	// Variables
	/** Array of well known RIDs */
	private $rids;
	/** Array of sambaGroupTypes */
	private $sambaGroupTypes;

	/**
	* Creates a new module for Samba 3 groups.
	*
	* @param string $scope account type
	*/
	function __construct($scope) {
		// load error messages
		$this->rids = array(
			_('Domain admins') => 512,
			_('Domain users') => 513,
			_('Domain guests') => 514,
			_('Domain computers') => 515,
			_('Domain controllers') => 516,
			_('Domain certificate admins') => 517,
			_('Domain schema admins') => 518,
			_('Domain enterprise admins') => 519,
			_('Domain policy admins') => 520);

		$this->sambaGroupTypes = array (
			_('User') => 1,
			_('Domain group') => 2,
			_('Domain') => 3,
			_('Local group') => 4,
			_('Builtin group') => 5,
			_('Deleted account') => 6,
			_('Invalid account') => 7
			);
		// call parent constructor
		parent::__construct($scope);
		$this->autoAddObjectClasses = false;
	}

	/**
	 * Gets the GID number from the Unix group module.
	 *
	 * @return String GID number
	 */
	private function getGID() {
		$modules = array('posixGroup', 'rfc2307bisPosixGroup');
		for ($i = 0; $i < sizeof($modules); $i++) {
			if ($this->getAccountContainer()->getAccountModule($modules[$i]) != null) {
				$attrs = $this->getAccountContainer()->getAccountModule($modules[$i])->getAttributes();
				if (isset($attrs['gidNumber'][0])) {
					return $attrs['gidNumber'][0];
				}
			}
		}
		return null;
	}
	
	/**
	 * Gets the cn from the Unix group module.
	 *
	 * @return String cn attribute
	 */
	private function getCn() {
		$modules = array('posixGroup', 'groupOfNames', 'groupOfUniqueNames');
		for ($i = 0; $i < sizeof($modules); $i++) {
			if ($this->getAccountContainer()->getAccountModule($modules[$i]) != null) {
				$attrs = $this->getAccountContainer()->getAccountModule($modules[$i])->getAttributes();
				if (isset($attrs['cn'][0])) {
					return $attrs['cn'][0];
				}
			}
		}
		return null;
	}
	
	/**
	* In this function the LDAP account is built up.
	*
	* @param array $rawAccounts list of hash arrays (name => value) from user input
	* @param array $partialAccounts list of hash arrays (name => value) which are later added to LDAP
	* @param array $ids list of IDs for column position (e.g. "posixAccount_uid" => 5)
	* @param array $selectedModules list of selected account modules
	* @return array list of error messages if any
	*/
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules) {
		// search existing Samba 3 domains
		$domains = search_domains();
		$nameToSID = array();
		// get domain SIDs
		for ($i = 0; $i < sizeof($domains); $i++) {
			$nameToSID[$domains[$i]->name] = $domains[$i]->SID;
		}
		// get domain RID bases
		$nameToRIDBase = array();
		for ($i = 0; $i < sizeof($domains); $i++) {
			$nameToRIDBase[$domains[$i]->name] = $domains[$i]->RIDbase;
		}
		$errors = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// group type
			if ($rawAccounts[$i][$ids['sambaGroupMapping_groupType']] != "") {
				if (in_array($rawAccounts[$i][$ids['sambaGroupMapping_groupType']], $this->sambaGroupTypes)) {  // number given
					$partialAccounts[$i]['sambaGroupType'] = $rawAccounts[$i][$ids['sambaGroupMapping_groupType']];
				}
				elseif (in_array($rawAccounts[$i][$ids['sambaGroupMapping_groupType']], array_keys($this->sambaGroupTypes))) {  // description given
					$partialAccounts[$i]['sambaGroupType'] = $this->sambaGroupTypes[$rawAccounts[$i][$ids['sambaGroupMapping_groupType']]];
				}
				else {  // invalid type
					$errMsg = $this->messages['groupType'][0];
					array_push($errMsg, array($i, implode(", ", array_keys($this->sambaGroupTypes) + $this->sambaGroupTypes)));
					$errors[] = $errMsg;
				}
			}
			else {
				$partialAccounts[$i]['sambaGroupType'] = "2";  // 2 is the default (domain group)
			}
			if (!in_array("sambaGroupMapping", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "sambaGroupMapping";
			// SID
			$domSID = $nameToSID[$rawAccounts[$i][$ids['sambaGroupMapping_domain']]];
			if (!isset($domSID)) {
				$errMsg = $this->messages['sambaSID'][1];
				array_push($errMsg, $rawAccounts[$i][$ids['sambaGroupMapping_domain']]);
				array_push($errMsg, $i);
				$errors[] = $errMsg;
			}
			else {
				// RID
				$rid = $rawAccounts[$i][$ids['sambaGroupMapping_rid']];
				if (isset($this->rids[$rid])) $rid = $this->rids[$rid];
				// check if RID has to be calculated
				if (($rid == "") || (!isset($rid))) {
					$ridBase = $nameToRIDBase[$rawAccounts[$i][$ids['sambaGroupMapping_domain']]];
					$partialAccounts[$i]['sambaSID'] = $domSID . "-" . (($partialAccounts[$i]['gidNumber'] * 2) + $ridBase + 1);
				}
				elseif (get_preg($rid, 'digit')) {
					$partialAccounts[$i]['sambaSID'] = $domSID . "-" . $rid;
				}
			}
			// display name (UTF-8, no regex check needed)
			if ($rawAccounts[$i][$ids['sambaGroupMapping_name']] == "") {
				$partialAccounts[$i]['displayName'] = $partialAccounts[$i]['cn'];
			}
			else {
				$partialAccounts[$i]['displayName'] = $rawAccounts[$i][$ids['sambaGroupMapping_name']];
			}
		}
		return $errors;
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		if (isset($_POST['addObjectClass'])) {
			$this->attributes['objectClass'][] = 'sambaGroupMapping';
		}
		$return = new htmlTable();
		if (in_array('sambaGroupMapping', $this->attributes['objectClass'])) {
			$sambaDomains = search_domains();
			if (sizeof($sambaDomains) == 0) {
				StatusMessage("ERROR", _('No Samba 3 domains found in LDAP! Please create one first.'), '');
				return array();
			}
			// Get Domain-SID from group SID
			if (isset($this->attributes['sambaSID'][0])) {
				$domainSID = substr($this->attributes['sambaSID'][0], 0, strrpos($this->attributes['sambaSID'][0], "-"));
			}
			for ($i=0; $i<count($sambaDomains); $i++ ) {
				// List with all valid domains
				$sambaDomainNames[] = $sambaDomains[$i]->name;
				if (isset($domainSID) && ($domainSID==$sambaDomains[$i]->SID)) {
					$SID = $sambaDomains[$i]->SID;
					$sel_domain = $sambaDomains[$i]->name;
				}
			}
			// display name
			$displayName = '';
			if (isset($this->attributes['displayName'][0])) $displayName = $this->attributes['displayName'][0];
			$displayNameInput = new htmlTableExtendedInputField(_('Display name'), 'displayName', $displayName, 'displayName');
			$displayNameInput->setFieldMaxLength(50);
			$return->addElement($displayNameInput, true);
			// Windows group
			$selected = array();
			$names = array_keys($this->rids);
			$wrid=false;
			for ($i=0; $i<count($names); $i++) {
				if (isset($this->attributes['sambaSID'][0]) && ($this->attributes['sambaSID'][0]==$SID."-".$this->rids[$names[$i]])) {
					$selected[] = $names[$i];
					$wrid=true;
				}
				$options[] = $names[$i];
			}
			if ($wrid) {
				$options[] = $this->getCn();
			}
			else {
				$selected[] = $this->getCn();
			}
			$return->addElement(new htmlTableExtendedSelect('sambaSID', $options, $selected, _('Windows group'), 'sambaSID'), true);
			// group type	
			$names = array_keys($this->sambaGroupTypes);
			$selected = array( _('Domain group') );
			for ($i=0; $i<count($names); $i++) {
				if (!isset($this->attributes['sambaGroupType'][0])) break;
				if ($this->attributes['sambaGroupType'][0]==$this->sambaGroupTypes[$names[$i]]) $selected = array( $names[$i] );
			}
			$return->addElement(new htmlTableExtendedSelect('sambaGroupType', $names, $selected, _('Group type'), 'type'), true);
			// domain
			$selectedDomain = array();
			if (isset($sel_domain)) $selectedDomain = array($sel_domain);
			$return->addElement(new htmlTableExtendedSelect('sambaDomainName', $sambaDomainNames, $selectedDomain, _('Domain'), 'sambaDomainName'), true);
		}
		else {
			$return->addElement(new htmlButton('addObjectClass', _('Add Samba 3 extension')));
		}
		return $return;
	}

	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	* 
	* @see baseModule::get_metaData()
	*/
	function get_metaData() {
		$return = array();
		// icon
		$return['icon'] = 'samba.png';
		// manages group accounts
		$return["account_types"] = array("group");
		// alias name
		$return["alias"] = _('Samba 3');
		// module dependencies
		$return['dependencies'] = array('depends' => array(array('posixGroup', 'rfc2307bisPosixGroup')), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('sambaGroupMapping');
		// managed attributes
		$return['attributes'] = array('gidNumber', 'sambaSID', 'sambaGroupType', 'displayName', 'sambaSIDList', 'description');
		// available PDF fields
		$return['PDF_fields'] = array(
			'gidNumber' => _('GID number'),
			'sambaSID' => _('Windows group'),
			'displayName' => _('Display name'),
			'sambaGroupType' => _('Samba group type'),
			'description' => _('Description')
		);
		// upload fields
		// search existing Samba 3 domains
		if (isset($_SESSION['loggedIn']) && ($_SESSION['loggedIn'] === true)) {
			$domains = search_domains();
			$domainNames = array();
			for ($i = 0; $i < sizeof($domains); $i++) $domainNames[] = $domains[$i]->name;
			$return['upload_columns'] = array(
				array(
					'name' => 'sambaGroupMapping_domain',
					'description' => _('Samba domain name'),
					'help' => 'sambaDomainName',
					'example' => $domainNames[0],
					'values' => implode(", ", $domainNames),
					'required' => true
				),
				array(
					'name' => 'sambaGroupMapping_name',
					'description' => _('Samba display name'),
					'help' => 'displayName',
					'example' => _('Domain administrators')
				),
				array(
					'name' => 'sambaGroupMapping_rid',
					'description' => _('Samba RID number'),
					'help' => 'rid',
					'example' => _('Domain admins')
				),
				array(
					'name' => 'sambaGroupMapping_groupType',
					'description' => _('Samba group type'),
					'help' => 'type',
					'values' => implode(", ", array_keys($this->sambaGroupTypes) + $this->sambaGroupTypes),
					'example' => '2'
				)
			);
			$return['upload_preDepends'] = array('posixGroup', 'rfc2307bisPosixGroup');
		}
		// help Entries
		$return['help'] = array(
			'displayName' => array(
				"Headline" => _("Display name"),
				"Text" => _("This is the group name which will be shown in Windows.")
			),
			'sambaSID' => array(
				"Headline" => _("Windows group name"),
				"Text" => _("If you want to use a well known RID you can selcet a well known group.")
			),
			'rid' => array(
				"Headline" => _("Samba RID number"),
				"Text" => _("This is the relative ID (similar to UID on Unix) for Windows accounts. If you leave this empty LAM will calculate the RID from the UID. This can be either a number or the name of a special group:") . ' ' . implode(", ", array_keys($this->rids))
			),
			'sambaDomainName' => array(
				"Headline" => _("Domain"),
				"Text" => _("Windows-Domain name of group."). ' '. _("Can be left empty.")
			),
			'type' => array(
				"Headline" => _("Samba group type"),
				"Text" => _("Windows group type.")
			)
		);

		return $return;
	}


	/**
	* Returns the PDF entries for this module.
	*
	* @return array list of possible PDF entries
	*/
	function get_pdfEntries() {
		return array(	'sambaGroupMapping_gidNumber' => array('<block><key>' . _('GID number') . '</key><value>' . $this->attributes['gidNumber'][0] . '</value></block>'),
							'sambaGroupMapping_sambaSID' => array('<block><key>' . _('Windows group') . '</key><value>' . $this->attributes['sambaSID'][0] . '</value></block>'),
							'sambaGroupMapping_displayName' => array('<block><key>' . _('Display name') . '</key><value>' . $this->attributes['displayName'][0] . '</value></block>'),
							'sambaGroupMapping_sambaGroupType' => array('<block><key>' . _('Samba group type') . '</key><value>' . $this->attributes['sambaGroupType'][0] . '</value></block>'),
							'sambaGroupMapping_description' => array('<block><key>' . _('Description') . '</key><value>' . $this->attributes['description'][0] . '</value></block>'));
	}


	/**
	* Returns a list of elements for the account profiles.
	*
	* @return htmlElement profile elements
	*/
	function get_profileOptions() {
		$return = new htmlTable();
		// get list of domains
		$sambaDomains = search_domains();
		$sambaDomainNames = array();
		for ($i = 0; $i < count($sambaDomains); $i++ ) {
			// extract names
			$sambaDomainNames[] = $sambaDomains[$i]->name;
		}
		// domain
		$return->addElement(new htmlTableExtendedSelect('sambaGroupMapping_sambaDomainName', $sambaDomainNames, null, _('Domain'), 'sambaDomainName'));
		return $return;
	}


	/**
	* Loads the values of an account profile into internal variables.
	*
	* @param array $profile hash array with profile values (identifier => value)
	*/
	function load_profile($profile) {
		if (isset($profile['sambaGroupMapping_sambaDomainName'][0])) {
			// get list of domains
			$sambaDomains = search_domains();
			for ($i = 0; $i < sizeof($sambaDomains); $i++) {
				if ($sambaDomains[$i]->name == $profile['sambaGroupMapping_sambaDomainName'][0]) {
					$this->attributes['sambaSID'][0] = $sambaDomains[$i]->SID . "-0";
					break;
				}
			}
		}
	}

	/** this functin fills the error message array with messages
	**/
	function load_Messages() {
		$this->messages['sambaSID'][0] = array('ERROR', _('There can be only one group of this type.'));  // third parameter must be set dynamically
		$this->messages['sambaSID'][1] = array('ERROR', _("Account %s:") . " (sambaGroupMapping_domain): " . _("LAM was unable to find a Samba 3 domain with this name!"));  // third parameter must be set dynamically
		$this->messages['groupType'][0] = array('ERROR', _("Account %s:") . " (sambaGroupMapping_type): " . _("This is not a valid Samba 3 group type!"), _("Possible values") . ": %s");
	}


	/**
	* This function is used to check if this module page can be displayed.
	* It returns false if a module depends on data from other modules which was not yet entered.
	*
	* @return boolean true, if page can be displayed
	*/
	function module_ready() {
		if (($this->getGID() == null) || ($this->getGID() == '')) {
			return false;
		}
		return true;
	}

	/**
	* This function is used to check if all settings for this module have been made.
	* 
	* @see baseModule::module_complete
	*
	* @return boolean true, if settings are complete
	*/
	public function module_complete() {
		if (!in_array('sambaGroupMapping', $this->attributes['objectClass'])) {
			return true;
		}
		if (!isset($this->attributes['sambaSID']) || ($this->attributes['sambaSID'] == '')) {
			return false;
		}
		return true;
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_attributes() {
		if (!in_array('sambaGroupMapping', $this->attributes['objectClass'])) {
			return array();
		}
		$errors = array();
		$sambaDomains = search_domains();
		if (sizeof($sambaDomains) == 0) {
			return array();
		}
		// Save attributes
		$this->attributes['displayName'][0] = $_POST['displayName'];
		$this->attributes['sambaGroupType'][0] = $this->sambaGroupTypes[$_POST['sambaGroupType']];

		// Get Domain SID from name
		for ($i=0; $i<count($sambaDomains); $i++ ) {
			if (!isset($_POST['sambaDomainName'])) break;
			if ($_POST['sambaDomainName'] == $sambaDomains[$i]->name) {
				$SID = $sambaDomains[$i]->SID;
				$RIDbase = $sambaDomains[$i]->RIDbase;
			}
		}
		// Load attributes
		$this->attributes['displayName'][0] = $_POST['displayName'];
		$rids = array_keys($this->rids);
		$wrid = false;
		for ($i=0; $i<count($rids); $i++) {
			if ($_POST['sambaSID'] == $rids[$i]) {
				$wrid = true;
				// Get Domain SID
				$this->attributes['sambaSID'][0] = $SID."-".$this->rids[$rids[$i]];
				// Do a check if special group is unique
				if ($this->getAccountContainer()->isNewAccount) {
					$result = searchLDAPByAttribute('sambaSID', $SID."-".$this->rids[$rids[$i]], 'sambaGroupMapping', array('sambaSID'), array('group'));
					if (sizeof($result) > 0) {
						$message = $this->messages['sambaSID'][0];
						$message[] = $rids[$i];
						$errors[] = $message;
					}
				}
			}
		}
		if (!$wrid) {
			$this->attributes['sambaSID'][0] = $SID . "-" . ($this->getGID()*2+$RIDbase+1);
		}
		// Return error-messages
		return $errors;
	}


	/**
	* Returns a list of modifications which have to be made to the LDAP account.
	*
	* @return array list of modifications
	* <br>This function returns an array with 3 entries:
	* <br>array( DN1 ('add' => array($attr), 'remove' => array($attr), 'modify' => array($attr)), DN2 .... )
	* <br>DN is the DN to change. It may be possible to change several DNs (e.g. create a new user and add him to some groups via attribute memberUid)
	* <br>"add" are attributes which have to be added to LDAP entry
	* <br>"remove" are attributes which have to be removed from LDAP entry
	* <br>"modify" are attributes which have to been modified in LDAP entry
	*/
	function save_attributes() {
		if (!in_array('sambaGroupMapping', $this->attributes['objectClass'])) {
			return array();
		}
		return $this->getAccountContainer()->save_module_attributes($this->attributes, $this->orig);
	}

}

?>
