<?php

use LAM\TYPES\ConfiguredType;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2008         Thomas Manninger
                2008 - 2026  Roland Gruber

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

	/**
	 * {@inheritDoc}
	 */
	public function can_manage(): bool {
		return $this->get_scope() === 'dhcp';
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_metaData(): array {
		$return = [];
		// alias name
		$return["alias"] = _("DDNS");
		// this is a base module
		$return["is_base"] = false;
		// icon
		$return['icon'] = 'world-color.svg';
		// RDN attribute
		$return["RDN"] = ["cn" => "high"];
		// LDAP filter
		$return["ldap_filter"] = [];
		// module dependencies
		$return['dependencies'] = ['depends' => ['dhcp_settings'], 'conflicts' => []];
		// managed object classes
		$return['objectClasses'] = [];
		// managed attributes
		$return['attributes'] = ['dhcpOption', 'dhcpStatements'];
		// help Entries
		$return['help'] = [
			'active' => [
				"Headline" => _("Activate DynDNS"),
				"Text" => _("Should DDNS (Dynamic DNS) be activated?")
			],
			'fixed_ips' => [
				"Headline" => _("Fix IP addresses"),
				"Text" => _("Should fix IP addresses be added to the DNS server?")
			],
			'client_insert' => [
				"Headline" => _("Disable client updates"),
				"Text" => _("Disables the client to update DNS entries.")
			],
			'keypath' => [
				"Headline" => _("Path to key for DNS updates"),
				"Text" => _("The key enables the DHCP server to perform DNS updates." .
					" " . "The key is generated with \"genDDNSkey\".")
			],
			'dns' => [
				"Headline" => _("IP address of the DNS server"),
				"Text" => _("Please enter the IP address of your DNS server.")
			],
			'zone' => [
				"Headline" => _("Zone name"),
				"Text" => _("Zone name for the DNS server (e.g. company.local).")
			],
			'zone_reverse' => [
				"Headline" => _("Reverse zone name"),
				"Text" => ("Name of the reverse zone of the DNS server (e.g. 0.168.192.in-addr.arpa).")
			],
		];
		// available PDF fields
		$return['PDF_fields'] = [
			'DNSserver' => _('IP address of the DNS server'),
			'zone' => _('Zone name'),
			'reverseZone' => _('Reverse zone name'),
		];
		// upload fields
		if (isLoggedIn() && $this->check_if_ddns_is_enable()) {
			$return['upload_columns'] = [
				[
					'name' => 'ddns_DNSserver',
					'description' => _('IP address of the DNS server'),
					'help' => 'dns',
					'example' => '123.123.123.123',
					'required' => true
				],
				[
					'name' => 'ddns_zone',
					'description' => _('Zone name'),
					'help' => 'zone',
					'example' => 'company.local',
					'required' => true
				],
				[
					'name' => 'ddns_reverseZone',
					'description' => _('Reverse zone name'),
					'help' => 'zone_reverse',
					'example' => '0.168.192.in-addr.arpa',
					'required' => true
				],
			];
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	protected function load_Messages(): void {
		$this->messages['key_path'][0] = ['ERROR', 'Please enter the path to the key.', ''];
		$this->messages['key_path'][1] = ['ERROR', 'The key path contains invalid characters.', ''];
		$this->messages['ip'][0] = ['ERROR', 'The IP address of the DNS server is invalid.'];
		$this->messages['ip'][1] = ['ERROR', _('Account %s:') . ' ddns_DNSserver', 'The IP address of the DNS server is invalid.'];
		$this->messages['zone'][0] = ['ERROR', 'Please enter a zone name.'];
		$this->messages['zone_reverse'][0] = ['ERROR', 'Please enter the reverse zone.'];
	}

	/**
	 * {@inheritDoc}
	 */
	public function module_complete(): bool {
		$this->attributes = &$this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes;
		$this->orig = &$this->getAccountContainer()->getAccountModule('dhcp_settings')->orig;
		if ($this->isRootNode()) {
			// check if DHCP main settings and valid DHCP entry
			if (!in_array_ignore_case('dhcpService', $this->attributes['objectClass']) && !in_array_ignore_case('dhcpServer', $this->attributes['objectClass'])) {
				return false;
			}
		}
		else {
			if (!$this->check_if_ddns_is_enable()) {
				return true;
			}
			// Account settings
			$ip = $this->getDNSServer();
			if (!empty($ip) && !check_ip($ip)) {
				return false;
			}

			$zones = $this->getZoneNames();
			if (count($zones) < 2) {
				return false;
			}
		}

		return true;
	}

	/**
	 * Returns if the current DN is the root entry.
	 *
	 * @return bool is root
	 */
	private function isRootNode(): bool {
		$rootSuffix = $this->getAccountContainer()->get_type()->getSuffix();
		return $this->getAccountContainer()->dn_orig == $rootSuffix;
	}

	/**
	 * {@inheritDoc}
	 */
	public function save_attributes(): array {
		// done by dhcp_server object
		return [];
	}

	/**
	 * This function check if ddns is enabled.
	 *
	 * @return bool is enabled
	 */
	private function check_if_ddns_is_enable(): bool {
		if ($this->getAccountContainer() == null) {
			return false;
		}
		$type = $this->getAccountContainer()->get_type();
		if ($type == null) {
			return false;
		}
		$suffix = $type->getSuffix();

		$results = searchLDAP($suffix, "dhcpStatements=ddns-update-style interim", ['dn']);
		return (!empty($results));
	}

	/**
	 * {@inheritDoc}
	 */
	public function process_attributes(): array {
		// Main Settings and Account have to different processes.
		if ($this->isRootNode()) {
			// main settings:
			$errors = $this->process_attributes_mainSettings();
		}
		else {
			// account
			if (!$this->check_if_ddns_is_enable()) {
				return [];
			}
			$errors = $this->process_attributes_account();
		}
		return $errors;
	}

	/**
	 * Process for mainsettings
	 *
	 * @return array<string|string[]>[] messages
	 */
	public function process_attributes_mainSettings(): array {
		if (!in_array_ignore_case('dhcpService', $this->attributes['objectClass']) && !in_array_ignore_case('dhcpServer', $this->attributes['objectClass'])) {
			return [];
		}
		$errors = [];

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
		if (!empty($key_path) && (str_replace("\"", "", $_POST['key_path']) != $key_path)) {
			$errors[] = $this->messages['key_path'][1];
		}

		return $errors;
	}

	/**
	 * Process for account
	 *
	 * @return array<string|string[]>[] messages
	 */
	public function process_attributes_account(): array {
		$errors = [];

		$ip = trim($_POST['ip']);
		$zone = trim($_POST['zone']);
		$zone_reverse = trim($_POST['zone_reverse']);

		// ip correct???
		if (!empty($ip) && !check_ip($ip)) {
			$errors[] = $this->messages['ip'][0];
		}

		for ($i = 0; $i < count($this->attributes['dhcpStatements']); $i++) {
			if (str_starts_with($this->attributes['dhcpStatements'][$i], 'zone ')) {
				unset($this->attributes['dhcpStatements'][$i]);
				$this->attributes['dhcpStatements'] = array_values($this->attributes['dhcpStatements']);
				$i--;
			}
		}

		// Zone inserted?
		if (!empty($zone)) {
			if (!empty($ip)) {
				$this->attributes['dhcpStatements'][] = "zone {$zone}. { primary {$ip}; key DHCP_UPDATER; }";
			}
			else {
				$this->attributes['dhcpStatements'][] = "zone {$zone}. { key DHCP_UPDATER; }";
			}
		}
		elseif (!empty($ip)) {
			$errors[] = $this->messages['zone'][0];
		}

		// Zone reverse inserted?
		if (!empty($zone_reverse)) {
			if (!empty($ip)) {
				$this->attributes['dhcpStatements'][] = "zone {$zone_reverse}. { primary {$ip}; key DHCP_UPDATER; }";
			}
			else {
				$this->attributes['dhcpStatements'][] = "zone {$zone_reverse}. { key DHCP_UPDATER; }";
			}
		}
		elseif (!empty($ip)) {
			$errors[] = $this->messages['zone_reverse'][0];
		}
		return $errors;
	}

	/**
	 * {@inheritDoc}
	 */
	public function display_html_attributes(): htmlElement|array {
		$this->attributes = &$this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes;
		$this->orig = &$this->getAccountContainer()->getAccountModule('dhcp_settings')->orig;
		$return = new htmlResponsiveRow();
		// check if DHCP main settings and valid DHCP entry
		if ($this->isRootNode()) {
			if (!in_array_ignore_case('dhcpService', $this->attributes['objectClass']) && !in_array_ignore_case('dhcpServer', $this->attributes['objectClass'])) {
				$return->add(new htmlStatusMessage('ERROR', _('Please set your LDAP suffix to an LDAP entry with object class "dhcpService" or "dhcpServer".')));
				return $return;
			}
			// DHCP main settings
			$return->add(new htmlResponsiveInputCheckbox('active', $this->isDynDNSActivated(), _('Activate DynDNS'), 'active'));
			$return->add(new htmlResponsiveInputCheckbox('insert_fixed', $this->addFixIPs(), _('Add fix IP addresses to DNS'), 'fixed_ips'));
			$return->add(new htmlResponsiveInputCheckbox('client_insert', $this->isIgnoreClientUpdates(), _('Disable client updates'), 'client_insert'));
			$keyInput = new htmlResponsiveInputField(_('Path to key for DNS updates'), 'key_path', $this->getUpdateKey(), 'keypath');
			$return->add($keyInput);
		}
		elseif (!$this->check_if_ddns_is_enable()) {
			// Account edit
			$return->add(new htmlOutputText(_("DDNS is not activated. You can activate it in the DHCP settings (DDNS).")));
		}
		else {
			// DNS server
			$serverInput = new htmlResponsiveInputField(_('IP address of the DNS server'), 'ip', $this->getDNSServer(), 'dns');
			$return->add($serverInput);

			$zones = $this->getZoneNames();
			$zone = '';
			$revzone = '';
			if (isset($zones[0])) {
				$zone = $zones[0];
			}
			if (isset($zones[1])) {
				$revzone = $zones[1];
			}
			// zone name
			$zoneInput = new htmlResponsiveInputField(_('Zone name'), 'zone', $zone, 'zone');
			$return->add($zoneInput);
			// reverse zone name
			$revZoneInput = new htmlResponsiveInputField(_('Reverse zone name'), 'zone_reverse', $revzone, 'zone_reverse');
			$return->add($revZoneInput);
		}

		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_pdfEntries(array $pdfKeys, string $typeId): array {
		// attributes are taken from DHCP server object
		$this->attributes = &$this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes;
		$this->orig = &$this->getAccountContainer()->getAccountModule('dhcp_settings')->orig;
		$zones = $this->getZoneNames();
		$zone = '';
		$revzone = '';
		if (isset($zones[0])) {
			$zone = $zones[0];
		}
		if (isset($zones[1])) {
			$revzone = $zones[1];
		}
		$result = [];
		$this->addPDFKeyValue($result, 'DNSserver', _('IP address of the DNS server'), $this->getDNSServer());
		$this->addPDFKeyValue($result, 'zone', _('Zone name'), $zone);
		$this->addPDFKeyValue($result, 'reverseZone', _('Reverse zone name'), $revzone);
		return $result;
	}

	/**
	 * Returns the IP of the DNS server.
	 *
	 * @return string|null IP address
	 */
	private function getDNSServer() {
		if (isset($this->attributes['dhcpStatements'][0])) {
			for ($i = 0; $i < count($this->attributes['dhcpStatements']); $i++) {
				if (str_starts_with($this->attributes['dhcpStatements'][$i], 'zone ')) {
					if (!str_contains($this->attributes['dhcpStatements'][$i], ' primary ')) {
						continue;
					}
					$parts = explode(". { primary ", $this->attributes['dhcpStatements'][$i]);
					$temp = array_pop($parts);
					$temp = explode(";", $temp);
					return array_shift($temp);
				}
			}
		}
		return null;
	}

	/**
	 * Returns the zone names.
	 *
	 * @return string[] zone names array(zone, reverse zone)
	 */
	private function getZoneNames(): array {
		$return = [];
		$zone = '';
		$revZone = '';
		if (is_array($this->attributes['dhcpStatements'])) {
			for ($i = 0; $i < count($this->attributes['dhcpStatements']); $i++) {
				if (str_starts_with($this->attributes['dhcpStatements'][$i], 'zone ')) {
					$parts = explode(" ", substr($this->attributes['dhcpStatements'][$i], 5));
					$value = substr(array_shift($parts), 0, -1);
					if (!str_contains($value, 'in-addr.arpa')) {
						$zone = $value;
					}
					else {
						$revZone = $value;
					}
				}
			}
		}
		$return[0] = $zone;
		$return[1] = $revZone;
		return $return;
	}

	/**
	 * Returns if DDNS is activated.
	 *
	 * @return bool activated
	 */
	private function isDynDNSActivated(): bool {
		$return = false;
		if (is_array($this->attributes['dhcpStatements'])) {
			for ($i = 0; $i < count($this->attributes['dhcpStatements']); $i++) {
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
	 * @param bool $activated activated
	 */
	private function setDynDNSActivated($activated): void {
		if (is_array($this->attributes['dhcpStatements'])) {
			for ($i = 0; $i < count($this->attributes['dhcpStatements']); $i++) {
				if (str_starts_with($this->attributes['dhcpStatements'][$i], 'ddns-update-style ')) {
					unset($this->attributes['dhcpStatements'][$i]);
					$this->attributes['dhcpStatements'] = array_values($this->attributes['dhcpStatements']);
				}
			}
		}
		$this->attributes['dhcpStatements'][] = $activated ? 'ddns-update-style interim' : 'ddns-update-style none';
	}

	/**
	 * Returns if fixed IPs are added to DDNS.
	 *
	 * @return bool add fixed IPs
	 */
	private function addFixIPs(): bool {
		$return = false;
		if (is_array($this->attributes['dhcpStatements'])) {
			for ($i = 0; $i < count($this->attributes['dhcpStatements']); $i++) {
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
	 * @param bool $add add fixed IPs
	 */
	private function setFixIPs($add): void {
		if (is_array($this->attributes['dhcpStatements'])) {
			for ($i = 0; $i < count($this->attributes['dhcpStatements']); $i++) {
				if (str_starts_with($this->attributes['dhcpStatements'][$i], 'update-static-leases ')) {
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
	 * @return bool ignore client updates
	 */
	private function isIgnoreClientUpdates(): bool {
		$return = false;
		if (is_array($this->attributes['dhcpStatements'])) {
			for ($i = 0; $i < count($this->attributes['dhcpStatements']); $i++) {
				if (preg_replace('/[ ]+/', ' ', $this->attributes['dhcpStatements'][$i]) == 'ignore client-updates') {
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
	 * @param boolean $ignore ignore client updates
	 */
	private function setIgnoreClientUpdates($ignore): void {
		if (is_array($this->attributes['dhcpStatements'])) {
			for ($i = 0; $i < count($this->attributes['dhcpStatements']); $i++) {
				if (preg_replace('/[ ]+/', ' ', $this->attributes['dhcpStatements'][$i]) == 'ignore client-updates') {
					if ($ignore) {
						return;  // option already set, no change
					}
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
			for ($i = 0; $i < count($this->attributes['dhcpStatements']); $i++) {
				if (str_starts_with($this->attributes['dhcpStatements'][$i], 'include ')) {
					$return = substr($this->attributes['dhcpStatements'][$i], 9, strlen($this->attributes['dhcpStatements'][$i]) - 10);
					break;
				}
			}
		}
		return $return;
	}

	/**
	 * Sets the key for DNS updates.
	 *
	 * @param string|null $key key
	 */
	private function setUpdateKey($key): void {
		if (!is_array($this->attributes['dhcpStatements'])) {
			$this->attributes['dhcpStatements'] = [];
		}
		for ($i = 0; $i < count($this->attributes['dhcpStatements']); $i++) {
			if (str_starts_with($this->attributes['dhcpStatements'][$i], 'include ')) {
				unset($this->attributes['dhcpStatements'][$i]);
				$this->attributes['dhcpStatements'] = array_values($this->attributes['dhcpStatements']);
			}
		}
		if (($key !== null) && ($key !== '')) {
			$this->attributes['dhcpStatements'][] = 'include "' . $key . '"';
		}
	}

	/**
	 * {@inheritDoc}
	 */
	public function load_attributes(array $attributes): void {
		// load nothing, attributes are saved in "dhcp_settings" module
	}

	/**
	 * {@inheritDoc}
	 */
	public function build_uploadAccounts(array $rawAccounts, array $ids, array &$partialAccounts, array $selectedModules, ConfiguredType &$type): array {
		$messages = [];
		if (!$this->check_if_ddns_is_enable()) {
			return $messages;
		}
		for ($i = 0; $i < count($rawAccounts); $i++) {
			// principal name
			if (!check_ip($rawAccounts[$i][$ids['ddns_DNSserver']])) {
				$error = $this->messages['ip'][1];
				$error[] = [$i];
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
