<?php
/*
* File: ClientManagerTest.php
* Category: -
* Author: M.Goldenbaum
* Created: 28.12.22 18:11
* Updated: -
*
* Description:
*  -
*/

namespace Tests;

use PHPUnit\Framework\TestCase;
use Webklex\PHPIMAP\Client;
use Webklex\PHPIMAP\ClientManager;
use Webklex\PHPIMAP\Exceptions\MaskNotFoundException;
use Webklex\PHPIMAP\IMAP;

class ClientManagerTest extends TestCase {

    /** @var ClientManager $cm */
    protected ClientManager $cm;

    /**
     * Setup the test environment.
     *
     * @return void
     */
    public function setUp(): void {
        $this->cm = new ClientManager();
    }

    /**
     * Test if the config can be accessed
     *
     * @return void
     */
    public function testConfigAccessorAccount(): void {
        self::assertSame("default", ClientManager::get("default"));
        self::assertSame("d-M-Y", ClientManager::get("date_format"));
        self::assertSame(IMAP::FT_PEEK, ClientManager::get("options.fetch"));
        self::assertSame([], ClientManager::get("options.open"));
    }

    /**
     * Test creating a client instance
     *
     * @throws MaskNotFoundException
     */
    public function testMakeClient(): void {
        self::assertInstanceOf(Client::class, $this->cm->make([]));
    }

    /**
     * Test accessing accounts
     *
     * @throws MaskNotFoundException
     */
    public function testAccountAccessor(): void {
        self::assertSame("default", $this->cm->getDefaultAccount());
        self::assertNotEmpty($this->cm->account("default"));

        $this->cm->setDefaultAccount("foo");
        self::assertSame("foo", $this->cm->getDefaultAccount());
        $this->cm->setDefaultAccount("default");
    }

    /**
     * Test setting a config
     *
     * @throws MaskNotFoundException
     */
    public function testSetConfig(): void {
        $config = [
            "default" => "foo",
            "options" => [
                "fetch" => IMAP::ST_MSGN,
                "open"  => "foo"
            ]
        ];
        $cm = new ClientManager($config);

        self::assertSame("foo", $cm->getDefaultAccount());
        self::assertInstanceOf(Client::class, $cm->account("foo"));
        self::assertSame(IMAP::ST_MSGN, $cm->get("options.fetch"));
        self::assertSame(false, is_array($cm->get("options.open")));

    }
}