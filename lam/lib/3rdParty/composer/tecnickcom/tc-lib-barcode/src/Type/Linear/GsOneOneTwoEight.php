<?php

declare(strict_types=1);

/**
 * GsOneOneTwoEight.php
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Linear;

use Com\Tecnick\Barcode\Exception as BarcodeException;
use Com\Tecnick\Barcode\Type\GsOneElementString;

/**
 * Com\Tecnick\Barcode\Type\Linear\GsOneOneTwoEight;
 *
 * GsOneOneTwoEight Barcode type class
 * GS1-128 (GS1 General Specifications section 5.4)
 *
 * CODE 128 symbol whose start character is immediately followed by FNC1 and
 * whose data is a concatenation of GS1 Application Identifier element strings.
 * The input is the bracketed form "(ai)value(ai)value...", which is also the
 * human readable interpretation returned by getExtendedCode(). Parentheses are
 * reserved as Application Identifier delimiters and cannot appear in a value.
 * Element strings whose Application Identifier is not listed in PREDEFINED are
 * of variable length and are followed by an FNC1 separator unless they are last.
 *
 * GS1 and GS1-128 are registered trademarks of GS1 AISBL.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class GsOneOneTwoEight extends \Com\Tecnick\Barcode\Type\Linear\CodeOneTwoEight
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'GS1128';

    /**
     * Function 1 Symbol Character, as accepted by the CODE 128 encoder
     *
     * @var string
     */
    protected const FNC1 = "\xF1";

    /**
     * Maximum number of data characters of a single symbol
     */
    protected const MAX_DATA_CHARS = 48;

    /**
     * Character sequence handed to the CODE 128 encoder
     */
    protected string $payload = '';

    /**
     * Get the bracketed source of the element strings.
     * The profiles of a single Application Identifier override this.
     */
    protected function getBracketedCode(): string
    {
        return $this->code;
    }

    protected function getEncodedPayload(): string
    {
        return $this->payload;
    }

    /**
     * Calculate the GS1 modulo 10 check digit of a numeric string.
     *
     * @param string $code Data digits without the check digit.
     */
    protected function getCheckDigit(string $code): int
    {
        return (new GsOneElementString())->getCheckDigit($code);
    }

    /**
     * Build the encoded payload and the human readable interpretation.
     *
     * @throws BarcodeException if the element strings cannot be encoded
     */
    protected function formatCode(): void
    {
        $parser = new GsOneElementString();
        $elements = $parser->parse($this->getBracketedCode());
        $data = $parser->getData($elements, $this::FNC1);

        if (\strlen($data) > $this::MAX_DATA_CHARS) {
            throw new BarcodeException(
                'The data is too long: '
                . \strlen($data)
                . ' characters (maximum '
                . $this::MAX_DATA_CHARS
                . ' for '
                . $this::FORMAT
                . ')',
            );
        }

        $this->extcode = $parser->getHumanReadable($elements);
        $this->payload = $this::FNC1 . $data;
    }

    /**
     * Set the bars array.
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        $this->formatCode();
        parent::setBars();
    }
}
