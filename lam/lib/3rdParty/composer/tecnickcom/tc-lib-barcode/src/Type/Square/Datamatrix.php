<?php

declare(strict_types=1);

/**
 * Datamatrix.php
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Square;

use Com\Tecnick\Barcode\Exception as BarcodeException;
use Com\Tecnick\Barcode\Type\Square\Datamatrix\Data;
use Com\Tecnick\Barcode\Type\Square\Datamatrix\DatamatrixEncoding;
use Com\Tecnick\Barcode\Type\Square\Datamatrix\DatamatrixShape;
use Com\Tecnick\Barcode\Type\Square\Datamatrix\Encode;

/**
 * Com\Tecnick\Barcode\Type\Square\Datamatrix
 *
 * Datamatrix Barcode type class
 * DATAMATRIX (ISO/IEC 16022)
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Datamatrix extends \Com\Tecnick\Barcode\Type\Square
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'DATAMATRIX';

    /**
     * Number of data codewords of the largest symbol of the shape.
     *
     * @var int
     */
    protected const MAXCDW = 1558;

    /**
     * Array of codewords.
     *
     * @var array<int, int>
     */
    protected array $cdw = [];

    /**
     * Binary grid
     *
     * @var array<int, array<int, int>>
     */
    protected array $grid = [];

    /**
     * Datamatrix Encoding object
     */
    protected Encode $dmx;

    /**
     * Datamatrix shape key (S=square, R=rectangular)
     */
    protected string $shape = 'S';

    /**
     * Symbol size as rows by columns, empty for the smallest size that fits
     */
    protected string $size = '';

    /**
     * Datamatrix variant (N=default, GS1=FNC1 codeword in first place)
     */
    protected bool $gsonemode = false;

    /**
     * Datamatrix default encoding.
     * See Data::SWITCHCDW for valid values.
     */
    protected int $defenc = Data::ENC_ASCII;

    /**
     * Set extra (optional) parameters:
     *     1: SHAPE: S=square (default), R=rectangular.
     *     2: MODE: N=default, GS1 = the FNC1 codeword is added in the first position of Data Matrix ECC 200 version.
     *     3: ENCODING: ASCII (default), C40, TXT, X12, EDIFACT, BASE256.
     */
    protected function setParameters(): void
    {
        parent::setParameters();

        // shape
        $this->shape = DatamatrixShape::fromLoose(\strval($this->params[0] ?? ''))->value;

        $this->setModeAndEncoding(1);
    }

    /**
     * Set the mode and the encoding from the parameters starting at the given index.
     *
     * @param int $idx Index of the mode parameter.
     */
    protected function setModeAndEncoding(int $idx): void
    {
        // mode
        $this->gsonemode = ($this->params[$idx] ?? null) === 'GS1';

        // encoding
        $encoding = $this->params[$idx + 1] ?? null;
        $this->defenc = $encoding === null
            ? Data::ENC_ASCII
            : Data::ENCOPTS[DatamatrixEncoding::fromLoose(\strval($encoding))->value] ?? Data::ENC_ASCII;
    }

    /**
     * Return to ASCII encodation, writing the unlatch codeword the current encodation requires.
     * A Base 256 field is self-terminating and needs no unlatch.
     *
     * @param array<int, int> $cdw          Codewords array
     * @param int             $cdw_num      Number of codewords
     * @param int             $enc          Current encodation
     * @param int             $field_length Length of the current field
     */
    protected function unlatchToAscii(array &$cdw, int &$cdw_num, int &$enc, int &$field_length): void
    {
        if ($enc === Data::ENC_ASCII) {
            return;
        }

        if ($enc !== Data::ENC_BASE256) {
            $cdw[] = $enc === Data::ENC_EDF ? 124 : 254;
            ++$cdw_num;
        }

        $enc = Data::ENC_ASCII;
        $this->dmx->last_enc = $enc;
        $field_length = 0;
    }

    /**
     * Add padding codewords
     *
     * @param int $size Max barcode size in codewords
     * @param int $ncw  Number of codewords
     *
     * @throws BarcodeException in case of error
     */
    protected function addPadding(int $size, int $ncw): void
    {
        if ($size <= $ncw) {
            return;
        }

        $lastEnc = (int) $this->dmx->last_enc;
        if ($lastEnc !== Data::ENC_ASCII && $lastEnc !== Data::ENC_BASE256) {
            // return to ASCII encodation before padding
            $this->cdw[] = $lastEnc === Data::ENC_EDF ? 124 : 254;

            ++$ncw;
        }

        if ($size > $ncw) {
            // add first pad
            $this->cdw[] = 129;
            ++$ncw;
            // add remaining pads, randomised by their position in the codeword
            // stream, which is one more than their index in it
            for ($i = $ncw; $i < $size; ++$i) {
                $this->cdw[] = $this->dmx->get253StateCodeword(129, $i + 1);
            }
        }
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
     * Get the codewords
     *
     * @return array{int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int} params
     *
     * @throws BarcodeException in case of error
     */
    protected function getCodewords(): array
    {
        $code = $this->getEncodedPayload();
        if (\strlen($code) === 0) {
            throw new BarcodeException('Empty input');
        }

        // get data codewords
        $this->cdw = $this->getHighLevelEncoding($code);

        // number of data codewords
        $ncw = \count($this->cdw);

        // check size
        if ($ncw > static::MAXCDW) {
            throw new BarcodeException('the input is too large to fit the barcode');
        }

        // get the requested matrix size, or the minimum required one.
        $params = Data::getPaddingSize($this->shape, $ncw, $this->size);
        $this->addPadding($params[11], $ncw);

        $errorCorrection = new \Com\Tecnick\Barcode\Type\Square\Datamatrix\ErrorCorrection();
        $this->cdw = $errorCorrection->getErrorCorrection($this->cdw, $params[13], $params[14], $params[15]);

        return $params;
    }

    /**
     * Set the grid
     *
     * @param int             $idx    Index
     * @param array<int, int> $places Places
     * @param int             $row    Row
     * @param int             $col    Column
     * @param int             $rdx    Region data row index
     * @param int             $cdx    Region data column index
     * @param int             $rdri   Region data row max index
     * @param int             $rdci   Region data column max index
     */
    protected function setGrid(
        int &$idx,
        array &$places,
        int &$row,
        int &$col,
        int &$rdx,
        int &$cdx,
        int &$rdri,
        int &$rdci,
    ): void {
        // draw bits by case
        if ($rdx === 0) {
            // top finder pattern
            $this->grid[$row][$col] = (int) (($cdx % 2) === 0);
            return;
        }

        if ($rdx === $rdri) {
            // bottom finder pattern
            $this->grid[$row][$col] = 1;
            return;
        }

        if ($cdx === 0) {
            // left finder pattern
            $this->grid[$row][$col] = 1;
            return;
        }

        if ($cdx === $rdci) {
            // right finder pattern
            $this->grid[$row][$col] = (int) (($rdx % 2) > 0);
            return;
        }

        // data bit
        $place = $places[$idx] ?? 0;
        if ($place < 2) {
            $this->grid[$row][$col] = $place;
            ++$idx;
            return;
        }

        // codeword ID
        $cdw_id = \floor($place / 10) - 1;
        // codeword BIT mask
        $cdw_bit = 2 ** (8 - ($place % 10));
        $cdw_val = $this->cdw[\intval($cdw_id)] ?? 0;
        $this->grid[$row][$col] = ($cdw_val & $cdw_bit) === 0 ? 0 : 1;

        ++$idx;
    }

    /**
     * Get high level encoding using the minimum symbol data characters for ECC 200
     *
     * @param string $data data to encode
     *
     * @return array<int, int> Codewords
     *
     * @throws BarcodeException in case of error
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     */
    protected function getHighLevelEncoding(string $data): array
    {
        // STEP A. Start in ASCII encodation and latch to the predefined one on the first data character.
        $enc = Data::ENC_ASCII; // current encoding mode
        $this->dmx->last_enc = $enc; // last used encoding
        $pos = 0; // current position
        $cdw = []; // array of codewords to be returned
        $cdw_num = 0; // number of data codewords
        $data_length = \strlen($data); // number of chars
        $field_length = 0; // number of chars in current field
        // the predefined encodation is latched once, after any leading FNC1 codeword
        $latch = $this->defenc !== Data::ENC_ASCII;

        while ($pos < $data_length) {
            if ($this->gsonemode) {
                // check for control characters
                $cco = \ord($data[$pos]);
                if (
                    $cco === 232 // FNC1 (ASCII 232 - HEX \xE8)
                    || $cco === 29 // <GS> (ASCII  29 - HEX \x1D)
                ) {
                    // FNC1 is an ASCII encodation codeword: leave the current encodation first
                    $this->unlatchToAscii($cdw, $cdw_num, $enc, $field_length);
                    $cdw[] = 232; // FNC1
                    ++$pos;
                    ++$cdw_num;
                    continue;
                }
            }

            if ($latch) {
                // Switch to the predefined encoding
                $latch = false;
                $enc = $this->defenc;
                $this->dmx->last_enc = $enc;
                $cdw[] = $this->dmx->getSwitchEncodingCodeword($enc);
                ++$cdw_num;
            }

            switch ($enc) {
                case Data::ENC_ASCII:
                    // STEP B. While in ASCII encodation
                    $this->dmx->encodeASCII($cdw, $cdw_num, $pos, $data_length, $data, $enc);
                    break;
                case Data::ENC_C40:
                // Upper-case alphanumeric
                case Data::ENC_TXT:
                // Lower-case alphanumeric
                case Data::ENC_X12:
                    // ANSI X12
                    $this->dmx->encodeTXT($cdw, $cdw_num, $pos, $data_length, $data, $enc);
                    break;
                case Data::ENC_EDF:
                    // F. While in EDIFACT (EDF) encodation
                    $this->dmx->encodeEDF($cdw, $cdw_num, $pos, $data_length, $field_length, $data, $enc);
                    break;
                case Data::ENC_BASE256:
                    // G. While in Base 256 (B256) encodation
                    $this->dmx->encodeBase256($cdw, $cdw_num, $pos, $data_length, $field_length, $data, $enc);
                    break;
            }

            $this->dmx->last_enc = $enc;
        }

        return $cdw;
    }

    /**
     * Get the bars array
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        $this->dmx = new Encode($this->shape, $this->gsonemode, $this->size);
        $params = $this->getCodewords();
        // get placement map
        $places = $this->dmx->getPlacementMap($params[2], $params[3]);
        // fill the grid with data
        $this->grid = [];
        $idx = 0;
        // region data row max index
        $rdri = $params[4] - 1;
        // region data column max index
        $rdci = $params[5] - 1;
        // for each horizontal region
        for ($hr = 0; $hr < $params[8]; ++$hr) {
            // for each row on region
            for ($rdx = 0; $rdx < $params[4]; ++$rdx) {
                $row = ($hr * $params[4]) + $rdx;
                // for each vertical region
                for ($vr = 0; $vr < $params[9]; ++$vr) {
                    // for each column on region
                    for ($cdx = 0; $cdx < $params[5]; ++$cdx) {
                        $col = ($vr * $params[5]) + $cdx;
                        $this->setGrid($idx, $places, $row, $col, $rdx, $cdx, $rdri, $rdci);
                    }
                }
            }
        }

        $this->processBinarySequence($this->grid);
    }
}
