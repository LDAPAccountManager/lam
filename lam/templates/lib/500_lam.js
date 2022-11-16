/**

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2022  Roland Gruber

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
	const pageNumber = document.getElementById('listNavPage').value;
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
	const dialogContent = document.getElementById('settingsDialog').cloneNode(true);
	dialogContent.classList.remove('hidden');
	dialogContent.firstElementChild.id = 'settingsDialogForm_dlg';
	Swal.fire({
		title: title,
		confirmButtonText: okText,
		cancelButtonText: cancelText,
		showCancelButton: true,
		html: dialogContent.outerHTML,
		width: '48em'
	}).then(result => {
		if (result.isConfirmed) {
			document.forms["settingsDialogForm_dlg"].submit();
		}
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
	const profileName = document.getElementsByName(selectFieldName)[0].value;
	// update text
	document.getElementById('deleteText').textContent = profileName;
	// update hidden input fields
	document.getElementById('profileDeleteType').value = scope;
	document.getElementById('profileDeleteName').value = profileName;
	const dialogContent = document.getElementById('deleteProfileDialog').cloneNode(true);
	dialogContent.classList.remove('hidden');
	dialogContent.firstElementChild.id = 'deleteProfileDialog_dlg';
	Swal.fire({
		title: title,
		confirmButtonText: okText,
		cancelButtonText: cancelText,
		showCancelButton: true,
		html: dialogContent.outerHTML,
		width: '48em'
	}).then(result => {
		if (result.isConfirmed) {
			document.forms["deleteProfileDialog_dlg"].submit();
		}
	});
}

/**
 * Manages the password change when a button is pressed.
 *
 * @param random "true" if random password should be generated
 * @param ajaxURL URL used for AJAX request
 * @param tokenName name of CSRF token
 * @param tokenValue value of CSRF token
 * @param okText text for ok button
 */
function passwordHandleInput(random, ajaxURL, tokenName, tokenValue, okText) {
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
	if (jQuery('#passwordDialog').find('[name=lamPasswordChangeMailAddress]')) {
		sendMailAlternateAddress = jQuery('#passwordDialog').find('[name=lamPasswordChangeMailAddress]').val();
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
	jQuery.post(ajaxURL, data, function(dataReturned) {
		document.querySelector(".modal").classList.remove("show-modal");
		Swal.fire({
			confirmButtonText: okText,
			html: dataReturned.messages
		});
	}, 'json');
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
 * @param okText text for OK button
 * @param cancelText text for cancel button
 * @param e event
 */
function confirmLoadProfile(text, okText, cancelText, e) {
	Swal.fire({
		confirmButtonText: okText,
		cancelButtonText: cancelText,
		showCancelButton: true,
		text: text,
	}).then(result => {
		if (result.isConfirmed) {
			const form = document.forms["inputForm"];
			let buttonValue = document.createElement("input");
			buttonValue.type = "hidden";
			buttonValue.name = "accountContainerLoadProfile";
			buttonValue.value = "yes";
			form.appendChild(buttonValue);
			form.submit();
		}
	});
	if (e.preventDefault) {
		e.preventDefault();
	}
	if (e.returnValue) {
		e.returnValue = false;
	}
	return false;
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
	let dialogId = '';
	let formId = '';
	if (type == 'export') {
		document.getElementById('name_' + typeId).value = document.getElementById(selectFieldName).value;
		dialogId = 'exportDialog_' + typeId;
		formId = "exportDialogForm_" + typeId;
	} else if (type == 'import') {
		dialogId = 'importDialog_' + typeId;
		formId = "importDialogForm_" + typeId;
	}
	const dialogContent = document.getElementById(dialogId).cloneNode(true);
	dialogContent.classList.remove('hidden');
	dialogContent.firstElementChild.id = formId + '_dlg';
	Swal.fire({
		title: title,
		confirmButtonText: okText,
		cancelButtonText: cancelText,
		showCancelButton: true,
		html: dialogContent.outerHTML,
		width: 'auto'
	}).then(result => {
		if (result.isConfirmed) {
			document.forms[formId + "_dlg"].submit();
		}
	});
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
	window.lam.dialog.showSimpleDialog(title, okText, cancelText, 'logoExportForm', 'logoExportDiv');
}

/**
 * Shows the dialog to import PDF logos.
 *
 * @param title dialog title
 * @param okText text for Ok button
 * @param cancelText text for Cancel button
 */
window.lam.profilePdfEditor.showPdfLogoImportDialog = function(title, okText, cancelText) {
	window.lam.dialog.showSimpleDialog(title, okText, cancelText, 'logoImportForm', 'logoImportDiv');
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
	window.lam.dialog.showSimpleDialog(title, okText, cancelText, 'newBindZoneDialogForm', 'newBindZoneDialog');
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
 * @param inputId HTML ID of hidden input field
 * @param containerId HTML ID of ul-container
 */
function updateModulePositions(inputId, containerId) {
	const input = document.getElementById(inputId);
	let positions = [];
	const container = document.getElementById(containerId);
	const childLiElements = container.children;
	for (let i = 0; i < childLiElements.length; i++) {
		positions[i] = childLiElements[i].getAttribute('data-position-orig');
	}
	input.value = positions.join(',');
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
			options[$(this).val()] = {
				selected: this.selected,
				text: jQuery(this).text(),
				cssClasses: jQuery(this).attr('class')
			};
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
				newOption.attr('selected', 'selected');
			}
			if (option.cssClasses) {
				newOption.attr('class', option.cssClasses);
			}
			selectField.append(newOption);
		}
	});
	// select first entry for single-selects
	if ((selectField[0].size === 1) && selectField[0].onchange) {
		selectField[0].onchange();
	}
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
 * @param text dialog text
 * @param okText ok button text
 */
window.lam.dialog.showInfo = function(text, okText) {
	Swal.fire({
		text: text,
		confirmButtonText: okText,
		width: 'auto'
	});
};

/**
 * Shows a dialog message.
 *
 * @param title dialog title
 * @param okText ok button text
 * @param divId DIV id with dialog content
 * @param callbackFunction callback function (optional)
 */
window.lam.dialog.showMessage = function(title, okText, divId, callbackFunction) {
	const dialogContent = document.getElementById(divId).cloneNode(true);
	dialogContent.classList.remove('hidden');
	Swal.fire({
		title: title,
		confirmButtonText: okText,
		html: dialogContent.outerHTML,
		width: 'auto'
	}).then(result => {
		if (callbackFunction) {
			callbackFunction();
		}
	});
};

/**
 * Shows a simple dialog.
 *
 * @param title dialog title
 * @param okText text for Ok button (optional, submits form)
 * @param cancelText text for Cancel button
 * @param formID form ID
 * @param dialogDivID ID of div that contains dialog content
 */
window.lam.dialog.showSimpleDialog = function(title, okText, cancelText, formID, dialogDivID) {
	const dialogContent = document.getElementById(dialogDivID).cloneNode(true);
	dialogContent.classList.remove('hidden');
	dialogContent.firstElementChild.id = formID + '_dlg';
	Swal.fire({
		title: title,
		confirmButtonText: okText,
		cancelButtonText: cancelText,
		showCancelButton: true,
		showConfirmButton: (okText !== null),
		html: dialogContent.outerHTML,
		width: 'auto'
	}).then(result => {
		if (result.isConfirmed) {
			document.forms[formID + '_dlg'].submit();
		}
	});
}

/**
 * Shows a dialog message.
 *
 * @param title dialog title
 * @param okText ok button text
 * @param cancelText cancel button text
 * @param message text message
 * @param formId form to submit when confirmed
 */
window.lam.dialog.confirmAndSendForm = function(title, okText, cancelText, message, formId) {
	Swal.fire({
		title: title,
		confirmButtonText: okText,
		cancelButtonText: cancelText,
		showCancelButton: true,
		text: message,
		width: 'auto'
	}).then(result => {
		if (result.isConfirmed) {
			document.forms[formId].submit();
		}
	});
};

/**
 * Shows a dialog with password input. The password is added to the form when confirmed.
 *
 * @param title dialog title
 * @param okText ok button text
 * @param cancelText cancel button text
 * @param passwordLabel password label
 * @param passwordInputName input field name for password
 * @param formId form to submit when confirmed
 */
window.lam.dialog.requestPasswordAndSendForm = async function (title, okText, cancelText, passwordLabel, passwordInputName, formId) {
	const {value} = await Swal.fire({
		title: title,
		confirmButtonText: okText,
		cancelButtonText: cancelText,
		showCancelButton: true,
		input: 'password',
		inputLabel: passwordLabel,
		width: 'auto'
	});
	if (value) {
		let passwordTag = document.createElement('input');
		passwordTag.name = passwordInputName;
		passwordTag.value = value;
		passwordTag.hidden = 'hidden';
		document.forms[formId].appendChild(passwordTag);
		document.forms[formId].submit();
	}
};

/**
 * Shows a modal dialog.
 *
 * @param selector selector to find modal content
 */
window.lam.dialog.showModal = function(selector) {
	let modal = document.querySelector(selector);
	modal.classList.add("show-modal");
	window.addEventListener("click", function(event) {
		if(event.target === modal) {
			modal.classList.remove("show-modal");
		}
	});
	// set focus on password field
	let myElement = modal.querySelector('input');
	if (!myElement) {
		myElement = modal.querySelector('select');
	}
	if (myElement) {
		myElement.focus();
	}
}

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
	const field = document.getElementById(fieldId);
	const dnValue = field.value;
	let data = {
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
		const dlgHtml = '<div id="dlg_' + fieldId + '">' + jsonData.dialogData + '</div>';
		Swal.fire({
			title: title,
			cancelButtonText: cancelText,
			showCancelButton: true,
			showConfirmButton: false,
			html: dlgHtml,
			width: 'auto'
		});
	});
};

/**
 * Selects the DN from dialog.
 *
 * @param el ok button in dialog
 * @param fieldId field id of input field
 * @returns boolean false
 */
window.lam.html.selectDn = function(el, fieldId) {
	let field = jQuery('#' + fieldId);
	const dn = jQuery(el).parents('.row').data('dn');
	field.val(dn);
	Swal.close();
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
	const dn = jQuery(el).parents('.row').data('dn');
	let data = {
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
		document.getElementById('dlg_' + fieldId).innerHTML = jsonData.dialogData;
	})
	.fail(function() {
		Swal.close();
	});
}

/**
 * Activates the lightboxes on images.
 */
window.lam.html.activateLightboxes = function() {
	document.querySelectorAll('.lam-lightbox').forEach(item => {
		item.onclick = function() {
			Swal.fire({
				imageUrl: item.src,
				confirmButtonText: item.dataset.lightboxLabelClose,
				width: 'auto'
			})
		};
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
			const registerFunction = function() {
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
				return false;
			}
			let registerButton = document.getElementById('btn_register_webauthn');
			if (!registerButton) {
				registerFunction();
			}
			else {
				registerButton.onclick = registerFunction;
			}
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
 * Opens the remove device dialog.
 *
 * @param element delete button
 * @param action action for request (delete|deleteOwn)
 * @param successCallback callback if all was fine (optional)
 */
window.lam.webauthn.removeDeviceDialog = function(element, action, successCallback) {
	var dialogTitle = element.data('dialogtitle');
	var okText = element.data('oktext');
	var cancelText = element.data('canceltext');
	const dialogContent = document.getElementById('webauthnDeleteConfirm').cloneNode(true);
	dialogContent.classList.remove('hidden');
	Swal.fire({
		title: dialogTitle,
		confirmButtonText: okText,
		cancelButtonText: cancelText,
		showCancelButton: true,
		html: dialogContent.outerHTML,
		width: 'auto'
	}).then(result => {
		if (result.isConfirmed) {
			window.lam.webauthn.sendRemoveDeviceRequest(element, action, successCallback);
		}
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
	data["dn"] = node.id;
	jQuery.ajax({
		url: "../misc/ajax.php?function=treeview&command=getNodes",
		method: "POST",
		data: data
	})
	.done(function(jsonData) {
		window.lam.treeview.checkSession(jsonData);
		callback.call(this, jsonData);
	})
}

/**
 * Creates a new node in tree view.
 *
 * @param tokenName security token name
 * @param tokenValue security token value
 * @param node tree node
 * @param tree tree
 */
window.lam.treeview.createNode = function (tokenName, tokenValue, node, tree) {
	var data = {
		jsonInput: ""
	};
	data[tokenName] = tokenValue;
	data["dn"] = node.id;
	jQuery.ajax({
		url: "../misc/ajax.php?function=treeview&command=createNewNode&step=getObjectClasses",
		method: "POST",
		data: data
	})
	.done(function(jsonData) {
		window.lam.treeview.checkSession(jsonData);
		jQuery('#ldap_actionarea').html(jsonData.content);
	});
}

/**
 * Selects the object classes.
 *
 * @param event event
 * @param tokenName security token name
 * @param tokenValue security token value
 */
window.lam.treeview.createNodeSelectObjectClassesStep = function (event, tokenName, tokenValue) {
	event.preventDefault();
	var data = {
		jsonInput: ""
	};
	data[tokenName] = tokenValue;
	data["dn"] = jQuery('#parentDn').val();
	data["objectClasses"] = jQuery('#objectClasses').val();
	jQuery.ajax({
		url: "../misc/ajax.php?function=treeview&command=createNewNode&step=checkObjectClasses",
		method: "POST",
		data: data
	})
	.done(function(jsonData) {
		window.lam.treeview.checkSession(jsonData);
		jQuery('#ldap_actionarea').html(jsonData.content);
		window.lam.treeview.addFileInputListeners();
	});
}

/**
 * Selects the attributes.
 *
 * @param event event
 * @param tokenName security token name
 * @param tokenValue security token value
 */
window.lam.treeview.createNodeEnterAttributesStep = function (event, tokenName, tokenValue) {
	event.preventDefault();
	var data = {
		jsonInput: ""
	};
	data[tokenName] = tokenValue;
	var parentDn = jQuery('#parentDn').val();
	data["dn"] = parentDn;
	data["rdn"] = jQuery('#rdn').val();
	data["objectClasses"] = jQuery('#objectClasses').val();
	// clear old values in data
	jQuery('.single-input').each(
		function() {
			var input = jQuery(this);
			input.attr('data-value-orig', '');
		}
	);
	jQuery('.multi-input').each(
		function() {
			var input = jQuery(this);
			input.attr('data-value-orig', '');
		}
	);
	// get attribute values
	var attributeChanges = window.lam.treeview.findAttributeChanges();
	data["attributes"] = JSON.stringify(attributeChanges);
	jQuery.ajax({
		url: "../misc/ajax.php?function=treeview&command=createNewNode&step=checkAttributes",
		method: "POST",
		data: data
	})
	.done(function(jsonData) {
		window.lam.treeview.checkSession(jsonData);
		jQuery('#ldap_actionarea').html(jsonData.content);
		var tree = jQuery.jstree.reference("#ldap_tree");
		tree.refresh_node(parentDn);
		tree.open_node(parentDn);
		jQuery("#ldap_actionarea").scrollTop(0);
	});
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
 * @param errorOkText text for OK button in error dialog
 * @param errorTitle dialog title in case of error
 */
window.lam.treeview.deleteNode = function (tokenName, tokenValue, node, tree, okText, cancelText, title, errorOkText, errorTitle) {
	var parent = node.parent;
	var textSpan = jQuery('#treeview_delete_dlg').find('.treeview-delete-entry');
	textSpan.text(node.text);
	const dialogContent = document.getElementById('treeview_delete_dlg').cloneNode(true);
	dialogContent.classList.remove('hidden');
	Swal.fire({
		title: title,
		confirmButtonText: okText,
		cancelButtonText: cancelText,
		showCancelButton: true,
		html: dialogContent.outerHTML,
		width: '48em'
	}).then(result => {
		if (result.isConfirmed) {
			let data = {
				jsonInput: ""
			};
			data[tokenName] = tokenValue;
			data["dn"] = node.id;
			jQuery.ajax({
				url: "../misc/ajax.php?function=treeview&command=deleteNode",
				method: "POST",
				data: data
			})
			.done(function(jsonData) {
				window.lam.treeview.checkSession(jsonData);
				tree.refresh_node(parent);
				var node = tree.get_node(parent, false);
				window.lam.treeview.getNodeContent(tokenName, tokenValue, node.id);
				if (jsonData['errors']) {
					var errTextTitle = jsonData['errors'][0][1];
					var textSpanErrorTitle = jQuery('#treeview_error_dlg').find('.treeview-error-title');
					textSpanErrorTitle.text(errTextTitle);
					var errText = jsonData['errors'][0][2];
					var textSpanErrorText = jQuery('#treeview_error_dlg').find('.treeview-error-text');
					textSpanErrorText.text(errText);
					window.lam.dialog.showSimpleDialog(errorTitle, null, errorOkText, null, 'treeview_error_dlg');
				}
			});
		}
	});
}

/**
 * Returns the node content in tree view action area.
 *
 * @param tokenName security token name
 * @param tokenValue security token value
 * @param dn DN (base64 encoded)
 * @param messages any messages that should be displayed (HTML code)
 * @param attributesToHighlight attributes to highlight
 */
window.lam.treeview.getNodeContent = function (tokenName, tokenValue, dn, messages, attributesToHighlight) {
	var data = {
		jsonInput: ""
	};
	data[tokenName] = tokenValue;
	data["dn"] = dn;
	data["highlight"] = attributesToHighlight;
	jQuery.ajax({
		url: "../misc/ajax.php?function=treeview&command=getNodeContent",
		method: "POST",
		data: data
	})
	.done(function(jsonData) {
		window.lam.treeview.checkSession(jsonData);
		jQuery('#ldap_actionarea').html(jsonData.content);
		if (messages) {
			jQuery('#ldap_actionarea_messages').html(messages);
		}
		jQuery("#ldap_actionarea").scrollTop(0);
		window.lam.html.activateLightboxes();
		window.lam.treeview.addFileInputListeners();
	});
}

/**
 * Adds a listener to each file input to write the file content to a data attribute.
 */
window.lam.treeview.addFileInputListeners = function () {
	jQuery('.image-upload').each(
		function () {
			var input = jQuery(this);
			input.change(function () {
				var files = input[0].files;
				if (!files[0]) {
					return;
				}
				var reader = new FileReader();
				reader.onload = function () {
					var content = btoa(reader.result);
					input.attr('data-binary', content);
				};
				reader.readAsBinaryString(files[0]);
			});
		}
	);
}

/**
 * Saves the attributes in tree view action area.
 *
 * @param event event
 * @param tokenName security token name
 * @param tokenValue security token value
 * @param dn DN
 */
window.lam.treeview.saveAttributes = function (event, tokenName, tokenValue, dn) {
	event.preventDefault();
	var data = {
		jsonInput: ""
	};
	data[tokenName] = tokenValue;
	data["dn"] = dn;
	var attributeChanges = window.lam.treeview.findAttributeChanges();
	var attributesToHighlight = Object.keys(attributeChanges);
	data["changes"] = JSON.stringify(attributeChanges);
	jQuery.ajax({
		url: "../misc/ajax.php?function=treeview&command=saveAttributes",
		method: "POST",
		data: data
	})
	.done(function(jsonData) {
		window.lam.treeview.checkSession(jsonData);
		if (jsonData['newDn']) {
			var tree = jQuery.jstree.reference("#ldap_tree");
			tree.refresh_node(jsonData['parent']);
			window.lam.treeview.getNodeContent(tokenName, tokenValue, jsonData['newDn'], jsonData.result, attributesToHighlight);
		}
		else {
			window.lam.treeview.getNodeContent(tokenName, tokenValue, dn, jsonData.result, attributesToHighlight);
		}
	});
}

/**
 * Finds the attributes that were changed by the user.
 *
 * @returns list of changes
 */
window.lam.treeview.findAttributeChanges = function () {
	var attributeChanges = {};
	jQuery('.single-input').each(
		function() {
			var input = jQuery(this);
			if (input.is(":hidden")) {
				return;
			}
			var attrName = input.data('attr-name');
			// avoid type conversion in .data()
			var valueOrig = input.attr('data-value-orig');
			var valueNew = input.val();
			if (valueNew != valueOrig) {
				attributeChanges[attrName] = {
					old: [valueOrig]
				};
				if (valueNew == '') {
					attributeChanges[attrName]["new"] = [];
				}
				else {
					attributeChanges[attrName]["new"] = [valueNew];
				}
			}
		}
	);
	var lastAttrName = '';
	var lastAttrValuesNew = [];
	var lastAttrValuesOld = [];
	var lastAttrHasChange = false;
	jQuery('.multi-input').each(
		function() {
			var input = jQuery(this);
			if (input.is(":hidden")) {
				return;
			}
			var attrName = input.data('attr-name');
			if (attrName != lastAttrName) {
				if (lastAttrHasChange) {
					attributeChanges[lastAttrName] = {
						old: lastAttrValuesOld,
						new: lastAttrValuesNew
					};
				}
				// reset
				lastAttrHasChange = false;
				lastAttrName = attrName;
				lastAttrValuesNew = [];
				lastAttrValuesOld = [];
			}
			// avoid type conversion in .data()
			var valueOrig = input.attr('data-value-orig');
			var valueNew = input.val();
			if (valueOrig != '') {
				lastAttrValuesOld.push(valueOrig);
			}
			if (valueNew != '') {
				lastAttrValuesNew.push(valueNew);
			}
			if (valueNew != valueOrig) {
				lastAttrHasChange = true;
			}
		}
	);
	if (lastAttrHasChange) {
		attributeChanges[lastAttrName] = {
			old: lastAttrValuesOld,
			new: lastAttrValuesNew
		};
	}
	jQuery('.hash-select').each(
		function() {
			var input = jQuery(this);
			var attrName = input.data('attr-name');
			if (!attributeChanges[attrName]) {
				return;
			}
			if (!attributeChanges[attrName]['hash']) {
				attributeChanges[attrName]['hash'] = [input.val()];
			}
			else {
				attributeChanges[attrName]['hash'].push(input.val());
			}
		}
	);
	jQuery('.image-input').each(
		function() {
			var input = jQuery(this);
			var toDelete = input.attr('data-delete');
			if (toDelete !== 'true') {
				return;
			}
			var attrName = input.attr('data-attr-name');
			var attrIndex = input.attr('data-index');
			if (!attrIndex) {
				return;
			}
			if (!attributeChanges[attrName]) {
				attributeChanges[attrName] = {delete: [attrIndex]};
			}
			else {
				attributeChanges[attrName]['delete'].push(attrIndex);
			}
		}
	);
	jQuery('.image-upload').each(
		function() {
			var input = jQuery(this);
			var content = input.attr('data-binary');
			if (!content) {
				return;
			}
			var attrName = input.attr('data-attr-name');
			if (!attrName) {
				return;
			}
			if (!attributeChanges[attrName]) {
				attributeChanges[attrName] = {upload: content};
			}
			else {
				attributeChanges[attrName]['upload'] = content;
			}
		}
	);
	return attributeChanges;
}

/**
 * Clears an LDAP attribute input field.
 *
 * @param event event
 * @param link link object
 */
window.lam.treeview.clearValue = function (event, link) {
	event.preventDefault();
	var linkObj = jQuery(link);
	var parentTr = jQuery(linkObj.parents('tr').get(0));
	parentTr.find('input, textarea').val('');
	var image = parentTr.find('.image-input');
	if (image.length > 0) {
		parentTr.addClass('hidden');
		image.attr('data-delete', 'true');
	}
}

/**
 * Adds an LDAP attribute input field.
 *
 * @param event event
 * @param link link object
 */
window.lam.treeview.addValue = function (event, link) {
	event.preventDefault();
	var linkObj = jQuery(link);
	var parentTr = jQuery(linkObj.parents('tr').get(0));
	var newTr = parentTr.clone();
	var newField = newTr.find('input, textarea');
	newField.val('');
	newField.attr('data-value-orig', '');
	newTr.insertAfter(parentTr);
}

/**
 * Updates the list of possible new attributes to add.
 *
 * @param tokenName security token name
 * @param tokenValue security token value
 */
window.lam.treeview.updatePossibleNewAttributes = function(tokenName, tokenValue) {
	// cancel running request
	if (window.lam.treeview.updatePossibleNewAttributesRequest) {
		window.lam.treeview.updatePossibleNewAttributesRequest.abort();
		window.lam.treeview.updatePossibleNewAttributesRequest = null;
	}
	const fields = document.querySelectorAll('.lam-attr-objectclass');
	// setup listener
	const listener = function() {
		window.lam.treeview.updatePossibleNewAttributes(tokenName, tokenValue);
	};
	fields.forEach(function(field) {
		field.removeEventListener('change', listener)
		field.addEventListener('change', listener);
	});
	let objectCLasses = [];
	fields.forEach(function(field) {
		objectCLasses.push(field.value);
	});
	let data = {
		jsonInput: "",
		dn: 'none'
	};
	data[tokenName] = tokenValue;
	data['objectClasses'] = objectCLasses;
	window.lam.treeview.updatePossibleNewAttributesRequest = jQuery.ajax({
		url: "../misc/ajax.php?function=treeview&command=getPossibleNewAttributes",
		method: "POST",
		data: data
	})
	.done(function(jsonData) {
		window.lam.treeview.checkSession(jsonData);
		const select = document.querySelector('#newAttribute');
		select.innerHTML = '';
		const data = jsonData['data'];
		for (const attributeName in data) {
			var option = document.createElement('option');
			option.value = data[attributeName];
			option.innerText = attributeName;
			select.appendChild(option);
		};
		window.lam.treeview.updatePossibleNewAttributesRequest = null;
	});
}

window.lam.treeview.updatePossibleNewAttributesRequest = null;

/**
 * Adds the input field for a new attribute.
 *
 * @param event event
 * @param select select object
 */
window.lam.treeview.addAttributeField = function (event, select) {
	event.preventDefault();
	var selectObj = jQuery(select);
	var attributeParts = selectObj.val();
	if (attributeParts == '') {
		return;
	}
	selectObj.children('[value="' + attributeParts + '"]').remove();
	attributeParts = attributeParts.split('__#__');
	var attributeName = attributeParts[0];
	var isSingleValue = attributeParts[1];
	var fieldType = attributeParts[2];
	var placeHolderId = 'new-attributes-' + isSingleValue + '-' + fieldType;
	var newContent = jQuery(jQuery('#' + placeHolderId).children('.row').get(0)).clone();
	jQuery(newContent.children().get(0)).text(attributeName);
	var inputField = newContent.find('input, textarea');
	inputField.attr('data-attr-name', attributeName);
	inputField.attr('name', 'lam_attr_' + attributeName);
	inputField.attr('id', 'lam_attr_' + attributeName);
	newContent.children().insertAfter(jQuery(selectObj.parents('div').get(0)));
	window.lam.treeview.addFileInputListeners();
}

/**
 * Returns the internal attributes content in tree view action area.
 *
 * @param event event
 * @param tokenName security token name
 * @param tokenValue security token value
 * @param dn DN
 */
window.lam.treeview.getInternalAttributesContent = function (event, tokenName, tokenValue, dn) {
	event.preventDefault();
	var data = {
		jsonInput: ""
	};
	data[tokenName] = tokenValue;
	data["dn"] = dn;
	jQuery.ajax({
		url: "../misc/ajax.php?function=treeview&command=getInternalAttributesContent",
		method: "POST",
		data: data
	})
	.done(function(jsonData) {
		window.lam.treeview.checkSession(jsonData);
		jQuery('#actionarea-internal-attributes').html(jsonData.content);
	});
}

/**
 * Searches the LDAP tree.
 *
 * @param tokenName security token name
 * @param tokenValue security token value
 * @param dn DN (base64 encoded)
 */
window.lam.treeview.search = function (tokenName, tokenValue, dn) {
	var data = {
		jsonInput: ""
	};
	data[tokenName] = tokenValue;
	data["dn"] = dn;
	jQuery.ajax({
		url: "../misc/ajax.php?function=treeview&command=search",
		method: "POST",
		data: data
	})
	.done(function(jsonData) {
		window.lam.treeview.checkSession(jsonData);
		jQuery('#ldap_actionarea').html(jsonData.content);
		jQuery("#ldap_actionarea").scrollTop(0);
	});
}

/**
 * Displays the search results.
 *
 * @param event event
 * @param tokenName security token name
 * @param tokenValue security token value
 * @param dn DN (base64 encoded)
 */
window.lam.treeview.searchResults = function (event, tokenName, tokenValue, dn) {
	event.preventDefault();
	var data = {
		jsonInput: ""
	};
	data[tokenName] = tokenValue;
	data["dn"] = dn;
	data["scope"] = jQuery('#scope').val();
	data["filter"] = jQuery('#filter').val();
	data["attributes"] = jQuery('#attributes').val();
	data["orderBy"] = jQuery('#orderBy').val();
	data["limit"] = jQuery('#limit').val();
	data["format"] = jQuery('#format').val();
	jQuery.ajax({
		url: "../misc/ajax.php?function=treeview&command=searchResults",
		method: "POST",
		data: data
	})
	.done(function(jsonData) {
		window.lam.treeview.checkSession(jsonData);
		jQuery('#ldap_actionarea').html(jsonData.content);
		jQuery("#ldap_actionarea").scrollTop(0);
	});
}

/**
 * Opens the given node IDs.
 *
 * @param tree tree object
 * @param ids array of node IDs.
 */
window.lam.treeview.openInitial = function(tree, ids) {
	if (ids.length == 0) {
		return;
	}
	var firstNodeId = ids.shift();
	tree.open_node(firstNodeId, function() {
		window.lam.treeview.openInitial(tree, ids);
	});
	if (ids.length == 0) {
		tree.select_node(firstNodeId);
	}
}

/**
 * Copies a node in the tree.
 *
 * @param node node
 * @param tree tree
 */
window.lam.treeview.copyNode = function(node, tree) {
	if (!window.sessionStorage) {
		return;
	}
	window.sessionStorage.setItem('LAM_COPY_PASTE_ACTION', 'COPY');
	window.sessionStorage.setItem('LAM_COPY_PASTE_OLD_ICON', node.icon);
	window.sessionStorage.setItem('LAM_COPY_PASTE_DN', node.id);
	tree.set_icon(node, '../../graphics/copy.svg');
	window.lam.treeview.contextMenuPasteDisabled = false;
}

/**
 * Cuts a node in the tree.
 *
 * @param node node
 * @param tree tree
 */
window.lam.treeview.cutNode = function(node, tree) {
	if (!window.sessionStorage) {
		return;
	}
	window.sessionStorage.setItem('LAM_COPY_PASTE_ACTION', 'CUT');
	window.sessionStorage.setItem('LAM_COPY_PASTE_OLD_ICON', node.icon);
	window.sessionStorage.setItem('LAM_COPY_PASTE_DN', node.id);
	tree.set_icon(node, '../../graphics/cut.svg');
	window.lam.treeview.contextMenuPasteDisabled = false;
}

/**
 * Pastes a copied/cut node.
 *
 * @param tokenName security token name
 * @param tokenValue security token value
 * @param node node
 * @param tree tree
 */
window.lam.treeview.pasteNode = function (tokenName, tokenValue, node, tree) {
	var dn = window.sessionStorage.getItem('LAM_COPY_PASTE_DN');
	tree.deselect_all();
	var oldIcon = window.sessionStorage.getItem('LAM_COPY_PASTE_OLD_ICON');
	var action = window.sessionStorage.getItem('LAM_COPY_PASTE_ACTION');
	var targetDn = node.id;
	var data = {
		jsonInput: ""
	};
	data[tokenName] = tokenValue;
	data["dn"] = dn;
	data["targetDn"] = targetDn;
	data["action"] = action;
	jQuery.ajax({
		url: "../misc/ajax.php?function=treeview&command=paste",
		method: "POST",
		data: data
	})
	.done(function(jsonData) {
		window.lam.treeview.checkSession(jsonData);
		if (jsonData.error) {
			jQuery('#ldap_actionarea_messages').html(jsonData.error);
			return;
		}
		tree.set_icon(dn, oldIcon);
		window.sessionStorage.removeItem('LAM_COPY_PASTE_ACTION');
		window.sessionStorage.removeItem('LAM_COPY_PASTE_OLD_ICON');
		window.sessionStorage.removeItem('LAM_COPY_PASTE_DN');
		tree.refresh_node(targetDn);
		tree.open_node(targetDn);
		tree.select_node(targetDn);
		if (action == 'CUT') {
			var parentDn = tree.get_parent(dn);
			tree.refresh_node(parentDn);
		}
		window.lam.treeview.contextMenuPasteDisabled = true;
	});
}

/**
 * Checks if the session expired and redirects to login.
 *
 * @param json JSON response
 */
window.lam.treeview.checkSession = function(json) {
	if (json && (json.sessionExpired == 'true')) {
		location.href = '../login.php?expired=yes';
	}
}

/**
 * Checks if the password matches a given value.
 *
 * @param event event
 * @param element element
 * @param tokenName security token name
 * @param tokenValue security token value
 * @param title dialog title
 * @param checkText label for check button
 * @param cancelText label for cancel button
 * @param okText label for ok button
 */
window.lam.treeview.checkPassword = function(event, element, tokenName, tokenValue, title,
											 checkText, cancelText, okText) {
	event.preventDefault();
	const outputDiv = document.getElementById('lam-pwd-check-dialog-result');
	outputDiv.innerHTML = '';
	const dialogContent = document.getElementById('lam-pwd-check-dialog').cloneNode(true);
	dialogContent.classList.remove('hidden');
	dialogContent.querySelector('.lam_pwd_check').classList.add('lam_pwd_check_dlg');
	Swal.fire({
		title: title,
		confirmButtonText: checkText,
		cancelButtonText: cancelText,
		showCancelButton: true,
		html: dialogContent.outerHTML,
		width: '48em'
	}).then(result => {
		if (result.isConfirmed) {
			const hashValue = element.closest('table').querySelector('input[type=password]').value;
			const checkValue = document.querySelector('.lam_pwd_check_dlg').value;
			let data = new FormData();
			data.append('jsonInput', '');
			data.append(tokenName, tokenValue);
			data.append('hashValue', hashValue);
			data.append('checkValue', checkValue);
			fetch("../misc/ajax.php?function=checkPassword", {
				method: 'POST',
				body: data
			})
			.then(async response => {
				const jsonData = await response.json();
				if (jsonData.resultHtml) {
					outputDiv.innerHTML = jsonData.resultHtml;
					window.lam.dialog.showSimpleDialog(null, null, okText, null, 'lam-pwd-check-dialog-result');
				}
			});
		}
	});
}

/**
 * Updates the positions of a sorted list of LDAP values.
 *
 * @param containerId HTML ID of ul-container
 */
window.lam.treeview.updateAttributePositionData = function(containerId) {
	const container = document.getElementById(containerId);
	const childLiElements = container.children;
	for (let i = 0; i < childLiElements.length; i++) {
		const inputField = childLiElements[i].querySelector('input');
		inputField.value = '{' + i + '}' + inputField.value.replace(/^\{[0-9]+\}/, '');
	}
}

window.lam.topmenu = window.lam.topmenu || {};

/**
 * Toggles the top navigation menu.
 */
window.lam.topmenu.toggle = function() {
	var topnav = document.getElementById('lam-topnav');
	if (topnav.className == 'lam-header') {
		topnav.className = 'lam-header lam-header-open';
	}
	else {
		topnav.className = 'lam-header';
	}
}

/**
 * Opens a submenu of the top navigation.
 *
 * @param event event
 * @param layerId layer ID
 * @param listener close listener
 */
window.lam.topmenu.openSubmenu = function(event, layerId, listener) {
	const layer = document.getElementById(layerId);
	if (layer.style.height && (layer.style.height !== '0px')) {
		// no action if already open
		return;
	}
	document.removeEventListener("click", listener);
	document.removeEventListener("mouseover", listener);
	event.preventDefault();
	event.stopImmediatePropagation();
	let layers = document.getElementsByClassName('lam-navigation-layer');
	for (let i = 0; i < layers.length; i++) {
		layers[i].style.height = "0px";
	}
	const height = layer.getElementsByClassName('lam-navigation-layer-content')[0].offsetHeight;
	layer.style.height = height + 'px';
	window.lam.topmenu.lastOpened = new Date().getTime();
	document.addEventListener("click", listener);
	document.addEventListener("mouseover", listener);
}

/**
 * Close listener for tools flyout.
 *
 * @param event event
 */
window.lam.topmenu.subMenuCloseListenerTools = function (event) {
	const timeLimit = new Date().getTime() - 100;
	if (!window.lam.topmenu.lastOpened || (timeLimit < window.lam.topmenu.lastOpened)) {
		return;
	}
	if ((event.type == 'click') && !event.target.closest('#lam-navigation-tools')) {
		document.getElementById('lam-navigation-tools').style.height = "0px";
	}
	if ((event.type == 'mouseover') && !event.target.closest('#lam-topnav')) {
		document.getElementById('lam-navigation-tools').style.height = "0px";
	}
}

/**
 * Close listener for account types flyout.
 *
 * @param event event
 */
window.lam.topmenu.subMenuCloseListenerTypes = function (event) {
	const timeLimit = new Date().getTime() - 100;
	if (!window.lam.topmenu.lastOpened || (timeLimit < window.lam.topmenu.lastOpened)) {
		return;
	}
	if ((event.type == 'click') && !event.target.closest('#lam-navigation-types')) {
		document.getElementById('lam-navigation-types').style.height = "0px";
	}
	if ((event.type == 'mouseover') && !event.target.closest('#lam-topnav')) {
		document.getElementById('lam-navigation-types').style.height = "0px";
	}
}

window.lam.autocomplete = window.lam.autocomplete || {};

/**
 * Initializes the autocompletion.
 */
window.lam.autocomplete.init = function() {
	const fields = document.getElementsByClassName('lam-autocomplete');
	for (let i = 0; i < fields.length; i++) {
		window.lam.autocomplete.activate(fields[i]);
	}
	const mutationObserver = new MutationObserver(function(mutations) {
		mutations.forEach(function(mutation) {
			if (mutation.addedNodes) {
				mutation.addedNodes.forEach(function(node){
					if (!node.tagName) {
						return;
					}
					window.lam.autocomplete.checkNode(node);
				});
			}
		});
	});
	mutationObserver.observe(document.documentElement, {
		attributes: false,
		characterData: false,
		childList: true,
		subtree: true,
		attributeOldValue: false,
		characterDataOldValue: false
	});
}

/**
 * Checks if autocompletion needs to be added on the given node or its subnodes.
 *
 * @param node node
 */
window.lam.autocomplete.checkNode = function(node) {
	if (node.classList && node.classList.contains('lam-autocomplete')) {
		window.lam.autocomplete.activate(node);
		return;
	}
	if (!node.childNodes) {
		return;
	}
	node.childNodes.forEach(function(child){
		window.lam.autocomplete.checkNode(child);
	});
}

/**
 * Activates the autocompletion on a given field.
 *
 * @param field field
 */
window.lam.autocomplete.activate = function(field) {
	const values = JSON.parse(atob(field.getAttribute('data-autocomplete')));
	const minLength = field.getAttribute('data-autocomplete-minLength');
	jQuery(field).autocomplete({
		source: values,
		minLength: minLength
	});
}

window.lam.tabs = window.lam.tabs || {};

window.lam.tabs.init = function() {
	const tabs = document.querySelectorAll('li.lam-tab');
	tabs.forEach(function(element) {
		if (element.dataset.tabid) {
			element.onclick = function() {
				window.lam.tabs.tabClick(element);
				return false;
			};
		}
	});
}

window.lam.tabs.tabClick = function(element) {
	const tabId = element.dataset.tabid;
	const contents = document.querySelectorAll('div.lam-tab-content');
	contents.forEach(function (element) {
		if (element.dataset.tabid == tabId) {
			element.classList.add('lam-tab-active');
		} else {
			element.classList.remove('lam-tab-active');
		}
	});
	const tabs = document.querySelectorAll('li.lam-tab');
	tabs.forEach(function (element) {
		if (element.dataset.tabid == tabId) {
			element.classList.add('lam-tab-active');
		}
		else {
			element.classList.remove('lam-tab-active');
		}
	});
}

window.lam.progressbar = window.lam.progressbar || {};

/**
 * Updates a progress bar.
 *
 * @param htmlId HTML id
 * @param progress new progress value (0..100)
 */
window.lam.progressbar.setProgress = function(htmlId, progress) {
	const bar = document.getElementById(htmlId).querySelector('.lam-progressbar-bar');
	bar.style.width = progress + '%';
}

jQuery(document).ready(function() {
	window.lam.gui.equalHeight();
	window.lam.form.autoTrim();
	window.lam.account.addDefaultProfileListener();
	window.lam.tools.addSavedSelectListener();
	window.lam.tools.setInitialFocus();
	window.lam.tools.webcam.init();
	window.lam.tools.schema.select();
	window.lam.html.activateLightboxes();
	window.lam.html.preventEnter();
	window.lam.dynamicSelect.activate();
	window.lam.webauthn.setupDeviceManagement();
	window.lam.autocomplete.init();
	window.lam.tabs.init();
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
