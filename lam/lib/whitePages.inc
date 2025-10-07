<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2025  Roland Gruber

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

namespace LAM\WHITE_PAGES;

use LAM\LIB\TWO_FACTOR\TwoFactorProviderService;

/**
 * Supported authentication types.
 */
enum WP_AUTHENTICATION {

	/**
	 * Anonymous authentication
	 */
	case AUTH_ANONYMOUS;

	/**
	 * User and password authentication.
	 */
	case AUTH_USER_PASSWORD;

	/**
	 * Trust the 2FA provider for authentication.
	 */
	case AUTH_2FA_ONLY;

}

/**
 * Display modes for white pages.
 */
enum WP_DISPLAY_MODE {
	case LIST_GALLERY;
	case LIST_ONLY;
	case GALLERY_ONLY;
}

/**
 * Defines the display of a single item.
 */
enum WP_ITEM_TYPE {

	case TITLE;
	case IMAGE_JPG;
	case TEXT;
	case EMAIL;
	case TELEPHONE;
	case LINK;

}

/**
 * White pages profile.
 */
class WhitePagesProfile {


	/**
	 * @var string LDAP server URL
	 */
	public string $serverUrl = 'ldap://example.com';
	/**
	 * @var bool activate TLS
	 */
	public bool $useTLS = false;
	/**
	 * @var bool follow referrals
	 */
	public bool $followReferrals = true;
	/**
	 * @var string LDAP technical user for connection
	 */
	public string $ldapConnectionUser = '';
	/**
	 * @var string password of technical user
	 */
	public string $ldapConnectionPassword = '';
	/**
	 * @var bool use technical user for all operations
	 */
	public bool $useForAllOperations = false;
	/**
	 * @var string language
	 */
	public string $language = 'en_gb';

	/**
	 * @var string authentication type
	 */
	public string $authenticationType = 'AUTH_ANONYMOUS';
	/**
	 * @var string LDAP suffix for users
	 */
	public string $authenticationLdapSuffix = 'ou=people,dc=example,dc=com';
	/**
	 * @var string LDAP search attribute to find the user for authentication
	 */
	public string $authenticationLdapSearchAttribute = 'uid';
	/**
	 * @var string additional LDAP search filter for user search
	 */
	public string $authenticationLdapAdditionalSearchFilter = '';

	/**
	 * @var string provider type
	 */
	public string $twoFactorAuthenticationType = TwoFactorProviderService::TWO_FACTOR_NONE;
	/**
	 * @var string authentication URL
	 */
	public string $twoFactorAuthenticationURL = 'https://localhost';
	/**
	 * @var bool allow insecure connection
	 */
	public bool $twoFactorAuthenticationInsecure = false;
	/**
	 * @var string label
	 */
	public string $twoFactorAuthenticationLabel = '';
	/**
	 * @var string caption
	 */
	public string $twoFactorAuthenticationCaption = '';
	/**
	 * @var string client ID
	 */
	public string $twoFactorAuthenticationClientId = '';
	/**
	 * @var string secret key
	 */
	public string $twoFactorAuthenticationSecretKey = '';
	/**
	 * @var string attribute that contains 2FA username
	 */
	public string $twoFactorAuthenticationAttribute = 'uid';
	/**
	 * @var string 2FA domain
	 */
	public string $twoFactorAuthenticationDomain = '';
	/**
	 * @var bool allow remembering the device
	 */
	public bool $twoFactorAllowToRememberDevice = false;
	/**
	 * @var int timeout for remembering the device
	 */
	public int $twoFactorRememberDeviceDuration = 28800;
	/**
	 * @var string password to reset remembered devices
	 */
	public string $twoFactorRememberDevicePassword = '';

	/**
	 * @var string header HTML
	 */
	public string $header = '';
	/**
	 * @var string header HTML for login
	 */
	public string $loginHeader = '';
	/**
	 * @var string footer HTML
	 */
	public string $footer = '';
	/**
	 * @var string custom CSS
	 */
	public string $customCss = '';
	/**
	 * @var string base color
	 */
	public string $baseColor = '';
	/**
	 * @var string secondary color
	 */
	public string $secondaryColor = '';
	/**
	 * @var string background image
	 */
	public string $backgroundImage = '';

	/**
	 * @var array tabs for white pages
	 */
	public array $tabs = [];

}

/**
 * Single tab inside white pages.
 */
class WhitePagesTab {

	/**
	 * @var string LDAP suffix to find the entries to display
	 */
	public string $ldapSuffix = 'ou=people,dc=example,dc=com';
	/**
	 * @var string additional LDAP search filter for entry search
	 */
	public string $additionalSearchFilter = '';

	/**
	 * @var string display label
	 */
	public string $label = 'Users';
	/**
	 * @var string[] searchable LDAP attributes
	 */
	public array $searchableAttributes = ['cn', 'sn', 'gn', 'mail', 'telephoneNumber'];

	/**
	 * @var WhitePagesDisplayItem[] list items
	 */
	public array $listItems = [];

	/**
	 * @var WhitePagesDisplayItem[] gallery items
	 */
	public array $galleryItems = [];

	/**
	 * @var WhitePagesDisplayItem[] detail view items
	 */
	public array $detailViewItems = [];

}

/**
 * Specifies how a line/column should be rendered.
 */
class WhitePagesDisplayItem {

	/**
	 * @var WP_ITEM_TYPE render type of item
	 */
	public WP_ITEM_TYPE $TYPE = WP_ITEM_TYPE::TEXT;

	/**
	 * @var string label of this item
	 */
	public string $label = 'First name';

	/**
	 * @var string attribute syntax of this item
	 */
	public string $attributes = 'gn';

}