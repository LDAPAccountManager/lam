<?php
/**
 * Object representation of a List-ID (RFC 2919) element.
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
 * Object representation of a List-ID (RFC 2919) element.
 *
 * @author    Michael Slusarz <slusarz@horde.org>
 * @category  Horde
 * @license   http://www.horde.org/licenses/lgpl21 LGPL 2.1
 * @package   ListHeaders
 *
 * @property string $id  List ID.
 * @property string $label  List label.
 */
class Horde_ListHeaders_Id extends Horde_ListHeaders_Object
{
    /**
     * List ID.
     *
     * @var string
     */
    protected $_id;

    /**
     * List label.
     *
     * @var string.
     */
    protected $_label = null;

    /**
     * Constructor.
     *
     * @param string $id     List ID.
     * @param string $label  List label.
     */
    public function __construct($id, $label = null)
    {
        // RFC 2919 [2]: Limited to 255 characters.
        $this->_id = substr($id, 0, 255);
        if (strlen($label)) {
            // MIME encoding NOT allowed, so it is ignored.
            $this->_label = $label;
        }
    }

    /**
     */
    public function __get($name)
    {
        switch ($name) {
        case 'id':
            return $this->_id;

        case 'label':
            return $this->_label;
        }
    }

}
