/* $Id: c_color.c,v 1.26 2003/07/07 01:42:15 felixlee Exp $ */
/* New Color Control */

#include "icb.h"

/* colortoken takes a string and turns it into a color value */
static int
colortoken(token)
char *token;
{
  char *p;
  int hi = 0;
  int ret = -1;
  
  /* we can ignore any token less than 3 chars long out of hand,
     since the shortest legal token ("red") is 3 chars   */
  
  if (strlen(token) < 3)
    return(-1);

  p = token;

  /* we don't need to check hi and normal colors separately, just detect
     and remember the prefix */
  
  if (!strncasecmp(token, "hi", 2))
    {
      hi = 8;
      p += (2 * sizeof(char));
    }
  
  /* now we're ready to rock'n'roll. */
  
  switch (*p) {
  case 'b':
    if (!strcasecmp(p, "black"))
      ret = hi;
    else
      if (!strcasecmp(p, "blue"))
	ret = hi + 4;
    break;
  case 'c':
    if (!strcasecmp(p, "cyan"))
      ret = hi + 6;
    break;
  case 'g':
    if (!strcasecmp(p, "green"))
      ret = hi + 2;
    break;
  case 'm':
    if (!strcasecmp(p, "magenta"))
      ret = hi + 5;
    break;
  case 'r':
    if (!strcasecmp(p, "red"))
      ret = hi + 1;
    break;
  case 'w':
    if (!strcasecmp(p, "white"))
      ret = hi + 7;
    break;
  case 'y':
    if (!strcasecmp(p, "yellow"))
      ret = hi + 3;
    break;
  default:
    break;
  }
  
  return (ret);
}


/* whichcolor takes a string and turns it into a color type */
static int
whichcolor(token)
char *token;
{
  char *p;
  int ret = -1;

  /* again, our minimum valid token is 3 chars long */
  
  if (strlen(token) < 3)
    return(-1);
  
  p = token;

  /* rock and roll. */
  
  switch (*p) {
  case 'a':
    if (!strcasecmp(token, "abrkt"))
      ret = ColABRKT;
    else
      if (!strcasecmp(token, "address"))
	ret = ColADDRESS;
    break;
  case 'b':
    if (!strcasecmp(token, "beep"))
      ret = ColBEEP;
    break;
  case 'e':
    if (!strcasecmp(token, "error"))
      ret = ColERROR;
    break;
  case 'i':
    if (!strcasecmp(token, "idletime"))
      ret = ColIDLETIME;
    break;
  case 'l':
    if (!strcasecmp(token, "logintime"))
      ret = ColLOGINTIME;
    break;
  case 'm':
    if (!strcasecmp(token, "mod"))
      ret = ColMOD;
    else
      if (!strcasecmp(token, "more"))
	ret = ColMORE;
    break;
  case 'n':
    if (!strcasecmp(token, "nickname"))
      ret = ColNICKNAME;
    else
      if (!strcasecmp(token, "normal"))
	ret = ColNORMAL;
    else
      if (!strcasecmp(token, "notice"))
	ret = ColNOTICE;
    break;
  case 'p':
    if (!strcasecmp(token, "pbrkt"))
      ret = ColPBRKT;
    else
      if (!strcasecmp(token, "persfrom"))
	ret = ColPERSFROM;
    else
      if (!strcasecmp(token, "persfromhilite"))
	ret = ColPERSFROMHILITE;
    else
      if (!strcasecmp(token, "pershilite"))
	ret = ColPERSHILITE;
    else
      if (!strcasecmp(token, "personal"))
	ret = ColPERSONAL;
    break;
  case 's':
    if (!strcasecmp(token, "sane"))
      ret = -1;                   /* Let's keep ColSANE inviolate for now */
    else
      if (!strcasecmp(token, "sbrkt"))
	ret = ColSBRKT;
    else
      if (!strcasecmp(token, "status"))
	ret = ColSTATUS;
    break;
  case 't':
    if (!strcasecmp(token, "timestamp"))
      ret = ColTIMESTAMP;
    break;
  case 'u':
    if (!strcasecmp(token, "unreg"))
      ret = ColUNREG;
    break;
  case 'w':
    if (!strcasecmp(token, "warning"))
      ret = ColWARNING;
    else
      if (!strcasecmp(token, "whead"))
	ret = ColWHEAD;
    else
      if (!strcasecmp(token, "wsub"))
	ret = ColWSUB;
    break;
  default:
    break;
    }  

  return (ret);
}


/* setcolor function sets a specific color */
int
setcolor(colortype,fg,bg)
int colortype; int fg; int bg;
{
	char cbuf[12]; 	

	if (colortype == -1) return 0;
	if (fg==-1) {
		colortable[colortype].defined=0;
		return 0;
	}
	if(bg==-1) {
		if(fg < 8) sprintf(cbuf,"\033[3%im",fg); 
		else sprintf(cbuf,"\033[1;3%im",fg-8);
	} else {
		if(bg < 8) {  
		   if(fg < 8) sprintf(cbuf,"\033[3%i;4%im",fg,bg); 
		   else sprintf(cbuf,"\033[1;3%i;4%im",fg-8,bg);
		} else {
		   if(fg < 8) sprintf(cbuf,"\033[3%i;4%im",fg,bg-8); 
		   else sprintf(cbuf,"\033[1;3%i;4%im",fg-8,bg-8);
		}
	}
	strcpy(colortable[colortype].color,cbuf);
	colortable[colortype].defined=1;
	return 0;
}

/* printcolor returns an ansi sequence based on the selected color */
/* suitable for use in a printf if that color is marked as defined */
char *printcolor(primary, fallback)
int primary;
int fallback;
{
	if (colortable[primary].defined && gv.colorize)
		return colortable[primary].color;
	else
	if (colortable[fallback].defined && gv.colorize)
		return colortable[fallback].color;
	else
		return ("");
}


int
c_color (ARGV_TCL)
{
  static char *usage = "usage: c_color [color] [foreground] [background]";
  int c, t1, t2;

	if ((argc == 3) || (argc == 4)) {
	  c = whichcolor(argv[1]);
	  t1 = colortoken(argv[2]);
	  t2 = (argc == 4 ? colortoken(argv[3]) : -1);
          setcolor(c,t1,t2);
	  return(TCL_OK);
	} else
	  TRETURNERR(usage)
}
