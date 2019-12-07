#!/bin/bash

set -eu # unset variables are errors & non-zero return values exit the whole script
[ "$DEBUG" ] && set -x

LAM_LANG="${LAM_LANG:-en_US}"
export LAM_PASSWORD="${LAM_PASSWORD:-lam}"
LAM_PASSWORD_SSHA=$(php -r '$password = getenv("LAM_PASSWORD"); mt_srand((microtime() * 1000000)); $rand = abs(hexdec(bin2hex(openssl_random_pseudo_bytes(5)))); $salt0 = substr(pack("h*", md5($rand)), 0, 8); $salt = substr(pack("H*", sha1($salt0 . $password)), 0, 4); print "{SSHA}" . base64_encode(pack("H*", sha1($password . $salt))) . " " . base64_encode($salt) . "\n";')
LAM_TIMEZONE="${LAM_TIMEZONE:-Europe/Berlin}"
LDAP_HOST="${LDAP_HOST:-ldap://ldap:389}"
LDAP_DOMAIN="${LDAP_DOMAIN:-mydomain.com}"
LDAP_BASE_DN="${LDAP_BASE_DN:-dc=${LDAP_DOMAIN//\./,dc=}}"
ADMIN_USER="${LDAP_USER:-cn=admin,${LDAP_BASE_DN}}"

echo "Setting LAM password to: $LAM_PASSWORD"
sed -i -f- /etc/ldap-account-manager/config.cfg <<- EOF
	s|^password:.*|password: ${LAM_PASSWORD_SSHA}|;
EOF
unset LAM_PASSWORD

sed -i -f- /var/lib/ldap-account-manager/config/lam.conf <<- EOF
	s|^ServerURL:.*|ServerURL: ${LDAP_HOST}|;
	s|^Admins:.*|Admins: ${ADMIN_USER}|;
	s|^Passwd:.*|Passwd: ${LAM_PASSWORD_SSHA}|;
	s|^treesuffix:.*|treesuffix: ${LDAP_BASE_DN}|;
	s|^defaultLanguage:.*|defaultLanguage: ${LAM_LANG}.utf8|;
	s|^types: suffix_user:.*|types: suffix_user: ${LDAP_BASE_DN}|;
	s|^types: suffix_group:.*|types: suffix_group: ${LDAP_BASE_DN}|;
EOF

echo "Starting Apache"
rm -f /run/apache2/apache2.pid
set +u
# shellcheck disable=SC1091
source /etc/apache2/envvars
exec /usr/sbin/apache2 -DFOREGROUND
