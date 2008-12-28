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
* Manages fixed IP addresses.
*
* @package modules
*
* @author Thomas Manninger
*/

/**
* Manages fixed IP addresses.
*
* @package modules
*/

class fixed_ip extends baseModule {

	// fixed ips
	public $fixed_ips;
	
	// already processed?
	public $processed = false;

	// For check, if IPs overlapd.
	public $overlapd;
	
	// Original IPs
	public $orig_ips = array();
	
	public $attributes;

	public function get_metaData() {

		$return = array();
		// manages host accounts
		$return["account_types"] = array("dhcp");
		// alias name
		$return["alias"] = _("Fixed IP");
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
		$return['attributes'] = array('dhcpOption');
		// help Entries
		$return['help'] = array(
				'pc' => array(
				"Headline" => _("PC name"),
				"Text" => _("The name of the PC.")
			) , 'mac' => array(
				"Headline" => _("MAC address"),
				"Text" => _("The MAC address of the PC. Example: 11:22:33:44:55:aa")
			) , 'ip' => array(
				"Headline" => _("IP address"),
				"Text" => _("The IP address of the PC.")
			) , 'drop_ip' => array(
				"Headline" => _("Delete IP"),
				"Text" => _("Deletes a fixed IP address.")
			) , 'add_ip' => array(
				"Headline" => _("Add IP"),
				"Text" => _("Adds input fields for a new fixed IP address.")
			) );
		// available PDF fields
		$return['PDF_fields'] = array();
		
		return $return;
	}
	
	public function load_Messages() {
		$this->messages['errors'][0] = array('ERROR', _('One or more errors occured. The invalid fields are marked.'), '');
		$this->messages['add_ip'][0] = array('ERROR', _('Adding of a fixed IP failed because of errors.'), '');
	}
	
	/**
	* Controls if the module button the account page is visible and activated.
	*
	* @return string status ("enabled", "disabled", "hidden")
	*/
	public function getButtonStatus() {
	    if ($_SESSION['account']->getAccountModule('dhcp_settings')->dn!=$_SESSION['config']->get_suffix('dhcp')) {
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
			if (!eregi("[0-9a-fA-F][0-9a-fA-F]", $value) || strlen($value)!="2") {
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
	    // Only run it, when ranges already exists:
	    if(is_array($this->fixed_ip)) {
		    $ex_subnet = explode(".", $_SESSION['account']->getAccountModule('dhcp_settings')->attributes['cn'][0]);
		    $ip_edit = false;        // Range were edit?
		    foreach ($this->fixed_ip AS $id=>$arr) {
		        if (!empty($this->fixed_ip[$id]['ip']) && !$_SESSION['account']->getAccountModule('range')->check_subnet_range($this->fixed_ip[$id]['ip'],$_SESSION['account']->getAccountModule('dhcp_settings')->$this->attributes['cn'][0])) {
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
	
	    if ($_SESSION['account']->getAccountModule('dhcp_settings')->dn!=$_SESSION['config']->get_suffix('dhcp')) {
	    
		    $sr = @ldap_search($_SESSION['ldap']->server(),'cn='.$_SESSION['account']->getAccountModule('dhcp_settings')->attributes['cn'][0].','.$_SESSION['config']->get_suffix('dhcp'),'(objectClass=dhcpHost)');
		    if ($sr) {
		        $entries = ldap_get_entries($_SESSION['ldap']->server(), $sr);
				for ($i=0; $i < $entries["count"]; $i++)
				{
					$this->fixed_ip[$i]['cn'] = $entries[$i]['cn'][0];
					$this->fixed_ip[$i]['mac'] = array_pop(explode(" ", $entries[$i]['dhcphwaddress'][0]));
					$this->fixed_ip[$i]['ip'] = array_pop(explode(" ", $entries[$i]['dhcpstatements'][0]));
	
					$this->orig_ips[$i]['cn'] = $entries[$i]['cn'][0];
					$this->orig_ips[$i]['mac'] = array_pop(explode(" ", $entries[$i]['dhcphwaddress'][0]));
					$this->orig_ips[$i]['ip'] = array_pop(explode(" ", $entries[$i]['dhcpstatements'][0]));
				}
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
		if ($_SESSION['account']->getAccountModule('dhcp_settings')->dn!=$_SESSION['config']->get_suffix('dhcp')) {
			$this->processed = true;

			$this->reset_overlapd_ip();

			if ($_SESSION['account']->getAccountModule('dhcp_settings')->attributes['cn'][0]!="") {

				$error = false;     // errors by process_attributes()?
				$pcs = array();
				foreach($this->fixed_ip AS $id=>$arr) {

			        // Check if ip is to drop
				    if (isset($_POST['drop_ip_'.$id])) {
						// Drop ip:
						unset($this->fixed_ip[$id]);
				        continue;
				    }

				    // If all three inputs are empty, then do nothing:
				    if (!empty($_POST['pc_'.$id]) && !empty($_POST['mac_'.$id]) && !empty($_POST['ip_'.$id]) ) {
						// MAC address
						$_POST['mac_'.$id] = strtolower(trim($_POST['mac_'.$id]));

						$invalid = $this->check_mac($_POST['mac_'.$id]);
						if ($invalid) {
						    $error = true;
						}
						else
						{
						    $this->attributes['dhcpHWAddress'][0] = "ethernet ". $_POST['mac'];
						}
						$this->fixed_ip[$id]['mac'] = $_POST['mac_'.$id];

						// Ip address
						if (!empty($_POST['ip_'.$id])) $_POST['ip_'.$id] = trim($_POST['ip_'.$id]);
						if (!check_ip($_POST['ip_'.$id]) && !empty($_POST['ip_'.$id])) {
							$error = true;
							$this->fixed_ip[$id]['ip'] = $_POST['ip_'.$id];
						}
						elseif (empty($_POST['ip_'.$id])) {
							$error = true;
							$this->fixed_ip[$id]['ip'] = $_POST['ip_'.$id];
						}
						elseif (!$this->overlapd_ip($_POST['ip_'.$id])) {
							$error = true;
							$this->fixed_ip[$id]['ip'] = $_POST['ip_'.$id];
						}
						else
						{
							$this->fixed_ip[$id]['ip'] = $_POST['ip_'.$id];
						}

						// Is ip correct with subnet:
						if (!$_SESSION['account']->getAccountModule('range')->check_subnet_range($_POST['ip_'.$id], $_SESSION['account']->getAccountModule('dhcp_settings')->attributes['cn'][0]) ) {
						    $error = true;
						}


						// cn:
						if (!empty($_POST['pc_'.$id])) $_POST['pc_'.$id] = trim($_POST['pc_'.$id]);
		                if (!empty($_POST['pc_'.$id])) {

							// Already use?
							if (in_array($_POST['pc_'.$id], $pcs) ) {
							    $error = true;
							}
							else
							{
							    $pcs[] = $_POST['pc_'.$id];
							}
						}
						else
						{
							$error = true;
						}
						if (strlen($_POST['pc_'.$id])>30) {
						    $error = true;
						}
						if (!eregi("^[A-Za-z0-9\._-]*$",$_POST['pc_'.$id])) {
						    $error = true;
						}
						$this->fixed_ip[$id]['cn'] = $_POST['pc_'.$id];
					}
				}
				if ($error) {
				    $errors[] = $this->messages['errors'][0];
				}
			}

			// Add new IP
			if(isset($_POST['add_ip'])) {
				// Check, if there where no errors:
				if ($error) {
				    $errors[] = $this->messages['add_ip'][0];
				}
				else
				{
				    // Add IP:
					$this->fixed_ip[] = array('cn'=>'','mac'=>'','ip'=>'');
				}
			}
		}
		
		return $errors;
	}
	
	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	public function display_html_attributes() {

		if ($_SESSION['account']->getAccountModule('dhcp_settings')->attributes['cn'][0]=="") {
            echo "<b>" . _("Please fill out the DHCP settings first.") . "</b>";
        }
        else
        {
			// Reset oberldapd ips
			$this->reset_overlapd_ip();

			// If $ranges is not a array, then create an:
			if (!is_array($this->fixed_ip)) {
				$this->fixed_ip[] = array();
			}
            $pcs = array();
	        foreach($this->fixed_ip AS $id=>$arr) {
				// pc name
				$result = @ldap_search($_SESSION['ldap']->server(),"cn=".$_SESSION['account']->getAccountModule('dhcp_settings')->attributes['cn'][0].",".$_SESSION['config']->get_Suffix('dhcp'),'(cn='.$_POST['pc_'.$id].')');
	            $num = (@ldap_get_entries($_SESSION['ldap']->server(), $result)=="")?0:ldap_get_entries($_SESSION['ldap']->server(), $result);
				$error = "";
				if (!$this->processed) {
                	$error = "";
				}
				elseif (strlen($this->fixed_ip[$id]['cn'])>20) {
				    $error = "&laquo;&laquo; " . _("The PC name may not be longer than 20 characters.");
				}
				elseif (strlen($this->fixed_ip[$id]['cn'])<2) {
				    $error = "&laquo;&laquo; " . _("The PC name needs to be at least 2 characters long.");
				}
				elseif (in_array($this->fixed_ip[$id]['cn'], $pcs) ) {
				    $error="&laquo;&laquo; " . _("This PC name already exists.");
				}
				elseif (!eregi("^[A-Za-z0-9\._-]*$",$_POST['pc_'.$id])) {
					$error="&laquo;&laquo; " . _("The PC name may only contain A-Z, a-z and 0-9.");
				}
				$pcs[] = $this->fixed_ip[$id]['cn'];
				$return[] = array(
					array('kind' => 'text', 'text' => _('PC name') . ":* "),
					array('kind' => 'input', 'name' => 'pc_'.$id.'', 'value' => $this->fixed_ip[$id]['cn']),
					array('kind' => 'help', 'value' => 'pc', 'scope' => 'user'),
                    array('kind' => 'text', 'text'=>$error));

				// MAC address
				$error = "";
				if (!$this->processed) {
                	$error = "";
				}
				elseif ($this->check_mac($this->fixed_ip[$id]['mac'])) {
				    $error = "&laquo;&laquo; " . _("Invalid MAC address.");
				}
				$return[] = array(
					array('kind' => 'text', 'text' => _('MAC address') . ":* "),
					array('kind' => 'input', 'name' => 'mac_'.$id.'', 'value' => $this->fixed_ip[$id]['mac']),
					array('kind' => 'help', 'value' => 'mac', 'scope' => 'user'),
                    array('kind' => 'text', 'text'=>$error));
                    
				// fixed ip
				$error = "";
				if (!$this->processed) {
                	$error = "";
				}
				elseif (!$_SESSION['account']->getAccountModule('range')->check_subnet_range($this->fixed_ip[$id]['ip'], $_SESSION['account']->getAccountModule('dhcp_settings')->attributes['cn'][0]) ) {
				    $error = "&laquo;&laquo; " . _("The IP address does not match the subnet.");
				}
				elseif (!$this->overlapd_ip($this->fixed_ip[$id]['ip'])) {
				    $error = "&laquo;&laquo; " . _("The IP address is already in use.");
				}
				$return[] = array(
					array('kind' => 'text', 'text' => _('IP address') . ":* "),
					array('kind' => 'input', 'name' => 'ip_'.$id.'', 'value' => $this->fixed_ip[$id]['ip']),
					array('kind' => 'help', 'value' => 'ip', 'scope' => 'user'),
					array('kind' => 'text', 'text'=>$error));

				// fixed_ip drop:
				$return[] = array(
					array('kind' => 'text', 'text' => _('Delete IP') . ':'),
					array('kind' => 'input', 'name' => 'drop_ip_'.$id, 'type' => 'submit', 'value' => _('Delete IP')),
					array('kind' => 'help', 'value' => 'drop_ip'));

				// Space Line
			    $return[] = array(array('kind' => 'text', 'text' => '<br />'));
			}
			// add fixed ip:
			$return[] = array(
				array('kind' => 'text', 'text' => _('Add IP') . ':'),
				array('kind' => 'input', 'name' => 'add_ip', 'type' => 'submit', 'value' => _('Add IP')),
				array('kind' => 'help', 'value' => 'add_ip'));
		}
		
		return $return;
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

	}
	
	/**
	 * This function will be overwrite, because this function
	 * creates the fixed ips after ldap_add command.
	 **/
	public function postModifyActions($newAccount) {
	    if ($_SESSION['account']->getAccountModule('dhcp_settings')->dn!=$_SESSION['config']->get_suffix('dhcp')) {
		    $add = array();
		    $delete = array();
			// Which dns are to delete and to add
			foreach($this->orig_ips AS $id=>$arr) {
				// Exist cn still?
				$in_arr = false;
				foreach($this->fixed_ip AS $idB=>$arr) {
				    if ($this->orig_ips[$id]['cn']==$this->fixed_ip[$idB]['cn']) {
				        $in_arr = true;

						// Was cn edit?
						if($this->orig_ips[$id]['ip']!=$this->fixed_ip[$idB]['ip']) {
							$delete[] = $this->orig_ips[$id]['cn'];
							$add[] = $this->fixed_ip[$idB];
						}
					}
				}
	            if (!$in_arr) {
	            	$delete[] = $this->orig_ips[$id]['cn'];
	            }
		    }

			if (!is_array($this->fixed_ip))
			    $this->fixed_ip = array();

		    // Which entrys are new:
		    foreach($this->fixed_ip AS $id=>$arr) {
		        $in_arr = false;
		        foreach($this->orig_ips AS $idB=>$arr) {
		            if ($this->orig_ips[$idB]['cn']==$this->fixed_ip[$id]['cn']) {
		                $in_arr = true;
		            }
		        }
		        if (!$in_arr) {
		            $add[] = $this->fixed_ip[$id];
		        }
		    }

			foreach($delete AS $dn) {
			    ldap_delete($_SESSION['ldap']->server(),'cn='.$dn.',cn='.$_SESSION['account']->getAccountModule('dhcp_settings')->attributes['cn'][0].','.$_SESSION['config']->get_suffix('dhcp'));
			}

		    foreach($add AS $id=>$arr) {
				$attr = array();
				$attr['cn'] = $add[$id]['cn'];
				$attr['objectClass'][0] = 'top';
				$attr['objectClass'][1] = 'dhcpHost';
				$attr['dhcpHWAddress'] = 'ethernet '.$add[$id]['mac'];
				$attr['dhcpStatements'] = 'fixed-address '.$add[$id]['ip'];
				if ($attr['cn']!="")
					ldap_add($_SESSION['ldap']->server(),'cn='.$add[$id]['cn'].',cn='.$_SESSION['account']->getAccountModule('dhcp_settings')->attributes['cn'][0].','.$_SESSION['config']->get_suffix('dhcp'),$attr);
		    }
		}
	}
}
?>