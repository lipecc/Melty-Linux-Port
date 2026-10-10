#define _XOPEN_SOURCE 700
#include <archive.h>
#include <archive_entry.h>
#include <ftw.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
static struct archive *writer;
static const char *base;
static size_t prefix;
static int failed=0;
static int visit(const char*path,const struct stat*st,int mode,struct FTW*ignored) {
  (void)ignored;
  if (failed) return 1;
  const char *rel=path+prefix;
  if (!*rel) return 0;
  if (*rel=='/')rel++;
  struct archive_entry *ent=archive_entry_new();
  archive_entry_set_pathname(ent,rel);
  archive_entry_set_perm(ent,st->st_mode&0777);
  archive_entry_set_uid(ent,0); archive_entry_set_gid(ent,0);
  archive_entry_set_mtime(ent,st->st_mtime,0);
  if (S_ISDIR(st->st_mode)) { archive_entry_set_filetype(ent,AE_IFDIR); archive_entry_set_size(ent,0); }
  else if (S_ISREG(st->st_mode)) { archive_entry_set_filetype(ent,AE_IFREG); archive_entry_set_size(ent,st->st_size); }
  else if (S_ISLNK(st->st_mode)) {
    archive_entry_set_filetype(ent,AE_IFLNK); archive_entry_set_size(ent,0);
    char link[PATH_MAX]; ssize_t n=readlink(path,link,sizeof(link)-1);
    if (n<0) { perror(path);failed=1;archive_entry_free(ent);return 1; }
    link[n]=0;archive_entry_set_symlink(ent,link);
  } else { archive_entry_free(ent);return 0; }
  if (archive_write_header(writer,ent)!=ARCHIVE_OK) {
    fprintf(stderr,"ISO write header %s: %s\n",rel,archive_error_string(writer));failed=1;
  } else if (S_ISREG(st->st_mode)) {
    int fd=open(path,O_RDONLY); if(fd<0){perror(path);failed=1;}
    else{ char buf[65536]; ssize_t n;
      while((n=read(fd,buf,sizeof(buf)))>0){if(archive_write_data(writer,buf,n)<0){failed=1;break;}}
      if(n<0){perror(path);failed=1;}close(fd);
    }
  }
  archive_entry_free(ent);
  return failed ? 1:0;
}
int main(int argc,char**argv){
  if(argc!=3){fprintf(stderr,"usage: make_iso AppDir output.iso\n");return 2;}
  base=argv[1];prefix=strlen(base); if(prefix&&base[prefix-1]=='/')prefix--;
  writer=archive_write_new();
  if(archive_write_set_format_iso9660(writer)!=ARCHIVE_OK){fprintf(stderr,"ISO format error\n");return 1;}
  if(archive_write_open_filename(writer,argv[2])!=ARCHIVE_OK){fprintf(stderr,"open ISO: %s\n",archive_error_string(writer));return 1;}
  nftw(base,visit,64,FTW_PHYS);
  if(archive_write_close(writer)!=ARCHIVE_OK){fprintf(stderr,"ISO close: %s\n",archive_error_string(writer));failed=1;}
  archive_write_free(writer);
  return failed ? 1:0;
}
