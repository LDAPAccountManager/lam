<?php

/**
 * BoxTest.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfPage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-page
 *
 * This file is part of tc-lib-pdf-page software library.
 */

namespace Test;

/**
 * Box Test
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfPage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-page
 */
class BoxTest extends TestUtil
{
    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    protected function getTestObject(): \Com\Tecnick\Pdf\Page\Page
    {
        $pdf = new \Com\Tecnick\Color\Pdf();
        $encrypt = $this->getEncryptObject();
        return new \Com\Tecnick\Pdf\Page\Page('mm', $pdf, $encrypt, false, false);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testSetBox(): void
    {
        $page = $this->getTestObject();
        $dims = $page->setBox([], 'CropBox', 2, 4, 6, 8);
        $this->bcAssertEqualsWithDelta([
            'CropBox' => [
                'llx' => 2,
                'lly' => 4,
                'urx' => 6,
                'ury' => 8,
                'bci' => [
                    'color' => '#000000',
                    'width' => 0.353,
                    'style' => 'S',
                    'dash' => [3],
                ],
            ],
        ], $dims);

        $dims = $page->setBox([], 'TrimBox', 3, 5, 7, 11, [
            'color' => 'aquamarine',
            'width' => 2,
            'style' => 'D',
            'dash' => [2, 3, 5, 7],
        ]);
        $this->bcAssertEqualsWithDelta([
            'TrimBox' => [
                'llx' => 3,
                'lly' => 5,
                'urx' => 7,
                'ury' => 11,
                'bci' => [
                    'color' => 'aquamarine',
                    'width' => 2,
                    'style' => 'D',
                    'dash' => [2, 3, 5, 7],
                ],
            ],
        ], $dims);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testSetBoxEx(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Page\Exception::class);
        $page = $this->getTestObject();
        $page->setBox([], 'ERROR', 1, 2, 3, 4);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testSwapCoordinates(): void
    {
        $page = $this->getTestObject();
        $dims = [
            'CropBox' => [
                'llx' => 2,
                'lly' => 4,
                'urx' => 6,
                'ury' => 8,
            ],
        ];
        $newpagedim = $page->swapCoordinates($dims);
        $this->assertEquals(
            [
                'CropBox' => [
                    'llx' => 4,
                    'lly' => 2,
                    'urx' => 8,
                    'ury' => 6,
                ],
            ],
            $newpagedim,
        );
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testSetPageBoxes(): void
    {
        $page = $this->getTestObject();
        $dims = $page->setPageBoxes(100, 200);
        $exp = [
            'llx' => 0,
            'lly' => 0,
            'urx' => 100,
            'ury' => 200,
            'bci' => [
                'color' => '#000000',
                'width' => 0.353,
                'style' => 'S',
                'dash' => [3],
            ],
        ];
        $this->bcAssertEqualsWithDelta([
            'MediaBox' => $exp,
            'CropBox' => $exp,
            'BleedBox' => $exp,
            'TrimBox' => $exp,
            'ArtBox' => $exp,
        ], $dims);
    }
}
