<?php

declare(strict_types=1);

/**
 * Init.php
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Square\QrCode;

use Com\Tecnick\Barcode\Exception as BarcodeException;

/**
 * Com\Tecnick\Barcode\Type\Square\QrCode\Init
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * @phpstan-type RSItem array{
 *          'alpha_to': array<int, int>,
 *          'fcr': int,
 *          'genpoly': array<int, int>,
 *          'gfpoly': int,
 *          'index_of': array<int, int>,
 *          'iprim': int,
 *          'mm': int,
 *          'nn': int,
 *          'nroots': int,
 *          'pad': int,
 *          'prim': int,
 *      }
 *
 * @phpstan-type RSblock array{
 *          'data': array<int, int>,
 *          'dataLength': int,
 *          'ecc': array<int, int>,
 *          'eccLength': int,
 *      }
 */
abstract class Init extends \Com\Tecnick\Barcode\Type\Square\QrCode\Mask
{
    /**
     * Data code
     *
     * @var array<int, int>
     */
    protected array $datacode = [];

    /**
     * Error correction code
     *
     * @var array<int, int>
     */
    protected array $ecccode = [];

    /**
     * Blocks
     */
    protected int $blocks;

    /**
     * Reed-Solomon blocks
     *
     * @var array<int, RSblock>
     */
    protected array $rsblocks = []; //of RSblock

    /**
     * Counter
     */
    protected int $count;

    /**
     * Data length
     */
    protected int $dataLength;

    /**
     * Error correction length
     */
    protected int $eccLength;

    /**
     * Value bv1
     */
    protected int $bv1;

    /**
     * Width.
     */
    protected int $width;

    /**
     * Frame
     *
     * @var array<int, string>
     */
    protected array $frame = [];

    /**
     * Horizontal bit position
     */
    protected int $xpos;

    /**
     * Vertical bit position
     */
    protected int $ypos;

    /**
     * Direction
     */
    protected int $dir;

    /**
     * Single bit value
     */
    protected int $bit;

    /**
     * Reed-Solomon items
     *
     * @var array<int, RSItem>
     */
    protected array $rsitems = [];

    /**
     * Initialize code
     *
     * @param array<int, int> $spec Array of ECC specification
     *
     * @throws BarcodeException in case RS initialization fails
     */
    protected function init(array $spec): void
    {
        $dlv = $this->spc->rsDataCodes1($spec);
        $elv = $this->spc->rsEccCodes1($spec);
        $rsv = $this->initRs(8, 0x11d, 0, 1, $elv, 255 - $dlv - $elv);
        $blockNo = 0;
        $dataPos = 0;
        $eccPos = 0;
        $ecc = [];
        $endfor = $this->spc->rsBlockNum1($spec);
        $this->initLoop($endfor, $dlv, $elv, $rsv, $eccPos, $blockNo, $dataPos, $ecc);
        if ($this->spc->rsBlockNum2($spec) === 0) {
            return;
        }

        $dlv = $this->spc->rsDataCodes2($spec);
        $elv = $this->spc->rsEccCodes2($spec);
        $rsv = $this->initRs(8, 0x11d, 0, 1, $elv, 255 - $dlv - $elv);
        $endfor = $this->spc->rsBlockNum2($spec);
        $this->initLoop($endfor, $dlv, $elv, $rsv, $eccPos, $blockNo, $dataPos, $ecc);
    }

    /**
     * Internal loop for init
     *
     * @param int   $endfor  End for
     * @param int   $dlv     Data length value
     * @param int   $elv     Error correction length value
     * @param RSItem $rsv Reed-Solomon values
     * @param int   $eccPos  Error correction code position
     * @param int   $blockNo Block number
     * @param int   $dataPos Data position
     * @param array<int, int> $ecc     Error correction code
     */
    protected function initLoop(
        int $endfor,
        int $dlv,
        int $elv,
        array $rsv,
        int &$eccPos,
        int &$blockNo,
        int &$dataPos,
        array &$ecc,
    ): void {
        for ($idx = 0; $idx < $endfor; ++$idx) {
            $data = \array_slice($this->datacode, $dataPos);
            $ecc = \array_slice($this->ecccode, $eccPos);
            $ecc = $this->encodeRsChar($rsv, $data, $ecc);
            $this->rsblocks[$blockNo] = [
                'data' => $data,
                'dataLength' => $dlv,
                'ecc' => $ecc,
                'eccLength' => $elv,
            ];
            $this->ecccode = \array_merge(\array_slice($this->ecccode, 0, $eccPos), $ecc);
            $dataPos += $dlv;
            $eccPos += $elv;
            ++$blockNo;
        }
    }

    /**
     * Initialize a Reed-Solomon codec and add it to existing rsitems
     *
     * @param int $symsize Symbol size, bits
     * @param int $gfpoly  Field generator polynomial coefficients
     * @param int $fcr     First root of RS code generator polynomial, index form
     * @param int $prim    Primitive element to generate polynomial roots
     * @param int $nroots  RS code generator polynomial degree (number of roots)
     * @param int $pad     Padding bytes at front of shortened block
     *
     * @return RSItem Array of RS values:
     *          mm = Bits per symbol;
     *          nn = Symbols per block;
     *          alpha_to = log lookup table array;
     *          index_of = Antilog lookup table array;
     *          genpoly = Generator polynomial array;
     *          nroots = Number of generator;
     *          roots = number of parity symbols;
     *          fcr = First consecutive root, index form;
     *          prim = Primitive element, index form;
     *          iprim = prim-th root of 1, index form;
     *          pad = Padding bytes in shortened block;
     *          gfpoly.
     *
     * @throws BarcodeException in case RS initialization fails
     */
    protected function initRs(int $symsize, int $gfpoly, int $fcr, int $prim, int $nroots, int $pad): array
    {
        foreach ($this->rsitems as $rsv) {
            if ($rsv['pad'] !== $pad) {
                continue;
            }

            if ($rsv['nroots'] !== $nroots) {
                continue;
            }

            if ($rsv['mm'] !== $symsize) {
                continue;
            }

            if ($rsv['gfpoly'] !== $gfpoly) {
                continue;
            }

            if ($rsv['fcr'] !== $fcr) {
                continue;
            }

            if ($rsv['prim'] !== $prim) {
                continue;
            }

            return $rsv;
        }

        $rsv = $this->initRsChar($symsize, $gfpoly, $fcr, $prim, $nroots, $pad);
        \array_unshift($this->rsitems, $rsv);
        return $rsv;
    }

    /**
     * modnn
     *
     * @param RSItem $rsv  RS values
     * @param int   $xpos X position
     *
     * @return int X position
     */
    protected function modnn(array $rsv, int $xpos): int
    {
        while ($xpos >= $rsv['nn']) {
            $xpos -= $rsv['nn'];
            $xpos = ($xpos >> $rsv['mm']) + ($xpos & $rsv['nn']);
        }

        return $xpos;
    }

    /**
     * Check the params for the initRsChar and throws an exception in case of error.
     *
     * @param int $symsize Symbol size, bits
     * @param int $fcr     First root of RS code generator polynomial, index form
     * @param int $prim    Primitive element to generate polynomial roots
     *
     * @throws BarcodeException in case of error
     */
    protected function checkRsCharParamsA(int $symsize, int $fcr, int $prim): void
    {
        $shfsymsize = 1 << $symsize;
        if ($symsize < 0 || $symsize > 8 || $fcr < 0 || $fcr >= $shfsymsize || $prim <= 0 || $prim >= $shfsymsize) {
            throw new BarcodeException('Invalid parameters');
        }
    }

    /**
     * Check the params for the initRsChar and throws an exception in case of error.
     *
     * @param int $symsize Symbol size, bits
     * @param int $nroots  RS code generator polynomial degree (number of roots)
     * @param int $pad     Padding bytes at front of shortened block
     *
     * @throws BarcodeException in case of error
     */
    protected function checkRsCharParamsB(int $symsize, int $nroots, int $pad): void
    {
        $shfsymsize = 1 << $symsize;
        if ($nroots < 0 || $nroots >= $shfsymsize || $pad < 0 || $pad >= ($shfsymsize - 1 - $nroots)) {
            throw new BarcodeException('Invalid parameters');
        }
    }

    /**
     * Initialize a Reed-Solomon codec and returns an array of values.
     *
     * @param int $symsize Symbol size, bits
     * @param int $gfpoly  Field generator polynomial coefficients
     * @param int $fcr     First root of RS code generator polynomial, index form
     * @param int $prim    Primitive element to generate polynomial roots
     * @param int $nroots  RS code generator polynomial degree (number of roots)
     * @param int $pad     Padding bytes at front of shortened block
     *
     * @return RSItem Array of RS values:
     *          mm = Bits per symbol;
     *          nn = Symbols per block;
     *          alpha_to = log lookup table array;
     *          index_of = Antilog lookup table array;
     *          genpoly = Generator polynomial array;
     *          nroots = Number of generator;
     *          roots = number of parity symbols;
     *          fcr = First consecutive root, index form;
     *          prim = Primitive element, index form;
     *          iprim = prim-th root of 1, index form;
     *          pad = Padding bytes in shortened block;
     *          gfpoly.
     *
     * @throws BarcodeException in case the field generator polynomial is invalid
     */
    protected function initRsChar(int $symsize, int $gfpoly, int $fcr, int $prim, int $nroots, int $pad): array
    {
        $this->checkRsCharParamsA($symsize, $fcr, $prim);
        $this->checkRsCharParamsB($symsize, $nroots, $pad);
        $nn = (1 << $symsize) - 1;
        $alphaTo = \array_fill(0, \max(0, $nn + 1), 0);
        $indexOf = \array_fill(0, \max(0, $nn + 1), 0);
        // Generate Galois field lookup tables
        $indexOf[0] = $nn; // \log(zero) = -inf
        $alphaTo[$nn] = 0; // alpha**-inf = 0
        $srv = 1;
        for ($idx = 0; $idx < $nn; ++$idx) {
            $indexOf[$srv] = $idx;
            $alphaTo[$idx] = $srv;
            $srv <<= 1;
            if (($srv & (1 << $symsize)) !== 0) {
                $srv ^= $gfpoly;
            }

            $srv &= $nn;
        }

        if ($srv !== 1) {
            throw new BarcodeException('field generator polynomial is not primitive!');
        }

        // form RS code generator polynomial from its roots
        $genpoly = \array_fill(0, \max(0, $nroots + 1), 0);
        // find prim-th root of 1, used in decoding

        $iprim = 1;
        while (($iprim % $prim) !== 0) {
            $iprim += $nn;
        }

        $iprim = (int) ($iprim / $prim);
        $genpoly[0] = 1;
        for ($idx = 0, $root = $fcr * $prim; $idx < $nroots; ++$idx, $root += $prim) {
            $genpoly[$idx + 1] = 1;
            // multiply rs->genpoly[] by  @**(root + x)
            for ($jdx = $idx; $jdx > 0; --$jdx) {
                $genpolyVal = $genpoly[$jdx] ?? 0;
                if ($genpolyVal !== 0) {
                    $prev = $genpoly[$jdx - 1] ?? 0;
                    $polyIndex = 0;
                    if ($genpolyVal >= 0) {
                        $polyIndex = $indexOf[$genpolyVal] ?? 0;
                    }
                    $alphaIndex = $this->modnnRaw($nn, $symsize, $polyIndex + $root);
                    $genpoly[$jdx] = $prev ^ ($alphaTo[$alphaIndex] ?? 0);
                    continue;
                }

                $genpoly[$jdx] = $genpoly[$jdx - 1] ?? 0;
            }

            // rs->genpoly[0] can never be zero
            $alphaIndex = $this->modnnRaw($nn, $symsize, ($indexOf[$genpoly[0] ?? 0] ?? 0) + $root);
            $genpoly[0] = $alphaTo[$alphaIndex] ?? 0;
        }

        // convert rs->genpoly[] to index form for quicker encoding
        for ($idx = 0; $idx <= $nroots; ++$idx) {
            $genpoly[$idx] = $indexOf[$genpoly[$idx] ?? 0] ?? 0;
        }

        return [
            'alpha_to' => $alphaTo,
            'fcr' => $fcr,
            'genpoly' => $genpoly,
            'gfpoly' => $gfpoly,
            'index_of' => $indexOf,
            'iprim' => $iprim,
            'mm' => $symsize,
            'nn' => $nn,
            'nroots' => $nroots,
            'pad' => $pad,
            'prim' => $prim,
        ];
    }

    protected function modnnRaw(int $nn, int $mm, int $xpos): int
    {
        while ($xpos >= $nn) {
            $xpos -= $nn;
            $xpos = ($xpos >> $mm) + ($xpos & $nn);
        }

        return $xpos;
    }

    /**
     * Encode a Reed-Solomon codec and returns the parity array
     *
     * @param RSItem $rsv    RS values
     * @param array<int, int> $data   Data
     * @param array<int, int> $parity Parity
     *
     * @return array<int, int> Parity array
     */
    protected function encodeRsChar(array $rsv, array $data, array $parity): array
    {
        $nn = $rsv['nn'];
        $alphaTo = $rsv['alpha_to'];
        $indexOf = $rsv['index_of'];
        $genpoly = $rsv['genpoly'];
        $nroots = $rsv['nroots'];
        $pad = $rsv['pad'];
        $parity = \array_values($parity);
        $parity = \array_fill(0, \max(0, $nroots), 0);
        for ($idx = 0; $idx < ($nn - $nroots - $pad); ++$idx) {
            $feedback = $indexOf[($data[$idx] ?? 0) ^ ($parity[0] ?? 0)] ?? 0;
            if ($feedback !== $nn) {
                // feedback term is non-zero
                // This line is unnecessary when GENPOLY[NROOTS] is unity, as it must
                // always be for the polynomials constructed by initRs()
                $feedback = $this->modnn($rsv, $nn - ($genpoly[$nroots] ?? 0) + $feedback);
                for ($jdx = 1; $jdx < $nroots; ++$jdx) {
                    $parity[$jdx] =
                        ($parity[$jdx] ?? 0)
                        ^ ($alphaTo[$this->modnn($rsv, $feedback + ($genpoly[$nroots - $jdx] ?? 0))] ?? 0);
                }
            }

            // Shift
            \array_shift($parity);
            $parity[] = $feedback !== $nn ? $alphaTo[$this->modnn($rsv, $feedback + ($genpoly[0] ?? 0))] ?? 0 : 0;
        }

        return \array_values($parity);
    }
}
