<?php

use Rector\CodeQuality\Rector\ClassMethod\ExplicitReturnNullRector;
use Rector\CodeQuality\Rector\ClassMethod\InlineArrayReturnAssignRector;
use Rector\CodeQuality\Rector\Empty_\SimplifyEmptyCheckOnEmptyArrayRector;
use Rector\CodeQuality\Rector\For_\ForRepeatedCountToOwnVariableRector;
use Rector\CodeQuality\Rector\Identical\FlipTypeControlToUseExclusiveTypeRector;
use Rector\Config\RectorConfig;
use Rector\DeadCode\Rector\If_\RemoveAlwaysTrueIfConditionRector;
use Rector\DeadCode\Rector\StaticCall\RemoveParentCallWithoutParentRector;
use Rector\Php73\Rector\FuncCall\StringifyStrNeedlesRector;
use Rector\Php74\Rector\Property\RestoreDefaultNullToNullableTypePropertyRector;
use Rector\Php80\Rector\Class_\ClassPropertyAssignToConstructorPromotionRector;
use Rector\Php81\Rector\Property\ReadOnlyPropertyRector;
use Rector\Set\ValueObject\LevelSetList;
use Rector\Set\ValueObject\SetList;
use Rector\TypeDeclaration\Rector\StmtsAwareInterface\SafeDeclareStrictTypesRector;

return RectorConfig::configure()
	->withParallel(240, 4)
	->withPaths([
		__DIR__ . '/lam/help',
		__DIR__ . '/lam/lib',
		__DIR__ . '/lam/templates',
		__DIR__ . '/lam/tests',
	])
	->withSets([
		SetList::DEAD_CODE,
		LevelSetList::UP_TO_PHP_82,
		SetList::CODE_QUALITY,
	])
	->withSkip([
		__DIR__ . '/lam/lib/3rdParty',
		ReadOnlyPropertyRector::class,
		ClassPropertyAssignToConstructorPromotionRector::class,
		StringifyStrNeedlesRector::class,
		RestoreDefaultNullToNullableTypePropertyRector::class,
		ForRepeatedCountToOwnVariableRector::class,
		SimplifyEmptyCheckOnEmptyArrayRector::class,
		FlipTypeControlToUseExclusiveTypeRector::class,
		InlineArrayReturnAssignRector::class,
		SafeDeclareStrictTypesRector::class,
		// TODO unreliable, recheck with newer rector version
		ExplicitReturnNullRector::class,
		RemoveParentCallWithoutParentRector::class,
		RemoveAlwaysTrueIfConditionRector::class,
	])
	->withFileExtensions([
		'php',
		'inc'
	]);
