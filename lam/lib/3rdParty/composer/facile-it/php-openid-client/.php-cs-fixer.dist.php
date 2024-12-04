<?php

/*
 * Additional rules or rules to override.
 * These rules will be added to default rules or will override them if the same key already exists.
 */
 
$additionalRules = [
    '@PHP74Migration' => true,
    '@PHP74Migration:risky' => true,
    'visibility_required' => true,
    'heredoc_indentation' => true,
    'heredoc_to_nowdoc' => true,
    'no_null_property_initialization' => true,
    'no_useless_else' => true,
    'no_useless_return' => true,
    'global_namespace_import' => [
        'import_classes' => true,
        'import_constants' => true,
        'import_functions' => true,
    ],
    'constant_case' => true,
    'declare_strict_types' => true,
    'indentation_type' => true,
    'no_superfluous_phpdoc_tags' => [
        'allow_mixed' => true,
    ],
    'phpdoc_line_span' => [
        'const' => 'single',
        'method' => 'multi',
        'property' => 'single',
    ],
    'phpdoc_trim_consecutive_blank_line_separation' => true,
    // risky rules
    'fopen_flag_order' => true,
    'fopen_flags' => true,
    'ereg_to_preg' => true,
    'implode_call' => true,
    'no_unset_on_property' => true,
    // custom
    'comment_to_phpdoc' => true,
    'phpdoc_to_comment' => false,
];
$rulesProvider = new Facile\CodingStandards\Rules\CompositeRulesProvider([
    new Facile\CodingStandards\Rules\DefaultRulesProvider(),
    new Facile\CodingStandards\Rules\ArrayRulesProvider($additionalRules),
]);

$config = new PhpCsFixer\Config();
$config->setRules($rulesProvider->getRules());

$finder = new PhpCsFixer\Finder();

/*
 * You can set manually these paths:
 */
$autoloadPathProvider = new Facile\CodingStandards\AutoloadPathProvider();
$finder->in($autoloadPathProvider->getPaths());

$config->setFinder($finder);

return $config;
