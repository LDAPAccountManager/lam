<?php

declare(strict_types=1);

/**
 * Output.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfImage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-image
 *
 * This file is part of tc-lib-pdf-image software library.
 */

namespace Com\Tecnick\Pdf\Image;

use Com\Tecnick\File\File as ObjFile;
use Com\Tecnick\Pdf\Encrypt\Encrypt;
use Com\Tecnick\Pdf\Image\Exception as ImageException;

/**
 * Com\Tecnick\Pdf\Image\Output
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfImage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-image
 *
 * @phpstan-type TFileOptions array{
 *   allowedHosts?: array<string>,
 *   maxRemoteSize?: int,
 *   curlopts?: array<int, bool|int|string>,
 *   defaultCurlOpts?: array<int, bool|int|string>,
 *   fixedCurlOpts?: array<int, bool|int|string>
 * }
 *
 * @phpstan-import-type ImageBaseData from \Com\Tecnick\Pdf\Image\Import
 * @phpstan-import-type ImageRawData from \Com\Tecnick\Pdf\Image\Import
 *
 * @SuppressWarnings("PHPMD.ExcessiveClassComplexity")
 */
abstract class Output
{
    /**
     * Current PDF object number.
     */
    protected int $pon;

    /**
     * Store image object IDs for the XObject Dictionary.
     *
     * @var array<string, int>
     */
    protected array $xobjdict = [];

    /**
     * Stack of added images.
     *
     * @var array<int, array{
     *          'iid': int,
     *          'key': string,
     *          'width': int,
     *          'height': int,
     *          'defprint': bool,
     *          'altimgs'?: array<int, int>,
     *      }>
     */
    protected array $image = [];

    /**
     * Cache used to store imported image data.
     * The same image data can be reused multiple times.
     *
     * @var array<string, ImageRawData>
     */
    protected array $cache = [];

    /**
     * Initialize images data.
     *
     * @param float                  $kunit      Unit of measure conversion ratio.
     * @param Encrypt                 $encrypt    Encrypt object.
     * @param ObjFile                 $fileHelper File helper for image loading and writing.
     * @param bool                    $pdfa       True if we are in PDF/A mode.
     * @param bool                    $compress   Set to false to disable stream compression.
     * @param ?ImageCacheInterface    $imageCache Optional external cache to persist processed image
     *                                            data across instances and processes (null = disabled).
     */
    public function __construct(
        protected float $kunit,
        protected Encrypt $encrypt,
        protected ObjFile $fileHelper,
        protected bool $pdfa = false,
        protected bool $compress = true,
        protected ?ImageCacheInterface $imageCache = null,
    ) {
        $this->fileHelper = $fileHelper;
    }

    /**
     * Returns current PDF object number.
     */
    public function getObjectNumber(): int
    {
        return $this->pon;
    }

    /**
     * Run a callback with a temporary file helper.
     *
     * @template TResult
     *
     * @param ObjFile $fileHelper Temporary file helper to use while the callback executes.
     * @param callable(): TResult $callback Callback executed with the temporary file helper.
     *
     * @return TResult
     */
    public function withFileHelper(ObjFile $fileHelper, callable $callback): mixed
    {
        $previousFileHelper = $this->fileHelper;
        $this->fileHelper = $fileHelper;

        try {
            return $callback();
        } finally {
            $this->fileHelper = $previousFileHelper;
        }
    }

    /**
     * Get the PDF output string to print the specified image ID.
     *
     * @param int   $iid        Image ID.
     * @param float $xpos       Abscissa (X coordinate) of the upper-left Image corner in user units.
     * @param float $ypos       Ordinate (Y coordinate) of the upper-left Image corner in user units.
     * @param float $width      Image width in user units.
     * @param float $height     Image height in user units.
     * @param float $pageheight Page height in user units.
     *
     * @return string Image PDF page content.
     *
     * @throws \Com\Tecnick\Pdf\Image\Exception If the image ID is not found.
     */
    public function getSetImage(
        int $iid,
        float $xpos,
        float $ypos,
        float $width,
        float $height,
        float $pageheight,
    ): string {
        $img = $this->image[$iid] ?? null;
        if ($img === null) {
            throw new ImageException('Unknownn image ID: ' . $iid);
        }

        $out = 'q';
        $out .= \sprintf(
            ' %F 0 0 %F %F %F cm',
            $width * $this->kunit,
            $height * $this->kunit,
            $xpos * $this->kunit,
            ($pageheight - $ypos - $height) * $this->kunit, // reverse coordinate
        );

        if (!isset($this->cache[$img['key']]['mask'])) {
            return $out . ' /IMG' . $iid . ' Do Q' . "\n";
        }

        if (!isset($this->cache[$img['key']]['plain'])) {
            return $out . ' /IMGmask' . $iid . ' Do Q' . "\n";
        }

        return $out . ' /IMGplain' . $iid . ' Do Q' . "\n";
    }

    /**
     * Get the PDF output string for Images.
     *
     * @param int $pon Current PDF Object Number.
     *
     * @return string PDF code for the images block.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception If image object encryption fails.
     */
    public function getOutImagesBlock(int $pon): string
    {
        $this->pon = $pon;
        $out = '';
        foreach ($this->image as $iid => $img) {
            if (!isset($this->cache[$img['key']]['out'])) {
                if (isset($this->cache[$img['key']]['mask'])) {
                    /** @var ImageRawData $mask */
                    $mask = &$this->cache[$img['key']]['mask'];
                    $out .= $this->getOutImage($img, $mask, 'mask');
                    if (isset($this->cache[$img['key']]['plain'])) {
                        /** @var ImageRawData $plain */
                        $plain = &$this->cache[$img['key']]['plain'];
                        $out .= $this->getOutImage($img, $plain, 'plain');
                    }
                }

                if (!isset($this->cache[$img['key']]['mask'])) {
                    $out .= $this->getOutImage($img, $this->cache[$img['key']]);
                }

                unset($mask, $plain);

                $this->image[$iid] = $img;
            }

            if (($this->cache[$img['key']]['mask']['obj'] ?? 0) === 0) {
                $this->xobjdict['IMG' . $img['iid']] = $this->cache[$img['key']]['obj'];
            } else {
                $maskData = $this->cache[$img['key']]['mask'] ?? null;
                if ($maskData === null) {
                    $this->xobjdict['IMG' . $img['iid']] = $this->cache[$img['key']]['obj'];
                    continue;
                }

                $plainData = $this->cache[$img['key']]['plain'] ?? null;
                if (($plainData['obj'] ?? 0) === 0) {
                    $this->xobjdict['IMGmask' . $img['iid']] = (int) $maskData['obj'];
                } elseif ($plainData !== null) {
                    $this->xobjdict['IMGplain' . $img['iid']] = (int) $plainData['obj'];
                }
            }
        }

        return $out;
    }

    /**
     * Get the PDF output string for Image object.
     *
     * @param array{
     *          'iid': int,
     *          'key': string,
     *          'width': int,
     *          'height': int,
     *          'defprint': bool,
     *          'altimgs'?: array<int, int>,
     *      }  $img  Image reference.
     * @param ImageRawData  $data Image raw data.
     * @param string $sub  Sub image ('mask', 'plain' or empty string).
     *
     * @return string PDF Image object.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception If image data encryption fails.
     */
    protected function getOutImage(array &$img, array &$data, string $sub = ''): string
    {
        $out = $this->getOutIcc($data) . $this->getOutPalette($data) . $this->getOutAltImages($img, $data, $sub);

        $data['obj'] = ++$this->pon;

        $out .=
            $data['obj']
            . ' 0 obj'
            . "\n"
            . '<<'
            . ' /Type /XObject'
            . ' /Subtype /Image'
            . ' /Width '
            . $data['width']
            . ' /Height '
            . $data['height']
            . $this->getOutColorInfo($data);

        if ($data['exturl']) {
            // external stream
            $out .=
                ' /Length 0 /F'
                . ' <<'
                . ' /FS /URL /F '
                . $this->encrypt->escapeDataString('true', $this->pon)
                . ' >>';
            if ($data['filter'] !== '') {
                $out .= ' /FFilter /' . $data['filter'];
            }

            $out .= ' >>' . "\n";
        } else {
            if ($data['filter'] !== '') {
                $out .= ' /Filter /' . $data['filter'];
            }

            if ($data['parms'] !== '') {
                $out .= ' ' . $data['parms'];
            }

            // Colour Key Masking
            if ($data['trns'] !== []) {
                $trns = $this->getOutTransparency($data);
                if ($trns !== '') {
                    $out .= ' /Mask [ ' . $trns . ']';
                }
            }

            $stream = $this->encrypt->encryptString($data['data'], $this->pon);
            $out .= ' /Length ' . \strlen($stream) . '>> stream' . "\n" . $stream . "\n" . 'endstream' . "\n";
        }

        $out .= 'endobj' . "\n";

        $this->cache[$img['key']]['out'] = true; // mark this as done

        return $out;
    }

    /**
     * Return XObjects Dictionary portion for the images.
     */
    public function getXobjectDict(): string
    {
        $out = '';
        foreach ($this->xobjdict as $iid => $objid) {
            $out .= ' /' . $iid . ' ' . $objid . ' 0 R';
        }

        return $out;
    }

    /**
     * Return XObjects Dictionary.
     *
     * @param array<int> $keys Image IDs.
     */
    public function getXobjectDictByKeys(array $keys): string
    {
        if ($keys === []) {
            return '';
        }

        $out = '';

        foreach ($keys as $iid) {
            $key = 'IMG' . $iid;
            if (isset($this->xobjdict[$key])) {
                $out .= ' /' . $key . ' ' . $this->xobjdict[$key] . ' 0 R';
                continue;
            }
            $key = 'IMGplain' . $iid;
            if (isset($this->xobjdict[$key])) {
                $out .= ' /' . $key . ' ' . $this->xobjdict[$key] . ' 0 R';
                continue;
            }
            $key = 'IMGmask' . $iid;
            if (isset($this->xobjdict[$key])) {
                $out .= ' /' . $key . ' ' . $this->xobjdict[$key] . ' 0 R';
                continue;
            }
        }

        return $out;
    }

    /**
     * Get the PDF output string for ICC object.
     *
     * @param ImageRawData $data Image raw data.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception If ICC profile encryption fails.
     */
    protected function getOutIcc(array &$data): string
    {
        if ($data['icc'] === '') {
            return '';
        }

        $data['obj_icc'] = ++$this->pon;
        $out =
            $data['obj_icc']
            . ' 0 obj'
            . "\n"
            . '<<'
            . ' /N '
            . $data['channels']
            . ' /Alternate /'
            . $data['colspace'];
        $icc = $data['icc'];
        if ($this->compress) {
            $out .= ' /Filter /FlateDecode';
            $cicc = \gzcompress($icc);
            if ($cicc !== false) {
                $icc = $cicc;
            }
        }

        $stream = $this->encrypt->encryptString($icc, $this->pon);
        return (
            $out . (
                ' /Length '
                . \strlen($stream)
                . ' >>'
                . ' stream'
                . "\n"
                . $stream
                . "\n"
                . 'endstream'
                . "\n"
                . 'endobj'
                . "\n"
            )
        );
    }

    /**
     * Get the PDF output string for Indexed palette object.
     *
     * @param ImageRawData $data Image raw data.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception If palette encryption fails.
     */
    protected function getOutPalette(array &$data): string
    {
        if ($data['colspace'] !== 'Indexed') {
            return '';
        }

        $data['obj_pal'] = ++$this->pon;
        $out = $data['obj_pal'] . ' 0 obj' . "\n" . '<<';
        $pal = $data['pal'];
        if ($this->compress) {
            $out .= '/Filter /FlateDecode';
            $cpal = \gzcompress($pal);
            if ($cpal !== false) {
                $pal = $cpal;
            }
        }

        $stream = $this->encrypt->encryptString($pal, $this->pon);
        return (
            $out . (
                ' /Length '
                . \strlen($stream)
                . '>>'
                . ' stream'
                . "\n"
                . $stream
                . "\n"
                . 'endstream'
                . "\n"
                . 'endobj'
                . "\n"
            )
        );
    }

    /**
     * Get the PDF output string for color and mask information.
     *
     * @param ImageRawData $data Image raw data.
     */
    protected function getOutColorInfo(array $data): string
    {
        $out = '';
        // set color space
        if ($data['obj_icc'] !== 0) {
            // ICC Colour Space
            $out .= ' /ColorSpace [/ICCBased ' . $data['obj_icc'] . ' 0 R]';
        } elseif ($data['obj_pal'] !== 0) {
            // Indexed Colour Space
            $out .=
                ' /ColorSpace [/Indexed /DeviceRGB '
                . ((\strlen($data['pal']) / 3) - 1)
                . ' '
                . $data['obj_pal']
                . ' 0 R]';
        } else {
            // Device Colour Space
            $out .= ' /ColorSpace /' . $data['colspace'];
        }

        if ($data['colspace'] === 'DeviceCMYK') {
            $out .= ' /Decode [1 0 1 0 1 0 1 0]';
        }

        $out .= ' /BitsPerComponent ' . $data['bits'];

        $maskobj = (int) ($this->cache[$data['key']]['mask']['obj'] ?? 0);
        if (!$data['ismask'] && $maskobj > 0) {
            $out .= ' /SMask ' . $maskobj . ' 0 R';
        }

        if ($data['obj_alt'] !== 0) {
            // reference to alternate images dictionary
            $out .= ' /Alternates ' . $data['obj_alt'] . ' 0 R';
        }

        return $out;
    }

    /**
     * Get the PDF output string for Alternate images object.
     *
     * @param array{
     *          'iid': int,
     *          'key': string,
     *          'width': int,
     *          'height': int,
     *          'defprint': bool,
     *          'altimgs'?: array<int, int>,
     *      } $img Image reference.
     * @param ImageRawData $data Image raw data.
     * @param string $sub Sub image ('mask', 'plain' or empty string).
     */
    protected function getOutAltImages(array $img, array &$data, string $sub = ''): string
    {
        if ($this->pdfa || !isset($img['altimgs']) || $img['altimgs'] === [] || $sub === 'mask') {
            return '';
        }

        $data['obj_alt'] = ++$this->pon;

        $out = $this->pon . ' 0 obj' . "\n" . '[';
        foreach ($img['altimgs'] as $iid) {
            $altimg = $this->image[$iid] ?? null;
            if ($altimg === null) {
                continue;
            }

            $altobj = (int) ($this->cache[$altimg['key']]['obj'] ?? 0);
            if ($altobj === 0) {
                continue;
            }

            $out .=
                ' << /Image '
                . $altobj
                . ' 0 R'
                . ' /DefaultForPrinting '
                . ($altimg['defprint'] ? 'true' : 'false')
                . ' >>';
        }

        return $out . (' ]' . "\n" . 'endobj' . "\n");
    }

    /**
     * Get the PDF output string for color transparency.
     *
     * @param ImageRawData $data Image raw data.
     */
    protected function getOutTransparency(array $data): string
    {
        $trns = '';

        if ($data['colspace'] === 'Indexed') {
            // Indexed: trns holds one alpha byte per palette entry; mask the
            // fully-transparent entries (alpha 0) by their palette index.
            foreach ($data['trns'] as $idx => $val) {
                if ($val !== 0) {
                    continue;
                }

                $trns .= $idx . ' ' . $idx . ' ';
            }

            return $trns;
        }

        // DeviceRGB / DeviceGray: trns holds the transparent colour sample
        // values themselves, emitted as single-value colour-key ranges.
        foreach ($data['trns'] as $val) {
            $trns .= $val . ' ' . $val . ' ';
        }

        return $trns;
    }
}
