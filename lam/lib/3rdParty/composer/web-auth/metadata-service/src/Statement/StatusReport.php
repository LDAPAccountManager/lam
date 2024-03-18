<?php

declare(strict_types=1);

namespace Webauthn\MetadataService\Statement;

use Assert\Assertion;
use function in_array;
use JsonSerializable;
use Webauthn\MetadataService\Utils;

class StatusReport implements JsonSerializable
{
    /**
     * @see AuthenticatorStatus
     */
    private readonly string $status;

    public function __construct(
        string $status,
        private readonly ?string $effectiveDate,
        private readonly ?string $certificate,
        private readonly ?string $url,
        private readonly ?string $certificationDescriptor,
        private readonly ?string $certificateNumber,
        private readonly ?string $certificationPolicyVersion,
        private readonly ?string $certificationRequirementsVersion
    ) {
        Assertion::inArray($status, AuthenticatorStatus::list(), 'The value of the key "status" is not acceptable');

        $this->status = $status;
    }

    public function isCompromised(): bool
    {
        return in_array($this->status, [
            AuthenticatorStatus::ATTESTATION_KEY_COMPROMISE,
            AuthenticatorStatus::USER_KEY_PHYSICAL_COMPROMISE,
            AuthenticatorStatus::USER_KEY_REMOTE_COMPROMISE,
            AuthenticatorStatus::USER_VERIFICATION_BYPASS,
        ], true);
    }

    public function getStatus(): string
    {
        return $this->status;
    }

    public function getEffectiveDate(): ?string
    {
        return $this->effectiveDate;
    }

    public function getCertificate(): ?string
    {
        return $this->certificate;
    }

    public function getUrl(): ?string
    {
        return $this->url;
    }

    public function getCertificationDescriptor(): ?string
    {
        return $this->certificationDescriptor;
    }

    public function getCertificateNumber(): ?string
    {
        return $this->certificateNumber;
    }

    public function getCertificationPolicyVersion(): ?string
    {
        return $this->certificationPolicyVersion;
    }

    public function getCertificationRequirementsVersion(): ?string
    {
        return $this->certificationRequirementsVersion;
    }

    /**
     * @param array<string, mixed> $data
     */
    public static function createFromArray(array $data): self
    {
        $data = Utils::filterNullValues($data);
        Assertion::keyExists($data, 'status', 'The key "status" is missing');
        foreach ([
            'effectiveDate',
            'certificate',
            'url',
            'certificationDescriptor',
            'certificateNumber',
            'certificationPolicyVersion',
            'certificationRequirementsVersion',
        ] as $key) {
            if (isset($data[$key])) {
                Assertion::nullOrString($data[$key], sprintf('The value of the key "%s" is invalid', $key));
            }
        }

        return new self(
            $data['status'],
            $data['effectiveDate'] ?? null,
            $data['certificate'] ?? null,
            $data['url'] ?? null,
            $data['certificationDescriptor'] ?? null,
            $data['certificateNumber'] ?? null,
            $data['certificationPolicyVersion'] ?? null,
            $data['certificationRequirementsVersion'] ?? null
        );
    }

    /**
     * @return array<string, mixed>
     */
    public function jsonSerialize(): array
    {
        $data = [
            'status' => $this->status,
            'effectiveDate' => $this->effectiveDate,
            'certificate' => $this->certificate,
            'url' => $this->url,
            'certificationDescriptor' => $this->certificationDescriptor,
            'certificateNumber' => $this->certificateNumber,
            'certificationPolicyVersion' => $this->certificationPolicyVersion,
            'certificationRequirementsVersion' => $this->certificationRequirementsVersion,
        ];

        return Utils::filterNullValues($data);
    }
}
