<?php

declare(strict_types=1);

/**
 * Import.php
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

use Com\Tecnick\Pdf\Image\Exception as ImageException;
use Com\Tecnick\Pdf\Image\Import\ImageImportInterface;

/**
 * Com\Tecnick\Pdf\Image\Import
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfImage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-image
 *
 * @phpstan-type ImageBaseData array{
 *          'bits': int,
 *          'channels': int,
 *          'colspace': string,
 *          'data': string,
 *          'exturl': bool,
 *          'file': string,
 *          'filter': string,
 *          'height': int,
 *          'icc': string,
 *          'ismask': bool,
 *          'key': string,
 *          'mapto': int,
 *          'native': bool,
 *          'obj': int,
 *          'obj_alt': int,
 *          'obj_icc': int,
 *          'obj_pal': int,
 *          'pal': string,
 *          'parms': string,
 *          'raw': string,
 *          'recode': bool,
 *          'recoded': bool,
 *          'splitalpha': bool,
 *          'trns': array<int, int>,
 *          'type': int,
 *          'width': int,
 *        }
 *
 * @phpstan-type ImageRawData array{
 *          'bits': int,
 *          'channels': int,
 *          'colspace': string,
 *          'data': string,
 *          'exturl': bool,
 *          'file': string,
 *          'filter': string,
 *          'height': int,
 *          'icc': string,
 *          'ismask': bool,
 *          'key': string,
 *          'mapto': int,
 *          'mask'?: ImageBaseData,
 *          'native': bool,
 *          'obj': int,
 *          'obj_alt': int,
 *          'obj_icc': int,
 *          'obj_pal': int,
 *          'out'?: true,
 *          'pal': string,
 *          'parms': string,
 *          'plain'?: ImageBaseData,
 *          'raw': string,
 *          'recode': bool,
 *          'recoded': bool,
 *          'splitalpha': bool,
 *          'trns': array<int, int>,
 *          'type': int,
 *          'width': int,
 *      }
 *
 * @SuppressWarnings("PHPMD.ExcessiveClassLength")
 * @SuppressWarnings("PHPMD.ExcessiveClassComplexity")
 */
class Import extends \Com\Tecnick\Pdf\Image\Output
{
    /**
     * Image index.
     * Count the number of added images.
     */
    protected int $iid = 0;

    /**
     * Namespace + schema version prefix for external cache keys.
     * Bump the version when the ImageRawData shape changes so that stale
     * persisted entries are ignored instead of being fed into output.
     */
    private const CACHE_PREFIX = 'tc-lib-pdf-image:v2:';

    /**
     * Native image types and associated importing class.
     * (Image types for which we have an import method).
     *
     * @var array<int, string>
     */
    private const NATIVE = [
        IMAGETYPE_PNG => 'Png',
        IMAGETYPE_JPEG => 'Jpeg',
    ];

    /**
     * Lossless image types.
     *
     * @var array<int>
     */
    protected const LOSSLESS = [
        IMAGETYPE_GIF, // 1
        IMAGETYPE_PNG, // 3
        IMAGETYPE_PSD, // 5
        IMAGETYPE_BMP, // 6
        IMAGETYPE_WBMP, // 15
        IMAGETYPE_XBM, // 16
        IMAGETYPE_TIFF_II, // 7
        IMAGETYPE_TIFF_MM, // 8
        IMAGETYPE_IFF, // 14
        IMAGETYPE_SWC, // 13
        IMAGETYPE_ICO, // 17
    ];

    /**
     * Map number of channels with color space name.
     *
     * @var array<int, string>
     */
    protected const COLSPACEMAP = [
        1 => 'DeviceGray',
        3 => 'DeviceRGB',
        4 => 'DeviceCMYK',
    ];

    /**
     * Add a new image.
     *
     * @param string          $image    Image file name, URL or a '@' character followed by the image data string.
     *                                  To link an image without embedding it on the document, set an asterisk
     *                                  character before the URL (i.e.: '*http://www.example.com/image.jpg').
     * @param ?int            $width    New width in pixels or null to keep the original value.
     * @param ?int            $height   New height in pixels or null to keep the original value.
     * @param bool            $ismask   True if the image is a transparency mask.
     * @param int             $quality  Quality for JPEG files (0 = max compression; 100 = best quality, bigger file).
     * @param bool            $defprint Indicate if the image is the default
     *                                  for printing when used as alternative image.
     * @param array<int, int> $altimgs  Arrays of alternate image keys.
     *
     * @return int Image ID.
     *
     * @throws \Com\Tecnick\Pdf\Image\Exception If the image cannot be added.
     * @throws \Com\Tecnick\File\Exception If reading image content fails.
     */
    public function add(
        string $image,
        ?int $width = null,
        ?int $height = null,
        bool $ismask = false,
        int $quality = 100,
        bool $defprint = false,
        array $altimgs = [],
    ): int {
        $data = $this->import($image, $width, $height, $ismask, $quality);
        ++$this->iid;
        $this->image[$this->iid] = [
            'iid' => $this->iid,
            'key' => $data['key'],
            'width' => $data['width'],
            'height' => $data['height'],
            'defprint' => $defprint,
            'altimgs' => $altimgs,
        ];
        return $this->iid;
    }

    /**
     * Get the Image key used for caching.
     *
     * @param string $image   Image file name or content.
     * @param int    $width   Width in pixels.
     * @param int    $height  Height in pixels.
     * @param int    $quality Quality for JPEG files.
     */
    public function getKey(string $image, int $width = 0, int $height = 0, int $quality = 100): string
    {
        // The parts are joined with a delimiter so that distinct inputs cannot
        // collapse into the same digest (e.g. ('img', 12, 3) vs ('img1', 2, 3)).
        $seed = $image . '|' . $width . '|' . $height . '|' . $quality;
        return \strtr(\rtrim(\base64_encode(\pack('H*', \md5($seed))), '='), '+/', '-_');
    }

    /**
     * Get an imported image by key.
     *
     * @param string $key Image key.
     *
     * @return ImageRawData Image raw data array.
     *
     * @throws \Com\Tecnick\Pdf\Image\Exception If the key is not found.
     */
    public function getImageDataByKey(string $key): array
    {
        $cache = $this->cache[$key] ?? null;
        if ($cache === null) {
            throw new ImageException('Unknownn key');
        }

        return $cache;
    }

    /**
     * Resolve the image dimensions for a cached image.
     *
     * - If both $width and $height are zero, the original image dimensions are returned.
     * - If only one of $width and $height is greater than zero, the other is
     *   calculated preserving the original aspect ratio.
     * - If both are greater than zero, they are returned as-is, unless $fit is
     *   true, in which case the image is scaled to fit within the $width x $height
     *   bounding box preserving the original aspect ratio.
     *
     * @param string $key    Image key.
     * @param int    $width  Target width in pixels (0 = unspecified).
     * @param int    $height Target height in pixels (0 = unspecified).
     * @param bool   $fit    If true and both sides are specified, scale to fit
     *                       within the bounding box preserving the aspect ratio.
     *
     * @return array{width: int, height: int} Image dimensions in pixels.
     *
     * @throws \Com\Tecnick\Pdf\Image\Exception If the key is not found.
     */
    public function getImageDimensionsByKey(string $key, int $width = 0, int $height = 0, bool $fit = false): array
    {
        $data = $this->getImageDataByKey($key);

        [$width, $height] = $this->scaleToAspect(
            $data['width'],
            $data['height'],
            $width > 0 ? $width : null,
            $height > 0 ? $height : null,
            $fit,
        );

        return [
            'width' => $width,
            'height' => $height,
        ];
    }

    /**
     * Import the original image raw data.
     *
     * @param string $image   Image file name, URL or a '@' character followed by the image data string.
     *                        To link an image without embedding it on the document, set an asterisk
     *                        character before the URL (i.e.: '*http://www.example.com/image.jpg').
     * @param ?int   $width   New width in pixels or null to keep the original value.
     * @param ?int   $height  New height in pixels or null to keep the original value.
     * @param bool   $ismask  True if the image is a transparency mask.
     * @param int    $quality Quality for JPEG files (0 = max compression; 100 = best quality, bigger file).
     *
     * @return ImageRawData Image raw data array
     *
     * @throws \Com\Tecnick\Pdf\Image\Exception If the image cannot be imported.
     * @throws \Com\Tecnick\File\Exception If reading image content fails.
     */
    protected function import(
        string $image,
        ?int $width = null,
        ?int $height = null,
        bool $ismask = false,
        int $quality = 100,
    ): array {
        $quality = \max(0, \min(100, $quality));
        $imgkey = $this->getKey($image, (int) $width, (int) $height, $quality);

        // Tier 1: in-process memory cache (fast path, also dedups within a document).
        if (isset($this->cache[$imgkey])) {
            return $this->cache[$imgkey];
        }

        // Tier 2: optional external cache (survives across instances and processes).
        $cache = $this->imageCache;
        $extkey = '';
        if ($cache !== null) {
            $extkey = $this->getExternalCacheKey($image, $imgkey, (int) $width, (int) $height, $quality);
            $cached = $cache->get($extkey);
            if ($cached !== null) {
                $cached['key'] = $imgkey;
                $this->cache[$imgkey] = $cached;
                return $cached;
            }
        }

        $data = $this->getRawData($image);
        $data['key'] = $imgkey;

        [$width, $height] = $this->resolveDimensions($data, $width, $height);

        if (!$data['native'] || $width !== $data['width'] || $height !== $data['height']) {
            $data = $this->getResizedRawData($this->getBaseData($data), $width, $height, true, $quality);
        }

        $data = $this->getData($data, $width, $height, $quality);

        $data = $this->enrichMaskData($data, $width, $height, $quality, $ismask);

        // store data in the in-process cache
        $this->cache[$imgkey] = $data;

        // write-through to the external cache (best-effort, persistable snapshot)
        if ($cache !== null) {
            $cache->set($extkey, $this->getPersistableData($data));
        }

        return $data;
    }

    /**
     * Build the persistent (external) cache key for an image.
     *
     * For local file inputs the file modification time and size are folded into
     * the key, so that editing a file in place invalidates stale persisted
     * entries. Inline data ('@...') and remote URLs are identified by their
     * string, which already carries their content/identity.
     *
     * @param string $image   Image file name, URL or '@'-prefixed data string.
     * @param string $imgkey  Image key already computed from the raw image string.
     * @param int    $width   Width in pixels.
     * @param int    $height  Height in pixels.
     * @param int    $quality Quality for JPEG files.
     */
    private function getExternalCacheKey(string $image, string $imgkey, int $width, int $height, int $quality): string
    {
        $identity = $image;

        if ($image !== '' && $image[0] !== '@') {
            $path = $image[0] === '*' ? \substr($image, 1) : $image;
            if (\is_file($path)) {
                $mtime = \filemtime($path);
                $size = \filesize($path);
                if ($mtime !== false && $size !== false) {
                    $identity = $image . ':' . $mtime . ':' . $size;
                }
            }
        }

        // When the identity is just the raw image string the key is identical
        // to the one already computed by the caller, so reuse it and avoid a
        // second md5/base64 round.
        $key = $identity === $image ? $imgkey : $this->getKey($identity, $width, $height, $quality);

        return self::CACHE_PREFIX . $key;
    }

    /**
     * Produce a persistable snapshot of the imported image data.
     *
     * Drops the original raw bytes (not used at output time) to shrink the
     * stored payload. PDF object numbers and the output flag are intentionally
     * left as imported (all zero / unset), so a loaded entry is ready for a
     * fresh document without any reset.
     *
     * @param ImageRawData $data Imported image data.
     *
     * @return ImageRawData Persistable image data.
     */
    private function getPersistableData(array $data): array
    {
        $data['raw'] = '';

        if (isset($data['mask'])) {
            $data['mask']['raw'] = '';
        }

        if (isset($data['plain'])) {
            $data['plain']['raw'] = '';
        }

        return $data;
    }

    /**
     * Resolve target image dimensions preserving aspect ratio when one side is omitted.
     *
     * @param ImageRawData $data   Image raw data.
     * @param ?int         $width  Target width in pixels (null = unspecified, derive from aspect ratio).
     * @param ?int         $height Target height in pixels (null = unspecified, derive from aspect ratio).
     *
     * @return array{0: int, 1: int}
     */
    private function resolveDimensions(array $data, ?int $width, ?int $height): array
    {
        return $this->scaleToAspect($data['width'], $data['height'], $width, $height);
    }

    /**
     * Scale target dimensions against a source size, preserving the aspect ratio.
     *
     * - If both $width and $height are null (unspecified), the source dimensions
     *   are returned.
     * - If only one side is specified, the other is derived from the source aspect
     *   ratio.
     * - If both sides are specified, they are returned as-is, unless $fit is true,
     *   in which case the result is scaled to fit within the $width x $height box
     *   while preserving the source aspect ratio.
     *
     * An explicit zero (as opposed to null) is kept as-is so that callers can
     * detect and reject invalid target dimensions downstream.
     *
     * @param int  $srcwidth  Source width in pixels.
     * @param int  $srcheight Source height in pixels.
     * @param ?int $width     Target width in pixels (null = unspecified).
     * @param ?int $height    Target height in pixels (null = unspecified).
     * @param bool $fit       Scale to fit within the box when both sides are given.
     *
     * @return array{0: int, 1: int}
     */
    private function scaleToAspect(int $srcwidth, int $srcheight, ?int $width, ?int $height, bool $fit = false): array
    {
        if ($width === null && $height === null) {
            return [\max(0, $srcwidth), \max(0, $srcheight)];
        }

        if ($fit && $width !== null && $width > 0 && $height !== null && $height > 0) {
            return $this->fitToBox($srcwidth, $srcheight, $width, $height);
        }

        if ($width !== null && $height === null && $srcwidth > 0) {
            $height = $this->scaleSide($width, $srcheight, $srcwidth);
        }

        if ($height !== null && $width === null && $srcheight > 0) {
            $width = $this->scaleSide($height, $srcwidth, $srcheight);
        }

        return [\max(0, $width ?? $srcwidth), \max(0, $height ?? $srcheight)];
    }

    /**
     * Scale a single dimension proportionally: round($value * $numerator / $denominator).
     *
     * @param int $value       The known side to scale from (e.g. the specified width or height).
     * @param int $numerator   Source size of the side being computed.
     * @param int $denominator Source size of the side $value corresponds to.
     *
     * @return int Scaled size of the computed side, in pixels.
     */
    private function scaleSide(int $value, int $numerator, int $denominator): int
    {
        return (int) \round(((float) $value * (float) $numerator) / (float) $denominator);
    }

    /**
     * Scale the source size to fit within the $width x $height box, preserving the aspect ratio.
     *
     * @param int $srcwidth  Source width in pixels.
     * @param int $srcheight Source height in pixels.
     * @param int $width     Bounding box width in pixels.
     * @param int $height    Bounding box height in pixels.
     *
     * @return array{0: int, 1: int}
     */
    private function fitToBox(int $srcwidth, int $srcheight, int $width, int $height): array
    {
        if ($srcwidth <= 0 || $srcheight <= 0) {
            return [\max(0, $width), \max(0, $height)];
        }

        $scale = \min((float) $width / (float) $srcwidth, (float) $height / (float) $srcheight);
        return [
            \max(0, (int) \round((float) $srcwidth * $scale)),
            \max(0, (int) \round((float) $srcheight * $scale)),
        ];
    }

    /**
     * Add plain/mask variants when required by mask mode or alpha splitting.
     *
     * @param ImageRawData $data Image raw data.
     *
     * @return ImageRawData
     *
     * @throws \Com\Tecnick\Pdf\Image\Exception If the image cannot be imported.
     */
    private function enrichMaskData(array $data, int $width, int $height, int $quality, bool $ismask): array
    {
        $basedata = $this->getBaseData($data);

        if ($ismask) {
            $data['mask'] = $basedata;
            return $data;
        }

        if (!$data['splitalpha']) {
            return $data;
        }

        // create 2 separate images: plain + mask
        $data['plain'] = $this->createPlainImage($basedata, $width, $height, $quality);
        $data['mask'] = $this->createMaskImage($basedata, $width, $height, $quality);

        return $data;
    }

    /**
     * @param ImageBaseData $basedata Image base data.
     *
     * @return ImageBaseData
     *
     * @throws \Com\Tecnick\Pdf\Image\Exception If the image cannot be imported.
     */
    private function createPlainImage(array $basedata, int $width, int $height, int $quality): array
    {
        $plain = $this->getResizedRawData($basedata, $width, $height, false, $quality);
        $plain = $this->getData($plain, $width, $height, $quality);

        return $this->getBaseData($plain);
    }

    /**
     * @param ImageBaseData $basedata Image base data.
     *
     * @return ImageBaseData
     *
     * @throws \Com\Tecnick\Pdf\Image\Exception If the image cannot be imported.
     */
    private function createMaskImage(array $basedata, int $width, int $height, int $quality): array
    {
        $mask = $this->getAlphaChannelRawData($basedata);
        $mask = $this->getData($mask, $width, $height, $quality);
        $mask = $this->getBaseData($mask);
        $mask['colspace'] = 'DeviceGray';

        return $mask;
    }

    /**
     * Extract the relevant data from the image.
     *
     * @param ImageRawData $data    Image raw data.
     * @param int          $width   Width in pixels.
     * @param int          $height  Height in pixels.
     * @param int          $quality Quality for JPEG files.
     *
     * @return ImageRawData Image raw data array.
     *
     * @throws \Com\Tecnick\Pdf\Image\Exception If the image cannot be imported.
     */
    protected function getData(array $data, int $width, int $height, int $quality): array
    {
        if (!$data['native']) {
            throw new ImageException('Unable to import image');
        }

        $imageImport = $this->createImportImage($data);
        $data = $imageImport->getData($this->getBaseData($data));

        if ($data['recode']) {
            // re-encode the image as it was not possible to decode it
            $data = $this->getResizedRawData($this->getBaseData($data), $width, $height, true, $quality);
            $data = $imageImport->getData($this->getBaseData($data));
        }

        return $data;
    }

    /**
     * @param array{'type': int}|ImageRawData $data Image raw data.
     *
     * @throws \Com\Tecnick\Pdf\Image\Exception If the image type is unknown.
     */
    private function createImportImage(array $data): ImageImportInterface
    {
        if (!isset(self::NATIVE[$data['type']])) {
            throw new ImageException('Unable to import image: unknown type');
        }

        return match (self::NATIVE[$data['type']]) {
            'Png' => new \Com\Tecnick\Pdf\Image\Import\Png(),
            'Jpeg' => new \Com\Tecnick\Pdf\Image\Import\Jpeg(),
            default => throw new ImageException('Unable to import image: unknown type'),
        };
    }

    /**
     * Normalize raw image data to its base shape.
     *
     * @param ImageRawData $data Image raw data.
     *
     * @return ImageBaseData Image base data.
     */
    private function getBaseData(array $data): array
    {
        return [
            'bits' => $data['bits'],
            'channels' => $data['channels'],
            'colspace' => $data['colspace'],
            'data' => $data['data'],
            'exturl' => $data['exturl'],
            'file' => $data['file'],
            'filter' => $data['filter'],
            'height' => $data['height'],
            'icc' => $data['icc'],
            'ismask' => $data['ismask'],
            'key' => $data['key'],
            'mapto' => $data['mapto'],
            'native' => $data['native'],
            'obj' => $data['obj'],
            'obj_alt' => $data['obj_alt'],
            'obj_icc' => $data['obj_icc'],
            'obj_pal' => $data['obj_pal'],
            'pal' => $data['pal'],
            'parms' => $data['parms'],
            'raw' => $data['raw'],
            'recode' => $data['recode'],
            'recoded' => $data['recoded'],
            'splitalpha' => $data['splitalpha'],
            'trns' => $data['trns'],
            'type' => $data['type'],
            'width' => $data['width'],
        ];
    }

    /**
     * Get the original image raw data.
     *
     * @param string $image Image file name, URL or a '@' character followed by the image data string.
     *                      To link an image without embedding it on the document, set an asterisk character
     *                      before the URL (i.e.: '*http://www.example.com/image.jpg').
     *
     * @return ImageRawData Image data array.
     *
     * @throws \Com\Tecnick\Pdf\Image\Exception If the image cannot be read.
     * @throws \Com\Tecnick\File\Exception If reading image content fails.
     */
    protected function getRawData(string $image): array
    {
        // default data to return
        $data = [
            'bits' => 8, // number of bits per channel
            'channels' => 3, // number of channels
            'colspace' => 'DeviceRGB', // color space
            'data' => '', // PDF image data
            'exturl' => false, // true if the image is an exernal URL that should not be embedded
            'file' => '', // source file name or URL
            'filter' => 'FlateDecode', // decoding filter
            'height' => 0, // image height in pixels
            'icc' => '', // ICC profile
            'ismask' => false, // true if the image is a transparency mask
            'key' => '', // image key
            'mapto' => IMAGETYPE_PNG, // type to convert to
            'native' => false, // true if the image is PNG or JPEG
            'obj' => 0, // PDF object number
            'obj_alt' => 0,
            'obj_icc' => 0,
            'obj_pal' => 0,
            'pal' => '', // colour palette
            'parms' => '', // additional PDF decoding parameters
            'raw' => '', // raw image data
            'recode' => false,
            'recoded' => false,
            'splitalpha' => false,
            'trns' => [], // colour key masking
            'type' => 0, // image type constant: IMAGETYPE_XXX
            'width' => 0, // image width in pixels
        ];

        if ($image === '' || ($image[0] === '@' || $image[0] === '*') && \strlen($image) === 1) {
            throw new ImageException('Empty image');
        }

        if ($image[0] === '@') { // image from string
            $data['raw'] = \substr($image, 1);
            return $this->getMetaData($data);
        }

        if ($image[0] === '*') { // not-embedded external URL
            $data['exturl'] = true;
            $image = \substr($image, 1);
        }

        $data['file'] = $image;
        $raw = $this->fileHelper->getFileData($image);
        if ($raw === false) {
            throw new ImageException('Unable to read image file: ' . $image);
        }

        $data['raw'] = $raw;

        return $this->getMetaData($data);
    }

    /**
     * Get the image meta data.
     *
     * @param ImageBaseData $data Image raw data.
     *
     * @return ImageRawData Image raw data array.
     *
     * @throws \Com\Tecnick\Pdf\Image\Exception If the image format is invalid.
     */
    protected function getMetaData(array $data): array
    {
        // getimagesizefromstring() does not throw: it emits warnings (silenced
        // below) and returns false on failure, which is handled right after.
        \set_error_handler(static fn(): bool => true);
        try {
            $meta = \getimagesizefromstring($data['raw']);
        } finally {
            \restore_error_handler();
        }

        if ($meta === false) {
            throw new ImageException('Invalid image format');
        }

        $data['width'] = $meta[0];
        $data['height'] = $meta[1];
        $data['type'] = $meta[2];
        $data['native'] = isset(self::NATIVE[$data['type']]);
        $data['mapto'] = \in_array($data['type'], self::LOSSLESS, true) ? IMAGETYPE_PNG : IMAGETYPE_JPEG;
        /** @var array{'bits'?: int, 'channels'?: int} $meta */
        if (isset($meta['bits'])) {
            $data['bits'] = $meta['bits'];
        }

        if (isset($meta['channels']) && $meta['channels'] !== 0) {
            $data['channels'] = (int) $meta['channels'];
        }

        if (isset(self::COLSPACEMAP[$data['channels']])) {
            $data['colspace'] = self::COLSPACEMAP[$data['channels']];
        }

        return $data;
    }

    /**
     * Get the resized image raw data
     * (always convert the image type to a native format: PNG or JPEG).
     *
     * @param ImageBaseData $data   Image raw data as returned by getImageRawData.
     * @param int          $width   New width in pixels.
     * @param int          $height  New height in pixels.
     * @param bool         $alpha   If true save the alpha channel information,
     *                              if false merge the alpha channel (PNG mode).
     * @param int          $quality Quality for JPEG files
     *                              (0 = max compression; 100 = best quality, bigger file).
     *
     * @return ImageRawData Image raw data array.
     *
     * @throws \Com\Tecnick\Pdf\Image\Exception If the image cannot be resized.
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     * @SuppressWarnings("PHPMD.NPathComplexity")
     */
    protected function getResizedRawData(
        array $data,
        int $width,
        int $height,
        bool $alpha = true,
        int $quality = 100,
    ): array {
        if ($width <= 0 || $height <= 0) {
            throw new ImageException('Image width and/or height are empty');
        }

        $img = \imagecreatefromstring($data['raw']);
        if ($img === false) {
            throw new ImageException('Unable to create new image from string');
        }

        $newimg = \imagecreatetruecolor($width, $height);
        if ($newimg === false) {
            throw new ImageException('Unable to create new resized image');
        }

        \imageinterlace($newimg, false);

        if ($alpha) {
            $trid = \imagecolorallocate($newimg, 0, 0, 0);
            if ($trid === false) {
                throw new ImageException('Unable to allocate alpha color for transparency');
            }
            \imagecolortransparent($newimg, $trid);
        } else {
            $tid = \imagecolortransparent($img);
            $palsize = \imagecolorstotal($img);
            if ($tid >= 0 && $palsize > 0 && $tid < $palsize) {
                // set transparency for Indexed image
                /** @var array{'red': int, 'green': int, 'blue': int, 'alpha': int} $tcol */
                $tcol = \imagecolorsforindex($img, $tid);
                $trid = \imagecolorallocate($newimg, $tcol['red'], $tcol['green'], $tcol['blue']);
                if ($trid === false) {
                    throw new ImageException('Unable to allocate color for transparency');
                }
                \imagefill($newimg, 0, 0, $trid);
                \imagecolortransparent($newimg, $trid);
            }
        }

        \imagealphablending($newimg, !$alpha);
        \imagesavealpha($newimg, $alpha);

        \imagecopyresampled($newimg, $img, 0, 0, 0, 0, $width, $height, $data['width'], $data['height']);

        \ob_start();
        if ($data['mapto'] === IMAGETYPE_PNG) {
            \imagepng($newimg, null, 9, PNG_ALL_FILTERS);
        } else {
            \imagejpeg($newimg, null, $quality);
        }
        $ogc = \ob_get_clean();
        if ($ogc === false) {
            throw new ImageException('Unable to extract alpha channel');
        }

        $data['raw'] = $ogc;
        $data['exturl'] = false;
        $data['recoded'] = true;
        return $this->getMetaData($data);
    }

    /**
     * Extract the alpha channel as separate image to be used as a mask.
     *
     * @param ImageBaseData $data Image raw data as returned by getImageRawData.
     *
     * @return ImageRawData Image raw data array.
     *
     * @throws \Com\Tecnick\Pdf\Image\Exception If the alpha channel cannot be extracted.
     */
    protected function getAlphaChannelRawData(array $data): array
    {
        $img = \imagecreatefromstring($data['raw']);
        if ($img === false) {
            throw new ImageException('Unable to create alpha channel image from string');
        }

        $newimg = \imagecreate(\max(1, $data['width']), \max(1, $data['height']));
        if ($newimg === false) {
            throw new ImageException('Unable to create new empty alpha channel image');
        }

        \imageinterlace($newimg, false);

        // generate gray scale palette (0 -> 255)
        for ($col = 0; $col < 256; ++$col) {
            \imagecolorallocate($newimg, $col, $col, $col);
        }

        // Precompute the gamma-corrected output for each of the 128 possible
        // GD alpha values (7-bit: 0 -> 127), so the expensive pow() runs 128
        // times instead of once per pixel.
        $alphamap = [];
        for ($alf = 0; $alf < 128; ++$alf) {
            // GD alpha is only 7 bit (0 -> 127); 2.2 is the gamma value
            $alphamap[$alf] = (int) ((((float) (127 - $alf) / 127) ** 2.2) * 255);
        }

        $truecolor = \imageistruecolor($img);

        // extract alpha channel
        for ($xpx = 0; $xpx < $data['width']; ++$xpx) {
            for ($ypx = 0; $ypx < $data['height']; ++$ypx) {
                $colindex = \imagecolorat($img, $xpx, $ypx);
                if ($colindex === false) {
                    throw new ImageException('Unable to extract alpha channel color index');
                }

                // get and correct gamma color (keys 0-127 are always present)
                if ($truecolor) {
                    // For truecolor images the alpha is packed in bits 24-30 of
                    // the pixel value, so it can be read directly instead of
                    // allocating a colour-component array per pixel.
                    $alpha = $alphamap[($colindex >> 24) & 0x7F] ?? 0;
                } else {
                    /** @var array{'red': int, 'green': int, 'blue': int, 'alpha': int} $color */
                    $color = \imagecolorsforindex($img, $colindex);
                    $alpha = $alphamap[$color['alpha']] ?? 0;
                }

                \imagesetpixel($newimg, $xpx, $ypx, $alpha);
            }
        }

        \ob_start();
        \imagepng($newimg, null, 9, PNG_ALL_FILTERS);
        $ogc = \ob_get_clean();
        if ($ogc === false) {
            throw new ImageException('Unable to extract alpha channel');
        }

        $data['raw'] = $ogc;
        $data['channels'] = 1;
        $data['colspace'] = 'DeviceGray';
        $data['exturl'] = false;
        $data['recoded'] = true;
        $data['ismask'] = true;
        return $this->getMetaData($data);
    }
}
