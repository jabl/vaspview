/*VASP Data Viewer - Viewer 3d data sets of molecular charge distribution
  Copyright (C) 1999 Timothy B. Terriberry (mailto:tterribe@vt.edu)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA*/
#if defined(WIN32)
# undef  WINGDIAPI
# undef  APIENTRY
# undef  STRICT
# define WIN32_LEAN_AND_MEAN
# define WIN32_EXTRA_LEAN
# include <windows.h>

# define MAX_ARGV           40
# define MAX_MODULE_NAME_SZ 260

/*Initial show command, kept for win32-specific code*/
int win32_cmd_show=SW_SHOW;



extern int main(int _argc,char *_argv[]);

/*Parses the command line into discreet arguments, since Windows does not do
  it for us*/
static int parseCommandLine(const char *_module_name,const char *_cmd_line,
                            int *_pargc,char *_argv[],int _max_argv){
 static char str[4096];
 static char filename[MAX_MODULE_NAME_SZ];
 char *prev_word=NULL;
 char *cmd_line=strncpy(str,_cmd_line,sizeof(str)-1);
 BOOL  in_quote=FALSE;
 BOOL  no_strip=FALSE;
 int   argc=0;
 strncpy(filename,_module_name,sizeof(filename)-1)[sizeof(filename)-1]='\0';
 _argv[argc++]=filename;
 cmd_line[sizeof(str)-1]='\0';
 while(*cmd_line!='\0'){
  switch(*cmd_line){
   case '"' :{
    if(prev_word!=NULL){
     if(in_quote){
      if(!no_strip)*cmd_line='\0';
      _argv[argc++]=prev_word;
      prev_word=NULL;}
     else no_strip=TRUE;}
    in_quote=!in_quote;}break;
   case ' ' :
   case '\t':{
    if(!in_quote){
     if(prev_word!=NULL){
      *cmd_line='\0';
      _argv[argc++]=prev_word;
      prev_word=NULL;
      no_strip=FALSE;} } }break;
   default  :{
    if(prev_word==NULL)prev_word=cmd_line;break;} }
  if(argc>=_max_argv-1)break;
  cmd_line++;}
 if((prev_word!=NULL||(in_quote&&prev_word!=NULL))&&argc<_max_argv-1){
  *cmd_line='\0';
  _argv[argc++]=prev_word;}
 _argv[argc]=NULL;
 /*Return updated parameters*/
 return (*_pargc=argc);}


/*Dispatches program execution to the ANSI main() function*/
int PASCAL WinMain(HINSTANCE _hinst,HINSTANCE _hprev,
                   LPSTR _cmd_line,int _cmd_show){
 char  module_name[MAX_MODULE_NAME_SZ];
 char *argv[MAX_ARGV];
 int   argc;
 win32_cmd_show=_cmd_show;
 GetModuleFileName(_hinst,module_name,MAX_MODULE_NAME_SZ);
 parseCommandLine(module_name,_cmd_line,&argc,argv,MAX_ARGV);
 return main(argc,argv);}
#endif