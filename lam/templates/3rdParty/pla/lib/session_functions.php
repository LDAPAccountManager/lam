<?php
/**
 * A collection of functions to handle sessions.
 *
 * @author The phpLDAPadmin development team
 * @package phpLDAPadmin
 * @subpackage Session
 */

include_once '../../../../lib/security.inc';

/**
 * The only function which should be called by a user
 *
 * @see common.php
 * @see APP_SESSION_ID
 * @return boolean Returns true if the session was started the first time
 */
function app_session_start() {
	if (session_id() != null) return;
	include_once '../../../../lib/config.inc';
	include_once '../../../../lib/ldap.inc';
	startSecureSession();
	$config_file = CONFDIR.'config.php';
	$config = check_config($config_file);
	# If we came via index.php, then set our $config.
	if (! isset($_SESSION[APPCONFIG]) && isset($config))
		$_SESSION[APPCONFIG] = $config;
}

?>
