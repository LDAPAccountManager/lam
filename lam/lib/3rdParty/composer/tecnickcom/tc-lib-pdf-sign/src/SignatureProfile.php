<?php

declare(strict_types=1);

/**
 * SignatureProfile.php
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
 * Com\Tecnick\Pdf\Sign\SignatureProfile
 *
 * Backed enum for the supported signature profiles. The backing value of each
 * case matches the corresponding Config::PROFILE_* constant validated by Config.
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
enum SignatureProfile: string
{
    case Legacy = 'legacy';

    case PadesBB = 'pades-b-b';

    case PadesBT = 'pades-b-t';

    case PadesBLT = 'pades-b-lt';

    case PadesBLTA = 'pades-b-lta';

    /**
     * Resolve a loose signature profile value to the matching enum case.
     *
     * Accepts the canonical profile string (as validated by Config) or an enum
     * instance (returned unchanged). Unknown values throw, matching the closed
     * set enforced by Config.
     *
     * @param string|self $value Signature profile identifier or enum case.
     *
     * @throws Exception if the value does not match a known signature profile.
     */
    public static function fromLoose(string|self $value): self
    {
        if ($value instanceof self) {
            return $value;
        }

        return self::tryFrom($value) ?? throw new Exception('Invalid signature profile: ' . $value);
    }
}
