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

include_once __DIR__ . "/security.inc";

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
		'_meta' => [
			'io.modelcontextprotocol/serverInfo' => [
				'name' => 'LAM',
				'version' => LAMVersion()
			]
		]
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
			'tools' => getToolList(),
		],
		'_meta' => [
			'io.modelcontextprotocol/serverInfo' => [
				'name' => 'LAM',
				'version' => LAMVersion(),
			],
		],
		'instructions' => 'This server exposes functionality of LDAP Account Manager.',
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
				'title' => 'List all available white pages profiles',
				'description' => 'White pages profiles define how LDAP data is exposed and searchable.',
				'inputSchema' => [],
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
			]
		];
	}
	return $data;
}