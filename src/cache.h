/* gpx-import/src/cache.h
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

#ifndef GPX_IMPORT_CACHE_H
#define GPX_IMPORT_CACHE_H

#include <stdbool.h>

extern bool cache_connect(void);
extern void cache_disconnect(void);
extern void cache_delete(const char *fmt, ...);

#endif /* GPX_IMPORT_CACHE_H */
