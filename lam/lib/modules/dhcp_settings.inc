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
* Manages DHCP entries.
*
* @package modules
*
* @author Thomas Manninger
*/

/**
* Manages DHCP entries.
*
* @package modules
*/

/**
 * This function checks if the given IP address ist valid.
 *
 * @param string IP to check
 * @param check Subnet
 * @return boolean true or false
 **/
if (!function_exists('check_ip')) {
	function check_ip($ip,$subnet=false) {
		$part = split("[.]", $ip);
		// Wenn... Keine 4 Segmente gefunden wurde
		if (count($part) != 4) {
			return false;
		}
		else {
			// check each segment
			for ($i = 0; $i < count($part); $i++) {
				// only numbers are allowed
				if (!is_numeric($part[$i])) {
					return false;
				}
				else {
					// if not check Subnet, than segment must be smaller than 256, else smaller than 255
					if($part[$i] > 255 || ($i==3 && (!$subnet && $part[$i]<1)) || ($i==3 && (!$subnet && $part[$i]<0))) {
						return false;
					}
				}
			}
		}
		return true;
	}
}

class dhcp_settings extends baseModule {
	
	// Netbios node type:
	public $netbios_node_type;
	
	// All netbios node types:
	private $all_netbios_node_types;
	
	// ALL DHCP Settings
	public $dhcpSettings;
	
	// All DHCP Statements
	public $dhcpStatements;
	
	// DN (cn=192.168.15.0,dc=entwicklung,dc=sin)
	public $dn="";
	
	public $attributes;
	
	/**
	* Creates a new dhcp_settings object.
	*
	* @param string $scope account type
	*/
	function __construct($scope) {
		// List of well known rids
		$this->all_netbios_node_types = array(
			"1" => _("B-Node (0x01)"),
			"2" => _("P-Node (0x02)"),
			"4" => _("M-Node (0x04)"),
			"8" => _("H-Node (0x08)")
		);
		// call parent constructor
		parent::__construct($scope);
	}

	public function get_metaData() {

		$return = array();
		// manages host accounts
		$return["account_types"] = array("dhcp");
		// alias name
		$return["alias"] = _("DHCP settings");
		// this is a base module
		$return["is_base"] = true;
		// RDN attribute
		$return["RDN"] = array("cn" => "high");
		// LDAP filter
		$return["ldap_filter"] = array();
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array();
		// managed attributes
		$return['attributes'] = array('cn','dhcpOtions');
		// help Entries
		$return['help'] = array(
				'domainname' => array(
				"Headline" => _("Domain name"),
				"Text" => _("The domain name of the subnet.")
			) , 'subnet' => array(
				"Headline" => _("Subnet"),
				"Text" => _("The name of the subnet. Example: 192.168.10.0")
			) , 'leasetime' => array(
				"Headline" => _("Lease time"),
				"Text" => _("The lease time specifies after how many seconds the client should request a new IP address.")
			) , 'max_leasetime' => array(
				"Headline" => _("Maximum lease time"),
				"Text" => _("The maximum lease time specifies after how many seconds the client must request a new IP address.")
			) , 'dns' => array(
				"Headline" => _("DNS (Domain Name System)"),
				"Text" => _("The IP address(es) of the DNS servers. Multiple addresses are separated by \",\". Example: 192.168.0.10, 192.168.0.11")
			) , 'gateway' => array(
				"Headline" => _("Default gateway"),
				"Text" => _("Packages are sent to the default gateway if the receiver does not reside in the same network. The default gateway routes them to the target network.")
			) , 'netbios' => array(
				"Headline" => _("Netbios name server"),
				"Text" => _("The IP address of the Netbios name server.")
			) , 'netbios_type' => array(
				"Headline" => _("Netbios node type"),
				"Text" => ("<b>B-Node (0x01): Broadcast.</b><br/>The client tries to find other workstations via broadcasting
(works only inside the same collision domain, viz. the same subnet).<br/><br/>

<b>P-Node (0x02): Point-To-Point</b><br />
The client contacts a Netbios name server (NBNS) from Microsoft Windows Name Service (WINS) for name resolution.<br/><br/>

<b>M-Node (0x04): Mixed</b><br />
The node tries broadcasting first. If that fails then it tries WINS.<br/><br/>

<b>H-Node (0x08): Hybrid</b><br />
The node tries WINS first. If that fails it tries broadcasting.<br/><br/>

By default, the nodes are configured as H-Nodes which fits for small networks. In large networks Point-to-Point (0x02) should be used.")
			) , 'subnetmask' => array(
				"Headline" => _("Subnet mask"),
				"Text" => _("The subnet mask of the network.")
			) , 'netmask' => array(
				"Headline" => _("Net mask"),
				"Text" => _("The net mask is derived from the subnet mask. If you leave this field empty then LAM will calculate it for you.")
			) );
		return $return;
	}
	
	/**
	* This function fills the message array.
	*/
	public function load_Messages() {
		$this->messages['cn'][0] = array('ERROR', _('No subnet entered.'));
		$this->messages['cn'][1] = array('ERROR', _('The subnet is already in use.'));
		$this->messages['cn'][2] = array('ERROR', _('The subnet is invalid.'));
		$this->messages['dns'][0] = array('ERROR', _('You entered one or more invalid DNS servers.'));
		$this->messages['lease_time'][0] = array('ERROR', _('The lease time is invalid.'));
		$this->messages['routers'][0] = array('ERROR', _('The default gateway is invalid.'));
		$this->messages['netbios'][0] = array('ERROR', _('The Netbios server is invalid.'));
		$this->messages['netbios_node_type'][0] = array('ERROR', _('The entered Netbios node type does not exist.'));
		$this->messages['max_lease_time'][0] = array('ERROR', _('The maximum lease time is invalid.'));
		$this->messages['subnet'][0] = array('ERROR', _('The subnet mask is invalid.'));
		$this->messages['netmask'][0] = array('INFO', _('The netmask was set.'));
		$this->messages['netmask'][1] = array('INFO', _('The net mask was invalid and was corrected.'));
		$this->messages['ranges_reload'][0] = array('INFO', _('The DHCP ranges were changed to fit for the new subnet.'));
		$this->messages['ips_reload'][0] = array('INFO', 'The fixed IP addresses were changed to fit for the new subnet.');
		$this->messages['domainname'][0] = array('ERROR', _('The domain name needs to be shorter than 15 characters.'));
		$this->messages['domainname'][1] = array('ERROR', _('The domain name needs to have at least 3 characters.'));
		$this->messages['domainname'][2] = array('ERROR', _('The domain name includes invalid characters. Valid characters are A-Z, a-z, 0-9, ".", "_","-".'));
	}
	
	/**
	* This functions returns true if all needed settings are done.
	*
	* @return boolean true if LDAP operation can be done
	*/
	public function module_complete() {
		
		if ($_SESSION['account']->getAccountModule('dhcp_settings')->dn!=$_SESSION['config']->get_suffix('dhcp')) {
			if (!check_ip($this->attributes['cn'][0],true)) return false;
        	if (array_pop(explode(".",  $this->attributes['cn'][0]))!=0) return false;
			if ($_SESSION['cache']->in_cache($this->attributes['cn'][0],'cn', array('dhcp')) && $this->orig['cn']['0']!=$this->attributes['cn'][0]) return false;
			if (!check_ip($this->subnet, true)) return false;
		}

		if (!empty($this->dhcpSettings['domainname']) && (strlen($this->dhcpSettings['domainname'])>15 ||
		    strlen($this->dhcpSettings['domainname'])<3 || !eregi("^[A-Za-z0-9\._-]*$", $this->dhcpSettings['domainname']))) return false;
		//DNS
		if (!empty($this->dhcpSettings['dns'])) {
			$ex = explode(",", $this->dhcpSettings['dns']);
			$dns = "";
			$is_first=true;
			$invalid = false;
			foreach($ex AS $string) {
				if ($is_first) {
					$dns .= $string;
					$is_first=false;
			    }
				else
				{
			    	$dns .= ",$string";
				}
	            if (!check_ip($string)) {
			        $invalid = true;
			    }
			}
			if ($invalid) {
				return false;
			}
		}
		if (!empty($this->dhcpStatements['lease_time']) && !is_numeric($this->dhcpStatements['lease_time'])) return false;
		if (!empty($this->dhcpStatements['max_lease_time']) && !is_numeric($this->dhcpStatements['max_lease_time'])) return false;
		if (!empty($this->dhcpSettings['routers']) && !check_ip($this->dhcpSettings['routers'])) return false;
		if (!empty($this->dhcpSettings['netbios']) && !check_ip($this->dhcpSettings['netbios'])) return false;
		
		return true;
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
		$this->attributes['objectClass'][0] = "top";
		$this->attributes['objectClass'][1] = "dhcpOptions";

		if ($_SESSION['account']->getAccountModule('dhcp_settings')->dn!=$_SESSION['config']->get_suffix('dhcp')) {
			// Standard Things
			$this->attributes['objectClass'][2] = "dhcpSubnet";
			// sort to array:
			if (is_array($this->attributes['dhcpOption'])) {
				$i = 0;
				foreach($this->attributes['dhcpOption'] AS $key=>$value) {
				    $this->attributestmp['dhcpOption'][$i] = $this->attributes['dhcpOption'][$key];
				    $i++;
				}
				unset($this->attributes['dhcpOption']);
				$this->attributes['dhcpOption'] = $this->attributestmp['dhcpOption'];
				unset($this->attributestmp['dhcpOption']);
			}
			if (is_array($this->attributes['dhcpStatements'])) {
				$i = 0;
				foreach($this->attributes['dhcpStatements'] AS $key=>$value) {
				    $this->attributestmp['dhcpStatements'][$i] = $this->attributes['dhcpStatements'][$key];
				    $i++;
				}
				unset($this->attributes['dhcpStatements']);
				$this->attributes['dhcpStatements'] = $this->attributestmp['dhcpStatements'];
				unset($this->attributestmp['dhcpStatements']);
			}
			$return = $this->getAccountContainer()->save_module_attributes($this->attributes, $this->orig);
		}
		else
		{
			// Basicsettings...
			
			if (!isset($this->getAccountContainer()->dn_already_edit)) {
			    $a = explode(",", $this->getAccountContainer()->dn);
				unset($a[0]);
				//$this->getAccountContainer()->dn = implode(",", $a);
				$this->getAccountContainer()->dn_already_edit = true;
			}
			
			if (is_array($this->attributes['dhcpOption'])) {
				$i = 0;
				foreach($this->attributes['dhcpOption'] AS $key=>$value) {
				    $this->attributestmp['dhcpOption'][$i] = $this->attributes['dhcpOption'][$key];
				    $i++;
				}
				unset($this->attributes['dhcpOption']);
				$this->attributes['dhcpOption'] = $this->attributestmp['dhcpOption'];
				unset($this->attributestmp['dhcpOption']);
			}
			if (is_array($this->attributes['dhcpStatements'])) {
				$i = 0;
				foreach($this->attributes['dhcpStatements'] AS $key=>$value) {
				    $this->attributestmp['dhcpStatements'][$i] = $this->attributes['dhcpStatements'][$key];
				    $i++;
				}
				unset($this->attributes['dhcpStatements']);
				$this->attributes['dhcpStatements'] = $this->attributestmp['dhcpStatements'];
				unset($this->attributestmp['dhcpStatements']);
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
		// Load DHCP Options:
		$this->dn = ereg_replace("'", "", $_GET['DN']);

        if (!is_array($attr['dhcpOption'])) {
        	$attr['dhcpOption'] = array();
        }
		
        // Alles aus den DDNS Bereich löschen:
        /*if (!is_array($attr['dhcpStatements'])) {
        	$attr['dhcpStatements'] = array();
        }
        
     	foreach($attr['dhcpStatements'] AS $index=>$value) {
        	$ex = explode(" ", $value);
        	
        	if ($ex[0]=="ignore") {
                unset($attr['dhcpStatements'][$index]);
			}
        	if ($ex[0]=="include") {
                unset($attr['dhcpStatements'][$index]);
			}
			if ($ex[0]=="ddns-update-style") {
                unset($attr['dhcpStatements'][$index]);
			}
        	if ($ex[0]=="update-static-leases") {
                unset($attr['dhcpStatements'][$index]);
			}
     	}
     	*/
     	if (count($attr['dhcpStatements']) == 0) {
     		unset($attr['dhcpStatements']);
     	}
     	
     	if (is_array($attr['dhcpStatements'])) {
	 		foreach($attr['dhcpStatements'] AS $id=>$value) {
			    $ex = explode(" ", $value);
			    // Is default lease time?
				if ($ex[0]=="default-lease-time") {
	                $this->dhcpStatements['lease_time'] = $ex[1];
				}
				// Is max lease time?
				if ($ex[0]=="max-lease-time") {
	                $this->dhcpStatements['max_lease_time'] = $ex[1];
				}
			}
     	}
        
     	if (is_array($attr['dhcpOption'])) {
			foreach($attr['dhcpOption'] AS $id=>$value) {
				$ex = explode(" ", $value);
				// Is domainname?
	            if ($ex[0]=="domain-name") {
					$this->dhcpSettings['domainname'] = $ex[1];
					$attr[] = $value;
				}
				// Is Gateway?
				if ($ex[0]=="routers") {
					$this->dhcpSettings['routers'] = $ex[1];
				}
				// Is subnetmask?
				if ($ex[0]=="subnet-mask") {
					$this->subnet = $ex[1];
				}
				// Is domainname?
				if ($ex[0]=="domain-name") {
					$this->dhcpSettings['domainname'] = ereg_replace("\"","", $ex[1]);
				}
				// Is netbios-name-servers?
				if ($ex[0]=="domain-name-servers") {
	
					$this->dhcpSettings['dns'] = $ex[1];
				}
				// Is netbios-node-type?
				if ($ex[0]=="netbios-node-type") {
					$this->netbios_node_type = $this->all_netbios_node_types[$ex[1]];
				}
				// Is dns?
				if ($ex[0]=="netbios-name-servers") {
					$this->dhcpSettings['netbios'] = $ex[1];
				}
			}
		}
		
		// Load DHCP Statements
		if (is_array($attr['dhcpStatements'])) {
			foreach($attr['dhcpStatements'] AS $id=>$value) {
			    $ex = explode(" ", $value);
			    // Is default lease time?
				if ($ex[0]=="default-lease-time") {
	                $this->dhcpStatements['lease_time'] = $ex[1];
				}
				// Is max lease time?
				if ($ex[0]=="max-lease-time") {
	                $this->dhcpStatements['max_lease_time'] = $ex[1];
				}
			}
		}
		
        if ($_SESSION['account']->getAccountModule('dhcp_settings')->dn!=$_SESSION['config']->get_suffix('dhcp')) {
	        if (!is_array($attr['dhcpNetMask'])) {
	        	$attr['dhcpNetMask'] = array();
	        }
		
			// Load DHCP netmask
			foreach($attr['dhcpNetMask'] AS $id=>$value) {
			    $ex = explode(" ", $value);
				// netmask
				$this->attributes['dhcpNetMask'][0] = $value;
			}
		}
		
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

		// Check if cn is not empty
		if ($_SESSION['config']->get_suffix('dhcp') != $this->getAccountContainer()->dn_orig) {
			if (!empty($_POST['cn'])) $_POST['cn'] = trim($_POST['cn']);
			if (empty($_POST['cn'])) {
			    $errors[] = $this->messages['cn'][0];
			    $this->attributes['cn'][0] = $_POST['cn'];
			}
			// Check, if cn is not already use:
		    elseif ( $_SESSION['cache']->in_cache($_POST['cn'],'cn', array('dhcp')) && $this->orig['cn']['0']!=$_POST['cn'] ) {
		       	$errors[] = $this->messages['cn'][1];
		      	$this->attributes['cn'][0] = $_POST['cn'];
		    }
			elseif (!check_ip($_POST['cn'],true)) {
				$errors[] = $this->messages['cn'][2];
				$this->attributes['cn'][0] = $_POST['cn'];
			}
			elseif (array_pop(explode(".",  $_POST['cn']))!=0) {
				$errors[] = $this->messages['cn'][2];
				$this->attributes['cn'][0] = $_POST['cn'];
			}
			else
			{
				$this->attributes['cn'][0] = $_POST['cn'];
					// if the cn was edit, reload the Ranges:
	            if ($_SESSION['account']->getAccountModule('range')->reload_ranges())
	            	$errors[] = $this->messages['ranges_reload'][0];
		            // if the cn was edit, reload the ips:
	            if ($_SESSION['account']->getAccountModule('fixed_ip')->reload_ips())
	            	$errors[] = $this->messages['ips_reload'][0];
			}
		}
		
		// Check domainname:
		if (!empty($_POST['domainname'])) $_POST['domainname'] = trim($_POST['domainname']);
		if (!empty($_POST['domainname'])) {
		    if (strlen($_POST['domainname'])>15) {
		        $errors[] = $this->messages['domainname'][0];
		        $this->dhcpSettings['domainname'] = $_POST['domainname'];
		        unset($this->attributes['dhcpOption'][5]);
			}
			elseif (strlen($_POST['domainname'])<3)
			{
			    $errors[] = $this->messages['domainname'][1];
			    $this->dhcpSettings['domainname'] = $_POST['domainname'];
			    unset($this->attributes['dhcpOption'][5]);
			}
			elseif (eregi("^[A-Za-z0-9\._-]*$", $_POST['domainname'])) {
				$this->dhcpSettings['domainname'] = $_POST['domainname'];
				$this->attributes['dhcpOption'][5] = "domain-name \"".$_POST['domainname']."\"";
			}
			else
			{
			    $errors[] = $this->messages['domainname'][2];
			    $this->dhcpSettings['domainname'] = $_POST['domainname'];
			    unset($this->attributes['dhcpOption'][5]);
			}
		}
		else
		{
		    $this->dhcpSettings['domainname'] = "";
		    unset($this->attributes['dhcpOption'][5]);
		}

		// Check DNS
		if (!empty($_POST['dns'])) $_POST['dns'] = trim($_POST['dns']);
		if (empty($_POST['dns'])) {
		    unset($this->attributes['dhcpOption'][0]);
		    $this->dhcpSettings['dns'] = "";
		}
		else
		{
			$ex = explode(",", $_POST['dns']);
			$dns = "";
			$is_first=true;
			$invalid = false;
			foreach($ex AS $string) {
				if ($is_first) {
					$dns .= $string;
					$is_first=false;
			    }
				else
				{
			    	$dns .= ",$string";
				}

	            if (!check_ip($string)) {
			        $invalid = true;
			    }
			}
			if ($invalid) {
			    $errors[] = $this->messages['dns'][0];
			    $this->dhcpSettings['dns'] = $dns;
			}
			else
			{
			    $this->dhcpSettings['dns'] = $dns;
			    $this->attributes['dhcpOption'][0] = "domain-name-servers ".$dns."";
			}
		}
		
		// Lease Time
		if (!empty($_POST['lease_time'])) $_POST['lease_time'] = trim($_POST['lease_time']);
		if (!is_numeric($_POST['lease_time']) && !empty($_POST['lease_time'])) {
			$errors[] = $this->messages['lease_time'][0];
			$this->dhcpStatements['lease_time'] = $_POST['lease_time'];
			unset($this->attributes['dhcpStatements'][0]);
		}
		elseif (empty($_POST['lease_time'])) {
		    unset($this->dhcpStatements['lease_time']);
		    unset($this->attributes['dhcpStatements'][0]);
		}
		else
		{
			$this->dhcpStatements['lease_time'] = $_POST['lease_time'];
			$this->attributes['dhcpStatements'][0] = "default-lease-time ".$_POST['lease_time'];
		}
		
		// Max lease Time
		if (!empty($_POST['max_lease_time'])) $_POST['max_lease_time'] = trim($_POST['max_lease_time']);
		if (!is_numeric($_POST['max_lease_time']) && !empty($_POST['max_lease_time'])) {
			$errors[] = $this->messages['max_lease_time'][0];
			$this->dhcpStatements['max_lease_time'] = $_POST['max_lease_time'];
			unset($this->attributes['dhcpStatements'][1]);
		}
		elseif (empty($_POST['max_lease_time'])) {
		    unset($this->dhcpStatements['max_lease_time']);
		    unset($this->attributes['dhcpStatements'][1]);
		}
		else
		{
			$this->dhcpStatements['max_lease_time'] = $_POST['max_lease_time'];
			$this->attributes['dhcpStatements'][1] = "max-lease-time ".$_POST['max_lease_time'];
		}
		
		// Default Gateway
		if (!empty($_POST['routers'])) $_POST['routers'] = trim($_POST['routers']);
		if (!check_ip($_POST['routers']) && !empty($_POST['routers'])) {
			$errors[] = $this->messages['routers'][0];
			$this->dhcpSettings['routers'] = $_POST['routers'];
			unset($this->attributes['dhcpOption'][1]);
		}
		elseif (empty($_POST['routers'])) {
		    unset($this->dhcpSettings['routers']);
		    unset($this->attributes['dhcpOption'][1]);
		}
		else
		{
			$this->dhcpSettings['routers'] = $_POST['routers'];
			$this->attributes['dhcpOption'][1] = "routers ".$_POST['routers'];
		}
		
		// Netbios
		if (!empty($_POST['netbios'])) $_POST['netbios'] = trim($_POST['netbios']);
		if (!check_ip($_POST['netbios']) && !empty($_POST['netbios'])) {
			$errors[] = $this->messages['netbios'][0];
			$this->dhcpSettings['netbios'] = $_POST['netbios'];
			unset($this->attributes['dhcpOption'][2]);
		}
		elseif (empty($_POST['netbios'])) {
		    // Nix tuhen
		    unset($this->attributes['dhcpOption'][2]);
		}
		else
		{
			$this->dhcpSettings['netbios'] = $_POST['netbios'];
			$this->attributes['dhcpOption'][2] = "netbios-name-servers ".$_POST['netbios'];
		}
		
		// Netbios Node Type:
		if (in_array($_POST['netbios_node_type'],$this->all_netbios_node_types)) {
			$this->netbios_node_type = $_POST['netbios_node_type'];
			// Nummer ermitteln
			$ex = explode("x", $_POST['netbios_node_type']);
			$ex = explode(")", $ex['1']);
			$netbios_node_type = (int) $ex['0'];
            $this->attributes['dhcpOption'][3] = "netbios-node-type ".$netbios_node_type;
		}
		else
		{
		    $errors[] = $this->messages['netbios_node_type'][0];
		    unset($this->attributes['dhcpOption'][3]);
		}
		if ($_SESSION['account']->getAccountModule('dhcp_settings')->dn!=$_SESSION['config']->get_suffix('dhcp')) {
			// Check subnet
			$_POST['subnet'] = trim($_POST['subnet']);
			if (!check_ip($_POST['subnet'], true)) {
			    $errors[] = $this->messages['subnet'][0];
			    $this->subnet = $_POST['subnet'];
			    unset($this->attributes['dhcpOption'][4]);
			}
			else
			{
			    $this->subnet = $_POST['subnet'];
			    $this->attributes['dhcpOption'][4] = "subnet-mask ".$_POST['subnet'];
			}

			// Check Netmask, obly for Range:
				$_POST['netmask'] = trim($_POST['netmask']);
				// calculate netmask from subnet:
				if (!empty($this->attributes['dhcpOption'][4])) {
					$ex=explode(".", $_POST['subnet']);
					$num = 0;
					foreach($ex AS $mask) {
					    $binär = decbin($mask);
					    $num += substr_count($binär, 1);
					}
					if (empty($_POST['netmask'])) {
					    $errors[] = $this->messages['netmask'][0];
					}
					elseif ($num!=$_POST['netmask']) {
					    $errors[] = $this->messages['netmask'][1];
					}
					$this->attributes['dhcpNetMask'][0] = $num;
				}
				else
				{
				    unset($this->attributes['dhcpNetMask'][0]);
				}
		}

		return $errors;
	}
	
	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	public function display_html_attributes() {
		// user name if no posixAccount
		$modules = $_SESSION['config']->get_AccountModules($this->get_scope());
		
		// Subnetz name
		if ($_SESSION['config']->get_suffix('dhcp') == $this->getAccountContainer()->dn_orig) {
	        $return[] = array(
	        	array('kind' => 'text', 'text' => _('Subnet') . ":* "),
				array('kind' => 'text', 'text' => $this->attributes['cn'][0]),
				array('kind' => 'help', 'value' => 'subnet', 'scope' => 'user'));
		}
		else
		{
	        $return[] = array(
	        	array('kind' => 'text', 'text' => _('Subnet') . ":* "),
				array('kind' => 'input', 'name' => 'cn', 'value' => $this->attributes['cn'][0]),
				array('kind' => 'help', 'value' => 'subnet', 'scope' => 'user'),
				array('kind' => 'text', 'text' => "&nbsp;&nbsp;&nbsp;" . _('Example') . ": 192.168.10.<b>0</b>") );
		}

		// Domainname
        $return[] = array(
        	array('kind' => 'text', 'text' => _('Domain name') . ": "),
			array('kind' => 'input', 'name' => 'domainname', 'value' => $this->dhcpSettings['domainname']),
			array('kind' => 'help', 'value' => 'domainname', 'scope' => 'user'));
			
		// Lease Time
        $return[] = array(
        	array('kind' => 'text', 'text' => _('Lease time') . ": "),
			array('kind' => 'input', 'name' => 'lease_time', 'value' => $this->dhcpStatements['lease_time']),
			array('kind' => 'help', 'value' => 'leasetime', 'scope' => 'user'));
			
		// Max lease Time
        $return[] = array(
        	array('kind' => 'text', 'text' => _('Maximum lease time') . ": "),
			array('kind' => 'input', 'name' => 'max_lease_time', 'value' => $this->dhcpStatements['max_lease_time']),
			array('kind' => 'help', 'value' => 'max_leasetime', 'scope' => 'user'));

		// DNS
        $return[] = array(
        	array('kind' => 'text', 'text' => _('DNS') . ": "),
			array('kind' => 'input', 'name' => 'dns', 'value' => $this->dhcpSettings['dns']),
			array('kind' => 'help', 'value' => 'dns', 'scope' => 'user'));

		// Gateway
        $return[] = array(
        	array('kind' => 'text', 'text' => _('Default gateway') . ": "),
			array('kind' => 'input', 'name' => 'routers', 'value' => $this->dhcpSettings['routers']),
			array('kind' => 'help', 'value' => 'gateway', 'scope' => 'user'));
			
		// Netbios Name Server
        $return[] = array(
        	array('kind' => 'text', 'text' => _('Netbios name server') . ": "),
			array('kind' => 'input', 'name' => 'netbios', 'value' => $this->dhcpSettings['netbios']),
			array('kind' => 'help', 'value' => 'netbios', 'scope' => 'user'));

		// Netbios Node Type
		if(empty($this->netbios_node_type)) $this->netbios_node_type = _("H-Node (0x08)");
        $return[] = array(
        	array('kind' => 'text', 'text' => _('Netbios node type') . ": "),
			array('kind' => 'select', 'name' => 'netbios_node_type', 'options' => $this->all_netbios_node_types, 'options_selected' => array($this->netbios_node_type)),
			array('kind' => 'help', 'value' => 'netbios_type', 'scope' => 'user'));

		if ($_SESSION['account']->getAccountModule('dhcp_settings')->dn!=$_SESSION['config']->get_suffix('dhcp')) {
			// subnetmask
	        $return[] = array(
	        	array('kind' => 'text', 'text' => _('Subnet mask') . ":* "),
				array('kind' => 'input', 'name' => 'subnet', 'value' => $this->subnet),
				array('kind' => 'help', 'value' => 'subnetmask', 'scope' => 'user'));

			// Netmask
			if ($this->type!=_("Fix IP")) {
		        $return[] = array(
		        	array('kind' => 'text', 'text' => _('Net mask') . ":* "),
					array('kind' => 'input', 'name' => 'netmask', 'value' => $this->attributes['dhcpNetMask'][0]),
					array('kind' => 'help', 'value' => 'netmask', 'scope' => 'user'));
			}
		}
		
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
			array('kind' => 'input', 'name' => 'cn', 'type'=>'text','value' => $this->attributes['cn'][0]),
			array('kind' => 'help', 'value' => 'subnet', 'scope' => 'user'));

		// Domainname
        $return[] = array(
        	array('kind' => 'text', 'text' => _('Domain name') . ": "),
			array('kind' => 'input', 'name' => 'domainname', 'type'=>'text','value' => $this->dhcpSettings['domainname']),
			array('kind' => 'help', 'value' => 'domainname', 'scope' => 'user'));

		// Lease Time
        $return[] = array(
        	array('kind' => 'text', 'text' => _('Lease time') . ": "),
			array('kind' => 'input', 'name' => 'lease_time', 'type'=>'text','value' => $this->dhcpStatements['lease_time']),
			array('kind' => 'help', 'value' => 'leasetime', 'scope' => 'user'));

		// Max lease Time
        $return[] = array(
        	array('kind' => 'text', 'text' => _('Maximum lease time') . ": "),
			array('kind' => 'input', 'name' => 'max_lease_time', 'type'=>'text','value' => $this->dhcpStatements['max_lease_time']),
			array('kind' => 'help', 'value' => 'max_leasetime', 'scope' => 'user'));

		// DNS
        $return[] = array(
        	array('kind' => 'text', 'text' => _('DNS') . ": "),
			array('kind' => 'input', 'name' => 'dns', 'type'=>'text','value' => $this->dhcpSettings['dns']),
			array('kind' => 'help', 'value' => 'dns', 'scope' => 'user'));

		// Gateway
        $return[] = array(
        	array('kind' => 'text', 'text' => _('Default gateway') . ": "),
			array('kind' => 'input', 'name' => 'routers', 'type'=>'text','value' => $this->dhcpSettings['routers']),
			array('kind' => 'help', 'value' => 'gateway', 'scope' => 'user'));

		// Netbios Name Server
        $return[] = array(
        	array('kind' => 'text', 'text' => _('Netbios name server') . ": "),
			array('kind' => 'input', 'name' => 'netbios', 'type'=>'text','value' => $this->dhcpSettings['netbios']),
			array('kind' => 'help', 'value' => 'netbios', 'scope' => 'user'));

		// Netbios Node Type
        $return[] = array(
        	array('kind' => 'text', 'text' => _('Netbios node type') . ": "),
			array('kind' => 'select', 'name' => 'netbios_node_type', 'options' => $this->all_netbios_node_types, 'options_selected' => array($this->netbios_node_type)),
			array('kind' => 'help', 'value' => 'netbios_type', 'scope' => 'user'));
		// subnetmask
        $return[] = array(
        	array('kind' => 'text', 'text' => _('Subnet mask') . ": "),
			array('kind' => 'input', 'name' => 'subnet', 'type'=>'text','value' => $this->subnet),
			array('kind' => 'help', 'value' => 'subnetmask', 'scope' => 'user'));
		return $return;
	}
	
	/**
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
		
		$this->dhcpStatements['lease_time'] = $profile['lease_time'][0];
		$this->attributes['dhcpStatements'][0] = "default-lease-time ".$profile['lease_time'][0];
		
		$this->dhcpStatements['max_lease_time'] = $profile['max_lease_time'][0];
		$this->attributes['dhcpStatements'][1] = "max-lease-time ".$profile['max_lease_time'][0];
		
		$this->dhcpSettings['dns'] = $profile['dns'][0];
		$this->attributes['dhcpOption'][0] = "domain-name-servers ". $profile['dns'][0];
		
		$this->dhcpSettings['routers'] = $profile['routers'][0];
		$this->attributes['dhcpOption'][1] = "routers ".$profile['routers'][0];
		
		$this->dhcpSettings['netbios'] = $profile['netbios'][0];
		$this->attributes['dhcpOption'][2] = "netbios-name-servers ".$profile['netbios'][0];
		
		$this->netbios_node_type =  $profile['netbios_node_type'][0];
		$this->attributes['dhcpOption'][3] = "netbios-node-type ".((int) array_shift(explode("(", array_pop(explode("x", $profile['netbios_node_type'][0])))));
		if ($_SESSION['account']->getAccountModule('dhcp_settings')->dn!=$_SESSION['config']->get_suffix('dhcp')) {
			$this->subnet = $profile['subnet'][0];
			$this->attributes['dhcpOption'][4] = "subnet-mask ". $profile['subnet'][0];

	        // calc the netmask:
			$ex=explode(".", $this->subnet);
			$num = 0;
			foreach($ex AS $mask) {
			    $binär = decbin($mask);
			    $num += substr_count($binär, 1);
			}
			$this->attributes['dhcpNetMask'][0] = $num;
		}
	}
}
?>