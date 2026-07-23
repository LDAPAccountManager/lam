<?php

/**
 * SpyImageCache.php
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

namespace Test;

use Com\Tecnick\Pdf\Image\ImageCacheInterface;

/**
 * In-memory ImageCacheInterface implementation that records access for tests.
 *
 * @phpstan-import-type ImageRawData from \Com\Tecnick\Pdf\Image\Import
 */
class SpyImageCache implements ImageCacheInterface
{
    /**
     * Backing store.
     *
     * @var array<string, ImageRawData>
     */
    public array $store = [];

    /**
     * Number of get() calls.
     */
    public int $getCount = 0;

    /**
     * Number of set() calls.
     */
    public int $setCount = 0;

    /**
     * Keys passed to set(), in order.
     *
     * @var list<string>
     */
    public array $setKeys = [];

    /**
     * @return ImageRawData|null
     */
    public function get(string $key): ?array
    {
        ++$this->getCount;
        return $this->store[$key] ?? null;
    }

    /**
     * Fetch a stored entry by key, failing loudly when it is absent.
     *
     * @return ImageRawData
     */
    public function fetch(string $key): array
    {
        $data = $this->store[$key] ?? null;
        if ($data === null) {
            throw new \LogicException('No cached entry for key: ' . $key);
        }

        return $data;
    }

    /**
     * @param ImageRawData $data
     */
    public function set(string $key, array $data): void
    {
        ++$this->setCount;
        $this->setKeys[] = $key;
        $this->store[$key] = $data;
    }
}
