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
		if (!is_string($base)) trigger_error(_('Please create a new module object with $accountContainer->add_objectClass(\'posixAccount\');'), E_USER_ERROR);
		$this->base = $base;
		// posixAccount is only a valid objectClass for user and host
		if (!($_SESSION[$this->base]->get_type() == 'user') && !($_SESSION[$this->base]->get_type() != 'host')) trigger_error(_('posixAccount can only be used for users or hosts.'), E_USER_WARNING);
		/* Check if ldap conatiner is in array and set type
		* users are using inetOrgPerson-, hosts account-container
		*/
		if (!isset($_SESSION[$this->base]->module['inetOrgPerson']) && $_SESSION[$this->base]->type=='user') $_SESSION[$this->base]->add_objectClass('inetOrgPerson');
		if (!isset($_SESSION[$this->base]->module['account']) && $_SESSION[$this->base]->type=='host') $_SESSION[$this->base]->add_objectClass('account');
		// Add Array with all attributes and type
		$this->attributes = $_SESSION[$this->base]->get_module_attributes('posixAccount');
		$_SESSION[$this->base]->add_attributes ('posixAccount');

		$this->alias = _('posixAccount');
		// Add attributes which should be cached
		$_SESSION[$_SESSION[$this->base]->cache]->add_cache(array ('user' => array('cn', 'uid', 'uidNumber'), 'host' => array('cn', 'uid', 'uidNumber'), 'group' => array('cn', 'memberUid')));
		/* Check if at least one group does exist in ldap
		*/
		$groups = findgroups(); // list of all groupnames
		if (count($groups)==0) trigger_error(_('No groups found in ldap.'), E_USER_WARNING);

		// Make references to attributes which already esists in ldap
		$newattributes = array_keys($this->attributes);
		$module = array_keys($_SESSION[$this->base]->module);
		// fixme *** do we have to unset module posixAccuont itself
		for ($i=0; $i<count($module); $i++) {
			foreach ($newattributes as $attribute)
				if (isset($_SESSION[$this->base]->module[$module[$i]]->attributes[$attribute])) $this->attributes[$attribute] =& $_SESSION[$this->base]->module[$module[$i]]->attributes[$attribute];
			}
		$this->orig = $this->attributes ;
		$this->attributes['objectClass'][0] = 'posixAccount';
		}

	// Variables
	// Alias Name. This name is shown in the menu instead of posixAccount
	var $alias;
	// name of accountContainer so we can read other classes in accuontArray
	var $base;

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

	/* This function returns a list with all required modules
	*/
	function dependencies() {
		if ($_SESSION[$this->base]->type=='user') return array('inetOrgPerson');
		if ($_SESSION[$this->base]->type=='host') return array('account');
		// return error if unsupported type is used
		return -1;
		}

	function module_ready() {
		return true;
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_attributes($post) {
		// Load attributes
		$this->attributes['uidNumber'][0] = $post['form_posixAccount_uidNumber'];
		$this->attributes['gidNumber'][0] = getgrnam($post['form_posixAccount_gidNumber']);
		$this->attributes['homeDirectory'][0] = $post['form_posixAccount_homeDirectory'];
		$this->attributes['loginShell'][0] = $post['form_posixAccount_loginShell'];
		$this->attributes['gecos'][0] = $post['form_posixAccount_gecos'];

		// Check if UID is valid. If none value was entered, the next useable value will be inserted
		// load min and may uidNumber
		if ($_SESSION[$this->base]['type']=='user') {
			$minID = intval($_SESSION[$_SESSION[$this->base]->config]->get_minUID());
			$maxID = intval($_SESSION[$_SESSION[$this->base]->config]->get_maxUID());
			}
		else {
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
						$errors[] = array('ERROR', _('ID-Number'), _('No free ID-Number!'));
						else {
							$this->attributes['uidNumber'][0] = $i;
							$errors[] = array('WARN', _('ID-Number'), _('It is possible that this ID-number is reused. This can cause several problems because files with old permissions might still exist. To avoid this warning set maxUID to a higher value.'));
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
			if ( ($this->attributes['uidNumber'][0]!=$post['form_posixAccount_uidNumber']) && ($this->attributes['uidNumber'][0] < $minID || $this->attributes['uidNumber'][0] > $maxID)) $errors[] = array('ERROR', _('ID-Number'), sprintf(_('Please enter a value between %s and %s!'), $minID, $maxID));
			// $uids is allways an array but not if no entries were found
			if (is_array($uids)) {
				// id-number is in use and account is a new account
				if ((in_array($this->attributes['uidNumber'][0], $uids)) && $this->orig['uidNumber'][0]=='') $errors[] = array('ERROR', _('ID-Number'), _('ID is already in use'));
				// id-number is in use, account is existing account and id-number is not used by itself
				if ((in_array($this->attributes['uidNumber'][0], $uids)) && $this->orig['uidNumber'][0]!='' && ($this->orig['uidNumber'][0] != $this->attributes['uidNumber'][0]) ) {
					$errors[] = array('ERROR', _('ID-Number'), _('ID is already in use'));
					$this->attributes['uidNumber'][0] = $this->orig['uidNumber'][0];
					}
				}
			}

		// Check if Homedir is valid
		$this->attributes['homeDirectory'][0] = str_replace('$group', getgrnam($this->attributes['gidNumber'][0]), $this->attributes['homeDirectory'][0]);
		if ($this->attributes['uid'][0] != '')
			$this->attributes['homeDirectory'][0] = str_replace('$user', $this->attributes['uid'][0], $this->attributes['homeDirectory'][0]);
		if ($this->attributes['homeDirectory'][0] != $post['form_posixAccount_homeDirectory']) $errors[] = array('INFO', _('Home directory'), _('Replaced $user or $group in homedir.'));
		if ( !ereg('^[/]([a-z]|[A-Z])([a-z]|[A-Z]|[0-9]|[.]|[-]|[_])*([/]([a-z]|[A-Z])([a-z]|[A-Z]|[0-9]|[.]|[-]|[_])*)*$', $this->attributes['homeDirectory'][0] ))
			$errors[] = array('ERROR', _('Home directory'), _('Homedirectory contains invalid characters.'));
		// Return error-messages
		if (is_array($errors)) return $errors;
		// Go to additional group page when no error did ocour and button was pressed
		if ($post['form_posixAccount_addgroup'])  return 'group';
		return 0;
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_group($post) {
		do { // X-Or, only one if() can be true
			if (isset($post['form_posixAccount_addgroups']) && isset($post['form_posixAccount_addgroups_button'])) { // Add groups to list
				// Add new group
				$this->groups = @array_merge($this->groups, $post['form_posixAccount_addgroups']);
				// remove doubles
				$this->groups = @array_flip($this->groups);
				array_unique($this->groups);
				$this->groups = @array_flip($this->groups);
				// sort groups
				sort($this->groups);
				break;
				}
			if (isset($post['form_posixAccount_removegroups']) && isset($post['form_posixAccount_removegroups_button'])) { // remove groups from list
				$this->groups = array_delete($post['form_posixAccount_removegroups'], $this->groups);
				break;
				}
			} while(0);
		if (isset($post['form_posixAccount_addgroups_button']) || isset($post['form_posixAccount_removegroups_button'])) return 'group';
		if ($post['form_posixAccount_toattributes']) return 'attributes';
		return 0;
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
				for ($i=0; $i<count($this->attributes[$attribute]); $i++) $this->attributes[$attribute][$i] = utf8_decode ($this->attributes[$attribute][$i]);
				}
			}
		// Values are kept as copy so we can compare old attributes with new attributes
		$this->attributes['objectClass'][0] = 'posixAccount';
		$this->orig = $this->attributes;
		// get all additional groupmemberships
		$dn_groups = $_SESSION[$_SESSION[$this->base]->cache]->get_cache('memberUid', 'posixGroup', 'group');
		$DNs = array_keys($dn_groups);
		foreach ($DNs as $DN) {
			if (in_array($attr['uid'], $dn_groups[$DN]))
				$this->groups[] = substr($DN, 3, strpos($DN, ',')-1);
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

		// Remove primary group from additional groups
		for ($i=0; $i<count($this->groups); $i++) {
			if ($this->groups[$i]==getgrnam($this->attributes['gidNumber'])) unset($this->groups[$i]);
			}

		// Set additional group memberships
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
					if (in_array($dn_cns[$DN], $add)) $return[$DN]['add']['memberUid'] = $this->attributes['uid'];
					if (in_array($dn_cns[$DN], $remove)) $return[$DN]['remove']['memberUid'] = $this->attributes['uid'];
					}
				// primary group mut also be removed if it has changed after setting additional groups
				if (in_array(getgrnam($this->attributes['gidNumber']), $this->groups_orig)) $return[$DN]['remove']['memberUid'] = $this->attributes['uid'];
				}
			else {
				// Add user to every group
				$dn_cns = $_SESSION[$_SESSION[$this->base]->cache]->get_cache('cn', 'posixGroup', 'group');
				// get_cache will return an array ( dn1 => array(cn1), dn2 => array(cn2), ... )
				$DNs = array_keys($dn_cns);
				foreach ($DNs as $DN) {
					if (in_array($dn_cns[$DN], $this->groups)) $return[$DN]['add']['memberUid'] = $this->attributes['uid'];
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
					if (in_array($dn_cns[$DN], $this->orig['groups'])) $return[$DN]['remove']['memberUid'] = $this->attributes['uid'];
					}
				}
			}
		return $return;
		}


	/* This function returns all ldap attributes
	* which are part of posixAccount and returns
	* also their values.
	*/
	function get_attributes() {
		return $this->attributes;
		}

	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_attributes($post) {
		$groups = findgroups(); // list of all groupnames
		$shelllist = getshells(); // list of all valid shells
		echo "<table border=0 width=\"100%\">\n<tr>\n";
		echo "<td>" . _('UID number') ."</td>\n";
		echo "<td><input name=\"form_posixAccount_uidNumber\" type=\"text\" size=\"6\" maxlength=\"6\" value=\"".$this->attributes['uidNumber'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=401\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Primary group') . "*</td>\n";
		echo "<td><select name=\"form_posixAccount_group\">";
		// loop trough existing groups
		foreach ($groups as $group)
			if (getgrnam($this->attributes['gidNumber'][0]) == $group) echo "<option selected> $group </option>\n";
				else echo "<option> $group </option>\n";
		echo "</select></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=406\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		if ($_SESSION[$this->base]->type=='user') {
			echo "<tr>\n";
			echo "<td>" . _('Additional groups') . "</td>\n";
			echo "<td><input name=\"form_posixAccount_addgroup\" type=\"submit\" value=\"" . _('Edit groups') . "\"></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=402\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			echo "<tr>\n";
			echo "<td>" . _('Home directory') . "*</td>\n";
			echo "<td><input name=\"form_posixAccount_homeDirectory\" type=\"text\" size=\"30\" maxlength=\"255\" value=\"".$this->attributes['homeDirectory'][0]."\"></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=403\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			}
		echo "<tr>\n";
		echo "<td>" . _('Gecos') . "</td>\n";
		echo "<td><input name=\"form_posixAccount_gecos\" type=\"text\" size=\"30\" maxlength=\"255\" value=\"".$this->attributes['gecos'][0]."\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=404\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		if ($_SESSION[$this->base]->type=='user') {
			if (count($shelllist)!=0) {
				echo "<tr>\n";
				echo "<td>" . _('Login shell') . "*</td>\n";
				echo "<td><select name=\"form_posixAccount_loginShell\">";
				// loop through shells
				foreach ($shelllist as $shell)
					if ($this->attributes['loginShell'][0]==trim($shell)) echo "<option selected> $shell </option>\n";
							else echo "<option> $shell </option>\n";
				echo "</select></td>\n";
				echo "<td><a href=\"../help.php?HelpNumber=405\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
				echo "</tr>\n";
				}
			}
		echo "</table>\n";
		return 0;
		}

	function display_html_group($post) {
		// load list with all groups
		$dn_groups = $_SESSION[$_SESSION[$this->base]->cache]->get_cache('uidNumber', 'posixGroup', 'group');
		foreach ($dn_groups as $group) $groups[] = $group[0];
		// sort groups
		sort($groups, SORT_STRING);
		// remove groups the user is member of from grouplist
		$groups = array_delete($this->groups, $groups);
		// Remove primary group from grouplist
		$groups = array_flip($groups);
		if (isset($groups[getgrnam($this->attributes['gidNumber'])])) unset ($groups[getgrnam($this->attributes['gidNumber'])]);
		$groups = array_flip($groups);

		echo "<table border=0 width=\"100%\">\n<tr>\n";
		echo "<td><fieldset class=\"".$_SESSION[$this->base]->type."edit-bright\">";
			echo "<legend class=\"".$_SESSION[$this->base]->type."edit-bright\"><b>" . _("Additional groups") . "</b></legend>\n";
			echo "<table border=0 width=\"100%\">\n<tr>\n";
				echo "<td valign=\"top\">";
				echo "<fieldset class=\"".$_SESSION[$this->base]->type."edit-bright\">";
				echo "<legend class=\"".$_SESSION[$this->base]->type."edit-bright\">" . _("Selected groups") . "</legend>\n";
					// Show all groups the user is additional member of
					if (count($this->groups)!=0) {
						echo "<select name=\"form_posixAccount_removegroups[]\" class=\"".$_SESSION[$this->base]->type."edit-bright\" size=15 multiple>\n";
						for ($i=0; $i<count($this->groups); $i++)
							if ($this->groups[$i]!='') echo "<option> $this->groups[$i] </option>\n";
						echo "</select>\n";
						}
					echo "</fieldset></td>\n";
				echo "<td align=\"center\" width=\"10%\"><input type=\"submit\" name=\"form_posixAccount_addgroups_button\" value=\"<=\">";
				echo " ";
				echo "<input type=\"submit\" name=\"form_posixAccount_removegroups_button\" value=\"=>\"><br><br>";
				echo "<a href=\""."../help.php?HelpNumber=402\" target=\"lamhelp\">"._('Help')."</a></td>\n";
				echo "<td valign=\"top\">\n";
				echo "<fieldset class=\"".$_SESSION[$this->base]->type."edit-bright\">";
				echo "<legend class=\"".$_SESSION[$this->base]->type."edit-bright\">" . _('Available groups') . "</legend>\n";
					// show all groups expect these the user is member of
					if (count($groups)!=0) {
						echo "<select name=\"form_posixAccount_addgroups[]\" size=15 multiple class=\"".$_SESSION[$this->base]->type."edit-bright\">\n";
						for ($i=0; $i<count($groups); $i++)
							if ($groups[$i]!='') echo "<option> $groups[$i] </option>\n";
						echo "</select>\n";
						}
					echo "</fieldset></td>\n";
				echo "</tr>\n";
				echo "</table>\n";
			echo "<input name=\"form_posixAccount_toattributes\" type=\"submit\" value=\""; echo _('Back'); echo "\">\n";
			echo "</fieldset>\n";
		echo "</td></tr></table>\n";
		return 0;
		}


	}



?>
