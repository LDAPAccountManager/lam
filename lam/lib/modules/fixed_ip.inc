<?php
use \LAM\PDF\PDFTable;
use \LAM\PDF\PDFTableCell;
use \LAM\PDF\PDFTableRow;
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2008         Thomas Manninger
                2008 - 2017  Roland Gruber

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
* Manages DHCP host entries.
*
* @package modules
*
* @author Thomas Manninger
* @author Roland Gruber
*/

/**
* Manages DHCP host entries.
*
* @package modules
*/

class fixed_ip extends baseModule {

	/** fixed ips */
	public $fixed_ip = array();

	/** already processed? */
	public $processed = false;

	/** for check if IPs overlap */
	public $overlapd;

	/** original IPs */
	public $orig_ips = array();

	/** LDAP attributes */
	public $attributes;

	/** cached host entries (list of array('cn' => ..., 'iphostnumber' => ..., 'macaddress' => ...)) */
	private $hostCache = null;

	/**
	* Returns true if this module can manage accounts of the current type, otherwise false.
	*
	* @return boolean true if module fits
	*/
	public function can_manage() {
		return in_array($this->get_scope(), array('dhcp'));
	}

	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	*
	* @see baseModule::get_metaData()
	*/
	public function get_metaData() {
		$return = array();
		// alias name
		$return["alias"] = _("Hosts");
		// this is a base module
		$return["is_base"] = false;
		// icon
		$return['icon'] = 'computer.png';
		// RDN attribute
		$return["RDN"] = array("cn" => "high");
		// LDAP filter
		$return["ldap_filter"] = array();
		// module dependencies
		$return['dependencies'] = array('depends' => array('dhcp_settings'), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array();
		// managed attributes
		$return['attributes'] = array('dhcpOption');
		// help Entries
		$return['help'] = array(
			'pc' => array(
				"Headline" => _("PC name"), 'attr' => 'dhcpOption, host-name',
				"Text" => _("The name of the PC.")
			),
			'mac' => array(
				"Headline" => _("MAC address"), 'attr' => 'dhcpHWAddress',
				"Text" => _("The MAC address of the PC. Example: 11:22:33:44:55:aa")
			),
			'ip' => array(
				"Headline" => _("IP address"), 'attr' => 'dhcpStatements, fixed-address',
				"Text" => _("The IP address of the PC.")
			),
			'description' => array(
				"Headline" => _("Description"), 'attr' => 'dhcpComments',
				"Text" => _("Optional description for the PC.")
			),
			'active' => array(
				"Headline" => _("Active"), 'attr' => 'dhcpStatements, booting',
				"Text" => _("Inactive hosts will not be able to get an address from the DHCP server.")
			),
		);
		// available PDF fields
		$return['PDF_fields'] = array('IPlist' => _('IP list'));
		return $return;
	}

	/**
	 *  This function fills the error message array with messages.
	 */
	public function load_Messages() {
		$this->messages['errors'][0] = array('ERROR', _('One or more errors occured. The invalid fields are marked.'), '');
	}

	/**
	* Controls if the module button the account page is visible and activated.
	*
	* @return string status ("enabled", "disabled", "hidden")
	*/
	public function getButtonStatus() {
	    if ($this->isRootNode()) {
			return "hidden";
	    }
		else {
	    	return "enabled";
		}
	}

	/**
	 *
	 * Checked, if ips are overlapd.
	 *
	 * @param ip
	 *
	 * @return false, if overlapd, else true.
	 *
	 **/
	public function overlapd_ip($ip) {
		if (in_array($ip, $this->overlapd)) {
		    return false;
		}

		$this->overlapd[] = $ip;
		return true;
	}

	/**
	 *
	 * Reset the overlapd_range() function
	 *
	 **/
	 public function reset_overlapd_ip() {
	    $this->overlapd = array();
	 }

	/**
	 *
	 * Check, if a mac address is invalid
	 * @param mac adress
	 *
	 * @return true, if mac is invalid
	 **/
	public function check_mac($mac) {
	    $ex = explode(":", $mac);
		$invalid = false;
		if (count($ex)!=6) {
		    $invalid = true;
		}

		foreach($ex AS $value) {
			if (!preg_match("/[0-9a-fA-F][0-9a-fA-F]/", $value) || strlen($value)!="2") {
				$invalid = true;
			}
		}
		return $invalid;
	}

	/**
	 *
	 * Adapt the fixed ip with the subnet.
	 *
	 * @return true, if ip were edit.
	 *
	 **/
	public function reload_ips() {
		$ip_edit = false;        // IPs were edited?
		// Only run it, when ranges already exists:
	    if(is_array($this->fixed_ip)) {
		    $ex_subnet = explode(".", $this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes['cn'][0]);
		    foreach ($this->fixed_ip AS $id=>$arr) {
		        if (!empty($this->fixed_ip[$id]['ip']) && !range::check_subnet_range($this->fixed_ip[$id]['ip'],
		        																	$this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes['cn'][0],
		        																	$this->getAccountContainer()->getAccountModule('dhcp_settings')->getDHCPOption('subnet-mask'))) {
		            // Range anpassen:
					$ex = explode(".", $this->fixed_ip[$id]['ip']);
					$tmp = $this->fixed_ip[$id]['ip'];
					$this->fixed_ip[$id]['ip'] = $ex_subnet['0'].".".$ex_subnet['1'].".".$ex_subnet['2'].".".$ex['3'];
					if ($tmp!=$this->fixed_ip[$id]['ip'])
						$ip_edit = true;
		        }
		    }
		}
		return $ip_edit;
	}

	/**
	 * This function loads all needed LDAP attributes.
	 *
	 * @param array $attr list of attributes
	 */
	function load_attributes($attr) {
	    if (!$this->isRootNode()) {
	    	$attributes = array('cn', 'dhcphwaddress', 'dhcpstatements', 'dhcpcomments');
	    	$entries = searchLDAP($this->getAccountContainer()->dn_orig, '(objectClass=dhcpHost)', $attributes);
			for ($i = 0; $i < sizeof($entries); $i++) {
				$dhcphwaddress = explode(" ", $entries[$i]['dhcphwaddress'][0]);
				$dhcphwaddress = array_pop($dhcphwaddress);
				$dhcpstatements = array();
				if (isset($entries[$i]['dhcpstatements'][0])) {
					$dhcpstatements = $entries[$i]['dhcpstatements'];
				}
				$this->fixed_ip[$i]['cn'] = $entries[$i]['cn'][0];
				$this->fixed_ip[$i]['mac'] = $dhcphwaddress;
				$this->fixed_ip[$i]['ip'] = self::extractIP($dhcpstatements);
				$this->fixed_ip[$i]['active'] = self::isActive($dhcpstatements);
				$this->fixed_ip[$i]['dhcpstatements'] = $dhcpstatements;
				$description = '';
				if (isset($entries[$i]['dhcpcomments'][0])) {
					$description = $entries[$i]['dhcpcomments'][0];
				}
				$this->fixed_ip[$i]['description'] = $description;

				$this->orig_ips[$i]['cn'] = $entries[$i]['cn'][0];
				$this->orig_ips[$i]['mac'] = $dhcphwaddress;
				$this->orig_ips[$i]['ip'] = self::extractIP($dhcpstatements);
				$this->orig_ips[$i]['active'] = self::isActive($dhcpstatements);
				$this->orig_ips[$i]['dhcpstatements'] = $dhcpstatements;
				$this->orig_ips[$i]['description'] = $description;
			}
			$this->orderByIP();
		}
	}

	/**
	 * Orders the host entries by IP address.
	 */
	private function orderByIP() {
	    // sort by IP
		$order = array();
		foreach ($this->fixed_ip as $key => $value) {
			$order[$key] = '';
			if (!empty($value['dhcpstatements'])) {
				$order[$key] = fixed_ip::extractIP($value['dhcpstatements']);
			}
		}
		natcasesort($order);
		$newVal = array();
		foreach ($order as $index => $sortval) {
			$newVal[] = $this->fixed_ip[$index];
		}
		$this->fixed_ip = $newVal;
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	public function process_attributes() {
		$errors = array();
		if (!$this->isRootNode()) {
			$this->processed = true;

			$this->reset_overlapd_ip();

			if ($this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes['cn'][0]!="") {

				$error = false;     // errors by process_attributes()?
				$pcs = array();
				foreach($this->fixed_ip AS $id=>$arr) {

			        // Check if ip is to drop
				    if (isset($_POST['drop_ip_'.$id])) {
						// Drop ip:
						unset($this->fixed_ip[$id]);
				        continue;
				    }

					// MAC address
					$_POST['mac_'.$id] = strtolower(trim($_POST['mac_'.$id]));

					$invalid = $this->check_mac($_POST['mac_'.$id]);
					if ($invalid) {
					    $error = true;
					}
					$this->fixed_ip[$id]['mac'] = $_POST['mac_'.$id];

					// description
					if (!get_preg($_POST['description_'.$id], 'ascii')) {
					    $error = true;
					}
					$this->fixed_ip[$id]['description'] = $_POST['description_'.$id];

					// active
					$this->fixed_ip[$id]['active'] = (isset($_POST['active_' . $id]) && ($_POST['active_' . $id] == 'on'));

					// Ip address
					if (!empty($_POST['ip_'.$id])) {
						$_POST['ip_'.$id] = trim($_POST['ip_'.$id]);
					}
					if (!empty($_POST['ip_'.$id]) && !(check_ip($_POST['ip_'.$id]))) {
						$error = true;
						$this->fixed_ip[$id]['ip'] = $_POST['ip_'.$id];
					}
					elseif (!empty($_POST['ip_'.$id]) && !$this->overlapd_ip($_POST['ip_'.$id])) {
						$error = true;
						$this->fixed_ip[$id]['ip'] = $_POST['ip_'.$id];
					}
					else {
						$this->fixed_ip[$id]['ip'] = $_POST['ip_'.$id];
					}

					// Is ip correct with subnet:
					if (!empty($_POST['ip_'.$id]) && check_ip($_POST['ip_'.$id]) && !range::check_subnet_range($_POST['ip_'.$id],
																				 $this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes['cn'][0],
																				 $this->getAccountContainer()->getAccountModule('dhcp_settings')->getDHCPOption('subnet-mask'))) {
					    $error = true;
					}

					// cn:
					if (!empty($_POST['pc_'.$id])) $_POST['pc_'.$id] = trim($_POST['pc_'.$id]);
	                if (!empty($_POST['pc_'.$id])) {

						// Already use?
						if (in_array($_POST['pc_'.$id], $pcs) ) {
						    $error = true;
						}
						else {
						    $pcs[] = $_POST['pc_'.$id];
						}
					}
					else {
						$error = true;
					}
					if (strlen($_POST['pc_'.$id])>30) {
					    $error = true;
					}
					if (!preg_match("/^[A-Za-z0-9\\._-]*$/",$_POST['pc_'.$id])) {
					    $error = true;
					}
					$this->fixed_ip[$id]['cn'] = $_POST['pc_'.$id];
				}
				if ($error) {
				    $errors[] = $this->messages['errors'][0];
				}
			}

			// Add new IP
			if (isset($_POST['add_ip']) || ($_POST['pc_add'] != '') || ($_POST['mac_add'] != '')) {
			    // Add IP:
			    $active = (isset($_POST['active_add']) && ($_POST['active_add'] == 'on'));
			    $dhcpstatements = array();
			    $this->setActive($dhcpstatements, $active);
			    $this->setIP($dhcpstatements, $_POST['ip_add']);
				$this->fixed_ip[] = array(
					'cn' => $_POST['pc_add'],
					'mac' => $_POST['mac_add'],
					'description' => $_POST['description_add'],
					'ip' => $_POST['ip_add'],
					'dhcpstatements' => $dhcpstatements,
					'active' => $active,
				);
				$this->orderByIP();
			}
		}

		return $errors;
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 *
	 * @return htmlElement HTML meta data
	 */
	public function display_html_attributes() {
		$return = new htmlTable();
		if ($this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes['cn'][0]=="") {
			$return->addElement(new htmlStatusMessage('ERROR', _("Please fill out the DHCP settings first.")), true);
			return $return;
        }
        else {
			$this->initCache();
        	// auto-completion for host names
			$autoNames = array();
			if (!empty($this->hostCache) && (sizeof($this->hostCache) < 200)) {
				foreach ($this->hostCache as $index => $attrs) {
					if (!empty($attrs['cn'][0])) {
						$autoNames[] = $attrs['cn'][0];
					}
				}
				$autoNames = array_values(array_unique($autoNames));
			}
        	// caption
        	$ipContainer = new htmlTable();
        	$ipContainer->addElement(new htmlOutputText(_('IP address')));
        	$ipContainer->addElement(new htmlHelpLink('ip'));
        	$return->addElement($ipContainer);
        	$pcContainer = new htmlTable();
        	$pcContainer->addElement(new htmlOutputText(_('PC name'), true, true));
        	$pcContainer->addElement(new htmlHelpLink('pc'));
        	$return->addElement($pcContainer);
        	$macContainer = new htmlTable();
        	$macContainer->addElement(new htmlOutputText(_('MAC address'), true, true));
        	$macContainer->addElement(new htmlHelpLink('mac'));
        	$return->addElement($macContainer);
        	$commentContainer = new htmlTable();
        	$commentContainer->addElement(new htmlOutputText(_('Description'), true));
        	$commentContainer->addElement(new htmlHelpLink('description'));
        	$return->addElement($commentContainer);
        	$activeContainer = new htmlTable();
        	$activeContainer->colspan = 2;
        	$activeContainer->addElement(new htmlOutputText(_('Active')));
        	$activeContainer->addElement(new htmlHelpLink('active'));
        	$return->addElement($activeContainer, true);
        	// Reset oberlaped ips
			$this->reset_overlapd_ip();

			// If $ranges is not a array, then create one:
			if (!is_array($this->fixed_ip)) {
				$this->fixed_ip = array();
			}
            $pcs = array();
	        foreach($this->fixed_ip AS $id=>$arr) {
				// pc name
				$pcError = "";
				if (!$this->processed) {
                	$pcError = "";
				}
				elseif (strlen($this->fixed_ip[$id]['cn'])>20) {
				    $pcError = _("The PC name may not be longer than 20 characters.");
				}
				elseif (strlen($this->fixed_ip[$id]['cn'])<2) {
				    $pcError = _("The PC name needs to be at least 2 characters long.");
				}
				elseif (in_array($this->fixed_ip[$id]['cn'], $pcs) ) {
				    $pcError = _("This PC name already exists.");
				}
				elseif (isset($_POST['pc_'.$id]) && !preg_match("/^[A-Za-z0-9\\._-]*$/", $_POST['pc_'.$id])) {
					$pcError = _("The PC name may only contain A-Z, a-z and 0-9.");
				}
				$pcs[] = $this->fixed_ip[$id]['cn'];

				// MAC address
				$macError = "";
				if (!$this->processed) {
                	$macError = "";
				}
				elseif ($this->check_mac($this->fixed_ip[$id]['mac'])) {
				    $macError = _("Invalid MAC address.");
				}

				// descripton
				$descriptionError = "";
				if (!$this->processed) {
                	$descriptionError = "";
				}
				elseif (!get_preg($this->fixed_ip[$id]['description'], 'ascii')) {
				    $descriptionError = _("Invalid description.");
				}

				// fixed ip
				$ipError = "";
				if (!$this->processed || ($this->fixed_ip[$id]['ip'] == '')) {
                	$ipError = "";
				}
				elseif (!check_ip($this->fixed_ip[$id]['ip'])) {
					$ipError = _("The IP address is invalid.");
				}
				elseif (!range::check_subnet_range($this->fixed_ip[$id]['ip'],
													$this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes['cn'][0],
													$this->getAccountContainer()->getAccountModule('dhcp_settings')->getDHCPOption('subnet-mask'))) {
				    $ipError = _("The IP address does not match the subnet.");
				}
				elseif (!$this->overlapd_ip($this->fixed_ip[$id]['ip'])) {
				    $ipError = _("The IP address is already in use.");
				}
				$error = '';
				if ($pcError != '') {
					$error .= ' ' . $pcError;
				}
				if ($macError != '') {
					$error .= ' ' . $macError;
				}
				if ($ipError != '') {
					$error .= ' ' . $ipError;
				}
				if ($descriptionError != '') {
					$error .= ' ' . $descriptionError;
				}
				$return->addElement(new htmlInputField('ip_'.$id, $this->fixed_ip[$id]['ip'], 20));
				$pcInput = new htmlInputField('pc_'.$id, $this->fixed_ip[$id]['cn'], 20);
				if (!empty($autoNames)) {
					$pcInput->enableAutocompletion($autoNames);
				}
				$return->addElement($pcInput);
				$return->addElement(new htmlInputField('mac_'.$id, $this->fixed_ip[$id]['mac'], 20));
				$return->addElement(new htmlInputField('description_'.$id, $this->fixed_ip[$id]['description'], 20));
				$return->addElement(new htmlInputCheckbox('active_'.$id, $this->fixed_ip[$id]['active']));
				$return->addElement(new htmlButton('drop_ip_'.$id, 'del.png', true));
				$return->addElement(new htmlOutputText($error), true);
			}
			$return->addElement(new htmlSpacer(null, '10px'), true);
			// add host
			$return->addElement(new htmlInputField('ip_add', '', 20));
			$newPCInput = new htmlInputField('pc_add', '', 20);
			if (!empty($autoNames)) {
				$newPCInput->enableAutocompletion($autoNames);
			}
			$return->addElement($newPCInput);
			$return->addElement(new htmlInputField('mac_add', '', 20));
			$return->addElement(new htmlInputField('description_add', '', 20));
			$return->addElement(new htmlInputCheckbox('active_add', true));
			$return->addElement(new htmlButton('add_ip', 'add.png', true), true);

			// add existing host entry
			if (!empty($this->hostCache)) {
				$return->addVerticalSpace('20px');
				$addHostButton = new htmlAccountPageButton(get_class($this), 'addHost', 'add', _('Add existing host'));
				$addHostButton->setIconClass('createButton');
				$return->addElement($addHostButton , true);
			}
		}
		return $return;
	}

	/**
	 * Returns the HTML meta data for the add host page.
	 *
	 * @return htmlElement HTML meta data
	 */
	public function display_html_addHost() {
		$return = new htmlTable();
		$this->initCache();
		$hostNames = array();
		$spacer = '####';
		foreach ($this->hostCache as $host) {
			if (!empty($host['cn'][0])) {
				$val = $host['cn'][0];
				if (!empty($host['iphostnumber'][0])) {
					$val .= $spacer . $host['iphostnumber'][0];
				}
				else {
					$val .= $spacer;
				}
				if (!empty($host['macaddress'][0])) {
					$val .= $spacer . $host['macaddress'][0];
				}
				else {
					$val .= $spacer;
				}
				$hostNames[$host['cn'][0]] = $val;
			}
		}
		$select = new htmlTableExtendedSelect('host', $hostNames, array(), _('Host'));
		$select->setHasDescriptiveElements(true);
		$return->addElement($select, true);
		$return->addVerticalSpace('20px');
		$buttonContainer = new htmlTable();
		$buttonContainer->colspan = 3;
		$buttonContainer->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'addHost', _('Add')));
		$buttonContainer->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'cancel', _('Cancel')));
		$return->addElement($buttonContainer, true);
		return $return;
	}

	/**
	* Processes user input of the add host page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	public function process_addHost() {
		$errors = array();
		if (isset($_POST['form_subpage_fixed_ip_attributes_addHost'])) {
			$val = explode('####', $_POST['host']);
		    $dhcpstatements = array();
		    $this->setActive($dhcpstatements, true);
		    $this->setIP($dhcpstatements, $val[1]);
			$this->fixed_ip[] = array(
				'cn' => $val[0],
				'mac' => $val[2],
				'description' => '',
				'ip' => $val[1],
				'dhcpstatements' => $dhcpstatements,
				'active' => true,
			);
			$this->orderByIP();
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

	}

	/**
	 * This function is overwritten because the fixed IPs are set after the ldap_add command.
	 *
	 * @see baseModule::postModifyActions()
	 *
	 * @param boolean $newAccount
	 * @param array $attributes LDAP attributes of this entry
	 * @return array array which contains status messages. Each entry is an array containing the status message parameters.
	 */
	public function postModifyActions($newAccount, $attributes) {
		if (!$this->isRootNode()) {
			$ldapSuffix = ',cn=' . $this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes['cn'][0] . ',' . $this->getAccountContainer()->get_type()->getSuffix();
		    $add = array();
		    $mod = array(); // DN => array(attr => values)
		    $delete = array();
			// Which dns are to delete and to add
			foreach($this->orig_ips AS $id => $arr) {
				// Exist cn still?
				$in_arr = false;
				foreach($this->fixed_ip AS $idB => $arr) {
				    if ($this->orig_ips[$id]['cn'] == $this->fixed_ip[$idB]['cn']) {
				        $in_arr = true;
						// check if IP changed
						if($this->orig_ips[$id]['ip'] != $this->fixed_ip[$idB]['ip']) {
							$this->setIP($this->fixed_ip[$idB]['dhcpstatements'], $this->fixed_ip[$idB]['ip']);
							$mod['cn=' . $this->orig_ips[$id]['cn'] . $ldapSuffix]['dhcpstatements'] = $this->fixed_ip[$idB]['dhcpstatements'];
						}
						// check if active changed
						if($this->orig_ips[$id]['active'] != $this->fixed_ip[$idB]['active']) {
							$this->setActive($this->fixed_ip[$idB]['dhcpstatements'], $this->fixed_ip[$idB]['active']);
							$mod['cn=' . $this->orig_ips[$id]['cn'] . $ldapSuffix]['dhcpstatements'] = $this->fixed_ip[$idB]['dhcpstatements'];
						}
						// check if MAC changed
						if($this->orig_ips[$id]['mac'] != $this->fixed_ip[$idB]['mac']) {
							$mod['cn=' . $this->orig_ips[$id]['cn'] . $ldapSuffix]['dhcpHWAddress'] = array('ethernet ' . $this->fixed_ip[$idB]['mac']);
						}
						// check if description changed
						if($this->orig_ips[$id]['description'] != $this->fixed_ip[$idB]['description']) {
							$mod['cn=' . $this->orig_ips[$id]['cn'] . $ldapSuffix]['dhcpComments'][] = $this->fixed_ip[$idB]['description'];
						}
						break;
				    }
				}
	            if (!$in_arr) {
	            	$delete[] = $this->orig_ips[$id]['cn'];
	            }
		    }

			if (!is_array($this->fixed_ip)) {
			    $this->fixed_ip = array();
			}
		    // Which entrys are new:
		    foreach($this->fixed_ip AS $id => $arr) {
		        $in_arr = false;
		        foreach($this->orig_ips AS $idB => $arr) {
		            if ($this->orig_ips[$idB]['cn'] == $this->fixed_ip[$id]['cn']) {
		                $in_arr = true;
		            }
		        }
		        if (!$in_arr) {
		            $add[] = $this->fixed_ip[$id];
		        }
		    }

			foreach($delete AS $cn) {
			    ldap_delete($_SESSION['ldap']->server(), 'cn=' . $cn . $ldapSuffix);
			}

		    foreach($add AS $id => $arr) {
				$attr = array();
				$attr['cn'] = $add[$id]['cn'];
				$attr['objectClass'][0] = 'top';
				$attr['objectClass'][1] = 'dhcpHost';
				$attr['dhcpHWAddress'] = 'ethernet ' . $add[$id]['mac'];
				if ($add[$id]['ip'] != '') {
					$attr['dhcpStatements'][] = 'fixed-address ' . $add[$id]['ip'];
				}
				if ($add[$id]['active'] === false) {
					$attr['dhcpStatements'][] = 'deny booting';
				}
				$attr['dhcpOption'] = 'host-name "' . $add[$id]['cn'] . '"';
				if (!empty($arr['description'])) {
					$attr['dhcpComments'][] = $arr['description'];
				}
				if ($attr['cn'] != "") {
					ldap_add($_SESSION['ldap']->server(), 'cn=' . $add[$id]['cn'] . $ldapSuffix, $attr);
				}
		    }
		    // entries to modify
		    foreach ($mod as $dn => $attrs) {
		    	ldap_modify($_SESSION['ldap']->server(), $dn, $attrs);
		    }
		}
		return array();
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	function get_pdfEntries($pdfKeys, $typeId) {
		$return = array();
		if (is_array($this->fixed_ip) && (sizeof($this->fixed_ip) > 0)) {
			$pdfTable = new PDFTable();
			$pdfRow = new PDFTableRow();
			$pdfRow->cells[] = new PDFTableCell(_('PC name'), '20%', null, true);
			$pdfRow->cells[] = new PDFTableCell(_('IP address'), '20%', null, true);
			$pdfRow->cells[] = new PDFTableCell(_('MAC address'), '20%', null, true);
			$pdfRow->cells[] = new PDFTableCell(_('Active'), '10%', null, true);
			$pdfRow->cells[] = new PDFTableCell(_('Description'), '30%', null, true);
			$pdfTable->rows[] = $pdfRow;
			for ($i = 0; $i < sizeof($this->fixed_ip); $i++) {
				$name = $this->fixed_ip[$i]['cn'];
				$mac = $this->fixed_ip[$i]['mac'];
				$ip = $this->fixed_ip[$i]['ip'];
				$active = _('yes');
				if (!$this->fixed_ip[$i]['active']) {
					$active = _('no');
				}
				$description = $this->fixed_ip[$i]['description'];
				$pdfRow = new PDFTableRow();
				$pdfRow->cells[] = new PDFTableCell($name, '20%');
				$pdfRow->cells[] = new PDFTableCell($ip, '20%');
				$pdfRow->cells[] = new PDFTableCell($mac, '20%');
				$pdfRow->cells[] = new PDFTableCell($active, '10%');
				$pdfRow->cells[] = new PDFTableCell($description, '30%');
				$pdfTable->rows[] = $pdfRow;
			}
			$this->addPDFTable($return, 'IPlist', $pdfTable);
		}
		return $return;
	}

	/**
	 * Extracts the IP from a list of DHCP statements.
	 *
	 * @param array $dhcpStatements values of dhcpStatements attribute
	 */
	public static function extractIP($dhcpStatements) {
		$return = null;
		if (is_array($dhcpStatements)) {
			for ($i = 0; $i < sizeof($dhcpStatements); $i++) {
				if (strpos($dhcpStatements[$i], 'fixed-address ') === 0) {
					$return = substr($dhcpStatements[$i], strlen('fixed-address') + 1);
					break;
				}
			}
		}
		return $return;
	}

	/**
	 * Sets the IP in a list of DHCP statements.
	 *
	 * @param array $dhcpStatements values of dhcpStatements attribute
	 * @param String $ip new IP
	 */
	private function setIP(&$dhcpStatements, $ip) {
		for ($i = 0; $i < sizeof($dhcpStatements); $i++) {
			if (strpos($dhcpStatements[$i], 'fixed-address ') === 0) {
				unset($dhcpStatements[$i]);
				$dhcpStatements = array_values($dhcpStatements);
			}
		}
		if (!empty($ip)) {
			$dhcpStatements[] = 'fixed-address ' . $ip;
		}
	}

	/**
	 * Returns if this host is active.
	 *
	 * @param array $dhcpStatements values of dhcpStatements attribute
	 */
	public static function isActive($dhcpStatements) {
		if (is_array($dhcpStatements)) {
			for ($i = 0; $i < sizeof($dhcpStatements); $i++) {
				if (strpos($dhcpStatements[$i], ' booting') === (strlen($dhcpStatements[$i]) - strlen(' booting'))) {
					$val = substr($dhcpStatements[$i], 0, (strlen($dhcpStatements[$i]) - strlen(' booting')));
					if ($val == 'deny') {
						return false;
					}
					break;
				}
			}
		}
		return true;
	}

	/**
	 * Sets if this host is active.
	 *
	 * @param array $dhcpStatements values of dhcpStatements attribute
	 * @param boolean $active is active
	 */
	private function setActive(&$dhcpStatements, $active) {
		for ($i = 0; $i < sizeof($dhcpStatements); $i++) {
			if (strpos($dhcpStatements[$i], ' booting') !== false) {
				unset($dhcpStatements[$i]);
				$dhcpStatements = array_values($dhcpStatements);
			}
		}
		if ($active) {
			$dhcpStatements[] = 'allow booting';
		}
		else {
			$dhcpStatements[] = 'deny booting';
		}
	}

	/**
	 * Loads cached host data from LDAP.
	 */
	private function initCache() {
		if ($this->hostCache != null) {
			return;
		}
		$attrs = array('cn', 'iphostnumber', 'macaddress');
		$this->hostCache = array();
		$result = searchLDAPByAttribute('cn', '*', null, $attrs, array('host'));
		foreach ($result as $attributes) {
			$this->hostCache[] = $attributes;
		}
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

}

?>
