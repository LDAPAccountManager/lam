<?php
/**
 * Object representation of a basic list header (RFC 2369) element.
 *
 * Copyright 2012-2017 Horde LLC (http://www.horde.org/)
 *
 * See the enclosed file COPYING for license information (LGPL). If you
 * did not receive this file, see http://www.horde.org/licenses/lgpl21.
 *
 * @author    Michael Slusarz <slusarz@horde.org>
 * @category  Horde
 * @license   http://www.horde.org/licenses/lgpl21 LGPL 2.1
 * @package   ListHeaders
 */

/**
 * Object representation of a basic list header (RFC 2369) element.
 *
 * @author    Michael Slusarz <slusarz@horde.org>
 * @category  Horde
 * @license   http://www.horde.org/licenses/lgpl21 LGPL 2.1
 * @package   ListHeaders
 *
 * @property string $comments  Comments.
 * @property string $url  URL.
 */
class Horde_ListHeaders_NoPost extends Horde_ListHeaders_Base
{
    /**
     * Constructor.
     *
     * @param array $comments  Comments.
     */
    public function __construct(array $comments = array())
    {
        $this->_comments = $comments;
    }

    /**
     */
    public function __get($name)
    {
        switch ($name) {
        case 'comments':
            return $this->_comments;

        case 'url':
            return null;
        }
    }

}
