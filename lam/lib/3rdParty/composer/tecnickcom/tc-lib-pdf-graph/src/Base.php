<?php

declare(strict_types=1);

/**
 * Base.php
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

use Com\Tecnick\Color\Pdf as PdfColor;
use Com\Tecnick\Pdf\Encrypt\Encrypt;

/**
 * Com\Tecnick\Pdf\Graph\Base
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfGraph
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-graph
 *
 * @phpstan-type TTMatrix array{
 *          float,
 *          float,
 *          float,
 *          float,
 *          float,
 *          float,
 *       }
 *
 * @phpstan-type StyleData array{
 *          'lineWidth': float,
 *          'lineCap': string,
 *          'lineJoin': string,
 *          'miterLimit': float,
 *          'dashArray': array<int>,
 *          'dashPhase': float,
 *          'lineColor': string,
 *          'fillColor': string,
 *      }
 *
 * @phpstan-type StyleDataOpt array{
 *          'lineWidth'?: float,
 *          'lineCap'?: string,
 *          'lineJoin'?: string,
 *          'miterLimit'?: float,
 *          'dashArray'?: array<int>,
 *          'dashPhase'?: float,
 *          'lineColor'?: string,
 *          'fillColor'?: string,
 *      }
 *
 * @phpstan-type GradientData array{
 *          'antialias': bool,
 *          'background': ?\Com\Tecnick\Color\Model,
 *          'colors': array<int, array{
 *              'color': string,
 *              'exponent'?: float,
 *              'opacity'?: float,
 *              'offset'?: float,
 *          }>,
 *          'colspace': string,
 *          'coords': array<float>,
 *          'id': int,
 *          'pattern': int,
 *          'stream': string,
 *          'transparency': bool,
 *          'type': int,
 *      }
 *
 * @SuppressWarnings("PHPMD.ExcessiveClassComplexity")
 */
abstract class Base
{
    /**
     * Pi constant
     * We use this instead of M_PI because HHVM has a different value.
     *
     * @var float
     */
    public const MPI = 3.141_592_653_589_793_238_462_643_383_279_502_884_197_169_399_375_10;

    /**
     * Identity matrix for transformations.
     *
     * @var TTMatrix
     */
    public const IDMATRIX = [1.0, 0.0, 0.0, 1.0, 0.0, 0.0];

    /**
     * PDF graphics state save operator.
     *
     * @var string
     */
    protected const GSAVE = 'q' . "\n";

    /**
     * PDF graphics state restore operator.
     *
     * @var string
     */
    protected const GRESTORE = 'Q' . "\n";

    /**
     * Current PDF object number
     */
    protected int $pon = 0;

    /**
     * Current page height
     */
    protected float $pageh = 0;

    /**
     * Current page width
     */
    protected float $pagew = 0;

    /**
     * Unit of measure conversion ratio
     */
    protected float $kunit = 1.0;

    /**
     * Stack index.
     */
    protected int $styleid = -1;

    /**
     * Stack containing style data.
     *
     * @var array<StyleDataOpt>
     */
    protected array $style = [];

    /**
     * Array of transparency objects and parameters.
     *
     * @var array<int, array{
     *          'n': int,
     *          'name': string,
     *          'parms': array<string, int|float|bool|string>,
     *      }>
     */
    protected array $extgstates = [];

    /**
     * Array of gradients
     *
     * @var array<int, GradientData>
     */
    protected array $gradients = [];

    /**
     * Bookkeeping for the luminosity soft-mask patterns generated for
     * transparent gradients, keyed by the transparency pattern index used as
     * the resource name (/p{key}) inside the soft-mask form XObject.
     *
     * These are internal helper objects, referenced only from within the
     * soft-mask XObject, and are deliberately kept out of $gradients so they
     * are not advertised in the page-level gradient resource dictionary.
     *
     * @var array<int, array{'id': int, 'pattern': int}>
     */
    protected array $gradientMasks = [];

    /**
     * Initialize
     *
     * @param float    $kunit    Unit of measure conversion ratio.
     * @param float    $pagew    Page width.
     * @param float    $pageh    Page height.
     * @param PdfColor $pdfColor Color object.
     * @param Encrypt  $encrypt  Encrypt object.
     * @param bool     $pdfa     True if we are in PDF/A mode.
     * @param bool     $compress Set to false to disable stream compression.
     */
    public function __construct(
        float $kunit,
        float $pagew,
        float $pageh,
        /**
         * Color object
         */
        protected PdfColor $pdfColor,
        /**
         * Encrypt object
         */
        protected Encrypt $encrypt,
        protected bool $pdfa = false,
        protected bool $compress = true,
    ) {
        $this->setKUnit($kunit);
        $this->setPageWidth($pagew);
        $this->setPageHeight($pageh);
        $this->initStyle();
    }

    /**
     * Initialize default style
     */
    public function initStyle(): void
    {
        $this->style[++$this->styleid] = $this->getDefaultStyle();
    }

    /**
     * Returns the default style.
     *
     * @param StyleDataOpt $style Style parameters to merge to the default ones.
     *
     * @return StyleData
     */
    public function getDefaultStyle(array $style = []): array
    {
        return [
            // line thickness in user units
            'lineWidth' => $style['lineWidth'] ?? (1.0 / $this->kunit),
            // shape of the endpoints for any open path that is stroked
            'lineCap' => $style['lineCap'] ?? 'butt',
            // shape of joints between connected segments of a stroked path
            'lineJoin' => $style['lineJoin'] ?? 'miter',
            // maximum length of mitered line joins for stroked paths
            'miterLimit' => $style['miterLimit'] ?? (10.0 / $this->kunit),
            // lengths of alternating dashes and gaps
            'dashArray' => $style['dashArray'] ?? [],
            // distance  at which to start the dash
            'dashPhase' => $style['dashPhase'] ?? 0.0,
            // line (drawing) color
            'lineColor' => $style['lineColor'] ?? 'black',
            // background (filling) color
            'fillColor' => $style['fillColor'] ?? 'black',
        ];
    }

    /**
     * Returns current PDF object number
     */
    public function getObjectNumber(): int
    {
        return $this->pon;
    }

    /**
     * Set page height and returns previous value.
     *
     * @param float $pageh Page height.
     *
     * @return float previous page height value.
     */
    public function setPageHeight(float $pageh): float
    {
        $ret = $this->pageh;
        $this->pageh = $pageh;
        return $ret;
    }

    /**
     * Set page width and returns previous value.
     *
     * @param float $pagew Page width.
     *
     * @return float previous page width value.
     */
    public function setPageWidth(float $pagew): float
    {
        $ret = $this->pagew;
        $this->pagew = $pagew;
        return $ret;
    }

    /**
     * Set unit of measure conversion ratio.
     *
     * @param float $kunit Unit of measure conversion ratio.
     */
    public function setKUnit(float $kunit): static
    {
        $this->kunit = $kunit;
        return $this;
    }

    /**
     * Get the PDF output string for ExtGState
     *
     * @param int $pon Current PDF Object Number
     *
     * @return string PDF command
     */
    public function getOutExtGState(int $pon): string
    {
        $this->pon = $pon;
        $out = '';
        foreach ($this->extgstates as $idx => $ext) {
            $this->extgstates[$idx]['n'] = ++$this->pon;
            $out .= $this->pon . ' 0 obj' . "\n" . '<< /Type /ExtGState';
            foreach ($ext['parms'] as $key => $val) {
                $val = match (true) {
                    \is_numeric($val) => \sprintf('%F', $val),
                    $val === true => 'true',
                    $val === false => 'false',
                    default => $val,
                };

                $out .= ' /' . $key . ' ' . $val;
            }

            $out .= ' >>' . "\n" . 'endobj' . "\n";
        }

        return $out;
    }

    /**
     * Returns the last extgstate ID to be used with XObjects.
     *
     * @return ?int
     */
    public function getLastExtGStateID(): ?int
    {
        return \array_key_last($this->extgstates);
    }

    /**
     * Returns the resource names of the registered ExtGState objects that carry
     * actual (non-trivial) transparency: a fill or stroke alpha below 1, a blend
     * mode other than Normal, or a soft mask.
     *
     * The returned names match the tokens used by the `gs` operator in content
     * streams (for example "GS3"), so callers can detect whether a given content
     * stream actually triggers transparency (as opposed to merely resetting the
     * graphics state back to fully opaque).
     *
     * @return array<int, string> List of ExtGState resource names.
     */
    public function getTransparencyExtGStateNames(): array
    {
        $names = [];
        foreach ($this->extgstates as $key => $ext) {
            if (!self::extGStateHasTransparency($ext['parms'])) {
                continue;
            }

            $names[] = $ext['name'] !== '' ? $ext['name'] : 'GS' . $key;
        }

        return $names;
    }

    /**
     * Whether the given ExtGState parameters describe actual transparency:
     * a constant alpha below 1, a non-Normal blend mode, or a soft mask.
     *
     * @param array<string, int|float|bool|string> $parms ExtGState parameters.
     */
    private static function extGStateHasTransparency(array $parms): bool
    {
        $belowOne = static fn(string $key): bool => (
            isset($parms[$key])
            && \is_numeric($parms[$key])
            && (float) $parms[$key] < 1.0
        );
        if ($belowOne('ca') || $belowOne('CA')) {
            return true;
        }

        $blend = isset($parms['BM']) ? \ltrim((string) $parms['BM'], '/') : 'Normal';
        if ($blend !== '' && $blend !== 'Normal') {
            return true;
        }

        $smask = isset($parms['SMask']) ? \ltrim((string) $parms['SMask'], '/') : 'None';
        return $smask !== '' && $smask !== 'None';
    }

    /**
     * Get the PDF output string for ExtGState Resource Dictionary.
     *
     * @param array<int, array{'name': string, 'n': int, 'parms'?: array<string, int|float|bool|string>}> $data extgstates data.
     *
     * @return string PDF command
     */
    private function getOutExtGStateResDict(array $data): string
    {
        if ($this->pdfa || $this->extgstates === []) {
            return '';
        }

        $out = ' /ExtGState <<';

        foreach ($data as $key => $ext) {
            $out .= $ext['name'] !== '' ? ' /' . $ext['name'] : ' /GS' . $key;
            $out .= ' ' . $ext['n'] . ' 0 R';
        }

        return $out . (' >>' . "\n");
    }

    /**
     * Get the PDF output string for ExtGState Resource Dictionary
     *
     * @return string PDF command
     */
    public function getOutExtGStateResources(): string
    {
        return $this->getOutExtGStateResDict($this->extgstates);
    }

    /**
     * Get the PDF output string for ExtGState Resource Dictionary for XObjects.
     *
     * @param array<int> $keys Array of extgstates keys.
     *
     * @return string PDF command
     */
    public function getOutExtGStateResourcesByKeys(array $keys): string
    {
        if ($keys === []) {
            return '';
        }

        $data = [];
        foreach ($keys as $key) {
            $ext = $this->extgstates[$key] ?? null;
            if (!\is_array($ext)) {
                continue;
            }

            $data[$key] = [
                'name' => $ext['name'],
                'n' => $ext['n'],
            ];
        }

        return $this->getOutExtGStateResDict($data);
    }

    /**
     * Get the PDF output string for Gradients Resource Dictionary.
     *
     * @param array<int, array{'id': int, 'pattern': int, 'antialias'?: bool, 'background'?: ?\Com\Tecnick\Color\Model, 'colors'?: array<int, array{'color': string, 'exponent'?: float, 'opacity'?: float, 'offset'?: float}>, 'colspace'?: string, 'coords'?: array<float>, 'stream'?: string, 'transparency'?: bool, 'type'?: int}> $data gradients data.
     *
     * @return string PDF command
     */
    private function getOutGradientResDict(array $data): string
    {
        if ($this->pdfa || $data === []) {
            return '';
        }

        $grp = '';
        $grs = '';

        foreach ($data as $idx => $grad) {
            // gradient patterns
            $grp .= ' /p' . $idx . ' ' . $grad['pattern'] . ' 0 R';
            // gradient shadings
            $grs .= ' /Sh' . $idx . ' ' . $grad['id'] . ' 0 R';
        }

        return ' /Pattern <<' . $grp . ' >>' . "\n" . ' /Shading <<' . $grs . ' >>' . "\n";
    }

    /**
     * Get the PDF output string for Gradients Resource Dictionary
     *
     * @return string PDF command
     */
    public function getOutGradientResources(): string
    {
        return $this->getOutGradientResDict($this->gradients);
    }

    /**
     * Returns the PDF command to output gradient resources.
     *
     * @param array<int> $keys Array of gradient keys.
     *
     * @return string PDF command
     */
    public function getOutGradientResourcesByKeys(array $keys): string
    {
        if ($keys === []) {
            return '';
        }

        $data = [];
        foreach ($keys as $key) {
            $grad = $this->gradients[$key] ?? null;
            if (!\is_array($grad)) {
                continue;
            }

            $data[$key] = [
                'id' => $grad['id'],
                'pattern' => $grad['pattern'],
            ];
        }

        return $this->getOutGradientResDict($data);
    }

    /**
     * Get the PDF output string for gradient colors and transparency
     *
     * @param GradientData $grad Array of gradient colors
     * @param string       $type Type of output: 'color' or 'opacity'
     *
     * @return string PDF command
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     */
    protected function getOutGradientCols(array $grad, string $type): string
    {
        if ($type === 'opacity' && !$grad['transparency']) {
            return '';
        }

        $out = '';
        if ($grad['type'] === 2 || $grad['type'] === 3) {
            $num_cols = \count($grad['colors']);
            $lastcols = $num_cols - 1;
            $funct = []; // color and transparency objects
            $bounds = [];
            $encode = [];

            for ($idx = 1; $idx < $num_cols; ++$idx) {
                $prevcol = $grad['colors'][$idx - 1] ?? null;
                $nextcol = $grad['colors'][$idx] ?? null;
                if (!\is_array($prevcol) || !\is_array($nextcol)) {
                    continue;
                }

                $col0 = $prevcol[$type] ?? null;
                $col1 = $nextcol[$type] ?? null;
                if ($type !== 'color') {
                    if (!\is_numeric($col0) || !\is_numeric($col1)) {
                        continue;
                    }

                    $col0 = \sprintf('%F', (float) $col0);
                    $col1 = \sprintf('%F', (float) $col1);
                } else {
                    if (!\is_string($col0) || !\is_string($col1)) {
                        continue;
                    }

                    $model0 = $this->pdfColor->getColorObject($col0);
                    $model1 = $this->pdfColor->getColorObject($col1);
                    if (!$model0 instanceof \Com\Tecnick\Color\Model) {
                        continue;
                    }

                    if (!$model1 instanceof \Com\Tecnick\Color\Model) {
                        continue;
                    }

                    $col0 = $model0->getComponentsString();
                    $col1 = $model1->getComponentsString();
                }

                $encode[] = '0 1';
                $offset = $nextcol['offset'] ?? null;
                if ($idx < $lastcols && \is_float($offset)) {
                    $bounds[] = \sprintf('%F ', $offset);
                }

                $out .=
                    ++$this->pon
                    . ' 0 obj'
                    . "\n"
                    . '<<'
                    . ' /FunctionType 2'
                    . ' /Domain [0 1]'
                    . ' /C0 ['
                    . $col0
                    . ']'
                    . ' /C1 ['
                    . $col1
                    . ']';
                $exponent = $nextcol['exponent'] ?? null;
                if (\is_float($exponent)) {
                    $out .= ' /N ' . $exponent;
                }

                $out .= ' >>' . "\n" . 'endobj' . "\n";
                $funct[] = $this->pon . ' 0 R';
            }

            $out .=
                ++$this->pon
                . ' 0 obj'
                . "\n"
                . '<<'
                . ' /FunctionType 3'
                . ' /Domain [0 1]'
                . ' /Functions ['
                . \implode(' ', $funct)
                . ']'
                . ' /Bounds ['
                . \implode(' ', $bounds)
                . ']'
                . ' /Encode ['
                . \implode(' ', $encode)
                . ']'
                . ' >>'
                . "\n"
                . 'endobj'
                . "\n";
        }

        return $out . $this->getOutPatternObj($grad, $this->pon);
    }

    /**
     * Get the PDF output string for the pattern and shading object
     *
     * @param GradientData $grad   Array of gradient colors
     * @param int          $objref Reference object number
     *
     * @return string PDF command
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function getOutPatternObj(array $grad, int $objref): string
    {
        // set shading object
        if ($grad['transparency']) {
            $grad['colspace'] = 'DeviceGray';
        }

        $oid = ++$this->pon;
        $coords = $grad['coords'];
        $out = $oid . ' 0 obj' . "\n" . '<< /ShadingType ' . $grad['type'] . ' /ColorSpace /' . $grad['colspace'];
        if ($grad['background'] !== null) {
            $out .= ' /Background [' . $grad['background']->getComponentsString() . ']';
        }

        if ($grad['antialias']) {
            $out .= ' /AntiAlias true';
        }

        if ($grad['type'] === 2) {
            $out .= \sprintf(
                ' /Coords [%F %F %F %F] /Domain [0 1] /Function %d 0 R /Extend [true true] >>' . "\n",
                $coords[0] ?? 0.0,
                $coords[1] ?? 0.0,
                $coords[2] ?? 0.0,
                $coords[3] ?? 0.0,
                $objref,
            );
        } elseif ($grad['type'] === 3) {
            // x0, y0, r0, x1, y1, r1
            // the  radius of the inner circle is 0
            $out .= \sprintf(
                ' /Coords [%F %F 0 %F %F %F] /Domain [0 1] /Function %d 0 R /Extend [true true] >>' . "\n",
                $coords[0] ?? 0.0,
                $coords[1] ?? 0.0,
                $coords[2] ?? 0.0,
                $coords[3] ?? 0.0,
                $coords[4] ?? 0.0,
                $objref,
            );
        } elseif ($grad['type'] === 6) {
            $stream = $this->encrypt->encryptString($grad['stream'], $this->pon);
            $out .=
                ' /BitsPerCoordinate 16 /BitsPerComponent 8/Decode[0 1 0 1 0 1 0 1 0 1] /BitsPerFlag 8 /Length '
                . \strlen($stream)
                . ' >>'
                . "\n"
                . ' stream'
                . "\n"
                . $stream
                . "\n"
                . 'endstream'
                . "\n";
        }

        $out .= 'endobj' . "\n";

        // pattern object
        $out .=
            ++$this->pon
            . ' 0 obj'
            . "\n"
            . '<<'
            . ' /Type /Pattern'
            . ' /PatternType 2'
            . ' /Shading '
            . $oid
            . ' 0 R'
            . ' >>'
            . "\n"
            . 'endobj'
            . "\n";

        return $out;
    }

    /**
     * Get the PDF output string for gradient shaders
     *
     * @param int $pon Current PDF Object Number
     *
     * @return string PDF command
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function getOutGradientShaders(int $pon): string
    {
        $this->pon = $pon;

        if ($this->pdfa || $this->gradients === []) {
            return '';
        }

        $idt = \count($this->gradients); // index for transparency gradients
        $out = '';
        foreach ($this->gradients as $idx => $grad) {
            $gcol = $this->getOutGradientCols($grad, 'color');
            if ($gcol !== '') {
                $out .= $gcol;
                $this->gradients[$idx]['id'] = $this->pon - 1;
                $this->gradients[$idx]['pattern'] = $this->pon;
            }

            $gopa = $this->getOutGradientCols($grad, 'opacity');
            $idgs = $idx + $idt;

            if ($gopa !== '') {
                $out .= $gopa;
                $this->gradientMasks[$idgs] = [
                    'id' => $this->pon - 1,
                    'pattern' => $this->pon,
                ];
            }

            if ($grad['transparency']) {
                $mask = $this->gradientMasks[$idgs] ?? null;
                if ($mask === null) {
                    // no transparency pattern was generated: skip without emitting a partial object
                    continue;
                }

                $oid = ++$this->pon;
                $pwidth = $this->pagew * $this->kunit;
                $pheight = $this->pageh * $this->kunit;
                $rect = \sprintf('%F %F', $pwidth, $pheight);

                $out .= $oid . ' 0 obj' . "\n" . '<< /Type /XObject /Subtype /Form /FormType 1';
                $stream = 'q /a0 gs /Pattern cs /p' . $idgs . ' scn 0 0 ' . $pwidth . ' ' . $pheight . ' re f Q';
                if ($this->compress) {
                    $cmpstream = \gzcompress($stream);
                    if ($cmpstream !== false) {
                        $stream = $cmpstream;
                        $out .= ' /Filter /FlateDecode';
                    }
                }

                $stream = $this->encrypt->encryptString($stream, $oid);
                $out .=
                    ' /Length '
                    . \strlen($stream)
                    . ' /BBox [0 0 '
                    . $rect
                    . ']'
                    . ' /Group << /Type /Group /S /Transparency /CS /DeviceGray >>'
                    . ' /Resources <<'
                    . ' /ExtGState << /a0 << /ca 1 /CA 1 >>  >>'
                    . ' /Pattern << /p'
                    . $idgs
                    . ' '
                    . $mask['pattern']
                    . ' 0 R >>'
                    . ' >>'
                    . ' >>'
                    . "\n"
                    . ' stream'
                    . "\n"
                    . $stream
                    . "\n"
                    . 'endstream'
                    . "\n"
                    . 'endobj'
                    . "\n";

                // SMask
                $objsm = ++$this->pon;
                $out .=
                    $objsm
                    . ' 0 obj'
                    . "\n"
                    . '<<'
                    . ' /Type /Mask'
                    . ' /S /Luminosity'
                    . ' /G '
                    . $oid
                    . ' 0 R'
                    . ' >>'
                    . "\n"
                    . 'endobj'
                    . "\n";

                // ExtGState
                $objext = ++$this->pon;
                $out .=
                    $objext
                    . ' 0 obj'
                    . "\n"
                    . '<<'
                    . ' /Type /ExtGState'
                    . ' /SMask '
                    . $objsm
                    . ' 0 R'
                    . ' /AIS false'
                    . ' >>'
                    . "\n"
                    . 'endobj'
                    . "\n";
                $this->extgstates[] = [
                    'n' => $objext,
                    'name' => 'TGS' . $idx,
                    // Record the soft mask so this entry is recognised as carrying
                    // actual transparency (see getTransparencyExtGStateNames()).
                    'parms' => [
                        'SMask' => $objsm . ' 0 R',
                    ],
                ];
            }
        }

        return $out;
    }
}
