<?php

namespace LAM\LOGIN\WEBAUTHN;

use Cose\Algorithm\Manager;
use Cose\Algorithm\Signature\ECDSA\ES256;
use Cose\Algorithm\Signature\ECDSA\ES384;
use Cose\Algorithm\Signature\ECDSA\ES512;
use Cose\Algorithm\Signature\EdDSA\EdDSA;
use Cose\Algorithm\Signature\RSA\RS1;
use Cose\Algorithm\Signature\RSA\RS256;
use Cose\Algorithm\Signature\RSA\RS384;
use Cose\Algorithm\Signature\RSA\RS512;
use Cose\Algorithms;
use LAM\PERSISTENCE\ConfigurationDatabase;
use LAM_INTERFACE;
use LAMCfgMain;
use LAMException;
use PDO;
use PDOException;
use Symfony\Component\Serializer\SerializerInterface;
use Throwable;
use Webauthn\AttestationStatement\AndroidKeyAttestationStatementSupport;
use Webauthn\AttestationStatement\AttestationStatementSupportManager;
use Webauthn\AttestationStatement\FidoU2FAttestationStatementSupport;
use Webauthn\AttestationStatement\NoneAttestationStatementSupport;
use Webauthn\AttestationStatement\PackedAttestationStatementSupport;
use Webauthn\AttestationStatement\TPMAttestationStatementSupport;
use Webauthn\AuthenticationExtensions\AuthenticationExtensions;
use Webauthn\AuthenticationExtensions\ExtensionOutputCheckerHandler;
use Webauthn\AuthenticatorAssertionResponse;
use Webauthn\AuthenticatorAssertionResponseValidator;
use Webauthn\AuthenticatorAttestationResponse;
use Webauthn\AuthenticatorAttestationResponseValidator;
use Webauthn\AuthenticatorSelectionCriteria;
use Webauthn\CeremonyStep\CeremonyStepManagerFactory;
use Webauthn\Denormalizer\WebauthnSerializerFactory;
use Webauthn\PublicKeyCredential;
use Webauthn\PublicKeyCredentialCreationOptions;
use Webauthn\PublicKeyCredentialDescriptor;
use Webauthn\PublicKeyCredentialParameters;
use Webauthn\PublicKeyCredentialRequestOptions;
use Webauthn\PublicKeyCredentialRpEntity;
use Webauthn\PublicKeyCredentialSource;
use Webauthn\PublicKeyCredentialUserEntity;
use function LAM\PERSISTENCE\dbTableExists;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2019 - 2026  Roland Gruber

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
	 * @return bool is registered
	 */
	public function isRegistered($dn): bool {
		$database = $this->getDatabase();
		$results = $database->findAllForUserDn($dn);
		return !empty($results);
	}

	/**
	 * Returns a challenge for a new token.
	 *
	 * @param string $dn DN
	 * @param LAM_INTERFACE $interface interface
	 * @param string[] $extraExcludedKeys credentialIds that should be added to excluded keys
	 * @return PublicKeyCredentialCreationOptions registration object
	 */
	public function getRegistrationObject($dn, LAM_INTERFACE $interface, $extraExcludedKeys = []) {
		$rpEntity = $this->createRpEntry($interface);
		$_SESSION['webauthn_rp_id'] = $rpEntity->id;
		$userEntity = $this->getUserEntity($dn);
		$challenge = $this->createChallenge();
		$credentialParameters = $this->getCredentialParameters();
		$excludedKeys = $this->getExcludedKeys($dn, $extraExcludedKeys);
		$timeout = $this->getTimeout();
		$authenticatorSelectionCriteria = AuthenticatorSelectionCriteria::create(null, AuthenticatorSelectionCriteria::USER_VERIFICATION_REQUIREMENT_DISCOURAGED);
		$registrationObject = PublicKeyCredentialCreationOptions::create(
			$rpEntity,
			$userEntity,
			$challenge,
			$credentialParameters,
			$authenticatorSelectionCriteria,
			PublicKeyCredentialCreationOptions::ATTESTATION_CONVEYANCE_PREFERENCE_NONE,
			$excludedKeys,
			$timeout,
			new AuthenticationExtensions()
		);
		logNewMessage(LOG_DEBUG, 'WebAuthn registration: ' . json_encode($registrationObject));
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
		$publicKeyCredentialLoader = $this->createPublicKeyCredentialLoader();
		$repository = $this->getDatabase();
		$csmFactory = new CeremonyStepManagerFactory();
		$creationCSM = $csmFactory->creationCeremony();
		$responseValidator = AuthenticatorAttestationResponseValidator::create($creationCSM);
		try {
			$publicKeyCredential = $publicKeyCredentialLoader->deserialize($clientResponse, PublicKeyCredential::class, 'json');
			$authenticatorAttestationResponse = $publicKeyCredential->response;
			if (!$authenticatorAttestationResponse instanceof AuthenticatorAttestationResponse) {
				logNewMessage(LOG_ERR, 'Invalid webauthn response: ' . $clientResponse);
				return false;
			}
			$publicKeyCredentialSource = $responseValidator->check($authenticatorAttestationResponse, $registration, $_SESSION['webauthn_rp_id']);
			$repository->saveCredentialSource($publicKeyCredentialSource);
			return true;
		}
		catch (Throwable $exception) {
			logNewMessage(LOG_ERR, 'WebAuthn validation failed: ' . $exception->getMessage() . $exception->getTraceAsString());
		}
		return false;
	}

	/**
	 * Returns a public key credential loader.
	 *
	 * @return SerializerInterface public key credential loader
	 */
	public function createPublicKeyCredentialLoader() {
		$attestationSupportManager = $this->getAttestationSupportManager();
		$factory = new WebauthnSerializerFactory($attestationSupportManager);
		return $factory->create();
	}

	/**
	 * Returns the user entity for the registration.
	 *
	 * @param string $dn DN
	 * @return PublicKeyCredentialUserEntity user entity
	 */
	private function getUserEntity($dn) {
		return new PublicKeyCredentialUserEntity(
			self::getUserIdFromDn($dn),
			self::getUserIdFromDn($dn),
			extractRDNValue($dn)
		);
	}

	/**
	 * Generates the user ID as hash of the user DN.
	 *
	 * @param string $dn user DN
	 * @return string user ID
	 */
	public static function getUserIdFromDn(string $dn) {
		return hash('sha256', $dn);
	}

	/**
	 * Returns the part that identifies the server and application.
	 *
	 * @param LAM_INTERFACE $interface interface
	 * @return PublicKeyCredentialRpEntity relying party entry
	 */
	private function createRpEntry(LAM_INTERFACE $interface) {
		$pathPrefix = in_array($interface, [LAM_INTERFACE::SELF_SERVICE, LAM_INTERFACE::WHITE_PAGES]) ? '../' : '';
		$icon = $pathPrefix . '../graphics/logo136.png';
		if ($interface === LAM_INTERFACE::SELF_SERVICE) {
			$domain = $_SESSION['selfServiceProfile']->twoFactorAuthenticationDomain;
		}
		elseif ($interface === LAM_INTERFACE::WHITE_PAGES) {
			$domain = $_SESSION['whitePagesProfile']->twoFactorAuthenticationDomain;
		}
		else {
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
	 * @return PublicKeyCredentialParameters[] algorithms
	 */
	private function getCredentialParameters(): array {
		return [
			new PublicKeyCredentialParameters('public-key', Algorithms::COSE_ALGORITHM_ES256),
			new PublicKeyCredentialParameters('public-key', Algorithms::COSE_ALGORITHM_RS256),
		];
	}

	/**
	 * Returns a list of all credential ids that are already registered.
	 *
	 * @param string $userDn user DN
	 * @param string[] $extraExcludedKeys credentialIds that should be added to excluded keys
	 * @return PublicKeyCredentialDescriptor[] credential ids
	 */
	private function getExcludedKeys(string $userDn, $extraExcludedKeys = []) {
		$keys = [];
		$repository = $this->getDatabase();
		$credentialSources = $repository->findAllForUserDn($userDn);
		foreach ($credentialSources as $credentialSource) {
			$keys[] = new PublicKeyCredentialDescriptor(PublicKeyCredentialDescriptor::CREDENTIAL_TYPE_PUBLIC_KEY, $credentialSource->publicKeyCredentialId);
		}
		foreach ($extraExcludedKeys as $extraExcludedKey) {
			$keys[] = new PublicKeyCredentialDescriptor(PublicKeyCredentialDescriptor::CREDENTIAL_TYPE_PUBLIC_KEY, $extraExcludedKey);
		}
		return $keys;
	}

	/**
	 * Creates the attestation support manager.
	 *
	 * @return AttestationStatementSupportManager manager
	 */
	private function getAttestationSupportManager() {
		$manager = new AttestationStatementSupportManager();
		$manager->add(new NoneAttestationStatementSupport());
		$manager->add(new FidoU2FAttestationStatementSupport());
		$manager->add(new AndroidKeyAttestationStatementSupport());
		$manager->add(new TPMAttestationStatementSupport());
		$coseManager = $this->getAlgorithmManager();
		$manager->add(new PackedAttestationStatementSupport($coseManager));
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
	 * @return PublicKeyCredentialSourceRepositoryBase database
	 * @throws LAMException error connecting to database
	 */
	public function getDatabase() {
		try {
			$configDatabase = new ConfigurationDatabase(new LAMCfgMain());
			if ($configDatabase->useRemoteDb()) {
				return new PublicKeyCredentialSourceRepositoryMySql($configDatabase->getPdo());
			}
			return new PublicKeyCredentialSourceRepositorySQLite();
		}
		catch (PDOException $e) {
			throw new LAMException(_('Unable to connect to database.'), $e->getMessage(), $e);
		}
	}

	/**
	 * Returns the timeout for user operations.
	 *
	 * @return positive-int timeout in ms
	 */
	private function getTimeout(): int {
		return 120000;
	}

	/**
	 * Creates a new challenge.
	 *
	 * @return String challenge
	 */
	private function createChallenge() {
		return generateRandomPassword(32, false);
	}

	/**
	 * Returns the authentication object for a registered user.
	 *
	 * @param string $userDN user DN
	 * @param LAM_INTERFACE $interface interface
	 * @return PublicKeyCredentialRequestOptions authentication object
	 */
	public function getAuthenticationObject($userDN, LAM_INTERFACE $interface) {
		$timeout = $this->getTimeout();
		$challenge = $this->createChallenge();
		$database = $this->getDatabase();
		$publicKeyCredentialSources = $database->findAllForUserDn($userDN);
		$userVerification = PublicKeyCredentialRequestOptions::USER_VERIFICATION_REQUIREMENT_DISCOURAGED;
		$extensions = new AuthenticationExtensions();
		$relyingParty = $this->createRpEntry($interface);
		$_SESSION['webauthn_rp_id'] = $relyingParty->id;
		$allowCredentials = [];
		foreach ($publicKeyCredentialSources as $publicKeyCredentialSource) {
			$allowCredentials[] = $publicKeyCredentialSource->getPublicKeyCredentialDescriptor();
		}
		return PublicKeyCredentialRequestOptions::create(
			$challenge,
			$relyingParty->id,
			$allowCredentials,
			$userVerification,
			$timeout,
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
		$csmFactory = new CeremonyStepManagerFactory();
		$requestCSM = $csmFactory->requestCeremony();
		$responseValidator = AuthenticatorAssertionResponseValidator::create(
			$requestCSM
		);
		$publicKeyCredentialLoader = $this->createPublicKeyCredentialLoader();
		$publicKeyCredential = $publicKeyCredentialLoader->deserialize($response, PublicKeyCredential::class, 'json');
		$authenticatorAssertionResponse = $publicKeyCredential->response;
		if (!$authenticatorAssertionResponse instanceof AuthenticatorAssertionResponse) {
			logNewMessage(LOG_ERR, 'Invalid authenticator assertion response');
			return false;
		}
		try {
			$loader = $this->createPublicKeyCredentialLoader();
			$publicKeyCredentialRequestOptions = $loader->deserialize($_SESSION['webauthn_authentication'], PublicKeyCredentialRequestOptions::class, 'json');
			$credential = $database->findOneByCredentialId($publicKeyCredential->rawId);
			if ($credential === null) {
				throw new LAMException('', 'Unable to find credential');
			}
			$userId = $credential->userHandle;
			// old entries use DN as user ID
			$userHandle = ($userId === $userDn) ? $userDn : self::getUserIdFromDn($userDn);
			$responseValidator->check(
				$credential,
				$authenticatorAssertionResponse,
				$publicKeyCredentialRequestOptions,
				$_SESSION['webauthn_rp_id'],
				$userHandle
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
abstract class PublicKeyCredentialSourceRepositoryBase {

	/**
	 * Returns the table name to use.
	 *
	 * @return string table name
	 */
	protected function getTableName(): string {
		return 'lam_webauthn';
	}

	/**
	 * Finds the public key for the given credential id.
	 *
	 * @param string $publicKeyCredentialId credential id
	 * @return PublicKeyCredentialSource|null credential source
	 */
	public function findOneByCredentialId(string $publicKeyCredentialId): ?PublicKeyCredentialSource {
		try {
			$pdo = $this->getPDO();
			$statement = $pdo->prepare('select * from ' . $this->getTableName() . ' where credentialId = :credentialid');
			$statement->execute([':credentialid' => base64_encode($publicKeyCredentialId)]);
			$results = $statement->fetchAll();
			if (!empty($results)) {
				$jsonString = $results[0]['credentialSource'];
				$jsonArray = json_decode($jsonString, true);
				if (!is_array($jsonArray)) {
					return null;
				}
				$manager = new WebauthnManager();
				$factory = $manager->createPublicKeyCredentialLoader();
				return $factory->deserialize($jsonString, PublicKeyCredentialSource::class, 'json');
			}
		}
		catch (PDOException $e) {
			logNewMessage(LOG_ERR, 'WebAuthn database error: ' . $e->getMessage());
		}
		return null;
	}

	/**
	 * Finds all credential entries for the given user.
	 *
	 * @param string $userDn user DN
	 * @return PublicKeyCredentialSource[] credential sources
	 */
	public function findAllForUserDn(string $userDn): array {
		$credentials = [];
		try {
			$pdo = $this->getPDO();
			$statement = $pdo->prepare('select * from ' . $this->getTableName() . ' where userDN = :userDN');
			$statement->execute([':userDN' => $userDn]);
			$results = $statement->fetchAll();
			$manager = new WebauthnManager();
			$factory = $manager->createPublicKeyCredentialLoader();
			foreach ($results as $result) {
				$jsonString = $result['credentialSource'];
				$jsonArray = json_decode($jsonString, true);
				if (!is_array($jsonArray)) {
					return $credentials;
				}
				$credentials[] = $factory->deserialize($jsonString, PublicKeyCredentialSource::class, 'json');
			}
		}
		catch (PDOException $e) {
			logNewMessage(LOG_ERR, 'WebAuthn database error: ' . $e->getMessage());
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
		$credentialId = base64_encode($publicKeyCredentialSource->publicKeyCredentialId);
		$userId = $publicKeyCredentialSource->userHandle;
		$currentTime = time();
		$pdo = $this->getPDO();
		if (isset($_SESSION['selfService_clientDN'])) {
			$userDn = lamDecrypt($_SESSION['selfService_clientDN'], 'SelfService');
		}
		else {
			$userDn = $_SESSION['ldap']->getUserName();
		}
		$statement = $pdo->prepare('select * from ' . $this->getTableName() . ' where userDN = :userDN and credentialId = :credentialId');
		$statement->execute([
			':userDN' => $userDn,
			':credentialId' => $credentialId
		]);
		$results = $statement->fetchAll();
		if (empty($results)) {
			$statement = $pdo->prepare('insert into ' . $this->getTableName() . ' (userId, credentialId, credentialSource, registrationTime, lastUseTime, userDN) VALUES (?, ?, ?, ?, ?, ?)');
			$statement->execute([
				$userId,
				$credentialId,
				$json,
				$currentTime,
				$currentTime,
				$userDn
			]);
			logNewMessage(LOG_DEBUG, 'Stored new credential for ' . $userDn);
		}
		else {
			$statement = $pdo->prepare(
				'update ' . $this->getTableName() .
				' set credentialSource = :credentialSource, lastUseTime = :lastUseTime' .
				' WHERE userDN = :userDN AND credentialId = :credentialId'
			);
			$statement->execute([
				':credentialSource' => $json,
				':lastUseTime' => $currentTime,
				':userDN' => $userDn,
				':credentialId' => $credentialId
			]);
			logNewMessage(LOG_DEBUG, 'Stored updated credential for ' . $userDn);
		}
	}

	/**
	 * Returns if there are any credentials in the database.
	 *
	 * @return bool at least one credential in the database
	 */
	public function hasRegisteredCredentials(): bool {
		try {
			$pdo = $this->getPDO();
			$statement = $pdo->prepare('select count(*) from ' . $this->getTableName());
			$statement->execute();
			$results = $statement->fetchAll();
			return ($results[0][0] > 0);
		}
		catch (PDOException $e) {
			logNewMessage(LOG_ERR, 'WebAuthn database error: ' . $e->getMessage());
		}
		return false;
	}

	/**
	 * Performs a full-text search on the usernames and returns all devices found.
	 *
	 * @param string $searchTerm search term for user field
	 * @return array<array<string, mixed>> list of devices array('dn' => ..., 'credentialId' => ..., 'lastUseTime' => ..., 'registrationTime' => ...)
	 */
	public function searchDevices(string $searchTerm) {
		$pdo = $this->getPDO();
		$statement = $pdo->prepare('select * from ' . $this->getTableName() . ' where userDN like :searchTerm order by userId,registrationTime');
		$statement->execute([
			':searchTerm' => $searchTerm
		]);
		$results = $statement->fetchAll();
		$devices = [];
		foreach ($results as $result) {
			$name = empty($result['name']) ? '' : $result['name'];
			$devices[] = [
				'dn' => $result['userDN'],
				'credentialId' => $result['credentialId'],
				'lastUseTime' => $result['lastUseTime'],
				'registrationTime' => $result['registrationTime'],
				'name' => $name
			];
		}
		return $devices;
	}

	/**
	 * Performs a full-text search on the usernames and returns all devices found.
	 *
	 * @param string $userDn user DN
	 * @return array<array<string, mixed>> list of devices array('dn' => ..., 'credentialId' => ..., 'lastUseTime' => ..., 'registrationTime' => ...)
	 */
	public function getAllUserDevices(string $userDn) {
		$pdo = $this->getPDO();
		$statement = $pdo->prepare('select * from ' . $this->getTableName() . ' where userDN = :userDn order by userId,registrationTime');
		$statement->execute([
			':userDn' => $userDn
		]);
		$results = $statement->fetchAll();
		$devices = [];
		foreach ($results as $result) {
			$name = empty($result['name']) ? '' : $result['name'];
			$devices[] = [
				'dn' => $result['userDN'],
				'credentialId' => $result['credentialId'],
				'lastUseTime' => $result['lastUseTime'],
				'registrationTime' => $result['registrationTime'],
				'name' => $name
			];
		}
		return $devices;
	}

	/**
	 * Deletes a single device from the database.
	 *
	 * @param string $dn user DN
	 * @param string $credentialId credential id
	 * @return bool deletion was ok
	 */
	public function deleteDevice(string $dn, string $credentialId) {
		logNewMessage(LOG_NOTICE, 'Delete webauthn device ' . $credentialId . ' of ' . $dn);
		$pdo = $this->getPDO();
		$statement = $pdo->prepare('delete from ' . $this->getTableName() . ' where userDN = :userDN and credentialId = :credentialId');
		$statement->execute([
			':userDN' => $dn,
			':credentialId' => $credentialId
		]);
		return $statement->rowCount() > 0;
	}

	/**
	 * Updates the device name.
	 *
	 * @param string $dn user DN
	 * @param string $credentialId credential id
	 * @param string $name new name
	 * @return bool success
	 */
	public function updateDeviceName(string $dn, string $credentialId, $name) {
		$pdo = $this->getPDO();
		$statement = $pdo->prepare('update ' . $this->getTableName() . ' set name = :name where userDN = :userDN and credentialId = :credentialId');
		$statement->execute([
			':userDN' => $dn,
			':credentialId' => $credentialId,
			':name' => $name
		]);
		return $statement->rowCount() > 0;
	}

	/**
	 * Returns the PDO.
	 *
	 * @return PDO PDO
	 */
	abstract protected function getPDO(): PDO;

	/**
	 * Checks if the schema needs to be updated.
	 */
	public function checkSchema(): void {
		try {
			$pdo = $this->getPDO();
		}
		catch (PDOException $e) {
			logNewMessage(LOG_ERR, 'Unable to connect to config database: ' . $e->getMessage());
			return;
		}
		// initial schema
		if (!dbTableExists($pdo, $this->getTableName())) {
			$this->createInitialSchema($pdo);
		}
		$this->addNameColumn($pdo);
		$this->addUserDnColumn($pdo);
	}

	/**
	 * Creates the initial schema.
	 *
	 * @param PDO $pdo PDO object
	 */
	public function createInitialSchema(PDO $pdo): void {
		logNewMessage(LOG_DEBUG, 'Creating database table ' . $this->getTableName());
		$sql = 'create table ' . $this->getTableName() . '('
			. 'userId TEXT NOT NULL,'
			. 'credentialId TEXT NOT NULL,'
			. 'credentialSource TEXT NOT NULL,'
			. 'registrationTime VARCHAR(11) NOT NULL,'
			. 'lastUseTime VARCHAR(11) NOT NULL,'
			. 'PRIMARY KEY(userId,credentialId)'
			. ');';
		$pdo->exec($sql);
	}

	/**
	 * Adds the name column if not existing.
	 *
	 * @param PDO $pdo PDO
	 */
	protected function addNameColumn(PDO $pdo): void {
		try {
			$statement = $pdo->query("select * from pragma_table_info('" . $this->getTableName() . "') where name = 'name';");
			if ($statement === false) {
				throw new PDOException('query failed');
			}
			$results = $statement->fetchAll();
			if (empty($results)) {
				$sql = 'alter table ' . $this->getTableName() . ' add column name VARCHAR(255);';
				logNewMessage(LOG_DEBUG, $sql);
				$pdo->exec($sql);
			}
		}
		catch (PDOException $e) {
			logNewMessage(LOG_ERR, 'Unable to add name column to table: ' . $e->getMessage());
		}
	}

	/**
	 * Adds the user DN column if not existing.
	 *
	 * @param PDO $pdo PDO
	 */
	protected function addUserDnColumn(PDO $pdo): void {
		try {
			$statement = $pdo->query("select * from pragma_table_info('" . $this->getTableName() . "') where name = 'userDN';");
			if ($statement === false) {
				throw new PDOException('query failed');
			}
			$results = $statement->fetchAll();
			if (empty($results)) {
				$sql = 'alter table ' . $this->getTableName() . ' add column userDN VARCHAR(255);';
				logNewMessage(LOG_DEBUG, $sql);
				$pdo->exec($sql);
				$sql = 'update ' . $this->getTableName() . ' set userDn = userId;';
				logNewMessage(LOG_DEBUG, $sql);
				$pdo->exec($sql);
			}
		}
		catch (PDOException $e) {
			logNewMessage(LOG_ERR, 'Unable to add userDN column to table: ' . $e->getMessage());
		}
	}

	/**
	 * Exports all entries.
	 *
	 * @return array<array<string, mixed>> data
	 */
	public function export(): array {
		$pdo = $this->getPDO();
		$statement = $pdo->prepare('select * from ' . $this->getTableName());
		$statement->execute();
		$dbData = $statement->fetchAll();
		$data = [];
		foreach ($dbData as $dbRow) {
			$data[] = [
				'userId' => $dbRow['userId'],
				'credentialId' => $dbRow['credentialId'],
				'credentialSource' => $dbRow['credentialSource'],
				'registrationTime' => $dbRow['registrationTime'],
				'lastUseTime' => $dbRow['lastUseTime'],
				'name' => $dbRow['name'],
				'userDN' => $dbRow['userDN'],
			];
		}
		return $data;
	}

	/**
	 * Imports entries from export data.
	 *
	 * @param null|array<array<string, mixed>> $data export data
	 */
	public function import($data): void {
		$pdo = $this->getPDO();
		$statement = $pdo->prepare('delete from ' . $this->getTableName());
		$statement->execute();
		if ($data === null) {
			return;
		}
		foreach ($data as $dbRow) {
			$statement = $pdo->prepare('insert into ' . $this->getTableName() . ' (userId, credentialId, credentialSource, registrationTime, lastUseTime, name, userDN) VALUES (?, ?, ?, ?, ?, ?, ?)');
			$statement->execute([
				$dbRow['userId'],
				$dbRow['credentialId'],
				$dbRow['credentialSource'],
				$dbRow['registrationTime'],
				$dbRow['lastUseTime'],
				$dbRow['name'],
				$dbRow['userDN'],
			]);
		}
	}

}

/**
 *  Credential repository that uses SQLite for data access.
 *
 * @package LAM\LOGIN\WEBAUTHN
 */
class PublicKeyCredentialSourceRepositorySQLite extends PublicKeyCredentialSourceRepositoryBase {

	/**
	 * Constructor
	 */
	public function __construct() {
		$this->checkSchema();
	}

	/**
	 * {@inheritDoc}
	 */
	protected function getPDO(): PDO {
		return new PDO($this->getPdoUrl(), null, null, [
			PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION
		]);
	}

	/**
	 * Returns the database URL.
	 *
	 * @return string the PDO URL
	 */
	public function getPdoUrl(): string {
		$fileName = __DIR__ . '/../config/__lam.webauthn.sqlite';
		if (!file_exists($fileName)) {
			$handle = fopen($fileName, 'w');
			if ($handle === false) {
				throw new PDOException('Unable to write ' . $fileName);
			}
			fclose($handle);
			chmod($fileName, 0600);
		}
		return 'sqlite:' . $fileName;
	}

}

/**
 * Credential repository that uses a given PDO for data access.
 *
 * @package LAM\LOGIN\WEBAUTHN
 */
class PublicKeyCredentialSourceRepositoryMySql extends PublicKeyCredentialSourceRepositoryBase {

	/** @var PDO */
	private $pdo;

	/**
	 * Constructor
	 *
	 * @param PDO $pdo PDO
	 */
	public function __construct(PDO $pdo) {
		$this->pdo = $pdo;
		$this->checkSchema();
	}

	/**
	 * @inheritDoc
	 */
	protected function getPDO(): PDO {
		return $this->pdo;
	}

	/**
	 * @inheritDoc
	 */
	protected function getTableName(): string {
		return 'webauthn';
	}

	/**
	 * Creates the initial schema.
	 *
	 * @param PDO $pdo PDO object
	 */
	public function createInitialSchema(PDO $pdo): void {
		logNewMessage(LOG_DEBUG, 'Creating database table ' . $this->getTableName());
		$sql = 'create table ' . $this->getTableName() . '('
			// 350 because primary key length is max 3072 bytes and each character can have 4 bytes
			. 'userId VARCHAR(350) NOT NULL,'
			. 'credentialId VARCHAR(350) NOT NULL,'
			. 'credentialSource TEXT NOT NULL,'
			. 'registrationTime VARCHAR(11) NOT NULL,'
			. 'lastUseTime VARCHAR(11) NOT NULL,'
			. 'name VARCHAR(255),'
			. 'PRIMARY KEY(userId,credentialId)'
			. ');';
		$pdo->exec($sql);
		$sql = 'insert into ' . ConfigurationDatabase::TABLE_SCHEMA_VERSIONS . ' (name, version) VALUES ("webauthn", 1);';
		$pdo->exec($sql);
	}

	/**
	 * Adds the name column if not existing.
	 *
	 * @param PDO $pdo PDO
	 */
	protected function addNameColumn(PDO $pdo): void {
		// added via initial schema
	}

	/**
	 * {@inheritDoc}
	 */
	protected function addUserDnColumn(PDO $pdo): void {
		try {
			$statement = $pdo->query("show columns from " . $this->getTableName() . " like 'userDN';");
			if ($statement === false) {
				throw new PDOException('query failed');
			}
			$results = $statement->fetchAll();
			if (empty($results)) {
				$sql = 'alter table ' . $this->getTableName() . ' add column userDN VARCHAR(255);';
				logNewMessage(LOG_DEBUG, $sql);
				$pdo->exec($sql);
				$sql = 'update ' . $this->getTableName() . ' set userDn = userId;';
				logNewMessage(LOG_DEBUG, $sql);
				$pdo->exec($sql);
			}
		}
		catch (PDOException $e) {
			logNewMessage(LOG_ERR, 'Unable to add userDN column to table: ' . $e->getMessage());
		}
	}

}


