<?php
/*
* File: ImapProtocolTest.php
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
use Webklex\PHPIMAP\Connection\Protocols\ImapProtocol;
use Webklex\PHPIMAP\Exceptions\ConnectionFailedException;

class ImapProtocolTest extends TestCase {


    /**
     * ImapProtocol test
     *
     * @return void
     */
    public function testImapProtocol(): void {

        $protocol = new ImapProtocol(false);
        self::assertSame(false, $protocol->getCertValidation());
        self::assertSame("", $protocol->getEncryption());

        $protocol->setCertValidation(true);
        $protocol->setEncryption("ssl");

        self::assertSame(true, $protocol->getCertValidation());
        self::assertSame("ssl", $protocol->getEncryption());
    }
}