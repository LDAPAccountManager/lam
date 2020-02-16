<?php
/**
 * Copyright 2014-2017 Horde LLC (http://www.horde.org/)
 *
 * See the enclosed file COPYING for license information (LGPL). If you
 * did not receive this file, see http://www.horde.org/licenses/lgpl21.
 *
 * @category  Horde
 * @copyright 2014-2017 Horde LLC
 * @license   http://www.horde.org/licenses/lgpl21 LGPL 2.1
 * @package   Imap_Client
 */

/**
 * Renames columns from an older version of the migration step 1.
 *
 * @author    Jan Schneider <jan@horde.org>
 * @category  Horde
 * @copyright 2014-2017 Horde LLC
 * @ignore
 * @license   http://www.horde.org/licenses/lgpl21 LGPL 2.1
 * @package   Imap_Client
 */
class HordeImapClientChangeColumnName extends Horde_Db_Migration_Base
{
    public function up()
    {
        if (array_key_exists('uid', $this->columns('horde_imap_client_data'))) {
            $this->renameColumn('horde_imap_client_data', 'uid', 'messageid');
        }
        if (array_key_exists('uid', $this->columns('horde_imap_client_message'))) {
            $this->renameColumn('horde_imap_client_message', 'uid', 'messageid');
        }
        if (array_key_exists('uid', $this->columns('horde_imap_client_metadata'))) {
            $this->renameColumn('horde_imap_client_metadata', 'uid', 'messageid');
        }
    }

    public function down()
    {
    }
}
