<?php
/**
 * @package Exception
 *
 * Copyright 2010-2016 Horde LLC (http://www.horde.org/)
 *
 * See the enclosed file COPYING for license information (LGPL). If you
 * did not receive this file, see http://www.horde.org/licenses/lgpl21.
 */

/**
 * Horde_Exception_Translation is the translation wrapper class for Horde_Exception.
 *
 * @author  Jan Schneider <jan@horde.org>
 * @package Exception
 */
class Horde_Exception_Translation extends Horde_Translation_Autodetect
{
    /**
     * The translation domain
     *
     * @var string
     */
    protected static $_domain = 'Horde_Exception';

    /**
     * The absolute PEAR path to the translations for the default gettext handler.
     *
     * @var string
     */
    protected static $_pearDirectory = '/daten/dev/lam/lam/lib/3rdParty/composer/pear-pear.horde.org/Horde_Exception/data';
}
