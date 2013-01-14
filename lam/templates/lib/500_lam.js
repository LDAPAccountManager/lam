/**

$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2013  Roland Gruber

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
 * Called when user clicks on a table row. This toggles the checkbox in the row.
 * 
 * @param box checkbox name
 */
function list_click(box) {
	var cbox = document.getElementsByName(box)[0];
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
    height -= 105;
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

/**
 * Submits the form by clicking on the given button if enter was pressed.
 * 
 * @param id button ID
 * @param e event
 * @returns Boolean result
 */
function SubmitForm(id, e) {
	if (e.keyCode == 13) {
		if (e.preventDefault) {
			e.preventDefault();
		}
		if (e.returnValue) {
			e.returnValue = false;
		}
		if (window.lastKeyCode) {
			// no submit if last key code was arrow key (browser autocompletion)
			if (window.lastKeyCode == 33 || window.lastKeyCode == 34 ||
				window.lastKeyCode == 38 || window.lastKeyCode == 40) {
				window.lastKeyCode = e.keyCode;
				return true;
			}
		}
		document.getElementsByName(id)[0].click();
		return false;
	}
	window.lastKeyCode = e.keyCode;
	return true;
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
		jQuery('input[name=loginSearchDN]').parent().parent().hide();
		jQuery('input[name=loginSearchPassword]').parent().parent().hide();
		jQuery('input[name=httpAuthentication]').parent().parent().hide();
	}
	else {
		jQuery('textarea[name=admins]').parent().parent().hide();
		jQuery('input[name=loginSearchSuffix]').parent().parent().show();
		jQuery('input[name=loginSearchFilter]').parent().parent().show();
		jQuery('input[name=loginSearchDN]').parent().parent().show();
		jQuery('input[name=loginSearchPassword]').parent().parent().show();
		jQuery('input[name=httpAuthentication]').parent().parent().show();
	}
}

/**
 * Shows the dialog to delete a profile.
 * 
 * @param title dialog title
 * @param okText text for Ok button
 * @param cancelText text for Cancel button
 * @param scope account type (e.g. user)
 * @param selectFieldName name of select box with profile name
 */
function profileShowDeleteDialog(title, okText, cancelText, scope, selectFieldName) {
	// get profile name
	var profileName = jQuery('[name=' + selectFieldName + ']').val();
	// update text
	jQuery('#deleteText').text(profileName);
	// update hidden input fields
	jQuery('#profileDeleteType').val(scope);
	jQuery('#profileDeleteName').val(profileName);
	var buttonList = {};
	buttonList[cancelText] = function() { jQuery(this).dialog("close"); };
	buttonList[okText] = function() { document.forms["deleteProfileForm"].submit(); };
	jQuery('#deleteProfileDialog').dialog({
		modal: true,
		title: title,
		dialogClass: 'defaultBackground',
		buttons: buttonList,
		width: 'auto'
	});
}

/**
 * Shows the dialog to create an automount map.
 * 
 * @param title dialog title
 * @param okText text for Ok button
 * @param cancelText text for Cancel button
 */
function automountShowNewMapDialog(title, okText, cancelText) {
	var buttonList = {};
	buttonList[cancelText] = function() { jQuery(this).dialog("close"); };
	buttonList[okText] = function() { document.forms["newAutomountMapDialogForm"].submit(); };
	jQuery('#newAutomountMapDialog').dialog({
		modal: true,
		title: title,
		dialogClass: 'defaultBackground',
		buttons: buttonList,
		width: 'auto'
	});
}

/**
 * Shows the dialog to change the password.
 * 
 * @param title dialog title
 * @param okText text for Ok button
 * @param cancelText text for Cancel button
 * @param randomText text for random password
 * @param ajaxURL URL used for AJAX request
 */
function passwordShowChangeDialog(title, okText, cancelText, randomText, ajaxURL) {
	var buttonList = {};
	buttonList[randomText] = function() { passwordHandleInput("true", ajaxURL); };
	buttonList[cancelText] = function() {
		jQuery('#passwordDialogMessageArea').html("");
		jQuery(this).dialog("close");
	};
	buttonList[okText] = function() { passwordHandleInput("false", ajaxURL); };
	jQuery('#passwordDialog').dialog({
		modal: true,
		title: title,
		dialogClass: 'defaultBackground',
		buttons: buttonList,
		width: 'auto'
	});
	// set focus on password field
	var myElement = document.getElementsByName('newPassword1')[0];
	myElement.focus();
}

/**
 * Manages the password change when a button is pressed.
 * 
 * @param random "true" if random password should be generated
 * @param ajaxURL URL used for AJAX request
 */
function passwordHandleInput(random, ajaxURL) {
	// get input values
	var modules = new Array();
	jQuery('#passwordDialog').find(':checked').each(function() {
		modules.push(jQuery(this).attr('name'));
	});
	var pwd1 = jQuery('#passwordDialog').find('[name=newPassword1]').val();
	var pwd2 = jQuery('#passwordDialog').find('[name=newPassword2]').val();
	var forcePasswordChange = jQuery('input[name=lamForcePasswordChange]').attr('checked');
	var sendMail = jQuery('input[name=lamPasswordChangeSendMail]').attr('checked');
	var pwdJSON = {
		"modules": modules,
		"password1": pwd1,
		"password2": pwd2,
		"random": random,
		"forcePasswordChange": forcePasswordChange,
		"sendMail": sendMail
	};
	// make AJAX call
	jQuery.post(ajaxURL, {jsonInput: pwdJSON}, function(data) {passwordHandleReply(data);}, 'json');
}

/**
 * Manages the server reply to a password change request.
 * 
 * @param data JSON reply
 */
function passwordHandleReply(data) {
	if (data.errorsOccured == "false") {
		jQuery('#passwordDialogMessageArea').html("");
		jQuery('#passwordDialog').dialog("close");
		jQuery('#passwordMessageArea').html(data.messages);
		if (data.forcePasswordChange) {
			jQuery('#forcePasswordChangeOption').attr('checked', 'checked');
		}
	}
	else {
		jQuery('#passwordDialogMessageArea').html(data.messages);
	}	
}

/**
 * Shows a general confirmation dialog and submits a form if the user accepted.
 * 
 * @param title dialog title
 * @param okText text for Ok button
 * @param cancelText text for Cancel button
 * @param dialogDiv div that contains dialog content
 * @param formName form to submit
 * @param resultField (hidden) input field whose value is set to ok/cancel when button is pressed
 */
function showConfirmationDialog(title, okText, cancelText, dialogDiv, formName, resultField) {
	var buttonList = {};
	buttonList[cancelText] = function() {
		if (resultField) {
			jQuery('#' + resultField).val('cancel');
		};
		jQuery(this).dialog("close");
	};
	buttonList[okText] = function() {
		if (resultField) {
			jQuery('#' + resultField).val('ok');
		};
		document.forms[formName].submit();
	};
	jQuery('#' + dialogDiv).dialog({
		modal: true,
		title: title,
		dialogClass: 'defaultBackground',
		buttons: buttonList,
		width: 'auto'
	});
	/* reattach dialog to form */
	jQuery('#' + dialogDiv).parent().appendTo(document.forms[formName]);
}

/**
 * Alines the elements with the given IDs to the same width.
 * 
 * @param elementIDs IDs
 */
function equalWidth(elementIDs) {
	var maxWidth = 0;
	for (var i = 0; i < elementIDs.length; ++i) {
		if (jQuery(elementIDs[i]).width() > maxWidth) {
			maxWidth = jQuery(elementIDs[i]).width();
		};
	}
	for (var i = 0; i < elementIDs.length; ++i) {
		jQuery(elementIDs[i]).css({'width': maxWidth - (jQuery(elementIDs[i]).outerWidth() - jQuery(elementIDs[i]).width())});
	}
}

/**
 * Shows the dialog to change the list settings.
 * 
 * @param title dialog title
 * @param okText text for Ok button
 * @param cancelText text for Cancel button
 * @param scope account type
 * @param selectFieldName name of select box with profile name
 * @param serverProfile profile name
 */
function showDistributionDialog(title, okText, cancelText, scope, type, selectFieldName, serverProfile) {
	// show dialog
	var buttonList = {};
	var dialogId = '';
	buttonList[cancelText] = function() { jQuery(this).dialog("close"); };
	
	if (type == 'export') {
		// show structure name to export
		jQuery('#exportName').text(jQuery('[name=' + selectFieldName + ']').val());
		dialogId = 'exportDialog';
		buttonList[okText] = function() { document.forms["exportDialogForm"].submit(); };
		jQuery('<input>').attr({
		    type: 'hidden',
		    name: 'exportProfiles[]',
		    value: serverProfile + '##' + jQuery('[name=' + selectFieldName + ']').val()
		}).appendTo('form');
		jQuery('<input>').attr({
		    type: 'hidden',
		    name: 'scope',
		    value: scope
		}).appendTo('form');
	} else if (type == 'import') {
		dialogId = 'importDialog_' + scope;
		buttonList[okText] = function() { document.forms["importDialogForm_" + scope].submit(); };
	}
	
	jQuery('#' + dialogId).dialog({
		modal: true,
		title: title,
		dialogClass: 'defaultBackground',
		buttons: buttonList,
		width: 'auto'
	});
	if (type == 'export') {
		equalWidth(new Array('#passwd', '#destServerProfiles'));
	} else if (type == 'import') {
		equalWidth(new Array('#passwd_' + scope, '#importProfiles_' + scope));
	}
}

