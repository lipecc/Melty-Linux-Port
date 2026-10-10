/* Melty Linux bootstrap: small native X11/libcurl graphical downloader.
   It downloads only the official Electron runtime and never requires root.
   Program retains a CLI fallback in AppRun for desktops without XWayland. */
#define _POSIX_C_SOURCE 200809L
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <locale.h>
#include "i18n.h"
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

static Display *dpy;
static Window win;
static GC gc;
static Atom wm_delete;
static int cancelled=0;
static int progress=-1;
static char subtitle[256]="";
static char subfooter[256]="";
static int stage=0;
static int lang=ML_EN;
static XFontSet fontset;
static unsigned long dark, light, muted, blue, track, soft;

static void rgb(XColor *c, const char *hex) {
    unsigned rr=0,gg=0,bb=0;
    sscanf(hex,"#%02x%02x%02x",&rr,&gg,&bb);
    c->red=(unsigned short)rr*257;c->green=(unsigned short)gg*257;c->blue=(unsigned short)bb*257;
    c->flags=DoRed|DoGreen|DoBlue;
    XAllocColor(dpy,DefaultColormap(dpy,DefaultScreen(dpy)),c);
}
static unsigned long color(const char* hex){XColor c;rgb(&c,hex);return c.pixel;}
static void rect(int x,int y,unsigned w,unsigned h,unsigned long c){XSetForeground(dpy,gc,c);XFillRectangle(dpy,win,gc,x,y,w,h);}
static void txt(int x,int y,const char*s,unsigned long c){
  XSetForeground(dpy,gc,c);
  if(fontset)Xutf8DrawString(dpy,win,fontset,gc,x,y,s,(int)strlen(s));
  else XDrawString(dpy,win,gc,x,y,s,(int)strlen(s));
}
static void paint(void) {
    rect(0,0,580,302,dark);
    rect(0,0,580,5,blue);
    txt(38,46,"MELTY  /  LINUX",blue);
    txt(38,85,ml_get(lang,M_DOWN_TITLE),light);
    txt(38,111,ml_get(lang,M_DOWN_HELP),muted);
    txt(38,162,subtitle,light);
    rect(38,181,504,15,track);
    if(progress>=0){int ww=504*progress/100;if(ww>0)rect(38,181,ww,15,blue);}
    txt(38,226,subfooter,muted);
    rect(428,249,114,34,soft);
    txt(444,271,ml_get(lang,M_CANCEL),light);
    XFlush(dpy);
}
static void events(void){
    XEvent e;
    while(XPending(dpy)) {
      XNextEvent(dpy,&e);
      if(e.type==Expose)paint();
      else if(e.type==ClientMessage && (Atom)e.xclient.data.l[0]==wm_delete)cancelled=1;
      else if(e.type==KeyPress){KeySym sym=XLookupKeysym(&e.xkey,0);if(sym==XK_Escape||sym==XK_q)cancelled=1;}
      else if(e.type==ButtonPress&&e.xbutton.x>=425&&e.xbutton.x<548&&e.xbutton.y>=245&&e.xbutton.y<289)cancelled=1;
    }
}
static int download(const char*url,const char*tmp){
   int p[2];if(pipe(p)!=0){perror("[Melty] pipe");return 1;}
   pid_t pid=fork();
   if(pid==0){
       close(p[0]);dup2(p[1],STDERR_FILENO);close(p[1]);
       execlp("curl","curl","--fail","--location","--retry","2",
          "--connect-timeout","25","--max-time","1500","--progress-bar",
          "--user-agent","Melty-Linux-Port/1.0.72","--output",tmp,url,(char*)NULL);
       _exit(127);
   }
   close(p[1]);if(pid<0){close(p[0]);return 1;}
   fcntl(p[0],F_SETFL,fcntl(p[0],F_GETFL)|O_NONBLOCK);
   progress=-1;snprintf(subtitle,sizeof subtitle,"%s",ml_get(lang,M_CONNECT_ELECTRON));paint();
   char lastbuf[2000]="";size_t used=0;
   int status=0,complete=0;
   while(!complete){
      events();if(cancelled){kill(pid,SIGTERM);}
      char buf[512];ssize_t n;
      while((n=read(p[0],buf,sizeof buf))>0){
         for(ssize_t i=0;i<n;i++){
           unsigned char c=buf[i];
           if(c=='\r'||c=='\n'){lastbuf[used]=0;
             char*pc=strrchr(lastbuf,'%');
             if(pc){char*start=pc;while(start>lastbuf&&(isdigit((unsigned char)start[-1])||start[-1]=='.'))start--;
               if(start<pc){int val=(int)strtod(start,NULL);if(val>=0&&val<=100)progress=val;}}
             if(used&&strstr(lastbuf,"curl:"))fprintf(stderr,"[Melty] %s\n",lastbuf);
             used=0;
           }else if(used<sizeof(lastbuf)-1)lastbuf[used++]=(char)c;
         }
      }
      struct stat st;double mb=0.0;
      if(stat(tmp,&st)==0)mb=(double)st.st_size/1048576.0;
      if(progress>=0)snprintf(subtitle,sizeof subtitle,ml_get(lang,M_DOWNLOADING_PERCENT),mb,progress);
      else snprintf(subtitle,sizeof subtitle,ml_get(lang,M_DOWNLOADING),mb);
      paint();
      pid_t wr=waitpid(pid,&status,WNOHANG);
      if(wr==pid||wr<0)complete=1;
      if(!complete){struct timespec ts={0,120*1000*1000};nanosleep(&ts,NULL);}
   }
   close(p[0]);
   return WIFEXITED(status)&&WEXITSTATUS(status)==0?0:1;
}
static void pause_frames(int ms){
  struct timespec ts={0,50*1000*1000};
  for(int i=0;i<ms/50;i++){events();if(cancelled)break;nanosleep(&ts,NULL);}
}
int main(int argc,char**argv){
   if(argc!=4){fprintf(stderr,"Usage: gui-downloader output.zip primary_url mirror_url\n");return 2;}
   setlocale(LC_CTYPE,"");lang=ml_detect();
   snprintf(subtitle,sizeof subtitle,"%s",ml_get(lang,M_CONNECT));
   snprintf(subfooter,sizeof subfooter,"%s",ml_get(lang,M_DOWN_PREPARE));
   if(!(dpy=XOpenDisplay(NULL))){fprintf(stderr,"[Melty] No X display\n");return 4;}
   int scr=DefaultScreen(dpy);
   dark=color("#141517");light=color("#F1F4F6");muted=color("#8C98A8");blue=color("#248EF5");track=color("#34383F");soft=color("#2D3138");
   win=XCreateSimpleWindow(dpy,RootWindow(dpy,scr),180,150,580,302,0,dark,dark);
   XStoreName(dpy,win,"Melty - Components");
   XSelectInput(dpy,win,ExposureMask|KeyPressMask|ButtonPressMask|StructureNotifyMask);
   wm_delete=XInternAtom(dpy,"WM_DELETE_WINDOW",False);
   XSetWMProtocols(dpy,win,&wm_delete,1);
   XSizeHints sz={0};sz.flags=PMinSize|PMaxSize;sz.min_width=sz.max_width=580;sz.min_height=sz.max_height=302;
   XSetWMNormalHints(dpy,win,&sz);
   gc=XCreateGC(dpy,win,0,NULL);
   char **missing=NULL;int missing_count=0;char *def=NULL;
   fontset=XCreateFontSet(dpy,"-*-*-medium-r-normal--14-*-*-*-*-*-*-*",&missing,&missing_count,&def);
   if(missing)XFreeStringList(missing);
   XFontStruct *font=XLoadQueryFont(dpy,"-misc-fixed-*-*-*-*-14-*-*-*-*-*-*-*");
   if(!font)font=XLoadQueryFont(dpy,"fixed");
   if(!fontset && font)XSetFont(dpy,gc,font->fid);
   XMapRaised(dpy,win);events();paint();
   char tmp[4096];snprintf(tmp,sizeof(tmp),"%s.partial.%ld",argv[1],(long)getpid());
   stage=1;
   int rc=download(argv[2],tmp);
   if(rc && !cancelled){
       snprintf(subfooter,sizeof subfooter,"%s",ml_get(lang,M_MIRROR));
       paint();pause_frames(600);
       rc=download(argv[3],tmp);
   }
   if(cancelled){snprintf(subtitle,sizeof subtitle,"%s",ml_get(lang,M_CANCELLED));paint();pause_frames(600);rc=3;}
   if(!rc && rename(tmp,argv[1])!=0){perror("[Melty] rename");rc=1;}
   if(!rc){progress=100;snprintf(subtitle,sizeof subtitle,"%s",ml_get(lang,M_FINISHED));snprintf(subfooter,sizeof subfooter,"%s",ml_get(lang,M_EXTRACT_NEXT));paint();pause_frames(800);}
   else {unlink(tmp);if(!cancelled){snprintf(subfooter,sizeof subfooter,"%s",ml_get(lang,M_DOWNLOAD_FAILED));paint();pause_frames(2600);}}
   if(fontset)XFreeFontSet(dpy,fontset);
   if(font)XFreeFont(dpy,font);
   XFreeGC(dpy,gc);XDestroyWindow(dpy,win);XCloseDisplay(dpy);
   return rc;
}
