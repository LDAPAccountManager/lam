<?php

/**
 * ImageCacheTest.php
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
use Com\Tecnick\Pdf\Image\Import;

/**
 * External image cache test.
 *
 * @since     2026-06-16
 * @category  Library
 * @package   PdfImage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-image
 */
class ImageCacheTest extends TestUtil
{
    private function getImport(?ImageCacheInterface $cache): Import
    {
        return new Import(0.75, $this->getTestEncrypt(), $this->getTestFileHelper(), imageCache: $cache);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testWriteThroughAndReadThrough(): void
    {
        $src = __DIR__ . '/images/200x100_RGB.png';

        $spy = new SpyImageCache();
        $img1 = $this->getImport($spy);
        $img1->add($src);

        // one external lookup (miss) + one write-through
        $this->assertSame(1, $spy->getCount);
        $this->assertCount(1, $spy->setKeys);

        // doctor the stored entry with a sentinel: a hit must be returned verbatim
        $key = $spy->setKeys[0] ?? '';
        $this->assertArrayHasKey($key, $spy->store);
        if (isset($spy->store[$key])) {
            $spy->store[$key]['width'] = 4321;
        }

        // a fresh Import (empty in-process cache) reusing the same backend
        $img2 = $this->getImport($spy);
        $img2->add($src);
        $data = $img2->getImageDataByKey($img2->getKey($src));

        // proves the external entry was used as-is (no recomputation)
        $this->assertSame(4321, $data['width']);
        // no additional write-through happened on a hit
        $this->assertCount(1, $spy->setKeys);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testDisabledByDefault(): void
    {
        $src = __DIR__ . '/images/200x100_RGB.png';

        $img = $this->getImport(null);
        $img->add($src);
        $data = $img->getImageDataByKey($img->getKey($src));

        // with no external cache, the in-process data keeps the original bytes
        $this->assertNotEmpty($data['data']);
        $this->assertNotEmpty($data['raw']);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testPersistedSnapshotIsClean(): void
    {
        $spy = new SpyImageCache();
        $img = $this->getImport($spy);
        $img->add(__DIR__ . '/images/200x100_RGBALPHA.png');

        $stored = $spy->fetch($spy->setKeys[0] ?? '');

        // original bytes dropped to shrink the payload
        $this->assertSame('', $stored['raw']);
        // no PDF object numbers / output flag are persisted
        $this->assertSame(0, $stored['obj']);
        $this->assertSame(0, $stored['obj_alt']);
        $this->assertSame(0, $stored['obj_icc']);
        $this->assertSame(0, $stored['obj_pal']);
        $this->assertArrayNotHasKey('out', $stored);

        // alpha image splits into plain + mask: their raw bytes are dropped too
        $this->assertArrayHasKey('mask', $stored);
        if (isset($stored['mask'])) {
            $this->assertSame('', $stored['mask']['raw']);
        }
        if (isset($stored['plain'])) {
            $this->assertSame('', $stored['plain']['raw']);
        }
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testStaleFileProducesDifferentKey(): void
    {
        $tmp = $this->makeTempFile('.png');
        \copy(__DIR__ . '/images/200x100_RGB.png', $tmp);
        \clearstatcache(true, $tmp);

        $spy = new SpyImageCache();
        $this->getImport($spy)->add($tmp);
        $this->assertCount(1, $spy->setKeys);
        $firstKey = $spy->setKeys[0] ?? '';

        // edit the file in place: different content/size and a newer mtime
        \copy(__DIR__ . '/images/200x100_GRAY.png', $tmp);
        \touch($tmp, \time() + 5);
        \clearstatcache(true, $tmp);

        $this->getImport($spy)->add($tmp);
        $this->assertCount(2, $spy->setKeys);

        // mtime+size folded into the persistent key invalidates the stale entry
        $this->assertNotSame($firstKey, $spy->setKeys[1] ?? '');

        \unlink($tmp);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testOutputIdenticalWithAndWithoutCache(): void
    {
        $src = __DIR__ . '/images/200x100_RGB.png';

        $ref = $this->getImport(null);
        $ref->add($src);
        $refOut = $ref->getOutImagesBlock(10);

        $spy = new SpyImageCache();
        // warm the cache, then import again from a fresh instance (external hit)
        $this->getImport($spy)->add($src);
        $cached = $this->getImport($spy);
        $cached->add($src);
        $cachedOut = $cached->getOutImagesBlock(10);

        // stripping raw never affects the emitted PDF objects
        $this->assertSame($refOut, $cachedOut);
    }

    private function makeTempFile(string $ext): string
    {
        return \sys_get_temp_dir() . \DIRECTORY_SEPARATOR . 'tclpi_' . \uniqid('', true) . $ext;
    }
}
