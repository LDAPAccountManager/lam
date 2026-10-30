<?php

namespace LAM\MCP;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2026  Roland Gruber

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

use LAM\WHITE_PAGES\WhitePagesPersistence;
use LAM\WHITE_PAGES\WP_AUTHENTICATION;
use LAMException;
use stdClass;

include_once __DIR__ . "/security.inc";
if (isLAMProVersion()) {
	include_once __DIR__ . "/whitePages.inc";
}

const MCP_VERSION = '2026-07-28';


/**
 * Checks if the request format is valid.
 */
function validateRequestFormat(): void {
	if (($_SERVER['REQUEST_METHOD'] ?? '') !== 'POST') {
		header('Allow: POST');
		http_response_code(405);
		exit;
	}
}

/**
 * Checks if the JSON data is valid.
 *
 * @param array<string, string|array<string, string[]|array<string, string[]>>> $jsonData data
 */
function validateJsonData(array $jsonData): void {
	logNewMessage(LOG_DEBUG, print_r($jsonData, true));
	$id = $jsonData['id'] ?? '-';
	if (empty($jsonData['jsonrpc']) || ($jsonData['jsonrpc'] !== '2.0')) {
		badMcpRequest($id, -1, 'Invalid JSON-RPC version');
	}
	if (empty($jsonData['id'])) {
		badMcpRequest($id, -2, 'No id provided');
	}
	if (empty($jsonData['method'])) {
		badMcpRequest($id, -3, 'No method provided');
	}
	if (empty($jsonData['params']['_meta']['io.modelcontextprotocol/protocolVersion'])
		|| ($jsonData['params']['_meta']['io.modelcontextprotocol/protocolVersion'] !== MCP_VERSION)) {
		badMcpRequest($id, -4, 'Unsupported protocol version');
	}
}

/**
 * Sends an error to the client and ends processing.
 *
 * @param string|int $id request id
 * @param int $code error code
 * @param string $message error message
 */
function badMcpRequest(int|string $id, int $code, string $message): void {
	logNewMessage(LOG_WARNING, $message);
	http_response_code(400);
	sendDefaultHeaders();
	$data = getResponseTemplate($id);
	$data['error'] = [
		'code' => $code,
		'message' => $message,
	];
	echo json_encode($data);
	exit;
}

/**
 * Sends default JSON headers.
 */
function sendDefaultHeaders(): void {
	header('Content-Type: application/json; charset=utf-8');
	header('Cache-Control: no-store');
}

/**
 * Returns the basic response data structure.
 *
 * @param string|int $id request id
 */
function getResponseTemplate(string|int $id) {
	return [
		'jsonrpc' => '2.0',
		'id' => $id,
	];
}

/**
 * Routes the request to the right handler.
 *
 * @param array<string, string|array<string, string[]|array<string, string[]>>> $jsonData data
 */
function routeRequest(array $jsonData): void {
	$id = $jsonData['id'];
	switch ($jsonData['method']) {
		case 'server/discover':
			answerDiscovery($id);
			break;
		case 'tools/list':
			answerToolList($id);
			break;
		case 'tools/call':
			answerToolCall($id, $jsonData['params']);
			break;
		default:
			badMcpRequest($id, -10, 'Invalid request');
	}
}

/**
 * Answers to the discovery call.
 *
 * @param string|int $id request id
 */
function answerDiscovery(string|int $id): void {
	sendDefaultHeaders();
	$data = getResponseTemplate($id);
	$data['result'] = [
		'resultType' => 'complete',
		'supportedVersions' => [MCP_VERSION],
		'capabilities' => [
			'tools' => new stdClass(),
		],
		'_meta' => [
			'io.modelcontextprotocol/serverInfo' => [
				'name' => 'LDAP Account Manager',
				'version' => LAMVersion(),
			],
		],
		'instructions' => 'This server exposes functionality of LDAP Account Manager. ' .
			'White pages tools are used to provide e.g. contact details about employees but can also be used to expose other data.',
		'ttlMs' => 60000,
		'cacheScope' => 'public',
	];
	echo json_encode($data);
}

/**
 * Answers to the tool listing call.
 *
 * @param string|int $id request id
 */
function answerToolList(string|int $id): void {
	sendDefaultHeaders();
	$data = getResponseTemplate($id);
	$data['result'] = [
		'resultType' => 'complete',
		'tools' => getToolList(),
		'ttlMs' => 300000,
		'cacheScope' => 'public',
	];
	echo json_encode($data);
}

/**
 * Returns a list of available tools.
 *
 * @return array<string, string|array<string, string|string[]|array<string, string>>> tools
 */
function getToolList(): array {
	$data = [];
	if (isLAMProVersion()) {
		$data += [
			[
				'name' => 'listWhitePagesProfiles',
				'title' => 'List all available white pages profiles.',
				'description' => 'White pages profiles define how LDAP data is exposed and searchable.',
				'inputSchema' => [
					"type" => "object",
				],
				'outputSchema' => [
					'type' => 'object',
					'properties' => [
						'profiles' => [
							'type' => 'array',
							'description' => 'Profile names'
						]
					],
					'required' => ['profiles']
				],
			],
			[
				'name' => 'listWhitePagesProfileSections',
				'title' => 'List all available sections within a white pages profile.',
				'description' => 'White pages profile sections define which kind of LDAP data is exposed (e.g. users or groups).',
				'inputSchema' => [
					"type" => "object",
					"properties" => [
						'profileName' => [
							'type' => 'string',
						]
					]
				],
				'outputSchema' => [
					'type' => 'object',
					'properties' => [
						'sections' => [
							'type' => 'array',
							'items' => [
								'type' => 'object',
								'properties' => [
									'sectionIndex' => [
										'type' => 'integer',
										'description' => 'Section index id'
									],
									'label' => [
										'type' => 'string',
										'description' => 'Section label'
									]
								]
							]
						]
					],
					'required' => ['sections']
				],
			],
		];
	}
	return $data;
}

/**
 * Answers to a tool call.
 *
 * @param string|int $id request id
 * @param array<string, string|array<string, string>> $params parameters
 */
function answerToolCall(string|int $id, array $params): void {
	sendDefaultHeaders();
	$data = getResponseTemplate($id);
	$command = $params['name'] ?? null;
	if ($command === null) {
		badMcpRequest($id, -11, 'Missing parameter "name"');
	}
	$arguments = $params['arguments'] ?? [];
	if (isLAMProVersion()) {
		switch ($command) {
			case 'listWhitePagesProfiles':
				listWhitePagesProfiles($id);
				return;
			case 'listWhitePagesProfileSections':
				listWhitePagesProfileSections($id, $params);
				return;
		}
	}
	badMcpRequest($id, -12, 'Unknown tool ' . $command);
}

/**
 * Lists the white pages profiles.
 *
 * @param int|string $id request ID
 */
function listWhitePagesProfiles(int|string $id): void {
	$wpPersistence = new WhitePagesPersistence();
	sendDefaultHeaders();
	$data = getResponseTemplate($id);
	$allProfiles = $wpPersistence->getProfiles();
	$profiles = [];
	foreach ($allProfiles as $profileName) {
		$profile = $wpPersistence->load($profileName);
		if ($profile->authenticationType === WP_AUTHENTICATION::AUTH_ANONYMOUS->name) {
			$profiles[] = $profileName;
		}
	}
	$data['result'] = [
		'resultType' => 'complete',
		'content' => [
			[
				'type' => 'text',
				'text' => json_encode($profiles)
			]
		],
		'structuredContent' => [
			'profiles' => $profiles
		],
		'ttlMs' => 60000,
		'cacheScope' => 'public',
	];
	echo json_encode($data);
}

/**
 * Lists the sections of a given white pages profile.
 *
 * @param int|string $id request ID
 * @param array<string, string|array<string, string>> $params parameters
 */
function listWhitePagesProfileSections(int|string $id, array $params): void {
	$profileName = $params['arguments']['profileName'] ?? null;
	if ($profileName === null) {
		badMcpRequest($id, -20, 'Missing parameter "profileName"');
	}
	$wpPersistence = new WhitePagesPersistence();
	try {
		$profile = $wpPersistence->load($profileName);
		sendDefaultHeaders();
		$sections = [];
		foreach ($profile->tabs as $index => $tab) {
			$sections[] = [
				'sectionIndex' => $index,
				'label' => $tab->label,
			];
		}
		$data = getResponseTemplate($id);
		$data['result'] = [
			'resultType' => 'complete',
			'content' => [
				[
					'type' => 'text',
					'text' => json_encode($sections)
				]
			],
			'structuredContent' => [
				'sections' => $sections
			],
			'ttlMs' => 60000,
			'cacheScope' => 'public',
		];
		echo json_encode($data);
	}
	catch (LAMException) {
		badMcpRequest($id, -21, 'Invalid profile name');
	}
}
