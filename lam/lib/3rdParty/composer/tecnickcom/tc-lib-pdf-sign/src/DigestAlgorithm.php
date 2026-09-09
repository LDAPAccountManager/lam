<?php

declare(strict_types=1);

/**
 * DigestAlgorithm.php
 *
 * @since     2026-07-17
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
 * Com\Tecnick\Pdf\Sign\DigestAlgorithm
 *
 * Backed enum for the supported message-digest algorithms. It is the single
 * closed set for both the CMS builder and the RFC 3161 message imprint. The
 * backing value is the lowercase algorithm name accepted by both.
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
enum DigestAlgorithm: string
{
    case Sha256 = 'sha256';

    case Sha384 = 'sha384';

    case Sha512 = 'sha512';

    /**
     * Resolve a loose digest algorithm value to the matching enum case.
     *
     * Accepts the canonical algorithm string (as validated by Config and
     * Timestamp\Config) or an enum instance (returned unchanged). Unknown values
     * throw, matching the closed set enforced by both configs.
     *
     * @param string|self $value Digest algorithm name or enum case.
     *
     * @throws Exception if the value does not match a known digest algorithm.
     */
    public static function fromLoose(string|self $value): self
    {
        if ($value instanceof self) {
            return $value;
        }

        return self::tryFrom($value) ?? throw new Exception('Invalid digest algorithm: ' . $value);
    }

    /**
     * The backing value of every case, in declaration order.
     *
     * @return list<string>
     */
    public static function values(): array
    {
        return \array_map(static fn(self $case): string => $case->value, self::cases());
    }

    /**
     * Resolve a digest AlgorithmIdentifier OID to the matching enum case.
     *
     * @return self|null Null when the OID names no supported algorithm.
     */
    public static function tryFromOid(string $oid): ?self
    {
        foreach (self::cases() as $case) {
            if ($case->oid() === $oid) {
                return $case;
            }
        }

        return null;
    }

    /**
     * Length in bytes of a raw digest produced by this algorithm.
     */
    public function digestLength(): int
    {
        return match ($this) {
            self::Sha256 => 32,
            self::Sha384 => 48,
            self::Sha512 => 64,
        };
    }

    /**
     * OID of this algorithm's AlgorithmIdentifier (NIST, RFC 5754 section 2).
     *
     * Read by the CMS builder, the CMS reader, and the RFC 3161 request.
     */
    public function oid(): string
    {
        return match ($this) {
            self::Sha256 => '2.16.840.1.101.3.4.2.1',
            self::Sha384 => '2.16.840.1.101.3.4.2.2',
            self::Sha512 => '2.16.840.1.101.3.4.2.3',
        };
    }
}
