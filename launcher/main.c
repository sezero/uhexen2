#include "com_sys.h"
#include "config_file.h"
#include "interface.h"
//#include "support.h"

char bin_dir[1024];
char userdir[1024];

int main (int argc, char *argv[]) {

  GtkWidget *window1;

/*
#ifdef ENABLE_NLS
  bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
  textdomain (PACKAGE);
#endif
  gtk_set_locale (); */

  gtk_init (&argc, &argv);

/*
  add_pixmap_directory (PACKAGE_DATA_DIR "/pixmaps");
  add_pixmap_directory (PACKAGE_SOURCE_DIR "/pixmaps");
*/
  printf("\nLinux Hexen2 Launcher, version %s\n", HOTL_VER);

  if (!(Sys_GetUserdir(userdir,sizeof(userdir)))) {
    fprintf (stderr,"Couldn't determine userspace directory");
    exit(0);
  }

  memset(bin_dir,0,1024);
  Sys_FindBinDir(argv[0], bin_dir);
  printf("Basedir  : %s\n",bin_dir);
  printf("Userdir  : %s\n",userdir);

  read_config_file();

  chdir(bin_dir);

  /*
   * The following code was added by Glade to create one of each component
   * (except popup menus), just so that you see something after building
   * the project. Delete any components that you don't want shown initially.
   */
  window1 = create_window1 ();
  gtk_widget_show (window1);

  gtk_main ();

  return 0;
}
