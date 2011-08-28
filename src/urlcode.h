/* 
 * urlcode.h
 * Header file for functions to encode and decode URLs in urlcode.c
 *
 * Graham Jones, August 2011.
 *
 */
#ifndef URLCODE_H
#define URLCODE_H

char from_hex(char ch);
char tohex(char code);
char *url_encode(char *str);
char *url_decode(char *str);


#endif /* URLCODE_H */
