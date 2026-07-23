<?php

declare(strict_types=1);

/**
 * ImageCacheInterface.php
 *
 * @since     2026-06-16
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

/**
 * Com\Tecnick\Pdf\Image\ImageCacheInterface
 *
 * Optional external cache used to persist processed image data across
 * Import instances and PHP processes, avoiding recomputation of images that
 * have already been imported, resized and encoded.
 *
 * Implementations are a thin bridge to any backend (filesystem, APCu, Redis,
 * a PSR-16 cache, ...). They only need to store and retrieve plain arrays;
 * (de)serialization and eviction are the backend's responsibility.
 *
 * The cached value is the import-time snapshot of an image: it never contains
 * PDF object numbers (those are assigned per document at output time). Both
 * methods MUST be best-effort and MUST NOT throw on a backend miss or
 * transient backend failure, so that a cache problem never breaks PDF
 * generation.
 *
 * Security: the cache store is a trust boundary. The stored arrays (image
 * data, palette and ICC bytes) are embedded verbatim into generated PDFs, so
 * anyone able to write to the backend can influence document output. Use a
 * store only your application can write to, and when an implementation
 * deserializes data it MUST disable object restoration
 * (e.g. unserialize($s, ['allowed_classes' => false])).
 *
 * @since     2026-06-16
 * @category  Library
 * @package   PdfImage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-image
 *
 * @phpstan-import-type ImageRawData from \Com\Tecnick\Pdf\Image\Import
 */
interface ImageCacheInterface
{
    /**
     * Retrieve a previously stored image data array.
     *
     * @param string $key Image cache key.
     *
     * @return ImageRawData|null Stored image data array, or null on a miss.
     */
    public function get(string $key): ?array;

    /**
     * Store an image data array.
     *
     * @param string       $key  Image cache key.
     * @param ImageRawData $data Image data array to store.
     */
    public function set(string $key, array $data): void;
}
