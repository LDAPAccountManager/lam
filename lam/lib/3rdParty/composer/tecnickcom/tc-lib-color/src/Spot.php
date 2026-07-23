<?php

declare(strict_types=1);

/**
 * Spot.php
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-color
 *
 * This file is part of tc-lib-color software library.
 */

namespace Com\Tecnick\Color;

use Com\Tecnick\Color\Exception as ColorException;
use Com\Tecnick\Color\Model\Cmyk;
use Com\Tecnick\Color\Model\Lab;

/**
 * Com\Tecnick\Color\Spot
 *
 * Spot Color class
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-color
 *
 * @phpstan-type TLabTriplet array{0: float, 1: float, 2: float}
 * @phpstan-type TLabRange array{0: float, 1: float, 2: float, 3: float}
 * @phpstan-type TSpotColor array{
 *                 'i': int,
 *                 'n': int,
 *                 'name': string,
 *                 'color': Cmyk,
 *                 'space': 'DeviceCMYK'|'Lab',
 *                 'lab': array{
 *                   'whitepoint': TLabTriplet,
 *                   'blackpoint': TLabTriplet,
 *                   'range': TLabRange,
 *                   'c0': TLabTriplet,
 *                   'model': Lab,
 *                 }|null,
 *               }
 */
class Spot extends \Com\Tecnick\Color\Web
{
    /**
     * Array of default Spot colors
     * Color keys must be in lowercase and without spaces.
     *
     * @var array<string, array{
     *       'name': string,
     *       'color': array{
     *           'cyan': int|float,
     *           'magenta': int|float,
     *           'yellow': int|float,
     *           'key': int|float,
     *           'alpha': int|float,
     *       }
     *     }>
     */
    public const DEFAULT_SPOT_COLORS = [
        'none' => [
            'name' => 'None',
            'color' => [
                'cyan' => 0,
                'magenta' => 0,
                'yellow' => 0,
                'key' => 0,
                'alpha' => 1,
            ],
        ],
        'all' => [
            'name' => 'All',
            'color' => [
                'cyan' => 1,
                'magenta' => 1,
                'yellow' => 1,
                'key' => 1,
                'alpha' => 1,
            ],
        ],
        'cyan' => [
            'name' => 'Cyan',
            'color' => [
                'cyan' => 1,
                'magenta' => 0,
                'yellow' => 0,
                'key' => 0,
                'alpha' => 1,
            ],
        ],
        'magenta' => [
            'name' => 'Magenta',
            'color' => [
                'cyan' => 0,
                'magenta' => 1,
                'yellow' => 0,
                'key' => 0,
                'alpha' => 1,
            ],
        ],
        'yellow' => [
            'name' => 'Yellow',
            'color' => [
                'cyan' => 0,
                'magenta' => 0,
                'yellow' => 1,
                'key' => 0,
                'alpha' => 1,
            ],
        ],
        'key' => [
            'name' => 'Key',
            'color' => [
                'cyan' => 0,
                'magenta' => 0,
                'yellow' => 0,
                'key' => 1,
                'alpha' => 1,
            ],
        ],
        'white' => [
            'name' => 'White',
            'color' => [
                'cyan' => 0,
                'magenta' => 0,
                'yellow' => 0,
                'key' => 0,
                'alpha' => 1,
            ],
        ],
        'black' => [
            'name' => 'Black',
            'color' => [
                'cyan' => 0,
                'magenta' => 0,
                'yellow' => 0,
                'key' => 1,
                'alpha' => 1,
            ],
        ],
        'red' => [
            'name' => 'Red',
            'color' => [
                'cyan' => 0,
                'magenta' => 1,
                'yellow' => 1,
                'key' => 0,
                'alpha' => 1,
            ],
        ],
        'green' => [
            'name' => 'Green',
            'color' => [
                'cyan' => 1,
                'magenta' => 0,
                'yellow' => 1,
                'key' => 0,
                'alpha' => 1,
            ],
        ],
        'blue' => [
            'name' => 'Blue',
            'color' => [
                'cyan' => 1,
                'magenta' => 1,
                'yellow' => 0,
                'key' => 0,
                'alpha' => 1,
            ],
        ],
    ];

    /**
     * Array of Spot colors
     *
     * @var array<string, TSpotColor>
     */
    protected array $spot_colors = [];

    /**
     * Returns the array of spot colors.
     *
     * @return array<string, TSpotColor>
     */
    public function getSpotColors(): array
    {
        return $this->spot_colors;
    }

    /**
     * Return the normalized case-insentitive version of the spot color name to be used as key.
     *
     * @param string $name Full name of the spot color.
     */
    public function normalizeSpotColorName(string $name): string
    {
        $ret = \preg_replace('/[^a-z0-9]+/', '', \strtolower($name));
        return $ret ?? '';
    }

    /**
     * Encode a spot color name as a PDF name object.
     *
     * The original name is preserved as-is, including spaces and uppercase
     * letters. Any byte that is not a regular PDF name character is escaped
     * using the "#" followed by a 2-digit uppercase hexadecimal code, as
     * required by the PDF name object syntax (ISO 32000-1:2008, 7.3.5).
     * For example "SPOTTYPE 279 C" becomes "SPOTTYPE#20279#20C".
     *
     * @param string $name Full name of the spot color.
     */
    public function encodeSpotColorName(string $name): string
    {
        $out = '';
        $len = \strlen($name);
        for ($i = 0; $i < $len; ++$i) {
            $char = $name[$i];
            $ord = \ord($char);
            // Regular characters are printable ASCII (0x21-0x7E),
            // excluding the number sign and the PDF delimiters.
            if ($ord < 0x21 || $ord > 0x7E || $char === '#' || \str_contains('()<>[]{}/%', $char)) {
                $out .= \sprintf('#%02X', $ord);
                continue;
            }

            $out .= $char;
        }

        return $out;
    }

    /**
     * Return the requested spot color data array, registering it on first use.
     *
     * If the name matches a default spot color that is not yet registered, it is
     * added to the internal registry so that it is emitted by
     * getPdfSpotObjects()/getPdfSpotResources(). Use resolveSpotColorData() for a
     * side-effect-free lookup.
     *
     * @param string $name Full name of the spot color.
     *
     * @return TSpotColor
     *
     * @throws ColorException if the color is not found
     */
    public function getSpotColor(string $name): array
    {
        $key = $this->normalizeSpotColorName($name);
        if (!\array_key_exists($key, $this->spot_colors)) {
            // resolve a default spot color and register it on first use
            $this->spot_colors[$key] = $this->resolveSpotColorData($name);
        }

        return $this->spot_colors[$key];
    }

    /**
     * Resolve the requested spot color data array WITHOUT registering it.
     *
     * Returns the already-registered spot color if present, otherwise builds a
     * transient entry from the default spot colors. Unlike getSpotColor(), this
     * has no side effect on the internal registry and therefore does not alter
     * the output of getPdfSpotObjects()/getPdfSpotResources(). It backs the
     * read-only color accessors.
     *
     * @param string $name Full name of the spot color.
     *
     * @return TSpotColor
     *
     * @throws ColorException if the color is not found
     */
    protected function resolveSpotColorData(string $name): array
    {
        $key = $this->normalizeSpotColorName($name);
        $spotColor = $this->spot_colors[$key] ?? null;
        if (\is_array($spotColor)) {
            return $spotColor;
        }

        $defaultSpotColor = self::DEFAULT_SPOT_COLORS[$key] ?? null;
        if ($defaultSpotColor === null) {
            throw new ColorException('unable to find the spot color: ' . $key);
        }

        return [
            'i' => \count($this->spot_colors) + 1, // color index
            'n' => 0, // PDF object number
            'name' => $defaultSpotColor['name'], // color name
            'color' => new Cmyk($defaultSpotColor['color']), // CMYK color object
            'space' => 'DeviceCMYK', // alternate color space in PDF Separation
            'lab' => null, // optional Lab metadata
        ];
    }

    /**
     * Return the requested spot color CMYK object.
     *
     * @param string $name Full name of the spot color.
     *
     * @throws ColorException if the color is not found
     */
    public function getSpotColorObj(string $name): Cmyk
    {
        return $this->getSpotColor($name)['color'];
    }

    /**
     * Return the requested spot color Lab object.
     *
     * @param string $name Full name of the spot color.
     *
     * @throws ColorException if the color is not found
     */
    public function getSpotLabColorObj(string $name): Lab
    {
        $spot = $this->getSpotColor($name);
        if (\is_array($spot['lab'])) {
            return $spot['lab']['model'];
        }

        return new Lab($spot['color']->toLabArray());
    }

    /**
     * Add a new spot color or overwrite an existing one with the same name.
     *
     * @param string $name Full name of the spot color.
     * @param Cmyk   $cmyk CMYK color object
     *
     * @return string Spot color key.
     */
    public function addSpotColor(string $name, Cmyk $cmyk): string
    {
        $key = $this->normalizeSpotColorName($name);
        $num = $this->spot_colors[$key]['i'] ?? (\count($this->spot_colors) + 1);

        $this->spot_colors[$key] = [
            'i' => $num, // color index
            'n' => 0, // PDF object number
            'name' => $name, // color name (key)
            'color' => $cmyk, // CMYK color object
            'space' => 'DeviceCMYK', // alternate color space in PDF Separation
            'lab' => null, // optional Lab metadata
        ];

        return $key;
    }

    /**
     * Add a new spot color from an array of CMYK components.
     *
     * @param string                         $name       Full name of the spot color.
     * @param array<string, int|float|string> $components CMYK components.
     *
     * @return string Spot color key.
     */
    public function addSpotColorFromArray(string $name, array $components): string
    {
        return $this->addSpotColor($name, new Cmyk($components));
    }

    /**
     * Add a new Lab-based spot color or overwrite an existing one with the same name.
     *
     * The optional Lab settings are passed as separate trailing array arguments,
     * in this order: whitepoint, blackpoint, range, col0. For example:
     *   addSpotLabColor('My Color', 50.0, 10.0, -20.0, $whitepoint, $blackpoint, $range, $col0);
     * Any omitted option falls back to its default (D65 whitepoint [0.9505, 1.0, 1.089],
     * zero blackpoint, [-128, 127, -128, 127] range, [100, 0, 0] col0).
     *
     * NOTE: the stored CMYK equivalent is an approximation computed from the Lab
     * values using the D65 whitepoint; the whitepoint option only affects the PDF
     * Lab color space metadata emitted by getPdfSpotObjects(), not the CMYK fallback.
     *
     * @param string                $name          Full name of the spot color.
     * @param float                 $lstar         Lab L* component in [0..100].
     * @param float                 $astar         Lab a* component.
     * @param float                 $bstar         Lab b* component.
     * @param TLabTriplet|TLabRange ...$labOptions Optional Lab settings: whitepoint, blackpoint, range, col0.
     *
     * @return string Spot color key.
     */
    public function addSpotLabColor(
        string $name,
        float $lstar,
        float $astar,
        float $bstar,
        array ...$labOptions,
    ): string {
        $whitepoint = $this->resolveTripletOption($labOptions, 0, [0.9505, 1.0000, 1.0890]);
        $blackpoint = $this->resolveTripletOption($labOptions, 1, [0.0, 0.0, 0.0]);
        $range = $this->resolveRangeOption($labOptions, 2, [-128.0, 127.0, -128.0, 127.0]);
        $col0 = $this->resolveTripletOption($labOptions, 3, [100.0, 0.0, 0.0]);

        $key = $this->normalizeSpotColorName($name);
        $num = $this->spot_colors[$key]['i'] ?? (\count($this->spot_colors) + 1);
        $labModel = $this->buildLabModel($lstar, $astar, $bstar, $range);

        $this->spot_colors[$key] = [
            'i' => $num, // color index
            'n' => 0, // PDF object number
            'name' => $name, // color name (key)
            'color' => new Cmyk($labModel->toCmykArray()), // CMYK equivalent
            'space' => 'Lab', // alternate color space in PDF Separation
            'lab' => $this->buildLabMetadata($whitepoint, $blackpoint, $range, $col0, $labModel),
        ];

        return $key;
    }

    /**
     * @param array<array-key, TLabTriplet|TLabRange> $labOptions
     * @param TLabTriplet                       $default
     *
     * @return TLabTriplet
     */
    private function resolveTripletOption(array $labOptions, int $index, array $default): array
    {
        $option = $labOptions[$index] ?? $default;
        return [
            $option[0] ?? $default[0],
            $option[1] ?? $default[1],
            $option[2] ?? $default[2],
        ];
    }

    /**
     * @param array<array-key, TLabTriplet|TLabRange> $labOptions
     * @param TLabRange                         $default
     *
     * @return TLabRange
     */
    private function resolveRangeOption(array $labOptions, int $index, array $default): array
    {
        $option = $labOptions[$index] ?? $default;
        return [
            $option[0] ?? $default[0],
            $option[1] ?? $default[1],
            $option[2] ?? $default[2],
            $option[3] ?? $default[3],
        ];
    }

    /**
     * @param TLabRange $labRange
     */
    private function buildLabModel(float $lstar, float $astar, float $bstar, array $labRange): Lab
    {
        return new Lab([
            'lstar' => \max(0.0, \min(100.0, $lstar)),
            'astar' => \max($labRange[0], \min($labRange[1], $astar)),
            'bstar' => \max($labRange[2], \min($labRange[3], $bstar)),
            'alpha' => 1.0,
        ]);
    }

    /**
     * @param TLabTriplet $whitepoint
     * @param TLabTriplet $blackpoint
     * @param TLabRange   $range
     * @param TLabTriplet $col0
     *
     * @return array{
     *     whitepoint: TLabTriplet,
     *     blackpoint: TLabTriplet,
     *     range: TLabRange,
     *     c0: TLabTriplet,
     *     model: Lab
     * }
     */
    private function buildLabMetadata(
        array $whitepoint,
        array $blackpoint,
        array $range,
        array $col0,
        Lab $labModel,
    ): array {
        return [
            'whitepoint' => $whitepoint,
            'blackpoint' => $blackpoint,
            'range' => $range,
            'c0' => [
                \max(0.0, \min(100.0, $col0[0])),
                \max($range[0], \min($range[1], $col0[1])),
                \max($range[2], \min($range[3], $col0[2])),
            ],
            'model' => $labModel,
        ];
    }

    /**
     * Returns a PDF-formatted numeric array.
     *
     * @param array<int, float> $values
     */
    private function getPdfNumArray(array $values): string
    {
        $out = [];
        foreach ($values as $value) {
            $out[] = \sprintf('%F', $value);
        }

        return '[' . \implode(' ', $out) . ']';
    }

    /**
     * Returns the PDF command to output Spot color objects.
     *
     * @param int $pon Current PDF object number
     *
     * @return string PDF command
     */
    public function getPdfSpotObjects(int &$pon): string
    {
        $out = '';
        foreach ($this->spot_colors as $key => $color) {
            if ($color['space'] === 'Lab' && \is_array($color['lab'])) {
                $out .= ++$pon . ' 0 obj' . "\n";
                $this->spot_colors[$key]['n'] = $pon;
                $lab = $color['lab']['model']->toLabArray();
                $out .=
                    '[/Separation /'
                    . $this->encodeSpotColorName($color['name'])
                    . ' [/Lab <<'
                    . ' /WhitePoint '
                    . $this->getPdfNumArray($color['lab']['whitepoint'])
                    . ' /BlackPoint '
                    . $this->getPdfNumArray($color['lab']['blackpoint'])
                    . ' /Range '
                    . $this->getPdfNumArray($color['lab']['range'])
                    . '>>] <<'
                    . ' /FunctionType 2'
                    . ' /Domain [0 1]'
                    . ' /C0 '
                    . $this->getPdfNumArray($color['lab']['c0'])
                    . ' /C1 '
                    . $this->getPdfNumArray([
                        $lab['lstar'] ?? 0.0,
                        $lab['astar'] ?? 0.0,
                        $lab['bstar'] ?? 0.0,
                    ])
                    . ' /N 1'
                    . '>>]'
                    . "\n"
                    . 'endobj'
                    . "\n";
                continue;
            }

            if (!$this->isCmykSpotColor($color['color'])) {
                // Skip entries that are neither valid Lab nor CMYK spot colors
                // without consuming a PDF object number or emitting a partial object.
                continue;
            }

            $out .= ++$pon . ' 0 obj' . "\n";
            $this->spot_colors[$key]['n'] = $pon;
            $out .=
                '[/Separation /'
                . $this->encodeSpotColorName($color['name'])
                . ' /DeviceCMYK <<'
                . '/Range [0 1 0 1 0 1 0 1]'
                . ' /C0 [0 0 0 0]'
                . ' /C1 ['
                . $color['color']->getComponentsString()
                . ']'
                . ' /FunctionType 2'
                . ' /Domain [0 1]'
                . ' /N 1'
                . '>>]'
                . "\n"
                . 'endobj'
                . "\n";
        }

        return $out;
    }

    /**
     * Check whether a value is a CMYK color model.
     */
    private function isCmykSpotColor(mixed $color): bool
    {
        return $color instanceof Cmyk;
    }

    /**
     * Returns the PDF command to output the provided Spot color resources.
     *
     * @param array<string, array{'i': int, 'n': int}|TSpotColor> $data Spot color array.
     *
     * @return string PDF command
     */
    private function getOutPdfSpotResources(array $data): string
    {
        if ($data === []) {
            return '';
        }

        $out = '/ColorSpace <<';

        foreach ($data as $spot_color) {
            $out .= ' /CS' . $spot_color['i'] . ' ' . $spot_color['n'] . ' 0 R';
        }

        return $out . ' >>' . "\n";
    }

    /**
     * Returns the PDF command to output Spot color resources.
     *
     * @return string PDF command
     */
    public function getPdfSpotResources(): string
    {
        return $this->getOutPdfSpotResources($this->spot_colors);
    }

    /**
     * Returns the PDF command to output Spot color resources.
     *
     * @param array<string> $keys Array of font keys.
     *
     * @return string PDF command
     */
    public function getPdfSpotResourcesByKeys(array $keys): string
    {
        if ($keys === []) {
            return '';
        }

        $data = [];
        foreach ($keys as $key) {
            if (!\array_key_exists($key, $this->spot_colors)) {
                continue;
            }

            $data[$key] = $this->spot_colors[$key];
        }

        return $this->getOutPdfSpotResources($data);
    }
}
