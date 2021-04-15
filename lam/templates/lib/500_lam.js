/**

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2021  Roland Gruber

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

window.lam = window.lam || {};

/**
 * Called when user clicks on a table row. This toggles the checkbox in the row.
 *
 * @param box checkbox name
 */
function list_click(box) {
	var cbox = document.getElementsByName(box)[0];
	if (cbox.checked) {
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
 * The user pressed a key in the page number box. On enter this will reload the list view with the new page.
 *
 * @param url target URL
 * @param e event
 */
function listPageNumberKeyPress(url, e) {
	var pageNumber = jQuery('#listNavPage').val();
	if (e.keyCode == 13) {
		if (e.preventDefault) {
			e.preventDefault();
		}
		location.href = url + '&page=' + pageNumber;
		return false;
	}
	return true;
}

/**
 * Shows the dialog to change the list settings.
 *
 * @param title dialog title
 * @param okText text for Ok button
 * @param cancelText text for Cancel button
 */
function listShowSettingsDialog(title, okText, cancelText) {
	var buttonList = {};
	buttonList[okText] = function() { document.forms["settingsDialogForm"].submit(); };
	buttonList[cancelText] = function() { jQuery(this).dialog("close"); };
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
 * Example: SubmitForm('apply_filter', event);
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
	jQuery(item).click(
		function() {
			if (jQuery(item).hasClass('imgExpanded')) {
				jQuery(item).animate({
					height: min
				});
			}
			else {
				jQuery(item).animate({
					height: max
				});
			}
			jQuery(item).toggleClass('imgExpanded');
		}
	);
}

/**
 * Selects/deselects all accounts on the page.
 */
function list_switchAccountSelection() {
	// set checkbox selection
	jQuery('input.accountBoxUnchecked').prop('checked', true);
	jQuery('input.accountBoxChecked').prop('checked', false);
	// switch CSS class
	var nowChecked = jQuery('.accountBoxUnchecked');
	var nowUnchecked = jQuery('.accountBoxChecked');
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
	buttonList[okText] = function() { document.forms["deleteProfileForm"].submit(); };
	buttonList[cancelText] = function() { jQuery(this).dialog("close"); };
	jQuery('#deleteProfileDialog').dialog({
		modal: true,
		title: title,
		dialogClass: 'defaultBackground',
		buttons: buttonList,
		width: 'auto'
	});
}

/**
 * Shows a simple dialog.
 *
 * @param title dialog title
 * @param okText text for Ok button (optional, submits form)
 * @param cancelText text for Cancel button
 * @param formID form ID
 * @param dialogDivID ID of div that contains dialog content
 */
function showSimpleDialog(title, okText, cancelText, formID, dialogDivID) {
	var buttonList = {};
	if (okText) {
		buttonList[okText] = function() { document.forms[formID].submit(); };
	}
	buttonList[cancelText] = function() { jQuery(this).dialog("close"); };
	jQuery('#' + dialogDivID).dialog({
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
 * @param tokenName name of CSRF token
 * @param tokenValue value of CSRF token
 */
function passwordShowChangeDialog(title, okText, cancelText, randomText, ajaxURL, tokenName, tokenValue) {
	var buttonList = {};
	buttonList[okText] = function() { passwordHandleInput("false", ajaxURL, tokenName, tokenValue); };
	buttonList[randomText] = function() { passwordHandleInput("true", ajaxURL, tokenName, tokenValue); };
	buttonList[cancelText] = function() {
		jQuery('#passwordDialogMessageArea').html("");
		jQuery(this).dialog("close");
	};
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
 * @param tokenName name of CSRF token
 * @param tokenValue value of CSRF token
 */
function passwordHandleInput(random, ajaxURL, tokenName, tokenValue) {
	// get input values
	var modules = new Array();
	jQuery('#passwordDialog').find(':checked').each(function() {
		modules.push(jQuery(this).prop('name'));
	});
	var pwd1 = jQuery('#passwordDialog').find('[name=newPassword1]').val();
	var pwd2 = jQuery('#passwordDialog').find('[name=newPassword2]').val();
	var forcePasswordChange = jQuery('input[name=lamForcePasswordChange]').prop('checked');
	var sendMail = jQuery('input[name=lamPasswordChangeSendMail]').prop('checked');
	var sendMailAlternateAddress = '';
	if (jQuery('#passwordDialog').find('[name=lamPasswordChangeSendMailAddress]')) {
		sendMailAlternateAddress = jQuery('#passwordDialog').find('[name=lamPasswordChangeSendMailAddress]').val();
	}
	var pwdJSON = {
		"modules": modules,
		"password1": pwd1,
		"password2": pwd2,
		"random": random,
		"forcePasswordChange": forcePasswordChange,
		"sendMail": sendMail,
		"sendMailAlternateAddress": sendMailAlternateAddress
	};
	var data = {jsonInput: pwdJSON};
	data[tokenName] = tokenValue;
	// make AJAX call
	jQuery.post(ajaxURL, data, function(dataReturned) {passwordHandleReply(dataReturned);}, 'json');
}

/**
 * Manages the server reply to a password change request.
 *
 * @param data JSON reply
 */
function passwordHandleReply(data) {
	if (data.errorsOccurred == "false") {
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
	buttonList[okText] = function() {
		jQuery('#' + dialogDiv).dialog('close');
		if (resultField) {
			jQuery('#' + resultField).val('ok');
		}
		appendDialogInputsToFormAndSubmit(dialogDiv, formName);
	};
	buttonList[cancelText] = function() {
		if (resultField) {
			jQuery('#' + resultField).val('cancel');
		}
		jQuery(this).dialog("close");
	};
	jQuery('#' + dialogDiv).dialog({
		modal: true,
		title: title,
		dialogClass: 'defaultBackground',
		buttons: buttonList,
		width: 'auto'
	});
}

/**
 * Appends the input fields of a dialog back to the form and submits it.
 *
 * @param dialogDiv ID of dialog div
 * @param formName name of form
 */
function appendDialogInputsToFormAndSubmit(dialogDiv, formName) {
	var inputs = jQuery('#' + dialogDiv + ' :input');
	inputs.each(function() {
		jQuery(this).addClass('hidden');
		jQuery(this).appendTo(document.forms[formName]);
    });
	document.forms[formName].submit();
}

/**
 * Shows a simple confirmation dialog.
 * If the user presses Cancel then the current action is stopped (event.preventDefault()).
 *
 * @param text dialog text
 * @param e event
 */
function confirmOrStopProcessing(text, e) {
	if (!confirm(text)) {
		if (e.preventDefault) {
			e.preventDefault();
		}
		if (e.returnValue) {
			e.returnValue = false;
		}
		return false;
	}
	return true;
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
		}
	}
	if (maxWidth < 5) {
		// no action if invalid width value (e.g. because of hidden tab)
		return;
	}
	for (var elementId = 0; elementId < elementIDs.length; ++elementId) {
		jQuery(elementIDs[elementId]).css({
			'width': maxWidth - (jQuery(elementIDs[elementId]).outerWidth() - jQuery(elementIDs[elementId]).width())
		});
	}
}

/**
 * Alines the elements with the given IDs to the same height.
 *
 * @param elementIDs IDs
 */
function equalHeight(elementIDs) {
	var max = 0;
	for (var i = 0; i < elementIDs.length; ++i) {
		if (jQuery(elementIDs[i]).height() > max) {
			max = jQuery(elementIDs[i]).height();
		}
	}
	for (var elementId = 0; elementId < elementIDs.length; ++elementId) {
		jQuery(elementIDs[elementId]).css({
			'height': max - (jQuery(elementIDs[elementId]).outerHeight() - jQuery(elementIDs[elementId]).height())
		});
	}
}

window.lam.profilePdfEditor = window.lam.profilePdfEditor || {};

/**
 * Shows the dialog to import/export account/PDF profiles.
 *
 * @param title dialog title
 * @param okText text for Ok button
 * @param cancelText text for Cancel button
 * @param typeId account type
 * @param selectFieldName name of select box with profile name
 */
window.lam.profilePdfEditor.showDistributionDialog = function(title, okText, cancelText, typeId, type, selectFieldName) {
	// show dialog
	var buttonList = {};
	var dialogId = '';

	if (type == 'export') {
		jQuery('#name_' + typeId).val(jQuery('#' + selectFieldName).val());
		dialogId = 'exportDialog_' + typeId;
		buttonList[okText] = function() { document.forms["exportDialogForm_" + typeId].submit(); };
	} else if (type == 'import') {
		dialogId = 'importDialog_' + typeId;
		buttonList[okText] = function() { document.forms["importDialogForm_" + typeId].submit(); };
	}
	buttonList[cancelText] = function() { jQuery(this).dialog("close"); };

	jQuery('#' + dialogId).dialog({
		modal: true,
		title: title,
		dialogClass: 'defaultBackground',
		buttons: buttonList,
		width: 'auto'
	});
	if (type == 'export') {
		equalWidth(new Array('#passwd_' + typeId, '#destServerProfiles_' + typeId));
	} else if (type == 'import') {
		equalWidth(new Array('#passwd_' + typeId, '#importProfiles'));
	}
}

/**
 * Shows the dialog to export PDF logos.
 *
 * @param title dialog title
 * @param okText text for Ok button
 * @param cancelText text for Cancel button
 */
window.lam.profilePdfEditor.showPdfLogoExportDialog = function(title, okText, cancelText) {
	var selectedLogo = document.getElementById('logo').value;
	document.getElementById('exportLogoName').value = selectedLogo;
	var buttonList = {};
	buttonList[okText] = function() {
		document.forms['logoExportForm'].submit();
	};
	buttonList[cancelText] = function() {
		jQuery(this).dialog("close");
	};
	jQuery('#logoExportDiv').dialog({
		modal: true,
		title: title,
		dialogClass: 'defaultBackground',
		buttons: buttonList,
		width: 'auto'
	});
}

/**
 * Shows the dialog to import PDF logos.
 *
 * @param title dialog title
 * @param okText text for Ok button
 * @param cancelText text for Cancel button
 */
window.lam.profilePdfEditor.showPdfLogoImportDialog = function(title, okText, cancelText) {
	var buttonList = {};
	buttonList[okText] = function() {
		document.forms['logoImportForm'].submit();
	};
	buttonList[cancelText] = function() {
		jQuery(this).dialog("close");
	};
	jQuery('#logoImportDiv').dialog({
		modal: true,
		title: title,
		dialogClass: 'defaultBackground',
		buttons: buttonList,
		width: 'auto'
	});
}

/**
 * Stores the current scroll position in the form.
 *
 * @param formName ID of form
 */
function saveScrollPosition(formName) {
	var top = jQuery(window).scrollTop();
	var left = jQuery(window).scrollLeft();
	jQuery('<input>').attr({
	    type: 'hidden',
	    name: 'scrollPositionTop',
	    value: top
	}).appendTo(jQuery('#' + formName));
	jQuery('<input>').attr({
	    type: 'hidden',
	    name: 'scrollPositionLeft',
	    value: left
	}).appendTo(jQuery('#' + formName));
}

/**
 * Shows the dialog to create a DNS zone.
 *
 * @param title dialog title
 * @param okText text for Ok button
 * @param cancelText text for Cancel button
 */
function bindShowNewZoneDialog(title, okText, cancelText) {
	var buttonList = {};
	buttonList[okText] = function() { document.forms["newBindZoneDialogForm"].submit(); };
	buttonList[cancelText] = function() { jQuery(this).dialog("close"); };
	jQuery('#newBindZoneDialog').dialog({
		modal: true,
		title: title,
		dialogClass: 'defaultBackground',
		buttons: buttonList,
		width: 'auto'
	});
}


// creates the tooltips for help buttons
jQuery(document).ready(
	function() {
		jQuery(document).tooltip({
			items: "[helpdata]",
			content: function() {
				var element = $(this);
				var helpString = "<table><tr><th class=\"help\">";
				helpString += element.attr("helptitle");
				helpString += "</th></tr><td class=\"help\">";
				helpString += element.attr("helpdata");
				helpString += "</td></tr></table>";
				return helpString;
			}
		})
	}
);

/**
 * Checks if the given field has the same value as the reference field.
 * Field is marked red if different and green if equal.
 *
 * @param fieldID ID of field to check
 * @param fieldIDReference ID of reference field
 */
function checkFieldsHaveSameValues(fieldID, fieldIDReference) {
	var field = jQuery('#' + fieldID);
	var fieldRef = jQuery('#' + fieldIDReference);
	var check =
		function() {
			var value = field.val();
			var valueRef = fieldRef.val();
			if ((value == '') && (valueRef == '')) {
				field.removeClass('markFail');
				field.removeClass('markOk');
			}
			else {
				if (value == valueRef) {
					field.removeClass('markFail');
					field.addClass('markOk');
				}
				else {
					field.addClass('markFail');
					field.removeClass('markOk');
				}
			}
		}
	jQuery(field).keyup(check);
	jQuery(fieldRef).keyup(check);
}

/**
 * Checks if the value of the given password field matches LAM's password policy.
 * Field is marked red if fail and green if ok.
 *
 * @param fieldID ID of field to check
 * @param tokenName name of CSRF token
 * @param tokenValue value of CSRF token
 */
function checkPasswordStrength(fieldID, ajaxURL, tokenName, tokenValue) {
	var field = jQuery('#' + fieldID);
	var check =
		function() {
			var value = field.val();
			var pwdJSON = {
					"password": value
			};
			var data = {jsonInput: pwdJSON};
			data[tokenName] = tokenValue;
			// make AJAX call
			jQuery.post(ajaxURL + "&function=passwordStrengthCheck", data, function(dataReturned) {checkPasswordStrengthHandleReply(dataReturned, fieldID);}, 'json');
		};
	jQuery(field).keyup(check);
}

/**
 * Manages the server reply to a password strength check request.
 *
 * @param data JSON reply
 * @param fieldID input field ID
 */
function checkPasswordStrengthHandleReply(data, fieldID) {
	var field = jQuery('#' + fieldID);
	if (data.result === true) {
		field.removeClass('markFail');
		field.addClass('markOk');
		field.prop('title', '');
	}
	else if (field.val() == '') {
		field.removeClass('markFail');
		field.removeClass('markOk');
	}
	else {
		field.addClass('markFail');
		field.removeClass('markOk');
		field.prop('title', data.result);
	}
}

/**
 * Updates the positions of a htmlSortable list in a hidden input field.
 * The positions must be separated by comma (e.g. "0,1,2,3").
 *
 * @param id HTML ID of hidden input field
 * @param oldPos old position
 * @param newPos new position
 */
function updateModulePositions(id, oldPos, newPos) {
	var positions = jQuery('#' + id).val().split(',');
	if (newPos > oldPos) {
		var save = positions[oldPos];
		for (var i = oldPos; i < newPos; i++) {
			positions[i] = positions[i + 1];
		}
		positions[newPos] = save;
	}
	if (newPos < oldPos) {
		var oldPosition = positions[oldPos];
		for (var position = oldPos; position > newPos; position--) {
			positions[position] = positions[position - 1];
		}
		positions[newPos] = oldPosition;
	}
	jQuery('#' + id).val(positions.join(','));
}

window.lam.filterSelect = window.lam.filterSelect || {};

/**
 * Filters a select box by the value of the filter input field.
 *
 * @param filterInput ID of input field for filter
 * @param select ID of select box to filter
 * @param event key event
 */
window.lam.filterSelect.activate = function (filterInput, select, event) {
	var inputField = jQuery('#' + filterInput);
	var selectField = jQuery('#' + select);
	if (selectField.hasClass('lam-dynamicOptions')) {
		window.lam.filterSelect.filterDynamic(inputField, selectField);
	}
	else {
		window.lam.filterSelect.filterStandard(inputField, selectField);
	}
}

/**
 * Filters a normal select field.
 *
 * @param inputField input field with filter value
 * @param selectField select field
 */
window.lam.filterSelect.filterStandard = function(inputField, selectField) {
	// if values were not yet saved, save them
	if (!selectField.data('options')) {
		var options = {};
		selectField.find('option').each(function() {
			options[$(this).val()] = {selected: this.selected, text: $(this).text()};
		});
		selectField.data('options', options);
	}
	// save selected values
	var storedOptions = selectField.data('options');
	selectField.find('option').each(function() {
		storedOptions[$(this).val()].selected = this.selected;
	});
	selectField.data('options', storedOptions);
	// get matching values
	selectField.empty().scrollTop(0);
	var search = jQuery.trim(inputField.val());
	var regex = new RegExp(search,'gi');
	jQuery.each(storedOptions, function(index, option) {
		if(option.text.match(regex) !== null) {
			var newOption = jQuery('<option>');
			newOption.text(option.text).val(index);
			if (option.selected) {
				newOption.attr('selected', 'selected')
			}
			selectField.append(newOption);
		}
	});
}

/**
 * Filters a select field with dynamic scrolling.
 *
 * @param inputField input field with filter value
 * @param selectField select field
 */
window.lam.filterSelect.filterDynamic = function(inputField, selectField) {
	var optionsOrig = selectField.data('dynamic-options-orig');
	if (optionsOrig === undefined) {
		selectField.data('dynamic-options-orig', selectField.data('dynamic-options'));
		optionsOrig = selectField.data('dynamic-options-orig');
	}
	selectField.empty().scrollTop(0);
	var newOptions = [];
	// get matching values
	var search = jQuery.trim(inputField.val());
	var regex = new RegExp(search,'gi');
	jQuery.each(optionsOrig, function(i) {
		var option = optionsOrig[i];
		if(option.label.match(regex) !== null) {
			newOptions.push(option);
		}
	});
	selectField.data('dynamic-options', newOptions);
	window.lam.dynamicSelect.initSelect(selectField);
}

window.lam.upload = window.lam.upload || {};

/**
 * Continues a CSV file upload.
 *
 * @param url URL where to get status JSON
 * @param tokenName name of CSRF token
 * @param tokenValue value of CSRF token
 */
window.lam.upload.continueUpload = function(url, tokenName, tokenValue) {
	var data = {
		jsonInput: ''
	};
	data[tokenName] = tokenValue;
	jQuery.ajax({
		url: url,
		method: 'POST',
		data: data
	})
	.done(function(jsonData){
		if (!jsonData.accountsFinished) {
			window.lam.upload.printBasicStatus(jsonData);
		}
		else if (!jsonData.postActionsFinished) {
			window.lam.upload.printPostActionStatus(jsonData);
		}
		else if (!jsonData.pdfFinished) {
			window.lam.upload.printPDFStatus(jsonData);
		}
		// next call if not finished
		if (!jsonData.allDone) {
			window.lam.upload.continueUpload(url, tokenName, tokenValue);
		}
		else {
			window.lam.upload.uploadDone(jsonData);
		}
	});
};

/**
 * Prints the upload status when accounts are still being created.
 *
 * @param jsonData status JSON
 */
window.lam.upload.printBasicStatus = function(jsonData) {
	var htmlOut = '<div class="title">';
	htmlOut += '<h2 class="titleText">' + jsonData.title + '</h2>';
	htmlOut += '</div>';
	htmlOut += '<div id="progressbarGeneral"></div>';
	jQuery('#uploadContent').html(htmlOut);
	jQuery('#progressbarGeneral').progressbar({
		value: jsonData.accountsProgress
	});
};

/**
 * Prints the upload status when post actions run.
 *
 * @param jsonData status JSON
 */
window.lam.upload.printPostActionStatus = function(jsonData) {
	var htmlOut = '<div class="title">';
	htmlOut += '<h2 class="titleText">' + jsonData.title + '</h2>';
	htmlOut += '</div>';
	htmlOut += '<div id="progressbarGeneral"></div>';
	if (jsonData.postActionsTitle) {
		htmlOut += '<h2>' + jsonData.postActionsTitle + '</h2>';
		htmlOut += '<div id="progressbarPostActions"></div>';
	}
	jQuery('#uploadContent').html(htmlOut);
	jQuery('#progressbarGeneral').progressbar({
		value: 100
	});
	if (jsonData.postActionsTitle) {
		jQuery('#progressbarPostActions').progressbar({
			value: jsonData.postActionsProgress
		});
	}
};

/**
 * Prints the upload status when PDFs are generated.
 *
 * @param jsonData status JSON
 */
window.lam.upload.printPDFStatus = function(jsonData) {
	var htmlOut = '<div class="title">';
	htmlOut += '<h2 class="titleText">' + jsonData.title + '</h2>';
	htmlOut += '</div>';
	htmlOut += '<div id="progressbarGeneral"></div>';
	htmlOut += '<h2>' + jsonData.titlePDF + '</h2>';
	htmlOut += '<div id="progressbarPDF"></div>';
	jQuery('#uploadContent').html(htmlOut);
	jQuery('#progressbarGeneral').progressbar({
		value: 100
	});
	jQuery('#progressbarPDF').progressbar({
		value: jsonData.pdfProgress
	});
};

/**
 * Upload finished, check for errors.
 *
 * @param jsonData status JSON
 */
window.lam.upload.uploadDone = function(jsonData) {
	if (jsonData.errorHtml) {
		var htmlOut = '<div class="subTitle">';
		htmlOut += '<h4  class="subTitleText">' + jsonData.titleErrors + '</h4>';
		htmlOut += '</div>';
		htmlOut += jsonData.errorHtml;
		jQuery('#uploadContent').html(htmlOut);
	}
	else {
		top.location.href = '../lists/list.php?type=' + jsonData.typeId + '&uploadAllOk';
	}
};

window.lam.gui = window.lam.gui || {};

/**
 * Resizes input fields etc. when they are marked as equally high.
 */
window.lam.gui.equalHeight = function() {
	var maxHeight = 20;
	jQuery('.lamEqualHeightTabContent').each(function() {
		if (jQuery(this).height() > maxHeight) {
			maxHeight = jQuery(this).height() + 20;
		}
	});
	jQuery('.lamEqualHeightTabContent').each(function() {
		jQuery(this).css({'height': maxHeight});
	});
};

window.lam.form = window.lam.form || {};

/**
 * Trims all marked input elements on form submission.
 */
window.lam.form.autoTrim = function() {
	jQuery('form').submit(function(e) {
		jQuery('.lam-autotrim').each(function() {
			this.value = String(this.value).trim();
		});
	});
};

window.lam.dialog = window.lam.dialog || {};

/**
 * Shows a dialog message.
 *
 * @param title dialog title
 * @param okText ok button text
 * @param divId DIV id with dialog content
 * @param callbackFunction callback function (optional)
 */
window.lam.dialog.showMessage = function(title, okText, divId, callbackFunction) {
    var buttonList = {};
    buttonList[okText] = function() {
    	jQuery(this).dialog("close");
    	if (callbackFunction) {
    		callbackFunction();
		}
    };
    jQuery('#' + divId).dialog({
		modal: true,
		title: title,
		dialogClass: 'defaultBackground',
		buttons: buttonList,
		width: 'auto'
	});
};

window.lam.account = window.lam.account || {};

/**
 * Adds a listener on the link to set default profile.
 */
window.lam.account.addDefaultProfileListener = function() {
	var defaultProfileLink = jQuery('#lam-make-default-profile');
	if (defaultProfileLink) {
		defaultProfileLink.click(function() {
			var link = $(this);
			var typeId = link.data('typeid');
			var name = link.data('name');
			var okText = link.data('ok');
			var date = new Date();
	        date.setTime(date.getTime() + (365*24*60*60*1000));
	        document.cookie = 'defaultProfile_' + typeId + '=' + name + '; expires=' + date.toUTCString();
	        window.lam.dialog.showMessage(null, okText, 'lam-make-default-profile-dlg');
		});
	}
};

window.lam.tools = window.lam.tools || {};

/**
 * Adds a listener on select lists to store the last value as default in local storage.
 * Select lists need to be marked with class "lam-save-selection".
 */
window.lam.tools.addSavedSelectListener = function() {
	if (!window.localStorage) {
		return;
	}
	var selects = jQuery('.lam-save-selection');
	if (selects) {
		selects.each(function() {
			var select = jQuery(this);
			var name = select.attr('name');
			var storageKey = 'lam_selectionStore_' + name;
			// load value from local storage
			var storageValue = window.localStorage.getItem(storageKey);
			if (storageValue) {
				var option = select.find('option[text="' + storageValue + '"]');
				if (option) {
					select.val(storageValue);
				}
			}
			// add change listener
			select.on('change', function() {
				var selectedValue = this.value;
				window.localStorage.setItem(storageKey, selectedValue);
			});
		});
	}
};

/**
 * Activates tabs.
 */
window.lam.tools.activateTab = function() {
	jQuery('.lam-active-tab').addClass('ui-tabs-active ui-state-active');
};

/**
 * Sets the focus on the initial field.
 */
window.lam.tools.setInitialFocus = function() {
	jQuery('.lam-initial-focus').focus();
};

window.lam.tools.webcam = window.lam.tools.webcam || {};

/**
 * Initializes the webcam capture.
 */
window.lam.tools.webcam.init = function() {
	var contentDiv = jQuery('#lam_webcam_div');
	if (contentDiv.length === 0) {
		return;
	}
	if (navigator.mediaDevices && navigator.mediaDevices.enumerateDevices) {
		navigator.mediaDevices.enumerateDevices()
			.then(function(mediaDevices) {
				for (var i = 0; i < mediaDevices.length; i++) {
					var mediaDevice = mediaDevices[i];
					if (mediaDevice.kind === 'videoinput') {
						contentDiv.show();
					}
				};
			});
	}
};

/**
 * Starts the webcam capture.
 */
window.lam.tools.webcam.capture = function(event) {
	event.preventDefault();
	var video = document.getElementById('lam-webcam-video');
	var msg = jQuery('.lam-webcam-message');
	msg.hide();
	navigator.mediaDevices.getUserMedia({
			video: {
				facingMode: 'user',
				width: { min: 1024, ideal: 1280, max: 1920 },
				height: { min: 576, ideal: 720, max: 1080 }
			},
			audio: false
		})
		.then(function(stream) {
			video.srcObject = stream;
			video.play();
			window.lam.tools.webcamStream = stream;
			jQuery('#btn_lam-webcam-capture').hide();
			jQuery('.btn-lam-webcam-upload').show();
			jQuery('#lam-webcam-video').show();
		})
		.catch(function(err) {
			msg.find('.statusTitle').text(err);
			msg.show();
		});
	return false;
}

/**
 * Starts the webcam upload.
 */
window.lam.tools.webcam.upload = function() {
	var form = jQuery('#lam-webcam-canvas').closest('form');
	var canvasData = window.lam.tools.webcam.prepareData();
	var canvasDataInput = jQuery("<input></input>");
	canvasDataInput.attr('name', 'webcamData');
	canvasDataInput.attr('id', 'webcamData');
	canvasDataInput.attr('type', 'hidden');
	canvasDataInput.attr('value', canvasData);
	form.append(canvasDataInput);
	form.submit();
	return true;
}

/**
 * Starts the webcam upload.
 *
 * @param event click event
 * @param tokenName security token name
 * @param tokenValue security token value
 * @param moduleName module name
 * @param scope account type
 * @param uploadErrorMessage error message if upload fails
 * @param contentId id of content to replace
 */
window.lam.tools.webcam.uploadSelfService = function(event, tokenName, tokenValue, moduleName, scope, uploadErrorMessage, contentId) {
	event.preventDefault();
	var msg = jQuery('.lam-webcam-message');
	var canvasData = window.lam.tools.webcam.prepareData();
	var data = {
		webcamData: canvasData
	};
	data[tokenName] = tokenValue;
	jQuery.ajax({
		url: '../misc/ajax.php?selfservice=1&action=ajaxPhotoUpload'
			+ '&module=' + moduleName + '&scope=' + scope,
		method: 'POST',
		data: data
	})
	.done(function(jsonData) {
		if (jsonData.success) {
			if (jsonData.html) {
				jQuery('#' + contentId).html(jsonData.html);
				window.lam.tools.webcam.init();
			}
			return false;
		}
		else if (jsonData.error) {
			msg.find('.statusTitle').text(jsonData.error);
			msg.show();
		}
	})
	.fail(function() {
		msg.find('.statusTitle').text(errorMessage);
		msg.show();
	});
	jQuery('#btn_lam-webcam-capture').show();
	jQuery('.btn-lam-webcam-upload').hide();
	return false;
}

/**
 * Starts the webcam upload.
 *
 * @return webcam data as string
 */
window.lam.tools.webcam.prepareData = function() {
	var canvas = document.getElementById('lam-webcam-canvas');
	var video = document.getElementById('lam-webcam-video');
	canvas.setAttribute('width', video.videoWidth);
	canvas.setAttribute('height', video.videoHeight);
	var context = canvas.getContext('2d');
	context.drawImage(video, 0, 0, video.videoWidth, video.videoHeight);
	var canvasData = canvas.toDataURL("image/png");
	video.pause();
	var tracks = window.lam.tools.webcamStream.getTracks();
	for (var i = 0; i < tracks.length; i++) {
		tracks[i].stop();
	}
	jQuery(canvas).hide();
	jQuery(video).hide();
	return canvasData;
}

window.lam.tools.schema = window.lam.tools.schema || {};

/**
 * Adds the onChange listener to schema selections.
 */
window.lam.tools.schema.select = function() {
	var select = jQuery('#lam-schema-select');
	var display = select.data('display');
	select.change(function() {
		var value = this.value;
		document.location = 'schema.php?display=' + display + '&sel=' + value;
	});
};

window.lam.importexport = window.lam.importexport || {};

/**
 * Starts the import process.
 *
 * @param tokenName name of CSRF token
 * @param tokenValue value of CSRF token
 */
window.lam.importexport.startImport = function(tokenName, tokenValue) {
	jQuery(document).ready(function() {
		jQuery('#progressbarImport').progressbar();
		var output = jQuery('#importResults');
		var data = {
			jsonInput: ''
		};
		data[tokenName] = tokenValue;
		jQuery.ajax({
			url: '../misc/ajax.php?function=import',
			method: 'POST',
			data: data
		})
		.done(function(jsonData){
			if (jsonData.data && (jsonData.data != '')) {
				output.append(jsonData.data);
			}
			if (jsonData.status == 'done') {
				jQuery('#progressbarImport').hide();
				jQuery('#btn_submitImportCancel').hide();
				jQuery('#statusImportInprogress').hide();
				jQuery('#statusImportDone').show();
				jQuery('.newimport').show();
			}
			else if (jsonData.status == 'failed') {
				jQuery('#btn_submitImportCancel').hide();
				jQuery('#statusImportInprogress').hide();
				jQuery('#statusImportFailed').show();
				jQuery('.newimport').show();
			}
			else {
				jQuery('#progressbarImport').progressbar({
					value: jsonData.progress
				});
				window.lam.import.startImport(tokenName, tokenValue);
			}
		});
	});
};

/**
 * Starts the export process.
 *
 * @param tokenName name of CSRF token
 * @param tokenValue value of CSRF token
 */
window.lam.importexport.startExport = function(tokenName, tokenValue) {
	jQuery(document).ready(function() {
		jQuery('#progressbarExport').progressbar({value: 50});
		var output = jQuery('#exportResults');
		var data = {
			jsonInput: ''
		};
		data[tokenName] = tokenValue;
		data['baseDn'] = jQuery('#baseDn').val();
		data['searchScope'] = jQuery('#searchScope').val();
		data['filter'] = jQuery('#filter').val();
		data['attributes'] = jQuery('#attributes').val();
		data['format'] = jQuery('#format').val();
		data['ending'] = jQuery('#ending').val();
		data['includeSystem'] = jQuery('#includeSystem').val();
		data['saveAsFile'] = jQuery('#saveAsFile').val();
		jQuery.ajax({
			url: '../misc/ajax.php?function=export',
			method: 'POST',
			data: data
		})
		.done(function(jsonData){
			if (jsonData.data && (jsonData.data != '')) {
				output.append(jsonData.data);
			}
			if (jsonData.status == 'done') {
				jQuery('#progressbarExport').hide();
				jQuery('#btn_submitExportCancel').hide();
				jQuery('#statusExportInprogress').hide();
				jQuery('#statusExportDone').show();
				jQuery('.newexport').show();
				if (jsonData.output) {
					jQuery('#exportResults > pre').text(jsonData.output);
				}
				else if (jsonData.file) {
					window.open(jsonData.file, '_blank');
				}
			}
			else {
				jQuery('#progressbarExport').hide();
				jQuery('#btn_submitExportCancel').hide();
				jQuery('#statusExportInprogress').hide();
				jQuery('#statusExportFailed').show();
				jQuery('.newexport').show();
			}
		})
		.fail(function() {
			jQuery('#progressbarExport').hide();
			jQuery('#btn_submitExportCancel').hide();
			jQuery('#statusExportInprogress').hide();
			jQuery('#statusExportFailed').show();
			jQuery('.newexport').show();
		});
	});
};

window.lam.html = window.lam.html || {};

/**
 * Shows a DN selection for the given input field.
 *
 * @param fieldId id of input field
 * @param title title of dialog
 * @param okText ok button text
 * @param cancelText cancel button text
 * @param tokenName CSRF token name
 * @param tokenValue CSRF token value
 */
window.lam.html.showDnSelection = function(fieldId, title, okText, cancelText, tokenName, tokenValue) {
	var field = jQuery('#' + fieldId);
	var fieldDiv = jQuery('#dlg_' + fieldId);
	if (fieldDiv.length == 0) {
		jQuery('body').append(jQuery('<div class="hidden" id="dlg_' + fieldId + '"></div>'));
	}
	var dnValue = field.val();
	var data = {
		jsonInput: ''
	};
	data[tokenName] = tokenValue;
	data['fieldId'] = fieldId;
	data['dn'] = dnValue;
	jQuery.ajax({
		url: '../misc/ajax.php?function=dnselection',
		method: 'POST',
		data: data
	})
	.done(function(jsonData) {
		jQuery('#dlg_' + fieldId).html(jsonData.dialogData);
		var buttonList = {};
		buttonList[cancelText] = function() { jQuery(this).dialog("destroy"); };
		jQuery('#dlg_' + fieldId).dialog({
			modal: true,
			title: title,
			dialogClass: 'defaultBackground',
			buttons: buttonList,
			width: 'auto',
			maxHeight: 600,
			position: {my: 'center', at: 'center', of: window}
		});
	});
};

/**
 * Selects the DN from dialog.
 *
 * @param el ok button in dialog
 * @param fieldId field id of input field
 * @returns false
 */
window.lam.html.selectDn = function(el, fieldId) {
	var field = jQuery('#' + fieldId);
	var dn = jQuery(el).parents('.row').data('dn');
	field.val(dn);
	jQuery('#dlg_' + fieldId).dialog("destroy");
	return false;
}

/**
 * Updates the DN selection.
 *
 * @param el element
 * @param fieldId field id of dialog
 * @param tokenName CSRF token name
 * @param tokenValue CSRF token value
 */
window.lam.html.updateDnSelection = function(el, fieldId, tokenName, tokenValue) {
	var fieldDiv = jQuery('#dlg_' + fieldId);
	var dn = jQuery(el).parents('.row').data('dn');
	var data = {
		jsonInput: ''
	};
	data[tokenName] = tokenValue;
	data['fieldId'] = fieldId;
	data['dn'] = dn;
	jQuery.ajax({
		url: '../misc/ajax.php?function=dnselection',
		method: 'POST',
		data: data
	})
	.done(function(jsonData) {
		jQuery('#dlg_' + fieldId).html(jsonData.dialogData);
		jQuery(fieldDiv).dialog({
		    position: {my: 'center', at: 'center', of: window}
		});
	})
	.fail(function() {
		jQuery(fieldDiv).dialog("close");
	});
}

/**
 * Activates the lightboxes on images.
 */
window.lam.html.activateLightboxes = function() {
	jQuery('.lam-lightbox').magnificPopup({
		type:'image',
		zoom: {
		    enabled: true
		}
	});
};

/**
 * Prevents enter key on input fields with class "lam-prevent-enter".
 */
window.lam.html.preventEnter = function() {
	jQuery('.lam-prevent-enter').keypress(function (event) {
	    if (event.keyCode === 10 || event.keyCode === 13) {
	        event.preventDefault();
	    }
	});
}

window.lam.dynamicSelect = window.lam.dynamicSelect || {};

/**
 * Activates dynamic selection for all marked select fields.
 */
window.lam.dynamicSelect.activate = function() {
	var dynamicSelects = jQuery('.lam-dynamicOptions');
	dynamicSelects.each(function() {
		var selectField = jQuery(this);
		window.lam.dynamicSelect.initSelect(selectField);
	});
}

/**
 * Sets up a select field for dynamic scrolling.
 *
 * @param selectField select
 */
window.lam.dynamicSelect.initSelect = function(selectField) {
	selectField.data('option-height', selectField.find("option").height());
	selectField.data('select-height', selectField.height());
	selectField.data('select-last-scroll-top', 0);
	selectField.data('select-current-scroll', 0);
	selectField.html('');
	var options = selectField.data('dynamic-options');
	var maxOptions = 3000;
	var numOfOptionBeforeToLoadNextSet = 10;
	var numberOfOptionsToLoad = 200;
	for (var i = 0; i < maxOptions; i++) {
		selectField.append(window.lam.dynamicSelect.createOption(options[i], i));
	}
	if (options.length > maxOptions) {
		// activate scrolling logic only if enough options are set
		selectField.scroll(function(event) {
			window.lam.dynamicSelect.onScroll(selectField, event, maxOptions, numOfOptionBeforeToLoadNextSet, numberOfOptionsToLoad);
		});
	}
}

/**
 * Creates an option field inside the select.
 *
 * @param data option data
 * @param index index in list of all options
 * @returns option
 */
window.lam.dynamicSelect.createOption = function(data, index) {
	var newOption = jQuery(document.createElement("option"));
	newOption.attr('value', data.value);
	newOption.data('index', index);
	newOption.text(data.label);
	return newOption;
}

/**
 * Onscroll event.
 *
 * @param selectField select field
 * @param event event
 * @param maxOptions maximum options to show
 * @param numOfOptionBeforeToLoadNextSet number of options to reach before end of list
 * @param numberOfOptionsToLoad number of options to add
 */
window.lam.dynamicSelect.onScroll = function(selectField, event, maxOptions, numOfOptionBeforeToLoadNextSet, numberOfOptionsToLoad) {
	var scrollTop = selectField.scrollTop();
	var totalHeight = selectField.find("option").length * selectField.data('option-height');
	var lastScrollTop = selectField.data('select-last-scroll-top');
	var selectBoxHeight = selectField.data('select-height');
	var singleOptionHeight = selectField.data('option-height');
	var currentScroll = scrollTop + selectBoxHeight;
	selectField.data('select-current-scroll-top', scrollTop);
	if ((scrollTop >= lastScrollTop)
			&& ((currentScroll + (numOfOptionBeforeToLoadNextSet * singleOptionHeight)) >= totalHeight)) {
		window.lam.dynamicSelect.loadNextOptions(selectField, maxOptions, numberOfOptionsToLoad);
	}
	else if ((scrollTop <= lastScrollTop)
			&& ((scrollTop - (numOfOptionBeforeToLoadNextSet * singleOptionHeight)) <= 0)) {
		window.lam.dynamicSelect.loadPreviousOptions(selectField, maxOptions, numberOfOptionsToLoad);
	}
	selectField.data('select-last-scroll-top', scrollTop);
}

/**
 * Loads the next bunch of options at the end.
 *
 * @param selectField select field
 * @param maxOptions maximum options to show
 * @param numberOfOptionsToLoad number of options to add
 */
window.lam.dynamicSelect.loadNextOptions = function(selectField, maxOptions, numberOfOptionsToLoad) {
	var selectBoxHeight = selectField.data('select-height');
	var singleOptionHeight = selectField.data('option-height');
	var currentScrollPosition = selectField.data('select-current-scroll-top') + selectBoxHeight;
	var options = selectField.data('dynamic-options');
	var lastIndex = selectField.children().last().data('index');
	for (var toAdd = 0; toAdd < numberOfOptionsToLoad; toAdd++) {
		var addPos = lastIndex + 1 + toAdd;
		if (options[addPos] === undefined) {
			break;
		}
		selectField.append(window.lam.dynamicSelect.createOption(options[addPos], addPos));
	}
	var numberOfOptions = selectField.children().length;
	var toRemove = numberOfOptions - maxOptions;
	if (toRemove > 0) {
		selectField.children().slice(0, toRemove).remove();
	}
	else {
		toRemove = 0;
	}
	selectField.scrollTop(currentScrollPosition - selectBoxHeight - (toRemove * singleOptionHeight));
}

/**
 * Loads the next bunch of options at the beginning.
 *
 * @param selectField select field
 * @param maxOptions maximum options to show
 * @param numberOfOptionsToLoad number of options to add
 */
window.lam.dynamicSelect.loadPreviousOptions = function(selectField, maxOptions, numberOfOptionsToLoad) {
	var singleOptionHeight = selectField.data('option-height');
	var currentScrollPosition = selectField.data('select-current-scroll-top');
	var options = selectField.data('dynamic-options');
	var lastIndex = selectField.children().first().data('index');
	var added = 0;
	for (var toAdd = 0; toAdd < numberOfOptionsToLoad; toAdd++) {
		var addPos = lastIndex - 1 - toAdd;
		if (options[addPos] === undefined) {
			break;
		}
		added++;
		selectField.prepend(window.lam.dynamicSelect.createOption(options[addPos], addPos));
	}
	var numberOfOptions = selectField.children().length;
	var toRemove = numberOfOptions - maxOptions;
	if (toRemove > 0) {
		selectField.children().slice(maxOptions).remove();
	}
	selectField.scrollTop(currentScrollPosition + (added * singleOptionHeight));
}

window.lam.selfservice = window.lam.selfservice || {};

/**
 * Deletes a value of a multi-value field.
 *
 * @param fieldNamePrefix prefix of input field name
 * @param delButton delete button that was clicked
 */
window.lam.selfservice.delMultiValue = function(fieldNamePrefix, delButton) {
	var fields = jQuery("input[name^='" + fieldNamePrefix + "']");
	var isOnlyOneField = (fields.length === 1);
	if (!isOnlyOneField) {
		// move add button if present
		var addButton = jQuery(delButton).siblings('.add-link');
		if (addButton.length === 1) {
			var lastLastDelLink = jQuery(fields[fields.length - 2]).parent().parent().find('.del-link');
			var lastLastDelLinkParent = jQuery(lastLastDelLink[0]).parent();
			jQuery(addButton[0]).appendTo(lastLastDelLinkParent[0]);
		}
		// delete row
		var row = jQuery(delButton).closest(".row").parent();
		row.remove();
	}
	else {
		fields[0].value = '';
	}
};

/**
 * Adds a value to a multi-value field.
 *
 * @param fieldNamePrefix prefix of input field name
 * @param addButton add button that was clicked
 */
window.lam.selfservice.addMultiValue = function(fieldNamePrefix, addButton) {
	var fields = jQuery("input[name^='" + fieldNamePrefix + "']");
	// get next field number
	var lastFieldName = fields[fields.length - 1].name;
	var lastFieldNameIndex = lastFieldName.substring(fieldNamePrefix.length);
	var newFieldNameIndex = parseInt(lastFieldNameIndex) + 1;
	// copy row
	var row = jQuery(addButton).closest(".row").parent();
	var clone = row.clone();
	clone = clone.appendTo(row.parent());
	var cloneInput = clone.find("input[name^='" + fieldNamePrefix + "']");
	cloneInput[0].name = fieldNamePrefix + newFieldNameIndex;
	cloneInput[0].id = fieldNamePrefix + newFieldNameIndex;
	cloneInput[0].value = '';
	// delete add link from old row
	jQuery(addButton).remove();
};

window.lam.webauthn = window.lam.webauthn || {};

/**
 * Returns the first unicode character.
 *
 * @param c char
 * @returns {number} character
 */
window.lam.webauthn.charAt = function (c) {
	return c.charCodeAt(0);
}

/**
 * Starts the webauthn process.
 *
 * @param prefix path prefix for Ajax endpoint
 * @param isSelfService runs as part of self service
 */
window.lam.webauthn.start = function(prefix, isSelfService) {
	jQuery(document).ready(
		function() {
			window.lam.webauthn.run(prefix, isSelfService);
		}
	);
}

/**
 * Checks if the user is registered and starts login/registration.
 *
 * @param prefix path prefix for Ajax endpoint
 * @param isSelfService runs as part of self service
 */
window.lam.webauthn.run = function(prefix, isSelfService) {
	jQuery('#btn_skip_webauthn').click(function () {
		var form = jQuery("#2faform");
		form.append('<input type="hidden" name="sig_response" value="skip"/>');
		form.submit();
		return;
	});
	var token = jQuery('#sec_token').val();
	// check for webauthn support
	if (!navigator.credentials || (typeof(PublicKeyCredential) === "undefined")) {
		jQuery('.webauthn-error').show();
		return;
	}

	var data = {
			action: 'status',
			jsonInput: '',
			sec_token: token
	};
	var extraParam = isSelfService ? '&selfservice=true' : '';
	jQuery.ajax({
		url: prefix + 'misc/ajax.php?function=webauthn' + extraParam,
		method: 'POST',
		data: data
	})
	.done(function(jsonData) {
		if (jsonData.action === 'register') {
			var successCallback = function (publicKeyCredential) {
				var form = jQuery("#2faform");
				var response = btoa(JSON.stringify(publicKeyCredential));
				form.append('<input type="hidden" name="sig_response" value="' + response + '"/>');
				form.submit();
			};
			var errorCallback = function(error) {
				var errorDiv = jQuery('#generic-webauthn-error');
				var buttonLabel = errorDiv.data('button');
				var dialogTitle = errorDiv.data('title');
				errorDiv.text(error.message);
				window.lam.dialog.showMessage(dialogTitle,
					buttonLabel,
					'generic-webauthn-error',
					function () {
						jQuery('#btn_logout').click();
				});
			};
			window.lam.webauthn.register(jsonData.registration, successCallback, errorCallback);
		}
		else if (jsonData.action === 'authenticate') {
			window.lam.webauthn.authenticate(jsonData.authentication);
		}
	})
	.fail(function() {
		console.log('WebAuthn failed');
	});
}

/**
 * Performs a webauthn registration.
 *
 * @param publicKey registration object
 * @param successCallback callback function in case of all went fine
 * @param errorCallback callback function in case of an error
 */
window.lam.webauthn.register = function(publicKey, successCallback, errorCallback) {
	if (!(publicKey.challenge instanceof Uint8Array)) {
		publicKey.challenge = Uint8Array.from(window.atob(publicKey.challenge), window.lam.webauthn.charAt);
		publicKey.user.id = Uint8Array.from(window.atob(publicKey.user.id), window.lam.webauthn.charAt);
		publicKey.rp.icon = window.location.href.substring(0, window.location.href.lastIndexOf("/")) + publicKey.rp.icon;
		if (publicKey.excludeCredentials) {
			for (var i = 0; i < publicKey.excludeCredentials.length; i++) {
				var idOrig = publicKey.excludeCredentials[i]['id'];
				idOrig = idOrig.replace(/-/g, "+").replace(/_/g, "/");
				var idOrigDecoded = atob(idOrig);
				var idArray = Uint8Array.from(idOrigDecoded, window.lam.webauthn.charAt)
				publicKey.excludeCredentials[i]['id'] = idArray;
			}
		}
	}
	navigator.credentials.create({publicKey: publicKey})
		.then(function (data) {
			var publicKeyCredential = {
				id: data.id,
				type: data.type,
				rawId: window.lam.webauthn.arrayToBase64String(new Uint8Array(data.rawId)),
				response: {
					clientDataJSON: window.lam.webauthn.arrayToBase64String(new Uint8Array(data.response.clientDataJSON)),
					attestationObject: window.lam.webauthn.arrayToBase64String(new Uint8Array(data.response.attestationObject))
				}
			};
			successCallback(publicKeyCredential);
		}, function (error) {
			console.log(error.message);
			errorCallback(error);
		});
}

/**
 * Performs a webauthn authentication.
 *
 * @param publicKey authentication object
 */
window.lam.webauthn.authenticate = function(publicKey) {
	publicKey.challenge = Uint8Array.from(window.atob(publicKey.challenge), window.lam.webauthn.charAt);
	for (var i = 0; i < publicKey.allowCredentials.length; i++) {
		var idOrig = publicKey.allowCredentials[i]['id'];
		idOrig = idOrig.replace(/-/g, "+").replace(/_/g, "/");
		var idOrigDecoded = atob(idOrig);
		var idArray = Uint8Array.from(idOrigDecoded, window.lam.webauthn.charAt)
		publicKey.allowCredentials[i]['id'] = idArray;
	}
	navigator.credentials.get({publicKey: publicKey})
		.then(function(data) {
			var publicKeyCredential = {
				id: data.id,
				type: data.type,
				rawId: window.lam.webauthn.arrayToBase64String(new Uint8Array(data.rawId)),
				response: {
					authenticatorData: window.lam.webauthn.arrayToBase64String(new Uint8Array(data.response.authenticatorData)),
					clientDataJSON: window.lam.webauthn.arrayToBase64String(new Uint8Array(data.response.clientDataJSON)),
					signature: window.lam.webauthn.arrayToBase64String(new Uint8Array(data.response.signature)),
					userHandle: data.response.userHandle ? window.lam.webauthn.arrayToBase64String(new Uint8Array(data.response.userHandle)) : null
				}
			};
			var form = jQuery("#2faform");
			var response = btoa(JSON.stringify(publicKeyCredential));
			form.append('<input type="hidden" name="sig_response" value="' + response + '"/>');
			form.submit();
		}, function(error) {
			console.log(error.message);
			var errorDiv = jQuery('#generic-webauthn-error');
			var buttonLabel = errorDiv.data('button');
			var dialogTitle = errorDiv.data('title');
			errorDiv.text(error.message);
			window.lam.dialog.showMessage(dialogTitle,
				buttonLabel,
				'generic-webauthn-error',
				function () {
					jQuery('#btn_logout').click();
				});
		});
}

/**
 * Converts an array to a base64 string.
 *
 * @param input array
 * @returns base64 string
 */
window.lam.webauthn.arrayToBase64String = function(input) {
	return btoa(String.fromCharCode.apply(null, input));
}

/**
 * Sets up the device management on the main configuration page.
 */
window.lam.webauthn.setupDeviceManagement = function() {
	var searchButton = jQuery('#btn_webauthn_search');
	if (searchButton) {
		searchButton.click(window.lam.webauthn.searchDevices);
	}
	var searchInput = jQuery('#webauthn_searchTerm');
	if (searchInput) {
		searchInput.keydown(function (event) {
			if (event.keyCode == 13) {
				event.preventDefault();
				searchButton.click();
				return false;
			}
		});
	}
}

/**
 * Searches for devices via Ajax call.
 *
 * @param event button click event
 * @returns {boolean} false
 */
window.lam.webauthn.searchDevices = function(event) {
	if (event !== null) {
		event.preventDefault();
	}
	var resultDiv = jQuery('#webauthn_results');
	var tokenValue = resultDiv.data('sec_token_value');
	var searchData = jQuery('#webauthn_searchTerm').val();
	var data = {
		action: 'search',
		jsonInput: '',
		sec_token: tokenValue,
		searchTerm: searchData
	};
	jQuery.ajax({
		url: '../misc/ajax.php?function=webauthnDevices',
		method: 'POST',
		data: data
	})
	.done(function(jsonData) {
		resultDiv.html(jsonData.content);
		window.lam.webauthn.addDeviceActionListeners();
	})
	.fail(function() {
		console.log('WebAuthn search failed');
	});
	return false;
}

/**
 * Adds listeners to the device action buttons.
 */
window.lam.webauthn.addDeviceActionListeners = function() {
	var inputs = jQuery('.webauthn-delete');
	inputs.each(function() {
		jQuery(this).click(function(event) {
			window.lam.webauthn.removeDevice(event);
		});
	});
}

/**
 * Removes a webauthn device.
 *
 * @param event click event
 */
window.lam.webauthn.removeDevice = function(event) {
	event.preventDefault();
	var element = jQuery(event.target);
	window.lam.webauthn.removeDeviceDialog(element, 'webauthnDevices');
	return false;
}

/**
 * Removes a user's own webauthn device.
 *
 * @param event click event
 * @param isSelfService run in self service or admin context
 */
window.lam.webauthn.removeOwnDevice = function(event, isSelfService) {
	event.preventDefault();
	var element = jQuery(event.currentTarget);
	var successCallback = null;
	if (!isSelfService) {
		successCallback = function () {
			var form = jQuery("#webauthnform");
			jQuery('<input>').attr({
				type: 'hidden',
				name: 'removed',
				value: 'true'
			}).appendTo(form);
			form.submit();
		};
	}
	var action = 'webauthnOwnDevices';
	if (isSelfService) {
		action = action + '&selfservice=true&module=webauthn&scope=user';
	}
	window.lam.webauthn.removeDeviceDialog(element, action, successCallback);
	return false;
}

/**
 * Opens the remove device diaog.
 *
 * @param element delete button
 * @param action action for request (delete|deleteOwn)
 * @param successCallback callback if all was fine (optional)
 */
window.lam.webauthn.removeDeviceDialog = function(element, action, successCallback) {
	var dialogTitle = element.data('dialogtitle');
	var okText = element.data('oktext');
	var cancelText = element.data('canceltext');
	var buttonList = {};
	buttonList[okText] = function() {
		jQuery('#webauthnDeleteConfirm').dialog('close');
		window.lam.webauthn.sendRemoveDeviceRequest(element, action, successCallback);
	};
	buttonList[cancelText] = function() {
		jQuery(this).dialog("close");
	};
	jQuery('#webauthnDeleteConfirm').dialog({
		modal: true,
		title: dialogTitle,
		dialogClass: 'defaultBackground',
		buttons: buttonList,
		width: 'auto'
	});
}

/**
 * Sends the remove request to server.
 *
 * @param element button element
 * @param action action (delete|deleteOwn)
 * @param successCallback callback if all was fine (optional)
 */
window.lam.webauthn.sendRemoveDeviceRequest = function(element, action, successCallback) {
	var dn = element.data('dn');
	var credential = element.data('credential');
	var resultDiv = jQuery('#webauthn_results');
	var tokenValue = resultDiv.data('sec_token_value');
	var data = {
		action: 'delete',
		jsonInput: '',
		sec_token: tokenValue,
		dn: dn,
		credentialId: credential
	};
	jQuery.ajax({
		url: '../misc/ajax.php?function=' + action,
		method: 'POST',
		data: data
	})
		.done(function(jsonData) {
			if (successCallback) {
				successCallback();
			}
			else {
				resultDiv.html(jsonData.content);
			}
		})
		.fail(function() {
			console.log('WebAuthn device deletion failed');
		});
}

/**
 * Updates a device name.
 *
 * @param event click event
 * @param isSelfService run in self service or admin context
 */
window.lam.webauthn.updateOwnDeviceName = function(event, isSelfService) {
	event.preventDefault();
	var element = jQuery(event.currentTarget);
	var dn = element.data('dn');
	var nameElementId = element.data('nameelement');
	var nameElement = jQuery('#' + nameElementId);
	var name = nameElement.val();
	var credential = element.data('credential');
	var resultDiv = jQuery('#webauthn_results');
	var tokenValue = resultDiv.data('sec_token_value');
	var data = {
		action: 'setName',
		name: name,
		jsonInput: '',
		sec_token: tokenValue,
		dn: dn,
		credentialId: credential
	};
	var action = 'webauthnOwnDevices';
	if (isSelfService) {
		action = action + '&selfservice=true&module=webauthn&scope=user';
	}
	jQuery.ajax({
		url: '../misc/ajax.php?function=' + action,
		method: 'POST',
		data: data
	})
	.done(function(jsonData) {
		if (isSelfService) {
			nameElement.addClass('markPass');
		}
		else {
			window.location.href = 'webauthn.php?updated=' + encodeURIComponent(credential);
		}
	})
	.fail(function() {
		console.log('WebAuthn device name change failed');
	});
	return false;
}

/**
 * Registers a user's own webauthn device.
 *
 * @param event click event
 * @param isSelfService runs in self service context
 */
window.lam.webauthn.registerOwnDevice = function(event, isSelfService) {
	event.preventDefault();
	var element = jQuery(event.target);
	var dn = element.data('dn');
	var tokenValue = element.data('sec_token_value');
	var publicKey = element.data('publickey');
	var successCallback = function (publicKeyCredential) {
		var form = jQuery("#webauthnform");
		var response = btoa(JSON.stringify(publicKeyCredential));
		var registrationData = jQuery('#registrationData');
		registrationData.val(response);
		form.submit();
	};
	if (isSelfService) {
		successCallback = function (publicKeyCredential) {
			var data = {
				action: 'register',
				jsonInput: '',
				sec_token: tokenValue,
				dn: dn,
				credential: btoa(JSON.stringify(publicKeyCredential))
			};
			jQuery.ajax({
				url: '../misc/ajax.php?selfservice=true&module=webauthn&scope=user',
				method: 'POST',
				data: data
			})
			.done(function(jsonData) {
				var resultDiv = jQuery('#webauthn_results');
				resultDiv.html(jsonData.content);
			})
			.fail(function() {
				console.log('WebAuthn device registration failed');
			});
		};
	}
	var errorCallback = function (error) {
		var errorDiv = jQuery('#generic-webauthn-error');
		var buttonLabel = errorDiv.data('button');
		var dialogTitle = errorDiv.data('title');
		errorDiv.text(error.message);
		window.lam.dialog.showMessage(dialogTitle,
			buttonLabel,
			'generic-webauthn-error'
		);
	};
	window.lam.webauthn.register(publicKey, successCallback, errorCallback);
	return false;
}

window.lam.treeview = window.lam.treeview || {};

/**
 * Returns the nodes in tree view.
 *
 * @param tokenName security token name
 * @param tokenValue security token value
 * @param node tree node
 * @param callback callback function
 */
window.lam.treeview.getNodes = function (tokenName, tokenValue, node, callback) {
	var data = {
		jsonInput: ""
	};
	data[tokenName] = tokenValue;
	data["dn"] = btoa(node.id);
	jQuery.ajax({
		url: "../misc/ajax.php?function=treeview&command=getNodes",
		method: "POST",
		data: data
	})
	.done(function(jsonData) {
		callback.call(this, jsonData);
	})
}

/**
 * Deletes a node in tree view.
 *
 * @param tokenName security token name
 * @param tokenValue security token value
 * @param node tree node
 * @param tree tree
 * @param okText text for OK button
 * @param cancelText text for cancel button
 * @param title dialog title
 * @param errorOxText text for OK button in error dialog
 * @param errorTitle dialog title in case of error
 */
window.lam.treeview.deleteNode = function (tokenName, tokenValue, node, tree, okText, cancelText, title, errorOxText, errorTitle) {
	var parent = node.parent;
	var textSpan = jQuery('#treeview_delete_dlg').find('.treeview-delete-entry');
	textSpan.text(node.text);
	var buttonList = {};
	buttonList[okText] = function() {
		var data = {
			jsonInput: ""
		};
		data[tokenName] = tokenValue;
		data["dn"] = btoa(node.id);
		jQuery.ajax({
			url: "../misc/ajax.php?function=treeview&command=deleteNode",
			method: "POST",
			data: data
		})
		.done(function(jsonData) {
			tree.refresh_node(parent);
			jQuery('#treeview_delete_dlg').dialog("close");
			if (jsonData['errors']) {
				var errTextTitle = jsonData['errors'][0][1];
				var textSpanErrorTitle = jQuery('#treeview_error_dlg').find('.treeview-error-title');
				textSpanErrorTitle.text(errTextTitle);
				var errText = jsonData['errors'][0][2];
				var textSpanErrorText = jQuery('#treeview_error_dlg').find('.treeview-error-text');
				textSpanErrorText.text(errText);
				var errorButtons = {};
				errorButtons[errorOxText] = function () {
					jQuery(this).dialog("close");
				};
				jQuery('#treeview_error_dlg').dialog({
					modal: true,
					title: errorTitle,
					dialogClass: 'defaultBackground',
					buttons: errorButtons,
					width: 'auto'
				});
			}
		});
	};
	buttonList[cancelText] = function() {
		jQuery(this).dialog("close");
	};
	jQuery('#treeview_delete_dlg').dialog({
		modal: true,
		title: title,
		dialogClass: 'defaultBackground',
		buttons: buttonList,
		width: 'auto'
	});
}

jQuery(document).ready(function() {
	window.lam.gui.equalHeight();
	window.lam.form.autoTrim();
	window.lam.account.addDefaultProfileListener();
	window.lam.tools.addSavedSelectListener();
	window.lam.tools.activateTab();
	window.lam.tools.setInitialFocus();
	window.lam.tools.webcam.init();
	window.lam.tools.schema.select();
	window.lam.html.activateLightboxes();
	window.lam.html.preventEnter();
	window.lam.dynamicSelect.activate();
	window.lam.webauthn.setupDeviceManagement();
});

/**
 * Setup service worker.
 */
if ("serviceWorker" in navigator) {
	if (!navigator.serviceWorker.controller) {
		var basePath = document.currentScript.src;
		basePath = basePath.replace(/\/[^/]+\.js/gi, '');
		var workerJS = basePath + '/../../pwa_worker.js';
		navigator.serviceWorker.register(workerJS, {
			scope : basePath + "../../"
		});
	}
}
