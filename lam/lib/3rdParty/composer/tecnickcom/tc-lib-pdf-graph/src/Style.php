<?php

declare(strict_types=1);

/**
 * Style.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfGraph
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-graph
 *
 * This file is part of tc-lib-pdf-graph software library.
 */

namespace Com\Tecnick\Pdf\Graph;

use Com\Tecnick\Pdf\Graph\Exception as GraphException;

/**
 * Com\Tecnick\Pdf\Graph\Style
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfGraph
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-graph
 *
 * @phpstan-import-type StyleDataOpt from \Com\Tecnick\Pdf\Graph\Base
 *
 * @SuppressWarnings("PHPMD.ExcessiveClassComplexity")
 */
abstract class Style extends \Com\Tecnick\Pdf\Graph\Base
{
    /**
     * Array of restore points (style ID).
     *
     * @var array<int>
     */
    protected array $stylemark = [0];

    /**
     * Map values for lineCap.
     *
     * @var array<int|string, int>
     */
    protected const LINECAPMAP = [
        0 => 0,
        1 => 1,
        2 => 2,
        'butt' => 0,
        'round' => 1,
        'square' => 2,
    ];

    /**
     * Map values for lineJoin.
     *
     * @var array<int|string, int>
     */
    protected const LINEJOINMAP = [
        0 => 0,
        1 => 1,
        2 => 2,
        'miter' => 0,
        'round' => 1,
        'bevel' => 2,
    ];

    /**
     * Map path paint operators.
     *
     * @var array<string, string>
     */
    protected const PPOPMAP = [
        'S' => 'S',
        'D' => 'S',
        's' => 's',
        'h S' => 's',
        'd' => 's',
        'f' => 'f',
        'F' => 'f',
        'h f' => 'h f',
        'f*' => 'f*',
        'F*' => 'f*',
        'h f*' => 'h f*',
        'B' => 'B',
        'FD' => 'B',
        'DF' => 'B',
        'B*' => 'B*',
        'F*D' => 'B*',
        'DF*' => 'B*',
        'b' => 'b',
        'h B' => 'b',
        'fd' => 'b',
        'df' => 'b',
        'b*' => 'b*',
        'h B*' => 'b*',
        'f*d' => 'b*',
        'df*' => 'b*',
        'W n' => 'W n',
        'CNZ' => 'W n',
        'W* n' => 'W* n',
        'CEO' => 'W* n',
        'h' => 'h',
        'n' => 'n',
    ];

    /**
     * Filling modes.
     *
     * @var array<string, bool>
     */
    protected const MODEFILLING = [
        'f' => true,
        'f*' => true,
        'B' => true,
        'B*' => true,
        'b' => true,
        'b*' => true,
    ];

    /**
     * Stroking Modes.
     *
     * @var array<string, bool>
     */
    protected const MODESTROKING = [
        'S' => true,
        's' => true,
        'B' => true,
        'B*' => true,
        'b' => true,
        'b*' => true,
    ];

    /**
     * Closing Modes.
     *
     * @var array<string, bool>
     */
    protected const MODECLOSING = [
        'b' => true,
        'b*' => true,
        's' => true,
    ];

    /**
     * Clipping Modes.
     *
     * @var array<string, bool>
     */
    protected const MODECLIPPING = [
        'CEO' => true,
        'CNZ' => true,
        'W n' => true,
        'W* n' => true,
    ];

    /**
     * Map of equivalent modes without close.
     *
     * @var array<string, string>
     */
    protected const MODETONOCLOSE = [
        's' => 'S',
        'b' => 'B',
        'b*' => 'B*',
    ];

    /**
     * Map of equivalent modes without fill.
     *
     * @var array<string, string>
     */
    protected const MODETONOFILL = [
        'f' => '',
        'f*' => '',
        'B' => 'S',
        'B*' => 'S',
        'b' => 's',
        'b*' => 's',
    ];

    /**
     * Map of equivalent modes without STROKE.
     *
     * @var array<string, string>
     */
    protected const MODETONOSTROKE = [
        'S' => '',
        's' => 'h',
        'B' => 'f',
        'B*' => 'f*',
        'b' => 'h f',
        'b*' => 'h f*',
    ];

    /**
     * Add a new style
     *
     * @param StyleDataOpt $style       Style to add.
     * @param bool         $inheritlast If true inherit missing values from the last style.
     *
     * @return string PDF style string
     */
    public function add(array $style = [], bool $inheritlast = false): string
    {
        if ($inheritlast) {
            $style = \array_merge($this->getCurrentStyleArray(), $style);
        }

        $this->style[++$this->styleid] = $style;
        return $this->getStyle();
    }

    /**
     * Remove and return last style.
     *
     * @return string PDF style string.
     *
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function pop(): string
    {
        if ($this->styleid <= 0) {
            throw new GraphException('The style stack is empty');
        }

        $style = $this->getStyle();
        unset($this->style[$this->styleid]);
        --$this->styleid;
        return $style;
    }

    /**
     * Save the current style ID to be restored later.
     */
    public function saveStyleStatus(): void
    {
        $this->stylemark[] = $this->styleid;
    }

    /**
     * Restore the saved style status.
     */
    public function restoreStyleStatus(): void
    {
        $styleid = \array_pop($this->stylemark);
        if ($styleid === null) {
            $styleid = 0;
        }

        $this->styleid = $styleid;

        $this->style = \array_slice($this->style, 0, $this->styleid + 1, true);
    }

    /**
     * Returns the last style array.
     *
     * @return StyleDataOpt
     */
    public function getCurrentStyleArray(): array
    {
        if (isset($this->style[$this->styleid])) {
            return $this->style[$this->styleid];
        }

        return $this->getDefaultStyle();
    }

    /**
     * Returns the last set value of the specified property.
     *
     * @param string $property Property to search.
     * @param int|float|bool|string|null  $default  Default value to return in case the property is not found.
     *
     * @return int|float|bool|string|null Property value or $default in case the property is not found.
     */
    public function getLastStyleProperty(
        string $property,
        int|float|bool|string|null $default = null,
    ): int|float|bool|string|null {
        for ($idx = $this->styleid; $idx >= 0; --$idx) {
            if (
                array_key_exists($property, $this->style[$idx] ?? [])
                && ($this->style[$idx][$property] ?? null) !== null
                && !\is_array($this->style[$idx][$property])
            ) {
                return $this->style[$idx][$property];
            }
        }

        return $default;
    }

    /**
     * Returns the value of the specified item from the last inserted style.
     *
     * @param string $item Item to search.
     *
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function getCurrentStyleItem(string $item): mixed
    {
        $style = $this->getCurrentStyleArray();
        if (!\array_key_exists($item, $style)) {
            throw new GraphException('The ' . $item . ' value is not set in the current style');
        }

        return $style[$item];
    }

    /**
     * Returns the PDF string of the last style added.
     */
    public function getStyle(): string
    {
        return $this->getStyleCmd($this->getCurrentStyleArray());
    }

    /**
     * Returns the PDF string of the specified style.
     *
     * @param StyleDataOpt $style Style to represent.
     */
    public function getStyleCmd(array $style = []): string
    {
        $out = '';
        if (array_key_exists('lineWidth', $style)) {
            $out .= \sprintf('%F w' . "\n", $style['lineWidth'] * $this->kunit);
        }

        $out .= $this->getLineModeCmd($style);

        if (array_key_exists('lineColor', $style)) {
            $out .= $this->pdfColor->getPdfColor($style['lineColor'], true);
        }

        if (array_key_exists('fillColor', $style)) {
            $out .= $this->pdfColor->getPdfColor($style['fillColor'], false);
        }

        return $out;
    }

    /**
     * Returns the PDF string of the specified line style.
     *
     * @param StyleDataOpt $style Style to represent.
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     */
    protected function getLineModeCmd(array $style = []): string
    {
        $out = '';

        if (array_key_exists('lineCap', $style)) {
            $lineCap = $style['lineCap'];
            if (isset(self::LINECAPMAP[$lineCap])) {
                $out .= self::LINECAPMAP[$lineCap] . ' J' . "\n";
            }
        }

        if (array_key_exists('lineJoin', $style)) {
            $lineJoin = $style['lineJoin'];
            if (isset(self::LINEJOINMAP[$lineJoin])) {
                $out .= self::LINEJOINMAP[$lineJoin] . ' j' . "\n";
            }
        }

        if (array_key_exists('miterLimit', $style)) {
            $out .= \sprintf('%F M' . "\n", $style['miterLimit'] * $this->kunit);
        }

        if (array_key_exists('dashArray', $style)) {
            $dash = [];
            foreach ($style['dashArray'] as $val) {
                $dash[] = \sprintf('%F', (float) $val * $this->kunit);
            }

            if (!array_key_exists('dashPhase', $style)) {
                $style['dashPhase'] = 0;
            }

            $out .= \sprintf('[%s] %F d' . "\n", \implode(' ', $dash), $style['dashPhase']);
        }

        return $out;
    }

    /**
     * Get the Path-Painting Operators.
     *
     * @param string|PathPaintOp $mode Mode of rendering (or PathPaintOp enum case). Possible values are:
     *                        - S or D: Stroke the path. - s or d:
     *                        Close and stroke the path. - f or F:
     *                        Fill the path, using the nonzero
     *                        winding number rule to determine the
     *                        region to fill. - f* or F*: Fill the
     *                        path, using the even-odd rule to
     *                        determine the region to fill. - B or FD
     *                        or DF: Fill and then stroke the path,
     *                        using the nonzero winding number rule
     *                        to determine the region to fill. - B*
     *                        or F*D or DF*: Fill and then stroke the
     *                        path, using the even-odd rule to
     *                        determine the region to fill. - b or fd
     *                        or df: Close, fill, and then stroke the
     *                        path, using the nonzero winding number
     *                        rule to determine the region to fill. -
     *                        b* or f*d or df*: Close, fill, and then
     *                        stroke the path, using the even-odd
     *                        rule to determine the region to fill. -
     *                        CNZ: Clipping mode using the nonzero
     *                        winding number rule to determine which
     *                        regions lie inside the clipping path. -
     *                        CEO: Clipping mode using the even-odd
     *                        rule to determine which regions lie
     *                        inside the clipping path - n: End
     *                        the path object without filling or
     *                        stroking it.
     * @param string|PathPaintOp $default Default style (or PathPaintOp enum case)
     */
    public function getPathPaintOp(string|PathPaintOp $mode, string|PathPaintOp $default = 'S'): string
    {
        if ($mode instanceof PathPaintOp) {
            $mode = $mode->value;
        }

        if ($default instanceof PathPaintOp) {
            $default = $default->value;
        }

        if ($mode === '' || !isset(self::PPOPMAP[$mode])) {
            return isset(self::PPOPMAP[$default]) ? self::PPOPMAP[$default] . "\n" : '';
        }
        return self::PPOPMAP[$mode] . "\n";
    }

    /**
     * Returns true if the specified path paint operator includes the filling option.
     *
     * @param string $mode Path paint operator (mode of rendering).
     */
    public function isFillingMode(string $mode): bool
    {
        return (
            isset(self::PPOPMAP[$mode])
            && (array_key_exists(self::PPOPMAP[$mode], self::MODEFILLING) || $this->isClippingMode($mode))
        );
    }

    /**
     * Returns true if the specified mode includes the stroking option.
     *
     * @param string $mode Path paint operator (mode of rendering).
     */
    public function isStrokingMode(string $mode): bool
    {
        if (!isset(self::PPOPMAP[$mode])) {
            return false;
        }

        $paintMode = self::PPOPMAP[$mode];
        return array_key_exists($paintMode, self::MODESTROKING);
    }

    /**
     * Returns true if the specified mode includes "closing the path" option.
     *
     * @param string $mode Path paint operator (mode of rendering).
     */
    public function isClosingMode(string $mode): bool
    {
        return (
            isset(self::PPOPMAP[$mode])
            && (array_key_exists(self::PPOPMAP[$mode], self::MODECLOSING) || $this->isClippingMode($mode))
        );
    }

    /**
     * Returns true if the specified mode is of clipping type.
     *
     * @param string $mode Path paint operator (mode of rendering).
     */
    public function isClippingMode(string $mode): bool
    {
        if (!isset(self::PPOPMAP[$mode])) {
            return false;
        }

        $paintMode = self::PPOPMAP[$mode];
        return array_key_exists($paintMode, self::MODECLIPPING);
    }

    /**
     * Remove the Close option from the specified Path paint operator.
     *
     * @param string $mode Path paint operator (mode of rendering).
     */
    public function getModeWithoutClose(string $mode): string
    {
        if (!isset(self::PPOPMAP[$mode])) {
            return $mode;
        }

        $paintMode = self::PPOPMAP[$mode];
        if (isset(self::MODETONOCLOSE[$paintMode])) {
            return self::MODETONOCLOSE[$paintMode];
        }

        return $mode;
    }

    /**
     * Remove the Fill option from the specified Path paint operator.
     *
     * @param string $mode Path paint operator (mode of rendering).
     */
    public function getModeWithoutFill(string $mode): string
    {
        if (!isset(self::PPOPMAP[$mode])) {
            return $mode;
        }

        $paintMode = self::PPOPMAP[$mode];
        if (isset(self::MODETONOFILL[$paintMode])) {
            return self::MODETONOFILL[$paintMode];
        }

        return $mode;
    }

    /**
     * Remove the Stroke option from the specified Path paint operator.
     *
     * @param string $mode Path paint operator (mode of rendering).
     */
    public function getModeWithoutStroke(string $mode): string
    {
        if (!isset(self::PPOPMAP[$mode])) {
            return $mode;
        }

        $paintMode = self::PPOPMAP[$mode];
        if (isset(self::MODETONOSTROKE[$paintMode])) {
            return self::MODETONOSTROKE[$paintMode];
        }

        return $mode;
    }

    /**
     * Add transparency parameters to the current extgstate.
     *
     * @param array<string, int|float|bool|string> $parms parameters.
     *
     * @return string PDF command.
     */
    public function getExtGState(array $parms): string
    {
        if ($this->pdfa) {
            return '';
        }

        $gsx = \count($this->extgstates) + 1;
        // check if this ExtGState already exist
        foreach ($this->extgstates as $idx => $ext) {
            if ($ext['parms'] !== $parms) {
                continue;
            }

            $gsx = $idx;
            break;
        }

        if (($this->extgstates[$gsx] ?? []) === []) {
            $this->extgstates[$gsx] = [
                'n' => 0,
                'name' => '',
                'parms' => $parms,
            ];
        }

        return '/GS' . $gsx . ' gs' . "\n";
    }
}
