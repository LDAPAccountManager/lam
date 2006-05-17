<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003 - 2006  Tilo Lutz

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
* Manages Unix shadow accounts for users.
*
* @package modules
*
* @author Tilo Lutz
* @author Roland Gruber
* @author Michael Duergner
*/

/**
* Manages the object class "shadowAccount" for users.
*
* @package modules
*/
class shadowAccount extends baseModule {

	/**
	* This function builds up the message array.
	*/
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
		// managed object classes
		$return['objectClasses'] = array('shadowAccount');
		// managed attributes
		$return['attributes'] = array('userPassword', 'shadowLastChange', 'shadowMin', 'shadowMax', 'shadowWarning',
			'shadowInactive', 'shadowExpire');
		// lists for expiration date
		$day = array(); $mon = array(); $year = array();
		for ( $i=1; $i<=31; $i++ ) $day[] = $i;
		for ( $i=1; $i<=12; $i++ ) $mon[] = $i;
		for ( $i=2003; $i<=2030; $i++ ) $year[] = $i;
		$return['profile_options'] = array(
			// password warning
			array(
				0 => array('kind' => 'text', 'text' => _('Password warning')),
				1 => array('kind' => 'input', 'name' => 'shadowAccount_shadowWarning', 'type' => 'text', 'size' => '5', 'maxlength' => '4', 'value' => ""),
				2 => array('kind' => 'help', 'value' => 'shadowWarning')),
			// password expiration
			array(
				0 => array('kind' => 'text', 'text' => _('Password expiration')),
				1 => array('kind' => 'input', 'name' => 'shadowAccount_shadowInactive', 'type' => 'text', 'size' => '5', 'maxlength' => '4', 'value' => ""),
				2 => array('kind' => 'help', 'value' => 'shadowInactive')),
			// minimum password age
			array(
				0 => array('kind' => 'text', 'text' => _('Minimum password age')),
				1 => array('kind' => 'input', 'name' => 'shadowAccount_shadowMin', 'type' => 'text', 'size' => '5', 'maxlength' => '5', 'value' => ""),
				2 => array('kind' => 'help', 'value' => 'shadowMin')),
			// maximum password age
			array(
				0 => array('kind' => 'text', 'text' => _('Maximum password age')),
				1 => array('kind' => 'input', 'name' => 'shadowAccount_shadowMax', 'type' => 'text', 'size' => '5', 'maxlength' => '5', 'value' => ""),
				2 => array('kind' => 'help', 'value' => 'shadowMax')),
			// expiration date
			array(
				0 => array('kind' => 'text', 'text' => _('Account expiration date')),
				1 => array('kind' => 'table', 'value' => array(
					0 => array (
						0 => array('kind' => 'select', 'name' => 'shadowAccount_shadowExpire_day', 'options' => $day, 'options_selected' => array('1')),
						1 => array('kind' => 'select', 'name' => 'shadowAccount_shadowExpire_mon', 'options' => $mon, 'options_selected' => array('1')),
						2 => array('kind' => 'select', 'name' => 'shadowAccount_shadowExpire_yea', 'options' => $year, 'options_selected' => array('2030'))
					)
				)),
				2 => array('kind' => 'help', 'value' => 'shadowExpire'))
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
			'shadowExpire'
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
				"Headline" => _("Account expiration date"),
				"Text" => _("This is the date when the account will expire. Format: DD-MM-YYYY")
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
				'description' => _('Account expiration date'),
				'help' => 'shadowExpire',
				'example' => '17-07-2011'
			)
		);
		return $return;
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
		$return = $_SESSION[$this->base]->save_module_attributes($this->attributes, $this->orig);
		// Set shadowLastchange manual.
		if (isset($_SESSION[$this->base]->module['posixAccount']->orig['userPassword'][0])) {
			if ($_SESSION[$this->base]->module['posixAccount']->orig['userPassword'][0] != $_SESSION[$this->base]->module['posixAccount']->attributes['userPassword'][0])
				$return[$_SESSION[$this->base]->dn]['modify']['shadowLastChange'] = array(intval(time()/3600/24));
		}
		elseif ($_SESSION[$this->base]->isNewAccount) {
			$return[$_SESSION[$this->base]->dn]['add']['shadowLastChange'] = array(intval(time()/3600/24));
		}
		// do not set password if posixAccount is active
		$modules = $_SESSION['config']->get_AccountModules($this->get_scope());
		if (in_array('posixAccount', $modules)) {
			if (isset($return[$_SESSION[$this->base]->dn]['modify']['userPassword'])) {
				unset($return[$_SESSION[$this->base]->dn]['modify']['userPassword']); 	 
			}
			if (isset($return[$_SESSION[$this->base]->dn]['add']['userPassword'])) {
				unset($return[$_SESSION[$this->base]->dn]['add']['userPassword']);
			}
		}
		return $return;
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @param array $post HTTP-POST values
	* @return array list of info/error messages
	*/
	function process_attributes(&$post) {
		$errors = array();
		// Load attributes
		$this->attributes['shadowMin'][0] = $post['shadowMin'];
		$this->attributes['shadowMax'][0] = $post['shadowMax'];
		$this->attributes['shadowWarning'][0] = $post['shadowWarning'];
		$this->attributes['shadowInactive'][0] = $post['shadowInactive'];
		$this->attributes['shadowExpire'][0] = intval(mktime(10, 0, 0, intval($post['shadowExpire_mon']), intval($post['shadowExpire_day']),
			intval($post['shadowExpire_yea']))/3600/24);
		if ( !get_preg($this->attributes['shadowMin'][0], 'digit'))  $errors['shadowMin'][] = $this->messages['shadowMin'][0];
		if ( !get_preg($this->attributes['shadowMax'][0], 'digit')) $errors['shadowMax'][] = $this->messages['shadowMax'][0];
		if ( $this->attributes['shadowMin'][0] > $this->attributes['shadowMax'][0]) $errors['shadowMin'][] = $this->messages['shadow_cmp'][0];
		if ( !get_preg($this->attributes['shadowInactive'][0], 'digit2')) $errors['shadowInactive'][] = $this->messages['inactive'][0];
		if ( !get_preg($this->attributes['shadowWarning'][0], 'digit')) $errors['shadowWarning'][] = $this->messages['shadowWarning'][0];
		return $errors;
	}

	/**
	* This function will create the meta HTML code to show a page with all attributes.
	*
	* @param array $post HTTP POST data
	* @return array meta HTML code
	*/
	function display_html_attributes(&$post) {
		// Use dd-mm-yyyy format of date because it's easier to read for humans
		$shAccExpirationDate = 0;
		if (isset($this->attributes['shadowExpire'][0])) $shAccExpirationDate = $this->attributes['shadowExpire'][0];
		$date = getdate($shAccExpirationDate*3600*24);

		$shWarning = '';
		if (isset($this->attributes['shadowWarning'][0])) $shWarning = $this->attributes['shadowWarning'][0];
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Password warning') ),
			1 => array ( 'kind' => 'input', 'name' => 'shadowWarning', 'type' => 'text', 'size' => '5', 'maxlength' => '4', 'value' => $shWarning),
			2 => array ( 'kind' => 'help', 'value' => 'shadowWarning' ));
		$shPwdExpiration = '';
		if (isset($this->attributes['shadowInactive'][0])) $shPwdExpiration = $this->attributes['shadowInactive'][0];
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Password expiration') ),
			1 => array ( 'kind' => 'input', 'name' => 'shadowInactive', 'type' => 'text', 'size' => '5', 'maxlength' => '4', 'value' => $shPwdExpiration),
			2 => array ( 'kind' => 'help', 'value' => 'shadowInactive' ));
		$shMinAge = '';
		if (isset($this->attributes['shadowMin'][0])) $shMinAge = $this->attributes['shadowMin'][0];
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Minimum password age') ),
			1 => array ( 'kind' => 'input', 'name' => 'shadowMin', 'type' => 'text', 'size' => '5', 'maxlength' => '5', 'value' => $shMinAge),
			2 => array ( 'kind' => 'help', 'value' => 'shadowMin' ));
		$shMaxAge = '';
		if (isset($this->attributes['shadowMax'][0])) $shMaxAge = $this->attributes['shadowMax'][0];
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Maximum password age') ),
			1 => array ( 'kind' => 'input', 'name' => 'shadowMax', 'type' => 'text', 'size' => '5', 'maxlength' => '5', 'value' => $shMaxAge),
			2 => array ( 'kind' => 'help', 'value' => 'shadowMax' ));

		for ( $i=1; $i<=31; $i++ ) $mday[] = $i;
		for ( $i=1; $i<=12; $i++ ) $mon[] = $i;
		for ( $i=2003; $i<=2030; $i++ ) $year[] = $i;
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Account expiration date') ),
			1 => array ( 'kind' => 'table', 'value' => array ( 0 => array ( 0 => array ( 'kind' => 'select', 'name' => 'shadowExpire_day',
					'options' => $mday, 'options_selected' => $date['mday']),
				1 => array ( 'kind' => 'select', 'name' => 'shadowExpire_mon',
					'options' => $mon, 'options_selected' => $date['mon']),
				2 => array ( 'kind' => 'select', 'name' => 'shadowExpire_yea',
					'options' => $year, 'options_selected' => $date['year'])))),
			2 => array ( 'kind' => 'help', 'value' => 'shadowExpire' ));

		return $return;
	}

	/**
	* Returns the PDF entries for this module.
	* 
	* @return array list of possible PDF entries
	*/
	function get_pdfEntries() {
		return array('shadowAccount_shadowLastChange' => array('<block><key>' . _('Last password change') . '</key><value>' . $this->attributes['shadowLastChange'][0] . '</value></block>'),
			'shadowAccount_shadowWarning' => array('<block><key>' . _('Password warning') . '</key><value>' . $this->attributes['shadowWarn'][0] . '</value><block>'),
			'shadowAccount_shadowInactive' => array('<block><key>' . _('Account inactive') . '</key><value>' . $this->attributes['shadowInactive'][0] . '</value></block>'),
			'shadowAccount_shadowExpire' => array('<block><key>' . _('Password expiration') . '</key><value>' . date('d. m. Y',$this->attributes['shadowExpire'][0]) . '</value></block>'));
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
			// shadow last change
			$partialAccounts[$i]['shadowLastChange'] = array(intval(time()/3600/24));
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
					$partialAccounts[$i]['shadowExpire'][] = intval(mktime(10, 0, 0, intval($parts[1]), intval($parts[0]), intval($parts[2]))/3600/24);
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
			$date = intval(mktime(10, 0, 0, intval($profile['shadowAccount_shadowExpire_mon'][0]),
				intval($profile['shadowAccount_shadowExpire_day'][0]), intval($profile['shadowAccount_shadowExpire_yea'][0]))/3600/24);
			$this->attributes['shadowExpire'][0] = $date;
		}
	}
	
}

?>
