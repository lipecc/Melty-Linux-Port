#define _GNU_SOURCE
#include <archive.h>
#include <archive_entry.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <inttypes.h>
#define BUNDLE_NAME "Melty-Linux-Port-1.0.72"
static char scratch[128*1024];
static int ensuredir(const char *path){char name[PATH_MAX];snprintf(name,sizeof(name),"%s",path);for(char*p=name+1;*p;p++)if(*p=='/'){*p=0;if(mkdir(name,0700)<0&&errno!=EEXIST)return -1;*p='/';}return mkdir(name,0700)<0&&errno!=EEXIST?-1:0;}
static uint64_t hashbinary(FILE*f){uint64_t h=UINT64_C(14695981039346656037);rewind(f);size_t n;while((n=fread(scratch,1,sizeof(scratch),f))>0){for(size_t i=0;i<n;i++){h^=(unsigned char)scratch[i];h*=UINT64_C(1099511628211);}}return h;}
static int writeiso(FILE*f,long off,const char*dest){int fd=open(dest,O_WRONLY|O_CREAT|O_TRUNC,0600);if(fd<0){perror("open ISO");return -1;}if(fseek(f,off,SEEK_SET)!=0){close(fd);return -1;}size_t n;while((n=fread(scratch,1,sizeof(scratch),f))>0){if(write(fd,scratch,n)!=(ssize_t)n){perror("ISO write");close(fd);return -1;}}close(fd);return 0;}
static int extractiso(const char*isofile,const char*dest){
 if(ensuredir(dest)<0){perror("mkdir extract");return -1;}
 struct archive*a=archive_read_new(); archive_read_support_filter_all(a);archive_read_support_format_iso9660(a);
 if(archive_read_open_filename(a,isofile,32768)!=ARCHIVE_OK){fprintf(stderr,"Error reading ISO: %s\n",archive_error_string(a));archive_read_free(a);return -1;}
 int back=open(".",O_RDONLY|O_DIRECTORY);if(back<0||chdir(dest)<0){perror("chdir");archive_read_free(a);return -1;}
 struct archive_entry*ent;int err=0;
 while(archive_read_next_header(a,&ent)==ARCHIVE_OK){const char*name=archive_entry_pathname(ent);
  if(!name||!strcmp(name,".")||name[0]=='/'||strstr(name,"../")||!strcmp(name,"..")){archive_read_data_skip(a);continue;}
  int opts=ARCHIVE_EXTRACT_PERM|ARCHIVE_EXTRACT_TIME|ARCHIVE_EXTRACT_SECURE_NODOTDOT|ARCHIVE_EXTRACT_SECURE_NOABSOLUTEPATHS;
  if(archive_read_extract(a,ent,opts)!=ARCHIVE_OK){fprintf(stderr,"Extraction failed for %s: %s\n",name,archive_error_string(a));err=1;break;}
 }
 archive_read_close(a);archive_read_free(a);fchdir(back);close(back);return err?-1:0;
}
int main(int argc,char**argv){
 FILE*f=fopen("/proc/self/exe","rb");if(!f){perror("self");return 1;}
 unsigned char ident[16];if(fread(ident,1,16,f)!=16||memcmp(ident,"\177ELF",4)||ident[8]!='A'||ident[9]!='I'||ident[10]!=1){fprintf(stderr,"Melty AppImage is corrupt\n");fclose(f);return 1;}
 uint32_t offset=(uint32_t)ident[11]|(uint32_t)ident[12]<<8|(uint32_t)ident[13]<<16|(uint32_t)ident[14]<<24;
 if(offset<4096){fprintf(stderr,"ISO header missing\n");fclose(f);return 1;}
 if(argc>1&&strcmp(argv[1],"--appimage-offset")==0){printf("%u\n",offset);return 0;}
 if(argc>1&&strcmp(argv[1],"--appimage-version")==0){puts(BUNDLE_NAME " (AppImage tipo 1, bootstrap Electron Linux)");return 0;}
 char base[PATH_MAX];const char*cache=getenv("XDG_CACHE_HOME");const char*home=getenv("HOME");
 if(cache&&*cache)snprintf(base,sizeof(base),"%s",cache);else if(home&&*home)snprintf(base,sizeof(base),"%s/.cache",home);else snprintf(base,sizeof(base),"/tmp/melty-cache-%d",getuid());
 uint64_t h=hashbinary(f);char folder[PATH_MAX];
 if(argc>1&&strcmp(argv[1],"--appimage-extract")==0){snprintf(folder,sizeof(folder),"%s","squashfs-root");}
 else snprintf(folder,sizeof(folder),"%s/melty-port-image-%016" PRIx64,base,h);
 char marker[PATH_MAX];snprintf(marker,sizeof(marker),"%s/.melty-extracted",folder);
 if(access(marker,F_OK)!=0){
  if(ensuredir(base)<0){perror("cache");return 1;}
  if(ensuredir(folder)<0){perror("extract root");return 1;}
  char iso[PATH_MAX];snprintf(iso,sizeof(iso),"%s/bundle.%d.iso",folder,getpid());
  if(writeiso(f,offset,iso)!=0||extractiso(iso,folder)!=0){unlink(iso);fprintf(stderr,"Unable to extract Melty contents.\n");return 1;}
  unlink(iso);int done=open(marker,O_CREAT|O_WRONLY|O_TRUNC,0600);if(done>=0){write(done,"ok\n",3);close(done);}
 }
 fclose(f);
 if(argc>1&&strcmp(argv[1],"--appimage-extract")==0){printf("Extracted to %s\n",folder);return 0;}
 char run[PATH_MAX];snprintf(run,sizeof(run),"%s/AppRun",folder);if(access(run,X_OK)!=0){chmod(run,0755);}
 // AppRun needs the original executable location to install a stable XDG handler.
 char selfpath[PATH_MAX];ssize_t sn=readlink("/proc/self/exe",selfpath,sizeof(selfpath)-1);
 if(sn>0){selfpath[sn]=0;setenv("APPIMAGE",selfpath,1);}
 setenv("APPDIR",folder,1);
 execv(run,argv);perror("AppRun");return 1;
}
