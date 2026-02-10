<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2013 - 2025  Roland Gruber

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
 * The account type for Kolab shared folders.
 *
 * @package types
 * @author Roland Gruber
 */

use LAM\TYPES\ConfiguredType;

/**
 * The account type for Kolab shared folders.
 *
 * @package types
 */
class kolabSharedFolderType extends baseType {

	/**
	 * Constructs a new shared folder object.
	 *
	 * @param ConfiguredType $type configuration
	 */
	public function __construct($type) {
		parent::__construct($type);
		$this->LABEL_CREATE_ANOTHER_ACCOUNT = _('Create another shared folder');
		$this->LABEL_BACK_TO_ACCOUNT_LIST = _('Back to shared folder list');
	}

	/**
	 * {@inheritDoc}
	 */
	public function getAlias(): string {
		return _("Kolab shared folders");
	}

	/**
	 * {@inheritDoc}
	 */
	public function getDescription(): string {
		return _("Kolab shared folders (e.g. mail folders)");
	}

	/**
	 * {@inheritDoc}
	 */
	public function getListClassName(): string {
		return kolabSharedFolderTypeList::class;
	}

	/**
	 * {@inheritDoc}
	 */
	public function getDefaultListAttributes(): string {
		return "#cn;#kolabDelegate;#alias";
	}

	/**
	 * {@inheritDoc}
	 */
	public function getTitleBarTitle(accountContainer $container): ?string {
		// get attributes
		$attributes = [];
		if ($container->getAccountModule('kolabSharedFolder') != null) {
			$attributes = $container->getAccountModule('kolabSharedFolder')->getAttributes();
		}
		// check if cn is set
		if (isset($attributes['cn'][0])) {
			return htmlspecialchars($attributes['cn'][0]);
		}
		// show new label
		if ($container->isNewAccount) {
			return _("New shared folder");
		}
		// fall back to default
		return parent::getTitleBarTitle($container);
	}

}


/**
 * Generates the list view.
 *
 * @package lists
 * @author Roland Gruber
 *
 */
class kolabSharedFolderTypeList extends lamList {

	/**
	 * Constructor
	 *
	 * @param ConfiguredType $type account type
	 */
	public function __construct(ConfiguredType $type) {
		parent::__construct($type);
		$this->labels = [
			'nav' => _("Shared folder count: %s"),
			'error_noneFound' => _("No shared folders found!"),
			'newEntry' => _("New shared folder"),
			'deleteEntry' => _("Delete selected shared folders")
		];
	}

}
