#
#  Docker image for LDAP Account Manager

#  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
#  Copyright (C) 2019 - 2024  Roland Gruber

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

#
#  Usage: run this command: docker run -p 8080:80 -it -d ldapaccountmanager/lam:stable
#
#  Then access LAM at http://localhost:8080/
#  You can change the port 8080 if needed.
#  See possible environment variables here: https://github.com/LDAPAccountManager/lam/blob/develop/lam-packaging/docker/.env
#

FROM debian:bookworm-slim
LABEL maintainer="Roland Gruber <post@rolandgruber.de>"

ARG LAM_RELEASE=8.9.RC1
EXPOSE 80

ENV \
    DEBIAN_FRONTEND=noninteractive \
    DEBUG=''

RUN apt-get update && \
    apt-get upgrade -y

# install locales
RUN apt-get install -y locales
RUN sed -i 's/^# *\(ca_ES.UTF-8\)/\1/' /etc/locale.gen && \
        sed -i 's/^# *\(cz_CZ.UTF-8\)/\1/' /etc/locale.gen && \
        sed -i 's/^# *\(de_DE.UTF-8\)/\1/' /etc/locale.gen && \
        sed -i 's/^# *\(en_GB.UTF-8\)/\1/' /etc/locale.gen && \
        sed -i 's/^# *\(en_US.UTF-8\)/\1/' /etc/locale.gen && \
        sed -i 's/^# *\(es_ES.UTF-8\)/\1/' /etc/locale.gen && \
        sed -i 's/^# *\(fr_FR.UTF-8\)/\1/' /etc/locale.gen && \
        sed -i 's/^# *\(it_IT.UTF-8\)/\1/' /etc/locale.gen && \
        sed -i 's/^# *\(hu_HU.UTF-8\)/\1/' /etc/locale.gen && \
        sed -i 's/^# *\(nl_NL.UTF-8\)/\1/' /etc/locale.gen && \
        sed -i 's/^# *\(pl_PL.UTF-8\)/\1/' /etc/locale.gen && \
        sed -i 's/^# *\(pt_BR.UTF-8\)/\1/' /etc/locale.gen && \
        sed -i 's/^# *\(ru_RU.UTF-8\)/\1/' /etc/locale.gen && \
        sed -i 's/^# *\(sk_SK.UTF-8\)/\1/' /etc/locale.gen && \
        sed -i 's/^# *\(tr_TR.UTF-8\)/\1/' /etc/locale.gen && \
        sed -i 's/^# *\(uk_UA.UTF-8\)/\1/' /etc/locale.gen && \
        sed -i 's/^# *\(ja_JP.UTF-8\)/\1/' /etc/locale.gen && \
        sed -i 's/^# *\(zh_TW.UTF-8\)/\1/' /etc/locale.gen && \
        sed -i 's/^# *\(zh_CN.UTF-8\)/\1/' /etc/locale.gen && \
        locale-gen

RUN apt-get install --no-install-recommends -y \
        apache2 \
        ca-certificates \
        dumb-init \
        fonts-dejavu \
        libapache2-mod-php \
        php \
        php-curl \
        php-gd \
        php-imagick \
        php-ldap \
        php-monolog \
        php-phpseclib \
        php-xml \
        php-zip \
        php-imap \
        php-gmp \
        php-mysql \
        php-sqlite3 \
        php-mbstring \
        wget \
        libldap-common \
        gettext \
        libjs-jquery-jstree \
        php-phpseclib3 \
        php-voku-portable-ascii \
        libphp-phpmailer \
    && \
    rm /etc/apache2/sites-enabled/*default* && \
    rm -rf /var/cache/apt /var/lib/apt/lists/*

# install letsencrypt certificates
RUN wget https://letsencrypt.org/certs/isrgrootx1.pem \
    -O /usr/local/share/ca-certificates/isrgrootx1.pem && \
    openssl x509 -in /usr/local/share/ca-certificates/isrgrootx1.pem -inform PEM -out /usr/local/share/ca-certificates/isrgrootx1.crt && \
    wget https://letsencrypt.org/certs/isrg-root-x2.pem \
    -O /usr/local/share/ca-certificates/isrg-root-x2.pem && \
    openssl x509 -in /usr/local/share/ca-certificates/isrg-root-x2.pem -inform PEM -out /usr/local/share/ca-certificates/isrg-root-x2.crt && \
    wget https://letsencrypt.org/certs/lets-encrypt-r3.pem \
    -O /usr/local/share/ca-certificates/lets-encrypt-r3.pem && \
    openssl x509 -in /usr/local/share/ca-certificates/lets-encrypt-r3.pem -inform PEM -out /usr/local/share/ca-certificates/lets-encrypt-r3.crt && \
    wget https://letsencrypt.org/certs/lets-encrypt-e1.pem \
    -O /usr/local/share/ca-certificates/lets-encrypt-e1.pem && \
    openssl x509 -in /usr/local/share/ca-certificates/lets-encrypt-e1.pem -inform PEM -out /usr/local/share/ca-certificates/lets-encrypt-e1.crt && \
    wget https://letsencrypt.org/certs/lets-encrypt-r4.pem \
    -O /usr/local/share/ca-certificates/lets-encrypt-r4.pem && \
    openssl x509 -in /usr/local/share/ca-certificates/lets-encrypt-r4.pem -inform PEM -out /usr/local/share/ca-certificates/lets-encrypt-r4.crt && \
    wget https://letsencrypt.org/certs/lets-encrypt-e2.pem \
    -O /usr/local/share/ca-certificates/lets-encrypt-e2.pem && \
    openssl x509 -in /usr/local/share/ca-certificates/lets-encrypt-e2.pem -inform PEM -out /usr/local/share/ca-certificates/lets-encrypt-e2.crt && \
    update-ca-certificates

# install LAM
RUN wget https://github.com/LDAPAccountManager/lam/releases/download/${LAM_RELEASE}/ldap-account-manager_${LAM_RELEASE}-1_all.deb \
    -O /tmp/ldap-account-manager_${LAM_RELEASE}-1_all.deb && \
    dpkg -i /tmp/ldap-account-manager_${LAM_RELEASE}-1_all.deb && \
    rm -f /tmp/ldap-account-manager_${LAM_RELEASE}-1_all.deb

# remove link to manual as contents are removed on Docker instances
RUN rm /usr/share/ldap-account-manager/docs

# redirect Apache logging
RUN sed -e 's,^ErrorLog.*,ErrorLog "|/bin/cat",' -i /etc/apache2/apache2.conf
# because there is no logging set in the lam vhost logging goes to other_vhost_access.log
RUN ln -sf /dev/stdout /var/log/apache2/other_vhosts_access.log

RUN a2enmod ssl

# add redirect for /
RUN a2enmod rewrite
RUN echo "<VirtualHost *:80>" >> /etc/apache2/sites-available/lam.conf \
 && echo "RewriteEngine on" >> /etc/apache2/sites-available/lam.conf \
 && echo "RewriteRule   ^/$  /lam/ [R,L]" >> /etc/apache2/sites-available/lam.conf \
 && echo "</VirtualHost>" >> /etc/apache2/sites-available/lam.conf
RUN ln -s /etc/apache2/sites-available/lam.conf /etc/apache2/sites-enabled/

COPY start.sh /usr/local/bin/start.sh

WORKDIR /var/lib/ldap-account-manager/config

# start Apache when container starts
ENTRYPOINT ["/usr/bin/dumb-init", "--"]
CMD [ "/usr/local/bin/start.sh" ]

HEALTHCHECK --interval=1m --timeout=10s \
    CMD wget -qO- http://localhost/lam/ | grep -q '<title>LDAP Account Manager</title>'
