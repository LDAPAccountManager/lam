ARG PHP_VERSION=8.1
FROM php:8.1-cli-alpine
RUN apk add --no-cache gmp-dev git && docker-php-ext-install -j$(nproc) gmp