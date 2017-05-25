<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Tilo Lutz
                2009 - 2017  Roland Gruber

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
 * Checks if a string exists in an array, ignoring case.
 *
 * @param String $needle search string
 * @param array $haystack array
 */
function in_array_ignore_case($needle, $haystack) {
    if( ! is_array( $haystack ) )
        return false;
    if( ! is_string( $needle ) )
        return false;
    foreach( $haystack as $element )
        if( is_string( $element ) && 0 == strcasecmp( $needle, $element ) )
            return true;
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
	// Needed to calculate Samba passwords
	include_once("createntlm.inc");
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
	include_once("createntlm.inc");
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
			$hash = "{CRYPT}" . crypt($password);
			break;
		case 'CRYPT-SHA512':
			$hash = "{CRYPT}" . crypt($password, '$6$' . generateSalt(16));
			break;
		case 'MD5':
			$hash = "{MD5}" . base64_encode(convertHex2bin(md5($password)));
			break;
		case 'SMD5':
			$salt = generateSalt(4);
			$hash = "{SMD5}" . base64_encode(convertHex2bin(md5($password . $salt)) . $salt);
			break;
		case 'SHA':
			$hash = "{SHA}" . base64_encode(convertHex2bin(sha1($password)));
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
			$hash = "{SSHA}" . base64_encode(convertHex2bin(sha1($password . $salt)) . $salt);
			break;
	}
	// enable/disable password
	if (! $enabled) return pwd_disable($hash);
	else return $hash;
}

/**
 * Returns the list of supported hash types (e.g. SSHA).
 *
 * @return array hash types
 */
function getSupportedHashTypes() {
	return array('CRYPT', 'CRYPT-SHA512', 'SHA', 'SSHA', 'MD5', 'SMD5', 'PLAIN', 'SASL', 'K5KEY');
}

/**
* Calculates a password salt of the given legth.
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
		$salt .= $chars{$pos};
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
 * Generates a random password with 12 digits.
 *
 * @return String password
 */
function generateRandomPassword() {
	$list = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.-_';
	$password = '';
	$length = $_SESSION['cfgMain']->passwordMinLength;
	if ($length < 12) {
		$length = 12;
	}
	$isOk = false;
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
 * Checks if the given password mathes the crypto hash.
 *
 * @param String type hash type (must be one of getSupportedHashTypes())
 * @param unknown_type $hash password hash value
 * @param unknown_type $password plain text password to check
 * @see getSupportedHashTypes()
 */
function checkPasswordHash($type, $hash, $password) {
	switch ($type) {
		case 'SSHA':
			$bin = base64_decode($hash);
			$salt = substr($bin, 20);
			$pwdHash = base64_encode(convertHex2bin(sha1($password . $salt)) . $salt);
			return (strcmp($hash, $pwdHash) == 0);
			break;
		case 'SHA':
			return (strcmp($hash, base64_encode(convertHex2bin(sha1($password)))) == 0);
			break;
		case 'SMD5':
			$bin = base64_decode($hash);
			$salt = substr($bin, 16);
			$pwdHash = base64_encode(convertHex2bin(md5($password . $salt)) . $salt);
			return (strcmp($hash, $pwdHash) == 0);
			break;
		case 'MD5':
			return (strcmp($hash, base64_encode(convertHex2bin(md5($password)))) == 0);
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
		if (isset($units[$i]['sambanextrid'][0])) $ret[$i]->nextRID = $units[$i]['sambanextrid'][0];
		if (isset($units[$i]['sambanextgrouprid'][0])) $ret[$i]->nextGroupRID = $units[$i]['sambanextgrouprid'][0];
		if (isset($units[$i]['sambanextuserrid'][0])) $ret[$i]->nextUserRID = $units[$i]['sambanextuserrid'][0];
		if (isset($units[$i]['sambaalgorithmicridbase'][0])) $ret[$i]->RIDbase = $units[$i]['sambaalgorithmicridbase'][0];
		if (isset($units[$i]['sambaminpwdage'][0])) $ret[$i]->minPwdAge = $units[$i]['sambaminpwdage'][0];
		if (isset($units[$i]['sambamaxpwdage'][0])) $ret[$i]->maxPwdAge = $units[$i]['sambamaxpwdage'][0];
		if (isset($units[$i]['sambapwdhistorylength'][0])) $ret[$i]->pwdHistoryLength = $units[$i]['sambapwdhistorylength'][0];
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
	// workaround for buggy PHP with Turkish
	if (($language == 'tr_TR.utf8') && (version_compare(phpversion(), '5.5') < 0)) {
		setlocale(LC_CTYPE, 'en_GB');
	}
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
	}
	if ($pregexpr!='')
		if (preg_match($pregexpr, $argument)) {
			/* Bug in php preg_match doesn't work correct with utf8 */
			setlocale(LC_ALL, $language);
			// workaround for buggy PHP with Turkish
			if (($language == 'tr_TR.utf8') && (version_compare(phpversion(), '5.5') < 0)) {
				setlocale(LC_CTYPE, 'en_GB');
			}
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
			/* Bug in php preg_match doesn't work correct with utf8 */
			setlocale(LC_ALL, $language);
			// workaround for buggy PHP with Turkish
			if (($language == 'tr_TR.utf8') && (version_compare(phpversion(), '5.5') < 0)) {
				setlocale(LC_CTYPE, 'en_GB');
			}
			return true;
		}
	/* Bug in php preg_match doesn't work correct with utf8 */
	setlocale(LC_ALL, $language);
	// workaround for buggy PHP with Turkish
	if (($language == 'tr_TR.utf8') && (version_compare(phpversion(), '5.5') < 0)) {
		setlocale(LC_CTYPE, 'en_GB');
	}
	return false;
}

/**
 * Escapes any special characters in an LDAP DN.
 *
 * @param String $dn DN
 * @return String escaped DN
 */
function escapeDN($dn) {
	$dn = preg_replace('/[ ]*,[ ]*/', ',', $dn);
	return str_replace(
		array(')',    '(',    ' ',    '*'),
		array('\\29', '\\28', '\\20', '\\2a'),
		$dn
	);
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
		$filterParts[] = '(' . $name . '=' . $value . ')';
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
		$handle = $_SESSION['ldap']->server();
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
				for ($e = 0; $e < sizeof($subErrors); $e++) $errors[] = $subErrors[$e];
			}
		}
		else {
			$errors[] = array ('ERROR', sprintf(_('Was unable to delete DN: %s.'), $dn), getDefaultLDAPErrorString($_SESSION['ldap']->server()));
			return $errors;
		}
	}
	// delete parent DN
	$success = @ldap_delete($_SESSION['ldap']->server(), $dn);
	$ldapUser = $_SESSION['ldap']->decrypt_login();
	$ldapUser = $ldapUser[0];
	if (!$success) {
		logNewMessage(LOG_ERR, '[' . $ldapUser .'] Unable to delete DN: ' . $dn . ' (' . ldap_error($_SESSION['ldap']->server()) . ').');
		$errors[] = array ('ERROR', sprintf(_('Was unable to delete DN: %s.'), $dn), getDefaultLDAPErrorString($_SESSION['ldap']->server()));
	}
	else {
		logNewMessage(LOG_NOTICE, '[' . $ldapUser .'] Deleted DN: ' . $dn);
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
	$parts = explode(',', $dn);
	for ($i = 0; $i < sizeof($parts); $i++) {
		$subparts = explode('=', $parts[$i]);
		if (sizeof($subparts) == 2) {
			$parts[$i] = $subparts[1];
		}
	}
	return implode(' > ', $parts);
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
				if ($len_a > $len_b) return 1;
				elseif ($len_a < $len_b) return -1;
				else return 0; // DNs are identical
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
 * @return String formated time
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
	if (strpos($deob, 'LAM_OBFUSCATE:') === 0) {
		return true;
	}
	else {
		return false;
	}
}

/**
 * Extracts the RDN attribute name from a given DN.
 *
 * @param String $dn DN
 * @return String RDN attribute name
 */
function extractRDNAttribute($dn) {
	if ($dn == null) return null;
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
	if (empty($dn)) return null;
	$parts = explode("=", substr($dn, 0, strpos($dn, ',')));
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
	if ($dn == null) return null;
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
	$headerLines = createEMailHeaders($mailFrom, ($mailIsHTML == 'true'), $mailReplyTo);
	$success = sendEMail($mailTo, $subject, $body, $headerLines);
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
 * Generates the email header text for the given parameters.
 *
 * @param String $from FROM address
 * @param boolean $isHTML mail is formatted as HTML or plain text
 * @param String $replyTo reply-to address (optional)
 * @param String $cc CC address
 * @param String $bcc BCC address
 * @return String header lines
 */
function createEMailHeaders($from, $isHTML, $replyTo = null, $cc = null, $bcc = null) {
	$headerLines = "X-Mailer: LDAP Account Manager\r\n";
	if (!empty($from)) {
		$headerLines .= 'From: ' . encodeMailAddress($from) . "\r\n";
	}
	if (!empty($replyTo)) {
		$headerLines .= 'Reply-To: ' . encodeMailAddress($replyTo) . "\r\n";
	}
	if (!empty($cc)) {
		$headerLines .= 'Cc: ' . encodeMailAddress($cc) . "\r\n";
	}
	if (!empty($bcc)) {
		$headerLines .= 'Bcc: ' . encodeMailAddress($bcc) . "\r\n";
	}
	$headerLines .= "MIME-Version: 1.0\r\n";
	if ($isHTML) {
		$headerLines .= "Content-type: text/html; charset=UTF-8\r\n";
	}
	else {
		$headerLines .= "Content-type: text/plain; charset=UTF-8\r\n";
	}
	return $headerLines;
}

/**
 * Encodes the email address for the header part of an email.
 *
 * @param String $address email address
 * @return String encoded mail address
 */
function encodeMailAddress($address) {
	$matches = array();
	// if the email contains a name part then base64 encode it
	if (preg_match('/^(.*)<(.*)>$/', $address, $matches)) {
		return base64EncodeForEMail($matches[1]) . ' <' . $matches[2] . '>';
	}
	return $address;
}

/**
 * Returns a base64 encoded string of the given values in a fomat that is used in emails.
 *
 * @param String $value value to encode
 * @return String base64 encoded value
 */
function base64EncodeForEMail($value) {
	return '=?UTF-8?B?' . base64_encode($value) . '?=';
}

/**
 * Sends out an email.
 *
 * @param String $to TO address
 * @param String $subject email subject
 * @param String $text mail body (with \r\n EOL)
 * @param String $headers header lines (with \r\n EOL)
 */
function sendEMail($to, $subject, $text, $headers) {
	if (!empty($_SESSION['cfgMain']->mailEOL) && ($_SESSION['cfgMain']->mailEOL === 'unix')) {
		$text = str_replace("\r\n", "\n", $text);
		$headers = str_replace("\r\n", "\n", $headers);
	}
	logNewMessage(LOG_DEBUG, "Send mail to $to\n" . $text);
	return mail($to, base64EncodeForEMail($subject), $text, $headers);
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
		if (isset(moduleCache::$cache[$name . ':' . $scope])) {
			return moduleCache::$cache[$name . ':' . $scope];
		}
		else {
			moduleCache::$cache[$name . ':' . $scope] = new $name($scope);
			return moduleCache::$cache[$name . ':' . $scope];
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
	$stream = @stream_context_create(array("ssl" => array("capture_peer_cert_chain" => true)));
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
		return null;
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
	if ((strpos($extError, 'DSID-031A120C') !== false) && (strpos($extError, '5003') !== false)) {
		logNewMessage(LOG_DEBUG, 'Password change failed because of ' . $extError);
		$extError = _('Your password does not meet the password strength qualifications. Please retry with another one.');
	}
	$message = _('LDAP error, server says:') . ' ' . ldap_error($server);
	if (!empty($extError)) {
		$message .= ' - ' . $extError;
	}
	return $message;
}

/**
 * Returns the URL under which the page was loaded.
 * This includes any GET parameters set.
 *
 * @return String URL
 */
function getCallingURL() {
	$url = null;
	if (!empty($_SERVER['HTTP_HOST']) && !empty($_SERVER['REQUEST_URI'])) {
		$proto = 'http://';
		if (!empty($_SERVER['HTTPS']) && ($_SERVER['HTTPS'] != 'off')) {
			$proto = 'https://';
		}
		$url = $proto . $_SERVER['HTTP_HOST'] . $_SERVER['REQUEST_URI'];
	}
	elseif (!empty($_SERVER['HTTP_REFERER'])) {
		$url = $_SERVER['HTTP_REFERER'];
	}
	logNewMessage(LOG_DEBUG, 'Calling URL detected as ' . $url);
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
 * @return String formated number
 */
function formatSecondsToShortFormat($numSeconds) {
	if (empty($numSeconds)) {
		return '';
	}
	$seconds = $numSeconds % 60;
	$seconds = ($seconds == 0) ? '' : $seconds . 's';
	$minutes = floor(($numSeconds % 3600) / 60);
	$minutes = ($minutes == 0) ? '' : $minutes . 'm';
	$hours = floor(($numSeconds % 86400) / 3600);
	$hours = ($hours == 0) ? '' : $hours . 'h';
	$days = floor($numSeconds / 86400);
	$days = ($days == 0) ? '' : $days . 'd';
	return $days . $hours . $minutes . $seconds;
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
	if (preg_match('/^([0-9]+d)?([0-9]+h)?([0-9]+m)?([0-9]+s)?$/', $text, $matches)) {
		$newValue = 0;
		if (!empty($matches[1])) {
			$newValue += $matches[1] * 86400;
		}
		if (!empty($matches[2])) {
			$newValue += $matches[2] * 3600;
		}
		if (!empty($matches[3])) {
			$newValue += $matches[3] * 60;
		}
		if (!empty($matches[4])) {
			$newValue += $matches[4];
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
		die();
		logNewMessage(LOG_WARNING, 'Detected unauthorized access to page that requires login (2nd factor not provided): ' . $_SERVER["SCRIPT_FILENAME"]);
	}
}

class LAMException extends Exception {

	private $title;

	/**
	 * Constructor.
	 *
	 * @param string $title title
	 * @param string $message message (optional)
	 * @param Exception $cause (optional)
	 */
	public function __construct($title, $message = null, $cause = null) {
		parent::__construct($message, null, $cause);
		$this->title = $title;
	}

	/**
	 * Returns the message title.
	 *
	 * @return string title
	 */
	public function getTitle() {
		return $this->title;
	}

}

?>
