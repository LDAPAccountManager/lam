/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Roland Gruber

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


// functions for row checking and mouseOver effects

// mouseOver function
function user_over(list, box) {
	cbox = document.getElementsByName(box)[0];
	if (cbox.checked == false) list.setAttribute('class','userlist-over', 0);
}

// mouseOut function
function user_out(list, box) {
	cbox = document.getElementsByName(box)[0];
	if (cbox.checked == false) list.setAttribute('class','userlist', 0);
}

// onClick function
function user_click(list, box) {
	cbox = document.getElementsByName(box)[0];
	if (cbox.checked == true) {
		cbox.checked = false;
		list.setAttribute('class','userlist-over', 0);
	}
	else {
		cbox.checked = true;
		list.setAttribute('class','userlist-checked', 0);
	}
}


// mouseOver function
function group_over(list, box) {
	cbox = document.getElementsByName(box)[0];
	if (cbox.checked == false) list.setAttribute('class','grouplist-over', 0);
}

// mouseOut function
function group_out(list, box) {
	cbox = document.getElementsByName(box)[0];
	if (cbox.checked == false) list.setAttribute('class','grouplist', 0);
}

// onClick function
function group_click(list, box) {
	cbox = document.getElementsByName(box)[0];
	if (cbox.checked == true) {
		cbox.checked = false;
		list.setAttribute('class','grouplist-over', 0);
	}
	else {
		cbox.checked = true;
		list.setAttribute('class','grouplist-checked', 0);
	}
}


// mouseOver function
function host_over(list, box) {
	cbox = document.getElementsByName(box)[0];
	if (cbox.checked == false) list.setAttribute('class','hostlist-over', 0);
}

// mouseOut function
function host_out(list, box) {
	cbox = document.getElementsByName(box)[0];
	if (cbox.checked == false) list.setAttribute('class','hostlist', 0);
}

// onClick function
function host_click(list, box) {
	cbox = document.getElementsByName(box)[0];
	if (cbox.checked == true) {
		cbox.checked = false;
		list.setAttribute('class','hostlist-over', 0);
	}
	else {
		cbox.checked = true;
		list.setAttribute('class','hostlist-checked', 0);
	}
}
