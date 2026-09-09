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

use Com\Tecnick\Pdf\Sign\Exception;

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
     * @param int $contentsLength Placeholder length reserved for the token, in hex
     *                            digits; must be even and positive.
     *
     * @throws Exception If the object number or the placeholder length is invalid.
     */
    public function valueObject(int $objectId, int $contentsLength = Signature::DEFAULT_CONTENTS_LENGTH): string
    {
        // ISO 32000-2 section 12.8.5 gives a document timestamp the same shape as a
        // signature, so the head is the one Signature emits. A /DocTimeStamp carries
        // no further entries.
        $out = Signature::objectHead(
            $objectId,
            'DocTimeStamp',
            self::SUB_FILTER,
            $contentsLength,
            'document timestamp',
        );

        return $out . " >>\nendobj\n";
    }
}
