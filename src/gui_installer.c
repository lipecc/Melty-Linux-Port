/* SPDX-License-Identifier: MIT
 * Melty Linux Port: standalone X11 installer, no sudo required.
 * Exit status: 0 install, 10 run once, 11 cancel, 4 no display.
 * Selected UI language is stored in XDG_CONFIG_HOME/Melty/language on confirmation.
 */
#define _POSIX_C_SOURCE 200809L
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>
#include "i18n.h"

static Display *d;
static Window w;
static GC gc;
static XFontSet fontset;
static Atom wm_delete;
static unsigned long bg, white, gray, accent, muted, border;
static int updating=0, lang=ML_EN;

static unsigned long rgb(const char *s) {
  XColor c={0};unsigned r=0,g=0,b=0;
  sscanf(s,"#%02x%02x%02x",&r,&g,&b);
  c.red=r*257;c.green=g*257;c.blue=b*257;c.flags=DoRed|DoGreen|DoBlue;
  XAllocColor(d,DefaultColormap(d,DefaultScreen(d)),&c);
  return c.pixel;
}
static void box(int x,int y,int width,int height,unsigned long color){
  XSetForeground(d,gc,color);XFillRectangle(d,w,gc,x,y,width,height);
}
static void label(int x,int y,const char *str,unsigned long color){
  XSetForeground(d,gc,color);
  if(fontset) Xutf8DrawString(d,w,fontset,gc,x,y,str,(int)strlen(str));
  else XDrawString(d,w,gc,x,y,str,(int)strlen(str));
}
static void render(void){
  box(0,0,680,368,bg);box(0,0,680,6,accent);
  label(36,43,"MELTY  /  LINUX EDITION",accent);
  label(419,43,ml_get(lang,M_LANG),gray);
  box(480,19,35,36,muted);label(491,43,"<",white);
  box(518,19,110,36,muted);label(529,43,ml_names[lang],white);
  box(630,19,35,36,muted);label(641,43,">",white);
  label(36,88,ml_get(lang,updating?M_UPDATE_TITLE:M_INSTALL_TITLE),white);
  label(36,119,ml_get(lang,updating?M_UPDATE_HELP:M_INSTALL_HELP),gray);
  box(36,142,608,1,border);
  label(36,170,ml_get(lang,M_INSTALL_PATH),white);
  label(36,196,ml_get(lang,M_INSTALL_SHORTCUT),gray);
  label(36,222,ml_get(lang,M_INSTALL_OAUTH),gray);
  label(36,247,ml_get(lang,M_INSTALL_NOPASS),gray);
  box(36,294,283,48,accent);
  label(49,323,ml_get(lang,updating?M_UPDATE_BUTTON:M_INSTALL_BUTTON),white);
  box(328,294,185,48,muted);label(340,323,ml_get(lang,M_RUN_BUTTON),white);
  box(523,294,121,48,muted);label(531,323,ml_get(lang,M_CANCEL),white);
  XFlush(d);
}
static void save_language(void){
  const char *home=getenv("HOME"), *config=getenv("XDG_CONFIG_HOME");
  if((!config||!*config)&&(!home||!*home))return;
  char path[PATH_MAX];
  if(config&&*config)snprintf(path,sizeof path,"%s/Melty",config);
  else snprintf(path,sizeof path,"%s/.config/Melty",home);
  /* XDG_CONFIG_HOME may not exist yet on a fresh desktop account. */
  char intermediate[PATH_MAX];snprintf(intermediate,sizeof intermediate,"%s",path);
  for(char *c=intermediate+1;*c;c++)if(*c=='/'){
    *c='\0';mkdir(intermediate,0700);*c='/';
  }
  mkdir(intermediate,0700);
  char file[PATH_MAX];
  if(strlen(path)+sizeof("/language")>=sizeof(file))return;
  snprintf(file,sizeof file,"%s/language",path);
  FILE *f=fopen(file,"w");if(f){fprintf(f,"%s\n",ml_codes[lang]);fclose(f);}
}
int main(int argc,char **argv){
  updating=argc>1&&!strcmp(argv[1],"--update");
  setlocale(LC_CTYPE,"");lang=ml_detect();
  d=XOpenDisplay(NULL);
  if(!d){fprintf(stderr,"Melty installer: no X display. Use --install.\n");return 4;}
  int screen=DefaultScreen(d);
  bg=rgb("#141517");white=rgb("#F4F4F4");gray=rgb("#ACB4C0");accent=rgb("#168BEC");muted=rgb("#353A41");border=rgb("#3C444D");
  w=XCreateSimpleWindow(d,RootWindow(d,screen),200,200,680,368,0,bg,bg);
  XStoreName(d,w,"Melty Linux - Installer");
  XSelectInput(d,w,ExposureMask|KeyPressMask|ButtonPressMask|StructureNotifyMask);
  wm_delete=XInternAtom(d,"WM_DELETE_WINDOW",False);XSetWMProtocols(d,w,&wm_delete,1);
  XSizeHints hints={0};hints.flags=PMinSize|PMaxSize;
  hints.min_width=hints.max_width=680;hints.min_height=hints.max_height=368;
  XSetWMNormalHints(d,w,&hints);
  gc=XCreateGC(d,w,0,NULL);
  char **missing=NULL;int missing_count=0;char *def=NULL;
  fontset=XCreateFontSet(d,"-*-*-medium-r-normal--14-*-*-*-*-*-*-*",&missing,&missing_count,&def);
  if(missing)XFreeStringList(missing);
  XFontStruct *font=NULL;
  if(!fontset){font=XLoadQueryFont(d,"fixed");if(font)XSetFont(d,gc,font->fid);}
  XMapRaised(d,w);render();
  int result=11;
  while(1){
    XEvent e;XNextEvent(d,&e);
    if(e.type==Expose){render();continue;}
    if(e.type==ClientMessage&&(Atom)e.xclient.data.l[0]==wm_delete)break;
    if(e.type==KeyPress){
      KeySym k=XLookupKeysym(&e.xkey,0);
      if(k==XK_Return||k==XK_KP_Enter){result=0;break;}
      if(k==XK_Escape){result=11;break;}
      if(k==XK_Left){lang=(lang+ML_LANG_COUNT-1)%ML_LANG_COUNT;render();}
      if(k==XK_Right){lang=(lang+1)%ML_LANG_COUNT;render();}
    }
    if(e.type==ButtonPress){
      int x=e.xbutton.x,y=e.xbutton.y;
      if(y>=19&&y<=55){
        if(x>=480&&x<=515){lang=(lang+ML_LANG_COUNT-1)%ML_LANG_COUNT;render();}
        if(x>=518&&x<=665){lang=(lang+1)%ML_LANG_COUNT;render();}
      }
      if(y>=294&&y<=342){
        if(x>=36&&x<=319){result=0;break;}
        if(x>=328&&x<=513){result=10;break;}
        if(x>=523&&x<=644){result=11;break;}
      }
    }
  }
  if(result==0||result==10)save_language();
  if(fontset)XFreeFontSet(d,fontset);
  if(font)XFreeFont(d,font);
  XFreeGC(d,gc);XDestroyWindow(d,w);XCloseDisplay(d);
  return result;
}
