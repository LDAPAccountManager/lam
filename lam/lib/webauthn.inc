<?php

namespace LAM\LOGIN\WEBAUTHN;

use \Cose\Algorithms;
use \Webauthn\PublicKeyCredentialCreationOptions;
use \Webauthn\PublicKeyCredentialRpEntity;
use \Webauthn\PublicKeyCredentialParameters;
use \Webauthn\PublicKeyCredentialUserEntity;
use \Webauthn\AuthenticationExtensions\AuthenticationExtensionsClientInputs;
use \Webauthn\AuthenticatorSelectionCriteria;

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
 * @param string $dn DN
 * @param bool $isSelfService is executed in self service
 * @return PublicKeyCredentialCreationOptions challenge
 */
function getRegistrationObject($dn, $isSelfService) {
	$rpEntity = createRpEntry($isSelfService);
	$userEntity = getUserEntity($dn);
	$challenge = generateRandomPassword(32);
	$credentialParameters = getCredentialParameters();
	$timeout = 20000;
	$registrationObject = new PublicKeyCredentialCreationOptions(
		$rpEntity,
		$userEntity,
		$challenge,
		$credentialParameters,
		$timeout,
		array(),
		new AuthenticatorSelectionCriteria(),
		PublicKeyCredentialCreationOptions::ATTESTATION_CONVEYANCE_PREFERENCE_NONE,
		new AuthenticationExtensionsClientInputs());
	logNewMessage(LOG_DEBUG, json_encode($registrationObject));
	return $registrationObject;
}

/**
 * Returns the part that identifies the server and application.
 *
 * @param bool $isSelfService is executed in self service
 * @return PublicKeyCredentialRpEntity relying party entry
 */
function createRpEntry($isSelfService) {
	$pathPrefix = $isSelfService ? '../' : '';
	$icon = $pathPrefix . '../graphics/logo136.png';
	if (!$isSelfService) {
		$domain = $_SESSION['config']->getTwoFactorAuthenticationDomain();
	}
	return new PublicKeyCredentialRpEntity(
		'LDAP Account Manager', //Name
		$domain,
		$icon
	);
}

/**
 * Returns the user entity for the registration.
 *
 * @param $dn DN
 * @return PublicKeyCredentialUserEntity user entity
 */
function getUserEntity($dn) {
	return new PublicKeyCredentialUserEntity(
		$dn,
		$dn,
		extractRDNValue($dn),
		null
	);
}

/**
 * Returns the supported credential algorithms.
 *
 * @return array algorithms
 */
function getCredentialParameters() {
	return array(
		new PublicKeyCredentialParameters('public-key', Algorithms::COSE_ALGORITHM_ES256),
		new PublicKeyCredentialParameters('public-key', Algorithms::COSE_ALGORITHM_RS256),
	);
}

