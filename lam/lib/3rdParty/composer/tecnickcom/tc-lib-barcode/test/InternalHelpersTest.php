<?php

/**
 * InternalHelpersTest.php
 *
 * @since       2026-04-19
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Test;

use PHPUnit\Framework\Attributes\DataProvider;
use Test\Fixture\InternalBarcodeType;
use Test\Fixture\InternalQrByteStream;

class InternalHelpersTest extends TestUtil
{
    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testBaseTypeDefaultHooksAreCovered(): void
    {
        $type = new InternalBarcodeType(true);
        $data = $type->getArray();

        $this->assertSame([], $data['params']);
        $this->assertSame([], $data['bars']);
        $this->assertSame(4, $data['width']);
        $this->assertSame(3, $data['height']);
        $this->assertSame(['T' => 6, 'R' => 2, 'B' => 0, 'L' => 1], $data['padding']);

        $type->setBackgroundColor('');
        $this->assertNull($type->getArray()['bg_color_obj']);
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testQrInputItemPaddingAndAppendBranches(): void
    {
        $helper = new InternalQrByteStream(\Com\Tecnick\Barcode\Type\Square\QrCode\Data::MODE_8B, 1, 0);

        $item = $helper->exposeNewInputItem(\Com\Tecnick\Barcode\Type\Square\QrCode\Data::MODE_NM, 3, ['1']);

        $this->assertSame(['1', '0', '0'], $item['data']);
        $this->assertSame(-1, $helper->exposeLookAnTable(128));

        $items = $helper->exposeAppendNewInputItem(
            [],
            \Com\Tecnick\Barcode\Type\Square\QrCode\Data::MODE_AN,
            2,
            ['A', 'Z'],
        );

        $this->assertCount(1, $items);
        $firstItem = $items[0] ?? null;
        $this->assertNotNull($firstItem);
        $this->assertSame(['A', 'Z'], $firstItem['data']);
    }

    /**
     * @param array<int, string> $data
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    #[DataProvider('getInvalidQrInputItemProvider')]
    public function testQrInputItemInvalidBranches(int $mode, int $size, array $data): void
    {
        $this->bcExpectException(\Com\Tecnick\Barcode\Exception::class);

        $helper = new InternalQrByteStream(\Com\Tecnick\Barcode\Type\Square\QrCode\Data::MODE_8B, 1, 0);

        $helper->exposeNewInputItem($mode, $size, $data);
    }

    /**
     * @return array<array{int, int, array<int, string>}>
     */
    public static function getInvalidQrInputItemProvider(): array
    {
        return [
            [\Com\Tecnick\Barcode\Type\Square\QrCode\Data::MODE_AN, 1, ["\x80"]],
            [\Com\Tecnick\Barcode\Type\Square\QrCode\Data::MODE_KJ, 1, ["\x81"]],
            [999, 1, ['A']],
            [\Com\Tecnick\Barcode\Type\Square\QrCode\Data::MODE_NM, 0, ['1']],
        ];
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testQrByteStreamPaddingAndBitstreamBranches(): void
    {
        $helper = new InternalQrByteStream(\Com\Tecnick\Barcode\Type\Square\QrCode\Data::MODE_8B, 1, 0);
        $spec = new \Com\Tecnick\Barcode\Type\Square\QrCode\Spec();
        $maxWords = $spec->getDataLength(1, 0);
        $maxBits = $maxWords * 8;

        $this->assertSame([], $helper->exposeAppendPaddingBit([]));

        $exact = \array_fill(0, \max(0, $maxBits), 0);
        $this->assertSame($exact, $helper->exposeAppendPaddingBit($exact));

        $shortBits = \max(0, $maxBits - 2);
        $short = \array_fill(0, $shortBits, 1);
        $this->assertCount($maxBits, $helper->exposeAppendPaddingBit($short));

        $padded = $helper->exposeAppendPaddingBit([1, 0, 1, 0]);
        $this->assertCount($maxWords, $helper->exposeBitstreamToByte($padded));

        $this->assertSame([], $helper->exposeBitstreamToByte([]));
        $this->assertSame([21], $helper->exposeBitstreamToByte([1, 0, 1, 0, 1]));
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testQrByteStreamCreateAndConvertBranches(): void
    {
        $helper = new InternalQrByteStream(\Com\Tecnick\Barcode\Type\Square\QrCode\Data::MODE_8B, 1, 0);

        $item = $helper->exposeNewInputItem(\Com\Tecnick\Barcode\Type\Square\QrCode\Data::MODE_NM, 3, ['1', '2', '3']);
        $stream = $helper->exposeCreateBitStream([$item]);
        $this->assertGreaterThan(0, $stream[1]);

        $helper->forcedEstVer = 2;
        $helper->forcedMinVer = 2;
        $encoded = $helper->encodeBitStream($item, 2);
        $helper->queuedCbs = [
            [[$encoded], \count($encoded['bstream'])],
        ];

        $converted = $helper->exposeConvertData([$item]);
        $firstConverted = $converted[0] ?? null;
        $this->assertNotNull($firstConverted);
        $this->assertSame($encoded['bstream'], $firstConverted['bstream']);
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testQrByteStreamNegativeBitsThrowsException(): void
    {
        $this->bcExpectException(\Com\Tecnick\Barcode\Exception::class);

        $helper = new InternalQrByteStream(\Com\Tecnick\Barcode\Type\Square\QrCode\Data::MODE_8B, 1, 0);
        $helper->forcedEstVer = 1;
        $helper->queuedCbs = [
            [[], -1],
        ];

        $helper->exposeConvertData([]);
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testQrByteStreamEncodeBranches(): void
    {
        $helper = new InternalQrByteStream(\Com\Tecnick\Barcode\Type\Square\QrCode\Data::MODE_8B, 1, 0);

        $spec = new \Com\Tecnick\Barcode\Type\Square\QrCode\Spec();
        $words = $spec->maximumWords(\Com\Tecnick\Barcode\Type\Square\QrCode\Data::MODE_8B, 1);
        $item = $helper->exposeNewInputItem(
            \Com\Tecnick\Barcode\Type\Square\QrCode\Data::MODE_8B,
            $words + 1,
            \array_fill(0, \max(0, $words + 1), 'A'),
        );

        $encoded = $helper->encodeBitStream($item, 1);
        $this->assertNotEmpty($encoded['bstream']);
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testQrByteStreamEncodeInvalidModeThrowsException(): void
    {
        $this->bcExpectException(\Com\Tecnick\Barcode\Exception::class);

        $helper = new InternalQrByteStream(\Com\Tecnick\Barcode\Type\Square\QrCode\Data::MODE_8B, 1, 0);

        $helper->encodeBitStream([
            'mode' => 999,
            'size' => 1,
            'data' => ['A'],
            'bstream' => [],
        ], 1);
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testDatamatrixPaddingSizeHelper(): void
    {
        $params = \Com\Tecnick\Barcode\Type\Square\Datamatrix\Data::getPaddingSize('S', 1);
        $this->assertCount(16, $params);

        $this->bcExpectException(\Com\Tecnick\Barcode\Exception::class);
        \Com\Tecnick\Barcode\Type\Square\Datamatrix\Data::getPaddingSize('S', PHP_INT_MAX);
    }
}
