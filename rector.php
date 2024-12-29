<?php

use Rector\CodeQuality\Rector\Class_\CompleteDynamicPropertiesRector;
use Rector\CodeQuality\Rector\ClassMethod\ExplicitReturnNullRector;
use Rector\CodeQuality\Rector\ClassMethod\InlineArrayReturnAssignRector;
use Rector\CodeQuality\Rector\Empty_\SimplifyEmptyCheckOnEmptyArrayRector;
use Rector\CodeQuality\Rector\For_\ForRepeatedCountToOwnVariableRector;
use Rector\CodeQuality\Rector\Identical\FlipTypeControlToUseExclusiveTypeRector;
use Rector\Config\RectorConfig;
use Rector\DeadCode\Rector\ClassMethod\RemoveUnusedPrivateMethodRector;
use Rector\DeadCode\Rector\StaticCall\RemoveParentCallWithoutParentRector;
use Rector\Php70\Rector\StaticCall\StaticCallOnNonStaticToInstanceCallRector;
use Rector\Php73\Rector\FuncCall\StringifyStrNeedlesRector;
use Rector\Php74\Rector\Property\RestoreDefaultNullToNullableTypePropertyRector;
use Rector\Php80\Rector\Class_\ClassPropertyAssignToConstructorPromotionRector;
use Rector\Php81\Rector\Property\ReadOnlyPropertyRector;
use Rector\Set\ValueObject\LevelSetList;
use Rector\Set\ValueObject\SetList;
use Rector\Php81\Rector\FuncCall\NullToStrictStringFuncCallArgRector;
use Rector\Strict\Rector\Empty_\DisallowedEmptyRuleFixerRector;

return RectorConfig::configure()
	->withParallel(240, 8)
	->withPaths([
		__DIR__ . '/lam/help',
		__DIR__ . '/lam/lib',
		__DIR__ . '/lam/templates',
		__DIR__ . '/lam/tests',
	])
	->withSets([
		SetList::DEAD_CODE,
		LevelSetList::UP_TO_PHP_81,
		SetList::CODE_QUALITY,
	])
	->withSkip([
		__DIR__ . '/lam/lib/3rdParty',
		NullToStrictStringFuncCallArgRector::class,
		ReadOnlyPropertyRector::class,
		ClassPropertyAssignToConstructorPromotionRector::class,
		StaticCallOnNonStaticToInstanceCallRector::class,
		StringifyStrNeedlesRector::class,
		RestoreDefaultNullToNullableTypePropertyRector::class,
		ForRepeatedCountToOwnVariableRector::class,
		SimplifyEmptyCheckOnEmptyArrayRector::class,
		DisallowedEmptyRuleFixerRector::class,
		FlipTypeControlToUseExclusiveTypeRector::class,
		InlineArrayReturnAssignRector::class,
		// TODO private method check is unreliable, recheck with newer rector version
		RemoveUnusedPrivateMethodRector::class,
		// TODO parent call check is unreliable, recheck with newer rector version
		RemoveParentCallWithoutParentRector::class,
		// TODO inherited properties check is unreliable, recheck with newer rector version
		CompleteDynamicPropertiesRector::class,
		// TODO unreliable, recheck with newer rector version
		ExplicitReturnNullRector::class
	])
	->withFileExtensions([
		'php',
		'inc'
	]);
