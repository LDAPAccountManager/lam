<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2004 - 2006  Roland Gruber

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
* Manages Kolab user accounts.
*
* @package modules
* @author Roland Gruber
*/

/**
* Manages Kolab user accounts.
*
* @package modules
*/
class kolabUser extends baseModule {

	/** list of invitation policies */
	private $invitationPolicies;

	/**
	* Creates a new kolabUser object.
	*
	* @param string $scope account type (user, group, host)
	*/
	function kolabUser($scope) {
		// list of invitation policies
		$this->invitationPolicies = array(
			'ACT_ALWAYS_ACCEPT' => _('Always accept'),
			'ACT_ALWAYS_REJECT' => _('Always reject'),
			'ACT_MANUAL' => _('Manual'),
			'ACT_REJECT_IF_CONFLICTS' => _('Reject if conflicts'),
			'ACT_MANUAL_IF_CONFLICTS' => _('Manual if conflicts')
		);
		// call parent constructor
		parent::baseModule($scope);
	}

	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	*/
	function get_metaData() {
		$return = array();
		// icon
		$return['icon'] = 'kolab.png';
		// manages host accounts
		$return["account_types"] = array("user");
		// alias name
		$return["alias"] = _("Kolab");
		// module dependencies
		$return['dependencies'] = array('depends' => array('inetOrgPerson'), 'conflicts' => array());
		// LDAP filter
		$return["ldap_filter"] = array('or' => "(objectClass=kolabInetOrgPerson)");
		// managed object classes
		$return['objectClasses'] = array('kolabInetOrgPerson');
		// managed attributes
		$return['attributes'] = array('c', 'alias', 'kolabHomeServer', 'kolabHomeMTA', 'kolabDelegate',
			'cyrus-userquota', 'kolabInvitationPolicy', 'kolabFreeBusyFuture', 'kolabDeleteflag');
		// profile options
		$return['profile_options'] = array(
			array(
				0 => array('kind' => 'text', 'text' => _('Mail server') . ":"),
				1 => array('kind' => 'input', 'name' => 'kolab_homeServer', 'type' => 'text', 'size' => '30', 'maxlength' => '255'),
				2 => array('kind' => 'help', 'value' => 'homeServer')),
			array(
				0 => array('kind' => 'text', 'text' => _('Country') . ":"),
				1 => array('kind' => 'input', 'name' => 'kolab_country', 'type' => 'text', 'size' => '30', 'maxlength' => '255'),
				2 => array('kind' => 'help', 'value' => 'country')),
			array(
				0 => array('kind' => 'text', 'text' => _('Free/Busy interval') . ":"),
				1 => array('kind' => 'input', 'name' => 'kolab_freeBusy', 'type' => 'text', 'size' => '30', 'maxlength' => '255'),
				2 => array('kind' => 'help', 'value' => 'freeBusy')),
			array(
				0 => array('kind' => 'text', 'text' => _('Mail quota') . ":"),
				1 => array('kind' => 'input', 'name' => 'kolab_quota', 'type' => 'text', 'size' => '30', 'maxlength' => '255'),
				2 => array('kind' => 'help', 'value' => 'quota'))
		);
		// profile checks
		$return['profile_checks']['kolab_homeServer'] = array(
			'type' => 'ext_preg',
			'regex' => 'DNSname',
			'error_message' => $this->messages['homeServer'][0]);
		$return['profile_checks']['kolab_country'] = array(
			'type' => 'ext_preg',
			'regex' => 'country',
			'error_message' => $this->messages['country'][0]);
		$return['profile_checks']['kolab_freeBusy'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => $this->messages['freeBusy'][0]);
		$return['profile_checks']['kolab_quota'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => $this->messages['quota'][0]);
		// profile mappings
		$return['profile_mappings'] = array(
			'kolab_homeServer' => 'kolabHomeServer',
			'kolab_country' => 'c',
			'kolab_freeBusy' => 'kolabFreeBusyFuture',
			'kolab_quota' => 'cyrus-userquota'
		);
		// help Entries
		$return['help'] = array(
			'invPol' => array(
				"Headline" => _("Invitation policy"),
				"Text" => _("For automatic invitation handling.")
			),
			'invPolList' => array(
				"Headline" => _("Invitation policy list"),
				"Text" => _("This is a comma separated list of invitation policies.")
			),
			'delegate' => array(
				"Headline" => _("Delegates"),
				"Text" => _("A user may define who is allowed to act on behalf of herself. This property is checked when using the Kolab smtp daemon (Postfix) to send emails.")
			),
			'delegateList' => array(
				"Headline" => _("Delegates"),
				"Text" => _("This is a comma separated list of delegates.")
			),
			'alias' => array(
				"Headline" => _("EMail alias"),
				"Text" => _("EMail alias for this account.")
			),
			'aliasList' => array(
				"Headline" => _("EMail alias list"),
				"Text" => _("This is a comma separated list of eMail aliases.")
			),
			'country' => array(
				"Headline" => _("Country"),
				"Text" => _("The country name of the user.")
			),
			'homeServer' => array(
				"Headline" => _("Mailbox home server"),
				"Text" => _("The name of the server where the mailbox is located.")
			),
			'freeBusy' => array(
				"Headline" => _("Free/Busy interval"),
				"Text" => _("This is the time limit (in days) for other users who want to check future appointments. If you leave this blank the default (60 days) will be set.")
			),
			'quota' => array(
				"Headline" => _("Mail quota"),
				"Text" => _("The Cyrus mail quota for users in MBytes, leave blank for unlimited space.")
			),
			'deleteFlag' => array(
				"Headline" => _("Mark for deletion"),
				"Text" => _("This will set a special flag on the account which tells Kolabd to remove it. Use this to cleanly delete Kolab accounts (e.g. this removes mail boxes).")
			));
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'kolabUser_invPol',
				'description' => _('Invitation policy'),
				'help' => 'invPolList',
				'example' => 'user@domain:ACT_ALWAYS_ACCEPT,user2@domain:ACT_MANUAL'
			),
			array(
				'name' => 'kolabUser_country',
				'description' => _('Country'),
				'help' => 'country',
				'example' => _('Germany')
			),
			array(
				'name' => 'kolabUser_homeServer',
				'description' => _('Mailbox home server'),
				'help' => 'homeServer',
				'example' => 'localhost',
				'required' => true
			),
			array(
				'name' => 'kolabUser_aliases',
				'description' => _('EMail aliases'),
				'help' => 'aliasList',
				'example' => 'user@domain,user2@domain'
			),
			array(
				'name' => 'kolabUser_delegates',
				'description' => _('Delegates'),
				'help' => 'delegateList',
				'example' => 'user@domain,user2@domain'
			),
			array(
				'name' => 'kolabUser_freeBusy',
				'description' => _('Free/Busy interval'),
				'help' => 'freeBusy',
				'example' => '60'
			),
			array(
				'name' => 'kolabUser_quota',
				'description' => _('Mail quota'),
				'help' => 'quota',
				'example' => '300'
			)
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'invPol', 'country', 'homeServer', 'aliases', 'delegate', 'freeBusy', 'quota'
		);
		return $return;
	}

	/**
	* This function fills the error message array with messages
	*/
	function load_Messages() {
		$this->messages['invPol'][0] = array('ERROR', _('Target of invitation policy is invalid!'));  // third array value is set dynamically
		$this->messages['invPol'][1] = array('ERROR', _('Account %s:') . ' kolabUser_invPol', _('Policy list has invalid format!'));
		$this->messages['alias'][0] = array('ERROR', _('EMail alias is invalid!'));  // third array value is set dynamically
		$this->messages['alias'][1] = array('ERROR', _('Account %s:') . ' kolabUser_aliases', _('EMail alias list has invalid format!'));
		$this->messages['country'][0] = array('ERROR', _('Country name is invalid!'));  // third array value is set dynamically
		$this->messages['country'][1] = array('ERROR', _('Account %s:') . ' kolabUser_country', _('Country name is invalid!'));
		$this->messages['homeServer'][0] = array('ERROR', _('Mailbox home server name is invalid!'));  // third array value is set dynamically
		$this->messages['homeServer'][1] = array('ERROR', _('Account %s:') . ' kolabUser_homeServer', _('Mailbox home server name is invalid!'));
		$this->messages['homeServer'][2] = array('ERROR', _('Mailbox home server name is empty!'), '');
		$this->messages['delegate'][0] = array('ERROR', _('Account %s:') . ' kolabUser_delegate', _('Delegates has invalid format!'));
		$this->messages['freeBusy'][0] = array('ERROR', _('Free/Busy interval must be a number!'));  // third array value is set dynamically
		$this->messages['freeBusy'][1] = array('ERROR', _('Account %s:') . ' kolabUser_freeBusy', _('Free/Busy interval must be a number!'));
		$this->messages['quota'][0] = array('ERROR', _('Mail quota must be a number!'));  // third array value is set dynamically
		$this->messages['quota'][1] = array('ERROR', _('Account %s:') . ' kolabUser_quota', _('Mail quota must be a number!'));
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
		// add object class if needed
		if (!isset($this->attributes['objectClass']) || !in_array('kolabInetOrgPerson', $this->attributes['objectClass'])) {
			$this->attributes['objectClass'][] = 'kolabInetOrgPerson';
		}
		return parent::save_attributes();
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return array HTML meta data
	 */
	function display_html_attributes() {
		$attrsI = $this->getAccountContainer()->getAccountModule('inetOrgPerson')->getAttributes();
		if ($this->getAccountContainer()->isNewAccount) {
			if ($this->getAccountContainer()->getAccountModule('posixAccount') != null) {
				$attrsP = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
				if (!$attrsP['userPassword'][0]) {
					StatusMessage('ERROR', _("Please enter a user password on this page: %s"), '', array($this->getAccountContainer()->getAccountModule('posixAccount')->get_alias()));
				}
			}
			elseif ($this->getAccountContainer()->getAccountModule('inetOrgPerson') != null) {
				if (!$attrsI['userPassword'][0]) {
					StatusMessage('ERROR', _("Please enter a user password on this page: %s"), '', array($this->getAccountContainer()->getAccountModule('inetOrgPerson')->get_alias()));
				}
			}
		}
		if (!$attrsI['mail'][0]) {
			StatusMessage('ERROR', _("Please enter an email address on this page: %s"), '', array($this->getAccountContainer()->getAccountModule('inetOrgPerson')->get_alias()));
		}
		$return = array();
		// check if account is marked for deletion
		if (isset($this->attributes['kolabDeleteflag'])) {
			$return[] = array(
			0 => array('kind' => 'text', 'text' => _('This account is marked for deletion.')));
			return $return;
		}
		// country
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Country')),
			1 => array('kind' => 'input', 'name' => 'country', 'type' => 'text', 'value' => $this->attributes['c'][0]),
			2 => array('kind' => 'help', 'value' => 'country'));
		// mailbox server
		if (!isset($this->orig['kolabHomeServer'][0])) {  // value currently not set
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Mailbox home server') . "*"),
				1 => array('kind' => 'input', 'name' => 'homeServer', 'type' => 'text', 'value' => $this->attributes['kolabHomeServer'][0]),
				2 => array('kind' => 'help', 'value' => 'homeServer'));
		}
		else {  // input is unchangable when set
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Mailbox home server')),
				1 => array('kind' => 'text', 'text' => $this->orig['kolabHomeServer'][0]),
				2 => array('kind' => 'help', 'value' => 'homeServer'));
		}
		// Cyrus mail quota
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Mail quota')),
			1 => array('kind' => 'input', 'name' => 'quota', 'type' => 'text', 'value' => $this->attributes['cyrus-userquota'][0]),
			2 => array('kind' => 'help', 'value' => 'quota'));
		// free/busy future
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Free/Busy interval')),
			1 => array('kind' => 'input', 'name' => 'freeBusy', 'type' => 'text', 'value' => $this->attributes['kolabFreeBusyFuture'][0]),
			2 => array('kind' => 'help', 'value' => 'freeBusy'));
		// invitation policies
		$returnPol = array();
		// default invitation policy
		$defaultInvPol = $this->invitationPolicies['ACT_MANUAL'];
		for ($i = 0; $i < sizeof($this->attributes['kolabInvitationPolicy']); $i++) {
			$parts = split(":", $this->attributes['kolabInvitationPolicy'][$i]);
			if (sizeof($parts) == 1) {
				$defaultInvPol = $this->invitationPolicies[$this->attributes['kolabInvitationPolicy'][$i]];
				unset($this->attributes['kolabInvitationPolicy'][$i]);
				$this->attributes['kolabInvitationPolicy'] = array_values($this->attributes['kolabInvitationPolicy']);
				break;
			}
		}
		$returnPol[] = array(
			0 => array('kind' => 'text', 'text' => _('Anyone')),
			1 => array('kind' => 'select', 'name' => 'defaultInvPol', 'options' => array_values($this->invitationPolicies), 'options_selected' => $defaultInvPol),
			2 => array('kind' => 'help', 'value' => 'invPol'));
		// other invitation policies
		for ($i = 0; $i < sizeof($this->attributes['kolabInvitationPolicy']); $i++) {
			$parts = split(":", $this->attributes['kolabInvitationPolicy'][$i]);
			if (sizeof($parts) == 2) {
				$returnPol[] = array(
					0 => array('kind' => 'input', 'name' => 'invPol1' . $i, 'type' => 'text', 'value' => $parts[0]),
					1 => array('kind' => 'select', 'name' => 'invPol2' . $i, 'options' => array_values($this->invitationPolicies), 'options_selected' => $this->invitationPolicies[$parts[1]]),
					2 => array('kind' => 'input', 'type' => 'submit', 'name' => 'delInvPol' . $i, 'value' => _("Remove")),
					3 => array('kind' => 'help', 'value' => 'invPol'));
			}
		}
		// input box for new invitation policy
		$returnPol[] = array(
			0 => array('kind' => 'input', 'name' => 'invPol1', 'type' => 'text', 'value' => ''),
			1 => array('kind' => 'select', 'name' => 'invPol2', 'options' => array_values($this->invitationPolicies)),
			2 => array('kind' => 'input', 'type' => 'submit', 'name' => 'addInvPol', 'value' => _("Add")),
			3 => array('kind' => 'help', 'value' => 'invPol'));
		$returnPol = array(
				array('kind' => 'fieldset', 'legend' => _('Invitation policy'), 'value' => $returnPol, 'td' => array('colspan' => 3))
			);
		$return[] = $returnPol;
		// mail aliases
		$returnAliases = array();
		for ($i = 0; $i < sizeof($this->attributes['alias']); $i++) {
			$returnAliases[] = array(
				0 => array('kind' => 'input', 'name' => 'alias' . $i, 'type' => 'text', 'value' => $this->attributes['alias'][$i]),
				1 => array('kind' => 'input', 'type' => 'submit', 'name' => 'delAlias' . $i, 'value' => _("Remove")),
				2 => array('kind' => 'help', 'value' => 'alias'));
		}
		// input box for new mail alias
		$returnAliases[] = array(
			0 => array('kind' => 'input', 'name' => 'alias', 'type' => 'text', 'value' => ''),
			1 => array('kind' => 'input', 'type' => 'submit', 'name' => 'addAlias', 'value' => _("Add")),
			2 => array('kind' => 'help', 'value' => 'alias'));
		$returnAliases = array(
				array('kind' => 'fieldset', 'legend' => _('EMail aliases'), 'value' => $returnAliases, 'td' => array('colspan' => 3))
			);
		$return[] = $returnAliases;
		// delegates
		$delegates = array();
		$delegatesTemp = $_SESSION['cache']->get_cache('mail', 'inetOrgPerson', 'user');
		if (is_array($delegatesTemp)) {
			$DNs = array_keys($delegatesTemp);
			foreach ($DNs as $DN) {
				$delegates[] = $delegatesTemp[$DN][0];
			}
			sort($delegates);
		}
		$returnDelegates = array();
		for ($i = 0; $i < sizeof($this->attributes['kolabDelegate']); $i++) {
			$returnDelegates[] = array(
				0 => array('kind' => 'select', 'name' => 'delegate' . $i, 'options' => $delegates, 'options_selected' => array($this->attributes['kolabDelegate'][$i])),
				1 => array('kind' => 'input', 'type' => 'submit', 'name' => 'delDelegate' . $i, 'value' => _("Remove")),
				2 => array('kind' => 'help', 'value' => 'delegate'));
		}
		// input box for new delegate
		$returnDelegates[] = array(
			0 => array('kind' => 'select', 'name' => 'delegate', 'options' => $delegates),
			1 => array('kind' => 'input', 'type' => 'submit', 'name' => 'addDelegate', 'value' => _("Add")),
			2 => array('kind' => 'help', 'value' => 'delegate'));
		$returnDelegates = array(
				array('kind' => 'fieldset', 'legend' => _('Delegates'), 'value' => $returnDelegates, 'td' => array('colspan' => 3))
			);
		$return[] = $returnDelegates;
		// delete flag
		if (!$this->getAccountContainer()->isNewAccount) {
			$returnDelete = array(
				0 => array('kind' => 'input', 'type' => 'submit', 'value' => _('Mark account for deletion'), 'name' => 'form_subpage_kolabUser_deleteUser_open'),
				1 => array('kind' => 'help', 'value' => 'deleteFlag'));
			$return[] = array(
				array('kind' => 'fieldset', 'legend' => _('Delete Kolab account'), 'value' => array($returnDelete), 'td' => array('colspan' => 3))
			);
		}
		return $return;
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_attributes() {
		$errors = array();
		if ($_POST['form_subpage_kolabUser_deleteUser_open'])  return array();
		$this->attributes['kolabInvitationPolicy'] = array();
		// country
		if (isset($_POST['country'])) {
			if (($_POST['country'] == "") || get_preg($_POST['country'], 'country')) {
				$this->attributes['c'][0] = $_POST['country'];
			}
			else {
				$message = $this->messages['country'][0];
				$message[] = $_POST['country'];
				$errors[] = $message;
			}
		}
		// mailbox server
		if (isset($_POST['homeServer'])) {
			if ($_POST['homeServer'] == "") {
				$errors[] = $this->messages['homeServer'][2];
			}
			elseif (get_preg($_POST['homeServer'], 'DNSname')) {
				$this->attributes['kolabHomeServer'][0] = $_POST['homeServer'];
			}
			else {
				$message = $this->messages['homeServer'][0];
				$message[] = $_POST['homeServer'];
				$errors[] = $message;
			}
		}
		// check old invitation policies
		$policies = array_flip($this->invitationPolicies);
		$targets = array();
		$i = 0;
		while (isset($_POST['invPol1' . $i])) {
			if (isset($_POST['delInvPol' . $i])) {
				$i++;
				continue;
			}
			if (isset($_POST['invPol2' . $i]) && ($_POST['invPol1' . $i] != "") && !in_array($_POST['invPol1' . $i], $targets)) {
				$targets[] = $_POST['invPol1' . $i];
				// check invitation policy
				if (!get_preg($_POST['invPol1' . $i], 'email')) {
					$message = $this->messages['invPol'][0];
					$message[] = $_POST['invPol1' . $i];
					$errors[] = $message;
				}
				else {
					$this->attributes['kolabInvitationPolicy'][] = $_POST['invPol1' . $i] . ':' . $policies[$_POST['invPol2' . $i]];
				}
			}
			$i++;
		}
		// check new invitation policy
		if (isset($_POST['invPol1']) && ($_POST['invPol1'] != "") && !in_array($_POST['invPol1'], $targets)) {
			// check new invitation policy
			if (!get_preg($_POST['invPol1'], 'email')) {
				$message = $this->messages['invPol'][0];
				$message[] = $_POST['invPol1'];
				$errors[] = $message;
			}
			else {
				$this->attributes['kolabInvitationPolicy'][] = $_POST['invPol1'] . ':' . $policies[$_POST['invPol2']];
			}
		}
		// default invitation policy
		if (isset($_POST['defaultInvPol']) && ($_POST['defaultInvPol'] != "")) {
			$this->attributes['kolabInvitationPolicy'][] = $policies[$_POST['defaultInvPol']];
		}
		// check old mail aliases
		$this->attributes['alias'] = array();
		$i = 0;
		while (isset($_POST['alias' . $i])) {
			if (isset($_POST['delAlias' . $i])) {
				$i++;
				continue;
			}
			if (isset($_POST['alias' . $i]) && ($_POST['alias' . $i] != "") && !in_array($_POST['alias' . $i], $this->attributes['alias'])) {
				// check mail alias
				if (!get_preg($_POST['alias' . $i], 'email')) {
					$message = $this->messages['alias'][0];
					$message[] = $_POST['alias' . $i];
					$errors[] = $message;
				}
				$this->attributes['alias'][] = $_POST['alias' . $i];
			}
			$i++;
		}
		// check new mail alias
		if (isset($_POST['alias']) && ($_POST['alias'] != "")) {
			// check new mail alias
			if (!get_preg($_POST['alias'], 'email')) {
				$message = $this->messages['alias'][0];
				$message[] = $_POST['alias'];
				$errors[] = $message;
			}
			else {
				$this->attributes['alias'][] = $_POST['alias'];
			}
		}
		$this->attributes['alias'] = array_unique($this->attributes['alias']);
		// check old delegates
		$this->attributes['kolabDelegate'] = array();
		$i = 0;
		while (isset($_POST['delegate' . $i])) {
			if (isset($_POST['delDelegate' . $i])) {
				$i++;
				continue;
			}
			$this->attributes['kolabDelegate'][] = $_POST['delegate' . $i];
			$i++;
		}
		// check new delegate
		if (isset($_POST['addDelegate']) && ($_POST['delegate'] != "")) {
			$this->attributes['kolabDelegate'][] = $_POST['delegate'];
		}
		$this->attributes['kolabDelegate'] = array_unique($this->attributes['kolabDelegate']);
		// free/busy future
		if (isset($_POST['freeBusy'])) {
			if (($_POST['freeBusy'] == "") || get_preg($_POST['freeBusy'], 'digit')) {
				$this->attributes['kolabFreeBusyFuture'][0] = $_POST['freeBusy'];
			}
			else {
				$message = $this->messages['freeBusy'][0];
				$message[] = $_POST['freeBusy'];
				$errors[] = $message;
			}
		}
		// Cyrus mail quota
		if (isset($_POST['quota'])) {
			if (($_POST['quota'] == "") || get_preg($_POST['quota'], 'digit')) {
				$this->attributes['cyrus-userquota'][0] = $_POST['quota'];
			}
			else {
				$message = $this->messages['quota'][0];
				$message[] = $_POST['quota'];
				$errors[] = $message;
			}
		}
		return $errors;
	}

	/**
	* This function will create the meta HTML code to show a page to delete accounts.
	*/
	function display_html_deleteUser() {
		$return = array();
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Do you really want to mark this account for deletion?'), 'td' => array('colspan' => 2)));
		$return[] = array(
			0 => array('kind' => 'text', 'text' => ' ', 'td' => array('colspan' => 2)));
		$return[] = array(
			0 => array('kind' => 'input', 'type' => 'submit', 'value' => _('Mark account for deletion'), 'name' => 'form_subpage_kolabUser_attributes_confirm'),
			1 => array('kind' => 'input', 'type' => 'submit', 'value' => _('Cancel'), 'name' => 'form_subpage_kolabUser_attributes_cancel'));
		return $return;
	}

	/**
	* Write variables into object and do some regex checks
	*/
	function process_deleteUser() {
		if ($_POST['form_subpage_kolabUser_attributes_confirm']) {
			// set delete flag
			$this->attributes['kolabDeleteflag'][0] = $this->attributes['kolabHomeServer'][0];
		}
	}

	/**
	* This function returns true if all needed settings are done.
	*
	* @return true, if account can be saved
	*/
	function module_complete() {
		$attrsI = $this->getAccountContainer()->getAccountModule('inetOrgPerson')->getAttributes();
		$attrsP = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
		if (!$attrsI['mail'][0]) return false;
		if ($this->getAccountContainer()->isNewAccount) {
			if ($this->getAccountContainer()->getAccountModule('posixAccount') != null) {
				if (!$attrsP['userPassword'][0]) return false;
			}
			elseif ($this->getAccountContainer()->getAccountModule('inetOrgPerson') != null) {
				if (!$attrsI['userPassword'][0]) return false;
			}
		}
		return true;
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
			if (!in_array("kolabInetOrgPerson", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "kolabInetOrgPerson";
			// country
			if ($rawAccounts[$i][$ids['kolabUser_country']] != "") {
				if (get_preg($rawAccounts[$i][$ids['kolabUser_country']], 'country')) {
					$partialAccounts[$i]['c'] = $rawAccounts[$i][$ids['kolabUser_country']];
				}
				else {
					$errMsg = $this->messages['country'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			// mailbox server
			if ($rawAccounts[$i][$ids['kolabUser_homeServer']] != "") {
				if (get_preg($rawAccounts[$i][$ids['kolabUser_homeServer']], 'DNSname')) {
					$partialAccounts[$i]['kolabHomeServer'] = $rawAccounts[$i][$ids['kolabUser_homeServer']];
				}
				else {
					$errMsg = $this->messages['homeServer'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			// add invitation policies
			if ($rawAccounts[$i][$ids['kolabUser_invPol']] != "") {
				$pols = explode(',', $rawAccounts[$i][$ids['kolabUser_invPol']]);
				// check format
				$policies = array_keys($this->invitationPolicies);
				$defaultFound = false;
				for ($p = 0; $p < sizeof($pols); $p++) {
					$parts = split(":", $pols[$p]);
					// default policy
					if (sizeof($parts) == 1) {
						if (!$defaultFound && get_preg($parts[0], 'email')) {
							$partialAccounts[$i]['kolabInvitationPolicy'][] = $parts[0];
						}
						else {
							$errMsg = $this->messages['invPol'][1];
							array_push($errMsg, array($i));
							$messages[] = $errMsg;
						}
						$defaultFound = true;
					}
					// additional policies
					elseif (sizeof($parts) == 2) {
						if (get_preg($parts[0], 'email') && in_array($parts[1], $policies)) {
							$partialAccounts[$i]['kolabInvitationPolicy'][] = $pols[$p];
						}
						else {
							$errMsg = $this->messages['invPol'][1];
							array_push($errMsg, array($i));
							$messages[] = $errMsg;
						}
					}
					// invalid format
					else {
						$errMsg = $this->messages['invPol'][1];
						array_push($errMsg, array($i));
						$messages[] = $errMsg;
					}
				}
			}
			// add mail aliases
			if ($rawAccounts[$i][$ids['kolabUser_aliases']] != "") {
				$aliases = explode(',', $rawAccounts[$i][$ids['kolabUser_aliases']]);
				// check format
				for ($a = 0; $a < sizeof($aliases); $a++) {
					if (get_preg($aliases[$a], 'email')) {
						$partialAccounts[$i]['alias'][] = $aliases[$a];
					}
					// invalid format
					else {
						$errMsg = $this->messages['alias'][1];
						array_push($errMsg, array($i));
						$messages[] = $errMsg;
					}
				}
			}
			// add delegates
			if ($rawAccounts[$i][$ids['kolabUser_delegates']] != "") {
				$delegates = array();
				$delegatesTemp = $_SESSION['cache']->get_cache('mail', 'inetOrgPerson', 'user');
				if (is_array($delegatesTemp)) {
					$DNs = array_keys($delegatesTemp);
					foreach ($DNs as $DN) {
						$delegates[] = $delegatesTemp[$DN][0];
					}
					sort($delegates);
				}
				$newDelegates = explode(',', $rawAccounts[$i][$ids['kolabUser_delegates']]);
				// check format
				for ($d = 0; $d < sizeof($newDelegates); $d++) {
					if (in_array($newDelegates[$d], $delegates)) {
						$partialAccounts[$i]['kolabDelegate'][] = $newDelegates[$d];
					}
					// invalid format
					else {
						$errMsg = $this->messages['delegate'][0];
						array_push($errMsg, array($i));
						$messages[] = $errMsg;
					}
				}
			}
			// free/busy
			if ($rawAccounts[$i][$ids['kolabUser_freeBusy']] != "") {
				if (get_preg($rawAccounts[$i][$ids['kolabUser_freeBusy']], 'digit')) {
					$partialAccounts[$i]['kolabFreeBusyFuture'] = $rawAccounts[$i][$ids['kolabUser_freeBusy']];
				}
				else {
					$errMsg = $this->messages['freeBusy'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			// Cyrus mail quota
			if ($rawAccounts[$i][$ids['kolabUser_quota']] != "") {
				if (get_preg($rawAccounts[$i][$ids['kolabUser_quota']], 'digit')) {
					$partialAccounts[$i]['cyrus-userquota'] = $rawAccounts[$i][$ids['kolabUser_quota']];
				}
				else {
					$errMsg = $this->messages['quota'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
		}
		return $messages;
	}

	/**
	* Returns the PDF entries for this module.
	*
	* @return array list of possible PDF entries
	*/
	function get_pdfEntries() {
		$return = array();
		// country
		if (sizeof($this->attributes['c']) > 0) {
			$return['kolabUser_country'][0] = '<block><key>' . _('Country') . '</key><value>' . $this->attributes['c'][0] . '</value></block>';
		}
		// mail server
		if (sizeof($this->attributes['kolabHomeServer']) > 0) {
			$return['kolabUser_homeServer'][0] = '<block><key>' . _('Mailbox home server') . '</key><value>' . $this->attributes['kolabHomeServer'][0] . '</value></block>';
		}
		// mail quota
		if (sizeof($this->attributes['cyrus-userquota']) > 0) {
			$return['kolabUser_quota'][0] = '<block><key>' . _('Mail quota') . '</key><value>' . $this->attributes['cyrus-userquota'][0] . '</value></block>';
		}
		// free/busy
		if (sizeof($this->attributes['kolabFreeBusyFuture']) > 0) {
			$return['kolabUser_freeBusy'][0] = '<block><key>' . _('Free/Busy interval') . '</key><value>' . $this->attributes['kolabFreeBusyFuture'][0] . '</value></block>';
		}
		// invitation policies
		if (sizeof($this->attributes['kolabInvitationPolicy']) > 0) {
			// find default policy
			$default = "";
			for ($i = 0; $i < sizeof($this->attributes['kolabInvitationPolicy']); $i++) {
				if (!strpos($this->attributes['kolabInvitationPolicy'][$i], ":")) {
					$default = $this->attributes['kolabInvitationPolicy'][$i];
					break;
				}
			}
			$return['kolabUser_invPol'][0] = '<block><key>' . _('Invitation policy') . '</key><tr><td align=\"L\">' . _('Anyone') . ": " . $this->invitationPolicies[$default] . '</td></tr></block>';
			for ($i = 0; $i < sizeof($this->attributes['kolabInvitationPolicy']); $i++) {
				$parts = split(':', $this->attributes['kolabInvitationPolicy'][$i]);
				if (sizeof($parts) == 2) {
					$return['kolabUser_invPol'][] = '<block><tr><td align=\"L\">' . $parts[0] . ": " . $this->invitationPolicies[$parts[1]] . '</td></tr></block>';
				}
			}
		}
		// email aliases
		if (sizeof($this->attributes['alias']) > 0) {
			$return['kolabUser_aliases'][0] = '<block><key>' . _('EMail aliases') . '</key><value>' . implode(", ", $this->attributes['alias']) . '</value></block>';
		}
		// delegates
		if (sizeof($this->attributes['kolabDelegate']) > 0) {
			$return['kolabUser_delegate'][0] = '<block><key>' . _('Delegates') . '</key><value>' . implode(", ", $this->attributes['kolabDelegate']) . '</value></block>';
		}
		return $return;
	}

}


?>
