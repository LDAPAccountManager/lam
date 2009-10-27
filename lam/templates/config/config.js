/**

$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2009  Roland Gruber

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
 * The following functions are used for the LAM configuration wizard.
 */

/**
 * Hides/unhides input fields for the login method.
 */
function configLoginMethodChanged() {
	selectLoginMethod = document.getElementsByName('loginMethod')[0];
	if ( selectLoginMethod.options[selectLoginMethod.selectedIndex].value == 'list' ) {
		 document.getElementById('trAdminList').style.display = '';
		 document.getElementById('trLoginSearchSuffix').style.display = 'none';
		 document.getElementById('trLoginSearchFilter').style.display = 'none';
	}
	else {
		 document.getElementById('trAdminList').style.display = 'none';
		 document.getElementById('trLoginSearchSuffix').style.display = '';
		 document.getElementById('trLoginSearchFilter').style.display = '';
	}
}
