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

/* This class contains all shadowAccount LDAP attributes
* and funtioncs required to deal with shadowAccount
* shadowAccount can only be created when it should be added
* to an array.
* basearray is the same array shadowAccount should be added
* to. If basearray is not given the constructor tries to
* create an array with shadowAccount and all other required
* objects.
* Example: $user[] = new shadowAccount($user);
*
* In container array the following things have to exist:
* account or inetOrgPerson object
* type: 'user' or 'host'
* 'attributes': this is a list of arrays with all ldap attributes wich are allowed for this account
*/
class shadowAccount {
	// Constructor
	function shadowAccount($base) {
		/* Return an error if shadowAccount should be created without
		* base container
		*/
		if (!$base) trigger_error(_('Please create a base object with $var = new accountContainer();'), E_USER_ERROR);
		if (!is_string($base)) trigger_error(_('Please create a new module object with $accountContainer->add_objectClass(\'shadowAccount\');'), E_USER_ERROR);
		$this->base = $base;
		// shadowAccount is only a valid objectClass for user and host
		if (!($_SESSION[$this->base]->get_type() == 'user')) trigger_error(_('shadowAccount can only be used for users.'), E_USER_WARNING);
		// Add Array with all attributes and type
		$this->attributes = $_SESSION[$this->base]->get_module_attributes('shadowAccount');
		$_SESSION[$this->base]->add_attributes ('shadowAccount');
		$this->alias = _('shadowAccount');
		// Make references to attributes which already esists in ldap
		$newattributes = array_keys($this->attributes);
		$module = array_keys($_SESSION[$this->base]->module);
		// fixme *** do we have to unset module shadowAccount itself
		for ($i=0; $i<count($module); $i++) {
			foreach ($newattributes as $attribute)
				if (isset($_SESSION[$this->base]->module[$module[$i]]->attributes[$attribute])) $this->attributes[$attribute] =& $_SESSION[$this->base]->module[$module[$i]]->attributes[$attribute];
			}
		$this->orig = $this->attributes ;
		$this->attributes['objectClass'][0] = 'shadowAccount';
		}

	// Variables
	// Alias Name. This name is shown in the menu instead of shadowAccount
	var $alias;
	// name of accountContainer so we can read other classes in accuontArray
	var $base;

	// This variable contains all inetOrgPerson attributes
	var $attributes;
	/* If an account was loaded all attributes are kept in this array
	* to compare it with new changed attributes
	*/
	var $orig;

	/* This function returns a list with all required modules
	*/
	function dependencies() {
		return array('inetOrgPerson');
		}

	function module_ready() {
		return true;
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_attributes($post) {
		// Load attributes
		$this->attributes['shadowMin'][0] = $post['form_shadowAccount_shadowMin'];
		$this->attributes['shadowMax'][0] = $post['form_shadowAccount_shadowMax'];
		$this->attributes['shadowWarning'][0] = $post['form_shadowAccount_shadowWarning'];
		$this->attributes['shadowInactive'][0] = $post['form_shadowAccount_shadowInactive'];
		$this->attributes['shadowExpire'][0] = mktime(10, 0, 0, $post['form_shadowAccount_shadowExpire_mon'],
			$post['form_shadowAccount_shadowExpire_day'], $post['form_shadowAccount_shadowExpire_yea'])/3600/24;

		if ( !ereg('^([0-9])*$', $this->attributes['shadowMin'][0]))  $errors[] = array('ERROR', _('Password minage'), _('Password minage must be are natural number.'));
		if ( $this->attributes['shadowMin'][0] > $this->attributes['shadowMax'][0] ) $errors[] = array('ERROR', _('Password maxage'), _('Password maxage must bigger as Password Minage.'));
		if ( !ereg('^([0-9]*)$', $this->attributes['shadowMax'][0])) $errors[] = array('ERROR', _('Password maxage'), _('Password maxage must be are natural number.'));
		if ( !ereg('^(([-][1])|([0-9]*))$', $this->attributes['shadowInactive'][0]))
			$errors[] = array('ERROR', _('Password Expire'), _('Password expire must be are natural number or -1.'));
		if ( !ereg('^([0-9]*)$', $this->attributes['shadowWarning'][0])) $errors[] = array('ERROR', _('Password warn'), _('Password warn must be are natural number.'));

		if (is_array($errors)) return $errors;
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
		$this->attributes['objectClass'][0] = 'shadowAccount';
		$this->orig = $this->attributes;
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

		// Set shadowLastchange manual.
		if ($_SESSION[$this->base]->module['inetOrgPerson']->userPassword()!='' || $_SESSION[$this->base]->module['inetOrgPerson']->userPassword_no)
			$return[$_SESSION[$this->base]->dn]['modify']['shadowLastChange'] = array(time()/3600/24);

		return $return;
		}


	/* This function returns all ldap attributes
	* which are part of shadowAccount and returns
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
		// Use dd-mm-yyyy format of date because it's easier to read for humans
		$date = getdate ($this->attributes['shadowExpire'][0]*3600*24);
		echo "<table border=0 width=\"100%\">\n<tr>\n";
		echo "<td>" . _('Password warn') . "</td>\n";
		echo "<td><input name=\"form_shadowAccount_shadowWarning\" type=\"text\" size=\"4\" maxlength=\"4\" value=\"" . $this->attributes['shadowWarning'][0] . "\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=414\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Password Expire') . "</td>\n";
		echo "<td><input name=\"form_shadowAccount_shadowInactive\" type=\"text\" size=\"4\" maxlength=\"4\" value=\"" . $this->attributes['shadowInactive'][0] . "\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=415\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Maximum password age') . "</td>\n";
		echo "<td><input name=\"form_shadowAccount_shadowMax\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"" . $this->attributes['shadowMax'][0] . "\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=416\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Minimum password age') . "</td>\n";
		echo "<td><input name=\"form_shadowAccount_shadowMin\" type=\"text\" size=\"4\" maxlength=\"4\" value=\"" . $this->attributes['shadowMin'][0] . "\"></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=417\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td>" . _('Expire date') . "</td>\n";
		echo "<td><select name=\"form_shadowAccount_shadowExpire_day\">";
			for ( $i=1; $i<=31; $i++ ) {
				if ($date['mday']==$i) echo "<option selected>$i</option>";
				else echo "<option>$i</option>";
				}
			echo "</select>\n<select name=\"form_shadowAccount_shadowExpire_mon\">";
			for ( $i=1; $i<=12; $i++ ) {
				if ($date['mon'] == $i) echo "<option selected>$i</option>";
				else echo "<option>$i</option>";
				}
			echo "</select>\n<select name=\"form_shadowAccount_shadowExpire_yea\">";
			for ( $i=2030; $i>=2003; $i-- ) {
				if ($date['year']==$i) echo "<option selected>$i</option>";
				else echo "<option>$i</option>";
				}
			echo "</select></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=418\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "</table>\n";
		return 0;
		}


	}



?>
