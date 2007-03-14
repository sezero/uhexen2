/*
	pr_strng.h
	For international stuff

	$Id: pr_strng.h,v 1.6 2007-03-14 21:03:17 sezero Exp $
*/

#ifndef __PR_STRINGS_H
#define __PR_STRINGS_H

extern	int	*pr_string_index;
extern	char	*pr_global_strings;
extern	int	pr_string_count;

void PR_LoadStrings (void);

#if !defined(SERVERONLY)
extern	char	*puzzle_strings;
extern	int	*puzzle_string_index;
extern	int	puzzle_string_count;

void PR_LoadPuzzleStrings (void);
#endif	/* !SERVERONLY */

#if !defined(SERVERONLY) && !defined(H2W)
// mission pack objectives strings
extern	int	*pr_info_string_index;
extern	char	*pr_global_info_strings;
extern	int	pr_info_string_count;

void PR_LoadInfoStrings (void);
#endif	/* !SERVERONLY && !H2W */

#endif	/* __PR_STRINGS_H */

