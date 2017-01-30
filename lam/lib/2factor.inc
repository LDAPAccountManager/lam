<?php
namespace LAM\LIB\TWO_FACTOR;
/*
  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2017  Roland Gruber

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
* 2-factor authentication
*
* @package two_factor
* @author Roland Gruber
*/

interface TwoFactorProvider {

	/**
	 * Verifies if the provided 2nd factor is valid.
	 *
	 * @param \selfServiceProfile $selfServiceProfile self service profile
	 * @param string $user user name
	 * @param string $password password
	 * @param string $twoFactorInput input for 2nd factor
	 * @throws \Exception error during check
	 */
	public function verify2ndFactor($selfServiceProfile, $user, $password, $twoFactorInput);

}

class PrivacyIDEAProvider implements TwoFactorProvider {

	/**
	 * {@inheritDoc}
	 * @see \LAM\LIB\TWO_FACTOR\TwoFactorProvider::verify2ndFactor()
	 */
	public function verify2ndFactor($selfServiceProfile, $user, $password, $twoFactorInput) {
		logNewMessage(LOG_DEBUG, 'PrivacyIDEAProvider: Checking 2nd factor for ' . $user);
		$token = $this->authenticate($selfServiceProfile, $user, $password);
	}

	/**
	 * Authenticates against the server
	 *
	 * @param unknown $selfServiceProfile
	 * @param unknown $user
	 * @param unknown $password
	 * @return string token
	 * @throws \Exception error during authentication
	 */
	private function authenticate($selfServiceProfile, $user, $password) {
		$curl = $this->getCurl($selfServiceProfile);
	}

	/**
	 * Returns the curl object.
	 *
	 * @param unknown $selfServiceProfile
	 * @return object curl handle
	 * @throws \Exception error during curl creation
	 */
	private function getCurl($selfServiceProfile) {
		$curl = curl_init();
		return $curl;
	}

}
