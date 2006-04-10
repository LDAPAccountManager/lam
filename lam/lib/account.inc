<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003 - 2006  Tilo Lutz

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
* @package lib
*/

/** Needed to calculate Samba passwords */
include_once("createntlm.inc");


/**
* Returns a list of shells listed in config/shells.
*
* @return array list of shell names
*/
function getshells() {
	// Load shells from file
	if (file_exists($_SESSION['lampath'] . 'config/shells')) {
		$shells = file($_SESSION['lampath'] . 'config/shells');
		$i = 0;
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
	else return array();
}


/**
* This function will return all values from $array without values of $values.
*
* @param array $values list of values which should be removed
* @param array $array list of original values
* @return array list of remaining values
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


/**
* This function will return a password with max. 8 characters.
*
* @return string password
*/
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


/**
* THIS FUNCTION GENERATES A RANDOM NUMBER THAT WILL BE USED TO
* RANDOMLY SELECT CHARACTERS.
*
* @param string $Format "letter" or "number"
* @return integer random number
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
}

/**
* This function will return the days from 1.1.1970 until now.
*
* @return number of days
*/
function getdays() {
	$days = time() / 86400;
	settype($days, 'integer');
	return $days;
	}

/**
* Takes a list of Samba flags and creates the corresponding flag string.
*
* @param array $input is an array of Samba flags (e.g. X or D)
* @return string Samba flag string
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
	$hash = new smbHash();
	return $hash->lmhash($password);
}

/**
* Generates the NT hash of a password.
*
* @param string password original password
* @return string password hash
*/
function ntPassword($password) {
	$hash = new smbHash();
	return $hash->nthash($password);
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
			if (function_exists('sha1')) {
				$hash = "{SHA}" . base64_encode(hex2bin(sha1($password)));
			}
			// otherwise use MHash
			elseif (function_exists('mHash')) {
				$hash = "{SHA}" . base64_encode(mHash(MHASH_SHA1, $password));
			}
			// if SHA1 is not possible use crypt()
			else {
				$hash = "{CRYPT}" . crypt($password);
			}
			break;
		case 'SSHA':
			// PHP 4.3+ can use sha1() function
			if (function_exists('sha1')) {
				$salt0 = substr(pack("h*", md5($_SESSION['ldap']->rand)), 0, 8);
				$salt = substr(pack("H*", sha1($salt0 . $password)), 0, 4);
				$hash = "{SSHA}" . base64_encode(hex2bin(sha1($password . $salt)) . $salt);
			}
			// otherwise use MHash
			elseif (function_exists('mHash')) {
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
			if (function_exists('sha1')) {
				$salt0 = substr(pack("h*", md5($_SESSION['ldap']->rand)), 0, 8);
				$salt = substr(pack("H*", sha1($salt0 . $password)), 0, 4);
				$hash = "{SSHA}" . base64_encode(hex2bin(sha1($password . $salt)) . $salt);
			}
			// otherwise use MHash
			elseif (function_exists('mHash')) {
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
* @return array list of samba3domain objects
*/
function search_domains() {
	$suffix = $_SESSION['config']->get_Suffix('smbDomain');
	$ret = array();
	$attr = array("DN", "sambaDomainName", "sambaSID", "sambaNextRid", "sambaNextGroupRid",
		"sambaNextUserRid", "sambaAlgorithmicRidBase");
	$sr = @ldap_search($_SESSION['ldap']->server(), $suffix, "objectClass=sambaDomain", $attr);
	if ($sr) {
		$units = ldap_get_entries($_SESSION['ldap']->server, $sr);
		// delete count entry
		unset($units['count']);
		// extract attributes
		for ($i = 0; $i < sizeof($units); $i++) {
			$ret[$i] = new samba3domain();
			$ret[$i]->dn = $units[$i]['dn'];
			$ret[$i]->name = $units[$i]['sambadomainname'][0];
			$ret[$i]->SID = $units[$i]['sambasid'][0];
			if (isset($units[$i]['sambanextrid'][0])) $ret[$i]->nextRID = $units[$i]['sambanextrid'][0];
			if (isset($units[$i]['sambanextgrouprid'][0])) $ret[$i]->nextGroupRID = $units[$i]['sambanextgrouprid'][0];
			if (isset($units[$i]['sambanextuserrid'][0])) $ret[$i]->nextUserRID = $units[$i]['sambanextuserrid'][0];
			if (isset($units[$i]['sambaalgorithmicridbase'][0])) $ret[$i]->RIDbase = $units[$i]['sambaalgorithmicridbase'][0];
		}
		// sort array by domain name
		usort($ret, "cmp_domain");
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

/**
* Checks if a given value matches the selected regular expression.
*
* @param string $argument value to check
* @param string $regexp pattern name
* @return boolean true if matches, otherwise false
*/
function get_preg($argument, $regexp) {
	/* Bug in php preg_match doesn't work correct with utf8
	*/
	$language = explode(":", $_SESSION['language']);
	$language2 = explode ('.', $language[0]);
	setlocale(LC_ALL, $language2[0]);
	// First we check "positive" cases
	$pregexpr = '';
	switch ($regexp) {
		case 'password':
					$pregexpr = '/^([[:alnum:]\\^\\ \\|\\#\\*\\,\\.\\;\\:\\_\\+\\!\\%\\&\\/\\?\\{\\(\\)\\}\\[\\]=-])*$/u';
					break;
		case 'groupname':	// first character must be a letter. All letters, numbers, space and ._- are allowed characters
		case 'username':	// first character must be a letter. All letters, numbers, space and ._- are allowed characters
					$pregexpr = '/^[[:alpha:]]([[:alnum:]\\.\\ \\_-])*$/u';
					break;
		case 'usernameList':	// comma separated list of user names
		case 'groupnameList':	// comma separated list of group names
					$pregexpr = '/^[[:alpha:]]([[:alnum:]\\.\\ \\_-])*(,[[:alpha:]]([[:alnum:]\\.\\ \\_-])*)*$/u';
					break;
		case 'hostname':	// first character must be letter, last must be $. Only normal letters, numbers and ._- are allowed
					$pregexpr = '/^[a-zA-Z]([a-zA-Z0-9\\.\\_-])*\\$$/u';
					break;
		case 'realname':	// Allow all but \, <, >, =, $, ?
		case 'cn':
					$pregexpr = '/^[^\\\<>=\\$\\?]+(\\$)?$/';
					break;
		case "telephone":	// Allow numbers, space, brackets, /-+.
					$pregexpr = '/^(\\+)*([0-9\\.\\ \\(\\)\\/-])*$/';
					break;
		case "email":
					$pregexpr = '/^([0-9a-zA-Z\\._-])+[@]([0-9a-zA-Z-])+([.]([0-9a-zA-Z-])+)*$/';
					break;
		case "mailLocalAddress":
					$pregexpr = '/^([0-9a-zA-Z\\._-])+([@]([0-9a-zA-Z-])+([.]([0-9a-zA-Z-])+)*)?$/';
					break;
		case "postalAddress":	// Allow all but \, <, >, =, $, ?
		case "postalCode":
		case "street":
		case "title":
		case "employeeType":
					$pregexpr = '/^[^\\\<>=\\$\\?]*$/';
					break;
		case "homeDirectory":		// Homapath, /path/......
					$pregexpr = '/^([\/]([[:alnum:]\\$\\.\\ \\_-])+)+$/u';
					break;
		case "digit":		// Normal number
					$pregexpr = '/^[[:digit:]]*$/';
					break;
		case "UNC":			// UNC Path, e.g. \\server\share\folder\...
					$pregexpr = '/^((([\\\][\\\])|(%))([a-zA-Z0-9%\\.-])+)([\\\]([[:alnum:]%\\.\\$\\ \\_-])+)+$/u';
					break;
		case "logonscript":	// path to login-script. normal unix file
					$pregexpr = '/^(([\/])*([[:alnum:]%\\.\\ \\$\\_-])+([\/]([[:alnum:]%\\.\\ \\$\\_-])+)*((\\.bat)|(\\.cmd)|(\\.exe)))*$/u';
					break;
		case "workstations":	// comma separated list with windows-hosts
					$pregexpr = '/^(([a-zA-Z0-9\\.\\_-])+(,[a-zA-Z0-9\\.\\_-])*)*$/';
					break;
		case "domainname":		// Windows Domainname
					$pregexpr = '/^([A-Za-z0-9\\.\\_-])+$/';
					break;
		case "unixhost":		// Unix hosts
					$pregexpr = '/^([a-z0-9,\\.\\_-])*$/';
					break;
		case 'digit2':			// Same as digit but also -1
					$pregexpr = '/^(([-][1])|([[:digit:]]*))$/';
					break;
		case 'gecos':
					$pregexpr = '/^[a-zA-z0-9 \\._-]+([,][a-zA-z0-9 \\._-]+)*$/';
					break;
		case 'macAddress':
					$pregexpr = '/^[0-9a-fA-F]{2}(:[0-9a-fA-F]{2}){5}$/';
					break;
		case 'date':
					$pregexpr = '/^((0?[1-9])|([1-2][0-9])|30|31)\\-((0?[1-9])|(1[0-2]))\\-[1-3][0-9][0-9][0-9]$/';
					break;
		case 'sambaLogonHours':
					$pregexpr = '/^[0-9a-fA-F]{42}$/';
					break;
		case 'DNSname':
					$pregexpr = '/^[0-9a-zA-Z_-]+(\\.[0-9a-zA-Z_-]+)*$/';
					break;
		case 'nis_alias':
					$pregexpr = '/^([[:alnum:]])+$/u';
					break;
		case 'nis_recipient':
					$pregexpr = '/^([[:alnum:]@\\.\\ \\_-])+$/u';
					break;
		case 'country':	// Allow all letters and space
					$pregexpr = '/^[[:alpha:]]([[:alpha:] ])+$/u';
					break;
		case 'dn':	// LDAP DN
					$pregexpr = '/^([^=,]+=[^=,]+)(,([^=,]+=[^=,]+))*$/';
					break;
		case 'domainSID': // Samba domain SID
					$pregexpr = "/^S\\-[0-9]\\-[0-9]\\-[0-9]{2,2}\\-[0-9]+\\-[0-9]+\\-[0-9]+$/";
	}
	if ($pregexpr!='')
		if (preg_match($pregexpr, $argument)) {
			/* Bug in php preg_match doesn't work correct with utf8
			*/
			setlocale(LC_ALL, $language[0]);
			return true;
		}
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
		if (!preg_match($pregexpr, $argument)) {
			/* Bug in php preg_match doesn't work correct with utf8
			*/
			setlocale(LC_ALL, $language[0]);
			return true;
		}
	/* Bug in php preg_match doesn't work correct with utf8
	*/
	setlocale(LC_ALL, $language[0]);
	return false;
}


?>
