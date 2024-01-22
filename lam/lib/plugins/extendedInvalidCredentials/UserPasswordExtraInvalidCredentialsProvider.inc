<?php
namespace LAM\PLUGINS\EXTRA_INVALID_CREDENTIALS;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2021 - 2024  Roland Gruber

*/

/**
 * Extra message for invalid credentials for PPolicy.
 *
 * @author Roland Gruber
 */

class UserPasswordExtraInvalidCredentialsProvider implements ExtraInvalidCredentialsProvider {

	/**
	 * @inheritDoc
	 */
	public function getAttributeNamesToRead(): array {
		return ['userpassword'];
	}

	/**
	 * @inheritDoc
	 */
	public function getExtraMessage(array $attributes, $ldap) {
		if (!empty($attributes['userpassword'][0]) && !pwd_is_enabled($attributes['userpassword'][0])) {
			logNewMessage(LOG_NOTICE, 'Account is locked by userPassword attribute: ' . $attributes['dn']);
			return _('Account is locked');
		}
		return null;
	}

}
