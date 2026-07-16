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
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Square;

use Com\Tecnick\Barcode\Exception as BarcodeException;
use Com\Tecnick\Barcode\Type\Square\QrCode\ByteStream;
use Com\Tecnick\Barcode\Type\Square\QrCode\Data;
use Com\Tecnick\Barcode\Type\Square\QrCode\Encoder;
use Com\Tecnick\Barcode\Type\Square\QrCode\Split;

/**
 * Com\Tecnick\Barcode\Type\Square\QrCode
 *
 * QrCode Barcode type class
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
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
     * QR code version.
     * The Size of QRcode is defined as version. Version is an integer value from 1 to 40.
     * Version 1 is 21*21 matrix. And 4 modules increases whenever 1 version increases.
     * So version 40 is 177*177 matrix.
     */
    protected int $version = 0;

    /**
     * Error correction level
     */
    protected int $level = 0;

    /**
     * Encoding mode
     */
    protected int $hint = 2;

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
     * If true, estimates the best mask (spec default, but slower);
     * set to false for a significant performance boost but (probably) lower quality code.
     */
    protected bool $best_mask = true;

    /**
     * Default mask used when $this->best_mask === false
     */
    protected int $default_mask = 2;

    /**
     * ByteStream class object
     */
    protected ByteStream $bsObj;

    protected function getEccLevel(string $level): int
    {
        return match ($level) {
            'L' => 0,
            'M' => 1,
            'Q' => 2,
            'H' => 3,
            default => 0,
        };
    }

    protected function getHintMode(string $mode): int
    {
        return match ($mode) {
            'NL' => -1,
            'NM' => 0,
            'AN' => 1,
            '8B' => 2,
            'KJ' => 3,
            'ST' => 4,
            default => 2,
        };
    }

    /**
     * Set extra (optional) parameters:
     *     1: LEVEL - error correction level: L, M, Q, H
     *     2: HINT - encoding mode: NL=variable, NM=numeric, AN=alphanumeric, 8B=8bit, KJ=KANJI, ST=STRUCTURED
     *     3: VERSION - integer value from 1 to 40
     *     4: CASE SENSITIVE - if 0 the input string will be converted to uppercase
     *     5: RANDOM MASK - false or number of masks to be checked
     *     6: BEST MASK - true to find the best mask (slow)
     *     7: DEFAULT MASK - mask to use when the best mask option is false
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     * @SuppressWarnings("PHPMD.NPathComplexity")
     */
    protected function setParameters(): void
    {
        parent::setParameters();

        // level
        if (($this->params[0] ?? null) === null || !\array_key_exists(\strval($this->params[0]), Data::ECC_LEVELS)) {
            $this->params[0] = 'L';
        }

        $this->level = $this->getEccLevel(\strval($this->params[0]));

        // hint
        if (($this->params[1] ?? null) === null || !\array_key_exists(\strval($this->params[1]), Data::ENC_MODES)) {
            $this->params[1] = '8B';
        }

        $this->hint = $this->getHintMode(\strval($this->params[1]));

        // version
        if (
            ($this->params[2] ?? null) === null
            || $this->params[2] < 0
            || $this->params[2] > Data::QRSPEC_VERSION_MAX
        ) {
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
        if (($this->params[6] ?? null) === null) {
            $this->params[6] = 2;
        }

        $this->default_mask = (int) $this->params[6];
    }

    /**
     * Get the bars array
     *
     * @throws BarcodeException in case of error
     * @throws \Random\RandomException in case of random generation error
     */
    protected function setBars(): void
    {
        if (\strlen($this->code) === 0) {
            throw new BarcodeException('Empty input');
        }

        $this->bsObj = new ByteStream($this->hint, $this->version, $this->level);
        // generate the qrcode
        $this->processBinarySequence($this->binarize($this->encodeString($this->code)));
    }

    /**
     * Convert the frame in binary form
     *
     * @param array<int, string> $frame Array to binarize
     *
     * @return array<int, string> frame in binary form
     */
    protected function binarize(array $frame): array
    {
        $len = \count($frame);
        // the frame is square (width = height)
        foreach ($frame as &$frameLine) {
            for ($idx = 0; $idx < $len; ++$idx) {
                $frameLine[$idx] = (\ord($frameLine[$idx]) & 1) !== 0 ? '1' : '0';
            }
        }

        return $frame;
    }

    /**
     * Encode the input string
     *
     * @param string $data input string to encode
     *
     * @return array<int, string> Encoded data
     *
     * @throws BarcodeException in case of split/encoding errors
     * @throws \Random\RandomException in case of random generation error
     */
    protected function encodeString(string $data): array
    {
        if (!$this->case_sensitive) {
            $data = $this->toUpper($data);
        }

        $split = new Split($this->bsObj, $this->hint, $this->version);
        $datacode = $this->bsObj->getByteStream($split->getSplittedString($data));
        $this->version = $this->bsObj->version;
        $encoder = new Encoder($this->version, $this->level, $this->random_mask, $this->best_mask, $this->default_mask);
        return $encoder->encodeMask(-1, $datacode);
    }

    /**
     * Convert input string into upper case mode
     *
     * @param string $data Data
     */
    protected function toUpper(string $data): string
    {
        $len = \strlen($data);
        $pos = 0;

        while ($pos < $len) {
            $mode = $this->bsObj->getEncodingMode($data, $pos);
            if ($mode === $this->getHintMode('KJ')) {
                $pos += 2;
                continue;
            }

            if (\ord($data[$pos]) >= \ord('a') && \ord($data[$pos]) <= \ord('z')) {
                $data[$pos] = \chr((\ord($data[$pos]) - 32) & 0xFF);
            }

            ++$pos;
        }

        return $data;
    }
}
