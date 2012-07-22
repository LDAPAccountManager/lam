<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2009 - 2012  Roland Gruber

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
include_once("../lib/security.inc");
/** access to configuration options */
include_once("../lib/config.inc");

// start session
startSecureSession();

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

$result = @ldap_read($_SESSION['ldap']->server(), '', 'objectclass=*', array('+', '*', 'subschemasubentry'), 0, 0, 0, LDAP_DEREF_NEVER);
if ($result) {
	$info = @ldap_get_entries($_SESSION['ldap']->server(), $result);
	if ($info) {
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
$monitorResult = searchLDAP('cn=monitor', 'objectClass=*', array('*', '+'));
$monitorEntries = array();
for ($i = 0; $i < sizeof($monitorResult); $i++) {
	$monitorEntries[$monitorResult[$i]['dn']] = array_change_key_case($monitorResult[$i], CASE_LOWER);
}
$monitorEntries = array_change_key_case($monitorEntries, CASE_LOWER);

include 'main_header.php';
echo '<div class="userlist-bright smallPaddingContent">';
$tabindex = 1;
$container = new htmlTable();
$spacer = new htmlSpacer('20px', null);

$container->addElement(new htmlTitle(_("Server information")), true);

$container->addElement(new htmlOutputText('<b>' . _("Managed suffixes") . '</b>', false));
$container->addElement($spacer);
$container->addElement(new htmlOutputText($namingContexts), true);

$container->addElement(new htmlOutputText('<b>' . _("LDAP version") . '</b>', false));
$container->addElement($spacer);
$container->addElement(new htmlOutputText($supportedldapversion), true);

if ($configcontext != '') {
	$container->addElement(new htmlOutputText('<b>' . _("Config suffix") . '</b>', false));
	$container->addElement($spacer);
	$container->addElement(new htmlOutputText($configcontext), true);
}

$container->addElement(new htmlOutputText('<b>' . _("Schema suffix") . '</b>', false));
$container->addElement($spacer);
$container->addElement(new htmlOutputText($subschemasubentry), true);

if ($dynamicSubtrees != '') {
	$container->addElement(new htmlOutputText('<b>' . _("Dynamic subtrees") . '</b>', false));
	$container->addElement($spacer);
	$container->addElement(new htmlOutputText($dynamicSubtrees), true);
}

$container->addElement(new htmlOutputText('<b>' . _("SASL mechanisms") . '</b>', false));
$container->addElement($spacer);
$container->addElement(new htmlOutputText($supportedsaslmechanisms), true);

if ($vendorname != '') {
	$container->addElement(new htmlOutputText('<b>' . _("Vendor name") . '</b>', false));
	$container->addElement($spacer);
	$container->addElement(new htmlOutputText($vendorname), true);
}

if ($vendorversion != '') {
	$container->addElement(new htmlOutputText('<b>' . _("Vendor version") . '</b>', false));
	$container->addElement($spacer);
	$container->addElement(new htmlOutputText($vendorversion), true);
}

// monitoring information
if (isset($monitorEntries['cn=monitor']['monitoredinfo'])) {
	$container->addElement(new htmlOutputText('<b>' . _("Name") . '</b>', false));
	$container->addElement($spacer);
	$container->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=monitor']['monitoredinfo'])), true);
}
if (isset($monitorEntries['cn=listeners,cn=monitor'])) {
	$container->addElement(new htmlOutputText('<b>' . _("Listeners") . '</b>', false));
	$container->addElement($spacer);
	$listeners = array();
	$l = 0;
	while (isset($monitorEntries['cn=listener ' . $l . ',cn=listeners,cn=monitor'])) {
		$listeners[] = $monitorEntries['cn=listener ' . $l . ',cn=listeners,cn=monitor']['monitorconnectionlocaladdress'][0];
		$l++;
	}
	$container->addElement(new htmlOutputText(implode(', ', $listeners)), true);
}
if (isset($monitorEntries['cn=backends,cn=monitor'])) {
	$container->addElement(new htmlOutputText('<b>' . _("Backends") . '</b>', false));
	$container->addElement($spacer);
	$container->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=backends,cn=monitor']['monitoredinfo'])), true);
}
if (isset($monitorEntries['cn=overlays,cn=monitor'])) {
	$container->addElement(new htmlOutputText('<b>' . _("Overlays") . '</b>', false));
	$container->addElement($spacer);
	$container->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=overlays,cn=monitor']['monitoredinfo'])), true);
}
if (isset($monitorEntries['cn=max file descriptors,cn=connections,cn=monitor'])) {
	$container->addElement(new htmlOutputText('<b>' . _("Max. file descriptors") . '</b>', false));
	$container->addElement($spacer);
	$container->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=max file descriptors,cn=connections,cn=monitor']['monitorcounter'])), true);
}

// server statistics
if (isset($monitorEntries['cn=time,cn=monitor']) || isset($monitorEntries['cn=statistics,cn=monitor']) || isset($monitorEntries['cn=monitor']['currenttime'])) {
	$container->addElement(new htmlSubTitle(_('Server statistics')), true);
	if (isset($monitorEntries['cn=entries,cn=statistics,cn=monitor'])) {
		$container->addElement(new htmlOutputText('<b>' . _("LDAP entries") . '</b>', false));
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=entries,cn=statistics,cn=monitor']['monitorcounter'])), true);
	}
	if (isset($monitorEntries['cn=referrals,cn=statistics,cn=monitor'])) {
		$container->addElement(new htmlOutputText('<b>' . _("Referrals") . '</b>', false));
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=referrals,cn=statistics,cn=monitor']['monitorcounter'])), true);
	}
	if (isset($monitorEntries['cn=start,cn=time,cn=monitor'])) {
		$time = formatLDAPTimestamp($monitorEntries['cn=start,cn=time,cn=monitor']['monitortimestamp'][0]);
		$container->addElement(new htmlOutputText('<b>' . _("Start time") . '</b>', false));
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText($time), true);
	}
	elseif (isset($monitorEntries['cn=monitor']['starttime'])) { // Fedora 389
		$time = formatLDAPTimestamp($monitorEntries['cn=monitor']['starttime'][0]);
		$container->addElement(new htmlOutputText('<b>' . _("Start time") . '</b>', false));
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText($time), true);
	}
	if (isset($monitorEntries['cn=current,cn=time,cn=monitor'])) {
		$time = formatLDAPTimestamp($monitorEntries['cn=current,cn=time,cn=monitor']['monitortimestamp'][0]);
		$container->addElement(new htmlOutputText('<b>' . _("Server time") . '</b>', false));
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText($time), true);
	}
	elseif (isset($monitorEntries['cn=monitor']['currenttime'])) { // Fedora 389
		$time = formatLDAPTimestamp($monitorEntries['cn=monitor']['currenttime'][0]);
		$container->addElement(new htmlOutputText('<b>' . _("Server time") . '</b>', false));
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText($time), true);
	}
	if (isset($monitorEntries['cn=uptime,cn=time,cn=monitor'])) {
		$uptime = $monitorEntries['cn=uptime,cn=time,cn=monitor']['monitoredinfo'][0];
		$days = floor($uptime / (3600 * 24));
		$daysRest = $uptime - ($days * 3600 * 24);
		$hours = floor($daysRest / 3600);
		$hoursRest = $daysRest - ($hours * 3600);
		$minutes = floor($hoursRest / 60);
		$container->addElement(new htmlOutputText('<b>' . _("Uptime") . '</b>', false));
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText($days . ':' . $hours . ':' . $minutes), true);
	}
}

// connection statistics
if (isset($monitorEntries['cn=connections,cn=monitor']) || isset($monitorEntries['cn=statistics,cn=monitor']) || isset($monitorEntries['cn=monitor']['currentconnections'])) {
	$container->addElement(new htmlSubTitle(_('Connection statistics')), true);
	if (isset($monitorEntries['cn=current,cn=connections,cn=monitor'])) {
		$container->addElement(new htmlOutputText('<b>' . _("Current connections") . '</b>', false));
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=current,cn=connections,cn=monitor']['monitorcounter'])), true);
	}
	elseif (isset($monitorEntries['cn=monitor']['currentconnections'])) { // Fedora 389
		$container->addElement(new htmlOutputText('<b>' . _("Current connections") . '</b>', false));
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=monitor']['currentconnections'])), true);
	}
	if (isset($monitorEntries['cn=total,cn=connections,cn=monitor'])) {
		$container->addElement(new htmlOutputText('<b>' . _("Total connections") . '</b>', false));
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=total,cn=connections,cn=monitor']['monitorcounter'])), true);
	}
	elseif (isset($monitorEntries['cn=monitor']['totalconnections'])) { // Fedora 389
		$container->addElement(new htmlOutputText('<b>' . _("Total connections") . '</b>', false));
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=monitor']['totalconnections'])), true);
	}
	if (isset($monitorEntries['cn=bytes,cn=statistics,cn=monitor'])) {
		$bytes = round($monitorEntries['cn=bytes,cn=statistics,cn=monitor']['monitorcounter'][0] / 1000000, 2) . 'MB';
		$container->addElement(new htmlOutputText('<b>' . _("Bytes sent") . '</b>', false));
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText($bytes), true);
	}
	elseif (isset($monitorEntries['cn=monitor']['bytessent'])) { // Fedora 389
		$bytes = round($monitorEntries['cn=monitor']['bytessent'][0] / 1000000, 2) . 'MB';
		$container->addElement(new htmlOutputText('<b>' . _("Bytes sent") . '</b>', false));
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText($bytes), true);
	}
	if (isset($monitorEntries['cn=pdu,cn=statistics,cn=monitor'])) {
		$container->addElement(new htmlOutputText('<b>' . _("PDUs sent") . '</b>', false));
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText($monitorEntries['cn=pdu,cn=statistics,cn=monitor']['monitorcounter'][0]), true);
	}
	if (isset($monitorEntries['cn=monitor']['entriessent'])) { // Fedora 389
		$container->addElement(new htmlOutputText('<b>' . _("Entries sent") . '</b>', false));
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText($monitorEntries['cn=monitor']['entriessent'][0]), true);
	}
}

// operation statistics (OpenLDAP)
if (isset($monitorEntries['cn=operations,cn=monitor'])) {
	$container->addElement(new htmlSubTitle(_('Operation statistics')), true);
	$opStats = new htmlTable();
	$opStats->colspan = 10;
	$opStats->addElement(new htmlOutputText(''));
	$opStats->addElement($spacer);
	$opStats->addElement(new htmlOutputText('<b>' . _("Initiated") . '</b>', false));
	$opStats->addElement($spacer);
	$opStats->addElement(new htmlOutputText('<b>' . _("Completed") . '</b>', false), true);
	if (isset($monitorEntries['cn=bind,cn=operations,cn=monitor'])) {
		$opStats->addElement(new htmlOutputText('<b>' . _("Bind") . '</b>', false));
		$opStats->addElement($spacer);
		$opStats->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=bind,cn=operations,cn=monitor']['monitoropinitiated'])));
		$opStats->addElement($spacer);
		$opStats->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=bind,cn=operations,cn=monitor']['monitoropcompleted'])), true);
	}
	if (isset($monitorEntries['cn=unbind,cn=operations,cn=monitor'])) {
		$opStats->addElement(new htmlOutputText('<b>' . _("Unbind") . '</b>', false));
		$opStats->addElement($spacer);
		$opStats->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=unbind,cn=operations,cn=monitor']['monitoropinitiated'])));
		$opStats->addElement($spacer);
		$opStats->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=unbind,cn=operations,cn=monitor']['monitoropcompleted'])), true);
	}
	if (isset($monitorEntries['cn=search,cn=operations,cn=monitor'])) {
		$opStats->addElement(new htmlOutputText('<b>' . _("Search") . '</b>', false));
		$opStats->addElement($spacer);
		$opStats->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=search,cn=operations,cn=monitor']['monitoropinitiated'])));
		$opStats->addElement($spacer);
		$opStats->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=search,cn=operations,cn=monitor']['monitoropcompleted'])), true);
	}
	if (isset($monitorEntries['cn=add,cn=operations,cn=monitor'])) {
		$opStats->addElement(new htmlOutputText('<b>' . _("Add") . '</b>', false));
		$opStats->addElement($spacer);
		$opStats->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=add,cn=operations,cn=monitor']['monitoropinitiated'])));
		$opStats->addElement($spacer);
		$opStats->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=add,cn=operations,cn=monitor']['monitoropcompleted'])), true);
	}
	if (isset($monitorEntries['cn=modify,cn=operations,cn=monitor'])) {
		$opStats->addElement(new htmlOutputText('<b>' . _("Modify") . '</b>', false));
		$opStats->addElement($spacer);
		$opStats->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=modify,cn=operations,cn=monitor']['monitoropinitiated'])));
		$opStats->addElement($spacer);
		$opStats->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=modify,cn=operations,cn=monitor']['monitoropcompleted'])), true);
	}
	if (isset($monitorEntries['cn=delete,cn=operations,cn=monitor'])) {
		$opStats->addElement(new htmlOutputText('<b>' . _("Delete") . '</b>', false));
		$opStats->addElement($spacer);
		$opStats->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=delete,cn=operations,cn=monitor']['monitoropinitiated'])));
		$opStats->addElement($spacer);
		$opStats->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=delete,cn=operations,cn=monitor']['monitoropcompleted'])), true);
	}
	if (isset($monitorEntries['cn=modrdn,cn=operations,cn=monitor'])) {
		$opStats->addElement(new htmlOutputText('<b>' . _("Modify RDN") . '</b>', false));
		$opStats->addElement($spacer);
		$opStats->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=modrdn,cn=operations,cn=monitor']['monitoropinitiated'])));
		$opStats->addElement($spacer);
		$opStats->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=modrdn,cn=operations,cn=monitor']['monitoropcompleted'])), true);
	}
	if (isset($monitorEntries['cn=compare,cn=operations,cn=monitor'])) {
		$opStats->addElement(new htmlOutputText('<b>' . _("Compare") . '</b>', false));
		$opStats->addElement($spacer);
		$opStats->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=compare,cn=operations,cn=monitor']['monitoropinitiated'])));
		$opStats->addElement($spacer);
		$opStats->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=compare,cn=operations,cn=monitor']['monitoropcompleted'])), true);
	}
	if (isset($monitorEntries['cn=abandon,cn=operations,cn=monitor'])) {
		$opStats->addElement(new htmlOutputText('<b>' . _("Abandon") . '</b>', false));
		$opStats->addElement($spacer);
		$opStats->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=abandon,cn=operations,cn=monitor']['monitoropinitiated'])));
		$opStats->addElement($spacer);
		$opStats->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=abandon,cn=operations,cn=monitor']['monitoropcompleted'])), true);
	}
	if (isset($monitorEntries['cn=extended,cn=operations,cn=monitor'])) {
		$opStats->addElement(new htmlOutputText('<b>' . _("Extended") . '</b>', false));
		$opStats->addElement($spacer);
		$opStats->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=extended,cn=operations,cn=monitor']['monitoropinitiated'])));
		$opStats->addElement($spacer);
		$opStats->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=extended,cn=operations,cn=monitor']['monitoropcompleted'])), true);
	}
	if (isset($monitorEntries['cn=operations,cn=monitor']['monitoropinitiated'])) {
		$opStats->addElement(new htmlSpacer(null, '10px'), true);
		$opStats->addElement(new htmlOutputText('<b>' . _("Total") . '</b>', false));
		$opStats->addElement($spacer);
		$opStats->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=operations,cn=monitor']['monitoropinitiated'])));
		$opStats->addElement($spacer);
		$opStats->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=operations,cn=monitor']['monitoropcompleted'])), true);
	}
	$container->addElement($opStats);
}
// operation statistics (389 server)
elseif (isset($monitorEntries['cn=monitor']['opsinitiated'])) {
	$container->addElement(new htmlSubTitle(_('Operation statistics')), true);
	$container->addElement(new htmlOutputText('<b>' . _("Initiated") . '</b>', false));
	$container->addElement($spacer);
	$container->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=monitor']['opsinitiated'])), true);
	$container->addElement(new htmlOutputText('<b>' . _("Completed") . '</b>', false));
	$container->addElement($spacer);
	$container->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=monitor']['opscompleted'])), true);
	if (isset($monitorEntries['cn=snmp,cn=monitor']['addentryops'])) {
		$container->addElement(new htmlOutputText('<b>' . _("Bind") . '</b>', false));
		$container->addElement($spacer);
		$binds = $monitorEntries['cn=snmp,cn=monitor']['anonymousbinds'][0] + $monitorEntries['cn=snmp,cn=monitor']['unauthbinds'][0]
					+ $monitorEntries['cn=snmp,cn=monitor']['simpleauthbinds'][0] + $monitorEntries['cn=snmp,cn=monitor']['strongauthbinds'][0];
		$container->addElement(new htmlOutputText($binds), true);
		$container->addElement(new htmlOutputText('<b>' . _("Search") . '</b>', false));
		$container->addElement($spacer);
		$searches = $monitorEntries['cn=snmp,cn=monitor']['searchops'][0];
		$container->addElement(new htmlOutputText($searches), true);
		$container->addElement(new htmlOutputText('<b>' . _("Add") . '</b>', false));
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=snmp,cn=monitor']['addentryops'])), true);
		$container->addElement(new htmlOutputText('<b>' . _("Modify") . '</b>', false));
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=snmp,cn=monitor']['modifyentryops'])), true);
		$container->addElement(new htmlOutputText('<b>' . _("Delete") . '</b>', false));
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=snmp,cn=monitor']['removeentryops'])), true);
		$container->addElement(new htmlOutputText('<b>' . _("Modify RDN") . '</b>', false));
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=snmp,cn=monitor']['modifyrdnops'])), true);
		$container->addElement(new htmlOutputText('<b>' . _("Compare") . '</b>', false));
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText(implode(', ', $monitorEntries['cn=snmp,cn=monitor']['compareops'])), true);
	}
}

parseHtml(null, $container, array(), true, $tabindex, 'user');

echo '</div>';
include 'main_footer.php';

?>
