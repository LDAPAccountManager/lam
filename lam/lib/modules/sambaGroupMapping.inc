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


class sambaGroupMapping extends baseModule {

	// Variables
	// Array of well known rids
	var $rids;

	
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

		// call parent constructor
		parent::baseModule($scope);
	}

	function delete_attributes($post) {
		return 0;
		}


	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_attributes($post, $profile=false) {
		// Get Domain SID from name
		$sambaDomains = search_domains($_SESSION['config']->get_domainSuffix());
		// Get Domain-SID from group SID
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

		if (!$profile) {
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
			}

		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Domain') ),
			1 => array ( 'kind' => 'select', 'name' => 'sambaDomainName', 'options' => $sambaDomainNames, 'options_selected' => array ( $sel_domain ) ),
			2 => array ( 'kind' => 'help', 'value' => 'sambaDomainName' ));

		return $return;
		}

	function display_html_delete($post) {
		return 0;
		}


	/* This function returns all ldap attributes
	* which are part of sambaGroupMapping and returns
	* also their values.
	*/
	function get_attributes() {
		return $this->attributes;
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
		$return['PDF_fields'] = array(	'gidNumber',
													'sambaSID',
													'displayName',
													'sambaGroupType',
													'description');
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'sambaGroupMapping_domain',
				'description' => _('Samba 3 domain name'),
				'help' => 'sambaDomainName',
				'example' => _('domain1'),
				'required' => true
			),
			array(
				'name' => 'sambaGroupMapping_name',
				'description' => _('Samba 3 display name'),
				'help' => 'displayName',
				'example' => _('Domain administrators'),
				'required' => false
			),
			array(
				'name' => 'sambaGroupMapping_rid',
				'description' => _('Samba 3 RID number'),
				'help' => 'rid',
				'example' => 'DOMAIN_ADMINS',
				'required' => false
			)
		);
		$return['upload_preDepends'] = array('posixGroup');
		// help Entries
		$return['help'] = array(
			'displayName' => array(
				"ext" => "FALSE",
				"Headline" => _("Display name"),
				"Text" => _("This is the group name which will be shown in Windows.")),
			'sambaSID' => array(
				"ext" => "FALSE",
				"Headline" => _("Windows groupname"),
				"Text" => _("If you want to use a well known RID you can selcet a well known group.")),
			'rid' => array(
				"ext" => "FALSE",
				"Headline" => _("Samba RID number"),
				"Text" => _("This is the relative ID (similar to UID on Unix) for Windows accounts. If you leave this empty LAM will calculate the RID from the UID. This can be either a number or the name of a special group:") . ' ' . implode(array_keys($this->rids))),
			'sambaDomainName' => array(
				"ext" => "FALSE",
				"Headline" => _("Domain"),
				"Text" => _("Windows-Domain name of group."). ' '. _("Can be left empty.")));
		
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
		$sambaDomains = search_domains($_SESSION['config']->get_domainSuffix());
		$sambaDomainNames = array();
		for ($i = 0; $i < count($sambaDomains); $i++ ) {
			// extract names
			$sambaDomainNames[] = $sambaDomains[$i]->name;
		}
		// domain
		$return[] = array (
			0 => array('kind' => 'text', 'text' => _('Domain')),
			1 => array('kind' => 'select', 'name' => 'sambaDomainName', 'options' => $sambaDomainNames, 'options_selected' => array ()),
			2 => array('kind' => 'help', 'value' => 'sambaDomainName' ));
		return $return;
	}


	/* This function loads all attributes into the object
	* $attr is an array as it's retured from ldap_get_attributes
	*/
	function load_attributes($attr) {
		$this->load_ldap_attributes($attr);
		return 0;
		}


	/** this functin fills the error message array with messages
	**/
	function load_Messages() {
		$this->messages['sambaSID'][0] = array('ERROR', _('Special Group'),sprintf( _('There can be only one group %s.'), $rids[$i]), 'sambaSID');
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


	/* This function returns a list of all html-pages in module
	* This is usefull for mass upload and pdf-files
	* because lam can walk trough all pages itself and do some
	* error checkings
	*/
	function pages() {
		return array('attributes');
		}


	/* Write variables into object and do some regexp checks
	*/
	function proccess_attributes($post, $profile=false) {
		// Load attributes
		$this->attributes['displayName'][0] = $post['displayName'];
		$this->attributes['sambaGroupType'][0] = 2;

		if (!$profile) {
			// Get Domain SID from name
			$sambaDomains = search_domains($_SESSION['config']->get_domainSuffix());
			for ($i=0; $i<count($sambaDomains); $i++ )
				if ($post['sambaDomainName'] == $sambaDomains[$i]->name) {
					$SID = $sambaDomains[$i]->SID;
					$RIDbase = $sambaDomain[$i]->RIDbase;
					}

			// Load attributes
			$this->attributes['displayName'][0] = $post['displayName'];
			$this->attributes['sambaGroupType'][0] = 2;

			$rids = array_keys($this->rids);
			$wrid = false;
			for ($i=0; $i<count($rids); $i++) {
				if ($post['sambaSID'] == $rids[$i]) {
					$wrid = true;
					// Get Domain SID
					$this->attributes['sambaSID'][0] = $SID."-".$this->rids[$rids[$i]];
					// Do a check if special grou pis unique
					if ($_SESSION['cache']->in_cache($SID."-".$this->rids[$rids[$i]], 'sambaSID', 'group'))
						$errors[] = $this->messages['sambaSID'][0];
					}
				}
			if (!$wrid) $this->attributes['sambaSID'][0] = $SID . "-" . ($_SESSION[$this->base]->module['posixGroup']->attributes['gidNumber'][0]*2+$RIDbase+1);
			}

		// Return error-messages
		if (is_array($errors)) return $errors;
		return 0;
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
		$sambaDomains = search_domains($_SESSION['config']->get_domainSuffix());
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
