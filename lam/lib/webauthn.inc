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
use Nyholm\Psr7\Factory\Psr17Factory;
use PDO;
use Symfony\Bridge\PsrHttpMessage\Factory\PsrHttpFactory;
use Symfony\Component\HttpFoundation\Request;
use Throwable;
use Webauthn\AttestationStatement\AndroidKeyAttestationStatementSupport;
use Webauthn\AttestationStatement\AttestationObjectLoader;
use Webauthn\AttestationStatement\AttestationStatementSupportManager;
use Webauthn\AttestationStatement\FidoU2FAttestationStatementSupport;
use Webauthn\AttestationStatement\NoneAttestationStatementSupport;
use Webauthn\AttestationStatement\PackedAttestationStatementSupport;
use Webauthn\AttestationStatement\TPMAttestationStatementSupport;
use Webauthn\AuthenticationExtensions\ExtensionOutputCheckerHandler;
use Webauthn\AuthenticatorAssertionResponse;
use Webauthn\AuthenticatorAssertionResponseValidator;
use Webauthn\AuthenticatorAttestationResponse;
use Webauthn\AuthenticatorAttestationResponseValidator;
use \Webauthn\PublicKeyCredentialCreationOptions;
use Webauthn\PublicKeyCredentialDescriptor;
use Webauthn\PublicKeyCredentialLoader;
use Webauthn\PublicKeyCredentialRequestOptions;
use \Webauthn\PublicKeyCredentialRpEntity;
use \Webauthn\PublicKeyCredentialParameters;
use Webauthn\PublicKeyCredentialSource;
use Webauthn\PublicKeyCredentialSourceRepository;
use \Webauthn\PublicKeyCredentialUserEntity;
use \Webauthn\AuthenticationExtensions\AuthenticationExtensionsClientInputs;
use \Webauthn\AuthenticatorSelectionCriteria;
use Webauthn\TokenBinding\IgnoreTokenBindingHandler;
use \LAMException;

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

include_once __DIR__ . '/3rdParty/composer/autoload.php';

/**
 * Manages Webauthn registrations and authentications.
 *
 * @package LAM\LOGIN\WEBAUTHN
 */
class WebauthnManager {

	/**
	 * Returns if the given DN is registered for webauthn.
	 *
	 * @param string $dn DN
	 * @return boolean is registered
	 */
	public function isRegistered($dn) {
		$database = $this->getDatabase();
		$userEntity = $this->getUserEntity($dn);
		$results = $database->findAllForUserEntity($userEntity);
		return !empty($results);
	}

	/**
	 * Returns a challenge for a new token.
	 *
	 * @param string $dn DN
	 * @param bool $isSelfService is executed in self service
	 * @return PublicKeyCredentialCreationOptions registration object
	 */
	public function getRegistrationObject($dn, $isSelfService) {
		$rpEntity = $this->createRpEntry($isSelfService);
		$userEntity = $this->getUserEntity($dn);
		$challenge = $this->createChallenge();
		$credentialParameters = $this->getCredentialParameters();
		$excludedKeys = $this->getExcludedKeys($userEntity);
		$timeout = $this->getTimeout();
		$registrationObject = new PublicKeyCredentialCreationOptions(
			$rpEntity,
			$userEntity,
			$challenge,
			$credentialParameters,
			$timeout,
			$excludedKeys,
			new AuthenticatorSelectionCriteria(),
			PublicKeyCredentialCreationOptions::ATTESTATION_CONVEYANCE_PREFERENCE_NONE,
			new AuthenticationExtensionsClientInputs());
		logNewMessage(LOG_DEBUG, 'Webauthn registration: ' . json_encode($registrationObject));
		return $registrationObject;
	}

	/**
	 * Verifies the registration and stores it in the database.
	 *
	 * @param PublicKeyCredentialCreationOptions $registration registration object
	 * @param string $clientResponse client response
	 * @return bool true if response is valid and registration succeeded
	 */
	public function storeNewRegistration($registration, $clientResponse) {
		$decoder = $this->getCborDecoder();
		$tokenBindingHandler = new IgnoreTokenBindingHandler();
		$attestationSupportManager = $this->getAttestationSupportManager($decoder);
		$attestationObjectLoader = $this->getAttestationObjectLoader($attestationSupportManager, $decoder);
		$publicKeyCredentialLoader = $this->getPublicKeyCredentialLoader($attestationObjectLoader, $decoder);
		$extensionOutputCheckerHandler = $this->getExtensionOutputChecker();
		$repository = new PublicKeyCredentialSourceRepositorySQLite();
		$responseValidator = new AuthenticatorAttestationResponseValidator(
			$attestationSupportManager, $repository, $tokenBindingHandler, $extensionOutputCheckerHandler);
		try {
			$publicKeyCredential = $publicKeyCredentialLoader->load($clientResponse);
			$authenticatorAttestationResponse = $publicKeyCredential->getResponse();
			if (!$authenticatorAttestationResponse instanceof AuthenticatorAttestationResponse) {
				logNewMessage(LOG_ERR, 'Invalid webauthn response: ' . $clientResponse);
				return false;
			}
			$symfonyRequest = Request::createFromGlobals();
			$psr17Factory = new Psr17Factory();
			$psrFactory = new PsrHttpFactory($psr17Factory, $psr17Factory, $psr17Factory, $psr17Factory);
			$psr7Request = $psrFactory->createRequest($symfonyRequest);
			$publicKeyCredentialSource = $responseValidator->check($authenticatorAttestationResponse, $registration, $psr7Request);
			$repository->saveCredentialSource($publicKeyCredentialSource);
			return true;
		}
		catch (\Throwable $exception) {
			logNewMessage(LOG_ERR, 'Webauthn validation failed: ' . $exception->getMessage() . $exception->getTraceAsString());
		}
		return false;
	}

	/**
	 * Returns the user entity for the registration.
	 *
	 * @param $dn DN
	 * @return PublicKeyCredentialUserEntity user entity
	 */
	private function getUserEntity($dn) {
		return new PublicKeyCredentialUserEntity(
			$dn,
			$dn,
			extractRDNValue($dn),
			null
		);
	}

	/**
	 * Returns the part that identifies the server and application.
	 *
	 * @param bool $isSelfService is executed in self service
	 * @return PublicKeyCredentialRpEntity relying party entry
	 */
	private function createRpEntry($isSelfService) {
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
	 * Returns the supported credential algorithms.
	 *
	 * @return array algorithms
	 */
	private function getCredentialParameters() {
		return array(
			new PublicKeyCredentialParameters('public-key', Algorithms::COSE_ALGORITHM_ES256),
			new PublicKeyCredentialParameters('public-key', Algorithms::COSE_ALGORITHM_RS256),
		);
	}

	/**
	 * Returns a list of all credential ids that are already registered.
	 *
	 * @param PublicKeyCredentialUserEntity $user user data
	 * @return PublicKeyCredentialDescriptor[] credential ids
	 */
	private function getExcludedKeys($user) {
		$keys = array();
		$repository = $this->getDatabase();
		$credentialSources = $repository->findAllForUserEntity($user);
		foreach ($credentialSources as $credentialSource) {
			$keys[] = new PublicKeyCredentialDescriptor(PublicKeyCredentialDescriptor::CREDENTIAL_TYPE_PUBLIC_KEY, $credentialSource->getPublicKeyCredentialId());
		}
		return $keys;
	}

	/**
	 * Returns a CBOR decoder.
	 *
	 * @return Decoder decoder
	 */
	private function getCborDecoder() {
		return new Decoder(new TagObjectManager(), new OtherObjectManager());
	}

	/**
	 * Creates the attestation support manager.
	 *
	 * @param Decoder $decoder decoder
	 * @return AttestationStatementSupportManager manager
	 */
	private function getAttestationSupportManager($decoder) {
		$manager = new AttestationStatementSupportManager();
		$manager->add(new NoneAttestationStatementSupport());
		$manager->add(new FidoU2FAttestationStatementSupport());
		$manager->add(new AndroidKeyAttestationStatementSupport($decoder));
		$manager->add(new TPMAttestationStatementSupport());
		$coseManager = $this->getAlgorithmManager();
		$manager->add(new PackedAttestationStatementSupport($decoder, $coseManager));
		return $manager;
	}

	/**
	 * Returns the COSE algorithm manager.
	 *
	 * @return Manager algorithm manager
	 */
	private function getAlgorithmManager() {
		$coseManager = new Manager();
		$coseManager->add(new ES256());
		$coseManager->add(new ES384());
		$coseManager->add(new ES512());
		$coseManager->add(new EdDSA());
		$coseManager->add(new RS1());
		$coseManager->add(new RS256());
		$coseManager->add(new RS384);
		$coseManager->add(new RS512());
		return $coseManager;
	}

	/**
	 * Returns the attestation object loader.
	 *
	 * @param AttestationStatementSupportManager $manager support manager
	 * @param Decoder $decoder decoder
	 * @return AttestationObjectLoader attestation object loader
	 */
	private function getAttestationObjectLoader($manager, $decoder) {
		return new AttestationObjectLoader($manager, $decoder);
	}

	/**
	 * Creates the public key credential loader.
	 *
	 * @param AttestationObjectLoader $attestationObjectLoader attestation object loader
	 * @param Decoder $decoder decoder
	 * @return PublicKeyCredentialLoader public key credential loader
	 */
	private function getPublicKeyCredentialLoader($attestationObjectLoader, $decoder) {
		return new PublicKeyCredentialLoader($attestationObjectLoader, $decoder);
	}

	/**
	 * Returns the extension output checker handler.
	 * No extensions are checked at this time.
	 *
	 * @return ExtensionOutputCheckerHandler handler
	 */
	private function getExtensionOutputChecker() {
		return new ExtensionOutputCheckerHandler();
	}

	/**
	 * Returns the webauthn database.
	 *
	 * @return PublicKeyCredentialSourceRepositorySQLite database
	 */
	public function getDatabase() {
		return new PublicKeyCredentialSourceRepositorySQLite();
	}

	/**
	 * Returns the timeout for user operations.
	 *
	 * @return int timeout in ms
	 */
	private function getTimeout() {
		return 120000;
	}

	/**
	 * Creates a new challenge.
	 *
	 * @return String challenge
	 */
	private function createChallenge() {
		return generateRandomPassword(32);
	}

	/**
	 * Returns the authentication object for a registered user.
	 *
	 * @param $userDN user DN
	 * @param bool $isSelfService self service
	 * @return PublicKeyCredentialRequestOptions authentication object
	 */
	public function getAuthenticationObject($userDN, bool $isSelfService) {
		$timeout = $this->getTimeout();
		$challenge = $this->createChallenge();
		$database = $this->getDatabase();
		$userEntity = $this->getUserEntity($userDN);
		$publicKeyCredentialSources = $database->findAllForUserEntity($userEntity);
		$publicKeyDescriptors = array();
		foreach ($publicKeyCredentialSources as $publicKeyCredentialSource) {
			$publicKeyDescriptors[] = $publicKeyCredentialSource->getPublicKeyCredentialDescriptor();
		}
		$userVerification = PublicKeyCredentialRequestOptions::USER_VERIFICATION_REQUIREMENT_DISCOURAGED;
		$extensions = new AuthenticationExtensionsClientInputs();
		$relyingParty = $this->createRpEntry($isSelfService);
		return new PublicKeyCredentialRequestOptions(
			$challenge,
			$timeout,
			$relyingParty->getId(),
			$publicKeyDescriptors,
			$userVerification,
			$extensions
		);
	}

	/**
	 * Checks if the provided authentication is valid.
	 *
	 * @param string $response authentication response
	 * @param string $userDn user DN
	 * @return bool true if all is ok
	 */
	public function isValidAuthentication(string $response, string $userDn) {
		$database = $this->getDatabase();
		$decoder = $this->getCborDecoder();
		$tokenBindingHandler = new IgnoreTokenBindingHandler();
		$extensionOutputCheckerHandler = $this->getExtensionOutputChecker();
		$algorithmManager = $this->getAlgorithmManager();
		$responseValidator = new AuthenticatorAssertionResponseValidator(
			$database,
			$decoder,
			$tokenBindingHandler,
			$extensionOutputCheckerHandler,
			$algorithmManager
		);
		$attestationSupportManager = $this->getAttestationSupportManager($decoder);
		$attestationObjectLoader = $this->getAttestationObjectLoader($attestationSupportManager, $decoder);
		$publicKeyCredentialLoader = $this->getPublicKeyCredentialLoader($attestationObjectLoader, $decoder);
		$publicKeyCredential = $publicKeyCredentialLoader->load($response);
		$authenticatorAssertionResponse = $publicKeyCredential->getResponse();
		if (!$authenticatorAssertionResponse instanceof AuthenticatorAssertionResponse) {
			logNewMessage(LOG_ERR, 'Invalid authenticator assertion response');
			return false;
		}
		try {
			$symfonyRequest = Request::createFromGlobals();
			$psr17Factory = new Psr17Factory();
			$psrFactory = new PsrHttpFactory($psr17Factory, $psr17Factory, $psr17Factory, $psr17Factory);
			$psr7Request = $psrFactory->createRequest($symfonyRequest);
			$publicKeyCredentialRequestOptions = PublicKeyCredentialRequestOptions::createFromString($_SESSION['webauthn_authentication']);
			$responseValidator->check(
				$publicKeyCredential->getRawId(),
				$publicKeyCredential->getResponse(),
				$publicKeyCredentialRequestOptions,
				$psr7Request,
				$userDn
			);
			return true;
		}
		catch (Throwable $e) {
			logNewMessage(LOG_ERR, 'Error validating webauthn authentication: ' . $e->getMessage());
		}
		return false;
	}

}

/**
 * Stores the public key credentials in the SQLite database.
 *
 * @package LAM\LOGIN\WEBAUTHN
 */
class PublicKeyCredentialSourceRepositorySQLite implements PublicKeyCredentialSourceRepository {

	const TABLE_NAME = 'lam_webauthn';

	/**
	 * Finds the public key for the given credential id.
	 *
	 * @param string $publicKeyCredentialId credential id
	 * @return PublicKeyCredentialSource|null credential source
	 */
	public function findOneByCredentialId(string $publicKeyCredentialId): ?PublicKeyCredentialSource {
		try {
			$pdo = $this->getPDO();
			$statement = $pdo->prepare('select * from ' . self::TABLE_NAME . ' where credentialId = :credentialid');
			$statement->execute(array(':credentialid' => base64_encode($publicKeyCredentialId)));
			$results = $statement->fetchAll();
			if (!empty($results)) {
				$jsonArray = json_decode($results[0]['credentialSource'], true);
				return PublicKeyCredentialSource::createFromArray($jsonArray);
			}
		}
		catch (\PDOException $e) {
			logNewMessage(LOG_ERR, 'Webauthn database error: ' . $e->getMessage());
		}
		return null;
	}

	/**
	 * Finds all credential entries for the given user.
	 *
	 * @param PublicKeyCredentialUserEntity $publicKeyCredentialUserEntity credential user entity
	 * @return PublicKeyCredentialSource[] credential sources
	 */
	public function findAllForUserEntity(PublicKeyCredentialUserEntity $publicKeyCredentialUserEntity): array {
		$credentials = array();
		try {
			$pdo = $this->getPDO();
			$statement = $pdo->prepare('select * from ' . self::TABLE_NAME . ' where userId = :userid');
			$statement->execute(array(':userid' => $publicKeyCredentialUserEntity->getId()));
			$results = $statement->fetchAll();
			foreach ($results as $result) {
				$jsonArray = json_decode($results[0]['credentialSource'], true);
				$credentials[] = PublicKeyCredentialSource::createFromArray($jsonArray);
			}
		}
		catch (\PDOException $e) {
			logNewMessage(LOG_ERR, 'Webauthn database error: ' . $e->getMessage());
		}
		return $credentials;
	}

	/**
	 * Saves the given credential in the database.
	 *
	 * @param PublicKeyCredentialSource $publicKeyCredentialSource credential
	 */
	public function saveCredentialSource(PublicKeyCredentialSource $publicKeyCredentialSource): void {
		$json = json_encode($publicKeyCredentialSource);
		$credentialId = base64_encode($publicKeyCredentialSource->getPublicKeyCredentialId());
		$userId = $publicKeyCredentialSource->getUserHandle();
		$currentTime = time();
		$pdo = $this->getPDO();
		$statement = $pdo->prepare('select * from ' . self::TABLE_NAME . ' where userId = :userId and credentialId = :credentialId');
		$statement->execute(array(
			':userId' => $userId,
			':credentialId' => $credentialId
		));
		$results = $statement->fetchAll();
		if (empty($results)) {
			$statement = $pdo->prepare('insert into ' . self::TABLE_NAME . ' (userId, credentialId, credentialSource, registrationTime, lastUseTime) VALUES (?, ?, ?, ?, ?)');
			$statement->execute(array(
				$userId,
				$credentialId,
				$json,
				$currentTime,
				$currentTime
			));
			logNewMessage(LOG_DEBUG, 'Stored new credential for ' . $userId);
		}
		else {
			$statement = $pdo->prepare(
				'update ' . self::TABLE_NAME .
				' set credentialSource = :credentialSource, lastUseTime = :lastUseTime' .
				' WHERE userId = :userId AND credentialId = :credentialId'
			);
			$statement->execute(array(
				':credentialSource' => $json,
				':lastUseTime' => $currentTime,
				':userId' => $userId,
				':credentialId' => $credentialId
			));
			logNewMessage(LOG_DEBUG, 'Stored updated credential for ' . $userId);
		}
	}

	/**
	 * Returns the database URL.
	 *
	 * @return string the PDO URL
	 */
	public function getPdoUrl() {
		$fileName = dirname(__FILE__) . '/../config/__lam.webauthn.sqlite';
		if (!file_exists($fileName)) {
			$handle = fopen($fileName, 'w');
			fclose($handle);
			chmod($fileName, 0600);
		}
		return 'sqlite:' . $fileName;
	}

	/**
	 * Returns the PDO.
	 *
	 * @return PDO PDO
	 */
	protected function getPDO() {
		$pdo = new PDO($this->getPdoUrl(), null, null, array(
			PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION
		));
		// initial schema
		if (!$this->tableExists($pdo, self::TABLE_NAME)) {
			$this->createInitialSchema($pdo);
		}
		return $pdo;
	}

	/**
	 * Checks if the given table name exists.
	 *
	 * @param PDO $pdo PDO object
	 * @param String $tableName table name to check
	 * @return boolean table exists
	 */
	protected function tableExists(&$pdo, $tableName) {
		try {
			$result = $pdo->query("SELECT 1 FROM $tableName LIMIT 1");
			return ($result === false) ? false : true;
		} catch (\PDOException $e) {
			return false;
		}
	}

	/**
	 * Creates the initial schema.
	 *
	 * @param PDO $pdo PDO object
	 */
	protected function createInitialSchema($pdo) {
		logNewMessage(LOG_DEBUG, 'Creating database table ' . self::TABLE_NAME);
		$sql = 'create table ' . self::TABLE_NAME . '('
			. 'userId TEXT NOT NULL,'
			. 'credentialId TEXT NOT NULL,'
			. 'credentialSource TEXT NOT NULL,'
			. 'registrationTime VARCHAR(11) NOT NULL,'
			. 'lastUseTime VARCHAR(11) NOT NULL,'
			. 'PRIMARY KEY(userId,credentialId)'
			. ');';
		$pdo->exec($sql);
	}

}


