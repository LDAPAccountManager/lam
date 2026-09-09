<?php

declare(strict_types=1);

/**
 * SpotRegistryInterface.php
 *
 * @since     2026-08-26
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
 * Com\Tecnick\Color\SpotRegistryInterface
 *
 * Registers spot colors and writes them as PDF Separation objects.
 *
 * Implemented by Spot.
 *
 * @since     2026-08-26
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-color
 */
interface SpotRegistryInterface
{
    /**
     * Add a new spot color or overwrite an existing one with the same name.
     *
     * @param string $name Full name of the spot color.
     * @param Cmyk   $cmyk CMYK color object
     *
     * @return string Spot color key.
     *
     * @throws ColorException if the name is unusable or the color has already been emitted
     */
    public function addSpotColor(string $name, Cmyk $cmyk): string;

    /**
     * Add a new Lab-based spot color or overwrite an existing one with the same name.
     *
     * @param string             $name         Full name of the spot color.
     * @param float              $lstar        Lab L* component in [0..100].
     * @param float              $astar        Lab a* component.
     * @param float              $bstar        Lab b* component.
     * @param array<int, float> ...$labOptions Optional Lab settings: whitepoint, blackpoint, range, col0.
     *
     * @return string Spot color key.
     *
     * @throws ColorException if the name is unusable or the color has already been emitted
     */
    public function addSpotLabColor(
        string $name,
        float $lstar,
        float $astar,
        float $bstar,
        array ...$labOptions,
    ): string;

    /**
     * Return a copy of the requested spot color CMYK object.
     *
     * @param string $name Full name of the spot color.
     *
     * @throws ColorException if the color is not found
     */
    public function getSpotColorObj(string $name): Cmyk;

    /**
     * Return a copy of the requested spot color Lab object.
     *
     * @param string $name Full name of the spot color.
     *
     * @throws ColorException if the color is not found
     */
    public function getSpotLabColorObj(string $name): Lab;

    /**
     * Returns the PDF command to output Spot color objects.
     *
     * @param int $pon Current PDF object number
     *
     * @return string PDF command
     */
    public function getPdfSpotObjects(int &$pon): string;

    /**
     * Returns the PDF command to output Spot color resources.
     *
     * @return string PDF command
     *
     * @throws ColorException if a registered spot color has not been emitted
     */
    public function getPdfSpotResources(): string;

    /**
     * Returns the PDF command to output the Spot color resources for the given keys.
     *
     * @param array<string> $keys Array of spot color keys.
     *
     * @return string PDF command
     *
     * @throws ColorException if a key is not registered or has not been emitted
     */
    public function getPdfSpotResourcesByKeys(array $keys): string;
}
