/* gpx-import/src/interpolate.c
 *
 * GPX file importer, email interpolation
 *
 * Copyright Daniel Silverstone <dsilvers@digital-scurf.org>
 *
 * Written for the OpenStreetMap project.
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
 *
 * The URL encoding routines (to_hex and url_encode) are public domain
 * and were derived from http://www.geekhideout.com/urlcode.shtml by
 * Graham Jones <grahamjones139@gmail.com>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <inttypes.h>
#include <ctype.h>

#include <errno.h>

#include "interpolate.h"

static char
to_hex(char c)
{
  static const char hex[] = "0123456789abcdef";

  return hex[c & 15];
}

static char *
url_encode(const char *str)
{
   const unsigned char *strptr = (const unsigned char *)str;
  char *buf = malloc(strlen(str) * 3 + 1);
  char *bufptr = buf;

  while (*strptr) {
    unsigned char c = *strptr++;

    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
       *bufptr++ = c;
    } else {
       *bufptr++ = '%';
       *bufptr++ = to_hex(c >> 4);
       *bufptr++ = to_hex(c & 15);
    }
  }

  *bufptr = '\0';

  return buf;
}

static void
do_interpolate(DBJob *job, FILE *input, FILE *output)
{
  int c;
  char *usrEncStr;

  while ((c = fgetc(input)) != EOF) {
    if (c != '%') {
      fputc(c, output);
      continue;
    }
    c = fgetc(input);
    switch (c) {
    case -1:
    case '%':
      fputc('%', output);
      break;
    case 'e':
      fputs(job->email, output);
      break;
    case 'E':
      fputs(job->error, output);
      break;
    case 't':
      fputs(job->title, output);
      break;
    case 'd':
      fputs(job->description, output);
      break;
    case 'g':
      fprintf(output, "%d", job->gpx->goodpoints);
      break;
    case 'p':
      fprintf(output, "%d", job->gpx->goodpoints + job->gpx->badpoints);
      break;
    case 'T':
      if (job->tags != NULL) {
        fputs("and the following tags:\n", output);
        for (const DBTag *tag = job->tags; tag; tag = tag->next ) {
          fprintf(output, "\n  %s", tag->name);
        }
      } else {
        fputs("and no tags.", output);
      }
      break;
    case 'm':
      if (job->gpx->missed_time > 0) {
        fprintf(output, "Of the failed points, %d lacked <time>", job->gpx->missed_time);
      }
      break;
    case 'l':
      if (job->gpx->bad_lat > 0) {
        fprintf(output, "Of the failed points, %d had bad latitude", job->gpx->bad_lat);
      }
      break;
    case 'L':
      if (job->gpx->bad_long > 0) {
        fprintf(output, "Of the failed points, %d had bad longitude", job->gpx->bad_long);
      }
      break;
    case 'u':
      usrEncStr = url_encode(job->name);
      fprintf(output, "http://www.openstreetmap.org/user/%s/traces/%"PRId64, usrEncStr, job->gpx_id);
      free(usrEncStr);
      break;
    default:
      fputs("\n\n[Unknown % escape: ", output);
      fputc(c, output);
      fputs("]\n\n", output);
    }
  }
}

void
interpolate(DBJob *job, const char *template)
{
  FILE *outputfile, *inputfile;
  char inputpath[PATH_MAX];

  if (getenv("GPX_INTERPOLATE_STDOUT") != NULL) {
    outputfile = stdout;
  } else {
    const char *sender;
    char command[256];
    if ((sender = getenv("GPX_MAIL_SENDER")) == NULL) {
       sender = "";
    }
    snprintf(command, sizeof(command), "/usr/lib/sendmail -t -r '<%s>'", sender);
    outputfile = popen(command, "w");
    if (outputfile == NULL) {
      ERROR("Unable to open sendmail! (errno=%s)", strerror(errno));
      return;
    }
  }

  snprintf(inputpath, PATH_MAX, "%s/%s", getenv("GPX_PATH_TEMPLATES"), template);

  inputfile = fopen(inputpath, "rb");

  if (inputfile == NULL) {
    ERROR("Unable to open input file %s (errno=%s)", inputpath, strerror(errno));
  } else {
    do_interpolate(job, inputfile, outputfile);
    fclose(inputfile);
  }

  if (outputfile != stdout) {
    if (pclose(outputfile) == -1) {
      ERROR("Failure while closing sendmail! (errno=%s)", strerror(errno));
    }
  }
}
