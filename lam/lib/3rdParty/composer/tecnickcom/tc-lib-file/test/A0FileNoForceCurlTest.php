<?php

declare(strict_types=1);

namespace Test;

class A0FileNoForceCurlTest extends TestUtil
{
    /**
     * Covers the early getUrlData() guard when allow_url_fopen is enabled and
     * FORCE_CURL is not defined yet.
     *
     * @throws \Com\Tecnick\File\Exception
     */
    public function testGetUrlDataReturnsFalseWhenAllowUrlFopenEnabledWithoutForceCurl(): void
    {
        if (\defined('FORCE_CURL')) {
            $this->markTestSkipped('FORCE_CURL already defined by another test class');
        }

        if (\ini_get('allow_url_fopen') === false || \ini_get('allow_url_fopen') === '0') {
            $this->markTestSkipped('allow_url_fopen is disabled in this environment');
        }

        $file = new \Com\Tecnick\File\File(['example.com']);
        $this->assertFalse($file->getUrlData('http://example.com/path.txt'));
    }
}
