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

/* Session variables which are used:
* $_SESSION['cacheAttributes']: This variable contains a list of attributes and their scope which should be cached
*
* Coockie variables which are used:
* $_COOKIE["IV"], $_COOKIE["Key"]: Needed to en/decrypt passwords.
*
* Variables in basearray which are no objects:
* type: Type of account. Can be user, group, host
* attributes: List of all attributes, how to get them and are theiy required or optional
* dn: current DN without uid= or cn=
* dn_orig: old DN if account was loaded with uid= or cn=

* External functions which are used
* account.inc: findgroups, incache, get_cache, array_delete, getshells
* ldap.inc: pwd_is_enabled, pwd_hash
*/

/* This class contains all sambaGroupMapping LDAP attributes
* and funtioncs required to deal with sambaGroupMapping
* sambaGroupMapping can only be created when it should be added
* to an array.
* basearray is the same array sambaGroupMapping should be added
* to. If basearray is not given the constructor tries to
* create an array with sambaGroupMapping and all other required
* objects.
* Example: $user[] = new sambaGroupMapping($user);
*
* In container array the following things have to exist:
* account or inetOrgPerson object
* type: 'user' or 'host'
* 'attributes': this is a list of arrays with all ldap attributes wich are allowed for this account
*/
class sambaGroupMapping extends baseModule {

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
				'help' => 'TODO',
				'example' => _('domain1'),
				'required' => true
			),
			array(
				'name' => 'sambaGroupMapping_name',
				'description' => _('Samba 3 display name'),
				'help' => 'TODO',
				'example' => _('Domain administrators'),
				'required' => false
			),
			array(
				'name' => 'sambaGroupMapping_rid',
				'description' => _('Samba 3 rid number'),
				'help' => 'TODO',
				'example' => 'DOMAIN_ADMINS',
				'required' => false
			)
		);
		// help Entries
		$return['help'] = array (	'displayName'				=>	array ("ext" => "FALSE", "Headline" => _("Display name"), "Text" => _("Windows clients will show display name as group description.")),
											'sambaSID'				=>	array ("ext" => "FALSE", "Headline" => _("Windows groupname"), "Text" => _("If you want to use a well known RID you can selcet a well known group.")),
											'sambaDomainName'	=>	array ("ext" => "FALSE", "Headline" => _("Domain"), "Text" => _("Windows-Domain of group."). ' '. _("Can be left empty.")));
		
		return $return;
	}

	// Constructor
	function init($base) {
		// call parent init
		parent::init($base);
		// Add Array with all attributes and type
		$this->attributes = $_SESSION[$this->base]->get_module_attributes('sambaGroupMapping');
		$_SESSION[$this->base]->add_attributes ('sambaGroupMapping');
		$this->alias = _('sambaGroupMapping');
		// Make references to attributes which already esists in ldap
		$newattributes = array_keys($this->attributes);
		$module = array_keys($_SESSION[$this->base]->module);
		// fixme *** do we have to unset module posixAccuont itself
		for ($i=0; $i<count($module); $i++) {
			foreach ($newattributes as $attribute)
				if (isset($_SESSION[$this->base]->module[$module[$i]]->attributes[$attribute])) $this->attributes[$attribute] =& $_SESSION[$this->base]->module[$module[$i]]->attributes[$attribute];
			}
		$this->orig = $this->attributes ;
		$this->rids = array ( _('Domain Admins') => 512, _('Domain Users') => 513, _('Domain Guests') => 514, _('Domain Computers') => 515, _('Domain Controllers') => 516,
			_('Domain Certificate Admins') => 517, _('Domain Schema Admins') => 518, _('Domain Enterprise Admins') => 519, _('Domain Policy Admins') => 520 );
		}

	// Variables

	// This variable contains all inetOrgPerson attributes
	var $attributes;
	/* If an account was loaded all attributes are kept in this array
	* to compare it with new changed attributes
	*/
	var $orig;
	// Array of well known rids
	var $rids;


	function module_ready() {
		if ($_SESSION[$this->base]->module['posixGroup']->attributes['gidNumber'][0]=='') return false;
		return true;
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
	
	/* This function returns a list of all html-pages in module
	* This is usefull for mass upload and pdf-files
	* because lam can walk trough all pages itself and do some
	* error checkings
	*/
	function pages() {
		return array('attributes');
		}

	/*
	*/
	function get_help($id) {
		switch ($id) {
			case 'displayName':
				return array ("ext" => "FALSE", "Headline" => _("Display name"), "Text" => _("Windows clients will show display name as group description."));
				break;
			case 'sambaSID':
				return array ("ext" => "FALSE", "Headline" => _("Windows groupname"), "Text" => _("If you want to use a well known RID you can selcet a well known group."));
				break;
			case 'sambaDomainName':
				return array ("ext" => "FALSE", "Headline" => _("Domain"), "Text" => _("Windows-Domain of group."). ' '. _("Can be left empty."));
				break;
		}
		return false;
	}

	/* This function returns all ldap attributes
	* which are part of sambaGroupMapping and returns
	* also their values.
	*/
	function get_attributes() {
		return $this->attributes;
		}

	/* This function loads all attributes into the object
	* $attr is an array as it's retured from ldap_get_attributes
	*/
	function load_attributes($attr) {
		// Load attributes which are displayed
		// unset count entries
		unset ($attr['count']);
		$attributes = array_keys($attr);
		foreach ($attributes as $attribute) unset ($attr[$attribute]['count']);
		// unset double entries
		for ($i=0; $i<count($attr); $i++)
			if (isset($attr[$i])) unset($attr[$i]);
		foreach ($attributes as $attribute) {
			if (isset($this->attributes[$attribute])) {
				// decode as unicode
				$this->attributes[$attribute] = $attr[$attribute];
				for ($i=0; $i<count($this->attributes[$attribute]); $i++) {
					$this->attributes[$attribute][$i] = utf8_decode ($this->attributes[$attribute][$i]);
					$this->orig[$attribute][$i] = utf8_decode ($this->attributes[$attribute][$i]);
					}
				}
			}
		// Values are kept as copy so we can compare old attributes with new attributes
		$this->attributes['objectClass'][0] = 'sambaGroupMapping';
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
		$sambaDomains = $_SESSION[$_SESSION[$this->base]->ldap]->search_domains($_SESSION[$_SESSION[$this->base]->config]->get_domainSuffix());
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

		// Add ObjectClass
		$return[$_SESSION[$this->base]->dn]['notchanged']['objectClass'][0] = 'sambaGroupMapping';
		return $return;
		}

	function delete_attributes($post) {
		return 0;
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_attributes($post, $profile=false) {
		// Load attributes
		$this->attributes['displayName'][0] = $post['displayName'];
		$this->attributes['sambaGroupType'][0] = 2;

		if (!$profile) {
			// Get Domain SID from name
			$sambaDomains = $_SESSION[$_SESSION[$this->base]->ldap]->search_domains($_SESSION[$_SESSION[$this->base]->config]->get_domainSuffix());
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
					if ($_SESSION[$_SESSION[$this->base]->cache]->in_cache($SID."-".$this->rids[$rids[$i]], 'sambaSID', 'group'))
						$errors[] = array('ERROR', _('Special Group'),sprintf( _('There can be only one group %s.'), $rids[$i]), 'sambaSID');
					}
				}
			if (!$wrid) $this->attributes['sambaSID'][0] = $SID."-".($_SESSION[$this->base]->module['posixGroup']->attributes['gidNumber'][0]*2)+$RIDbase+1;
			}

		// Return error-messages
		if (is_array($errors)) return $errors;
		return 0;
		}

	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_attributes($post, $profile=false) {
		// Get Domain SID from name
		$sambaDomains = $_SESSION[$_SESSION[$this->base]->ldap]->search_domains($_SESSION[$_SESSION[$this->base]->config]->get_domainSuffix());
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

	function get_profileOptions() {
		$return = array();
		// get list of domains
		$sambaDomains = $_SESSION['ldap']->search_domains($_SESSION['config']->get_domainSuffix());
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

}

?>
