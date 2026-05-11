/**

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2026  Roland Gruber

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
	cbox.checked = !cbox.checked;
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
	if (e.key === 'Enter') {
		if (e.preventDefault) {
			e.preventDefault();
		}
		location.href = url + '&page=' + encodeURIComponent(pageNumber);
		return false;
	}
	return true;
}

window.lam.accountLists = window.lam.accountLists || {};

/**
 * Shows the dialog to change the list settings.
 *
 * @param title dialog title
 * @param okText text for Ok button
 * @param cancelText text for Cancel button
 */
window.lam.accountLists.listShowSettingsDialog = function (title, okText, cancelText) {
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
 * Executed when radio buttons change for (un)locking accounts.
 */
window.lam.accountLists.lockUnlockRadioChange = function () {
	if (document.querySelector('#lam_accountStatusAction0:checked')) {
		document.getElementById('lam_accountStatusDialogLockDiv').classList.remove('hidden');
		document.getElementById('lam_accountStatusDialogUnlockDiv').classList.add('hidden');
	}
	else {
		document.getElementById('lam_accountStatusDialogLockDiv').classList.add('hidden');
		document.getElementById('lam_accountStatusDialogUnlockDiv').classList.remove('hidden');
	}
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
	if (e.key === 'Enter') {
		if (e.preventDefault) {
			e.preventDefault();
		}
		if (e.returnValue) {
			e.returnValue = false;
		}
		if (window.lam_lastKey) {
			// no submit if last key code was arrow key (browser autocompletion)
			if (window.lam_lastKey === 'ArrowUp' || window.lam_lastKey === 'ArrowDown' ||
				window.lam_lastKey === 'PageUp' || window.lam_lastKey === 'PageDown'
			) {
				window.lam_lastKey = e.key;
				return true;
			}
		}
		document.getElementsByName(id)[0].click();
		return false;
	}
	window.lam_lastKey = e.key;
	return true;
}

/**
 * Selects/deselects all accounts on the page.
 */
function list_switchAccountSelection() {
	// set checkbox selection
	document.querySelectorAll('input.accountBoxUnchecked').forEach(item => {
		item.checked = true;
	});
	document.querySelectorAll('input.accountBoxChecked').forEach(item => {
		item.checked = false;
	});
	// switch CSS class
	const nowChecked = document.querySelectorAll('input.accountBoxUnchecked');
	const nowUnchecked = document.querySelectorAll('input.accountBoxChecked');
	nowChecked.forEach(item => {
		item.classList.add('accountBoxChecked');
		item.classList.remove('accountBoxUnchecked');
	});
	nowUnchecked.forEach(item => {
		item.classList.remove('accountBoxChecked');
		item.classList.add('accountBoxUnchecked');
	});
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
	let modules = new Array();
	const passwordDialog = document.getElementById('passwordDialog');
	passwordDialog.querySelectorAll(':checked').forEach(item => {
		modules.push(item.name);
	});
	const pwd1 = passwordDialog.querySelector('[name=newPassword1]').value;
	const pwd2 = passwordDialog.querySelector('[name=newPassword2]').value;
	let forcePasswordChange = false;
	const lamForcePasswordChangeBox = passwordDialog.querySelector('input[name=lamForcePasswordChange]');
	if (lamForcePasswordChangeBox && lamForcePasswordChangeBox.checked) {
		forcePasswordChange = true;
	}
	let sendMail = false;
	const lamPasswordChangeSendMailBox = passwordDialog.querySelector('input[name=lamPasswordChangeSendMail]');
	if (lamPasswordChangeSendMailBox && lamPasswordChangeSendMailBox.checked) {
		sendMail = true;
	}
	let sendSms = false;
	const lamPasswordChangeSendSmsBox = passwordDialog.querySelector('input[name=lamPasswordChangeSendSms]');
	if (lamPasswordChangeSendSmsBox && lamPasswordChangeSendSmsBox.checked) {
		sendSms = true;
	}

	let sendMailAlternateAddress = '';
	const lamPasswordChangeMailAddress = passwordDialog.querySelector('[name=lamPasswordChangeMailAddress]');
	if (lamPasswordChangeMailAddress) {
		sendMailAlternateAddress = lamPasswordChangeMailAddress.value;
	}
	const pwdJSON = {
		"modules": modules,
		"password1": pwd1,
		"password2": pwd2,
		"random": random,
		"forcePasswordChange": forcePasswordChange,
		"sendMail": sendMail,
		"sendSms": sendSms,
		"sendMailAlternateAddress": sendMailAlternateAddress
	};
	let data = new FormData();
	data.append('jsonInput', JSON.stringify(pwdJSON));
	data.append(tokenName, tokenValue);
	// make AJAX call
	fetch(ajaxURL, {
		method: 'POST',
		body: data
	})
	.then(async response => {
		const jsonData = await response.json();
		document.querySelector(".modal").classList.remove("show-modal");
		Swal.fire({
			confirmButtonText: okText,
			html: jsonData.messages
		});
	});
}

/**
 * Appends the input fields of a dialog back to the form and submits it.
 *
 * @param dialogDiv ID of dialog div
 * @param formName name of form
 */
function appendDialogInputsToFormAndSubmit(dialogDiv, formName) {
	const inputFields = document.getElementById(dialogDiv).querySelectorAll('input');
	inputFields.forEach(item => {
		item.classList.add('hidden');
		document.forms[formName].append(item);
	});
	document.forms[formName].submit();
}

/**
 * Shows a simple confirmation dialog.
 * If the user presses Cancel then the current action is stopped (event.preventDefault()).
 *
 * @param title dialog title
 * @param text dialog text
 * @param okText text for OK button
 * @param cancelText text for cancel button
 * @param e event
 */
async function confirmLoadProfile(title, text, okText, cancelText, e) {
	const button = document.getElementById('btn_accountContainerLoadProfileButton');
	const lastProfile = button.dataset.lastprofile;
	const profiles = JSON.parse(button.dataset.profiles);
	const {value: selectedProfile} = await Swal.fire({
		confirmButtonText: okText,
		cancelButtonText: cancelText,
		showCancelButton: true,
		title: title,
		text: text,
		input: 'select',
		inputValue: lastProfile,
		inputOptions: profiles
	});
	if (selectedProfile) {
		const form = document.forms["inputForm"];
		let buttonValue = document.createElement("input");
		buttonValue.type = "hidden";
		buttonValue.name = "accountContainerLoadProfile";
		buttonValue.value = "yes";
		form.appendChild(buttonValue);
		let selectValue = document.createElement('input');
		selectValue.type = 'hidden';
		selectValue.name = 'accountContainerSelectLoadProfile';
		selectValue.value = selectedProfile;
		form.appendChild(selectValue);
		form.submit();
	}
	if (e.preventDefault) {
		e.preventDefault();
	}
	if (e.returnValue) {
		e.returnValue = false;
	}
	return false;
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
 * Shows the dialog to create a DNS zone.
 *
 * @param title dialog title
 * @param okText text for Ok button
 * @param cancelText text for Cancel button
 */
function bindShowNewZoneDialog(title, okText, cancelText) {
	window.lam.dialog.showSimpleDialog(title, okText, cancelText, 'newBindZoneDialogForm', 'newBindZoneDialog');
}


/**
 * Checks if the given field has the same value as the reference field.
 * Field is marked red if different and green if equal.
 *
 * @param fieldID ID of field to check
 * @param fieldIDReference ID of reference field
 */
function checkFieldsHaveSameValues(fieldID, fieldIDReference) {
	const field = document.getElementById(fieldID);
	const fieldRef = document.getElementById(fieldIDReference);
	const check =
		function() {
			const value = field.value;
			const valueRef = fieldRef.value;
			if ((value == '') && (valueRef == '')) {
				field.classList.remove('markFail');
				field.classList.remove('markOk');
			}
			else {
				if (value == valueRef) {
					field.classList.remove('markFail');
					field.classList.add('markOk');
				}
				else {
					field.classList.add('markFail');
					field.classList.remove('markOk');
				}
			}
		};
	field.addEventListener('keyup', check);
	fieldRef.addEventListener('keyup', check);
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
	const field = document.getElementById(fieldID);
	const check =
		function() {
			const value = field.value;
			const pwdJSON = {
					"password": value
			};
			let data = new FormData();
			data.append('jsonInput', JSON.stringify(pwdJSON));
			data.append(tokenName, tokenValue);
			// make AJAX call
			fetch(ajaxURL + "&function=passwordStrengthCheck", {
				method: 'POST',
				body: data
			})
			.then(async response => {
				const jsonData = await response.json();
				checkPasswordStrengthHandleReply(jsonData, fieldID);
			});
		};
	field.addEventListener('keyup', check);
}

/**
 * Manages the server reply to a password strength check request.
 *
 * @param data JSON reply
 * @param fieldID input field ID
 */
function checkPasswordStrengthHandleReply(data, fieldID) {
	const field = document.getElementById(fieldID);
	if (data.result === true) {
		field.classList.remove('markFail');
		field.classList.add('markOk');
		field.title = '';
	}
	else if (field.value === '') {
		field.classList.remove('markFail');
		field.classList.remove('markOk');
	}
	else {
		field.classList.add('markFail');
		field.classList.remove('markOk');
		field.title = data.message;
	}
    const issuesDiv = document.getElementById(fieldID + '-pwd-strength-issues');
    issuesDiv.innerHTML = '';
    if (field.value !== '') {
        data.issues.forEach(issue => {
            const line = document.createElement('p');
            line.classList.add('password-strength');
            line.classList.add('password-strength-fail');
            line.innerText = issue;
            issuesDiv.appendChild(line);
        });
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
	const inputField = document.getElementById(filterInput);
	const selectField = document.getElementById(select);
	if (selectField.classList.contains('lam-dynamicOptions')) {
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
	selectField.classList.add('lam-filteredOptions');
	// if values were not yet saved, save them
	if (!selectField.dataset.options) {
		let options = {};
		selectField.querySelectorAll('option').forEach((item) => {
			options[item.value] = {
				selected: item.selected,
				text: item.innerText,
				cssClasses: item.classList
			};
		});
		selectField.dataset.options = JSON.stringify(options);
	}
	// save selected values
	let storedOptions = JSON.parse(selectField.dataset.options);
	selectField.querySelectorAll('option').forEach((item) => {
		storedOptions[item.value].selected = item.selected;
	});
	selectField.dataset.options = JSON.stringify(storedOptions);
	// get matching values
	selectField.innerHTML = '';
	let search = inputField.value.trim();
	search = search.replace('*', '.*');
	const regex = new RegExp(search,'gi');
	let index = 0;
	for (const value in storedOptions) {
		const option = storedOptions[value];
		if (option.text.match(regex) !== null) {
			const newOption = document.createElement('option');
			newOption.innerText = option.text;
			newOption.value = value;
			if (option.selected) {
				newOption.selected = true;
				if (selectField.size === 1) {
					selectField.selectedIndex = index;
				}
			}
			if (option.cssClasses) {
				newOption.classList = option.cssClasses;
			}
			selectField.appendChild(newOption);
		}
		index++;
	}
	// select first entry for single-selects
	if ((selectField.size === 1) && selectField.onchange) {
		selectField.onchange();
	}
}

/**
 * Adds a form listener to clear the filter on select elements.
 */
window.lam.filterSelect.addFormListener = function() {
	const forms = Array.from(document.forms);
	forms.forEach((form) => {
		form.addEventListener('submit', function () {
			const selectFields = form.querySelectorAll('.lam-filteredOptions[multiple]');
			selectFields.forEach((item) => {

				const storedOptions = JSON.parse(item.dataset.options);
				item.querySelectorAll('option').forEach((optionItem) => {
					storedOptions[optionItem.value].selected = optionItem.selected;
				});
				item.innerHTML = '';
				for (const value in storedOptions) {
					const option = storedOptions[value];
					if (option.selected) {
						const newOption = document.createElement('option');
						newOption.innerText = option.text;
						newOption.value = value;
						newOption.selected = true;
						item.appendChild(newOption);
					}
				}
			});
		})
	});
}

/**
 * Filters a select field with dynamic scrolling.
 *
 * @param inputField input field with filter value
 * @param selectField select field
 */
window.lam.filterSelect.filterDynamic = function(inputField, selectField) {
	let optionsOrig = selectField.dataset.dynamicOptionsOrig;
	if (optionsOrig === undefined) {
		selectField.dataset.dynamicOptionsOrig = selectField.dataset.dynamicOptions;
		optionsOrig = selectField.dataset.dynamicOptionsOrig;
	}
	optionsOrig = JSON.parse(optionsOrig);
	// save selection
	const optionTags = Array.from(selectField.children);
	optionTags.forEach((element) => {
		optionsOrig[parseInt(element.dataset.index)].selected = element.selected;
	});
	selectField.dataset.dynamicOptionsOrig = JSON.stringify(optionsOrig);
	selectField.innerHTML = '';
	const newOptions = [];
	// get matching values
	let search = inputField.value.trim();
	search = search.replace('*', '.*');
	const regex = new RegExp(search,'gi');
	optionsOrig.forEach(function(option, i) {
		if(option.label.match(regex) !== null) {
			newOptions.push(option);
		}
	});
	selectField.dataset.dynamicOptions = JSON.stringify(newOptions);
	selectField.append(window.lam.dynamicSelect.createOption('#', 0));
	window.lam.dynamicSelect.initSelect(selectField);
}

window.lam.dynamicSelect = window.lam.dynamicSelect || {};

/**
 * Activates dynamic selection for all marked select fields.
 */
window.lam.dynamicSelect.activate = function() {
	const dynamicSelects = document.querySelectorAll('.lam-dynamicOptions');
	dynamicSelects.forEach(item => {
		window.lam.dynamicSelect.initSelect(item);
	});
	const forms = Array.from(document.forms);
	forms.forEach((form) => {
		form.addEventListener('submit', function () {
			window.lam.dynamicSelect.formListener(form);
		})
	});
}

/**
 * Restores the right values before submitting a form.
 *
 * @param form HTMLFormElement
 */
window.lam.dynamicSelect.formListener = function(form) {
	form.querySelectorAll('.lam-dynamicOptions').forEach(select => {
		let dynamicOptions = select.dataset.dynamicOptions;
		if (select.dataset.dynamicOptionsOrig) {
			dynamicOptions = select.dataset.dynamicOptionsOrig;
		}
		dynamicOptions = JSON.parse(dynamicOptions);
		if (select.dataset.dynamicOptionsOrig) {
			// restore selections if list was filtered and selection was removed due to scrolling
			const dynamicOptionsOld = JSON.parse(select.dataset.dynamicOptions);
			for (const dynamicOptionOld of dynamicOptionsOld) {
				dynamicOptions[dynamicOptionOld.index].selected = dynamicOptionOld.selected;
			}
		}
		const children = Array.from(select.children);
		children.forEach((option) => {
			// get selection state from currently displayed options
			dynamicOptions[parseInt(option.dataset.index)].selected = option.selected;
		});
		select.innerHTML = '';
		dynamicOptions.forEach((item) => {
			if (item.selected) {
				select.appendChild(window.lam.dynamicSelect.createOption(item, item.index));
			}
		});
	});
}

/**
 * Sets up a select field for dynamic scrolling.
 *
 * @param selectField select
 */
window.lam.dynamicSelect.initSelect = function(selectField) {
	const optionTag = selectField.querySelector("option");
	if (optionTag) {
		selectField.dataset.optionHeight = optionTag.clientHeight;
	}
	else {
		selectField.dataset.optionHeight = 10;
	}
	selectField.dataset.selectHeight = selectField.clientHeight;
	selectField.dataset.selectLastScrollTop = '0';
	selectField.dataset.selectCurrentScroll = '0';
	selectField.innerHTML = '';
	const options = JSON.parse(selectField.dataset.dynamicOptions);
	const maxOptions = 3000;
	const numOfOptionBeforeToLoadNextSet = 10;
	const numberOfOptionsToLoad = 200;
	for (let i = 0; (i < maxOptions) && (i < options.length); i++) {
		selectField.append(window.lam.dynamicSelect.createOption(options[i], options[i].index));
	}
	if (options.length > maxOptions) {
		// activate scrolling logic only if enough options are set
		selectField.onscroll = function() {
			window.lam.dynamicSelect.onScroll(selectField, maxOptions, numOfOptionBeforeToLoadNextSet, numberOfOptionsToLoad);
		};
	}
}

/**
 * Creates an option field inside the select.
 *
 * @param data option data
 * @param index index in list of all options
 * @returns HTMLOptionElement
 */
window.lam.dynamicSelect.createOption = function(data, index) {
	const newOption = document.createElement("option");
	newOption.setAttribute('value', data.value);
	newOption.dataset.index = index;
	newOption.textContent = data.label;
	if (data.selected) {
		newOption.selected = true;
	}
	return newOption;
}

/**
 * Onscroll event.
 *
 * @param selectField select field
 * @param maxOptions maximum options to show
 * @param numOfOptionBeforeToLoadNextSet number of options to reach before end of list
 * @param numberOfOptionsToLoad number of options to add
 */
window.lam.dynamicSelect.onScroll = function(selectField, maxOptions, numOfOptionBeforeToLoadNextSet, numberOfOptionsToLoad) {
	const scrollTop = selectField.scrollTop;
	const totalHeight = selectField.querySelectorAll("option").length * selectField.dataset.optionHeight;
	const lastScrollTop = parseInt(selectField.dataset.selectLastScrollTop);
	const selectBoxHeight = parseInt(selectField.dataset.selectHeight);
	const singleOptionHeight = parseInt(selectField.dataset.optionHeight);
	const currentScroll = scrollTop + selectBoxHeight;
	selectField.dataset.selectCurrentScrollTop = scrollTop;
	if ((scrollTop >= lastScrollTop)
		&& ((currentScroll + (numOfOptionBeforeToLoadNextSet * singleOptionHeight)) >= totalHeight)) {
		window.lam.dynamicSelect.loadNextOptions(selectField, maxOptions, numberOfOptionsToLoad);
	}
	else if ((scrollTop <= lastScrollTop)
		&& ((scrollTop - (numOfOptionBeforeToLoadNextSet * singleOptionHeight)) <= 0)) {
		window.lam.dynamicSelect.loadPreviousOptions(selectField, maxOptions, numberOfOptionsToLoad);
	}
	selectField.dataset.selectLastScrollTop = scrollTop;
}

/**
 * Loads the next bunch of options at the end.
 *
 * @param selectField select field
 * @param maxOptions maximum options to show
 * @param numberOfOptionsToLoad number of options to add
 */
window.lam.dynamicSelect.loadNextOptions = function(selectField, maxOptions, numberOfOptionsToLoad) {
	if (selectField.children.length === 0) {
		return;
	}
	const selectBoxHeight = parseInt(selectField.dataset.selectHeight);
	const singleOptionHeight = parseInt(selectField.dataset.optionHeight);
	const currentScrollPosition = parseInt(selectField.dataset.selectCurrentScrollTop) + selectBoxHeight;
	const options = JSON.parse(selectField.dataset.dynamicOptions);
	let lastIndex = parseInt(selectField.children[selectField.children.length - 1].dataset.index);
	for (let i = 0; i < options.length; i++) {
		if (options[i].index === lastIndex) {
			lastIndex = i;
			break;
		}
	}
	for (let toAdd = 0; toAdd < numberOfOptionsToLoad; toAdd++) {
		const addPos = lastIndex + 1 + toAdd;
		if (options[addPos] === undefined) {
			break;
		}
		selectField.append(window.lam.dynamicSelect.createOption(options[addPos], options[addPos].index));
	}
	const numberOfOptions = selectField.children.length;
	let toRemove = numberOfOptions - maxOptions;
	if (toRemove > 0) {
		for (let i = toRemove; i >= 0; i--) {
			const optionToRemove = selectField.children[i];
			const indexToRemove = parseInt(optionToRemove.dataset.index);
			for (const option of options) {
				if (option.index === indexToRemove) {
					option.selected = optionToRemove.selected;
				}
			}
			optionToRemove.remove();
		}
	}
	else {
		toRemove = 0;
	}
	selectField.scrollTop = currentScrollPosition - selectBoxHeight - (toRemove * singleOptionHeight);
	selectField.dataset.dynamicOptions = JSON.stringify(options);
}

/**
 * Loads the next bunch of options at the beginning.
 *
 * @param selectField select field
 * @param maxOptions maximum options to show
 * @param numberOfOptionsToLoad number of options to add
 */
window.lam.dynamicSelect.loadPreviousOptions = function(selectField, maxOptions, numberOfOptionsToLoad) {
	if (selectField.children.length === 0) {
		return;
	}
	const singleOptionHeight = parseInt(selectField.dataset.optionHeight);
	const currentScrollPosition = parseInt(selectField.dataset.selectCurrentScrollTop);
	const options = JSON.parse(selectField.dataset.dynamicOptions);
	let lastIndex = parseInt(selectField.children[0].dataset.index);
	for (let i = 0; i < options.length; i++) {
		if (options[i].index === lastIndex) {
			lastIndex = i;
			break;
		}
	}
	let added = 0;
	for (let toAdd = 0; toAdd < numberOfOptionsToLoad; toAdd++) {
		const addPos = lastIndex - 1 - toAdd;
		if (options[addPos] === undefined) {
			break;
		}
		added++;
		selectField.prepend(window.lam.dynamicSelect.createOption(options[addPos], options[addPos].index));
	}
	const numberOfOptions = selectField.children.length;
	const toRemove = numberOfOptions - maxOptions;
	if (toRemove > 0) {
		for (let i = maxOptions; i < selectField.children.length; i++) {
			const optionToRemove = selectField.children[i];
			const indexToRemove = parseInt(optionToRemove.dataset.index);
			for (const option of options) {
				if (option.index === indexToRemove) {
					option.selected = optionToRemove.selected;
				}
			}
			optionToRemove.remove();
		}
	}
	selectField.scrollTop = currentScrollPosition + (added * singleOptionHeight);
	selectField.dataset.dynamicOptions = JSON.stringify(options);
}

window.lam.upload = window.lam.upload || {};

/**
 * Toggles the display of selectable modules.
 *
 * @param selectBox account type selection
 */
window.lam.upload.changeVisibleModules = function(selectBox) {
	document.querySelectorAll('div.typeOptions').forEach(item => {
		item.classList.add('hidden');
	});
	document.getElementById(selectBox.options[selectBox.selectedIndex].value).classList.toggle('hidden');
}

/**
 * Continues a CSV file upload.
 *
 * @param url URL where to get status JSON
 * @param tokenName name of CSRF token
 * @param tokenValue value of CSRF token
 */
window.lam.upload.continueUpload = function(url, tokenName, tokenValue) {
	let data = new FormData();
	data.append(tokenName, tokenValue);
	fetch(url, {
		method: 'POST',
		body: data
	})
	.then(async response => {
		const jsonData = await response.json();
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
	let htmlOut = '<div class="title">';
	htmlOut += '<h2 class="titleText">' + jsonData.title + '</h2>';
	htmlOut += '</div>';
	htmlOut += window.lam.progressbar.getMarkup('progressbarGeneral');
	document.getElementById('uploadContent').innerHTML = htmlOut;
	window.lam.progressbar.setProgress('progressbarGeneral', jsonData.accountsProgress);
};

/**
 * Prints the upload status when post actions run.
 *
 * @param jsonData status JSON
 */
window.lam.upload.printPostActionStatus = function(jsonData) {
	let htmlOut = '<div class="title">';
	htmlOut += '<h2 class="titleText">' + jsonData.title + '</h2>';
	htmlOut += '</div>';
	htmlOut += window.lam.progressbar.getMarkup('progressbarGeneral');
	if (jsonData.postActionsTitle) {
		htmlOut += '<h2>' + jsonData.postActionsTitle + '</h2>';
		htmlOut += window.lam.progressbar.getMarkup('progressbarPostActions');
	}
	document.getElementById('uploadContent').innerHTML = htmlOut;
	window.lam.progressbar.setProgress('progressbarGeneral', 100);
	if (jsonData.postActionsTitle) {
		window.lam.progressbar.setProgress('progressbarPostActions', jsonData.postActionsProgress);
	}
};

/**
 * Prints the upload status when PDFs are generated.
 *
 * @param jsonData status JSON
 */
window.lam.upload.printPDFStatus = function(jsonData) {
	let htmlOut = '<div class="title">';
	htmlOut += '<h2 class="titleText">' + jsonData.title + '</h2>';
	htmlOut += '</div>';
	htmlOut += window.lam.progressbar.getMarkup('progressbarGeneral');
	htmlOut += '<h2>' + jsonData.titlePDF + '</h2>';
	htmlOut += window.lam.progressbar.getMarkup('progressbarPDF');
	document.getElementById('uploadContent').innerHTML = htmlOut;
	window.lam.progressbar.setProgress('progressbarGeneral', 100);
	window.lam.progressbar.setProgress('progressbarPDF', jsonData.pdfProgress);
};

/**
 * Upload finished, check for errors.
 *
 * @param jsonData status JSON
 */
window.lam.upload.uploadDone = function(jsonData) {
	if (jsonData.errorHtml) {
		let htmlOut = '<div class="subTitle">';
		htmlOut += '<h4  class="subTitleText">' + jsonData.titleErrors + '</h4>';
		htmlOut += '</div>';
		htmlOut += jsonData.errorHtml;
		document.getElementById('uploadContent').innerHTML = htmlOut;
	}
	else {
		top.location.href = '../lists/list.php?type=' + jsonData.typeId + '&uploadAllOk';
	}
};

window.lam.form = window.lam.form || {};

/**
 * Trims all marked input elements on form submission.
 */
window.lam.form.autoTrim = function() {
	for (let singleForm of document.forms) {
		singleForm.addEventListener('submit', function() {
			const autoTrimFields = singleForm.querySelectorAll('.lam-autotrim');
			autoTrimFields.forEach(item => {
				item.value = String(item.value).trim();
			});
		});
	}
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
 * Shows an error dialog.
 *
 * @param title dialog title
 * @param text dialog text
 * @param okText ok button text
 */
window.lam.dialog.showError = function(title, text, okText) {
	Swal.fire({
		icon: 'error',
		title: title,
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
 * @param callbackFunction callback function on close (optional)
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
 * Shows a confirmation message.
 *
 * @param title dialog title
 * @param okText ok button text
 * @param cancelText cancel button text
 * @param divId DIV id with dialog content
 * @param callbackFunction callback function (optional, gets result of preConfirmFunction as parameter)
 * @param preConfirmFunction preConfirm function (optional)
 * @param postDisplayFunction called after dialog gets visible (optional)
 */
window.lam.dialog.showConfirmation = async function(title, okText, cancelText, divId, callbackFunction, preConfirmFunction, postDisplayFunction) {
	const div = document.getElementById(divId);
	const dialogContent = div.cloneNode(true);
	const divContent = div.innerHTML;
	div.innerHTML = '';
	dialogContent.classList.remove('hidden');
	let options = {
		title: title,
		confirmButtonText: okText,
		cancelButtonText: cancelText,
		showCancelButton: true,
		html: dialogContent.outerHTML,
		width: 'auto'
	};
	if (preConfirmFunction) {
		options.preConfirm = preConfirmFunction;
	}
	if (postDisplayFunction) {
		options.didOpen = postDisplayFunction;
	}
	const {value: formValues} = await Swal.fire(options);
	if (callbackFunction && formValues) {
		callbackFunction(formValues);
	}
	div.innerHTML = divContent;
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

/**
 * Shows a success message and redirects to the given URL when dialog is closed.
 * Dialog auto-closes after 3 seconds.
 *
 * @param title title
 * @param message detail message
 * @param buttonText text for confirm button
 * @param redirectUrl redirect URL
 */
window.lam.dialog.showSuccessMessageAndRedirect = function(title, message, buttonText, redirectUrl) {
	Swal.fire({
		icon: 'success',
		title: title,
		confirmButtonText: buttonText,
		text: message,
		width: 'auto',
		timer: 3000,
		timerProgressBar: true
	}).then(result => {
		window.location = redirectUrl;
	});
}

/**
 * Shows an error message and redirects to the given URL when dialog is closed.
 *
 * @param title title
 * @param message detail message
 * @param buttonText text for confirm button
 * @param redirectUrl redirect URL
 */
window.lam.dialog.showErrorMessageAndRedirect = function(title, message, buttonText, redirectUrl) {
	Swal.fire({
		icon: 'error',
		title: title,
		confirmButtonText: buttonText,
		text: message,
		width: 'auto'
	}).then(result => {
		window.location = redirectUrl;
	});
}

window.lam.account = window.lam.account || {};

/**
 * Adds a listener on the link to set default profile.
 */
window.lam.account.addDefaultProfileListener = function() {
	const defaultProfileLink = document.getElementById('lam-make-default-profile');
	if (defaultProfileLink) {
		defaultProfileLink.addEventListener('click', function() {
			const typeId = this.dataset.typeid;
			const name = this.dataset.name;
			const okText = this.dataset.ok;
			let date = new Date();
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
	const selects = document.querySelectorAll('select.lam-save-selection');
	selects.forEach(item => {
		const name = item.name;
		const storageKey = 'lam_selectionStore_' + name;
		// load value from local storage
		const storageValue = window.localStorage.getItem(storageKey);
		if (storageValue) {
			const option = item.querySelector('option[value="' + storageValue + '"]');
			if (option) {
				item.value = storageValue;
			}
		}
		// add change listener
		item.addEventListener('change', function() {
			const selectedValue = item.value;
			window.localStorage.setItem(storageKey, selectedValue);
		});
	});
	const checkboxes = document.querySelectorAll('input.lam-save-selection');
	checkboxes.forEach(item => {
		const name = item.name;
		const storageKey = 'lam_selectionStore_' + name;
		// load value from local storage
		const storageValue = window.localStorage.getItem(storageKey);
		if (storageValue) {
			item.checked = (storageValue === 'true');
		}
		// add change listener
		item.addEventListener('change', function() {
			const selectedValue = item.checked;
			window.localStorage.setItem(storageKey, selectedValue);
		});
	});
};

/**
 * Sets the focus on the initial field.
 */
window.lam.tools.setInitialFocus = function() {
	const elementToFocus = document.querySelector('.lam-initial-focus');
	if (elementToFocus) {
		elementToFocus.focus();
	}
};

window.lam.tools.webcam = window.lam.tools.webcam || {};

/**
 * Initializes the webcam capture.
 */
window.lam.tools.webcam.init = function() {
	const contentDiv = document.getElementById('lam_webcam_div');
	if (contentDiv && navigator.mediaDevices && navigator.mediaDevices.enumerateDevices) {
		navigator.mediaDevices.enumerateDevices()
			.then(function(mediaDevices) {
				for (let i = 0; i < mediaDevices.length; i++) {
					const mediaDevice = mediaDevices[i];
					if (mediaDevice.kind === 'videoinput') {
						contentDiv.classList.remove('hidden');
					}
				}
			});
	}
};

/**
 * Starts the webcam capture.
 */
window.lam.tools.webcam.capture = function(event) {
	event.preventDefault();
	const video = document.getElementById('lam-webcam-video');
	const msg = document.querySelector('.lam-webcam-message');
	msg.classList.add('hidden');
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
			document.getElementById('btn_lam-webcam-capture').classList.add('hidden');
			document.querySelector('.btn-lam-webcam-upload').classList.remove('hidden');
			document.getElementById('lam-webcam-video').classList.remove('hidden');
		})
		.catch(function(err) {
			msg.querySelector('.statusTitle').textContent = err.message;
			msg.classList.remove('hidden');
		});
	return false;
}

/**
 * Starts the webcam upload.
 */
window.lam.tools.webcam.upload = function() {
	const form = document.getElementById('lam-webcam-canvas').closest('form');
	const canvasData = window.lam.tools.webcam.prepareData();
	const canvasDataInput = document.createElement('input');
	canvasDataInput.setAttribute('name', 'webcamData');
	canvasDataInput.setAttribute('id', 'webcamData');
	canvasDataInput.setAttribute('type', 'hidden');
	canvasDataInput.setAttribute('value', canvasData);
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
	const msg = document.querySelector('.lam-webcam-message');
	const canvasData = window.lam.tools.webcam.prepareData();
	let data = new FormData();
	data.append('webcamData', canvasData);
	data.append(tokenName, tokenValue);
	const ajaxURL = '../misc/ajax.php?selfservice=1&action=ajaxPhotoUpload'
		+ '&module=' + moduleName + '&scope=' + scope;
	fetch(ajaxURL, {
		method: 'POST',
		body: data
	})
	.then(async response => {
		if (response.ok === false) {
			msg.querySelector('.statusTitle').innerText = uploadErrorMessage;
			msg.classList.remove('hidden');
			return;
		}
		const jsonData = await response.json();
		if (jsonData.success) {
			if (jsonData.html) {
				document.getElementById(contentId).innerHTML = jsonData.html;
				window.lam.tools.webcam.init();
				window.lam.html.initCropping();
			}
		}
		else if (jsonData.error) {
			msg.querySelector('.statusTitle').innerText = jsonData.error;
			msg.classList.remove('hidden');
		}
	});
	document.getElementById('btn_lam-webcam-capture').classList.remove('hidden');
	document.getElementById('btn-lam-webcam-upload').classList.add('hidden');
	return false;
}

/**
 * Starts the webcam upload.
 *
 * @return webcam data as string
 */
window.lam.tools.webcam.prepareData = function() {
	const canvas = document.getElementById('lam-webcam-canvas');
	const video = document.getElementById('lam-webcam-video');
	canvas.setAttribute('width', video.videoWidth);
	canvas.setAttribute('height', video.videoHeight);
	const context = canvas.getContext('2d');
	context.drawImage(video, 0, 0, video.videoWidth, video.videoHeight);
	const canvasData = canvas.toDataURL("image/png");
	video.pause();
	const tracks = window.lam.tools.webcamStream.getTracks();
	for (let i = 0; i < tracks.length; i++) {
		tracks[i].stop();
	}
	canvas.classList.add('hidden');
	video.classList.add('hidden');
	return canvasData;
}

window.lam.tools.schema = window.lam.tools.schema || {};

/**
 * Adds the onChange listener to schema selections.
 */
window.lam.tools.schema.select = function() {
	const select = document.getElementById('lam-schema-select');
	if (!select) {
		return;
	}
	const display = select.dataset.display;
	select.addEventListener('change', function() {
		const value = select.value;
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
	if (document.readyState === 'loading') {
		document.addEventListener('DOMContentLoaded', function() {
			window.lam.importexport.startImport(tokenName, tokenValue);
		});
		return;
	}
	const output = document.getElementById('importResults');
	let data = new FormData();
	data.append(tokenName, tokenValue);
	const ajaxURL = '../misc/ajax.php?function=import';
	fetch(ajaxURL, {
		method: 'POST',
		body: data
	})
	.then(async response => {
		const jsonData = await response.json();
		if (jsonData.data && (jsonData.data !== '')) {
			output.innerHTML += jsonData.data;
		}
		if (jsonData.status == 'done') {
			document.getElementById('progressbarImport').classList.add('hidden');
			document.getElementById('btn_submitImportCancel').classList.add('hidden');
			document.getElementById('statusImportInprogress').classList.add('hidden');
			document.getElementById('statusImportDone').classList.remove('hidden');
			document.querySelector('.newimport').classList.remove('hidden');
		}
		else if (jsonData.status == 'failed') {
			document.getElementById('btn_submitImportCancel').classList.add('hidden');
			document.getElementById('statusImportInprogress').classList.add('hidden');
			document.getElementById('statusImportFailed').classList.remove('hidden');
			document.querySelector('.newimport').classList.remove('hidden');
		}
		else {
			window.lam.progressbar.setProgress('progressbarImport', jsonData.progress);
			window.lam.importexport.startImport(tokenName, tokenValue);
		}
	});
};

/**
 * Starts the export process.
 *
 * @param tokenName name of CSRF token
 * @param tokenValue value of CSRF token
 */
window.lam.importexport.startExport = function(tokenName, tokenValue) {
	if (document.readyState === 'loading') {
		document.addEventListener('DOMContentLoaded', function() {
			window.lam.importexport.startExport(tokenName, tokenValue);
		});
		return;
	}
	window.lam.progressbar.setProgress('progressbarExport', 50);
	const output = document.getElementById('exportResults');
	let data = new FormData();
	data.append(tokenName, tokenValue);
	data.append('baseDn', document.getElementById('baseDn').value);
	data.append('searchScope', document.getElementById('searchScope').value);
	data.append('filter', document.getElementById('filter').value);
	data.append('attributes', document.getElementById('attributes').value);
	data.append('format', document.getElementById('format').value);
	data.append('ending', document.getElementById('ending').value);
	data.append('includeSystem', document.getElementById('includeSystem').value);
	data.append('saveAsFile', document.getElementById('saveAsFile').value);
	fetch('../misc/ajax.php?function=export', {
		method: 'POST',
		body: data
	})
	.then(async response => {
		const jsonData = await response.json();
		if (jsonData.data && (jsonData.data != '')) {
			output.append(jsonData.data);
		}
		if (jsonData.status == 'done') {
			document.getElementById('progressbarExport').classList.add('hidden');
			document.getElementById('btn_submitExportCancel').classList.add('hidden');
			document.getElementById('statusExportInprogress').classList.add('hidden');
			document.getElementById('statusExportDone').classList.remove('hidden');
			document.querySelector('.newexport').classList.remove('hidden');
			if (jsonData.output) {
				document.getElementById('exportResults').querySelector('pre').innerText = jsonData.output;
			}
			else if (jsonData.file) {
				window.open(jsonData.file, '_blank');
			}
		}
		else {
			document.getElementById('progressbarExport').classList.add('hidden');
			document.getElementById('btn_submitExportCancel').classList.add('hidden');
			document.getElementById('statusExportDone').classList.add('hidden');
			document.getElementById('statusExportInprogress').classList.add('hidden');
			document.getElementById('statusExportFailed').classList.remove('hidden');
			document.querySelector('.newexport').classList.remove('hidden');
		}
	})
	.catch((error) => {
		document.getElementById('progressbarExport').classList.add('hidden');
		document.getElementById('btn_submitExportCancel').classList.add('hidden');
		document.getElementById('statusExportDone').classList.add('hidden');
		document.getElementById('statusExportInprogress').classList.add('hidden');
		document.getElementById('statusExportFailed').classList.remove('hidden');
		document.querySelector('.newexport').classList.remove('hidden');
	});
};

window.lam.multiedit = window.lam.multiedit || {};

window.lam.multiedit.runActions = function() {
	// disable all input elements
	document.querySelectorAll('input, select, button').forEach(function (item) {
		item.disabled = true;
	});
	// run actions
	fetch('multiEdit.php?ajaxStatus', {
		method: 'GET'
	})
	.then(async response => {
		const jsonData = await response.json();
		window.lam.multiedit.runActionsHandleReply(jsonData);
	});
}

window.lam.multiedit.runActionsHandleReply = function(data) {
	window.lam.progressbar.setProgress('progressBar', data.progress);
	document.getElementById('progressArea').innerHTML = data.content;
	if (data.status != "finished") {
		fetch('multiEdit.php?ajaxStatus', {
			method: 'GET'
		})
		.then(async response => {
			const jsonData = await response.json();
			window.lam.multiedit.runActionsHandleReply(jsonData);
		});
	}
	else {
		document.querySelectorAll('input, select, button').forEach(function (item) {
			item.disabled = false;
		});
		document.getElementById('progressBar').classList.add('hidden');
	}
}

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
	let data = new FormData();
	data.append(tokenName, tokenValue);
	data.append('fieldId', fieldId);
	data.append('dn', dnValue);
	fetch('../misc/ajax.php?function=dnselection', {
		method: 'POST',
		body: data
	})
	.then(async response => {
		const jsonData = await response.json();
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
	const field = document.getElementById(fieldId);
	let parent = el.parentElement;
	while (!parent.classList.contains('row')) {
		parent = parent.parentElement;
	}
	const dn = parent.dataset.dn;
	field.value = dn;
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
	let parent = el.parentElement;
	while (!parent.classList.contains('row')) {
		parent = parent.parentElement;
	}
	const dn = parent.dataset.dn;
	let data = new FormData();
	data.append(tokenName, tokenValue);
	data.append('fieldId', fieldId);
	data.append('dn', dn);
	fetch('../misc/ajax.php?function=dnselection', {
		method: 'POST',
		body: data
	})
	.then(async response => {
		const jsonData = await response.json();
		document.getElementById('dlg_' + fieldId).innerHTML = jsonData.dialogData;
	})
	.catch((error) => {
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
	document.querySelectorAll('.lam-prevent-enter').forEach(item => {
		item.addEventListener('keypress', function (event) {
			if (event.key === 'Enter') {
				event.preventDefault();
			}
		});
	});
}

/**
 * Inits the cropping area on images.
 */
window.lam.html.initCropping = function() {
	const image = document.querySelector('.cropperjsImage');
	const cropper = new Cropper.default(image);
    const inSelection = function(selection, maxSelection) {
        return (
            selection.x >= maxSelection.x
            && selection.y >= maxSelection.y
            && (selection.x + selection.width) <= (maxSelection.x + maxSelection.width)
            && (selection.y + selection.height) <= (maxSelection.y + maxSelection.height)
        );
    };
    cropper.getCropperSelection().onchange = function(event) {
        const cropperCanvas = cropper.getCropperCanvas();
        if (!cropperCanvas) {
            return;
        }
        const cropperImageRect = cropper.getCropperImage().getBoundingClientRect();
        const cropperCanvasRect = cropperCanvas.getBoundingClientRect();
        const offsetLeft = cropperImageRect.left - cropperCanvasRect.left;
        const offsetTop = cropperImageRect.top - cropperCanvasRect.top;
        const maxSelection = {
            x: offsetLeft,
            y: offsetTop,
            width: cropperImageRect.width,
            height: cropperImageRect.height,
        };
        const selection = event.detail;
        if (!inSelection(selection, maxSelection)) {
            event.preventDefault();
            return;
        }
        document.getElementById('croppingDataX').value = selection.x - offsetLeft;
        document.getElementById('croppingDataY').value = selection.y - offsetTop;
        document.getElementById('croppingDataSelectionWidth').value = selection.width;
        document.getElementById('croppingDataSelectionHeight').value = selection.height;
        document.getElementById('croppingDataWidth').value = cropperImageRect.width;
        document.getElementById('croppingDataHeight').value = cropperImageRect.height;
    };
    cropper.getCropperImage().$ready(function() {
        const cropperImageRect = cropper.getCropperImage().getBoundingClientRect();
        const cropperCanvasRect = cropper.getCropperCanvas().getBoundingClientRect();
        cropper.getCropperSelection().$change(
            cropperImageRect.left - cropperCanvasRect.left + (0.1 * cropperImageRect.width),
            cropperImageRect.top - cropperCanvasRect.top + (0.1 * cropperImageRect.height),
            cropperImageRect.width - (0.2 * cropperImageRect.width),
            cropperImageRect.height - (0.2 * cropperImageRect.height));
    });
    window.mycropper = cropper;
}

/**
 * Updates the field to store the sorting of an accordion.
 *
 * @param accordionId ID of accordion container
 */
window.lam.html.updateAccordionSorting = function (accordionId) {
	const positionsField = document.getElementById(accordionId + '_sorting');
	let positions = [];
	document.getElementById(accordionId).querySelectorAll('.lam-accordion-button').forEach(function (item) {
		const index = item.dataset.index;
		positions.push(index);
	});
	positionsField.value = positions.join(',');
}

window.lam.selfservice = window.lam.selfservice || {};

/**
 * Deletes a value of a multi-value field.
 *
 * @param fieldNamePrefix prefix of input field name
 * @param delButton delete button that was clicked
 */
window.lam.selfservice.delMultiValue = function(fieldNamePrefix, delButton) {
	const fields = document.querySelectorAll("input[name^='" + fieldNamePrefix + "'], textarea[name^='" + fieldNamePrefix + "']");
	const isOnlyOneField = (fields.length === 1);
	if (!isOnlyOneField) {
		// move add button if present
		const addButton = delButton.nextElementSibling;
		if (addButton) {
			const lastLastDelLink = fields[fields.length - 2].parentElement.parentElement.querySelector('.del-link');
			const lastLastDelLinkParent = lastLastDelLink.parentElement;
			lastLastDelLinkParent.append(addButton);
		}
		// delete row
		let parent = delButton.parentElement;
		while (!parent.classList.contains('row')) {
			parent = parent.parentElement;
		}
		const row = parent.parentElement;
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
	const fields = document.querySelectorAll("input[name^='" + fieldNamePrefix + "'], textarea[name^='" + fieldNamePrefix + "']");
	// get next field number
	const lastFieldName = fields[fields.length - 1].name;
	const lastFieldNameIndex = lastFieldName.substring(fieldNamePrefix.length);
	const newFieldNameIndex = parseInt(lastFieldNameIndex) + 1;
	// copy row
	let parent = addButton.parentElement;
	while (!parent.classList.contains('row')) {
		parent = parent.parentElement;
	}
	const row = parent.parentElement;
	const clone = row.cloneNode(true);
	row.parentElement.append(clone);
	const cloneInput = clone.querySelector("input[name^='" + fieldNamePrefix + "'], textarea[name^='" + fieldNamePrefix + "']");
	cloneInput.name = fieldNamePrefix + newFieldNameIndex;
	cloneInput.id = fieldNamePrefix + newFieldNameIndex;
	cloneInput.value = '';
	// delete add link from old row
	addButton.remove();
};

window.lam.whitePages = window.lam.whitePages || {};

/**
 * Shows the detail view for an entry.
 *
 * @param dn base64 encoded DN
 * @param tabIndex tab index
 * @param okText label for Ok button
 * @param tokenName CSRF token name
 * @param tokenValue CSRF token value
 * @param dnBefore DN that was opened before, reopen on close
 * @param tabIndexBefore tab index of DN that was opened before, reopen on close
 */
window.lam.whitePages.showDetailView = function (dn, tabIndex, okText, tokenName, tokenValue, dnBefore, tabIndexBefore) {
    let data = new FormData();
    data.append(tokenName, tokenValue);
    data.append('action', 'displayDetails');
    data.append('dn', dn);
    data.append('tabIndex', tabIndex);
    fetch('../misc/ajax.php?whitepages=1&function=whitePages', {
        method: 'POST',
        body: data
    })
    .then(async response => {
        const jsonData = await response.json();
        if (jsonData.error) {
            window.lam.dialog.showError(null, jsonData.error, okText);
            return;
        }
        document.getElementById('detailView').innerHTML = jsonData.content;
        window.lam.dialog.showMessage(jsonData.title, okText, 'detailView',
        function () {
            if (dnBefore) {
                window.lam.whitePages.showDetailView(dnBefore, tabIndexBefore, okText, tokenName, tokenValue);
            }
        });
    })
}

/**
 * Switches the view to gallery mode.
 */
window.lam.whitePages.switchToGallery = function(tabIndex) {
    document.getElementById('container-list_' + tabIndex).classList.add('hidden');
    document.getElementById('modeSwitchGallery_' + tabIndex).classList.add('hidden');
    document.getElementById('modeSwitchList_' + tabIndex).classList.remove('hidden');
    document.getElementById('container-gallery_' + tabIndex).classList.remove('hidden');
}

/**
 * Switches the view to list mode.
 */
window.lam.whitePages.switchToList = function(tabIndex) {
    document.getElementById('container-list_' + tabIndex).classList.remove('hidden');
    document.getElementById('modeSwitchGallery_' + tabIndex).classList.remove('hidden');
    document.getElementById('modeSwitchList_' + tabIndex).classList.add('hidden');
    document.getElementById('container-gallery_' + tabIndex).classList.add('hidden');
}

/**
 * Adds the search listener for the gallery view.
 *
 * @param containerId container ID
 * @param searchFieldId search field ID
 */
window.lam.whitePages.addGallerySearchListener = function(containerId, searchFieldId) {
    document.getElementById(searchFieldId).addEventListener('keyup', function(event) {
        const container = document.getElementById(containerId);
        const cards = container.querySelectorAll('.whitepages-card');
        const filterValueString = document.getElementById(searchFieldId).value;
        const filterValues = filterValueString.split(/\s+/);
        cards.forEach(card => {
            let found = true;
            const searchData = card.dataset._lam_search_;
            filterValues.forEach(value => {
                found = found && searchData.toLowerCase().includes(value);
            })
            if (found && card.classList.contains('hidden')) {
                card.classList.remove('hidden');
            }
            else if (!found && !card.classList.contains('hidden')) {
                card.classList.add('hidden');
            }
        })

    });
}

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
 * @param extraParam additional parameter for request (e.g. "selfservice=true")
 * @param newDeviceNameTitle title for new device name dialog
 * @param newDeviceNameLabel label for new device name
 * @param okText text for Ok button
 * @param cancelText text for cancelButton
 */
window.lam.webauthn.start = function(prefix, extraParam, newDeviceNameTitle, newDeviceNameLabel, okText, cancelText) {
	document.addEventListener("DOMContentLoaded", function(){
		window.lam.webauthn.run(prefix, extraParam, newDeviceNameTitle, newDeviceNameLabel, okText, cancelText);
	});
}

/**
 * Checks if the user is registered and starts login/registration.
 *
 * @param prefix path prefix for Ajax endpoint
 * @param extraParam additional parameter for request (e.g. "selfservice=true")
 * @param newDeviceNameTitle title for new device name dialog
 * @param newDeviceNameLabel label for new device name
 * @param okText text for Ok button
 * @param cancelText text for cancelButton
 */
window.lam.webauthn.run = function(prefix, extraParam, newDeviceNameTitle, newDeviceNameLabel, okText, cancelText) {
	const skipButton = document.getElementById('btn_skip_webauthn');
	if (skipButton) {
		skipButton.onclick = function () {
			const form = document.getElementById("2faform");
			const hiddenSkip = document.createElement('input');
			hiddenSkip.type = 'hidden';
			hiddenSkip.name = 'sig_response';
			hiddenSkip.value = 'skip';
			form.appendChild(hiddenSkip);
			form.submit();
		};
	}
	const token = document.getElementById('sec_token').value;
	// check for webauthn support
	if (!navigator.credentials || (typeof(PublicKeyCredential) === "undefined")) {
		document.querySelector('.webauthn-error').classList.remove('hidden');
		return;
	}

	let data = new FormData();
	data.append('sec_token', token);
	data.append('action', 'status');
	extraParam = (extraParam) ? '&' + extraParam : '';
	fetch(prefix + 'misc/ajax.php?function=webauthn' + extraParam, {
		method: 'POST',
		body: data
	})
	.then(async response => {
		const jsonData = await response.json();
		if (jsonData.action === 'register') {
			const registerFunction = function() {
				const successCallback = async function (publicKeyCredential) {
					const {value: newName} = await Swal.fire({
						title: newDeviceNameTitle,
						confirmButtonText: okText,
						cancelButtonText: cancelText,
						showCancelButton: true,
						input: 'text',
						inputLabel: newDeviceNameLabel,
						width: 'auto'
					});
					const form = document.getElementById("2faform");
					if (newName) {
						const newNameElement = document.createElement('input');
						newNameElement.type = 'hidden';
						newNameElement.name = 'newName';
						newNameElement.value = newName;
						form.appendChild(newNameElement);
					}
					const response = btoa(JSON.stringify(publicKeyCredential));
					const hiddenResponse = document.createElement('input');
					hiddenResponse.type = 'hidden';
					hiddenResponse.name = 'sig_response';
					hiddenResponse.value = response;
					form.appendChild(hiddenResponse);
					form.submit();
				};
				const errorCallback = function(error) {
					const errorDiv = document.getElementById('generic-webauthn-error');
					const buttonLabel = errorDiv.dataset.button;
					const dialogTitle = errorDiv.dataset.title;
					errorDiv.innerText = error.message;
					window.lam.dialog.showMessage(dialogTitle,
						buttonLabel,
						'generic-webauthn-error',
						function () {
							document.getElementById('btn_logout').click();
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
	.catch(function(err) {
		console.log('WebAuthn failed: ' + err);
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
		let idOrig = publicKey.allowCredentials[i]['id'];
		idOrig = idOrig.replace(/-/g, "+").replace(/_/g, "/");
		const idOrigDecoded = atob(idOrig);
		const idArray = Uint8Array.from(idOrigDecoded, window.lam.webauthn.charAt)
		publicKey.allowCredentials[i]['id'] = idArray;
	}
	navigator.credentials.get({publicKey: publicKey})
		.then(function(data) {
			const publicKeyCredential = {
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
			const form = document.getElementById("2faform");
			const response = btoa(JSON.stringify(publicKeyCredential));
			const hiddenResponse = document.createElement('input');
			hiddenResponse.type = 'hidden';
			hiddenResponse.name = 'sig_response';
			hiddenResponse.value = response;
			form.appendChild(hiddenResponse);
			form.submit();
		}, function(error) {
			console.log(error.message);
			const errorDiv = document.getElementById('generic-webauthn-error');
			const buttonLabel = errorDiv.dataset.button;
			const dialogTitle = errorDiv.dataset.title;
			errorDiv.innerText = error.message;
			window.lam.dialog.showMessage(dialogTitle,
				buttonLabel,
				'generic-webauthn-error',
				function () {
					document.getElementById('btn_logout').click();
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
	return btoa(String.fromCharCode(...input))
        .replaceAll("=", "")
        .replaceAll("+", "-")
        .replaceAll("/", "_");
}

/**
 * Sets up the device management on the main configuration page.
 */
window.lam.webauthn.setupDeviceManagement = function() {
	const searchButton = document.getElementById('btn_webauthn_search');
	if (searchButton) {
		searchButton.onclick = window.lam.webauthn.searchDevices;
	}
	const searchInput = document.getElementById('webauthn_searchTerm');
	if (searchInput) {
		searchInput.onkeydown = function (event) {
			if (event.key === "Enter") {
				event.preventDefault();
				searchButton.click();
				return false;
			}
		};
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
	const resultDiv = document.getElementById('webauthn_results');
	const tokenValue = resultDiv.dataset.sec_token_value;
	const searchData = document.getElementById('webauthn_searchTerm').value;
	let data = new FormData();
	data.append('sec_token', tokenValue);
	data.append('action', 'search');
	data.append('searchTerm', searchData);
	fetch('../misc/ajax.php?function=webauthnDevices', {
		method: 'POST',
		body: data
	})
	.then(async response => {
		const jsonData = await response.json();
		resultDiv.innerHTML = jsonData.content;
		window.lam.webauthn.addDeviceActionListeners();
	})
	.catch(function (error) {
		console.log('WebAuthn search failed: ' + error.message);
	});
	return false;
}

/**
 * Adds listeners to the device action buttons.
 */
window.lam.webauthn.addDeviceActionListeners = function() {
	const inputs = document.querySelectorAll('.webauthn-delete');
	inputs.forEach(item => {
		item.onclick = function(event) {
			window.lam.webauthn.removeDevice(event);
			return false;
		};
	});
}

/**
 * Removes a webauthn device.
 *
 * @param event click event
 */
window.lam.webauthn.removeDevice = function(event) {
	event.preventDefault();
	const element = event.target;
	window.lam.webauthn.removeDeviceDialog(element, 'webauthnDevices');
}

/**
 * Removes a user's own webauthn device.
 *
 * @param event click event
 * @param isSelfService run in self-service or admin context
 */
window.lam.webauthn.removeOwnDevice = function(event, isSelfService) {
	event.preventDefault();
	const element = event.currentTarget;
	let successCallback = null;
	if (!isSelfService) {
		successCallback = function () {
			const form = document.getElementById("webauthnform");
			const hiddenRemoved = document.createElement('input');
			hiddenRemoved.type = 'hidden';
			hiddenRemoved.name = 'removed';
			hiddenRemoved.value = 'true';
			form.appendChild(hiddenRemoved);
			form.submit();
		};
	}
	let action = 'webauthnOwnDevices';
	if (isSelfService) {
		action = action + '&selfservice=true&module=webauthn&scope=user';
	}
	window.lam.webauthn.removeDeviceDialog(element, action, successCallback, isSelfService);
	return false;
}

/**
 * Opens the remove device dialog.
 *
 * @param element delete button
 * @param action action for request (delete|deleteOwn)
 * @param successCallback callback if all was fine (optional)
 * @param isSelfService run in self-service or admin context
 */
window.lam.webauthn.removeDeviceDialog = function(element, action, successCallback, isSelfService) {
	const dialogTitle = element.dataset.dialogtitle;
	const okText = element.dataset.oktext;
	const cancelText = element.dataset.canceltext;
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
			window.lam.webauthn.sendRemoveDeviceRequest(element, action, successCallback, isSelfService);
		}
	});
}

/**
 * Sends the remove request to server.
 *
 * @param element button element
 * @param action action (delete|deleteOwn)
 * @param successCallback callback if all was fine (optional)
 * @param isSelfService run in self-service or admin context
 */
window.lam.webauthn.sendRemoveDeviceRequest = function(element, action, successCallback, isSelfService) {
	const dn = element.dataset.dn;
	const credential = element.dataset.credential;
	const resultDiv = document.getElementById('webauthn_results');
	const tokenValue = resultDiv.dataset.sec_token_value;
	let data = new FormData();
	data.append('sec_token', tokenValue);
	data.append('action', 'delete');
	data.append('dn', dn);
	data.append('credentialId', credential);
	if (isSelfService) {
		document.querySelectorAll('.webauthn_device_name').forEach(item => {
			data.append(item.name, item.value);
		});
	}
	fetch('../misc/ajax.php?function=' + action, {
		method: 'POST',
		body: data
	})
	.then(async response => {
		const jsonData = await response.json();
		if (successCallback) {
			successCallback();
		}
		else {
			resultDiv.innerHTML = jsonData.content;
		}
	})
	.catch(function(err) {
		console.log('WebAuthn device deletion failed: ' + err.message);
	});
}

/**
 * Updates a device name.
 *
 * @param event click event
 */
window.lam.webauthn.updateOwnDeviceName = function(event) {
	event.preventDefault();
	const element = event.currentTarget;
	const dn = element.dataset.dn;
	const nameElementId = element.dataset.nameelement;
	const nameElement = document.getElementById(nameElementId);
	const name = nameElement.value;
	const credential = element.dataset.credential;
	const resultDiv = document.getElementById('webauthn_results');
	const tokenValue = resultDiv.dataset.sec_token_value;
	let data = new FormData();
	data.append('sec_token', tokenValue);
	data.append('action', 'setName');
	data.append('dn', dn);
	data.append('name', name);
	data.append('credentialId', credential);
	let action = 'webauthnOwnDevices';
	fetch('../misc/ajax.php?function=' + action, {
		method: 'POST',
		body: data
	})
	.then(async response => {
		window.location.href = 'webauthn.php?updated=' + encodeURIComponent(credential);
	})
	.catch(function(err) {
		console.log('WebAuthn device name change failed: ' + err.message);
	});
	return false;
}

/**
 * Registers a user's own webauthn device.
 *
 * @param event click event
 * @param isSelfService runs in self-service context
 */
window.lam.webauthn.registerOwnDevice = function(event, isSelfService) {
	event.preventDefault();
	const element = event.target;
	const dn = element.dataset.dn;
	const tokenValue = element.dataset.sec_token_value;
	const publicKey = JSON.parse(element.dataset.publickey);
	let successCallback = function (publicKeyCredential) {
		const form = document.getElementById("webauthnform");
		const response = btoa(JSON.stringify(publicKeyCredential));
		const registrationData = document.getElementById('registrationData');
		registrationData.value = response;
		form.submit();
	};
	if (isSelfService) {
		successCallback = function (publicKeyCredential) {
			let data = new FormData();
			data.append('sec_token', tokenValue);
			data.append('action', 'register');
			data.append('dn', dn);
			data.append('credential', btoa(JSON.stringify(publicKeyCredential)));
			document.querySelectorAll('.webauthn_device_name').forEach(item => {
				data.append(item.name, item.value);
			});
			fetch('../misc/ajax.php?selfservice=true&module=webauthn&scope=user', {
				method: 'POST',
				body: data
			})
			.then(async response => {
				const jsonData = await response.json();
				const resultDiv = document.getElementById('webauthn_results');
				resultDiv.innerHTML = jsonData.content;
			})
			.catch((error) => {
				console.log('WebAuthn device registration failed: ' + error.message);
			});
		};
	}
	const errorCallback = function (error) {
		const errorDiv = document.getElementById('generic-webauthn-error');
		const buttonLabel = errorDiv.dataset.button;
		const dialogTitle = errorDiv.dataset.title;
		errorDiv.innerText = error.message;
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
 * Creates a new node in tree view.
 *
 * @param tokenName security token name
 * @param tokenValue security token value
 * @param dn base64 encode DN
 */
window.lam.treeview.createNode = function (tokenName, tokenValue, dn) {
	let data = new FormData();
	data.append(tokenName, tokenValue);
	data.append('dn', dn);
	fetch('../misc/ajax.php?function=treeview&command=createNewNode&step=getObjectClasses', {
		method: 'POST',
		body: data
	})
	.then(async response => {
		const jsonData = await response.json();
		window.lam.treeview.checkSession(jsonData);
		document.getElementById('ldap_actionarea').innerHTML = jsonData.content;
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
	let data = new FormData();
	data.append(tokenName, tokenValue);
	data.append('dn', document.getElementById('parentDn').value);
	const selectedOptions = document.getElementById('objectClasses').selectedOptions;
	let objectClasses = [];
	for (const selectedOption of selectedOptions) {
		objectClasses.push(selectedOption.value);
	}
	data.append('objectClasses', objectClasses.join(','))
	fetch('../misc/ajax.php?function=treeview&command=createNewNode&step=checkObjectClasses', {
		method: 'POST',
		body: data
	})
	.then(async response => {
		const jsonData = await response.json();
		window.lam.treeview.checkSession(jsonData);
		document.getElementById('ldap_actionarea').innerHTML = jsonData.content;
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
	let data = new FormData();
	data.append(tokenName, tokenValue);
	const parentDn = document.getElementById('parentDn').value;
	data.append('dn', parentDn);
	data.append('rdn', document.getElementById('rdn').value);
	data.append('objectClasses', document.getElementById('objectClasses').value);
	// clear old values in data
	document.querySelectorAll('.single-input').forEach(item => {
			item.dataset.valueOrig = '';
		}
	);
	document.querySelectorAll('.multi-input').forEach(item => {
			item.dataset.valueOrig = '';
		}
	);
	// get attribute values
	const attributeChanges = window.lam.treeview.findAttributeChanges();
	data.append('attributes', JSON.stringify(attributeChanges));
	fetch("../misc/ajax.php?function=treeview&command=createNewNode&step=checkAttributes", {
		method: 'POST',
		body: data
	})
	.then(async response => {
		const jsonData = await response.json();
		window.lam.treeview.checkSession(jsonData);
		document.getElementById('ldap_actionarea').innerHTML = jsonData.content;
		const tree = mar10.Wunderbaum.getTree("#ldap_tree");
		const node = tree.findKey(parentDn);
		if (node) {
			await node.setExpanded(true);
			node.lazy = true;
			node.loadLazy(true);
		}
		window.scrollTo(0, 0);
	});
}

/**
 * Deletes a node in tree view.
 *
 * @param tokenName security token name
 * @param tokenValue security token value
 * @param dn base64 encoded DN
 * @param text text for dialog body
 * @param okText text for OK button
 * @param cancelText text for cancel button
 * @param title dialog title
 * @param errorOkText text for OK button in error dialog
 * @param errorTitle dialog title in case of error
 */
window.lam.treeview.deleteNode = function (tokenName, tokenValue, dn, text, okText, cancelText, title, errorOkText, errorTitle) {
	const textSpan = document.querySelector('.treeview-delete-entry');
	textSpan.innerText = window.atob(text);
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
			let data = new FormData();
			data.append(tokenName, tokenValue);
			data.append('dn', dn)
			fetch("../misc/ajax.php?function=treeview&command=deleteNode", {
				method: 'POST',
				body: data
			})
			.then(async response => {
				const jsonData = await response.json();
				window.lam.treeview.checkSession(jsonData);
				const tree = mar10.Wunderbaum.getTree("#ldap_tree");
				const parentNode = tree.findKey(dn).parent;
				await parentNode.setActive();
				parentNode.loadLazy(true);
				window.lam.treeview.getNodeContent(tokenName, tokenValue, parentNode.key);
				if (jsonData['errors']) {
					const errTextTitle = jsonData['errors'][0][1];
					const textSpanErrorTitle = document.querySelector('.treeview-error-title');
					textSpanErrorTitle.innerText = errTextTitle;
					const errText = jsonData['errors'][0][2];
					const textSpanErrorText = document.querySelector('.treeview-error-text');
					textSpanErrorText.innerText = errText;
					window.lam.dialog.showSimpleDialog(errorTitle, null, errorOkText, null, 'treeview_error_dlg');
				}
			});
		}
	});
}

/**
 * Restores a node in tree view.
 *
 * @param tokenName security token name
 * @param tokenValue security token value
 * @param dn base64 encoded DN
 * @param text text for dialog body
 * @param okText text for OK button
 * @param cancelText text for cancel button
 * @param title dialog title
 * @param errorOkText text for OK button in error dialog
 * @param errorTitle dialog title in case of error
 */
window.lam.treeview.restoreNode = function (tokenName, tokenValue, dn, text, okText, cancelText, title, errorOkText, errorTitle) {
	const textSpan = document.querySelector('.treeview-restore-entry');
	textSpan.innerText = window.atob(text);
	const dialogContent = document.getElementById('treeview_restore_dlg').cloneNode(true);
	dialogContent.classList.remove('hidden');
	const targetDn = document.getElementById('treeview-restore-dn').value;
	Swal.fire({
		title: title,
		confirmButtonText: okText,
		cancelButtonText: cancelText,
		showCancelButton: true,
		html: dialogContent.outerHTML,
		width: '48em'
	}).then(result => {
		if (result.isConfirmed) {
			let data = new FormData();
			data.append(tokenName, tokenValue);
			data.append('dn', dn)
			data.append('targetDn', targetDn)
			fetch("../misc/ajax.php?function=treeview&command=restoreNode", {
				method: 'POST',
				body: data
			})
			.then(async response => {
				const jsonData = await response.json();
				window.lam.treeview.checkSession(jsonData);
				const tree = mar10.Wunderbaum.getTree("#ldap_tree");
				const parentNode = tree.findKey(dn).parent;
				await parentNode.setActive();
				parentNode.loadLazy(true);
				window.lam.treeview.getNodeContent(tokenName, tokenValue, parentNode.key);
				if (jsonData['errorTitle']) {
					const errTextTitle = jsonData['errorTitle'];
					const textSpanErrorTitle = document.querySelector('.treeview-error-title');
					textSpanErrorTitle.innerText = errTextTitle;
					const errText = jsonData['errorText'];
					const textSpanErrorText = document.querySelector('.treeview-error-text');
					textSpanErrorText.innerText = errText;
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
	let data = new FormData();
	data.append(tokenName, tokenValue);
	data.append('dn', dn);
	data.append('highlight', (attributesToHighlight) ? attributesToHighlight : '');
	fetch("../misc/ajax.php?function=treeview&command=getNodeContent", {
		method: 'POST',
		body: data
	})
	.then(async response => {
		const jsonData = await response.json();
		window.lam.treeview.checkSession(jsonData);
		document.getElementById('ldap_actionarea').innerHTML = jsonData.content;
		if (messages) {
			document.getElementById('ldap_actionarea_messages').innerHTML = messages;
		}
		window.scrollTo(0, 0);
		window.lam.html.activateLightboxes();
		window.lam.treeview.addFileInputListeners();
		window.lam.treeview.activateSortableInputs();
		window.lam.treeview.setActionButtonsVisibility(dn);
	});
}

/**
 * Adds a listener to each file input to write the file content to a data attribute.
 */
window.lam.treeview.addFileInputListeners = function () {
	document.querySelectorAll('.image-upload').forEach(input => {
		input.addEventListener('change', function () {
			const files = input.files;
			if (!files[0]) {
				return;
			}
			const reader = new FileReader();
			reader.onload = function () {
				const content = btoa(reader.result);
				input.dataset.dataBinary = content;
			};
			reader.readAsBinaryString(files[0]);
		});
	});
}

/**
 * Activates the sortable feature for lists.
 */
window.lam.treeview.activateSortableInputs = function () {
	document.querySelectorAll('.tree-attribute-sorted-list').forEach(list => {
		const listId = list.id;
		Sortable.create(
			document.getElementById(listId),
			{
				onUpdate: function() {
					window.lam.treeview.updateAttributePositionData(listId);
				}
			}
		);
	});
}

/**
 * Sets the visibility of the action buttons.
 *
 * @param dn current DN (base64 encoded)
 */
window.lam.treeview.setActionButtonsVisibility = function (dn) {
	const pasteButton = document.getElementById('action_button_paste');
	if (pasteButton) {
		const copyPasteDn = window.sessionStorage.getItem('LAM_COPY_PASTE_DN');
		if (copyPasteDn === null) {
			pasteButton.classList.add('hidden');
		}
		else {
			pasteButton.classList.remove('hidden');
		}
	}
	const addToCompareButton = document.getElementById('action_button_compare_add');
	if (addToCompareButton) {
		const compareDNs = window.lam.treeview.getComparisonDNs();
		if (compareDNs.includes(dn)) {
			addToCompareButton.classList.add('hidden');
		}
		else {
			addToCompareButton.classList.remove('hidden');
		}
	}
	const removeFromCompareButton = document.getElementById('action_button_compare_remove');
	if (removeFromCompareButton) {
		const compareDNs = window.lam.treeview.getComparisonDNs();
		if (compareDNs.includes(dn)) {
			removeFromCompareButton.classList.remove('hidden');
		}
		else {
			removeFromCompareButton.classList.add('hidden');
		}
	}
	const startCompareButton = document.getElementById('action_button_compare');
	if (startCompareButton) {
		const compareDNs = window.lam.treeview.getComparisonDNs();
		if (compareDNs.length > 1) {
			startCompareButton.classList.remove('hidden');
		}
		else {
			startCompareButton.classList.add('hidden');
		}
	}
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
	let data = new FormData();
	data.append(tokenName, tokenValue);
	data.append('dn', dn);
	const attributeChanges = window.lam.treeview.findAttributeChanges();
	const attributesToHighlight = Object.keys(attributeChanges).join(',');
	data.append('changes', JSON.stringify(attributeChanges));
	fetch("../misc/ajax.php?function=treeview&command=saveAttributes", {
		method: 'POST',
		body: data
	})
	.then(async response => {
		const jsonData = await response.json();
		window.lam.treeview.checkSession(jsonData);
		if (jsonData.newDn) {
			const tree = mar10.Wunderbaum.getTree("#ldap_tree");
			tree.findKey(jsonData['parent']).loadLazy(true);
			window.lam.treeview.getNodeContent(tokenName, tokenValue, jsonData.newDn, jsonData.result, attributesToHighlight);
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
	let attributeChanges = {};
	document.querySelectorAll('.single-input').forEach(input => {
			if (window.lam.utility.isHidden(input)) {
				return;
			}
			const attrName = input.dataset.attrName;
			// avoid type conversion in .data()
			const valueOrig = input.dataset.valueOrig;
			const valueNew = input.value;
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
	let lastAttrName = '';
	let lastAttrValuesNew = [];
	let lastAttrValuesOld = [];
	let lastAttrHasChange = false;
	document.querySelectorAll('.multi-input').forEach(input => {
			if (window.lam.utility.isHidden(input)) {
				return;
			}
			const attrName = input.dataset.attrName;
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
			const valueOrig = input.dataset.valueOrig;
			const valueNew = input.value;
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
	document.querySelectorAll('.hash-select').forEach(input => {
			const attrName = input.dataset.attrName;
			if (!attributeChanges[attrName]) {
				return;
			}
			if (!attributeChanges[attrName]['hash']) {
				attributeChanges[attrName]['hash'] = [input.value];
			}
			else {
				attributeChanges[attrName]['hash'].push(input.value);
			}
		}
	);
	document.querySelectorAll('.image-input').forEach(input => {
			const toDelete = input.dataset.delete;
			if (toDelete !== 'true') {
				return;
			}
			const attrName = input.dataset.attrName;
			const attrIndex = input.dataset.index;
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
	document.querySelectorAll('.image-upload').forEach(input => {
			const content = input.dataset.dataBinary;
			if (!content) {
				return;
			}
			const attrName = input.dataset.attrName;
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
	const parentTr = link.closest('tr');
	const input = parentTr.querySelector('input, textarea');
	if (input !== null) {
		input.value = '';
	}
	const image = parentTr.querySelector('.image-input');
	if (image) {
		parentTr.classList.add('hidden');
		image.dataset.delete = 'true';
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
	const parentTr = link.closest('tr');
	const newTr = parentTr.cloneNode(true);
	const newField = newTr.querySelector('input, textarea');
	newField.value = '';
	newField.dataset.valueOrig = '';
	parentTr.after(newTr);
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
	let data = new FormData();
	data.append('dn', 'none');
	data.append(tokenName, tokenValue);
	data.append('objectClasses', JSON.stringify(objectCLasses));
	fetch("../misc/ajax.php?function=treeview&command=getPossibleNewAttributes", {
		method: 'POST',
		body: data
	})
	.then(async response => {
		const jsonData = await response.json();
		window.lam.treeview.checkSession(jsonData);
		const select = document.getElementById('newAttribute');
		select.innerHTML = '';
		const existingAttributeFieldNames = [];
		document.querySelectorAll('.attribute-field').forEach(input => {
			if (input.dataset.attrName) {
				existingAttributeFieldNames.push(input.dataset.attrName);
			}
		});
		const data = jsonData['data'];
		for (const attributeName in data) {
			if (existingAttributeFieldNames.includes(attributeName)) {
				continue;
			}
			const option = document.createElement('option');
			option.value = data[attributeName];
			option.innerText = attributeName;
			select.appendChild(option);
		}
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
	let attributeParts = select.value;
	if (attributeParts == '') {
		return;
	}
	select.options.remove(select.options.selectedIndex);
	attributeParts = attributeParts.split('__#__');
	const attributeName = attributeParts[0];
	const isSingleValue = attributeParts[1];
	const fieldType = attributeParts[2];
	const placeHolderId = 'new-attributes-' + isSingleValue + '-' + fieldType;
	let newContent = document.getElementById(placeHolderId).querySelector('.row').cloneNode(true);
	newContent.children[0].innerText = attributeName;
	const inputField = newContent.querySelector('input, textarea');
	inputField.dataset.attrName = attributeName;
	inputField.name = 'lam_attr_' + attributeName;
	inputField.id = 'lam_attr_' + attributeName;
	const hashSelect = newContent.querySelector('.hash-select');
	if (hashSelect) {
		hashSelect.id = 'lam_hash_' + attributeName;
		hashSelect.name = 'lam_hash_' + attributeName;
		hashSelect.dataset.attrName = attributeName;
	}
	newContent = [...newContent.children];
	while (newContent.length > 0) {
		select.closest('div').after(newContent.pop());
	}
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
	let data = new FormData();
	data.append(tokenName, tokenValue);
	data.append('dn', dn);
	fetch("../misc/ajax.php?function=treeview&command=getInternalAttributesContent", {
		method: 'POST',
		body: data
	})
	.then(async response => {
		const jsonData = await response.json();
		window.lam.treeview.checkSession(jsonData);
		document.getElementById('actionarea-internal-attributes').innerHTML = jsonData.content;
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
	let data = new FormData();
	data.append(tokenName, tokenValue);
	data.append('dn', dn);
	fetch("../misc/ajax.php?function=treeview&command=search", {
		method: 'POST',
		body: data
	})
	.then(async response => {
		const jsonData = await response.json();
		window.lam.treeview.checkSession(jsonData);
		document.getElementById('ldap_actionarea').innerHTML = jsonData.content;
		window.scrollTo(0, 0);
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
	let data = new FormData();
	data.append(tokenName, tokenValue);
	data.append('dn', dn);
	data.append('scope', document.getElementById('scope').value);
	data.append('filter', document.getElementById('filter').value);
	data.append('attributes', document.getElementById('attributes').value);
	data.append('orderBy', document.getElementById('orderBy').value);
	data.append('limit', document.getElementById('limit').value);
	data.append('format', document.getElementById('format').value);
	fetch("../misc/ajax.php?function=treeview&command=searchResults", {
		method: 'POST',
		body: data
	})
	.then(async response => {
		const jsonData = await response.json();
		window.lam.treeview.checkSession(jsonData);
		document.getElementById('ldap_actionarea').innerHTML = jsonData.content;
		window.scrollTo(0, 0);
	});
}

/**
 * Opens the given node IDs.
 *
 * @param tree tree object
 * @param ids array of node IDs.
 */
window.lam.treeview.openInitial = async function (tree, ids) {
	if (ids.length === 0) {
		return;
	}
	let lastNode = null;
	for (const id of ids) {
		const node = tree.findKey(id);
		if (node) {
			await node.setExpanded();
			lastNode = node;
		}
	}
	if (lastNode) {
		await lastNode.setActive();
	}
}

/**
 * Copies a node in the tree.
 *
 * @param dn base64 encoded DN
 */
window.lam.treeview.copyNode = function(dn) {
	if (!window.sessionStorage) {
		return;
	}
	const tree = mar10.Wunderbaum.getTree("#ldap_tree");
	const node = tree.findKey(dn);
	window.sessionStorage.setItem('LAM_COPY_PASTE_ACTION', 'COPY');
	const oldNodeDn = window.sessionStorage.getItem('LAM_COPY_PASTE_DN');
	if (oldNodeDn) {
		const oldNode = tree.findKey(oldNodeDn);
		if (oldNode) {
			oldNode.data.badge = null;
			oldNode.update();
		}
	}
	window.sessionStorage.setItem('LAM_COPY_PASTE_DN', node.key);
	node.data.badge = '../../graphics/copy.svg';
	node.update();
	window.lam.treeview.setActionButtonsVisibility(dn);
}

/**
 * Cuts a node in the tree.
 *
 * @param dn base64 encoded DN
 */
window.lam.treeview.cutNode = function(dn) {
	if (!window.sessionStorage) {
		return;
	}
	const tree = mar10.Wunderbaum.getTree("#ldap_tree");
	const node = tree.findKey(dn);
	window.sessionStorage.setItem('LAM_COPY_PASTE_ACTION', 'CUT');
	const oldNodeDn = window.sessionStorage.getItem('LAM_COPY_PASTE_DN');
	if (oldNodeDn) {
		const oldNode = tree.findKey(oldNodeDn);
		if (oldNode) {
			oldNode.data.badge = null;
			oldNode.update();
		}
	}
	window.sessionStorage.setItem('LAM_COPY_PASTE_DN', node.key);
	node.data.badge = '../../graphics/cut.svg';
	node.update();
	window.lam.treeview.setActionButtonsVisibility(dn);
}

/**
 * Pastes a copied/cut node.
 *
 * @param tokenName security token name
 * @param tokenValue security token value
 * @param destinationDn base64 encoded DN
 */
window.lam.treeview.pasteNode = function (tokenName, tokenValue, destinationDn) {
	const dn = window.sessionStorage.getItem('LAM_COPY_PASTE_DN');
	if (!dn) {
		return;
	}
	const tree = mar10.Wunderbaum.getTree("#ldap_tree");
	const action = window.sessionStorage.getItem('LAM_COPY_PASTE_ACTION');
	let data = new FormData();
	data.append(tokenName, tokenValue);
	data.append('dn', dn);
	data.append('targetDn', destinationDn);
	data.append('action', action);
	fetch('../misc/ajax.php?function=treeview&command=paste', {
		method: 'POST',
		body: data
	})
	.then(async response => {
		const jsonData = await response.json();
		window.lam.treeview.checkSession(jsonData);
		if (jsonData.error) {
			document.getElementById('ldap_actionarea_messages').innerHTML = jsonData.error;
			return;
		}
		window.sessionStorage.removeItem('LAM_COPY_PASTE_ACTION');
		window.sessionStorage.removeItem('LAM_COPY_PASTE_DN');
		const oldNode = tree.findKey(dn);
		if (action == 'CUT') {
			const parentNode = oldNode.parent;
			parentNode.loadLazy(true);
		}
		else {
			oldNode.data.badge = null;
			oldNode.update();
		}
		const newParentNode = tree.findKey(destinationDn);
		newParentNode.lazy = true;
		newParentNode.expanded = true;
		newParentNode.loadLazy(true);
		window.lam.treeview.setActionButtonsVisibility(dn);
	});
}

/**
 * Adds a DN to the comparison.
 *
 * @param dn DN (base64 encoded)
 */
window.lam.treeview.addToComparison = function(dn) {
	let comparisonDNs = window.lam.treeview.getComparisonDNs();
	if (comparisonDNs.includes(dn)) {
		return;
	}
	comparisonDNs.push(dn);
	window.lam.treeview.setComparisonDNs(comparisonDNs);
	window.lam.treeview.setActionButtonsVisibility(dn);
}

/**
 * Removes a DN from the comparison.
 *
 * @param dn DN (base64 encoded)
 */
window.lam.treeview.removeFromComparison = function(dn) {
	let comparisonDNs = window.lam.treeview.getComparisonDNs();
	if (!comparisonDNs.includes(dn)) {
		return;
	}
	comparisonDNs.splice(comparisonDNs.indexOf(dn), 1);
	window.lam.treeview.setComparisonDNs(comparisonDNs);
	window.lam.treeview.setActionButtonsVisibility(dn);
}

/**
 * Compares the selected DNs.
 *
 * @param tokenName security token name
 * @param tokenValue security token value
 * @param dn DN
 */
window.lam.treeview.openComparison = function(tokenName, tokenValue, dn) {
	const comparisonDNs = window.lam.treeview.getComparisonDNs();
	if (comparisonDNs.length < 2) {
		return;
	}
	let data = new FormData();
	data.append(tokenName, tokenValue);
	data.append('dnList', comparisonDNs.join(','));
	data.append('dn', dn);
	fetch("../misc/ajax.php?function=treeview&command=compare", {
		method: 'POST',
		body: data
	})
	.then(async response => {
		const jsonData = await response.json();
		window.lam.treeview.checkSession(jsonData);
		document.getElementById('ldap_actionarea').innerHTML = jsonData.content;
		window.scrollTo(0, 0);
	});
}

/**
 * Returns the comparison DNs.
 *
 * @return the comparison DNs
 */
window.lam.treeview.getComparisonDNs = function() {
	const comparisonDNs = window.sessionStorage.getItem('LAM_COMPARE_DNS');
	return (comparisonDNs) ? comparisonDNs.split(',') : [];
}

/**
 * Sets the comparison DNs.
 *
 * @param dnList the comparison DNs
 */
window.lam.treeview.setComparisonDNs = function(dnList) {
	window.sessionStorage.setItem('LAM_COMPARE_DNS', dnList.join(','));
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
		let inputField = childLiElements[i].querySelector('input');
		if (inputField === null) {
			inputField = childLiElements[i].querySelector('textarea');
		}
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
	const selectedTabInput = document.querySelector('#selectedTab');
	if (selectedTabInput) {
		selectedTabInput.value = tabId;
	}
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

/**
 * Creates the markup for a progress bar.
 *
 * @param htmlId HTML id
 */
window.lam.progressbar.getMarkup = function(htmlId) {
	return '<div id="' + htmlId + '" class="lam-progressbar">' +
		'<div class="lam-progressbar-bar" style="width: 0%"></div>' +
		'</div>';
}

window.lam.accordion = window.lam.accordion || {};

/**
 * Initializes the accordions.
 */
window.lam.accordion.init = function() {
	const accordionButtons = document.getElementsByClassName('lam-accordion-button');
	Array.from(accordionButtons).forEach(function (button) {
		button.addEventListener('click', function(event) {
			window.lam.accordion.onClick(event, this);
			return false;
		});
	});
	const accordions = document.getElementsByClassName('lam-accordion-container');
	Array.from(accordions).forEach(function (accordion) {
		const isSaveState = accordion.dataset.savestate && (accordion.dataset.savestate === 'true');
		let openInitial = false;
		const storageKey = 'lam_accordionStore_' + accordion.id;
		if (isSaveState && window.localStorage.getItem(storageKey)) {
			// load value from local storage
			openInitial = window.localStorage.getItem(storageKey);
		}
		else if (accordion.dataset.openinitial) {
			openInitial = accordion.dataset.openinitial;
		}
		if (openInitial !== false) {
			accordion.querySelectorAll('.lam-accordion-button').forEach(item => {
				const buttonIndex = item.dataset.index;
				if (openInitial === buttonIndex) {
					item.classList.add('lam-accordion-button-active');
				}
			});
			accordion.querySelectorAll('.lam-accordion-content').forEach(item => {
				const contentIndex = item.dataset.index;
				if (openInitial === contentIndex) {
					item.style.maxHeight = null;
					item.classList.add('lam-accordion-content-active')
					item.style.maxHeight = item.scrollHeight + 'px';
				}
			});
		}
	});
}

/**
 * Onclick handler for accordion element.
 *
 * @param event event
 * @param button button that was clicked
 */
window.lam.accordion.onClick = function(event, button) {
	event.preventDefault();
	// mark button active
	button.classList.toggle('lam-accordion-button-active');
	// open corresponding content area
	const content = button.nextElementSibling;
	if (content.style.maxHeight) {
		content.style.maxHeight = null;
		content.classList.remove('lam-accordion-content-active')
	}
	else {
		content.style.maxHeight = content.scrollHeight + 'px';
		content.classList.add('lam-accordion-content-active')
	}
	const indexActive = button.dataset.index;
	const accordion = button.parentElement;
	const isSaveState = accordion.dataset.savestate && (accordion.dataset.savestate === 'true');
	if (isSaveState) {
		const storageKey = 'lam_accordionStore_' + accordion.id;
		window.localStorage.setItem(storageKey, indexActive);
	}
	// deactivate other buttons
	accordion.querySelectorAll('.lam-accordion-button').forEach(item => {
		const buttonIndex = item.dataset.index;
		if (indexActive !== buttonIndex) {
			item.classList.remove('lam-accordion-button-active');
		}
	});
	// close other content areas
	accordion.querySelectorAll('.lam-accordion-content').forEach(item => {
		const contentIndex = item.dataset.index;
		if (indexActive !== contentIndex) {
			item.style.maxHeight = null;
			item.classList.remove('lam-accordion-content-active')
		}
	});
}

window.lam.tooltip = window.lam.tooltip || {};

/**
 * Shows the help popup.
 *
 * @param link help link
 */
window.lam.tooltip.show = function(link) {
    const tooltip =  link.nextElementSibling;
    const { computePosition, flip, shift, offset } = FloatingUIDOM;
    const options = {
        placement: 'top',
        middleware: [offset(6), flip(), shift()]
    };
    computePosition(link, tooltip, options).then(({x, y}) => {
        Object.assign(tooltip.style, {
            left: `${x}px`,
            top: `${y}px`,
        });
    });
    tooltip.classList.remove('hidden');
}

/**
 * Hides the help popup.
 *
 * @param link help link
 */
window.lam.tooltip.hide = function(link) {
    link.nextElementSibling.classList.add('hidden');
}

window.lam.smtp = window.lam.smtp || {};

/**
 * Tests the SMTP settings.
 *
 * @param event event
 * @param tokenName security token name
 * @param tokenValue security token value
 * @param okText text to close dialog
 * @param cancelText text to cancel test
 * @param title dialog title
 */
window.lam.smtp.test = function(event, tokenName, tokenValue, okText, cancelText, title) {
	event.preventDefault();
	const runTestPreCallback = function() {
		const mailFrom = document.getElementById('testSmtpFrom').value;
		const mailTo = document.getElementById('testSmtpTo').value;
		if (!mailFrom || !mailTo) {
			return false;
		}
		return {
			mailFrom: mailFrom,
			mailTo: mailTo
		}
	}
	const runTestCallback = function(formData) {
		document.getElementById('btn_testSmtp').disabled = true;
		let data = new FormData();
		data.append(tokenName, tokenValue);
		data.append('server', document.getElementById('mailServer').value);
		data.append('user', document.getElementById('mailUser').value);
		data.append('password', document.getElementById('mailPassword').value);
		data.append('encryption', document.getElementById('mailEncryption').value);
		data.append('mailFrom', formData.mailFrom);
		data.append('mailTo', formData.mailTo);
		const url = '../misc/ajax.php?function=testSmtp';
		fetch(url, {
			method: 'POST',
			body: data
		})
		.then(async response => {
			const jsonData = await response.json();
			if (jsonData.info) {
				window.lam.dialog.showInfo(jsonData.info, okText);
			}
			else if (jsonData.error) {
				window.lam.dialog.showError(jsonData.error, jsonData.details, okText);
			}
			document.getElementById('btn_testSmtp').disabled = false;
		});
	}
	window.lam.dialog.showConfirmation(title, okText, cancelText, 'smtpTestDialogDiv', runTestCallback, runTestPreCallback);
}

window.lam.sms = window.lam.sms || {};

/**
 * Tests the SMS settings.
 *
 * @param event event
 * @param tokenName security token name
 * @param tokenValue security token value
 * @param okText text to close dialog
 * @param cancelText text to cancel test
 * @param title dialog title
 */
window.lam.sms.test = function(event, tokenName, tokenValue, okText, cancelText, title) {
	event.preventDefault();
	const runTestPreCallback = function() {
		const number = document.getElementById('testSmsNumber').value;
		if (!number) {
			return false;
		}
		return {
			number: number
		}
	}
	const runTestCallback = function(formData) {
		document.getElementById('btn_testSms').disabled = true;
		let data = new FormData();
		data.append(tokenName, tokenValue);
		data.append('provider', document.getElementById('smsProvider').value);
		data.append('apiKey', document.getElementById('smsApiKey').value);
		data.append('apiToken', document.getElementById('smsApiToken').value);
		data.append('accountId', document.getElementById('smsAccountId').value);
		data.append('region', document.getElementById('smsRegion').value);
		data.append('from', document.getElementById('smsFrom').value);
		data.append('subject', document.getElementById('smsSubject').value);
		data.append('number', formData.number);
		const url = '../misc/ajax.php?function=testSms';
		fetch(url, {
			method: 'POST',
			body: data
		})
		.then(async response => {
			const jsonData = await response.json();
			if (jsonData.info) {
				window.lam.dialog.showInfo(jsonData.info, okText);
			}
			else if (jsonData.error) {
				window.lam.dialog.showError(jsonData.error, jsonData.details, okText);
			}
			document.getElementById('btn_testSms').disabled = false;
		});
	}
	window.lam.dialog.showConfirmation(title, okText, cancelText, 'smsTestDialogDiv', runTestCallback, runTestPreCallback);
}

window.lam.config = window.lam.config || {};

window.lam.config.updateModuleFilter = function(inputField) {
	const filterValue = inputField.value.toLowerCase();
	const table = inputField.parentElement.parentElement.querySelector('table');
	table.querySelectorAll('tr').forEach(row => {
		let matches = false;
		for (let i = 0; i < row.children.length; i++) {
			const cell = row.children[i];
			if (cell.children.length > 0) {
				continue;
			}
			if (cell.innerText.toLowerCase().includes(filterValue)) {
				matches = true;
			}
		}
		if (matches) {
			row.classList.remove('hidden');
		}
		else {
			row.classList.add('hidden');
		}
	});
}

window.lam.richEdit = window.lam.richEdit || {};

/**
 * Inits the rich-text editors.
 */
window.lam.richEdit.init = function() {
	document.querySelectorAll('.lam-rich-edit').forEach(textarea => {
		Jodit.make(textarea, {
			beautifyHTML: false,
			sourceEditorCDNUrlsJS: [],
			sourceEditor: 'area',
			uploader: {
				"insertImageAsBase64URI": true
			}
		});
	});
}

window.lam.datatable = window.lam.datatable || {};
window.lam.datatable.tables = window.lam.datatable.tables || {};
window.lam.datatable.unfinishedTables = window.lam.datatable.unfinishedTables || {};

window.lam.datatable.init = function(id, table) {
	window.lam.datatable.tables[id] = table;
	window.lam.datatable.unfinishedTables[id] = true;
	table.on("tableBuilt", () => {
		window.lam.datatable.unfinishedTables[id] = false;
	});
}

/**
 * Refreshes the data of a datatable using the AjaxURL.
 *
 * @param id table ID
 */
window.lam.datatable.refreshTableData = function(id) {
	const tableDiv = document.getElementById(id);
	if (!tableDiv) {
		return;
	}
	const ajaxUrl = tableDiv.dataset.ajaxurl;
	const tokenName = tableDiv.dataset.tokenname;
	const tokenValue = tableDiv.dataset.tokenvalue;
	const action = tableDiv.dataset.action;
	const okText = tableDiv.dataset.oktext;
	let data = new FormData();
	data.append(tokenName, tokenValue);
	data.append("action", action);
	fetch(ajaxUrl, {
		method: "POST",
		body: data
	})
	.then(async response => {
		const jsonData = await response.json();
		if (jsonData.message) {
			window.lam.dialog.showError(jsonData.message, okText);
		}
		else {
			window.lam.datatable.setData(id, jsonData);
		}
	});
}

/**
 * Sets the data of a datatable.
 *
 * @param id table ID
 * @param data list of rows ([{firstName:"Steve", lastName:"Miller"}])
 */
window.lam.datatable.setData = function(id, data) {
	const table = window.lam.datatable.tables[id];
	for (let i = 0; i < data.length; i++) {
		data[i].id = i;
	}
	if (window.lam.datatable.unfinishedTables[id] === false) {
		table.replaceData(data);
	}
	else {
		table.on("tableBuilt", () => {
			window.lam.datatable.unfinishedTables[id] = false;
			table.replaceData(data);
		});
	}
}

/**
 * Adds a search field to the data table.
 *
 * @param tableId table ID
 * @param fieldId field ID
 */
window.lam.datatable.addSearchField = function(tableId, fieldId) {
    const table = window.lam.datatable.tables[tableId];
    const field = document.getElementById(fieldId);
    field.addEventListener('keyup', function() {
        const filter = field.value.toLowerCase();
        table.clearFilter();
        if (filter === '') {
            return;
        }
        table.setFilter(function(data, filterValue) {
            const filterValues = filterValue.split(/\s+/);
            let found = true;
            filterValues.forEach(value => {
                found = found && data['_lam_search_'].toLowerCase().includes(value);
            })
            return found;
        }, filter);
    });
}

window.lam.loadingIndicator = window.lam.loadingIndicator || {};

/**
 * Starts the loading indicator.
 */
window.lam.loadingIndicator.start = function() {
	Swal.fire({
		title:"",
		imageUrl: "../../style/loading.gif",
		showConfirmButton: false,
		allowOutsideClick: false,
		timer: 60000,
	});
}

/**
 * Stops the loading indicator.
 */
window.lam.loadingIndicator.stop = function() {
	Swal.close();
}


window.lam.showMore = window.lam.showMore || {};

/**
 * Shows items hidden initially.
 *
 * @param element link element
 */
window.lam.showMore.showMore = function (element) {
    const root = element.closest('.lam-showmore-root');
    if (root) {
        root.querySelectorAll('.lam-showmore-more').forEach(showMoreElement => {
            showMoreElement.classList.remove('hidden');
        });
        root.querySelectorAll('.lam-showmore-less').forEach(showMoreElement => {
            showMoreElement.classList.add('hidden');
        });
    }
}

/**
 * Hides items shown by show more.
 *
 * @param element link element
 */
window.lam.showMore.showLess = function (element) {
    const root = element.closest('.lam-showmore-root');
    if (root) {
        root.querySelectorAll('.lam-showmore-more').forEach(showMoreElement => {
            showMoreElement.classList.add('hidden');
        });
        root.querySelectorAll('.lam-showmore-less').forEach(showMoreElement => {
            showMoreElement.classList.remove('hidden');
        });
    }
}


window.lam.utility = window.lam.utility || {};

/**
 * Run actions on document ready.
 *
 * @param callback callback function
 */
window.lam.utility.documentReady = function(callback) {
	if (document.readyState !== 'loading') {
		callback();
	} else {
		document.addEventListener('DOMContentLoaded', callback);
	}
}

/**
 * Stores the current scroll position in the form.
 *
 * @param formName ID of form
 */
window.lam.utility.saveScrollPosition = function(formName) {
	const top = window.scrollY;
	const left = window.scrollX;
	let scrollPositionTop = document.createElement('input');
	scrollPositionTop.name = 'scrollPositionTop';
	scrollPositionTop.value = top;
	scrollPositionTop.hidden = 'hidden';
	document.forms[formName].appendChild(scrollPositionTop);
	let scrollPositionLeft = document.createElement('input');
	scrollPositionLeft.name = 'scrollPositionLeft';
	scrollPositionLeft.value = left;
	scrollPositionLeft.hidden = 'hidden';
	document.forms[formName].appendChild(scrollPositionLeft);
}

/**
 * Restores the scroll position.
 *
 * @param topValue top value
 * @param leftValue left value
 */
window.lam.utility.restoreScrollPosition = function(topValue, leftValue) {
	window.lam.utility.documentReady(function() {
		window.scrollTo(leftValue, topValue);
	});
}

/**
 * Checks if an element is hidden.
 *
 * @param element element
 */
window.lam.utility.isHidden = function(element) {
	return !element.offsetWidth || !element.offsetHeight || (element.getClientRects().length == 0);
}


window.lam.utility.documentReady(function() {
	window.lam.form.autoTrim();
	window.lam.account.addDefaultProfileListener();
	window.lam.tools.addSavedSelectListener();
	window.lam.tools.setInitialFocus();
	window.lam.tools.webcam.init();
	window.lam.tools.schema.select();
	window.lam.html.activateLightboxes();
	window.lam.html.preventEnter();
	window.lam.filterSelect.addFormListener();
	window.lam.dynamicSelect.activate();
	window.lam.webauthn.setupDeviceManagement();
	window.lam.tabs.init();
	window.lam.accordion.init();
	window.lam.richEdit.init();
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
