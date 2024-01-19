<?php

use Rector\Config\RectorConfig;
use Rector\Set\ValueObject\LevelSetList;
use Rector\Set\ValueObject\SetList;

return static function (RectorConfig $rectorConfig): void {
    $rectorConfig->paths([
		__DIR__ . '/lam/help',
        __DIR__ . '/lam/lib',
		__DIR__ . '/lam/templates',
		__DIR__ . '/lam/tests',
    ]);

    //$rectorConfig->fileExtensions(['php', 'inc']);

    $rectorConfig->skip([
        __DIR__ . '/lam/lib/3rdParty',
    ]);

    $rectorConfig->sets([
        SetList::DEAD_CODE,
		LevelSetList::UP_TO_PHP_80
    ]);
};
