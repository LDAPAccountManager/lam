<?php

declare(strict_types=1);

/**
 * Signature.php
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

namespace Com\Tecnick\Pdf\Sign\Output;

use Com\Tecnick\Pdf\Sign\Exception;

/**
 * Com\Tecnick\Pdf\Sign\Output\Signature
 *
 * Emits the /Sig value dictionary (the object referenced by a signature field's
 * /V): the fixed skeleton, the /SubFilter, and the /ByteRange and /Contents
 * placeholders that the host rewrites while signing, plus the optional
 * Name/Location/Reason/ContactInfo strings.
 *
 * The /Reference (DocMDP or UR3 transform) and the /M date token are supplied by
 * the caller as ready fragments, their content and formatting depending on host
 * state (certification level, user rights, timezone, encryption). String encoding
 * (escaping, UTF-16, encryption) of the info values is delegated to an injected
 * encoder.
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
final class Signature
{
    /**
     * ByteRange placeholder rewritten by the host once the byte offsets are known.
     */
    public const BYTE_RANGE_PLACEHOLDER = '/ByteRange[0 ********** ********** **********]';

    /**
     * Default number of hex zero placeholder characters reserved for /Contents.
     */
    public const DEFAULT_CONTENTS_LENGTH = 11_742;

    /**
     * Largest /Contents placeholder this emitter reserves, in hex characters.
     *
     * The window holds one CMS signature or one RFC 3161 token, so a megabyte of
     * hex is far beyond what either needs. Past it str_repeat() would exhaust memory
     * rather than throw.
     */
    public const MAX_CONTENTS_LENGTH = 1_048_576;

    /**
     * Info string entries, in output order.
     *
     * @var list<string>
     */
    private const INFO_KEYS = ['Name', 'Location', 'Reason', 'ContactInfo'];

    /**
     * Emit the /Sig value object.
     *
     * @param int                  $objectId       Object number for the /Sig value object.
     * @param string               $subFilter      e.g. "ETSI.CAdES.detached" or "adbe.pkcs7.detached".
     * @param string               $reference      Ready /Reference fragment (DocMDP or UR3 transform),
     *                                             leading space included, or '' for an approval signature.
     * @param array<string, string> $info          Optional Name/Location/Reason/ContactInfo.
     * @param string               $dateValue      Ready (already encoded) PDF string token for /M,
     *                                             or '' to omit the entry.
     * @param int                  $contentsLength Placeholder length for /Contents, in hex
     *                                             digits; must be even and positive.
     * @param callable|null        $stringEncoder  fn(string $text, int $objectId): string returning a PDF string token.
     *
     * @throws Exception If the object number, the /SubFilter, an info value, or the
     *                   placeholder length is invalid, or the string encoder returns a
     *                   non-string value.
     */
    public function valueObject(
        int $objectId,
        string $subFilter,
        string $reference,
        array $info,
        string $dateValue,
        int $contentsLength = self::DEFAULT_CONTENTS_LENGTH,
        ?callable $stringEncoder = null,
    ): string {
        $out = self::objectHead($objectId, 'Sig', $subFilter, $contentsLength, 'signature');
        $out .= $reference;

        foreach (self::INFO_KEYS as $key) {
            // A value that is not a string would reach the encoder as a TypeError
            // rather than an Exception.
            /** @var mixed $value */
            $value = $info[$key] ?? '';
            if (!\is_string($value)) {
                throw new Exception('Invalid signature info value: ' . $key);
            }

            if ($value !== '') {
                $out .= ' /' . $key . ' ' . PdfString::encode($value, $objectId, $stringEncoder);
            }
        }

        // As with the info strings, an empty value omits the entry.
        if ($dateValue !== '') {
            $out .= ' /M ' . $dateValue;
        }

        return $out . " >>\nendobj\n";
    }

    /**
     * Emit the head of a signature value object, up to and including /Contents.
     *
     * ISO 32000-2 section 12.8.5 gives a /Sig and a /DocTimeStamp the same shape:
     * the Adobe.PPKLite filter, the ByteRange placeholder the host rewrites once it
     * knows the offsets, and a /Contents window reserved as hex zeros. Emitted here
     * for both.
     *
     * @param int    $objectId       Object number for the value object.
     * @param string $type           /Type name, without the leading solidus.
     * @param string $subFilter      /SubFilter name, without the leading solidus.
     * @param int    $contentsLength Placeholder length reserved for the signature, in
     *                               hex digits; must be even and positive.
     * @param string $label          Name of the object, for the error message.
     *
     * @throws Exception If a name, the object number, or the placeholder length is invalid.
     */
    public static function objectHead(
        int $objectId,
        string $type,
        string $subFilter,
        int $contentsLength,
        string $label,
    ): string {
        // Both names are interpolated into the dictionary as written, so they are
        // held to the name character set, as Widget::annotation() holds its own two.
        foreach (['/Type' => $type, '/SubFilter' => $subFilter] as $entry => $name) {
            // \z rather than $, which PCRE matches before a final newline.
            if (\preg_match('/^[A-Za-z0-9.]+\z/', $name) !== 1) {
                throw new Exception('Invalid ' . $entry . ': ' . $name);
            }
        }

        // ISO 32000-1 section 7.3.10 admits only positive object numbers. The number
        // is also what the string encoder derives an encryption key from.
        if ($objectId < 1) {
            throw new Exception('Invalid ' . $label . ' object number: ' . $objectId);
        }

        // A hex string with an odd number of digits is read with a trailing zero
        // appended (ISO 32000-1 section 7.3.4.3), so the reserved window and the
        // signature written into it would disagree by half a byte.
        if ($contentsLength < 2 || $contentsLength > self::MAX_CONTENTS_LENGTH || ($contentsLength % 2) !== 0) {
            throw new Exception('Invalid /Contents placeholder length: ' . $contentsLength);
        }

        return (
            $objectId
            . " 0 obj\n"
            . '<< /Type /'
            . $type
            . ' /Filter /Adobe.PPKLite /SubFilter /'
            . $subFilter
            . ' '
            . self::BYTE_RANGE_PLACEHOLDER
            . ' /Contents<'
            . \str_repeat('0', $contentsLength)
            . '>'
        );
    }
}
