<?php
namespace LAM\TOOLS\WEBAUTHN;
use LAM\LIB\TWO_FACTOR\TwoFactorProviderService;
use LAMTool;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2020 - 2021  Roland Gruber

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
* Webauthn device management.
*
* @author Roland Gruber
* @package tools
*/

/**
 * Webauthn device management.
 *
 * @package tools
 */
class toolWebauthn extends LAMTool {

	/**
	 * {@inheritDoc}
	 */
	public function getName(): string {
	 	return _('WebAuthn devices');
	 }

	/**
	 * {@inheritDoc}
	 */
	public function getDescription(): string {
		return _("Here you can manage your webauthn devices.");
	}

	/**
	 * {@inheritDoc}
	 */
	public function getLink(): string {
		return "tools/webauthn.php";
	}

	/**
	 * {@inheritDoc}
	 */
	public function getRequiresWriteAccess(): bool {
		return false;
	}

	/**
	 * {@inheritDoc}
	 */
	public function getRequiresPasswordChangeRights(): bool {
		return false;
	}

	/**
	 * {@inheritDoc}
	 */
	public function getImageLink(): string {
		return 'webauthn.png';
	}

	/**
	 * {@inheritDoc}
	 */
	public function getPosition(): int {
		return 800;
	}

	/**
	 * {@inheritDoc}
	 */
	public function isVisible(): bool {
		return ($_SESSION['config']->getTwoFactorAuthentication() === TwoFactorProviderService::TWO_FACTOR_WEBAUTHN);
	}

}
