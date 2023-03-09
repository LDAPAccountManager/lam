#!/bin/bash
#
#  Docker start script for LDAP Account Manager

#  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
#  Copyright (C) 2019  Felix Bartels

#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.

#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.

#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

set -e # non-zero return values exit the whole script
[ "${DEBUG:-false}" = "true" ] && set -x

if [ "${LAM_DISABLE_TLS_CHECK:-}" = "true" ]; then
  if ! grep -e '^TLS_REQCERT never$' /etc/ldap/ldap.conf > /dev/null; then
    echo "TLS_REQCERT never" >> /etc/ldap/ldap.conf
  fi
fi

sed -i -f- /etc/php/7.4/apache2/php.ini <<- EOF
    s|^max_execution_time =.*|max_execution_time = 60|;
    s|^post_max_size =.*|post_max_size = 100M|;
    s|^upload_max_filesize =.*|upload_max_filesize = 100M|;
    s|^memory_limit =.*|memory_limit = 256M|;
EOF


LAM_SKIP_PRECONFIGURE="${LAM_SKIP_PRECONFIGURE:-false}"
if [ "$LAM_SKIP_PRECONFIGURE" != "true" ]; then
  echo "Configuring LAM"

  LAM_LANG="${LAM_LANG:-en_US}"
  export LAM_PASSWORD="${LAM_PASSWORD:-lam}"
  LAM_PASSWORD_SSHA=$(php -r '$password = getenv("LAM_PASSWORD"); $rand = abs(hexdec(bin2hex(openssl_random_pseudo_bytes(5)))); $salt0 = substr(pack("h*", md5($rand)), 0, 8); $salt = substr(pack("H*", sha1($salt0 . $password)), 0, 4); print "{SSHA}" . base64_encode(pack("H*", sha1($password . $salt))) . " " . base64_encode($salt) . "\n";')
  LDAP_SERVER="${LDAP_SERVER:-ldap://ldap:389}"
  LDAP_DOMAIN="${LDAP_DOMAIN:-my-domain.com}"
  LDAP_BASE_DN="${LDAP_BASE_DN:-dc=${LDAP_DOMAIN//\./,dc=}}"
  LDAP_USERS_DN="${LDAP_USERS_DN:-${LDAP_BASE_DN}}"
  LDAP_GROUPS_DN="${LDAP_GROUPS_DN:-${LDAP_BASE_DN}}"
  LDAP_ADMIN_USER="${LDAP_USER:-cn=admin,${LDAP_BASE_DN}}"
  LAM_LICENSE="${LAM_LICENSE:-}"
  LAM_CONFIGURATION_DATABASE="${LAM_CONFIGURATION_DATABASE:-files}"
  LAM_CONFIGURATION_HOST="${LAM_CONFIGURATION_HOST:-}"
  LAM_CONFIGURATION_PORT="${LAM_CONFIGURATION_PORT:-}"
  LAM_CONFIGURATION_DATABASE_NAME="${LAM_CONFIGURATION_DATABASE_NAME:-}"
  LAM_CONFIGURATION_USER="${LAM_CONFIGURATION_USER:-}"
  LAM_CONFIGURATION_PASSWORD="${LAM_CONFIGURATION_PASSWORD:-}"

  sed -i -f- /etc/ldap-account-manager/config.cfg <<- EOF
    s|^password:.*|password: ${LAM_PASSWORD_SSHA}|;
    s|^license:.*|license: ${LAM_LICENSE}|;
    s|^configDatabaseType:.*|configDatabaseType: ${LAM_CONFIGURATION_DATABASE}|;
    s|^configDatabaseServer:.*|configDatabaseServer: ${LAM_CONFIGURATION_HOST}|;
    s|^configDatabasePort:.*|configDatabasePort: ${LAM_CONFIGURATION_PORT}|;
    s|^configDatabaseName:.*|configDatabaseName: ${LAM_CONFIGURATION_DATABASE_NAME}|;
    s|^configDatabaseUser:.*|configDatabaseUser: ${LAM_CONFIGURATION_USER}|;
    s|^configDatabasePassword:.*|configDatabasePassword: ${LAM_CONFIGURATION_PASSWORD}|;
EOF
  unset LAM_PASSWORD

  set +e
  ls -l /var/lib/ldap-account-manager/config/lam.conf
  cfgFilesExist=$?
  set -e
  if [ $cfgFilesExist -ne 0 ]; then
    cp /var/lib/ldap-account-manager/config/unix.sample.conf /var/lib/ldap-account-manager/config/lam.conf
	  chown www-data /var/lib/ldap-account-manager/config/lam.conf
  fi

  sed -i -f- /var/lib/ldap-account-manager/config/lam.conf <<- EOF
    s|^ServerURL:.*|ServerURL: ${LDAP_SERVER}|;
    s|^Admins:.*|Admins: ${LDAP_ADMIN_USER}|;
    s|^Passwd:.*|Passwd: ${LAM_PASSWORD_SSHA}|;
    s|^tools: treeViewSuffix:.*|tools: treeViewSuffix: ${LDAP_BASE_DN}|;
    s|^defaultLanguage:.*|defaultLanguage: ${LAM_LANG}.utf8|;
    s|^.*suffix_user:.*|types: suffix_user: ${LDAP_USERS_DN}|;
    s|^.*suffix_group:.*|types: suffix_group: ${LDAP_GROUPS_DN}|;
EOF

fi

echo "Starting Apache"
rm -f /run/apache2/apache2.pid
set +u
# shellcheck disable=SC1091
source /etc/apache2/envvars
exec /usr/sbin/apache2 -DFOREGROUND
