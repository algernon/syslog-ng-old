#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "libtest/testutils.h"
#include "filemonitor.c"
#include "lib/messages.h"

#define TESTDIR "test.resolve_path"
const gchar *basedir = NULL;

void create_dir(const gchar *dirname)
{
  assert_true(mkdir(dirname, 0700) == 0, "create create dir : %s", dirname);
}

void create_file(const gchar *fname)
{
  FILE *fp;

  fp = fopen(fname, "wt");
  assert_true(fp != NULL, "cannot create file :%s, reason: %s", fname, strerror(errno));

  fclose(fp);
}

void create_symlink(const gchar *original, const gchar *link)
{
  assert_true(symlink(original, link) == 0, "cannot create link: %s -> %s", link, original);
}

void create_symlinks(const gchar *link_base, const gchar *original, int level)
{
  int i;

  gchar *link = NULL;
  gchar *prev_link = NULL;

  link = g_strdup_printf("%s%d", link_base, 0);
  create_symlink(original, link);
  g_free(link);

  for (i = 1; i < level; i++)
  {
    link  =  g_strdup_printf("%s%d", link_base, i);
    prev_link = g_strdup_printf("%s%d", link_base, i-1);
    create_symlink(prev_link, link);
    
    g_free(link);
    g_free(prev_link);
  }
}

void delete_symlinks(const gchar *symlink, int level)
{
  int i;
  gchar link[4096];

  for (i = 0; i < level; i++)
  {
    sprintf(link, "%s%d", symlink, level-1-i);
    remove(link);
  }
}

void test_resolve_path_containing_dots()
{
  gchar *r_path;
  gchar *expected = g_build_filename(basedir, "test.txt", NULL);
  gchar *unresolved = g_build_filename(basedir, "./", "./", "test.txt", NULL);

  create_file(expected);
  r_path = resolve_to_absolute_path(unresolved, "/");
  remove(expected);
  
  assert_string(r_path, expected, "%s;%s;%d", __FILE__, __func__, __LINE__);

  g_free(unresolved);
  g_free(expected);
  g_free(r_path);
}

void test_resolve_symlinks()
{
  int link_level = 8;
  gchar *r_path;
  gchar *expected = g_build_filename(basedir, "original.txt", NULL);
  gchar *baselink = g_build_filename(basedir, "symlink", NULL);
  gchar *unresolved = g_strdup_printf("%s/%s%d", basedir, "symlink", link_level-1);

  create_file(expected);
  create_symlinks(baselink, expected, link_level);

  r_path = resolve_to_absolute_path(unresolved, "/");

  delete_symlinks(baselink, link_level);
  remove(expected);

  assert_string(r_path, expected, "%s;%s;%d", __FILE__, __func__, __LINE__);

  g_free(r_path);
  g_free(expected);
  g_free(baselink);
  g_free(unresolved);
}

void test_resolve_nonexistent_file()
{
  gchar *r_path;
  gchar *expected = g_build_filename(basedir, "test.txt", NULL);

  remove(expected);
  r_path = resolve_to_absolute_path(expected, "/");

  assert_string(r_path, expected, "%s;%s;%d", __FILE__, __func__, __LINE__);

  g_free(r_path);
  g_free(expected);
}

void init_test_dir()
{
  gchar cwd[1024];
  char *r_cwd;

  r_cwd = getcwd(cwd, sizeof(cwd));
  assert_true(r_cwd != NULL, "getcwd failed; reason: %s", strerror(errno));

  basedir = g_build_filename(r_cwd, TESTDIR, NULL);
  create_dir(basedir);
}

void remove_path(const char *path)
{
  gchar *cmd = g_strdup_printf("rm -r %s", path);
  assert_true(system(cmd) != -1, "failed to run : [%s]", cmd);
  g_free(cmd);
}

void teardown()
{
  remove_path(basedir);
  g_free(basedir);
}

void setup()
{
  msg_init(FALSE);
  atexit(teardown);
  init_test_dir();
}

int main(int argc, char **argv)
{
  setup();

  test_resolve_path_containing_dots();
  test_resolve_symlinks();
  test_resolve_nonexistent_file();
  
  return 0;
}
