<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2008         Thomas Manninger
                2008 - 2009  Roland Gruber

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
* @author Roland Gruber
*/

/**
* Manages DDNS entries.
*
* @package modules
*/

class ddns extends baseModule {
	
	// Standard DDNS Settings
	public $ddns = array();
	

	/**
	* Initializes the module after it became part of an {@link accountContainer}
	* 
	* Calling this method requires the existence of an enclosing {@link accountContainer}.
	*
	* @param string $base the name of the {@link accountContainer} object ($_SESSION[$base])
	*/
	public function init($base) {
		parent::init($base);
		$this->attributes = &$this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes;
		$this->orig = &$this->getAccountContainer()->getAccountModule('dhcp_settings')->orig;
	}

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
		$return['attributes'] = array('dhcpOption', 'dhcpStatements');
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
		// available PDF fields
		$return['PDF_fields'] = array(
			'DNSserver',
			'zone',
			'reverseZone',
		);
		// upload fields
		if (isset($_SESSION['loggedIn']) && $this->check_if_ddns_is_enable()) {
			$return['upload_columns'] = array(
				array(
					'name' => 'ddns_DNSserver',
					'description' => _('IP address of the DNS server'),
					'help' => 'dns',
					'example' => '123.123.123.123',
					'required' => true
				),
				array(
					'name' => 'ddns_zone',
					'description' => _('Zone names'),
					'help' => 'zone',
					'example' => 'company.local',
					'required' => true
				),
				array(
					'name' => 'ddns_reverseZone',
					'description' => _('Reverse zone names'),
					'help' => 'zone_reverse',
					'example' => '0.168.192.in-addr.arpa',
					'required' => true
				),
			);
		}
		return $return;
	}
	
	/**
	* This function fills the message array.
	*/
	public function load_Messages() {
		$this->messages['key_path'][0] = array('ERROR', 'Please enter the path to the key.', '');
		$this->messages['key_path'][1] = array('ERROR', 'The key path contains invalid characters.', '');
		$this->messages['ip'][0] = array('ERROR', 'The IP address of the DNS server is invalid.');
		$this->messages['ip'][1] = array('ERROR', _('Account %s:') . ' ddns_DNSserver', 'The IP address of the DNS server is invalid.');
		$this->messages['zone'][0] = array('ERROR', 'Please enter a zone name.');
		$this->messages['zone_reverse'][0] = array('ERROR', 'Please enter the reverse zone.');
	}
	
	/**
	* This functions returns true if all needed settings are done.
	*
	* @return boolean true if LDAP operation can be done
	*/
	public function module_complete() {
		if ($this->getAccountContainer()->dn_orig==$_SESSION['config']->get_suffix('dhcp')) {
			// check if DHCP main settings and valid DHCP entry
			if (!in_array_ignore_case('dhcpServer', $this->attributes['objectClass'])) {
				return false;
			}
			//Main settings
			if ($this->isDynDNSActivated() && (($this->getUpdateKey() == null) || ($this->getUpdateKey() == ''))) {
				return false;
			}
		}
		else {
			if (!$this->check_if_ddns_is_enable()) {
				return true;
			}
			// Account settings
			$ip = $this->getDNSServer();
			if (!empty($ip) && !check_ip($ip)) return false;
			
			$zones = $this->getZoneNames();
			if (sizeof($zones) < 2) {
				return false;
			}
		}
		
		return true;
	}
	
	/**
	 * This function check if ddns is enable.
	 */
	private function check_if_ddns_is_enable() {
		$ldap = $_SESSION['ldap']->server();
		$dn = $_SESSION['config']->get_suffix('dhcp');
		
		$search = @ldap_search($ldap,$dn,"dhcpStatements=ddns-update-style interim", array(), 0, 0, 0, LDAP_DEREF_NEVER);
		if ($search) {
			$info = @ldap_get_entries($ldap,$search);
			if ($info && ($info['count'] > 0)) {
				return true;
			}
			else {
				return false;
			}
		}
		return false;
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
		if (!in_array_ignore_case('dhcpServer', $this->attributes['objectClass'])) {
			return array();
		}
		$errors = array();
		
		// Is DDNS active?
		$active = $_POST['active'];
		
		// Insert fixed IPs into DNS?
		$insert_fixed = $_POST['insert_fixed'];
		
		// Client can contribute which is registered into DNS 
		$client_insert = $_POST['client_insert'];
		
		// The path to the key:
		$key_path = trim($_POST['key_path']);
		
		$this->setDynDNSActivated(($active == 'on'));

		$this->setFixIPs(($insert_fixed == 'on'));

		$this->setIgnoreClientUpdates(($client_insert == 'on'));
		
		$this->setUpdateKey($key_path);
		// key path must be insert, when ddns is active
		if ($active == 'on' && empty($key_path)) {
			$errors[] = $this->messages['key_path'][0];
		}
		elseif (!empty($key_path)) {
			if (str_replace("\"","",$_POST['key_path']) != $key_path) {
				$errors[] = $this->messages['key_path'][1];
			}
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
		
		for ($i = 0; $i < sizeof($this->attributes['dhcpStatements']); $i++) {
			if (substr($this->attributes['dhcpStatements'][$i], 0, 5) == 'zone ') {
				unset($this->attributes['dhcpStatements'][$i]);
				$this->attributes['dhcpStatements'] = array_values($this->attributes['dhcpStatements']);
				$i--;
			}
		}
		
		// Zone inserted?
		if (!empty($zone)) {
			$this->attributes['dhcpStatements'][] = "zone {$zone}. { primary {$ip}; key DHCP_UPDATER; }";
		}
		else {
			if (!empty($ip)) {
				$errors[] = $this->messages['zone'][0];
			}
		}
		
		// Zone reverse inserted?
		if (!empty($zone_reverse)) {
			$this->attributes['dhcpStatements'][] = "zone {$zone_reverse}. { primary {$ip}; key DHCP_UPDATER; }";
		}
		else {
			if (!empty($ip)) {
				$errors[] = $this->messages['zone_reverse'][0];
			}
		}
		return $errors;
	}
	
	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return array HTML meta data
	 */
	public function display_html_attributes() {
		// check if DHCP main settings and valid DHCP entry
		if ($_SESSION['config']->get_suffix('dhcp') == $this->getAccountContainer()->dn_orig) {
			if (!in_array_ignore_case('dhcpServer', $this->attributes['objectClass'])) {
				StatusMessage("ERROR", _('Please set your LDAP suffix to an LDAP entry with object class "dhcpServer".'));
				return array();
			}
		}
		if ($_SESSION['account']->getAccountModule('dhcp_settings')->dn==$_SESSION['config']->get_suffix('dhcp')) {
			// DHCP main settings
	        $return[] = array(
	        	array('kind' => 'text', 'text' => _('Activate DynDNS') . ":* "),
				array('kind' => 'input', 'type' => 'checkbox', 'name' => 'active',  'checked' => $this->isDynDNSActivated()),
				array('kind' => 'help', 'value' => 'active', 'scope' => 'user'));
				
	        $return[] = array(
	        	array('kind' => 'text', 'text' => _('Add fix IP addresses to DNS') . ":* "),
				array('kind' => 'input', 'type' => 'checkbox', 'name' => 'insert_fixed',  'checked' => $this->addFixIPs()),
				array('kind' => 'help', 'value' => 'fixed_ips', 'scope' => 'user'));

	        $return[] = array(
	        	array('kind' => 'text', 'text' => _('Disable client updates') . ":* "),
				array('kind' => 'input', 'type' => 'checkbox', 'name' => 'client_insert',  'checked' => $this->isIgnoreClientUpdates()),
				array('kind' => 'help', 'value' => 'client_insert', 'scope' => 'user'));
		
	        $return[] = array(
	        	array('kind' => 'text', 'text' => _('Path to key for DNS updates') . ":* "),
				array('kind' => 'input', 'type' => 'text', 'name' => 'key_path', 'value' => $this->getUpdateKey()),
				array('kind' => 'help', 'value' => 'keypath', 'scope' => 'user'));
		}
		else {
			// Account edit 
			if (!$this->check_if_ddns_is_enable()) {
				echo _("DDNS ist not activated. You can activate it in the DHCP settings (DDNS).") . "<br/><br/>";
			}
			else {		
				$return[] = array(
					array('kind' => 'text', 'text' => _('IP address of the DNS server') . ":* "),
					array('kind' => 'input', 'type' => 'text', 'name' => 'ip', 'value' => $this->getDNSServer()),
					array('kind' => 'help', 'value' => 'dns', 'scope' => 'user'));
				
					
				$zones = $this->getZoneNames();
				$zone = '';
				$revzone = '';
				if (isset($zones[0])) {
					$zone = $zones[0];
				}
				if (isset($zones[1])) {
					$revzone = $zones[1];
				}
				$return[] = array(
					array('kind' => 'text', 'text' => _('Zone names') . ":* "),
					array('kind' => 'input', 'type' => 'text', 'name' => 'zone', 'value' => $zone),
					array('kind' => 'help', 'value' => 'zone', 'scope' => 'user'));
					
		        $return[] = array(
		        	array('kind' => 'text', 'text' => _('Reverse zone names') . ":* "),
					array('kind' => 'input', 'type' => 'text', 'name' => 'zone_reverse', 'value' => $revzone),
					array('kind' => 'help', 'value' => 'zone_reverse', 'scope' => 'user'));
			}
		}
		
		echo _("Attention: The DHCP service needs to be restarted after changes in DDNS.") . "<br />";
		
		return $return;
	}
	
	/**
	* Returns the PDF entries for this module.
	*
	* @return array list of possible PDF entries
	*/
	public function get_pdfEntries() {
		$zones = $this->getZoneNames();
		$zone = '';
		$revzone = '';
		if (isset($zones[0])) {
			$zone = $zones[0];
		}
		if (isset($zones[1])) {
			$revzone = $zones[1];
		}
		return array(
			get_class($this) . '_DNSserver' => array('<block><key>' . _('IP address of the DNS server') . '</key><value>' . $this->getDNSServer() . '</value></block>'),
			get_class($this) . '_zone' => array('<block><key>' . _('Zone names') . '</key><value>' . $zone . '</value></block>'),
			get_class($this) . '_reverseZone' => array('<block><key>' . _('Reverse zone names') . '</key><value>' . $revzone . '</value></block>'),
		);
	}
	
	/**
	 * Returns the IP of the DNS server.
	 *
	 * @return String IP address
	 */
	private function getDNSServer() {
		$return = null;
		if (is_array($this->attributes['dhcpStatements'])) {
			for ($i = 0; $i < sizeof($this->attributes['dhcpStatements']); $i++) {
				if (substr($this->attributes['dhcpStatements'][$i], 0, 5) == 'zone ') {
					return array_shift(explode(";",array_pop(explode(". { primary ", $this->attributes['dhcpStatements'][$i]))));
				}
			}
		}
		return $return;
	}
	
	/**
	 * Returns the zone names.
	 *
	 * @return array zone names
	 */
	private function getZoneNames() {
		$return = array();
		if (is_array($this->attributes['dhcpStatements'])) {
			for ($i = 0; $i < sizeof($this->attributes['dhcpStatements']); $i++) {
				if (substr($this->attributes['dhcpStatements'][$i], 0, 5) == 'zone ') {
					$return[] = substr(array_shift(explode(" ",substr($this->attributes['dhcpStatements'][$i],5))),0,-1);
				}
			}
		}
		return $return;
	}
	
	/**
	 * Returns if DDNS is activated.
	 * 
	 * @return boolean activated
	 */
	private function isDynDNSActivated() {
		$return = false;
		if (is_array($this->attributes['dhcpStatements'])) {
			for ($i = 0; $i < sizeof($this->attributes['dhcpStatements']); $i++) {
				if ($this->attributes['dhcpStatements'][$i] == 'ddns-update-style interim') {
					$return = true;
					break;
				}
			}
		}
		return $return;
	}
	
	/**
	 * Sets if DDNS is activated.
	 * 
	 * $activated boolean activated
	 */
	private function setDynDNSActivated($activated) {
		if (is_array($this->attributes['dhcpStatements'])) {
			for ($i = 0; $i < sizeof($this->attributes['dhcpStatements']); $i++) {
				if (substr($this->attributes['dhcpStatements'][$i], 0, 18) == 'ddns-update-style ') {
					unset($this->attributes['dhcpStatements'][$i]);
					$this->attributes['dhcpStatements'] = array_values($this->attributes['dhcpStatements']);
				}
			}
		}
		if ($activated) {
			$this->attributes['dhcpStatements'][] = 'ddns-update-style interim';
		}
		else {
			$this->attributes['dhcpStatements'][] = 'ddns-update-style none';
		}
	}
	
	/**
	 * Returns if fixed IPs are added to DDNS.
	 * 
	 * @return boolean add fixed IPs
	 */
	private function addFixIPs() {
		$return = false;
		if (is_array($this->attributes['dhcpStatements'])) {
			for ($i = 0; $i < sizeof($this->attributes['dhcpStatements']); $i++) {
				if ($this->attributes['dhcpStatements'][$i] == 'update-static-leases true') {
					$return = true;
					break;
				}
			}
		}
		return $return;
	}
	
	/**
	 * Sets if client updates are ignored.
	 * 
	 * $add boolean add fixed IPs
	 */
	private function setFixIPs($add) {
		if (is_array($this->attributes['dhcpStatements'])) {
			for ($i = 0; $i < sizeof($this->attributes['dhcpStatements']); $i++) {
				if (substr($this->attributes['dhcpStatements'][$i], 0, 21) == 'update-static-leases ') {
					unset($this->attributes['dhcpStatements'][$i]);
					$this->attributes['dhcpStatements'] = array_values($this->attributes['dhcpStatements']);
				}
			}
		}
		if ($add) {
			$this->attributes['dhcpStatements'][] = 'update-static-leases true';
		}
	}
	
	/**
	 * Returns if client updates are ignored.
	 * 
	 * @return boolean ignore client updates
	 */
	private function isIgnoreClientUpdates() {
		$return = false;
		if (is_array($this->attributes['dhcpStatements'])) {
			for ($i = 0; $i < sizeof($this->attributes['dhcpStatements']); $i++) {
				if ($this->attributes['dhcpStatements'][$i] == 'ignore client-updates') {
					$return = true;
					break;
				}
			}
		}
		return $return;
	}
	
	/**
	 * Sets if client updates are ignored.
	 * 
	 * $ignore boolean ignore client updates
	 */
	private function setIgnoreClientUpdates($ignore) {
		if (is_array($this->attributes['dhcpStatements'])) {
			for ($i = 0; $i < sizeof($this->attributes['dhcpStatements']); $i++) {
				if ($this->attributes['dhcpStatements'][$i] == 'ignore client-updates') {
					unset($this->attributes['dhcpStatements'][$i]);
					$this->attributes['dhcpStatements'] = array_values($this->attributes['dhcpStatements']);
				}
			}
		}
		if ($ignore) {
			$this->attributes['dhcpStatements'][] = 'ignore client-updates';
		}
	}
	
	/**
	 * Returns the key for DNS updates.
	 * 
	 * @return String key
	 */
	private function getUpdateKey() {
		$return = null;
		if (is_array($this->attributes['dhcpStatements'])) {
			for ($i = 0; $i < sizeof($this->attributes['dhcpStatements']); $i++) {
				if (substr($this->attributes['dhcpStatements'][$i], 0, 8) == 'include ') {
					$return = substr($this->attributes['dhcpStatements'][$i],9, strlen($this->attributes['dhcpStatements'][$i]) - 10);
					break;
				}
			}
		}
		return $return;
	}
	
	/**
	 * Sets the key for DNS updates.
	 * 
	 * $key String key
	 */
	private function setUpdateKey($key) {
		if (!is_array($this->attributes['dhcpStatements'])) {
			$this->attributes['dhcpStatements'] = array();
		}
		for ($i = 0; $i < sizeof($this->attributes['dhcpStatements']); $i++) {
			if (substr($this->attributes['dhcpStatements'][$i], 0, 8) == 'include ') {
				unset($this->attributes['dhcpStatements'][$i]);
				$this->attributes['dhcpStatements'] = array_values($this->attributes['dhcpStatements']);
			}
		}
		if (($key != null) && ($key != '')) {
			$this->attributes['dhcpStatements'][] = 'include "' . $key . '"';
		}
	}

	/**
	* This function loads the LDAP attributes when an account should be loaded.
	* 
	* Calling this method requires the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	* By default this method loads the object classes and accounts which are specified in {@link getManagedObjectClasses()}
	* and {@link getManagedAttributes()}.
	*
	* @param array $attributes array like the array returned by get_ldap_attributes(dn of account) but without count indices
	*/
	public function load_attributes($attributes) {
		// load nothing, attributes are saved in "dhcp_settings" module
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
		if (!$this->check_if_ddns_is_enable()) {
			return $messages;
		}
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// principal name
			if (!check_ip($rawAccounts[$i][$ids['ddns_DNSserver']])) {
				$error = $this->messages['ip'][1];
				array_push($error, $i);
				$messages[] = $error;
			}
			else {
				$partialAccounts[$i]['dhcpStatements'][] = "zone {$rawAccounts[$i][$ids['ddns_zone']]}. { primary {$rawAccounts[$i][$ids['ddns_DNSserver']]}; key DHCP_UPDATER; }";
				$partialAccounts[$i]['dhcpStatements'][] = "zone {$rawAccounts[$i][$ids['ddns_reverseZone']]}. { primary {$rawAccounts[$i][$ids['ddns_DNSserver']]}; key DHCP_UPDATER; }";
			}
		}
		return $messages;
	}
	
}

?>
