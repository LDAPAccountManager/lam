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
 * the caller as ready fragments, because their content and formatting depend on
 * host state (certification level, user rights, timezone, encryption). This keeps
 * the byte skeleton and the signing-critical placeholders in one place while
 * letting the host own the semantic parts. String encoding (escaping, UTF-16,
 * encryption) of the info values is delegated to an injected encoder.
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
     * @param string               $dateValue      Ready (already encoded) PDF string token for /M.
     * @param int                  $contentsLength Placeholder length for /Contents.
     * @param callable|null        $stringEncoder  fn(string $text, int $objectId): string returning a PDF string token.
     *
     * @throws Exception If the string encoder returns a non-string value.
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
        $out = $objectId . " 0 obj\n";
        $out .= '<< /Type /Sig /Filter /Adobe.PPKLite /SubFilter /' . $subFilter . ' ';
        $out .= self::BYTE_RANGE_PLACEHOLDER;
        $out .= ' /Contents<' . \str_repeat('0', \max(0, $contentsLength)) . '>';
        $out .= $reference;

        foreach (self::INFO_KEYS as $key) {
            $value = $info[$key] ?? '';
            if ($value !== '') {
                $out .= ' /' . $key . ' ' . PdfString::encode($value, $objectId, $stringEncoder);
            }
        }

        return $out . ' /M ' . $dateValue . " >>\nendobj\n";
    }
}
