<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2008        Thomas Manninger
                2008 -2009  Roland Gruber

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
* @author Roland Gruber
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

/**
* Manages DHCP entries.
*
* @package modules
*/
class dhcp_settings extends baseModule {
	
	// Netbios node type:
	public $netbios_node_type;
	
	// All netbios node types:
	private $all_netbios_node_types;
	
	// ALL DHCP Settings
	public $dhcpSettings;
	
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
		$return['objectClasses'] = array('top', 'dhcpOptions', 'dhcpComments');
		// managed attributes
		$return['attributes'] = array('cn', 'dhcpOption');
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
				"Text" => _("Packets are sent to the default gateway if the receiver does not reside in the same network. The default gateway routes them to the target network.")
			) , 'netbios' => array(
				"Headline" => _("Netbios name servers"),
				"Text" => _("The IP addresses of the Netbios name servers (e.g. \"123.123.123.123, 123.123.123.124\").")
			) , 'netbios_type' => array(
				"Headline" => _("Netbios node type"),
				"Text" => _("<b>B-Node (0x01): Broadcast.</b><br/>The client tries to find other workstations via broadcasting
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
			), 'description' => array(
				"Headline" => _("Description"),
				"Text" => _("Here you can enter a description for this DHCP entry.")
			) );
		// available PDF fields
		$return['PDF_fields'] = array(
			'subnet',
			'domainName',
			'leaseTime',
			'maxLeaseTime',
			'DNSserver',
			'gateway',
			'netbiosServer',
			'netbiosType',
			'subnetMask',
			'netMask',
			'description'
		);
		// profile elements
		$return['profile_options'] = array(
			// Subnetz name
	        array(
	        	array('kind' => 'text', 'text' => _('Subnet') . ": "),
				array('kind' => 'input', 'name' => 'cn', 'type'=>'text'),
				array('kind' => 'help', 'value' => 'subnet', 'scope' => 'user')),
			// Domainname
	        array(
	        	array('kind' => 'text', 'text' => _('Domain name') . ": "),
				array('kind' => 'input', 'name' => 'domainname', 'type'=>'text'),
				array('kind' => 'help', 'value' => 'domainname', 'scope' => 'user')),
			// Lease Time
	        array(
	        	array('kind' => 'text', 'text' => _('Lease time') . ": "),
				array('kind' => 'input', 'name' => 'lease_time', 'type'=>'text'),
				array('kind' => 'help', 'value' => 'leasetime', 'scope' => 'user')),
			// Max lease Time
	        array(
	        	array('kind' => 'text', 'text' => _('Maximum lease time') . ": "),
				array('kind' => 'input', 'name' => 'max_lease_time', 'type'=>'text'),
				array('kind' => 'help', 'value' => 'max_leasetime', 'scope' => 'user')),
			// DNS
	        array(
	        	array('kind' => 'text', 'text' => _('DNS') . ": "),
				array('kind' => 'input', 'name' => 'dns', 'type'=>'text'),
				array('kind' => 'help', 'value' => 'dns', 'scope' => 'user')),
			// Gateway
	        array(
	        	array('kind' => 'text', 'text' => _('Default gateway') . ": "),
				array('kind' => 'input', 'name' => 'routers', 'type'=>'text'),
				array('kind' => 'help', 'value' => 'gateway', 'scope' => 'user')),
			// Netbios Name Servers
	        array(
	        	array('kind' => 'text', 'text' => _('Netbios name servers') . ": "),
				array('kind' => 'input', 'name' => 'netbios', 'type'=>'text'),
				array('kind' => 'help', 'value' => 'netbios', 'scope' => 'user')),
			// Netbios Node Type
	        array(
	        	array('kind' => 'text', 'text' => _('Netbios node type') . ": "),
				array('kind' => 'select', 'name' => 'netbios_node_type', 'options' => $this->all_netbios_node_types),
				array('kind' => 'help', 'value' => 'netbios_type', 'scope' => 'user')),
			// subnetmask
	        array(
	        	array('kind' => 'text', 'text' => _('Subnet mask') . ": "),
				array('kind' => 'input', 'name' => 'subnet', 'type'=>'text'),
				array('kind' => 'help', 'value' => 'subnetmask', 'scope' => 'user'))
			);
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'dhcp_settings_subnet',
				'description' => _('Subnet'),
				'help' => 'subnet',
				'example' => '192.168.10.0',
				'required' => true,
				'unique' => 'true'
			),
			array(
				'name' => 'dhcp_settings_domainName',
				'description' => _('Domain name'),
				'help' => 'domainname',
				'example' => 'mydomain.com',
			),
			array(
				'name' => 'dhcp_settings_leaseTime',
				'description' => _('Lease time'),
				'help' => 'leasetime',
				'example' => '86400',
			),
			array(
				'name' => 'dhcp_settings_maxLeaseTime',
				'description' => _('Maximum lease time'),
				'help' => 'max_leasetime',
				'example' => '172800',
			),
			array(
				'name' => 'dhcp_settings_DNSserver',
				'description' => _('DNS'),
				'help' => 'dns',
				'example' => '192.168.10.250',
			),
			array(
				'name' => 'dhcp_settings_gateway',
				'description' => _('Default gateway'),
				'help' => 'gateway',
				'example' => '192.168.10.251',
			),
			array(
				'name' => 'dhcp_settings_netbiosServer',
				'description' => _('Netbios name servers'),
				'help' => 'netbios',
				'example' => '192.168.10.252, 192.168.10.253',
			),
			array(
				'name' => 'dhcp_settings_netbiosType',
				'description' => _('Netbios node type'),
				'help' => 'netbios_type',
				'example' => 'M',
				'default' => 'H',
				'values' => 'B, H, M, P'
			),
			array(
				'name' => 'dhcp_settings_subnetMask',
				'description' => _('Subnet mask'),
				'help' => 'subnetmask',
				'required' => true,
				'example' => '255.255.255.0',
			),
			array(
				'name' => 'dhcp_settings_description',
				'description' => _('Description'),
				'help' => 'description',
			),
		);
		return $return;
	}
	
	/**
	* This function fills the message array.
	*/
	public function load_Messages() {
		$this->messages['cn'][0] = array('ERROR', _('No subnet entered.'));
		$this->messages['cn'][1] = array('ERROR', _('The subnet is already in use.'));
		$this->messages['cn'][2] = array('ERROR', _('The subnet is invalid.'));
		$this->messages['cn'][3] = array('ERROR', _('Account %s:') . ' dhcp_settings_subnet', _('The subnet is invalid.'));
		$this->messages['dns'][0] = array('ERROR', _('You entered one or more invalid DNS servers.'));
		$this->messages['dns'][1] = array('ERROR', _('Account %s:') . ' dhcp_settings_DNSserver', _('You entered one or more invalid DNS servers.'));
		$this->messages['lease_time'][0] = array('ERROR', _('The lease time is invalid.'));
		$this->messages['lease_time'][1] = array('ERROR', _('Account %s:') . ' dhcp_settings_leaseTime', _('The lease time is invalid.'));
		$this->messages['routers'][0] = array('ERROR', _('The default gateway is invalid.'));
		$this->messages['routers'][1] = array('ERROR', _('Account %s:') . ' dhcp_settings_gateway', _('The default gateway is invalid.'));
		$this->messages['netbios'][0] = array('ERROR', _('The Netbios server is invalid.'));
		$this->messages['netbios'][1] = array('ERROR', _('Account %s:') . ' dhcp_settings_netbiosServer', _('The Netbios server is invalid.'));
		$this->messages['netbios_node_type'][0] = array('ERROR', _('The entered Netbios node type does not exist.'));
		$this->messages['netbios_node_type'][1] = array('ERROR', _('Account %s:') . ' dhcp_settings_netbiosType', _('The entered Netbios node type does not exist.'));
		$this->messages['max_lease_time'][0] = array('ERROR', _('The maximum lease time is invalid.'));
		$this->messages['max_lease_time'][1] = array('ERROR', _('Account %s:') . ' dhcp_settings_maxLeaseTime', _('The maximum lease time is invalid.'));
		$this->messages['subnet'][0] = array('ERROR', _('The subnet mask is invalid.'));
		$this->messages['subnet'][1] = array('ERROR', _('Account %s:') . ' dhcp_settings_subnetMask', _('The subnet mask is invalid.'));
		$this->messages['netmask'][0] = array('INFO', _('The netmask was set.'));
		$this->messages['netmask'][1] = array('INFO', _('The net mask was invalid and was corrected.'));
		$this->messages['ranges_reload'][0] = array('INFO', _('The DHCP ranges were changed to fit for the new subnet.'));
		$this->messages['ips_reload'][0] = array('INFO', 'The fixed IP addresses were changed to fit for the new subnet.');
		$this->messages['domainname'][0] = array('ERROR', _('The domain name needs to be shorter than 15 characters.'));
		$this->messages['domainname'][1] = array('ERROR', _('The domain name needs to have at least 3 characters.'));
		$this->messages['domainname'][2] = array('ERROR', _('The domain name includes invalid characters. Valid characters are A-Z, a-z, 0-9, ".", "_","-".'));
		$this->messages['domainname'][3] = array('ERROR', _('Account %s:') . ' dhcp_settings_domainName', _('The domain name needs to be shorter than 15 characters.'));
		$this->messages['domainname'][4] = array('ERROR', _('Account %s:') . ' dhcp_settings_domainName', _('The domain name needs to have at least 3 characters.'));
		$this->messages['domainname'][5] = array('ERROR', _('Account %s:') . ' dhcp_settings_domainName', _('The domain name includes invalid characters. Valid characters are A-Z, a-z, 0-9, ".", "_","-".'));
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
		if (!empty($this->dhcpSettings['routers']) && !check_ip($this->dhcpSettings['routers'])) return false;
		
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
		if ($_SESSION['account']->getAccountModule('dhcp_settings')->dn!=$_SESSION['config']->get_suffix('dhcp')) {
			// Standard Things
			if (!in_array_ignore_case("dhcpSubnet", $this->attributes['objectClass'])) {
				$this->attributes['objectClass'][] = "dhcpSubnet";
			}
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
			$return = $this->getAccountContainer()->save_module_attributes($this->attributes, $this->orig);
		}
		else {
			// Basicsettings...
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
		$this->dn = $this->getAccountContainer()->dn_orig;

        if (!is_array($attr['dhcpOption'])) {
        	$attr['dhcpOption'] = array();
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
					$this->dhcpSettings['netbios'] = substr($value, 21);
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
		// check if DHCP main settings and valid DHCP entry
		if ($_SESSION['config']->get_suffix('dhcp') == $this->getAccountContainer()->dn_orig) {
			if (!in_array_ignore_case('dhcpServer', $this->attributes['objectClass'])) {
				return array();
			}
		}
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
		else {
			$ex = explode(",", $_POST['dns']);
			$dns = "";
			$is_first=true;
			$invalid = false;
			foreach($ex AS $string) {
				if ($is_first) {
					$dns .= $string;
					$is_first=false;
			    }
				else {
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
			else {
			    $this->dhcpSettings['dns'] = $dns;
			    $this->attributes['dhcpOption'][0] = "domain-name-servers ".$dns."";
			}
		}
		
		// Lease Time
		if (!empty($_POST['lease_time'])) {
			$_POST['lease_time'] = trim($_POST['lease_time']);
		}
		$this->setDefaultLeaseTime($_POST['lease_time']);
		if (!is_numeric($_POST['lease_time']) && !empty($_POST['lease_time'])) {
			$errors[] = $this->messages['lease_time'][0];
		}
		
		// Max lease Time
		if (!empty($_POST['max_lease_time'])) {
			$_POST['max_lease_time'] = trim($_POST['max_lease_time']);
		}
		$this->setMaxLeaseTime($_POST['max_lease_time']);
		if (!is_numeric($_POST['max_lease_time']) && !empty($_POST['max_lease_time'])) {
			$errors[] = $this->messages['max_lease_time'][0];
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
		$netbiosServers = explode(', ', $_POST['netbios']);
		$netbiosServersOk = true;
		for ($i = 0; $i < sizeof($netbiosServers); $i++) {
			if (!check_ip($netbiosServers[$i])) {
				$netbiosServersOk = false;
				break;
			}
		}
		if (!$netbiosServersOk && !empty($_POST['netbios'])) {
			$errors[] = $this->messages['netbios'][0];
			$this->dhcpSettings['netbios'] = $_POST['netbios'];
			unset($this->attributes['dhcpOption'][2]);
		}
		elseif (empty($_POST['netbios'])) {
		    unset($this->attributes['dhcpOption'][2]);
		}
		else {
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
		else {
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
			else {
			    $this->subnet = $_POST['subnet'];
			    $this->attributes['dhcpOption'][4] = "subnet-mask ".$_POST['subnet'];
			}

			// Check Netmask, obly for Range:
			$_POST['netmask'] = trim($_POST['netmask']);
			// calculate netmask from subnet:
			if (!empty($this->attributes['dhcpOption'][4])) {
				$num = $this->calculateNetMask($_POST['subnet']);
				if (empty($_POST['netmask'])) {
				    $errors[] = $this->messages['netmask'][0];
				}
				elseif ($num!=$_POST['netmask']) {
				    $errors[] = $this->messages['netmask'][1];
				}
				$this->attributes['dhcpNetMask'][0] = $num;
			}
			else {
			    unset($this->attributes['dhcpNetMask'][0]);
			}
		}
		$this->attributes['dhcpComments'][0] = $_POST['description'];
		
		return $errors;
	}
	
	/**
	 * Calculates the net mask from the subnet.
	 *
	 * @param String $subnet subnet
	 * @return integer netmask
	 */
	private function calculateNetMask($subnet) {
		$ex = explode(".", $subnet);
		$num = 0;
		foreach($ex AS $mask) {
		    $binary = decbin($mask);
		    $num += substr_count($binary, 1);
		}
		return $num;
	}
	
	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	public function display_html_attributes() {
		// check if DHCP main settings and valid DHCP entry
		if ($_SESSION['config']->get_suffix('dhcp') == $this->getAccountContainer()->dn_orig) {
			if (!in_array_ignore_case('dhcpServer', $this->attributes['objectClass'])) {
				StatusMessage("ERROR", _('Please set your LDAP suffix to an LDAP entry with object class "dhcpServer".'));
				return array();
			}
		}
		// Subnetz name
		if ($_SESSION['config']->get_suffix('dhcp') == $this->getAccountContainer()->dn_orig) {
	        $return[] = array(
	        	array('kind' => 'text', 'text' => _('Subnet') . ":* "),
				array('kind' => 'text', 'text' => $this->attributes['cn'][0]),
				array('kind' => 'help', 'value' => 'subnet', 'scope' => 'user'));
		}
		else {
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
			array('kind' => 'input', 'name' => 'lease_time', 'value' => $this->getDefaultLeaseTime()),
			array('kind' => 'help', 'value' => 'leasetime', 'scope' => 'user'));
			
		// Max lease Time
        $return[] = array(
        	array('kind' => 'text', 'text' => _('Maximum lease time') . ": "),
			array('kind' => 'input', 'name' => 'max_lease_time', 'value' => $this->getMaxLeaseTime()),
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
			
		// Netbios Name Servers
        $return[] = array(
        	array('kind' => 'text', 'text' => _('Netbios name servers') . ": "),
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
			if ($this->type!=_("Fixed IP")) {
		        $return[] = array(
		        	array('kind' => 'text', 'text' => _('Net mask') . ":* "),
					array('kind' => 'input', 'name' => 'netmask', 'value' => $this->attributes['dhcpNetMask'][0]),
					array('kind' => 'help', 'value' => 'netmask', 'scope' => 'user'));
			}
		}
		
		// description
        $return[] = array(
        	array('kind' => 'text', 'text' => _('Description') . ": "),
			array('kind' => 'input', 'name' => 'description', 'value' => $this->attributes['dhcpComments'][0]),
			array('kind' => 'help', 'value' => 'description', 'scope' => 'user'));

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
		
		$this->setDefaultLeaseTime($profile['lease_time'][0]);
		
		$this->setMaxLeaseTime($profile['max_lease_time'][0]);
		
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
			    $binary = decbin($mask);
			    $num += substr_count($binary, 1);
			}
			$this->attributes['dhcpNetMask'][0] = $num;
		}
	}
	
	/**
	* Returns the PDF entries for this module.
	*
	* @return array list of possible PDF entries
	*/
	function get_pdfEntries() {
		return array(
			get_class($this) . '_subnet' => array('<block><key>' . _('Subnet') . '</key><value>' . $this->attributes['cn'][0] . '</value></block>'),
			get_class($this) . '_domainName' => array('<block><key>' . _('Domain name') . '</key><value>' . $this->dhcpSettings['domainname'] . '</value></block>'),
			get_class($this) . '_leaseTime' => array('<block><key>' . _('Lease time') . '</key><value>' . $this->getDefaultLeaseTime() . '</value></block>'),
			get_class($this) . '_maxLeaseTime' => array('<block><key>' . _('Maximum lease time') . '</key><value>' . $this->getMaxLeaseTime() . '</value></block>'),
			get_class($this) . '_DNSserver' => array('<block><key>' . _('DNS') . '</key><value>' . $this->dhcpSettings['dns'] . '</value></block>'),
			get_class($this) . '_gateway' => array('<block><key>' . _('Default gateway') . '</key><value>' . $this->dhcpSettings['routers'] . '</value></block>'),
			get_class($this) . '_netbiosServer' => array('<block><key>' . _('Netbios name servers') . '</key><value>' . $this->dhcpSettings['netbios'] . '</value></block>'),
			get_class($this) . '_netbiosType' => array('<block><key>' . _('Netbios node type') . '</key><value>' . $this->netbios_node_type . '</value></block>'),
			get_class($this) . '_subnetMask' => array('<block><key>' . _('Subnet mask') . '</key><value>' . $this->subnet . '</value></block>'),
			get_class($this) . '_netMask' => array('<block><key>' . _('Net mask') . '</key><value>' . $this->attributes['dhcpNetMask'][0] . '</value></block>'),
			get_class($this) . '_description' => array('<block><key>' . _('Description') . '</key><value>' . $this->attributes['dhcpComments'][0] . '</value></block>'),
		);
	}
	
	/**
	 * Returns the default lease time.
	 * 
	 * @return String time
	 */
	private function getDefaultLeaseTime() {
		$return = null;
		if (is_array($this->attributes['dhcpStatements'])) {
			for ($i = 0; $i < sizeof($this->attributes['dhcpStatements']); $i++) {
				if (substr($this->attributes['dhcpStatements'][$i], 0, 19) == 'default-lease-time ') {
					$return = substr($this->attributes['dhcpStatements'][$i],19);
					break;
				}
			}
		}
		return $return;
	}
	
	/**
	 * Sets the default lease time.
	 * 
	 * $time String time
	 */
	private function setDefaultLeaseTime($time) {
		if (!is_array($this->attributes['dhcpStatements'])) {
			$this->attributes['dhcpStatements'] = array();
		}
		for ($i = 0; $i < sizeof($this->attributes['dhcpStatements']); $i++) {
			if (substr($this->attributes['dhcpStatements'][$i], 0, 19) == 'default-lease-time ') {
				unset($this->attributes['dhcpStatements'][$i]);
				$this->attributes['dhcpStatements'] = array_values($this->attributes['dhcpStatements']);
			}
		}
		if (($time != null) && ($time != '')) {
			$this->attributes['dhcpStatements'][] = 'default-lease-time ' . $time;
		}
	}

	/**
	 * Returns the maximum lease time.
	 * 
	 * @return String time
	 */
	private function getMaxLeaseTime() {
		$return = null;
		if (is_array($this->attributes['dhcpStatements'])) {
			for ($i = 0; $i < sizeof($this->attributes['dhcpStatements']); $i++) {
				if (substr($this->attributes['dhcpStatements'][$i], 0, 15) == 'max-lease-time ') {
					$return = substr($this->attributes['dhcpStatements'][$i],15);
					break;
				}
			}
		}
		return $return;
	}
	
	/**
	 * Sets the maximum lease time.
	 * 
	 * $time String time
	 */
	private function setMaxLeaseTime($time) {
		if (!is_array($this->attributes['dhcpStatements'])) {
			$this->attributes['dhcpStatements'] = array();
		}
		for ($i = 0; $i < sizeof($this->attributes['dhcpStatements']); $i++) {
			if (substr($this->attributes['dhcpStatements'][$i], 0, 15) == 'max-lease-time ') {
				unset($this->attributes['dhcpStatements'][$i]);
				$this->attributes['dhcpStatements'] = array_values($this->attributes['dhcpStatements']);
			}
		}
		if (($time != null) && ($time != '')) {
			$this->attributes['dhcpStatements'][] = 'max-lease-time ' . $time;
		}
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
			if (!in_array("dhcpOptions", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "dhcpOptions";
			if (!in_array("dhcpSubnet", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "dhcpSubnet";
			// subnet
			if (check_ip($rawAccounts[$i][$ids['dhcp_settings_subnet']],true)) {
				$partialAccounts[$i]['cn'] = $rawAccounts[$i][$ids['dhcp_settings_subnet']];
			}
			else {
				$error = $this->messages['cn'][3];
				array_push($error, $i);
				$messages[] = $error;
			}
			// domain name
			if ($rawAccounts[$i][$ids['dhcp_settings_domainName']] != "") {
				if (strlen($rawAccounts[$i][$ids['dhcp_settings_domainName']])>15) {
					$error = $this->messages['domainname'][3];
					array_push($error, $i);
					$messages[] = $error;
				}
				elseif (strlen($rawAccounts[$i][$ids['dhcp_settings_domainName']])<3) {
					$error = $this->messages['domainname'][4];
					array_push($error, $i);
					$messages[] = $error;
				}
				elseif (eregi("^[A-Za-z0-9\._-]*$", $rawAccounts[$i][$ids['dhcp_settings_domainName']])) {
					$partialAccounts[$i]['dhcpOption'][] = "domain-name \"".$rawAccounts[$i][$ids['dhcp_settings_domainName']]."\"";
				}
				else {
					$error = $this->messages['domainname'][5];
					array_push($error, $i);
					$messages[] = $error;
				}
			}
			// lease time
			if ($rawAccounts[$i][$ids['dhcp_settings_leaseTime']] != '') {
				if (is_numeric($rawAccounts[$i][$ids['dhcp_settings_leaseTime']])) {
					$partialAccounts[$i]['dhcpStatements'][] = 'default-lease-time ' . $rawAccounts[$i][$ids['dhcp_settings_leaseTime']];
				}
				else {
					$error = $this->messages['lease_time'][1];
					array_push($error, $i);
					$messages[] = $error;
				}
			}
			// max lease time
			if ($rawAccounts[$i][$ids['dhcp_settings_maxLeaseTime']] != '') {
				if (is_numeric($rawAccounts[$i][$ids['dhcp_settings_maxLeaseTime']])) {
					$partialAccounts[$i]['dhcpStatements'][] = 'max-lease-time ' . $rawAccounts[$i][$ids['dhcp_settings_maxLeaseTime']];
				}
				else {
					$error = $this->messages['max_lease_time'][1];
					array_push($error, $i);
					$messages[] = $error;
				}
			}
			// DNS
			if ($rawAccounts[$i][$ids['dhcp_settings_DNSserver']] != '') {
				$ex = explode(",", $rawAccounts[$i][$ids['dhcp_settings_DNSserver']]);
				$invalid = false;
				foreach($ex AS $string) {
		            if (!check_ip($string)) {
				        $invalid = true;
				    }
				}
				if (!$invalid) {
					$partialAccounts[$i]['dhcpOption'][] = 'domain-name-servers ' . $rawAccounts[$i][$ids['dhcp_settings_DNSserver']];
				}
				else {
					$error = $this->messages['dns'][1];
					array_push($error, $i);
					$messages[] = $error;
				}
			}
			// gateway
			if ($rawAccounts[$i][$ids['dhcp_settings_gateway']] != '') {
				if (check_ip($rawAccounts[$i][$ids['dhcp_settings_gateway']])) {
					$partialAccounts[$i]['dhcpOption'][] = "routers ".$rawAccounts[$i][$ids['dhcp_settings_gateway']];
				}
				else {
					$error = $this->messages['routers'][1];
					array_push($error, $i);
					$messages[] = $error;
				}
			}
			// netbios name servers
			if ($rawAccounts[$i][$ids['dhcp_settings_netbiosServer']] != '') {
				$ex = explode(",", $rawAccounts[$i][$ids['dhcp_settings_netbiosServer']]);
				$invalid = false;
				foreach($ex AS $string) {
		            if (!check_ip($string)) {
				        $invalid = true;
				    }
				}
				if (!$invalid) {
					$partialAccounts[$i]['dhcpOption'][] = "netbios-name-servers " . $rawAccounts[$i][$ids['dhcp_settings_netbiosServer']];
				}
				else {
					$error = $this->messages['netbios'][1];
					array_push($error, $i);
					$messages[] = $error;
				}
			}
			// node type
			if ($rawAccounts[$i][$ids['dhcp_settings_netbiosType']] != '') {
				if (in_array($rawAccounts[$i][$ids['dhcp_settings_netbiosType']], array('B', 'P', 'M', 'H'))) {
					if ($rawAccounts[$i][$ids['dhcp_settings_netbiosType']] == 'B') {
						$partialAccounts[$i]['dhcpOption'][] = "netbios-node-type 1";
					}
					elseif ($rawAccounts[$i][$ids['dhcp_settings_netbiosType']] == 'P') {
						$partialAccounts[$i]['dhcpOption'][] = "netbios-node-type 2";
					}
					elseif ($rawAccounts[$i][$ids['dhcp_settings_netbiosType']] == 'M') {
						$partialAccounts[$i]['dhcpOption'][] = "netbios-node-type 4";
					}
					elseif ($rawAccounts[$i][$ids['dhcp_settings_netbiosType']] == 'H') {
						$partialAccounts[$i]['dhcpOption'][] = "netbios-node-type 8";
					}
				}
				else {
					$error = $this->messages['netbios_node_type'][1];
					array_push($error, $i);
					$messages[] = $error;
				}
			}
			else {
				$partialAccounts[$i]['dhcpOption'][] = "netbios-node-type 8"; // default H
			}
			// subnet mask
			if (check_ip($rawAccounts[$i][$ids['dhcp_settings_subnetMask']],true)) {
				$partialAccounts[$i]['dhcpOption'][] = "subnet-mask ".$rawAccounts[$i][$ids['dhcp_settings_subnetMask']];
			}
			else {
				$error = $this->messages['subnet'][1];
				array_push($error, $i);
				$messages[] = $error;
			}
			// net mask
			$mask = $this->calculateNetMask($rawAccounts[$i][$ids['dhcp_settings_subnetMask']]);
			$partialAccounts[$i]['dhcpNetMask'][0] = $mask;
			// description
			if (isset($rawAccounts[$i][$ids['dhcp_settings_description']]) && ($rawAccounts[$i][$ids['dhcp_settings_description']] != '')) {
				$partialAccounts[$i]['dhcpComments'][0] = $rawAccounts[$i][$ids['dhcp_settings_description']];
			}
		}
		return $messages;
	}


}

?>
