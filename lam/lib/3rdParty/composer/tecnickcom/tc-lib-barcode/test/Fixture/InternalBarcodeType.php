<?php

/**
 * InternalBarcodeType.php
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

namespace Test\Fixture;

class InternalBarcodeType extends \Com\Tecnick\Barcode\Type
{
    protected const TYPE = 'linear';

    protected const FORMAT = 'INTERNAL';

    /**
     * @var array<int, array{int, int, int, int}>|null
     */
    private ?array $rotatedBarsOverride = null;

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function __construct(
        private bool $useParentHooks = false,
    ) {
        $this->ncols = 2;
        $this->nrows = 1;

        parent::__construct('12', -2, -3, 'black', [], [-2, -1, 0, 1]);
    }

    protected function setParameters(): void
    {
        if ($this->useParentHooks) {
            parent::setParameters();
            return;
        }

        $this->params = ['ok'];
    }

    protected function setBars(): void
    {
        if ($this->useParentHooks) {
            parent::setBars();
            return;
        }

        $this->bars = [[0, 0, 1, 1]];
    }

    /**
     * @param array<int, array{int, int, int, int}> $bars
     */
    public function setBarsForTest(array $bars): void
    {
        $this->bars = $bars;
    }

    public function setRowsColsForTest(int $nrows, int $ncols): void
    {
        $this->nrows = $nrows;
        $this->ncols = $ncols;
    }

    public function setRatiosForTest(float $widthRatio, float $heightRatio): void
    {
        $this->width_ratio = $widthRatio;
        $this->height_ratio = $heightRatio;
    }

    /**
     * @param array<int, array{int, int, int, int}>|null $bars
     */
    public function setRotatedBarsOverrideForTest(?array $bars): void
    {
        $this->rotatedBarsOverride = $bars;
    }

    /**
     * @return array<int, array{int, int, int, int}>
     */
    protected function getRotatedBarArray(): array
    {
        if ($this->rotatedBarsOverride !== null) {
            return $this->rotatedBarsOverride;
        }

        return parent::getRotatedBarArray();
    }
}
