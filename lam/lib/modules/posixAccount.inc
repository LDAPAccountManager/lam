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

/* This class contains all posixAccount LDAP attributes
* and funtioncs required to deal with posixAccount
* posixAccount can only be created when it should be added
* to an array.
* basearray is the same array posixAccount should be added
* to. If basearray is not given the constructor tries to
* create an array with posixAccount and all other required
* objects.
* Example: $user[] = new posixAccount($user);
*
* In container array the following things have to exist:
* account or inetOrgPerson object
* type: 'user' or 'host'
* 'attributes': this is a list of arrays with all ldap attributes wich are allowed for this account
*/
class posixAccount {
	// Constructor
	function posixAccount($base) {
		/* Return an error if posixAccount should be created without
		* base container
		*/
		if (!$base) trigger_error(_('Please create a base object with $var = new accountContainer();'), E_USER_ERROR);
		if (!is_string($base)) trigger_error(_('Please create a new module object in an accountContainer object first.'), E_USER_ERROR);
		$this->base = $base;
		// posixAccount is only a valid objectClass for user and host
		if (!($_SESSION[$this->base]->get_type() == 'user') && !($_SESSION[$this->base]->get_type() == 'host')) trigger_error(_('posixAccount can only be used for users or hosts.'), E_USER_WARNING);
		/* Check if ldap conatiner is in array and set type
		* users are using inetOrgPerson-, hosts account-container
		*/
		if (!isset($_SESSION[$this->base]->module['inetOrgPerson']) && $_SESSION[$this->base]->type=='user') $_SESSION[$this->base]->modules['inetOrgPerson'] = new inetOrgPerson($this->base);
		if (!isset($_SESSION[$this->base]->module['account']) && $_SESSION[$this->base]->type=='host') $_SESSION[$this->base]->modules['account'] = new account($this->base);
		// Add Array with all attributes and type
		$this->orig = $_SESSION[$this->base]->get_module_attributes('posixAccount');
		$this->attributes = $_SESSION[$this->base]->get_module_attributes('posixAccount');

		$groups = $_SESSION[$_SESSION[$this->base]->cache]->findgroups(); // list of all groupnames
		if (count($groups)==0) trigger_error(_('No groups found in ldap.'), E_USER_WARNING);

		// Make references to attributes which already esists in ldap
		$newattributes = array_keys($this->attributes);
		$module = array_keys($_SESSION[$this->base]->module);
		for ($i=0; $i<count($module); $i++) {
			foreach ($newattributes as $attribute)
				if (isset($_SESSION[$this->base]->module[$module[$i]]->attributes[$attribute])) $this->attributes[$attribute] =& $_SESSION[$this->base]->module[$module[$i]]->attributes[$attribute];
			}
		$this->attributes['objectClass'][0] = 'posixAccount';
		$this->createhomedir=false;
		}

	// Variables
	// name of accountContainer so we can read other classes in accuontArray
	var $base;
	// Use a unix password?
	var $userPassword_no;
	// Lock account?
	var $userPassword_lock;

	// This variable contains all inetOrgPerson attributes
	var $attributes;
	/* If an account was loaded all attributes are kept in this array
	* to compare it with new changed attributes
	*/
	var $orig;
	/* These two variables keep an array of groups the
	* user is also member of.
	*/
	var $groups;
	var $groups_orig;
	var $createhomedir;

	/* $attribute['userPassword'] can't accessed directly because it's enrcypted
	* To read / write password function userPassword is needed
	* This function will return the unencrypted password when
	* called without a variable
	* If it's called with a new password, the
	* new password will be stored encrypted
	*/
	function userPassword($newpassword=false) {
		if (is_string($newpassword)) {
			// Write new password
			$this->attributes['userPassword'][0] = base64_encode($_SESSION[$_SESSION[$this->base]->ldap]->encrypt($newpassword));
			return 0;
			}
		else {
			if ($this->attributes['userPassword'][0]!='') {
				// Read existing password if set
				return $_SESSION[$_SESSION[$this->base]->ldap]->decrypt(base64_decode($this->attributes['userPassword'][0]));
				}
			else return '';
			}
		}

	function get_alias($scope) {
		return "posixAccount";
	}

	function is_base_module() {
		return true;
	}

	/* This function returns a list with all required modules
	*/
	function get_dependencies($scope) {
		if ($scope=='host') return array('require' => array('account'), 'conflict' => array() );
		if ($scope=='user') return array('require' => array('inetOrgPerson'), 'conflict' => array() );
		return -1;
		}

	function module_ready() {
		return true;
		}

	/* This functions return true
	* if all needed settings are done
	*/
	function module_complete() {
		if (!$this->module_ready()) return false;
		if ($this->attributes['uid'][0] == '') return false;
		if ($this->attributes['uidNumber'][0] == '') return false;
		if ($this->attributes['gidNumber'][0] == '') return false;
		if ($this->attributes['homeDirectory'][0] == '') return false;
		if ($this->attributes['loginShell'][0] == '') return false;
		return true;
		}	
	
	/* This function returns a list of all html-pages in module
	* This is usefull for mass upload and pdf-files
	* because lam can walk trough all pages itself and do some
	* error checkings
	*/
	function pages() {
		return array('attributes', 'group');
		}

	/*
	*/
	function get_help($id) {
		switch ($id) {
			case "description":
				return array ("ext" => "FALSE", "Headline" => _("Description"),
					"Text" => _("Host Description."));
				break;
			}
		return false;
		}

	/* This function returns all ldap attributes
	* which are part of posixAccount and returns
	* also their values.
	*/
	function get_attributes() {
		$return = $this->attributes;
		$return['userPassword'] = $this->userPassword();
		return $return;
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
		$this->attributes['objectClass'][0] = 'posixAccount';

		// get all additional groupmemberships
		$dn_groups = $_SESSION[$_SESSION[$this->base]->cache]->get_cache('memberUid', 'posixGroup', 'group');
		$DNs = array_keys($dn_groups);
		foreach ($DNs as $DN) {
			if (in_array($attr['uid'][0], $dn_groups[$DN])) {
				$this->groups[] = substr($DN, 3, strpos($DN, ',')-3);
				}
			}
		$this->groups_orig = $this->groups;
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
		$return = $_SESSION[$this->base]->save_module_attributes($this->attributes, $this->orig);

		if (isset($return[$_SESSION[$this->base]->dn]['modify']['userPassword']))
			unset($return[$_SESSION[$this->base]->dn]['modify']['userPassword']);
		// Set unix password
		if (count($this->orig['userPassword'])==0) {
			// New user or no old password set
			if ($this->userPassword_no) {
				$return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = pwd_hash ('', !$this->userPassword_lock);
				}
				else $return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = utf8_encode(pwd_hash ($this->userPassword(), !$this->userPassword_lock));
			}
		else {
			if (($this->attributes['userPassword'][0] != $this->orig['userPassword'][0] && $this->userPassword()!='' ) || $this->userPassword_no) {
				// Write new password
				if ($this->userPassword_no) $return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = pwd_hash ('', !$this->userPassword_lock);
					else $return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = utf8_encode(pwd_hash ($this->userPassword(), !$this->userPassword_lock));
				}
			else { // No new password but old password
				// (un)lock password
				if ($this->userPassword_lock == pwd_is_enabled($this->orig['userPassword'][0])) {
					if ($this->userPassword_lock) {
						$return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = utf8_encode(pwd_disable($this->orig['userPassword'][0]));
					}
					else {
						$return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = utf8_encode(pwd_enable($this->orig['userPassword'][0]));
					}
				}
			}
		}

		// Remove primary group from additional groups
		for ($i=0; $i<count($this->groups); $i++) {
			if ($this->groups[$i]==$_SESSION[$_SESSION[$this->base]->cache]->getgrnam($this->attributes['gidNumber'][0])) unset($this->groups[$i]);
			}

		// Set additional group memberships
		if ($this->orig['uid'][0]!='' && $this->attributes['uid'][0]!=$this->orig['uid'][0]) {
			// remove old memberships
			$dn_groups = $_SESSION[$_SESSION[$this->base]->cache]->get_cache('memberUid', 'posixGroup', 'group');
			$DNs = array_keys($dn_groups);
			foreach ($DNs as $DN)
				if (in_array($this->orig['uid'][0], $dn_groups[$DN]))
					$return[$DN]['remove']['memberUid'][0] = $this->orig['uid'][0];
			// Add new memberships
			if (is_array($this->groups))
				foreach ($this->groups as $group) {
					$dn = $_SESSION[$_SESSION[$this->base]->ldap]->in_cache ($group, 'cn', 'group');
					$return[$dn]['add']['memberUid'][0] = $this->attributes['uid'][0];
					}
			}
		else {
			if (is_array($this->groups)) {
				// There are some additional groups defined
				if (is_array($this->groups_orig)) {
					//There are some old groups.
					$add = array_delete($this->groups_orig, $this->groups);
					$remove = array_delete($this->groups, $this->groups_orig);
					$dn_cns = $_SESSION[$_SESSION[$this->base]->cache]->get_cache('cn', 'posixGroup', 'group');
					// get_cache will return an array ( dn1 => array(cn1), dn2 => array(cn2), ... )
					$DNs = array_keys($dn_cns);
					foreach ($DNs as $DN) {
						if (is_array($add))
							if (in_array($dn_cns[$DN][0], $add)) $return[$DN]['add']['memberUid'] = $this->attributes['uid'][0];
						if (is_array($remove))
							if (in_array($dn_cns[$DN][0], $remove)) $return[$DN]['remove']['memberUid'] = $this->attributes['uid'][0];
						}
					// primary group mut also be removed if it has changed after setting additional groups
					if (in_array($_SESSION[$_SESSION[$this->base]->cache]->getgrnam($this->attributes['gidNumber'][0]), $this->groups_orig)) $return[$DN]['remove']['memberUid'] = $this->attributes['uid'];
					}
				else {
					// Add user to every group
					$dn_cns = $_SESSION[$_SESSION[$this->base]->cache]->get_cache('cn', 'posixGroup', 'group');
					// get_cache will return an array ( dn1 => array(cn1), dn2 => array(cn2), ... )
					$DNs = array_keys($dn_cns);
					foreach ($DNs as $DN) {
						if (in_array($dn_cns[$DN][0], $this->groups)) $return[$DN]['add']['memberUid'] = $this->attributes['uid'][0];
						}
					}
				}
			else {
				if (is_array($this->groups_orig)) {
					//There are some old groups which have to be removed
					$dn_cns = $_SESSION[$_SESSION[$this->base]->cache]->get_cache('cn', 'posixGroup', 'group');
					// get_cache will return an array ( dn1 => array(cn1), dn2 => array(cn2), ... )
					$DNs = array_keys($dn_cns);
					foreach ($DNs as $DN) {
						if (in_array($dn_cns[$DN][0], $this->orig['groups'])) $return[$DN]['remove']['memberUid'] = $this->attributes['uid'][0];
						}
					}
				}
			}

		if ($this->createhomedir) $return[$_SESSION[$this->base]->dn]['lamdaemon']['command'][] = $this->attributes['uid'][0] . " home add";
		return $return;
		}

	function delete_attributes($post) {
		$return = array();
		// remove memberUids if set
		$groups = $_SESSION[$_SESSION[$this->base]->cache]->get_cache('memberUid', 'posixGroup', 'group');
		$DNs = array_keys($groups);
		for ($i=0; $i<count($DNs); $i++) {
			if (in_array($this->attributes['uid'][0], $groups[$DNs[$i]])) $return[$DNs[$i]]['remove']['memberUid'][] = $this->attributes['uid'][0];
			}
		if ($post['deletehomedir']) $return[$_SESSION[$this->base]->dn_orig]['lamdaemon']['command'][] = $this->attributes['uid'][0] . " home rem";
		return $return;
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_attributes($post, $profile=false) {
		if ($this->orig['uid'][0]!='' && $post['uid']!=$this->attributes['uid'][0])
			$errors['uid'][] = array('INFO', _('UID'), _('UID has changed. Do you want to change home directory?'));
		if ($this->orig['gidNumber'][0]!='' && $_SESSION[$_SESSION[$this->base]->cache]->getgid($post['gidNumber'])!=$this->attributes['gidNumber'][0])
			$errors['gidNumber'][] = array('INFO', _('GID number'), sprintf(_('GID number has changed. To keep file ownership you have to run the following command as root: \'find / -gid %s -uid %s -exec chgrp %s {} \;\''), $this->orig['gidNumber'][0], $this->orig['uidNumber'][0], $_SESSION[$_SESSION[$this->base]->cache]->getgid($post['gidNumber'])));
		if ($this->orig['uidNumber'][0]!='' && $post['uidNumber']!=$this->attributes['uidNumber'][0])
			$errors['uidNumber'][] = array('INFO', _('UID number'), sprintf(_('UID number has changed. To keep file ownership you have to run the following command as root: \'find / -uid %s -exec chown %s {} \;\''), $this->orig['uidNumber'][0], $this->attributes['uidNumber'][0]));
		if (isset($post['homeDirectory']) && $this->orig['homeDirectory'][0]!='' && $post['homeDirectory']!=$this->attributes['homeDirectory'][0])
			$errors['homeDirectory'][] = array('INFO', _('Home directory'), sprintf(_('Home directory changed. To keep home directory you have to run the following command as root: \'mv %s %s\''), $this->orig['homeDirectory'][0], $this->attributes['homeDirectory'][0]));

		// Load attributes
		$this->attributes['uid'][0] = $post['uid'];
		$this->attributes['cn'][0] = $this->attributes['uid'][0];
		$this->attributes['uidNumber'][0] = $post['uidNumber'];
		$this->attributes['gidNumber'][0] = $_SESSION[$_SESSION[$this->base]->cache]->getgid($post['gidNumber']);
		$this->attributes['homeDirectory'][0] = $post['homeDirectory'];
		$this->attributes['loginShell'][0] = $post['loginShell'];
		$this->attributes['gecos'][0] = $post['gecos'];
		if ($post['createhomedir']) $this->createhomedir = true;
			else $this->createhomedir = false;
		if ($post['userPassword_no']) $this->userPassword_no=true;
			else $this->userPassword_no=false;
		if ($post['userPassword_lock']) $this->userPassword_lock=true;
			else $this->userPassword_lock=false;
		if (!$profile) {
			if ($post['genpass']) $this->userPassword(genpasswd());
				else if (isset($post['userPassword'])) {
					if ($post['userPassword'] != $post['userPassword2']) {
						$errors['userPassword'][] = array('ERROR', _('Password'), _('Please enter the same password in both password-fields.'));
						unset ($post['userPassword2']);
						}
						else $this->userPassword($post['userPassword']);
					}
			
			// Check if UID is valid. If none value was entered, the next useable value will be inserted
			// load min and may uidNumber
			if ($_SESSION[$this->base]->type=='user') {
				$minID = intval($_SESSION[$_SESSION[$this->base]->config]->get_minUID());
				$maxID = intval($_SESSION[$_SESSION[$this->base]->config]->get_maxUID());
				}
			if ($_SESSION[$this->base]->type=='host') {
				$minID = intval($_SESSION[$_SESSION[$this->base]->config]->get_minMachine());
				$maxID = intval($_SESSION[$_SESSION[$this->base]->config]->get_maxMachine());
				}
			$dn_uids = $_SESSION[$_SESSION[$this->base]->cache]->get_cache('uidNumber', 'posixAccount', '*');
			// get_cache will return an array ( dn1 => array(uidnumber1), dn2 => array(uidnumber2), ... )
			foreach ($dn_uids as $uid) $uids[] = $uid[0];
			if(is_array($uids)) sort ($uids, SORT_NUMERIC);
			if ($this->attributes['uidNumber'][0]=='') {
				// No id-number given
				if ($this->orig['uidNumber'][0]=='') {
					// new account -> we have to find a free id-number
					if (count($uids)!=0) {
						// There are some uids
						// Store highest id-number
						$id = $uids[count($uids)-1];
						// Return minimum allowed id-number if all found id-numbers are too low
						if ($id < $minID) $this->attributes['uidNumber'][0] = $minID;
						// Return higesht used id-number + 1 if it's still in valid range
						if ($id < $maxID) $this->attributes['uidNumber'][0] = $id+1;
						/* If this function is still running we have to fid a free id-number between
						* the used id-numbers
						*/
						$i = intval($minID);
						while (in_array($i, $uids)) $i++;
						if ($i>$maxID)
							$errors['uidNumber'][] = array('ERROR', _('ID-Number'), _('No free ID-Number!'));
							else {
								$this->attributes['uidNumber'][0] = $i;
								$errors['uidNumber'][] = array('WARN', _('ID-Number'), _('It is possible that this ID-number is reused. This can cause several problems because files with old permissions might still exist. To avoid this warning set maxUID to a higher value.'));
								}
						}
					else $this->attributes['uidNumber'][0] = $minID;
					// return minimum allowed id-number if no id-numbers are found
					}
				else $this->attributes['uidNumber'][0] = $this->orig['uidNumber'][0];
				// old account -> return id-number which has been used
				}
			else {
				// Check manual ID
				// id-number is out of valid range
				if ( ($this->attributes['uidNumber'][0]!=$post['uidNumber']) && ($this->attributes['uidNumber'][0] < $minID || $this->attributes['uidNumber'][0] > $maxID)) $errors['uidNumber'][] = array('ERROR', _('ID-Number'), sprintf(_('Please enter a value between %s and %s!'), $minID, $maxID));
				// $uids is allways an array but not if no entries were found
				if (is_array($uids)) {
					// id-number is in use and account is a new account
					if ((in_array($this->attributes['uidNumber'][0], $uids)) && $this->orig['uidNumber'][0]=='') $errors['uidNumber'][] = array('ERROR', _('ID-Number'), _('ID is already in use'));
					// id-number is in use, account is existing account and id-number is not used by itself
					if ((in_array($this->attributes['uidNumber'][0], $uids)) && $this->orig['uidNumber'][0]!='' && ($this->orig['uidNumber'][0] != $this->attributes['uidNumber'][0]) ) {
						$errors['uidNumber'][] = array('ERROR', _('ID-Number'), _('ID is already in use'));
						$this->attributes['uidNumber'][0] = $this->orig['uidNumber'][0];
						}
					}
				}
			}
		if ($_SESSION[$this->base]->type=='user') {
			if (($this->attributes['uid'][0] != $post['uid']) &&  ereg('[A-Z]$', $post['uid']) && !$profile)
				$errors['uid'][] = array('WARN', _('Username'), _('You are using a capital letters. This can cause problems because windows isn\'t case-sensitive.'));
			// Check if Homedir is valid
			if (!$profile) {
					$this->attributes['homeDirectory'][0] = str_replace('$group', $_SESSION[$_SESSION[$this->base]->cache]->getgrnam($this->attributes['gidNumber'][0]), $this->attributes['homeDirectory'][0]);
				if ($this->attributes['uid'][0] != '')
					$this->attributes['homeDirectory'][0] = str_replace('$user', $this->attributes['uid'][0], $this->attributes['homeDirectory'][0]);
				if ($this->attributes['homeDirectory'][0] != $post['homeDirectory']) $errors['homeDirecotry'][] = array('INFO', _('Home directory'), _('Replaced $user or $group in homedir.'));
				}
			if ( !ereg('^[/]([a-z]|[A-Z])([a-z]|[A-Z]|[0-9]|[.]|[-]|[_])*([/]([a-z]|[A-Z])([a-z]|[A-Z]|[0-9]|[.]|[-]|[_])*)*$', $this->attributes['homeDirectory'][0] ))
				$errors['homeDirecotry'][] = array('ERROR', _('Home directory'), _('Homedirectory contains invalid characters.'), 'homeDirectory');
			// Check if Username contains only valid characters
			if ( !ereg('^([a-z]|[A-Z]|[0-9]|[.]|[-]|[_])+$', $this->attributes['uid'][0]) && !$profile)
				$errors['uid'][] = array('ERROR', _('Username'), _('Username contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
			}

		if ($_SESSION[$this->base]->type=='host' && !$profile) {
			if (($this->attributes['uid'][0] != $post['form_account_uid']) &&  ereg('[A-Z]$', $post['form_account_uid']))
				$errors['uid'][] = array('WARN', _('Hostname'), _('You are using a capital letters. This can cause problems because windows isn\'t case-sensitive.'));
			// Check if Username contains only valid characters
			if ( !ereg('^([a-z]|[A-Z]|[0-9]|[.]|[-]|[_])+[$]$', $this->attributes['uid'][0]))
				$errors['uid'][] = array('ERROR', _('Hostname'), _('Hostname contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ ! Hostname must end with $ !'));
			}

		// Create automatic useraccount with number if original user already exists
		// Reset name to original name if new name is in use
		// Set username back to original name if new username is in use
		if (!$profile) {
			if ($_SESSION[$_SESSION[$this->base]->cache]->in_cache($this->attributes['uid'][0],'uid', '*')!=false && ($this->orig['uid'][0]!='')) {
				$this->attributes['uid'][0] = $this->orig['uid'][0];
				}
			// Change uid to a new uid until a free uid is found
			else while ($_SESSION[$_SESSION[$this->base]->cache]->in_cache($this->attributes['uid'][0], 'uid', '*')) {
				if ($_SESSION[$this->base]->type=='host') $this->attributes['uid'][0] = substr($this->attributes['uid'][0], 0, -1);
					// get last character of username
				$lastchar = substr($this->attributes['uid'][0], strlen($this->attributes['uid'][0])-1, 1);
				// Last character is no number
				if ( !ereg('^([0-9])+$', $lastchar))
					/* Last character is no number. Therefore we only have to
					* add "2" to it.
					*/
					if ($_SESSION[$this->base]->type=='host') $this->attributes['uid'][0] = $this->attributes['uid'][0] . '2$';
						else $this->attributes['uid'][0] = $this->attributes['uid'][0] . '2';
				else {
					/* Last character is a number -> we have to increase the number until we've
					* found a groupname with trailing number which is not in use.
					*
					* $i will show us were we have to split groupname so we get a part
					* with the groupname and a part with the trailing number
					*/
					$i=strlen($this->attributes['uid'][0])-1;
					$mark = false;
					// Set $i to the last character which is a number in $account_new->general_username
					while (!$mark) {
						if (ereg('^([0-9])+$',substr($this->attributes['uid'][0], $i, strlen($this->attributes['uid'][0])-$i))) $i--;
							else $mark=true;
						}
					// increase last number with one
					$firstchars = substr($this->attributes['uid'][0], 0, $i+1);
					$lastchars = substr($this->attributes['uid'][0], $i+1, strlen($this->attributes['uid'][0])-$i);
					// Put username together
					if ($_SESSION[$this->base]->type=='host') $this->attributes['uid'][0] = $firstchars . (intval($lastchars)+1)."$";
						else $this->attributes['uid'][0] = $firstchars . (intval($lastchars)+1);
					}
				}
		// Show warning if lam has changed username
		if ($_SESSION[$this->base]->type=='user')
			if ($this->attributes['uid'][0] != $post['uid']) {
				$errors['uid'][] = array('WARN', _('Username'), _('Username in use. Selected next free username.'));
				}
		if ($_SESSION[$this->base]->type=='host')
			if ($this->attributes['uid'][0] != $post['uid']) {
				$errors['uid'][] = array('WARN', _('Hostname'), _('Hostname in use. Selected next free hostname.'));
				}
			if (!ereg('^([a-z]|[A-Z]|[0-9]|[\|]|[\#]|[\*]|[\,]|[\.]|[\;]|[\:]|[\_]|[\-]|[\+]|[\!]|[\%]|[\&]|[\/]|[\?]|[\{]|[\[]|[\(]|[\)]|[\]]|[\}])*$', $this->userPassword()))
				$errors['userPassword'][] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !'));
			}
		// Return error-messages
		if (is_array($errors)) return $errors;
		// Go to additional group page when no error did ocour and button was pressed
		if ($post['addgroup'])  return 'group';
		return 0;
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_group($post, $profile=false) {
		do { // X-Or, only one if() can be true
			if (isset($post['addgroups']) && isset($post['addgroups_button'])) { // Add groups to list
				// Add new group
				$this->groups = @array_merge($this->groups, $post['addgroups']);
				// sort groups
				sort($this->groups);
				break;
				}
			if (isset($post['removegroups']) && isset($post['removegroups_button'])) { // remove groups from list
				$this->groups = array_delete($post['removegroups'], $this->groups);
				break;
				}
			} while(0);
		if (isset($post['addgroups_button']) || isset($post['removegroups_button'])) return 'group';
		if ($post['back']) return 'attributes';
		return 0;
		}

	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_attributes($post, $profile=false) {
		$groups = $_SESSION[$_SESSION[$this->base]->cache]->findgroups(); // list of all groupnames
		$shelllist = getshells(); // list of all valid shells

		if (!$profile) {
			if ($this->attributes['userPassword'][0] != $this->orig['userPassword'][0]) $password=$this->userPassword();
				else $password='';
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _("Username").'*' ),
				1 => array ( 'kind' => 'input', 'name' => 'uid', 'type' => 'text', 'size' => '20', 'maxlength' => '20', 'value' => $this->attributes['uid'][0]),
				2 => array ('kind' => 'help', 'value' => 'uid'));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('UID number').'*' ),
				1 => array ( 'kind' => 'input', 'name' => 'uidNumber', 'type' => 'text', 'size' => '6', 'maxlength' => '6', 'value' => $this->attributes['uidNumber'][0]),
				2 => array ('kind' => 'help', 'value' => 'uidNumber'));
			}
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Gecos') ),
			1 => array ( 'kind' => 'input', 'name' => 'gecos', 'type' => 'text', 'size' => '30', 'maxlength' => '255', 'value' => $this->attributes['gecos'][0]),
			2 => array ('kind' => 'help', 'value' => 'gecos'));
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Primary group').'*' ),
			1 => array ( 'kind' => 'select', 'name' => 'gidNumber', 'options' => $groups, 'options_selected' =>
				array ($_SESSION[$_SESSION[$this->base]->cache]->getgrnam($this->attributes['gidNumber'][0]))),
			2 => array ('kind' => 'help', 'value' => 'gidNumber'));

		if ($_SESSION[$this->base]->type=='user') {
			if (!$profile) {
				$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Additional groups') ),
					1 => array ( 'kind' => 'input', 'name' => 'addgroup', 'type' => 'submit', 'value' => _('Edit groups')),
					2 => array ('kind' => 'help', 'value' => 'addgroup'));
				}
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Home directory').'*' ),
				1 => array ( 'kind' => 'input', 'name' => 'homeDirectory', 'type' => 'text', 'size' => '30', 'maxlength' => '255', 'value' => $this->attributes['homeDirectory'][0]),
				2 => array ('kind' => 'help', 'value' => 'homeDirectory'));
			if (!$profile) {
				if ($this->orig['homeDirectory']=='' && isset($_SESSION[$_SESSION[$this->base]->config]->scriptPath)) {
					$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Create home directory') ),
						1 => array ( 'kind' => 'input', 'name' => 'createhomedir', 'type' => 'checkbox', 'checked' => $this->createhomedir),
						2 => array ('kind' => 'help', 'value' => 'createhomedir'));
					}
				$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Password') ),
					1 => array ( 'kind' => 'input', 'name' => 'userPassword', 'type' => 'password', 'size' => '20', 'maxlength' => '255', 'value' => $password),
					2 => array ( 'kind' => 'input', 'name' => 'genpass', 'type' => 'submit', 'value' => _('Generate password')));
				if ($post['userPassword2']!='') $password2 = $post['userPassword2'];
				 else $password2 = $password;
				$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Repeat password') ),
					1 => array ( 'kind' => 'input', 'name' => 'userPassword2', 'type' => 'password', 'size' => '20', 'maxlength' => '255', 'value' => $password2),
					2 => array ('kind' => 'help', 'value' => 'userPassword'));
				}
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Use no password') ),
				1 => array ( 'kind' => 'input', 'name' => 'userPassword_no', 'type' => 'checkbox', 'checked' => $this->userPassword_no),
				2 => array ('kind' => 'help', 'value' => 'userPassword_no'));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Lock password') ),
				1 => array ( 'kind' => 'input', 'name' => 'userPassword_lock', 'type' => 'checkbox', 'checked' => $this->userPassword_lock),
				2 => array ('kind' => 'help', 'value' => 'userPassword_lock'));
			if (count($shelllist)!=0)
				$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Login shell').'*' ),
					1 => array ( 'kind' => 'select', 'name' => 'loginShell', 'options' => $shelllist, 'options_selected' =>
						array ($this->attributes['loginShell'][0])),
					2 => array ('kind' => 'help', 'value' => 'loginShell'));
			}
		return $return;
		}

	function display_html_delete($post) {
		if ($_SESSION[$this->base]->type=='user' && isset($_SESSION[$_SESSION[$this->base]->config]->scriptPath)) {
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Delete home directory') ),
				1 => array ( 'kind' => 'input', 'name' => 'deletehomedir', 'type' => 'checkbox'),
				2 => array ('kind' => 'help', 'value' => 'deletehomedir'));
			}
		return $return;
		}

	function display_html_group($post, $profile=false) {
		// load list with all groups
		$dn_groups = $_SESSION[$_SESSION[$this->base]->cache]->get_cache('gidNumber', 'posixGroup', 'group');
		$DNs = array_keys($dn_groups);
		foreach ($DNs as $DN)
			$groups[] = substr($DN, 3, strpos($DN, ',')-3);
		// remove groups the user is member of from grouplist
		$groups = array_delete($this->groups, $groups);
		// Remove primary group from grouplist
		$group = $_SESSION[$_SESSION[$this->base]->cache]->getgrnam($this->attributes['gidNumber'][0]);
		$groups = array_flip($groups);
		unset ($groups[$group]);
		$groups = array_flip($groups);
		// sort groups
		sort($groups, SORT_STRING);
		
		$return[] = array ( 0 => array ( 'kind' => 'fieldset', 'legend' => _("Additional groups"), 'value' =>
			array ( 0 => array ( 0 => array ('kind' => 'fieldset', 'td' => array ('valign' => 'top'), 'legend' => _("Selected groups"), 'value' =>
				array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'removegroups[]', 'size' => '15', 'multiple', 'options' => $this->groups)))),
			1 => array ( 'kind' => 'table', 'value' => array ( 0 => array ( 0 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'addgroups_button',
				'value' => '<=')), 1 => array ( 0 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'removegroups_button', 'value' => '=>' )),
				2 => array ( 0 => array ( 'kind' => 'help', 'value' => 'addgroup' )))),
			2 => array ('kind' => 'fieldset', 'td' => array ('valign' => 'top'), 'legend' => _("Available groups"), 'value' =>
				array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'addgroups[]', 'size' => '15', 'multiple', 'options' => $groups))))
				))));

		$return[] = array ( 0 => array ( 'kind' => 'input', 'type' => 'submit', 'value' => _('Back'), 'name' => 'back' ),
			1 => array ( 'kind' => 'text'),
			2 => array ('kind' => 'text'));
		return $return;
		}


	}

?>
