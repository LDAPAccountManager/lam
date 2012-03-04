<?php

/*
  $Id$
  
  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2009 - 2012  Pavel Pozdniak
                2009 - 2012  Roland Gruber

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
 * Manages Asterisk extensions.
 *
 * @package modules
 *
 * @author Pavel Pozdniak
 * @author Roland Gruber
 */

/**
 * Manages Asterisk extensions.
 *
 * @package modules
 */
class asteriskExtensionNewUI extends baseModule {

    /** if true then only Asterisk accounts are shown as possible owners */
    private $filterOwnerForAsteriskAccounts = true;

    /** if true then the tree suffix will be used to search potential owners */
    private $searchOwnersInTreeSuffix = false;
    /*     * Containe all attributes information for set of accounts */
    private $extensionRows = array();
    /*     * Containe all original attributes information for set of accounts */
    private $extensionRowsOrig = array();
    /*     * Extension owners */
    private $extensionOwners = array();
    private $addRuleFlag = false;

    /**
     * Returns meta data that is interpreted by parent class
     *
     * @return array array with meta data
     */
    function get_metaData() {
        $return = array();
        // manages users accounts
        $return["account_types"] = array("asteriskExtNewUI");
        $return["is_base"] = true;
        // RDN attribute
        $return["RDN"] = array("cn" => "normal");
        // LDAP filter
        $return["ldap_filter"] = array('or' => "(objectClass=AsteriskExtension)");
        // alias name
        $return["alias"] = _("Asterisk extension New UI");
        // module dependencies
        $return['dependencies'] = array('depends' => array(), 'conflicts' => array());
        // managed object classes
        $return['objectClasses'] = array('AsteriskExtension', 'groupOfNames');
        // managed attributes
        $return['attributes'] = array('cn', 'member', 'AstExtension', 'AstPriority',
            'AstApplication', 'AstApplicationData', 'AstContext');
        //ldap aliases
        $return['LDAPaliases'] = array('commonName' => 'cn');
        // icon
        $return['icon'] = 'asterisk.png';
        // help
        $return['help'] = array(
            'AstApplicationData' => array(
                "Headline" => _("Application data"), 'attr' => 'AstApplicationData',
                "Text" => _("This defines what application Asterisk has to start for this user (e.g. SIP/user1).")
            ),
            'AstContext' => array(
                "Headline" => _("Account context"), 'attr' => 'AstContext',
                "Text" => _("The account context stores information about the dial plan.")
            ),
            'AstExtension' => array(
                "Headline" => _("Extension name"), 'attr' => 'AstExtension',
                "Text" => _("The name of the extension (e.g. voicemail or sip).")
            ),
            'AstPriority' => array(
                "Headline" => _("Priority"), 'attr' => 'AstPriority',
                "Text" => _("The priority is a sequence number used to order the execution of commands. Commands are executed beginning with the lowest sequence number.")
            ),
            'AstApplication' => array(
                "Headline" => _("Application"), 'attr' => 'AstApplication',
                "Text" => _("The name of a command to execute. The available commands may be listed at the Asterisk command line by executing \"core show applications\".")
            ),
            'member' => array(
                "Headline" => _("Extension owners"), 'attr' => 'member',
                "Text" => _("DNs of the extension owners")
            ),
            'cn' => array(
                "Headline" => _("Common name"), 'attr' => 'cn',
                "Text" => _("Common name of the Asterisk extension.")
            ),
            'ownerOptions' => array(
                "Headline" => _("Options"),
                "Text" => _("These options change the list of potential extension owners below. You can select to show Asterisk accounts or all users. It is also possible to search the tree suffix if you have users which are not in the standard user suffix.")
            ),
            'MigrateSuffix' => array(
                "Headline" => _("Suffix to migrate"),
                "Text" => _("This is suffix of LDAP tree to migrate entryes after testing or other staff.")
            ),
            'UseCreatorAsOwner' => array(
                "Headline" => _("Use an extension creator as its owner"),
                "Text" => _("This feature hide owner's selection button from an extension managment interface and set a logged in user as an extension owner. You can use this feature as a leverage to grand/reject users to manage thair extensions")
            ),
        );

        // profile options
        $profileContainer = new htmlTable();
        $profileContainer->addElement(new htmlTableExtendedInputField(_('Account context'), 'AsteriskExtension_AstContext', null, 'AstContext'), true);
        $return['profile_options'] = $profileContainer;
        $return['profile_mappings'] = array(
            'AsteriskExtension_AstContext' => 'AstContext',
        );
        // available PDF fields
        $return['PDF_fields'] = array(
            'AstContext' => _('Account context'),
            'AstExtension' => _('Extension name'),
            'owners' => _('Extension owners'),
            'rules' => _('Rules'),
        );
        
        $return['upload_columns'] = array(
			array(
				'name' => 'asteriskExtensionNewUI_AstExtension',
				'description' => _('Extension name'),
				'help' => 'AstExtension',
				'example' => '500',
				'required' => true
			),			
			array(
				'name' => 'asteriskExtensionNewUI_AstContext',
				'description' => _('Account context'),
				'help' => 'AstContext',
				'example' => _('test'),
				'required' => true,
			),
                        array(
				'name' => 'asteriskExtensionNewUI_owner',
				'description' => _('Extension owners'),
				'help' => 'member',
				'example' => 'uid=user1,o=test;uid=user2,o=test',				
			),
                        array(
				'name' => 'asteriskExtensionNewUI_AstApplication',
				'description' => _('Application'),
				'help' => 'AstApplication',
				'example' => 'Playback',
				'required' => true
			),
			array(
				'name' => 'asteriskExtensionNewUI_AstApplicationData',
				'description' => _('Application data'),
				'help' => 'AstApplicationData',
				'example' => _('test-start'),
			),		
			
		);
        return $return;
    }

    /**
     * This function fills the error message array with messages
     */
    function load_Messages() {
        $this->messages['cn'][0] = array('ERROR', _('Please enter a common name.'));
        $this->messages['AstApplication'][0] = array('ERROR', _('Please enter the application.'));
        $this->messages['AstApplication'][1] = array('ERROR', _('Number of applications and application data in bulk upload is not the same.'));
        $this->messages['AstApplicationData'][0] = array('ERROR', _('Please enter the application data.'));
        $this->messages['AstContext'][0] = array('ERROR', _('Please enter the account context.'));
        $this->messages['AstExtension'][0] = array('ERROR', _('Please enter the extension name.'));
        $this->messages['AstExtension'][1] = array('ERROR', _('Extension with this name is already exists.'));
        $this->messages['AstPriority'][0] = array('ERROR', _('Please enter the priority.'));
        $this->messages['AstExtensionAstPriority'][0] = array('ERROR', _('This pair of extension name and priority already exists.'));
        $this->messages['member'][0] = array('ERROR', _('Please add at least one extension owner.'));
        
    }

    /**
     * This function will create the meta HTML code to show a page with all attributes.
     *
     * @return htmlElement HTML meta data
     */
    function display_html_attributes() {
    	$return = new htmlTable();

        $extName = '';
        if (isset($this->attributes['AstExtension'][0])) {
            $extName = $this->attributes['AstExtension'][0];
        }
        if (isset($this->extensionRows[0]['astextension'][0])) {
            $extName = $this->extensionRows[0]['astextension'][0];
        }
        //This procedure LOAD all data from LDAP and then print to a page
        $this->render_extensions_by_priority($extName, $return);

        // owners
        //Set default owner as logged in user if no other owner is set
        if ( !$this->isExtensionOwnerSet()) {
                $this->setDefaultExtensionOwner();
        }
        
        $this->render_exten_owners_set_controls($return);

        return $return;
    }

    /**
     * This function prints management elements to manipulate owners of an extension.
     * 
     * @param htmlTable container
     */
    function render_exten_owners_set_controls($renderContainer) {
        $renderContainer->addElement(new htmlOutputText(_("Extension owners") . '*'));
        $renderContainer->addElement(new htmlAccountPageButton(get_class($this), 'user', 'open', _('Change')));
        $renderContainer->addElement(new htmlHelpLink('member'));
        $renderContainer->addNewLine();
        $renderContainer->addElement(new htmlOutputText(''));
        $ownerList = new htmlTable();
        if ((!isset($this->extensionOwners) || sizeof($this->extensionOwners) == 0) && isset($this->extensionRows[0]['member'])) {
            $this->extensionOwners = $this->extensionRows[0]['member'];
        }
        if (isset($this->extensionOwners)) {
            for ($i = 0; $i < sizeof($this->extensionOwners); $i++) {
                $ownerList->addElement(new htmlOutputText(getAbstractDN($this->extensionOwners[$i])), true);
            }
        }
        $renderContainer->addElement($ownerList);
    }

    /**
     * Loads all related extension entries.
     * 
     * @param String $extension extension name
     */
    function load_extension_parts($extension) {
        if (!isset($this->extensionRows[0]['astcontext'][0])) {
            $entries = searchLDAP($this->getAccountContainer()->dnSuffix, '(&(objectClass=AsteriskExtension)(AstExtension=' . $extension . '))', array('cn', 'AstContext', 'AstPriority',
                'AstApplication', 'AstApplicationData', 'AstExtension', 'member', 'ObjectClass'));

            $entries = $this->array_sort($entries, 'astpriority');
            $this->extensionRowsOrig = $entries;
            $this->extensionRows = $entries;
            if(isset($this->extensionRows[0]['member'])){
                $this->extensionOwners = $this->extensionRows[0]['member'];
            }
        } else {
            $entries = $this->extensionRows;
        }
        return $entries;
    }

    /**
     * Generates the meta HTML for the rules.
     * 
     * @param String $extension extension name
     * @param htmlTable $renderContainer container
     */
    function render_extensions_by_priority($extension, $renderContainer) {
        if (!isset($extension) || $extension == "") {
            $extension = "";
        }

        $entries = $this->load_extension_parts($extension);

        if ($this->getAccountContainer()->isNewAccount) {
            $suggestedExtName = $this->generateNextExtensionName();
            $extNameInput = new htmlTableExtendedInputField(_("Extension name"), 'AstExtension', $suggestedExtName, 'AstExtension');
            $extNameInput->setRequired(true);
            $renderContainer->addElement($extNameInput, true);            
        } else {
            $extNameInput = new htmlTableExtendedInputField(_("Extension name"), 'AstExtension', $extension, 'AstExtension');
            $extNameInput->setRequired(true);
            $renderContainer->addElement($extNameInput, true);
        }

        $accountContext = '';
        if (isset($entries[0]['astcontext'][0])) {
            //LOAD context from already entities that  already exists
            $accountContext = $entries[0]['astcontext'][0];
        }  elseif (isset($this->attributes['AstContext'][0])) {
            //LOAD context from attributes, this is treak to deal with profile
            $accountContext = $this->attributes['AstContext'][0];
	}
        $accountContextInput = new htmlTableExtendedInputField(_("Account context"), 'AstContext', $accountContext, 'AstContext');
        $accountContextInput->setRequired(true);
        $renderContainer->addElement($accountContextInput, true);


        $renderContainer->addElement(new htmlSubTitle(_('Rules')));
        $renderContainer->addElement(new htmlSpacer(null, '20px'), true);

        for ($i = 0; $i < sizeof($entries); $i++) {
            $this->render_extension($entries[$i], $i, $renderContainer);
            $renderContainer->addElement(new htmlButton("delete_rule_" . $i, _('Delete rule')), false);

            $upDownButtons = new htmlTable();
            if ($i > 0) {
                $upDownButtons->addElement(new htmlButton('rule_up_button_' . $i, 'up.gif', true), false);
            }
            if (($i < sizeof($entries) - 1) &&
                    ((sizeof($entries) > 1) || ($this->addRuleFlag) )) {
                $upDownButtons->addElement(new htmlButton('rule_down_button_' . $i, 'down.gif', true));
            }
            $renderContainer->addElement($upDownButtons, true);
            
            $renderContainer->addElement(new htmlSpacer(null, '30px'), true);
        }

        $displayEntrNum = sizeof($entries);

        if ($this->addRuleFlag || sizeof($entries) == 0) {

            $this->render_extension(null, sizeof($entries), $renderContainer);
            
            if ($this->addRuleFlag) {
                $upDownButtons = new htmlTable();
                $renderContainer->addElement(new htmlButton("delete_rule_" . $i, _('Delete rule')), false);
                $upDownButtons->addElement(new htmlButton('rule_up_button_' . $i, 'up.gif', true), false);
                $renderContainer->addElement($upDownButtons, true);                
            }
            
            $displayEntrNum++;
            $this->addRuleFlag = false;
        }
        $hidenInput = new htmlHiddenInput("extension_rows", $displayEntrNum); //the size of found rows plus 1 for new one;
        $renderContainer->addElement($hidenInput, true);
        
        $renderContainer->addElement(new htmlSpacer(null, '30px'), true);
        $renderContainer->addElement(new htmlButton("add_rule", _('Add another rule')), true);
    }

    /**
     * Generates the meta HTML for a single rule.
     * 
     * @param array $extensLine attributes of rule
     * @param int $placeInList rule position
     * @param htmlTable $renderContainer container
     */
    function render_extension($extensLine, $placeInList, $renderContainer) {
        // application
        $application = '';
        if (isset($extensLine['astapplication'][0])) {
            $application = $extensLine['astapplication'][0];
        }
        $applicationInput = new htmlTableExtendedInputField(_("Application"), "AstApplication_" . $placeInList, $application, 'AstApplication');
        $applicationInput->setRequired(true);
        $renderContainer->addElement($applicationInput, true);
        // application data
        $applicationData = '';
        if (isset($extensLine['astapplicationdata'][0])) {
            $applicationData = $extensLine['astapplicationdata'][0];
        }
        $renderContainer->addElement(new htmlTableExtendedInputField(_("Application data"), "AstApplicationData_" . $placeInList, $applicationData, 'AstApplicationData'), true);
        //delimeter
        $renderContainer->addElement(new htmlSpacer(null, '5px'), true);
    }

    function array_sort($array, $on, $order='SORT_ASC') {
        $new_array = array();
        $sortable_array = array();

        if (count($array) > 0) {
            foreach ($array as $k => $v) {
                if (is_array($v)) {
                    foreach ($v as $k2 => $v2) {
                        if ($k2 == $on) {
                            $sortable_array[$k] = $v2;
                        }
                    }
                } else {
                    $sortable_array[$k] = $v;
                }
            }

            switch ($order) {
                case 'SORT_ASC':
                    asort($sortable_array);
                    break;
                case 'SORT_DESC':
                    arsort($sortable_array);
                    break;
            }

            foreach ($sortable_array as $k => $v) {
                $new_array[] = $array[$k];
            }
        }
        return $new_array;
    }

    /**
     * Displays a list of possible owners of this extension.
     *
     * @return htmlElement HTML meta data
     */
    function display_html_user() {
    	$return = new htmlTable();
        // load list with all potential owners
        $searchScope = 'user';
        if ($this->searchOwnersInTreeSuffix) {
            $searchScope = 'tree';
        }
        $searchClass = 'inetOrgPerson';
        if ($this->filterOwnerForAsteriskAccounts) {
            $searchClass = 'AsteriskSIPUser';
        }
        $entries = searchLDAPByAttribute(null, null, $searchClass, array('dn'), array($searchScope));
        $users_dn = array();
        for ($i = 0; $i < sizeof($entries); $i++) {
            $dn = $entries[$i]['dn'];
            if (isset($dn) && (!isset($this->extensionOwners) || !in_array($dn, $this->extensionOwners))) {
                $users_dn[getAbstractDN($dn)] = $dn;
            }
        }
        if (isset($this->extensionOwners)) {
            $memberList = $this->extensionOwners;
        } else {
            $memberList = array();
        }
        $members = array();
        for ($i = 0; $i < sizeof($memberList); $i++) {
            $members[getAbstractDN($memberList[$i])] = $memberList[$i];
        }
        // options
        $return->addElement(new htmlTableExtendedInputCheckbox('filterAsteriskUsers', $this->filterOwnerForAsteriskAccounts, _('Show only Asterisk accounts'), 'ownerOptions', false));
        $refreshButton = new htmlButton('changeFilter', _('Refresh'));
        $refreshButton->rowspan = 2;
        $return->addElement($refreshButton, true);
        $return->addElement(new htmlTableExtendedInputCheckbox('useTreeSuffix', $this->searchOwnersInTreeSuffix, _('Search tree suffix for users'), 'ownerOptions', false), true);
        $return->addElement(new htmlSpacer(null, '10px'), true);
        // owners
        $ownerContent = new htmlTable();
        $ownerContent->addElement(new htmlOutputText(_("Selected users")));
        $ownerContent->addElement(new htmlOutputText(""));
        $ownerContent->addElement(new htmlOutputText(_("Available users")));
        $ownerContent->addNewLine();
        $selectedList = new htmlSelect('removeusers', $members, null, 15);
        $selectedList->setHasDescriptiveElements(true);
        $selectedList->setMultiSelect(true);
        $selectedList->setRightToLeftTextDirection(true);
        $ownerContent->addElement($selectedList);
        $ownerButtons = new htmlTable();
        $ownerButtons->addElement(new htmlButton('addusers_button', 'back.gif', true), true);
        $ownerButtons->addElement(new htmlButton('removeusers_button', 'forward.gif', true));
        $ownerContent->addElement($ownerButtons);
        $availableList = new htmlSelect('addusers', $users_dn, null, 15);
        $availableList->setTransformSingleSelect(false);
        $availableList->setHasDescriptiveElements(true);
        $availableList->setMultiSelect(true);
        $availableList->setRightToLeftTextDirection(true);
        $ownerContent->addElement($availableList);
        $ownerContent->colspan = 4;
        $return->addElement(new htmlSubTitle(_("Extension owners")), true);
        $return->addElement($ownerContent, true);
        // back button
        $backButton = new htmlAccountPageButton(get_class($this), 'attributes', 'back', _('Ok'));
        $backButton->colspan = 3;
        $return->addElement($backButton);
        return $return;
    }

    /**
     * Processes user input of the user selection page.
     * It checks if all input values are correct and updates the associated LDAP attributes.
     *
     * @return array list of info/error messages
     */
    function process_user() {
        if (isset($_POST['changeFilter'])) {
            // update filter value
            if (isset($_POST['filterAsteriskUsers'])) {
                $this->filterOwnerForAsteriskAccounts = ($_POST['filterAsteriskUsers'] == 'on');
            } else {
                $this->filterOwnerForAsteriskAccounts = false;
            }
            // update search suffix
            if (isset($_POST['useTreeSuffix'])) {
                $this->searchOwnersInTreeSuffix = ($_POST['useTreeSuffix'] == 'on');
            } else {
                $this->searchOwnersInTreeSuffix = false;
            }
        }
        if (!isset($this->extensionOwners))
            $this->extensionOwners = array();
        if (isset($_POST['addusers']) && isset($_POST['addusers_button'])) { // Add users to list
            // Add new user
            $this->extensionOwners = @array_merge($this->extensionOwners, $_POST['addusers']);
        } elseif (isset($_POST['removeusers']) && isset($_POST['removeusers_button'])) { // remove users from list
            $this->extensionOwners = array_delete($_POST['removeusers'], $this->extensionOwners);
        }
        $this->attributes['member'] = $this->extensionOwners;

        return array();
    }
    
    /**
     * Returns if the extension was moved to another OU.
     * 
     * @return boolean true if moved
     */
    function isMoveToNewSuffix(){
        $orig_suffix = extractDNSuffix($this->getAccountContainer()->dn_orig);
        if (strcmp($orig_suffix, $this->getAccountContainer()->dnSuffix) != 0 && !$this->getAccountContainer()->isNewAccount){
            return true;
        }
        return false;
    }
    
    
    
    /**
     * Returns true if at least one owner is set and false otherwise
     * 
     * @return boolean true if one or more owners
     */
    function isExtensionOwnerSet(){
        if ( sizeof($this->extensionOwners) > 0 ) {
            return true;
        }
        return false;
    }
    
    /**
     * Writes variables into object and does some regex checks.
     *
     * @return array error messages
     */
    function process_attributes() {
        $errors = array();
        $extensionName = array();
        
        if (!isset($_POST['generate_extension_name'])) {

            //perform normal set of operations
            if (isset($_POST['AstExtension']) && $_POST['AstExtension'] != '') {
            	$this->attributes['AstExtension'][0] = $_POST['AstExtension'];
                $extensionName[0] = $_POST['AstExtension'];
            } else {
                $errors[] = $this->messages['AstExtension'][0];
            }
            $extensionContext = array();
            if (isset($_POST['AstContext'])) {
            	$this->attributes['AstContext'][0] = $_POST['AstContext'];
                $extensionContext[0] = $_POST['AstContext'];
                if ($extensionContext[0] == '') {
                    $errors[] = $this->messages['AstContext'][0];
                }
            }

            if ($this->getAccountContainer()->isNewAccount && isset($extensionName[0])) {
                $isPresented = $this->isThisExtensionPresented($extensionName[0]);
                if ($isPresented) {
                    $errors[] = $this->messages['AstExtension'][1];
                }
            }

            if (isset($_POST['add_rule'])) {
                $this->addRuleFlag = true;
            }
            //process fileds that are individual for each row
            $extrow_errors = $this->processExtensionRows($extensionName, $extensionContext);

            if (isset($extrow_errors) && count($extrow_errors) > 0) {

                $errors = array_merge($errors, $extrow_errors);
            }
        } else {
            //find new free extension name
            $this->extensionRows[0]['astextension'][0] = $this->generateNextExtensionName();
        }

        return $errors;
    }

    /**
     * Processes the rule data.
     * 
     * @param String $extensionName extension name
     * @param String $extensionContext extension context
     * @return array error messages
     */
    function processExtensionRows($extensionName, $extensionContext) {
        $errors = array();
        if (isset($_POST['extension_rows']) && get_preg($_POST['extension_rows'], 'digit')) {
            
            $extensionPriorityCntr = 1;
            $this->extensionRows = array();
            for ($entryCounter = 0; $entryCounter < $_POST['extension_rows']; $entryCounter++) {
                $extRow = array();
                if (!isset($_POST['delete_rule_' . $entryCounter])) {

                    $singleExtAddErrors = $this->processSingleExtension($extRow,$extensionName, $extensionContext, $entryCounter,$extensionPriorityCntr);
                    $errors = $errors + $singleExtAddErrors;
                    
                    if ((isset($extRow['astapplication'][0]) && $extRow['astapplication'][0] != "") ||
                            (isset($extRow['astapplicationdata'][0]) && $extRow['astapplicationdata'][0] != "")
                    ) {
                        $this->extensionRows[$extensionPriorityCntr - 1] = $extRow;
                        $extensionPriorityCntr++;
                    }
                }
            }
            
            //trow error banner if last row unporpertly filled (for now the only reason for that is unfilled AstApplication filed)
            if (isset($_POST['AstApplicationData_' . ($_POST['extension_rows'] - 1)]) && $_POST['AstApplicationData_' . ($_POST['extension_rows'] - 1)] != "" &&
                    ($_POST['AstApplication_' . ($_POST['extension_rows'] - 1)] == "" || !isset($_POST['AstApplication_' . ($_POST['extension_rows'] - 1)]))) {
                $errors[] = $this->messages['AstApplication'][0];
            }
            
            //process priority change on rule
            $this->processPriorityChange();
            
            //finally sort extensions by priority
            $this->extensionRows = $this->array_sort($this->extensionRows, 'astpriority');
        }

        return $errors;
    }
    
    /**
     * Set extension owner as current logged in user.
     */
    function setDefaultExtensionOwner(){
        $login = $this->getDefaultExtensionOwner();
        $this->extensionOwners[0] = $login;
        $this->attributes['member'] = array($login);
    }
    
    /**
     * Returns the default extension owner.
     * 
     * @return String owner
     */
    function getDefaultExtensionOwner(){
        $credentials = $_SESSION['ldap']->decrypt_login();
		$login = $credentials[0];
        return $login;
    }
    
    /**
     * Fills the fileds of a single extension row.
     * In Asterisk it would only be an extension name,a priority,an application, but LDAP spicific 
     * add to processing context field.
     * 
     * @param array $extRow - hash array to store single extension properties;
     * @param String $extensionName extension name
     * @param String $extensionContext extension context
     * @param int $entryCounter - counter to distinguish single extensuion properties from $_POST
     * @param int $extensionPriorityCntr - it is the variable where actual number of extension rules wuld be aggregated 
     */
    function processSingleExtension(&$extRow,$extensionName, $extensionContext ,$entryCounter,$extensionPriorityCntr) {
        $errors = array();
        if (isset($extensionContext[0])) {
            $extRow['astcontext'][0] = $extensionContext[0];
        }
        if (isset($extensionName[0])) {
            $extRow['astextension'][0] = $extensionName[0];
        }

        $extRow["objectclass"][0] = "top";
        $extRow["objectclass"][1] = "groupOfNames";
        $extRow["objectclass"][2] = "AsteriskExtension";

        $extRow['astapplication'][0] = $_POST['AstApplication_' . $entryCounter];
        if (!isset($_POST['AstApplication_' . $entryCounter]) || $_POST['AstApplication_' . $entryCounter] == '') {
            if ($entryCounter < ($_POST['extension_rows'] - 1)) {
                $errors[] = $this->messages['AstApplication'][0];
            }
        }

        $extRow['astpriority'][0] = $extensionPriorityCntr;

        $extRow['cn'][0] = $extRow['astextension'][0] . "-" . $extRow['astpriority'][0];

        if (isset($_POST['AstApplicationData_' . $entryCounter]) && $_POST['AstApplicationData_' . $entryCounter] != "") {
            $extRow['astapplicationdata'][0] = $_POST['AstApplicationData_' . $entryCounter];
        }

        //Fille the member filed        
        $extRow['member'] = $this->extensionOwners;
        if (!isset($extRow['member']) || count($extRow['member']) == 0) {
            if (!isset($_POST['form_subpage_' . get_class($this) . '_user_open'])) {
                $errors[] = $this->messages['member'][0];
            }
        } 
        return $errors;
    }

    /**
     * Reorders the rules if the user clicked on a move button.
     */
    function processPriorityChange() {
        for ($entryCounter = 0; $entryCounter < sizeof($this->extensionRows); $entryCounter++) {
            if (isset($_POST["rule_up_button_" . $entryCounter])) {

                $this->extensionRows[$entryCounter]['astpriority'][0]--;
                $this->extensionRows[$entryCounter - 1]['astpriority'][0]++;
            } else if (isset($_POST["rule_down_button_" . $entryCounter])) {
                $this->extensionRows[$entryCounter]['astpriority'][0]++;
                $this->extensionRows[$entryCounter + 1]['astpriority'][0]--;
            }
        }
    }

    /**
     * Search by extension name and retun true if fields with this extension name is presented
     * and false otherwise.
     * 
     * @return boolean true if there are entries with this extension name.
     */
    function isThisExtensionPresented($extension) {
        $searchClass = "AsteriskExtension";
        $searchScope = 'asteriskExtNewUI';

        $entries = searchLDAPByAttribute("AstExtension", $extension, $searchClass, array('dn'), array($searchScope));

        $isPresented = false;
        if (count($entries) > 0) {
            $isPresented = true;
        }

        return $isPresented;
    }

    /**
     * This function searches in the base subtree and finds all extensions names within.
     * The generation algorithm is the naive one, so only work when extension is numbers.
     * All extension name is sorted and new extension name will be greates on + 1.
     * 
     * @return String suggested extension name
     */
    function generateNextExtensionName() {
        $searchClass = "AsteriskExtension";
        $searchScope = 'asteriskExtNewUI';
        
        $suggeted_extension = ''; //default empty value
        if( isset($this->extensionRows[0]['astextension'][0]) ){
            $suggeted_extension = $this->extensionRows[0]['astextension'][0];
        }else{
            $entries = searchLDAPByAttribute(null, null, $searchClass, array('astextension'), array($searchScope));
            $entries = $this->array_sort($entries, 'astextension');

            if (isset($entries[sizeof($entries) - 1]['astextension'][0])) {
                $greater_ext_name = $entries[sizeof($entries) - 1]['astextension'][0];
            } else {
                $greater_ext_name = 0;
            }
            if(is_numeric($greater_ext_name)){
                $suggeted_extension = $greater_ext_name + 1;
            }else{
                $ptn = "/(?P<name>[A-Za-z0-9]+)_(?P<digit>\\d+)/";
                if(preg_match_all($ptn, $greater_ext_name, $matches, PREG_PATTERN_ORDER)){
                    $next_digit = $matches['digit'][0] + 1;
                    $suggeted_extension = $matches['name'][0] . '_' . $next_digit;
                }else{
                    $suggeted_extension = $greater_ext_name . '_1';
                }
            }
        }
        
        return $suggeted_extension;
    }

    /**
     * Returns a list of PDF entries
     */
    function get_pdfEntries() {
    	$a = $this->attributes;
    	$o = $this->orig;
        $return = array();
        $extName = '';
        if (isset($this->attributes['AstExtension'][0])) {
            $extName = $this->attributes['AstExtension'][0];
        }
        if (isset($this->extensionRows[0]['astextension'][0])) {
            $extName = $this->extensionRows[0]['astextension'][0];
        }
        // global values
        if (isset($this->attributes['AstContext'])) {
            $return[get_class($this) . '_AstContext'] = array('<block><key>' . _('Account context') . '</key><value>' . $this->attributes['AstContext'][0] . '</value></block>');
        }
        if (isset($this->attributes['AstExtension'])) {
            $return[get_class($this) . '_AstExtension'] = array('<block><key>' . _('Extension name') . '</key><value>' . $this->attributes['AstExtension'][0] . '</value></block>');
        }
        if (isset($this->attributes['member'])) {
            $return[get_class($this) . '_owners'] = array('<block><key>' . _('Extension owners') . '</key><value>' . implode('; ', $this->attributes['member']) . '</value></block>');
        }
        // rules
        $entries = $this->load_extension_parts($extName);
        $rulePDF = array();
        $rulePDF[] = '<block><tr><td width="80%"> </td></tr></block>';
        $rulePDF[] = '<block><tr><td width="80%"> </td></tr></block>';
        $rulePDF[] = '<block><tr><td width="80%"> </td></tr></block>';
        $rulePDF[] = '<block><tr>' .
                '<td width="20%"><b>' . _('Name') . '</b></td>' .
                '<td width="30%"><b>' . _('Application') . '</b></td>' .
                '<td width="30%"><b>' . _('Application data') . '</b></td>' .
                '<td width="20%"><b>' . _('Priority') . '</b></td>' .
                '</tr></block>';
        for ($i = 0; $i < sizeof($entries); $i++) {
            $appdata = ' ';
            if (isset($entries[$i]['astapplicationdata'][0])) {
                $appdata = $entries[$i]['astapplicationdata'][0];
            }
            $rulePDF[] = '<block><tr>' .
                    '<td width="20%" align=\"L\">' . $entries[$i]['cn'][0] . '</td>' .
                    '<td width="30%" align=\"L\">' . $entries[$i]['astapplication'][0] . '</td>' .
                    '<td width="30%" align=\"L\">' . $appdata . '</td>' .
                    '<td width="20%" align=\"L\">' . $entries[$i]['astpriority'][0] . '</td></tr></block>';
        }
        $return[get_class($this) . '_rules'] = $rulePDF;
        return $return;
    }

    /**
     * In this function the LDAP account is built up.
     *
     * @param array $rawAccounts list of hash arrays (name => value) from user input
     * @param array $partialAccounts list of hash arrays (name => value) which are later added to LDAP
     * @param array $ids list of IDs for column position (e.g. "posixAccount_uid" => 5)
     * @param array $selectedModules list of selected account modules
     * @return array list of error messages if any
     */
    function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules) {
        $messages = array();
        //hash to strore extension_name => priority. For new extensoin priority will be 1
        $extensionNamePriorityMap = array();
        for ($rawAccIdx = 0; $rawAccIdx < sizeof($rawAccounts); $rawAccIdx++) {            
            // add object class
            if (!in_array("AsteriskExtension", $partialAccounts[$rawAccIdx]['objectClass']))
                $partialAccounts[$rawAccIdx]['objectClass'][] = "AsteriskExtension";
            if (!in_array("groupOfNames", $partialAccounts[$rawAccIdx]['objectClass']))
                $partialAccounts[$rawAccIdx]['objectClass'][] = "groupOfNames";
            // attributes
            $partialAccounts[$rawAccIdx]['AstExtension'] = $rawAccounts[$rawAccIdx][$ids['asteriskExtensionNewUI_AstExtension']];
            
            $extensionName = $partialAccounts[$rawAccIdx]['AstExtension'];
            $astPriorityTmp = 0;
            if(isset ($extensionNamePriorityMap[$extensionName])){
                $astPriorityTmp = $extensionNamePriorityMap[$extensionName] + 1;
                $extensionNamePriorityMap[$extensionName] = $astPriorityTmp;
            }else{
                $astPriorityTmp = 1;
                $extensionNamePriorityMap[$extensionName] = $astPriorityTmp;
            }            
            $partialAccounts[$rawAccIdx]['AstPriority'] = $astPriorityTmp;
            
            $partialAccounts[$rawAccIdx]['cn'] =   $extensionName  . '-' . $astPriorityTmp;
            
            if(isset($rawAccounts[$rawAccIdx][$ids['asteriskExtensionNewUI_owner']]) && $rawAccounts[$rawAccIdx][$ids['asteriskExtensionNewUI_owner']] != '' ){
                $partialAccounts[$rawAccIdx]['member'] = explode(';', $rawAccounts[$rawAccIdx][$ids['asteriskExtensionNewUI_owner']]);
            }else{
                $partialAccounts[$rawAccIdx]['member'] = $this->getDefaultExtensionOwner();
            }
            if ($rawAccounts[$rawAccIdx][$ids['asteriskExtensionNewUI_AstApplication']] != "") {
                $partialAccounts[$rawAccIdx]['AstApplication'] = $rawAccounts[$rawAccIdx][$ids['asteriskExtensionNewUI_AstApplication']];
            }
            if (isset($rawAccounts[$rawAccIdx][$ids['asteriskExtensionNewUI_AstApplicationData']])
                    && $rawAccounts[$rawAccIdx][$ids['asteriskExtensionNewUI_AstApplicationData']] != "") {
                $partialAccounts[$rawAccIdx]['AstApplicationData'] = $rawAccounts[$rawAccIdx][$ids['asteriskExtensionNewUI_AstApplicationData']];
            }
            if ($rawAccounts[$rawAccIdx][$ids['asteriskExtensionNewUI_AstContext']] != "") {
                $partialAccounts[$rawAccIdx]['AstContext'] = $rawAccounts[$rawAccIdx][$ids['asteriskExtensionNewUI_AstContext']];
            }
        }
        
       return $messages;
    }
    
    /**
     * Get list of all applications for given extension and move it into new suffix.
     * 
     * @return array list of error messages
     */
    function moveExtentionToNewSuffix($rowOrig) {
        if ($this->isMoveToNewSuffix()) {
            $orig_suffix = extractDNSuffix($this->getAccountContainer()->dn_orig);
            $oldDN = "cn=" . $rowOrig["cn"][0] . "," . extractDNSuffix($this->getAccountContainer()->dn_orig);
            $newRDN = "cn=" . $rowOrig["cn"][0];
            $is_rename_success = false;
            $is_rename_success = @ldap_rename($_SESSION['ldap']->server(), $oldDN, $newRDN, $this->getAccountContainer()->dnSuffix, true);
            if (!$is_rename_success) {
                $errors[] = array('ERROR', sprintf(_('Was unable to rename DN: %s.'), $this->getAccountContainer()->dn_orig), ldap_error($_SESSION['ldap']->server()));
                logNewMessage(LOG_ERR, 'Unable to rename ' . $oldDN . ' to ' . $newRDN . ',' . $this->getAccountContainer()->dnSuffix);
            }
        }        
    }
    
	/**
	* Returns a list of modifications which have to be made to the LDAP account.
	* 
	* Calling this method requires the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	*
	* <br>This function returns an array with 3 entries:
	* <br>array( DN1 ('add' => array($attr), 'remove' => array($attr), 'modify' => array($attr)), DN2 .... )
	* <br>DN is the DN to change. It is possible to change several DNs (e.g. create a new user and add him
	* to some groups via attribute memberUid)<br>
	* <br><b>"add"</b> are attributes which have to be added to the LDAP entry
	* <br><b>"remove"</b> are attributes which have to be removed from the LDAP entry
	* <br><b>"modify"</b> are attributes which have to be modified in the LDAP entry
	* <br><b>"notchanged"</b> are attributes which stay unchanged
	* <br><b>"info"</b> values with informational value (e.g. to be used later by pre/postModify actions)
	* <br>
	* <br>This builds the required comands from $this-attributes and $this->orig.
	* 
	* @return array list of modifications
	*/
	function save_attributes() {
	$errors = array();
        $this->extensionRows = $this->array_sort($this->extensionRows, 'astpriority');

        //Modify existent config roes if necessary
        for ($rowCounter = 0; $rowCounter < count($this->extensionRowsOrig); $rowCounter++) {
            $rowOrig = $this->extensionRowsOrig[$rowCounter];
            unset($rowOrig['dn']);
            if ($rowCounter < count($this->extensionRows)) {
                $row = $this->extensionRows[$rowCounter];

                if (count($diffVals = array_diff_key($rowOrig, $row)) != 0) {
                    ldap_mod_del($_SESSION['ldap']->server(), "cn=" . $row["cn"][0] . "," . $this->getAccountContainer()->dnSuffix, $diffVals);
                }
                if (count($diffVals = array_diff_key($row, $rowOrig)) != 0) {
                    ldap_mod_add($_SESSION['ldap']->server(), "cn=" . $row["cn"][0] . "," . $this->getAccountContainer()->dnSuffix, $diffVals);
                }
                $diffValsSerialysed = array_diff(array_map("serialize", array_intersect_key($row, $rowOrig)), array_map("serialize", $rowOrig));
               
                //if new suffix jast move old rows to the new suffix and go on
                $this->moveExtentionToNewSuffix($rowOrig);
                
                if (count($diffValsSerialysed) != 0) {
                    $diffVals = array_map("unserialize", $diffValsSerialysed);
                    if($row["cn"][0] ==  $rowOrig["cn"][0]){
                        ldap_mod_replace($_SESSION['ldap']->server(), "cn=" . $row["cn"][0] . "," . $this->getAccountContainer()->dnSuffix, $diffVals);
                    }else{
                        $origDN = "cn=" . $rowOrig["cn"][0] . "," . $this->getAccountContainer()->dnSuffix;
                        $newRDN = "cn=" . $row["cn"][0];
                        
                        ldap_rename($_SESSION['ldap']->server(), $origDN, $newRDN, $this->getAccountContainer()->dnSuffix, true);
                        ldap_mod_replace($_SESSION['ldap']->server(), $newRDN . "," . $this->getAccountContainer()->dnSuffix, $diffVals);
                    }
                }
            } else {
                ldap_delete($_SESSION['ldap']->server(), "cn=" . $rowOrig["cn"][0] . "," . extractDNSuffix($this->getAccountContainer()->dn_orig));
            }
        }
        //Add new config rows
        for ($rowCounter = count($this->extensionRowsOrig); $rowCounter < count($this->extensionRows); $rowCounter++) {
            $row = $this->extensionRows[$rowCounter];
            ldap_add($_SESSION['ldap']->server(), "cn=" . $row["cn"][0] . "," . $this->getAccountContainer()->dnSuffix, $row);
        }
        
        //a trick for Edit again to work
        $this->getAccountContainer()->dn_orig = "cn=" .  $this->extensionRows[0]['cn'][0] . "," . $this->getAccountContainer()->dnSuffix;
        $this->getAccountContainer()->finalDN = "cn=" .  $this->extensionRows[0]['cn'][0] . "," . $this->getAccountContainer()->dnSuffix;
        
        $retun_obj = $this->getAccountContainer()->save_module_attributes($this->orig, $this->orig);
        
        return $retun_obj;
    }

    /**
     * Runs ufter main deltete procedure was done and do postmorten for other parts of extension
     * wtith priority > 1.
     * 
     * @return array error messages 
     */
    function postDeleteActions() {

        $extension = $this->attributes['AstExtension'][0];
        $searchClass = "AsteriskExtension";
        $searchScope = 'asteriskExtNewUI';
        $entries = searchLDAPByAttribute("AstExtension", $extension, $searchClass, array('cn', 'AstContext', 'AstPriority',
            'AstApplication', 'AstApplicationData', 'AstExtension', 'member', 'ObjectClass', 'dn'), array($searchScope));
        $entries = $this->array_sort($entries, 'astpriority');

        for ($rowCounter = 0; $rowCounter < count($entries); $rowCounter++) {
            $rowOrig = $entries[$rowCounter];
            if ($rowOrig["astpriority"][0] > 1) {                
                ldap_delete($_SESSION['ldap']->server(), $rowOrig['dn']);                  
            }
        }
        return array();
    }

}

?>
