//
// pr_strng.h
//

extern	int	*pr_string_index;
extern	char	*pr_global_strings;
extern	int	pr_string_count;

extern	char	*puzzle_strings;

void PR_LoadStrings (void);

#if !defined(H2W)
// mission pack objectives strings
extern	int	*pr_info_string_index;
extern	char	*pr_global_info_strings;
extern	int	pr_info_string_count;

void PR_LoadInfoStrings (void);
#endif	// !H2W

