<?php
namespace LAM\PLUGINS\EXTRA_INVALID_CREDENTIALS;

use DateTime;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2020  Roland Gruber

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
 * Extra message for invalid credentials for MIT Kerberos.
 *
 * @author Roland Gruber
 */

class MitKerberosExtraInvalidCredentialsProvider implements ExtraInvalidCredentialsProvider {

	/**
	 * @inheritDoc
	 */
	public function getAttributeNamesToRead(): array {
		return array('krbprincipalexpiration', 'krbpasswordexpiration');
	}

	/**
	 * @inheritDoc
	 */
	public function getExtraMessage(array $attributes) {
		$now = new DateTime('now', getTimeZone());
		if (!empty($attributes['krbprincipalexpiration'][0])) {
			$kerberosExpirationDate = parseLDAPTimestamp($attributes['krbprincipalexpiration'][0]);
			if ($now >= $kerberosExpirationDate) {
				return _('Kerberos account is expired');
			}
		}
		if (!empty($attributes['krbpasswordexpiration'][0])) {
			$kerberosExpirationDate = parseLDAPTimestamp($attributes['krbpasswordexpiration'][0]);
			if ($now >= $kerberosExpirationDate) {
				return _('Kerberos password is expired');
			}
		}
		return null;
	}

}
