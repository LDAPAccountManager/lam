<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2008        Thomas Manninger
                2008 - 2024 Roland Gruber

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
 * This function checks if the given IP address is valid.
 *
 * @param string IP to check
 * @param check Subnet
 * @return boolean true or false
 **/

use LAM\TYPES\ConfiguredType;

if (!function_exists('check_ip')) {

	/**
	 * Checks if the given IP is valid.
	 *
	 * @param String $ip IP address
	 * @param boolean $subnet IP must be a subnet
	 */
	function check_ip($ip, $subnet = false) {
		$part = explode(".", $ip);
		// IP must contain 4 segments
		if (count($part) != 4) {
			return false;
		}
		else {
			// check each segment
			for ($i = 0; $i < count($part); $i++) {
				// only numbers are allowed
				if (!is_numeric($part[$i])
					|| ($part[$i] > 255)
					// non-subnet must be > 0 on last digit
					|| (!$subnet && ($i == 3) && ($part[$i] < 1))
					// subnet must be >= 0 on last digit
					|| ($subnet && ($i == 3) && ($part[$i] < 0))) {
					return false;
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

	private const UNKNOWN_CLIENTS = 'unknown-clients';
	private const DEFAULT_LEASE_TIME = 'default-lease-time';
	private const MAX_LEASE_TIME = 'max-lease-time';
	private const AUTHORITATIVE = 'authoritative';
	private const DOMAIN_SEARCH = 'domain-search';
	private const DOMAIN_NAME = 'domain-name';
	private const DOMAIN_NAME_SERVERS = 'domain-name-servers';
	private const ROUTERS = 'routers';
	private const NETBIOS_NAME_SERVERS = 'netbios-name-servers';
	private const NETBIOS_NODE_TYPE = 'netbios-node-type';
	private const SUBNET_MASK = 'subnet-mask';

	/** all netbios node types */
	private $all_netbios_node_types;
	/** unknown-client options */
	private $allowDenyOptions;
	/** LDAP attributes */
	public $attributes;

	/**
	 * Creates a new dhcp_settings object.
	 *
	 * @param string $scope account type
	 */
	function __construct($scope) {
		// list of node types
		$this->all_netbios_node_types = [
			"1" => _("B-Node (0x01)"),
			"2" => _("P-Node (0x02)"),
			"4" => _("M-Node (0x04)"),
			"8" => _("H-Node (0x08)")
		];
		$this->allowDenyOptions = [
			'-' => '',
			'allow' => _("Allow"),
			'deny' => _("Deny"),
		];
		// call parent constructor
		parent::__construct($scope);
	}

	/**
	 * Returns true if this module can manage accounts of the current type, otherwise false.
	 *
	 * @return boolean true if module fits
	 */
	public function can_manage() {
		return $this->get_scope() === 'dhcp';
	}

	/**
	 * Returns meta data that is interpreted by parent class
	 *
	 * @return array array with meta data
	 */
	public function get_metaData() {
		$return = [];
		// alias name
		$return["alias"] = _("DHCP settings");
		// this is a base module
		$return["is_base"] = true;
		// icon
		$return['icon'] = 'world-color.svg';
		// RDN attribute
		$return["RDN"] = ["cn" => "high"];
		// LDAP filter
		$return["ldap_filter"] = [];
		// module dependencies
		$return['dependencies'] = ['depends' => [], 'conflicts' => []];
		// managed object classes
		$return['objectClasses'] = ['top', 'dhcpOptions', 'dhcpSubnet'];
		// managed attributes
		$return['attributes'] = ['cn', 'dhcpOption', 'dhcpComments', 'dhcpNetMask', 'dhcpStatements'];
		// help Entries
		$return['help'] = [
			'domainname' => [
				"Headline" => _("Domain name"),
				"Text" => _("The domain name of the subnet.")
			],
			'subnet' => [
				"Headline" => _("Subnet"), 'attr' => 'cn',
				"Text" => _("The name of the subnet. Example: 192.168.10.0")
			],
			'leasetime' => [
				"Headline" => _("Lease time"),
				"Text" => _("The lease time specifies after how many seconds the client should request a new IP address.")
			],
			'max_leasetime' => [
				"Headline" => _("Maximum lease time"),
				"Text" => _("The maximum lease time specifies after how many seconds the client must request a new IP address.")
			],
			'dns' => [
				"Headline" => _("DNS (Domain Name System)"),
				"Text" => _("The IP addresses of the DNS servers. Multiple addresses are separated by \",\". Example: 192.168.0.10, 192.168.0.11")
			],
			'gateway' => [
				"Headline" => _("Default gateway"),
				"Text" => _("Packets are sent to the default gateway if the receiver does not reside in the same network. The default gateway routes them to the target network.")
			],
			'netbios' => [
				"Headline" => _("Netbios name servers"),
				"Text" => _("The IP addresses of the Netbios name servers (e.g. \"123.123.123.123, 123.123.123.124\").")
			],
			'netbios_type' => [
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
			],
			'subnetmask' => [
				"Headline" => _("Subnet mask"),
				"Text" => _("The subnet mask of the network.")
			],
			'netmask' => [
				"Headline" => _("Net mask"), 'attr' => 'dhcpNetMask',
				"Text" => _("The net mask is derived from the subnet mask. LAM will calculate it automatically.")
			],
			'description' => [
				"Headline" => _("Description"), 'attr' => 'dhcpComments',
				"Text" => _("Here you can enter a description for this DHCP entry.")
			],
			'unknownClients' => [
				"Headline" => _("Unknown clients"), 'attr' => 'dhcpStatements',
				"Text" => _("Specifies if unknown clients are allowed.")
			],
			'INFO_domain-search' => [
				'Headline' => _('Search domains'), 'attr' => 'dhcpOptions',
				'Text' => _('This is a list of domain names to be used by the client to locate not-fully-qualified domain names.')
			],
			'domain-search' => [
				'Headline' => _('Search domains'), 'attr' => 'dhcpOptions',
				'Text' => _('This is a list of domain names to be used by the client to locate not-fully-qualified domain names.')
					. ' ' . _("Multiple values are separated by semicolon.")
			],
			'authoritative' => [
				'Headline' => _('Authoritative'), 'attr' => 'dhcpStatements',
				'Text' => _('Authoritative servers reject requests for invalid IP addresses.')
			],
			'extraStatements' => [
				'Headline' => _('Extra statements'), 'attr' => 'dhcpStatements',
				'Text' => _('Please enter any additional DHCP statements.')
			],
			'extraOptions' => [
				'Headline' => _('Extra statements'), 'attr' => 'dhcpOptions',
				'Text' => _('Please enter any additional DHCP options.')
			],
		];
		// available PDF fields
		$return['PDF_fields'] = [
			'subnet' => _('Subnet'),
			'domainName' => _('Domain name'),
			'leaseTime' => _('Lease time'),
			'maxLeaseTime' => _('Maximum lease time'),
			'DNSserver' => _('DNS'),
			'gateway' => _('Default gateway'),
			'netbiosServer' => _('Netbios name servers'),
			'netbiosType' => _('Netbios node type'),
			'subnetMask' => _('Subnet mask'),
			'netMask' => _('Net mask'),
			'description' => _('Description'),
			'unknownClients' => _('Unknown clients'),
			self::DOMAIN_SEARCH => _('Search domains'),
			self::AUTHORITATIVE => _('Authoritative')
		];
		// profile elements
		$profileContainer = new htmlResponsiveRow();
		$profileContainer->add(new htmlResponsiveInputField(_('Subnet'), 'cn', null, 'subnet'));
		$profileContainer->add(new htmlResponsiveInputField(_('Domain name'), 'domainname', null, 'domainname'));
		$profileContainer->add(new htmlResponsiveInputField(_('Lease time'), 'lease_time', null, 'leasetime'));
		$profileContainer->add(new htmlResponsiveInputField(_('Maximum lease time'), 'max_lease_time', null, 'max_leasetime'));
		$profileContainer->add(new htmlResponsiveInputField(_('DNS'), 'dns', null, 'dns'));
		$profileContainer->add(new htmlResponsiveInputField(_('Search domains'), self::DOMAIN_SEARCH, null, 'domain-search'));
		$profileContainer->add(new htmlResponsiveInputField(_('Default gateway'), self::ROUTERS, null, 'gateway'));
		$profileContainer->add(new htmlResponsiveInputField(_('Netbios name servers'), 'netbios', null, 'netbios'));
		$nodeList = array_flip($this->all_netbios_node_types);
		$profileNodeSelect = new htmlResponsiveSelect('netbios_node_type', $nodeList, null, _('Netbios node type'), 'netbios_type');
		$profileNodeSelect->setHasDescriptiveElements(true);
		$profileContainer->add($profileNodeSelect);
		$profileContainer->add(new htmlResponsiveInputField(_('Subnet mask'), 'subnet', null, 'subnetmask'));
		$profileContainer->add(new htmlResponsiveInputCheckbox(self::AUTHORITATIVE, false, _('Authoritative'), 'authoritative'));
		$return['profile_options'] = $profileContainer;
		// upload fields
		$uploadAllowDenyOption = $this->allowDenyOptions;
		unset($uploadAllowDenyOption['-']);
		$uploadAllowDenyOption = implode(', ', $uploadAllowDenyOption);
		$return['upload_columns'] = [
			[
				'name' => 'dhcp_settings_subnet',
				'description' => _('Subnet'),
				'help' => 'subnet',
				'example' => '192.168.10.0',
				'required' => true,
				'unique' => true
			],
			[
				'name' => 'dhcp_settings_domainName',
				'description' => _('Domain name'),
				'help' => 'domainname',
				'example' => 'mydomain.com',
			],
			[
				'name' => 'dhcp_settings_leaseTime',
				'description' => _('Lease time'),
				'help' => 'leasetime',
				'example' => '86400',
			],
			[
				'name' => 'dhcp_settings_maxLeaseTime',
				'description' => _('Maximum lease time'),
				'help' => 'max_leasetime',
				'example' => '172800',
			],
			[
				'name' => 'dhcp_settings_DNSserver',
				'description' => _('DNS'),
				'help' => 'dns',
				'example' => '192.168.10.250',
			],
			[
				'name' => 'dhcp_settings_domain-search',
				'description' => _('Search domains'),
				'help' => 'domain-search',
				'example' => 'example.com',
			],
			[
				'name' => 'dhcp_settings_gateway',
				'description' => _('Default gateway'),
				'help' => 'gateway',
				'example' => '192.168.10.251',
			],
			[
				'name' => 'dhcp_settings_netbiosServer',
				'description' => _('Netbios name servers'),
				'help' => 'netbios',
				'example' => '192.168.10.252, 192.168.10.253',
			],
			[
				'name' => 'dhcp_settings_netbiosType',
				'description' => _('Netbios node type'),
				'help' => 'netbios_type',
				'example' => 'M',
				'default' => 'H',
				'values' => 'B, H, M, P'
			],
			[
				'name' => 'dhcp_settings_unknownClients',
				'description' => _('Unknown clients'),
				'help' => 'unknownClients',
				'values' => $uploadAllowDenyOption
			],
			[
				'name' => 'dhcp_settings_subnetMask',
				'description' => _('Subnet mask'),
				'help' => 'subnetmask',
				'required' => true,
				'example' => '255.255.255.0',
			],
			[
				'name' => 'dhcp_settings_authoritative',
				'description' => _('Authoritative'),
				'help' => 'authoritative',
				'values' => 'true, false',
				'default' => 'false'
			],
			[
				'name' => 'dhcp_settings_description',
				'description' => _('Description'),
				'help' => 'description',
			],
		];
		return $return;
	}

	/**
	 * This function fills the message array.
	 */
	public function load_Messages() {
		$this->messages['cn'][0] = ['ERROR', _('No subnet entered.')];
		$this->messages['cn'][1] = ['ERROR', _('The subnet is already in use.')];
		$this->messages['cn'][2] = ['ERROR', _('The subnet is invalid.')];
		$this->messages['cn'][3] = ['ERROR', _('Account %s:') . ' dhcp_settings_subnet', _('The subnet is invalid.')];
		$this->messages['dns'][0] = ['ERROR', _('You entered one or more invalid DNS servers.')];
		$this->messages['dns'][1] = ['ERROR', _('Account %s:') . ' dhcp_settings_DNSserver', _('You entered one or more invalid DNS servers.')];
		$this->messages['lease_time'][0] = ['ERROR', _('The lease time is invalid.')];
		$this->messages['lease_time'][1] = ['ERROR', _('Account %s:') . ' dhcp_settings_leaseTime', _('The lease time is invalid.')];
		$this->messages['routers'][0] = ['ERROR', _('The default gateway is invalid.')];
		$this->messages['routers'][1] = ['ERROR', _('Account %s:') . ' dhcp_settings_gateway', _('The default gateway is invalid.')];
		$this->messages['netbios'][0] = ['ERROR', _('The Netbios server is invalid.')];
		$this->messages['netbios'][1] = ['ERROR', _('Account %s:') . ' dhcp_settings_netbiosServer', _('The Netbios server is invalid.')];
		$this->messages['netbios_node_type'][1] = ['ERROR', _('Account %s:') . ' dhcp_settings_netbiosType', _('The entered Netbios node type does not exist.')];
		$this->messages['unknownClients'][0] = ['ERROR', _('Account %s:') . ' dhcp_settings_unknownClients', _('Please enter a valid option.')];
		$this->messages['max_lease_time'][0] = ['ERROR', _('The maximum lease time is invalid.')];
		$this->messages['max_lease_time'][1] = ['ERROR', _('Account %s:') . ' dhcp_settings_maxLeaseTime', _('The maximum lease time is invalid.')];
		$this->messages['subnet'][0] = ['ERROR', _('The subnet mask is invalid.')];
		$this->messages['subnet'][1] = ['ERROR', _('Account %s:') . ' dhcp_settings_subnetMask', _('The subnet mask is invalid.')];
		$this->messages['ranges_reload'][0] = ['INFO', _('The DHCP ranges were changed to fit for the new subnet.')];
		$this->messages['ips_reload'][0] = ['INFO', _('The fixed IP addresses were changed to fit for the new subnet.')];
		$this->messages['domainname'][2] = ['ERROR', _('The domain name includes invalid characters. Valid characters are A-Z, a-z, 0-9, ".", "_","-".')];
		$this->messages['domainname'][5] = ['ERROR', _('Account %s:') . ' dhcp_settings_domainName', _('The domain name includes invalid characters. Valid characters are A-Z, a-z, 0-9, ".", "_","-".')];
		$this->messages['INFO_domain-search'][0] = ['ERROR', _('Please enter valid domain names for search domains.')];
		$this->messages['INFO_domain-search'][1] = ['ERROR', _('Account %s:') . ' dhcp_settings_domain-search', _('Please enter valid domain names for search domains.')];
		$this->messages['authoritative'][0] = ['ERROR', _('Account %s:') . ' dhcp_settings_authoritative', _('This value can only be "true" or "false".')];
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
	 * <br>"info" are values with informational value (e.g. to be used later by pre/postModify actions)
	 */
	public function save_attributes() {
		// remove dhcpSubnet object class if only the DHCP settings were changed
		if ($this->isRootNode()
			&& !in_array_ignore_case("dhcpSubnet", $this->orig['objectClass'])
			&& in_array_ignore_case("dhcpSubnet", $this->attributes['objectClass'])) {
			$this->attributes['objectClass'] = array_delete(["dhcpSubnet"], $this->attributes['objectClass']);
		}
		return parent::save_attributes();
	}

	/**
	 * Processes user input of the primary module page.
	 * It checks if all input values are correct and updates the associated LDAP attributes.
	 *
	 * @return array list of info/error messages
	 */
	public function process_attributes() {
		// check if DHCP main settings and valid DHCP entry
		if ($this->isRootNode()
			&& !in_array_ignore_case('dhcpService', $this->attributes['objectClass'])
			&& !in_array_ignore_case('dhcpServer', $this->attributes['objectClass'])) {
			return [];
		}
		$errors = [];

		// Check if cn is not empty
		if (!$this->isRootNode()) {
			if (!empty($_POST['cn'])) {
				$_POST['cn'] = trim($_POST['cn']);
			}
			$this->attributes['cn'][0] = $_POST['cn'];
			if (empty($_POST['cn'])) {
				$errors[] = $this->messages['cn'][0];
			}
			// Check, if cn is not already use:
			elseif ((sizeof(searchLDAPByAttribute('cn', $_POST['cn'], 'dhcpOptions', ['cn'], ['dhcp'])) > 0) && $this->orig['cn']['0'] != $_POST['cn']) {
				$errors[] = $this->messages['cn'][1];
			}
			elseif (!check_ip($_POST['cn'], true)) {
				$errors[] = $this->messages['cn'][2];
			}
			else {
				// if the cn was edit, reload the Ranges:
				if ($this->getAccountContainer()->getAccountModule('range')->reload_ranges()) {
					$errors[] = $this->messages['ranges_reload'][0];
				}
				// if the cn was edit, reload the ips:
				if ($this->getAccountContainer()->getAccountModule('fixed_ip')->reload_ips()) {
					$errors[] = $this->messages['ips_reload'][0];
				}
			}
			$this->setUnknownClients($_POST['unknownClients']);
		}

		$this->setAuthoritative(isset($_POST['authoritative']) && ($_POST['authoritative'] === 'on'));

		// Check domainname:
		if (!empty($_POST['domainname'])) {
			$_POST['domainname'] = trim($_POST['domainname']);
		}
		if (!empty($_POST['domainname']) && !preg_match("/^[A-Za-z0-9\\._-]*$/", $_POST['domainname'])) {
			$errors[] = $this->messages['domainname'][2];
		}
		$this->setDHCPOption(self::DOMAIN_NAME, '"' . $_POST['domainname'] . '"');

		// Check DNS
		if (!empty($_POST['dns'])) {
			$_POST['dns'] = trim($_POST['dns']);
			$ex = explode(",", $_POST['dns']);
			$is_first = true;
			$invalid = false;
			foreach ($ex as $string) {
				if ($is_first) {
					$is_first = false;
				}

				if (!check_ip($string)) {
					$invalid = true;
				}
			}
			if ($invalid) {
				$errors[] = $this->messages['dns'][0];
			}
		}
		$this->setDHCPOption(self::DOMAIN_NAME_SERVERS, $_POST['dns']);
		// domain search
		$this->processMultiValueInputTextField('INFO_domain-search', $errors, 'domainname');
		foreach ($this->attributes['INFO_domain-search'] as $key => $domainName) {
			$this->attributes['INFO_domain-search'][$key] = '"' . $domainName . '"';
		}
		$domainSearch = implode(', ', $this->attributes['INFO_domain-search']);
		$this->setDHCPOption(self::DOMAIN_SEARCH, $domainSearch);
		unset($this->attributes['INFO_domain-search']);

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
		if (!empty($_POST['routers'])) {
			$_POST['routers'] = trim($_POST['routers']);
			if (!check_ip($_POST['routers'])) {
				$errors[] = $this->messages['routers'][0];
			}
		}
		$this->setDHCPOption(self::ROUTERS, $_POST['routers']);

		// Netbios
		if (!empty($_POST['netbios'])) {
			$_POST['netbios'] = trim($_POST['netbios']);
		}
		$netbiosServers = explode(', ', $_POST['netbios']);
		$netbiosServersOk = true;
		for ($i = 0; $i < sizeof($netbiosServers); $i++) {
			if (!check_ip($netbiosServers[$i]) && !get_preg($netbiosServers[$i], 'DNSname')) {
				$netbiosServersOk = false;
				break;
			}
		}
		if (!$netbiosServersOk && !empty($_POST['netbios'])) {
			$errors[] = $this->messages['netbios'][0];
		}
		$this->setDHCPOption(self::NETBIOS_NAME_SERVERS, $_POST['netbios']);

		$this->setDHCPOption(self::NETBIOS_NODE_TYPE, $_POST['netbios_node_type']);

		if (!$this->isRootNode()) {
			// Check subnet
			$_POST['subnet'] = trim($_POST['subnet']);
			if (!$this->checkSubnetMask($_POST['subnet'])) {
				$errors[] = $this->messages['subnet'][0];
			}
			$this->setDHCPOption(self::SUBNET_MASK, $_POST['subnet']);

			// calculate netmask from subnet:
			if (!empty($_POST['subnet'])) {
				$this->attributes['dhcpNetMask'][0] = $this->calculateNetMask($_POST['subnet']);
			}
		}

		// description
		$this->attributes['dhcpComments'][0] = $_POST['description'];

		// extra statements
		$extraStatements = isset($_POST['extraStatements']) ? explode("\r\n", $_POST['extraStatements']) : [];
		$this->setExtraStatements($extraStatements);

		// extra options
		$extraOptions = isset($_POST['extraOptions']) ? explode("\r\n", $_POST['extraOptions']) : [];
		$this->setExtraOptions($extraOptions);

		return $errors;
	}

	/**
	 * Checks if the subnet mask is valid.
	 *
	 * @param String $mask subnet mask
	 * @return boolean correct or incorrect
	 */
	private function checkSubnetMask($mask) {
		// check basic format
		if (!check_ip($mask, true)) {
			return false;
		}
		// check if bit order is 11...00...
		$parts = explode('.', $mask);
		$bits = '';
		for ($i = 0; $i < sizeof($parts); $i++) {
			$bits .= decbin($parts[$i]);
		}
		return preg_match('/^1*0*$/', $bits);
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
		foreach ($ex as $mask) {
			$binary = decbin($mask);
			$num += substr_count($binary, 1);
		}
		return $num;
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 *
	 * @return htmlElement HTML meta data
	 */
	public function display_html_attributes() {
		$return = new htmlResponsiveRow();
		// check if DHCP main settings and valid DHCP entry
		if ($this->isRootNode()
			&& !in_array_ignore_case('dhcpService', $this->attributes['objectClass'])
			&& !in_array_ignore_case('dhcpServer', $this->attributes['objectClass'])) {
			$return->add(new htmlStatusMessage('ERROR', _('Please set your LDAP suffix to an LDAP entry with object class "dhcpService" or "dhcpServer".')));
			return $return;
		}
		// Subnet name
		if (!$this->isRootNode()) {
			$cn = '';
			if (isset($this->attributes['cn'][0])) {
				$cn = $this->attributes['cn'][0];
			}
			$subnetInput = new htmlResponsiveInputField(_('Subnet'), 'cn', $cn, 'subnet');
			$subnetInput->setRequired(true);
			$return->add($subnetInput);
		}
		// description
		$description = '';
		if (isset($this->attributes['dhcpComments'][0])) {
			$description = $this->attributes['dhcpComments'][0];
		}
		$return->add(new htmlResponsiveInputField(_('Description'), 'description', $description, 'description'));
		// domain name
		$return->add(new htmlResponsiveInputField(_('Domain name'), 'domainname', $this->getDHCPOption(self::DOMAIN_NAME), 'domainname'));
		// lease Time
		$leasetimeInput = new htmlResponsiveInputField(_('Lease time'), 'lease_time', $this->getDefaultLeaseTime(), 'leasetime');
		$leasetimeInput->setMinimumAndMaximumNumber();
		$return->add($leasetimeInput);
		// max lease time
		$max_leasetimeInput = new htmlResponsiveInputField(_('Maximum lease time'), 'max_lease_time', $this->getMaxLeaseTime(), 'max_leasetime');
		$max_leasetimeInput->setMinimumAndMaximumNumber();
		$return->add($max_leasetimeInput);
		// DNS
		$return->add(new htmlResponsiveInputField(_('DNS'), 'dns', $this->getDHCPOption(self::DOMAIN_NAME_SERVERS), 'dns'));
		// domain search
		$domainSearchEntries = $this->getDHCPOption(self::DOMAIN_SEARCH);
		if ($domainSearchEntries === null) {
			$domainSearchEntries = '';
		}
		$domainSearchEntries = explode(', ', $domainSearchEntries);
		$this->attributes['INFO_domain-search'] = $domainSearchEntries;
		$this->addMultiValueInputTextField($return, 'INFO_domain-search', _('Search domains'));
		// gateway
		$return->add(new htmlResponsiveInputField(_('Default gateway'), 'routers', $this->getDHCPOption(self::ROUTERS), 'gateway'));
		// netbios name servers
		$return->add(new htmlResponsiveInputField(_('Netbios name servers'), 'netbios', $this->getDHCPOption(self::NETBIOS_NAME_SERVERS), 'netbios'));
		// netbios node type
		$nodeType = $this->getDHCPOption(self::NETBIOS_NODE_TYPE);
		if ($nodeType == '') {
			$nodeType = 8;
		}
		$nodeOptions = [];
		foreach ($this->all_netbios_node_types as $key => $value) {
			$nodeOptions[$value] = $key;
		}
		$nodeSelect = new htmlResponsiveSelect('netbios_node_type', $nodeOptions, [$nodeType], _('Netbios node type'), 'netbios_type');
		$nodeSelect->setHasDescriptiveElements(true);
		$return->add($nodeSelect);

		if (!$this->isRootNode()) {
			// unknown clients
			$unknownClients = $this->getUnknownClients();
			if (empty($unknownClients)) {
				$unknownClients = '-';
			}
			$unknownClientsOptions = array_flip($this->allowDenyOptions);
			$unknownClientsSelect = new htmlResponsiveSelect('unknownClients', $unknownClientsOptions, [$unknownClients], _('Unknown clients'), 'unknownClients');
			$unknownClientsSelect->setHasDescriptiveElements(true);
			$return->add($unknownClientsSelect);
			// subnetmask
			$subnetMaskInput = new htmlResponsiveInputField(_('Subnet mask'), 'subnet', $this->getDHCPOption(self::SUBNET_MASK), 'subnetmask');
			$subnetMaskInput->setRequired(true);
			$return->add($subnetMaskInput);
		}

		if (!$this->isRootNode()) {
			// netmask
			$return->addLabel(new htmlOutputText(_('Net mask')));
			$return->addField(new htmlOutputText($this->attributes['dhcpNetMask'][0]));
		}
		if (!$this->isRootNode()) {
			$return->add(new htmlResponsiveInputCheckbox('authoritative', $this->isAuthoritative(), _('Authoritative'), 'authoritative'));
		}
		$return->add(new htmlResponsiveInputTextarea('extraStatements', implode("\r\n", $this->getExtraStatements()), 30, 4, _('Extra statements'), 'extraStatements'));
		$return->add(new htmlResponsiveInputTextarea('extraOptions', implode("\r\n", $this->getExtraOptions()), 30, 4, _('Extra options'), 'extraOptions'));
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

		$this->setDefaultLeaseTime($profile['lease_time'][0]);
		$this->setMaxLeaseTime($profile['max_lease_time'][0]);
		$this->setAuthoritative(isset($profile['authoritative'][0]) && ($profile['authoritative'][0] === 'true'));

		$this->setDHCPOption(self::DOMAIN_NAME, $profile['domainname'][0]);
		$this->setDHCPOption(self::DOMAIN_NAME_SERVERS, $profile['dns'][0]);
		$this->setDHCPOption(self::ROUTERS, $profile[self::ROUTERS][0]);
		$this->setDHCPOption(self::NETBIOS_NAME_SERVERS, $profile['netbios'][0]);
		$this->setDHCPOption(self::NETBIOS_NODE_TYPE, $profile['netbios_node_type'][0]);
		if (!empty($profile[self::DOMAIN_SEARCH][0])) {
			$domains = '"' . $profile[self::DOMAIN_SEARCH][0] . '"';
			$domains = str_replace(';', '", "', $domains);
			$this->setDHCPOption(self::DOMAIN_SEARCH, $domains);
		}

		if (!$this->isRootNode()) {
			$this->setDHCPOption(self::SUBNET_MASK, $profile['subnet'][0]);

			// calc the netmask:
			$ex = explode(".", $profile['subnet'][0]);
			$num = 0;
			foreach ($ex as $mask) {
				$binary = decbin($mask);
				$num += substr_count($binary, 1);
			}
			$this->attributes['dhcpNetMask'][0] = $num;
		}
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	function get_pdfEntries($pdfKeys, $typeId) {
		$nodeType = $this->getDHCPOption(self::NETBIOS_NODE_TYPE);
		$nodeTypeValue = '';
		if (isset($this->all_netbios_node_types[$nodeType])) {
			$nodeTypeValue = $this->all_netbios_node_types[$nodeType];
		}
		$unknownClients = '';
		$unknownClientsVal = $this->getUnknownClients();
		if (!empty($unknownClientsVal)) {
			$unknownClients = $this->allowDenyOptions[$unknownClientsVal];
		}
		$authoritative = $this->isAuthoritative() ? _('Yes') : _('No');
		$return = [];
		$this->addPDFKeyValue($return, 'domainName', _('Domain name'), $this->getDHCPOption(self::DOMAIN_NAME));
		$this->addPDFKeyValue($return, 'leaseTime', _('Lease time'), $this->getDefaultLeaseTime());
		$this->addPDFKeyValue($return, 'maxLeaseTime', _('Maximum lease time'), $this->getMaxLeaseTime());
		$this->addPDFKeyValue($return, 'DNSserver', _('DNS'), $this->getDHCPOption(self::DOMAIN_NAME_SERVERS));
		$this->addPDFKeyValue($return, self::DOMAIN_SEARCH, _('Search domains'), $this->getDHCPOption(self::DOMAIN_SEARCH));
		$this->addPDFKeyValue($return, 'gateway', _('Default gateway'), $this->getDHCPOption(self::ROUTERS));
		$this->addPDFKeyValue($return, 'netbiosServer', _('Netbios name servers'), $this->getDHCPOption(self::NETBIOS_NAME_SERVERS));
		$this->addPDFKeyValue($return, 'netbiosType', _('Netbios node type'), $nodeTypeValue);
		$this->addPDFKeyValue($return, 'subnetMask', _('Subnet mask'), $this->getDHCPOption(self::SUBNET_MASK));
		$this->addPDFKeyValue($return, 'unknownClients', _('Unknown clients'), $unknownClients);
		$this->addSimplePDFField($return, 'description', _('Description'), 'dhcpComments');
		$this->addSimplePDFField($return, 'subnet', _('Subnet'), 'cn');
		$this->addSimplePDFField($return, 'netMask', _('Net mask'), 'dhcpNetMask');
		$this->addPDFKeyValue($return, self::AUTHORITATIVE, _('Authoritative'), $authoritative);
		return $return;
	}

	/**
	 * Returns a DHCP option.
	 *
	 * @param String $name option name
	 * @return String value
	 */
	public function getDHCPOption($name) {
		$return = null;
		if (is_array($this->attributes['dhcpOption'])) {
			for ($i = 0; $i < sizeof($this->attributes['dhcpOption']); $i++) {
				$val = $this->attributes['dhcpOption'][$i];
				if (substr($val, 0, strlen($name) + 1) == ($name . ' ')) {
					$return = substr($val, strlen($name) + 1);
					$return = str_replace('"', '', $return);
					break;
				}
			}
		}
		return $return;
	}

	/**
	 * Sets a DHCP option.
	 *
	 * @param String $name option name
	 * @param String $value option value
	 */
	private function setDHCPOption($name, $value) {
		if (empty($this->attributes['dhcpOption'])) {
			$this->attributes['dhcpOption'] = [];
		}
		for ($i = 0; $i < sizeof($this->attributes['dhcpOption']); $i++) {
			if (substr($this->attributes['dhcpOption'][$i], 0, strlen($name) + 1) == ($name . ' ')) {
				unset($this->attributes['dhcpOption'][$i]);
				$this->attributes['dhcpOption'] = array_values($this->attributes['dhcpOption']);
			}
		}
		if (($value != null) && ($value != '')) {
			$this->attributes['dhcpOption'][] = $name . ' ' . $value;
		}
	}

	/**
	 * Returns the default lease time.
	 *
	 * @return String time
	 */
	private function getDefaultLeaseTime() {
		$return = null;
		if (!empty($this->attributes['dhcpStatements'])) {
			for ($i = 0; $i < sizeof($this->attributes['dhcpStatements']); $i++) {
				if (substr($this->attributes['dhcpStatements'][$i], 0, 19) == (self::DEFAULT_LEASE_TIME . ' ')) {
					$return = substr($this->attributes['dhcpStatements'][$i], 19);
					break;
				}
			}
		}
		return $return;
	}

	/**
	 * Sets the default lease time.
	 *
	 * @param String $time time
	 */
	private function setDefaultLeaseTime($time) {
		if (empty($this->attributes['dhcpStatements'])) {
			$this->attributes['dhcpStatements'] = [];
		}
		for ($i = 0; $i < sizeof($this->attributes['dhcpStatements']); $i++) {
			if (substr($this->attributes['dhcpStatements'][$i], 0, 19) == (self::DEFAULT_LEASE_TIME . ' ')) {
				unset($this->attributes['dhcpStatements'][$i]);
				$this->attributes['dhcpStatements'] = array_values($this->attributes['dhcpStatements']);
			}
		}
		if (($time != null) && ($time != '')) {
			$this->attributes['dhcpStatements'][] = self::DEFAULT_LEASE_TIME . ' ' . $time;
		}
	}

	/**
	 * Returns the unknown clients option.
	 *
	 * @return String unknown clients value
	 */
	private function getUnknownClients() {
		$return = null;
		if (!empty($this->attributes['dhcpStatements'])) {
			for ($i = 0; $i < sizeof($this->attributes['dhcpStatements']); $i++) {
				$val = $this->attributes['dhcpStatements'][$i];
				if (strpos($val, self::UNKNOWN_CLIENTS) === (strlen($val) - strlen(self::UNKNOWN_CLIENTS))) {
					$return = substr($val, 0, (strlen($val) - strlen(self::UNKNOWN_CLIENTS) - 1));
					break;
				}
			}
		}
		return $return;
	}

	/**
	 * Sets the unknown clients option.
	 *
	 * @param String $option allow/deny
	 */
	private function setUnknownClients($option) {
		if (!isset($this->attributes['dhcpStatements'])) {
			$this->attributes['dhcpStatements'] = [];
		}
		for ($i = 0; $i < sizeof($this->attributes['dhcpStatements']); $i++) {
			$val = $this->attributes['dhcpStatements'][$i];
			if (strpos($val, self::UNKNOWN_CLIENTS) === (strlen($val) - strlen(self::UNKNOWN_CLIENTS))) {
				unset($this->attributes['dhcpStatements'][$i]);
				$this->attributes['dhcpStatements'] = array_values($this->attributes['dhcpStatements']);
			}
		}
		if (!empty($option) && ($option != '-')) {
			$this->attributes['dhcpStatements'][] = $option . ' ' . self::UNKNOWN_CLIENTS;
		}
	}

	/**
	 * Returns the maximum lease time.
	 *
	 * @return String time
	 */
	private function getMaxLeaseTime() {
		$return = null;
		if (!empty($this->attributes['dhcpStatements'])) {
			for ($i = 0; $i < sizeof($this->attributes['dhcpStatements']); $i++) {
				if (substr($this->attributes['dhcpStatements'][$i], 0, 15) == (self::MAX_LEASE_TIME . ' ')) {
					$return = substr($this->attributes['dhcpStatements'][$i], 15);
					break;
				}
			}
		}
		return $return;
	}

	/**
	 * Sets the maximum lease time.
	 *
	 * @param String $time time
	 */
	private function setMaxLeaseTime($time) {
		if (!is_array($this->attributes['dhcpStatements'])) {
			$this->attributes['dhcpStatements'] = [];
		}
		for ($i = 0; $i < sizeof($this->attributes['dhcpStatements']); $i++) {
			if (substr($this->attributes['dhcpStatements'][$i], 0, 15) == (self::MAX_LEASE_TIME . ' ')) {
				unset($this->attributes['dhcpStatements'][$i]);
				$this->attributes['dhcpStatements'] = array_values($this->attributes['dhcpStatements']);
			}
		}
		if (($time != null) && ($time != '')) {
			$this->attributes['dhcpStatements'][] = self::MAX_LEASE_TIME . ' ' . $time;
		}
	}

	/**
	 * Returns if the subnet is authoritative.
	 *
	 * @return bool authoritative
	 */
	private function isAuthoritative(): bool {
		if (!empty($this->attributes['dhcpStatements'])) {
			for ($i = 0; $i < sizeof($this->attributes['dhcpStatements']); $i++) {
				if ($this->attributes['dhcpStatements'][$i] === self::AUTHORITATIVE) {
					return true;
				}
			}
		}
		return false;
	}

	/**
	 * Sets if the subnet is authoritative.
	 *
	 * @param bool $isAuthoritative authoritative
	 */
	private function setAuthoritative(bool $isAuthoritative): void {
		if (!is_array($this->attributes['dhcpStatements'])) {
			$this->attributes['dhcpStatements'] = [];
		}
		for ($i = 0; $i < sizeof($this->attributes['dhcpStatements']); $i++) {
			if ($this->attributes['dhcpStatements'][$i] === self::AUTHORITATIVE) {
				unset($this->attributes['dhcpStatements'][$i]);
				$this->attributes['dhcpStatements'] = array_values($this->attributes['dhcpStatements']);
				break;
			}
		}
		if ($isAuthoritative) {
			$this->attributes['dhcpStatements'][] = self::AUTHORITATIVE;
		}
	}

	/**
	 * Returns a list of extra DHCP statements.
	 *
	 * @return array statements
	 */
	private function getExtraStatements(): array {
		$extraStatements = [];
		if (!empty($this->attributes['dhcpStatements'])) {
			foreach ($this->attributes['dhcpStatements'] as $statement) {
				if (($statement !== self::AUTHORITATIVE)
					&& (!str_starts_with($statement, self::DEFAULT_LEASE_TIME))
					&& (!str_starts_with($statement, self::MAX_LEASE_TIME))
					&& (strpos($statement, self::UNKNOWN_CLIENTS) !== (strlen($statement) - strlen(self::UNKNOWN_CLIENTS)))) {
					$extraStatements[] = $statement;
				}
			}
		}
		return $extraStatements;
	}

	/**
	 * Sets extra DHCP statements.
	 *
	 * @param array $statements statements
	 */
	private function setExtraStatements(array $statements): void {
		if (!is_array($this->attributes['dhcpStatements'])) {
			$this->attributes['dhcpStatements'] = [];
		}
		$toDelete = [];
		foreach ($this->attributes['dhcpStatements'] as $statement) {
			if (($statement !== self::AUTHORITATIVE)
				&& (!str_starts_with($statement, self::DEFAULT_LEASE_TIME))
				&& (!str_starts_with($statement, self::MAX_LEASE_TIME))
				&& (strpos($statement, self::UNKNOWN_CLIENTS) !== (strlen($statement) - strlen(self::UNKNOWN_CLIENTS)))) {
				$toDelete[] = $statement;
			}
		}
		$this->attributes['dhcpStatements'] = array_delete($toDelete, $this->attributes['dhcpStatements']);
		$this->attributes['dhcpStatements'] = array_merge($this->attributes['dhcpStatements'], $statements);
	}

	/**
	 * Returns a list of extra DHCP options.
	 *
	 * @return array options
	 */
	private function getExtraOptions(): array {
		$extraOptions = [];
		if (!empty($this->attributes['dhcpOption'])) {
			foreach ($this->attributes['dhcpOption'] as $option) {
				if ((!str_starts_with($option, self::DOMAIN_NAME))
					&& (!str_starts_with($option, self::DOMAIN_NAME_SERVERS))
					&& (!str_starts_with($option, self::DOMAIN_SEARCH))
					&& (!str_starts_with($option, self::ROUTERS))
					&& (!str_starts_with($option, self::NETBIOS_NAME_SERVERS))
					&& (!str_starts_with($option, self::NETBIOS_NODE_TYPE))
					&& (!str_starts_with($option, self::SUBNET_MASK))
				) {
					$extraOptions[] = $option;
				}
			}
		}
		return $extraOptions;
	}

	/**
	 * Sets extra DHCP options.
	 *
	 * @param array $options options
	 */
	private function setExtraOptions(array $options): void {
		if (!is_array($this->attributes['dhcpOption'])) {
			$this->attributes['dhcpOption'] = [];
		}
		$toDelete = [];
		foreach ($this->attributes['dhcpOption'] as $option) {
			if ((!str_starts_with($option, self::DOMAIN_NAME))
				&& (!str_starts_with($option, self::DOMAIN_NAME_SERVERS))
				&& (!str_starts_with($option, self::DOMAIN_SEARCH))
				&& (!str_starts_with($option, self::ROUTERS))
				&& (!str_starts_with($option, self::NETBIOS_NAME_SERVERS))
				&& (!str_starts_with($option, self::NETBIOS_NODE_TYPE))
				&& (!str_starts_with($option, self::SUBNET_MASK))
			) {
				$toDelete[] = $option;
			}
		}
		$this->attributes['dhcpOption'] = array_delete($toDelete, $this->attributes['dhcpOption']);
		$this->attributes['dhcpOption'] = array_merge($this->attributes['dhcpOption'], $options);
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::build_uploadAccounts()
	 */
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
		$messages = [];
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array("dhcpOptions", $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = "dhcpOptions";
			}
			if (!in_array("dhcpSubnet", $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = "dhcpSubnet";
			}
			// subnet
			if (check_ip($rawAccounts[$i][$ids['dhcp_settings_subnet']], true)) {
				$partialAccounts[$i]['cn'] = $rawAccounts[$i][$ids['dhcp_settings_subnet']];
			}
			else {
				$error = $this->messages['cn'][3];
				$error[] = [$i];
				$messages[] = $error;
			}
			// domain name
			if ($rawAccounts[$i][$ids['dhcp_settings_domainName']] != "") {
				if (preg_match("/^[A-Za-z0-9\\._-]*$/", $rawAccounts[$i][$ids['dhcp_settings_domainName']])) {
					$partialAccounts[$i]['dhcpOption'][] = self::DOMAIN_NAME . ' "' . $rawAccounts[$i][$ids['dhcp_settings_domainName']] . '"';
				}
				else {
					$error = $this->messages['domainname'][5];
					$error[] = [$i];
					$messages[] = $error;
				}
			}
			// lease time
			if ($rawAccounts[$i][$ids['dhcp_settings_leaseTime']] != '') {
				if (is_numeric($rawAccounts[$i][$ids['dhcp_settings_leaseTime']])) {
					$partialAccounts[$i]['dhcpStatements'][] = self::DEFAULT_LEASE_TIME . ' ' . $rawAccounts[$i][$ids['dhcp_settings_leaseTime']];
				}
				else {
					$error = $this->messages['lease_time'][1];
					$error[] = [$i];
					$messages[] = $error;
				}
			}
			// max lease time
			if ($rawAccounts[$i][$ids['dhcp_settings_maxLeaseTime']] != '') {
				if (is_numeric($rawAccounts[$i][$ids['dhcp_settings_maxLeaseTime']])) {
					$partialAccounts[$i]['dhcpStatements'][] = self::MAX_LEASE_TIME . ' ' . $rawAccounts[$i][$ids['dhcp_settings_maxLeaseTime']];
				}
				else {
					$error = $this->messages['max_lease_time'][1];
					$error[] = [$i];
					$messages[] = $error;
				}
			}
			// DNS
			if ($rawAccounts[$i][$ids['dhcp_settings_DNSserver']] != '') {
				$ex = explode(",", $rawAccounts[$i][$ids['dhcp_settings_DNSserver']]);
				$invalid = false;
				foreach ($ex as $string) {
					if (!check_ip($string)) {
						$invalid = true;
					}
				}
				if (!$invalid) {
					$partialAccounts[$i]['dhcpOption'][] = self::DOMAIN_NAME_SERVERS . ' ' . $rawAccounts[$i][$ids['dhcp_settings_DNSserver']];
				}
				else {
					$error = $this->messages['dns'][1];
					$error[] = [$i];
					$messages[] = $error;
				}
			}
			// search domains
			if ($rawAccounts[$i][$ids['dhcp_settings_domain-search']] != '') {
				$domains = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['dhcp_settings_domain-search']]);
				$invalid = false;
				foreach ($domains as $key => $name) {
					if (!get_preg($name, 'domainname')) {
						$invalid = true;
					}
					else {
						$domains[$key] = '"' . $name . '"';
					}
				}
				if (!$invalid) {
					$partialAccounts[$i]['dhcpOption'][] = self::DOMAIN_SEARCH . ' ' . implode(', ', $domains);
				}
				else {
					$error = $this->messages['INFO_domain-search'][1];
					$error[] = [$i];
					$messages[] = $error;
				}
			}
			// gateway
			if ($rawAccounts[$i][$ids['dhcp_settings_gateway']] != '') {
				if (check_ip($rawAccounts[$i][$ids['dhcp_settings_gateway']])) {
					$partialAccounts[$i]['dhcpOption'][] = self::ROUTERS . ' ' . $rawAccounts[$i][$ids['dhcp_settings_gateway']];
				}
				else {
					$error = $this->messages['routers'][1];
					$error[] = [$i];
					$messages[] = $error;
				}
			}
			// netbios name servers
			if ($rawAccounts[$i][$ids['dhcp_settings_netbiosServer']] != '') {
				$ex = explode(",", $rawAccounts[$i][$ids['dhcp_settings_netbiosServer']]);
				$invalid = false;
				foreach ($ex as $string) {
					if (!check_ip($string) && !get_preg($string, 'DNSname')) {
						$invalid = true;
					}
				}
				if (!$invalid) {
					$partialAccounts[$i]['dhcpOption'][] = self::NETBIOS_NAME_SERVERS . ' ' . $rawAccounts[$i][$ids['dhcp_settings_netbiosServer']];
				}
				else {
					$error = $this->messages['netbios'][1];
					$error[] = [$i];
					$messages[] = $error;
				}
			}
			// node type
			if ($rawAccounts[$i][$ids['dhcp_settings_netbiosType']] != '') {
				if (in_array($rawAccounts[$i][$ids['dhcp_settings_netbiosType']], ['B', 'P', 'M', 'H'])) {
					if ($rawAccounts[$i][$ids['dhcp_settings_netbiosType']] == 'B') {
						$partialAccounts[$i]['dhcpOption'][] = self::NETBIOS_NODE_TYPE . ' 1';
					}
					elseif ($rawAccounts[$i][$ids['dhcp_settings_netbiosType']] == 'P') {
						$partialAccounts[$i]['dhcpOption'][] = self::NETBIOS_NODE_TYPE . ' 2';
					}
					elseif ($rawAccounts[$i][$ids['dhcp_settings_netbiosType']] == 'M') {
						$partialAccounts[$i]['dhcpOption'][] = self::NETBIOS_NODE_TYPE . ' 4';
					}
					elseif ($rawAccounts[$i][$ids['dhcp_settings_netbiosType']] == 'H') {
						$partialAccounts[$i]['dhcpOption'][] = self::NETBIOS_NODE_TYPE . ' 8';
					}
				}
				else {
					$error = $this->messages['netbios_node_type'][1];
					$error[] = [$i];
					$messages[] = $error;
				}
			}
			else {
				$partialAccounts[$i]['dhcpOption'][] = self::NETBIOS_NODE_TYPE . ' 8'; // default H
			}
			// unknown clients
			if (!empty($rawAccounts[$i][$ids['dhcp_settings_unknownClients']])) {
				$unknownClients = $rawAccounts[$i][$ids['dhcp_settings_unknownClients']];
				if (in_array($unknownClients, $this->allowDenyOptions)) {
					$allowDenyOptions = array_flip($this->allowDenyOptions);
					$partialAccounts[$i]['dhcpStatements'][] = $allowDenyOptions[$unknownClients] . ' ' . self::UNKNOWN_CLIENTS;
				}
				else {
					$error = $this->messages['unknownClients'][0];
					$error[] = [$i];
					$messages[] = $error;
				}
			}
			// node type
			if ($rawAccounts[$i][$ids['dhcp_settings_authoritative']] !== '') {
				if (in_array($rawAccounts[$i][$ids['dhcp_settings_authoritative']], ['true', 'false'])) {
					if ($rawAccounts[$i][$ids['dhcp_settings_authoritative']] === 'true') {
						$partialAccounts[$i]['dhcpStatements'][] = self::AUTHORITATIVE;
					}
				}
				else {
					$error = $this->messages['authoritative'][0];
					$error[] = [$i];
					$messages[] = $error;
				}
			}
			// subnet mask
			if (check_ip($rawAccounts[$i][$ids['dhcp_settings_subnetMask']], true)) {
				$partialAccounts[$i]['dhcpOption'][] = self::SUBNET_MASK . ' ' . $rawAccounts[$i][$ids['dhcp_settings_subnetMask']];
			}
			else {
				$error = $this->messages['subnet'][1];
				$error[] = [$i];
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

	/**
	 * Returns if the current DN is the root entry.
	 *
	 * @return bool is root
	 */
	private function isRootNode() {
		$rootSuffix = $this->getAccountContainer()->get_type()->getSuffix();
		return $this->getAccountContainer()->dn_orig == $rootSuffix;
	}

	/**
	 * @inheritDoc
	 */
	public function getListAttributeDescriptions(ConfiguredType $type): array {
		return [
			"cn" => _("Subnet"),
			'dhcpcomments' => _('Description'),
		];
	}

}
