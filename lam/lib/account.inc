<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Tilo Lutz
                2009 - 2011  Roland Gruber

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
* Returns a list of shells listed in config/shells.
*
* @return array list of shell names
*/
function getshells() {
	$return = array();
	$shellPath = dirname(__FILE__) . '/../config/shells';
	// load shells from file
	if (file_exists($shellPath)) {
		$shells = file($shellPath);
		for ($i = 0; $i < sizeof($shells); $i++) {
			// remove whitespaces and line end
			$shells[$i] = trim($shells[$i]);
			// remove comments
			if ((strlen($shells[$i]) == 0) || $shells[$i]{0}=='#') {
				continue;
			}
			$return[] = $shells[$i];
		}
		return $return;
	}
	return $return;
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
 * Checks if a string exists in an array, ignoring case.
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
	if (isset($_SESSION['ldap'])) {
		$rand = $_SESSION['ldap']->new_rand();
	}
	else {
		mt_srand((microtime() * 1000000));
		$rand = mt_rand();
	}
	$hash = "";
	switch ($hashType) {
		case 'CRYPT':
			$hash = "{CRYPT}" . crypt($password);
			break;
		case 'MD5':
			$hash = "{MD5}" . base64_encode(hex2bin(md5($password)));
			break;
		case 'SMD5':
				$salt0 = substr(pack("h*", md5($rand)), 0, 8);
				$salt = substr(pack("H*", md5($salt0 . $password)), 0, 4);
				$hash = "{SMD5}" . base64_encode(hex2bin(md5($password . $salt)) . $salt);
			break;
		case 'SHA':
			$hash = "{SHA}" . base64_encode(hex2bin(sha1($password)));
			break;
		case 'PLAIN':
			$hash = $password;
			break;
		case 'SSHA':
		default: // use SSHA if the setting is invalid
			$salt0 = substr(pack("h*", md5($rand)), 0, 8);
			$salt = substr(pack("H*", sha1($salt0 . $password)), 0, 4);
			$hash = "{SSHA}" . base64_encode(hex2bin(sha1($password . $salt)) . $salt);
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
			$rand = $_SESSION['ldap']->new_rand() % 65;
			$password .= $list[$rand];
		}
		if (checkPasswordStrength($password) === true) {
			break;
		}
	}
	return $password;
}

/**
* Returns an array with all Samba 3 domain entries under the given suffix
*
* @return array list of samba3domain objects
*/
function search_domains($server = null, $suffix = null) {
	if ($suffix == null) {
		$suffix = $_SESSION['config']->get_Suffix('smbDomain');
	}
	$ret = array();
	$attr = array("DN", "sambaDomainName", "sambaSID", "sambaNextRid", "sambaNextGroupRid",
		"sambaNextUserRid", "sambaAlgorithmicRidBase", 'sambaMinPwdAge', 'sambaMaxPwdAge');
	if ($server == null) {
		$server = $_SESSION['ldap']->server();
	}
	$units = searchLDAPByAttribute(null, null, 'sambaDomain', $attr, array('smbDomain'));
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
					$pregexpr = '/^([[:alnum:]\\^\\ \\|\\#\\*\\,\\.\\;\\:\\_\\+\\!\\%\\&\\/\\?\\{\\(\\)\\}\\[\\]\\$@=-])*$/u';
					break;
		case 'groupname':	// all letters, numbers, space and ._- are allowed characters
		case 'username':
		case 'hostname':	
					$pregexpr = '/^([[:alnum:]@\\.\\ \\_\\$-])+$/u';
					break;
		case 'hostObject':
					$pregexpr = '/^[!]?([[:alnum:]@\\.\\ \\_\\$\\*-])+$/u';
					break;
		case 'usernameList':	// comma separated list of user names
		case 'groupnameList':	// comma separated list of group names
					$pregexpr = '/^([[:alnum:]@\\.\\ \\_-])+(,([[:alnum:]@\\.\\ \\_-])+)*$/u';
					break;
		case 'realname':	// Allow all but \, <, >, =, $, ?
		case 'cn':
					$pregexpr = '/^[^\\\<>=\\$\\?]+(\\$)?$/';
					break;
		case "telephone":	// Allow letters, numbers, space, brackets, /-+.
					$pregexpr = '/^(\\+)*([0-9a-zA-Z\\.\\ \\(\\)\\/-])*$/';
					break;
		case "email":
					$pregexpr = '/^([0-9a-zA-Z+\\/\\._-])+[@]([0-9a-zA-Z-])+([.]([0-9a-zA-Z-])+)*$/';
					break;
		case "emailWithName":
					$pregexpr = '/^([[:alnum:] ])+ <([0-9a-zA-Z+\\/\\._-])+[@]([0-9a-zA-Z-])+([.]([0-9a-zA-Z-])+)*>$/u';
					break;
		case "mailLocalAddress":
					$pregexpr = '/^([0-9a-zA-Z+\\/\\._-])+([@]([0-9a-zA-Z-])+([.]([0-9a-zA-Z-])+)*)?$/';
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
		case "UNC":			// UNC Path, e.g. \\server\share\folder\...
					$pregexpr = '/^((([\\\][\\\])|(%))([a-zA-Z0-9%\\.-])+)([\\\]([[:alnum:]%\\.\\$\\ \\_-])+)+$/u';
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
		case 'date':
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
		case 'ascii': // ASCII
					$pregexpr = '/^[' . chr(1) . '-' . chr(128) . ']*$/';
					break;
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
	for ($s = 0; $s < sizeof($scopes); $s++) {
		// search LDAP
		$sr = @ldap_search($_SESSION['ldap']->server(), escapeDN($_SESSION['config']->get_Suffix($scopes[$s])),
							$filter, $attributes, 0, $_SESSION['config']->get_searchLimit(), 0, LDAP_DEREF_NEVER);
		if ($sr) {
			$entries = ldap_get_entries($_SESSION['ldap']->server(), $sr);
			if ($entries) {
				cleanLDAPResult($entries);
				$return = array_merge($return, $entries);
			}
			@ldap_free_result($sr);
		}
	}
	return $return;
}

/**
 * This will search the given LDAP suffix for all entries which match the given filter.
 *
 * @param String $filter
 * @param array $attributes list of attributes to return
 * @param array $scopes account types
 * @return array list of found entries
 */
function searchLDAPByFilter($filter, $attributes, $scopes) {
	$return = array();
	for ($s = 0; $s < sizeof($scopes); $s++) {
		// search LDAP
		$sr = @ldap_search($_SESSION['ldap']->server(), escapeDN($_SESSION['config']->get_Suffix($scopes[$s])),
							$filter, $attributes, 0, $_SESSION['config']->get_searchLimit(), 0, LDAP_DEREF_NEVER);
		if ($sr) {
			$entries = ldap_get_entries($_SESSION['ldap']->server(), $sr);
			if ($entries) {
				cleanLDAPResult($entries);
				$return = array_merge($return, $entries);
			}
			@ldap_free_result($sr);
		}
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
	$return = array();
	$sr = @ldap_search($_SESSION['ldap']->server(), escapeDN($suffix), $filter, $attributes,
						0, $_SESSION['config']->get_searchLimit(), 0, LDAP_DEREF_NEVER);
	if ($sr) {
		$entries = ldap_get_entries($_SESSION['ldap']->server(), $sr);
		if ($entries) {
			cleanLDAPResult($entries);
			@ldap_free_result($sr);
			return $entries;
		}
	}
	return $return;
}

/**
 * Returns the given DN.
 * 
 * @param String $dn DN
 * @param array $attributes list of attributes to fetch
 * @return array attributes or null if not found
 */
function ldapGetDN($dn, $attributes = array('dn')) {
	$return = null;
	$sr = @ldap_read($_SESSION['ldap']->server(), escapeDN($dn), 'objectClass=*', $attributes);
	if ($sr) {
		$entries = ldap_get_entries($_SESSION['ldap']->server(), $sr);
		if ($entries) {
			cleanLDAPResult($entries);
			$return = $entries[0];
		}
		@ldap_free_result($sr);
	}
	return $return;
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
function compareDN($a, $b) {
	// split DNs
	$array_a = explode(",", $a);
	$array_b = explode(",", $b);
	$len_a = sizeof($array_a);
	$len_b = sizeof($array_b);
	// check how many parts to compare
	$len = min($len_a, $len_b);
	// compare from last part on
	for ($i = 0; $i < $len; $i++) {
		// get parts to compare
		$part_a = strtolower($array_a[$len_a - $i - 1]);
		$part_b = strtolower($array_b[$len_b - $i - 1]);
		// compare parts
		if ($part_a == $part_b) { // part is identical
			if ($i == ($len - 1)) {
				if ($len_a > $len_b) return 1;
				elseif ($len_a < $len_b) return -1;
				else return 0; // DNs are identical
			}
		}
		elseif ($part_a == max($part_a, $part_b)) return 1;
		else return -1;
	}
	return -1;
}

/**
 * Formats an LDAP time string (e.g. from createTimestamp).
 * 
 * @return String formated time
 */
function formatLDAPTimestamp($time) {
	return substr($time, 6, 2) . '.' . substr($time, 4, 2) . '.' . substr($time, 0, 4) .
		' ' . substr($time, 8, 2) . ':' . substr($time, 10, 2) . ':' . substr($time, 12, 2) . ' GMT';
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

?>
