<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Tilo Lutz

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
	var $rids;
	/** Array of sambaGroupTypes */
	var $sambaGroupTypes;
	
	/**
	* Creates a new module for Samba 3 groups.
	*
	* @param string $scope account type
	*/
	function sambaGroupMapping($scope) {
		// load error messages
		$this->rids = array(
			_('Domain Admins') => 512,
			_('Domain Users') => 513,
			_('Domain Guests') => 514,
			_('Domain Computers') => 515,
			_('Domain Controllers') => 516,
			_('Domain Certificate Admins') => 517,
			_('Domain Schema Admins') => 518,
			_('Domain Enterprise Admins') => 519,
			_('Domain Policy Admins') => 520);

		$this->sambaGroupTypes = array (
			_('User') => 1,
			_('Domain Group') => 2,
			_('Domain') => 3,
			_('Local Group') => 4,
			_('Builtin Group') => 5,
			_('Deleted Account') => 6,
			_('Invalid Account') => 7
			);
		// call parent constructor
		parent::baseModule($scope);
	}

	/**
	* In this function the LDAP account is built up.
	*
	* @param array $rawAccounts list of hash arrays (name => value) from user input
	* @param array $partialAccounts list of hash arrays (name => value) which are later added to LDAP
	* @param array $ids list of IDs for column position (e.g. "posixAccount_uid" => 5)
	* @return array list of error messages if any
	*/
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts) {
		// search existing Samba 3 domains
		$domains = search_domains($_SESSION['config']->get_Suffix('domain'));
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
		$triggered_messages = array();
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
					$triggered_messages[] = $errMsg;
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
				$triggered_messages[] = $errMsg;
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
		return $triggered_messages;
	}

	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_attributes(&$post) {
		// Get Domain SID from name
		$sambaDomains = search_domains($_SESSION['config']->get_Suffix('domain'));
		// Get Domain-SID from group SID
		if ($this->attributes['sambaSID'][0]!='')
			$domainSID = substr($this->attributes['sambaSID'][0], 0, strrpos($this->attributes['sambaSID'][0], "-"));
		for ($i=0; $i<count($sambaDomains); $i++ ) {
			// List with all valid domains
			$sambaDomainNames[] = $sambaDomains[$i]->name;
			if ($domainSID==$sambaDomains[$i]->SID) {
				$SID = $sambaDomains[$i]->SID;
				$sel_domain = $sambaDomains[$i]->name;
				}
			}
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Display name') ),
			1 => array ( 'kind' => 'input', 'name' => 'displayName', 'type' => 'text', 'size' => '30', 'maxlength' => '50', 'value' => $this->attributes['displayName'][0]),
			2 => array ( 'kind' => 'help', 'value' => 'displayName' ));

		$names = array_keys($this->rids);
		$wrid=false;
		for ($i=0; $i<count($names); $i++) {
			if ($this->attributes['sambaSID'][0]==$SID."-".$this->rids[$names[$i]]) {
				$selected[] = $names[$i];
				$wrid=true;
				}
			else $options[] = $names[$i];
			}
		if ($wrid) $options[] = $_SESSION[$this->base]->module['posixGroup']->attributes['cn'][0];
			else $selected[] = $_SESSION[$this->base]->module['posixGroup']->attributes['cn'][0];
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Windows group') ),
			1 => array ( 'kind' => 'select', 'name' => 'sambaSID', 'options' => $options, 'options_selected' => $selected),
			2 => array ( 'kind' => 'help', 'value' => 'sambaSID' ));

		$names = array_keys($this->sambaGroupTypes);
		$selected = array( _('Domain Group') );
		for ($i=0; $i<count($names); $i++) {
			if ($this->attributes['sambaGroupType'][0]==$this->sambaGroupTypes[$names[$i]])
				$selected = array( $names[$i] );
				}
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Group type') ),
			1 => array ( 'kind' => 'select', 'name' => 'sambaGroupType', 'options' => $names , 'options_selected' => $selected ),
			2 => array ( 'kind' => 'help', 'value' => 'sambaDomainName' ));
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Domain') ),
			1 => array ( 'kind' => 'select', 'name' => 'sambaDomainName', 'options' => $sambaDomainNames, 'options_selected' => array ( $sel_domain ) ),
			2 => array ( 'kind' => 'help', 'value' => 'sambaDomainName' ));

		return $return;
		}

	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	*/
	function get_metaData() {
		$return = array();
		// manages group accounts
		$return["account_types"] = array("group");
		// alias name
		$return["alias"] = _('Samba 3');
		// module dependencies
		$return['dependencies'] = array('depends' => array('posixGroup'), 'conflicts' => array());
		// available PDF fields
		$return['PDF_fields'] = array(
			'gidNumber',
			'sambaSID',
			'displayName',
			'sambaGroupType',
			'description'
		);
		// upload fields
		// search existing Samba 3 domains
		if ($_SESSION['loggedIn']) {
			$domains = search_domains($_SESSION['config']->get_Suffix('domain'));
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
					'example' => _('Domain Admins')
				),
				array(
					'name' => 'sambaGroupMapping_groupType',
					'description' => _('Samba group type'),
					'help' => 'type',
					'values' => implode(", ", array_keys($this->sambaGroupTypes) + $this->sambaGroupTypes),
					'example' => '2'
				)
			);
			$return['upload_preDepends'] = array('posixGroup');
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


	/*
	 * (non-PHPDoc)
	 * @see baseModule#get_pdfEntries
	 */
	function get_pdfEntries($account_type = "User") {
		return array(	'sambaGroupMapping_gidNumber' => array('<block><key>' . _('GID number') . '</key><value>' . $this->attributes['gidNumber'][0] . '</value></block>'),
							'sambaGroupMapping_sambaSID' => array('<block><key>' . _('Windows group') . '</key><value>' . $this->attributes['sambaSID'][0] . '</value></block>'),
							'sambaGroupMapping_displayName' => array('<block><key>' . _('Display name') . '</key><value>' . $this->attributes['displayName'][0] . '</value></block>'),
							'sambaGroupMapping_sambaGroupType' => array('<block><key>' . _('Samba group type') . '</key><value>' . $this->attributes['sambaGroupType'][0] . '</value></block>'),
							'sambaGroupMapping_description' => array('<block><key>' . _('Description') . '</key><value>' . $this->attributes['description'][0] . '</value></block>'));
	}


	/**
	* Returns a list of elements for the account profiles.
	*
	* @return profile elements
	*/
	function get_profileOptions() {
		$return = array();
		// get list of domains
		$sambaDomains = search_domains($_SESSION['config']->get_Suffix('domain'));
		$sambaDomainNames = array();
		for ($i = 0; $i < count($sambaDomains); $i++ ) {
			// extract names
			$sambaDomainNames[] = $sambaDomains[$i]->name;
		}
		// domain
		$return[] = array (
			0 => array('kind' => 'text', 'text' => _('Domain')),
			1 => array('kind' => 'select', 'name' => 'sambaGroupMapping_sambaDomainName', 'options' => $sambaDomainNames, 'options_selected' => array ()),
			2 => array('kind' => 'help', 'value' => 'sambaDomainName' ));
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
			$sambaDomains = search_domains($_SESSION['config']->get_Suffix('domain'));
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


	/* This functions return true
	* if all needed settings are done
	*/
	function module_complete() {
		if (!$this->module_ready()) return false;
		if ($this->attributes['sambaSID'][0] == '') return false;
		if ($this->attributes['sambaGroupType'][0] == '') return false;
		return true;
	}


	function module_ready() {
		if ($_SESSION[$this->base]->module['posixGroup']->attributes['gidNumber'][0]=='') return false;
		return true;
	}


	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @param array $post HTTP-POST values
	* @return array list of info/error messages
	*/
	function process_attributes(&$post) {
		// Save attributes
		$this->attributes['displayName'][0] = $post['displayName'];
		$this->attributes['sambaGroupType'][0] = $this->sambaGroupTypes[$post['sambaGroupType']];

		// Get Domain SID from name
		$sambaDomains = search_domains($_SESSION['config']->get_Suffix('domain'));
		for ($i=0; $i<count($sambaDomains); $i++ )
			if ($post['sambaDomainName'] == $sambaDomains[$i]->name) {
				$SID = $sambaDomains[$i]->SID;
				$RIDbase = $sambaDomain[$i]->RIDbase;
				}
		// Load attributes
		$this->attributes['displayName'][0] = $post['displayName'];
		$rids = array_keys($this->rids);
		$wrid = false;
		for ($i=0; $i<count($rids); $i++) {
			if ($post['sambaSID'] == $rids[$i]) {
				$wrid = true;
				// Get Domain SID
				$this->attributes['sambaSID'][0] = $SID."-".$this->rids[$rids[$i]];
				// Do a check if special group is unique
				if ($_SESSION['cache']->in_cache($SID."-".$this->rids[$rids[$i]], 'sambaSID', 'group')) {
					$message = $this->messages['sambaSID'][0];
					$message[] = $rids[$i];
					$triggered_messages['sambaSID'][] = $message;
				}
			}
		}
		if (!$wrid) $this->attributes['sambaSID'][0] = $SID . "-" . ($_SESSION[$this->base]->module['posixGroup']->attributes['gidNumber'][0]*2+$RIDbase+1);

		// Return error-messages
		if (count($triggered_messages)!=0) {
			$this->triggered_messages = $triggered_messages;
			return $triggered_messages;
		}
		else $this->triggered_messages = array();
	}


	/* This function returns an array with 3 entries:
	* array( DN1 ('add' => array($attr), 'remove' => array($attr), 'modify' => array($attr)), DN2 .... )
	* DN is the DN to change. It may be possible to change several DNs,
	* e.g. create a new user and add him to some groups via attribute memberUid
	* add are attributes which have to be added to ldap entry
	* remove are attributes which have to be removed from ldap entry
	* modify are attributes which have to been modified in ldap entry
	*/
	function save_attributes() {
		// Get Domain SID from name
		$sambaDomains = search_domains($_SESSION['config']->get_Suffix('domain'));
		// Get Domain-SID from group SID
		$domainSID = substr($this->attributes['sambaSID'][0], 0, strrpos($this->attributes['sambaSID'][0], "-"));
		for ($i=0; $i<count($sambaDomains); $i++ )
			if ($domainSID==$sambaDomains[$i]->SID)
				$SID = $sambaDomains[$i]->SID;
		$names = array_keys($this->rids);
		$wrid=false;
		for ($i=0; $i<count($names); $i++)
			if ($this->attributes['sambaSID'][0]==$SID."-".$this->rids[$names[$i]]) {
				$wrid=true;
				}
		if (!$wrid) $this->attributes['sambaSID'][0] == $SID."-".($_SESSION[$this->base]->module['posixGroup']->attributes['gidNumber'][0]*2+1+$RIDbase);
		$return = $_SESSION[$this->base]->save_module_attributes($this->attributes, $this->orig);

		return $return;
		}

}

?>
