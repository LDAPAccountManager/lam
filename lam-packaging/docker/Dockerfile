#
#  Docker image for LDAP Account Manager

#  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
#  Copyright (C) 2019  Roland Gruber

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
#

FROM debian:stretch
MAINTAINER Roland Gruber <post@rolandgruber.de>

ARG LAM_RELEASE=6.7

# update OS
RUN apt-get update \
 && apt-get upgrade -y

# install requirements
RUN apt-get install -y wget apache2 libapache2-mod-php php php-ldap php-zip php-xml php-curl php-gd php-imagick php-mcrypt php-tcpdf php-phpseclib fonts-dejavu

# install LAM
RUN wget http://prdownloads.sourceforge.net/lam/ldap-account-manager_${LAM_RELEASE}-1_all.deb?download -O /tmp/ldap-account-manager_${LAM_RELEASE}-1_all.deb \
 && dpkg -i /tmp/ldap-account-manager_${LAM_RELEASE}-1_all.deb

# cleanup
RUN apt-get autoremove -y && apt-get clean all \
 && rm -f /tmp/ldap-account-manager_${LAM_RELEASE}-1_all.deb \
 && rm /etc/apache2/sites-enabled/*default*

# add redirect for /
RUN a2enmod rewrite
RUN echo "RewriteEngine on" >> /etc/apache2/conf-enabled/laminit.conf \
 && echo "RewriteRule   ^/$  /lam/ [R,L]" >> /etc/apache2/conf-enabled/laminit.conf

# start Apache when container starts
ENTRYPOINT service apache2 start && sleep infinity

