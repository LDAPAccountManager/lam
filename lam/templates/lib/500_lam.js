/**

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2023  Roland Gruber

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
	else if (field.value == '') {
		field.classList.remove('markFail');
		field.classList.remove('markOk');
	}
	else {
		field.classList.add('markFail');
		field.classList.remove('markOk');
		field.title = data.result;
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
	const search = inputField.value.trim();
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
			const selectFields = form.querySelectorAll('.lam-filteredOptions');
			selectFields.forEach((item) => {
				const storedOptions = JSON.parse(item.dataset.options);
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
	const currentOptions = JSON.parse(selectField.dataset.dynamicOptions);
	// save selection
	const optionTags = Array.from(selectField.children);
	optionTags.forEach((element) => {
		const index = currentOptions[parseInt(element.dataset.index)].index;
		optionsOrig[index].selected = element.selected;
	});
	selectField.dataset.dynamicOptionsOrig = JSON.stringify(optionsOrig);
	selectField.innerHTML = '';
	const newOptions = [];
	// get matching values
	const search = inputField.value.trim();
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
		const children = Array.from(select.children);
		children.forEach((option) => {
			dynamicOptions[option.dataset.index].selected = option.selected;
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
		selectField.append(window.lam.dynamicSelect.createOption(options[i], i));
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
	const selectBoxHeight = parseInt(selectField.dataset.selectHeight);
	const singleOptionHeight = parseInt(selectField.dataset.optionHeight);
	const currentScrollPosition = parseInt(selectField.dataset.selectCurrentScrollTop) + selectBoxHeight;
	const options = JSON.parse(selectField.dataset.dynamicOptions);
	const lastIndex = parseInt(selectField.children[selectField.children.length - 1].dataset.index);
	for (let toAdd = 0; toAdd < numberOfOptionsToLoad; toAdd++) {
		const addPos = lastIndex + 1 + toAdd;
		if (options[addPos] === undefined) {
			break;
		}
		selectField.append(window.lam.dynamicSelect.createOption(options[addPos], addPos));
	}
	const numberOfOptions = selectField.children.length;
	let toRemove = numberOfOptions - maxOptions;
	if (toRemove > 0) {
		for (let i = toRemove; i >= 0; i--) {
			const optionToRemove = selectField.children[i];
			const indexToRemove = parseInt(optionToRemove.dataset.index);
			options[indexToRemove].selected = optionToRemove.selected;
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
	const singleOptionHeight = parseInt(selectField.dataset.optionHeight);
	const currentScrollPosition = parseInt(selectField.dataset.selectCurrentScrollTop);
	const options = JSON.parse(selectField.dataset.dynamicOptions);
	const lastIndex = parseInt(selectField.children[0].dataset.index);
	let added = 0;
	for (let toAdd = 0; toAdd < numberOfOptionsToLoad; toAdd++) {
		const addPos = lastIndex - 1 - toAdd;
		if (options[addPos] === undefined) {
			break;
		}
		added++;
		selectField.prepend(window.lam.dynamicSelect.createOption(options[addPos], addPos));
	}
	const numberOfOptions = selectField.children.length;
	const toRemove = numberOfOptions - maxOptions;
	if (toRemove > 0) {
		for (let i = maxOptions; i < selectField.children.length; i++) {
			const optionToRemove = selectField.children[i];
			const indexToRemove = parseInt(optionToRemove.dataset.index);
			options[indexToRemove].selected = optionToRemove.selected;
			optionToRemove.remove();
		}
	}
	selectField.scrollTop = currentScrollPosition + (added * singleOptionHeight);
	selectField.dataset.dynamicOptions = JSON.stringify(options);
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
	htmlOut += window.lam.progressbar.getMarkup('progressbarGeneral');
	jQuery('#uploadContent').html(htmlOut);
	window.lam.progressbar.setProgress('progressbarGeneral', jsonData.accountsProgress);
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
	htmlOut += window.lam.progressbar.getMarkup('progressbarGeneral');
	if (jsonData.postActionsTitle) {
		htmlOut += '<h2>' + jsonData.postActionsTitle + '</h2>';
		htmlOut += window.lam.progressbar.getMarkup('progressbarPostActions');
	}
	jQuery('#uploadContent').html(htmlOut);
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
	var htmlOut = '<div class="title">';
	htmlOut += '<h2 class="titleText">' + jsonData.title + '</h2>';
	htmlOut += '</div>';
	htmlOut += window.lam.progressbar.getMarkup('progressbarGeneral');
	htmlOut += '<h2>' + jsonData.titlePDF + '</h2>';
	htmlOut += window.lam.progressbar.getMarkup('progressbarPDF');
	jQuery('#uploadContent').html(htmlOut);
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
			const link = $(this);
			const typeId = link.data('typeid');
			const name = link.data('name');
			const okText = link.data('ok');
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
				};
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
			}
			return false;
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
	data.append('baseDn', document.getElementById('baseDn').value)
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
	document.querySelectorAll('.lam-prevent-enter').forEach(item => {
		item.addEventListener('keypress', function (event) {
			if (event.keyCode === 10 || event.keyCode === 13) {
				event.preventDefault();
			}
		});
	});
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
	const accordionContentAreas = document.getElementsByClassName('lam-accordion-content-active');
	Array.from(accordionContentAreas).forEach(function (contentArea) {
		contentArea.style.maxHeight = contentArea.scrollHeight + 'px';
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
	const parent = button.parentElement;
	// deactivate other buttons
	parent.querySelectorAll('.lam-accordion-button').forEach(item => {
		const buttonIndex = item.dataset.index;
		if (indexActive !== buttonIndex) {
			item.classList.remove('lam-accordion-button-active');
		}
	});
	// close other content areas
	parent.querySelectorAll('.lam-accordion-content').forEach(item => {
		const contentIndex = item.dataset.index;
		if (indexActive !== contentIndex) {
			item.style.maxHeight = null;
			item.classList.remove('lam-accordion-content-active')
		}
	});
}

window.lam.tooltip = window.lam.tooltip || {};

/**
 * Creates the tooltips for help buttons.
 */
window.lam.tooltip.init = function() {
	document.querySelectorAll('[helpdata]').forEach(item => {
		let helpString = "<div class='lam-tooltip'><h4 class=\"lam-tooltip-title\">";
		helpString += item.attributes.helptitle.value;
		helpString += "</h4><div class=\"lam-tooltip-content\">";
		helpString += item.attributes.helpdata.value;
		helpString += "</div></h4></div>";
		tippy(item, {
			content: helpString,
			allowHTML: true,
			arrow: false,
			delay: [100, 20]
		});
	});
}


jQuery(document).ready(function() {
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
	window.lam.tooltip.init();
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
