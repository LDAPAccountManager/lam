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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-image
 *
 * This file is part of tc-lib-pdf-image software library.
 */

namespace Com\Tecnick\Pdf\Image;

/**
 * Com\Tecnick\Pdf\Image\ImageCacheInterface
 *
 * External cache used to persist processed image data across Import instances
 * and PHP processes.
 *
 * Implementations bridge to any backend (filesystem, APCu, Redis, a PSR-16
 * cache, ...). They store and retrieve plain arrays; (de)serialization and
 * eviction are the backend's responsibility.
 *
 * The cached value is the import-time snapshot of an image and never contains
 * PDF object numbers, which are assigned per document at output time. Both
 * methods MUST be best-effort and MUST NOT throw on a miss or on a backend
 * failure.
 *
 * Security: the cache store is a trust boundary. The stored arrays (image
 * data, palette and ICC bytes) are embedded verbatim into generated PDFs. Use
 * a store only the application can write to, and when an implementation
 * deserializes data it MUST disable object restoration
 * (e.g. unserialize($s, ['allowed_classes' => false])).
 *
 * @since     2026-06-16
 * @category  Library
 * @package   PdfImage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
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
