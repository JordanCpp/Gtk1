/* Implementation of the internal dcigettext function.
   Copyright (C) 1995-1999, 2000, 2001 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA.  */

/* Tell glibc's <string.h> to provide a prototype for mempcpy().
   This must come before <config.h> because <config.h> may include
   <features.h>, and once <features.h> has been included, it's too late.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE	1
#endif

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/types.h>

#ifdef __GNUC__
# define alloca __builtin_alloca
# define HAVE_ALLOCA 1
#else
# if defined HAVE_ALLOCA_H || defined _LIBC
#  include <alloca.h>
# else
#  ifdef _AIX
 #pragma alloca
#  else
#   ifndef alloca
char *alloca ();
#   endif
#  endif
# endif
#endif

#include <errno.h>
#ifndef errno
extern int errno;
#endif
#ifndef __set_errno
# define __set_errno(val) errno = (val)
#endif

#include <stddef.h>
#include <stdlib.h>

#include <string.h>
#if !HAVE_STRCHR && !defined _LIBC
# ifndef strchr
#  define strchr index
# endif
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define STRICT
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef STRICT
#endif

#if defined HAVE_UNISTD_H || defined _LIBC
# include <unistd.h>
#endif

#include <locale.h>

#if defined HAVE_SYS_PARAM_H || defined _LIBC
# include <sys/param.h>
#endif

#include "gettextP.h"
#ifdef _LIBC
# include <libintl.h>
#else
# include "libgnuintl.h"
#endif
#include "hash-string.h"

/* Thread safetyness.  */
#ifdef _LIBC
# include <bits/libc-lock.h>
#else
/* Provide dummy implementation if this is outside glibc.  */
# define __libc_lock_define_initialized(CLASS, NAME)
# define __libc_lock_lock(NAME)
# define __libc_lock_unlock(NAME)
# define __libc_rwlock_define_initialized(CLASS, NAME)
# define __libc_rwlock_rdlock(NAME)
# define __libc_rwlock_unlock(NAME)
#endif

/* Alignment of types.  */
#if defined __GNUC__ && __GNUC__ >= 2
# define alignof(TYPE) __alignof__ (TYPE)
#else
# define alignof(TYPE) \
    ((int) &((struct { char dummy1; TYPE dummy2; } *) 0)->dummy2)
#endif

/* The internal variables in the standalone libintl.a must have different
   names than the internal variables in GNU libc, otherwise programs
   using libintl.a cannot be linked statically.  */
#if !defined _LIBC
# define _nl_default_default_domain _nl_default_default_domain__
# define _nl_current_default_domain _nl_current_default_domain__
# define _nl_default_dirname _nl_default_dirname__
# define _nl_domain_bindings _nl_domain_bindings__
#endif

/* Some compilers, like SunOS4 cc, don't have offsetof in <stddef.h>.  */
#ifndef offsetof
# define offsetof(type,ident) ((size_t)&(((type*)0)->ident))
#endif

/* @@ end of prolog @@ */

#ifdef _LIBC
/* Rename the non ANSI C functions.  This is required by the standard
   because some ANSI C functions will require linking with this object
   file and the name space must not be polluted.  */
# define getcwd __getcwd
# ifndef stpcpy
#  define stpcpy __stpcpy
# endif
# define tfind __tfind
#else
# if !defined HAVE_GETCWD
char *getwd ();
#  define getcwd(buf, max) getwd (buf)
# else
char *getcwd ();
# endif
# ifndef HAVE_STPCPY
static char *stpcpy PARAMS ((char *dest, const char *src));
# endif
# ifndef HAVE_MEMPCPY
static void *mempcpy PARAMS ((void *dest, const void *src, size_t n));
# endif
#endif

/* Amount to increase buffer size by in each try.  */
#define PATH_INCR 32

/* The following is from pathmax.h.  */
/* Non-POSIX BSD systems might have gcc's limits.h, which doesn't define
   PATH_MAX but might cause redefinition warnings when sys/param.h is
   later included (as on MORE/BSD 4.3).  */
#if defined _POSIX_VERSION || (defined HAVE_LIMITS_H && !defined __GNUC__)
# include <limits.h>
#endif

#ifndef _POSIX_PATH_MAX
# define _POSIX_PATH_MAX 255
#endif

#if !defined PATH_MAX && defined _PC_PATH_MAX
# define PATH_MAX (pathconf ("/", _PC_PATH_MAX) < 1 ? 1024 : pathconf ("/", _PC_PATH_MAX))
#endif

/* Don't include sys/param.h if it already has been.  */
#if defined HAVE_SYS_PARAM_H && !defined PATH_MAX && !defined MAXPATHLEN
# include <sys/param.h>
#endif

#if !defined PATH_MAX && defined MAXPATHLEN
# define PATH_MAX MAXPATHLEN
#endif

#ifndef PATH_MAX
# define PATH_MAX _POSIX_PATH_MAX
#endif

/* Pathname support.
   ISSLASH(C)           tests whether C is a directory separator character.
   IS_ABSOLUTE_PATH(P)  tests whether P is an absolute path.  If it is not,
                        it may be concatenated to a directory pathname.
   IS_PATH_WITH_DIR(P)  tests whether P contains a directory specification.
 */
#if defined _WIN32 || defined __WIN32__ || defined __EMX__ || defined __DJGPP__
  /* Win32, OS/2, DOS */
# define ISSLASH(C) ((C) == '/' || (C) == '\\')
# define HAS_DEVICE(P) \
    ((((P)[0] >= 'A' && (P)[0] <= 'Z') || ((P)[0] >= 'a' && (P)[0] <= 'z')) \
     && (P)[1] == ':')
# define IS_ABSOLUTE_PATH(P) (ISSLASH ((P)[0]) || HAS_DEVICE (P))
# define IS_PATH_WITH_DIR(P) \
    (strchr (P, '/') != NULL || strchr (P, '\\') != NULL || HAS_DEVICE (P))
#else
  /* Unix */
# define ISSLASH(C) ((C) == '/')
# define IS_ABSOLUTE_PATH(P) ISSLASH ((P)[0])
# define IS_PATH_WITH_DIR(P) (strchr (P, '/') != NULL)
#endif

/* XPG3 defines the result of `setlocale (category, NULL)' as:
   ``Directs `setlocale()' to query `category' and return the current
     setting of `local'.''
   However it does not specify the exact format.  Neither do SUSV2 and
   ISO C 99.  So we can use this feature only on selected systems (e.g.
   those using GNU C Library).  */
#if defined _LIBC || (defined __GNU_LIBRARY__ && __GNU_LIBRARY__ >= 2)
# define HAVE_LOCALE_NULL
#endif

/* This is the type used for the search tree where known translations
   are stored.  */
struct known_translation_t
{
  /* Domain in which to search.  */
  char *domainname;

  /* The category.  */
  int category;

  /* State of the catalog counter at the point the string was found.  */
  int counter;

  /* Catalog where the string was found.  */
  struct loaded_l10nfile *domain;

  /* And finally the translation.  */
  const char *translation;
  size_t translation_length;

  /* Pointer to the string in question.  */
  char msgid[ZERO];
};

/* Root of the search tree with known translations.  We can use this
   only if the system provides the `tsearch' function family.  */
#if defined HAVE_TSEARCH || defined _LIBC
# include <search.h>

static void *root;

# ifdef _LIBC
#  define tsearch __tsearch
# endif

/* Function to compare two entries in the table of known translations.  */
static int transcmp PARAMS ((const void *p1, const void *p2));
static int
transcmp (p1, p2)
     const void *p1;
     const void *p2;
{
  const struct known_translation_t *s1;
  const struct known_translation_t *s2;
  int result;

  s1 = (const struct known_translation_t *) p1;
  s2 = (const struct known_translation_t *) p2;

  result = strcmp (s1->msgid, s2->msgid);
  if (result == 0)
    {
      result = strcmp (s1->domainname, s2->domainname);
      if (result == 0)
	/* We compare the category last (though this is the cheapest
	   operation) since it is hopefully always the same (namely
	   LC_MESSAGES).  */
	result = s1->category - s2->category;
    }

  return result;
}
#endif

/* Name of the default domain used for gettext(3) prior any call to
   textdomain(3).  The default value for this is "messages".  */
const char _nl_default_default_domain[] = "messages";

/* Value used as the default domain for gettext(3).  */
const char *_nl_current_default_domain = _nl_default_default_domain;

/* Contains the default location of the message catalogs.  */
#ifdef _WIN32
/* On Windows we don't want any hardcoded paths */
#undef LOCALEDIR
char _nl_default_dirname[MAX_PATH] = "";
#else
const char _nl_default_dirname[] = LOCALEDIR;
#endif

/* List with bindings of specific domains created by bindtextdomain()
   calls.  */
struct binding *_nl_domain_bindings;

/* Prototypes for local functions.  */
static char *plural_lookup PARAMS ((struct loaded_l10nfile *domain,
				    unsigned long int n,
				    const char *translation,
				    size_t translation_len))
     internal_function;
static unsigned long int plural_eval PARAMS ((struct expression *pexp,
					      unsigned long int n))
     internal_function;
static const char *category_to_name PARAMS ((int category)) internal_function;
static const char *guess_category_value PARAMS ((int category,
						 const char *categoryname))
     internal_function;


/* For those loosing systems which don't have `alloca' we have to add
   some additional code emulating it.  */
#ifdef HAVE_ALLOCA
/* Nothing has to be done.  */
# define ADD_BLOCK(list, address) /* nothing */
# define FREE_BLOCKS(list) /* nothing */
#else
struct block_list
{
  void *address;
  struct block_list *next;
};
# define ADD_BLOCK(list, addr)						      \
  do {									      \
    struct block_list *newp = (struct block_list *) malloc (sizeof (*newp));  \
    /* If we cannot get a free block we cannot add the new element to	      \
       the list.  */							      \
    if (newp != NULL) {							      \
      newp->address = (addr);						      \
      newp->next = (list);						      \
      (list) = newp;							      \
    }									      \
  } while (0)
# define FREE_BLOCKS(list)						      \
  do {									      \
    while (list != NULL) {						      \
      struct block_list *old = list;					      \
      list = list->next;						      \
      free (old);							      \
    }									      \
  } while (0)
# undef alloca
# define alloca(size) (malloc (size))
#endif	/* have alloca */


#ifdef _LIBC
/* List of blocks allocated for translations.  */
typedef struct transmem_list
{
  struct transmem_list *next;
  char data[ZERO];
} transmem_block_t;
static struct transmem_list *transmem_list;
#else
typedef unsigned char transmem_block_t;
#endif


/* Names for the libintl functions are a problem.  They must not clash
   with existing names and they should follow ANSI C.  But this source
   code is also used in GNU C Library where the names have a __
   prefix.  So we have to make a difference here.  */
#ifdef _LIBC
# define DCIGETTEXT __dcigettext
#else
# define DCIGETTEXT dcigettext__
#endif

/* Lock variable to protect the global data in the gettext implementation.  */
#ifdef _LIBC
__libc_rwlock_define_initialized (, _nl_state_lock)
#endif

/* Checking whether the binaries runs SUID must be done and glibc provides
   easier methods therefore we make a difference here.  */
#ifdef _LIBC
# define ENABLE_SECURE __libc_enable_secure
# define DETERMINE_SECURE
#else
# ifndef HAVE_GETUID
#  define getuid() 0
# endif
# ifndef HAVE_GETGID
#  define getgid() 0
# endif
# ifndef HAVE_GETEUID
#  define geteuid() getuid()
# endif
# ifndef HAVE_GETEGID
#  define getegid() getgid()
# endif
static int enable_secure;
# define ENABLE_SECURE (enable_secure == 1)
# define DETERMINE_SECURE \
  if (enable_secure == 0)						      \
    {									      \
      if (getuid () != geteuid () || getgid () != getegid ())		      \
	enable_secure = 1;						      \
      else								      \
	enable_secure = -1;						      \
    }
#endif

#ifdef _WIN32

/* These routines lifted from GLib and simplified */

typedef char gchar;
#define G_DIR_SEPARATOR '\\'
#define G_DIR_SEPARATOR_S "\\"

static gchar *
get_package_directory_from_module (gchar *module_name)
{
  HMODULE hmodule = NULL;
  gchar fn[MAX_PATH];
  gchar *p;

  if (module_name)
    {
      hmodule = GetModuleHandle (module_name);
      if (!hmodule)
	return NULL;
    }

  if (!GetModuleFileName (hmodule, fn, MAX_PATH))
    return NULL;

  if ((p = strrchr (fn, G_DIR_SEPARATOR)) != NULL)
    *p = '\0';

  p = strrchr (fn, G_DIR_SEPARATOR);
  if (p && (stricmp (p + 1, "bin") == 0 ||
	    stricmp (p + 1, "lib") == 0))
    *p = '\0';

  return strdup (fn);
}

/**
 * g_win32_get_package_installation_directory:
 * @package: An identifier for a software package, or NULL
 * @dll_name: The name of a DLL that a package provides, or NULL
 *
 * Try to determine the installation directory for a software package.
 * Typically used by GNU software packages.
 *
 * @package should be a short identifier for the package. Typically it
 * is the same identifier as used for GETTEXT_PACKAGE in software
 * configured accoring to GNU standards. The function first looks in
 * the Windows Registry for the value #InstallationDirectory in the
 * key #HKLM\Software\@package, and if that value exists and is a
 * string, returns that.
 *
 * If @package is NULL, or the above value isn't found in the
 * Registry, but @dll_name is non-NULL, it should name a DLL loaded
 * into the current process. Typically that would be the name of the
 * DLL calling this function, looking for its installation
 * directory. The function then asks Windows what directory that DLL
 * was loaded from. If that directory's last component is "bin" or
 * "lib", the parent directory is returned, otherwise the directory
 * itself. If that DLL isn't loaded, the function proceeds as if
 * @dll_name was NULL.
 *
 * If both @package and @dll_name are NULL, the directory from where
 * the main executable of the process was loaded is uses instead in
 * the same way as above.
 *
 * The return value should be freed with g_free() when not needed any longer.  */

static gchar *
g_win32_get_package_installation_directory (gchar *package,
					    gchar *dll_name)
{
  gchar *result = NULL;
  gchar key[1000];
  HKEY reg_key = NULL;
  DWORD type;
  DWORD nbytes;

  if (package != NULL)
    {
      strcpy (key, "Software\\");
      strcat (key, package);
      
      nbytes = 0;
      if ((RegOpenKeyEx (HKEY_CURRENT_USER, key, 0,
			 KEY_QUERY_VALUE, &reg_key) == ERROR_SUCCESS
	   && RegQueryValueEx (reg_key, "InstallationDirectory", 0,
			       &type, NULL, &nbytes) == ERROR_SUCCESS)
	  ||
	  ((RegOpenKeyEx (HKEY_LOCAL_MACHINE, key, 0,
			 KEY_QUERY_VALUE, &reg_key) == ERROR_SUCCESS
	   && RegQueryValueEx (reg_key, "InstallationDirectory", 0,
			       &type, NULL, &nbytes) == ERROR_SUCCESS)
	   && type == REG_SZ))
	{
	  result = malloc (nbytes + 1);
	  RegQueryValueEx (reg_key, "InstallationDirectory", 0,
			   &type, result, &nbytes);
	  result[nbytes] = '\0';
	}

      if (reg_key != NULL)
	RegCloseKey (reg_key);
    }
  if (result)
    return result;

  if (dll_name != NULL)
    result = get_package_directory_from_module (dll_name);

  if (result == NULL)
    result = get_package_directory_from_module (NULL);

  return result;
}

/**
 * g_win32_get_package_installation_subdirectory:
 * @package: An identifier for a software package, or NULL
 * @dll_name: The name of a DLL that a package provides, or NULL
 * @subdir: A subdirectory of the package installation directory.
 *
 * Returns a string containg the path of the subdirectory @subdir in
 * the return value from calling
 * g_win32_get_package_installation_directory() with the @package and
 * @dll_name parameters. The return value should be freed with
 * g_free() when no longer needed.
 */

static gchar *
g_win32_get_package_installation_subdirectory (gchar *package,
					       gchar *dll_name,
					       gchar *subdir)
{
  gchar *prefix;
  gchar *sep;
  gchar tmp[MAX_PATH];

  prefix = g_win32_get_package_installation_directory (package, dll_name);

  if (subdir == NULL)
    subdir = "";

  sep = (subdir[0] == '\0' ||
	 prefix[strlen (prefix) - 1] == G_DIR_SEPARATOR) ?
    "" : G_DIR_SEPARATOR_S;

  strcpy (tmp, prefix);
  free (prefix);
  strcat (tmp, sep);
  strcat (tmp, subdir);
  return strdup (tmp);
}

static gchar *my_dll_name;

BOOL WINAPI
DllMain (HINSTANCE hinstDLL,
	 DWORD     fdwReason,
	 LPVOID    lpvReserved)
{
  char bfr[MAX_PATH];

  switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
      GetModuleFileName ((HMODULE) hinstDLL, bfr, sizeof (bfr));
      my_dll_name = strdup (strrchr (bfr, '\\') + 1);
      break;
    }

  return TRUE;
}

void
_set_nl_default_dirname (void)
{
  char *dir;

  if (_nl_default_dirname[0])
    return;

  dir = g_win32_get_package_installation_subdirectory
    ("GNU libintl", my_dll_name, "share\\locale");
  strcpy (_nl_default_dirname, dir);
  free (dir);
}
#endif

/* Look up MSGID in the DOMAINNAME message catalog for the current
   CATEGORY locale and, if PLURAL is nonzero, search over string
   depending on the plural form determined by N.  */
char *
DCIGETTEXT (domainname, msgid1, msgid2, plural, n, category)
     const char *domainname;
     const char *msgid1;
     const char *msgid2;
     int plural;
     unsigned long int n;
     int category;
{
#ifndef HAVE_ALLOCA
  struct block_list *block_list = NULL;
#endif
  struct loaded_l10nfile *domain;
  struct binding *binding;
  const char *categoryname;
  const char *categoryvalue;
  char *dirname, *xdomainname;
  char *single_locale;
  char *retval;
  size_t retlen;
  int saved_errno;
#if defined HAVE_TSEARCH || defined _LIBC
  struct known_translation_t *search;
  struct known_translation_t **foundp = NULL;
  size_t msgid_len;
#endif
  size_t domainname_len;

  /* If no real MSGID is given return NULL.  */
  if (msgid1 == NULL)
    return NULL;

  __libc_rwlock_rdlock (_nl_state_lock);

  /* If DOMAINNAME is NULL, we are interested in the default domain.  If
     CATEGORY is not LC_MESSAGES this might not make much sense but the
     definition left this undefined.  */
  if (domainname == NULL)
    domainname = _nl_current_default_domain;

#if defined HAVE_TSEARCH || defined _LIBC
  msgid_len = strlen (msgid1) + 1;

  /* Try to find the translation among those which we found at
     some time.  */
  search = (struct known_translation_t *)
	   alloca (offsetof (struct known_translation_t, msgid) + msgid_len);
  memcpy (search->msgid, msgid1, msgid_len);
  search->domainname = (char *) domainname;
  search->category = category;

  foundp = (struct known_translation_t **) tfind (search, &root, transcmp);
  if (foundp != NULL && (*foundp)->counter == _nl_msg_cat_cntr)
    {
      /* Now deal with plural.  */
      if (plural)
	retval = plural_lookup ((*foundp)->domain, n, (*foundp)->translation,
				(*foundp)->translation_length);
      else
	retval = (char *) (*foundp)->translation;

      __libc_rwlock_unlock (_nl_state_lock);
      return retval;
    }
#endif

  /* Preserve the `errno' value.  */
  saved_errno = errno;

  /* See whether this is a SUID binary or not.  */
  DETERMINE_SECURE;

  /* First find matching binding.  */
  for (binding = _nl_domain_bindings; binding != NULL; binding = binding->next)
    {
      int compare = strcmp (domainname, binding->domainname);
      if (compare == 0)
	/* We found it!  */
	break;
      if (compare < 0)
	{
	  /* It is not in the list.  */
	  binding = NULL;
	  break;
	}
    }

  if (binding == NULL)
    {
#ifdef _WIN32
      _set_nl_default_dirname();
#endif
      dirname = (char *) _nl_default_dirname;
    }
  else if (IS_ABSOLUTE_PATH (binding->dirname))
    dirname = binding->dirname;
  else
    {
      /* We have a relative path.  Make it absolute now.  */
      size_t dirname_len = strlen (binding->dirname) + 1;
      size_t path_max;
      char *ret;

      path_max = (unsigned int) PATH_MAX;
      path_max += 2;		/* The getcwd docs say to do this.  */

      for (;;)
	{
	  dirname = (char *) alloca (path_max + dirname_len);
	  ADD_BLOCK (block_list, dirname);

	  __set_errno (0);
	  ret = getcwd (dirname, path_max);
	  if (ret != NULL || errno != ERANGE)
	    break;

	  path_max += path_max / 2;
	  path_max += PATH_INCR;
	}

      if (ret == NULL)
	{
	  /* We cannot get the current working directory.  Don't signal an
	     error but simply return the default string.  */
	  FREE_BLOCKS (block_list);
	  __libc_rwlock_unlock (_nl_state_lock);
	  __set_errno (saved_errno);
	  return (plural == 0
		  ? (char *) msgid1
		  /* Use the Germanic plural rule.  */
		  : n == 1 ? (char *) msgid1 : (char *) msgid2);
	}

      stpcpy (stpcpy (strchr (dirname, '\0'), "/"), binding->dirname);
    }

  /* Now determine the symbolic name of CATEGORY and its value.  */
  categoryname = category_to_name (category);
  categoryvalue = guess_category_value (category, categoryname);

  domainname_len = strlen (domainname);
  xdomainname = (char *) alloca (strlen (categoryname)
				 + domainname_len + 5);
  ADD_BLOCK (block_list, xdomainname);

  stpcpy (mempcpy (stpcpy (stpcpy (xdomainname, categoryname), "/"),
		  domainname, domainname_len),
	  ".mo");

  /* Creating working area.  */
  single_locale = (char *) alloca (strlen (categoryvalue) + 1);
  ADD_BLOCK (block_list, single_locale);


  /* Search for the given string.  This is a loop because we perhaps
     got an ordered list of languages to consider for the translation.  */
  while (1)
    {
      /* Make CATEGORYVALUE point to the next element of the list.  */
      while (categoryvalue[0] != '\0' && categoryvalue[0] == ':')
	++categoryvalue;
      if (categoryvalue[0] == '\0')
	{
	  /* The whole contents of CATEGORYVALUE has been searched but
	     no valid entry has been found.  We solve this situation
	     by implicitly appending a "C" entry, i.e. no translation
	     will take place.  */
	  single_locale[0] = 'C';
	  single_locale[1] = '\0';
	}
      else
	{
	  char *cp = single_locale;
	  while (categoryvalue[0] != '\0' && categoryvalue[0] != ':')
	    *cp++ = *categoryvalue++;
	  *cp = '\0';

	  /* When this is a SUID binary we must not allow accessing files
	     outside the dedicated directories.  */
	  if (ENABLE_SECURE && IS_PATH_WITH_DIR (single_locale))
	    /* Ingore this entry.  */
	    continue;
	}

      /* If the current locale value is C (or POSIX) we don't load a
	 domain.  Return the MSGID.  */
      if (strcmp (single_locale, "C") == 0
	  || strcmp (single_locale, "POSIX") == 0)
	{
	  FREE_BLOCKS (block_list);
	  __libc_rwlock_unlock (_nl_state_lock);
	  __set_errno (saved_errno);
	  return (plural == 0
		  ? (char *) msgid1
		  /* Use the Germanic plural rule.  */
		  : n == 1 ? (char *) msgid1 : (char *) msgid2);
	}


      /* Find structure describing the message catalog matching the
	 DOMAINNAME and CATEGORY.  */
      domain = _nl_find_domain (dirname, single_locale, xdomainname, binding);

      if (domain != NULL)
	{
	  retval = _nl_find_msg (domain, binding, msgid1, &retlen);

	  if (retval == NULL)
	    {
	      int cnt;

	      for (cnt = 0; domain->successor[cnt] != NULL; ++cnt)
		{
		  retval = _nl_find_msg (domain->successor[cnt], binding,
					 msgid1, &retlen);

		  if (retval != NULL)
		    {
		      domain = domain->successor[cnt];
		      break;
		    }
		}
	    }

	  if (retval != NULL)
	    {
	      /* Found the translation of MSGID1 in domain DOMAIN:
		 starting at RETVAL, RETLEN bytes.  */
	      FREE_BLOCKS (block_list);
	      __set_errno (saved_errno);
#if defined HAVE_TSEARCH || defined _LIBC
	      if (foundp == NULL)
		{
		  /* Create a new entry and add it to the search tree.  */
		  struct known_translation_t *newp;

		  newp = (struct known_translation_t *)
		    malloc (offsetof (struct known_translation_t, msgid)
			    + msgid_len + domainname_len + 1);
		  if (newp != NULL)
		    {
		      newp->domainname =
			mempcpy (newp->msgid, msgid1, msgid_len);
		      memcpy (newp->domainname, domainname, domainname_len + 1);
		      newp->category = category;
		      newp->counter = _nl_msg_cat_cntr;
		      newp->domain = domain;
		      newp->translation = retval;
		      newp->translation_length = retlen;

		      /* Insert the entry in the search tree.  */
		      foundp = (struct known_translation_t **)
			tsearch (newp, &root, transcmp);
		      if (foundp == NULL
			  || __builtin_expect (*foundp != newp, 0))
			/* The insert failed.  */
			free (newp);
		    }
		}
	      else
		{
		  /* We can update the existing entry.  */
		  (*foundp)->counter = _nl_msg_cat_cntr;
		  (*foundp)->domain = domain;
		  (*foundp)->translation = retval;
		  (*foundp)->translation_length = retlen;
		}
#endif
	      /* Now deal with plural.  */
	      if (plural)
		retval = plural_lookup (domain, n, retval, retlen);

	      __libc_rwlock_unlock (_nl_state_lock);
	      return retval;
	    }
	}
    }
  /* NOTREACHED */
}


char *
internal_function
_nl_find_msg (domain_file, domainbinding, msgid, lengthp)
     struct loaded_l10nfile *domain_file;
     struct binding *domainbinding;
     const char *msgid;
     size_t *lengthp;
{
  struct loaded_domain *domain;
  size_t act;
  char *result;
  size_t resultlen;

  if (domain_file->decided == 0)
    _nl_load_domain (domain_file, domainbinding);

  if (domain_file->data == NULL)
    return NULL;

  domain = (struct loaded_domain *) domain_file->data;

  /* Locate the MSGID and its translation.  */
  if (domain->hash_size > 2 && domain->hash_tab != NULL)
    {
      /* Use the hashing table.  */
      nls_uint32 len = strlen (msgid);
      nls_uint32 hash_val = hash_string (msgid);
      nls_uint32 idx = hash_val % domain->hash_size;
      nls_uint32 incr = 1 + (hash_val % (domain->hash_size - 2));

      while (1)
	{
	  nls_uint32 nstr = W (domain->must_swap, domain->hash_tab[idx]);

	  if (nstr == 0)
	    /* Hash table entry is empty.  */
	    return NULL;

	  /* Compare msgid with the original string at index nstr-1.
	     We compare the lengths with >=, not ==, because plural entries
	     are represented by strings with an embedded NUL.  */
	  if (W (domain->must_swap, domain->orig_tab[nstr - 1].length) >= len
	      && (strcmp (msgid,
			  domain->data + W (domain->must_swap,
					    domain->orig_tab[nstr - 1].offset))
		  == 0))
	    {
	      act = nstr - 1;
	      goto found;
	    }

	  if (idx >= domain->hash_size - incr)
	    idx -= domain->hash_size - incr;
	  else
	    idx += incr;
	}
      /* NOTREACHED */
    }
  else
    {
      /* Try the default method:  binary search in the sorted array of
	 messages.  */
      size_t top, bottom;

      bottom = 0;
      top = domain->nstrings;
      while (bottom < top)
	{
	  int cmp_val;

	  act = (bottom + top) / 2;
	  cmp_val = strcmp (msgid, (domain->data
				    + W (domain->must_swap,
					 domain->orig_tab[act].offset)));
	  if (cmp_val < 0)
	    top = act;
	  else if (cmp_val > 0)
	    bottom = act + 1;
	  else
	    goto found;
	}
      /* No translation was found.  */
      return NULL;
    }

 found:
  /* The translation was found at index ACT.  If we have to convert the
     string to use a different character set, this is the time.  */
  result = ((char *) domain->data
	    + W (domain->must_swap, domain->trans_tab[act].offset));
  resultlen = W (domain->must_swap, domain->trans_tab[act].length) + 1;

#if defined _LIBC || HAVE_ICONV
  if (domain->codeset_cntr
      != (domainbinding != NULL ? domainbinding->codeset_cntr : 0))
    {
      /* The domain's codeset has changed through bind_textdomain_codeset()
	 since the message catalog was initialized or last accessed.  We
	 have to reinitialize the converter.  */
      _nl_free_domain_conv (domain);
      _nl_init_domain_conv (domain_file, domain, domainbinding);
    }

  if (
# ifdef _LIBC
      domain->conv != (__gconv_t) -1
# else
#  if HAVE_ICONV
      domain->conv != (iconv_t) -1
#  endif
# endif
      )
    {
      /* We are supposed to do a conversion.  First allocate an
	 appropriate table with the same structure as the table
	 of translations in the file, where we can put the pointers
	 to the converted strings in.
	 There is a slight complication with plural entries.  They
	 are represented by consecutive NUL terminated strings.  We
	 handle this case by converting RESULTLEN bytes, including
	 NULs.  */

      if (domain->conv_tab == NULL
	  && ((domain->conv_tab = (char **) calloc (domain->nstrings,
						    sizeof (char *)))
	      == NULL))
	/* Mark that we didn't succeed allocating a table.  */
	domain->conv_tab = (char **) -1;

      if (__builtin_expect (domain->conv_tab == (char **) -1, 0))
	/* Nothing we can do, no more memory.  */
	goto converted;

      if (domain->conv_tab[act] == NULL)
	{
	  /* We haven't used this string so far, so it is not
	     translated yet.  Do this now.  */
	  /* We use a bit more efficient memory handling.
	     We allocate always larger blocks which get used over
	     time.  This is faster than many small allocations.   */
	  __libc_lock_define_initialized (static, lock)
# define INITIAL_BLOCK_SIZE	4080
	  static unsigned char *freemem;
	  static size_t freemem_size;

	  const unsigned char *inbuf;
	  unsigned char *outbuf;
	  int malloc_count;
# ifndef _LIBC
	  transmem_block_t *transmem_list = NULL;
# endif

	  __libc_lock_lock (lock);

	  inbuf = (const unsigned char *) result;
	  outbuf = freemem + sizeof (size_t);

	  malloc_count = 0;
	  while (1)
	    {
	      transmem_block_t *newmem;
# ifdef _LIBC
	      size_t non_reversible;
	      int res;

	      if (freemem_size < sizeof (size_t))
		goto resize_freemem;

	      res = __gconv (domain->conv,
			     &inbuf, inbuf + resultlen,
			     &outbuf,
			     outbuf + freemem_size - sizeof (size_t),
			     &non_reversible);

	      if (res == __GCONV_OK || res == __GCONV_EMPTY_INPUT)
		break;

	      if (res != __GCONV_FULL_OUTPUT)
		{
		  __libc_lock_unlock (lock);
		  goto converted;
		}

	      inbuf = result;
# else
#  if HAVE_ICONV
	      const char *inptr = (const char *) inbuf;
	      size_t inleft = resultlen;
	      char *outptr = (char *) outbuf;
	      size_t outleft;

	      if (freemem_size < sizeof (size_t))
		goto resize_freemem;

	      outleft = freemem_size - sizeof (size_t);
	      if (iconv (domain->conv,
			 (ICONV_CONST char **) &inptr, &inleft,
			 &outptr, &outleft)
		  != (size_t) (-1))
		{
		  outbuf = (unsigned char *) outptr;
		  break;
		}
	      if (errno != E2BIG)
		{
		  __libc_lock_unlock (lock);
		  goto converted;
		}
#  endif
# endif

	    resize_freemem:
	      /* We must allocate a new buffer or resize the old one.  */
	      if (malloc_count > 0)
		{
		  ++malloc_count;
		  freemem_size = malloc_count * INITIAL_BLOCK_SIZE;
		  newmem = (transmem_block_t *) realloc (transmem_list,
							 freemem_size);
# ifdef _LIBC
		  if (newmem != NULL)
		    transmem_list = transmem_list->next;
		  else
		    {
		      struct transmem_list *old = transmem_list;

		      transmem_list = transmem_list->next;
		      free (old);
		    }
# endif
		}
	      else
		{
		  malloc_count = 1;
		  freemem_size = INITIAL_BLOCK_SIZE;
		  newmem = (transmem_block_t *) malloc (freemem_size);
		}
	      if (__builtin_expect (newmem == NULL, 0))
		{
		  freemem = NULL;
		  freemem_size = 0;
		  __libc_lock_unlock (lock);
		  goto converted;
		}

# ifdef _LIBC
	      /* Add the block to the list of blocks we have to free
                 at some point.  */
	      newmem->next = transmem_list;
	      transmem_list = newmem;

	      freemem = newmem->data;
	      freemem_size -= offsetof (struct transmem_list, data);
# else
	      transmem_list = newmem;
	      freemem = newmem;
# endif

	      outbuf = freemem + sizeof (size_t);
	    }

	  /* We have now in our buffer a converted string.  Put this
	     into the table of conversions.  */
	  *(size_t *) freemem = outbuf - freemem - sizeof (size_t);
	  domain->conv_tab[act] = (char *) freemem;
	  /* Shrink freemem, but keep it aligned.  */
	  freemem_size -= outbuf - freemem;
	  freemem = outbuf;
	  freemem += freemem_size & (alignof (size_t) - 1);
	  freemem_size = freemem_size & ~ (alignof (size_t) - 1);

	  __libc_lock_unlock (lock);
	}

      /* Now domain->conv_tab[act] contains the translation of all
	 the plural variants.  */
      result = domain->conv_tab[act] + sizeof (size_t);
      resultlen = *(size_t *) domain->conv_tab[act];
    }

 converted:
  /* The result string is converted.  */

#endif /* _LIBC || HAVE_ICONV */

  *lengthp = resultlen;
  return result;
}


/* Look up a plural variant.  */
static char *
internal_function
plural_lookup (domain, n, translation, translation_len)
     struct loaded_l10nfile *domain;
     unsigned long int n;
     const char *translation;
     size_t translation_len;
{
  struct loaded_domain *domaindata = (struct loaded_domain *) domain->data;
  unsigned long int index;
  const char *p;

  index = plural_eval (domaindata->plural, n);
  if (index >= domaindata->nplurals)
    /* This should never happen.  It means the plural expression and the
       given maximum value do not match.  */
    index = 0;

  /* Skip INDEX strings at TRANSLATION.  */
  p = translation;
  while (index-- > 0)
    {
#ifdef _LIBC
      p = __rawmemchr (p, '\0');
#else
      p = strchr (p, '\0');
#endif
      /* And skip over the NUL byte.  */
      p++;

      if (p >= translation + translation_len)
	/* This should never happen.  It means the plural expression
	   evaluated to a value larger than the number of variants
	   available for MSGID1.  */
	return (char *) translation;
    }
  return (char *) p;
}


/* Function to evaluate the plural expression and return an index value.  */
static unsigned long int
internal_function
plural_eval (pexp, n)
     struct expression *pexp;
     unsigned long int n;
{
  switch (pexp->nargs)
    {
    case 0:
      switch (pexp->operation)
	{
	case var:
	  return n;
	case num:
	  return pexp->val.num;
	default:
	  break;
	}
      /* NOTREACHED */
      break;
    case 1:
      {
	/* pexp->operation must be lnot.  */
	unsigned long int arg = plural_eval (pexp->val.args[0], n);
	return ! arg;
      }
    case 2:
      {
	unsigned long int leftarg = plural_eval (pexp->val.args[0], n);
	if (pexp->operation == lor)
	  return leftarg || plural_eval (pexp->val.args[1], n);
	else if (pexp->operation == land)
	  return leftarg && plural_eval (pexp->val.args[1], n);
	else
	  {
	    unsigned long int rightarg = plural_eval (pexp->val.args[1], n);

	    switch (pexp->operation)
	      {
	      case mult:
		return leftarg * rightarg;
	      case divide:
		return leftarg / rightarg;
	      case module:
		return leftarg % rightarg;
	      case plus:
		return leftarg + rightarg;
	      case minus:
		return leftarg - rightarg;
	      case less_than:
		return leftarg < rightarg;
	      case greater_than:
		return leftarg > rightarg;
	      case less_or_equal:
		return leftarg <= rightarg;
	      case greater_or_equal:
		return leftarg >= rightarg;
	      case equal:
		return leftarg == rightarg;
	      case not_equal:
		return leftarg != rightarg;
	      default:
		break;
	      }
	  }
	/* NOTREACHED */
	break;
      }
    case 3:
      {
	/* pexp->operation must be qmop.  */
	unsigned long int boolarg = plural_eval (pexp->val.args[0], n);
	return plural_eval (pexp->val.args[boolarg ? 1 : 2], n);
      }
    }
  /* NOTREACHED */
  return 0;
}


/* Return string representation of locale CATEGORY.  */
static const char *
internal_function
category_to_name (category)
     int category;
{
  const char *retval;

  switch (category)
  {
#ifdef LC_COLLATE
  case LC_COLLATE:
    retval = "LC_COLLATE";
    break;
#endif
#ifdef LC_CTYPE
  case LC_CTYPE:
    retval = "LC_CTYPE";
    break;
#endif
#ifdef LC_MONETARY
  case LC_MONETARY:
    retval = "LC_MONETARY";
    break;
#endif
#ifdef LC_NUMERIC
  case LC_NUMERIC:
    retval = "LC_NUMERIC";
    break;
#endif
#ifdef LC_TIME
  case LC_TIME:
    retval = "LC_TIME";
    break;
#endif
#ifdef LC_MESSAGES
  case LC_MESSAGES:
    retval = "LC_MESSAGES";
    break;
#endif
#ifdef LC_RESPONSE
  case LC_RESPONSE:
    retval = "LC_RESPONSE";
    break;
#endif
#ifdef LC_ALL
  case LC_ALL:
    /* This might not make sense but is perhaps better than any other
       value.  */
    retval = "LC_ALL";
    break;
#endif
  default:
    /* If you have a better idea for a default value let me know.  */
    retval = "LC_XXX";
  }

  return retval;
}

#ifdef _WIN32
/* Lifted from GLib: */

/**
 * g_win32_getlocale:
 *
 * The setlocale in the Microsoft C library uses locale names of the
 * form "English_United States.1252" etc. We want the Unixish standard
 * form "en", "zh_TW" etc. This function gets the current thread
 * locale from Windows and returns it as a string of the above form
 * for use in forming file names etc. The return value is a static buffer.
 *
 * Returns: locale name
 */

static char *
g_win32_getlocale (void)
{
  LCID lcid;
  LANGID langid;
  char *ev;
  int primary, sub;
  char *l = NULL, *sl = NULL;
  static char bfr[20];

  /* Use native Win32 API locale ID.  */
  lcid = GetThreadLocale ();

  /* Strip off the sorting rules, keep only the language part.  */
  langid = LANGIDFROMLCID (lcid);

  /* Split into language and territory part.  */
  primary = PRIMARYLANGID (langid);
  sub = SUBLANGID (langid);
  switch (primary)
    {
    case LANG_AFRIKAANS: l = "af"; sl = "ZA"; break;
    case LANG_ALBANIAN: l = "sq"; sl = "AL"; break;
    case LANG_ARABIC:
      l = "ar";
      switch (sub)
	{
	case SUBLANG_ARABIC_SAUDI_ARABIA: sl = "SA"; break;
	case SUBLANG_ARABIC_IRAQ: sl = "IQ"; break;
	case SUBLANG_ARABIC_EGYPT: sl = "EG"; break;
	case SUBLANG_ARABIC_LIBYA: sl = "LY"; break;
	case SUBLANG_ARABIC_ALGERIA: sl = "DZ"; break;
	case SUBLANG_ARABIC_MOROCCO: sl = "MA"; break;
	case SUBLANG_ARABIC_TUNISIA: sl = "TN"; break;
	case SUBLANG_ARABIC_OMAN: sl = "OM"; break;
	case SUBLANG_ARABIC_YEMEN: sl = "YE"; break;
	case SUBLANG_ARABIC_SYRIA: sl = "SY"; break;
	case SUBLANG_ARABIC_JORDAN: sl = "JO"; break;
	case SUBLANG_ARABIC_LEBANON: sl = "LB"; break;
	case SUBLANG_ARABIC_KUWAIT: sl = "KW"; break;
	case SUBLANG_ARABIC_UAE: sl = "AE"; break;
	case SUBLANG_ARABIC_BAHRAIN: sl = "BH"; break;
	case SUBLANG_ARABIC_QATAR: sl = "QA"; break;
	}
      break;
#ifdef LANG_ARMENIAN
    case LANG_ARMENIAN: l = "hy"; sl = "AM"; break;
#endif
#ifdef LANG_ASSAMESE
    case LANG_ASSAMESE: l = "as"; sl = "IN"; break;
#endif
#ifdef LANG_AZERI
    case LANG_AZERI:
      l = "az";
#if defined (SUBLANG_AZERI_LATIN) && defined (SUBLANG_AZERI_CYRILLIC)
      switch (sub)
	{
	/* FIXME: Adjust this when Azerbaijani locales appear on Unix.  */
	case SUBLANG_AZERI_LATIN: sl = "@latin"; break;
	case SUBLANG_AZERI_CYRILLIC: sl = "@cyrillic"; break;
	}
#endif
      break;
#endif
    case LANG_BASQUE:
      l = "eu"; /* sl could be "ES" or "FR".  */
      break;
    case LANG_BELARUSIAN: l = "be"; sl = "BY"; break;
#ifdef LANG_BENGALI
    case LANG_BENGALI: l = "bn"; sl = "IN"; break;
#endif
    case LANG_BULGARIAN: l = "bg"; sl = "BG"; break;
    case LANG_CATALAN: l = "ca"; sl = "ES"; break;
    case LANG_CHINESE:
      l = "zh";
      switch (sub)
	{
	case SUBLANG_CHINESE_TRADITIONAL: sl = "TW"; break;
	case SUBLANG_CHINESE_SIMPLIFIED: sl = "CN"; break;
	case SUBLANG_CHINESE_HONGKONG: sl = "HK"; break;
	case SUBLANG_CHINESE_SINGAPORE: sl = "SG"; break;
#ifdef SUBLANG_CHINESE_MACAU
	case SUBLANG_CHINESE_MACAU: sl = "MO"; break;
#endif
	}
      break;
    case LANG_CROATIAN:		/* LANG_CROATIAN == LANG_SERBIAN
				 * What used to be called Serbo-Croatian
				 * should really now be two separate
				 * languages because of political reasons.
				 * (Says tml, who knows nothing about Serbian
				 * or Croatian.)
				 * (I can feel those flames coming already.)
				 */
      switch (sub)
	{
	/* FIXME: How to distinguish Croatian and Latin Serbian locales?  */
	case SUBLANG_SERBIAN_LATIN: l = "sr"; break;
	case SUBLANG_SERBIAN_CYRILLIC: l = "sr"; sl = "@cyrillic"; break;
	default: l = "hr"; sl = "HR";
	}
      break;
    case LANG_CZECH: l = "cs"; sl = "CZ"; break;
    case LANG_DANISH: l = "da"; sl = "DK"; break;
#ifdef LANG_DIVEHI
    case LANG_DIVEHI: l = "div"; sl = "MV"; break;
#endif
    case LANG_DUTCH:
      l = "nl";
      switch (sub)
	{
	case SUBLANG_DUTCH: sl = "NL"; break;
	case SUBLANG_DUTCH_BELGIAN: sl = "BE"; break;
	}
      break;
    case LANG_ENGLISH:
      l = "en";
      switch (sub)
	{
	case SUBLANG_ENGLISH_US: sl = "US"; break;
	case SUBLANG_ENGLISH_UK: sl = "GB"; break;
	case SUBLANG_ENGLISH_AUS: sl = "AU"; break;
	case SUBLANG_ENGLISH_CAN: sl = "CA"; break;
	case SUBLANG_ENGLISH_NZ: sl = "NZ"; break;
	case SUBLANG_ENGLISH_EIRE: sl = "IE"; break;
	case SUBLANG_ENGLISH_SOUTH_AFRICA: sl = "ZA"; break;
	case SUBLANG_ENGLISH_JAMAICA: sl = "JM"; break;
	case SUBLANG_ENGLISH_CARIBBEAN: sl = "GD"; break; /* Grenada? */
	case SUBLANG_ENGLISH_BELIZE: sl = "BZ"; break;
	case SUBLANG_ENGLISH_TRINIDAD: sl = "TT"; break;
#ifdef SUBLANG_ENGLISH_ZIMBABWE
	case SUBLANG_ENGLISH_ZIMBABWE: sl = "ZW"; break;
#endif
#ifdef SUBLANG_ENGLISH_PHILIPPINES
	case SUBLANG_ENGLISH_PHILIPPINES: sl = "PH"; break;
#endif
	}
      break;
    case LANG_ESTONIAN: l = "et"; sl = "EE"; break;
    case LANG_FAEROESE: l = "fo"; sl = "FO"; break;
    case LANG_FARSI: l = "fa"; sl = "IR"; break;
    case LANG_FINNISH: l = "fi"; sl = "FI"; break;
    case LANG_FRENCH:
      l = "fr";
      switch (sub)
	{
	case SUBLANG_FRENCH: sl = "FR"; break;
	case SUBLANG_FRENCH_BELGIAN: sl = "BE"; break;
	case SUBLANG_FRENCH_CANADIAN: sl = "CA"; break;
	case SUBLANG_FRENCH_SWISS: sl = "CH"; break;
	case SUBLANG_FRENCH_LUXEMBOURG: sl = "LU"; break;
#ifdef SUBLANG_FRENCH_MONACO
	case SUBLANG_FRENCH_MONACO: sl = "MC"; break;
#endif
	}
      break;
      /* FIXME: LANG_GALICIAN: What's the code for Galician? */
#ifdef LANG_GEORGIAN
    case LANG_GEORGIAN: l = "ka"; sl = "GE"; break;
#endif
    case LANG_GERMAN:
      l = "de";
      switch (sub)
	{
	case SUBLANG_GERMAN: sl = "DE"; break;
	case SUBLANG_GERMAN_SWISS: sl = "CH"; break;
	case SUBLANG_GERMAN_AUSTRIAN: sl = "AT"; break;
	case SUBLANG_GERMAN_LUXEMBOURG: sl = "LU"; break;
	case SUBLANG_GERMAN_LIECHTENSTEIN: sl = "LI"; break;
	}
      break;
    case LANG_GREEK: l = "el"; sl = "GR"; break;
#ifdef LANG_GUJARATI
    case LANG_GUJARATI: l = "gu"; sl = "IN"; break;
#endif
    case LANG_HEBREW: l = "he"; sl = "IL"; break;
#ifdef LANG_HINDI
    case LANG_HINDI: l = "hi"; sl = "IN"; break;
#endif
    case LANG_HUNGARIAN: l = "hu"; sl = "HU"; break;
    case LANG_ICELANDIC: l = "is"; sl = "IS"; break;
    case LANG_INDONESIAN: l = "id"; sl = "ID"; break;
    case LANG_ITALIAN:
      l = "it";
      switch (sub)
	{
	case SUBLANG_ITALIAN: sl = "IT"; break;
	case SUBLANG_ITALIAN_SWISS: sl = "CH"; break;
	}
      break;
    case LANG_JAPANESE: l = "ja"; sl = "JP"; break;
#ifdef LANG_KANNADA
    case LANG_KANNADA: l = "kn"; sl = "IN"; break;
#endif
#ifdef LANG_KASHMIRI
    case LANG_KASHMIRI:
      l = "ks";
      switch (sub)
	{
	case SUBLANG_DEFAULT: sl = "PK"; break;
#ifdef SUBLANG_KASHMIRI_INDIA
	case SUBLANG_KASHMIRI_INDIA: sl = "IN"; break;
#endif
	}
      break;
#endif
#ifdef LANG_KAZAK
    case LANG_KAZAK: l = "kk"; sl = "KZ"; break;
#endif
#ifdef LANG_KONKANI
    case LANG_KONKANI:
      /* FIXME: Adjust this when such locales appear on Unix.  */
      l = "kok"; sl = "IN";
      break;
#endif
    case LANG_KOREAN: l = "ko"; sl = "KR"; break;
#ifdef LANG_KYRGYZ
    case LANG_KYRGYZ: l = "ky"; sl = "KG"; /* ??? */ break; 
#endif
    case LANG_LATVIAN: l = "lv"; sl = "LV"; break;
    case LANG_LITHUANIAN: l = "lt"; sl = "LT"; break;
#ifdef LANG_MACEDONIAN
    case LANG_MACEDONIAN: l = "mk"; sl = "MK"; break;
#endif
#ifdef LANG_MALAY
    case LANG_MALAY:
      l = "ms";
      switch (sub)
	{
#ifdef SUBLANG_MALAY_MALAYSIA
	case SUBLANG_MALAY_MALAYSIA: sl = "MY"; break;
#endif
#ifdef SUBLANG_MALAY_BRUNEI_DARUSSALAM
	case SUBLANG_MALAY_BRUNEI_DARUSSALAM: sl = "BN"; break;
#endif
	}
      break;
#endif
#ifdef LANG_MALAYALAM
    case LANG_MALAYALAM: l = "ml"; sl = "IN"; break;
#endif
#ifdef LANG_MANIPURI
    case LANG_MANIPURI:
      /* FIXME: Adjust this when such locales appear on Unix.  */
      l = "mni"; sl = "IN";
      break;
#endif
#ifdef LANG_MARATHI
    case LANG_MARATHI: l = "mr"; sl = "IN"; break;
#endif
#ifdef LANG_MONGOLIAN
    case LANG_MONGOLIAN: l = "mn"; sl = "MN"; break;
#endif
#ifdef LANG_NEPALI
    case LANG_NEPALI:
      l = "ne";
      switch (sub)
	{
	case SUBLANG_DEFAULT: sl = "NP"; break;
#ifdef SUBLANG_NEPALI_INDIA
	case SUBLANG_NEPALI_INDIA: sl = "IN"; break;
#endif
	}
      break;
#endif
    case LANG_NORWEGIAN:
      l = "no";
      switch (sub)
	{
	case SUBLANG_NORWEGIAN_BOKMAL: sl = "NO"; break;
	case SUBLANG_NORWEGIAN_NYNORSK: l = "nn"; sl = "NO"; break;
	}
      break;
#ifdef LANG_ORIYA
    case LANG_ORIYA: l = "or"; sl = "IN"; break;
#endif
    case LANG_POLISH: l = "pl"; sl = "PL"; break;
    case LANG_PORTUGUESE:
      l = "pt";
      switch (sub)
	{
	case SUBLANG_PORTUGUESE: sl = "PT"; break;
	case SUBLANG_PORTUGUESE_BRAZILIAN: sl = "BR"; break;
	}
      break;
#ifdef LANG_PUNJABI
    case LANG_PUNJABI: l = "pa"; sl = "IN"; break;
#endif
    case LANG_ROMANIAN: l = "ro"; sl = "RO"; break;
    case LANG_RUSSIAN:
      l = "ru"; /* sl could be "RU" or "UA".  */
      break;
#ifdef LANG_SANSKRIT
    case LANG_SANSKRIT: l = "sa"; sl = "IN"; break;
#endif
#ifdef LANG_SINDHI
    case LANG_SINDHI: l = "sd"; break;
#endif
    case LANG_SLOVAK: l = "sk"; sl = "SK"; break;
    case LANG_SLOVENIAN: l = "sl"; sl = "SI"; break;
#ifdef LANG_SORBIAN
    case LANG_SORBIAN:
      /* FIXME: Adjust this when such locales appear on Unix.  */
      l = "wen"; sl = "DE";
      break;
#endif
    case LANG_SPANISH:
      l = "es";
      switch (sub)
	{
	case SUBLANG_SPANISH: sl = "ES"; break;
	case SUBLANG_SPANISH_MEXICAN: sl = "MX"; break;
	case SUBLANG_SPANISH_MODERN:
	  sl = "ES@modern"; break;	/* not seen on Unix */
	case SUBLANG_SPANISH_GUATEMALA: sl = "GT"; break;
	case SUBLANG_SPANISH_COSTA_RICA: sl = "CR"; break;
	case SUBLANG_SPANISH_PANAMA: sl = "PA"; break;
	case SUBLANG_SPANISH_DOMINICAN_REPUBLIC: sl = "DO"; break;
	case SUBLANG_SPANISH_VENEZUELA: sl = "VE"; break;
	case SUBLANG_SPANISH_COLOMBIA: sl = "CO"; break;
	case SUBLANG_SPANISH_PERU: sl = "PE"; break;
	case SUBLANG_SPANISH_ARGENTINA: sl = "AR"; break;
	case SUBLANG_SPANISH_ECUADOR: sl = "EC"; break;
	case SUBLANG_SPANISH_CHILE: sl = "CL"; break;
	case SUBLANG_SPANISH_URUGUAY: sl = "UY"; break;
	case SUBLANG_SPANISH_PARAGUAY: sl = "PY"; break;
	case SUBLANG_SPANISH_BOLIVIA: sl = "BO"; break;
	case SUBLANG_SPANISH_EL_SALVADOR: sl = "SV"; break;
	case SUBLANG_SPANISH_HONDURAS: sl = "HN"; break;
	case SUBLANG_SPANISH_NICARAGUA: sl = "NI"; break;
	case SUBLANG_SPANISH_PUERTO_RICO: sl = "PR"; break;
	}
      break;
#ifdef LANG_SWAHILI
    case LANG_SWAHILI: l = "sw"; break;
#endif
    case LANG_SWEDISH:
      l = "sv";
      switch (sub)
	{
	case SUBLANG_DEFAULT: sl = "SE"; break;
	case SUBLANG_SWEDISH_FINLAND: sl = "FI"; break;
	}
      break;
#ifdef LANG_SYRIAC
    case LANG_SYRIAC: l = "syr"; break;
#endif
#ifdef LANG_TAMIL
    case LANG_TAMIL:
      l = "ta"; /* sl could be "IN" or "LK".  */
      break;
#endif
#ifdef LANG_TATAR
    case LANG_TATAR: l = "tt"; break;
#endif
#ifdef LANG_TELUGU
    case LANG_TELUGU: l = "te"; sl = "IN"; break;
#endif
    case LANG_THAI: l = "th"; sl = "TH"; break;
    case LANG_TURKISH: l = "tr"; sl = "TR"; break;
    case LANG_UKRAINIAN: l = "uk"; sl = "UA"; break;
#ifdef LANG_URDU
    case LANG_URDU:
      l = "ur";
      switch (sub)
	{
#ifdef SUBLANG_URDU_PAKISTAN
	case SUBLANG_URDU_PAKISTAN: sl = "PK"; break;
#endif
#ifdef SUBLANG_URDU_INDIA
	case SUBLANG_URDU_INDIA: sl = "IN"; break;
#endif
	}
      break;
#endif
#ifdef LANG_UZBEK
    case LANG_UZBEK:
      l = "uz";
      switch (sub)
	{
	/* FIXME: Adjust this when Uzbek locales appear on Unix.  */
#ifdef SUBLANG_UZBEK_LATIN
	case SUBLANG_UZBEK_LATIN: sl = "UZ@latin"; break;
#endif
#ifdef SUBLANG_UZBEK_CYRILLIC
	case SUBLANG_UZBEK_CYRILLIC: sl = "UZ@cyrillic"; break;
#endif
	}
      break;
#endif
    case LANG_VIETNAMESE: l = "vi"; sl = "VN"; break;
    default: l = "xx"; break;
    }
  strcpy (bfr, l);
  if (sl != NULL)
    {
      if (sl[0] != '@')
	strcat (bfr, "_");
      strcat (bfr, sl);
    }

  return bfr;
}

#endif

/* Guess value of current locale from value of the environment variables.  */
static const char *
internal_function
guess_category_value (category, categoryname)
     int category;
     const char *categoryname;
{
  const char *language;
  const char *retval;

  /* The highest priority value is the `LANGUAGE' environment
     variable.  But we don't use the value if the currently selected
     locale is the C locale.  This is a GNU extension.  */
  language = getenv ("LANGUAGE");
  if (language != NULL && language[0] == '\0')
    language = NULL;

  /* We have to proceed with the POSIX methods of looking to `LC_ALL',
     `LC_xxx', and `LANG'.  On some systems this can be done by the
     `setlocale' function itself.  */
#if defined _LIBC || (defined HAVE_SETLOCALE && defined HAVE_LC_MESSAGES && defined HAVE_LOCALE_NULL)
  retval = setlocale (category, NULL);
#else
  /* Setting of LC_ALL overwrites all other.  */
  retval = getenv ("LC_ALL");
  if (retval == NULL || retval[0] == '\0')
    {
      /* Next comes the name of the desired category.  */
      retval = getenv (categoryname);
      if (retval == NULL || retval[0] == '\0')
	{
	  /* Last possibility is the LANG environment variable.  */
	  retval = getenv ("LANG");
	  if (retval == NULL || retval[0] == '\0')
#ifdef _WIN32
	    return g_win32_getlocale ();
#else
	    /* We use C as the default domain.  POSIX says this is
	       implementation defined.  */
	    return "C";
#endif
	}
    }
#endif
  return language != NULL && strcmp (retval, "C") != 0 ? language : retval;
}

/* @@ begin of epilog @@ */

/* We don't want libintl.a to depend on any other library.  So we
   avoid the non-standard function stpcpy.  In GNU C Library this
   function is available, though.  Also allow the symbol HAVE_STPCPY
   to be defined.  */
#if !_LIBC && !HAVE_STPCPY
static char *
stpcpy (dest, src)
     char *dest;
     const char *src;
{
  while ((*dest++ = *src++) != '\0')
    /* Do nothing. */ ;
  return dest - 1;
}
#endif

#if !_LIBC && !HAVE_MEMPCPY
static void *
mempcpy (dest, src, n)
     void *dest;
     const void *src;
     size_t n;
{
  return (void *) ((char *) memcpy (dest, src, n) + n);
}
#endif


#ifdef _LIBC
/* If we want to free all resources we have to do some work at
   program's end.  */
static void __attribute__ ((unused))
free_mem (void)
{
  void *old;

  while (_nl_domain_bindings != NULL)
    {
      struct binding *oldp = _nl_domain_bindings;
      _nl_domain_bindings = _nl_domain_bindings->next;
      if (oldp->dirname != _nl_default_dirname)
	/* Yes, this is a pointer comparison.  */
	free (oldp->dirname);
      free (oldp->codeset);
      free (oldp);
    }

  if (_nl_current_default_domain != _nl_default_default_domain)
    /* Yes, again a pointer comparison.  */
    free ((char *) _nl_current_default_domain);

  /* Remove the search tree with the known translations.  */
  __tdestroy (root, free);
  root = NULL;

  while (transmem_list != NULL)
    {
      old = transmem_list;
      transmem_list = transmem_list->next;
      free (old);
    }
}

text_set_element (__libc_subfreeres, free_mem);
#endif
