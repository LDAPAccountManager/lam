<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2008  Thomas Manninger

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
* Manages DDNS entries.
*
* @package modules
*
* @author Thomas Manninger
*/

/**
* Manages DDNS entries.
*
* @package modules
*/

class ddns extends baseModule {
	
	// Standard DDNS Settings
	public $ddns = array();
	

	public function get_metaData() {

		$return = array();
		// manages host accounts
		$return["account_types"] = array("dhcp");
		// alias name
		$return["alias"] = _("DDNS");
		// this is a base module
		$return["is_base"] = false;
		// RDN attribute
		$return["RDN"] = array("cn" => "high");
		// LDAP filter
		$return["ldap_filter"] = array();
		// module dependencies
		$return['dependencies'] = array('depends' => array('dhcp_settings'), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array();
		// managed attributes
		$return['attributes'] = array();
		// help Entries
		$return['help'] = array(
				'active' => array(
				"Headline" => _("Activate DynDNS"),
				"Text" => _("Should DDNS (Dynamic DNS) be activated?")
			) , 'fixed_ips' => array(
				"Headline" => _("Fix IP addresses"),
				"Text" => _("Should fix IP addresses be added to the DNS server?")
			) , 'client_insert' => array(
				"Headline" => _("Disable client updates"),
				"Text" => _("Disables the client to update DNS entries.")
			) , 'keypath' => array(
				"Headline" => _("Path to key for DNS updates"),
				"Text" => _("The key enables the DHCP server to perform DNS updates." .
				" " . "The key is generated with \"genDDNSkey\".")
			) , 'dns' => array(
				"Headline" => _("IP address of the DNS server"),
				"Text" => _("Please enter the IP address of your DNS server.")
			) , 'zone' => array(
				"Headline" => _("Zone names"),
				"Text" => _("Zone names for the DNS server (e.g. company.local).")
			) , 'zone_reverse' => array(
				"Headline" => _("Reverse zone names"),
				"Text" => ("Name of the reverse zones of the DNS server (e.g. 0.168.192.in-addr.arpa).")
			) ,);
		return $return;
	}
	
	/**
	* This function fills the message array.
	*/
	public function load_Messages() {
		$this->messages['key_path'][0] = array('ERROR', 'Please enter the path to the key.', '');
		$this->messages['key_path'][1] = array('ERROR', 'The key path contains invalid characters.', '');
		$this->messages['ip'][0] = array('ERROR', 'The IP address of the DNS server is invalid.');
		$this->messages['zone'][0] = array('ERROR', 'Please enter a zone name.');
		$this->messages['zone_reverse'][0] = array('ERROR', 'Please enter the reverse zone.');
	}
	
	/**
	* This functions returns true if all needed settings are done.
	*
	* @return boolean true if LDAP operation can be done
	*/
	public function module_complete() {
		if ($_SESSION['account']->getAccountModule('dhcp_settings')->dn==$_SESSION['config']->get_suffix('dhcp')) {
			/**
			 * Main settings
			 */
			if ($this->attributes['dhcpStatements'][0]=='ddns-update-style interim' && empty($this->attributes['dhcpStatements'][3])) {
				return false;
			}
		}
		else {
			/**
			 * Account settings
			 */
			$ip = array_shift(explode(";",array_pop(explode(". { primary ", $this->attributes['dhcpStatements'][0]))));
			if (!empty($ip) && !check_ip($ip)) return false;
			
			$zone = substr(array_shift(explode(" ",substr($this->attributes['dhcpStatements'][0],5))),0,-1);
			if (empty($zone) && !empty($ip)) return false;
			
			$zone_reverse = substr(array_shift(explode(" ",substr($this->attributes['dhcpStatements'][1],5))),0,-1);
			if (empty($zone_reverse) && !empty($ip)) return false;
		}
		
		return true;
	}
	
	/**
	 * This function check if ddns is enable.
	 */
	private function check_if_ddns_is_enable() {
		$ldap = $_SESSION['ldap']->server();
		$dn = $_SESSION['config']->get_suffix('dhcp');
		
		$search = ldap_search($ldap,$dn,"dhcpStatements=ddns-update-style interim");
		$info = ldap_get_entries($ldap,$search);
		
		if ($info['count'] == 0) {
			return false;
		}
		else {
			return true;
		}
	}

	/* This function returns an array with 4 entries:
	* array( DN1 ('add' => array($attr), 'remove' => array($attr), 'modify' => array($attr), 'lamdaemon' => array(cmds)), DN2 .... )
	* DN is the DN to change. It may be possible to change several DNs,
	* e.g. create a new user and add him to some groups via attribute memberUid
	* add are attributes which have to be added to ldap entry
	* remove are attributes which have to be removed from ldap entry
	* lamdaemon are lamdaemon commands to modify homedir, quotas, ...
	*/
	public function save_attributes() {
		// Get easy attributes
		if ($_SESSION['account']->getAccountModule('dhcp_settings')->dn==$_SESSION['config']->get_suffix('dhcp')) {
			/**
			 * Save main settings
			 */

			$return = $this->getAccountContainer()->save_module_attributes($this->attributes, $this->orig);
		}
		else {
			/**
			 * Save account settings
			 */
			if (!$this->check_if_ddns_is_enable()) {
				return array();
			}

			$return = $this->getAccountContainer()->save_module_attributes($this->attributes, $this->orig);
		}

		// Return attributes
		return $return;
	}
	
	/**
	 * This function loads all needed LDAP attributes.
	 *
	 * @param array $attr list of attributes
	 */
	function load_attributes($attr) {
		parent::load_attributes($attr);

		$this->dn = ereg_replace("'", "", $_GET['DN']);
		$dn = str_replace("'","",$_GET['DN']);
		if ($dn==$_SESSION['config']->get_suffix('dhcp')) {
			
			// main settings
			$this->load_attributes_dhcpSettings($attr);
		}
		else {
			if (!$this->check_if_ddns_is_enable()) {
				return "";
			}
			
			// account edit
			$this->load_attributes_account($attr);
		}
	}
	
	private function load_attributes_account($attr) {
		$attrTmp = $attr;
		
		unset($attr);
		foreach($attrTmp['dhcpStatements'] AS $value) {
			$ex = explode(" ", $value);
			
			if ($ex[0] == 'zone') {
				$attr['dhcpStatements'][] = $value;
			}
		}

		unset($attrTmp);
		
		// Alles, was über dem ArrayIndex 3 liegt, wird gelöscht:
		if (!is_array($attr)) $attr = array();
			
		$this->orig = $attr;
		$this->attributes = $attr;
		
	}

	private function load_attributes_dhcpSettings($attr) {
		$attrTmp = $attr;

		/**
		 * WARNING
		 * By new attributes for ddns edit ne load function of the dhcp settings.
		 */
		
		unset($attr);
		if (!is_array($attrTmp['dhcpStatements'])) $attrTmp['dhcpStatements'] = array();
		foreach($attrTmp['dhcpStatements'] AS $value) {
			$ex = explode(" ", $value);
			
			// ddns active
			if ($ex[0] == 'ddns-update-style') {
				
				if ($ex[1] == 'interim') {
					$attr['dhcpStatements'][0] = "ddns-update-style interim";
				} else {
					$attr['dhcpStatements'][0] = "ddns-update-style none";
				}
			}
			
			// fixed ips into dns
			if ($ex[0] == 'update-static-leases') {
				$attr['dhcpStatements'][1] = "update-static-leases true";
			}
			
			// Client can contribute, which is registered into dns 
			if ($value == 'ignore client-updates') {
				$attr['dhcpStatements'][2] = "ignore client-updates";
			}
			
			// Path to the Key
			if ($ex[0] == 'include') {
				$attr['dhcpStatements'][3] = $value;
			}
		}

		unset($attrTmp);
		
		if (!is_array($attr)) $attr = array();
			
		$this->orig = $attr;
		$this->attributes = $attr;
		

	}
	
	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	public function process_attributes() {
		$errors = array();
		
		// Main Settings and Account have to different processes.
		if ($_SESSION['account']->getAccountModule('dhcp_settings')->dn==$_SESSION['config']->get_suffix('dhcp')) {
			// main settings:
			$errors = $this->process_attributes_mainSettings();
		}
		else {
			// account
			if (!$this->check_if_ddns_is_enable()) {
				return array();
			}
			
			$errors = $this->process_attributes_account();
		}

		return $errors;
	}

	/**
	 * Process for mainsettings
	 */
	public function process_attributes_mainSettings() {
		$errors = array();
		
		// Is DDNS active?
		$active = $_POST['active'];
		
		// Insert fixed IPs into DNS?
		$insert_fixed = $_POST['insert_fixed'];
		
		// Client can contribute, which is registered into dns 
		/* Client kann mitbestimmen, was im DNS eingetragen wird. */
		$client_insert = $_POST['client_insert'];
		
		// The path to the key:
		$key_path = trim($_POST['key_path']);
		
		// Is DDNS active?
		if ($active == 'on') {
			$this->attributes['dhcpStatements'][0] = "ddns-update-style interim";
		}
		else {
			$this->attributes['dhcpStatements'][0] = "ddns-update-style none";
		}
	
		// fixed_ips into dns?
		if ($insert_fixed == 'on') {
			$this->attributes['dhcpStatements'][1] = "update-static-leases true";
		}
		else {
			unset($this->attributes['dhcpStatements'][1]);
		}
		
		// client can contribute?
		if ($client_insert == 'on') {
			$this->attributes['dhcpStatements'][2] = "ignore client-updates";
		}
		else {
			unset($this->attributes['dhcpStatements'][2]);
		}

		// key path must be insert, when ddns is active
		if ($active == 'on' && empty($key_path)) {
			$errors[] = $this->messages['key_path'][0];
			unset($this->attributes['dhcpStatements'][3]);
		}
		elseif (empty($key_path)) {
			unset($this->attributes['dhcpStatements'][3]);
		}
		else {
			if (str_replace("\"","",$_POST['key_path']) != $key_path) {
				$errors[] = $this->messages['key_path'][1];
			}
			$this->attributes['dhcpStatements'][3] = "include \"$key_path\"";
		}
		
		return $errors;
	}
	
	/**
	 * Process for account
	 */
	public function process_attributes_account() {
		$errors = array();
		
		$ip = trim($_POST['ip']);
		$zone = trim($_POST['zone']);
		$zone_reverse = trim($_POST['zone_reverse']);

		// ip correct???
		if (!empty($ip)) {
			if (!check_ip($ip)) {
				$errors[] = $this->messages['ip'][0];
			}
		}
		
		
		// Zone inserted?
		if (!empty($zone)) {
			$this->attributes['dhcpStatements'][0] = "zone {$zone}. { primary {$ip}; key DHCP_UPDATER; }";
		}
		else {
			if (!empty($ip)) {
				$errors[] = $this->messages['zone'][0];
			}
			unset($this->attributes['dhcpStatements'][0]);
		}
		
		// Zone reverse inserted?
		if (!empty($zone_reverse)) {
			$this->attributes['dhcpStatements'][1] = "zone {$zone_reverse}. { primary {$ip}; key DHCP_UPDATER; }";
		}
		else {
			if (!empty($ip)) {
				$errors[] = $this->messages['zone_reverse'][0];
			}
			unset($this->attributes['dhcpStatements'][1]);
		}
		
		
		return $errors;
	}
	
	/* This function will create the page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	public function display_html_attributes() {

		if ($_SESSION['account']->getAccountModule('dhcp_settings')->dn==$_SESSION['config']->get_suffix('dhcp')) {
			/**
			 * DHCP main settings
			 */

			if ($this->attributes['dhcpStatements'][0] == 'ddns-update-style interim') { $checkedStat = true; } else { $checkedStat = false; }
	        $return[] = array(
	        	array('kind' => 'text', 'text' => _('Activate DynDNS') . ":* "),
				array('kind' => 'input', 'type' => 'checkbox', 'name' => 'active',  'checked' => $checkedStat),
				array('kind' => 'help', 'value' => 'active', 'scope' => 'user'));
				
			if (!empty($this->attributes['dhcpStatements'][1])) { $checkedStat = true; } else { $checkedStat = false; }
	        $return[] = array(
	        	array('kind' => 'text', 'text' => _('Add fix IP addresses to DNS') . ":* "),
				array('kind' => 'input', 'type' => 'checkbox', 'name' => 'insert_fixed',  'checked' => $checkedStat),
				array('kind' => 'help', 'value' => 'fixed_ips', 'scope' => 'user'));

			if (!empty($this->attributes['dhcpStatements'][2])) { $checkedStat = true; } else { $checkedStat = false; }
	        $return[] = array(
	        	array('kind' => 'text', 'text' => _('Disable client updates') . ":* "),
				array('kind' => 'input', 'type' => 'checkbox', 'name' => 'client_insert',  'checked' => $checkedStat),
				array('kind' => 'help', 'value' => 'client_insert', 'scope' => 'user'));
				
			$keyPath = str_replace(array("include \"","\""),"",$this->attributes['dhcpStatements'][3]);
	        $return[] = array(
	        	array('kind' => 'text', 'text' => _('Path to key for DNS updates') . ":* "),
				array('kind' => 'input', 'type' => 'text', 'name' => 'key_path', 'value' => $keyPath),
				array('kind' => 'help', 'value' => 'keypath', 'scope' => 'user'));
		}
		else {
			/**
			 * Account Edit.
			 */

			if (!$this->check_if_ddns_is_enable()) {
				echo _("DDNS ist not activated. You can activate it in the DHCP settings (DDNS).") . "<br/><br/>";
			}
			else {		
				$ip = (isset($_POST['ip']))?$_POST['ip']:array_shift(explode(";",array_pop(explode(". { primary ", $this->attributes['dhcpStatements'][0]))));			
				$return[] = array(
					array('kind' => 'text', 'text' => _('IP address of the DNS server') . ":* "),
					array('kind' => 'input', 'type' => 'text', 'name' => 'ip', 'value' => $ip),
					array('kind' => 'help', 'value' => 'dns', 'scope' => 'user'));
				
					
				$zone = (isset($_POST['zone']))?$_POST['zone']:substr(array_shift(explode(" ",substr($this->attributes['dhcpStatements'][0],5))),0,-1);
				$return[] = array(
					array('kind' => 'text', 'text' => _('Zone names') . ":* "),
					array('kind' => 'input', 'type' => 'text', 'name' => 'zone', 'value' => $zone),
					array('kind' => 'help', 'value' => 'zone', 'scope' => 'user'));
					
				$zone_reverse = (isset($_POST['zone_reverse']))?$_POST['zone_reverse']:substr(array_shift(explode(" ",substr($this->attributes['dhcpStatements'][1],5))),0,-1);
		        $return[] = array(
		        	array('kind' => 'text', 'text' => _('Reverse zone names') . ":* "),
					array('kind' => 'input', 'type' => 'text', 'name' => 'zone_reverse', 'value' => $zone_reverse),
					array('kind' => 'help', 'value' => 'zone_reverse', 'scope' => 'user'));
			}
		}
		
		echo _("Attention: The DHCP service needs to be restarted after changes in DDNS.") . "<br />";
		
		return $return;
	}
	
	/**
	* Returns a list of elements for the account profiles.
	*
	* @return profile elements
	*/
	function get_profileOptions() {
		$return = array();

		// Subnetz name
        $return[] = array(
        	array('kind' => 'text', 'text' => _('Subnet') . ": "),
			array('kind' => 'input', 'name' => 'cn', 'type'=>'checkbox'),
			array('kind' => 'help', 'value' => 'type', 'scope' => 'user'));
	}
	
	/**
	* TODO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 
	* Loads the values of an account profile into internal variables.
	*
	* @param array $profile hash array with profile values (identifier => value)
	*/
	function load_profile($profile) {
		// profile mappings in meta data
		parent::load_profile($profile);
		$this->attributes['cn'][0] = $profile['cn'][0];
		
		$this->dhcpSettings['domainname'] = $profile['domainname'][0];
		$this->attributes['dhcpOption'][5] = "domain-name \"". $profile['domainname'][0]."\"";
	}
}
?>