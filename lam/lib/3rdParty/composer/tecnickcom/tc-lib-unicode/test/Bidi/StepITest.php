<?php

/**
 * StepITest.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 *
 * This file is part of tc-lib-unicode software library.
 */

namespace Test\Bidi;

use Com\Tecnick\Unicode\Bidi\StepI;
use PHPUnit\Framework\Attributes\DataProvider;
use PHPUnit\Framework\TestCase;

/**
 * Bidi Test
 *
 * @since     2011-05-23
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 */
class StepITest extends TestCase
{
    /**
     * @param array{
     *        'e': int,
     *        'edir': string,
     *        'end': int,
     *        'eos': string,
     *        'length': int,
     *        'maxlevel': int,
     *        'sos': string,
     *        'start': int,
     *        'item': array<int, array{
     *            'char': int,
     *            'i': int,
     *            'level': int,
     *            'otype': string,
     *            'pdimatch': int,
     *            'pos': int,
     *            'type': string,
     *            'x': int,
     *          }>,
     *        } $seq     Isolated Sequence array
     *
     * @param mixed $expected Expected result
     */
    #[DataProvider('stepIDataProvider')]
    public function testStepI(array $seq, mixed $expected): void
    {
        $stepi = new StepI($seq);
        $this->assertEquals($expected, $stepi->getSequence());
    }

    /**
     * @return array<int,  array<int, array{
     *        'e': int,
     *        'edir': string,
     *        'end': int,
     *        'eos': string,
     *        'length': int,
     *        'maxlevel': int,
     *        'sos': string,
     *        'start': int,
     *        'item': array<int, array{
     *            'char': int,
     *            'i': int,
     *            'level': int,
     *            'otype': string,
     *            'pdimatch': int,
     *            'pos': int,
     *            'type': string,
     *            'x': int,
     *          }>,
     *        }>>
     */
    public static function stepIDataProvider(): array
    {
        return [
            [
                [
                    'e' => 0,
                    'edir' => 'L',
                    'end' => 3,
                    'eos' => 'L',
                    'length' => 4,
                    'maxlevel' => 0,
                    'sos' => 'L',
                    'start' => 0,
                    'item' => [
                        [
                            'char' => 65,
                            'i' => -1,
                            'level' => 0,
                            'otype' => 'L',
                            'pos' => 0,
                            'pdimatch' => -1,
                            'type' => 'L',
                            'x' => 0,
                        ],
                        [
                            'char' => 8207,
                            'i' => -1,
                            'level' => 0,
                            'otype' => 'R',
                            'pos' => 1,
                            'pdimatch' => -1,
                            'type' => 'R',
                            'x' => 0,
                        ],
                        [
                            'char' => 1632,
                            'i' => -1,
                            'level' => 0,
                            'otype' => 'AN',
                            'pos' => 2,
                            'pdimatch' => -1,
                            'type' => 'AN',
                            'x' => 0,
                        ],
                        [
                            'char' => 1776,
                            'i' => -1,
                            'level' => 0,
                            'otype' => 'EN',
                            'pos' => 3,
                            'pdimatch' => -1,
                            'type' => 'EN',
                            'x' => 0,
                        ],
                    ],
                ],
                [
                    'e' => 0,
                    'edir' => 'L',
                    'end' => 3,
                    'eos' => 'L',
                    'length' => 4,
                    'maxlevel' => 2,
                    'sos' => 'L',
                    'start' => 0,
                    'item' => [
                        [
                            'char' => 65,
                            'i' => -1,
                            'level' => 0,
                            'otype' => 'L',
                            'pos' => 0,
                            'pdimatch' => -1,
                            'type' => 'L',
                            'x' => 0,
                        ],
                        [
                            'char' => 8207,
                            'i' => -1,
                            'level' => 1,
                            'otype' => 'R',
                            'pos' => 1,
                            'pdimatch' => -1,
                            'type' => 'R',
                            'x' => 0,
                        ],
                        [
                            'char' => 1632,
                            'i' => -1,
                            'level' => 2,
                            'otype' => 'AN',
                            'pos' => 2,
                            'pdimatch' => -1,
                            'type' => 'AN',
                            'x' => 0,
                        ],
                        [
                            'char' => 1776,
                            'i' => -1,
                            'level' => 2,
                            'otype' => 'EN',
                            'pos' => 3,
                            'pdimatch' => -1,
                            'type' => 'EN',
                            'x' => 0,
                        ],
                    ],
                ],
            ],
            [
                [
                    'e' => 1,
                    'edir' => 'R',
                    'end' => 3,
                    'eos' => 'R',
                    'length' => 4,
                    'maxlevel' => 0,
                    'sos' => 'R',
                    'start' => 0,
                    'item' => [
                        [
                            'char' => 65,
                            'i' => -1,
                            'level' => 1,
                            'otype' => 'L',
                            'pos' => 0,
                            'pdimatch' => -1,
                            'type' => 'L',
                            'x' => 0,
                        ],
                        [
                            'char' => 8207,
                            'i' => -1,
                            'level' => 1,
                            'otype' => 'R',
                            'pos' => 1,
                            'pdimatch' => -1,
                            'type' => 'R',
                            'x' => 0,
                        ],
                        [
                            'char' => 1632,
                            'i' => -1,
                            'level' => 1,
                            'otype' => 'AN',
                            'pos' => 2,
                            'pdimatch' => -1,
                            'type' => 'AN',
                            'x' => 0,
                        ],
                        [
                            'char' => 1776,
                            'i' => -1,
                            'level' => 1,
                            'otype' => 'EN',
                            'pos' => 3,
                            'pdimatch' => -1,
                            'type' => 'EN',
                            'x' => 0,
                        ],
                    ],
                ],
                [
                    'e' => 1,
                    'edir' => 'R',
                    'end' => 3,
                    'eos' => 'R',
                    'length' => 4,
                    'maxlevel' => 2,
                    'sos' => 'R',
                    'start' => 0,
                    'item' => [
                        [
                            'char' => 65,
                            'i' => -1,
                            'level' => 2,
                            'otype' => 'L',
                            'pos' => 0,
                            'pdimatch' => -1,
                            'type' => 'L',
                            'x' => 0,
                        ],
                        [
                            'char' => 8207,
                            'i' => -1,
                            'level' => 1,
                            'otype' => 'R',
                            'pos' => 1,
                            'pdimatch' => -1,
                            'type' => 'R',
                            'x' => 0,
                        ],
                        [
                            'char' => 1632,
                            'i' => -1,
                            'level' => 2,
                            'otype' => 'AN',
                            'pos' => 2,
                            'pdimatch' => -1,
                            'type' => 'AN',
                            'x' => 0,
                        ],
                        [
                            'char' => 1776,
                            'i' => -1,
                            'level' => 2,
                            'otype' => 'EN',
                            'pos' => 3,
                            'pdimatch' => -1,
                            'type' => 'EN',
                            'x' => 0,
                        ],
                    ],
                ],
            ],
        ];
    }
}
