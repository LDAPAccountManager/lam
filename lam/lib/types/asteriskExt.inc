<?php

/*

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2009 - 2012  Pozdnyak Pavel
                2010 - 2026  Roland Gruber

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
 * @author Roland Gruber
 */

use LAM\TYPES\ConfiguredType;

/**
 * The account type for Asterisk extensions.
 *
 * @package types
 */
class asteriskExt extends baseType {

	/**
	 * Constructs a new domain type object.
	 *
	 * @param ConfiguredType $type configuration
	 */
	public function __construct($type) {
		parent::__construct($type);
		$this->LABEL_CREATE_ANOTHER_ACCOUNT = _('Create another extension');
		$this->LABEL_BACK_TO_ACCOUNT_LIST = _('Back to extensions list');
	}

	/**
	 * {@inheritDoc}
	 */
	public function getAlias(): string {
		return _("Asterisk extensions");
	}

	/**
	 * {@inheritDoc}
	 */
	public function getDescription(): string {
		return _("Asterisk extensions entries");
	}

	/**
	 * {@inheritDoc}
	 */
	public function getListClassName(): string {
		return lamAsteriskExtList::class;
	}

	/**
	 * {@inheritDoc}
	 */
	public function getDefaultListAttributes(): string {
		return "#AstExtension;#AstContext;#member";
	}

	/**
	 * {@inheritDoc}
	 */
	public function getTitleBarTitle(accountContainer $container): ?string {
		$attributes = null;
		if ($container->getAccountModule('asteriskExtension') != null) {
			$attributes = $container->getAccountModule('asteriskExtension')->getAttributes();
		}
		// check if a common name is set
		if (isset($attributes['AstExtension'][0])) {
			return htmlspecialchars($attributes['AstExtension'][0]);
		}
		// new account
		if ($container->isNewAccount) {
			return _("New extension");
		}
		// fall back to default
		return parent::getTitleBarTitle($container);
	}

}

/**
 * Generates the list view.
 *
 * @package lists
 * @author Pozdnyak Pavel
 *
 */
class lamAsteriskExtList extends lamList {

	/**
	 * Constructor
	 *
	 * @param ConfiguredType $type account type
	 */
	public function __construct(ConfiguredType $type) {
		parent::__construct($type);
		$this->labels = [
			'nav' => _("Extension count: %s"),
			'error_noneFound' => _("No Asterisk extensions found."),
			'newEntry' => _("New extension"),
			'deleteEntry' => _("Delete selected extensions")
		];
	}

	/**
	 * {@inheritDoc}
	 */
	protected function listGetParams(): void {
		parent::listGetParams();
		$this->attrArray = ["astextension", "astcontext", "member"];
		$this->descArray = [_("Extension name"), _("Account context"), _("Owner")];
	}

	/**
	 * {@inheritDoc}
	 */
	protected function listRefreshData(): void {
		// configure search filter
		$module_filter = get_ldap_filter($this->type->getId());  // basic filter is provided by modules
		$filter = "(&" . $module_filter . $this->buildLDAPAttributeFilter() . ")";
		$attrs = $this->attrArray;
		$attrs[] = "astpriority";
		$entries = searchLDAP($this->suffix, $filter, $attrs);
		$lastError = getLastLDAPError();
		if ($lastError != null) {
			call_user_func_array(StatusMessage(...), $lastError);
		}

		$this->ldapEntries = $this->normalizeLdapOutput($entries);
		$this->entries = [];
		foreach ($this->ldapEntries as $index) {
			$this->entries[] = $index;
		}
		// generate list of possible suffixes
		$this->possibleSuffixes = $this->type->getSuffixList();
	}

	/**
	 * Groups the extensions.
	 *
	 * @param array<string, string[]|string>[] $entries extension entries
	 * @return array<string, string[]|string>[] filtered extensions
	 */
	private function normalizeLdapOutput($entries): array {
		$filteredEntries = [];
		foreach ($entries as $entry) {
			if ($entry["astpriority"][0] == '1') {
				$filteredEntries[] = $entry;
			}
		}
		return $filteredEntries;
	}

}
