<?php

declare(strict_types=1);

/**
 * Arabic.php
 *
 * @since       2011-05-23
 * @category    Library
 * @package     UnicodeData
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-unicode-data
 *
 * This file is part of tc-lib-unicode-data software library.
 */

namespace Com\Tecnick\Unicode\Data;

/**
 * Com\Tecnick\Unicode\Data\Arabic
 *
 * @since       2011-05-23
 * @category    Library
 * @package     UnicodeData
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-unicode-data
 */
class Arabic
{
    /**
     * Unicode code for ARABIC QUESTION MARK (U+061F)
     */
    public const QUESTION_MARK = 1567;

    /**
     * Unicode code for ARABIC LETTER LAM (U+0644)
     */
    public const LAM = 1604;

    /**
     * Unicode code for ARABIC LETTER HEH (U+0647)
     */
    public const HEH = 1607;

    /**
     * Unicode code for ARABIC SHADDA (U+0651)
     */
    public const SHADDA = 1617;

    /**
     * Unicode code for ARABIC LIGATURE ALLAH ISOLATED FORM (U+FDF2)
     */
    public const LIGATURE_ALLAH_ISOLATED_FORM = 65_010;

    /**
     * Arabic shape substitutions: char code => ([isolated, final, initial, medial]).
     *
     * @var array<int, array<int>>
     */
    public const SUBSTITUTE = [
        1569 => [65_152],
        1570 => [65_153, 65_154, 65_153, 65_154],
        1571 => [65_155, 65_156, 65_155, 65_156],
        1572 => [65_157, 65_158],
        1573 => [65_159, 65_160, 65_159, 65_160],
        1574 => [65_161, 65_162, 65_163, 65_164],
        1575 => [65_165, 65_166, 65_165, 65_166],
        1576 => [65_167, 65_168, 65_169, 65_170],
        1577 => [65_171, 65_172],
        1578 => [65_173, 65_174, 65_175, 65_176],
        1579 => [65_177, 65_178, 65_179, 65_180],
        1580 => [65_181, 65_182, 65_183, 65_184],
        1581 => [65_185, 65_186, 65_187, 65_188],
        1582 => [65_189, 65_190, 65_191, 65_192],
        1583 => [65_193, 65_194, 65_193, 65_194],
        1584 => [65_195, 65_196, 65_195, 65_196],
        1585 => [65_197, 65_198, 65_197, 65_198],
        1586 => [65_199, 65_200, 65_199, 65_200],
        1587 => [65_201, 65_202, 65_203, 65_204],
        1588 => [65_205, 65_206, 65_207, 65_208],
        1589 => [65_209, 65_210, 65_211, 65_212],
        1590 => [65_213, 65_214, 65_215, 65_216],
        1591 => [65_217, 65_218, 65_219, 65_220],
        1592 => [65_221, 65_222, 65_223, 65_224],
        1593 => [65_225, 65_226, 65_227, 65_228],
        1594 => [65_229, 65_230, 65_231, 65_232],
        1601 => [65_233, 65_234, 65_235, 65_236],
        1602 => [65_237, 65_238, 65_239, 65_240],
        1603 => [65_241, 65_242, 65_243, 65_244],
        1604 => [65_245, 65_246, 65_247, 65_248],
        1605 => [65_249, 65_250, 65_251, 65_252],
        1606 => [65_253, 65_254, 65_255, 65_256],
        1607 => [65_257, 65_258, 65_259, 65_260],
        1608 => [65_261, 65_262, 65_261, 65_262],
        1609 => [65_263, 65_264, 64_488, 64_489],
        1610 => [65_265, 65_266, 65_267, 65_268],
        1649 => [64_336, 64_337],
        1655 => [64_477],
        1657 => [64_358, 64_359, 64_360, 64_361],
        1658 => [64_350, 64_351, 64_352, 64_353],
        1659 => [64_338, 64_339, 64_340, 64_341],
        1662 => [64_342, 64_343, 64_344, 64_345],
        1663 => [64_354, 64_355, 64_356, 64_357],
        1664 => [64_346, 64_347, 64_348, 64_349],
        1667 => [64_374, 64_375, 64_376, 64_377],
        1668 => [64_370, 64_371, 64_372, 64_373],
        1670 => [64_378, 64_379, 64_380, 64_381],
        1671 => [64_382, 64_383, 64_384, 64_385],
        1672 => [64_392, 64_393],
        1676 => [64_388, 64_389],
        1677 => [64_386, 64_387],
        1678 => [64_390, 64_391],
        1681 => [64_396, 64_397],
        1688 => [64_394, 64_395, 64_394, 64_395],
        1700 => [64_362, 64_363, 64_364, 64_365],
        1702 => [64_366, 64_367, 64_368, 64_369],
        1705 => [64_398, 64_399, 64_400, 64_401],
        1709 => [64_467, 64_468, 64_469, 64_470],
        1711 => [64_402, 64_403, 64_404, 64_405],
        1713 => [64_410, 64_411, 64_412, 64_413],
        1715 => [64_406, 64_407, 64_408, 64_409],
        1722 => [64_414, 64_415],
        1723 => [64_416, 64_417, 64_418, 64_419],
        1726 => [64_426, 64_427, 64_428, 64_429],
        1728 => [64_420, 64_421],
        1729 => [64_422, 64_423, 64_424, 64_425],
        1733 => [64_480, 64_481],
        1734 => [64_473, 64_474],
        1735 => [64_471, 64_472],
        1736 => [64_475, 64_476],
        1737 => [64_482, 64_483],
        1739 => [64_478, 64_479],
        1740 => [64_508, 64_509, 64_510, 64_511],
        1744 => [64_484, 64_485, 64_486, 64_487],
        1746 => [64_430, 64_431],
        1747 => [64_432, 64_433],
    ];

    /**
     * Arabic laa letter: (char code => [isolated, final, initial, medial]).
     *
     * @var array<int, array<int>>
     */
    public const LAA = [
        1570 => [65_269, 65_270, 65_269, 65_270], // ALEF (U+0627) with MADDAH ABOVE (U+0653)
        1571 => [65_271, 65_272, 65_271, 65_272], // ALEF (U+0627) with HAMZA ABOVE (U+0654)
        1573 => [65_273, 65_274, 65_273, 65_274], // ALEF (U+0627) with HAMZA BELOW (U+0655)
        1575 => [65_275, 65_276, 65_275, 65_276], // ALEF (U+0627)
    ];

    /**
     * Array of character substitutions for sequences of two diacritics symbols.
     * Putting the combining mark and character in the same glyph allows us
     * to avoid the two marks overlapping each other in an illegible manner.
     * second NSM char code => substitution char.
     *
     * @var array<int, int>
     */
    public const DIACRITIC = [
        1612 => 64_606, // Shadda + Dammatan
        1613 => 64_607, // Shadda + Kasratan
        1614 => 64_608, // Shadda + Fatha
        1615 => 64_609, // Shadda + Damma
        1616 => 64_610, // Shadda + Kasra
    ];

    /**
     * Array of Arabic end letters
     *
     * @var array<int>
     */
    public const END = [
        1569, // HAMZAH (U+621)
        1570, // ALEF (U+0627) with MADDAH ABOVE (U+0653)
        1571, // ALEF (U+0627) with HAMZA ABOVE (U+0654)
        1572, // WAW (U+0648) with HAMZA ABOVE (U+0654)
        1573, // ALEF (U+0627) with HAMZA BELOW (U+0655)
        1575, // ALEF (U+0627)
        1577, // TEH MARBUTA (U+0629)
        1583, // DAL (U+062F)
        1584, // THAL (U+0630)
        1585, // REH (U+0631)
        1586, // ZAIN (U+0632)
        1608, // WAW (U+0648)
        1688, // JEH (U+0698)
    ];
}
