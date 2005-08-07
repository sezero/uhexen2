// sys.h -- non-portable functions

#if defined(_WIN32) && !defined(F_OK)
// values for the second argument to access(). MS does not define them
#define	R_OK	4		/* Test for read permission.  */
#define	W_OK	2		/* Test for write permission.  */
#define	X_OK	1		/* Test for execute permission.  */
#define	F_OK	0		/* Test for existence.  */
#endif

int	Sys_mkdir (char *path);

void Sys_Error (char *error, ...);
// an error will cause the entire program to exit

void Sys_Printf (char *fmt, ...);
// send text to the console

void Sys_Quit (void);
double Sys_DoubleTime (void);
char *Sys_ConsoleInput (void);
void Sys_Init (void);
