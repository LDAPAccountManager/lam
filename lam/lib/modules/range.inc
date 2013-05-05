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
	
	/** Range -> Function attibute_processed already running? */
	public $processed;
	
	/** For check, if IPs overlaped. */
	public $overlaped;

	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	* 
	* @see baseModule::get_metaData()
	*/
	public function get_metaData() {
		$return = array();
		// manages dhcp accounts
		$return["account_types"] = array("dhcp");
		// alias name
		$return["alias"] = _("Ranges");
		// this is a base module
		$return["is_base"] = false;
		// icon
		$return['icon'] = 'ipRange.png';
		// RDN attribute
		$return["RDN"] = array("cn" => "high");
		// LDAP filter
		$return["ldap_filter"] = array('or'=>"(objectClass=dhcpSubnet)");
		// module dependencies
		$return['dependencies'] = array('depends' => array('dhcp_settings'), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array();
		// managed attributes
		$return['attributes'] = array('dhcpRange');
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
				"Headline" => _("New range"),
				"Text" => _("Adds input fields for a new IP range.")
			) );
			
		// available PDF fields
		$return['PDF_fields'] = array('ranges' => _('Ranges'));
		return $return;
	}
	
	/**
	 *  This function fills the error message array with messages.
	 */
	public function load_Messages() {
		$this->messages['range_errors'][0] = array('ERROR', _('One or more errors occured. The invalid fields are marked.'), '');
		$this->messages['add_range'][0] = array('ERROR', _('New range'), _('Adding the range failed because errors occured.'));
		$this->messages['drop_range'][0] = array('ERROR', _('Delete range'), _('It is not possible to delete all ranges.'));
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
		if ($ex_first[0]<$ex_second[0])
		    return true;
		if ($ex_first[1]<$ex_second[1])
		    return true;
		if ($ex_first[2]<$ex_second[2])
		    return true;
		if ($ex_first[3]<$ex_second[3]) {
			return true;
		}
		return false;
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
		    else {
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
	    if ($this->getAccountContainer()->dn_orig!=$_SESSION['config']->get_suffix('dhcp')) {
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
        if ($this->getAccountContainer()->dn_orig!=$_SESSION['config']->get_suffix('dhcp')) {
			$this->orig = $attr;
			$this->attributes = $attr;


			// Load DHCP Options:
	        if (isset($attr['dhcpRange']) && is_array($attr['dhcpRange'])) {
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
		    $ex_subnet = explode(".", $this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes['cn'][0]);
		    $range_edit = false;        // Range were edit?
		    $dhcpAttrs = $this->getAccountContainer()->getAccountModule('dhcp_settings')->getAttributes();
		    $mask = $this->getAccountContainer()->getAccountModule('dhcp_settings')->getDHCPOption('subnet-mask');
		    foreach ($this->ranges AS $id=>$arr) {
		        if (!empty($this->ranges[$id]['range_start']) && !range::check_subnet_range($this->ranges[$id]['range_start'],$dhcpAttrs['cn'][0], $mask)) {
		            // Range anpassen:
					$ex = explode(".", $this->ranges[$id]['range_start']);
					$tmp = $this->ranges[$id]['range_start'];
					$this->ranges[$id]['range_start'] = $ex_subnet['0'].".".$ex_subnet['1'].".".$ex_subnet['2'].".".$ex['3'];
					if($tmp!=$this->ranges[$id]['range_start'])
						$range_edit = true;
		        }
		        if (!empty($this->ranges[$id]['range_end']) && !range::check_subnet_range($this->ranges[$id]['range_end'],$dhcpAttrs['cn'][0], $mask)) {
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
		if ($this->getAccountContainer()->dn_orig!=$_SESSION['config']->get_suffix('dhcp')) {
			if ($this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes['cn'][0]!="") {
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
			        else {
			        	$mask = $this->getAccountContainer()->getAccountModule('dhcp_settings')->getDHCPOption('subnet-mask');
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
						if (!range::check_subnet_range($_POST['range_start_'.$id],$this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes['cn'][0], $mask)) {
						    $was_a_error = true;
						}

						// Check if Subnet and range last are valid:
						if (!range::check_subnet_range($_POST['range_end_'.$id],$this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes['cn'][0], $mask)) {
						    $was_a_error = true;
						}

				        // Check if Range is valid
				        if (!$this->check_range($_POST['range_start_'.$id],$_POST['range_end_'.$id])) {
				            $was_a_error = true;
				        }

				        // Check, if range_start and range_end are ok!
				        if (!$was_a_error) {
				            $this->attributes['dhcpRange'][$id] = $_POST['range_start_'.$id]." ".$_POST['range_end_'.$id];
				        }
				        else {
				            unset($this->attributes['dhcpRange'][$id]);
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

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return htmlElement HTML meta data
	 */
	public function display_html_attributes() {
		$return = new htmlTable();
		if ($this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes['cn'][0]=="") {
			$return->addElement(new htmlStatusMessage('INFO', _("Please fill out the DHCP settings first.")));
        }
        else {
        
            // If $ranges is not a array, then create it
            if (!is_array($this->ranges)) {
            	$this->ranges = array();
            }
            $this->reset_overlaped_range();
            $mask = $this->getAccountContainer()->getAccountModule('dhcp_settings')->getDHCPOption('subnet-mask');
            foreach($this->ranges AS $id=>$arr) {

				// Range start
				$error = "";
				if (isset($this->ranges[$id]['range_start']) && !empty($this->ranges[$id]['range_start'])) {
					if ($this->processed && !check_ip($this->ranges[$id]['range_start'])) {
					    $error = _("The IP address is invalid.");
					} elseif($this->processed && !$this->check_range($this->ranges[$id]['range_start'],$this->ranges[$id]['range_end'])) {
	                    $error = _("The range end needs to be greater than the range start.");
					} elseif ($this->processed && !range::check_subnet_range($this->ranges[$id]['range_start'],$this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes['cn'][0], $mask)) {
						$error = _("The IP does not match the subnet.");
					} elseif ($this->processed && !$this->overlaped_range($this->ranges[$id]['range_start'],$this->ranges[$id]['range_end']) ) {
						$error = _("The range conflicts with another range.");
					}
				}
				$fromInput = new htmlTableExtendedInputField(_('Range from'), 'range_start_'.$id, $this->ranges[$id]['range_start'], 'range_from');
				$fromInput->setRequired(true);
				$return->addElement($fromInput);
				$return->addElement(new htmlOutputText($error), true);
				// Range end
				$error = "";
				if (isset($this->ranges[$id]['range_end']) && !empty($this->ranges[$id]['range_end'])) {
					if ($this->processed && !check_ip($this->ranges[$id]['range_end'])) {
					    $error = _("The IP address is invalid.");
					} elseif ($this->processed && !range::check_subnet_range($this->ranges[$id]['range_end'],$this->getAccountContainer()->getAccountModule('dhcp_settings')->attributes['cn'][0], $mask)) {
						$error = _("The IP does not match the subnet.");
					}
				}
				$toInput = new htmlTableExtendedInputField(_('Range to'), 'range_end_'.$id, $this->ranges[$id]['range_end'], 'range_to');
				$toInput->setRequired(true);
				$return->addElement($toInput);
				$return->addElement(new htmlOutputText($error), true);
				// Drop range:
				$dropButton = new htmlButton('drop_range_'.$id, _('Delete range'));
				$dropButton->colspan = 2;
				$return->addElement($dropButton);
				$return->addElement(new htmlHelpLink('drop_range'), true);

				$return->addElement(new htmlSpacer(null, '10px'), true);
			}
			
			// add new range
			$addButton = new htmlButton('add_range', _('New range'));
			$addButton->colspan = 2;
			$return->addElement($addButton);
			$return->addElement(new htmlHelpLink('add_range'));
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
		$return = array();
		// Get easy attributes
		if ($this->getAccountContainer()->dn_orig!=$_SESSION['config']->get_suffix('dhcp')) {
			$return = $this->getAccountContainer()->save_module_attributes($this->attributes, $this->orig);
		}
		// Return attributes
		return $return;
	}
	
	/**
	* Returns the PDF entries for this module.
	*
	* @return array list of possible PDF entries
	*/
	function get_pdfEntries() {
		$return = array();
		if (is_array($this->ranges) && (sizeof($this->ranges) > 0)) {
			$start = $this->ranges[0]['range_start'];
			$end = $this->ranges[0]['range_end'];
			$return[get_class($this) . '_ranges'] = array('<block><key>' . _('Ranges') . '</key><tr><td align=\"L\">' . $start . " - " . $end . '</td></tr></block>');
			for ($i = 1; $i < sizeof($this->ranges); $i++) {
				$start = $this->ranges[$i]['range_start'];
				$end = $this->ranges[$i]['range_end'];
				$return[get_class($this) . '_ranges'][] = '<block><tr><td align=\"L\">' . $start . " - " . $end . '</td></tr></block>';
			}
		}
		return $return;
	}
	
}

?>
