<?

class account {
	var $general_username;
	var $general_uidNumber;
	var $general_surname;
	var $general_givenname;
	var $general_dn;
	var $general_group;
	var $general_groupadd;
	var $general_homedir;
	var $general_shell;
	var $general_gecos;
	var $general_memberUid;
	// Unix Password Settings
	var $unix_password;
	var $unix_pwdwarn;
	var $unix_pwdallowlogin;
	var $unix_pwdmaxage;
	var $unix_pwdminage;
	var $unix_pwdexpire_day;
	var $unix_pwdexpire_mon;
	var $unix_pwdexpire_yea;
	var $unix_deactivated;
	var $unix_shadowLastChange;
	// Samba Account
	var $smb_password;
	var $smb_useunixpwd;
	var $smb_pwdcanchange;
	var $smb_pwdmustchange;
	var $smb_homedrive;
	var $smb_scriptpath;
	var $smb_profilePath;
	var $smb_smbuserworkstations;
	var $smb_smbhome;
	var $smb_domain;
	var $smb_flagsW;
	var $smb_flagsD;
	var $smb_flagsX;
	}

function registervars() { // This function registers all needes session-varibales needed by account.php
	session_save_path('../sess');
	@session_start();
	if ( !session_is_registered("type2")) session_register("type2");
	if ( !session_is_registered("modify")) session_register("modify");
	if ( !session_is_registered("account")) session_register("account");
	if ( !session_is_registered("account_temp")) session_register("account_temp");
	if ( !session_is_registered("account_old")) session_register("account_old");
	if (!is_object($account)) $account = new account();
	if (!is_object($account_temp)) $account_temp = new account();
	if (!is_object($account_old)) $account = new account();
	}

function checkglobal() { // This functions checks all global account parameters
	// Check if username has been entered
	$error = "0";
	switch ( $_SESSION['type2'] ) {
		case 'user' :
			// Check if Username-length is OK. minLength=3, maxLength=20
			if ( !ereg('.{3,20}', $_SESSION['account_temp']->general_username))   $error = _('Username must content between 3 and 20 characters.');
			// Check if Username starts with letter
			if ( !ereg('^[a-z].*$', $_SESSION['account_temp']->general_username)) $error = _('Username contents invalid characters. First character must be a letter');
			// Check if Username contents only valid characters
			if ( !ereg('^([a-z]|[0-9]|[.]|[-]|[_])*$', $_SESSION['account_temp']->general_username)) $error = _('Username contents invalid characters. Valid characters are: a-z, 0-9 and .-_ !');
			// Check if user already exists
			$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_UserSuffix(), 'cn=' . $_SESSION['account_temp']->general_username);
			$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
			if ($entry) $dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
			if ($dn) {
				if ($_SESSION['modify']==1 && $_SESSION['account_temp']->general_username != $_SESSION['account_old']->general_username) $error =  _('User already exists!');
				if ($_SESSION['modify']==0) $error =  _('User already exists!');
				}
			// Check if surname is valid
			if ( !ereg('^([a-z]|[A-Z])*$', $_SESSION['account_temp']->surname)) $error = _('Surname contents invalid characters');
			// Check if givenname is valid
			if ( !ereg('^([a-z]|[A-Z])*$', $_SESSION['account_temp']->givenname)) $error = _('Givenname contents invalid characters');
			// Check if Homedir is valid
			$_SESSION['account_temp']->general_homedir = str_replace('$user', $_SESSION['account_temp']->general_username, $_SESSION['account_temp']->general_homedir);
			$_SESSION['account_temp']->general_homedir = str_replace('$group', $_SESSION['account_temp']->general_group, $_SESSION['account_temp']->general_homedir);
			if ( !ereg('^[/][a-z]([a-z]|[0-9]|[.]|[-]|[_])*([/][a-z]([a-z]|[0-9]|[.]|[-]|[_])*)*$', $_SESSION['account_temp']->general_homedir )) $error = _('Homedirectory contents invalid characters.');
			if ($_SESSION['account_temp']->general_gecos=='') $_SESSION['account_temp']->general_gecos = $_SESSION['account_temp']->general_givenname . " " . $_SESSION['account_temp']->general_surname ;
			// Check if UID is valid. If none value was entered, the next useable value will be inserted
			if (($_SESSION['account_temp']->general_uidNumber=='') && $_SESSION['modify']==0) $_SESSION['account_temp']->general_uidNumber = getfreeid('user');
				else {
					if (($_SESSION['account_temp']->general_uidNumber=='') && $_SESSION['modify'] == 1) $_SESSION['account_temp']->general_uidNumber = $_SESSION['account_old']->general_uidNumber ;
					$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_UserSuffix(), 'uidNumber=' . $_SESSION['account_temp']->general_uidNumber);
					$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
					if ($entry) $dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
					if ( $dn && $_SESSION['modify']==0) $error = _('UID is used from group' . $dn . ' !');
					if ( $_SESSION['account_temp']->general_uidNumber < $_SESSION['config']->get_minUID() || $_SESSION['account_temp']->general_uidNumber > $_SESSION['config']->get_maxUID()) $error = _('Please enter a value between '. $_SESSION['config']->get_minUID() . ' and ' . $_SESSION['config']->get_maxUID() . '!');
					if ( $dn && ($dn != $_SESSION['account_old']->general_dn) && $_SESSION['modify']==1) $error = _('UID is used from user ' . $dn . ' !');
				}
			break;
		case 'group' :
			// Check if Groupname-length is OK. minLength=3, maxLength=20
			if ( !ereg('.{3,20}', $_SESSION['account_temp']->general_username))   $error = _('Groupname must content between 3 and 20 characters.');
			// Check if Groupname starts with letter
			if ( !ereg('^[a-z].*$', $_SESSION['account_temp']->general_username)) $error = _('Groupname contents invalid characters. First character must be a letter');
			// Check if Groupname contents only valid characters
			if ( !ereg('^([a-z]|[0-9]|[.]|[-]|[_])*$', $_SESSION['account_temp']->general_username)) $error = _('Groupname contents invalid characters. Valid characters are: a-z, 0-9 and .-_ !');
			// Check if group already exists
			$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_GroupSuffix(), 'cn=' . $_SESSION['account_temp']->general_username);
			$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
			if ($entry) $dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
			if ($dn) {
				if ($_SESSION['modify']==1 && $_SESSION['account_temp']->general_username != $_SESSION['account_old']->general_username) $error =  _('Group already exists!');
				if ($_SESSION['modify']==0) $error =  _('Group already exists!');
				}

			// Check if GID is valid. If none value was entered, the next useable value will be inserted
			if (($_SESSION['account_temp']->general_uidNumber=='') && $_SESSION['modify']==0) $_SESSION['account_temp']->general_uidNumber = getfreeid('group');
				else {
					if (($_SESSION['account_temp']->general_uidNumber=='') && $_SESSION['modify'] == 1) $_SESSION['account_temp']->general_uidNumber = $_SESSION['account_old']->general_uidNumber ;
					$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_GroupSuffix(), 'gidNumber=' . $_SESSION['account_temp']->general_uidNumber);
					$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
					if ($entry) $dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
					if ( $dn && $_SESSION['modify']==0) $error = _('GID is used from group' . $dn . ' !');
					if ( $_SESSION['account_temp']->general_uidNumber < $_SESSION['config']->get_minGID() || $_SESSION['account_temp']->general_uidNumber > $_SESSION['config']->get_maxGID()) $error = _('Please enter a value between '. $_SESSION['config']->get_minGID() . ' and ' . $_SESSION['config']->get_maxGID() . '!');
					if ( $dn && ($dn != $_SESSION['account_old']->general_dn) && $_SESSION['modify']==1) $error = _('GID is used from group ' . $dn . ' !');
				}
			if ($_SESSION['account_temp']->general_gecos=='') $_SESSION['account_temp']->general_gecos = $_SESSION['account_temp']->general_username;
			break;
		case 'host' :
			if ( substr($_SESSION['account_temp']->general_username, strlen($_SESSION['account_temp']->general_username)-1, strlen($_SESSION['account_temp']->general_username)) != '$' ) $_SESSION['account_temp']->general_username = $_SESSION['account_temp']->general_username . '$';
			// Check if Hostname-length is OK. minLength=3, maxLength=20
			if ( !ereg('.{3,20}', $_SESSION['account_temp']->general_username))   $error = _('Hostname must content between 3 and 20 characters.');
			// Check if Hostname starts with letter
			if ( !ereg('^[a-z].*$', $_SESSION['account_temp']->general_username)) $error = _('Hostname contents invalid characters. First character must be a letter');
			// Check if Hostname contents only valid characters
			if ( !ereg('^([a-z]|[0-9]|[.]|[-]|[$])*$', $_SESSION['account_temp']->general_username)) $error = _('Hostname contents invalid characters. Valid characters are: a-z, 0-9 and .-_ !');
			// Check if Hostname already exists
			$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_HostSuffix(), 'cn=' . $_SESSION['account_temp']->general_username);
			$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
			if ($entry) $dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
			if ($dn) {
				if ($_SESSION['modify']==1 && $_SESSION['account_temp']->general_username != $_SESSION['account_old']->general_username) $error =  _('Host already exists!');
				if ($_SESSION['modify']==0) $error =  _('Host already exists!');
				}
			$_SESSION['account_temp']->general_homedir = '/dev/null';
			$_SESSION['account_temp']->general_shell = '/bin/false';
			// Check if UID is valid. If none value was entered, the next useable value will be inserted
			if (($_SESSION['account_temp']->general_uidNumber=='') && $_SESSION['modify']==0) $_SESSION['account_temp']->general_uidNumber = getfreeid('host');
				else {
					if (($_SESSION['account_temp']->general_uidNumber=='') && $_SESSION['modify'] == 1) $_SESSION['account_temp']->general_uidNumber = $_SESSION['account_old']->general_uidNumber ;
					$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_UserSuffix(), 'uidNumber=' . $_SESSION['account_temp']->general_uidNumber);
					$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
					if ($entry) $dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
					if ( $dn && $_SESSION['modify']==0) $error = _('UID is used from group' . $dn . ' !');
					if ( $_SESSION['account_temp']->general_uidNumber < $_SESSION['config']->get_minMachine() || $_SESSION['account_temp']->general_uidNumber > $_SESSION['config']->get_maxMachine()) $error = _('Please enter a value between '. $_SESSION['config']->get_minMaschine() . ' and ' . $_SESSION['config']->get_maxMachine() . '!');
					if ( $dn && ($dn != $_SESSION['account_old']->general_dn) && $_SESSION['modify']==1) $error = _('UID is used from user ' . $dn . ' !');
				}
			if ($_SESSION['account_temp']->general_gecos=='') $_SESSION['account_temp']->general_gecos = $_SESSION['account_temp']->general_username;

			break;
		}
	if ($_SESSION['account_temp']->general_username) $_SESSION['account']->general_username = $_SESSION['account_temp']->general_username;
	if ($_SESSION['account_temp']->general_surname) $_SESSION['account']->general_surname = $_SESSION['account_temp']->general_surname;
	if ($_SESSION['account_temp']->general_givenname) $_SESSION['account']->general_givenname = $_SESSION['account_temp']->general_givenname;
	if ($_SESSION['account_temp']->general_uidNumber) $_SESSION['account']->general_uidNumber = $_SESSION['account_temp']->general_uidNumber;
	if ($_SESSION['account_temp']->general_group) $_SESSION['account']->general_group = $_SESSION['account_temp']->general_group;
	if ($_SESSION['account_temp']->general_groupadd) $_SESSION['account']->general_groupadd = $_SESSION['account_temp']->general_groupadd;
	if ($_SESSION['account_temp']->general_homedir) $_SESSION['account']->general_homedir = $_SESSION['account_temp']->general_homedir;
	if ($_SESSION['account_temp']->general_shell) $_SESSION['account']->general_shell = $_SESSION['account_temp']->general_shell;
	if ($_SESSION['account_temp']->general_dn) $_SESSION['account']->general_dn = $_SESSION['account_temp']->general_dn;
	if ($_SESSION['account_temp']->general_gecos) $_SESSION['account']->general_gecos = $_SESSION['account_temp']->general_gecos;
	return $error;
	}


function checkunix() { // This function checks all unix account paramters
	$error = "0";
	switch ( $_SESSION['type2'] ) {
		case 'user' :
			// Sonderzeichen |#*,.;:_-+!$%&/|?{[()]} ****************************************************
			if ( !ereg('^([a-z]|[A-Z]|[0-9])*$', $_SESSION['account_temp']->unix_password)) $error = _('Password contents invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !');
			if ( !ereg('^([0-9]*)$', $_SESSION['account_temp']->unix_pwdwarn))  $error = _('Password Warn must be are natural number.');
			if ( !ereg('^(([-][1])|([0-9]*))$', $_SESSION['account_temp']->unix_pwdallowlogin))  $error = _('Password Expire must be are natural number or -1.');
			if ( !ereg('^([0-9]*)$', $_SESSION['account_temp']->unix_pwdmaxage))  $error = _('Password Maxage must be are natural number.');
			if ( !ereg('^([0-9]*)$', $_SESSION['account_temp']->unix_pwdminage))  $error = _('Password Minage must be are natural number.');
			if ( $_SESSION['account_temp']->unix_pwdminage > $_SESSION['account_temp']->unix_pwdmaxage ) $error = _('Password Maxage must bigger as Password Minage.');
			break;
		case 'host' :
			// Sonderzeichen |#*,.;:_-+!$%&/|?{[()]} ****************************************************
			if ( !ereg('^([a-z]|[A-Z]|[0-9])*$', $_SESSION['account_temp']->unix_password)) $error = _('Password contents invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !');
			if ( !ereg('^([0-9]*)$', $_SESSION['account_temp']->unix_pwdwarn))  $error = _('Password Warn must be are natural number.');
			if ( !ereg('^(([-][1])|([0-9]*))$', $_SESSION['account_temp']->unix_pwdallowlogin))  $error = _('Password Expire must be are natural number or -1.');
			if ( !ereg('^([0-9]*)$', $_SESSION['account_temp']->unix_pwdmaxage))  $error = _('Password Maxage must be are natural number.');
			if ( !ereg('^([0-9]*)$', $_SESSION['account_temp']->unix_pwdminage))  $error = _('Password Minage must be are natural number.');
			if ( $_SESSION['account_temp']->unix_pwdminage > $_SESSION['account_temp']->unix_pwdmaxage ) $error = _('Password Maxage must bigger as Password Minage.');
			break;
		}
	// Write Values from Webpage to Session-Variables
	if ($_SESSION['account_temp']->unix_password) $_SESSION['account']->unix_password = $_SESSION['account_temp']->unix_password;
	if ($_SESSION['account_temp']->unix_pwdwarn) $_SESSION['account']->unix_pwdwarn = $_SESSION['account_temp']->unix_pwdwarn;
	if ($_SESSION['account_temp']->unix_pwdallowlogin) $_SESSION['account']->unix_pwdallowlogin = $_SESSION['account_temp']->unix_pwdallowlogin;
	if ($_SESSION['account_temp']->unix_pwdmaxage) $_SESSION['account']->unix_pwdmaxage = $_SESSION['account_temp']->unix_pwdmaxage;
	if ($_SESSION['account_temp']->unix_pwdminage) $_SESSION['account']->unix_pwdminage = $_SESSION['account_temp']->unix_pwdminage;
	if ($_SESSION['account_temp']->unix_pwdexpire_day) $_SESSION['account']->unix_pwdexpire_day = $_SESSION['account_temp']->unix_pwdexpire_day;
	if ($_SESSION['account_temp']->unix_pwdexpire_mon) $_SESSION['account']->unix_pwdexpire_mon = $_SESSION['account_temp']->unix_pwdexpire_mon;
	if ($_SESSION['account_temp']->unix_pwdexpire_yea) $_SESSION['account']->unix_pwdexpire_yea = $_SESSION['account_temp']->unix_pwdexpire_yea;
	if ($_SESSION['account_temp']->unix_deactivated) $_SESSION['account']->unix_deactivated = 1; else $_SESSION['account']->unix_deactivated = 0;
	return $error;
	}

function checksamba() { // This function checks all samba account paramters
	$error = "0";
	if ($_SESSION['account_temp']->smb_useunixpwd) $_SESSION['account_temp']->smb_password = $_SESSION['account_temp']->unix_password;
	switch ( $_SESSION['type2'] ) {
		case 'user' :
			// Sonderzeichen |#*,.;:_-+!$%&/|?{[()]} ****************************************************
			if ( !ereg('^([a-z]|[A-Z]|[0-9])*$', $_SESSION['account_temp']->smb_password)) $error = _('Password contents invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !');
			if ( !ereg('^([/])*[a-z]([a-z]|[0-9]|[.]|[-]|[_])*([/][a-z]([a-z]|[0-9]|[.]|[-]|[_])*)*$', $_SESSION['account_temp']->smb_scriptpath)) $error = _('Scriptpath is invalid');
			if ( (!ereg('^[/][a-z]([a-z]|[0-9]|[.]|[-]|[_])*([/][a-z]([a-z]|[0-9]|[.]|[-]|[_])*)*$', $_SESSION['account_temp']->smb_profilePath)) && (!ereg('^[\][\]([a-z]|[A-Z]|[0-9]|[.]|[-])+([\]([a-z]|[A-Z]|[0-9]|[.]|[-])+)+$', $_SESSION['account_temp']->smb_profilePath))) $error = _('ProfilePath is invalid.');
			if ( !ereg('^[\][\]([a-z]|[A-Z]|[0-9]|[.]|[-])+([\]([a-z]|[A-Z]|[0-9]|[.]|[-])+)+$', $_SESSION['account_temp']->smb_smbhome)) $error = _('smbHome is invalid.');
			if ( ($_SESSION['account_temp']->smb_smbuserworkstations!='*') && (!ereg('^([a-z]|[A-Z]|[0-9]|[.]|[-])+(([ ])+([a-z]|[A-Z]|[0-9]|[.]|[-])+)*$', $_SESSION['account_temp']->smb_smbuserworkstations))) $error = _('User Workstations is invalid.');
			if ( !ereg('^([a-z]|[A-Z]|[0-9]|[-])+$', $_SESSION['account_temp']->smb_domain)) $error = _('Domain Name contents invalid characters. Valid characters are: a-z, A-Z, 0-9 and -.');
			$_SESSION['account_temp']->smb_flagsW = 0;
			break;
		case 'host' :
			// Sonderzeichen |#*,.;:_-+!$%&/|?{[()]} ****************************************************
			if ( !ereg('^([a-z]|[A-Z]|[0-9])*$', $_SESSION['account_temp']->smb_password)) $error = _('Password contents invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !');
			if ( !ereg('^([a-z]|[A-Z]|[0-9]|[-])+$', $_SESSION['account_temp']->smb_domain)) $error = _('Domain Name contents invalid characters. Valid characters are: a-z, A-Z, 0-9 and -.');
			$_SESSION['account_temp']->smb_flagsW = 1;
			break;
		}
	// Write Values from Webpage to Session-Variables
	if ($_SESSION['account_temp']->smb_password) $_SESSION['account']->smb_password = $_SESSION['account_temp']->smb_password;
	if ($_SESSION['account_temp']->smb_useunixpwd) $_SESSION['account']->smb_useunixpwd = 1; else $_SESSION['account']->smb_useunixpwd = 0;
	if ($_SESSION['account_temp']->smb_pwdcanchange) $_SESSION['account']->smb_pwdcanchange = 1; else $_SESSION['account']->smb_pwdcanchange = 0;
	if ($_SESSION['account_temp']->smb_pwdmustchange) $_SESSION['account']->smb_pwdmustchange = 1; else $_SESSION['account']->smb_pwdmustchange = 0;
	if ($_SESSION['account_temp']->smb_homedrive) $_SESSION['account']->smb_homedrive = $_SESSION['account_temp']->smb_homedrive;
	if ($_SESSION['account_temp']->smb_profilePath) $_SESSION['account']->smb_profilePath = $_SESSION['account_temp']->smb_profilePath;
	if ($_SESSION['account_temp']->smb_scriptpath) $_SESSION['account']->smb_scriptpath = $_SESSION['account_temp']->smb_scriptpath;
	if ($_SESSION['account_temp']->smb_smbuserworkstations) $_SESSION['account']->smb_smbuserworkstations = $_SESSION['account_temp']->smb_smbuserworkstations;
	if ($_SESSION['account_temp']->smb_smbhome) $_SESSION['account']->smb_smbhome = $_SESSION['account_temp']->smb_smbhome;
	if ($_SESSION['account_temp']->smb_domain) $_SESSION['account']->smb_domain = $_SESSION['account_temp']->smb_domain;
	if ($_SESSION['account_temp']->smb_flagsW) $_SESSION['account']->smb_flagsW = 1; else $_SESSION['account']->smb_flagsW = 0;
	if ($_SESSION['account_temp']->smb_flagsD) $_SESSION['account']->smb_flagsD = 1; else $_SESSION['account']->smb_flagsD = 0;
	if ($_SESSION['account_temp']->smb_flagsX) $_SESSION['account']->smb_flagsX = 1; else $_SESSION['account']->smb_flagsX = 0;
	return $error;
	}

function genpasswd() { // This function will return a password with max. 8 characters
	// Allowed Characters to generate passwords
	$LCase = 'abcdefghjkmnpqrstuvwxyz';
	$UCase = 'ABCDEFGHJKLMNPQRSTUVWXYZ';
	$Integer = '23456789';
	// DEFINE CONSTANTS FOR ALGORTTHM
	define("LEN", '1');
 	/* THIS FUNCTION GENERATES A RANDOM NUMBER THAT WILL BE USED TO
 	* RANDOMLY SELECT CHARACTERS FROM THE STRINGS ABOVE
 	*/
	function RndInt($Format){
		switch ($Format){
			case 'letter':
				$Rnd = rand(0,25);
				if ($Rnd > 25){
					$Rnd = $Rnd - 1;
					}
				break;
			case 'number':
				$Rnd = rand(0,9);
				if ($Rnd > 9){
					$Rnd = $Rnd - 1;
					}
				break;
			}
		return $Rnd;
		} // END RndInt() FUNCTION

 	/* RUN THE FUNCTION TO GENERATE RANDOM INTEGERS FOR EACH OF THE
	* 8 CHARACTERS IN THE PASSWORD PRODUCED.
	*/
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

function findgroups() { // Will return an array with all Groupnames found in LDAP
	$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_GroupSuffix(), 'ObjectClass=PosixGroup');
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	while ($entry) {
		$group[] = strtok(ldap_dn2ufn(ldap_get_dn($_SESSION['ldap']->server(), $entry)),',');
		$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
		}
	return $group;
	}


function getgid($groupname) { // Will return the the gid to an existing Groupname
	// Check if group already exists
	$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_GroupSuffix(), 'cn=' . $groupname);
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
	return $attr['gidNumber'][0];
	}

function getfreeid($scope) { // Will return an unused id from all ids found in LDAP
	switch ($scope) {
		case 'user':
			$ObjectClass = 'PosixAccount';
			$search = 'uidNumber';
			$minID = $_SESSION['config']->get_minUID();
			$maxID = $_SESSION['config']->get_maxUID();
			$suffix = $_SESSION['config']->get_UserSuffix();
			break;
		case 'group':
			$ObjectClass = 'PosixGroup';
			$search = 'gidNumber';
			$minID = $_SESSION['config']->get_MinGID();
			$maxID = $_SESSION['config']->get_MaxGID();
			$suffix = $_SESSION['config']->get_GroupSuffix();
			break;
		case 'host':
			$ObjectClass = 'PosixAccount';
			$search = 'uidNumber';
			$minID = $_SESSION['config']->get_MinMachine();
			$maxID = $_SESSION['config']->get_MaxMachine();
			$suffix = $_SESSION['config']->get_HostSuffix();
			break;
		}
	$result = ldap_search($_SESSION['ldap']->server(), $suffix, 'ObjectClass='.$ObjectClass);
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	while ($entry) {
		$vals = ldap_get_values($_SESSION['ldap']->server(), $entry, $search);
		$ids[] = $vals[0];
		$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
		}
	sort ($ids, SORT_NUMERIC);
	if ($ids[count($ids)-1] < $maxID) {
		if ($minID > $ids[count($ids)-1]) return $minID;
		else return $ids[count($ids)-1]+1;
		}
	else {
		$i=$minID;
		foreach ($ids as $id) if ($id == $i) $i++;
		return $i;
		}
	}

function getdays() { // will return the days from 1.1.1970 until now
	$days = time() / 86400;
	settype($days, 'integer');
	return $days;
	}

function smbflag() { // Creates te attribute attrFlags
	$flag = "[";
	if ($_SESSION['account']->smb_flagsW) $flag = $flag . "W"; else $flag = $flag . "U";
	if ($_SESSION['account']->smb_flagsD) $flag = $flag . "D";
	// ****************** Fixme What to do eith the X-Flag?
	$flag = $flag. "]";
	return $flag;
	}

function loadaccount($dn) { // Will load all needed values from an existing account
	$result = ldap_search($_SESSION['ldap']->server(), $dn, "objectclass=*");
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
	if ($attr['uid'][0]) $_SESSION['account']->general_username = $attr['uid'][0];
	}

function loadgroup($dn) { // Will load all needed values from an existing group
	$result = ldap_search($_SESSION['ldap']->server(), $dn, "objectclass=*");
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
	if ($attr['gidNumber'][0]) $_SESSION['account']->general_uidNumber = $attr['gidNumber'][0];
	if ($attr['description'][0]) $_SESSION['account']->general_gecos = $attr['description'][0];
	if ($attr['cn'][0]) $_SESSION['account']->general_username = $attr['cn'][0];
	if ($attr['description'][0]) $_SESSION['account']->general_gecos = $attr['description'][0];
	if ($attr['memberUid']) $_SESSION['account']->general_memberUid = $attr['memberUid'];
	array_shift($_SESSION['account']->general_memberUid);
	$_SESSION['account']->general_dn = $dn;
	if ($attr['gidNumber'][0]) $_SESSION['account_old']->general_uidNumber = $attr['gidNumber'][0];
	if ($attr['description'][0]) $_SESSION['account_old']->general_gecos = $attr['description'][0];
	if ($attr['cn'][0]) $_SESSION['account_old']->general_username = $attr['cn'][0];
	if ($attr['description'][0]) $_SESSION['account_old']->general_gecos = $attr['description'][0];
	if ($attr['memberUid']) $_SESSION['account']->general_memberUid = $attr['memberUid'];
	array_shift($_SESSION['account']->general_memberUid);
	$_SESSION['account_old']->general_dn = $dn;
	}


function createaccount() { // Will create the LDAP-Account
	// 2 == Account allready exists at different location
	// 1 == Account has been created
	// 3 == Account has been modified
	// 4 == Error while creating Account
	// 5 == Error while modifying Account

	// Value stored in shadowExpire, days since 1.1.1970
	$date = mktime(0,0,0, $_SESSION['account']->unix_pwdexpire_day, $_SESSION['account']->unix_pwdexpire_mon, $_SESSION['account']->unix_pwdexpire_yea) / 86400 ;
	settype($date, 'integer');
	$_SESSION['account']->general_dn = 'cn=' . $_SESSION['account']->general_username . ',' . $_SESSION['config']->get_UserSuffix();

	// All Values need for an user-account
	// General Objectclasses
	$attr['objectClass'][0] = 'inetOrgPerson';
	$attr['objectClass'][1] = 'posixAccount';
	$attr['objectClass'][2] = 'shadowAccount';
	$attr['objectClass'][3] = 'sambaAccount';
	$attr['cn'] = $_SESSION['account']->general_username; // posixAccount_req shadowAccount_req sambaAccount_may
	$attr['uid'] = $_SESSION['account']->general_username; // posixAccount_req
	$attr['uidNumber'] = $_SESSION['account']->general_uidNumber; // posixAccount_req
	$attr['gidNumber'] = getgid($_SESSION['account']->general_group); // posixAccount_req
	$attr['homeDirectory'] = $_SESSION['account']->general_homedir; // posixAccount_req
	// posixAccount_may shadowAccount_may
	if ($_SESSION['modify']==1) {
		$password_old = str_replace('{CRYPT}', '',$_SESSION['account_old']->unix_password);
		if (substr($password_old,0,1) == '!' ) $password_old = substr($password_old,1,strlen($password_old));
		if (!$_SESSION['account']->unix_password) {
			if ($_SESSION['account']->unix_deactivated) $attr['userPassword'] = '{CRYPT}!' . $password_old;
			else $attr['userPassword'] = '{CRYPT}' . $password_old;
			$attr['shadowLastChange'] =  $_SESSION['account_old']->unix_shadowLastChange; // shadowAccunt_may
			}
		 else {
			if ($_SESSION['account']->unix_deactivated) $attr['userPassword'] = '{CRYPT}!' . crypt($_SESSION['account']->unix_password);
			else $attr['userPassword'] = '{CRYPT}' . crypt($_SESSION['account']->unix_password);
			$attr['shadowLastChange'] = getdays(); // shadowAccunt_may
			}
		}
	 else {
		if ($_SESSION['account']->unix_deactivated) $attr['userPassword'] = '{CRYPT}!' . crypt($_SESSION['account']->unix_password);
		else $attr['userPassword'] = '{CRYPT}' . crypt($_SESSION['account']->unix_password);
		$attr['shadowLastChange'] = getdays(); // shadowAccunt_may
		}
	$attr['loginShell'] = $_SESSION['account']->general_shell; // posixAccount_may
	$attr['gecos'] = $_SESSION['account']->general_gecos; // posixAccount_may
	$attr['description'] = $_SESSION['account']->general_gecos; // posixAccount_may sambaAccount_may

	$attr['shadowMin'] = $_SESSION['account']->unix_pwdminage; // shadowAccunt_may
	$attr['shadowMax'] = $_SESSION['account']->unix_pwdmaxage; // shadowAccunt_may
	$attr['shadowWarning'] = $_SESSION['account']->unix_pwdwarn; // shadowAccunt_may
	$attr['shadowInactive'] = $_SESSION['account']->unix_pwdallowlogin; // shadowAccunt_may
	$attr['shadowExpire'] =  $date ; // shadowAccunt_may
	$attr['rid'] = (2 * $_SESSION['account']->general_uidNumber + 1000); // sambaAccount_may
	$attr['PrimaryGroupID'] = (2 * getgid($_SESSION['account']->general_group) + 1001); // sambaAccount_req
	// Samba-Passwort?
	//$attr['lmPassword'] = ""; // sambaAccount_may
	//$attr['ntPassword'] = ""; // sambaAccount_may
	$attr['pwdLastSet'] = time(); // sambaAccount_may *************************************** only change if password has been changed
	// $attr['logonTime'] = ""; // sambaAccount_may
	// $attr['logoffTime'] = ""; // sambaAccount_may
	// $attr['kickoffTime'] = ""; // sambaAccount_may
	if ($_SESSION['account']->smb_pwdcanchange) $attr['pwdCanChange'] = "1"; else $attr['pwdCanChange'] = "0"; // sambaAccount_may
	if ($_SESSION['account']->smb_pwdmustchange) $attr['pwdMustChange'] = "1"; else $attr['pwdMustChange'] = "0"; // sambaAccount_may
	$attr['acctFlags'] = smbflag(); // sambaAccount_may
	$attr['displayName'] = $_SESSION['account']->general_gecos; // sambaAccount_may
	$attr['smbHome'] = $_SESSION['account']->smb_smbhome; // sambaAccount_may
	$attr['homeDrive'] = $_SESSION['account']->smb_homedrive; // sambaAccount_may
	$attr['scriptPath']  = $_SESSION['account']->smb_scriptpath; // sambaAccount_may
	$attr['profilePath'] = $_SESSION['account']->smb_profilePath; // sambaAccount_may
	$attr['userWorkstations'] = $_SESSION['account']->smb_smbuserworkstations; // sambaAccount_may
	$attr['domain'] = $_SESSION['account']->smb_domain; // sambaAccount_may

	$attr['gn'] = $_SESSION['account']->general_givenname;
	$attr['sn'] = $_SESSION['account']->general_surname;

	if ( $_SESSION['modify'] == 1 ) {
		echo "Modify User";
		//$success = ldap_modify($_SESSION['ldap']->server(),$_SESSION['account']->general_dn, $attr);
		//if ($success) return 3;
		//else return 5;
		return 5;
		}
	 else {
		// Write a new entry if user doesn't exists
		$success = ldap_add($_SESSION['ldap']->server(),$_SESSION['account']->general_dn, $attr);
		if (!$success) return 4;
		// Add user to groups
		$result = ldap_search($_SESSION['ldap']->server(), 'cn='.$_SESSION['account']->general_group.','.$_SESSION['config']->get_GroupSuffix(), "objectclass=posixGroup");
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		$group = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
		$attr_group['objectClass'] = 'posixGroup';
		$attr_group['cn'] = $group['cn'][0];
		$attr_group['gidNumber'] = $group['gidNumber'][0];
		$attr_group['description'] = $group['description'][0];
		if ($group['memberUid']) foreach ($group['memberUid'] as $group_int) $attr_group['memberUid'][] = $group_int;
		array_shift($attr_group['memberUid']);
		if (! in_array($_SESSION['account']->general_username, $attr_group['memberUid'])) $attr_group['memberUid'][] = $_SESSION['account']->general_username;
		$success = ldap_mod_replace($_SESSION['ldap']->server(), 'cn='.$_SESSION['account']->general_group.','.$_SESSION['config']->get_GroupSuffix(), $attr_group);
		if (!$success) return 4;
		// Add User to Additional Groups
		if ($_SESSION['account']->general_groupadd)
			foreach ($_SESSION['account']->general_groupadd as $group2) {
				$attr_group = "";
				$result = ldap_search($_SESSION['ldap']->server(), 'cn='.$group2.','.$_SESSION['config']->get_GroupSuffix(), "objectclass=PosixGroup");
				$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
				$group = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
				$attr_group['objectClass'] = 'posixGroup';
				$attr_group['cn'] = $group['cn'][0];
				$attr_group['gidNumber'] = $group['gidNumber'][0];
				$attr_group['description'] = $group['description'][0];
				if ($group['memberUid']) foreach ($group['memberUid'] as $group_int) $attr_group['memberUid'][] = $group_int;
				array_shift($attr_group['memberUid']);
				if (! in_array($_SESSION['account']->general_username, $attr_group['memberUid'])) $attr_group['memberUid'][] = $_SESSION['account']->general_username;
				$success = ldap_modify($_SESSION['ldap']->server(), 'cn='.$group2.','.$_SESSION['config']->get_GroupSuffix(), $attr_group);
				if (!$success) return 4;
			}
		return 1;
		}
	}

function createhost() { // Will create the LDAP-Host
	// 2 == Host allready exists at different location
	// 1 == Host has been created
	// 3 == Host has been modified
	// 4 == Error while creating Host
	// 5 == Error while modifying Host

	// Value stored in shadowExpire, days since 1.1.1970
	$date = mktime(0,0,0, $_SESSION['account']->unix_pwdexpire_day, $_SESSION['account']->unix_pwdexpire_mon, $_SESSION['account']->unix_pwdexpire_yea) / 86400 ;
	settype($date, 'integer');
	$_SESSION['account']->general_dn = 'cn=' . $_SESSION['account']->general_username . ',' . $_SESSION['config']->get_HostSuffix();

	// All Values need for an user-account
	// General Objectclasses
	$attr['objectClass'][0] = 'inetOrgPerson';
	$attr['objectClass'][1] = 'posixAccount';
	$attr['objectClass'][2] = 'shadowAccount';
	$attr['objectClass'][3] = 'sambaAccount';
	$attr['cn'] = $_SESSION['account']->general_username; // posixAccount_req shadowAccount_req sambaAccount_may
	$attr['uid'] = $_SESSION['account']->general_username; // posixAccount_req
	$attr['uidNumber'] = $_SESSION['account']->general_uidNumber; // posixAccount_req
	$attr['gidNumber'] = getgid($_SESSION['account']->general_group); // posixAccount_req
	$attr['homeDirectory'] = $_SESSION['account']->general_homedir; // posixAccount_req
	// posixAccount_may shadowAccount_may
	if ($_SESSION['modify']==1) {
		$password_old = str_replace('{CRYPT}', '',$_SESSION['account_old']->unix_password);
		if (substr($password_old,0,1) == '!' ) $password_old = substr($password_old,1,strlen($password_old));
		if (!$_SESSION['account']->unix_password) {
			if ($_SESSION['account']->unix_deactivated) $attr['userPassword'] = '{CRYPT}!' . $password_old;
			else $attr['userPassword'] = '{CRYPT}' . $password_old;
			$attr['shadowLastChange'] =  $_SESSION['account_old']->unix_shadowLastChange; // shadowAccunt_may
			}
		 else {
			if ($_SESSION['account']->unix_deactivated) $attr['userPassword'] = '{CRYPT}!' . crypt($_SESSION['account']->unix_password);
			else $attr['userPassword'] = '{CRYPT}' . crypt($_SESSION['account']->unix_password);
			$attr['shadowLastChange'] = getdays(); // shadowAccunt_may
			}
		}
	 else {
		if ($_SESSION['account']->unix_deactivated) $attr['userPassword'] = '{CRYPT}!' . crypt($_SESSION['account']->unix_password);
		else $attr['userPassword'] = '{CRYPT}' . crypt($_SESSION['account']->unix_password);
		$attr['shadowLastChange'] = getdays(); // shadowAccunt_may
		}
	$attr['loginShell'] = $_SESSION['account']->general_shell; // posixAccount_may
	$attr['gecos'] = $_SESSION['account']->general_gecos; // posixAccount_may
	$attr['description'] = $_SESSION['account']->general_gecos; // posixAccount_may sambaAccount_may

	$attr['shadowMin'] = $_SESSION['account']->unix_pwdminage; // shadowAccunt_may
	$attr['shadowMax'] = $_SESSION['account']->unix_pwdmaxage; // shadowAccunt_may
	$attr['shadowWarning'] = $_SESSION['account']->unix_pwdwarn; // shadowAccunt_may
	$attr['shadowInactive'] = $_SESSION['account']->unix_pwdallowlogin; // shadowAccunt_may
	$attr['shadowExpire'] =  $date ; // shadowAccunt_may
	$attr['rid'] = (2 * $_SESSION['account']->general_uidNumber + 1000); // sambaAccount_may
	$attr['PrimaryGroupID'] = (2 * getgid($_SESSION['account']->general_group) + 1001); // sambaAccount_req
	// Samba-Passwort?
	//$attr['lmPassword'] = ""; // sambaAccount_may
	//$attr['ntPassword'] = ""; // sambaAccount_may
	$attr['pwdLastSet'] = time(); // sambaAccount_may *************************************** only change if password has been changed
	// $attr['logonTime'] = ""; // sambaAccount_may
	// $attr['logoffTime'] = ""; // sambaAccount_may
	// $attr['kickoffTime'] = ""; // sambaAccount_may
	if ($_SESSION['account']->smb_pwdcanchange) $attr['pwdCanChange'] = "1"; else $attr['pwdCanChange'] = "0"; // sambaAccount_may
	if ($_SESSION['account']->smb_pwdmustchange) $attr['pwdMustChange'] = "1"; else $attr['pwdMustChange'] = "0"; // sambaAccount_may
	$attr['acctFlags'] = smbflag(); // sambaAccount_may
	$attr['displayName'] = $_SESSION['account']->general_gecos; // sambaAccount_may
	$attr['domain'] = $_SESSION['account']->smb_domain; // sambaAccount_may

	$attr['gn'] = $_SESSION['account']->general_username;
	$attr['sn'] = $_SESSION['account']->general_username;

	if ( $_SESSION['modify'] == 1 ) {
		echo "Modify User";
		//$success = ldap_modify($_SESSION['ldap']->server(),$_SESSION['account']->general_dn, $attr);
		//if ($success) return 3;
		//else return 5;
		return 5;
		}
	 else {
		// Write a new entry if user doesn't exists
		$success = ldap_add($_SESSION['ldap']->server(),$_SESSION['account']->general_dn, $attr);
		if (!$success) return 4;
		// Add Host to groups
		$result = ldap_search($_SESSION['ldap']->server(), 'cn='.$_SESSION['account']->general_group.','.$_SESSION['config']->get_GroupSuffix(), "objectclass=posixGroup");
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		$group = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
		$attr_group['objectClass'] = 'posixGroup';
		$attr_group['cn'] = $group['cn'][0];
		$attr_group['gidNumber'] = $group['gidNumber'][0];
		$attr_group['description'] = $group['description'][0];
		if ($group['memberUid']) foreach ($group['memberUid'] as $group_int) $attr_group['memberUid'][] = $group_int;
		array_shift($attr_group['memberUid']);
		if (! in_array($_SESSION['account']->general_username, $attr_group['memberUid'])) $attr_group['memberUid'][] = $_SESSION['account']->general_username;
		$success = ldap_mod_replace($_SESSION['ldap']->server(), 'cn='.$_SESSION['account']->general_group.','.$_SESSION['config']->get_GroupSuffix(), $attr_group);
		if (!$success) return 4;
		// Add Host to Additional Groups
		if ($_SESSION['account']->general_groupadd)
			foreach ($_SESSION['account']->general_groupadd as $group2) {
				$attr_group = "";
				$result = ldap_search($_SESSION['ldap']->server(), 'cn='.$group2.','.$_SESSION['config']->get_GroupSuffix(), "objectclass=PosixGroup");
				$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
				$group = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
				$attr_group['objectClass'] = 'posixGroup';
				$attr_group['cn'] = $group['cn'][0];
				$attr_group['gidNumber'] = $group['gidNumber'][0];
				$attr_group['description'] = $group['description'][0];
				if ($group['memberUid']) foreach ($group['memberUid'] as $group_int) $attr_group['memberUid'][] = $group_int;
				array_shift($attr_group['memberUid']);
				if (! in_array($_SESSION['account']->general_username, $attr_group['memberUid'])) $attr_group['memberUid'][] = $_SESSION['account']->general_username;
				$success = ldap_modify($_SESSION['ldap']->server(), 'cn='.$group2.','.$_SESSION['config']->get_GroupSuffix(), $attr_group);
				if (!$success) return 4;
			}
		return 1;
		}
	}


function creategroup() { // Will create the LDAP-Group
	// 2 == Group allready exists at different location
	// 1 == Group has been created
	// 3 == Group has been modified
	// 4 == Error while creating Group
	// 5 == Error while modifying Group
	$_SESSION['account']->general_dn = 'cn=' . $_SESSION['account']->general_username . ',' . $_SESSION['config']->get_GroupSuffix();
	$attr['objectClass'] = 'posixGroup';
	$attr['cn'] = $_SESSION['account']->general_username;
	$attr['gidNumber'] = $_SESSION['account']->general_uidNumber;
	$attr['description'] = $_SESSION['account']->general_gecos;
	if ($_SESSION['account']->memeberUid) $attr['memberUid'] = $_SESSION['account']->memberUid;
	if ( $_SESSION['modify']==0 ) { // Write a new entry if group doesn't exists
		$success = ldap_add($_SESSION['ldap']->server(),$_SESSION['account']->general_dn, $attr);
		if ($success) return 1;
		else return 4;
		}
	 else { // Modify an Existing entry
		if ($_SESSION['account']->general_username == $_SESSION['account_old']->general_username) // Groupname hasn't changed
			$success = ldap_modify($_SESSION['ldap']->server(),$_SESSION['account']->general_dn, $attr);
			else {
				ldap_delete($_SESSION['ldap']->server(),$_SESSION['account_old']->general_dn);
				$success = ldap_add($_SESSION['ldap']->server(),$_SESSION['account']->general_dn, $attr);
				}
		if ($_SESSION['account']->uidNumber != $_SESSION['account_old']->uidNumber) {
		// Fragen, ob bei geänderter gid die gids der Beutzer in der Gruppe geändert werden sollen.
		echo ('find / -gid ' . $_SESSION['account_old' ]->general_uidNumber . ' -exec chgrp ' . $_SESSION['account']->general_uidNumber . ' {} \;');

			}
		if ($success) return 3;
		else return 5;
		}
	}

?>
