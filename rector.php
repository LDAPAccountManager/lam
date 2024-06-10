<?php

use Rector\Config\RectorConfig;
use Rector\Set\ValueObject\LevelSetList;
use Rector\Set\ValueObject\SetList;
use Rector\Php81\Rector\FuncCall\NullToStrictStringFuncCallArgRector;

return RectorConfig::configure()
	->withPaths([
		__DIR__ . '/lam/help',
		__DIR__ . '/lam/lib',
		__DIR__ . '/lam/templates',
		__DIR__ . '/lam/tests',
	])
	->withSets([
		SetList::DEAD_CODE,
		LevelSetList::UP_TO_PHP_81
	])
	->withSkip([
		__DIR__ . '/lam/lib/3rdParty',
		NullToStrictStringFuncCallArgRector::class
	])
	->withFileExtensions([
		'php',
//		'inc'
	]);
