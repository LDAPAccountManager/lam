<?php
namespace LAM\TOOLS\MULTI_EDIT;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2023  Roland Gruber

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

/**
 * Multi edit tool functions that allows LDAP operations on multiple entries.
 *
 * @author Roland Gruber
 * @package tools
 */

/**
 * Extracts any wildcards from the value field.
 *
 * @param string $value value field
 * @return array wildcard attributes
 */
function extractWildcards(string $value): array {
	$value = strtolower($value);
	$attributes = array();
	// ((attr)) -> " "
	$matches = array();
	if (preg_match_all('/\\(\\(([a-z0-9-_]+)\\)\\)/i', $value, $matches) > 0) {
		foreach ($matches[1] as $match) {
			$attributes[] = strtolower($match);
		}
	}
	// %attr% -> value
	$matches = array();
	if (preg_match_all('/%([a-z0-9-_]+)%/i', $value, $matches) > 0) {
		foreach ($matches[1] as $match) {
			$attributes[] = strtolower($match);
		}
	}
	// @attr@ -> v
	$matches = array();
	if (preg_match_all('/@([a-z0-9-_]+)@/i', $value, $matches) > 0) {
		foreach ($matches[1] as $match) {
			$attributes[] = strtolower($match);
		}
	}
	// ??attr?? -> value
	$matches = array();
	if (preg_match_all('/\\?\\?([a-z0-9-_]+)\\?\\?/i', $value, $matches) > 0) {
		foreach ($matches[1] as $match) {
			$attributes[] = strtolower($match);
		}
	}
	// !!attr!! -> VALUE
	$matches = array();
	if (preg_match_all('/!!([a-z0-9-_]+)!!/i', $value, $matches) > 0) {
		foreach ($matches[1] as $match) {
			$attributes[] = strtolower($match);
		}
	}
	// ?attr? -> v
	$matches = array();
	if (preg_match_all('/\\?([a-z0-9-_]+)\\?/i', $value, $matches) > 0) {
		foreach ($matches[1] as $match) {
			$attributes[] = strtolower($match);
		}
	}
	// !attr! -> V
	$matches = array();
	if (preg_match_all('/!([a-z0-9-_]+)!/i', $value, $matches) > 0) {
		foreach ($matches[1] as $match) {
			$attributes[] = strtolower($match);
		}
	}
	// §attr|;§ -> "val1;val2"
	$matches = array();
	if (preg_match_all('/§([a-z0-9-_]+)\\|([^§]+)§/i', $value, $matches) > 0) {
		foreach ($matches[1] as $match) {
			$attributes[] = strtolower($match);
		}
	}
	return array_values(array_unique($attributes));
}

/**
 * Replaces wildcards in the user entered value.
 *
 * @param string $value user entered value
 * @param array $entry LDAP entry
 * @return string result
 */
function replaceWildcards(string $value, array $entry): string {
	$attrsLower = array_change_key_case($entry, CASE_LOWER);
	$replacement = $value;
	$attrsLower['dn'] = array($attrsLower['dn']);
	// ((attr)) -> " "
	$matches = array();
	if (preg_match_all('/\\(\\(([a-z0-9-_]+)\\)\\)/i', $replacement, $matches) > 0) {
		foreach ($matches[1] as $match) {
			$attributeName = strtolower($match);
			if (!empty($attrsLower[$attributeName][0])) {
				$replacement = str_replace('((' . $match . '))', ' ', $replacement);
			}
			else {
				$replacement = str_replace('((' . $match . '))', '', $replacement);
			}
		}
	}
	// %attr% -> value
	$matches = array();
	if (preg_match_all('/%([a-z0-9-_]+)%/i', $replacement, $matches) > 0) {
		foreach ($matches[1] as $match) {
			$attributeName = strtolower($match);
			if (!empty($attrsLower[$attributeName][0])) {
				$replacement = str_replace('%' . $match . '%', $attrsLower[$attributeName][0], $replacement);
			}
			else {
				$replacement = str_replace('%' . $match . '%', '', $replacement);
			}
		}
	}
	// @attr@ -> v
	$matches = array();
	if (preg_match_all('/@([a-z0-9-_]+)@/i', $replacement, $matches) > 0) {
		foreach ($matches[1] as $match) {
			$attributeName = strtolower($match);
			if (!empty($attrsLower[$attributeName][0])) {
				$replacement = str_replace('@' . $match . '@', substr($attrsLower[$attributeName][0], 0, 1), $replacement);
			}
			else {
				$replacement = str_replace('@' . $match . '@', '', $replacement);
			}
		}
	}
	// ??attr?? -> value
	$matches = array();
	if (preg_match_all('/\\?\\?([a-z0-9-_]+)\\?\\?/i', $replacement, $matches) > 0) {
		foreach ($matches[1] as $match) {
			$attributeName = strtolower($match);
			if (!empty($attrsLower[$attributeName][0])) {
				$replacement = str_replace('??' . $match . '??', strtolower($attrsLower[$attributeName][0]), $replacement);
			}
			else {
				$replacement = str_replace('??' . $match . '??', '', $replacement);
			}
		}
	}
	// !!attr!! -> VALUE
	$matches = array();
	if (preg_match_all('/!!([a-z0-9-_]+)!!/i', $replacement, $matches) > 0) {
		foreach ($matches[1] as $match) {
			$attributeName = strtolower($match);
			if (!empty($attrsLower[$attributeName][0])) {
				$replacement = str_replace('!!' . $match . '!!', strtoupper($attrsLower[$attributeName][0]), $replacement);
			}
			else {
				$replacement = str_replace('!!' . $match . '!!', '', $replacement);
			}
		}
	}
	// ?attr? -> v
	$matches = array();
	if (preg_match_all('/\\?([a-z0-9-_]+)\\?/i', $replacement, $matches) > 0) {
		foreach ($matches[1] as $match) {
			$attributeName = strtolower($match);
			if (!empty($attrsLower[$attributeName][0])) {
				$replacement = str_replace('?' . $match . '?', strtolower(substr($attrsLower[$attributeName][0], 0, 1)), $replacement);
			}
			else {
				$replacement = str_replace('?' . $match . '?', '', $replacement);
			}
		}
	}
	// !attr! -> V
	$matches = array();
	if (preg_match_all('/!([a-z0-9-_]+)!/i', $replacement, $matches) > 0) {
		foreach ($matches[1] as $match) {
			$attributeName = strtolower($match);
			if (!empty($attrsLower[$attributeName][0])) {
				$replacement = str_replace('!' . $match . '!', strtoupper(substr($attrsLower[$attributeName][0], 0, 1)), $replacement);
			}
			else {
				$replacement = str_replace('!' . $match . '!', '', $replacement);
			}
		}
	}
	// §attr|;§ -> "val1;val2"
	$matches = array();
	if (preg_match_all('/§([a-z0-9-_]+)\\|([^§]+)§/i', $replacement, $matches) > 0) {
		for ($i = 0; $i < sizeof($matches[1]); $i++) {
			$attributeName = strtolower($matches[1][$i]);
			$separator = $matches[2][$i];
			$match = $matches[0][$i];
			if (!empty($attrsLower[$attributeName][0])) {
				$replacement = str_replace($match, implode($separator, $attrsLower[$attributeName]), $replacement);
			}
			else {
				$replacement = str_replace($match, '', $replacement);
			}
		}
	}
	return $replacement;
}
