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
* Manages DHCP ranges for DHCP server.
*
* @package modules
*
* @author Thomas Manninger
*/

/**
* Manages DHCP ranges for DHCP server.
*
* @package modules
*/

class range extends baseModule {

	// subnet
	public $subnet;
	
	// ranges
	public $ranges;
	
	// Range -> Function attibute_processed already running?
	public $processed;
	
	// Are the ranges ok???
	public $ranges_ok;
	
	// For check, if IPs overlaped.
	public $overlaped;

	public function get_metaData() {

		$return = array();
		// manages dhcp accounts
		$return["account_types"] = array("dhcp");
		// alias name
		$return["alias"] = "Range";
		// this is a base module
		$return["is_base"] = false;
		// RDN attribute
		$return["RDN"] = array("cn" => "high");
		// LDAP filter
		$return["ldap_filter"] = array('or'=>"(objectClass=dhcpSubnet)");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array();
		// managed attributes
		$return['attributes'] = array();
		// help Entries
		$return['help'] = array(
				'range_from' => array(
				"Headline" => _("Range from"),
				"Text" => _("The starting IP address of the range.")
			) , 'range_to' => array(
				"Headline" => _("Range to"),
				"Text" => _("The ending IP address of the range.")
			) , 'drop_range' => array(
				"Headline" => _("Delete range"),
				"Text" => _("Deletes an IP range.")
			) , 'add_range' => array(
				"Headline" => _("New Range"),
				"Text" => _("Adds input fields for a new IP range.")
			) );
			
		// available PDF fields
		$return['PDF_fields'] = array();
		return $return;
	}
	
	public function load_Messages() {
		$this->messages['range_errors'][0] = array('ERROR', _('One or more errors occured. All wrong fields are marked.'), '');
		$this->messages['add_range'][0] = array('ERROR', _('New range'), _('Adding the range failed because errors occured.'));
		$this->messages['drop_range'][0] = array('ERROR', _('Delete range'), _('It is not possible to delete all ranges.'));
	}
	
	/**
	 *
	 * Checked, if it's a valid range
	 *
	 * @param first ip
	 * @param second ip
	 *
	 * @return true, if it's a valid Range, else false;
	 **/
	
	public function check_range($first_ip,$second_ip) {

		$ex_first = explode(".", $first_ip);
		$ex_second = explode(".", $second_ip);

		if ($ex_first[0]!=$ex_second[0])
		    return false;
		    
		if ($ex_first[1]!=$ex_second[1])
		    return false;

		if ($ex_first[2]!=$ex_second[2])
		    return false;

		if ($ex_first[3]>$ex_second[3]) {
			return false;
		}
		return true;
	}
	
	/**
	 *
	 * Check if the range and subnet are valid.
	 *
	 * @param IP
	 * @param Subnet
	 *
	 * @return true if the range and subnet valid, else false!
	 *
	 **/
	
	public function check_subnet_range($ip,$subnet) {

		// Check if the range was valid with the subnet:
		$ex = explode(".", $ip);
		$ex_subnet = explode(".", $subnet);

		if ($ex[0]==$ex_subnet[0] && $ex[1]==$ex_subnet[1] && $ex[2]==$ex_subnet[2]) {

			return true;
		}
		else
		{
		    return false;
		}

	}
	
	/**
	 *
	 * Checked, if Ranges are overlaped.
	 *
	 * @param first ip
	 * @param second ip
	 *
	 * @return false, if overlaped, else true.
	 *
	 **/
	function overlaped_range($ip,$ipB) {
		$ex = explode(".", $ip);
		$exB = explode(".", $ipB);
		
		if(!is_array($this->overlaped)) {
		    $this->overlaped = array();
		}
		for($n=$ex[3];$n<=$exB[3];$n++) {
		    if (in_array($n, $this->overlaped)) {
				return false;
		    }
		    else
		    {
		    	$this->overlaped[] = $n;
			}
		}
		return true;
	}
	
	/**
	 *
	 * Reset the overlaped_range() function
	 *
	 **/
	 function reset_overlaped_range() {
	    $this->overlaped = array();
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
	 * This function loads all needed LDAP attributes.
	 *
	 * @param array $attr list of attributes
	 */
	function load_attributes($attr) {
		parent::load_attributes($attr);
		// Load DHCP Options:
        if ($_SESSION['account']->getAccountModule('dhcp_settings')->dn!=$_SESSION['config']->get_suffix('dhcp')) {
			$this->orig = $attr;
			$this->attributes = $attr;


			// Load DHCP Options:
	        if (is_array($attr['dhcpRange'])) {
				foreach($attr['dhcpRange'] AS $id=>$value) {
				    $ex = explode(" ", $value);

					// DHCP Range ins Array laden:
					$this->ranges[$id] = array('range_start'=>$ex[0],'range_end'=>$ex[1]);
				}
			}
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
	    // Only run it, when ranges already exists:
	    if(is_array($this->ranges)) {
		    $ex_subnet = explode(".", $_SESSION['account']->getAccountModule('dhcp_settings')->attributes['cn'][0]);
		    $range_edit = false;        // Range were edit?
		    foreach ($this->ranges AS $id=>$arr) {
		        if (!empty($this->ranges[$id]['range_start']) && !$this->check_subnet_range($this->ranges[$id]['range_start'],$_SESSION['account']->getAccountModule('dhcp_settings')->$this->attributes['cn'][0])) {
		            // Range anpassen:
					$ex = explode(".", $this->ranges[$id]['range_start']);
					$tmp = $this->ranges[$id]['range_start'];
					$this->ranges[$id]['range_start'] = $ex_subnet['0'].".".$ex_subnet['1'].".".$ex_subnet['2'].".".$ex['3'];
					if($tmp!=$this->ranges[$id]['range_start'])
						$range_edit = true;
		        }
		        if (!empty($this->ranges[$id]['range_end']) && !$this->check_subnet_range($this->ranges[$id]['range_start'],$_SESSION['account']->getAccountModule('dhcp_settings')->$this->attributes['cn'][0])) {
		            // Range anpassen:
					$ex = explode(".", $this->ranges[$id]['range_end']);
					$tmp = $this->ranges[$id]['range_end'];
					$this->ranges[$id]['range_end'] = $ex_subnet['0'].".".$ex_subnet['1'].".".$ex_subnet['2'].".".$ex['3'];
					if($tmp!=$this->ranges[$id]['range_end'])
						$range_edit = true;
		        }
		    }
		    if ($range_edit) {
		        // sort the range new, id it was edit.
				foreach($this->ranges AS $id=>$arr) {
				    $this->attributes['dhcpRange'][$id] = $this->ranges[$id]['range_start']." ".$this->ranges[$id]['range_end'];
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
		$errors = array();
		
		$droped = false;    // Was a Range droped???
		if ($_SESSION['account']->getAccountModule('dhcp_settings')->dn!=$_SESSION['config']->get_suffix('dhcp')) {
			if ($_SESSION['account']->getAccountModule('dhcp_settings')->attributes['cn'][0]!="") {
			    $was_a_error = false;
			    $this->reset_overlaped_range();

			    foreach($this->ranges AS $id=>$arr) {

			        // Check if range is to drop
				    if (isset($_POST['drop_range_'.$id])) {
						// Drop Range:
						unset($this->ranges[$id]);
						unset($this->attributes['dhcpRange'][$id]);
				        $droped = true;
				        continue;
				    }

			        // if the inputs are empty, then do nothing:
			        if ($_POST['range_start_'.$id]=="" && $_POST['range_end_'.$id]=="") {
			            unset($this->attributes['dhcpRange'][$id]);
			        }
			        else
			        {
						// Check range_start:
						$_POST['range_start_'.$id] = trim($_POST['range_start_'.$id]);
				        if (!check_ip($_POST['range_start_'.$id])) {
				        	$this->ranges[$id]['range_start'] = $_POST['range_start_'.$id];
				        	$was_a_error = true;
				        }
				        else
				        {
				            $this->ranges[$id]['range_start'] = $_POST['range_start_'.$id];
				        }

						// Check end:
						$_POST['range_end_'.$id] = trim($_POST['range_end_'.$id]);
				        if (!check_ip($_POST['range_end_'.$id])) {
				        	$this->ranges[$id]['range_end'] = $_POST['range_end_'.$id];
				        	$was_a_error = true;
				        }
				        else
				        {
				            $this->ranges[$id]['range_end'] = $_POST['range_end_'.$id];
				        }

				        // Check if ip overlaped:
				        if(!$this->overlaped_range($_POST['range_start_'.$id],$_POST['range_end_'.$id])) {
				            $was_a_error = true;
				        }

						// Check if Subnet and range first are valid:
						if (!$this->check_subnet_range($_POST['range_start_'.$id],$_SESSION['account']->getAccountModule('dhcp_settings')->attributes['cn'][0])) {
						    $was_a_error = true;
						}

						// Check if Subnet and range last are valid:
						if (!$this->check_subnet_range($_POST['range_last_'.$id],$_SESSION['account']->getAccountModule('dhcp_settings')->$this->attributes['cn'][0])) {
						    $was_a_error = true;
						}

				        // Check if Range is valid
				        if (!$this->check_range($_POST['range_start_'.$id],$_POST['range_end_'.$id])) {
				            $was_a_error = true;
				        }

				        // Check, if range_start and range_end are ok!
				        if (!$was_a_error) {
				            $this->attributes['dhcpRange'][$id] = $_POST['range_start_'.$id]." ".$_POST['range_end_'.$id];
				            $this->ranges_ok = true;
				        }
				        else
				        {
				            unset($this->attributes['dhcpRange'][$id]);
							$this->ranges_ok = false;
				        }
					}
				}
			}

			// Check if there was a error:
			if ($was_a_error) {
			    $errors[] = $this->messages['range_errors'][0];
			}

			// Add new Range
			if(isset($_POST['add_range'])) {
				// Check, if there where no errors:
				if ($was_a_error) {
				    $errors[] = $this->messages['add_range'][0];
				}
				else
				{
				    // Add Range:
				    $this->ranges[] = array('range_start'=>'','range_end'=>'');
				}
			}


			$this->processed = true;
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
		if ($_SESSION['account']->getAccountModule('dhcp_settings')->attributes['cn'][0]=="") {
            echo "<b>" . _("Please fill the DHCP settings first.") . "</b>";
        }
        else
        {
        
            // If $ranges is not a array, then create an:
            if (!is_array($this->ranges)) {
            	$this->ranges[] = array();
            }
            $this->reset_overlaped_range();
            foreach($this->ranges AS $id=>$arr) {

				// Range start
				if ($this->processed && !check_ip($this->ranges[$id]['range_start'])) {
				    $error = "&laquo;&laquo; " . _("The IP address is invalid.");
				} elseif($this->processed && !$this->check_range($this->ranges[$id]['range_start'],$this->ranges[$id]['range_end'])) {
                    $error = "&laquo;&laquo; " . _("The range end needs to be greater than the range start.");
				} elseif ($this->processed && !$this->check_subnet_range($this->ranges[$id]['range_start'],$_SESSION['account']->getAccountModule('dhcp_settings')->attributes['cn'][0])) {
					$error = "&laquo;&laquo; " . _("The IP does not match the subnet.");
				} elseif ($this->processed && !$this->overlaped_range($this->ranges[$id]['range_start'],$this->ranges[$id]['range_end']) ) {
					$error = "&laquo;&laquo; " . _("The range conflicts with another range.");
				} else {
				    $error = "";
				}
		        $return[] = array(0 => array('kind' => 'text', 'text' => _('Range from') . ":* "),
					1 => array('kind' => 'input', 'name' => 'range_start_'.$id.'', 'value' => $this->ranges[$id]['range_start']),
					2 => array('kind' => 'help', 'value' => 'range_from', 'scope' => 'user'),
					3 => array('kind' => 'text', 'text'=>$error));

				// Range end
				if ($this->processed && !check_ip($this->ranges[$id]['range_end'])) {
				    $error = "&laquo;&laquo; " . _("The IP address is invalid.");
				} elseif ($this->processed && !$this->check_subnet_range($this->ranges[$id]['range_end'],$_SESSION['account']->getAccountModule('dhcp_settings')->attributes['cn'][0])) {
					$error = "&laquo;&laquo; " . _("The IP does not match the subnet.");
				} else {
				    $error = "";
				}
		        $return[] = array(0 => array('kind' => 'text', 'text' => _('Range to') . ":* "),
					1 => array('kind' => 'input', 'name' => 'range_end_'.$id.'', 'value' => $this->ranges[$id]['range_end']),
					2 => array('kind' => 'help', 'value' => 'range_to', 'scope' => 'user'),
					3 => array('kind' => 'text', 'text'=>$error));

				// Drop range:
				$return[] = array(
					0 => array('kind' => 'text', 'text' => _('Delete Range') . ':'),
					1 => array('kind' => 'input', 'name' => 'drop_range_'.$id, 'type' => 'submit', 'value' => _('Delete range')),
					2 => array('kind' => 'help', 'value' => 'drop_range'));

				// Space Line
		        $return[] = array(0 => array('kind' => 'text', 'text' => '<br />'));
			}
			
			// Range hinzufügen:
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('New range') . ':'),
				1 => array('kind' => 'input', 'name' => 'add_range', 'type' => 'submit', 'value' => _('New range')),
				2 => array('kind' => 'help', 'value' => 'add_range'));
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
		$return = array();
		// Get easy attributes
		if ($_SESSION['account']->getAccountModule('dhcp_settings')->dn!=$_SESSION['config']->get_suffix('dhcp')) {
			$return = $this->getAccountContainer()->save_module_attributes($this->attributes, $this->orig);
		}
		// Return attributes
		return $return;
	}
	
}
?>