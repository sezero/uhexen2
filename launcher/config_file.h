
#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H

int Sys_GetUserdir(char *buff, unsigned int len);
void fill_default_options();
int write_config_file();
int read_config_file();

#endif
