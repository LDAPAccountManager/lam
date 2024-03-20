<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2009 - 2024  Roland Gruber

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
* Provides a list of LAM tests.
*
* @author Roland Gruber
* @package tools
*/

/** security functions */
include_once(__DIR__ . "/../../lib/security.inc");
/** access to configuration options */
include_once(__DIR__ . "/../../lib/config.inc");

// start session
startSecureSession();
enforceUserIsLoggedIn();

checkIfToolIsActive('toolServerInformation');

setlanguage();

$namingContexts = '';
$configcontext = '';
$supportedldapversion = '';
$supportedsaslmechanisms = '';
$subschemasubentry = '';
$vendorname = '';
$vendorversion = '';
$dynamicSubtrees = '';

$result = ldap_read($_SESSION['ldap']->server(), '', '(objectclass=*)', ['+', '*', 'subschemasubentry'], 0, 0, 0, LDAP_DEREF_NEVER);
if ($result) {
	$info = ldap_get_entries($_SESSION['ldap']->server(), $result);
	if (is_array($info) && is_array($info[0])) {
		$info = $info[0];
		foreach ($info as $key => $value) {
			if (is_array($info[$key]) && isset($info[$key]['count'])) {
				unset($info[$key]['count']);
			}
		}
		if (isset($info['namingcontexts'])) {
			$namingContexts = implode(', ', $info['namingcontexts']);
		}
		if (isset($info['configcontext'])) {
			$configcontext = $info['configcontext'][0];
		}
		if (isset($info['supportedldapversion'])) {
			$supportedldapversion = implode(', ', $info['supportedldapversion']);
		}
		if (isset($info['supportedsaslmechanisms'])) {
			$supportedsaslmechanisms = implode(', ', $info['supportedsaslmechanisms']);
		}
		if (isset($info['subschemasubentry'])) {
			$subschemasubentry = $info['subschemasubentry'][0];
		}
		if (isset($info['vendorname'])) {
			$vendorname = $info['vendorname'][0];
		}
		if (isset($info['vendorversion'])) {
			$vendorversion = $info['vendorversion'][0];
		}
		if (isset($info['dynamicsubtrees'])) {
			$dynamicSubtrees = implode(', ', $info['dynamicsubtrees']);
		}
	}
}

// get additional information if monitoring is enabled
$monitorResults = searchLDAP('cn=monitor', '(objectClass=*)', ['*', '+']);
$monitorEntries = [];
foreach ($monitorResults as $monitorResult) {
	$monitorEntries[$monitorResult['dn']] = array_change_key_case($monitorResult, CASE_LOWER);
}
$monitorEntries = array_change_key_case($monitorEntries, CASE_LOWER);

include __DIR__ . '/../../lib/adminHeader.inc';
echo '<div class="smallPaddingContent">';
$container = new htmlResponsiveRow();

$container->add(new htmlTitle(_("Server information")), 12);

if (!empty($namingContexts)) {
	$container->addLabel(new htmlOutputText('<b>' . _("Managed suffixes") . '</b>', false));
	$container->addField(new htmlOutputText($namingContexts));
}

if (!empty($supportedldapversion)) {
	$container->addLabel(new htmlOutputText('<b>' . _("LDAP version") . '</b>', false));
	$container->addField(new htmlOutputText($supportedldapversion));
}

if ($configcontext != '') {
	$container->addLabel(new htmlOutputText('<b>' . _("Config suffix") . '</b>', false));
	$container->addField(new htmlOutputText($configcontext));
}

$container->addLabel(new htmlOutputText('<b>' . _("Schema suffix") . '</b>', false));
$container->addField(new htmlOutputText($subschemasubentry));

if ($dynamicSubtrees != '') {
	$container->addLabel(new htmlOutputText('<b>' . _("Dynamic subtrees") . '</b>', false));
	$container->addField(new htmlOutputText($dynamicSubtrees));
}

if (!empty($supportedsaslmechanisms)) {
	$container->addLabel(new htmlOutputText('<b>' . _("SASL mechanisms") . '</b>', false));
	$container->addField(new htmlOutputText($supportedsaslmechanisms));
}

if ($vendorname != '') {
	$container->addLabel(new htmlOutputText('<b>' . _("Vendor name") . '</b>', false));
	$container->addField(new htmlOutputText($vendorname));
}

if ($vendorversion != '') {
	$container->addLabel(new htmlOutputText('<b>' . _("Vendor version") . '</b>', false));
	$container->addField(new htmlOutputText($vendorversion));
}

// monitoring information
if (isset($monitorEntries['cn=monitor']['monitoredinfo'])) {
	$container->addLabel(new htmlOutputText('<b>' . _("Name") . '</b>', false));
	$container->addField(new htmlOutputText(implode(', ', $monitorEntries['cn=monitor']['monitoredinfo'])));
}
if (isset($monitorEntries['cn=listeners,cn=monitor'])) {
	$container->addLabel(new htmlOutputText('<b>' . _("Listeners") . '</b>', false));
	$listeners = [];
	$l = 0;
	while (isset($monitorEntries['cn=listener ' . $l . ',cn=listeners,cn=monitor'])) {
		$listeners[] = $monitorEntries['cn=listener ' . $l . ',cn=listeners,cn=monitor']['monitorconnectionlocaladdress'][0];
		$l++;
	}
	$container->addField(new htmlOutputText(implode(', ', $listeners)));
}
if (isset($monitorEntries['cn=backends,cn=monitor'])) {
	$container->addLabel(new htmlOutputText('<b>' . _("Backends") . '</b>', false));
	$container->addField(new htmlOutputText(implode(', ', $monitorEntries['cn=backends,cn=monitor']['monitoredinfo'])));
}
if (isset($monitorEntries['cn=overlays,cn=monitor'])) {
	$container->addLabel(new htmlOutputText('<b>' . _("Overlays") . '</b>', false));
	$container->addField(new htmlOutputText(implode(', ', $monitorEntries['cn=overlays,cn=monitor']['monitoredinfo'])));
}
if (isset($monitorEntries['cn=max file descriptors,cn=connections,cn=monitor'])) {
	$container->addLabel(new htmlOutputText('<b>' . _("Max. file descriptors") . '</b>', false));
	$container->addField(new htmlOutputText(implode(', ', $monitorEntries['cn=max file descriptors,cn=connections,cn=monitor']['monitorcounter'])));
}

// server statistics
if (isset($monitorEntries['cn=time,cn=monitor']) || isset($monitorEntries['cn=statistics,cn=monitor']) || isset($monitorEntries['cn=monitor']['currenttime'])) {
	$container->add(new htmlSubTitle(_('Server statistics')), 12);
	if (isset($monitorEntries['cn=entries,cn=statistics,cn=monitor'])) {
		$container->addLabel(new htmlOutputText('<b>' . _("LDAP entries") . '</b>', false));
		$container->addField(new htmlOutputText(implode(', ', $monitorEntries['cn=entries,cn=statistics,cn=monitor']['monitorcounter'])));
	}
	if (isset($monitorEntries['cn=referrals,cn=statistics,cn=monitor'])) {
		$container->addLabel(new htmlOutputText('<b>' . _("Referrals") . '</b>', false));
		$container->addField(new htmlOutputText(implode(', ', $monitorEntries['cn=referrals,cn=statistics,cn=monitor']['monitorcounter'])));
	}
	if (isset($monitorEntries['cn=start,cn=time,cn=monitor'])) {
		$time = formatLDAPTimestamp($monitorEntries['cn=start,cn=time,cn=monitor']['monitortimestamp'][0]);
		$container->addLabel(new htmlOutputText('<b>' . _("Start time") . '</b>', false));
		$container->addField(new htmlOutputText($time), 12);
	}
	elseif (isset($monitorEntries['cn=monitor']['starttime'])) { // Fedora 389
		$time = formatLDAPTimestamp($monitorEntries['cn=monitor']['starttime'][0]);
		$container->addLabel(new htmlOutputText('<b>' . _("Start time") . '</b>', false));
		$container->addField(new htmlOutputText($time));
	}
	if (isset($monitorEntries['cn=current,cn=time,cn=monitor'])) {
		$time = formatLDAPTimestamp($monitorEntries['cn=current,cn=time,cn=monitor']['monitortimestamp'][0]);
		$container->addLabel(new htmlOutputText('<b>' . _("Server time") . '</b>', false));
		$container->addField(new htmlOutputText($time));
	}
	elseif (isset($monitorEntries['cn=monitor']['currenttime'])) { // Fedora 389
		$time = formatLDAPTimestamp($monitorEntries['cn=monitor']['currenttime'][0]);
		$container->addLabel(new htmlOutputText('<b>' . _("Server time") . '</b>', false));
		$container->addField(new htmlOutputText($time));
	}
	if (isset($monitorEntries['cn=uptime,cn=time,cn=monitor'])) {
		$uptime = $monitorEntries['cn=uptime,cn=time,cn=monitor']['monitoredinfo'][0];
		$days = floor($uptime / (3600 * 24));
		$daysRest = $uptime - ($days * 3600 * 24);
		$hours = floor($daysRest / 3600);
		$hoursRest = $daysRest - ($hours * 3600);
		$minutes = floor($hoursRest / 60);
		$container->addLabel(new htmlOutputText('<b>' . _("Uptime") . '</b>', false));
		$container->addField(new htmlOutputText($days . ':' . $hours . ':' . $minutes));
	}
}

// connection statistics
if (isset($monitorEntries['cn=connections,cn=monitor']) || isset($monitorEntries['cn=statistics,cn=monitor']) || isset($monitorEntries['cn=monitor']['currentconnections'])) {
	$container->add(new htmlSubTitle(_('Connection statistics')), 12);
	if (isset($monitorEntries['cn=current,cn=connections,cn=monitor'])) {
		$container->addLabel(new htmlOutputText('<b>' . _("Current connections") . '</b>', false));
		$container->addField(new htmlOutputText(implode(', ', $monitorEntries['cn=current,cn=connections,cn=monitor']['monitorcounter'])));
	}
	elseif (isset($monitorEntries['cn=monitor']['currentconnections'])) { // Fedora 389
		$container->addLabel(new htmlOutputText('<b>' . _("Current connections") . '</b>', false));
		$container->addField(new htmlOutputText(implode(', ', $monitorEntries['cn=monitor']['currentconnections'])));
	}
	if (isset($monitorEntries['cn=total,cn=connections,cn=monitor'])) {
		$container->addLabel(new htmlOutputText('<b>' . _("Total connections") . '</b>', false));
		$container->addField(new htmlOutputText(implode(', ', $monitorEntries['cn=total,cn=connections,cn=monitor']['monitorcounter'])));
	}
	elseif (isset($monitorEntries['cn=monitor']['totalconnections'])) { // Fedora 389
		$container->addLabel(new htmlOutputText('<b>' . _("Total connections") . '</b>', false));
		$container->addField(new htmlOutputText(implode(', ', $monitorEntries['cn=monitor']['totalconnections'])));
	}
	if (isset($monitorEntries['cn=bytes,cn=statistics,cn=monitor'])) {
		$bytes = round($monitorEntries['cn=bytes,cn=statistics,cn=monitor']['monitorcounter'][0] / 1_000_000, 2) . 'MB';
		$container->addLabel(new htmlOutputText('<b>' . _("Bytes sent") . '</b>', false));
		$container->addField(new htmlOutputText($bytes));
	}
	elseif (isset($monitorEntries['cn=monitor']['bytessent'])) { // Fedora 389
		$bytes = round($monitorEntries['cn=monitor']['bytessent'][0] / 1_000_000, 2) . 'MB';
		$container->addLabel(new htmlOutputText('<b>' . _("Bytes sent") . '</b>', false));
		$container->addField(new htmlOutputText($bytes));
	}
	if (isset($monitorEntries['cn=pdu,cn=statistics,cn=monitor'])) {
		$container->addLabel(new htmlOutputText('<b>' . _("PDUs sent") . '</b>', false));
		$container->addField(new htmlOutputText($monitorEntries['cn=pdu,cn=statistics,cn=monitor']['monitorcounter'][0]));
	}
	if (isset($monitorEntries['cn=monitor']['entriessent'])) { // Fedora 389
		$container->addLabel(new htmlOutputText('<b>' . _("Entries sent") . '</b>', false));
		$container->addField(new htmlOutputText($monitorEntries['cn=monitor']['entriessent'][0]));
	}
}

// operation statistics (OpenLDAP)
if (isset($monitorEntries['cn=operations,cn=monitor'])) {
	$container->add(new htmlSubTitle(_('Operation statistics')), 12);
	$data = [];
	if (isset($monitorEntries['cn=bind,cn=operations,cn=monitor'])) {
		$data[] = [
			new htmlOutputText('<b>' . _("Bind") . '</b>', false),
			new htmlOutputText(implode(', ', $monitorEntries['cn=bind,cn=operations,cn=monitor']['monitoropinitiated'])),
			new htmlOutputText(implode(', ', $monitorEntries['cn=bind,cn=operations,cn=monitor']['monitoropcompleted'])),
		];
	}
	if (isset($monitorEntries['cn=unbind,cn=operations,cn=monitor'])) {
		$data[] = [
			new htmlOutputText('<b>' . _("Unbind") . '</b>', false),
			new htmlOutputText(implode(', ', $monitorEntries['cn=unbind,cn=operations,cn=monitor']['monitoropinitiated'])),
			new htmlOutputText(implode(', ', $monitorEntries['cn=unbind,cn=operations,cn=monitor']['monitoropcompleted'])),
		];
	}
	if (isset($monitorEntries['cn=search,cn=operations,cn=monitor'])) {
		$data[] = [
			new htmlOutputText('<b>' . _("Search") . '</b>', false),
			new htmlOutputText(implode(', ', $monitorEntries['cn=search,cn=operations,cn=monitor']['monitoropinitiated'])),
			new htmlOutputText(implode(', ', $monitorEntries['cn=search,cn=operations,cn=monitor']['monitoropcompleted'])),
		];
	}
	if (isset($monitorEntries['cn=add,cn=operations,cn=monitor'])) {
		$data[] = [
			new htmlOutputText('<b>' . _("Add") . '</b>', false),
			new htmlOutputText(implode(', ', $monitorEntries['cn=add,cn=operations,cn=monitor']['monitoropinitiated'])),
			new htmlOutputText(implode(', ', $monitorEntries['cn=add,cn=operations,cn=monitor']['monitoropcompleted'])),
		];
	}
	if (isset($monitorEntries['cn=modify,cn=operations,cn=monitor'])) {
		$data[] = [
			new htmlOutputText('<b>' . _("Modify") . '</b>', false),
			new htmlOutputText(implode(', ', $monitorEntries['cn=modify,cn=operations,cn=monitor']['monitoropinitiated'])),
			new htmlOutputText(implode(', ', $monitorEntries['cn=modify,cn=operations,cn=monitor']['monitoropcompleted'])),
		];
	}
	if (isset($monitorEntries['cn=delete,cn=operations,cn=monitor'])) {
		$data[] = [
			new htmlOutputText('<b>' . _("Delete") . '</b>', false),
			new htmlOutputText(implode(', ', $monitorEntries['cn=delete,cn=operations,cn=monitor']['monitoropinitiated'])),
			new htmlOutputText(implode(', ', $monitorEntries['cn=delete,cn=operations,cn=monitor']['monitoropcompleted'])),
		];
	}
	if (isset($monitorEntries['cn=modrdn,cn=operations,cn=monitor'])) {
		$data[] = [
			new htmlOutputText('<b>' . _("Modify RDN") . '</b>', false),
			new htmlOutputText(implode(', ', $monitorEntries['cn=modrdn,cn=operations,cn=monitor']['monitoropinitiated'])),
			new htmlOutputText(implode(', ', $monitorEntries['cn=modrdn,cn=operations,cn=monitor']['monitoropcompleted'])),
		];
	}
	if (isset($monitorEntries['cn=compare,cn=operations,cn=monitor'])) {
		$data[] = [
			new htmlOutputText('<b>' . _("Compare") . '</b>', false),
			new htmlOutputText(implode(', ', $monitorEntries['cn=compare,cn=operations,cn=monitor']['monitoropinitiated'])),
			new htmlOutputText(implode(', ', $monitorEntries['cn=compare,cn=operations,cn=monitor']['monitoropcompleted'])),
		];
	}
	if (isset($monitorEntries['cn=abandon,cn=operations,cn=monitor'])) {
		$data[] = [
			new htmlOutputText('<b>' . _("Abandon") . '</b>', false),
			new htmlOutputText(implode(', ', $monitorEntries['cn=abandon,cn=operations,cn=monitor']['monitoropinitiated'])),
			new htmlOutputText(implode(', ', $monitorEntries['cn=abandon,cn=operations,cn=monitor']['monitoropcompleted'])),
		];
	}
	if (isset($monitorEntries['cn=extended,cn=operations,cn=monitor'])) {
		$data[] = [
			new htmlOutputText('<b>' . _("Extended") . '</b>', false),
			new htmlOutputText(implode(', ', $monitorEntries['cn=extended,cn=operations,cn=monitor']['monitoropinitiated'])),
			new htmlOutputText(implode(', ', $monitorEntries['cn=extended,cn=operations,cn=monitor']['monitoropcompleted'])),
		];
	}
	if (isset($monitorEntries['cn=operations,cn=monitor']['monitoropinitiated'])) {
		$data[] = [
			new htmlOutputText('<b>' . _("Total") . '</b>', false),
			new htmlOutputText(implode(', ', $monitorEntries['cn=operations,cn=monitor']['monitoropinitiated'])),
			new htmlOutputText(implode(', ', $monitorEntries['cn=operations,cn=monitor']['monitoropcompleted'])),
		];
	}
	$opStats = new htmlResponsiveTable(['', _("Initiated"), _("Completed")], $data);
	$container->add($opStats, 12);
}
// operation statistics (389 server)
elseif (isset($monitorEntries['cn=monitor']['opsinitiated'])) {
	$container->add(new htmlSubTitle(_('Operation statistics')), 12);
	$container->addLabel(new htmlOutputText('<b>' . _("Initiated") . '</b>', false));
	$container->addField(new htmlOutputText(implode(', ', $monitorEntries['cn=monitor']['opsinitiated'])));
	$container->addLabel(new htmlOutputText('<b>' . _("Completed") . '</b>', false));
	$container->addField(new htmlOutputText(implode(', ', $monitorEntries['cn=monitor']['opscompleted'])));
	if (isset($monitorEntries['cn=snmp,cn=monitor']['addentryops'])) {
		$container->addLabel(new htmlOutputText('<b>' . _("Bind") . '</b>', false));
		$binds = $monitorEntries['cn=snmp,cn=monitor']['anonymousbinds'][0] + $monitorEntries['cn=snmp,cn=monitor']['unauthbinds'][0]
					+ $monitorEntries['cn=snmp,cn=monitor']['simpleauthbinds'][0] + $monitorEntries['cn=snmp,cn=monitor']['strongauthbinds'][0];
		$container->addField(new htmlOutputText($binds));
		$container->addLabel(new htmlOutputText('<b>' . _("Search") . '</b>', false));
		$searches = $monitorEntries['cn=snmp,cn=monitor']['searchops'][0];
		$container->addField(new htmlOutputText($searches));
		$container->addLabel(new htmlOutputText('<b>' . _("Add") . '</b>', false));
		$container->addField(new htmlOutputText(implode(', ', $monitorEntries['cn=snmp,cn=monitor']['addentryops'])));
		$container->addLabel(new htmlOutputText('<b>' . _("Modify") . '</b>', false));
		$container->addField(new htmlOutputText(implode(', ', $monitorEntries['cn=snmp,cn=monitor']['modifyentryops'])));
		$container->addLabel(new htmlOutputText('<b>' . _("Delete") . '</b>', false));
		$container->addField(new htmlOutputText(implode(', ', $monitorEntries['cn=snmp,cn=monitor']['removeentryops'])));
		$container->addLabel(new htmlOutputText('<b>' . _("Modify RDN") . '</b>', false));
		$container->addField(new htmlOutputText(implode(', ', $monitorEntries['cn=snmp,cn=monitor']['modifyrdnops'])));
		$container->addLabel(new htmlOutputText('<b>' . _("Compare") . '</b>', false));
		$container->addField(new htmlOutputText(implode(', ', $monitorEntries['cn=snmp,cn=monitor']['compareops'])));
	}
}

parseHtml(null, $container, [], true, 'user');

echo '</div>';
include __DIR__ . '/../../lib/adminFooter.inc';
