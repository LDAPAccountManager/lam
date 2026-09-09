<?php

declare(strict_types=1);

/**
 * Config.php
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 *
 * This file is part of tc-lib-pdf-sign software library.
 */

namespace Com\Tecnick\Pdf\Sign;

/**
 * Com\Tecnick\Pdf\Sign\Config
 *
 * Immutable signature configuration value object. Captures the signing profile,
 * digest algorithm, and certification level, and derives the PDF /SubFilter
 * from the selected profile.
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
final class Config
{
    /**
     * Legacy ISO 32000-1 signature (/SubFilter /adbe.pkcs7.detached).
     */
    public const PROFILE_LEGACY = SignatureProfile::Legacy->value;

    /**
     * PAdES baseline B-B (CAdES-based, /SubFilter /ETSI.CAdES.detached).
     */
    public const PROFILE_PADES_B_B = SignatureProfile::PadesBB->value;

    /**
     * PAdES baseline B-T (B-B plus a signature timestamp).
     */
    public const PROFILE_PADES_B_T = SignatureProfile::PadesBT->value;

    /**
     * PAdES baseline B-LT (B-T plus a Document Security Store).
     */
    public const PROFILE_PADES_B_LT = SignatureProfile::PadesBLT->value;

    /**
     * PAdES baseline B-LTA (B-LT plus a document timestamp).
     */
    public const PROFILE_PADES_B_LTA = SignatureProfile::PadesBLTA->value;

    /**
     * Supported signature profiles.
     *
     * Derived from SignatureProfile, which is the closed set the constructor
     * enforces.
     *
     * @var list<string>
     */
    public const PROFILES = [
        SignatureProfile::Legacy->value,
        SignatureProfile::PadesBB->value,
        SignatureProfile::PadesBT->value,
        SignatureProfile::PadesBLT->value,
        SignatureProfile::PadesBLTA->value,
    ];

    /**
     * Supported CMS digest algorithms.
     *
     * Derived from DigestAlgorithm, as PROFILES is from SignatureProfile.
     *
     * @var list<string>
     */
    public const DIGEST_ALGORITHMS = [
        DigestAlgorithm::Sha256->value,
        DigestAlgorithm::Sha384->value,
        DigestAlgorithm::Sha512->value,
    ];

    /**
     * Selected signature profile (one of the PROFILE_* constants).
     */
    public readonly string $profile;

    /**
     * Selected CMS digest algorithm (one of the DigestAlgorithm backing values).
     */
    public readonly string $digestAlgorithm;

    /**
     * @param string|SignatureProfile $profile         Profile identifier or enum case.
     * @param string|DigestAlgorithm  $digestAlgorithm Digest algorithm name or enum case.
     * @param int                     $certType        Certification level (DocMDP P value):
     *                                                 0 = approval/UR signature,
     *                                                 1 = no changes permitted,
     *                                                 2 = form fill-in and signing permitted,
     *                                                 3 = as 2 plus annotation changes.
     *
     * @throws Exception If any option is invalid.
     */
    public function __construct(
        string|SignatureProfile $profile = self::PROFILE_LEGACY,
        string|DigestAlgorithm $digestAlgorithm = 'sha256',
        public readonly int $certType = 2,
    ) {
        // The enums are the closed set for both options and throw on anything
        // outside it.
        $selectedProfile = SignatureProfile::fromLoose($profile);
        $selectedDigest = DigestAlgorithm::fromLoose($digestAlgorithm);

        if ($certType < 0 || $certType > 3) {
            throw new Exception('Invalid certification level (cert_type): ' . $certType);
        }

        $this->profile = $selectedProfile->value;
        $this->digestAlgorithm = $selectedDigest->value;
    }

    /**
     * True when a PAdES profile is selected.
     */
    public function isPades(): bool
    {
        return $this->profile !== self::PROFILE_LEGACY;
    }

    /**
     * PDF /SubFilter value for the selected profile.
     */
    public function subFilter(): string
    {
        return $this->isPades() ? 'ETSI.CAdES.detached' : 'adbe.pkcs7.detached';
    }

    /**
     * Build a Config from the associative-array shape used by Tcpdf::setSignature().
     *
     * @param array<string, mixed> $data Signature options.
     *
     * @throws Exception If any option is present but of the wrong type or value.
     */
    public static function fromArray(array $data): self
    {
        /** @var mixed $profile */
        $profile = $data['profile'] ?? self::PROFILE_LEGACY;
        if (!\is_string($profile) && !$profile instanceof SignatureProfile) {
            throw new Exception('Invalid signature profile');
        }

        /** @var mixed $digest */
        $digest = $data['digest_algorithm'] ?? 'sha256';
        if (!\is_string($digest) && !$digest instanceof DigestAlgorithm) {
            throw new Exception('Invalid digest algorithm');
        }

        /** @var mixed $certType */
        $certType = $data['cert_type'] ?? 2;
        if (!\is_int($certType)) {
            throw new Exception('Invalid certification level (cert_type)');
        }

        return new self($profile, $digest, $certType);
    }
}
