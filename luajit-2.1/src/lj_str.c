/*
** String handling.
** Copyright (C) 2005-2015 Mike Pall. See Copyright Notice in luajit.h
*/

#define lj_str_c
#define LUA_CORE

#include "lj_obj.h"
#include "lj_gc.h"
#include "lj_err.h"
#include "lj_str.h"
#include "lj_char.h"

/* -- Hashing ------------------------------------------------------------- */

/* https://github.com/amadvance/tommyds */

/* Copyright (c) 2010, Andrea Mazzoleni. All rights reserved. */

/*   Redistribution and use in source and binary forms, with or without */
/*   modification, are permitted provided that the following conditions */
/*   are met: */

/*   1. Redistributions of source code must retain the above copyright */
/*   notice, this list of conditions and the following disclaimer. */

/*   2. Redistributions in binary form must reproduce the above copyright */
/*   notice, this list of conditions and the following disclaimer in the */
/*   documentation and/or other materials provided with the distribution. */

/*   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" */
/*   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE */
/*   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE */
/*   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE */
/*   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR */
/*   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF */
/*   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS */
/*   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN */
/*   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) */
/*   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE */
/*   POSSIBILITY OF SUCH DAMAGE. */

#define tommy_cast(type, value) (value)

#define tommy_rot(x, k)                         \
	(((x) << (k)) | ((x) >> (32 - (k))))

#define tommy_mix(a, b, c)                   \
	do {                                       \
		a -= c;  a ^= tommy_rot(c, 4);  c += b;  \
		b -= a;  b ^= tommy_rot(a, 6);  a += c;  \
		c -= b;  c ^= tommy_rot(b, 8);  b += a;  \
		a -= c;  a ^= tommy_rot(c, 16);  c += b; \
		b -= a;  b ^= tommy_rot(a, 19);  a += c; \
		c -= b;  c ^= tommy_rot(b, 4);  b += a;  \
	} while (0)

#define tommy_final(a, b, c)       \
	do {                             \
		c ^= b; c -= tommy_rot(b, 14); \
		a ^= c; a -= tommy_rot(c, 11); \
		b ^= a; b -= tommy_rot(a, 25); \
		c ^= b; c -= tommy_rot(b, 16); \
		a ^= c; a -= tommy_rot(c, 4);  \
		b ^= a; b -= tommy_rot(a, 14); \
		c ^= b; c -= tommy_rot(b, 24); \
	} while (0)

LJ_AINLINE MSize tommy_le_uint32_read(const void* ptr)
{
	/* allow unaligned read on Intel x86 and x86_64 platforms */
#if defined(__i386__) || defined(_M_IX86) || defined(_X86_) || defined(__x86_64__) || defined(_M_X64)
	/* defines from http://predef.sourceforge.net/ */
	return *(const MSize*)ptr;
#else
	const unsigned char* ptr8 = tommy_cast(const unsigned char*, ptr);
	return ptr8[0] + ((MSize)ptr8[1] << 8) + ((MSize)ptr8[2] << 16) + ((MSize)ptr8[3] << 24);
#endif
}

MSize tommy_hash_u32(MSize init_val, const void* void_key, size_t key_len)
{
	const unsigned char* key = tommy_cast(const unsigned char*, void_key);
	MSize a, b, c;

	a = b = c = 0xdeadbeef + ((MSize)key_len) + init_val;

	while (key_len > 12) {
		a += tommy_le_uint32_read(key + 0);
		b += tommy_le_uint32_read(key + 4);
		c += tommy_le_uint32_read(key + 8);

		tommy_mix(a, b, c);

		key_len -= 12;
		key += 12;
	}

	switch (key_len) {
    case 0 :
      return c; /* used only when called with a zero length */
    case 12 :
      c += tommy_le_uint32_read(key + 8);
      b += tommy_le_uint32_read(key + 4);
      a += tommy_le_uint32_read(key + 0);
      break;
    case 11 : c += ((MSize)key[10]) << 16;
    case 10 : c += ((MSize)key[9]) << 8;
    case 9 : c += key[8];
    case 8 :
      b += tommy_le_uint32_read(key + 4);
      a += tommy_le_uint32_read(key + 0);
      break;
    case 7 : b += ((MSize)key[6]) << 16;
    case 6 : b += ((MSize)key[5]) << 8;
    case 5 : b += key[4];
    case 4 :
      a += tommy_le_uint32_read(key + 0);
      break;
    case 3 : a += ((MSize)key[2]) << 16;
    case 2 : a += ((MSize)key[1]) << 8;
    case 1 : a += key[0];
	}

	tommy_final(a, b, c);

	return c;
}

/* -- String helpers ------------------------------------------------------ */

/* Ordered compare of strings. Assumes string data is 4-byte aligned. */
int32_t LJ_FASTCALL lj_str_cmp(GCstr *a, GCstr *b)
{
  MSize i, n = a->len > b->len ? b->len : a->len;
  for (i = 0; i < n; i += 4) {
    /* Note: innocuous access up to end of string + 3. */
    uint32_t va = *(const uint32_t *)(strdata(a)+i);
    uint32_t vb = *(const uint32_t *)(strdata(b)+i);
    if (va != vb) {
#if LJ_LE
      va = lj_bswap(va); vb = lj_bswap(vb);
#endif
      i -= n;
      if ((int32_t)i >= -3) {
	va >>= 32+(i<<3); vb >>= 32+(i<<3);
	if (va == vb) break;
      }
      return va < vb ? -1 : 1;
    }
  }
  return (int32_t)(a->len - b->len);
}

/* Fast string data comparison. Caveat: unaligned access to 1st string! */
static LJ_AINLINE int str_fastcmp(const char *a, const char *b, MSize len)
{
  MSize i = 0;
  lua_assert(len > 0);
  lua_assert((((uintptr_t)a+len-1) & (LJ_PAGESIZE-1)) <= LJ_PAGESIZE-4);
  do {  /* Note: innocuous access up to end of string + 3. */
    uint32_t v = lj_getu32(a+i) ^ *(const uint32_t *)(b+i);
    if (v) {
      i -= len;
#if LJ_LE
      return (int32_t)i >= -3 ? (v << (32+(i<<3))) : 1;
#else
      return (int32_t)i >= -3 ? (v >> (32+(i<<3))) : 1;
#endif
    }
    i += 4;
  } while (i < len);
  return 0;
}

/* Find fixed string p inside string s. */
const char *lj_str_find(const char *s, const char *p, MSize slen, MSize plen)
{
  if (plen <= slen) {
    if (plen == 0) {
      return s;
    } else {
      int c = *(const uint8_t *)p++;
      plen--; slen -= plen;
      while (slen) {
	const char *q = (const char *)memchr(s, c, slen);
	if (!q) break;
	if (memcmp(q+1, p, plen) == 0) return q;
	q++; slen -= (MSize)(q-s); s = q;
      }
    }
  }
  return NULL;
}

/* Check whether a string has a pattern matching character. */
int lj_str_haspattern(GCstr *s)
{
  const char *p = strdata(s), *q = p + s->len;
  while (p < q) {
    int c = *(const uint8_t *)p++;
    if (lj_char_ispunct(c) && strchr("^$*+?.([%-", c))
      return 1;  /* Found a pattern matching char. */
  }
  return 0;  /* No pattern matching chars found. */
}

/* -- String interning ---------------------------------------------------- */

/* Resize the string hash table (grow and shrink). */
void lj_str_resize(lua_State *L, MSize newmask)
{
  global_State *g = G(L);
  GCRef *newhash;
  MSize i;
  if (g->gc.state == GCSsweepstring || newmask >= LJ_MAX_STRTAB-1)
    return;  /* No resizing during GC traversal or if already too big. */
  newhash = lj_mem_newvec(L, newmask+1, GCRef);
  memset(newhash, 0, (newmask+1)*sizeof(GCRef));
  for (i = g->strmask; i != ~(MSize)0; i--) {  /* Rehash old table. */
    GCobj *p = gcref(g->strhash[i]);
    while (p) {  /* Follow each hash chain and reinsert all strings. */
      MSize h = gco2str(p)->hash & newmask;
      GCobj *next = gcnext(p);
      /* NOBARRIER: The string table is a GC root. */
      setgcrefr(p->gch.nextgc, newhash[h]);
      setgcref(newhash[h], p);
      p = next;
    }
  }
  lj_mem_freevec(g, g->strhash, g->strmask+1, GCRef);
  g->strmask = newmask;
  g->strhash = newhash;
}

/* Intern a string and return string object. */
GCstr *lj_str_new(lua_State *L, const char *str, size_t lenx)
{
  global_State *g;
  GCstr *s;
  GCobj *o;
  MSize len = (MSize)lenx;
  MSize h = len;
  if (lenx >= LJ_MAX_STR)
    lj_err_msg(L, LJ_ERR_STROV);
  g = G(L);
  if (len > 0) {
    h = tommy_hash_u32(0, str, len);
  } else {
    return &g->strempty;
  }
  /* Check if the string has already been interned. */
  o = gcref(g->strhash[h & g->strmask]);
  if (LJ_LIKELY((((uintptr_t)str+len-1) & (LJ_PAGESIZE-1)) <= LJ_PAGESIZE-4)) {
    while (o != NULL) {
      GCstr *sx = gco2str(o);
      if (sx->len == len && str_fastcmp(str, strdata(sx), len) == 0) {
	/* Resurrect if dead. Can only happen with fixstring() (keywords). */
	if (isdead(g, o)) flipwhite(o);
	return sx;  /* Return existing string. */
      }
      o = gcnext(o);
    }
  } else {  /* Slow path: end of string is too close to a page boundary. */
    while (o != NULL) {
      GCstr *sx = gco2str(o);
      if (sx->len == len && memcmp(str, strdata(sx), len) == 0) {
	/* Resurrect if dead. Can only happen with fixstring() (keywords). */
	if (isdead(g, o)) flipwhite(o);
	return sx;  /* Return existing string. */
      }
      o = gcnext(o);
    }
  }
  /* Nope, create a new string. */
  s = lj_mem_newt(L, sizeof(GCstr)+len+1, GCstr);
  newwhite(g, s);
  s->gct = ~LJ_TSTR;
  s->len = len;
  s->hash = h;
  s->reserved = 0;
  memcpy(strdatawr(s), str, len);
  strdatawr(s)[len] = '\0';  /* Zero-terminate string. */
  /* Add it to string hash table. */
  h &= g->strmask;
  s->nextgc = g->strhash[h];
  /* NOBARRIER: The string table is a GC root. */
  setgcref(g->strhash[h], obj2gco(s));
  if (g->strnum++ > g->strmask)  /* Allow a 100% load factor. */
    lj_str_resize(L, (g->strmask<<1)+1);  /* Grow string table. */
  return s;  /* Return newly interned string. */
}

void LJ_FASTCALL lj_str_free(global_State *g, GCstr *s)
{
  g->strnum--;
  lj_mem_free(g, s, sizestring(s));
}

