<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Tilo Lutz
                2009 - 2020  Roland Gruber

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

use LAM\PLUGINS\EXTRA_INVALID_CREDENTIALS\ExtraInvalidCredentials;


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
		foreach ($array as $array_value) {
			if (!@in_array($array_value, $values)) {
				$return[] = $array_value;
			}
		}
		return $return;
	}
	return array();
}


/**
 * Checks if a string exists in an array, ignoring case.
 *
 * @param String $needle search string
 * @param array $haystack array
 */
function in_array_ignore_case($needle, $haystack) {
    if (!is_array($haystack)) {
        return false;
    }
    if (!is_string($needle)) {
        return false;
    }
    foreach ($haystack as $element) {
        if( is_string( $element ) && 0 == strcasecmp( $needle, $element ) ) {
            return true;
        }
    }
    return false;
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
	if ($input['W']) {
		$flag .= "W";
	}
	else {
		$flag .= "U";
	}
	if ($input['D']) {
		$flag .= "D";
	}
	if ($input['X']) {
		$flag .= "X";
	}
	if ($input['N']) {
		$flag .= "N";
	}
	if ($input['S']) {
		$flag .= "S";
	}
	if ($input['H']) {
		$flag .= "H";
	}
	// Expand string to fixed length
	$flag = str_pad($flag, 12);
	// End character
	return $flag . "]";
}

/**
* Generates the LM hash of a password.
*
* @param string password original password
* @return string password hash
*/
function lmPassword($password) {
	// Needed to calculate Samba passwords
	include_once(__DIR__ . "/createntlm.inc");
	// get hash
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
	// Needed to calculate Samba passwords
	include_once(__DIR__ . "/createntlm.inc");
	// get hash
	$hash = new smbHash();
	return $hash->nthash($password);
}


/**
* Returns the hash value of a plain text password.
* @see getSupportedHashTypes()
*
* @param string $password the password string
* @param boolean $enabled marks the hash as enabled/disabled (e.g. by prefixing "!")
* @param string $hashType password hash type (CRYPT, CRYPT-SHA512, SHA, SSHA, MD5, SMD5, PLAIN, K5KEY)
* @return string the password hash
*/
function pwd_hash($password, $enabled = true, $hashType = 'SSHA') {
	// check for empty password
	if (! $password || ($password == "")) {
		return "";
	}
	$hash = "";
	switch ($hashType) {
		case 'CRYPT':
			$hash = "{CRYPT}" . crypt($password, generateSalt(2));
			break;
		case 'CRYPT-SHA512':
			$hash = "{CRYPT}" . crypt($password, '$6$' . generateSalt(16));
			break;
		case 'PBKDF2-SHA512':
			$iterations = 200000;
			if (function_exists('openssl_pbkdf2')) {
				$salt = openssl_random_pseudo_bytes(16);
				$hashBinary = openssl_pbkdf2($password, $salt, 64, $iterations, 'sha512');
				$hash = "{PBKDF2-SHA512}${iterations}" . '$' . base64_encode($salt) . '$' . base64_encode($hashBinary);
			}
			else {
				$salt = generateSalt(16);
				$hashBinary = hex2bin(hash_pbkdf2('sha512', $password, $salt, $iterations));
				$hash = "{PBKDF2-SHA512}${iterations}" . '$' . base64_encode($salt) . '$' . base64_encode($hashBinary);
			}
			break;
		case 'MD5':
			$hash = "{MD5}" . base64_encode(hex2bin(md5($password)));
			break;
		case 'SMD5':
			$salt = generateSalt(4);
			$hash = "{SMD5}" . base64_encode(hex2bin(md5($password . $salt)) . $salt);
			break;
		case 'SHA':
			$hash = "{SHA}" . base64_encode(hex2bin(sha1($password)));
			break;
		case 'PLAIN':
			$hash = $password;
			break;
		case 'K5KEY':
			$hash = '{K5KEY}';
			break;
		case 'SSHA':
		default: // use SSHA if the setting is invalid
			$salt = generateSalt(4);
			$hash = "{SSHA}" . base64_encode(hex2bin(sha1($password . $salt)) . $salt);
			break;
	}
	// enable/disable password
	if (!$enabled) {
		return pwd_disable($hash);
	}
	return $hash;
}

/**
 * Returns the list of supported hash types (e.g. SSHA).
 *
 * @return array hash types
 */
function getSupportedHashTypes() {
	$hashes = array('CRYPT', 'CRYPT-SHA512', 'SHA', 'SSHA', 'MD5', 'SMD5', 'PLAIN', 'SASL', 'K5KEY');
	if (version_compare(phpversion(), '7.2.0') >= 0) {
		$hashes[] = 'LDAP_EXOP';
	}
	if (function_exists('openssl_pbkdf2') || function_exists('hash_pbkdf2')) {
		$hashes[] = 'PBKDF2-SHA512';
	}
	return $hashes;
}

/**
* Calculates a password salt of the given length.
*
* @param int $len salt length
* @return String the salt string
*
*/
function generateSalt($len) {
	$chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890./';
	$salt = '';
	for ($i = 0; $i < $len; $i++) {
		$pos = abs(getRandomNumber() % strlen($chars));
		$salt .= $chars[$pos];
	}
	return $salt;
}

/**
* Marks an password hash as enabled and returns the new hash string
*
* @param string $hash hash value to enable
* @return string enabled password hash
*/
function pwd_enable($hash) {
	// check if password is disabled (old wrong LAM method)
	if ((substr($hash, 0, 2) == "!{") || (substr($hash, 0, 2) == "*{")) {
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
			else {
				return $hash;  // not disabled
			}
		}
		else {
			return $hash;  // password is plain text
		}
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
	if ((substr($hash, 0, 2) == "!{") || (substr($hash, 0, 2) == "*{")) {
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
			else {
				return substr($hash, 0, $pos + 1) . "!" . substr($hash, $pos + 1, strlen($hash));  // not disabled
			}
		}
		else {
			return $hash;  // password is plain text
		}
	}
}

/**
 * Checks if a Unix password can be locked.
 * This checks if the password is not plain text but e.g. contains {SSHA}.
 *
 * @param String $password password value
 * @return boolean can be locked
 */
function pwd_is_lockable($password) {
	if (($password == null) || (strlen($password) < 5)) {
		return false;
	}
	// SASL is not lockable
	if (strpos($password, '{SASL}') === 0) {
		return false;
	}
	return ((substr($password, 0, 1) == "{") || (substr($password, 1, 1) == "{")) && (strpos($password, "}") > 3);
}

/**
* Checks if a password hash is enabled/disabled
*
* @param string $hash password hash to check
* @return boolean true if the password is marked as enabled
*/
function pwd_is_enabled($hash) {
	// disabled passwords have a "!" or "*" at the beginning (old wrong LAM method)
	if ((substr($hash, 0, 2) == "!{") || (substr($hash, 0, 2) == "*{")) {
		return false;
	}
	if (substr($hash, 0, 1) == "{") {
		$pos = strrpos($hash, "}");
		// check if hash starts with "!" or "*"
		return ((substr($hash, $pos + 1, 1) !== "!") && (substr($hash, $pos + 1, 1) !== "*"));
	}
	else {
		return true;
	}
}

/**
 * Generates a random password with 12 digits by default.
 *
 * @param int $length length of password (defaults to 12)
 * @return String password
 */
function generateRandomPassword($length = 12) {
	$list = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.-_';
	$password = '';
	$minLength = $_SESSION['cfgMain']->passwordMinLength;
	if ($minLength > $length) {
		$length = $minLength;
	}
	for ($x = 0; $x < 10000; $x++) {
		$password = '';
		for ($i = 0; $i < $length; $i++) {
			$rand = abs(getRandomNumber() % 65);
			$password .= $list[$rand];
		}
		if (checkPasswordStrength($password, null, null) === true) {
			break;
		}
	}
	return $password;
}

/**
 * Checks if the given password matches the crypto hash.
 *
 * @param String type hash type (must be one of getSupportedHashTypes())
 * @param string $hash password hash value
 * @param string $password plain text password to check
 * @return bool hash matches
 * @see getSupportedHashTypes()
 */
function checkPasswordHash($type, $hash, $password) {
	switch ($type) {
		case 'SSHA':
			$bin = base64_decode($hash);
			$salt = substr($bin, 20);
			$pwdHash = base64_encode(hex2bin(sha1($password . $salt)) . $salt);
			return (strcmp($hash, $pwdHash) == 0);
			break;
		case 'SHA':
			return (strcmp($hash, base64_encode(hex2bin(sha1($password)))) == 0);
			break;
		case 'SMD5':
			$bin = base64_decode($hash);
			$salt = substr($bin, 16);
			$pwdHash = base64_encode(hex2bin(md5($password . $salt)) . $salt);
			return (strcmp($hash, $pwdHash) == 0);
			break;
		case 'MD5':
			return (strcmp($hash, base64_encode(hex2bin(md5($password)))) == 0);
			break;
		case 'CRYPT':
			$parts = explode('$', $hash);
			if (sizeof($parts) == 4) {
				$version = $parts[1];
				$salt = $parts[2];
				$pwdHash = crypt($password, '$' . $version . '$' . $salt);
				return (strcmp($hash, $pwdHash) == 0);
			}
			elseif (sizeof($parts) == 5) {
				$version = $parts[1];
				$rounds = $parts[2];
				$salt = $parts[3];
				$pwdHash = crypt($password, '$' . $version . '$' . $rounds . '$' . $salt);
				return (strcmp($hash, $pwdHash) == 0);
			}
			return false;
			break;
		default:
			return false;
	}
	return false;
}

/**
* Returns an array with all Samba 3 domain entries under the given suffix
*
* @param handle LDAP handle (if null then $_SESSION['ldap']->server() is used)
* @param String $suffix LDAP suffix to search (if null then $_SESSION['config']->get_Suffix('smbDomain') is used)
* @return array list of samba3domain objects
*/
function search_domains($server = null, $suffix = null) {
	if ($suffix == null) {
		$suffix = $_SESSION['config']->get_Suffix('smbDomain');
	}
	$ret = array();
	$attr = array("DN", "sambaDomainName", "sambaSID", "sambaNextRid", "sambaNextGroupRid",
		"sambaNextUserRid", "sambaAlgorithmicRidBase", 'sambaMinPwdAge', 'sambaMaxPwdAge',
		'sambaPwdHistoryLength'
	);
	if ($server == null) {
		$server = $_SESSION['ldap']->server();
	}
	$filter = '(objectclass=sambaDomain)';
	$units = searchLDAPPaged($server, $suffix, $filter, $attr, false, 0);
	// extract attributes
	for ($i = 0; $i < sizeof($units); $i++) {
		$ret[$i] = new samba3domain();
		$ret[$i]->dn = $units[$i]['dn'];
		$ret[$i]->name = $units[$i]['sambadomainname'][0];
		$ret[$i]->SID = $units[$i]['sambasid'][0];
		if (isset($units[$i]['sambanextrid'][0])) {
			$ret[$i]->nextRID = $units[$i]['sambanextrid'][0];
		}
		if (isset($units[$i]['sambanextgrouprid'][0])) {
			$ret[$i]->nextGroupRID = $units[$i]['sambanextgrouprid'][0];
		}
		if (isset($units[$i]['sambanextuserrid'][0])) {
			$ret[$i]->nextUserRID = $units[$i]['sambanextuserrid'][0];
		}
		if (isset($units[$i]['sambaalgorithmicridbase'][0])) {
			$ret[$i]->RIDbase = $units[$i]['sambaalgorithmicridbase'][0];
		}
		if (isset($units[$i]['sambaminpwdage'][0])) {
			$ret[$i]->minPwdAge = $units[$i]['sambaminpwdage'][0];
		}
		if (isset($units[$i]['sambamaxpwdage'][0])) {
			$ret[$i]->maxPwdAge = $units[$i]['sambamaxpwdage'][0];
		}
		if (isset($units[$i]['sambapwdhistorylength'][0])) {
			$ret[$i]->pwdHistoryLength = $units[$i]['sambapwdhistorylength'][0];
		}
	}
	return $ret;
}

/**
* Represents a Samba 3 domain entry
*
* @package modules
*/
class samba3domain {

	/** DN */
	public $dn;

	/** Domain name */
	public $name;

	/** Domain SID */
	public $SID;

	/** Next RID */
	public $nextRID;

	/** Next user RID */
	public $nextUserRID;

	/** Next group RID */
	public $nextGroupRID;

	/** RID base to calculate RIDs, default 1000 */
	public $RIDbase = 1000;

	/** seconds after the password can be changed */
	public $minPwdAge;

	/** seconds after the password must be changed */
	public $maxPwdAge;

	/** password history length */
	public $pwdHistoryLength;
}

/**
* Checks if a given value matches the selected regular expression.
*
* @param string $argument value to check
* @param string $regexp pattern name
* @return boolean true if matches, otherwise false
*/
function get_preg($argument, $regexp) {
	/* Bug in php preg_match doesn't work correct with utf8 */
	$language = $_SESSION['language'];
	$language2 = explode ('.', $language);
	setlocale(LC_ALL, $language2[0]);
	// First we check "positive" cases
	$pregexpr = '';
	switch ($regexp) {
		case 'password':
					$pregexpr = '/^([[:alnum:]\\^\\ \\|\\#\\*\\,\\.\\;\\:\\_\\+\\!\\%\\&\\/\\?\\{\\(\\)\\}\\[\\]\\$§°@=-])*$/u';
					break;
		case 'groupname':	// all letters, numbers, space and ._- are allowed characters
		case 'username':
		case 'hostname':
					$pregexpr = '/^([[:alnum:]%#@\\.\\ \\_\\$-])+$/u';
					break;
		case 'krbUserName':
					$pregexpr = '/^([[:alnum:]#@\\/\\.\\ \\_\\$-])+$/u';
					break;
		case 'hostObject':
					$pregexpr = '/^[!]?([[:alnum:]@\\.\\ \\_\\$\\*-])+$/u';
					break;
		case 'usernameList':	// comma separated list of user names
		case 'groupnameList':	// comma separated list of group names
					$pregexpr = '/^([[:alnum:]%#@\\.\\ \\_-])+(,([[:alnum:]%#@\\.\\ \\_-])+)*$/u';
					break;
		case 'realname':	// Allow all but \, <, >, =, $, ?
		case 'cn':
					$pregexpr = '/^[^\\\<>=\\$\\?]+(\\$)?$/';
					break;
		case "telephone":	// Allow letters, numbers, space, brackets, /-+.
					$pregexpr = '/^(\\+)*([0-9a-zA-Z\\.\\ \\(\\)\\/-])*$/';
					break;
		case "email":
					$pregexpr = '/^([0-9a-zA-Z\'!~#+*%\\$\\/\\._-])+[@]([0-9a-zA-Z-])+([.]([0-9a-zA-Z-])+)*$/';
					break;
		case "emailWithName":
					$pregexpr = '/^([[:alnum:] \'!~#+*%\\$\\(\\)_-])+ <([0-9a-zA-Z\'!~#+*%\\$\\/\\._-])+[@]([0-9a-zA-Z-])+([.]([0-9a-zA-Z-])+)*>$/u';
					break;
		case "mailLocalAddress":
					$pregexpr = '/^([0-9a-zA-Z+\\/\\._-])+([@]([0-9a-zA-Z-])+([.]([0-9a-zA-Z-])+)*)?$/';
					break;
		case 'kolabEmailPrefix':
					$pregexpr = '/^([-])?([0-9a-zA-Z+\\/\\._-])*([@]([0-9a-zA-Z\\.-])*)?$/';
					break;
		case "postalAddress":	// Allow all but \, <, >, =, ?
					$pregexpr = '/^[^\\\<>=\\?]*$/';
					break;
		case "postalCode":	// Allow all but \, <, >, =, ?
		case "street":
		case "title":
		case "employeeType":
		case "businessCategory":
					$pregexpr = '/^[^\\\<>=\\$\\?]*$/';
					break;
		case "homeDirectory":		// Homapath, /path/......
		case "filePath":
					$pregexpr = '/^([\/]([[:alnum:]@\\$\\.\\ \\_-])+)+(\/)?$/u';
					break;
		case "digit":		// Normal number
					$pregexpr = '/^[[:digit:]]*$/';
					break;
		case "float":		// float value
					$pregexpr = '/^[[:digit:]]+(\\.[[:digit:]]+)?$/';
					break;
		case "UNC":			// UNC Path, e.g. \\server\share\folder\...
					$pregexpr = '/^((([\\\][\\\])|(%))([a-zA-Z0-9@%\\.-])+)([\\\]([[:alnum:]@%\\.\\$\\ \\_-])+)+$/u';
					break;
		case "logonscript":	// path to login-script. normal unix file
					$pregexpr = '/^(([\/\\\])*([[:alnum:]%\\.\\ \\$\\_-])+([\/\\\]([[:alnum:]%\\.\\ \\$\\_-])+)*((\\.bat)|(\\.cmd)|(\\.exe)|(\\.vbs)))*$/u';
					break;
		case "workstations":	// comma separated list with windows-hosts
					$pregexpr = '/^(([a-zA-Z0-9\\.\\_-])+(,[a-zA-Z0-9\\.\\_-])*)*$/';
					break;
		case "domainname":		// Windows Domainname
					$pregexpr = '/^([A-Za-z0-9\\.\\_-])+$/';
					break;
		case "unixhost":		// Unix hosts
					$pregexpr = '/^([a-z0-9,\\.\\*_-])*$/';
					break;
		case 'digit2':			// Same as digit but also -1
					$pregexpr = '/^(([-][1])|([[:digit:]]*))$/';
					break;
		case 'gecos':
					$pregexpr = '/^[[:alnum:] \\._-]+([,][[:alnum:] \\._-]+)*$/u';
					break;
		case 'macAddress':
					$pregexpr = '/^[0-9a-fA-F]{2}(:[0-9a-fA-F]{2}){5}$/';
					break;
		case 'date':			// 31-12-2012
					$pregexpr = '/^((0?[1-9])|([1-2][0-9])|30|31)\\-((0?[1-9])|(1[0-2]))\\-[1-3][0-9][0-9][0-9]$/';
					break;
		case 'date2':
					$pregexpr = '/^((0[1-9])|([1-2][0-9])|30|31)\\.((0[1-9])|(1[0-2]))\\.[1-3][0-9][0-9][0-9]$/';
					break;
		case 'dateTime':
					$pregexpr = '/^[1-3][0-9][0-9][0-9]\\-((0[1-9])|(1[0-2]))\\-((0[1-9])|([1-2][0-9])|30|31) ((0[0-9])|([1][0-9])|20|21|22|23):((0[0-9])|([1-5][0-9])):((0[0-9])|([1-5][0-9]))$/';
					break;
		case 'sambaLogonHours':
					$pregexpr = '/^[0-9a-fA-F]{42}$/';
					break;
		case 'DNSname':
					$pregexpr = '/^[0-9a-zA-Z_-]+(\\.[0-9a-zA-Z_-]+)*$/';
					break;
		case 'nis_alias':
					$pregexpr = '/^([[:alnum:]@\\.\\ \\_-])+$/u';
					break;
		case 'nis_recipient':
					$pregexpr = '/^([[:alnum:]+@\\.\\ \\_-])+$/u';
					break;
		case 'country':	// Allow all letters and space
					$pregexpr = '/^[[:alpha:]]([[:alpha:] ])+$/u';
					break;
		case 'dn':	// LDAP DN
					$pregexpr = '/^([^=,]+=[^=,]+)(,([^=,]+=[^=,]+))*$/';
					break;
		case 'domainSID': // Samba domain SID
					$pregexpr = "/^S\\-[0-9]\\-[0-9]\\-[0-9]{2,2}\\-[0-9]+\\-[0-9]+\\-[0-9]+$/";
					break;
		case 'ip': // IP address
					$pregexpr = '/^[0-9]{1,3}[.][0-9]{1,3}[.][0-9]{1,3}[.][0-9]{1,3}$/';
					break;
		case 'ip6': // IPv6 address (only basic check)
					$pregexpr = '/^[0-9a-f:]+$/i';
					break;
		case 'ascii': // ASCII
					$pregexpr = '/^[' . chr(1) . '-' . chr(128) . ']*$/';
					break;
		case 'objectClass':
					$pregexpr = '/^[[:alnum:]_-]+$/';
					break;
		case 'quotaNumber':
					$pregexpr = '/^[[:digit:]]+[KMGTkmgt]?$/';
					break;
		case 'hostAndPort':
			$pregexpr = '/^[[:alnum:]\\._-]+:[[:digit:]]+$/';
			break;
	}
	if (($pregexpr != '') && preg_match($pregexpr, $argument)) {
		/* Bug in php preg_match doesn't work correct with utf8 */
		setlocale(LC_ALL, $language);
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
	if (($pregexpr != '') && !preg_match($pregexpr, $argument)) {
		/* Bug in php preg_match doesn't work correct with utf8 */
		setlocale(LC_ALL, $language);
		return true;
	}
	/* Bug in php preg_match doesn't work correct with utf8 */
	setlocale(LC_ALL, $language);
	return false;
}

/**
 * Escapes any special characters in an LDAP DN.
 *
 * @param String $dn DN
 * @return String escaped DN
 */
function escapeDN($dn) {
	return str_replace(
		array(')',    '(',    ' ',    '*'),
		array('\\29', '\\28', '\\20', '\\2a'),
		$dn
	);
}

/**
 * Escapes special characters in RDN part.
 *
 * @param string $rdn RDN
 */
function escapeRDN($rdn) {
	return str_replace(
		array(','),
		array('\\2C'),
		$rdn);
}

/**
 * Converts the comma escaping from Windows to OpenLDAP style.
 *
 * @param string $dn DN
 * @return string DN
 */
function convertCommaEscaping($dn) {
	return str_replace(
		array('\\,'),
		array('\\2C'),
		$dn);
}

/**
 * Connects to an LDAP server using the given URL.
 *
 * @param string $serverURL URL
 */
function connectToLDAP($serverURL, $startTLS) {
	$server = ldap_connect($serverURL);
	if (!$server) {
		return null;
	}
	if (defined('LDAP_OPT_X_TLS_CACERTFILE')) {
		$cfgMain = new LAMCfgMain();
		$certificates = $cfgMain->getSSLCaCertificates();
		if (!empty($certificates)) {
			ldap_set_option($server, LDAP_OPT_X_TLS_CACERTFILE, $cfgMain->getSSLCaCertPath());
		}
	}
	// use LDAPv3
	ldap_set_option($server, LDAP_OPT_PROTOCOL_VERSION, 3);
	// start TLS if possible
	if ($startTLS) {
		ldap_start_tls($server);
		if (ldap_errno($server) != 0) {
			ldap_close($server);
			logNewMessage(LOG_ERR, 'Unable to start TLS encryption. Please check if your server certificate is valid and if the LDAP server supports TLS at all.');
			return null;
		}
	}
	return $server;
}

/**
 * This will search the given LDAP suffix for all entries which have the given attribute.
 *
 * @param String $name attribute name (may be null)
 * @param String $value attribute value
 * @param String $objectClass object class (may be null)
 * @param array $attributes list of attributes to return
 * @param array $scopes account types
 * @return array list of found entries
 */
function searchLDAPByAttribute($name, $value, $objectClass, $attributes, $scopes) {
	$return = array();
	// build filter
	$filter = '';
	$filterParts = array();
	if ($name != null) {
		$filterParts[] = '(' . $name . '=' . ldap_escape($value, '*', LDAP_ESCAPE_FILTER) . ')';
	}
	if ($objectClass != null) {
		$filterParts[] = '(objectClass=' . $objectClass . ')';
	}
	if (sizeof($filterParts) == 1) {
		$filter = $filterParts[0];
	}
	elseif (sizeof($filterParts) > 1) {
		$filter = '(& ' . implode(' ', $filterParts) . ')';
	}
	$typeManager = new \LAM\TYPES\TypeManager();
	$activeTypes = $typeManager->getConfiguredTypes();
	foreach ($activeTypes as $type) {
		if (!in_array($type->getScope(), $scopes)) {
			continue;  // skip non-active account types
		}
		// search LDAP
		$entries = searchLDAPPaged($_SESSION['ldap']->server(), escapeDN($type->getSuffix()),
					$filter, $attributes, 0, $_SESSION['config']->get_searchLimit());
		if (ldap_errno($_SESSION['ldap']->server()) == 4) {
			logNewMessage(LOG_WARNING, 'LDAP size limit exeeded. Please increase the limit on your server.');
		}
		$return = array_merge($return, $entries);
	}
	return $return;
}

/**
 * This will search the given LDAP suffix for all entries which match the given filter.
 *
 * @param String $filter
 * @param array $attributes list of attributes to return
 * @param array $scopes account types
 * @param boolean $attrsOnly get only attributes but no values (default: false)
 * @return array list of found entries
 */
function searchLDAPByFilter($filter, $attributes, $scopes, $attrsOnly = false) {
	$return = array();
	$readAttributesOnly = 0;
	if ($attrsOnly) {
		$readAttributesOnly = 1;
	}
	$typeManager = new \LAM\TYPES\TypeManager();
	$types = $typeManager->getConfiguredTypesForScopes($scopes);
	foreach ($types as $type) {
		$additionalFilter = $type->getAdditionalLdapFilter();
		if (!empty($additionalFilter)) {
			if (strpos($additionalFilter, '(') !== 0) {
				$additionalFilter = '(' . $additionalFilter . ')';
			}
			if (strpos($filter, '(') !== 0) {
				$filter = '(' . $filter . ')';
			}
			$filter = '(&' . $additionalFilter . $filter . ')';
		}
		// search LDAP
		$entries = searchLDAPPaged($_SESSION['ldap']->server(), escapeDN($type->getSuffix()),
					$filter, $attributes, $readAttributesOnly, $_SESSION['config']->get_searchLimit());
		if (ldap_errno($_SESSION['ldap']->server()) == 4) {
			logNewMessage(LOG_WARNING, 'LDAP size limit exeeded. Please increase the limit on your server.');
		}
		$return = array_merge($return, $entries);
	}
	return $return;
}

/**
 * Runs an LDAP search.
 *
 * @param String $suffix LDAP suffix
 * @param String $filter filter
 * @param array $attributes list of attributes to return
 * @return array list of found entries
 */
function searchLDAP($suffix, $filter, $attributes) {
	$limit = 0;
	if (!empty($_SESSION['config'])) {
		$limit = $_SESSION['config']->get_searchLimit();
	}
	$return = searchLDAPPaged(getLDAPServerHandle(), escapeDN($suffix), $filter, $attributes,
				0, $limit);
	if (ldap_errno(getLDAPServerHandle()) == 4) {
		logNewMessage(LOG_WARNING, 'LDAP size limit exeeded. Please increase the limit on your server.');
	}
	return $return;
}

/**
 * Returns the LDAP server handle.
 *
 * @return handle LDAP handle
 */
function getLDAPServerHandle() {
	if (!empty($_SESSION['ldap'])) {
		// admin pages
		return $_SESSION['ldap']->server();
	}
	else {
		// self service
		return $_SESSION['ldapHandle'];
	}
}

/**
 * Runs an LDAP search and uses paging if configured.
 *
 * @param handle $server LDAP connection handle
 * @param String $dn DN
 * @param String $filter filter
 * @param array $attributes attribute list
 * @param boolean $attrsOnly return only attribute names
 * @param int $limit size limit
 */
function searchLDAPPaged($server, $dn, $filter, $attributes, $attrsOnly, $limit) {
	if (empty($_SESSION['config']) || ($_SESSION['config']->getPagedResults() !== 'true')) {
		$sr = @ldap_search($server, $dn, $filter, $attributes, $attrsOnly, $limit, 0, LDAP_DEREF_NEVER);
		if ($sr) {
			$entries = ldap_get_entries($server, $sr);
			if (!$entries) {
				return array();
			}
			cleanLDAPResult($entries);
			return $entries;
		}
		else {
			return array();
		}
	}
	$pageSize = 999;
	$cookie = '';
	$return = array();
	do {
		@ldap_control_paged_result($server, $pageSize, true, $cookie);
		$sr  = @ldap_search($server, $dn, $filter, $attributes, $attrsOnly, $limit, 0, LDAP_DEREF_NEVER);
		if (!$sr) {
			break;
		}
		$entries = ldap_get_entries($server, $sr);
		if (!$entries) {
			break;
		}
		$return = array_merge($return, $entries);
		@ldap_control_paged_result_response($server, $sr, $cookie);
	} while($cookie !== null && $cookie != '');
	cleanLDAPResult($return);
	return $return;
}

/**
 * Returns the given DN.
 *
 * @param String $dn DN
 * @param array $attributes list of attributes to fetch
 * @param handle $handle LDAP handle (optional for admin interface pages)
 * @return array attributes or null if not found
 */
function ldapGetDN($dn, $attributes = array('dn'), $handle = null) {
	if ($handle == null) {
		$handle = getLDAPServerHandle();
	}
	$return = null;
	$sr = @ldap_read($handle, escapeDN($dn), 'objectClass=*', $attributes, 0, 0, 0, LDAP_DEREF_NEVER);
	if ($sr) {
		$entries = ldap_get_entries($handle, $sr);
		if ($entries) {
			cleanLDAPResult($entries);
			$return = $entries[0];
		}
		@ldap_free_result($sr);
	}
	return $return;
}

/**
 * Returns the DN and children of a given DN.
 *
 * @param String $dn DN
 * @param String $filter LDAP filter
 * @param array $attributes list of attributes to fetch
 * @param handle $handle LDAP handle (optional for admin interface pages)
 * @return array attributes or null if not found
 */
function ldapListDN($dn, $filter = '(objectclass=*)', $attributes = array('dn'), $handle = null) {
	if ($handle == null) {
		$handle = $_SESSION['ldap']->server();
	}
	$return = null;
	$sr = @ldap_list($handle, escapeDN($dn), $filter, $attributes, 0, 0, 0, LDAP_DEREF_NEVER);
	if ($sr) {
		$entries = ldap_get_entries($handle, $sr);
		if ($entries) {
			cleanLDAPResult($entries);
			$return = $entries;
		}
		@ldap_free_result($sr);
	}
	return $return;
}

/**
* Deletes a DN and all child entries.
*
* @param string $dn DN to delete
* @param boolean $recursive recursive delete also child entries
* @return array error messages
*/
function deleteDN($dn, $recursive) {
	$errors = array();
	if (($dn == null) || ($dn == '')) {
		$errors[] = array('ERROR', _('Entry does not exist'));
		return $errors;
	}
	if ($recursive) {
		$sr = @ldap_list($_SESSION['ldap']->server(), $dn, 'objectClass=*', array('dn'), 0, 0, 0, LDAP_DEREF_NEVER);
		if ($sr) {
			$entries = ldap_get_entries($_SESSION['ldap']->server(), $sr);
			cleanLDAPResult($entries);
			for ($i = 0; $i < sizeof($entries); $i++) {
				// delete recursively
				$subErrors = deleteDN($entries[$i]['dn'], $recursive);
				for ($e = 0; $e < sizeof($subErrors); $e++) {
					$errors[] = $subErrors[$e];
				}
			}
		}
		else {
			$errors[] = array ('ERROR', sprintf(_('Was unable to delete DN: %s.'), $dn), getDefaultLDAPErrorString($_SESSION['ldap']->server()));
			return $errors;
		}
	}
	// delete parent DN
	$success = @ldap_delete($_SESSION['ldap']->server(), $dn);
	if (!$success) {
		logNewMessage(LOG_ERR, 'Unable to delete DN: ' . $dn . ' (' . ldap_error($_SESSION['ldap']->server()) . ').');
		$errors[] = array ('ERROR', sprintf(_('Was unable to delete DN: %s.'), $dn), getDefaultLDAPErrorString($_SESSION['ldap']->server()));
	}
	else {
		logNewMessage(LOG_NOTICE, 'Deleted DN: ' . $dn);
	}
	return $errors;
}

/**
 * Returns the parameters for a StatusMessage of the last LDAP search.
 *
 * @return array parameters for StatusMessage or null if all was ok
 */
function getLastLDAPError() {
	$errorNumber = ldap_errno($_SESSION["ldap"]->server());
	switch ($errorNumber) {
		// all ok
		case 0:
			return null;
			break;
		// size limit exceeded
		case 4:
			$error = array("WARN", _("LDAP sizelimit exceeded, not all entries are shown."));
			if ($_SESSION['config']->get_searchLimit() == 0) {
				// server limit exceeded
				$error[] = _("See the manual for instructions to solve this problem.");
			}
			return $error;
			break;
		// other errors
		default:
			return array("ERROR", _("LDAP search failed! Please check your preferences."), ldap_error($_SESSION["ldap"]->server()));
		break;
	}
}

/**
 * Cleans the result of an LDAP search.
 * This will remove all 'count' entries and also all numeric array keys.
 *
 * @param array $entries LDAP entries in format $entries[entry number][attribute name][attribute values]
 */
function cleanLDAPResult(&$entries) {
	if (isset($entries['count'])) {
		unset($entries['count']);
	}
	// iterate over all results
	$count = sizeof($entries);
	for ($e = 0; $e < $count; $e++) {
		// remove 'count' entries and numerical entries
		for ($i = 0; $i < $entries[$e]['count']; $i++) {
			if (isset($entries[$e][$i])) {
				unset($entries[$e][$i]);
			}
		}
		unset($entries[$e]['count']);
		$attrNames = array_keys($entries[$e]);
		$attrCount = sizeof($attrNames);
		for ($i = 0; $i < $attrCount; $i++) {
			if (is_array($entries[$e][$attrNames[$i]])) {
				unset($entries[$e][$attrNames[$i]]['count']);
			}
		}
	}
}

/**
 * Transforms a DN into a more user friendly format.
 * E.g. "dc=company,dc=de" is transformed to "company > de".
 *
 * @param String $dn DN
 * @return String transformed DN
 */
function getAbstractDN($dn) {
	if ($dn == '') {
		return '';
	}
	$dn = str_replace('\\,', '\\2C', $dn);
	if (!empty($_SESSION['config']) && !empty($_SESSION['config']->getHideDnPart())) {
		$partToCut = ',' . $_SESSION['config']->getHideDnPart();
		$dn = str_replace($partToCut, '', $dn);
	}
	$parts = explode(',', $dn);
	for ($i = 0; $i < sizeof($parts); $i++) {
		$subparts = explode('=', $parts[$i]);
		if (sizeof($subparts) == 2) {
			$parts[$i] = $subparts[1];
		}
	}
	$abstractDn = implode(' > ', $parts);
	return str_replace(array('\\2C', '\\,'), array(',', ','), $abstractDn);
}

/**
* Helper function to sort DNs.
*
* @param string $a first argument to compare
* @param string $b second argument to compare
* @return integer 0 if equal, 1 if $a is greater, -1 if $b is greater
*/
function compareDN(&$a, &$b) {
	// split DNs
	$array_a = explode(",", strtolower($a));
	$array_b = explode(",", strtolower($b));
	$len_a = sizeof($array_a);
	$len_b = sizeof($array_b);
	// check how many parts to compare
	$len = min($len_a, $len_b);
	// compare from last part on
	for ($i = 0; $i < $len; $i++) {
		// get parts to compare
		$part_a = $array_a[$len_a - $i - 1];
		$part_a = explode('=', $part_a);
		$part_a = isset($part_a[1]) ? $part_a[1] : $part_a[0];
		$part_b = $array_b[$len_b - $i - 1];
		$part_b = explode('=', $part_b);
		$part_b = isset($part_b[1]) ? $part_b[1] : $part_b[0];
		// compare parts
		if ($part_a == $part_b) { // part is identical
			if ($i == ($len - 1)) {
				if ($len_a > $len_b) {
					return 1;
				}
				elseif ($len_a < $len_b) {
					return -1;
				}
				else {
					return 0; // DNs are identical
				}
			}
		}
		else {
			return strnatcasecmp($part_a, $part_b);
		}
	}
	return -1;
}

/**
 * Formats an LDAP time string (e.g. from createTimestamp).
 *
 * @param String $time LDAP time value
 * @return String formatted time
 */
function formatLDAPTimestamp($time) {
	$dateTime = parseLDAPTimestamp($time);
	return $dateTime->format('d.m.Y H:i:s');
}

/**
 * Parses an LDAP time stamp and returns a DateTime in current time zone.
 *
 * @param String $time LDAP time value
 * @return DateTime time
 */
function parseLDAPTimestamp($time) {
	// Windows format:  20140118093807.0Z
	// OpenLDAP format: 20140118093807Z
	// cut off "Z"
	$timeNumbers = substr($time, 0, -1);
	// for Windows cut off ".0"
	if (strpos($timeNumbers, '.') == (strlen($timeNumbers) - 2)) {
		$timeNumbers = substr($timeNumbers, 0, -2);
	}
	$dateTime = DateTime::createFromFormat('YmdHis', $timeNumbers, new DateTimeZone('UTC'));
	$dateTime->setTimezone(getTimeZone());
	return $dateTime;
}

/**
 * Simple function to obfuscate strings.
 *
 * @param String $text text to obfuscate
 */
function obfuscateText($text) {
	if (($text == null) || ($text == '')) {
		return $text;
	}
	return str_rot13(base64_encode('LAM_OBFUSCATE:'.$text));
}

/**
 * Simple function to deobfuscate strings.
 *
 * @param String $text text to deobfuscate
 */
function deobfuscateText($text) {
	if (($text == null) || ($text == '')) {
		return $text;
	}
	if (!isObfuscatedText($text)) {
		return $text;
	}
	return str_replace('LAM_OBFUSCATE:', '', base64_decode(str_rot13($text)));
}

/**
 * Checks if the given text is obfuscated.
 *
 * @param String $text text to check
 * @return boolean obfuscated or not
 */
function isObfuscatedText($text) {
	if (($text == null) || ($text == '')) {
		return false;
	}
	$deob = base64_decode(str_rot13($text));
	return (strpos($deob, 'LAM_OBFUSCATE:') === 0);
}

/**
 * Extracts the RDN attribute name from a given DN.
 *
 * @param String $dn DN
 * @return String RDN attribute name
 */
function extractRDNAttribute($dn) {
	if ($dn == null) {
		return null;
	}
	$parts = explode("=", substr($dn, 0, strpos($dn, ',')));
	return $parts[0];
}

/**
 * Extracts the RDN attribute value from a given DN.
 *
 * @param String $dn DN
 * @return String RDN attribute value
 */
function extractRDNValue($dn) {
	if (empty($dn)) {
		return null;
	}
	if (strpos($dn, '=') === false) {
		return $dn;
	}
	$dnWork = $dn;
	if (strpos($dnWork, ',') !== false) {
		$dnWork = substr($dn, 0, strpos($dnWork, ','));
	}
	$parts = explode("=", $dnWork);
	return $parts[1];
}

/**
 * Extracts the DN suffix from a given DN.
 * E.g. ou=people,dc=test,dc=com will result in dc=test,dc=com.
 *
 * @param String $dn DN
 * @return String DN suffix
 */
function extractDNSuffix($dn) {
	if ($dn == null) {
		return null;
	}
	$dn = convertCommaEscaping($dn);
	return substr($dn, strpos($dn, ',')+1);
}

/**
 * Sends the password mail.
 *
 * @param String $pwd new password
 * @param array $user LDAP attributes of user
 * @param String $recipient recipient address (optional, $user['mail'][0] used by default)
 * @return array list of arrays that can be used to create status messages
 */
function sendPasswordMail($pwd, $user, $recipient = null) {
	$user = array_change_key_case($user, CASE_LOWER);
	// read mail data
	$mailTo = $user['mail'][0];
	if (!empty($recipient)) {
		$mailTo = $recipient;
	}
	if (empty($mailTo)) {
		logNewMessage(LOG_ERR, 'Unable to send password mail, no TO address set.');
		return array(
			array('ERROR', _('Unable to send mail!'))
		);
	}
	$mailFrom = $_SESSION['config']->getLamProMailFrom();
	$mailReplyTo = $_SESSION['config']->getLamProMailReplyTo();
	$mailSubject = $_SESSION['config']->getLamProMailSubject();
	$mailText = $_SESSION['config']->getLamProMailText();
	$mailIsHTML = $_SESSION['config']->getLamProMailIsHTML();
	$subject = $mailSubject;
	$body = $mailText;
	$body = str_replace('@@newPassword@@', $pwd, $body);
	$results = array();
	$found = preg_match('/\@\@[^\@]+\@\@/', $body, $results);
	while ($found == 1) {
		$attr = str_replace('@', '', $results[0]);
		$value = '';
		if (isset($user[strtolower($attr)][0])) {
			if (is_array($user[strtolower($attr)])) {
				$value = $user[strtolower($attr)][0];
			}
			else {
				$value = $user[strtolower($attr)];
			}
		}
		$body = str_replace('@@' . $attr . '@@', $value, $body);
		$found = preg_match('/\@\@[^\@]+\@\@/', $body, $results);
	}
	$success = sendEMail($mailTo, $subject, $body, $mailFrom, ($mailIsHTML == 'true'), $mailReplyTo);
	if ($success) {
		logNewMessage(LOG_DEBUG, 'Sent password mail to ' . $mailTo);
		return array(
			array('INFO', sprintf(_('Mail successfully sent to %s.'), htmlspecialchars($mailTo)))
		);
	}
	else {
		logNewMessage(LOG_ERR, 'Unable to send password mail to ' . htmlspecialchars($mailTo));
		return array(
			array('ERROR', _('Unable to send mail!'))
		);
	}
}

/**
 * Sends out an email.
 *
 * @param String|array $to TO address
 * @param String $subject email subject
 * @param String $text mail body (with \r\n EOL)
 * @param String $from FROM address
 * @param bool $isHTML HTML format
 * @param String $replyTo REPLY-TO address (optional)
 * @param String $cc CC address (optional)
 * @param String $bcc BCC address (optional)
 */
function sendEMail($to, $subject, $text, $from, $isHTML, $replyTo = null, $cc = null, $bcc = null) {
	include_once __DIR__ . '/3rdParty/composer/autoload.php';
	$returnPath = ($replyTo === null) ? $from : $replyTo;
	$returnPathParsed = PHPMailer\PHPMailer\PHPMailer::parseAddresses($returnPath);
	logNewMessage(LOG_DEBUG, "Send mail to " . print_r($to, true) . "\n" . $text);
	$mailer = new PHPMailer\PHPMailer\PHPMailer(true);
	try {
		$cfgMain = $_SESSION['cfgMain'];
		if (!empty($cfgMain->mailServer)) {
			$mailer->isSMTP();
			$serverParts = explode(':', $cfgMain->mailServer);
			$mailer->Host = $serverParts[0];
			$mailer->Port = $serverParts[1];
			if (!empty($cfgMain->mailUser)) {
				$mailer->SMTPAuth = true;
				$mailer->Username = $cfgMain->mailUser;
				$mailer->Password = $cfgMain->mailPassword;
				$mailer->SMTPSecure = PHPMailer\PHPMailer\PHPMailer::ENCRYPTION_STARTTLS;
			}
		}
		$mailer->CharSet = PHPMailer\PHPMailer\PHPMailer::CHARSET_UTF8;
		if (is_array($to)) {
			foreach ($to as $toAddress) {
				$mailer->addAddress($toAddress);
			}
		}
		else {
			$mailer->addAddress($to);
		}
		$mailer->Subject = $subject;
		$mailer->Body = $text;
		$mailer->Sender = $returnPathParsed[0]['address'];
		$fromParsed = PHPMailer\PHPMailer\PHPMailer::parseAddresses($from);
		$mailer->setFrom($fromParsed[0]['address'], $fromParsed[0]['name']);
		$mailer->isHTML($isHTML);
		if (!empty($replyTo)) {
			$replyToParsed = PHPMailer\PHPMailer\PHPMailer::parseAddresses($replyTo);
			$mailer->addReplyTo($replyToParsed[0]['address'], $replyToParsed[0]['name']);
		}
		if (!empty($cc)) {
			$ccParsed = PHPMailer\PHPMailer\PHPMailer::parseAddresses($cc);
			$mailer->addCC($ccParsed[0]['address'], $ccParsed[0]['name']);
		}
		if (!empty($bcc)) {
			$bccParsed = PHPMailer\PHPMailer\PHPMailer::parseAddresses($bcc);
			$mailer->addBCC($bccParsed[0]['address'], $bccParsed[0]['name']);
		}
		$mailer->XMailer = 'LDAP Account Manager';
		$mailer->send();
		return true;
	}
	catch (Exception $e) {
		logNewMessage(LOG_ERR, 'Mail sending failed: ' . $e->getMessage());
		return false;
	}
}

/**
 * Checks if an email address is safe for use on commandline
 *
 * @param $address email address
 * @return bool is safe
 */
function isCommandlineSafeEmailAddress($address) {
	$cmdEscaped = escapeshellcmd($address);
	$argEscaped = escapeshellarg($address);
	if (($address !== $cmdEscaped) || ("'$address'" !== $argEscaped)) {
		return false;
	}
	$addressLength = strlen($address);
	$allowedSpecialChars = array('@', '_', '-', '.');
	for ($i = 0; $i < $addressLength; $i++) {
		$char = $address[$i];
		if (!ctype_alnum($char) && !in_array($char, $allowedSpecialChars)) {
			return false;
		}
	}
	return true;
}

/**
 * Caches module objects.
 * This improves performance if the same module does not need to be created multiple times (calling get_metaData() each time).
 *
 * @author Roland Gruber
 */
class moduleCache {

	/** module cache ("name:scope" => module) */
	private static $cache = array();

	/**
	 * Returns a new/cached module with the given name and scope.
	 *
	 * @param String $name module name
	 * @param String $scope module scope (e.g. user)
	 */
	public static function getModule($name, $scope) {
		if (isset(self::$cache[$name . ':' . $scope])) {
			return self::$cache[$name . ':' . $scope];
		}
		else {
			self::$cache[$name . ':' . $scope] = new $name($scope);
			return self::$cache[$name . ':' . $scope];
		}
	}

}

/**
 * Returns a random number.
 *
 * @return int random number
 */
function getRandomNumber() {
	if (function_exists('openssl_random_pseudo_bytes')) {
		return abs(hexdec(bin2hex(openssl_random_pseudo_bytes(5))));
	}
	return abs(mt_rand());
}

/**
 * Connects to the LDAP server and extracts the certificates.
 *
 * @param String $server server name
 * @param String $port server port
 * @return mixed false on error and certificate if extracted successfully
 */
function getLDAPSSLCertificate($server, $port) {
	$stream = @stream_context_create(array("ssl" => array("capture_peer_cert_chain" => true, "verify_peer" => false, "allow_self_signed" => true)));
	if (!$stream) {
		return false;
	}
	$client = @stream_socket_client('ssl://' . $server . ':' . $port, $errno, $errstr, 30, STREAM_CLIENT_CONNECT, $stream);
	if (!$client) {
		return false;
	}
	$context = stream_context_get_params($client);
	if (!isset($context['options']['ssl']['peer_certificate_chain'])) {
		return false;
	}
	$finalPEM = '';
	for ($i = 0; $i < sizeof($context['options']['ssl']['peer_certificate_chain']); $i++) {
		$cert = $context['options']['ssl']['peer_certificate_chain'][$i];
		$pemData = null;
		$pemResult = @openssl_x509_export($cert, $pemData);
		if ($pemResult) {
			$finalPEM .= $pemData;
		}
		else {
			return false;
		}
	}
	return $finalPEM;
}

/**
 * Returns the extended LDAP error message if any.
 *
 * @param handle $server LDAP server handle
 * @return String error message
 */
function getExtendedLDAPErrorMessage($server) {
	$ldapMsg = null;
	ldap_get_option($server, LDAP_OPT_ERROR_STRING, $ldapMsg);
	if (empty($ldapMsg)) {
		return ldap_error($server);
	}
	return $ldapMsg;
}

/**
 * Returns the default error message to display on the web page.
 * HTML special characters are already escaped.
 *
 * @param handle $server LDAP server handle
 * @return String error message
 */
function getDefaultLDAPErrorString($server) {
	$extError = htmlspecialchars(getExtendedLDAPErrorMessage($server));
	// Active Directory message translations
	if (strpos($extError, 'DSID') !== false) {
		if (strpos($extError, '5003') !== false) {
			logNewMessage(LOG_DEBUG, 'Password change failed because of ' . $extError);
			$extError = _('Your password does not meet the password strength qualifications. Please retry with another one.');
		}
		elseif (strpos($extError, 'data 530,') !== false) {
			logNewMessage(LOG_DEBUG, 'Login failed because of ' . $extError);
			$extError = _('Logon not permitted at this time');
		}
		elseif (strpos($extError, 'data 532,') !== false) {
			logNewMessage(LOG_DEBUG, 'Login failed because of ' . $extError);
			$extError = _('Password expired');
		}
		elseif (strpos($extError, 'data 533,') !== false) {
			logNewMessage(LOG_DEBUG, 'Login failed because of ' . $extError);
			$extError = _('Account is deactivated');
		}
		elseif (strpos($extError, 'data 701,') !== false) {
			logNewMessage(LOG_DEBUG, 'Login failed because of ' . $extError);
			$extError = _('Account is expired');
		}
		elseif (strpos($extError, 'data 773,') !== false) {
			logNewMessage(LOG_DEBUG, 'Login failed because of ' . $extError);
			$extError = _('Password change required');
		}
		elseif (strpos($extError, 'data 775,') !== false) {
			logNewMessage(LOG_DEBUG, 'Login failed because of ' . $extError);
			$extError = _('Account is locked');
		}
	}
	$genericErrorMessage = ldap_error($server);
	$message = _('LDAP error, server says:') . ' ' . $genericErrorMessage;
	if (!empty($extError) && ($genericErrorMessage != $extError)) {
		$message .= ' - ' . $extError;
	}
	return $message;
}

/**
 * Tries to get additional information why invalid credentials was returned. E.g. account is locked.
 *
 * @param handle $ldap LDAP object to connect for getting extra data
 * @param string $userDn failed DN
 * @return string extra message
 */
function getExtraInvalidCredentialsMessage($ldap, $userDn) {
	include_once __DIR__ . '/plugins/extendedInvalidCredentials/ExtraInvalidCredentials.inc';
	$extraInvalidCredentials = new ExtraInvalidCredentials();
	return $extraInvalidCredentials->getExtraMessage($ldap, $userDn);
}

/**
 * Returns the URL under which the page was loaded.
 * This includes any GET parameters set.
 *
 * @param $baseUrl base URL (e.g. http://www.example.com)
 * @return String URL
 */
function getCallingURL($baseUrl = '') {
	$url = null;
	if (!empty($baseUrl) && !empty($_SERVER['REQUEST_URI'])) {
		$url = $baseUrl . $_SERVER['REQUEST_URI'];
	}
	elseif (!empty($_SERVER['REQUEST_URI']) && !empty($_SERVER['HTTP_HOST'])) {
		$proto = 'http://';
		if (!empty($_SERVER['HTTPS']) && ($_SERVER['HTTPS'] != 'off')) {
			$proto = 'https://';
		}
		$url = $proto . $_SERVER['HTTP_HOST'] . $_SERVER['REQUEST_URI'];
	}
	elseif (!empty($_SERVER['HTTP_REFERER'])) {
		$url = $_SERVER['HTTP_REFERER'];
	}
	return $url;
}

/**
 * Returns the offset in hours from configured time zone to GMT.
 *
 * @return int offset
 */
function getTimeZoneOffsetHours() {
	$dtz = getTimeZone();
	return round($dtz->getOffset(new DateTime('UTC')) / 3600);
}

/**
 * Returns the configured time zone.
 *
 * @return DateTimeZone time zone
 */
function getTimeZone() {
	$timeZoneName = 'UTC';
	if (!empty($_SESSION['config'])) {
		$timeZoneName = $_SESSION['config']->getTimeZone();
	}
	elseif (!empty($_SESSION['selfServiceProfile']->timeZone)) {
		$timeZoneName = $_SESSION['selfServiceProfile']->timeZone;
	}
	return new DateTimeZone($timeZoneName);
}

/**
 * Returns the current time in formatted form.
 *
 * @param unknown $format format to use (e.g. 'Y-m-d H:i:s')
 */
function getFormattedTime($format) {
	$time = new DateTime(null, getTimeZone());
	return $time->format($format);
}

/**
 * Formats a number of seconds to a more human readable format with minutes, hours, etc.
 * E.g. 70 seconds will return 1m10s.
 *
 * @param int $numSeconds number of seconds
 * @return String formatted number
 */
function formatSecondsToShortFormat($numSeconds) {
	if (empty($numSeconds)) {
		return '';
	}
	if (!is_numeric($numSeconds)) {
		return $numSeconds;
	}
	$years = '';
	if ($numSeconds >= 31536000) {
		$years = floor($numSeconds / 31536000);
		$numSeconds = $numSeconds - ($years * 31536000);
		$years .= 'y';
	}
	$seconds = $numSeconds % 60;
	$seconds = ($seconds == 0) ? '' : $seconds . 's';
	$minutes = floor(($numSeconds % 3600) / 60);
	$minutes = ($minutes == 0) ? '' : $minutes . 'm';
	$hours = floor(($numSeconds % 86400) / 3600);
	$hours = ($hours == 0) ? '' : $hours . 'h';
	$days = floor(($numSeconds % 604800) / 86400);
	$days = ($days == 0) ? '' : $days . 'd';
	$weeks = floor($numSeconds / 604800);
	$weeks = ($weeks == 0) ? '' : $weeks . 'w';
	return $years . $weeks . $days . $hours . $minutes . $seconds;
}

/**
 * Unformats text like 1m10s back to number of seconds.
 *
 * @param String $text formatted text
 * @return int number of seconds
 */
function unformatShortFormatToSeconds($text) {
	if (empty($text)) {
		return $text;
	}
	$matches = array();
	if (preg_match('/^(([0-9]+)y)?(([0-9]+)w)?(([0-9]+)d)?(([0-9]+)h)?(([0-9]+)m)?(([0-9]+)s)?$/', $text, $matches)) {
		$newValue = 0;
		if (!empty($matches[2])) {
			$newValue += $matches[2] * 31536000;
		}
		if (!empty($matches[4])) {
			$newValue += $matches[4] * 604800;
		}
		if (!empty($matches[6])) {
			$newValue += $matches[6] * 86400;
		}
		if (!empty($matches[8])) {
			$newValue += $matches[8] * 3600;
		}
		if (!empty($matches[10])) {
			$newValue += $matches[10] * 60;
		}
		if (!empty($matches[12])) {
			$newValue += $matches[12];
		}
		return $newValue;
	}
	return $text;
}

/**
 * Validates the Google reCAPTCHA input.
 *
 * @param String $secretKey secret key
 * @return boolean valid
 */
function validateReCAPTCHA($secretKey) {
	$url = 'https://www.google.com/recaptcha/api/siteverify';
	$vars = array('secret' => $secretKey, 'response' => $_POST['g-recaptcha-response']);
	$options = array(
		'http' => array(
			'header'  => "Content-type: application/x-www-form-urlencoded\r\n",
			'method'  => 'POST',
			'content' => http_build_query($vars)
		)
	);
	$context = stream_context_create($options);
	$result = file_get_contents($url, false, $context);
	if ($result === FALSE) {
		logNewMessage(LOG_ERR, 'reCAPTCHA validation failed, invalid server response.');
		return false;
	}
	$responseJSON = json_decode($result);
	logNewMessage(LOG_DEBUG, "ReCAPTCHA result: " . $result);
	return $responseJSON->{'success'} === true;
}

/**
 * Checks if the user is logged in. Stops script execution if not.
 *
 * @param boolean $check2ndFactor check if the 2nd factor was provided if required
 */
function enforceUserIsLoggedIn($check2ndFactor = true) {
	if ((!isset($_SESSION['loggedIn']) || ($_SESSION['loggedIn'] !== true)) && empty($_SESSION['selfService_clientPassword'])) {
		logNewMessage(LOG_WARNING, 'Detected unauthorized access to page that requires login: ' . $_SERVER["SCRIPT_FILENAME"]);
		die();
	}
	if ($check2ndFactor && isset($_SESSION['2factorRequired'])) {
		logNewMessage(LOG_WARNING, 'Detected unauthorized access to page that requires login (2nd factor not provided): ' . $_SERVER["SCRIPT_FILENAME"]);
		die();
	}
}

/**
 * Prints the content of the header part.
 *
 * @param string $title page title
 * @param string $prefix prefix to LAM main folder (e.g. "..")
 */
function printHeaderContents($title, $prefix) {
	echo '<meta name="viewport" content="width=device-width, initial-scale=1.0">';
	echo '<title>';
	echo $title;
	echo '</title>';
	// include responsive CSS
	$cssDirName = dirname(__FILE__) . '/../style/responsive';
	$cssDir = dir($cssDirName);
	$cssFiles = array();
	$cssEntry = $cssDir->read();
	while ($cssEntry !== false) {
		if (substr($cssEntry, strlen($cssEntry) - 4, 4) == '.css') {
			$cssFiles[] = $cssEntry;
		}
		$cssEntry = $cssDir->read();
	}
	sort($cssFiles);
	foreach ($cssFiles as $cssEntry) {
		echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"$prefix/style/responsive/$cssEntry\">";
	}
	// include all other CSS files
	$cssDirName = dirname(__FILE__) . '/../style';
	$cssDir = dir($cssDirName);
	$cssFiles = array();
	$cssEntry = $cssDir->read();
	while ($cssEntry !== false) {
		if (substr($cssEntry, strlen($cssEntry) - 4, 4) == '.css') {
			$cssFiles[] = $cssEntry;
		}
		$cssEntry = $cssDir->read();
	}
	sort($cssFiles);
	foreach ($cssFiles as $cssEntry) {
		echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"$prefix/style/$cssEntry\">";
	}
	echo '<link rel="shortcut icon" type="image/x-icon" href="' . $prefix . '/graphics/favicon.ico">';
	echo '<link rel="icon" href="' . $prefix . '/graphics/logo136.png">';
}

/**
 * Prints script tags for all LAM JS files.
 *
 * @param string $prefix prefix to LAM main folder (e.g. "..")
 */
function printJsIncludes($prefix) {
	$jsDirName = dirname(__FILE__) . '/../templates/lib';
	$jsDir = dir($jsDirName);
	$jsFiles = array();
	$jsEntry = $jsDir->read();
	while ($jsEntry !== false) {
		if ((substr($jsEntry, strlen($jsEntry) - 3, 3) == '.js') || (substr($jsEntry, strlen($jsEntry) - 4, 4) == '.php')) {
			$jsFiles[] = $jsEntry;
		}
		$jsEntry = $jsDir->read();
	}
	sort($jsFiles);
	foreach ($jsFiles as $jsEntry) {
		echo "<script type=\"text/javascript\" src=\"$prefix/templates/lib/" . $jsEntry . "\"></script>\n";
	}
}

/**
 * Converts an UTF-8 string to UTF16LE.
 *
 * @param string $input UTF-8 value
 */
function convertUtf8ToUtf16Le($input) {
	if (($input == null) || (strlen($input) == 0)) {
		return $input;
	}
	$output = iconv('UTF-8', 'UTF-16LE', $input);
	if (($output === false) || ($output == '')) {
		$output = mb_convert_encoding($input, 'UTF-8', 'UTF-16LE');
	}
	return $output;
}

/**
 * Returns the text with LAM and its version for header area.
 *
 * @return string LAM version text
 */
function getLAMVersionText() {
	$text = 'LDAP Account Manager';
	if (isLAMProVersion()) {
		$text .= ' Pro';
	}
	return $text . ' - ' . LAMVersion();
}

/**
 * Returns if the given release is a developer version.
 *
 * @param string version
 * @return bool is developer version
 */
function isDeveloperVersion($version) {
	return strpos($version, 'DEV') !== false;
}

/**
 * LAM exception with title and message.
 *
 * @author Roland Gruber
 */
class LAMException extends Exception {

	private $title;
	
	private $ldapErrorCode;

	/**
	 * Constructor.
	 *
	 * @param string $title title
	 * @param string $message message (optional)
	 * @param Exception $cause (optional)
	 * @param int $ldapErrorCode original LDAP error code
	 */
	public function __construct($title, $message = null, $cause = null, $ldapErrorCode = null) {
		parent::__construct($message, null, $cause);
		$this->title = $title;
		$this->ldapErrorCode = $ldapErrorCode;
	}

	/**
	 * Returns the message title.
	 *
	 * @return string title
	 */
	public function getTitle() {
		return $this->title;
	}

	/**
	 * Returns the original LDAP error code.
	 *
	 * @return int error code
	 */
	public function getLdapErrorCode() {
		return $this->ldapErrorCode;
	}

}

?>
