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
class shadowAccount extends baseModule {

	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	*/
	function get_metaData() {
		$return = array();
		// manages user accounts
		$return["account_types"] = array("user");
		// alias name
		$return["alias"] = _('Shadow');
		return $return;
	}

	// Constructor
	function init($base) {
		/* Return an error if shadowAccount should be created without
		* base container
		*/
		if (!$base) trigger_error(_('Please create a base object with $var = new accountContainer();'), E_USER_ERROR);
		if (!is_string($base)) trigger_error(_('Please create a new module object in an accountContainer object first.'), E_USER_ERROR);
		$this->base = $base;
		// shadowAccount is only a valid objectClass for user and host
		if (!($_SESSION[$this->base]->get_type() == 'user')) trigger_error(_('shadowAccount can only be used for users.'), E_USER_WARNING);
		// Add Array with all attributes and type
		$this->attributes = $_SESSION[$this->base]->get_module_attributes('shadowAccount');
		$_SESSION[$this->base]->add_attributes ('shadowAccount');
		// Make references to attributes which already esists in ldap
		$newattributes = array_keys($this->attributes);
		$module = array_keys($_SESSION[$this->base]->module);
		for ($i=0; $i<count($module); $i++) {
			foreach ($newattributes as $attribute)
				if (isset($_SESSION[$this->base]->module[$module[$i]]->attributes[$attribute])) $this->attributes[$attribute] =& $_SESSION[$this->base]->module[$module[$i]]->attributes[$attribute];
			}
		$this->orig = $this->attributes ;
		$this->attributes['objectClass'][0] = 'shadowAccount';
		}

	// Variables
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
	function get_dependencies($scope) {
		if ($scope=='user') return array('depends' => array('inetOrgPerson'), 'conflicts' => array() );
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
			case "description":
				return array ("ext" => "FALSE", "Headline" => _("Description"),
					"Text" => _("Host Description."));
				break;
			}
		return false;
		}

	/* This function returns all ldap attributes
	* which are part of shadowAccount and returns
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
		$this->attributes['objectClass'][0] = 'shadowAccount';
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
		if (($_SESSION[$this->base]->module['posixAccount']->orig['userPassword'][0] != $_SESSION[$this->base]->module['posixAccount']->attributes['userPassword'][0]  && $_SESSION[$this->base]->module['posixAccount']->userPassword()!='') || $_SESSION[$this->base]->module['posixAccount']->userPassword_no)
			$return[$_SESSION[$this->base]->dn]['modify']['shadowLastChange'] = array(intval(time()/3600/24));
		return $return;
		}

	function delete_attributes($post) {
		return 0;
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_attributes($post, $profile=false) {
		// Load attributes
		$this->attributes['shadowMin'][0] = $post['shadowMin'];
		$this->attributes['shadowMax'][0] = $post['shadowMax'];
		$this->attributes['shadowWarning'][0] = $post['shadowWarning'];
		$this->attributes['shadowInactive'][0] = $post['shadowInactive'];
		$this->attributes['shadowExpire'][0] = intval(mktime(10, 0, 0, $post['shadowExpire_mon'],
			$post['shadowExpire_day'], $post['shadowExpire_yea'])/3600/24);

		if ( !ereg('^([0-9])*$', $this->attributes['shadowMin'][0]))  $errors['shadowMin'][] = array('ERROR', _('Password minage'), _('Password minage must be are natural number.'));
		if ( $this->attributes['shadowMin'][0] > $this->attributes['shadowMax'][0]) $errors['shadowMin'][] = array('ERROR', _('Password maxage'), _('Password maxage must bigger as Password Minage.'));
		if ( !ereg('^([0-9]*)$', $this->attributes['shadowMax'][0])) $errors['shadowMax'][] = array('ERROR', _('Password maxage'), _('Password maxage must be are natural number.'));
		if ( !ereg('^(([-][1])|([0-9]*))$', $this->attributes['shadowInactive'][0]))
			$errors['shadowInactive'][] = array('ERROR', _('Password Expire'), _('Password expire must be are natural number or -1.'));
		if ( !ereg('^([0-9]*)$', $this->attributes['shadowWarning'][0])) $errors['shadowWarning'][] = array('ERROR', _('Password warn'), _('Password warn must be are natural number.'));
		if (is_array($errors)) return $errors;
		return 0;
		}

	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_attributes($post, $profile=false) {
		// Use dd-mm-yyyy format of date because it's easier to read for humans
		$date = getdate ($this->attributes['shadowExpire'][0]*3600*24);

		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Password warn') ),
			1 => array ( 'kind' => 'input', 'name' => 'shadowWarning', 'type' => 'text', 'size' => '4', 'maxlength' => '4', 'value' => $this->attributes['shadowWarning'][0] ),
			2 => array ( 'kind' => 'help', 'value' => 'shadowWarning' ));
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Password expire') ),
			1 => array ( 'kind' => 'input', 'name' => 'shadowInactive', 'type' => 'text', 'size' => '4', 'maxlength' => '4', 'value' => $this->attributes['shadowInactive'][0] ),
			2 => array ( 'kind' => 'help', 'value' => 'shadowInactive' ));
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Maximum password age') ),
			1 => array ( 'kind' => 'input', 'name' => 'shadowMax', 'type' => 'text', 'size' => '5', 'maxlength' => '5', 'value' => $this->attributes['shadowMax'][0] ),
			2 => array ( 'kind' => 'help', 'value' => 'shadowMax' ));
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Minimum password age') ),
			1 => array ( 'kind' => 'input', 'name' => 'shadowMin', 'type' => 'text', 'size' => '5', 'maxlength' => '5', 'value' => $this->attributes['shadowMin'][0] ),
			2 => array ( 'kind' => 'help', 'value' => 'shadowMin' ));

		for ( $i=1; $i<=31; $i++ ) $mday[] = $i;
		for ( $i=1; $i<=12; $i++ ) $mon[] = $i;
		for ( $i=2003; $i<=2030; $i++ ) $year[] = $i;
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Expire day') ),
			1 => array ( 'kind' => 'table', 'value' => array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'shadowExpire_day',
					'options' => $mday, 'options_selectd' => $date['mday']),
				1 => array ( 'kind' => 'select', 'name' => 'shadowExpire_mon',
					'options' => $mon, 'options_selectd' => $date['mon']),
				2 => array ( 'kind' => 'select', 'name' => 'shadowExpire_yea',
					'options' => $year, 'options_selectd' => $date['year'])))),
			2 => array ( 'kind' => 'help', 'value' => 'shadowExpire' ));

		return $return;
		}

	function display_html_delete($post) {
		return 0;
		}

	function get_profileOptions() {
		$return = array();
		// password warning
		$return[] = array(0 => array('kind' => 'text', 'text' => _('Password warn')),
			1 => array('kind' => 'input', 'name' => 'shadowAccount_shadowWarning', 'type' => 'text', 'size' => '4', 'maxlength' => '4', 'value' => ""),
			2 => array('kind' => 'help', 'value' => 'TODO'));
		// password expiration
		$return[] = array(0 => array('kind' => 'text', 'text' => _('Password expire')),
			1 => array('kind' => 'input', 'name' => 'shadowAccount_shadowInactive', 'type' => 'text', 'size' => '4', 'maxlength' => '4', 'value' => ""),
			2 => array('kind' => 'help', 'value' => 'TODO'));
		// maximum password age
		$return[] = array(0 => array('kind' => 'text', 'text' => _('Maximum password age')),
			1 => array('kind' => 'input', 'name' => 'shadowAccount_shadowMax', 'type' => 'text', 'size' => '5', 'maxlength' => '5', 'value' => ""),
			2 => array('kind' => 'help', 'value' => 'TODO'));
		// minimum password age
		$return[] = array(0 => array('kind' => 'text', 'text' => _('Minimum password age')),
			1 => array('kind' => 'input', 'name' => 'shadowAccount_shadowMin', 'type' => 'text', 'size' => '5', 'maxlength' => '5', 'value' => ""),
			2 => array('kind' => 'help', 'value' => 'TODO'));
		// expiration date
		$day = array(); $mon = array(); $year = array();
		for ( $i=1; $i<=31; $i++ ) $day[] = $i;
		for ( $i=1; $i<=12; $i++ ) $mon[] = $i;
		for ( $i=2003; $i<=2030; $i++ ) $year[] = $i;
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Expire day')),
			1 => array('kind' => 'table', 'value' => array(
				0 => array (
					0 => array('kind' => 'select', 'name' => 'shadowAccount_shadowExpire_day',
						'options' => $day, 'options_selectd' => ""),
					1 => array('kind' => 'select', 'name' => 'shadowAccount_shadowExpire_mon',
						'options' => $mon, 'options_selectd' => ""),
					2 => array('kind' => 'select', 'name' => 'shadowAccount_shadowExpire_yea',
						'options' => $year, 'options_selectd' => "")
				)
			)),
			2 => array('kind' => 'help', 'value' => 'TODO'));
		return $return;
	}

	// checks if the values of a new or modified profile are valid
	// $scope: the account type (user, group, host, ...)
	// $options: a hash array (name => value) containing the options
	function check_profileOptions($scope, $options) {
		return array();
	}
	
	function get_pdfFields($account_type="user") {
		return array(	'shadowLastChange',
							'shadowWarning',
							'shadowInactive',
							'shadowExpire',
							'shadowFlag',
							'description');
	}
	
	function get_pdfEntries($account_type = "user") {
		return array(	'shadowAccount_shadowLastChange' => array('<block><key>' . _('Last shadow password change') . '</key><value>' . $this->attributes['shadowLastChange'][0] . '</value></block>'),
							'shadowAccount_shadowWarning' => array('<block><key>' . _('Password warn') . '</key><value>' . $this->attributes['shadowWarn'][0] . '</value><block>'),
							'shadowAccount_shadowInactive' => array('<block><key>' . _('Account inactive') . '</key><value>' . $this->attributes['shadowInactive'][0] . '</value></block>'),
							'shadowAccount_shadowExpire' => array('<block><key>' . _('Password expire') . '</key><value>' . date('d. m. Y',$this->attributes['shadowExpire'][0]) . '</value></block>'),
							'shadowAccount_shadowFlag' => array('<block><key>' . _('Shadow flag') . '</key><value>' . $this->attributes['shadowFlag'][0] . '</value></bock>'),
							'shadowAccount_description' => array('<block><key>' . _('Description') . '</key><value>' . $this->attributes['description'][0] . '</value></block>'));
	}

}

?>
