/* text.c - Text manipulation functions
 *	Copyright (c) 1995-1997 Stefan Jokisch
 *
 * This file is part of Frotz.
 *
 * Frotz is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Frotz is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "frotz.h"

enum string_type {
    LOW_STRING, ABBREVIATION, HIGH_STRING, EMBEDDED_STRING, VOCABULARY
};

extern zword object_name (zword);
extern zword get_window_font (zword);

static zchar decoded[10];
static zword encoded[3];

/* 
 * According to Matteo De Luigi <matteo.de.luigi@libero.it>, 
 * 0xab and 0xbb were in each other's proper positions.
 *   Sat Apr 21, 2001
 */
static zword zscii_to_latin1[] = {
    0x0e4, 0x0f6, 0x0fc, 0x0c4, 0x0d6, 0x0dc, 0x0df, 0x0bb,
    0x0ab, 0x0eb, 0x0ef, 0x0ff, 0x0cb, 0x0cf, 0x0e1, 0x0e9,
    0x0ed, 0x0f3, 0x0fa, 0x0fd, 0x0c1, 0x0c9, 0x0cd, 0x0d3,
    0x0da, 0x0dd, 0x0e0, 0x0e8, 0x0ec, 0x0f2, 0x0f9, 0x0c0,
    0x0c8, 0x0cc, 0x0d2, 0x0d9, 0x0e2, 0x0ea, 0x0ee, 0x0f4,
    0x0fb, 0x0c2, 0x0ca, 0x0ce, 0x0d4, 0x0db, 0x0e5, 0x0c5,
    0x0f8, 0x0d8, 0x0e3, 0x0f1, 0x0f5, 0x0c3, 0x0d1, 0x0d5,
    0x0e6, 0x0c6, 0x0e7, 0x0c7, 0x0fe, 0x0f0, 0x0de, 0x0d0,
    0x0a3, 0x153, 0x152, 0x0a1, 0x0bf
};

/*
 * translate_from_zscii
 *
 * Map a ZSCII character into Unicode.
 *
 */

zchar translate_from_zscii (zbyte c)
{

    if (c == 0xfc)
	return ZC_MENU_CLICK;
    if (c == 0xfd)
	return ZC_DOUBLE_CLICK;
    if (c == 0xfe)
	return ZC_SINGLE_CLICK;

    if (c >= 0x9b && story_id != BEYOND_ZORK) {

	if (hx_unicode_table != 0) {	/* game has its own Unicode table */

	    zbyte N;

	    LOW_BYTE (hx_unicode_table, N)

	    if (c - 0x9b < N) {

		zword addr = hx_unicode_table + 1 + 2 * (c - 0x9b);
		zword unicode;

		LOW_WORD (addr, unicode)

		if (unicode < 0x20)
			return '?';

		return unicode;

	    } else return '?';

	} else				/* game uses standard set */

	    if (c <= 0xdf) {

		return zscii_to_latin1[c - 0x9b];

	    } else return '?';
    }

    return c;

}/* translate_from_zscii */

/*
 * unicode_to_zscii
 *
 * Convert a Unicode character to ZSCII, returning 0 on failure.
 *
 */

zbyte unicode_to_zscii (zchar c)
{
    int i;

    if (c >= ZC_LATIN1_MIN) {

	if (hx_unicode_table != 0) {	/* game has its own Unicode table */

	    zbyte N;
	    int i;

	    LOW_BYTE (hx_unicode_table, N)

	    for (i = 0x9b; i < 0x9b + N; i++) {

		zword addr = hx_unicode_table + 1 + 2 * (i - 0x9b);
		zword unicode;

		LOW_WORD (addr, unicode)

		if (c == unicode)
		    return (zbyte) i;

	    }

	    return 0;

	} else {			/* game uses standard set */

	    for (i = 0x9b; i <= 0xdf; i++)
		if (c == zscii_to_latin1[i - 0x9b])
		    return (zbyte) i;

	    return 0;

	}
    }

    return (zbyte) c;

}/* unicode_to_zscii */

/*
 * translate_to_zscii
 *
 * Map a Unicode character onto the ZSCII alphabet.
 *
 */

zbyte translate_to_zscii (zchar c)
{

    if (c == ZC_SINGLE_CLICK)
	return 0xfe;
    if (c == ZC_DOUBLE_CLICK)
	return 0xfd;
    if (c == ZC_MENU_CLICK)
	return 0xfc;
    if (c == 0)
	return 0;

    c = unicode_to_zscii (c);
    if (c == 0)
	c = '?';

    return (zbyte) c;

}/* translate_to_zscii */

/*
 * alphabet
 *
 * Return a character from one of the three character sets.
 *
 */

static zchar alphabet (int set, int index)
{
    if (h_version > V1 && set == 2 && index == 1)
	return 0x0D;		/* always newline */

    if (h_alphabet != 0) {	/* game uses its own alphabet */

	zbyte c;

	zword addr = h_alphabet + 26 * set + index;
	LOW_BYTE (addr, c)

	return translate_from_zscii (c);

    } else			/* game uses default alphabet */

	if (set == 0)
	    return 'a' + index;
	else if (set == 1)
	    return 'A' + index;
	else if (h_version == V1)
	    return " 0123456789.,!?_#'\"/\\<-:()"[index];
	else
	    return " ^0123456789.,!?_#'\"/\\-:()"[index];

}/* alphabet */

/*
 * load_string
 *
 * Copy a ZSCII string from the memory to the global "decoded" string.
 *
 */

static void load_string (zword addr, zword length)
{
    int resolution = (h_version <= V3) ? 2 : 3;
    int i = 0;

    while (i < 3 * resolution)

	if (i < length) {

	    zbyte c;

	    LOW_BYTE (addr, c)
	    addr++;

	    decoded[i++] = translate_from_zscii (c);

	} else decoded[i++] = 0;

}/* load_string */

/*
 * encode_text
 *
 * Encode the Unicode text in the global "decoded" string then write
 * the result to the global "encoded" array. (This is used to look up
 * words in the dictionary.) Up to V3 the vocabulary resolution is
 * two, since V4 it is three words.
 * Because each word contains three Z-characters, that makes six or
 * nine Z-characters respectively. Longer words are chopped to the
 * proper size, shorter words are are padded out with 5's. For word
 * completion we pad with 0s and 31s, the minimum and maximum
 * Z-characters.
 *
 */

static void encode_text (int padding)
{
    static zchar again[] = { 'a', 'g', 'a', 'i', 'n', 0, 0, 0, 0 };
    static zchar examine[] = { 'e', 'x', 'a', 'm', 'i', 'n', 'e', 0, 0 };
    static zchar wait[] = { 'w', 'a', 'i', 't', 0, 0, 0, 0, 0 };

    zbyte zchars[12];
    const zchar *ptr = decoded;
    zchar c;
    int resolution = (h_version <= V3) ? 2 : 3;
    int i = 0;

    /* Expand abbreviations that some old Infocom games lack */

    if (option_expand_abbreviations)

	if (padding == 0x05 && decoded[1] == 0)

	    switch (decoded[0]) {
		case 'g': ptr = again; break;
		case 'x': ptr = examine; break;
		case 'z': ptr = wait; break;
	    }

    /* Translate string to a sequence of Z-characters */

    while (i < 3 * resolution)

	if ((c = *ptr++) != 0) {

	    int index, set;
	    zbyte c2;

	    if (c == 32) {

		zchars[i++] = 0;

		continue;

	    }

	    /* Search character in the alphabet */

	    for (set = 0; set < 3; set++)
		for (index = 0; index < 26; index++)
		    if (c == alphabet (set, index))
			goto letter_found;

	    /* Character not found, store its ZSCII value */

	    c2 = translate_to_zscii (c);

	    zchars[i++] = 5;
	    zchars[i++] = 6;
	    zchars[i++] = c2 >> 5;
	    zchars[i++] = c2 & 0x1f;

	    continue;

	letter_found:

	    /* Character found, store its index */

	    if (set != 0)
		zchars[i++] = ((h_version <= V2) ? 1 : 3) + set;

	    zchars[i++] = index + 6;

	} else zchars[i++] = padding;

    /* Three Z-characters make a 16bit word */

    for (i = 0; i < resolution; i++)

	encoded[i] =
	    (zchars[3 * i + 0] << 10) |
	    (zchars[3 * i + 1] << 5) |
	    (zchars[3 * i + 2]);

    encoded[resolution - 1] |= 0x8000;

}/* encode_text */

/*
 * z_check_unicode, test if a unicode character can be printed (bit 0) and read (bit 1).
 *
 * 	zargs[0] = Unicode
 *
 */

void z_check_unicode (void)
{
    zword c = zargs[0];
    zword result = 0;

    if (c <= 0x1f)
    {
	if ((c == 0x08) || (c == 0x0d) || (c == 0x1b))
	    result = 2;
    }
    else if (c <= 0x7e)
	result = 3;
    else
	result = os_check_unicode (get_window_font(cwin), c);

    store (result);

}/* z_check_unicode */

/*
 * z_encode_text, encode a ZSCII string for use in a dictionary.
 *
 *	zargs[0] = address of text buffer
 *	zargs[1] = length of ASCII string
 *	zargs[2] = offset of ASCII string within the text buffer
 *	zargs[3] = address to store encoded text in
 *
 * This is a V5+ opcode and therefore the dictionary resolution must be
 * three 16bit words.
 *
 */

void z_encode_text (void)
{
    int i;

    load_string ((zword) (zargs[0] + zargs[2]), zargs[1]);

    encode_text (0x05);

    for (i = 0; i < 3; i++)
	storew ((zword) (zargs[3] + 2 * i), encoded[i]);

}/* z_encode_text */

/*
 * decode_text
 *
 * Convert encoded text to Unicode. The encoded text consists of 16bit
 * words. Every word holds 3 Z-characters (5 bits each) plus a spare
 * bit to mark the last word. The Z-characters translate to ZSCII by
 * looking at the current current character set. Some select another
 * character set, others refer to abbreviations.
 *
 * There are several different string types:
 *
 *    LOW_STRING - from the lower 64KB (byte address)
 *    ABBREVIATION - from the abbreviations table (word address)
 *    HIGH_STRING - from the end of the memory map (packed address)
 *    EMBEDDED_STRING - from the instruction stream (at PC)
 *    VOCABULARY - from the dictionary (byte address)
 *
 * The last type is only used for word completion.
 *
 */

#define outchar(c)	if (st==VOCABULARY) *ptr++=c; else print_char(c)

static void decode_text (enum string_type st, zword addr)
{
    zchar *ptr;
    long byte_addr;
    zchar c2;
    zword code;
    zbyte c, prev_c = 0;
    int shift_state = 0;
    int shift_lock = 0;
    int status = 0;

    ptr = NULL;		/* makes compilers shut up */
    byte_addr = 0;

    /* Calculate the byte address if necessary */

    if (st == ABBREVIATION)

	byte_addr = (long) addr << 1;

    else if (st == HIGH_STRING) {

	if (h_version <= V3)
	    byte_addr = (long) addr << 1;
	else if (h_version <= V5)
	    byte_addr = (long) addr << 2;
	else if (h_version <= V7)
	    byte_addr = ((long) addr << 2) + ((long) h_strings_offset << 3);
	else /* (h_version == V8) */
	    byte_addr = (long) addr << 3;

	if (byte_addr >= story_size)
	    runtime_error (ERR_ILL_PRINT_ADDR);

    }

    /* Loop until a 16bit word has the highest bit set */

    if (st == VOCABULARY)
	ptr = decoded;

    do {

	int i;

	/* Fetch the next 16bit word */

	if (st == LOW_STRING || st == VOCABULARY) {
	    LOW_WORD (addr, code)
	    addr += 2;
	} else if (st == HIGH_STRING || st == ABBREVIATION) {
	    HIGH_WORD (byte_addr, code)
	    byte_addr += 2;
	} else
	    CODE_WORD (code)

	/* Read its three Z-characters */

	for (i = 10; i >= 0; i -= 5) {

	    zword abbr_addr;
	    zword ptr_addr;

	    c = (code >> i) & 0x1f;

	    switch (status) {

	    case 0:	/* normal operation */

		if (shift_state == 2 && c == 6)
		    status = 2;

		else if (h_version == V1 && c == 1)
		    new_line ();

		else if (h_version >= V2 && shift_state == 2 && c == 7)
		    new_line ();

		else if (c >= 6)
		    outchar (alphabet (shift_state, c - 6));

		else if (c == 0)
		    outchar (' ');

		else if (h_version >= V2 && c == 1)
		    status = 1;

		else if (h_version >= V3 && c <= 3)
		    status = 1;

		else {

		    shift_state = (shift_lock + (c & 1) + 1) % 3;

		    if (h_version <= V2 && c >= 4)
			shift_lock = shift_state;

		    break;

		}

		shift_state = shift_lock;

		break;

	    case 1:	/* abbreviation */

		ptr_addr = h_abbreviations + 64 * (prev_c - 1) + 2 * c;

		LOW_WORD (ptr_addr, abbr_addr)
		decode_text (ABBREVIATION, abbr_addr);

		status = 0;
		break;

	    case 2:	/* ZSCII character - first part */

		status = 3;
		break;

	    case 3:	/* ZSCII character - second part */

		c2 = translate_from_zscii ((prev_c << 5) | c);
		outchar (c2);

		status = 0;
		break;

	    }

	    prev_c = c;

	}

    } while (!(code & 0x8000));

    if (st == VOCABULARY)
	*ptr = 0;

}/* decode_text */

#undef outchar

/*
 * z_new_line, print a new line.
 *
 * 	no zargs used
 *
 */

void z_new_line (void)
{

    new_line ();

}/* z_new_line */

/*
 * z_print, print a string embedded in the instruction stream.
 *
 *	no zargs used
 *
 */

void z_print (void)
{

    decode_text (EMBEDDED_STRING, 0);

}/* z_print */

/*
 * z_print_addr, print a string from the lower 64KB.
 *
 *	zargs[0] = address of string to print
 *
 */

void z_print_addr (void)
{

    decode_text (LOW_STRING, zargs[0]);

}/* z_print_addr */

/*
 * z_print_char print a single ZSCII character.
 *
 *	zargs[0] = ZSCII character to be printed
 *
 */

void z_print_char (void)
{

    print_char (translate_from_zscii (zargs[0]));

}/* z_print_char */

/*
 * z_print_form, print a formatted table.
 *
 *	zargs[0] = address of formatted table to be printed
 *
 */

void z_print_form (void)
{
    zword count;
    zword addr = zargs[0];

    bool first = TRUE;

    for (;;) {

	LOW_WORD (addr, count)
	addr += 2;

	if (count == 0)
	    break;

	if (!first)
	    new_line ();

	while (count--) {

	    zbyte c;

	    LOW_BYTE (addr, c)
	    addr++;

	    print_char (translate_from_zscii (c));

	}

	first = FALSE;

    }

}/* z_print_form */

/*
 * print_num
 *
 * Print a signed 16bit number.
 *
 */

void print_num (zword value)
{
    int i;

    /* Print sign */

    if ((short) value < 0) {
	print_char ('-');
	value = - (short) value;
    }

    /* Print absolute value */

    for (i = 10000; i != 0; i /= 10)
	if (value >= i || i == 1)
	    print_char ('0' + (value / i) % 10);

}/* print_num */

/*
 * z_print_num, print a signed number.
 *
 * 	zargs[0] = number to print
 *
 */

void z_print_num (void)
{

    print_num (zargs[0]);

}/* z_print_num */

/*
 * print_object
 *
 * Print an object description.
 *
 */

void print_object (zword object)
{
    zword addr = object_name (object);
    zword code = 0x94a5;
    zbyte length;

    LOW_BYTE (addr, length)
    addr++;

    if (length != 0)
	LOW_WORD (addr, code)

    if (code == 0x94a5) { 	/* encoded text 0x94a5 == empty string */

	print_string ("object#");	/* supply a generic name */
	print_num (object);		/* for anonymous objects */

    } else decode_text (LOW_STRING, addr);

}/* print_object */

/*
 * z_print_obj, print an object description.
 *
 * 	zargs[0] = number of object to be printed
 *
 */

void z_print_obj (void)
{

    print_object (zargs[0]);

}/* z_print_obj */

/*
 * z_print_paddr, print the string at the given packed address.
 *
 * 	zargs[0] = packed address of string to be printed
 *
 */

void z_print_paddr (void)
{

    decode_text (HIGH_STRING, zargs[0]);

}/* z_print_paddr */

/*
 * z_print_ret, print the string at PC, print newline then return true.
 *
 * 	no zargs used
 *
 */

void z_print_ret (void)
{

    decode_text (EMBEDDED_STRING, 0);
    new_line ();
    ret (1);

}/* z_print_ret */

/*
 * print_string
 *
 * Print a string of ASCII characters.
 *
 */

void print_string (const char *s)
{
    char c;

    while ((c = *s++) != 0)

	if (c == '\n')
	    new_line ();
	else
	    print_char (c);

}/* print_string */

/*
 * z_print_unicode
 *
 * 	zargs[0] = Unicode
 *
 */

void z_print_unicode (void)
{

    if (zargs[0] < 0x20)
	print_char ('?');
    else
	print_char (zargs[0]);

}/* z_print_unicode */

/*
 * lookup_text
 *
 * Scan a dictionary searching for the given word. The first argument
 * can be
 *
 * 0x00 - find the first word which is >= the given one
 * 0x05 - find the word which exactly matches the given one
 * 0x1f - find the last word which is <= the given one
 *
 * The return value is 0 if the search fails.
 *
 */

static zword lookup_text (int padding, zword dct)
{
    zword entry_addr;
    zword entry_count;
    zword entry;
    zword addr;
    zbyte entry_len;
    zbyte sep_count;
    int resolution = (h_version <= V3) ? 2 : 3;
    int entry_number;
    int lower, upper;
    int i;
    bool sorted;

    encode_text (padding);

    LOW_BYTE (dct, sep_count)		/* skip word separators */
    dct += 1 + sep_count;
    LOW_BYTE (dct, entry_len)		/* get length of entries */
    dct += 1;
    LOW_WORD (dct, entry_count)		/* get number of entries */
    dct += 2;

    if ((short) entry_count < 0) {	/* bad luck, entries aren't sorted */

	entry_count = - (short) entry_count;
	sorted = FALSE;

    } else sorted = TRUE;		/* entries are sorted */

    lower = 0;
    upper = entry_count - 1;

    while (lower <= upper) {

	if (sorted)                             /* binary search */
	    entry_number = (lower + upper) / 2;
	else                                    /* linear search */
	    entry_number = lower;

	entry_addr = dct + entry_number * entry_len;

	/* Compare word to dictionary entry */

	addr = entry_addr;

	for (i = 0; i < resolution; i++) {
	    LOW_WORD (addr, entry)
	    if (encoded[i] != entry)
		goto continuing;
	    addr += 2;
	}

	return entry_addr;		/* exact match found, return now */

    continuing:

	if (sorted)				/* binary search */

	    if (encoded[i] > entry)
		lower = entry_number + 1;
	    else
		upper = entry_number - 1;

	else lower++;                           /* linear search */

    }

    /* No exact match has been found */

    if (padding == 0x05)
	return 0;

    entry_number = (padding == 0x00) ? lower : upper;

    if (entry_number == -1 || entry_number == entry_count)
	return 0;

    return dct + entry_number * entry_len;

}/* lookup_text */

/*
 * tokenise_text
 *
 * Translate a single word to a token and append it to the token
 * buffer. Every token consists of the address of the dictionary
 * entry, the length of the word and the offset of the word from
 * the start of the text buffer. Unknown words cause empty slots
 * if the flag is set (such that the text can be scanned several
 * times with different dictionaries); otherwise they are zero.
 *
 */

static void tokenise_text (zword text, zword length, zword from, zword parse, zword dct, bool flag)
{
    zword addr;
    zbyte token_max, token_count;

    LOW_BYTE (parse, token_max)
    parse++;
    LOW_BYTE (parse, token_count)

    if (token_count < token_max) {	/* sufficient space left for token? */

	storeb (parse++, token_count + 1);

	load_string ((zword) (text + from), length);

	addr = lookup_text (0x05, dct);

	if (addr != 0 || !flag) {

	    parse += 4 * token_count;

	    storew ((zword) (parse + 0), addr);
	    storeb ((zword) (parse + 2), length);
	    storeb ((zword) (parse + 3), from);

	}

    }

}/* tokenise_text */

/*
 * tokenise_line
 *
 * Split an input line into words and translate the words to tokens.
 *
 */

void tokenise_line (zword text, zword token, zword dct, bool flag)
{
    zword addr1;
    zword addr2;
    zbyte length;
    zbyte c;

    length = 0;		/* makes compilers shut up */

    /* Use standard dictionary if the given dictionary is zero */

    if (dct == 0)
	dct = h_dictionary;

    /* Remove all tokens before inserting new ones */

    storeb ((zword) (token + 1), 0);

    /* Move the first pointer across the text buffer searching for the
       beginning of a word. If this succeeds, store the position in a
       second pointer. Move the first pointer searching for the end of
       the word. When it is found, "tokenise" the word. Continue until
       the end of the buffer is reached. */

    addr1 = text;
    addr2 = 0;

    if (h_version >= V5) {
	addr1++;
	LOW_BYTE (addr1, length)
    }

    do {

	zword sep_addr;
	zbyte sep_count;
	zbyte separator;

	/* Fetch next ZSCII character */

	addr1++;

	if (h_version >= V5 && addr1 == text + 2 + length)
	    c = 0;
	else
	    LOW_BYTE (addr1, c)

	/* Check for separator */

	sep_addr = dct;

	LOW_BYTE (sep_addr, sep_count)
	sep_addr++;

	do {

	    LOW_BYTE (sep_addr, separator)
	    sep_addr++;

	} while (c != separator && --sep_count != 0);

	/* This could be the start or the end of a word */

	if (sep_count == 0 && c != ' ' && c != 0) {

	    if (addr2 == 0)
		addr2 = addr1;

	} else if (addr2 != 0) {

	    tokenise_text (
		text,
		(zword) (addr1 - addr2),
		(zword) (addr2 - text),
		token, dct, flag );

	    addr2 = 0;

	}

	/* Translate separator (which is a word in its own right) */

	if (sep_count != 0)

	    tokenise_text (
		text,
		(zword) (1),
		(zword) (addr1 - text),
		token, dct, flag );

    } while (c != 0);

}/* tokenise_line */

/*
 * z_tokenise, make a lexical analysis of a ZSCII string.
 *
 *	zargs[0] = address of string to analyze
 *	zargs[1] = address of token buffer
 *	zargs[2] = address of dictionary (optional)
 *	zargs[3] = set when unknown words cause empty slots (optional)
 *
 */

void z_tokenise (void)
{

    /* Supply default arguments */

    if (zargc < 3)
	zargs[2] = 0;
    if (zargc < 4)
	zargs[3] = 0;

    /* Call tokenise_line to do the real work */

    tokenise_line (zargs[0], zargs[1], zargs[2], zargs[3] != 0);

}/* z_tokenise */

/*
 * completion
 *
 * Scan the vocabulary to complete the last word on the input line
 * (similar to "tcsh" under Unix). The return value is
 *
 *    2 ==> completion is impossible
 *    1 ==> completion is ambiguous
 *    0 ==> completion is successful
 *
 * The function also returns a string in its second argument. In case
 * of 2, the string is empty; in case of 1, the string is the longest
 * extension of the last word on the input line that is common to all
 * possible completions (for instance, if the last word on the input
 * is "fo" and its only possible completions are "follow" and "folly"
 * then the string is "ll"); in case of 0, the string is an extension
 * to the last word that results in the only possible completion.
 *
 */

int completion (const zchar *buffer, zchar *result)
{
    zword minaddr;
    zword maxaddr;
    zchar *ptr;
    zchar c;
    int len;
    int i;

    *result = 0;

    /* Copy last word to "decoded" string */

    len = 0;

    while ((c = *buffer++) != 0)

	if (c != ' ') {

	    if (len < 9)
		decoded[len++] = c;

	} else len = 0;

    decoded[len] = 0;

    /* Search the dictionary for first and last possible extensions */

    minaddr = lookup_text (0x00, h_dictionary);
    maxaddr = lookup_text (0x1f, h_dictionary);

    if (minaddr == 0 || maxaddr == 0 || minaddr > maxaddr)
	return 2;

    /* Copy first extension to "result" string */

    decode_text (VOCABULARY, minaddr);

    ptr = result;

    for (i = len; (c = decoded[i]) != 0; i++)
	*ptr++ = c;
    *ptr = 0;

    /* Merge second extension with "result" string */

    decode_text (VOCABULARY, maxaddr);

    for (i = len, ptr = result; (c = decoded[i]) != 0; i++, ptr++)
	if (*ptr != c) break;
    *ptr = 0;

    /* Search was ambiguous or successful */

    return (minaddr == maxaddr) ? 0 : 1;

}/* completion */

/*
 * unicode_tolower
 *
 * Convert a Unicode character to lowercase.
 * Taken from Zip2000 by Kevin Bracey.
 *
 */

zword unicode_tolower (zword c)
{
    const static unsigned char tolower_basic_latin[0x100] = {
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
	0x40,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
	0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x5B,0x5C,0x5D,0x5E,0x5F,
	0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
	0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
	0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
	0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
	0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
	0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
	0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,
	0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xD7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xDF,
	0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,
	0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF
    };
    const static unsigned char tolower_latin_extended_a[0x80] = {
	0x01,0x01,0x03,0x03,0x05,0x05,0x07,0x07,0x09,0x09,0x0B,0x0B,0x0D,0x0D,0x0F,0x0F,
	0x11,0x11,0x13,0x13,0x15,0x15,0x17,0x17,0x19,0x19,0x1B,0x1B,0x1D,0x1D,0x1F,0x1F,
	0x21,0x21,0x23,0x23,0x25,0x25,0x27,0x27,0x29,0x29,0x2B,0x2B,0x2D,0x2D,0x2F,0x2F,
	0x00,0x31,0x33,0x33,0x35,0x35,0x37,0x37,0x38,0x3A,0x3A,0x3C,0x3C,0x3E,0x3E,0x40,
	0x40,0x42,0x42,0x44,0x44,0x46,0x46,0x48,0x48,0x49,0x4B,0x4B,0x4D,0x4D,0x4F,0x4F,
	0x51,0x51,0x53,0x53,0x55,0x55,0x57,0x57,0x59,0x59,0x5B,0x5B,0x5D,0x5D,0x5F,0x5F,
	0x61,0x61,0x63,0x63,0x65,0x65,0x67,0x67,0x69,0x69,0x6B,0x6B,0x6D,0x6D,0x6F,0x6F,
	0x71,0x71,0x73,0x73,0x75,0x75,0x77,0x77,0x00,0x7A,0x7A,0x7C,0x7C,0x7E,0x7E,0x7F
    };
    const static unsigned char tolower_greek[0x50] = {
	0x80,0x81,0x82,0x83,0x84,0x85,0xAC,0x87,0xAD,0xAE,0xAF,0x8B,0xCC,0x8D,0xCD,0xCE,
	0x90,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
	0xC0,0xC1,0xA2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xAC,0xAD,0xAE,0xAF,
	0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
	0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF
    };
    const static unsigned char tolower_cyrillic[0x60] = {
	0x00,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
	0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
	0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
	0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F
    };

    if (c < 0x0100)
	c = tolower_basic_latin[c];
    else if (c == 0x0130)
	c = 0x0069;	/* Capital I with dot -> lower case i */
    else if (c == 0x0178)
	c = 0x00FF;	/* Capital Y diaeresis -> lower case y diaeresis */
    else if (c < 0x0180)
	c = tolower_latin_extended_a[c-0x100] + 0x100;
    else if (c >= 0x380 && c < 0x3D0)
	c = tolower_greek[c-0x380] + 0x300;
    else if (c >= 0x400 && c < 0x460)
	c = tolower_cyrillic[c-0x400] + 0x400;

    return c;
}

