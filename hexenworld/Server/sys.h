// sys.h -- non-portable functions

int	Sys_FileTime (char *path);

void Sys_mkdir (char *path);

void Sys_Error (char *error, ...);
// an error will cause the entire program to exit

void Sys_Printf (char *fmt, ...);
// send text to the console

void Sys_Quit (void);
double Sys_DoubleTime (void);
char *Sys_ConsoleInput (void);
void Sys_Init (void);
