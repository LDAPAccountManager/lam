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
use \Cose\Algorithms;
use Http\Discovery\Psr17Factory;
use LAM\PERSISTENCE\ConfigurationDatabase;
use LAMCfgMain;
use PDO;
use PDOException;
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
use function LAM\PERSISTENCE\dbTableExists;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2019 - 2023  Roland Gruber

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
	 * @param array $extraExcludedKeys credentialIds that should be added to excluded keys
	 * @return PublicKeyCredentialCreationOptions registration object
	 */
	public function getRegistrationObject($dn, $isSelfService, $extraExcludedKeys = []) {
		$rpEntity = $this->createRpEntry($isSelfService);
		$userEntity = $this->getUserEntity($dn);
		$challenge = $this->createChallenge();
		$credentialParameters = $this->getCredentialParameters();
		$excludedKeys = $this->getExcludedKeys($userEntity, $extraExcludedKeys);
		$timeout = $this->getTimeout();
		$authenticatorSelectionCriteria = new AuthenticatorSelectionCriteria(null, false, AuthenticatorSelectionCriteria::USER_VERIFICATION_REQUIREMENT_DISCOURAGED);
		$registrationObject = new PublicKeyCredentialCreationOptions(
			$rpEntity,
			$userEntity,
			$challenge,
			$credentialParameters,
			$timeout,
			$excludedKeys,
			$authenticatorSelectionCriteria,
			PublicKeyCredentialCreationOptions::ATTESTATION_CONVEYANCE_PREFERENCE_NONE,
			new AuthenticationExtensionsClientInputs());
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
		$tokenBindingHandler = new IgnoreTokenBindingHandler();
		$attestationSupportManager = $this->getAttestationSupportManager();
		$attestationObjectLoader = $this->getAttestationObjectLoader($attestationSupportManager);
		$publicKeyCredentialLoader = $this->getPublicKeyCredentialLoader($attestationObjectLoader);
		$extensionOutputCheckerHandler = $this->getExtensionOutputChecker();
		$repository = $this->getDatabase();
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
			logNewMessage(LOG_ERR, 'WebAuthn validation failed: ' . $exception->getMessage() . $exception->getTraceAsString());
		}
		return false;
	}

	/**
	 * Returns a public key credential loader.
	 *
	 * @return PublicKeyCredentialLoader public key credential loader
	 */
	public function createPublicKeyCredentialLoader() {
		$attestationSupportManager = $this->getAttestationSupportManager();
		$attestationObjectLoader = $this->getAttestationObjectLoader($attestationSupportManager);
		return $this->getPublicKeyCredentialLoader($attestationObjectLoader);
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
		if ($isSelfService) {
			$domain = $_SESSION['selfServiceProfile']->twoFactorAuthenticationDomain;
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
	 * @param array $extraExcludedKeys credentialIds that should be added to excluded keys
	 * @return PublicKeyCredentialDescriptor[] credential ids
	 */
	private function getExcludedKeys($user, $extraExcludedKeys = []) {
		$keys = [];
		$repository = $this->getDatabase();
		$credentialSources = $repository->findAllForUserEntity($user);
		foreach ($credentialSources as $credentialSource) {
			$keys[] = new PublicKeyCredentialDescriptor(PublicKeyCredentialDescriptor::CREDENTIAL_TYPE_PUBLIC_KEY, $credentialSource->getPublicKeyCredentialId());
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
	 * Returns the attestation object loader.
	 *
	 * @param AttestationStatementSupportManager $manager support manager
	 * @return AttestationObjectLoader attestation object loader
	 */
	private function getAttestationObjectLoader($manager) {
		return new AttestationObjectLoader($manager);
	}

	/**
	 * Creates the public key credential loader.
	 *
	 * @param AttestationObjectLoader $attestationObjectLoader attestation object loader
	 * @return PublicKeyCredentialLoader public key credential loader
	 */
	private function getPublicKeyCredentialLoader($attestationObjectLoader) {
		return new PublicKeyCredentialLoader($attestationObjectLoader);
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
			throw new LAMException(_('Unable to connect to database.'), $e->getMessage());
		}
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
		return generateRandomPassword(32, false);
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
		$publicKeyDescriptors = [];
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
		$tokenBindingHandler = new IgnoreTokenBindingHandler();
		$extensionOutputCheckerHandler = $this->getExtensionOutputChecker();
		$algorithmManager = $this->getAlgorithmManager();
		$responseValidator = new AuthenticatorAssertionResponseValidator(
			$database,
			$tokenBindingHandler,
			$extensionOutputCheckerHandler,
			$algorithmManager
		);
		$attestationSupportManager = $this->getAttestationSupportManager();
		$attestationObjectLoader = $this->getAttestationObjectLoader($attestationSupportManager);
		$publicKeyCredentialLoader = $this->getPublicKeyCredentialLoader($attestationObjectLoader);
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
abstract class PublicKeyCredentialSourceRepositoryBase implements PublicKeyCredentialSourceRepository {

	/**
	 * Returns the table name to use.
	 *
	 * @return string table name
	 */
	protected function getTableName() : string {
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
			$statement->execute(array(':credentialid' => base64_encode($publicKeyCredentialId)));
			$results = $statement->fetchAll();
			if (!empty($results)) {
				$jsonArray = json_decode($results[0]['credentialSource'], true);
				return PublicKeyCredentialSource::createFromArray($jsonArray);
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
	 * @param PublicKeyCredentialUserEntity $publicKeyCredentialUserEntity credential user entity
	 * @return PublicKeyCredentialSource[] credential sources
	 */
	public function findAllForUserEntity(PublicKeyCredentialUserEntity $publicKeyCredentialUserEntity): array {
		$credentials = [];
		try {
			$pdo = $this->getPDO();
			$statement = $pdo->prepare('select * from ' . $this->getTableName() . ' where userId = :userid');
			$statement->execute(array(':userid' => $publicKeyCredentialUserEntity->getId()));
			$results = $statement->fetchAll();
			foreach ($results as $result) {
				$jsonArray = json_decode($result['credentialSource'], true);
				$credentials[] = PublicKeyCredentialSource::createFromArray($jsonArray);
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
		$credentialId = base64_encode($publicKeyCredentialSource->getPublicKeyCredentialId());
		$userId = $publicKeyCredentialSource->getUserHandle();
		$currentTime = time();
		$pdo = $this->getPDO();
		$statement = $pdo->prepare('select * from ' . $this->getTableName() . ' where userId = :userId and credentialId = :credentialId');
		$statement->execute(array(
			':userId' => $userId,
			':credentialId' => $credentialId
		));
		$results = $statement->fetchAll();
		if (empty($results)) {
			$statement = $pdo->prepare('insert into ' . $this->getTableName() . ' (userId, credentialId, credentialSource, registrationTime, lastUseTime) VALUES (?, ?, ?, ?, ?)');
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
				'update ' . $this->getTableName() .
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
	 * Performs a full-text search on the user names and returns all devices found.
	 *
	 * @param string $searchTerm search term for user field
	 * @return array list of devices array('dn' => ..., 'credentialId' => ..., 'lastUseTime' => ..., 'registrationTime' => ...)
	 */
	public function searchDevices(string $searchTerm) {
		$pdo = $this->getPDO();
		$statement = $pdo->prepare('select * from ' . $this->getTableName() . ' where userId like :searchTerm order by userId,registrationTime');
		$statement->execute(array(
			':searchTerm' => $searchTerm
		));
		$results = $statement->fetchAll();
		$devices = [];
		foreach ($results as $result) {
			$name = !empty($result['name']) ? $result['name'] : '';
			$devices[] = array(
				'dn' => $result['userId'],
				'credentialId' => $result['credentialId'],
				'lastUseTime' => $result['lastUseTime'],
				'registrationTime' => $result['registrationTime'],
				'name' => $name
			);
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
		$statement = $pdo->prepare('delete from ' . $this->getTableName() . ' where userId = :userId and credentialId = :credentialId');
		$statement->execute(array(
			':userId' => $dn,
			':credentialId' => $credentialId
		));
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
		$statement = $pdo->prepare('update ' . $this->getTableName() . ' set name = :name where userId = :userId and credentialId = :credentialId');
		$statement->execute(array(
			':userId' => $dn,
			':credentialId' => $credentialId,
			':name' => $name
		));
		return $statement->rowCount() > 0;
	}

	/**
	 * Returns the PDO.
	 *
	 * @return PDO PDO
	 */
	protected abstract function getPDO(): PDO;

	/**
	 * Checks if the schema needs to be updated.
	 */
	public function checkSchema() :void {
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
	}

	/**
	 * Creates the initial schema.
	 *
	 * @param PDO $pdo PDO object
	 */
	public function createInitialSchema(PDO $pdo) {
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
	protected function addNameColumn(PDO $pdo) {
		try {
			$statement = $pdo->query("select * from pragma_table_info('" . $this->getTableName() . "') where name = 'name';");
			$results = $statement->fetchAll();
			if (empty($results)) {
				$sql = 'alter table ' . $this->getTableName() . ' add column name VARCHAR(255);';
				logNewMessage(LOG_DEBUG, $sql);
				$pdo->exec($sql);
			}
		} catch (PDOException $e) {
			logNewMessage(LOG_ERR, 'Unable to add name column to table: ' . $e->getMessage());
		}
	}

	/**
	 * Exports all entries.
	 *
	 * @return array data
	 */
	public function export() {
		$pdo = $this->getPDO();
		$statement = $pdo->prepare('select * from ' . $this->getTableName());
		$statement->execute();
		$dbData = $statement->fetchAll();
		$data = [];
		foreach ($dbData as $dbRow) {
			$data[] = array(
				'userId' => $dbRow['userId'],
				'credentialId' => $dbRow['credentialId'],
				'credentialSource' => $dbRow['credentialSource'],
				'registrationTime' => $dbRow['registrationTime'],
				'lastUseTime' => $dbRow['lastUseTime'],
				'name' => $dbRow['name'],
			);
		}
		return $data;
	}

	/**
	 * Imports entries from export data.
	 *
	 * @param array $data export data
	 */
	public function import($data) {
		$pdo = $this->getPDO();
		$statement = $pdo->prepare('delete from ' . $this->getTableName());
		$statement->execute();
		if ($data === null) {
			return;
		}
		foreach ($data as $dbRow) {
			$statement = $pdo->prepare('insert into ' . $this->getTableName() . ' (userId, credentialId, credentialSource, registrationTime, lastUseTime, name) VALUES (?, ?, ?, ?, ?, ?)');
			$statement->execute(array(
				$dbRow['userId'],
				$dbRow['credentialId'],
				$dbRow['credentialSource'],
				$dbRow['registrationTime'],
				$dbRow['lastUseTime'],
				$dbRow['name']
			));
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
		return new PDO($this->getPdoUrl(), null, null, array(
			PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION
		));
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
	protected function getTableName() : string {
		return 'webauthn';
	}

	/**
	 * Creates the initial schema.
	 *
	 * @param PDO $pdo PDO object
	 */
	public function createInitialSchema(PDO $pdo) {
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
	protected function addNameColumn(PDO $pdo) {
		// added via initial schema
	}

}


