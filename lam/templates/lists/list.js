/**

$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003 - 2007  Roland Gruber

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


function list_over(list, box, scope) {
	cbox = document.getElementsByName(box)[0];
	if (cbox.checked == false) {
		list.setAttribute('className', scope + 'list-over', 0);
		list.setAttribute('class', scope + 'list-over', 0);
	}
}

function list_out(list, box, scope) {
	cbox = document.getElementsByName(box)[0];
	if (cbox.checked == false) {
		list.setAttribute('className', scope + 'list', 0);
		list.setAttribute('class', scope + 'list', 0);
	}
}

function list_click(list, box, scope) {
	cbox = document.getElementsByName(box)[0];
	if (cbox.checked == true) {
		cbox.checked = false;
		list.setAttribute('className', scope + 'list-over', 0);
		list.setAttribute('class', scope + 'list-over', 0);
	}
	else {
		cbox.checked = true;
		list.setAttribute('className', scope + 'list-checked', 0);
		list.setAttribute('class', scope + 'list-checked', 0);
	}
}

function listOUchanged(type) {
	selectOU = document.getElementsByName('suffix')[0];
	location.href='list.php?type=' + type + '&suffix=' + selectOU.options[selectOU.selectedIndex].value;
}

function SubmitForm(id, e) {
	if (e.keyCode == 13) {
		document.getElementsByName(id)[0].click();
		return false;
	}
}
