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

class shadowAccount extends baseModule {

	/**
	* Creates a new shadowAccount object.
	*
	* @param string $scope account type (user, group, host)
	*/
	function shadowAccount($scope) {
	// call parent constructor
	parent::baseModule($scope);
	}

	/** this functin fills the error message array with messages
	**/
	function load_Messages() {
		// error messages for input checks
		$this->messages['shadowMin'][0] = array('ERROR', _('Password minage'), _('Password minimum age must be are natural number.'));
		$this->messages['shadowMin'][1] = array('ERROR', _('Account %s:') . ' shadowAccount_minAge', _('Password minimum age must be are natural number.'));
		$this->messages['shadowMax'][0] = array('ERROR', _('Password maxage'), _('Password maximum age must be are natural number.'));
		$this->messages['shadowMax'][1] = array('ERROR', _('Account %s:') . ' shadowAccount_maxAge', _('Password maximum age must be are natural number.'));
		$this->messages['inactive'][0] = array('ERROR', _('Password expiration'), _('Password expiration must be are natural number or -1.'));
		$this->messages['inactive'][1] = array('ERROR', _('Account %s:') . ' shadowAccount_ignoreExpire', _('Password expiration must be are natural number or -1.'));
		$this->messages['shadowWarning'][0] = array('ERROR', _('Password warning'), _('Password warning must be are natural number.'));
		$this->messages['shadowWarning'][1] = array('ERROR', _('Account %s:') . ' shadowAccount_warning', _('Password warning must be are natural number.'));
		$this->messages['shadow_cmp'][0] = array('ERROR', _('Password maxage'), _('Password maximum age must be bigger as password minimum age.'));
		$this->messages['shadow_cmp'][1] = array('ERROR',  _('Account %s:') . ' shadowAccount_min/maxAge', _('Password maximum age must be bigger as password minimum age.'));
		$this->messages['shadow_expireDate'][0] = array('ERROR',  _('Account %s:') . ' shadowAccount_expireDate', _('The expiration date is invalid.'));
	}
	
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
		// module dependencies
		$return['dependencies'] = array('depends' => array('posixAccount'), 'conflicts' => array());
		// lists for expiration date
		$day = array(); $mon = array(); $year = array();
		for ( $i=1; $i<=31; $i++ ) $day[] = $i;
		for ( $i=1; $i<=12; $i++ ) $mon[] = $i;
		for ( $i=2003; $i<=2030; $i++ ) $year[] = $i;
		$return['profile_options'] = array(
			// password warning
			array(
				0 => array('kind' => 'text', 'text' => _('Password warning')),
				1 => array('kind' => 'input', 'name' => 'shadowAccount_shadowWarning', 'type' => 'text', 'size' => '4', 'maxlength' => '4', 'value' => ""),
				2 => array('kind' => 'help', 'value' => 'TODO')),
			// password expiration
			array(
				0 => array('kind' => 'text', 'text' => _('Password expiration')),
				1 => array('kind' => 'input', 'name' => 'shadowAccount_shadowInactive', 'type' => 'text', 'size' => '4', 'maxlength' => '4', 'value' => ""),
				2 => array('kind' => 'help', 'value' => 'TODO')),
			// minimum password age
			array(
				0 => array('kind' => 'text', 'text' => _('Minimum password age')),
				1 => array('kind' => 'input', 'name' => 'shadowAccount_shadowMin', 'type' => 'text', 'size' => '5', 'maxlength' => '5', 'value' => ""),
				2 => array('kind' => 'help', 'value' => 'TODO')),
			// maximum password age
			array(
				0 => array('kind' => 'text', 'text' => _('Maximum password age')),
				1 => array('kind' => 'input', 'name' => 'shadowAccount_shadowMax', 'type' => 'text', 'size' => '5', 'maxlength' => '5', 'value' => ""),
				2 => array('kind' => 'help', 'value' => 'TODO')),
			// expiration date
			array(
				0 => array('kind' => 'text', 'text' => _('Expiration date')),
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
				2 => array('kind' => 'help', 'value' => 'TODO'))
		);
		// profile checks
		$return['profile_checks']['shadowAccount_shadowMin'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => $this->messages['shadowMin'][0]);
		$return['profile_checks']['shadowAccount_shadowMax'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => $this->messages['shadowMax'][0]);
		$return['profile_checks']['shadowAccount_cmp'] = array(
			'type' => 'int_greater',
			'cmp_name1' => 'shadowAccount_shadowMax',
			'cmp_name2' => 'shadowAccount_shadowMin',
			'error_message' => $this->messages['shadow_cmp'][0]);
		$return['profile_checks']['shadowAccount_shadowInactive'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit2',
			'error_message' => $this->messages['inactive'][0]);
		$return['profile_checks']['shadowAccount_shadowWarning'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => $this->messages['shadowWarning'][0]);
		// profile mappings
		$return['profile_mappings'] = array(
			'shadowAccount_shadowWarning' => 'shadowWarning',
			'shadowAccount_shadowInactive' => 'shadowInactive',
			'shadowAccount_shadowMin' => 'shadowMin',
			'shadowAccount_shadowMax' => 'shadowMax'
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'shadowLastChange',
			'shadowWarning',
			'shadowInactive',
			'shadowExpire',
			'description'
		);
		// help Entries
		$return['help'] = array (
			'shadowWarning' => array (
				"Headline" => _("Password warning"),
				"Text" => _("Days before password is to expire that user is warned of pending password expiration. If set value must be 0<."). ' '. _("Can be left empty.")
			),
			'shadowInactive' => array (
				"Headline" => _("Password expiration"),
				"Text" => _("Number of days a user can login even his password has expired. -1=always."). ' '. _("Can be left empty.")
			),
			'shadowMin' => array (
				"Headline" => _("Minimum password age"),
				"Text" => _("Number of days a user has to wait until he\'s allowed to change his password again. If set value must be 0<."). ' '. _("Can be left empty.")
			),
			'shadowMax' => array (
				"Headline" => _("Maximum password age"),
				"Text" => _("Number of days after a user has to change his password again. If set value must be 0<."). ' '. _("Can be left empty.")
			),
			'shadowExpire' => array (
				"Headline" => _("Expiration date"),
				"Text" => _("Account expiration date. Format: DD-MM-YYYY")
			)
		);
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'shadowAccount_warning',
				'description' => _('Password warning'),
				'help' => 'shadowWarning',
				'example' => '14'
			),
			array(
				'name' => 'shadowAccount_expiration',
				'description' => _('Password expiration'),
				'help' => 'shadowInactive',
				'example' => '7'
			),
			array(
				'name' => 'shadowAccount_minAge',
				'description' => _('Minimum password age'),
				'help' => 'shadowMin',
				'example' => '1'
			),
			array(
				'name' => 'shadowAccount_maxAge',
				'description' => _('Maximum password age'),
				'help' => 'shadowMax',
				'example' => '365'
			),
			array(
				'name' => 'shadowAccount_expireDate',
				'description' => _('Expiration date'),
				'help' => 'shadowExpire',
				'example' => '17-07-2011'
			)
		);
		return $return;
	}

	// Constructor
	function init($base) {
		// call parent init
		parent::init($base);
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

	/* This function loads all attributes into the object
	* $attr is an array as it's retured from ldap_get_attributes
	*/
	function load_attributes($attr) {
		/* unset userPassword because:
		 * it is used by posixAccount
		 * it is a special attribute and stores encrypted in session
		 */
		unset($this->attributes['userPassword']);
		unset($this->orig['userPassword']);
		parent::load_attributes($attr);
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

		// Set shadowLastchange manual.
		if (isset($_SESSION[$this->base]->module['posixAccount']->orig['userPassword'][0])) {
			// TODO fixme ****** fix this behavoir 
			if ($_SESSION[$this->base]->module['posixAccount']->orig['userPassword'][0] != $_SESSION[$this->base]->module['posixAccount']->attributes['userPassword'][0] && $_SESSION[$this->base]->module['posixAccount']->attributes['userPassword'][0]!='')
				$return[$_SESSION[$this->base]->dn]['modify']['shadowLastChange'] = array(intval(time()/3600/24));
		}
		
		return $return;
		}

	function delete_attributes($post) {
		return 0;
		}

	/* Write variables into object and do some regexp checks
	*/
	function process_attributes(&$post) {
		// Load attributes
		$this->attributes['shadowMin'][0] = $post['shadowMin'];
		$this->attributes['shadowMax'][0] = $post['shadowMax'];
		$this->attributes['shadowWarning'][0] = $post['shadowWarning'];
		$this->attributes['shadowInactive'][0] = $post['shadowInactive'];
		$this->attributes['shadowExpire'][0] = intval(mktime(10, 0, 0, $post['shadowExpire_mon'],
			$post['shadowExpire_day'], $post['shadowExpire_yea'])/3600/24);

		if ( !get_preg($this->attributes['shadowMin'][0], 'digit'))  $triggered_messages['shadowMin'][] = $this->messages['shadowMin'][0];
		if ( !get_preg($this->attributes['shadowMax'][0], 'digit')) $triggered_messages['shadowMax'][] = $this->messages['shadowMax'][0];
		if ( $this->attributes['shadowMin'][0] > $this->attributes['shadowMax'][0]) $triggered_messages['shadowMin'][] = $this->messages['shadow_cmp'][0];
		if ( !get_preg($this->attributes['shadowInactive'][0], 'digit2')) $triggered_messages['shadowInactive'][] = $this->messages['inactive'][0];
		if ( !get_preg($this->attributes['shadowWarning'][0], 'digit')) $triggered_messages['shadowWarning'][] = $this->messages['shadowWarning'][0];
		if (count($triggered_messages)!=0) {
			$this->triggered_messages = $triggered_messages;
			return $triggered_messages;
		}
		else $this->triggered_messages = array();
		return 0;
		}

	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_attributes(&$post) {
		// Use dd-mm-yyyy format of date because it's easier to read for humans
		$date = getdate ($this->attributes['shadowExpire'][0]*3600*24);

		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Password warning') ),
			1 => array ( 'kind' => 'input', 'name' => 'shadowWarning', 'type' => 'text', 'size' => '4', 'maxlength' => '4', 'value' => $this->attributes['shadowWarning'][0] ),
			2 => array ( 'kind' => 'help', 'value' => 'shadowWarning' ));
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Password expiration') ),
			1 => array ( 'kind' => 'input', 'name' => 'shadowInactive', 'type' => 'text', 'size' => '4', 'maxlength' => '4', 'value' => $this->attributes['shadowInactive'][0] ),
			2 => array ( 'kind' => 'help', 'value' => 'shadowInactive' ));
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Minimum password age') ),
			1 => array ( 'kind' => 'input', 'name' => 'shadowMin', 'type' => 'text', 'size' => '5', 'maxlength' => '5', 'value' => $this->attributes['shadowMin'][0] ),
			2 => array ( 'kind' => 'help', 'value' => 'shadowMin' ));
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Maximum password age') ),
			1 => array ( 'kind' => 'input', 'name' => 'shadowMax', 'type' => 'text', 'size' => '5', 'maxlength' => '5', 'value' => $this->attributes['shadowMax'][0] ),
			2 => array ( 'kind' => 'help', 'value' => 'shadowMax' ));

		for ( $i=1; $i<=31; $i++ ) $mday[] = $i;
		for ( $i=1; $i<=12; $i++ ) $mon[] = $i;
		for ( $i=2003; $i<=2030; $i++ ) $year[] = $i;
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Expiration date') ),
			1 => array ( 'kind' => 'table', 'value' => array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'shadowExpire_day',
					'options' => $mday, 'options_selected' => $date['mday']),
				1 => array ( 'kind' => 'select', 'name' => 'shadowExpire_mon',
					'options' => $mon, 'options_selected' => $date['mon']),
				2 => array ( 'kind' => 'select', 'name' => 'shadowExpire_yea',
					'options' => $year, 'options_selected' => $date['year'])))),
			2 => array ( 'kind' => 'help', 'value' => 'shadowExpire' ));

		return $return;
		}

	function display_html_delete(&$post) {
		return 0;
		}

	/*
	 * (non-PHPDoc)
	 * @see baseModule#get_pdfEntries
	 */
	function get_pdfEntries($account_type = "user") {
		return array(	'shadowAccount_shadowLastChange' => array('<block><key>' . _('Last password change') . '</key><value>' . $this->attributes['shadowLastChange'][0] . '</value></block>'),
							'shadowAccount_shadowWarning' => array('<block><key>' . _('Password warning') . '</key><value>' . $this->attributes['shadowWarn'][0] . '</value><block>'),
							'shadowAccount_shadowInactive' => array('<block><key>' . _('Account inactive') . '</key><value>' . $this->attributes['shadowInactive'][0] . '</value></block>'),
							'shadowAccount_shadowExpire' => array('<block><key>' . _('Password expiration') . '</key><value>' . date('d. m. Y',$this->attributes['shadowExpire'][0]) . '</value></block>'),
							'shadowAccount_description' => array('<block><key>' . _('Description') . '</key><value>' . $this->attributes['description'][0] . '</value></block>'));
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
		$messages = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array("shadowAccount", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "shadowAccount";
			// password warning
			if ($rawAccounts[$i][$ids['shadowAccount_warning']] != '') {
				if (get_preg($rawAccounts[$i][$ids['shadowAccount_warning']], 'digit')) {
					$partialAccounts[$i]['shadowWarning'][] = $rawAccounts[$i][$ids['shadowAccount_warning']];
				}
				else {
					$errMsg = $this->messages['shadowWarning'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			// password expire ignoration
			if ($rawAccounts[$i][$ids['shadowAccount_ignoreExpire']] != '') {
				if (get_preg($rawAccounts[$i][$ids['shadowAccount_ignoreExpire']], 'digit2')) {
					$partialAccounts[$i]['shadowInactive'][] = $rawAccounts[$i][$ids['shadowAccount_ignoreExpire']];
				}
				else {
					$errMsg = $this->messages['inactive'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			// password minAge
			if ($rawAccounts[$i][$ids['shadowAccount_minAge']] != '') {
				if (get_preg($rawAccounts[$i][$ids['shadowAccount_minAge']], 'digit')) {
					$partialAccounts[$i]['shadowMin'][] = $rawAccounts[$i][$ids['shadowAccount_minAge']];
				}
				else {
					$errMsg = $this->messages['shadowMin'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			// password maxAge
			if ($rawAccounts[$i][$ids['shadowAccount_maxAge']] != '') {
				if (get_preg($rawAccounts[$i][$ids['shadowAccount_maxAge']], 'digit')) {
					$partialAccounts[$i]['shadowMax'][] = $rawAccounts[$i][$ids['shadowAccount_maxAge']];
				}
				else {
					$errMsg = $this->messages['shadowMax'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			// minAge <= maxAge
			if ((($rawAccounts[$i][$ids['shadowAccount_minAge']] != '') || ($rawAccounts[$i][$ids['shadowAccount_maxAge']] != '')) &&  // if at least one is set
			(($rawAccounts[$i][$ids['shadowAccount_minAge']] == '') || ($rawAccounts[$i][$ids['shadowAccount_maxAge']] == '') || (  // and one is not set
			($rawAccounts[$i][$ids['shadowAccount_minAge']] > $rawAccounts[$i][$ids['shadowAccount_maxAge']])))) {  // or minAge > maxAge
				$errMsg = $this->messages['shadow_cmp'][1];
				array_push($errMsg, array($i));
				$messages[] = $errMsg;
			}
			// expiration date
			if ($rawAccounts[$i][$ids['shadowAccount_expireDay']] != '') {
				if (get_preg($rawAccounts[$i][$ids['shadowAccount_expireDay']], 'date')) {
					$parts = explode('-', $rawAccounts[$i][$ids['shadowAccount_expireDay']]);
					$partialAccounts[$i]['shadowExpire'][] = intval(mktime(10, 0, 0, $parts[1], $parts[0], $parts[2])/3600/24);
				}
				else {
					$errMsg = $this->messages['shadow_expireDate'][0];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
		}
		return $messages;
	}

	/**
	* Loads the values of an account profile into internal variables.
	*
	* @param array $profile hash array with profile values (identifier => value)
	*/
	function load_profile($profile) {
		// profile mappings in meta data
		parent::load_profile($profile);
		// special profile options
		// expiration date
		if (isset($profile['shadowAccount_shadowExpire_day'][0]) && ($profile['shadowAccount_shadowExpire_day'][0] != "")) {
			$date = intval(mktime(10, 0, 0, $profile['shadowAccount_shadowExpire_mon'][0],
				$profile['shadowAccount_shadowExpire_day'][0], $profile['shadowAccount_shadowExpire_yea'][0])/3600/24);
			$this->attributes['shadowExpire'][0] = $date;
		}
	}
	
}

?>
