<?php

/*
  $Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2009  Pozdnyak Pavel
  2010  Roland Gruber

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
 * The account type for Asterisk extensions.
 *
 * @package types
 * @author Pozdnyak Pavel
 */

/**
 * The account type for Asterisk extensions.
 *
 * @package types
 */
class asteriskExtNewUI extends baseType {

    /**
     * Constructs a new domain type object.
     */
    public function __construct() {
        parent::__construct();
        $this->LABEL_CREATE_ANOTHER_ACCOUNT = _('Create another extension');
        $this->LABEL_BACK_TO_ACCOUNT_LIST = _('Back to extensions list');
    }

    /**
     * Returns the alias name of this account type.
     *
     * @return string alias name
     */
    function getAlias() {
        return _("Asterisk extensions new UI");
    }

    /**
     * Returns the description of this account type.
     *
     * @return string description
     */
    function getDescription() {
        return _("Asterisk extensions entries");
    }

    /**
     * Returns the class name for the list object.
     *
     * @return string class name
     */
    function getListClassName() {
        return "lamAsteriskExtNewUIList";
    }

    /**
     * Returns the default attribute list for this account type.
     *
     * @return string attribute list
     */
    function getDefaultListAttributes() {
        return "#AstExtension;#AstContext;#member";
    }

    /**
     * Returns a list of attributes which have a translated description.
     * This is used for the head row in the list view.
     *
     * @return array list of descriptions
     */
    function getListAttributeDescriptions() {
        return array(
//"cn" => _("Extension name"),
            "astextension" => _("Label"),
            "astcontext" => _("Context"),
            "member" => _("Extension owner DN"),
        );
    }

    /**
     * Returns the the title text for the title bar on the new/edit page.
     *
     * @param array $attributes list of LDAP attributes for the displayed account (null, if new account)
     * @return String title text
     */
    public function getTitleBarTitle($attributes) {
        if ($attributes == null) {
            return _("New extension");
        }
// check if a common name is set
        if (isset($attributes['cn'][0])) {
            return htmlspecialchars($attributes['cn'][0]);
        }
// fall back to default
        return parent::getTitleBarTitle($attributes);
    }

}

/**
 * Generates the list view.
 *
 * @package lists
 * @author Pozdnyak Pavel
 *
 */
class lamAsteriskExtNewUIList extends lamList {

    /**
     * Constructor
     *
     * @param string $type account type
     * @return lamList list object
     */
    function __construct($type) {
        parent::__construct($type);
        $this->labels = array(
            'nav' => _("%s extension(s) found"),
            'error_noneFound' => _("No extension(s) found!"),
            'newEntry' => _("New extension"),
            'deleteEntry' => _("Delete extension(s)"));
    }

    protected function listGetParams() {
        // check if only PDF should be shown
        parent::listGetParams();
        $this->attrArray = array("astextension", "astcontext", "member");
    }

    protected function listRefreshData() {
        parent::listRefreshData();
        // configure search filter
        $module_filter = get_ldap_filter($this->type);  // basic filter is provided by modules
        $filter = "(&" . $module_filter . $this->filterPart . ")";
        $attrs = $this->attrArray;
        $attrs[] = "astpriority";
        $entries = searchLDAP($this->suffix, $filter, $attrs);
        $lastError = getLastLDAPError();
        if ($lastError != null) {
            call_user_func_array('StatusMessage', $lastError);
        }
        
        //array_walk_recursive($entries, create_function('&$item, $key, $keyToElum', 'print($key. "<br>"); if(strcmp ($key, $keyToElum) == 0){print("ITEM - " . $item . "<br>"); $item = "dn=none";}'), 'dn');
        $entries = $this->normalizeLdapOutput($entries);        
        $this->entries = $entries;        
        // generate list of possible suffixes
        $typeObj = new $this->type();
        $this->possibleSuffixes = $typeObj->getSuffixList();
    }

    private function normalizeLdapOutput($entries){
        $entries = array_map("unserialize", array_unique(array_map("serialize", $entries)));
        foreach($entries as $key=> $value){
            if($entries[$key]["astpriority"][0] > 1){
                unset($entries[$key]);
            }
        }
        return $entries;
    }

    
     
    
}

?>
