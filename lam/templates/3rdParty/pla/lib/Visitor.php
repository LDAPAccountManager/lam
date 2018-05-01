<?php
/**
 * Classes and functions for the template engines.
 *
 * @author The phpLDAPadmin development team
 * @package phpLDAPadmin
 */

/**
 * Abstract Visitor class
 *
 * @package phpLDAPadmin
 * @subpackage Templates
 */
abstract class Visitor {
	# The server that was used to configure the templates
	protected $server_id;

	public function __call($method,$args) {
		if (! in_array($method,array('get','visit','draw')))
			debug_dump_backtrace(sprintf('Incorrect use of method loading [%s]',$method),1);

		$methods = array();

		$fnct = array_shift($args);

		$object = $args[0];
		$class = get_class($object);

		$call = "$method$fnct$class";

		array_push($methods,$call);

		while ($class && ! method_exists($this,$call)) {
			$class = get_parent_class($class);
			$call = "$method$fnct$class";
			array_push($methods,$call);
		}

		if (method_exists($this,$call)) {
			$r = call_user_func_array(array($this, $call), $args);

			if (isset($r))
				return $r;
			else
				return;

		}

		printf('<font size=-2><i>NO Methods: %s</i></font><br />',implode('|',$methods));
	}

	/**
	 * Return the LDAP server ID
	 *
	 * @return int Server ID
	 */
	public function getServerID() {
		if (isset($this->server_id))
			return $this->server_id;
		else
			return null;
	}

	/**
	 * Return this LDAP Server object
	 *
	 * @return object DataStore Server
	 */
	protected function getServer() {
		return $_SESSION[APPCONFIG]->getServer($this->getServerID());
	}
}
?>
