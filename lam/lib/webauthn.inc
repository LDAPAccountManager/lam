<?php
namespace LAM\LOGIN\WEBAUTHN;
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2019  Roland Gruber

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
* Manages webauthn requests.
*
* @author Roland Gruber
*/

/**
 * Returns if the given DN is registered for webauthn.
 *
 * @param string $dn DN
 * @return boolean is registered
 */
function isRegistered($dn) {
	return false;
}

/**
 * Returns a challenge for a new token.
 *
 * @return string challenge
 */
function getChallenge() {

}


