<?php

namespace LAM\LOGIN\WEBAUTHN;

use CBOR\Decoder;
use CBOR\OtherObject\OtherObjectManager;
use CBOR\Tag\TagObjectManager;
use Cose\Algorithm\Manager;
use Cose\Algorithm\Signature\ECDSA\ES256;
use Cose\Algorithm\Signature\ECDSA\ES384;
use Cose\Algorithm\Signature\ECDSA\ES512;
use Cose\Algorithm\Signature\EdDSA\EdDSA;
use Cose\Algorithm\Signature\RSA\RS1;
use Cose\Algorithm\Signature\RSA\RS256;
use Cose\Algorithm\Signature\RSA\RS384;
use Cose\Algorithm\Signature\RSA\RS512;
use \Cose\Algorithms;
use Symfony\Component\HttpFoundation\Request;
use Webauthn\AttestationStatement\AndroidKeyAttestationStatementSupport;
use Webauthn\AttestationStatement\AttestationObjectLoader;
use Webauthn\AttestationStatement\AttestationStatementSupportManager;
use Webauthn\AttestationStatement\FidoU2FAttestationStatementSupport;
use Webauthn\AttestationStatement\NoneAttestationStatementSupport;
use Webauthn\AttestationStatement\PackedAttestationStatementSupport;
use Webauthn\AttestationStatement\TPMAttestationStatementSupport;
use Webauthn\AuthenticationExtensions\ExtensionOutputCheckerHandler;
use Webauthn\AuthenticatorAttestationResponse;
use Webauthn\AuthenticatorAttestationResponseValidator;
use \Webauthn\PublicKeyCredentialCreationOptions;
use Webauthn\PublicKeyCredentialLoader;
use \Webauthn\PublicKeyCredentialRpEntity;
use \Webauthn\PublicKeyCredentialParameters;
use Webauthn\PublicKeyCredentialSource;
use Webauthn\PublicKeyCredentialSourceRepository;
use \Webauthn\PublicKeyCredentialUserEntity;
use \Webauthn\AuthenticationExtensions\AuthenticationExtensionsClientInputs;
use \Webauthn\AuthenticatorSelectionCriteria;
use Webauthn\TokenBinding\IgnoreTokenBindingHandler;

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
 * @return PublicKeyCredentialCreationOptions registration object
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

/**
 * Verifies the registration and stores it in the database.
 *
 * @param PublicKeyCredentialCreationOptions $registration registration object
 * @param string $clientResponse client response
 * @return bool true if response is valid and registration succeeded
 */
function storeNewRegistration($registration, $clientResponse) {
	$decoder = getCborDecoder();
	$tokenBindingHandler = new IgnoreTokenBindingHandler();
	$attestationSupportManager = getAttestationSupportManager($decoder);
	$attestationObjectLoader = getAttestationObjectLoader($attestationSupportManager, $decoder);
	$publicKeyCredentialLoader = getPublicKeyCredentialLoader($attestationObjectLoader, $decoder);
	$extensionOutputCheckerHandler = getExtensionOutputChecker();
	$repository = new PublicKeyCredentialSourceRepositorySQLite();
	$responseValidator = new AuthenticatorAttestationResponseValidator(
		$attestationSupportManager, $repository, $tokenBindingHandler, $extensionOutputCheckerHandler);
	try {
		logNewMessage(LOG_ERR, 'RESPONSE: ' . $clientResponse);
		$publicKeyCredential = $publicKeyCredentialLoader->load($clientResponse);
		$authenticatorAttestationResponse = $publicKeyCredential->getResponse();
		if (!$authenticatorAttestationResponse instanceof AuthenticatorAttestationResponse) {
			logNewMessage(LOG_ERR, 'Invalid webauthn response: ' . $clientResponse);
			return false;
		}
		$symfonyRequest = Request::createFromGlobals();
		$responseValidator->check($authenticatorAttestationResponse, $registration, $symfonyRequest);
		return true;
	}
	catch (\Throwable $exception) {
		logNewMessage(LOG_ERR, 'Webauthn validation failed: ' . $exception->getMessage() . $exception->getTraceAsString());
	}
	return false;
}

/**
 * Returns a CBOR decoder.
 *
 * @return Decoder decoder
 */
function getCborDecoder() {
	return new Decoder(new TagObjectManager(), new OtherObjectManager());
}

/**
 * Creates the attestation support manager.
 *
 * @param Decoder $decoder decoder
 * @return AttestationStatementSupportManager manager
 */
function getAttestationSupportManager($decoder) {
	$manager = new AttestationStatementSupportManager();
	$manager->add(new NoneAttestationStatementSupport());
	$manager->add(new FidoU2FAttestationStatementSupport());
	$manager->add(new AndroidKeyAttestationStatementSupport($decoder));
	$manager->add(new TPMAttestationStatementSupport());
	$coseManager = new Manager();
	$coseManager->add(new ES256());
	$coseManager->add(new ES384());
	$coseManager->add(new ES512());
	$coseManager->add(new EdDSA());
	$coseManager->add(new RS1());
	$coseManager->add(new RS256());
	$coseManager->add(new RS384);
	$coseManager->add(new RS512());
	$manager->add(new PackedAttestationStatementSupport($decoder, $coseManager));
	return $manager;
}

/**
 * Returns the attestation object loader.
 *
 * @param AttestationStatementSupportManager $manager support manager
 * @param Decoder $decoder decoder
 * @return AttestationObjectLoader attestation object loader
 */
function getAttestationObjectLoader($manager, $decoder) {
	return new AttestationObjectLoader($manager, $decoder);
}

/**
 * Creates the public key credential loader.
 *
 * @param AttestationObjectLoader $attestationObjectLoader attestation object loader
 * @param Decoder $decoder decoder
 * @return PublicKeyCredentialLoader public key credential loader
 */
function getPublicKeyCredentialLoader($attestationObjectLoader, $decoder) {
	return new PublicKeyCredentialLoader($attestationObjectLoader, $decoder);
}

/**
 * Returns the extension output checker handler.
 * No extensions are checked at this time.
 *
 * @return ExtensionOutputCheckerHandler handler
 */
function getExtensionOutputChecker() {
	return new ExtensionOutputCheckerHandler();
}

/**
 * Stores the public key credentials in the SQLite database.
 *
 * @package LAM\LOGIN\WEBAUTHN
 */
class PublicKeyCredentialSourceRepositorySQLite implements PublicKeyCredentialSourceRepository {

	/**
	 * Finds the public key for the given credential id.
	 *
	 * @param string $publicKeyCredentialId credential id
	 * @return PublicKeyCredentialSource|null credential source
	 */
	public function findOneByCredentialId(string $publicKeyCredentialId): ?PublicKeyCredentialSource {
		// TODO: Implement findOneByCredentialId() method.
		logNewMessage(LOG_WARNING, 'FIND ONE: ' . $publicKeyCredentialId);
		return null;
	}

	/**
	 * Finds all credential entries for the given user.
	 *
	 * @param PublicKeyCredentialUserEntity $publicKeyCredentialUserEntity credential user entity
	 * @return PublicKeyCredentialSource[] credential sources
	 */
	public function findAllForUserEntity(PublicKeyCredentialUserEntity $publicKeyCredentialUserEntity): array {
		// TODO: Implement findAllForUserEntity() method.
		logNewMessage(LOG_WARNING, 'FIND ALL: ' . json_encode($publicKeyCredentialUserEntity));
		return array();
	}

	/**
	 * Saves the given credential in the database.
	 *
	 * @param PublicKeyCredentialSource $publicKeyCredentialSource credential
	 */
	public function saveCredentialSource(PublicKeyCredentialSource $publicKeyCredentialSource): void {
		// TODO: Implement saveCredentialSource() method.
		logNewMessage(LOG_WARNING, 'SAVE: ' . json_encode($publicKeyCredentialSource));
	}

}


