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

use LAMCfgMain;

include_once __DIR__ . "/../../lib/config.inc";
include_once __DIR__ . "/../../lib/mcp.inc";

$cfg = $_SESSION['cfgMain'] ?? new LAMCfgMain();
if ($cfg->activateMcp !== true) {
	http_response_code(401);
	exit;
}

validateRequestFormat();

$bodyData = file_get_contents('php://input');
$json = json_decode($bodyData, true);
validateJsonData($json);
routeRequest($json);
