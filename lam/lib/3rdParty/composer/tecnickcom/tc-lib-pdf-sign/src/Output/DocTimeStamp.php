<?php

declare(strict_types=1);

/**
 * DocTimeStamp.php
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

/**
 * Com\Tecnick\Pdf\Sign\Output\DocTimeStamp
 *
 * Emits a document timestamp value object (/Type /DocTimeStamp,
 * /SubFilter /ETSI.RFC3161) whose /Contents is a bare RFC 3161 timestamp token.
 * It is added in an incremental update to reach PAdES B-LTA. It shares the
 * /ByteRange and /Contents placeholders with the signature value object so the
 * host's signing pass locates them the same way for either object type.
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
final class DocTimeStamp
{
    /**
     * SubFilter for an RFC 3161 document timestamp.
     */
    public const SUB_FILTER = 'ETSI.RFC3161';

    /**
     * Emit the /DocTimeStamp value object.
     *
     * @param int $objectId       Object number for the value object.
     * @param int $contentsLength Placeholder length reserved for the token.
     */
    public function valueObject(int $objectId, int $contentsLength = Signature::DEFAULT_CONTENTS_LENGTH): string
    {
        $out = $objectId . " 0 obj\n";
        $out .= '<< /Type /DocTimeStamp /Filter /Adobe.PPKLite /SubFilter /' . self::SUB_FILTER . ' ';
        $out .= Signature::BYTE_RANGE_PLACEHOLDER;
        $out .= ' /Contents<' . \str_repeat('0', \max(0, $contentsLength)) . '>';

        return $out . " >>\nendobj\n";
    }
}
