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

/* This class contains all posixGroup LDAP attributes
* and funtioncs required to deal with posixGroup
* posixGroup can only be created when it should be added
* to an array.
* basearray is the same array posixGroup should be added
* to. If basearray is not given the constructor tries to
* create an array with posixGroup and all other required
* objects.
* Example: $user[] = new posixGroup($user);
*
* In container array the following things have to exist:
* account or inetOrgPerson object
* type: 'user' or 'host'
* 'attributes': this is a list of arrays with all ldap attributes wich are allowed for this account
*/
class posixGroup {
	// Constructor
	function posixGroup($base) {
		/* Return an error if posixGroup should be created without
		* base container
		*/
		if (!$base) trigger_error(_('Please create a base object with $var = new accountContainer();'), E_USER_ERROR);
		if (!is_string($base)) trigger_error(_('Please create a new module object with $accountContainer->add_objectClass(\'posixGroup\');'), E_USER_ERROR);
		$this->base = $base;
		// posixGroup is only a valid objectClass for user and host
		if ($_SESSION[$this->base]->get_type() != 'group') trigger_error(_('posixGroup can only be used for groups.'), E_USER_WARNING);
		// Add Array with all attributes and type
		$this->attributes = $_SESSION[$this->base]->get_module_attributes('posixGroup');
		$_SESSION[$this->base]->add_attributes ('posixGroup');
		$this->alias = _('posixGroup');
		// Make references to attributes which already esists in ldap
		$newattributes = array_keys($this->attributes);
		$module = array_keys($_SESSION[$this->base]->module);
		// fixme *** do we have to unset module posixAccuont itself
		for ($i=0; $i<count($module); $i++) {
			foreach ($newattributes as $attribute)
				if (isset($_SESSION[$this->base]->module[$module[$i]]->attributes[$attribute]))
					$this->attributes[$attribute] =& $_SESSION[$this->base]->module[$module[$i]]->attributes[$attribute];
			}
		$this->orig = $this->attributes ;
		$this->changegids=false;
		}

	// Variables
	// name of accountContainer so we can read other classes in accuontArray
	var $base;
	// Use a unix password?
	var $userPassword_no;
	// Lock account?
	var $userPassword_lock;
	// change gids of users and hosts?
	var $changegids;

	// This variable contains all inetOrgPerson attributes
	var $attributes;
	/* If an account was loaded all attributes are kept in this array
	* to compare it with new changed attributes
	*/
	var $orig;

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
			if ($newpassword!='') {
				$this->attributes['userPassword'][0] = base64_encode($_SESSION['ldap']->encrypt($newpassword));
				}
			else $this->attributes['userPassword'][0] = '';
			return 0;
			}
		else {
			if ($this->attributes['userPassword'][0]!='') {
				// Read existing password if set
				$password = $_SESSION['ldap']->decrypt(base64_decode($this->attributes['userPassword'][0]));
				return $password;
				}
			else return '';
			}
		}

	function get_alias() {
		return _('posixGroup');
		}

	/* This function returns a list with all required modules
	*/
	function get_dependencies($scope) {
		if ($scope=='group') return array('require' => array('main'), 'conflict' => array('inetOrgPerson', 'account', 'sambaDomain') );
		return -1;
		}

	function module_ready() {
		return true;
		}

	/* This function returns a list of all html-pages in module
	* This is usefull for mass upload and pdf-files
	* because lam can walk trough all pages itself and do some
	* error checkings
	*/
	function pages() {
		return array('attributes', 'user');
		}

	/* This function returns all ldap attributes
	* which are part of posixGroup and returns
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
		$this->attributes['objectClass'][0] = 'posixGroup';
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
					// Split old password hash in {CRYPT} and password-hash
					$i = 0;
					while ($this->orig['userPassword'][0]{$i} != '}') $i++;
					$passwd = substr($this->orig['userPassword'][0], $i+1 );
					$crypt = substr($this->orig['userPassword'][0], 0, $i+1 );
					// remove trailing ! from password hash
					if ($passwd{0} == '!') $passwd = substr($passwd, 1);
					// Write new password
					if ($this->userPassword_lock) $return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = utf8_encode("$crypt!$passwd");
					else $return[$_SESSION[$this->base]->dn]['modify']['userPassword'][0] = utf8_encode("$crypt$passwd");
					}
				}
			}

		// Remove primary group from users from memberUid
		$users_dn = $_SESSION[$_SESSION[$this->base]->cache]->get_cache('gidNumber', 'posixAccount', 'user');
		$DNs = array_keys($users_dn);
		for ($i=0; $i<count($DNs); $i++) {
			if ($users_dn[$DNs[$i]][0]==$this->attributes['gidNumber'][0]) {
				$thisuser = substr($DNs[$i], 4, strpos($DNs[$i], ",")-4);
				if (@in_array($thisuser, $this->attribtues['memberUid'])) {
					$this->attribtues['memberUid'] = @array_flip($this->attribtues['memberUid']);
					unset($this->attribtues['memberUid'][$thisuser]);
					$this->attribtues['memberUid'] = @array_flip($this->attribtues['memberUid']);
					}
				}
			}

		// Change gids of users and hosts?
		if ($this->changegids) {
			// get gidNumber
			$line=-1;
			for ($i=0; $i<count($_SESSION[$this->ldap]->objectClasses) || $i==-1; $i++) {
				if (strpos($_SESSION[$this->ldap]->objectClasses[$i], "NAME 'posixAccount'")) $line = $i;
				}
			if ($line!=-1) {
				$result = $_SESSION[$_SESSION[$this->base]->cache]->get_cache('gidNumber', 'posixAccount', '*');
				$DNs = array_keys($result);
				for ($i=0; $i<count($DNs); $i++)
					if ($result[$DNs[$i]][0] == $this->orig['gidNumber'][0]) $return[$DNs[$i]]['modify']['gidNumber'][0] = $this->attributes['gidNumber'][0];
				}
			// change primaryGroupID
			$line=-1;
			for ($i=0; $i<count($_SESSION[$this->ldap]->objectClasses) || $i==-1; $i++) {
				if (strpos($_SESSION[$this->ldap]->objectClasses[$i], "NAME 'sambaAccount'")) $line = $i;
				}
			if ($line!=-1) {
				$result = $_SESSION[$_SESSION[$this->base]->cache]->get_cache('primaryGroupID', 'sambaAccount', '*');
				$DNs = array_keys($result);
				for ($i=0; $i<count($DNs); $i++) {
					if ($result[$DNs[$i]][0] == $this->orig['gidNumber'][0]*2+1001 ) $return[$DNs[$i]]['modify']['PrimaryGroupID'][0] = $this->attributes['gidNumber'][0]*2+1001;
					}
				}
			// change sambaPrimaryGroupSID
			$line=-1;
			for ($i=0; $i<count($_SESSION[$this->ldap]->objectClasses) || $i==-1; $i++) {
				if (strpos($_SESSION[$this->ldap]->objectClasses[$i], "NAME 'sambaSamAccount'")) $line = $i;
				}
			if ($line!=-1) {
				$result = $_SESSION[$_SESSION[$this->base]->cache]->get_cache('sambaPrimaryGroupSID', 'sambaSamAccount', '*');
				$DNs = array_keys($result);
				for ($i=0; $i<count($DNs); $i++) {
					// Get Domain SID from name
					$sambaDomains = $_SESSION[$_SESSION[$this->base]->ldap]->search_domains($_SESSION[$_SESSION[$this->base]->config]->get_domainSuffix());
					// Get Domain-SID from group SID
					$domainSID = substr($result[$DNs[$i]], 0, strrpos($result[$DNs[$i]], "-"));
					for ($i=0; $i<count($sambaDomains); $i++ )
						if ($domainSID==$sambaDomains[$i]->SID)
							$RIDbase = $sambaDomains[$i]->RIDbase;
					if ($result[$DNs[$i]][0] == $SID . "-" . $this->orig['gidNumber'][0]*2+1+$RIDbase ) $return[$DNs[$i]]['modify']['sambaPrimaryGroupSID'][0] = $SID . "-" . $this->attributes['gidNumber'][0]*2+1+$RIDbase;
					}
				}
			}

		return $return;
		}

	function delete_attributes($post) {
		$return = array();
		return $return;
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_attributes($post) {
		if ($this->orig['gidNumber'][0]!='' && $post['gidNumber']!=$this->attributes['gidNumber'][0])
			$errors[] = array('INFO', _('GID number'), _('GID number has changed. Please select checkbox to change GID number of users and hosts.'), 'gidNumber');

		// Load attributes
		$this->attributes['cn'][0] = $post['cn'];
		$this->attributes['gidNumber'][0] = $post['gidNumber'];
		$this->attributes['description'][0] = $post['description'];
		if ($post['userPassword_no']) $this->userPassword_no=true;
			else $this->userPassword_no=false;
		if ($post['userPassword_lock']) $this->userPassword_lock=true;
			else $this->userPassword_lock=false;
		if ($post['changegids']) $this->changegids=true;
			else $this->changegids=false;

		if (isset($post['userPassword'])) {
			if ($post['userPassword'] != $post['userPassword2']) {
				$errors[] = array('ERROR', _('Password'), _('Please enter the same password in both password-fields.'), 'userPassword');
				unset ($post['userPassword2']);
				}
				else $this->userPassword($post['userPassword']);
			}
		if ($post['genpass']) $this->userPassword(genpasswd());

		// Check if UID is valid. If none value was entered, the next useable value will be inserted
		// load min and may uidNumber
		$minID = intval($_SESSION[$_SESSION[$this->base]->config]->get_minGID());
		$maxID = intval($_SESSION[$_SESSION[$this->base]->config]->get_maxGID());
		$dn_gids = $_SESSION[$_SESSION[$this->base]->cache]->get_cache('gidNumber', 'posixGroup', '*');
		// get_cache will return an array ( dn1 => array(uidnumber1), dn2 => array(uidnumber2), ... )
		foreach ($dn_gids as $gid) $gids[] = $gid[0];
		if(is_array($gids)) sort ($gids, SORT_NUMERIC);
		if ($this->attributes['gidNumber'][0]=='') {
			// No id-number given
			if ($this->orig['gidNumber'][0]=='') {
				// new account -> we have to find a free id-number
				if (count($gids)!=0) {
					// There are some uids
					// Store highest id-number
					$id = $gids[count($gids)-1];
					// Return minimum allowed id-number if all found id-numbers are too low
					if ($id < $minID) $this->attributes['gidNumber'][0] = $minID;
					// Return higesht used id-number + 1 if it's still in valid range
					if ($id < $maxID) $this->attributes['gidNumber'][0] = $id+1;
					/* If this function is still running we have to fid a free id-number between
					* the used id-numbers
					*/
					$i = intval($minID);
					while (in_array($i, $gids)) $i++;
					if ($i>$maxID)
						$errors[] = array('ERROR', _('ID-Number'), _('No free ID-Number!'), 'gidNumber');
						else {
							$this->attributes['gidNumber'][0] = $i;
							$errors[] = array('WARN', _('ID-Number'), _('It is possible that this ID-number is reused. This can cause several problems because files with old permissions might still exist. To avoid this warning set maxUID to a higher value.'), 'gidNumber');
							}
					}
				else $this->attributes['gidNumber'][0] = $minID;
				// return minimum allowed id-number if no id-numbers are found
				}
			else $this->attributes['gidNumber'][0] = $this->orig['gidNumber'][0];
			// old account -> return id-number which has been used
			}
		else {
			// Check manual ID
			// id-number is out of valid range
			if ( ($this->attributes['gidNumber'][0]!=$post['gidNumber']) && ($this->attributes['gidNumber'][0] < $minID || $this->attributes['gidNumber'][0] > $maxID)) $errors[] = array('ERROR', _('ID-Number'), sprintf(_('Please enter a value between %s and %s!'), $minID, $maxID), 'gidNumber');
			// $uids is allways an array but not if no entries were found
			if (is_array($gids)) {
				// id-number is in use and account is a new account
				if ((in_array($this->attributes['gidNumber'][0], $gids)) && $this->orig['gidNumber'][0]=='') $errors[] = array('ERROR', _('ID-Number'), _('ID is already in use'), 'gidNumber');
				// id-number is in use, account is existing account and id-number is not used by itself
				if ((in_array($this->attributes['gidNumber'][0], $gids)) && $this->orig['gidNumber'][0]!='' && ($this->orig['gidNumber'][0] != $this->attributes['gidNumber'][0]) ) {
					$errors[] = array('ERROR', _('ID-Number'), _('ID is already in use'), 'gidNumber');
					$this->attributes['gidNumber'][0] = $this->orig['gidNumber'][0];
					}
				}
			}

		if (($this->attributes['cn'][0] != $post['cn']) &&  ereg('[A-Z]$', $post['cn']))
			$errors[] = array('WARN', _('Groupname'), _('You are using a capital letters. This can cause problems because windows isn\'t case-sensitive.'), 'cn');
		// Check if Username contains only valid characters
		if ( !ereg('^([a-z]|[A-Z]|[0-9]|[.]|[-]|[_])+$', $this->attributes['cn'][0]))
			$errors[] = array('ERROR', _('Groupname'), _('Groupname contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'), 'cn');

		// Create automatic useraccount with number if original user already exists
		// Reset name to original name if new name is in use
		// Set username back to original name if new username is in use
		if ($_SESSION[$_SESSION[$this->base]->cache]->in_cache($this->attributes['cn'][0],'cn', '*')!=false && ($this->orig['cn'][0]!='')) {
			$this->attributes['cn'][0] = $this->orig['cn'][0];
			}
		// Change uid to a new uid until a free uid is found
		else while ($_SESSION[$_SESSION[$this->base]->cache]->in_cache($this->attributes['cn'][0], 'cn', '*')) {
			// get last character of username
			$lastchar = substr($this->attributes['cn'][0], strlen($this->attributes['cn'][0])-1, 1);
			// Last character is no number
			if ( !ereg('^([0-9])+$', $lastchar))
				/* Last character is no number. Therefore we only have to
				* add "2" to it.
				*/
				$this->attributes['cn'][0] = $this->attributes['cn'][0] . '2';
			 else {
				/* Last character is a number -> we have to increase the number until we've
				* found a groupname with trailing number which is not in use.
				*
				* $i will show us were we have to split groupname so we get a part
				* with the groupname and a part with the trailing number
				*/
				$i=strlen($this->attributes['cn'][0])-1;
				$mark = false;
				// Set $i to the last character which is a number in $account_new->general_username
				while (!$mark) {
					if (ereg('^([0-9])+$',substr($this->attributes['cn'][0], $i, strlen($this->attributes['cn'][0])-$i))) $i--;
						else $mark=true;
					}
				// increase last number with one
				$firstchars = substr($this->attributes['cn'][0], 0, $i+1);
				$lastchars = substr($this->attributes['cn'][0], $i+1, strlen($this->attributes['cn'][0])-$i);
				// Put username together
				$this->attributes['cn'][0] = $firstchars . (intval($lastchars)+1);
				}
			}

		// Show warning if lam has changed username
		if ($this->attributes['cn'][0] != $post['cn']) {
			$errors[] = array('WARN', _('Groupname'), _('Groupname in use. Selected next free groupname.'), 'cn');
			}

		if (!ereg('^([a-z]|[A-Z]|[0-9]|[\|]|[\#]|[\*]|[\,]|[\.]|[\;]|[\:]|[\_]|[\-]|[\+]|[\!]|[\%]|[\&]|[\/]|[\?]|[\{]|[\[]|[\(]|[\)]|[\]]|[\}])*$', $this->userPassword()))
			$errors[] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !'), 'userPassword');
		// Return error-messages
		if (is_array($errors)) return $errors;
		// Go to additional group page when no error did ocour and button was pressed
		if ($post['adduser'])  return 'user';
		return 0;
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_user($post) {
		do { // X-Or, only one if() can be true
			if (isset($post['addusers']) && isset($post['addusers_button'])) { // Add groups to list
				// Add new user
				$this->attributes['memberUid'] = @array_merge($this->attributes['memberUid'], $post['addusers']);
				// remove doubles
				$this->attributes['memberUid'] = @array_flip($this->attributes['memberUid']);
				array_unique($this->attributes['memberUid']);
				$this->attributes['memberUid'] = @array_flip($this->attributes['memberUid']);
				// sort groups
				sort($this->attributes['memberUid']);
				break;
				}
			if (isset($post['removeusers']) && isset($post['removeusers_button'])) { // remove groups from list
				$this->attributes['memberUid'] = array_delete($post['removeusers'], $this->attributes['memberUid']);
				break;
				}
			} while(0);
		if (isset($post['adduser_button']) || isset($post['removeuser_button'])) return 'user';
		if ($post['toattributes']) return 'attributes';
		return 0;
		}

	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_attributes($post) {
		if ($this->attributes['userPassword'][0] != $this->orig['userPassword'][0]) $password=$this->userPassword();
			else $password='';
		echo "<table border=0 width=\"100%\">\n";
		echo "<tr>\n";
		echo '<td>' . _('Groupname') . "*</td>\n";
		echo "<td><input name=\"cn\" type=\"text\" size=\"20\" maxlength=\"20\" value=\"".$this->attributes['cn'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=400\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('GID number') ."</td>\n";
		echo "<td><input name=\"gidNumber\" type=\"text\" size=\"6\" maxlength=\"6\" value=\"".$this->attributes['gidNumber'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=401\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Description') . "</td>\n";
		echo "<td><input name=\"description\" type=\"text\" size=\"30\" maxlength=\"255\" value=\"".$this->attributes['description'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=404\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Group members') . "</td>\n";
		echo "<td><input name=\"adduser\" type=\"submit\" value=\"" . _('Edit groups') . "\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=402\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Password') . "</td>\n";
		echo "<td><input name=\"userPassword\" type=\"password\" size=\"20\" maxlength=\"20\" value=\"$password\"></td>\n";
		echo "<td><input name=\"genpass\" type=\"submit\" value=\"" . _('Generate password') . "\"></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Repeat password') . "</td>\n";
		echo "<td><input name=\"userPassword2\" type=\"password\" size=\"20\" maxlength=\"20\" value=\"";
			if ($post['userPassword2']!='') echo $post['userPassword2'];
			else echo $password;
		echo "\"></td>\n";
		echo "<td></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Use no password') . "</td>\n";
		echo "<td><input name=\"userPassword_no\" type=\"checkbox\"";
			if ($this->userPassword_no) echo " checked ";
			echo "></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=426\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Lock password') . "</td>\n";
		echo "<td><input name=\"userPassword_lock\" type=\"checkbox\"";
			if ($this->userPassword_lock) echo " checked ";
			echo "></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=426\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		if ($this->attributes['gidNumber'][0]!=$this->orig['gidNumber'][0] && $this->orig['gidNumber'][0]!='') {
			echo "<tr>\n";
			echo "<td>" . _('Change GID number of users and hosts') . "</td>\n";
			echo "<td><input name=\"changegids\" type=\"checkbox\"";
				if ($this->changegids) echo " checked ";
				echo "></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=XXX\" target=\"lamhelp\">" . _('Help-XX') . "</a></td>\n";
			echo "</tr>\n";
			}
		echo "</table>\n";
		return 0;
		}

	function display_html_delete($post) {
		return 0;
		}

	function display_html_user($post) {
		// load list with all groups
		$dn_users = $_SESSION[$_SESSION[$this->base]->cache]->get_cache('uid', 'posixAccount', 'user');
		foreach ($dn_users as $user) $users[] = $user[0];
		// sort groups
		sort($users, SORT_STRING);
		// remove groups the user is member of from grouplist
		$users = array_delete($this->attributes['memberUid'], $users);
		// Remove primary group from grouplist
		$users_dn = $_SESSION[$_SESSION[$this->base]->cache]->get_cache('gidNumber', 'posixAccount', 'user');
		$DNs = array_keys($users_dn);
		for ($i=0; $i<count($DNs); $i++) {
			if ($users_dn[$DNs[$i]][0]==$this->attributes['gidNumber'][0]) {
				$thisuser = substr($DNs[$i], 4, strpos($DNs[$i], ",")-4);
				if (in_array($thisuser, $users)) {
					$users = @array_flip($users);
					unset($users[$thisuser]);
					$users = @array_flip($users);
					}
				}
			}
		// sort users
		sort($users);

		echo "<table border=0 width=\"100%\">\n<tr>\n";
		echo "<td><fieldset class=\"".$_SESSION[$this->base]->type."edit-bright\">";
			echo "<legend class=\"".$_SESSION[$this->base]->type."edit-bright\"><b>" . _("Group members") . "</b></legend>\n";
			echo "<table border=0 width=\"100%\">\n<tr>\n";
				echo "<td valign=\"top\">";
				echo "<fieldset class=\"".$_SESSION[$this->base]->type."edit-bright\">";
				echo "<legend class=\"".$_SESSION[$this->base]->type."edit-bright\">" . _("Selected users") . "</legend>\n";
					// Show all groups the user is additional member of
					if (count($this->attributes['memberUid'])!=0) {
						echo "<select name=\"removeusers[]\" class=\"".$_SESSION[$this->base]->type."edit-bright\" size=15 multiple>\n";
						foreach ($this->attributes['memberUid'] as $member)
							if ($member!='') echo "<option>$member</option>\n";
						echo "</select>\n";
						}
					echo "</fieldset></td>\n";
				echo "<td align=\"center\" width=\"10%\"><input type=\"submit\" name=\"addusers_button\" value=\"<=\">";
				echo " ";
				echo "<input type=\"submit\" name=\"removeusers_button\" value=\"=>\"><br><br>";
				echo "<a href=\""."../help.php?HelpNumber=402\" target=\"lamhelp\">"._('Help')."</a></td>\n";
				echo "<td valign=\"top\">\n";
				echo "<fieldset class=\"".$_SESSION[$this->base]->type."edit-bright\">";
				echo "<legend class=\"".$_SESSION[$this->base]->type."edit-bright\">" . _('Available users') . "</legend>\n";
					// show all groups expect these the user is member of
					if (count($users)!=0) {
						echo "<select name=\"addusers[]\" size=15 multiple class=\"".$_SESSION[$this->base]->type."edit-bright\">\n";
						for ($i=0; $i<count($users); $i++)
							if ($users[$i]!='') echo "<option> $users[$i] </option>\n";
						echo "</select>\n";
						}
					echo "</fieldset></td>\n";
				echo "</tr>\n";
				echo "</table>\n";
			echo "<input name=\"toattributes\" type=\"submit\" value=\""; echo _('Back'); echo "\">\n";
			echo "</fieldset>\n";
		echo "</td></tr></table>\n";
		return 0;
		}

	}

?>
