<?php

declare(strict_types=1);

/**
 * PdfFourOneSeven.php
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
use Com\Tecnick\Barcode\Type\Square\PdfFourOneSeven\Data;

/**
 * Com\Tecnick\Barcode\Type\Square\PdfFourOneSeven
 *
 * PdfFourOneSeven Barcode type class
 * PDF417 (ISO/IEC 15438:2006)
 *
 * Continuous multi-row symbology encoding the 128 ASCII characters and the extended ones.
 *     Symbol height:               3 to 90 rows
 *     Symbol width:                90X to 583X
 *     Error correction characters: 2 to 512
 *     Maximum data characters:     1850 text, 2710 digits, 1108 bytes
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class PdfFourOneSeven extends \Com\Tecnick\Barcode\Type\Square\PdfFourOneSeven\Compaction
{
    /**
     * @param array<int, int> $codewords
     */
    protected function getCodewordValue(array $codewords, int $index): int
    {
        return $codewords[$index] ?? 0;
    }

    protected function getClusterCodewordValue(int $clusterId, int $codeword): int
    {
        return Data::CLUSTERS[$clusterId][$codeword] ?? 0;
    }

    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'PDF417';

    /**
     * Truncated symbol: the right row indicator and the stop pattern are
     * replaced by a single narrow bar.
     */
    protected bool $truncated = false;

    /**
     * Row height respect X dimension of single module
     */
    protected int $row_height = 2;

    /**
     * Vertical quiet zone in modules
     */
    protected int $quiet_vertical = 2;

    /**
     * Horizontal quiet zone in modules
     */
    protected int $quiet_horizontal = 2;

    /**
     * Aspect ratio (width / height)
     */
    protected float $aspectratio = 2;

    /**
     * Error correction level (0-8);
     * Default -1 = automatic correction level
     */
    protected int $ecl = -1;

    /**
     * Information for macro block
     *
     * @var array<string, int|string>
     */
    protected array $macro = [];

    /**
     * Set extra (optional) parameters
     *
     * @throws BarcodeException in case of invalid macro control block fields
     */
    protected function setParameters(): void
    {
        parent::setParameters();

        // aspect ratio
        if (
            ($this->params[0] ?? null) !== null
            && $this->params[0] !== ''
            && ($aspectratio = (float) $this->params[0]) >= 1
        ) {
            $this->aspectratio = $aspectratio;
        }

        // error correction level (auto)
        if (($this->params[1] ?? null) !== null && ($ecl = (int) $this->params[1]) >= 0 && $ecl <= 8) {
            $this->ecl = $ecl;
        }

        // macro block
        $this->setMacroBlockParam();
    }

    /**
     * Characters that Text Compaction can represent.
     *
     * @var string
     */
    protected const TEXT_COMPACTION_CHARS = '/^[\x09\x0a\x0d\x20-\x7e]*$/';

    /**
     * Compaction mode of each optional macro field: 900 = text, 902 = numeric.
     *
     * @var array<int, int>
     */
    protected const MACRO_OPTION_MODES = [900, 902, 902, 900, 900, 902, 902];

    /**
     * Maximum value of each optional macro field, for the numeric ones with a fixed size.
     *
     * @var array<int, int>
     */
    protected const MACRO_OPTION_MAX = [1 => 99_999, 2 => 9_999_999_999, 6 => 99_999];

    /**
     * Highest value of the fixed two-codeword segment index field.
     *
     * @var int
     */
    protected const MACRO_MAX_SEGMENT = 99_999;

    /**
     * Validate a macro control block field.
     *
     * @param string $value Field value
     * @param int    $mode  Compaction mode: 900 = text, 902 = numeric
     * @param int    $max   Maximum value of a numeric field, 0 for no limit
     * @param string $name  Field name to report in the error message
     *
     * @throws BarcodeException if the field cannot be represented
     */
    protected function checkMacroField(string $value, int $mode, int $max, string $name): void
    {
        if ($mode === 902) {
            if (!\ctype_digit($value)) {
                throw new BarcodeException('The macro ' . $name . ' must be a number: ' . $value);
            }

            if ($max > 0 && (int) $value > $max) {
                throw new BarcodeException('The macro ' . $name . ' must not be greater than ' . $max . ': ' . $value);
            }

            return;
        }

        if (\preg_match(self::TEXT_COMPACTION_CHARS, $value) !== 1) {
            throw new BarcodeException(
                'The macro ' . $name . ' contains characters that Text Compaction cannot represent',
            );
        }
    }

    /**
     * Set macro block parameter
     *
     * @throws BarcodeException in case of invalid macro control block fields
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     */
    protected function setMacroBlockParam(): void
    {
        $segmentTotal = (string) ($this->params[2] ?? '');
        $segmentIndex = (string) ($this->params[3] ?? '');
        $fileId = (string) ($this->params[4] ?? '');
        if ($segmentTotal === '' || $segmentIndex === '' || $fileId === '') {
            return;
        }

        $fileId = \strtr($fileId, "\xff", ',');
        $this->checkMacroField($segmentTotal, 902, self::MACRO_MAX_SEGMENT, 'segment total');
        $this->checkMacroField($segmentIndex, 902, self::MACRO_MAX_SEGMENT, 'segment index');
        $this->checkMacroField($fileId, 900, 0, 'file ID');
        $this->macro['segment_total'] = (int) $segmentTotal;
        $this->macro['segment_index'] = (int) $segmentIndex;
        $this->macro['file_id'] = $fileId;
        for ($idx = 0; $idx < 7; ++$idx) {
            $option = \strtr((string) ($this->params[$idx + 5] ?? ''), "\xff", ',');
            if ($option === '') {
                continue;
            }

            $this->checkMacroField(
                $option,
                self::MACRO_OPTION_MODES[$idx] ?? 900,
                self::MACRO_OPTION_MAX[$idx] ?? 0,
                'option ' . $idx,
            );
            $this->macro['option_' . $idx] = $option;
        }
    }

    /**
     * Get the bars array
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        if (\strlen($this->code) === 0) {
            throw new BarcodeException('Empty input');
        }

        $seq = $this->getBinSequence();
        $this->processBinarySequence($this->getRawCodeRows($seq));
    }

    /**
     * Get macro control block codewords
     *
     * @param int $numcw Number of codewords
     *
     * @return array<int, int>
     */
    protected function getMacroBlock(int &$numcw): array
    {
        if ($this->macro === []) {
            return [];
        }

        $macrocw = [];
        $segmentIndex = (int) ($this->macro['segment_index'] ?? 0);
        $segmentTotal = (int) ($this->macro['segment_total'] ?? 0);
        $fileId = (string) ($this->macro['file_id'] ?? '');
        // beginning of macro control block
        $macrocw[] = 928;
        // segment index
        $cdw = $this->getCompaction(902, \sprintf('%05d', $segmentIndex), false);
        $macrocw = \array_merge($macrocw, $cdw);
        // file ID
        $cdw = $this->getCompaction(900, $fileId, false);
        $macrocw = \array_merge($macrocw, $cdw);
        // optional fields
        $optmodes = [900, 902, 902, 900, 900, 902, 902];
        $optsize = [-1, 2, 4, -1, -1, -1, 2];
        foreach ($optmodes as $key => $omode) {
            $optionKey = 'option_' . $key;
            if (($this->macro[$optionKey] ?? null) !== null) {
                $option = (string) $this->macro[$optionKey];
                $macrocw[] = 923;
                $macrocw[] = $key;
                $option = match ($optsize[$key] ?? -1) {
                    2 => \sprintf('%05d', $option),
                    4 => \sprintf('%010d', $option),
                    default => $option,
                };

                $cdw = $this->getCompaction($omode, $option, false);
                $macrocw = \array_merge($macrocw, $cdw);
            }
        }

        if ($segmentIndex === ($segmentTotal - 1)) {
            // end of control block
            $macrocw[] = 922;
        }

        // update total codewords
        $numcw += \count($macrocw);
        return $macrocw;
    }

    /**
     * Get codewords
     *
     * @param int $rows number of rows
     * @param int $cols number of columns
     * @param int $ecl error correction level
     *
     * @return array<int, int>
     *
     * @throws BarcodeException in case of error
     */
    public function getCodewords(int &$rows, int &$cols, int &$ecl): array
    {
        $codewords = []; // array of code-words
        // get the input sequence array
        $sequence = $this->getInputSequences($this->code);
        foreach ($sequence as $seq) {
            $cws = $this->getCompaction($seq[0], $seq[1], true);
            $codewords = \array_merge($codewords, $cws);
        }

        if (($codewords[0] ?? 0) === 900) {
            // Text Alpha is the default mode, so remove the first code
            \array_shift($codewords);
        }

        // count number of codewords
        $numcw = \count($codewords);
        if ($numcw > 925) {
            throw new BarcodeException('The maximum codeword capacity has been reached: ' . $numcw . ' > 925');
        }

        $macrocw = $this->getMacroBlock($numcw);
        // set error correction level
        $ecl = $this->getErrorCorrectionLevel($this->ecl, $numcw);
        // number of codewords for error correction
        $errsize = 2 << $ecl;
        // calculate number of columns (number of codewords per row) and rows
        $nce = $numcw + $errsize + 1;
        $cols = (int) \min(30, \max(
            1,
            \round((\sqrt(4761 + (68 * $this->aspectratio * $this->row_height * $nce)) - 69) / 34),
        ));
        $rows = (int) \min(90, \max(3, \ceil($nce / $cols)));
        $size = $cols * $rows;
        if ($size > 928 || $size < $nce) {
            // the requested aspect ratio does not fit the data within the 30 columns
            // and 90 rows limits: use the dimensions of maximum capacity
            $cols = 16;
            $rows = 58;
            if (\abs($this->aspectratio - ((17 * 29) / 32)) < \abs($this->aspectratio - ((17 * 16) / 58))) {
                $cols = 29;
                $rows = 32;
            }

            $size = 928;
        }

        if ($size < $nce) {
            throw new BarcodeException(
                'The data does not fit in a PDF417 symbol: ' . $nce . ' codewords required, maximum is ' . $size,
            );
        }

        if ($macrocw !== []) {
            // the macro control block follows the data codewords
            $codewords = \array_merge($codewords, $macrocw);
        }

        // calculate padding
        $pad = (int) ($size - $nce);
        if ($pad > 0) {
            // add padding
            $codewords = \array_merge($codewords, \array_fill(0, $pad, 900));
        }

        // Symbol Length Descriptor (number of data codewords including Symbol Length Descriptor and pad codewords)
        $sld = (int) ($size - $errsize);
        // add symbol length description
        \array_unshift($codewords, $sld);
        // calculate error correction
        $ecw = $this->getErrorCorrection($codewords, $ecl);
        // add error correction codewords
        return \array_merge($codewords, $ecw);
    }

    /**
     * Creates a PDF417 object as binary string
     *
     * @return string barcode as binary string
     *
     * @throws BarcodeException in case of error
     */
    public function getBinSequence(): string
    {
        $rows = 0;
        $cols = 0;
        $ecl = 0;
        $codewords = $this->getCodewords($rows, $cols, $ecl);
        $barcode = '';
        // add horizontal quiet zones to start and stop patterns
        $pstart = \str_repeat('0', \max(0, $this->quiet_horizontal)) . Data::START_PATTERN;
        $this->nrows = ($rows * $this->row_height) + (2 * $this->quiet_vertical);
        // start pattern, left row indicator, data columns and, unless truncated,
        // the right row indicator and the stop pattern
        $this->ncols = $this->truncated
            ? (($cols + 1) * 17) + 18 + (2 * $this->quiet_horizontal)
            : (($cols + 2) * 17) + 35 + (2 * $this->quiet_horizontal);
        // build rows for vertical quiet zone
        $empty_row = ',' . \str_repeat('0', \max(0, $this->ncols));
        $empty_rows = \str_repeat($empty_row, \max(0, $this->quiet_vertical));
        $barcode .= $empty_rows;
        $kcw = 0; // codeword index
        $cid = 0; // initial cluster
        // for each row
        for ($rix = 0; $rix < $rows; ++$rix) {
            // row start code
            $row = $pstart;
            $rval = 0;
            $cval = 0;
            switch ($cid) {
                case 0:
                    $rval = (30 * (int) ($rix / 3)) + (int) (($rows - 1) / 3);
                    $cval = (30 * (int) ($rix / 3)) + ($cols - 1);
                    break;
                case 1:
                    $rval = (30 * (int) ($rix / 3)) + ($ecl * 3) + (($rows - 1) % 3);
                    $cval = (30 * (int) ($rix / 3)) + (int) (($rows - 1) / 3);
                    break;
                case 2:
                    $rval = (30 * (int) ($rix / 3)) + ($cols - 1);
                    $cval = (30 * (int) ($rix / 3)) + ($ecl * 3) + (($rows - 1) % 3);
                    break;
            }

            // left row indicator
            $row .= \sprintf('%17b', $this->getClusterCodewordValue($cid, $rval));
            // for each column
            for ($cix = 0; $cix < $cols; ++$cix) {
                $row .= \sprintf('%17b', $this->getClusterCodewordValue($cid, $this->getCodewordValue(
                    $codewords,
                    $kcw,
                )));
                ++$kcw;
            }

            // right row indicator and row stop code, or the single bar that
            // terminates a truncated row
            $row .= $this->truncated
                ? '1'
                : \sprintf('%17b' . Data::STOP_PATTERN, $this->getClusterCodewordValue($cid, $cval));
            $row .= \str_repeat('0', \max(0, $this->quiet_horizontal));
            // each codeword row is repeated over $row_height rows of modules
            $barcode .= \str_repeat(',' . $row, \max(0, $this->row_height));
            ++$cid;
            if ($cid > 2) {
                $cid = 0;
            }
        }

        return $barcode . $empty_rows;
    }
}
