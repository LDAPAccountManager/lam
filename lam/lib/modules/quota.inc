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


class quota extends baseModule {

	/**
	* Creates a new quota object.
	*/
	function quota($scope) {
	// error messages for input checks
	$this->messages['softblock'] = array('ERROR', _('Block soft quota'), _('Block soft quota contains invalid characters. Only natural numbers are allowed'));
	$this->messages['hardblock'] = array('ERROR', _('Block hard quota'), _('Block hard quota contains invalid characters. Only natural numbers are allowed'));
	$this->messages['softinode'] = array('ERROR', _('Inode soft quota'), _('Inode soft quota contains invalid characters. Only natural numbers are allowed'));
	$this->messages['hardinode'] = array('ERROR', _('Inode hard quota'), _('Inode hard quota contains invalid characters. Only natural numbers are allowed'));
	$this->messages['block_cmp'] = array('ERROR', _('Block quota'), _('Block soft quota must be smaller than block hard quota'));
	$this->messages['inode_cmp'] = array('ERROR', _('Inode quota'), _('Inode soft quota must be smaller than inode hard quota'));
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
		$return['PDF_fields'] =  array(	'quotas');
		return $return;
	}

	// Constructor
	function init($base) {
		// call parent init
		parent::init($base);
		// Get basic quotas for new account
		$output_array = $_SESSION[$this->base]->lamdaemon(array("+ quota get " . $_SESSION[$this->base]->type));
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

	/** regular expression for quota values */
	var $regex_quota = '^[0-9]*$';

	/** list of possible error messages */
	var $messages = array();


	function module_ready() {
		if (!isset($_SESSION['config']->scriptPath)) return $false;
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
	function get_help($id,$scope) {
		switch ($id) {
			case "description":
				return array ("ext" => "FALSE", "Headline" => _("Description"),
					"Text" => _("Host Description."));
				break;
			}
		return false;
		}

	/* This function returns all ldap attributes
	* which are part of quota and returns
	* also their values.
	*/
	function get_attributes() {
		return $this->quota;
		}

	/* This function loads all attributes into the object
	* $attr is an array as it's retured from ldap_get_attributes
	*/
	function load_attributes($attr) {
		// Load name
		if ($_SESSION[$this->base]->type=='user') $id = $attr['uid'][0];
		if ($_SESSION[$this->base]->type=='group') $id = $attr['cn'][0];
		// Get quotas
		$output_array = $_SESSION[$this->base]->lamdaemon(array("$id quota get " . $_SESSION[$this->base]->type));
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

	function delete_attributes($post) {
		$i=0;
		while ($this->quota[$i][0]) {
			$quotastring = $quotastring . $this->quota[$i][0] . ',0,0,0,0:';
			$i++;
			}
		if ($_SESSION[$this->base]->type=='user') $id = $_SESSION[$this->base]->module['posixAccount']->attributes['uid'][0];
		if ($_SESSION[$this->base]->type=='group') $id = $_SESSION[$this->base]->module['posixGroup']->attributes['cn'][0];
		$return[$_SESSION[$this->base]->dn_orig]['lamdaemon']['command'][] = $id . " quota set " . $_SESSION[$this->base]->type . " $quotastring\n";
		return $return;
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_attributes($post, $profile=false) {
		// Write all general values into $account_new
		$i=0;
		// loop for every mointpoint with quotas
		while ($this->quota[$i][0]) {
			$this->quota[$i][2] = $post[$i . '_2'];
			$this->quota[$i][3] = $post[$i . '_3'];
			$this->quota[$i][6] = $post[$i . '_6'];
			$this->quota[$i][7] = $post[$i . '_7'];
			// Check if values are OK and set automatic values. if not error-variable will be set
			if (!ereg($this->regex_quota, $this->quota[$i][2]))
				$errors[$this->quota[$i][2]][] = $this->messages['softblock'];
			if (!ereg($this->regex_quota, $this->quota[$i][3]))
				$errors[$this->quota[$i][3]][] = $this->messages['hardblock'];
			if (!ereg($this->regex_quota, $this->quota[$i][6]))
				$errors[$this->quota[$i][6]][] = $this->messages['softinode'];
			if (!ereg($this->regex_quota, $this->quota[$i][7]))
				$errors[$this->quota[$i][7]][] = $this->messages['hardinode'];
			if (intval($this->quota[$i][2]) > intval($this->quota[$i][3]))
				$errors[$this->quota[$i][2]][] = $this->messages['block_cmp'];
			if (intval($this->quota[$i][6]) > intval($this->quota[$i][7]))
				$errors[$this->quota[$i][6]][] = $this->messages['inode_cmp'];
			$i++;
			}

		// Return error-messages
		if (is_array($errors)) return $errors;
		// Go to additional group page when no error did ocour and button was pressed
		return 0;
		}

	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_attributes($post, $profile=false) {

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

	function display_html_delete($post) {
		return 0;
		}

	/**
	* Returns a list of elements for the account profiles.
	*
	* @return profile elements
	*/
	function get_profileOptions() {
		// Get quotas
		$quotas = $_SESSION[$this->base]->lamdaemon(array("+ quota get " . $this->get_scope()));
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
			0 => array('kind' => 'help', 'value' => 'TODO', 'align' => 'center'),
			1 => array('kind' => 'help', 'value' => 'TODO', 'align' => 'center'),
			2 => array('kind' => 'help', 'value' => 'TODO', 'align' => 'center'),
			3 => array('kind' => 'help', 'value' => 'TODO', 'align' => 'center'),
			4 => array('kind' => 'help', 'value' => 'TODO', 'align' => 'center'),
		);
		for ($i = 0; $i < sizeof($dirs); $i++) {
			$return[] = array(
				0 => array('kind' => 'text', 'text' => $dirs[$i], 'align' => 'left'),
				1 => array('kind' => 'input', 'type' => 'text', 'text' => "", 'align' => 'center', 'size' => '12', 'name' => "quota_softblock_$i"),
				2 => array('kind' => 'input', 'type' => 'text', 'text' => "", 'align' => 'center', 'size' => '12', 'name' => "quota_hardblock_$i"),
				3 => array('kind' => 'input', 'type' => 'text', 'text' => "", 'align' => 'center', 'size' => '12', 'name' => "quota_softinode_$i"),
				4 => array('kind' => 'input', 'type' => 'text', 'text' => "", 'align' => 'center', 'size' => '12', 'name' => "quota_hardinode_$i"),
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
		$i = 0;
		while (isset($options["quota_softblock_$i"])) {
			if (!ereg($this->regex_quota, $options["quota_softblock_$i"][0])) $return[] = $this->messages['softblock'];
			if (!ereg($this->regex_quota, $options["quota_hardblock_$i"][0])) $return[] = $this->messages['hardblock'];
			if (!ereg($this->regex_quota, $options["quota_softinode_$i"][0])) $return[] = $this->messages['softinode'];
			if (!ereg($this->regex_quota, $options["quota_hardinode_$i"][0])) $return[] = $this->messages['hardinode'];
			if (intval($options["quota_softblock_$i"][0]) > $options["quota_hardblock_$i"][0]) $return[] = $this->messages['block_cmp'];
			if (intval($options["quota_softinode_$i"][0]) > $options["quota_hardinode_$i"][0]) $return[] = $this->messages['inode_cmp'];
			$i++;
		}
		return $return;
	}
	
	/*
	 * (non-PHPDoc)
	 * @see baseModule#get_pdfEntries
	 */
	function get_pdfEntries($account_type = "user") {
		return array(	'quota_quotas' => array('<block><key>' . _('User quota') . '</key><td width="20%"><b>' . _('Mountpoint') . '</b></td><td width="20%"><b>' . _('Soft block') . '</b></td><td width="20%"><b>' . _('Soft inode') . '</b></td><td width="20%"><b>' . _('Hard block') . '</b></td><td width="20%"><b>' . _('Hard inode') . '</b></td></block>'));
	}

}

?>
