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
				"Text" => _("Soft block limit."), "SeeAlso" => '<a href="http://www.ibiblio.org/pub/Linux/docs/HOWTO/other-formats/html_single/Quota.html#ss4.4">'.'Quota How-To</a>'
			),
			"HardBlockLimit" => array(
				"Headline" => _("Hard block limit"),
				"Text" => _("Hard block limit").'.', "SeeAlso" => '<a href="http://www.ibiblio.org/pub/Linux/docs/HOWTO/other-formats/html_single/Quota.html#ss4.5">'.'Quota How-To</a>'
			),
			"GraceBlockPeriod" => array(
				"Headline" => _("Grace block period"),
				"Text" => _("Grace block period. Most filesystems use a fixed maximum value of 7 days."), "SeeAlso" => '<a href="http://www.ibiblio.org/pub/Linux/docs/HOWTO/other-formats/html_single/Quota.html#ss4.6">'.'Quota How-To</a>'
			),
			"UsedInodes" => array(
				"Headline" => _("Used inodes"),
				"Text" => _("Used inodes (files)").'.'
			),
			"SoftInodeLimit" => array(
				"Headline" => _("Soft inode limit"),
				"Text" => _("Soft inode (files) limit."), "SeeAlso" => '<a href="http://www.ibiblio.org/pub/Linux/docs/HOWTO/other-formats/html_single/Quota.html#ss4.4">'.'Quota How-To</a>'
			),
			"HardInodeLimit" => array(
				"Headline" => _("Hard inode limit"),
				"Text" => _("Hard inode (files) limit").'.', "SeeAlso" => '<a href="http://www.ibiblio.org/pub/Linux/docs/HOWTO/other-formats/html_single/Quota.html#ss4.5">'.'Quota How-To</a>'
			),
			"GraceInodePeriod" => array(
				"Headline" => _("Grace inode period"),
				"Text" => _("Grace inode (files) period. Most filesystems use a fixed maximum value of 7 days."), "SeeAlso" => '<a href="http://www.ibiblio.org/pub/Linux/docs/HOWTO/other-formats/html_single/Quota.html#ss4.6">'.'Quota How-To</a>'
			),
			"upload" => array(
				"Headline" => _("Quota"),
				"Text" => _("Please enter the quota settings for this mount point. The syntax is: {soft block limit},{hard block limit},{soft inode limit},{hard inode limit}")
			)
		);
		return $return;
	}

	// Constructor
	function init($base) {
		// call parent init
		parent::init($base);
		// Get basic quotas for new account
		$output_array = lamdaemon(array("+ quota get " . $_SESSION[$this->base]->type));
		// process quotas
		if (is_array($output_array)) {
			$all_quota = explode(':', $output_array[0]);
			for ($j=0; $j<sizeof($all_quota)-1; $j++) {
				$single_quota = explode(',', $all_quota[$j]);
				// unset not existing quotas from account-object
				// collect all existing mountpoints in array
				$real_quotas[] = $single_quota[0];
				for ($k=0; $k<sizeof($single_quota); $k++)
					$this->quota[$j][$k] = $single_quota[$k];
				if ($this->quota[$j][4]<time()) $this->quota[$j][4] = '';
					else $this->quota[$j][4] = strval(intval(($this->quota[$j][4]-time())/3600)) .' '. _('hours');
				if ($this->quota[$j][8]<time()) $this->quota[$j][8] = '';
					else $this->quota[$j][8] = strval(intval(($this->quota[$j][8]-time())/3600)) .' '. _('hours');
				}
			$j=0;
			while (isset($this->quota[$j][0]))
				// remove invalid quotas
				if (!in_array($this->quota[$j][0], $real_quotas)) unset($this->quota[$j]);
					else $j++;
			// Beautify array, repair index
			if (is_array($this->quota)) $this->quota = array_values($this->quota);
			}
		}

	// Variables
	var $quota;

	function module_ready() {
		if ($_SESSION[$this->base]->type=='user' && $_SESSION[$this->base]->module['posixAccount']->attributes['uid'][0]=='') return false;
		if ($_SESSION[$this->base]->type=='group' && $_SESSION[$this->base]->module['posixGroup']->attributes['cn'][0]=='') return false;
		return true;
	}

	/* This functions return true
	* if all needed settings are done
	*/
	function module_complete() {
		if (!$this->module_ready()) return false;
		return true;
	}

	/* This function loads all attributes into the object
	* $attr is an array as it's retured from ldap_get_attributes
	*/
	function load_attributes($attr) {
		// Load name
		if ($_SESSION[$this->base]->type=='user') $id = $attr['uid'][0];
		if ($_SESSION[$this->base]->type=='group') $id = $attr['cn'][0];
		// Get quotas
		$output_array = lamdaemon(array("$id quota get " . $_SESSION[$this->base]->type));
		// process quotas
		if (is_array($output_array)) {
			$all_quota = explode(':', $output_array[0]);
			for ($j=0; $j<sizeof($all_quota)-1; $j++) {
				$single_quota = explode(',', $all_quota[$j]);
				// unset not existing quotas from account-object
				// collect all existing mountpoints in array
				$real_quotas[] = $single_quota[0];
				for ($k=0; $k<sizeof($single_quota); $k++)
					$this->quota[$j][$k] = $single_quota[$k];
				if ($this->quota[$j][4]<time()) $this->quota[$j][4] = '';
					else $this->quota[$j][4] = strval(intval(($this->quota[$j][4]-time())/3600)) .' '. _('hours');
				if ($this->quota[$j][8]<time()) $this->quota[$j][8] = '';
					else $this->quota[$j][8] = strval(intval(($this->quota[$j][8]-time())/3600)) .' '. _('hours');
				}
			$j=0;
			while (isset($this->quota[$j][0]))
				// remove invalid quotas
				if (!in_array($this->quota[$j][0], $real_quotas)) unset($this->quota[$j]);
					else $j++;
			// Beautify array, repair index
			if (is_array($this->quota)) $this->quota = array_values($this->quota);
			}
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
		/* Check wich quotas have changed
		* Because we can not send an array to lamdaemon.pl we have to put all
		* values in a string. ':' sepraeates the first array, ',' the second
		*
		* $values->quota[][] First array is an index for every chare with active quotas
		* second array Contains values for every share:
		* mountpoint, used blocks, soft block limit, hard block limit, grace block period, used inodes,
		* soft inode limit, hard inode limit, grace inode period
		*/

		$i=0;
		$quotastring = "";
		while ($this->quota[$i][0]) {
			$quotastring = $quotastring . $this->quota[$i][0] . ',' . $this->quota[$i][2] . ',' . $this->quota[$i][3]
				. ',' . $this->quota[$i][6] . ',' . $this->quota[$i][7] . ':';
			$i++;
			}

		if ($_SESSION[$this->base]->type=='user') $id = $_SESSION[$this->base]->module['posixAccount']->attributes['uid'][0];
		if ($_SESSION[$this->base]->type=='group') $id = $_SESSION[$this->base]->module['posixGroup']->attributes['cn'][0];
		$return[$_SESSION[$this->base]->dn]['lamdaemon']['command'][] = $id . " quota set " . $_SESSION[$this->base]->type . " $quotastring\n";

		return $return;
		}

	function delete_attributes() {
		$i=0;
		$quotastring = "";
		while ($this->quota[$i][0]) {
			$quotastring = $quotastring . $this->quota[$i][0] . ',0,0,0,0:';
			$i++;
			}
		if ($_SESSION[$this->base]->type=='user') $id = $_SESSION[$this->base]->module['posixAccount']->attributes['uid'][0];
		if ($_SESSION[$this->base]->type=='group') $id = $_SESSION[$this->base]->module['posixGroup']->attributes['cn'][0];
		$return[$_SESSION[$this->base]->dn_orig]['lamdaemon']['command'][] = $id . " quota set " . $_SESSION[$this->base]->type . " $quotastring\n";
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
		// Write all general values into $account_new
		$i=0;
		// loop for every mointpoint with quotas
		while ($this->quota[$i][0]) {
			$this->quota[$i][2] = $_POST[$i . '_2'];
			$this->quota[$i][3] = $_POST[$i . '_3'];
			$this->quota[$i][6] = $_POST[$i . '_6'];
			$this->quota[$i][7] = $_POST[$i . '_7'];
			// Check if values are OK and set automatic values. if not error-variable will be set
			if (!get_preg($this->quota[$i][2], 'digit'))
				$errors[$this->quota[$i][2]][] = $this->messages['softblock'][0];
			if (!get_preg($this->quota[$i][3], 'digit'))
				$errors[$this->quota[$i][3]][] = $this->messages['hardblock'][0];
			if (!get_preg($this->quota[$i][6], 'digit'))
				$errors[$this->quota[$i][6]][] = $this->messages['softinode'][0];
			if (!get_preg($this->quota[$i][7], 'digit'))
				$errors[$this->quota[$i][7]][] = $this->messages['hardinode'][0];
			if (intval($this->quota[$i][2]) > intval($this->quota[$i][3]))
				$errors[$this->quota[$i][2]][] = $this->messages['block_cmp'][0];
			if (intval($this->quota[$i][6]) > intval($this->quota[$i][7]))
				$errors[$this->quota[$i][6]][] = $this->messages['inode_cmp'][0];
			$i++;
			}
		return $errors;
	}

	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_attributes() {

		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Mountpoint') ),
				1 => array ( 'kind' => 'text', 'text' => _('Used blocks') ),
				2 => array ( 'kind' => 'text', 'text' => _('Soft block limit') ),
				3 => array ( 'kind' => 'text', 'text' => _('Hard block limit') ),
				4 => array ( 'kind' => 'text', 'text' => _('Grace block period') ),
				5 => array ( 'kind' => 'text', 'text' => _('Used inodes') ),
				6 => array ( 'kind' => 'text', 'text' => _('Soft inode limit') ),
				7 => array ( 'kind' => 'text', 'text' => _('Hard inode limit') ),
				8 => array ( 'kind' => 'text', 'text' => _('Grace inode period') ));

		$return[] = array ( 0 => array ( 'kind' => 'help', 'value' => 'Mountpoint' ),
				1 => array ( 'kind' => 'help', 'value' => 'UsedBlocks' ),
				2 => array ( 'kind' => 'help', 'value' => 'SoftBlockLimit' ),
				3 => array ( 'kind' => 'help', 'value' => 'HardBlockLimit' ),
				4 => array ( 'kind' => 'help', 'value' => 'GraceBlockPeriod' ),
				5 => array ( 'kind' => 'help', 'value' => 'UsedInodes' ),
				6 => array ( 'kind' => 'help', 'value' => 'SoftInodeLimit' ),
				7 => array ( 'kind' => 'help', 'value' => 'HardInodeLimit' ),
				8 => array ( 'kind' => 'help', 'value' => 'GraceInodePeriod' ));

		$i=0;
		// loop for every mointpoint with enabled quotas
		while ($this->quota[$i][0]) {
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => $this->quota[$i][0] ),
				1 => array ( 'kind' => 'text', 'text' => $this->quota[$i][1] ),
				2 => array ( 'kind' => 'input', 'name' => $i . '_2', 'type' => 'text', 'size' => '12', 'maxlength' => '20', 'value' => $this->quota[$i][2]),
				3 => array ( 'kind' => 'input', 'name' => $i . '_3', 'type' => 'text', 'size' => '12', 'maxlength' => '20', 'value' => $this->quota[$i][3]),
				4 => array ( 'kind' => 'text', 'text' => $this->quota[$i][4] ),
				5 => array ( 'kind' => 'text', 'text' => $this->quota[$i][5] ),
				6 => array ( 'kind' => 'input', 'name' => $i . '_6', 'type' => 'text', 'size' => '12', 'maxlength' => '20', 'value' => $this->quota[$i][6]),
				7 => array ( 'kind' => 'input', 'name' => $i . '_7', 'type' => 'text', 'size' => '12', 'maxlength' => '20', 'value' => $this->quota[$i][7]),
				8 => array ( 'kind' => 'text', 'text' => $this->quota[$i][8] ));
			$i++;
			}
		return $return;
		}

	/**
	* Returns a list of elements for the account profiles.
	*
	* @return profile elements
	*/
	function get_profileOptions() {
		// Get quotas
		$quotas = lamdaemon(array("+ quota get " . $this->get_scope()));
		$dirs = split(":", $quotas[0]);
		array_pop($dirs); // remove empty element at the end
		for ($i = 0; $i < sizeof($dirs); $i++) {
			$dirs[$i] = split(",", $dirs[$i]);
			$dirs[$i] = $dirs[$i][0];
		}
		$return = array();
		if (sizeof($dirs) < 1) return $return; // stop if no quota directories were found
		$return[] = array (
			0 => array('kind' => 'text', 'text' => '<b>' . _('Mountpoint') . '&nbsp;&nbsp;</b>', 'align' => 'center'),
			1 => array('kind' => 'text', 'text' => '<b>' . _('Soft block limit') . '&nbsp;&nbsp;</b>', 'align' => 'center'),
			2 => array('kind' => 'text', 'text' => '<b>' . _('Hard block limit') . '&nbsp;&nbsp;</b>', 'align' => 'center'),
			3 => array('kind' => 'text', 'text' => '<b>' . _('Soft inode limit') . '&nbsp;&nbsp;</b>', 'align' => 'center'),
			4 => array('kind' => 'text', 'text' => '<b>' . _('Hard inode limit') . '&nbsp;&nbsp;</b>', 'align' => 'center'),
		);
		$return[] = array (
			0 => array('kind' => 'help', 'value' => 'Mountpoint', 'align' => 'center'),
			1 => array('kind' => 'help', 'value' => 'SoftBlockLimit', 'align' => 'center'),
			2 => array('kind' => 'help', 'value' => 'HardBlockLimit', 'align' => 'center'),
			3 => array('kind' => 'help', 'value' => 'SoftInodeLimit', 'align' => 'center'),
			4 => array('kind' => 'help', 'value' => 'HardInodeLimit', 'align' => 'center'),
		);
		for ($i = 0; $i < sizeof($dirs); $i++) {
			$return[] = array(
				0 => array('kind' => 'text', 'text' => $dirs[$i], 'align' => 'left'),
				1 => array('kind' => 'input', 'type' => 'text', 'align' => 'center', 'size' => '12', 'name' => "quota_softblock_" . $dirs[$i]),
				2 => array('kind' => 'input', 'type' => 'text', 'align' => 'center', 'size' => '12', 'name' => "quota_hardblock_" . $dirs[$i]),
				3 => array('kind' => 'input', 'type' => 'text', 'align' => 'center', 'size' => '12', 'name' => "quota_softinode_" . $dirs[$i]),
				4 => array('kind' => 'input', 'type' => 'text', 'align' => 'center', 'size' => '12', 'name' => "quota_hardinode_" . $dirs[$i]),
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
		// Get quotas
		$quotas = lamdaemon(array("+ quota get " . $this->get_scope()));
		$dirs = split(":", $quotas[0]);
		array_pop($dirs); // remove empty element at the end
		for ($i = 0; $i < sizeof($dirs); $i++) {
			$dirs[$i] = split(",", $dirs[$i]);
			$dirs[$i] = $dirs[$i][0];
		}
		for ($i = 0; $i < sizeof($dirs); $i++) {
			if (!get_preg($options["quota_softblock_" . $dirs[$i]][0], 'digit')) $return[] = $this->messages['softblock'][0];
			if (!get_preg($options["quota_hardblock_" . $dirs[$i]][0], 'digit')) $return[] = $this->messages['hardblock'][0];
			if (!get_preg($options["quota_softinode_" . $dirs[$i]][0], 'digit')) $return[] = $this->messages['softinode'][0];
			if (!get_preg($options["quota_hardinode_" . $dirs[$i]][0], 'digit')) $return[] = $this->messages['hardinode'][0];
			if (intval($options["quota_softblock_" . $dirs[$i]][0]) > intval($options["quota_hardblock_" . $dirs[$i]][0])) $return[] = $this->messages['block_cmp'][0];
			if (intval($options["quota_softinode_" . $dirs[$i]][0]) > intval($options["quota_hardinode_" . $dirs[$i]][0])) $return[] = $this->messages['inode_cmp'][0];
			$i++;
		}
		return $return;
	}

	/**
	* Loads the values of an account profile into internal variables.
	*
	* @param array $profile hash array with profile values (identifier => value)
	*/
	function load_profile($profile) {
		for ($i = 0; $i < sizeof($this->quota); $i++) {
			$dir = $this->quota[$i][0];
			if (isset($profile["quota_softblock_" . $dir])) $this->quota[$i][2] = $profile["quota_softblock_" . $dir][0];
			if (isset($profile["quota_hardblock_" . $dir])) $this->quota[$i][3] = $profile["quota_hardblock_" . $dir][0];
			if (isset($profile["quota_softinode_" . $dir])) $this->quota[$i][6] = $profile["quota_softinode_" . $dir][0];
			if (isset($profile["quota_hardinode_" . $dir])) $this->quota[$i][7] = $profile["quota_hardinode_" . $dir][0];
		}
	}

	/**
	* Returns the PDF entries for this module.
	*
	* @return array list of possible PDF entries
	*/
	function get_pdfEntries() {
		if (sizeof($this->quota) > 0) {
			$quotas = array();
			$quotas[] = '<block><tr>' .
					'<td width="20%"><b>' . _('Mountpoint') . '</b></td>' .
					'<td width="20%"><b>' . _('Soft block') . '</b></td>' .
					'<td width="20%"><b>' . _('Hard block') . '</b></td>' .
					'<td width="20%"><b>' . _('Soft inode') . '</b></td>' .
					'<td width="20%"><b>' . _('Hard inode') . '</b></td></tr></block>';
			for ($i = 0; $i < sizeof($this->quota); $i++) {
				$quotas[] = '<block><tr>' .
					'<td width="20%" align=\"L\">' . $this->quota[$i][0] . '</td>' .
					'<td width="20%" align=\"L\">' . $this->quota[$i][2] . '</td>' .
					'<td width="20%" align=\"L\">' . $this->quota[$i][3] . '</td>' .
					'<td width="20%" align=\"L\">' . $this->quota[$i][6] . '</td>' .
					'<td width="20%" align=\"L\">' . $this->quota[$i][7] . '</td></tr></block>';
			}
			return array(
				'quota_quotas' => $quotas);
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
		$return = array();
		// Get quotas
		$quotas = lamdaemon(array("+ quota get " . $this->get_scope()));
		$dirs = split(":", $quotas[0]);
		array_pop($dirs); // remove empty element at the end
		for ($i = 0; $i < sizeof($dirs); $i++) {
			$dirs[$i] = split(",", $dirs[$i]);
			$dirs[$i] = $dirs[$i][0];
		}
		for ($i = 0; $i < sizeof($dirs); $i++) {
			$return[] = array(
				'name' => 'quota_' . $dirs[$i],
				'description' => _('Quota for:') . ' ' . $dirs[$i],
				'help' => 'upload',
				'example' => '2000,2500,3000,3500');
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
			$quotaString = $name . " quota set " . $this->scope . " ";
			for ($m = 0; $m < sizeof($mountPoints); $m++) {
				$partString = $mountPoints[$m] . ',' . implode(',', $temp['accounts'][$name][$mountPoints[$m]]) . ':';
				$quotaString .= $partString;
			}
			$quotaString .= "\n";
			$result = lamdaemon(array($quotaString));
			if (is_array($result)) {
				$errors[] = array('ERROR', implode('<br>', $result), '');
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
