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
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
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
 * PDF417 (ISO/IEC 15438:2006) is a 2-dimensional stacked bar code created by Symbol Technologies in 1991.
 * It is one of the most popular 2D codes because of its ability to be read with slightly modified handheld
 * laser or linear CCD scanners.
 * TECHNICAL DATA / FEATURES OF PDF417:
 *     Encodable Character Set:     All 128 ASCII Characters (including extended)
 *     Code Type:                   Continuous, Multi-Row
 *     Symbol Height:               3 - 90 Rows
 *     Symbol Width:                90X - 583X
 *     Bidirectional Decoding:      Yes
 *     Error Correction Characters: 2 - 512
 *     Maximum Data Characters:     1850 text, 2710 digits, 1108 bytes
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
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
     * Row height respect X dimension of single module
     */
    protected int $row_height = 2;

    /**
     * Horizontal quiet zone in modules
     */
    protected int $quiet_vertical = 2;

    /**
     * Vertical quiet zone in modules
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
     * Set macro block parameter
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     */
    protected function setMacroBlockParam(): void
    {
        if (
            ($this->params[4] ?? null) !== null
            && \is_string($this->params[4])
            && ($this->params[2] ?? '') !== ''
            && ($this->params[3] ?? '') !== ''
            && $this->params[4] !== ''
        ) {
            $this->macro['segment_total'] = (int) $this->params[2];
            $this->macro['segment_index'] = (int) $this->params[3];
            $this->macro['file_id'] = \strtr($this->params[4], "\xff", ',');
            for ($idx = 0; $idx < 7; ++$idx) {
                $opt = $idx + 5;
                if (
                    ($this->params[$opt] ?? null) !== null
                    && \is_string($this->params[$opt])
                    && $this->params[$opt] !== ''
                ) {
                    /* @phpstan-ignore-next-line */
                    $this->macro['option_' . $idx] = \strtr($this->params[$opt], "\xff", ',');
                }
            }
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
     * @param int $ecl eroor correction level
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
            throw new BarcodeException('The maximum codeword capaciy has been reached: ' . $numcw . ' > 925');
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
        if ($size > 928) {
            // set dimensions to get maximum capacity
            $cols = 16;
            $rows = 58;
            if (\abs($this->aspectratio - ((17 * 29) / 32)) < \abs($this->aspectratio - ((17 * 16) / 58))) {
                $cols = 29;
                $rows = 32;
            }

            $size = 928;
        }

        // calculate padding
        $pad = (int) ($size - $nce);
        if ($pad > 0) {
            // add padding
            $codewords = \array_merge($codewords, \array_fill(0, $pad, 900));
        }

        if ($macrocw !== []) {
            // add macro section
            $codewords = \array_merge($codewords, $macrocw);
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
        $this->ncols = (($cols + 2) * 17) + 35 + (2 * $this->quiet_horizontal);
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

            // right row indicator
            $row .= \sprintf('%17b', $this->getClusterCodewordValue($cid, $cval));
            // row stop code
            $row .= Data::STOP_PATTERN . \str_repeat('0', \max(0, $this->quiet_horizontal));
            $brow = ',' . \str_repeat($row, \max(0, $this->row_height));
            $barcode .= $brow;
            ++$cid;
            if ($cid > 2) {
                $cid = 0;
            }
        }

        return $barcode . $empty_rows;
    }
}
