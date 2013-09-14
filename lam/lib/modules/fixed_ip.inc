<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2008         Thomas Manninger
                2008 - 2013  Roland Gruber

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
	public $fixed_ip;
	
	/** already processed? */
	public $processed = false;

	/** for check if IPs overlap */
	public $overlapd;
	
	/** original IPs */
	public $orig_ips = array();
	
	/** LDAP attributes */
	public $attributes;

	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	* 
	* @see baseModule::get_metaData()
	*/
	public function get_metaData() {
		$return = array();
		// manages host accounts
		$return["account_types"] = array("dhcp");
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
	    if ($this->getAccountContainer()->dn_orig!=$_SESSION['config']->get_suffix('dhcp')) {
	    	return "enabled";
		}
		else {
		    return "hidden";
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
	    if ($this->getAccountContainer()->dn_orig!=$_SESSION['config']->get_suffix('dhcp')) {
	    	$entries = searchLDAP($this->getAccountContainer()->dn_orig, '(objectClass=dhcpHost)', array('cn', 'dhcphwaddress', 'dhcpstatements'));
			for ($i=0; $i < sizeof($entries); $i++) {
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
				
				$this->orig_ips[$i]['cn'] = $entries[$i]['cn'][0];
				$this->orig_ips[$i]['mac'] = $dhcphwaddress;
				$this->orig_ips[$i]['ip'] = self::extractIP($dhcpstatements);
				$this->orig_ips[$i]['active'] = self::isActive($dhcpstatements);
				$this->orig_ips[$i]['dhcpstatements'] = $dhcpstatements;
			}
		}
	}

	
	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	public function process_attributes() {
		$errors = array();
		if ($this->getAccountContainer()->dn_orig!=$_SESSION['config']->get_suffix('dhcp')) {
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
				$this->fixed_ip[] = array(
					'cn' => $_POST['pc_add'],
					'mac' => $_POST['mac_add'],
					'ip' => $_POST['ip_add'],
					'dhcpstatements' => array(),
					'active' => (isset($_POST['active_add']) && ($_POST['active_add'] == 'on')),
				);
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
        	// caption
        	$pcContainer = new htmlTable();
        	$pcContainer->addElement(new htmlOutputText(_('PC name') . "*"));
        	$pcContainer->addElement(new htmlHelpLink('pc'));
        	$return->addElement($pcContainer);
        	$macContainer = new htmlTable();
        	$macContainer->addElement(new htmlOutputText(_('MAC address') . "*"));
        	$macContainer->addElement(new htmlHelpLink('mac'));
        	$return->addElement($macContainer);
        	$ipContainer = new htmlTable();
        	$ipContainer->addElement(new htmlOutputText(_('IP address')));
        	$ipContainer->addElement(new htmlHelpLink('ip'));
        	$return->addElement($ipContainer);
        	$activeContainer = new htmlTable();
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
				$return->addElement(new htmlInputField('pc_'.$id, $this->fixed_ip[$id]['cn']));
				$return->addElement(new htmlInputField('mac_'.$id, $this->fixed_ip[$id]['mac']));
				$return->addElement(new htmlInputField('ip_'.$id, $this->fixed_ip[$id]['ip']));
				$return->addElement(new htmlInputCheckbox('active', $this->fixed_ip[$id]['active']));
				$return->addElement(new htmlButton('drop_ip_'.$id, 'del.png', true));
				$return->addElement(new htmlOutputText($error), true);
			}
			$return->addElement(new htmlSpacer(null, '10px'), true);
			// add fixed ip:
			$return->addElement(new htmlInputField('pc_add', ''));
			$return->addElement(new htmlInputField('mac_add', ''));
			$return->addElement(new htmlInputField('ip_add', ''));
			$return->addElement(new htmlInputCheckbox('active_add', true));
			$return->addElement(new htmlButton('add_ip', 'add.png', true));
		}
		return $return;
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
		if ($this->getAccountContainer()->dn_orig!=$_SESSION['config']->get_suffix('dhcp')) {
			$ldapSuffix = ',cn=' . $this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes['cn'][0] . ',' . $_SESSION['config']->get_suffix('dhcp');
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
						// check if MAC changed
						elseif($this->orig_ips[$id]['mac'] != $this->fixed_ip[$idB]['mac']) {
							$mod['cn=' . $this->orig_ips[$id]['cn'] . $ldapSuffix]['dhcpHWAddress'] = array('ethernet ' . $this->fixed_ip[$idB]['mac']);
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
				if ($attr['cn'] != "")
					ldap_add($_SESSION['ldap']->server(), 'cn=' . $add[$id]['cn'] . $ldapSuffix, $attr);
		    }
		    // entries to modify
		    foreach ($mod as $dn => $attrs) {
		    	ldap_modify($_SESSION['ldap']->server(), $dn, $attrs);
		    }
		}
		return array();
	}

	/**
	* Returns the PDF entries for this module.
	*
	* @return array list of possible PDF entries
	*/
	function get_pdfEntries() {
		$return = array();
		if (is_array($this->fixed_ip) && (sizeof($this->fixed_ip) > 0)) {
			$return[get_class($this) . '_IPlist'] = array(
				'<block><tr>' .
				'<td width="20%" align=\"L\"><b>' . _('PC name') . "</b></td>" .
				"<td width=\"20%\" align=\"L\"><b>" . _('IP address') . "</b></td>" .
				"<td width=\"20%\" align=\"L\"><b>" . _('MAC address') . '</b></td>' .
				"<td width=\"20%\" align=\"L\"><b>" . _('Active') . '</b></td>' .
			'</tr></block>');
			for ($i = 0; $i < sizeof($this->fixed_ip); $i++) {
				$name = $this->fixed_ip[$i]['cn'];
				$mac = $this->fixed_ip[$i]['mac'];
				$ip = $this->fixed_ip[$i]['ip'];
				$active = _('yes');
				if (!$this->fixed_ip[$i]['active']) {
					$active = _('no');
				}
				$return[get_class($this) . '_IPlist'][] = '<block><tr>' .
					'<td width="20%" align=\"L\">' . $name . "</td>" .
					"<td width=\"20%\" align=\"L\">" . $ip . " </td>" .
					"<td width=\"20%\" align=\"L\">" . $mac . '</td>'. 
					"<td width=\"20%\" align=\"L\">" . $active . '</td>'. 
				'</tr></block>';
			}
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
			if (strpos($dhcpStatements[$i], ' booting') === (strlen($dhcpStatements[$i]) - strlen(' booting'))) {
				unset($dhcpStatements[$i]);
				$dhcpStatements = array_values($dhcpStatements);
			}
		}
		if (!$active) {
			$dhcpStatements[] = 'allow booting ';
		}
	}
	
}

?>
