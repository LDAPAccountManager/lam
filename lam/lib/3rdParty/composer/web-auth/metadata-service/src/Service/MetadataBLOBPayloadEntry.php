<?php

declare(strict_types=1);

namespace Webauthn\MetadataService\Service;

use function array_key_exists;
use Assert\Assertion;
use function count;
use JsonSerializable;
use LogicException;
use Webauthn\MetadataService\Statement\BiometricStatusReport;
use Webauthn\MetadataService\Statement\MetadataStatement;
use Webauthn\MetadataService\Statement\StatusReport;
use Webauthn\MetadataService\Utils;

class MetadataBLOBPayloadEntry implements JsonSerializable
{
    /**
     * @var string[]
     */
    private array $attestationCertificateKeyIdentifiers = [];

    /**
     * @var BiometricStatusReport[]
     */
    private array $biometricStatusReports = [];

    /**
     * @var StatusReport[]
     */
    private array $statusReports = [];

    /**
     * @param string[] $attestationCertificateKeyIdentifiers
     */
    public function __construct(
        private readonly ?string $aaid,
        private readonly ?string $aaguid,
        array $attestationCertificateKeyIdentifiers,
        private readonly ?MetadataStatement $metadataStatement,
        private readonly string $timeOfLastStatusChange,
        private readonly ?string $rogueListURL,
        private readonly ?string $rogueListHash
    ) {
        if ($aaid !== null && $aaguid !== null) {
            throw new LogicException('Authenticators cannot support both AAID and AAGUID');
        }
        if ($aaid === null && $aaguid === null && count($attestationCertificateKeyIdentifiers) === 0) {
            throw new LogicException(
                'If neither AAID nor AAGUID are set, the attestation certificate identifier list shall not be empty'
            );
        }
        foreach ($attestationCertificateKeyIdentifiers as $attestationCertificateKeyIdentifier) {
            Assertion::string(
                $attestationCertificateKeyIdentifier,
                'Invalid attestation certificate identifier. Shall be a list of strings'
            );
            Assertion::notEmpty(
                $attestationCertificateKeyIdentifier,
                'Invalid attestation certificate identifier. Shall be a list of strings'
            );
            Assertion::regex(
                $attestationCertificateKeyIdentifier,
                '/^[0-9a-f]+$/',
                'Invalid attestation certificate identifier. Shall be a list of strings'
            );
        }
        $this->attestationCertificateKeyIdentifiers = $attestationCertificateKeyIdentifiers;
    }

    public function getAaid(): ?string
    {
        return $this->aaid;
    }

    public function getAaguid(): ?string
    {
        return $this->aaguid;
    }

    /**
     * @return string[]
     */
    public function getAttestationCertificateKeyIdentifiers(): array
    {
        return $this->attestationCertificateKeyIdentifiers;
    }

    public function getMetadataStatement(): ?MetadataStatement
    {
        return $this->metadataStatement;
    }

    public function addBiometricStatusReports(BiometricStatusReport ...$biometricStatusReports): self
    {
        foreach ($biometricStatusReports as $biometricStatusReport) {
            $this->biometricStatusReports[] = $biometricStatusReport;
        }

        return $this;
    }

    /**
     * @return BiometricStatusReport[]
     */
    public function getBiometricStatusReports(): array
    {
        return $this->biometricStatusReports;
    }

    public function addStatusReports(StatusReport ...$statusReports): self
    {
        foreach ($statusReports as $statusReport) {
            $this->statusReports[] = $statusReport;
        }

        return $this;
    }

    /**
     * @return StatusReport[]
     */
    public function getStatusReports(): array
    {
        return $this->statusReports;
    }

    public function getTimeOfLastStatusChange(): string
    {
        return $this->timeOfLastStatusChange;
    }

    public function getRogueListURL(): string|null
    {
        return $this->rogueListURL;
    }

    public function getRogueListHash(): string|null
    {
        return $this->rogueListHash;
    }

    /**
     * @param array<string, mixed> $data
     */
    public static function createFromArray(array $data): self
    {
        $data = Utils::filterNullValues($data);
        Assertion::keyExists(
            $data,
            'timeOfLastStatusChange',
            'Invalid data. The parameter "timeOfLastStatusChange" is missing'
        );
        Assertion::keyExists($data, 'statusReports', 'Invalid data. The parameter "statusReports" is missing');
        Assertion::isArray(
            $data['statusReports'],
            'Invalid data. The parameter "statusReports" shall be an array of StatusReport objects'
        );
        $object = new self(
            $data['aaid'] ?? null,
            $data['aaguid'] ?? null,
            $data['attestationCertificateKeyIdentifiers'] ?? [],
            isset($data['metadataStatement']) ? MetadataStatement::createFromArray($data['metadataStatement']) : null,
            $data['timeOfLastStatusChange'],
            $data['rogueListURL'] ?? null,
            $data['rogueListHash'] ?? null
        );
        foreach ($data['statusReports'] as $statusReport) {
            $object->addStatusReports(StatusReport::createFromArray($statusReport));
        }
        if (array_key_exists('biometricStatusReport', $data)) {
            Assertion::isArray(
                $data['biometricStatusReport'],
                'Invalid data. The parameter "biometricStatusReport" shall be an array of BiometricStatusReport objects'
            );
            foreach ($data['biometricStatusReport'] as $biometricStatusReport) {
                $object->addBiometricStatusReports(BiometricStatusReport::createFromArray($biometricStatusReport));
            }
        }

        return $object;
    }

    /**
     * @return array<string, mixed>
     */
    public function jsonSerialize(): array
    {
        $data = [
            'aaid' => $this->aaid,
            'aaguid' => $this->aaguid,
            'attestationCertificateKeyIdentifiers' => $this->attestationCertificateKeyIdentifiers,
            'statusReports' => array_map(
                static fn (StatusReport $object): array => $object->jsonSerialize(),
                $this->statusReports
            ),
            'timeOfLastStatusChange' => $this->timeOfLastStatusChange,
            'rogueListURL' => $this->rogueListURL,
            'rogueListHash' => $this->rogueListHash,
        ];

        return Utils::filterNullValues($data);
    }
}
