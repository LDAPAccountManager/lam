<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Tilo Lutz
                2009 - 2026  Roland Gruber

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
use LAM\PLUGINS\SMS\SmsService;
use LAM\TYPES\TypeManager;
use LDAP\Connection;
use Random\Randomizer;

/**
 * Adapted version of preg_split that returns an empty array instead of false on error.
 * @see preg_split()
 *
 * @param string $pattern search pattern
 * @param string $input input string
 * @param int $limit result limit
 * @param int $flags flags
 * @return string[] result
 */
function lam_preg_split(string $pattern, string $input, int $limit = -1, int $flags = 0): array {
	$result = preg_split($pattern, $input, $limit, $flags);
	if ($result === false) {
		return [];
	}
	return $result;
}

/**
 * This function will return all values from $array without values of $values.
 *
 * @template T
 * @param T[] $values list of values which should be removed
 * @param T[]|null $array list of original values
 * @return T[] list of remaining values
 */
function array_delete($values, $array) {
	// Loop for every entry and check if it should be removed
	if (is_array($array)) {
		$return = [];
		foreach ($array as $array_value) {
			if (!@in_array($array_value, $values)) {
				$return[] = $array_value;
			}
		}
		return $return;
	}
	return [];
}

/**
 * Flattens the values of a multidimensional array.
 * Only unique values are returned.
 *
 * @param mixed[] $array multidimensional array
 * @return mixed[] flat array
 */
function flattenArray(array $array): array {
	$result = [];
	foreach ($array as $element) {
		if (is_array($element)) {
			$result = array_merge($result, flattenArray($element));
		}
		else {
			$result[] = $element;
		}
	}
	return array_values(array_unique($result));
}

/**
 * Checks if a string exists in an array, ignoring case.
 *
 * @param string|null $needle search string
 * @param string[]|null $haystack array
 * @return bool needle contained in haystack
 */
function in_array_ignore_case(?string $needle, ?array $haystack): bool {
	if (!is_array($haystack)) {
		return false;
	}
	if (!is_string($needle)) {
		return false;
	}
	foreach ($haystack as $element) {
		if (0 === strcasecmp($needle, $element)) {
			return true;
		}
	}
	return false;
}

/**
 * Checks if two arrays have the same content.
 *
 * @template T
 * @param T[] $array1 array 1
 * @param T[] $array2 array 2
 * @return bool same content
 */
function areArrayContentsEqual(array $array1, array $array2): bool {
	$intersect = array_intersect($array1, $array2);
	return ((count($array1) === count($array2)) && (count($intersect) === count($array1)));
}

/**
 * Sorts an array in natural order by its keys.
 *
 * @template T
 * @param T[] $toSort array to sort
 * @return T[] sorted array
 */
function natCaseKeySort(array $toSort): array {
	$keys = array_keys($toSort);
	natcasesort($keys);
	$newElements = [];
	foreach ($keys as $key) {
		$newElements[$key] = $toSort[$key];
	}
	return $newElements;
}

/**
 * Generates the NT hash of a password.
 *
 * @param string $password password original password
 * @return string password hash
 */
function ntPassword(string $password): string {
	return strtoupper(hash('md4', convertUtf8ToUtf16Le($password)));
}

/**
 * Returns the hash value of a plain text password.
 *
 * @param string|null $password the password string
 * @param boolean $enabled marks the hash as enabled/disabled (e.g. by prefixing "!")
 * @param string $hashType password hash type (CRYPT, CRYPT-SHA512, SHA, SSHA, MD5, SMD5, PLAIN, K5KEY)
 * @return string the password hash
 *
 * @see getSupportedHashTypes()
 */
function pwd_hash(?string $password, bool $enabled = true, string $hashType = 'SSHA'): string {
	// check for empty password
	if (($password === null) || ($password === "")) {
		return "";
	}
	switch ($hashType) {
		case 'CRYPT':
			$hash = "{CRYPT}" . crypt($password, generateSalt(2));
			break;
		case 'CRYPT-SHA512':
			$hash = "{CRYPT}" . crypt($password, '$6$' . generateSalt(16));
			break;
		case 'PBKDF2-SHA512':
			$iterations = 200000;
			$salt = openssl_random_pseudo_bytes(16);
			$hashBinary = openssl_pbkdf2($password, $salt, 64, $iterations, 'sha512');
			$hash = "{PBKDF2-SHA512}{$iterations}" . '$' . base64_encode($salt) . '$' . base64_encode($hashBinary);
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
		case 'ARGON2ID':
			$threads = max(4, PASSWORD_ARGON2_DEFAULT_THREADS);
			$hash = "{ARGON2}" . password_hash($password, PASSWORD_ARGON2ID, ['threads' => $threads]);
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
 * Returns the hash type of the given password hash.
 * This will return PLAIN if no supported hash type was found.
 *
 * @param string|null $hash password hash
 * @return string type (e.g. SSHA)
 */
function getHashType(?string $hash): string {
	if (empty($hash)) {
		return 'PLAIN';
	}
	$matches = [];
	if (!preg_match('/\\{([A-Z0-9-]+)\\}.+/', $hash, $matches)) {
		return 'PLAIN';
	}
	$type = $matches[1];
	if ($type === 'ARGON2') {
		return 'ARGON2ID';
	}
	if (in_array($type, getSupportedHashTypes())) {
		return $type;
	}
	return 'PLAIN';
}

/**
 * Returns the list of supported hash types (e.g. SSHA).
 *
 * @return string[] hash types
 */
function getSupportedHashTypes(): array {
	return ['CRYPT', 'CRYPT-SHA512', 'SHA', 'SSHA', 'MD5', 'SMD5', 'PLAIN', 'SASL', 'K5KEY', 'LDAP_EXOP', 'ARGON2ID', 'PBKDF2-SHA512'];
}

/**
 * Calculates a password salt of the given length.
 *
 * @param int $len salt length
 * @return string the salt string
 *
 */
function generateSalt($len): string {
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
	if ((str_starts_with($hash, "!{")) || (str_starts_with($hash, "*{"))) {
		return substr($hash, 1, strlen($hash));
	}
	elseif (str_starts_with($hash, "{")) {
		$pos = strpos($hash, "}");
		if ((substr($hash, $pos + 1, 1) === "!") || (substr($hash, $pos + 1, 1) === "*")) {
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

/**
 * Marks an password hash as disabled and returns the new hash string
 *
 * @param string $hash hash value to disable
 * @return string disabled hash value
 */
function pwd_disable($hash) {
	// check if password is disabled (old wrong LAM method)
	if ((str_starts_with($hash, "!{")) || (str_starts_with($hash, "*{"))) {
		return $hash;
	}
	elseif (str_starts_with($hash, "{")) {
		$pos = strpos($hash, "}");
		if ((substr($hash, $pos + 1, 1) === "!") || (substr($hash, $pos + 1, 1) === "*")) {
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
	if (str_starts_with($password, '{SASL}')) {
		return false;
	}
	return ((str_starts_with($password, "{")) || (substr($password, 1, 1) === "{")) && (strpos($password, "}") > 3);
}

/**
 * Checks if a password hash is enabled/disabled
 *
 * @param string $hash password hash to check
 * @return boolean true if the password is marked as enabled
 */
function pwd_is_enabled($hash) {
	// disabled passwords have a "!" or "*" at the beginning (old wrong LAM method)
	if ((str_starts_with($hash, "!{")) || (str_starts_with($hash, "*{"))) {
		return false;
	}
	if (str_starts_with($hash, "{")) {
		$pos = strrpos($hash, "}");
		// check if hash starts with "!" or "*"
		return ((substr($hash, $pos + 1, 1) !== "!") && (substr($hash, $pos + 1, 1) !== "*"));
	}
	else {
		return true;
	}
}

/**
 * Generates a random password with 14 digits by default.
 *
 * @param int $length length of password (defaults to 14)
 * @param bool $checkStrength check if the password matches the policy
 * @return String password
 */
function generateRandomPassword($length = 14, bool $checkStrength = true): string {
	$cfg = $_SESSION['cfgMain'] ?? new LAMCfgMain();
	$minLength = $cfg->passwordMinLength;
	$length = max($length, $minLength);
	if (isset($_SESSION['config']) && is_numeric($_SESSION['config']->getPwdPolicyMinLength())) {
		$length = max($length, $_SESSION['config']->getPwdPolicyMinLength());
	}
	$listAll = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.-_';
	$allLength = strlen($listAll);
	$listLower = 'abcdefghijklmnopqrstuvwxyz';
	$lowerLength = strlen($listLower);
	$minLower = $cfg->passwordMinLower;
	if (isset($_SESSION['config']) && is_numeric($_SESSION['config']->getpwdPolicyMinLowercase())) {
		$minLower = intval($_SESSION['config']->getpwdPolicyMinLowercase());
	}
	$listUpper = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ';
	$upperLength = strlen($listUpper);
	$minUpper = $cfg->passwordMinUpper;
	if (isset($_SESSION['config']) && is_numeric($_SESSION['config']->getPwdPolicyMinUppercase())) {
		$minUpper = $_SESSION['config']->getPwdPolicyMinUppercase();
	}
	$listNumeric = '0123456789';
	$numericLength = strlen($listNumeric);
	$minNumeric = $cfg->passwordMinNumeric;
	if (isset($_SESSION['config']) && is_numeric($_SESSION['config']->getPwdPolicyMinNumeric())) {
		$minNumeric = $_SESSION['config']->getPwdPolicyMinNumeric();
	}
	$listSymbol = '.-_';
	$symbolLength = strlen($listSymbol);
	$minSymbols = $cfg->passwordMinSymbol;
	if (isset($_SESSION['config']) && is_numeric($_SESSION['config']->getPwdPolicyMinSymbolic())) {
		$minSymbols = $_SESSION['config']->getPwdPolicyMinSymbolic();
	}
	for ($x = 0; $x < 10000; $x++) {
		$password = '';
		for ($i = 0; $i < $minLower; $i++) {
			$rand = abs(getRandomNumber() % $lowerLength);
			$password .= $listLower[$rand];
		}
		for ($i = 0; $i < $minUpper; $i++) {
			$rand = abs(getRandomNumber() % $upperLength);
			$password .= $listUpper[$rand];
		}
		for ($i = 0; $i < $minNumeric; $i++) {
			$rand = abs(getRandomNumber() % $numericLength);
			$password .= $listNumeric[$rand];
		}
		for ($i = 0; $i < $minSymbols; $i++) {
			$rand = abs(getRandomNumber() % $symbolLength);
			$password .= $listSymbol[$rand];
		}
		for ($i = 0; $i < $length; $i++) {
			$rand = abs(getRandomNumber() % $allLength);
			$password .= $listAll[$rand];
		}
		if (!$checkStrength || checkPasswordStrength($password, null, [])->isValid()) {
			break;
		}
	}
	$randomizer = new Randomizer(new Random\Engine\Secure());
	return $randomizer->shuffleBytes($password);
}

/**
 * Generates a random text with 20 letters by default.
 *
 * @param int $length length of password (defaults to 20)
 * @return string text
 */
function generateRandomText($length = 20): string {
	$list = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';
	$text = '';
	for ($i = 0; $i < $length; $i++) {
		$rand = abs(getRandomNumber() % 62);
		$text .= $list[$rand];
	}
	return $text;
}

/**
 * Checks if the given password matches the crypto hash.
 *
 * @param string $type type hash type (must be one of getSupportedHashTypes())
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
			return (strcmp($hash, $pwdHash) === 0);
		case 'SHA':
			return (strcmp($hash, base64_encode(hex2bin(sha1($password)))) === 0);
		case 'SMD5':
			$bin = base64_decode($hash);
			$salt = substr($bin, 16);
			$pwdHash = base64_encode(hex2bin(md5($password . $salt)) . $salt);
			return (strcmp($hash, $pwdHash) === 0);
		case 'MD5':
			return (strcmp($hash, base64_encode(hex2bin(md5($password)))) === 0);
		case 'CRYPT':
			$parts = explode('$', $hash);
			if (count($parts) === 1) {
				$salt = substr($hash, 0, 2);
				$pwdHash = crypt($password, $salt);
				return (strcmp($hash, $pwdHash) === 0);
			}
			if (count($parts) === 4) {
				$version = $parts[1];
				$salt = $parts[2];
				$pwdHash = crypt($password, '$' . $version . '$' . $salt);
				return (strcmp($hash, $pwdHash) === 0);
			}
			elseif (count($parts) === 5) {
				$version = $parts[1];
				$rounds = $parts[2];
				$salt = $parts[3];
				$pwdHash = crypt($password, '$' . $version . '$' . $rounds . '$' . $salt);
				return (strcmp($hash, $pwdHash) === 0);
			}
			return false;
		case 'ARGON2ID':
			return password_verify($password, $hash);
		case 'PLAIN':
			return $password === $hash;
		default:
			return false;
	}
}

/**
 * Returns the number of character classes in a password.
 *
 * @param string|null $password password
 * @return int number of classes
 */
function getNumberOfCharacterClasses($password): int {
	if (($password === null) || ($password === '')) {
		return 0;
	}
	$classesCount = 0;
	if (preg_match('/[a-z]/', $password)) {
		$classesCount++;
	}
	if (preg_match('/[A-Z]/', $password)) {
		$classesCount++;
	}
	if (preg_match('/\d/', $password)) {
		$classesCount++;
	}
	if (preg_match('/[^a-z0-9]/i', $password)) {
		$classesCount++;
	}
	return $classesCount;
}

/**
 * Returns an array with all Samba 3 domain entries under the given suffix
 *
 * @param Connection|null $server LDAP handle (if null then $_SESSION['ldap']->server() is used)
 * @param string|null $suffix LDAP suffix to search (if null then $_SESSION['config']->get_Suffix('smbDomain') is used)
 * @return samba3domain[] list of samba3domain objects
 */
function search_domains(?Connection $server = null, ?string $suffix = null): array {
	if ($suffix == null) {
		$suffix = $_SESSION['config']->get_Suffix('smbDomain');
	}
	$ret = [];
	$attr = ["DN", "sambaDomainName", "sambaSID", "sambaNextRid", "sambaNextGroupRid",
		"sambaNextUserRid", "sambaAlgorithmicRidBase", 'sambaMinPwdAge', 'sambaMaxPwdAge',
		'sambaPwdHistoryLength'
	];
	if ($server === null) {
		$server = $_SESSION['ldap']->server();
	}
	$filter = '(objectclass=sambaDomain)';
	$units = searchLDAPPaged($server, $suffix, $filter, $attr, 0, 0);
	// extract attributes
	for ($i = 0; $i < count($units); $i++) {
		$ret[$i] = new samba3domain();
		$ret[$i]->dn = $units[$i]['dn'];
		$ret[$i]->name = $units[$i]['sambadomainname'][0];
		$ret[$i]->SID = $units[$i]['sambasid'][0];
		if (isset($units[$i]['sambanextrid'][0])) {
			$ret[$i]->nextRID = intval($units[$i]['sambanextrid'][0]);
		}
		if (isset($units[$i]['sambanextgrouprid'][0])) {
			$ret[$i]->nextGroupRID = intval($units[$i]['sambanextgrouprid'][0]);
		}
		if (isset($units[$i]['sambanextuserrid'][0])) {
			$ret[$i]->nextUserRID = intval($units[$i]['sambanextuserrid'][0]);
		}
		if (isset($units[$i]['sambaalgorithmicridbase'][0])) {
			$ret[$i]->RIDbase = intval($units[$i]['sambaalgorithmicridbase'][0]);
		}
		if (isset($units[$i]['sambaminpwdage'][0])) {
			$ret[$i]->minPwdAge = intval($units[$i]['sambaminpwdage'][0]);
		}
		if (isset($units[$i]['sambamaxpwdage'][0])) {
			$ret[$i]->maxPwdAge = intval($units[$i]['sambamaxpwdage'][0]);
		}
		if (isset($units[$i]['sambapwdhistorylength'][0])) {
			$ret[$i]->pwdHistoryLength = intval($units[$i]['sambapwdhistorylength'][0]);
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
	public ?string $dn = null;

	/** Domain name */
	public ?string $name = null;

	/** Domain SID */
	public ?string $SID = null;

	/** Next RID */
	public ?int $nextRID = null;

	/** Next user RID */
	public ?int $nextUserRID = null;

	/** Next group RID */
	public ?int $nextGroupRID = null;

	/** RID base to calculate RIDs, default 1000 */
	public int $RIDbase = 1000;

	/** seconds after the password can be changed */
	public ?int $minPwdAge = null;

	/** seconds after the password must be changed */
	public ?int $maxPwdAge = null;

	/** password history length */
	public ?int $pwdHistoryLength = null;
}

/**
 * Checks if a given value matches the selected regular expression.
 *
 * @param string $argument value to check
 * @param string $regexp pattern name
 * @return boolean true if matches, otherwise false
 */
function get_preg($argument, $regexp) {
	// First we check "positive" cases
	$pregexpr = '';
	switch ($regexp) {
		case 'password':
			$pregexpr = '/^([[:alnum:]^ |#*,.;:_+!%&\/?{()}\[\]$§°@=-])*$/u';
			break;
		case 'groupname':    // all letters, numbers, space and ._- are allowed characters
		case 'username':
		case 'hostname':
			$pregexpr = '/^([[:alnum:]%#@. _$-])+$/u';
			break;
		case 'krbUserName':
			$pregexpr = '/^([[:alnum:]#@\/. _$-])+$/u';
			break;
		case 'hostObject':
			$pregexpr = '/^!?([[:alnum:]@. _$:*-])+$/u';
			break;
		case 'usernameList':    // comma separated list of usernames
		case 'groupnameList':    // comma separated list of group names
			$pregexpr = '/^([[:alnum:]%#@. _-])+(,([[:alnum:]%#@. _-])+)*$/u';
			break;
		case 'realname':    // Allow all but \, <, >, =, $, ?
		case 'cn':
			$pregexpr = '/^[^\\\\<>=$?]+(\$)?$/';
			break;
		case "telephone":    // Allow letters, numbers, space, brackets, /-+.
			$pregexpr = '/^(\+)*([0-9a-zA-Z. ()\/-])*$/';
			break;
		case "email":
			$pregexpr = '/^([0-9a-zA-Z\'!~#+*%$\/._-])+[@]([0-9a-zA-Z-])+([.]([0-9a-zA-Z-])+)*$/';
			break;
		case "emailWithName":
			$pregexpr = '/^([[:alnum:] \'!~#+*%$()_-])+ <([0-9a-zA-Z\'!~#+*%$\/._-])+[@]([0-9a-zA-Z-])+([.]([0-9a-zA-Z-])+)*>$/u';
			break;
		case "mailLocalAddress":
			$pregexpr = '/^([0-9a-zA-Z+\\/._-])*([@]([0-9a-zA-Z-])+([.]([0-9a-zA-Z-])+)*)?$/';
			break;
		case 'kolabEmailPrefix':
			$pregexpr = '/^([-])?([0-9a-zA-Z+\\/._-])*([@]([0-9a-zA-Z.-])*)?$/';
			break;
		case "postalAddress":    // Allow all but \, <, >, =, ?
			$pregexpr = '/^[^\\\<>=?]*$/';
			break;
		case "postalCode":    // Allow all but \, <, >, =, ?
		case "street":
		case "title":
		case "employeeType":
		case "businessCategory":
			$pregexpr = '/^[^\\\<>=$?]*$/';
			break;
		case "homeDirectory":        // Home path, /path/......
		case "filePath":
			$pregexpr = '/^([\/]([[:alnum:]@$. _-])+)+(\/)?$/u';
			break;
		case "digit":        // Normal number
			$pregexpr = '/^[[:digit:]]*$/';
			break;
		case "digitWithNegativeValues":        // Normal number incl. negative values
			$pregexpr = '/^[-]?[[:digit:]]*$/';
			break;
		case "float":        // float value
			$pregexpr = '/^[[:digit:]]+(\\.[[:digit:]]+)?$/';
			break;
		case "UNC":            // UNC Path, e.g. \\server\share\folder\...
			$pregexpr = '/^((([\\\][\\\])|(%))([a-zA-Z0-9@%.-])+)([\\\]([[:alnum:]@%.$ _-])+)+$/u';
			break;
		case "logonscript":    // path to login-script. normal unix file
			$pregexpr = '/^(([\/\\\])*([[:alnum:]%. $_-])+([\/\\\]([[:alnum:]%. $_-])+)*((\\.bat)|(\\.cmd)|(\\.exe)|(\\.vbs)))*$/u';
			break;
		case "workstations":    // comma separated list with windows-hosts
			$pregexpr = '/^(([a-zA-Z0-9._-])+(,[a-zA-Z0-9._-])*)*$/';
			break;
		case "domainname":        // Windows Domainname
			$pregexpr = '/^([A-Za-z0-9._-])+$/';
			break;
		case "unixhost":        // Unix hosts
			$pregexpr = '/^([a-z0-9,.*_-])*$/';
			break;
		case 'digit2':            // Same as digit but also -1
			$pregexpr = '/^(([-][1])|([[:digit:]]*))$/';
			break;
		case 'gecos':
			$pregexpr = '/^[[:alnum:] ._-]+([,][[:alnum:] ._-]+)*$/u';
			break;
		case 'macAddress':
			$pregexpr = '/^[0-9a-fA-F]{2}(:[0-9a-fA-F]{2}){5}$/';
			break;
		case 'date':            // 31-12-2012
			$pregexpr = '/^((0?[1-9])|([1-2]\d)|30|31)-((0?[1-9])|(1[0-2]))-[1-3]\d\d\d$/';
			break;
		case 'dateTime':
			$pregexpr = '/^[1-3]\d\d\d\-((0[1-9])|(1[0-2]))-((0[1-9])|([1-2]\d)|30|31) ((0\d)|([1]\d)|20|21|22|23):((0\d)|([1-5]\d)):((0\d)|([1-5]\d))$/';
			break;
		case 'sambaLogonHours':
			$pregexpr = '/^[0-9a-fA-F]{42}$/';
			break;
		case 'DNSname':  // server.example.com
			$pregexpr = '/^[0-9a-zA-Z_-]+(\\.[0-9a-zA-Z_-]+)*$/';
			break;
		case 'DNSnameAbsolute':  // server.example.com.
			$pregexpr = '/^[0-9a-zA-Z_-]+(\\.[0-9a-zA-Z_-]+)*\\.$/';
			break;
		case 'DNSnameRelativeOrAbsolute':  // server.example.com. or server.example.com
			$pregexpr = '/^[0-9a-zA-Z_-]+(\\.[0-9a-zA-Z_-]+)*(\\.)?$/';
			break;
		case 'nis_alias':
			$pregexpr = '/^([[:alnum:]@. _-])+$/u';
			break;
		case 'nis_recipient':
			$pregexpr = '/^([[:alnum:]+@. _-])+$/u';
			break;
		case 'country':    // Allow all letters and space
			$pregexpr = '/^[[:alpha:]]([[:alpha:] ])+$/u';
			break;
		case 'dn':    // LDAP DN
			$pregexpr = '/^([^=,]+=[^=,]+)(,([^=,]+=[^=,]+))*$/';
			break;
		case 'domainSID': // Samba domain SID
			$pregexpr = "/^S-\\d-\\d-\\d{2,2}-\\d+-\\d+-\\d+\$/";
			break;
		case 'ip': // IP address
			$pregexpr = '/^\d{1,3}[.]\d{1,3}[.]\d{1,3}[.]\d{1,3}$/';
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
			$pregexpr = '/^[[:alnum:]._-]+:[[:digit:]]+$/';
			break;
		case 'ldapDateAndTime':
			$pregexpr = '/^\d{14}Z$/';
			break;
		case 'guacamoleProtocol':
			$pregexpr = '/^(rdp|vnc)$/';
			break;
	}
	if (($pregexpr !== '') && preg_match($pregexpr, $argument)) {
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
 return ($pregexpr !== '') && !preg_match($pregexpr, $argument);
}

/**
 * Converts the comma escaping from Windows to OpenLDAP style.
 *
 * @param string $dn DN
 * @return string DN
 */
function convertCommaEscaping(string $dn): string {
	return str_replace(
		['\\,'],
		['\\2C'],
		$dn);
}

/**
 * Connects to an LDAP server using the given URL.
 *
 * @param string $serverURL URL
 * @param bool $startTLS start TLS
 * @return Connection|null LDAP connection
 */
function connectToLDAP(string $serverURL, bool $startTLS): ?Connection {
	logNewMessage(LOG_DEBUG, 'Connecting to LDAP server ' . $serverURL);
	$server = ldap_connect($serverURL);
	if ($server === false) {
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
		if (ldap_errno($server) !== 0) {
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
 * @param string|null $name attribute name
 * @param string|null $value attribute value (may be null if $name is null)
 * @param string|null $objectClass object class
 * @param string[] $attributes list of attributes to return
 * @param string[] $scopes account types
 * @return array<string, string[]|string>[] list of found entries
 */
function searchLDAPByAttribute(?string $name, ?string $value, ?string $objectClass, array $attributes, array $scopes): array {
	$return = [];
	// build filter
	$filter = '';
	$filterParts = [];
	if ($name !== null) {
		$filterParts[] = '(' . $name . '=' . ldap_escape($value, '*', LDAP_ESCAPE_FILTER) . ')';
	}
	if ($objectClass !== null) {
		$filterParts[] = '(objectClass=' . $objectClass . ')';
	}
	if (count($filterParts) === 1) {
		$filter = $filterParts[0];
	}
	elseif (count($filterParts) > 1) {
		$filter = '(& ' . implode(' ', $filterParts) . ')';
	}
	$typeManager = new TypeManager();
	$activeTypes = $typeManager->getConfiguredTypes();
	foreach ($activeTypes as $type) {
		if (!in_array($type->getScope(), $scopes)) {
			continue;  // skip non-active account types
		}
		// search LDAP
		$entries = searchLDAPPaged($_SESSION['ldap']->server(), $type->getSuffix(),
			$filter, $attributes, 0, $_SESSION['config']->get_searchLimit());
		if (ldap_errno($_SESSION['ldap']->server()) === 4) {
			logNewMessage(LOG_WARNING, 'LDAP size limit exceeded. Please increase the limit on your server.');
		}
		$return = array_merge($return, $entries);
	}
	return $return;
}

/**
 * This will search the given LDAP suffix for all entries which match the given filter.
 *
 * @param string $filter
 * @param string[] $attributes list of attributes to return
 * @param string[] $scopes account types
 * @param bool $attrsOnly get only attributes but no values (default: false)
 * @return array<string, string[]|string>[] list of found entries
 */
function searchLDAPByFilter($filter, $attributes, $scopes, $attrsOnly = false): array {
	$return = [];
	$readAttributesOnly = 0;
	if ($attrsOnly) {
		$readAttributesOnly = 1;
	}
	$typeManager = new TypeManager();
	$types = $typeManager->getConfiguredTypesForScopes($scopes);
	foreach ($types as $type) {
		$additionalFilter = $type->getAdditionalLdapFilter();
		if (!empty($additionalFilter)) {
			if (!str_starts_with($additionalFilter, '(')) {
				$additionalFilter = '(' . $additionalFilter . ')';
			}
			if (!str_starts_with($filter, '(')) {
				$filter = '(' . $filter . ')';
			}
			$filter = '(&' . $additionalFilter . $filter . ')';
		}
		// search LDAP
		$entries = searchLDAPPaged($_SESSION['ldap']->server(), $type->getSuffix(),
			$filter, $attributes, $readAttributesOnly, $_SESSION['config']->get_searchLimit());
		if (ldap_errno($_SESSION['ldap']->server()) === 4) {
			logNewMessage(LOG_WARNING, 'LDAP size limit exceeded. Please increase the limit on your server.');
		}
		$return = array_merge($return, $entries);
	}
	return $return;
}

/**
 * Runs an LDAP search.
 *
 * @param string $suffix LDAP suffix
 * @param string $filter filter
 * @param string[] $attributes list of attributes to return
 * @param int $limit result limit
 * @return array<string, string[]|string>[] list of found entries
 */
function searchLDAP($suffix, $filter, $attributes, $limit = -1): array {
	if ($limit === -1) {
		$limit = empty($_SESSION['config']) ? 0 : $_SESSION['config']->get_searchLimit();
	}
	$return = searchLDAPPaged(getLDAPServerHandle(), $suffix, $filter, $attributes,
		0, $limit);
	if (ldap_errno(getLDAPServerHandle()) === 4) {
		logNewMessage(LOG_WARNING, 'LDAP size limit exceeded. Please increase the limit on your server.');
	}
	return $return;
}

/**
 * Returns the LDAP server handle.
 *
 * @return Connection LDAP handle
 */
function getLDAPServerHandle() {
	if (!empty($_SESSION['ldap'])) {
		// admin pages
		return $_SESSION['ldap']->server();
	}
	else {
		// self-service/white pages
		return $_SESSION['ldapHandle']->getServer();
	}
}

/**
 * Runs an LDAP search and uses paging if configured.
 *
 * @param Connection $server LDAP connection handle
 * @param string $dn DN
 * @param string $filter filter
 * @param string[] $attributes attribute list
 * @param int $attrsOnly return only attribute names (1) or also the values (0)
 * @param int $limit size limit
 * @return array<string, string[]|string>[] results
 */
function searchLDAPPaged($server, $dn, $filter, $attributes, $attrsOnly, $limit): array {
	if (empty($_SESSION['config']) || ($_SESSION['config']->getPagedResults() !== 'true')) {
		$sr = @ldap_search($server, $dn, $filter, $attributes, $attrsOnly, $limit, 0, LDAP_DEREF_NEVER, getCommonLdapControls());
		if ($sr) {
			$entries = ldap_get_entries($server, $sr);
			if (!$entries) {
				return [];
			}
			cleanLDAPResult($entries);
			return $entries;
		}
		else {
			return [];
		}
	}
	$pageSize = 999;
	$cookie = '';
	$return = [];
	do {
		$controls = [
			[
				'oid' => LDAP_CONTROL_PAGEDRESULTS,
				'value' => [
					'size' => $pageSize,
					'cookie' => $cookie]
			]
		];
		$commonControls = getCommonLdapControls();
		if ($commonControls !== null) {
			$controls += $commonControls;
		}
		$sr = @ldap_search($server, $dn, $filter, $attributes, $attrsOnly, $limit,
			0, LDAP_DEREF_NEVER, $controls);
		if (!$sr) {
			break;
		}
		$errorCode = 0;
		$matchedDn = '';
		$errorMessage = '';
		$referrals = [];
		$resultControls = [];
		ldap_parse_result($server, $sr, $errorCode , $matchedDn , $errorMessage , $referrals, $resultControls);
		$entries = ldap_get_entries($server, $sr);
		if (!$entries) {
			break;
		}
		$return = array_merge($return, $entries);
		$cookie = $resultControls[LDAP_CONTROL_PAGEDRESULTS]['value']['cookie'] ?? '';
	}
	while ($cookie != '');
	cleanLDAPResult($return);
	return $return;
}

/**
 * Returns the given DN.
 *
 * @param string $dn DN
 * @param string[] $attributes list of attributes to fetch
 * @param Connection $handle LDAP handle (optional for admin interface pages)
 * @return array<string, string[]|string>|null attributes or null if not found
 */
function ldapGetDN($dn, $attributes = ['dn'], $handle = null): ?array {
	if ($handle == null) {
		$handle = getLDAPServerHandle();
	}
	$return = null;
	$sr = @ldap_read($handle, $dn, '(objectClass=*)', $attributes, 0, 0, 0, LDAP_DEREF_NEVER, getCommonLdapControls());
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
 * @param string $dn DN
 * @param string $filter LDAP filter
 * @param string[] $attributes list of attributes to fetch
 * @param Connection|null $handle LDAP handle (optional for admin interface pages)
 * @param int $limit result limit
 * @return array<string, string[]|string>|null attributes or null if not found
 */
function ldapListDN($dn, $filter = '(objectclass=*)', $attributes = ['dn'], $handle = null, $limit = -1) {
	if ($limit === -1) {
		$limit = empty($_SESSION['config']) ? 0 : $_SESSION['config']->get_searchLimit();
	}
	if ($handle == null) {
		$handle = $_SESSION['ldap']->server();
	}
	$return = null;
	$sr = @ldap_list($handle, $dn, $filter, $attributes, 0, $limit, 0, LDAP_DEREF_NEVER, getCommonLdapControls());
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
 * @param string|null $dn DN to delete
 * @param boolean $recursive recursive delete also child entries
 * @return string[][] error messages
 */
function deleteDN($dn, $recursive): array {
	$errors = [];
	if (($dn === null) || ($dn === '')) {
		$errors[] = ['ERROR', _('Entry does not exist')];
		return $errors;
	}
	if ($recursive) {
		$sr = @ldap_list($_SESSION['ldap']->server(), $dn, '(objectClass=*)', ['dn'], 0, 0, 0, LDAP_DEREF_NEVER, getCommonLdapControls());
		if ($sr) {
			$entries = ldap_get_entries($_SESSION['ldap']->server(), $sr);
			cleanLDAPResult($entries);
			for ($i = 0; $i < count($entries); $i++) {
				// delete recursively
				$subErrors = deleteDN($entries[$i]['dn'], $recursive);
				for ($e = 0; $e < count($subErrors); $e++) {
					$errors[] = $subErrors[$e];
				}
			}
		}
		else {
			$errors[] = ['ERROR', sprintf(_('Was unable to delete DN: %s.'), $dn), getDefaultLDAPErrorString($_SESSION['ldap']->server())];
			return $errors;
		}
	}
	// delete parent DN
	$success = @ldap_delete($_SESSION['ldap']->server(), $dn);
	if (!$success) {
		logNewMessage(LOG_ERR, 'Unable to delete DN: ' . $dn . ' (' . ldap_error($_SESSION['ldap']->server()) . ').');
		$errors[] = ['ERROR', sprintf(_('Was unable to delete DN: %s.'), $dn), getDefaultLDAPErrorString($_SESSION['ldap']->server())];
	}
	else {
		logNewMessage(LOG_NOTICE, 'Deleted DN: ' . $dn);
	}
	return $errors;
}

/**
 * Returns a list of LDAP controls for all LDAP calls.
 *
 * @return array<string, string>[]|null controls
 */
function getCommonLdapControls(): ?array {
	if (isset($_SESSION['config']) && ($_SESSION['config']->getAdShowDeleted() === 'true')) {
		return [['oid' => '1.2.840.113556.1.4.417']];
	}
	return null;
}

/**
 * Performs a recursive copy from old DN under target DN.
 *
 * @param string $oldDn old DN to copy
 * @param string $targetDn copy nodes under this DN
 * @throws LAMException error on copy
 */
function copyDnRecursive(string $oldDn, string $targetDn): void {
	$oldRdn = extractRDN($oldDn);
	$newDn = $oldRdn . ',' . $targetDn;
	logNewMessage(LOG_DEBUG, 'Copy DN ' . $oldDn . ' to ' . $newDn);
	$attributes = ldapGetDN($oldDn, ['*']);
	if (empty($attributes)) {
		logNewMessage(LOG_ERR, sprintf(_("Unable to read %s."), unescapeLdapSpecialCharacters($oldDn)));
		throw new LAMException(sprintf(_("Unable to read %s."), htmlspecialchars(unescapeLdapSpecialCharacters($oldDn))));
	}
	unset($attributes['dn']);
	$systemAttributesToSkip = ['iscriticalsystemobject', 'primarygroupid', 'samaccounttype', 'objectsid'];
	foreach ($systemAttributesToSkip as $systemAttributeToSkip) {
		if (isset($attributes[$systemAttributeToSkip])) {
			unset($attributes[$systemAttributeToSkip]);
		}
	}
	$success = ldap_add($_SESSION['ldap']->server(), $newDn, $attributes);
	if (!$success) {
		logNewMessage(LOG_ERR, sprintf(_('Was unable to create DN: %s.'), unescapeLdapSpecialCharacters($oldDn)) . ' ' .
			getExtendedLDAPErrorMessage($_SESSION['ldap']->server()));
		throw new LAMException(sprintf(_('Was unable to create DN: %s.'), htmlspecialchars(unescapeLdapSpecialCharacters($oldDn))),
			getExtendedLDAPErrorMessage($_SESSION['ldap']->server()));
	}
	$children = ldapListDN($oldDn);
	foreach ($children as $child) {
		copyDnRecursive($child['dn'], $newDn);
	}
}

/**
 * Moves an LDAP entry.
 *
 * @param string $oldDn old DN
 * @param string $targetDn target container DN
 * @throws LAMException error during move
 */
function moveDn(string $oldDn, string $targetDn): void {
	$deleteOldRdn = $_SESSION['ldap']->isActiveDirectory();
	$rdn = extractRDN($oldDn);
	$success = ldap_rename($_SESSION['ldap']->server(), $oldDn, $rdn, $targetDn, $deleteOldRdn);
	if (!$success) {
		throw new LAMException(sprintf(_('Was unable to rename DN: %s.'), $oldDn), getDefaultLDAPErrorString($_SESSION['ldap']->server()));
	}
}

/**
 * Returns the parameters for a StatusMessage of the last LDAP search.
 *
 * @return array<string|string[]>|null parameters for StatusMessage or null if all was ok
 */
function getLastLDAPError(): ?array {
	$errorNumber = ldap_errno($_SESSION["ldap"]->server());
	switch ($errorNumber) {
		// all ok
		case 0:
			return null;
		// size limit exceeded
		case 4:
			$error = ["WARN", _("LDAP sizelimit exceeded, not all entries are shown.")];
			if ($_SESSION['config']->get_searchLimit() == 0) {
				// server limit exceeded
				$error[] = _("See the manual for instructions to solve this problem.");
			}
			return $error;
		// other errors
		default:
			return ["ERROR", _("LDAP search failed! Please check your preferences."), ldap_error($_SESSION["ldap"]->server())];
	}
}

/**
 * Cleans the result of an LDAP search.
 * This will remove all 'count' entries and also all numeric array keys.
 *
 * @param array<string|int, mixed> $entries LDAP entries in format $entries[entry number][attribute name][attribute values]
 */
function cleanLDAPResult(array &$entries): void {
	if (isset($entries['count'])) {
		unset($entries['count']);
	}
	// iterate over all results
	$count = count($entries);
	for ($e = 0; $e < $count; $e++) {
		// remove 'count' entries and numerical entries
		for ($i = 0; $i < $entries[$e]['count']; $i++) {
			if (isset($entries[$e][$i])) {
				unset($entries[$e][$i]);
			}
		}
		unset($entries[$e]['count']);
		$attrNames = array_keys($entries[$e]);
		$attrCount = count($attrNames);
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
 * @param string $dn DN
 * @return string transformed DN
 */
function getAbstractDN($dn): string {
	if ($dn == '') {
		return '';
	}
	$dn = str_replace('\\,', '\\2C', $dn);
	if (!empty($_SESSION['config']) && !empty($_SESSION['config']->getHideDnPart())) {
		$partToCut = ',' . $_SESSION['config']->getHideDnPart();
		$dn = str_replace($partToCut, '', $dn);
	}
	$parts = explode(',', $dn);
	for ($i = 0; $i < count($parts); $i++) {
		$subparts = explode('=', $parts[$i]);
		if (count($subparts) === 2) {
			$parts[$i] = $subparts[1];
		}
	}
	$abstractDn = implode(' ❭ ', $parts);
	return unescapeLdapSpecialCharacters($abstractDn);
}

/**
 * Unescapes LDAP special characters for readability.
 *
 * @param string $dn escaped DN
 * @return string unescaped DN
 */
function unescapeLdapSpecialCharacters(string $dn): string {
	return preg_replace_callback('/\\\([0-9A-Fa-f]{2})/', unescapeLdapSpecialCharactersCallback(...), $dn);
}

/**
 * Callback function for unescaping DN.
 *
 * @param array<int,string> $matches HEX value that was found
 * @return string unescaped string
 */
function unescapeLdapSpecialCharactersCallback(array $matches): string {
	return chr(hexdec($matches[1]));
}

/**
 * Helper function to sort DNs.
 *
 * @param string $a first argument to compare
 * @param string $b second argument to compare
 * @return int 0 if equal, 1 if $a is greater, -1 if $b is greater
 */
function compareDN($a, $b): int {
	// split DNs
	$array_a = explode(",", strtolower($a));
	$array_b = explode(",", strtolower($b));
	$len_a = count($array_a);
	$len_b = count($array_b);
	// check how many parts to compare
	$len = min($len_a, $len_b);
	// compare from last part on
	for ($i = 0; $i < $len; $i++) {
		// get parts to compare
		$part_a = $array_a[$len_a - $i - 1];
		$part_a = explode('=', $part_a);
		$part_a = $part_a[1] ?? $part_a[0];
		$part_b = $array_b[$len_b - $i - 1];
		$part_b = explode('=', $part_b);
		$part_b = $part_b[1] ?? $part_b[0];
		// compare parts
		if ($part_a === $part_b) { // part is identical
			if ($i == ($len - 1)) {
				return $len_a <=> $len_b;
			}
		}
		else {
			return strnatcasecmp($part_a, $part_b);
		}
	}
	return -1;
}

/**
 * Helper function to sort LDAP entries by DN.
 *
 * @param array<string, string[]|string> $a first argument to compare
 * @param array<string, string[]|string> $b second argument to compare
 * @return int 0 if equal, 1 if $a is greater, -1 if $b is greater
 */
function compareLDAPEntriesByDn(array $a, array $b): int {
	if (!isset($a['dn']) || !isset($b['dn'])) {
		return 0;
	}
	return compareDN($a['dn'], $b['dn']);
}

/**
 * Does a Base64 encoding that is URL safe.
 *
 * @param string $data input
 * @return string encoded output
 */
function lam_base64url_encode(string $data): string {
	return rtrim(strtr(base64_encode($data), '+/', '-_'), '=');
}

/**
 * Does a Base64 decoding that is URL safe.
 *
 * @param string $data encoded input
 * @return string decoded output
 */
function lam_base64url_decode(string $data): string {
	return base64_decode(str_pad(strtr($data, '-_', '+/'), strlen($data) % 4, '='), true);
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
 * @param string|null $text text to obfuscate
 * @return string|null obfuscated text
 */
function obfuscateText($text) {
	if (($text === null) || ($text === '')) {
		return $text;
	}
	return str_rot13(base64_encode('LAM_OBFUSCATE:' . $text));
}

/**
 * Simple function to deobfuscate strings.
 *
 * @param string|null $text text to deobfuscate
 * @return string|null deobfuscated text
 */
function deobfuscateText($text) {
	if (($text === null) || ($text === '')) {
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
 * @param string|null $text text to check
 * @return bool obfuscated or not
 */
function isObfuscatedText($text): bool {
	if (($text === null) || ($text === '')) {
		return false;
	}
	$deob = base64_decode(str_rot13($text));
	return (str_starts_with($deob, 'LAM_OBFUSCATE:'));
}

/**
 * Extracts the RDN attribute name from a given DN.
 *
 * @param string $dn DN
 * @return string|null RDN attribute name
 */
function extractRDNAttribute($dn) {
	$rdn = extractRDN($dn);
	if (empty($rdn)) {
		return null;
	}
	$parts = explode("=", $rdn);
	return $parts[0];
}

/**
 * Extracts the RDN attribute value from a given DN.
 *
 * @param string $dn DN
 * @return string|null RDN attribute value
 */
function extractRDNValue($dn) {
	$rdn = extractRDN($dn);
	if (empty($rdn)) {
		return null;
	}
	$parts = explode("=", $rdn);
	return $parts[1];
}

/**
 * Extracts the RDN part of the DN.
 *
 * @param string|null $dn DN
 * @return string|null RDN part
 */
function extractRDN(?string $dn): ?string {
	if (empty($dn)) {
		return null;
	}
	$dn = convertCommaEscaping($dn);
	$parts = ldap_explode_dn($dn, 0);
	if (empty($parts[0])) {
		return null;
	}
	$rdn = unescapeLdapSpecialCharacters($parts[0]);
	return str_replace(',', '\\2C', $rdn);
}

/**
 * Extracts the DN suffix from a given DN.
 * E.g. ou=people,dc=test,dc=com will result in dc=test,dc=com.
 *
 * @param string|null $dn DN
 * @return string|null DN suffix
 */
function extractDNSuffix($dn) {
	if ($dn === null) {
		return null;
	}
	$dn = convertCommaEscaping($dn);
	$parts = ldap_explode_dn($dn, 0);
	if ($parts === false) {
		return null;
	}
	unset($parts['count']);
	array_shift($parts);
	for ($i = 0; $i < count($parts); $i++) {
		$parts[$i] = unescapeLdapSpecialCharacters($parts[$i]);
		$parts[$i] = str_replace(',', '\\2C', $parts[$i]);
	}
	return implode(',', $parts);
}

/**
 * Checks if the SMTP connection with the given settings is fine.
 *
 * @param string $server SMTP server
 * @param string $user user name
 * @param string $password password
 * @param string $encryption encryption type
 * @throws LAMException error during SMTP connection
 */
function testSmtpConnection(string $server, string $user, string $password, string $encryption): void {
	include_once __DIR__ . '/3rdParty/composer/autoload.php';
	$mailer = new PHPMailer\PHPMailer\PHPMailer(true);
	try {
		$mailer->isSMTP();
		$serverParts = explode(':', $server);
		$mailer->Host = $serverParts[0];
		$mailer->Port = (int) $serverParts[1];
		if (!empty($user)) {
			$mailer->SMTPAuth = true;
			$mailer->Username = $user;
			$mailer->Password = $password;
		}
		$mailEncryption = $encryption;
		if (empty($mailEncryption) || ($mailEncryption === LAMCfgMain::SMTP_TLS)) {
			$mailer->SMTPSecure = PHPMailer\PHPMailer\PHPMailer::ENCRYPTION_STARTTLS;
		}
		elseif ($mailEncryption === LAMCfgMain::SMTP_SSL) {
			$mailer->SMTPSecure = PHPMailer\PHPMailer\PHPMailer::ENCRYPTION_SMTPS;
		}
		else {
			$mailer->SMTPAutoTLS = false;
		}
		$mailer->CharSet = PHPMailer\PHPMailer\PHPMailer::CHARSET_UTF8;
		$mailer->Timeout = 30;
		$mailer->smtpConnect();
	}
	catch (Exception $e) {
		throw new LAMException(null, $e->getMessage(), $e);
	}
}

/**
 * Sends the password mail.
 *
 * @param string $pwd new password
 * @param array<string, string[]|string> $user LDAP attributes of user
 * @param string|null $recipient recipient address (optional, $user['mail'][0] used by default)
 * @return array<string|string[]>[] list of arrays that can be used to create status messages
 */
function sendPasswordMail($pwd, $user, $recipient = null) {
	$user = array_change_key_case($user);
	// read mail data
	$cfgMain = $_SESSION['cfgMain'];
	$mailTo = null;
	if (!empty($recipient)) {
		$mailTo = $recipient;
	}
	elseif (!empty($user[$cfgMain->getMailAttribute()][0])) {
		$mailTo = $user[$cfgMain->getMailAttribute()][0];
	}
	if (empty($mailTo)) {
		logNewMessage(LOG_ERR, 'Unable to send password mail, no TO address set.');
		return [
			['ERROR', _('Unable to send mail!')]
		];
	}
	$mailFrom = $_SESSION['config']->getLamProMailFrom();
	$mailReplyTo = $_SESSION['config']->getLamProMailReplyTo();
	$mailSubject = $_SESSION['config']->getLamProMailSubject();
	$mailText = $_SESSION['config']->getLamProMailText();
	$mailIsHTML = $_SESSION['config']->getLamProMailIsHTML();
	$subject = $mailSubject;
	$body = $mailText;
	$body = str_replace('@@newPassword@@', $pwd, $body);
	$results = [];
	$found = preg_match('/\@\@[^\@]+\@\@/', $body, $results);
	while ($found == 1) {
		$attr = str_replace('@', '', $results[0]);
		$value = '';
		if (isset($user[strtolower($attr)][0])) {
			$value = is_array($user[strtolower($attr)]) ? $user[strtolower($attr)][0] : $user[strtolower($attr)];
		}
		$body = str_replace('@@' . $attr . '@@', $value, $body);
		$found = preg_match('/\@\@[^\@]+\@\@/', $body, $results);
	}
	$success = sendEMail($mailTo, $subject, $body, $mailFrom, ($mailIsHTML == 'true'), $mailReplyTo);
	if ($success) {
		logNewMessage(LOG_DEBUG, 'Sent password mail to ' . $mailTo);
		return [
			['INFO', sprintf(_('Mail successfully sent to %s.'), htmlspecialchars($mailTo))]
		];
	}
	else {
		logNewMessage(LOG_ERR, 'Unable to send password mail to ' . htmlspecialchars($mailTo));
		return [
			['ERROR', _('Unable to send mail!')]
		];
	}
}

/**
 * Sends out an email.
 *
 * @param string|string[] $to TO address(es)
 * @param string $subject email subject
 * @param string $text mail body (with \r\n EOL)
 * @param string $from FROM address
 * @param bool $isHTML HTML format
 * @param string $replyTo REPLY-TO address (optional)
 * @param string $cc CC address (optional)
 * @param string $bcc BCC address (optional)
 * @return bool successful
 */
function sendEMail($to, $subject, $text, $from, $isHTML, $replyTo = null, $cc = null, $bcc = null): bool {
	include_once __DIR__ . '/3rdParty/composer/autoload.php';
	$returnPath = empty($replyTo) ? $from : $replyTo;
	$returnPathParsed = parseEmailAddress($returnPath);
	logNewMessage(LOG_DEBUG, "Send mail from " . $from . " to " . print_r($to, true) . "\n" . $text);
	$mailer = new PHPMailer\PHPMailer\PHPMailer(true);
	try {
		$cfgMain = $_SESSION['cfgMain'];
		if (!empty($cfgMain->mailServer)) {
			$mailer->isSMTP();
			$serverParts = explode(':', $cfgMain->mailServer);
			$mailer->Host = $serverParts[0];
			$mailer->Port = (int) $serverParts[1];
			if (!empty($cfgMain->mailUser)) {
				$mailer->SMTPAuth = true;
				$mailer->Username = $cfgMain->mailUser;
				$mailer->Password = $cfgMain->mailPassword;
			}
			$mailEncryption = $cfgMain->mailEncryption;
			if (empty($mailEncryption) || ($mailEncryption === LAMCfgMain::SMTP_TLS)) {
				$mailer->SMTPSecure = PHPMailer\PHPMailer\PHPMailer::ENCRYPTION_STARTTLS;
			}
			elseif ($mailEncryption === LAMCfgMain::SMTP_SSL) {
				$mailer->SMTPSecure = PHPMailer\PHPMailer\PHPMailer::ENCRYPTION_SMTPS;
			}
			else {
				$mailer->SMTPAutoTLS = false;
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
		$mailer->Sender = $returnPathParsed['address'];
		$fromParsed = parseEmailAddress($from);
		$mailer->setFrom($fromParsed['address'], $fromParsed['name']);
		$mailer->isHTML($isHTML);
		if (!empty($replyTo)) {
			$replyToParsed = parseEmailAddress($replyTo);
			$mailer->addReplyTo($replyToParsed['address'], $replyToParsed['name']);
		}
		if (!empty($cc)) {
			$ccParsed = parseEmailAddress($cc);
			$mailer->addCC($ccParsed['address'], $ccParsed['name']);
		}
		if (!empty($bcc)) {
			$bccParsed = parseEmailAddress($bcc);
			$mailer->addBCC($bccParsed['address'], $bccParsed['name']);
		}
		$mailer->XMailer = 'LDAP Account Manager';
		$mailSentOk = $mailer->send();
		if (!$mailSentOk) {
			logNewMessage(LOG_ERR, 'Mail sending failed: ' . $mailer->ErrorInfo);
		}
		else {
			logNewMessage(LOG_DEBUG, 'Mail sent');
		}
		return $mailSentOk;
	}
	catch (Exception $e) {
		logNewMessage(LOG_ERR, 'Mail sending failed: ' . $e->getMessage());
		return false;
	}
}

/**
 * Parses an email address text that optionally contains a display name.
 *
 * @param string $addressText address text (either plain email or with display name)
 * @return array<string, string> parsed email address data ('address' => 'adr@example.com', 'name' => 'John Smith')
 * @throws LAMException unknown email format
 */
function parseEmailAddress(string $addressText): array {
	$addressText = trim($addressText);
	if (!str_contains($addressText, '>')) {
		return [
			'address' => $addressText,
			'name' => '',
		];
	}
	$matches = [];
	if (preg_match('/"([^"]+)"\s*<([^<]+)>/', $addressText, $matches)) {
		return [
			'address' => trim($matches[2]),
			'name' => trim($matches[1]),
		];
	}
	elseif (preg_match('/\'([^\']+)\'\s*<([^<]+)>/', $addressText, $matches)) {
		return [
			'address' => trim($matches[2]),
			'name' => trim($matches[1]),
		];
	}
	elseif (preg_match('/([^"\']+)\s*<([^<]+)>/', $addressText, $matches)) {
		return [
			'address' => trim($matches[2]),
			'name' => trim($matches[1]),
		];
	}
	logNewMessage(LOG_ERR, 'Unable to parse email address: ' . $addressText);
	throw new LAMException(null, _('Please enter a valid email address!'));
}

/**
 * Checks if an email address is safe for use on commandline
 *
 * @param string $address email address
 * @return bool is safe
 */
function isCommandlineSafeEmailAddress(string $address): bool {
	$cmdEscaped = escapeshellcmd($address);
	$argEscaped = escapeshellarg($address);
	if (($address !== $cmdEscaped) || ("'$address'" !== $argEscaped)) {
		return false;
	}
	$addressLength = strlen($address);
	$allowedSpecialChars = ['@', '_', '-', '.'];
	for ($i = 0; $i < $addressLength; $i++) {
		$char = $address[$i];
		if (!ctype_alnum($char) && !in_array($char, $allowedSpecialChars)) {
			return false;
		}
	}
	return true;
}

/**
 * Sends the password SMS.
 *
 * @param string $pwd new password
 * @param array<string, string[]|string> $user LDAP attributes of user
 * @return array<string|string[]>[] list of arrays that can be used to create status messages
 */
function sendPasswordSms(string $pwd, array $user): array {
	$errors = [];
	$phoneNumber = getSmsPhoneNumber($user);
	include_once __DIR__ . '/plugins/sms/SmsService.inc';
	$smsService = new SmsService();
	try {
		$smsService->sendSms(sprintf(_('Your new password: %s'), $pwd), $phoneNumber);
	}
	catch (LAMException $e) {
		logNewMessage(LOG_ERR, 'SMS sending failed: ' . $e->getMessage());
		$errors[] = ['ERROR', _('Unable to send SMS.'), $e->getMessage()];
	}
	return $errors;
}

/**
 * Sends an SMS test message.
 *
 * @param string $providerId SMS provider ID
 * @param string|null $apiKey API key
 * @param string|null $apiToken API token
 * @param string|null $accountId account ID
 * @param string|null $region region
 * @param string|null $from from
 * @param string|null $subject subject text
 * @param string $number mobile phone number
 * @throws LAMException error during SMS connection
 */
function sendSmsTestMessage(string $providerId, ?string $apiKey, ?string $apiToken, ?string $accountId,
							?string $region, ?string $from, ?string $subject, string $number): void {
	include_once __DIR__ . '/plugins/sms/SmsService.inc';
	$smsService = new SmsService();
	try {
		$smsService->sendSms(_('LAM test message'), $number, $providerId, $apiKey, $apiToken, $accountId, $region, $from, $subject);
	}
	catch (Exception $e) {
		throw new LAMException(null, $e->getMessage(), $e);
	}
}

/**
 * Returns the SMS phone number of a user.
 *
 * @param array<string, string[]|string> $attributes LDAP attributes
 * @return string|null phone number if found
 */
function getSmsPhoneNumber(array $attributes): ?string {
	if (empty($_SESSION['cfgMain']->smsProvider)) {
		return null;
	}
	$phoneAttributes = array_change_key_case($_SESSION['cfgMain']->getSmsAttributes());
	$attributes = array_change_key_case($attributes);
	foreach ($phoneAttributes as $phoneAttribute) {
		if (!empty($attributes[$phoneAttribute][0])) {
			return $attributes[$phoneAttribute][0];
		}
	}
	return null;
}

/**
 * Caches module objects.
 * This improves performance if the same module does not need to be created multiple times (calling get_metaData() each time).
 *
 * @author Roland Gruber
 */
class moduleCache {

	/** @var array<string, object> module cache ("name:scope" => module) */
	private static array $cache = [];

	/**
	 * Returns a new/cached module with the given name and scope.
	 *
	 * @param string $name module name
	 * @param string|null $scope module scope (e.g. user)
	 * @return null|baseModule module object
	 */
	public static function getModule($name, $scope): ?object {
		if (!ScopeAndModuleValidation::isValidModuleName($name)
			|| (!empty($scope) && !ScopeAndModuleValidation::isValidScopeName($scope))) {
			return null;
		}
		if (!isset(self::$cache[$name . ':' . $scope])) {
			if (!class_exists($name)) {
				return null;
			}
			self::$cache[$name . ':' . $scope] = new $name($scope);
		}
		return self::$cache[$name . ':' . $scope];
	}

}

/**
 * Returns a random number.
 *
 * @return int random number
 */
function getRandomNumber(): int {
	return abs(hexdec(bin2hex(openssl_random_pseudo_bytes(6))));
}

/**
 * Connects to the LDAP server and extracts the certificates.
 *
 * @param String $server server name
 * @param String $port server port
 * @return mixed false on error and certificate if extracted successfully
 */
function getLDAPSSLCertificate($server, $port) {
	$stream = stream_context_create(["ssl" => ["capture_peer_cert_chain" => true, "verify_peer" => false, "allow_self_signed" => true]]);
	$client = @stream_socket_client('ssl://' . $server . ':' . $port, $errno, $errstr, 30, STREAM_CLIENT_CONNECT, $stream);
	if (!$client) {
		return false;
	}
	$context = stream_context_get_params($client);
	if (!isset($context['options']['ssl']['peer_certificate_chain'])) {
		return false;
	}
	$finalPEM = '';
	for ($i = 0; $i < count($context['options']['ssl']['peer_certificate_chain']); $i++) {
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
 * @param Connection $server LDAP server handle
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
 * @param Connection $server LDAP server handle
 * @return String error message
 */
function getDefaultLDAPErrorString($server) {
	$extError = htmlspecialchars(getExtendedLDAPErrorMessage($server));
	// Active Directory message translations
	if (str_contains($extError, 'DSID')) {
		if (str_contains($extError, '5003')) {
			logNewMessage(LOG_DEBUG, 'Password change failed because of ' . $extError);
			$extError = _('Your password does not meet the password strength qualifications. Please retry with another one.');
		}
		elseif (str_contains($extError, 'data 530,')) {
			logNewMessage(LOG_DEBUG, 'Login failed because of ' . $extError);
			$extError = _('Logon not permitted at this time');
		}
		elseif (str_contains($extError, 'data 532,')) {
			logNewMessage(LOG_DEBUG, 'Login failed because of ' . $extError);
			$extError = _('Password expired');
		}
		elseif (str_contains($extError, 'data 533,')) {
			logNewMessage(LOG_DEBUG, 'Login failed because of ' . $extError);
			$extError = _('Account is deactivated');
		}
		elseif (str_contains($extError, 'data 701,')) {
			logNewMessage(LOG_DEBUG, 'Login failed because of ' . $extError);
			$extError = _('Account is expired');
		}
		elseif (str_contains($extError, 'data 773,')) {
			logNewMessage(LOG_DEBUG, 'Login failed because of ' . $extError);
			$extError = _('Password change required');
		}
		elseif (str_contains($extError, 'data 775,')) {
			logNewMessage(LOG_DEBUG, 'Login failed because of ' . $extError);
			$extError = _('Account is locked');
		}
	}
	$genericErrorMessage = ldap_error($server);
	$message = _('LDAP error, server says:') . ' ' . $genericErrorMessage;
	if (!empty($extError) && ($genericErrorMessage !== $extError)) {
		$message .= ' - ' . $extError;
	}
	return $message;
}

/**
 * Returns if the last LDAP error was due to expired password or forced password change (AD only).
 *
 * @param Connection $server LDAP handle
 * @return bool password expired
 */
function ldapIsPasswordExpired($server): bool {
	$extError = htmlspecialchars(getExtendedLDAPErrorMessage($server));
	if (str_contains($extError, 'data 532,')) {
		// password expired
		return true;
	}
	elseif (str_contains($extError, 'data 773,')) {
		// password change required
		return true;
	}
	return false;
}

/**
 * Tries to get additional information why invalid credentials was returned. E.g. account is locked.
 *
 * @param LDAP\Connection $ldap LDAP object to connect for getting extra data
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
 * @param string $baseUrl base URL (e.g. http://www.example.com)
 * @return string|null URL
 */
function getCallingURL($baseUrl = ''): ?string {
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
 * Returns the offset in hours from the configured time zone to GMT.
 *
 * @return float offset
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
function getTimeZone(): DateTimeZone {
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
 * @param string $format format to use (e.g. 'Y-m-d H:i:s')
 * @return string formatted value
 */
function getFormattedTime(string $format): string {
	$time = new DateTime('now', getTimeZone());
	return $time->format($format);
}

/**
 * Formats a number of seconds to a more human readable format with minutes, hours, etc.
 * E.g. 70 seconds will return 1m10s.
 *
 * @param int|string $numSeconds number of seconds
 * @return string formatted number
 */
function formatSecondsToShortFormat($numSeconds) {
	if (($numSeconds === '0') || ($numSeconds === 0)) {
		return '0';
	}
	if (empty($numSeconds)) {
		return '';
	}
	if (!is_numeric($numSeconds)) {
		return $numSeconds;
	}
	$years = '';
	if ($numSeconds >= 31536000) {
		$years = floor($numSeconds / 31536000);
		$numSeconds -= $years * 31536000;
		$years .= 'y';
	}
	$seconds = $numSeconds % 60;
	$seconds = ($seconds === 0) ? '' : $seconds . 's';
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
 * @param string $text formatted text
 * @return int|string number of seconds
 */
function unformatShortFormatToSeconds($text) {
	if (empty($text)) {
		return $text;
	}
	$matches = [];
	if (preg_match('/^((\d+)y)?((\d+)w)?((\d+)d)?((\d+)h)?((\d+)m)?((\d+)s)?$/', $text, $matches)) {
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
 * Checks if the user is logged in. Stops script execution if not.
 *
 * @param bool $check2ndFactor check if the 2nd factor was provided if required
 */
function enforceUserIsLoggedIn($check2ndFactor = true): void {
	if ((!isset($_SESSION['loggedIn']) || ($_SESSION['loggedIn'] !== true)) && empty($_SESSION['selfService_clientPassword']) && empty($_SESSION['whitePages_clientPassword'])) {
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
function printHeaderContents($title, $prefix): void {
	echo '<meta name="viewport" content="width=device-width, initial-scale=1.0">';
	echo '<title>';
	echo $title;
	echo "</title>\n";
	// include CSS files
	$cssDirName = __DIR__ . '/../style';
	$cssDir = dir($cssDirName);
	$cssFiles = [];
	$cssEntry = $cssDir->read();
	while ($cssEntry !== false) {
		if (str_ends_with($cssEntry, '.css')) {
			$cssFiles[] = $cssEntry;
		}
		$cssEntry = $cssDir->read();
	}
	sort($cssFiles);
	foreach ($cssFiles as $cssEntry) {
		echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"$prefix/style/$cssEntry\">\n";
	}
	echo '<link rel="shortcut icon" type="image/x-icon" href="' . $prefix . '/graphics/favicon.ico">';
	echo '<link rel="icon" href="' . $prefix . '/graphics/logo136.png">';
}

/**
 * Prints script tags for all LAM JS files.
 *
 * @param string $prefix prefix to LAM main folder (e.g. "..")
 */
function printJsIncludes(string $prefix): void {
	$jsDirName = __DIR__ . '/../templates/lib';
	$jsDir = dir($jsDirName);
	$jsFiles = [];
	$jsEntry = $jsDir->read();
	while ($jsEntry !== false) {
		if ((str_ends_with($jsEntry, '.js')) || (str_ends_with($jsEntry, '.php'))) {
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
 * Converts a UTF-8 string to UTF16LE.
 *
 * @param string|null $input UTF-8 value
 * @return string|null converted value
 */
function convertUtf8ToUtf16Le(?string $input): ?string {
	if (($input === null) || ($input === '')) {
		return $input;
	}
	$output = iconv('UTF-8', 'UTF-16LE', $input);
	if ($output == '') {
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
 * @param string $version version
 * @return bool is developer version
 */
function isDeveloperVersion($version): bool {
	return str_contains($version, 'DEV');
}

/**
 * LAM exception with title and message.
 *
 * @author Roland Gruber
 */
class LAMException extends Exception {

	private ?string $title;

	private ?int $ldapErrorCode;

	/**
	 * Constructor.
	 *
	 * @param string|null $title title
	 * @param string $message message (optional)
	 * @param Exception|null $cause (optional)
	 * @param int|null $ldapErrorCode original LDAP error code
	 */
	public function __construct($title, $message = '', $cause = null, ?int $ldapErrorCode = null) {
		parent::__construct($message, 0, $cause);
		$this->title = $title;
		$this->ldapErrorCode = $ldapErrorCode;
	}

	/**
	 * Returns the message title.
	 *
	 * @return string|null title
	 */
	public function getTitle(): ?string {
		return $this->title;
	}

	/**
	 * Returns the original LDAP error code.
	 *
	 * @return int|null error code
	 */
	public function getLdapErrorCode() {
		return $this->ldapErrorCode;
	}

}

/**
 * Color calculations
 */
class LamColorUtil {

	/**
	 * Adjusts the brightness of the given hex color (e.g. #123456).
	 *
	 * @param string $color hex color (e.g. #123456)
	 * @param float $percentage percentage to darken (< 1 darkens, > 1 brightens)
	 * @return string darker color (e.g. #123456)
	 */
	public static function adjustBrightness(string $color, float $percentage): string {
		$color = self::parseColor($color);
		// darken
		$red = max(0, min(255, $color->red * $percentage));
		$green = max(0, min(255, $color->green * $percentage));
		$blue = max(0, min(255, $color->blue * $percentage));
		return sprintf("#%02x%02x%02x", $red, $green, $blue);
	}

	/**
	 * Returns the text color that should be used for the given background color.
	 *
	 * @param string $backgroundColor background color (e.g. #123456)
	 * @return string text color (e.g. #000000)
	 */
	public static function getTextColor(string $backgroundColor): string {
		$color = self::parseColor($backgroundColor);
		if (($color->red + $color->blue + $color->green) > (127 * 3)) {
			return '#000000';
		}
		return '#FFFFFF';
	}

	/**
	 * Parses a color value.
	 *
	 * @param string $color color
	 * @return LamColor color
	 */
	public static function parseColor(string $color): LamColor {
		$color = str_replace("#", '', $color);
		$red = hexdec(substr($color, 0, 2));
		$green = hexdec(substr($color, 2, 2));
		$blue = hexdec(substr($color, 4, 2));
		return new LamColor($red, $green, $blue);
	}

}

/**
 * Color value.
 */
class LamColor {

	public int $red;
	public int $green;
	public int $blue;

	/**
	 * @param int $red red
	 * @param int $green green
	 * @param int $blue blue
	 */
	public function __construct(int $red, int $green, int $blue) {
		$this->red = $red;
		$this->green = $green;
		$this->blue = $blue;
	}

}
