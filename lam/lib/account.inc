<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Tilo Lutz

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
* This provides several helper function for the account modules.
*
* @author Tilo Lutz
* @author Roland Gruber
*
* @package modules
*/


/* Return a list of all shells listed in ../config/shells
* Normally ../config/shells is a symbolic link to /etc/shells
*/
function getshells() {
	// Load shells from file
	$shells =  file($_SESSION['lampath'].'config/shells');
	$i=0;
	while (count($shells) > $i) {
		// remove whitespaces
		trim($shells[$i]);
		// remove lineend
		$shells[$i] = substr($shells[$i], 0, strpos($shells[$i], "\n"));
		// remove comments
		if ($shells[$i]{0}=='#') unset ($shells[$i]);
		 else $i++;
		}
	// $shells is array with all valid shells
	return $shells;
	}


/* This function will replace umlates with ascci-chars
* fixme ***
* In order to map all non-ascii characters this function should be changed
*/
function replace_umlaut($text) {
	$aTranslate = array("�"=>"ae", "�"=>"Ae",
		"�"=>"oe", "�"=>"Oe",
		"�"=>"ue", "�"=>"Ue",
		"�"=>"ss"
		);
	return strtr($text, $aTranslate);
	}


/* This function will return all values from $array without values of $values
* $values, $array and $return are arrays
*/
function array_delete($values, $array) {
	// Loop for every entry and check if it should be removed
	if (is_array($array)) {
		$return = array();
		foreach ($array as $array_value)
			if (!@in_array($array_value, $values))
				$return[] = $array_value;
		return $return;
		}
	else return array();
	}


// This function will return a password with max. 8 characters
function genpasswd() {
	// Allowed Characters to generate passwords
	// I'Ve removed characters like l and 1 because they are too similar
	$LCase = 'abcdefghjkmnpqrstuvwxyz';
	$UCase = 'ABCDEFGHJKMNPQRSTUVWXYZ';
	$Integer = '23456789';
	// DEFINE CONSTANTS FOR ALGORTTHM
	define("LEN", '1');
	$a = RndInt('letter');
	$b = RndInt('letter');
	$c = RndInt('letter');
	$d = RndInt('letter');
	$e = RndInt('number');
	$f = RndInt('number');
	$g = RndInt('letter');
	$h = RndInt('letter');
	// EXTRACT 8 CHARACTERS RANDOMLY FROM TH // E DEFINITION STRINGS
	$L1 = substr($LCase, $a, LEN);
	$L2 = substr($LCase, $b, LEN);
	$L3 = substr($LCase, $h, LEN);
	$U1 = substr($UCase, $c, LEN);
	$U2 = substr($UCase, $d, LEN);
	$U3 = substr($UCase, $g, LEN);
	$I1 = substr($Integer, $e, LEN);
	$I2 = substr($Integer, $f, LEN);
	// COMBINE THE CHARACTERS AND DISPLAY TH // E NEW PASSWORD
	$PW = $L1 . $U2 . $I1 . $L2 . $I2 . $U1 . $U3 . $L3;
	return $PW;
	}


/* THIS FUNCTION GENERATES A RANDOM NUMBER THAT WILL BE USED TO
* RANDOMLY SELECT CHARACTERS FROM THE STRINGS ABOVE
*/
function RndInt($Format){
	switch ($Format){
		case 'letter':
			$Rnd = rand(0,23);
			if ($Rnd > 23){
				$Rnd = $Rnd - 1;
				}
			break;
		case 'number':
			$Rnd = rand(2,9);
			if ($Rnd > 8){
				$Rnd = $Rnd - 1;
				}
			break;
		}
	return $Rnd;
	} // END RndInt() FUNCTION

// This function will return the days from 1.1.1970 until now
function getdays() {
	$days = time() / 86400;
	settype($days, 'integer');
	return $days;
	}

/* This function creates all attributes stored in attrFlags. It's the same
* syntax used in smbpasswd
* $values is an array of samba flags as defined in account object
* Return value is a string
*/
function smbflag($input) {
	// Start character
	$flag = "[";
	// Add Options
	if ($input['W']) $flag .= "W"; else $flag .= "U";
	if ($input['D']) $flag .= "D";
	if ($input['X']) $flag .= "X";
	if ($input['N']) $flag .= "N";
	if ($input['S']) $flag .= "S";
	if ($input['H']) $flag .= "H";
	// Expand string to fixed length
	$flag = str_pad($flag, 12);
	// End character
	$flag = $flag. "]";
	return $flag;
	}

/**
* Generates the LM hash of a password.
*
* @param string password original password
* @return string password hash
*/
function lmPassword($password) {
	return exec(escapeshellarg($_SESSION['lampath'] . 'lib/createntlm.pl') . " lm " . escapeshellarg($password));
}

/**
* Generates the NT hash of a password.
*
* @param string password original password
* @return string password hash
*/
function ntPassword($password) {
	return exec(escapeshellarg($_SESSION['lampath'] . 'lib/createntlm.pl') . " nt " . escapeshellarg($password));
}

/**
* Returns the hash value of a plain text password
* the hash algorithm depends on the configuration file
*
* @param string $password the password string
* @param boolean $enabled marks the hash as enabled/disabled (e.g. by prefixing "!")
* @param string $hashType password hash type (CRYPT, SHA, SSHA, MD5, SMD5, PLAIN)
* @return string the password hash
*/
function pwd_hash($password, $enabled = true, $hashType = 'SSHA') {
	// check for empty password
	if (! $password || ($password == "")) {
		return "";
	}
	// calculate new random number
	$_SESSION['ldap']->new_rand();
	$hash = "";
	switch ($hashType) {
		case 'CRYPT':
			$hash = "{CRYPT}" . crypt($password);
			break;
		case 'MD5':
			$hash = "{MD5}" . base64_encode(hex2bin(md5($password)));
			break;
		case 'SMD5':
				$salt0 = substr(pack("h*", md5($_SESSION['ldap']->rand)), 0, 8);
				$salt = substr(pack("H*", md5($salt0 . $password)), 0, 4);
				$hash = "{SMD5}" . base64_encode(hex2bin(md5($password . $salt)) . $salt);
			break;
		case 'SHA':
			// PHP 4.3+ can use sha1() function
			if (function_exists(sha1)) {
				$hash = "{SHA}" . base64_encode(hex2bin(sha1($password)));
			}
			// otherwise use MHash
			elseif (function_exists(mHash)) {
				$hash = "{SHA}" . base64_encode(mHash(MHASH_SHA1, $password));
			}
			// if SHA1 is not possible use crypt()
			else {
				$hash = "{CRYPT}" . crypt($password);
			}
			break;
		case 'SSHA':
			// PHP 4.3+ can use sha1() function
			if (function_exists(sha1)) {
				$salt0 = substr(pack("h*", md5($_SESSION['ldap']->rand)), 0, 8);
				$salt = substr(pack("H*", sha1($salt0 . $password)), 0, 4);
				$hash = "{SSHA}" . base64_encode(hex2bin(sha1($password . $salt)) . $salt);
			}
			// otherwise use MHash
			elseif (function_exists(mHash)) {
				$salt = mhash_keygen_s2k(MHASH_SHA1, $password, substr(pack("h*", md5($_SESSION['ldap']->rand)), 0, 8), 4);
				$hash = base64_encode(mHash(MHASH_SHA1, $password . $salt) . $salt);
				$hash = "{SSHA}" . $hash;
			}
			// if SSHA is not possible use crypt()
			else {
				$hash = "{CRYPT}" . crypt($password);
			}
			break;
		case 'PLAIN':
			$hash = $password;
			break;
		// use SSHA if the setting is invalid
		default:
			// PHP 4.3+ can use sha1() function
			if (function_exists(sha1)) {
				$salt0 = substr(pack("h*", md5($_SESSION['ldap']->rand)), 0, 8);
				$salt = substr(pack("H*", sha1($salt0 . $password)), 0, 4);
				$hash = "{SSHA}" . base64_encode(hex2bin(sha1($password . $salt)) . $salt);
			}
			// otherwise use MHash
			elseif (function_exists(mHash)) {
				$salt = mhash_keygen_s2k(MHASH_SHA1, $password, substr(pack("h*", md5($_SESSION['ldap']->rand)), 0, 8), 4);
				$hash = base64_encode(mHash(MHASH_SHA1, $password . $salt) . $salt);
				$hash = "{SSHA}" . $hash;
			}
			// if SSHA is not possible use crypt()
			else {
				$hash = "{CRYPT}" . crypt($password);
			}
		break;
	}
	// enable/disable password
	if (! $enabled) return pwd_disable($hash);
	else return $hash;
}

/**
* Marks an password hash as enabled and returns the new hash string
*
* @param string $hash hash value to enable
* @return string enabled password hash
*/
function pwd_enable($hash) {
	// check if password is disabled (old wrong LAM method)
	if ((substr($hash, 0, 2) == "!{") || ((substr($hash, 0, 2) == "*{"))) {
		return substr($hash, 1, strlen($hash));
	}
	// check for "!" or "*" at beginning of password hash
	else {
		if (substr($hash, 0, 1) == "{") {
			$pos = strpos($hash, "}");
			if ((substr($hash, $pos + 1, 1) == "!") || (substr($hash, $pos + 1, 1) == "*")) {
				// enable hash
				return substr($hash, 0, $pos + 1) . substr($hash, $pos + 2, strlen($hash));
			}
			else return $hash;  // not disabled
		}
		else return $hash;  // password is plain text
	}
}

/**
* Marks an password hash as disabled and returns the new hash string
*
* @param string $hash hash value to disable
* @return string disabled hash value
*/
function pwd_disable($hash) {
	// check if password is disabled (old wrong LAM method)
	if ((substr($hash, 0, 2) == "!{") || ((substr($hash, 0, 2) == "*{"))) {
		return $hash;
	}
	// check for "!" or "*" at beginning of password hash
	else {
		if (substr($hash, 0, 1) == "{") {
			$pos = strpos($hash, "}");
			if ((substr($hash, $pos + 1, 1) == "!") || (substr($hash, $pos + 1, 1) == "*")) {
				// hash already disabled
				return $hash;
			}
			else return substr($hash, 0, $pos + 1) . "!" . substr($hash, $pos + 1, strlen($hash));  // not disabled
		}
		else return $hash;  // password is plain text
	}
}

/**
* Checks if a password hash is enabled/disabled
*
* @param string $hash password hash to check
* @return boolean true if the password is marked as enabled
*/
function pwd_is_enabled($hash) {
	// disabled passwords have a "!" or "*" at the beginning (old wrong LAM method)
	if ((substr($hash, 0, 2) == "!{") || ((substr($hash, 0, 2) == "*{"))) return false;
	if (substr($hash, 0, 1) == "{") {
		$pos = strrpos($hash, "}");
		// check if hash starts with "!" or "*"
		if ((substr($hash, $pos + 1, 1) == "!") || (substr($hash, $pos + 1, 1) == "*")) return false;
		else return true;
	}
	else return true;
}

	/**
	* Returns an array with all Samba 3 domain entries under the given suffix
	*
	* @param string $suffix search suffix
	* @return array list of samba3domain objects
	*/
	function search_domains($suffix) {
		$ret = array();
		$attr = array("DN", "sambaDomainName", "sambaSID", "sambaNextRid", "sambaNextGroupRid",
			"sambaNextUserRid", "sambaAlgorithmicRidBase");
		$sr = @ldap_search($_SESSION['ldap']->server(), $suffix, "objectClass=sambaDomain", $attr);
		if ($sr) {
			$units = ldap_get_entries($_SESSION['ldap']->server, $sr);
			// delete count entry
			array_shift($units);
			// extract attributes
			for ($i = 0; $i < sizeof($units); $i++) {
				$ret[$i] = new samba3domain();
				$ret[$i]->dn = $units[$i]['dn'];
				$ret[$i]->name = $units[$i]['sambadomainname'][0];
				$ret[$i]->SID = $units[$i]['sambasid'][0];
				$ret[$i]->nextRID = $units[$i]['sambanextrid'][0];
				$ret[$i]->nextGroupRID = $units[$i]['sambanextgrouprid'][0];
				$ret[$i]->nextUserRID = $units[$i]['sambanextuserrid'][0];
				if (isset($units[$i]['sambaalgorithmicridbase'][0])) $ret[$i]->RIDbase = $units[$i]['sambaalgorithmicridbase'][0];
			}
			// sort array by domain name
			usort($ret, array($this,"cmp_domain"));
		}
		return $ret;
	}

	/**
	* Helper function to sort the domains
	*
	* @param string $a first argument to compare
	* @param string $b second argument to compare
	* @return integer 0 if equal, 1 if $a is greater, -1 if $b is greater
	*/
	function cmp_domain($a, $b) {
		if ($a->name == $b->name) return 0;
		elseif ($a->name == max($a->name, $b->name)) return 1;
		else return -1;
	}


/**
* Represents a Samba 3 domain entry
*
* @package modules
*/
class samba3domain {

	/** DN */
	var $dn;

	/** Domain name */
	var $name;

	/** Domain SID */
	var $SID;

	/** Next RID */
	var $nextRID;

	/** Next user RID */
	var $nextUserRID;

	/** Next group RID */
	var $nextGroupRID;

	/** RID base to calculate RIDs, default 1000 */
	var $RIDbase = 1000;
}

/** This functions contains a collection of regular expressions
* It's much easier to handle them here than in every module
* because many of them are used several times.
**/
function get_preg($argument, $regexp) {
	// First we check "positive" cases
	$pregexpr = '';
	switch ($regexp) {
		case 'password':	// fixme where do i get an exact regexp?
					$pregexpr = '/^([[:alnum:]\\ \\|\\#\\*\\,\\.\\;\\:\\_\\+\\!\\%\\&\\/\\?\\{\\(\\)\\}-])*$/';
					break;
		case 'groupname':	// first character must be a letter. All letters, numbers, space and ._- are allowed characters
		case 'username':	// first character must be a letter. All letters, numbers, space and ._- are allowed characters
					$pregexpr = '/^[[:alpha:]]([[:alnum:]\\.\\ \\_-])*$/';
					break;
		case 'hostname':	// first character must be letter, last must be $. Only normal letters, numbers and ._- are allowed
					$pregexpr = '/^[a-zA-Z]([a-zA-Z0-9\\.\\_-])*\\$$/';
					break;
		case 'realname':	// Allow all letters, space and  .-_
					$pregexpr = '/^[[:alpha:]]([[:alpha:]\\.\\ \\_-])*$/';
					break;
		case "telephone":	// Allow numbers, space, brackets, /-+.
					$pregexpr = '/^(\\+)*([0-9\\.\\ \\(\\)\\/-])*$/';
					break;
		case "email":
					$pregexpr = '^(([0-9a-z\\._-])+[@]([0-9a-z-])+([.]([0-9a-z-])+)*)*$';
					break;
		case "street":		// Allow all letters, numbers, space and .-_
					$pregexpr = '/^([[:alnum:]\\.\\ \\_-])*$/';
					break;
		case "postalAddress":	// Allow all letters, numbers, space and .-_
		case "postalCode":		// Allow all letters, numbers, space and .-_
		case "title":			// Allow all letters, numbers, space and .-_
		case "employeeType":	// Allow all letters, numbers, space and .-_
					$pregexpr = '/^([[:alnum:]\\.\\ \\_-])*$/';
					break;
		case "homeDirectory":		// Homapath, /path/......
					$pregexpr = '/^[/]([[:alnum:]\\.\\ \\_-)+([/]([[:alnum:]\\.\\ \\_-)+)*$/';
					break;
		case "digit":		// Normal number
					$pregexpr = '/^[[:digit:]]*$/';
					break;
		case "UNC":			// UNC Path, e.g. \\server\share\folder\...
					$pregexpr = '/^([\][\]([a-zA-Z0-9\\.-])+([\]([[:alnum:]\\.\\ \\_-])+)+)*$/';
					break;
		case "logonscript":	// path to login-script. normal unix file
					$pregexpr = '/^(([/])*([[:alnum:]\\.\\ \\_-])+([/]([[:alnum:]\\.\\ \\_-])+)*((\\.bat)|(\\.cmd)))*$/';
					break;
		case "workstations":	// comma separated list with windows-hosts
					$pregexpr = '/^(([a-zA-Z0-9\\.\\_-])+(,[a-zA-Z0-9\\.\\_-])*)*$/';
					break;
		case "domainname":		// Windows Domainname
					$pregexpr = '/^([a-z0-9\\.\\_-])+$/';
					break;
		case "unixhost":		// Unix hosts
					$pregexpr = '/^([a-z0-9\\.\\_-])*$/';
					break;
		case 'digit2':			// Same as digit but also -1
					$pregexpr = '/^(([-][1])|([:digit:]*))$/';
					break;
	}
	if ($pregexpr!='')
			if (preg_match($pregexpr, $argument)) return true;
	
	// Now we check "negative" cases, characters which are not allowed
	$pregexpr = '';
	switch ($regexp) {
		case "!lower":
			$pregexpr = '/[[:lower:]]/';
			break;
		case "!upper":
			$pregexpr = '/[[:upper:]]/';
			break;
		case "!digit":
			$pregexpr = '/[[:digit:]]/';
			break;
	}
	if ($pregexpr!='')
			if (!preg_match($pregexpr, $argument)) return true;
	return false;
}

?>
