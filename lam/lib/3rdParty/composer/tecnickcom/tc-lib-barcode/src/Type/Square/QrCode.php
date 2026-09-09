<?php

declare(strict_types=1);

/**
 * QrCode.php
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Square;

use Com\Tecnick\Barcode\Exception as BarcodeException;
use Com\Tecnick\Barcode\Type\Square\QrCode\Data;
use Com\Tecnick\Barcode\Type\Square\QrCode\Encode;
use Com\Tecnick\Barcode\Type\Square\QrCode\QrEccLevel;
use Com\Tecnick\Barcode\Type\Square\QrCode\QrEncodingMode;

/**
 * Com\Tecnick\Barcode\Type\Square\QrCode
 *
 * QrCode Barcode type class
 * QR Code (ISO/IEC 18004)
 *
 * Matrix symbology with three finder patterns and forty symbol versions.
 *     Symbol sizes:                21x21 to 177x177 modules, in steps of four
 *     Error correction levels:     L, M, Q and H
 *     Maximum data characters:     7089 digits, 4296 alphanumeric or 2953 bytes
 *
 * QR Code is a registered trademark of DENSO WAVE INCORPORATED.
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class QrCode extends \Com\Tecnick\Barcode\Type\Square
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'QRCODE';

    /**
     * QR Code version, from 1 to 40, or 0 to select the smallest symbol able to
     * carry the data. Version 1 is a 21x21 matrix and each version adds 4
     * modules per side, up to the 177x177 matrix of version 40.
     */
    protected int $version = 0;

    /**
     * Error correction level, from 0 to 3, that is L, M, Q and H.
     */
    protected int $level = 0;

    /**
     * Whether the kanji mode may be used, which is the case when the requested
     * encoding mode is KJ.
     */
    protected bool $kanji = false;

    /**
     * Boolean flag, if false the input string will be converted to uppercase.
     */
    protected bool $case_sensitive = true;

    /**
     * If negative, checks all masks available,
     * otherwise the value indicates the number of masks to be checked,
     * mask ids are random.
     */
    protected int $random_mask = -1;

    /**
     * If true, evaluates every mask and selects the best one, as required by the
     * specification; if false, the default mask is applied.
     */
    protected bool $best_mask = true;

    /**
     * Default mask used when $this->best_mask === false
     */
    protected int $default_mask = 2;

    protected function getEccLevel(string $level): int
    {
        return Data::ECC_LEVELS[$level] ?? 0;
    }

    /**
     * Set extra (optional) parameters:
     *     1: LEVEL - error correction level: L, M, Q, H
     *     2: HINT - encoding mode: NL, NM, AN, 8B, KJ or ST
     *     3: VERSION - integer value from 1 to 40
     *     4: CASE SENSITIVE - if 0 the input string will be converted to uppercase
     *     5: RANDOM MASK - false or number of masks to be checked
     *     6: BEST MASK - true to find the best mask (slow)
     *     7: DEFAULT MASK - mask to use when the best mask option is false
     *
     * The encoding mode selects the modes the encoder may use. The numeric, the
     * alphanumeric and the byte modes are always available and are mixed as
     * section 7.4.7 of ISO/IEC 18004 allows; KJ adds the kanji mode. Every other
     * token leaves that set unchanged, the structured append of ST included.
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     * @SuppressWarnings("PHPMD.NPathComplexity")
     */
    protected function setParameters(): void
    {
        parent::setParameters();

        // level
        $eccLevel = QrEccLevel::fromLoose(\strval($this->params[0] ?? ''));
        $this->params[0] = $eccLevel->value;
        $this->level = $this->getEccLevel($eccLevel->value);

        // hint
        $encMode = QrEncodingMode::fromLoose(\strval($this->params[1] ?? ''));
        $this->params[1] = $encMode->value;
        $this->kanji = $encMode->value === 'KJ';

        // version
        if (($this->params[2] ?? null) === null || $this->params[2] < 0 || $this->params[2] > Data::VERSION_MAX) {
            $this->params[2] = 0;
        }

        $this->version = (int) $this->params[2];

        // case sensitive
        if (($this->params[3] ?? null) === null) {
            $this->params[3] = 1;
        }

        $this->case_sensitive = (bool) $this->params[3];

        // random mask mode - number of masks to be checked
        if (($this->params[4] ?? null) !== null && $this->params[4] !== '' && (int) $this->params[4] !== 0) {
            $this->random_mask = (int) $this->params[4];
        }

        // find best mask
        if (($this->params[5] ?? null) === null) {
            $this->params[5] = 1;
        }

        $this->best_mask = (bool) $this->params[5];

        // default mask
        if (
            ($this->params[6] ?? null) === null
            || $this->params[6] === ''
            || $this->params[6] < 0
            || $this->params[6] > 7
        ) {
            $this->params[6] = 2;
        }

        $this->default_mask = (int) $this->params[6];
    }

    /**
     * Get the character sequence to encode.
     * Subclasses that build the payload from the input code override this.
     */
    protected function getEncodedPayload(): string
    {
        return $this->code;
    }

    /**
     * Get the bars array
     *
     * @throws BarcodeException in case of error
     * @throws \Random\RandomException in case of random generation error
     */
    protected function setBars(): void
    {
        $code = $this->getEncodedPayload();
        if (\strlen($code) === 0) {
            throw new BarcodeException('Empty input');
        }

        if (!$this->case_sensitive) {
            $code = $this->toUpper($code);
        }

        $encode = new Encode(
            $code,
            $this->level,
            $this->version,
            $this->kanji,
            $this->random_mask,
            $this->best_mask,
            $this->default_mask,
        );
        $this->version = $encode->getVersion();
        $this->processBinarySequence($encode->getGrid());
    }

    /**
     * Convert input string into upper case mode, leaving the two byte characters
     * of the kanji mode alone.
     *
     * @param string $data Data
     */
    protected function toUpper(string $data): string
    {
        $len = \strlen($data);
        $pos = 0;

        while ($pos < $len) {
            if ($this->kanji && $this->isKanjiAt($data, $pos)) {
                $pos += 2;
                continue;
            }

            if ($data[$pos] >= 'a' && $data[$pos] <= 'z') {
                $data[$pos] = \chr((\ord($data[$pos]) - 32) & 0xFF);
            }

            ++$pos;
        }

        return $data;
    }

    /**
     * Returns whether the byte pair at the given offset is a character of the
     * two Shift JIS ranges the kanji mode encodes.
     *
     * @param string $data Data
     * @param int    $pos  Byte offset.
     */
    protected function isKanjiAt(string $data, int $pos): bool
    {
        $high = $data[$pos] ?? '';
        $low = $data[$pos + 1] ?? '';
        if ($high === '' || $low === '') {
            return false;
        }

        $value = (\ord($high) << 8) | \ord($low);
        foreach (Data::KANJI_RANGES as $range) {
            if ($value >= $range[0] && $value <= $range[1]) {
                return true;
            }
        }

        return false;
    }
}
