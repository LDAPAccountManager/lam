<?php
namespace LAM\LIB\TWO_FACTOR;
use LAM\LIB\TWO_FACTOR\TwoFactorProviderService;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2023 - 2024  Roland Gruber

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

include_once __DIR__ . '/../../lib/2factor.inc';

class TwoFactorProviderServiceSpy extends TwoFactorProviderService {

	private $cookieValue;

	/**
	 * @inheritDoc
	 */
	protected function setCookie(string $name, string $value, array $options): void {
		$this->cookieValue = $value;
	}

	/**
	 * Returns the cookie value if any.
	 *
	 * @return string value
	 */
	public function getCookieValue() {
		return $this->cookieValue;
	}

}