<?php

declare(strict_types=1);

/**
 * SkipReason.php
 *
 * @since     2026-08-24
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 *
 * This file is part of tc-lib-pdf-sign software library.
 */

namespace Com\Tecnick\Pdf\Sign\Ltv;

use Com\Tecnick\Pdf\Sign\Exception;
use Com\Tecnick\Pdf\Sign\RevokedException;

/**
 * Com\Tecnick\Pdf\Sign\Ltv\SkipReason
 *
 * Machine-readable classification of a discarded revocation URL, passed to the
 * $onSkip observer alongside the human-readable message.
 *
 * Revoked stands apart from the rest: it is an answer the responder gave, not a
 * failure to obtain one.
 *
 * @since     2026-08-24
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
enum SkipReason: string
{
    /**
     * The responder or the CRL states that the certificate is revoked.
     */
    case Revoked = 'revoked';

    /**
     * The answer was received but rejected: malformed, unsigned, stale, or for
     * another certificate.
     */
    case Invalid = 'invalid';

    /**
     * The transport did not produce an answer.
     */
    case Unreachable = 'unreachable';

    /**
     * The answer was byte-identical to one an earlier URL already produced.
     */
    case Duplicate = 'duplicate';

    /**
     * No lookup was attempted, the material needed to check the answer not being
     * available.
     */
    case NotAttempted = 'not-attempted';

    /**
     * Classify the failure a fetch callback raised.
     *
     * A library Exception means the answer arrived and was refused; anything
     * else came out of the host's transport and means no answer arrived.
     */
    public static function fromThrowable(\Throwable $error): self
    {
        return match (true) {
            $error instanceof RevokedException => self::Revoked,
            $error instanceof Exception => self::Invalid,
            default => self::Unreachable,
        };
    }
}
