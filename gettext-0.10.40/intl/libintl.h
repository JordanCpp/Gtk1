
#ifndef _LIBINTL_H
#define _LIBINTL_H

#ifdef __cplusplus
extern "C" {
#endif

char *gettext(const char *msgid);
char *dgettext(const char *domainname, const char *msgid);
char *dcgettext(const char *domainname, const char *msgid, int category);
char *textdomain(const char *domainname);
char *bindtextdomain(const char *domainname, const char *dirname);
char *bind_textdomain_codeset(const char *domainname, const char *codeset);

/* Фикс для GLib: мапим имена, если линкер их не видит */
#define _dgettext dgettext
#define _bindtextdomain bindtextdomain

#ifdef __cplusplus
}
#endif

#endif
