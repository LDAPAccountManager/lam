<?php

declare(strict_types=1);

/**
 * Predictor.php
 *
 * @since     2026-08-27
 * @category  Library
 * @package   PdfFilter
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 *
 * This file is part of tc-lib-pdf-filter software library.
 */

namespace Com\Tecnick\Pdf\Filter;

use Com\Tecnick\Pdf\Filter\Exception as PPException;

/**
 * Com\Tecnick\Pdf\Filter\Predictor
 *
 * Reverses the predictor applied before LZW or Flate compression
 * (PDF 32000-1:2008 Table 8).
 *
 * Predictor 1 (default) leaves the data unchanged, 2 is the TIFF horizontal
 * differencing predictor, and 10 to 15 are the PNG predictors of RFC 2083,
 * where each row is prefixed by the tag of the algorithm used to encode it.
 *
 * @since     2026-08-27
 * @category  Library
 * @package   PdfFilter
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 */
class Predictor
{
    /**
     * @var int Largest accepted Colors value.
     */
    private const MAX_COLORS = 0xFFFF;

    /**
     * @var int Largest accepted Columns value.
     */
    private const MAX_COLUMNS = 0x7FFF_FFFF;

    /**
     * Reverse the predictor described by the DecodeParms.
     *
     * @param string               $data   Decompressed data.
     * @param array<string, mixed> $params DecodeParms dictionary.
     *   - 'Predictor' (int): 1 = none (default), 2 = TIFF, 10 to 15 = PNG.
     *   - 'Colors' (int): colour components per sample, 1 to 65535 (default 1).
     *   - 'BitsPerComponent' (int): bits per component: 1, 2, 4, 8 or 16 (default 8).
     *   - 'Columns' (int): samples per row, 1 to 2147483647 (default 1).
     *
     * @return string Data with the prediction removed.
     *
     * @throws PPException
     */
    public function apply(string $data, array $params = []): string
    {
        $predictor = (int) ($params['Predictor'] ?? 1);
        if ($predictor <= 1) {
            return $data;
        }

        // PDF 32000-1:2008 Table 8 defines 1, 2 and 10 to 15; anything else is malformed
        if ($predictor !== 2 && ($predictor < 10 || $predictor > 15)) {
            throw new PPException('unsupported Predictor: ' . $predictor);
        }

        // validated before the empty-data shortcut, so malformed parameters are
        // reported whatever the length of the stream
        [$bpp, $rowlen] = $this->rowGeometry($params);

        if ($data === '') {
            return '';
        }

        if ($predictor === 2) {
            $bpc = (int) ($params['BitsPerComponent'] ?? 8);
            $colors = \max(1, (int) ($params['Colors'] ?? 1));

            return $this->tiffPredictor($data, $bpc, $colors, $rowlen);
        }

        return $this->pngPredictor($data, $bpp, $rowlen);
    }

    /**
     * Bytes per pixel (the prediction stride) and bytes per row.
     *
     * BitsPerComponent, Colors and Columns are bounded before being multiplied,
     * so the product stays inside PHP_INT_MAX.
     *
     * @param array<string, mixed> $params DecodeParms dictionary.
     *
     * @return array{int, int} Bytes per pixel and bytes per row.
     *
     * @throws PPException
     */
    private function rowGeometry(array $params): array
    {
        $bpc = (int) ($params['BitsPerComponent'] ?? 8);
        if (!\in_array($bpc, [1, 2, 4, 8, 16], true)) {
            throw new PPException('unsupported BitsPerComponent: ' . $bpc);
        }

        $colors = \max(1, (int) ($params['Colors'] ?? 1));
        if ($colors > self::MAX_COLORS) {
            throw new PPException('unsupported Colors: ' . $colors);
        }

        $columns = \max(1, (int) ($params['Columns'] ?? 1));
        if ($columns > self::MAX_COLUMNS) {
            throw new PPException('unsupported Columns: ' . $columns);
        }

        return [
            \max(1, \intdiv(($colors * $bpc) + 7, 8)),
            \max(1, \intdiv(($colors * $bpc * $columns) + 7, 8)),
        ];
    }

    /**
     * Reverse the TIFF horizontal differencing predictor (Predictor 2).
     *
     * @param string $data   Predicted data.
     * @param int    $bpc    Bits per component.
     * @param int    $colors Colour components per sample.
     * @param int    $rowlen Bytes per row.
     *
     * @return string Data with the prediction removed.
     *
     * @throws PPException
     */
    private function tiffPredictor(string $data, int $bpc, int $colors, int $rowlen): string
    {
        if ($bpc !== 8 && $bpc !== 16) {
            // sub-byte components would need bit-level differencing
            throw new PPException('TIFF Predictor 2 requires BitsPerComponent 8 or 16');
        }

        $out = '';
        $rows = \str_split($data, \max(1, $rowlen));
        foreach ($rows as $row) {
            $out .= $bpc === 8 ? $this->tiffRowEightBit($row, $colors) : $this->tiffRowSixteenBit($row, $colors);
        }

        return $out;
    }

    /**
     * Reverse horizontal differencing over one row of 8-bit components.
     *
     * @param string $row    Predicted row.
     * @param int    $colors Colour components per sample.
     *
     * @return string Row with the prediction removed.
     */
    private function tiffRowEightBit(string $row, int $colors): string
    {
        /** @var array<int, int> $bytes */
        $bytes = \array_values((array) \unpack('C*', $row));
        $count = \count($bytes);
        for ($i = $colors; $i < $count; ++$i) {
            $bytes[$i] = (($bytes[$i] ?? 0) + ($bytes[$i - $colors] ?? 0)) & 0xFF;
        }

        return \pack('C*', ...$bytes);
    }

    /**
     * Reverse horizontal differencing over one row of 16-bit components.
     *
     * @param string $row    Predicted row.
     * @param int    $colors Colour components per sample.
     *
     * @return string Row with the prediction removed.
     */
    private function tiffRowSixteenBit(string $row, int $colors): string
    {
        // a trailing odd byte cannot hold a component: leave it untouched
        $tail = (\strlen($row) % 2) === 0 ? '' : \substr($row, -1);
        if ($tail !== '') {
            $row = \substr($row, 0, -1);
        }

        /** @var array<int, int> $words */
        $words = \array_values((array) \unpack('n*', $row));
        $count = \count($words);
        for ($i = $colors; $i < $count; ++$i) {
            $words[$i] = (($words[$i] ?? 0) + ($words[$i - $colors] ?? 0)) & 0xFFFF;
        }

        return \pack('n*', ...$words) . $tail;
    }

    /**
     * Reverse the PNG predictors (RFC 2083), one algorithm tag per row.
     *
     * @param string $data   Predicted data, each row prefixed by its tag byte.
     * @param int    $bpp    Bytes per pixel (prediction stride).
     * @param int    $rowlen Bytes per row, excluding the tag byte.
     *
     * @return string Data with the prediction removed.
     *
     * @throws PPException
     */
    private function pngPredictor(string $data, int $bpp, int $rowlen): string
    {
        $length = \strlen($data);
        // checked before the row buffer is sized from $rowlen
        if (($rowlen + 1) > $length) {
            throw new PPException('invalid code: truncated predictor row');
        }

        $out = '';
        $prev = \array_fill(0, \max(0, $rowlen), 0);
        $pos = 0;

        while ($pos < $length) {
            if (($pos + 1 + $rowlen) > $length) {
                throw new PPException('invalid code: truncated predictor row');
            }

            $tag = \ord($data[$pos]);
            /** @var array<int, int> $row */
            $row = \array_values((array) \unpack('C*', \substr($data, $pos + 1, $rowlen)));
            $pos += 1 + $rowlen;

            $row = $this->pngRow($tag, $row, $prev, $bpp, $rowlen);
            $out .= \pack('C*', ...$row);
            $prev = $row;
        }

        return $out;
    }

    /**
     * Reverse one PNG-predicted row.
     *
     * @param int             $tag    Row algorithm tag (0 to 4).
     * @param array<int, int> $row    Predicted row bytes.
     * @param array<int, int> $prev   Bytes of the row above, already decoded.
     * @param int             $bpp    Bytes per pixel (prediction stride).
     * @param int             $rowlen Bytes per row.
     *
     * @return array<int, int> Row with the prediction removed.
     *
     * @throws PPException
     */
    private function pngRow(int $tag, array $row, array $prev, int $bpp, int $rowlen): array
    {
        return match ($tag) {
            0 => $row,
            1 => $this->pngRowSub($row, $bpp, $rowlen),
            2 => $this->pngRowUp($row, $prev, $rowlen),
            3 => $this->pngRowAverage($row, $prev, $bpp, $rowlen),
            4 => $this->pngRowPaeth($row, $prev, $bpp, $rowlen),
            default => throw new PPException('invalid code: unknown predictor row tag ' . $tag),
        };
    }

    /**
     * Reverse row filter 1 (Sub): each byte was differenced against the one $bpp to its left.
     *
     * The first $bpp bytes have no left neighbour and are left unchanged.
     *
     * @param array<int, int> $row    Predicted row bytes.
     * @param int             $bpp    Bytes per pixel (prediction stride).
     * @param int             $rowlen Bytes per row.
     *
     * @return array<int, int> Row with the prediction removed.
     */
    private function pngRowSub(array $row, int $bpp, int $rowlen): array
    {
        for ($i = $bpp; $i < $rowlen; ++$i) {
            $row[$i] = (($row[$i] ?? 0) + ($row[$i - $bpp] ?? 0)) & 0xFF;
        }

        return $row;
    }

    /**
     * Reverse row filter 2 (Up): each byte was differenced against the byte above it.
     *
     * @param array<int, int> $row    Predicted row bytes.
     * @param array<int, int> $prev   Bytes of the row above, already decoded.
     * @param int             $rowlen Bytes per row.
     *
     * @return array<int, int> Row with the prediction removed.
     */
    private function pngRowUp(array $row, array $prev, int $rowlen): array
    {
        for ($i = 0; $i < $rowlen; ++$i) {
            $row[$i] = (($row[$i] ?? 0) + ($prev[$i] ?? 0)) & 0xFF;
        }

        return $row;
    }

    /**
     * Reverse row filter 3 (Average): the predictor is floor((left + up) / 2).
     *
     * Over the first $bpp bytes the left neighbour is 0, so it reduces to up >> 1.
     *
     * @param array<int, int> $row    Predicted row bytes.
     * @param array<int, int> $prev   Bytes of the row above, already decoded.
     * @param int             $bpp    Bytes per pixel (prediction stride).
     * @param int             $rowlen Bytes per row.
     *
     * @return array<int, int> Row with the prediction removed.
     */
    private function pngRowAverage(array $row, array $prev, int $bpp, int $rowlen): array
    {
        $head = \min($bpp, $rowlen);
        for ($i = 0; $i < $head; ++$i) {
            $row[$i] = (($row[$i] ?? 0) + (($prev[$i] ?? 0) >> 1)) & 0xFF;
        }

        for ($i = $bpp; $i < $rowlen; ++$i) {
            $row[$i] = (($row[$i] ?? 0) + ((($row[$i - $bpp] ?? 0) + ($prev[$i] ?? 0)) >> 1)) & 0xFF;
        }

        return $row;
    }

    /**
     * Reverse row filter 4 (Paeth): the neighbour closest to the linear estimate.
     *
     * Over the first $bpp bytes the left and upper-left neighbours are 0, which
     * reduces the predictor to the byte above.
     *
     * @param array<int, int> $row    Predicted row bytes.
     * @param array<int, int> $prev   Bytes of the row above, already decoded.
     * @param int             $bpp    Bytes per pixel (prediction stride).
     * @param int             $rowlen Bytes per row.
     *
     * @return array<int, int> Row with the prediction removed.
     */
    private function pngRowPaeth(array $row, array $prev, int $bpp, int $rowlen): array
    {
        $head = \min($bpp, $rowlen);
        for ($i = 0; $i < $head; ++$i) {
            $row[$i] = (($row[$i] ?? 0) + ($prev[$i] ?? 0)) & 0xFF;
        }

        for ($i = $bpp; $i < $rowlen; ++$i) {
            $predicted = $this->paeth($row[$i - $bpp] ?? 0, $prev[$i] ?? 0, $prev[$i - $bpp] ?? 0);
            $row[$i] = (($row[$i] ?? 0) + $predicted) & 0xFF;
        }

        return $row;
    }

    /**
     * Paeth predictor of RFC 2083: the neighbour closest to the linear estimate
     * left + up - upLeft.
     *
     * @param int $left   Byte to the left.
     * @param int $up     Byte above.
     * @param int $upLeft Byte above and to the left.
     */
    private function paeth(int $left, int $up, int $upLeft): int
    {
        $distLeft = \abs($up - $upLeft);
        $distUp = \abs($left - $upLeft);
        $distUpLeft = \abs($left + $up - $upLeft - $upLeft);

        if ($distLeft <= $distUp && $distLeft <= $distUpLeft) {
            return $left;
        }

        return $distUp <= $distUpLeft ? $up : $upLeft;
    }
}
