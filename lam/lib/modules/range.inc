<?php

use LAM\PDF\PDFTable;
use LAM\PDF\PDFTableCell;
use LAM\PDF\PDFTableRow;
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
 * Manages DHCP ranges for DHCP server.
 *
 * @package modules
 *
 * @author Thomas Manninger
 * @author Roland Gruber
 */

/**
 * Manages DHCP ranges for DHCP server.
 *
 * @package modules
 */
class range extends baseModule {

	/** @var array<int, array<string, string>> $ranges ranges */
	public array $ranges = [];

	/** Range -> Function attribute_processed already running? */
	public bool $processed = false;

	/** @var int[] $overlapped For check, if IPs overlapped. */
	private array $overlapped = [];

	/** @var array<int, array<string, string[]|string>> $poolsOrig list of pools that currently exist in LDAP */
	private array $poolsOrig = [];
	/** @var array<int, array<string, string[]|string>> $poolsNew list of pools that need to be updated in LDAP */
	private array $poolsNew = [];

	/**
	 * {@inheritDoc}
	 */
	public function can_manage(): bool {
		return $this->get_scope() == 'dhcp';
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_metaData(): array {
		$return = [];
		// alias name
		$return["alias"] = _("Ranges");
		// this is a base module
		$return["is_base"] = false;
		// icon
		$return['icon'] = 'world-color.svg';
		// RDN attribute
		$return["RDN"] = ["cn" => "high"];
		// LDAP filter
		$return["ldap_filter"] = ['or' => "(objectClass=dhcpSubnet)"];
		// module dependencies
		$return['dependencies'] = ['depends' => ['dhcp_settings'], 'conflicts' => []];
		// managed object classes
		$return['objectClasses'] = [];
		// managed attributes
		$return['attributes'] = ['dhcpRange'];
		// help Entries
		$return['help'] = [
			'range_from' => [
				"Headline" => _("Range from"),
				"Text" => _("The starting IP address of the range.")
			],
			'range_to' => [
				"Headline" => _("Range to"),
				"Text" => _("The ending IP address of the range.")
			],
			'poolName' => [
				"Headline" => _("Name"), 'attr' => 'cn',
				"Text" => _("The pool's name.")
			],
			'poolPeer' => [
				"Headline" => _("Failover peer"), 'attr' => 'dhcpStatements',
				"Text" => _("The pool's failover peer.")
			],
		];

		// available PDF fields
		$return['PDF_fields'] = ['ranges' => _('Ranges')];
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	protected function load_Messages(): void {
		$this->messages['range_errors'][0] = ['ERROR', _('One or more errors occurred. The invalid fields are marked.'), ''];
		$this->messages['add_range'][0] = ['ERROR', _('New range'), _('Adding the range failed because errors occurred.')];
		$this->messages['drop_range'][0] = ['ERROR', _('Delete range'), _('It is not possible to delete all ranges.')];
		$this->messages['pool_cn'][0] = ['ERROR', _('Name'), _('Please enter a pool name.')];
		$this->messages['pool_range'][0] = ['ERROR', _('Please enter at least one range for pool "%s".'), null];
	}

	/**
	 * Calculates the subnet for a given IP and netmask.
	 *
	 * @param string $ip IP address
	 * @param string $mask network mask
	 * @return string long IP
	 */
	private static function calculateSubnet(string $ip, string $mask): string {
		return long2ip(ip2long($ip) & ip2long($mask));
	}

	/**
	 * Checks if the first IP is smaller than the second IP.
	 *
	 * @param String $first_ip first ip
	 * @param String $second_ip second ip
	 *
	 * @return bool true, if it's a valid Range, else false;
	 **/

	public function check_range($first_ip, $second_ip): bool {
		$ex_first = explode(".", $first_ip);
		$ex_second = explode(".", $second_ip);
		return ($ex_first[0] < $ex_second[0])
			|| ($ex_first[1] < $ex_second[1])
			|| ($ex_first[2] < $ex_second[2])
			|| ($ex_first[3] < $ex_second[3]);
	}

	/**
	 * Check if an IP address is in the correct subnet.
	 *
	 * @param String $ip IP address
	 * @param String $subnet subnet
	 * @param String $mask network mask
	 * @return bool true if the range and subnet valid, else false
	 **/
	public static function check_subnet_range($ip, $subnet, $mask): bool {
		$ipSubnet = range::calculateSubnet($ip, $mask);
		return ($subnet == $ipSubnet);
	}

	/**
	 *
	 * Checks if ranges are not overlapped.
	 *
	 * @param string $ip first ip
	 * @param string $ipB second ip
	 *
	 * @return bool not overlapped
	 *
	 **/
	private function isNotOverlappedRange(string $ip, string $ipB): bool {
		$ex = explode(".", $ip);
		$exB = explode(".", $ipB);
		for ($n = $ex[3]; $n <= $exB[3]; $n++) {
			if (in_array($n, $this->overlapped)) {
				return false;
			}
			else {
				$this->overlapped[] = $n;
			}
		}
		return true;
	}

	/**
	 *
	 * Reset the overlapped_range() function
	 *
	 **/
	private function reset_overlapped_range(): void {
		$this->overlapped = [];
	}

	/**
	 * {@inheritDoc}
	 */
	public function getButtonStatus(): string {
		if (!$this->isRootNode()) {
			return "enabled";
		}
		else {
			return "hidden";
		}
	}

	/**
	 * {@inheritDoc}
	 */
	public function load_attributes(array $attributes): void {
		parent::load_attributes($attributes);
		// Load DHCP Options:
		if (!$this->isRootNode()) {
			$this->orig = $attributes;
			$this->attributes = $attributes;
			// Load DHCP Options:
			if (isset($attributes['dhcpRange']) && is_array($attributes['dhcpRange'])) {
				natcasesort($attributes['dhcpRange']);
				foreach ($attributes['dhcpRange'] as $id => $value) {
					$ex = explode(" ", $value);

					// DHCP Range ins Array laden:
					$this->ranges[$id] = ['range_start' => $ex[0], 'range_end' => $ex[1]];
				}
			}
			// load pools
			$this->loadPools();
		}
	}

	/**
	 *
	 * Adapt the Ranges with the subnet.
	 *
	 * @return bool true, if ranges were edit.
	 *
	 **/
	public function reload_ranges() {
		$range_edit = false;        // Range were edited?
		// Only run it, when ranges already exists:
		$ex_subnet = explode(".", $this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes['cn'][0]);
		$dhcpAttrs = $this->getAccountContainer()->getAccountModule('dhcp_settings')->getAttributes();
		$mask = $this->getAccountContainer()->getAccountModule('dhcp_settings')->getDHCPOption('subnet-mask');
		foreach ($this->ranges as $id => $range) {
			if (!empty($range['range_start']) && !range::check_subnet_range($range['range_start'], $dhcpAttrs['cn'][0], $mask)) {
				// adapt range
				$ex = explode(".", $range['range_start']);
				$tmp = $range['range_start'];
				$this->ranges[$id]['range_start'] = $ex_subnet[0] . "." . $ex_subnet[1] . "." . $ex_subnet[2] . "." . $ex[3];
				if ($tmp != $this->ranges[$id]['range_start']) {
					$range_edit = true;
				}
			}
			if (!empty($range['range_end']) && !range::check_subnet_range($range['range_end'], $dhcpAttrs['cn'][0], $mask)) {
				// adapt range
				$ex = explode(".", $range['range_end']);
				$tmp = $range['range_end'];
				$this->ranges[$id]['range_end'] = $ex_subnet[0] . "." . $ex_subnet[1] . "." . $ex_subnet[2] . "." . $ex[3];
				if ($tmp != $this->ranges[$id]['range_end']) {
					$range_edit = true;
				}
			}
		}
		if ($range_edit) {
			// sort the range new, id it was edit.
			foreach ($this->ranges as $id => $range) {
				$this->attributes['dhcpRange'][$id] = $range['range_start'] . " " . $range['range_end'];
			}
		}
		return $range_edit;
	}

	/**
	 * {@inheritDoc}
	 */
	public function process_attributes(): array {
		$errors = [];
		if ($this->isRootNode()) {
			return $errors;
		}
		$subnet = $this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes['cn'][0];
		$mask = $this->getAccountContainer()->getAccountModule('dhcp_settings')->getDHCPOption('subnet-mask');
		$errorOccurred = false;
		$this->reset_overlapped_range();
		if ($this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes['cn'][0] != "") {

			foreach (array_keys($this->ranges) as $id) {
				// Check if range is to drop
				if (isset($_POST['drop_range_' . $id])) {
					// drop Range:
					unset($this->ranges[$id]);
					unset($this->attributes['dhcpRange'][$id]);
					continue;
				}

				// if the inputs are empty, then do nothing:
				if ($_POST['range_start_' . $id] == "" && $_POST['range_end_' . $id] == "") {
					unset($this->attributes['dhcpRange'][$id]);
				}
				else {
					// Check range_start:
					$_POST['range_start_' . $id] = trim($_POST['range_start_' . $id]);
					if (!check_ip($_POST['range_start_' . $id])) {
						$errorOccurred = true;
					}
					$this->ranges[$id]['range_start'] = $_POST['range_start_' . $id];

					// Check end:
					$_POST['range_end_' . $id] = trim($_POST['range_end_' . $id]);
					if (!check_ip($_POST['range_end_' . $id])) {
						$errorOccurred = true;
					}
					$this->ranges[$id]['range_end'] = $_POST['range_end_' . $id];

					// Check if ip overlapped:
					if (!$this->isNotOverlappedRange($_POST['range_start_' . $id], $_POST['range_end_' . $id])) {
						$errorOccurred = true;
					}

					// Check if Subnet and range first are valid:
					if (!range::check_subnet_range($_POST['range_start_' . $id], $subnet, $mask)) {
						$errorOccurred = true;
					}

					// Check if Subnet and range last are valid:
					if (!range::check_subnet_range($_POST['range_end_' . $id], $subnet, $mask)) {
						$errorOccurred = true;
					}

					// Check if Range is valid
					if (!$this->check_range($_POST['range_start_' . $id], $_POST['range_end_' . $id])) {
						$errorOccurred = true;
					}

					// Check, if range_start and range_end are ok!
					if (!$errorOccurred) {
						$this->attributes['dhcpRange'][$id] = $_POST['range_start_' . $id] . " " . $_POST['range_end_' . $id];
					}
					else {
						unset($this->attributes['dhcpRange'][$id]);
					}
				}
			}
		}

		// Add new Range
		if (isset($_POST['add_range'])) {
			// Check, if there were no errors:
			if ($errorOccurred) {
				$errors[] = $this->messages['add_range'][0];
			}
			else {
				// Add Range:
				$this->ranges[] = ['range_start' => '', 'range_end' => ''];
			}
		}

		// update pool data
		foreach (array_keys($this->poolsNew) as $index) {
			// delete pool
			if (isset($_POST['del_pool_' . $index])) {
				unset($this->poolsNew[$index]);
				continue;
			}
			// name
			$this->poolsNew[$index]['cn'][0] = trim($_POST['pool_cn_' . $index]);
			if (empty($this->poolsNew[$index]['cn'][0])) {
				$errors[] = $this->messages['pool_cn'][0];
			}
			// failover peer
			$peer = trim($_POST['pool_peer_' . $index]);
			if (!empty($this->poolsNew[$index]['dhcpstatements'])) {
				// remove old peer setting
				foreach ($this->poolsNew[$index]['dhcpstatements'] as $indexStmt => $stmt) {
					if (str_starts_with($stmt, 'failover peer "')) {
						unset($this->poolsNew[$index]['dhcpstatements'][$indexStmt]);
						break;
					}
				}
			}
			if (!empty($peer)) {
				$this->poolsNew[$index]['dhcpstatements'][] = 'failover peer "' . $peer . '"';
			}
			if (!empty($this->poolsNew[$index]['dhcprange'])) {
				foreach (array_keys($this->poolsNew[$index]['dhcprange']) as $rIndex) {
					// delete pool range
					if (isset($_POST['drop_poolrange_' . $index . '_' . $rIndex])) {
						unset($this->poolsNew[$index]['dhcprange'][$rIndex]);
						continue;
					}
					$from = trim($_POST['pool_from_' . $index . '_' . $rIndex]);
					$to = trim($_POST['pool_to_' . $index . '_' . $rIndex]);
					if (empty($from) && empty($to)) {
						unset($this->poolsNew[$index]['dhcprange'][$rIndex]);
					}
					else {
						$this->poolsNew[$index]['dhcprange'][$rIndex] = $from . ' ' . $to;
						// check ranges
						if (!check_ip($from) || !check_ip($to)
							|| !$this->isNotOverlappedRange($from, $to)
							|| !range::check_subnet_range($from, $subnet, $mask)
							|| !range::check_subnet_range($to, $subnet, $mask)
							|| !$this->check_range($from, $to)) {
							$errorOccurred = true;
						}
					}
				}
			}
			// check if at least one range is still set
			if (empty($this->poolsNew[$index]['dhcprange'])) {
				$error = $this->messages['pool_range'][0];
				$error[] = [htmlspecialchars($this->poolsNew[$index]['cn'][0])];
				$errors[] = $error;
			}
			// add pool range
			if (isset($_POST['add_poolrange_' . $index])) {
				$this->poolsNew[$index]['dhcprange'][] = '';
			}
		}
		// add a new pool
		if (isset($_POST['add_pool'])) {
			$this->poolsNew[] = [
				'objectclass' => ['dhcpPool'],
				'dhcprange' => ['']
			];
		}

		// Check if there was a error:
		if ($errorOccurred) {
			$errors[] = $this->messages['range_errors'][0];
		}

		$this->processed = true;
		return $errors;
	}

	/**
	 * {@inheritDoc}
	 */
	public function display_html_attributes(): htmlElement|array {
		$return = new htmlResponsiveRow();
		if ($this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes['cn'][0] == "") {
			$return->add(new htmlStatusMessage('INFO', _("Please fill out the DHCP settings first.")));
			return $return;
		}
		$this->reset_overlapped_range();
		$mask = $this->getAccountContainer()->getAccountModule('dhcp_settings')->getDHCPOption('subnet-mask');
		$subnet = $this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes['cn'][0];
		foreach ($this->ranges as $id => $range) {
			// Range start
			$error = "";
			if (isset($range['range_start']) && !empty($range['range_start'])) {
				if ($this->processed && !check_ip($range['range_start'])) {
					$error = _("The IP address is invalid.");
				}
				elseif ($this->processed && !$this->check_range($range['range_start'], $range['range_end'])) {
					$error = _("The range end needs to be greater than the range start.");
				}
				elseif ($this->processed && !range::check_subnet_range($range['range_start'], $subnet, $mask)) {
					$error = _("The IP does not match the subnet.");
				}
				elseif ($this->processed && !$this->isNotOverlappedRange($range['range_start'], $range['range_end'])) {
					$error = _("The range conflicts with another range.");
				}
			}
			$fromInput = new htmlResponsiveInputField(_('Range from'), 'range_start_' . $id, $range['range_start'], 'range_from', true);
			$return->add($fromInput);
			if (!empty($error)) {
				$return->add(new htmlOutputText($error));
			}
			// Range end
			$error = "";
			if (isset($range['range_end']) && !empty($range['range_end'])) {
				if ($this->processed && !check_ip($range['range_end'])) {
					$error = _("The IP address is invalid.");
				}
				elseif ($this->processed && !range::check_subnet_range($range['range_end'], $subnet, $mask)) {
					$error = _("The IP does not match the subnet.");
				}
			}
			$toInput = new htmlResponsiveInputField(_('Range to'), 'range_end_' . $id, $range['range_end'], 'range_to', true);
			$return->add($toInput);
			if (!empty($error)) {
				$return->add(new htmlOutputText($error));
			}
			// Drop range:
			$dropButton = new htmlButton('drop_range_' . $id, _('Delete range'));
			$dropButton->setCSSClasses(['lam-danger']);
			$return->add($dropButton, 12, 12, 12, 'text-center');

			$return->addVerticalSpacer('2rem');
		}

		// add new range
		$addButton = new htmlButton('add_range', _('New range'));
		$return->add($addButton);

		// pools
		if (!empty($this->poolsNew)) {
			$return->add(new htmlSubTitle(_('Pools')));
			foreach ($this->poolsNew as $index => $poolAttrs) {
				$cn = empty($poolAttrs['cn'][0]) ? '' : $poolAttrs['cn'][0];
				$nameField = new htmlResponsiveInputField(_('Name'), 'pool_cn_' . $index, $cn, 'poolName', true);
				$return->add($nameField);
				$peer = '';
				if (!empty($poolAttrs['dhcpstatements'])) {
					foreach ($poolAttrs['dhcpstatements'] as $statement) {
						if (str_starts_with($statement, 'failover peer "')) {
							$peer = substr($statement, strlen('failover peer "'), -1);
						}
					}
				}
				$return->add(new htmlResponsiveInputField(_('Failover peer'), 'pool_peer_' . $index, $peer, 'poolPeer'));
				$delPoolButton = new htmlButton('del_pool_' . $index, _('Delete pool'));
				$delPoolButton->setCSSClasses(['lam-danger']);
				$return->add($delPoolButton, 12, 12, 12, 'text-center');
				$return->addVerticalSpacer('2rem');
				if (!empty($poolAttrs['dhcprange'])) {
					foreach ($poolAttrs['dhcprange'] as $rIndex => $range) {
						$range = explode(' ', $range);
						$from = empty($range[0]) ? '' : $range[0];
						$to = empty($range[1]) ? '' : $range[1];
						$fromInput = new htmlResponsiveInputField(_('Range from'), 'pool_from_' . $index . '_' . $rIndex, $from, 'range_from', true);
						$toInput = new htmlResponsiveInputField(_('Range to'), 'pool_to_' . $index . '_' . $rIndex, $to, 'range_to', true);
						$return->add($fromInput);
						$message = '';
						if (!empty($from) && $this->processed) {
							if (!check_ip($from)) {
								$message = _("The IP address is invalid.");
							}
							elseif (!$this->check_range($from, $to)) {
								$message = _("The range end needs to be greater than the range start.");
							}
							elseif (!range::check_subnet_range($from, $subnet, $mask)) {
								$message = _("The IP does not match the subnet.");
							}
							elseif (!$this->isNotOverlappedRange($from, $to)) {
								$message = _("The range conflicts with another range.");
							}
						}
						$return->add($toInput);
						if (!empty($message)) {
							$return->add(new htmlOutputText($message));
						}
						$message = '';
						if (!empty($to) && $this->processed) {
							if (!check_ip($to)) {
								$message = _("The IP address is invalid.");
							}
							elseif (!range::check_subnet_range($to, $subnet, $mask)) {
								$message = _("The IP does not match the subnet.");
							}
						}
						if (!empty($message)) {
							$return->add(new htmlOutputText($message));
						}
						// drop range
						$dropButton = new htmlButton('drop_poolrange_' . $index . '_' . $rIndex, _('Delete range'));
						$dropButton->setCSSClasses(['lam-danger']);
						$return->add($dropButton, 12, 12, 12, 'text-center');
						$return->addVerticalSpacer('4rem');
					}
				}
				// add new range
				$newButton = new htmlButton('add_poolrange_' . $index, _('New range'));
				$return->add($newButton);
				$return->addVerticalSpacer('4rem');
			}
			$return->addVerticalSpacer('4rem');
		}

		// add new range pool
		$addButton = new htmlButton('add_pool', _('New pool'));
		$return->add($addButton);
		$return->addVerticalSpacer('4rem');

		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function postModifyActions(bool $newAccount, array $attributes): array {
		$errors = [];
		// update pool data
		// add new pools
		foreach ($this->poolsNew as $pool) {
			$found = false;
			foreach ($this->poolsOrig as $poolOrig) {
				if ($pool['cn'][0] == $poolOrig['cn'][0]) {
					$found = true;
					break;
				}
			}
			if (!$found) {
				// create new pool
				$dn = 'cn=' . $pool['cn'][0] . ',' . $this->getAccountContainer()->finalDN;
				if (isset($pool['dn'])) {
					unset($pool['dn']);
				}
				$success = ldapAddNewEntry($_SESSION['ldap']->server(), $dn, $pool);
				if (!$success) {
					$msg = sprintf(_('Was unable to create DN: %s.'), $dn);
					logNewMessage(LOG_ERR, $msg . getDefaultLDAPErrorString($_SESSION['ldap']->server()));
					$errors[] = ['ERROR', $msg, getDefaultLDAPErrorString($_SESSION['ldap']->server())];
				}
			}
		}
		// update existing pools
		foreach ($this->poolsNew as $pool) {
			foreach ($this->poolsOrig as $poolOrig) {
				if ($pool['cn'][0] == $poolOrig['cn'][0]) {
					// check for changes
					$mod = [];
					// check attributes
					$attrs = ['dhcpstatements', 'dhcprange'];
					foreach ($attrs as $attr) {
						$changed = false;
						if (empty($pool[$attr]) && empty($poolOrig[$attr])) {
							continue;
						}
						if ((!empty($pool[$attr]) && empty($poolOrig[$attr]))
							|| (empty($pool[$attr]) && !empty($poolOrig[$attr]))
							|| (count($pool[$attr]) !== count($poolOrig[$attr]))) {
							$changed = true;
						}
						else {
							sort($pool[$attr]);
							sort($poolOrig[$attr]);
							foreach ($pool[$attr] as $index => $val) {
								if ($val != $poolOrig[$attr][$index]) {
									$changed = true;
								}
							}
						}
						if ($changed && isset($pool[$attr])) {
							$mod[$attr] = $pool[$attr];
						}
						elseif ($changed) {
							$mod[$attr] = [];
						}
					}
					if (!empty($mod)) {
						$dn = 'cn=' . $pool['cn'][0] . ',' . $this->getAccountContainer()->finalDN;
						$success = @ldap_modify($_SESSION['ldap']->server(), $dn, $mod);
						if (!$success) {
							$msg = sprintf(_('Was unable to modify attributes of DN: %s.'), $dn);
							logNewMessage(LOG_ERR, $msg . getDefaultLDAPErrorString($_SESSION['ldap']->server()));
							$errors[] = ['ERROR', $msg, getDefaultLDAPErrorString($_SESSION['ldap']->server())];
						}
					}
					break;
				}
			}
		}
		// delete obsolete pools
		foreach ($this->poolsOrig as $poolOrig) {
			$found = false;
			foreach ($this->poolsNew as $pool) {
				if ($poolOrig['cn'][0] == $pool['cn'][0]) {
					$found = true;
					break;
				}
			}
			if (!$found) {
				// delete pool
				$dn = 'cn=' . $poolOrig['cn'][0] . ',' . $this->getAccountContainer()->finalDN;
				$success = ldapDeleteEntry($_SESSION['ldap']->server(), $dn);
				if (!$success) {
					$msg = sprintf(_('Was unable to delete DN: %s.'), $dn);
					logNewMessage(LOG_ERR, $msg . getDefaultLDAPErrorString($_SESSION['ldap']->server()));
					$errors[] = ['ERROR', $msg, getDefaultLDAPErrorString($_SESSION['ldap']->server())];
				}
			}
		}
		return $errors;
	}

	/**
	 * {@inheritDoc}
	 */
	public function save_attributes(): array {
		$return = [];
		// Get easy attributes
		if (!$this->isRootNode()) {
			$return = $this->getAccountContainer()->save_module_attributes($this->attributes, $this->orig);
		}
		// Return attributes
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_pdfEntries(array $pdfKeys, string $typeId): array {
		$return = [];
		if ($this->ranges !== []) {
			$pdfTable = new PDFTable(_('Ranges'));
			for ($i = 0; $i < count($this->ranges); $i++) {
				$start = $this->ranges[$i]['range_start'];
				$end = $this->ranges[$i]['range_end'];
				$pdfRow = new PDFTableRow();
				$pdfRow->cells[] = new PDFTableCell($start . " - " . $end);
				$pdfTable->rows[] = $pdfRow;
			}
			foreach ($this->poolsNew as $poolAttrs) {
				$cn = empty($poolAttrs['cn'][0]) ? '' : $poolAttrs['cn'][0];
				$peer = '';
				if (!empty($poolAttrs['dhcpstatements'])) {
					foreach ($poolAttrs['dhcpstatements'] as $statement) {
						if (str_starts_with($statement, 'failover peer "')) {
							$peer = ' (' . substr($statement, strlen('failover peer "'), -1) . ')';
						}
					}
				}
				if (!empty($poolAttrs['dhcprange'])) {
					foreach ($poolAttrs['dhcprange'] as $range) {
						$range = explode(' ', $range);
						$from = empty($range[0]) ? '' : $range[0];
						$to = empty($range[1]) ? '' : $range[1];
						$pdfRow = new PDFTableRow();
						$pdfRow->cells[] = new PDFTableCell($cn . $peer . ': ' . $from . " - " . $to);
						$pdfTable->rows[] = $pdfRow;
					}
				}
			}
			$this->addPDFTable($return, 'ranges', $pdfTable);
		}
		return $return;
	}

	/**
	 * Loads the list of pools from LDAP.
	 */
	private function loadPools(): void {
		$dn = $this->getAccountContainer()->dn_orig;
		$this->poolsOrig = searchLDAP($dn, '(objectclass=dhcpPool)', ['*']);
		foreach ($this->poolsOrig as &$pool) {
			if (!empty($pool['dhcprange'])) {
				sort($pool['dhcprange']);
			}
		}
		$this->poolsNew = $this->poolsOrig;
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
			"dhcprange" => _("Ranges"),
		];
	}

	/**
	 * @inheritDoc
	 */
	public function getListRenderFunction(string $attributeName, ConfiguredType $type): ?callable {
		if ($attributeName === 'dhcprange') {
			return function(array $entry, string $attribute): htmlElement {
				if (!empty($entry[$attribute][0])) {
					$table = new htmlTable();
					$table->setCSSClasses(['nowrap']);
					$ranges = [];
					foreach ($entry['dhcprange'] as $value) {
						if (!empty($value) && !is_numeric($value)) {
							$ex = explode(" ", $value);
							$row = [
								new htmlOutputText($ex[0]),
								new htmlOutputText(' - '),
								new htmlOutputText($ex[1])
							];
							$ranges[$ex[0] . ' - ' . $ex[1]] = $row;
						}
					}
					$pooledRanges = searchLDAP($entry['dn'], '(objectclass=dhcpPool)', ['dhcprange']);
					foreach ($pooledRanges as $pool) {
						if (empty($pool['dhcprange'])) {
							continue;
						}
						foreach ($pool['dhcprange'] as $value) {
							if (!empty($value) && !is_numeric($value)) {
								$ex = explode(" ", $value);
								$row = [
									new htmlOutputText($ex[0]),
									new htmlOutputText(' - '),
									new htmlOutputText($ex[1])
								];
								$ranges[$ex[0] . ' - ' . $ex[1]] = $row;
							}
						}
					}
					uksort($ranges, strnatcasecmp(...));
					foreach ($ranges as $row) {
						$table->addElement($row[0]);
						$table->addElement($row[1]);
						$table->addElement($row[2], true);
					}
					return $table;
				}
				return new htmlGroup();
			};
		}
		return null;
	}

}
