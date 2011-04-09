/**

$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2011  Roland Gruber

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
 * Used to highlight the row under the mouse cursor.
 * 
 * @param list table row
 */
function list_over(list) {
	jQuery(list).addClass('highlight');
}

/**
 * Used to unhighlight a row if the mouse cursor leaves it.
 * 
 * @param list table row
 */
function list_out(list) {
	jQuery(list).removeClass('highlight');
}

/**
 * Called when user clicks on a table row. This toggles the checkbox in the row.
 * 
 * @param box checkbox name
 */
function list_click(box) {
	cbox = document.getElementsByName(box)[0];
	if (cbox.checked == true) {
		cbox.checked = false;
	}
	else {
		cbox.checked = true;
	}
}

/**
 * The user changed the value in the OU selection box. This will reload the list view with the new suffix.
 * 
 * @param type account type
 * @param element dropdown box
 */
function listOUchanged(type, element) {
	location.href='list.php?type=' + type + '&suffix=' + element.options[element.selectedIndex].value;
}

/**
 * Resizes the content area of the account lists to fit the window size.
 * This prevents that the whole page is scrolled in the browser. Only the account table has scroll bars.
 */
function listResizeITabContentDiv() {
	var myDiv = document.getElementById("listTabContentArea");
    var height = document.documentElement.clientHeight;
    height -= myDiv.offsetTop;
    height -= 105
    myDiv.style.height = height +"px";

    var myDivScroll = document.getElementById("listScrollArea");
	var top = myDivScroll.offsetTop;
	var scrollHeight = height - (top - myDiv.offsetTop);
	myDivScroll.style.height = scrollHeight + "px";
};

/**
 * Shows the dialog to change the list settings.
 * 
 * @param title dialog title
 * @param okText text for Ok button
 * @param cancelText text for Cancel button
 */
function listShowSettingsDialog(title, okText, cancelText) {
	var buttonList = {};
	buttonList[cancelText] = function() { jQuery(this).dialog("close"); };
	buttonList[okText] = function() { document.forms["settingsDialogForm"].submit(); };
	jQuery('#settingsDialog').dialog({
		modal: true,
		title: title,
		dialogClass: 'defaultBackground',
		buttons: buttonList,
		width: 'auto'
	});
}

function SubmitForm(id, e) {
	if (e.keyCode == 13) {
		document.getElementsByName(id)[0].click();
		return false;
	}
}

function addResizeHandler(item, min, max) {
	jQuery(item).toggle(
		function(){
			jQuery(item).animate({
				height: max
			});
		},
		function(){
			jQuery(item).animate({
				height: min
			});
		}
	);	
}

/**
 * Selects/deselects all accounts on the page.
 */
function list_switchAccountSelection() {
	// set checkbox selection
	jQuery('.accountBoxUnchecked').attr('checked', 'checked');
	jQuery('.accountBoxChecked').removeAttr('checked');
	// switch CSS class
	nowChecked = jQuery('.accountBoxUnchecked');
	nowUnchecked = jQuery('.accountBoxChecked');
	nowChecked.addClass('accountBoxChecked');
	nowChecked.removeClass('accountBoxUnchecked');
	nowUnchecked.addClass('accountBoxUnchecked');
	nowUnchecked.removeClass('accountBoxChecked');
}

/**
 * The user changed the value in the profile selection box. This will reload the login page with the new profile.
 * 
 * @param element dropdown box
 */
function loginProfileChanged(element) {
	location.href='login.php?useProfile=' + element.options[element.selectedIndex].value;
}

/**
 * Hides/unhides input fields for the login method.
 */
function configLoginMethodChanged() {
	selectLoginMethod = document.getElementsByName('loginMethod')[0];
	if ( selectLoginMethod.options[selectLoginMethod.selectedIndex].value == 'list' ) {
		jQuery('textarea[name=admins]').parent().parent().show();
		jQuery('input[name=loginSearchSuffix]').parent().parent().hide();
		jQuery('input[name=loginSearchFilter]').parent().parent().hide();
	}
	else {
		jQuery('textarea[name=admins]').parent().parent().hide();
		jQuery('input[name=loginSearchSuffix]').parent().parent().show();
		jQuery('input[name=loginSearchFilter]').parent().parent().show();
	}
}

