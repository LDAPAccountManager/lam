<?php

use LAM\PDF\PDFTable;
use LAM\PDF\PDFTableCell;
use LAM\PDF\PDFTableRow;
use LAM\TYPES\ConfiguredType;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2008         Thomas Manninger
                2008 - 2024  Roland Gruber

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

	/** subnet */
	public $subnet;

	/** ranges */
	public $ranges;

	/** Range -> Function attribute_processed already running? */
	public $processed;

	/** For check, if IPs overlapped. */
	private $overlapped;

	/** list of pools that currently exist in LDAP */
	private $poolsOrig = [];
	/** list of pools that need to be updated in LDAP */
	private $poolsNew = [];

	/**
	 * Returns true if this module can manage accounts of the current type, otherwise false.
	 *
	 * @return boolean true if module fits
	 */
	public function can_manage() {
		return $this->get_scope() == 'dhcp';
	}

	/**
	 * Returns meta data that is interpreted by parent class
	 *
	 * @return array array with meta data
	 *
	 * @see baseModule::get_metaData()
	 */
	public function get_metaData() {
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
	 *  This function fills the error message array with messages.
	 */
	public function load_Messages() {
		$this->messages['range_errors'][0] = ['ERROR', _('One or more errors occurred. The invalid fields are marked.'), ''];
		$this->messages['add_range'][0] = ['ERROR', _('New range'), _('Adding the range failed because errors occurred.')];
		$this->messages['drop_range'][0] = ['ERROR', _('Delete range'), _('It is not possible to delete all ranges.')];
		$this->messages['pool_cn'][0] = ['ERROR', _('Name'), _('Please enter a pool name.')];
		$this->messages['pool_range'][0] = ['ERROR', _('Please enter at least one range for pool "%s".'), null];
	}

	/**
	 * Calculates the subnet for a given IP and netmask.
	 *
	 * @param String $ip IP address
	 * @param String $mask network mask
	 */
	private static function calculateSubnet($ip, $mask) {
		return long2ip(ip2long($ip) & ip2long($mask));
	}

	/**
	 * Checks if the first IP is smaller than the second IP.
	 *
	 * @param String $first_ip first ip
	 * @param String $second_ip second ip
	 *
	 * @return true, if it's a valid Range, else false;
	 **/

	public function check_range($first_ip, $second_ip) {
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
	 * @return true if the range and subnet valid, else false
	 **/
	public static function check_subnet_range($ip, $subnet, $mask) {
		$ipSubnet = range::calculateSubnet($ip, $mask);
		return ($subnet == $ipSubnet);
	}

	/**
	 *
	 * Checks if ranges are not overlapped.
	 *
	 * @param first ip
	 * @param second ip
	 *
	 * @return bool not overlapped
	 *
	 **/
	private function isNotOverlappedRange($ip, $ipB) {
		$ex = explode(".", $ip);
		$exB = explode(".", $ipB);

		if (!is_array($this->overlapped)) {
			$this->overlapped = [];
		}
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
	private function reset_overlapped_range() {
		$this->overlapped = [];
	}

	/**
	 * Controls if the module button the account page is visible and activated.
	 *
	 * @return string status ("enabled", "disabled", "hidden")
	 */
	public function getButtonStatus() {
		if (!$this->isRootNode()) {
			return "enabled";
		}
		else {
			return "hidden";
		}
	}

	/**
	 * This function loads all needed LDAP attributes.
	 *
	 * @param array $attr list of attributes
	 */
	function load_attributes($attr) {
		parent::load_attributes($attr);
		// Load DHCP Options:
		if (!$this->isRootNode()) {
			$this->orig = $attr;
			$this->attributes = $attr;
			// Load DHCP Options:
			if (isset($attr['dhcpRange']) && is_array($attr['dhcpRange'])) {
				natcasesort($attr['dhcpRange']);
				foreach ($attr['dhcpRange'] as $id => $value) {
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
	 * @return true, if ranges were edit.
	 *
	 **/
	public function reload_ranges() {
		$range_edit = false;        // Range were edited?
		// Only run it, when ranges already exists:
		if (is_array($this->ranges)) {
			$ex_subnet = explode(".", $this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes['cn'][0]);
			$dhcpAttrs = $this->getAccountContainer()->getAccountModule('dhcp_settings')->getAttributes();
			$mask = $this->getAccountContainer()->getAccountModule('dhcp_settings')->getDHCPOption('subnet-mask');
			foreach ($this->ranges as $id => $arr) {
				if (!empty($this->ranges[$id]['range_start']) && !range::check_subnet_range($this->ranges[$id]['range_start'], $dhcpAttrs['cn'][0], $mask)) {
					// Range anpassen:
					$ex = explode(".", $this->ranges[$id]['range_start']);
					$tmp = $this->ranges[$id]['range_start'];
					$this->ranges[$id]['range_start'] = $ex_subnet['0'] . "." . $ex_subnet['1'] . "." . $ex_subnet['2'] . "." . $ex['3'];
					if ($tmp != $this->ranges[$id]['range_start']) {
						$range_edit = true;
					}
				}
				if (!empty($this->ranges[$id]['range_end']) && !range::check_subnet_range($this->ranges[$id]['range_end'], $dhcpAttrs['cn'][0], $mask)) {
					// Range anpassen:
					$ex = explode(".", $this->ranges[$id]['range_end']);
					$tmp = $this->ranges[$id]['range_end'];
					$this->ranges[$id]['range_end'] = $ex_subnet['0'] . "." . $ex_subnet['1'] . "." . $ex_subnet['2'] . "." . $ex['3'];
					if ($tmp != $this->ranges[$id]['range_end']) {
						$range_edit = true;
					}
				}
			}
			if ($range_edit) {
				// sort the range new, id it was edit.
				foreach ($this->ranges as $id => $arr) {
					$this->attributes['dhcpRange'][$id] = $this->ranges[$id]['range_start'] . " " . $this->ranges[$id]['range_end'];
				}
			}
		}
		return $range_edit;
	}

	/**
	 * Processes user input of the primary module page.
	 * It checks if all input values are correct and updates the associated LDAP attributes.
	 *
	 * @return array list of info/error messages
	 */
	public function process_attributes() {
		$errors = [];
		if ($this->isRootNode()) {
			return $errors;
		}
		$subnet = $this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes['cn'][0];
		$mask = $this->getAccountContainer()->getAccountModule('dhcp_settings')->getDHCPOption('subnet-mask');
		$errorOccurred = false;
		$this->reset_overlapped_range();
		if ($this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes['cn'][0] != "") {

			foreach ($this->ranges as $id => $arr) {
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
		foreach ($this->poolsNew as $index => $pool) {
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
				foreach ($this->poolsNew[$index]['dhcprange'] as $rIndex => $range) {
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
	 * Returns the HTML meta data for the main account page.
	 *
	 * @return htmlElement HTML meta data
	 */
	public function display_html_attributes() {
		$return = new htmlResponsiveRow();
		if ($this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes['cn'][0] == "") {
			$return->add(new htmlStatusMessage('INFO', _("Please fill out the DHCP settings first.")));
			return $return;
		}

		// If $ranges is not a array, then create it
		if (!is_array($this->ranges)) {
			$this->ranges = [];
		}
		$this->reset_overlapped_range();
		$mask = $this->getAccountContainer()->getAccountModule('dhcp_settings')->getDHCPOption('subnet-mask');
		$subnet = $this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes['cn'][0];
		foreach ($this->ranges as $id => $arr) {
			// Range start
			$error = "";
			if (isset($this->ranges[$id]['range_start']) && !empty($this->ranges[$id]['range_start'])) {
				if ($this->processed && !check_ip($this->ranges[$id]['range_start'])) {
					$error = _("The IP address is invalid.");
				}
				elseif ($this->processed && !$this->check_range($this->ranges[$id]['range_start'], $this->ranges[$id]['range_end'])) {
					$error = _("The range end needs to be greater than the range start.");
				}
				elseif ($this->processed && !range::check_subnet_range($this->ranges[$id]['range_start'], $subnet, $mask)) {
					$error = _("The IP does not match the subnet.");
				}
				elseif ($this->processed && !$this->isNotOverlappedRange($this->ranges[$id]['range_start'], $this->ranges[$id]['range_end'])) {
					$error = _("The range conflicts with another range.");
				}
			}
			$fromInput = new htmlResponsiveInputField(_('Range from'), 'range_start_' . $id, $this->ranges[$id]['range_start'], 'range_from', true);
			$return->add($fromInput);
			if (!empty($error)) {
				$return->add(new htmlOutputText($error));
			}
			// Range end
			$error = "";
			if (isset($this->ranges[$id]['range_end']) && !empty($this->ranges[$id]['range_end'])) {
				if ($this->processed && !check_ip($this->ranges[$id]['range_end'])) {
					$error = _("The IP address is invalid.");
				}
				elseif ($this->processed && !range::check_subnet_range($this->ranges[$id]['range_end'], $subnet, $mask)) {
					$error = _("The IP does not match the subnet.");
				}
			}
			$toInput = new htmlResponsiveInputField(_('Range to'), 'range_end_' . $id, $this->ranges[$id]['range_end'], 'range_to', true);
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
				$cn = !empty($poolAttrs['cn'][0]) ? $poolAttrs['cn'][0] : '';
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
						$from = !empty($range[0]) ? $range[0] : '';
						$to = !empty($range[1]) ? $range[1] : '';
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
	 * Allows the module to run commands after the LDAP entry is changed or created.
	 *
	 * Calling this method requires the existence of an enclosing {@link accountContainer}.
	 *
	 * @param boolean $newAccount new account
	 * @param array $attributes LDAP attributes of this entry
	 * @return array array which contains status messages. Each entry is an array containing the status message parameters.
	 */
	public function postModifyActions($newAccount, $attributes) {
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
				$success = @ldap_add($_SESSION['ldap']->server(), $dn, $pool);
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
							|| (sizeof($pool[$attr]) != sizeof($poolOrig[$attr]))) {
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
				$success = @ldap_delete($_SESSION['ldap']->server(), $dn);
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
	 * @see baseModule::get_pdfEntries()
	 */
	function get_pdfEntries($pdfKeys, $typeId) {
		$return = [];
		if (is_array($this->ranges) && (sizeof($this->ranges) > 0)) {
			$pdfTable = new PDFTable(_('Ranges'));
			for ($i = 0; $i < sizeof($this->ranges); $i++) {
				$start = $this->ranges[$i]['range_start'];
				$end = $this->ranges[$i]['range_end'];
				$pdfRow = new PDFTableRow();
				$pdfRow->cells[] = new PDFTableCell($start . " - " . $end);
				$pdfTable->rows[] = $pdfRow;
			}


			if (!empty($this->poolsNew)) {
				foreach ($this->poolsNew as $poolAttrs) {
					$cn = !empty($poolAttrs['cn'][0]) ? $poolAttrs['cn'][0] : '';
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
							$from = !empty($range[0]) ? $range[0] : '';
							$to = !empty($range[1]) ? $range[1] : '';
							$pdfRow = new PDFTableRow();
							$pdfRow->cells[] = new PDFTableCell($cn . $peer . ': ' . $from . " - " . $to);
							$pdfTable->rows[] = $pdfRow;
						}
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
	private function loadPools() {
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
	public function getListRenderFunction(string $attributeName): ?callable {
		if ($attributeName === 'dhcprange') {
			return function(array $entry, string $attribute): ?htmlElement {
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
				return null;
			};
		}
		return null;
	}

}
