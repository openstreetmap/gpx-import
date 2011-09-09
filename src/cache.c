/* gpx-import/src/cache.c
 *
 * Rails cache expiry
 *
 * Copyright 2010 Tom Hughes <tom@compton.nu>
 *
 * Written for OpenStreetMap
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2 of the License
 * only.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <libmemcached/memcached.h>

#include "cache.h"

static const char *locales[] = {
  "af", "aln", "ar", "arz", "be-TARASK", "be", "bg", "br", "bs",
  "ca", "cs", "da", "de", "dsb", "el", "en", "eo", "es", "et", "eu",
  "fa", "fi", "fr", "fur", "gcf", "gl", "gsw", "he", "hi", "hr", "hsb", "hu",
  "ia", "is", "it", "ja", "ka", "km", "ko", "ksh", "lt", "lv", "mk",
  "nb", "nds", "ne", "nl", "nn", "no", "pl", "ps", "pt-BR", "pt",
  "ro", "ru", "sk", "sl", "sq", "sr-EC", "sv", "te", "tr", "uk",
  "vi", "yi", "yo", "zh-CN", "zh-TW", NULL
};

static memcached_st *ptr;

bool
cache_connect(void)
{
  const char *servers;

  if ((servers = getenv("GPX_MEMCACHE_SERVERS")) != NULL &&
      (ptr = memcached_create(NULL)) != NULL)
  {
    memcached_server_st *server_list;

    memcached_behavior_set(ptr, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, 1);
    memcached_behavior_set(ptr, MEMCACHED_BEHAVIOR_KETAMA_WEIGHTED, 1);
    memcached_behavior_set(ptr, MEMCACHED_BEHAVIOR_HASH, MEMCACHED_HASH_FNV1_32);

    server_list = memcached_servers_parse(servers);

    memcached_server_push(ptr, server_list);

    memcached_server_list_free(server_list);
  }

  return servers == NULL || ptr != NULL;
}

void
cache_disconnect(void)
{
  if (ptr) {
    memcached_free(ptr);
    ptr = NULL;
  }

  return;
}

void
cache_delete(const char *fmt, ...)
{
  if (ptr) {
    va_list va1;
    va_list va2;
    int keylen;
    char *key;

    va_start(va1, fmt);
    va_copy(va2, va1);
    keylen = vsnprintf(NULL, 0, fmt, va2);
    va_end(va2);
    key = malloc(keylen + 18);
    vsnprintf(key, keylen + 1, fmt, va1);
    va_end(va1);

    for (const char **localep = locales; *localep; localep++)
    {
      snprintf(key + keylen, 18, "?locale=%s", *localep);

      DEBUG("Delete cache entry: %s", key);

      memcached_delete(ptr, key, strlen(key), 0);
    }

    free(key);
  }

  return;
}
