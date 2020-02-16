<?php
/**
 * Copyright 2010-2017 Horde LLC (http://www.horde.org/)
 *
 * See the enclosed file COPYING for license information (LGPL). If you
 * did not receive this file, see http://www.horde.org/licenses/lgpl21.
 *
 * @category  Horde
 * @copyright 2010-2017 Horde LLC
 * @license   http://www.horde.org/licenses/lgpl21 LGPL 2.1
 * @package   ListHeaders
 */

/**
 * Horde_ListHeaders_Translation is the translation wrapper class for
 * Horde_ListHeaders.
 *
 * @author    Jan Schneider <jan@horde.org>
 * @category  Horde
 * @copyright 2010-2017 Horde LLC
 * @license   http://www.horde.org/licenses/lgpl21 LGPL 2.1
 * @package   ListHeaders
 */
class Horde_ListHeaders_Translation extends Horde_Translation_Autodetect
{
    /**
     * The translation domain
     *
     * @var string
     */
    protected static $_domain = 'Horde_ListHeaders';

    /**
     * The absolute PEAR path to the translations for the default gettext handler.
     *
     * @var string
     */
    protected static $_pearDirectory = '/daten/dev/lam/lam/lib/3rdParty/composer/pear-pear.horde.org/Horde_ListHeaders/data';
}
