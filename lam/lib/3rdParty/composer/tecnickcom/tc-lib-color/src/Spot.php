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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
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
 * Extension points: getSpotColor() and the protected resolveSpotColorData().
 * The registry mutators and the PDF emitters are final.
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
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
class Spot extends \Com\Tecnick\Color\Web implements SpotRegistryInterface
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
     * The color models are copies of the registered ones.
     *
     * @return array<string, TSpotColor>
     */
    final public function getSpotColors(): array
    {
        $out = [];
        foreach ($this->spot_colors as $key => $spotColor) {
            $out[$key] = $this->copySpotColorData($spotColor);
        }

        return $out;
    }

    /**
     * Copy a spot color entry, including the mutable color models it holds.
     *
     * @param TSpotColor $spotColor Entry to copy.
     *
     * @return TSpotColor
     */
    private function copySpotColorData(array $spotColor): array
    {
        $spotColor['color'] = clone $spotColor['color'];
        if (\is_array($spotColor['lab'])) {
            $spotColor['lab']['model'] = clone $spotColor['lab']['model'];
        }

        return $spotColor;
    }

    /**
     * Return the normalized case-insensitive version of the spot color name to be used as key.
     *
     * Every character outside [a-z0-9] is dropped, so names that differ only in
     * punctuation, spacing or accents share a key: 'Bleu Ciel é' and
     * 'Bleu-Ciel' both normalize to 'bleuciel'.
     *
     * @param string $name Full name of the spot color.
     */
    final public function normalizeSpotColorName(string $name): string
    {
        $ret = \preg_replace('/[^a-z0-9]+/', '', \strtolower($name));
        return $ret ?? '';
    }

    /**
     * Encode a spot color name as a PDF name object.
     *
     * The original name is preserved as-is, including spaces and uppercase
     * letters. Any byte that is not a regular PDF name character is escaped as
     * "#" followed by a 2-digit uppercase hexadecimal code, as required by the
     * PDF name object syntax (ISO 32000-1:2008, 7.3.5). For example
     * "SPOTTYPE 279 C" becomes "SPOTTYPE#20279#20C".
     *
     * The result is not truncated to the 127-byte name object limit set by
     * ISO 32000-1:2008, Annex C.2.
     *
     * @param string $name Full name of the spot color.
     */
    final public function encodeSpotColorName(string $name): string
    {
        $delimiters = ['(', ')', '<', '>', '[', ']', '{', '}', '/', '%'];
        $out = '';
        $len = \strlen($name);
        for ($i = 0; $i < $len; ++$i) {
            $char = $name[$i];
            $ord = \ord($char);
            // Regular characters are printable ASCII (0x21-0x7E),
            // excluding the number sign and the PDF delimiters.
            if ($ord < 0x21 || $ord > 0x7E || $char === '#' || \in_array($char, $delimiters, true)) {
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
     * A default spot color that is not yet registered is added to the internal
     * registry, so that it is emitted by getPdfSpotObjects() and
     * getPdfSpotResources(). Use resolveSpotColorData() for a lookup with no
     * side effect.
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
     * transient entry from the default spot colors. The internal registry is
     * left unchanged, so the output of getPdfSpotObjects() and
     * getPdfSpotResources() is not altered.
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
        if ($key === '') {
            throw new ColorException('invalid spot color name: ' . $name);
        }

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
     * Return a copy of the requested spot color CMYK object.
     *
     * @param string $name Full name of the spot color.
     *
     * @throws ColorException if the color is not found
     */
    final public function getSpotColorObj(string $name): Cmyk
    {
        return clone $this->getSpotColor($name)['color'];
    }

    /**
     * Return a copy of the requested spot color Lab object.
     *
     * A CMYK-defined spot color has no Lab metadata, so its Lab equivalent is
     * derived on the fly.
     *
     * @param string $name Full name of the spot color.
     *
     * @throws ColorException if the color is not found
     */
    final public function getSpotLabColorObj(string $name): Lab
    {
        $spot = $this->getSpotColor($name);
        if (\is_array($spot['lab'])) {
            return clone $spot['lab']['model'];
        }

        return new Lab($spot['color']->toLabArray());
    }

    /**
     * Reserve the registry slot for a spot color name.
     *
     * Returns the color index to use: the one already assigned to the name, or
     * the next free one.
     *
     * @param string $name Full name of the spot color.
     *
     * @throws ColorException if the name has no usable characters or the color
     *                        has already been emitted as a PDF object
     */
    private function reserveSpotColorIndex(string $name): int
    {
        $key = $this->normalizeSpotColorName($name);
        if ($key === '') {
            throw new ColorException('invalid spot color name: ' . $name);
        }

        if (($this->spot_colors[$key]['n'] ?? 0) !== 0) {
            throw new ColorException('unable to redefine an emitted spot color: ' . $key);
        }

        return $this->spot_colors[$key]['i'] ?? (\count($this->spot_colors) + 1);
    }

    /**
     * Add a new spot color or overwrite an existing one with the same name.
     *
     * @param string $name Full name of the spot color.
     * @param Cmyk   $cmyk CMYK color object
     *
     * @return string Spot color key.
     *
     * @throws ColorException if the name is unusable or the color has already
     *                        been emitted by getPdfSpotObjects()
     */
    final public function addSpotColor(string $name, Cmyk $cmyk): string
    {
        $num = $this->reserveSpotColorIndex($name);
        $key = $this->normalizeSpotColorName($name);

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
     *
     * @throws ColorException                               if the name is unusable or the color has already been emitted
     * @throws \Com\Tecnick\Color\UnknownComponentException if $components carries a name the CMYK model does not define
     */
    final public function addSpotColorFromArray(string $name, array $components): string
    {
        return $this->addSpotColor($name, new Cmyk($components));
    }

    /**
     * Add a new Lab-based spot color or overwrite an existing one with the same name.
     *
     * The optional Lab settings are passed as separate trailing array arguments,
     * in this order: whitepoint, blackpoint, range, col0. For example:
     *   addSpotLabColor('My Color', 50.0, 10.0, -20.0, $whitepoint, $blackpoint, $range, $col0);
     * Each option array is read element by element, so a shorter array is
     * allowed and any missing element falls back to its default: D65 whitepoint
     * [0.9505, 1.0, 1.089], zero blackpoint, [-128, 127, -128, 127] range and
     * [100, 0, 0] col0.
     *
     * NOTE: the stored CMYK equivalent is an approximation computed from the Lab
     * values using the D65 whitepoint; the whitepoint option only affects the PDF
     * Lab color space metadata emitted by getPdfSpotObjects(), not the CMYK fallback.
     *
     * The range option is clamped to [-128..127], the interval the Lab color
     * model represents.
     *
     * @param string             $name          Full name of the spot color.
     * @param float              $lstar         Lab L* component in [0..100].
     * @param float              $astar         Lab a* component.
     * @param float              $bstar         Lab b* component.
     * @param array<int, float> ...$labOptions Optional Lab settings: whitepoint, blackpoint, range, col0.
     *
     * @return string Spot color key.
     *
     * @throws ColorException if the name is unusable or the color has already
     *                        been emitted by getPdfSpotObjects()
     */
    final public function addSpotLabColor(
        string $name,
        float $lstar,
        float $astar,
        float $bstar,
        array ...$labOptions,
    ): string {
        $whitepoint = $this->resolveTripletOption($labOptions, 0, [0.9505, 1.0000, 1.0890]);
        $blackpoint = $this->resolveTripletOption($labOptions, 1, [0.0, 0.0, 0.0]);
        $range = $this->clampLabRange($this->resolveRangeOption($labOptions, 2, [-128.0, 127.0, -128.0, 127.0]));
        $col0 = $this->resolveTripletOption($labOptions, 3, [100.0, 0.0, 0.0]);

        $num = $this->reserveSpotColorIndex($name);
        $key = $this->normalizeSpotColorName($name);
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
     * @param array<array-key, array<int, float>> $labOptions
     * @param TLabTriplet                         $default
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
     * @param array<array-key, array<int, float>> $labOptions
     * @param TLabRange                           $default
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
     * Narrow a Lab a* and b* range to the [-128..127] interval the Lab color
     * model represents.
     *
     * @param TLabRange $labRange
     *
     * @return TLabRange
     */
    private function clampLabRange(array $labRange): array
    {
        return [
            \max(-128.0, \min(127.0, $labRange[0])),
            \max(-128.0, \min(127.0, $labRange[1])),
            \max(-128.0, \min(127.0, $labRange[2])),
            \max(-128.0, \min(127.0, $labRange[3])),
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
     * Only the entries emitted here can be listed as a resource by
     * getPdfSpotResources().
     *
     * @param int $pon Current PDF object number
     *
     * @return string PDF command
     */
    final public function getPdfSpotObjects(int &$pon): string
    {
        $out = '';
        foreach ($this->spot_colors as $key => $color) {
            if ($color['n'] !== 0) {
                // already emitted
                continue;
            }

            if ($color['space'] === 'Lab' && \is_array($color['lab'])) {
                $body = $this->getPdfLabSeparation($color['name'], $color['lab']);
            } elseif ($this->isCmykSpotColor($color['color'])) {
                $body = $this->getPdfCmykSeparation($color['name'], $color['color']);
            } else {
                // neither a Lab nor a CMYK spot color: no object number is consumed
                continue;
            }

            $this->spot_colors[$key]['n'] = ++$pon;
            $out .= $pon . ' 0 obj' . "\n" . $body . "\n" . 'endobj' . "\n";
        }

        return $out;
    }

    /**
     * Build the Separation array of a Lab-based spot color.
     *
     * The tint transform declares the same /Range as the Lab color space.
     *
     * @param string $name Full name of the spot color.
     * @param array{
     *     whitepoint: TLabTriplet,
     *     blackpoint: TLabTriplet,
     *     range: TLabRange,
     *     c0: TLabTriplet,
     *     model: Lab
     * } $lab Lab metadata.
     */
    private function getPdfLabSeparation(string $name, array $lab): string
    {
        $col1 = $lab['model']->toLabArray();

        return (
            '[/Separation /'
            . $this->encodeSpotColorName($name)
            . ' [/Lab <<'
            . ' /WhitePoint '
            . $this->getPdfNumArray($lab['whitepoint'])
            . ' /BlackPoint '
            . $this->getPdfNumArray($lab['blackpoint'])
            . ' /Range '
            . $this->getPdfNumArray($lab['range'])
            . '>>] <<'
            . ' /FunctionType 2'
            . ' /Domain [0 1]'
            . ' /Range '
            . $this->getPdfNumArray([
                0.0,
                100.0,
                $lab['range'][0],
                $lab['range'][1],
                $lab['range'][2],
                $lab['range'][3],
            ])
            . ' /C0 '
            . $this->getPdfNumArray($lab['c0'])
            . ' /C1 '
            . $this->getPdfNumArray([$col1['lstar'] ?? 0.0, $col1['astar'] ?? 0.0, $col1['bstar'] ?? 0.0])
            . ' /N 1'
            . '>>]'
        );
    }

    /**
     * Build the Separation array of a CMYK-based spot color.
     *
     * @param string $name Full name of the spot color.
     * @param Cmyk   $cmyk CMYK color object.
     */
    private function getPdfCmykSeparation(string $name, Cmyk $cmyk): string
    {
        return (
            '[/Separation /'
            . $this->encodeSpotColorName($name)
            . ' /DeviceCMYK <<'
            . '/Range [0 1 0 1 0 1 0 1]'
            . ' /C0 [0 0 0 0]'
            . ' /C1 ['
            . $cmyk->getComponentsString()
            . ']'
            . ' /FunctionType 2'
            . ' /Domain [0 1]'
            . ' /N 1'
            . '>>]'
        );
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
     * An entry carries a PDF object number only after getPdfSpotObjects() has
     * emitted it; one that does not raises.
     *
     * @param array<string, array{'i': int, 'n': int}|TSpotColor> $data Spot color array.
     *
     * @return string PDF command
     *
     * @throws ColorException if an entry has not been emitted as a PDF object
     */
    private function getOutPdfSpotResources(array $data): string
    {
        if ($data === []) {
            return '';
        }

        $out = '/ColorSpace <<';

        foreach ($data as $key => $spot_color) {
            if ($spot_color['n'] === 0) {
                throw new ColorException(
                    'unable to reference a spot color that has no PDF object,'
                    . ' call getPdfSpotObjects() first: '
                    . $key,
                );
            }

            $out .= ' /CS' . $spot_color['i'] . ' ' . $spot_color['n'] . ' 0 R';
        }

        return $out . ' >>' . "\n";
    }

    /**
     * Returns the PDF command to output Spot color resources.
     *
     * Call getPdfSpotObjects() first: every registered spot color must have been
     * emitted as a PDF object.
     *
     * @return string PDF command
     *
     * @throws ColorException if a registered spot color has not been emitted
     */
    final public function getPdfSpotResources(): string
    {
        return $this->getOutPdfSpotResources($this->spot_colors);
    }

    /**
     * Returns the PDF command to output the Spot color resources for the given keys.
     *
     * @param array<string> $keys Array of spot color keys.
     *
     * @return string PDF command
     *
     * @throws ColorException if a key is not registered or has not been emitted
     */
    final public function getPdfSpotResourcesByKeys(array $keys): string
    {
        if ($keys === []) {
            return '';
        }

        $data = [];
        foreach ($keys as $key) {
            if (!\array_key_exists($key, $this->spot_colors)) {
                throw new ColorException('unable to find the spot color: ' . $key);
            }

            $data[$key] = $this->spot_colors[$key];
        }

        return $this->getOutPdfSpotResources($data);
    }
}
