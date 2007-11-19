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
* Manages quotas for users and groups.
*
* @package modules
*
* @author Tilo Lutz
* @author Roland Gruber
* @author Michael Duergner
*/

/**
* Manages quotas for users and groups.
*
* @package modules
*/
class quota extends baseModule {

	/** this functin fills the error message array with messages
	**/
	function load_Messages() {
	// error messages for input checks
	$this->messages['softblock'][0] = array('ERROR', _('Block soft quota'), _('Block soft quota contains invalid characters. Only natural numbers are allowed.'));
	$this->messages['softblock'][1] = array('ERROR', _('Account %s:') . ' %s', _('Block soft quota contains invalid characters. Only natural numbers are allowed.'));
	$this->messages['hardblock'][0] = array('ERROR', _('Block hard quota'), _('Block hard quota contains invalid characters. Only natural numbers are allowed.'));
	$this->messages['hardblock'][1] = array('ERROR', _('Account %s:') . ' %s', _('Block hard quota contains invalid characters. Only natural numbers are allowed.'));
	$this->messages['softinode'][0] = array('ERROR', _('Inode soft quota'), _('Inode soft quota contains invalid characters. Only natural numbers are allowed.'));
	$this->messages['softinode'][1] = array('ERROR', _('Account %s:') . ' %s', _('Inode soft quota contains invalid characters. Only natural numbers are allowed.'));
	$this->messages['hardinode'][0] = array('ERROR', _('Inode hard quota'), _('Inode hard quota contains invalid characters. Only natural numbers are allowed.'));
	$this->messages['hardinode'][1] = array('ERROR', _('Account %s:') . ' %s', _('Inode hard quota contains invalid characters. Only natural numbers are allowed.'));
	$this->messages['block_cmp'][0] = array('ERROR', _('Block quota'), _('Block soft quota must be smaller than block hard quota.'));
	$this->messages['block_cmp'][1] = array('ERROR', _('Account %s:') . ' %s', _('Block soft quota must be smaller than block hard quota.'));
	$this->messages['inode_cmp'][0] = array('ERROR', _('Inode quota'), _('Inode soft quota must be smaller than inode hard quota.'));
	$this->messages['inode_cmp'][1] = array('ERROR', _('Account %s:') . ' %s', _('Inode soft quota must be smaller than inode hard quota.'));
	$this->messages['upload'][0] = array('ERROR', _('Account %s:') . ' %s', _('Quota has wrong format!'));
	}

	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	*/
	function get_metaData() {
		$return = array();
		// icon
		$return['icon'] = 'hard-drive.png';
		// manages user and group accounts
		$return["account_types"] = array("user", "group");
		// alias name
		$return["alias"] = _('Quota');
		if ($this->get_scope() == 'group') {
			// module dependencies
			$return['dependencies'] = array('depends' => array('posixGroup'), 'conflicts' => array());
		}
		if ($this->get_scope() == 'user') {
			// module dependencies
			$return['dependencies'] = array('depends' => array('posixAccount'), 'conflicts' => array());
		}
		// managed attributes
		$return['attributes'] = array('uid', 'cn');
		// available PDF fields
		$return['PDF_fields'] =  array(
			'quotas'
		);
		// help entries
		$return['help'] = array(
			"Mountpoint" => array(
				"Headline" => _("Mountpoint"),
				"Text" => _("Mountpoint of device with enabled quotas.")
			),
			"UsedBlocks" => array(
				"Headline" => _("Used blocks"),
				"Text" => _("Used blocks. 1000 blocks are usually 1MB")
			),
			"SoftBlockLimit" => array(
				"Headline" => _("Soft block limit"),
				"Text" => _("Soft block limit."), "SeeAlso" => array(
					'link' => 'http://www.ibiblio.org/pub/Linux/docs/HOWTO/other-formats/html_single/Quota.html#ss4.4"',
					'text' => 'Quota How-To')
			),
			"HardBlockLimit" => array(
				"Headline" => _("Hard block limit"),
				"Text" => _("Hard block limit"), "SeeAlso" => array(
					'link' => 'http://www.ibiblio.org/pub/Linux/docs/HOWTO/other-formats/html_single/Quota.html#ss4.5',
					'text' => 'Quota How-To')
			),
			"GraceBlockPeriod" => array(
				"Headline" => _("Grace block period"),
				"Text" => _("Grace block period. Most filesystems use a fixed maximum value of 7 days."), "SeeAlso" => array(
					'link' => 'http://www.ibiblio.org/pub/Linux/docs/HOWTO/other-formats/html_single/Quota.html#ss4.6',
					'text' => 'Quota How-To')
			),
			"UsedInodes" => array(
				"Headline" => _("Used inodes"),
				"Text" => _("Used inodes (files)").'.'
			),
			"SoftInodeLimit" => array(
				"Headline" => _("Soft inode limit"),
				"Text" => _("Soft inode (files) limit."), "SeeAlso" => array(
					'link' => 'http://www.ibiblio.org/pub/Linux/docs/HOWTO/other-formats/html_single/Quota.html#ss4.4',
					'text' => 'Quota How-To')
			),
			"HardInodeLimit" => array(
				"Headline" => _("Hard inode limit"),
				"Text" => _("Hard inode (files) limit"), "SeeAlso" => array(
					'link' => 'http://www.ibiblio.org/pub/Linux/docs/HOWTO/other-formats/html_single/Quota.html#ss4.5',
					'text' => 'Quota How-To')
			),
			"GraceInodePeriod" => array(
				"Headline" => _("Grace inode period"),
				"Text" => _("Grace inode (files) period. Most filesystems use a fixed maximum value of 7 days."), "SeeAlso" => array(
					'link' => 'http://www.ibiblio.org/pub/Linux/docs/HOWTO/other-formats/html_single/Quota.html#ss4.6',
					'text' => 'Quota How-To')
			),
			"upload" => array(
				"Headline" => _("Quota"),
				"Text" => _("Please enter the quota settings for this mount point. The syntax is: {soft block limit},{hard block limit},{soft inode limit},{hard inode limit}")
			)
		);
		return $return;
	}

	/** Saves the quota settings */
	private $quota;
	
	/**
	 * Initializes the quota values.
	 *
	 */
	function initQuotas() {
		if (isset($this->quota)) return;
		$userName = '+';
		if (($this->getAccountContainer() != null) && !$this->getAccountContainer()->isNewAccount) {
			if ($this->get_scope() == 'user') {
				if (!isset($this->attributes['uid'][0])) return;
				$userName = $this->attributes['uid'][0];
			}
			else if ($this->get_scope() == 'group') {
				if (!isset($this->attributes['cn'][0])) return;
				$userName = $this->attributes['cn'][0];
			}
		}
		// get list of lamdaemon servers
		$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
		for ($s = 0; $s < sizeof($lamdaemonServers); $s++) {
			$temp = explode(":", $lamdaemonServers[$s]);
			$server = $temp[0];
			// get quotas
			$quotas = lamdaemon(array($userName . " quota get " . $this->get_scope()), $server);
			$allQuotas = explode(":", $quotas[0]);
			array_pop($allQuotas); // remove empty element at the end
			for ($i = 0; $i < sizeof($allQuotas); $i++) {
				$singleQuota = explode(",", $allQuotas[$i]);
				$this->quota[$server][$i] = $singleQuota;
				if ($this->quota[$server][$i][4] < time()) $this->quota[$server][$i][4] = '';
				else $this->quota[$server][$i][4] = strval(intval(($this->quota[$server][$i][4] - time())/3600)) .' '. _('hours');
				if ($this->quota[$server][$i][8] < time()) $this->quota[$server][$i][8] = '';
				else $this->quota[$server][$i][8] = strval(intval(($this->quota[$server][$i][8] - time())/3600)) .' '. _('hours');
			}
		}
	}

	/**
	* This function is used to check if this module page can be displayed.
	* It returns false if a module depends on data from other modules which was not yet entered.
	*
	* @return boolean true, if page can be displayed
	*/
	function module_ready() {
		if ($this->get_scope()=='user') {
			$attrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
			if ($attrs['uid'][0]=='') return false;
		}
		if ($this->get_scope()=='group') {
			$attrs = $this->getAccountContainer()->getAccountModule('posixGroup')->getAttributes();
			if ($attrs['cn'][0]=='') return false;
		}
		return true;
	}

	/**
	 * Allows the module to run commands after the LDAP entry is changed or created.
	 *
	 * @param boolean $newAccount new account
	 */
	function postModifyActions($newAccount) {
		if (!isset($this->quota) || !is_array($this->quota)) return;
		// determine if this is a user or group account
		if ($this->get_scope()=='user') {
			$attrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
			$id = $attrs['uid'][0];
		}
		if ($this->get_scope()=='group') {
			$attrs = $this->getAccountContainer()->getAccountModule('posixGroup')->getAttributes();
			$id = $attrs['cn'][0];
		}
		// get list of lamdaemon servers
		$serverDescriptions = array();
		$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
		for ($s = 0; $s < sizeof($lamdaemonServers); $s++) {
			$temp = explode(":", $lamdaemonServers[$s]);
			if (isset($temp[1])) {
				$serverDescriptions[$temp[0]] = $temp[1];
			}
		}
		$servers = array_keys($this->quota);
		for ($q = 0; $q < sizeof($servers); $q++) {
			$server = $servers[$q];
			$i=0;
			$quotastring = "";
			for ($i = 0; $i < sizeof($this->quota[$server]); $i++) {
				$quotastring = $quotastring . $this->quota[$server][$i][0] . ',' . $this->quota[$server][$i][2] . ',' . $this->quota[$server][$i][3]
					. ',' . $this->quota[$server][$i][6] . ',' . $this->quota[$server][$i][7] . ':';
			}
			$ret = lamdaemon(array($id . " quota set " . $this->get_scope() . " $quotastring\n"), $server);
		}
	}

	/**
	 * Allows the module to run commands before the LDAP entry is deleted.
	 * An error message should be printed if the function returns false.
	 * 
	 * @return true, if no problems occured
	 */
	function preDeleteActions() {
		$this->initQuotas();
		if (!isset($this->quota) || !is_array($this->quota)) return true;
		// determine if this is a user or group account
		if ($this->get_scope()=='user') {
			$attrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
			$id = $attrs['uid'][0];
		}
		if ($this->get_scope()=='group') {
			$attrs = $this->getAccountContainer()->getAccountModule('posixGroup')->getAttributes();
			$id = $attrs['cn'][0];
		}
		// get list of lamdaemon servers
		$serverDescriptions = array();
		$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
		for ($s = 0; $s < sizeof($lamdaemonServers); $s++) {
			$temp = explode(":", $lamdaemonServers[$s]);
			if (isset($temp[1])) {
				$serverDescriptions[$temp[0]] = $temp[1];
			}
		}
		$servers = array_keys($this->quota);
		for ($q = 0; $q < sizeof($servers); $q++) {
			$server = $servers[$q];
			$i=0;
			$quotastring = "";
			while ($this->quota[$server][$i][0]) {
				$quotastring = $quotastring . $this->quota[$server][$i][0] . ',0,0,0,0:';
				$i++;
				}
			$ret = lamdaemon(array($id . " quota set " . $this->get_scope() . " $quotastring\n"), $server);
		}
		return true;
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_attributes() {
		if (!isset($this->quota) || !is_array($this->quota)) return array();
		$errors = array();
		// get list of lamdaemon servers
		$serverDescriptions = array();
		$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
		for ($s = 0; $s < sizeof($lamdaemonServers); $s++) {
			$temp = explode(":", $lamdaemonServers[$s]);
			if (isset($temp[1])) {
				$serverDescriptions[$temp[0]] = $temp[1];
			}
		}
		$servers = array_keys($this->quota);
		for ($q = 0; $q < sizeof($servers); $q++) {
			$server = $servers[$q];
			$id = $this->replaceSpecialChars($server);
			$i=0;
			// loop for every mointpoint with quotas
			while (isset($this->quota[$server][$i][0])) {
				$this->quota[$server][$i][2] = $_POST[$i . '_2_' . $id];
				$this->quota[$server][$i][3] = $_POST[$i . '_3_' . $id];
				$this->quota[$server][$i][6] = $_POST[$i . '_6_' . $id];
				$this->quota[$server][$i][7] = $_POST[$i . '_7_' . $id];
				// Check if values are OK and set automatic values. if not error-variable will be set
				if (!get_preg($this->quota[$server][$i][2], 'digit'))
					$errors[] = $this->messages['softblock'][0];
				if (!get_preg($this->quota[$server][$i][3], 'digit'))
					$errors[] = $this->messages['hardblock'][0];
				if (!get_preg($this->quota[$server][$i][6], 'digit'))
					$errors[] = $this->messages['softinode'][0];
				if (!get_preg($this->quota[$server][$i][7], 'digit'))
					$errors[] = $this->messages['hardinode'][0];
				if (intval($this->quota[$server][$i][2]) > intval($this->quota[$server][$i][3]))
					$errors[] = $this->messages['block_cmp'][0];
				if (intval($this->quota[$server][$i][6]) > intval($this->quota[$server][$i][7]))
					$errors[] = $this->messages['inode_cmp'][0];
				$i++;
			}
		}
		return $errors;
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return array HTML meta data
	 */
	function display_html_attributes() {
		$return = array();
		$this->initQuotas();
		if (!is_array($this->quota)) return $return;
		// get list of lamdaemon servers
		$serverDescriptions = array();
		$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
		for ($s = 0; $s < sizeof($lamdaemonServers); $s++) {
			$temp = explode(":", $lamdaemonServers[$s]);
			if (isset($temp[1])) {
				$serverDescriptions[$temp[0]] = $temp[1];
			}
		}
		$servers = array_keys($this->quota);
		for ($q = 0; $q < sizeof($servers); $q++) {
			$server = $servers[$q];
			$id = $this->replaceSpecialChars($server);
			$fieldContent = array();
			$fieldContent[] = array(
					array('kind' => 'text', 'text' => _('Mountpoint')),
					array('kind' => 'text', 'text' => _('Used blocks')),
					array('kind' => 'text', 'text' => _('Soft block limit')),
					array('kind' => 'text', 'text' => _('Hard block limit')),
					array('kind' => 'text', 'text' => _('Grace block period')),
					array('kind' => 'text', 'text' => _('Used inodes')),
					array('kind' => 'text', 'text' => _('Soft inode limit')),
					array('kind' => 'text', 'text' => _('Hard inode limit')),
					array('kind' => 'text', 'text' => _('Grace inode period')));
	
			$fieldContent[] = array(
					array('kind' => 'help', 'value' => 'Mountpoint'),
					array('kind' => 'help', 'value' => 'UsedBlocks'),
					array('kind' => 'help', 'value' => 'SoftBlockLimit'),
					array('kind' => 'help', 'value' => 'HardBlockLimit'),
					array('kind' => 'help', 'value' => 'GraceBlockPeriod'),
					array('kind' => 'help', 'value' => 'UsedInodes'),
					array('kind' => 'help', 'value' => 'SoftInodeLimit'),
					array('kind' => 'help', 'value' => 'HardInodeLimit'),
					array('kind' => 'help', 'value' => 'GraceInodePeriod')
				);
	
			$i=0;
			// loop for every mointpoint with enabled quotas
			while (isset($this->quota[$server][$i][0])) {
				$fieldContent[] = array(
					array('kind' => 'text', 'text' => $this->quota[$server][$i][0]),
					array('kind' => 'text', 'text' => $this->quota[$server][$i][1]),
					array('kind' => 'input', 'name' => $i . '_2_' . $id, 'type' => 'text', 'size' => '12', 'maxlength' => '20', 'value' => $this->quota[$server][$i][2]),
					array('kind' => 'input', 'name' => $i . '_3_' . $id, 'type' => 'text', 'size' => '12', 'maxlength' => '20', 'value' => $this->quota[$server][$i][3]),
					array('kind' => 'text', 'text' => $this->quota[$server][$i][4]),
					array('kind' => 'text', 'text' => $this->quota[$server][$i][5]),
					array('kind' => 'input', 'name' => $i . '_6_' . $id, 'type' => 'text', 'size' => '12', 'maxlength' => '20', 'value' => $this->quota[$server][$i][6]),
					array('kind' => 'input', 'name' => $i . '_7_' . $id, 'type' => 'text', 'size' => '12', 'maxlength' => '20', 'value' => $this->quota[$server][$i][7]),
					array('kind' => 'text', 'text' => $this->quota[$server][$i][8]));
				$i++;
			}
			$legend = $server;
			if (isset($serverDescriptions[$server])) {
				$legend = $serverDescriptions[$server] . " (" . $server . ")";
			}
			$return[] = array(
				array('kind' => 'fieldset', 'legend' => $legend, 'value' => $fieldContent)
			);
		}
		return $return;
	}
	
	/**
	 * Replaces special characters in HTML name values.
	 *
	 * @param string $input input string
	 * @return string output string
	 */
	function replaceSpecialChars($input) {
		$ret = str_replace(".", "_", $input);
		return $ret;
	}

	/**
	* Returns a list of elements for the account profiles.
	*
	* @return profile elements
	*/
	function get_profileOptions() {
		$return = array();
		// get list of lamdaemon servers
		$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
		for ($s = 0; $s < sizeof($lamdaemonServers); $s++) {
			$temp = explode(":", $lamdaemonServers[$s]);
			$server = $temp[0];
			$id = $this->replaceSpecialChars($server);
			$description = $temp[0];
			if (isset($temp[1])) $description = $temp[1];

			// Get quotas
			$quotas = lamdaemon(array("+ quota get " . $this->get_scope()), $server);
			$dirs = split(":", $quotas[0]);
			array_pop($dirs); // remove empty element at the end
			for ($i = 0; $i < sizeof($dirs); $i++) {
				$dirs[$i] = split(",", $dirs[$i]);
				$dirs[$i] = $dirs[$i][0];
			}
			if (sizeof($dirs) < 1) continue; // stop if no quota directories were found
			$fieldContent = array();
			$fieldContent[] = array (
				array('kind' => 'text', 'text' => '<b>' . _('Mountpoint') . '&nbsp;&nbsp;</b>', 'align' => 'center'),
				array('kind' => 'text', 'text' => '<b>' . _('Soft block limit') . '&nbsp;&nbsp;</b>', 'align' => 'center'),
				array('kind' => 'text', 'text' => '<b>' . _('Hard block limit') . '&nbsp;&nbsp;</b>', 'align' => 'center'),
				array('kind' => 'text', 'text' => '<b>' . _('Soft inode limit') . '&nbsp;&nbsp;</b>', 'align' => 'center'),
				array('kind' => 'text', 'text' => '<b>' . _('Hard inode limit') . '&nbsp;&nbsp;</b>', 'align' => 'center'),
			);
			$fieldContent[] = array (
				array('kind' => 'help', 'value' => 'Mountpoint', 'align' => 'center'),
				array('kind' => 'help', 'value' => 'SoftBlockLimit', 'align' => 'center'),
				array('kind' => 'help', 'value' => 'HardBlockLimit', 'align' => 'center'),
				array('kind' => 'help', 'value' => 'SoftInodeLimit', 'align' => 'center'),
				array('kind' => 'help', 'value' => 'HardInodeLimit', 'align' => 'center'),
			);
			for ($i = 0; $i < sizeof($dirs); $i++) {
				$fieldContent[] = array(
					array('kind' => 'text', 'text' => $dirs[$i], 'align' => 'left'),
					array('kind' => 'input', 'type' => 'text', 'align' => 'center', 'size' => '12', 'name' => "quota_softblock_" . $id . "_" . $dirs[$i]),
					array('kind' => 'input', 'type' => 'text', 'align' => 'center', 'size' => '12', 'name' => "quota_hardblock_" . $id . "_" . $dirs[$i]),
					array('kind' => 'input', 'type' => 'text', 'align' => 'center', 'size' => '12', 'name' => "quota_softinode_" . $id . "_" . $dirs[$i]),
					array('kind' => 'input', 'type' => 'text', 'align' => 'center', 'size' => '12', 'name' => "quota_hardinode_" . $id . "_" . $dirs[$i]),
				);
			}
			$return[] = array(
				array('kind' => 'fieldset', 'legend' => $description . " (" . $server . ")", 'value' => $fieldContent)
			);
		}
		return $return;
	}

	/**
	* Checks input values of account profiles.
	*
	* @param array $options a hash array (name => value) containing the options
	* @return array list of error messages (array(type, title, text)) to generate StatusMessages, if any
	*/
	function check_profileOptions($options) {
		$return = array();
		// get list of lamdaemon servers
		$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
		for ($s = 0; $s < sizeof($lamdaemonServers); $s++) {
			$temp = explode(":", $lamdaemonServers[$s]);
			$server = $temp[0];
			$id = $this->replaceSpecialChars($server);
			// Get quotas
			$quotas = lamdaemon(array("+ quota get " . $this->get_scope()), $server);
			$dirs = split(":", $quotas[0]);
			array_pop($dirs); // remove empty element at the end
			for ($i = 0; $i < sizeof($dirs); $i++) {
				$dirs[$i] = split(",", $dirs[$i]);
				$dirs[$i] = $dirs[$i][0];
			}
			for ($i = 0; $i < sizeof($dirs); $i++) {
				if (!get_preg($options["quota_softblock_" . $id . "_" . $dirs[$i]][0], 'digit')) $return[] = $this->messages['softblock'][0];
				if (!get_preg($options["quota_hardblock_" . $id . "_" . $dirs[$i]][0], 'digit')) $return[] = $this->messages['hardblock'][0];
				if (!get_preg($options["quota_softinode_" . $id . "_" . $dirs[$i]][0], 'digit')) $return[] = $this->messages['softinode'][0];
				if (!get_preg($options["quota_hardinode_" . $id . "_" . $dirs[$i]][0], 'digit')) $return[] = $this->messages['hardinode'][0];
				if (intval($options["quota_softblock_" . $id . "_" . $dirs[$i]][0]) > intval($options["quota_hardblock_" . $id . "_" . $dirs[$i]][0])) $return[] = $this->messages['block_cmp'][0];
				if (intval($options["quota_softinode_" . $id . "_" . $dirs[$i]][0]) > intval($options["quota_hardinode_" . $id . "_" . $dirs[$i]][0])) $return[] = $this->messages['inode_cmp'][0];
				$i++;
			}
		}
		return $return;
	}

	/**
	* Loads the values of an account profile into internal variables.
	*
	* @param array $profile hash array with profile values (identifier => value)
	*/
	function load_profile($profile) {
		$this->initQuotas();
		if (!isset($this->quota) || !is_array($this->quota)) return;
		$servers = array_keys($this->quota);
		for ($s = 0; $s < sizeof($servers); $s++) {
			$server = $servers[$s];
			$id = $this->replaceSpecialChars($server);
			for ($i = 0; $i < sizeof($this->quota[$server]); $i++) {
				$dir = $this->quota[$server][$i][0];
				if (isset($profile["quota_softblock_" . $id . "_" . $dir])) $this->quota[$server][$i][2] = $profile["quota_softblock_" . $id . "_" . $dir][0];
				if (isset($profile["quota_hardblock_" . $id . "_" . $dir])) $this->quota[$server][$i][3] = $profile["quota_hardblock_" . $id . "_" . $dir][0];
				if (isset($profile["quota_softinode_" . $id . "_" . $dir])) $this->quota[$server][$i][6] = $profile["quota_softinode_" . $id . "_" . $dir][0];
				if (isset($profile["quota_hardinode_" . $id . "_" . $dir])) $this->quota[$server][$i][7] = $profile["quota_hardinode_" . $id . "_" . $dir][0];
			}
		}
	}

	/**
	* Returns the PDF entries for this module.
	*
	* @return array list of possible PDF entries
	*/
	function get_pdfEntries() {
		$this->initQuotas();
		if (!isset($this->quota) || !is_array($this->quota)) return array();
		if (sizeof($this->quota) > 0) {
			$quotas = array();
			// get list of lamdaemon servers
			$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
			for ($s = 0; $s < sizeof($lamdaemonServers); $s++) {
				$temp = explode(":", $lamdaemonServers[$s]);
				$server = $temp[0];
				$description = $server;
				if (isset($temp[1])) $description = $temp[1] . " (" . $server . ")";
				if (!isset($this->quota[$server]) || (sizeof($this->quota[$server]) < 1)) continue;
				
				$quotas[] = '<block><tr>' .
						'<td width="80%"><b>' . $description . '</b></td>' .
						'</tr></block>';
				$quotas[] = '<block><tr>' .
						'<td width="20%"><b>' . _('Mountpoint') . '</b></td>' .
						'<td width="20%"><b>' . _('Soft block') . '</b></td>' .
						'<td width="20%"><b>' . _('Hard block') . '</b></td>' .
						'<td width="20%"><b>' . _('Soft inode') . '</b></td>' .
						'<td width="20%"><b>' . _('Hard inode') . '</b></td></tr></block>';
				for ($i = 0; $i < sizeof($this->quota[$server]); $i++) {
					$quotas[] = '<block><tr>' .
						'<td width="20%" align=\"L\">' . $this->quota[$server][$i][0] . '</td>' .
						'<td width="20%" align=\"L\">' . $this->quota[$server][$i][2] . '</td>' .
						'<td width="20%" align=\"L\">' . $this->quota[$server][$i][3] . '</td>' .
						'<td width="20%" align=\"L\">' . $this->quota[$server][$i][6] . '</td>' .
						'<td width="20%" align=\"L\">' . $this->quota[$server][$i][7] . '</td></tr></block>';
				}
				$quotas[] = '<block><tr><td width="80%"> </td></tr></block>';
			}
			return array('quota_quotas' => $quotas);
		}
		else {
			return array();
		}
	}

	/**
	* Returns an array containing all input columns for the file upload.
	*
	* Syntax:
	* <br> array(
	* <br>  string: name,  // fixed non-translated name which is used as column name (should be of format: <module name>_<column name>)
	* <br>  string: description,  // short descriptive name
	* <br>  string: help,  // help ID
	* <br>  string: example,  // example value
	* <br>  boolean: required  // true, if user must set a value for this column
	* <br> )
	*
	* @return array column list
	*/
	function get_uploadColumns() {
		$this->initQuotas();
		if (!isset($this->quota) || !is_array($this->quota)) return array();
		$return = array();
		if (sizeof($this->quota) > 0) {
			// get list of lamdaemon servers
			$lamdaemonServers = explode(";", $_SESSION['config']->get_scriptServers());
			for ($s = 0; $s < sizeof($lamdaemonServers); $s++) {
				$temp = explode(":", $lamdaemonServers[$s]);
				$server = $temp[0];
				// Get quotas
				$quotas = lamdaemon(array("+ quota get " . $this->get_scope()), $server);
				$dirs = split(":", $quotas[0]);
				array_pop($dirs); // remove empty element at the end
				for ($i = 0; $i < sizeof($dirs); $i++) {
					$dirs[$i] = split(",", $dirs[$i]);
					$dirs[$i] = $dirs[$i][0];
				}
				for ($i = 0; $i < sizeof($dirs); $i++) {
					$return[] = array(
						'name' => 'quota_' . $server . ':' . $dirs[$i],
						'description' => sprintf(_('Quota for %s on %s'), $dirs[$i], $server),
						'help' => 'upload',
						'example' => '2000,2500,3000,3500');
				}
			}
		}
		return $return;
	}

	/**
	* This function executes one post upload action.
	*
	* @param array $data array containing one account in each element
	* @param array $ids array(<column_name> => <column number>)
	* @param array $failed list of accounts which were not created successfully
	* @param array $temp variable to store temporary data between two post actions
	* @return array current status
	* <br> array (
	* <br>  'status' => 'finished' | 'inProgress'
	* <br>  'progress' => 0..100
	* <br>  'errors' => array (<array of parameters for StatusMessage>)
	* <br> )
	*/
	function doUploadPostActions($data, $ids, $failed, &$temp) {
		$errors = array();
		// first call, get list of user names and quota values
		if (!isset($temp['counter'])) {
			$temp['counter'] = 0;
			// create list of quota columns
			$temp['quotas'] = array();
			$columns = array_keys($ids);
			for ($i = 0; $i < sizeof($columns); $i++) {
				if (strpos($columns[$i], 'quota_') === 0) {
					$temp['quotas'][] = substr($columns[$i], 6);
				}
			}
			// select user/group name depending on current scope
			$temp['accounts'] = array();
			$col = 'invalid';
			if ($this->get_scope() == 'user') $col = $ids['posixAccount_userName'];
			elseif ($this->get_scope() == 'group') $col = $ids['posixGroup_cn'];
			// create list of account names and their quota values
			for ($i = 0; $i < sizeof($data); $i++) {
				if (in_array($i, $failed)) continue; // ignore failed accounts
				$name = $data[$i][$col];
				for ($m = 0; $m < sizeof($temp['quotas']); $m++) {
					if ($data[$i][$ids['quota_' . $temp['quotas'][$m]]] != '') {
						$parts = explode(',', $data[$i][$ids['quota_' . $temp['quotas'][$m]]]);
						// check syntax
						if (sizeof($parts) != 4) {
							$errMsg = $this->messages['upload'][0];
							array_push($errMsg, array($i, 'quota_' . $temp['quotas'][$m]));
							$errors[] = $errMsg;
							continue;
						}
						if (!get_preg($parts[0], 'digit')) {
							$errMsg = $this->messages['softblock'][1];
							array_push($errMsg, array($i, 'quota_' . $temp['quotas'][$m]));
							$errors[] = $errMsg;
							continue;
						}
						if (!get_preg($parts[1], 'digit')) {
							$errMsg = $this->messages['hardblock'][1];
							array_push($errMsg, array($i, 'quota_' . $temp['quotas'][$m]));
							$errors[] = $errMsg;
							continue;
						}
						if (!get_preg($parts[2], 'digit')) {
							$errMsg = $this->messages['softinode'][1];
							array_push($errMsg, array($i, 'quota_' . $temp['quotas'][$m]));
							$errors[] = $errMsg;
							continue;
						}
						if (!get_preg($parts[3], 'digit')) {
							$errMsg = $this->messages['hardinode'][1];
							array_push($errMsg, array($i, 'quota_' . $temp['quotas'][$m]));
							$errors[] = $errMsg;
							continue;
						}
						if ($parts[0] > $parts[1]) {
							$errMsg = $this->messages['block_cmp'][1];
							array_push($errMsg, array($i, 'quota_' . $temp['quotas'][$m]));
							$errors[] = $errMsg;
							continue;
						}
						if ($parts[2] > $parts[3]) {
							$errMsg = $this->messages['inode_cmp'][1];
							array_push($errMsg, array($i, 'quota_' . $temp['quotas'][$m]));
							$errors[] = $errMsg;
							continue;
						}
						// save quota settings
						$temp['accounts'][$name][$temp['quotas'][$m]] = $parts;
					}
				}
			}
			return array('status' => 'inProgress', 'progress' => 5, 'errors' => $errors);
		}
		// quotas are ready to set
		elseif ($temp['counter'] < sizeof($temp['accounts'])) {
			$names = array_keys($temp['accounts']);
			$name = $names[$temp['counter']];
			$mountPoints = array_keys($temp['accounts'][$name]);
			// set quota
			for ($m = 0; $m < sizeof($mountPoints); $m++) {
				$mpParts = split(":", $mountPoints[$m]);
				$server = $mpParts[0];
				$dir = $mpParts[1];
				$quotaString = $name . " quota set " . $this->get_scope() . " " . $dir . ',' .
					implode(',', $temp['accounts'][$name][$mountPoints[$m]]) . "\n";
				$result = lamdaemon(array($quotaString), $server);
				if (is_array($result)) {
					for ($i = 0; $i < sizeof($result); $i++) {
						$parts = explode(",", $result);
						if ($parts[0] == 'ERROR') {
							$errors[] = array('ERROR', $parts[1], $parts[2]);
						}
					}
				}
			}
			// set counters to next account/mount point
			$temp['counter']++;
			return array(
				'status' => 'inProgress',
				'progress' => 5 + (95 * ($temp['counter'] / sizeof($temp['accounts']))),
				'errors' => $errors);
		}
		return array('status' => 'finished');
	}

}

?>
