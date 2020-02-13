<?php
/**
 * Copyright 2013-2017 Horde LLC (http://www.horde.org/)
 *
 * See the enclosed file COPYING for license information (LGPL). If you
 * did not receive this file, see http://www.horde.org/licenses/lgpl21.
 *
 * @category  Horde
 * @copyright 2013-2017 Horde LLC
 * @license   http://www.horde.org/licenses/lgpl21 LGPL 2.1
 * @package   Imap_Client
 */

/**
 * SQL schema for the Db IMAP/POP cache driver.
 *
 * @author    Michael Slusarz <slusarz@horde.org>
 * @category  Horde
 * @copyright 2013-2017 Horde LLC
 * @ignore
 * @license   http://www.horde.org/licenses/lgpl21 LGPL 2.1
 * @package   Imap_Client
 */
class HordeImapClientBaseTables extends Horde_Db_Migration_Base
{
    public function up()
    {
        if (in_array('horde_imap_client_data', $this->tables())) {
            return;
        }

        $t = $this->createTable('horde_imap_client_data', array(
            'autoincrementKey' => 'messageid'
        ));
        $t->column('hostspec', 'string', array(
            'limit' => 255,
            'null' => false
        ));
        $t->column('mailbox', 'string', array(
            'limit' => 255,
            'null' => false
        ));
        $t->column('modified', 'bigint');
        $t->column('port', 'integer', array(
            'null' => false
        ));
        $t->column('username', 'string', array(
            'limit' => 255,
            'null' => false
        ));
        $t->end();

        $this->addIndex(
            'horde_imap_client_data',
            array('hostspec', 'mailbox', 'port', 'username')
        );

        $t = $this->createTable('horde_imap_client_message', array(
            'autoincrementKey' => false
        ));
        $t->column('data', 'binary');
        $t->column('msguid', 'string', array(
            'null' => false
        ));
        $t->column('messageid', 'bigint', array(
            'null' => false
        ));
        $t->end();

        $this->addIndex(
            'horde_imap_client_message',
            array('msguid', 'messageid')
        );

        $t = $this->createTable('horde_imap_client_metadata', array(
            'autoincrementKey' => false
        ));
        $t->column('data', 'binary');
        $t->column('field', 'string', array(
            'null' => false
        ));
        $t->column('messageid', 'bigint', array(
            'null' => false
        ));
        $t->end();

        $this->addIndex(
            'horde_imap_client_metadata',
            array('messageid')
        );
    }

    public function down()
    {
        $this->dropTable('horde_imap_client_data');
        $this->dropTable('horde_imap_client_message');
        $this->dropTable('horde_imap_client_metadata');
    }
}
