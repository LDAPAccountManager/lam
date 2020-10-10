<?php
namespace LAM\PLUGINS\EXTRA_INVALID_CREDENTIALS;

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
 * Extra message for invalid credentials for PPolicy.
 *
 * @author Roland Gruber
 */

class PPolicyExtraInvalidCredentialsProvider implements ExtraInvalidCredentialsProvider {

	/**
	 * @inheritDoc
	 */
	public function getAttributeNamesToRead(): array {
		return array('pwdaccountlockedtime');
	}

	/**
	 * @inheritDoc
	 */
	public function getExtraMessage(array $attributes) {
		if (!empty($attributes['pwdaccountlockedtime'][0])) {
			return _('Account is locked');
		}
		return null;
	}

}
