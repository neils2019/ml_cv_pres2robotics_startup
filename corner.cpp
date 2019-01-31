#define MAX_CORNERS   15000
typedef  struct {int x,y,info, dx, dy, I;} CORNER_LIST[MAX_CORNERS];

#define SEVEN_SUPP           /* size for non-max corner suppression; SEVEN_SUPP or FIVE_SUPP */

#include <iostream>
#include <cv.h>
#include <highgui.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/file.h>    /* may want to remove this line */
#include <malloc.h>      /* may want to remove this line */
#define  exit_error(IFB,IFC) { fprintf(stderr,IFB,IFC); exit(0); }
#define  FTOI(a) ( (a) < 0 ? ((int)(a-0.5)) : ((int)(a+0.5)) )
typedef  unsigned char uchar;
typedef  struct {int x,y,info, dx, dy, I;} CORNER_LIST[MAX_CORNERS];


void usage(void)
{
  printf("Usage: susan <in.pgm> <out.pgm> [options]\n\n");

  printf("-s : Smoothing mode (default)\n");
  printf("-e : Edges mode\n");
  printf("-c : Corners mode\n\n");

  printf("See source code for more information about setting the thresholds\n");
  printf("-t <thresh> : Brightness threshold, all modes (default=20)\n");
  printf("-d <thresh> : Distance threshold, smoothing mode, (default=4) (use next option instead for flat 3x3 mask)\n");
  printf("-3 : Use flat 3x3 mask, edges or smoothing mode\n");
  printf("-n : No post-processing on the binary edge map (runs much faster); edges mode\n");
  printf("-q : Use faster (and usually stabler) corner mode; edge-like corner suppression not carried out; corners mode\n");
  printf("-b : Mark corners/edges with single black points instead of black with white border; corners or edges mode\n");
  printf("-p : Output initial enhancement image only; corners or edges mode (default is edges mode)\n");

  printf("\nSUSAN Version 2l (C) 1995-1997 Stephen Smith, DRA UK. steve@fmrib.ox.ac.uk\n");

  exit(0);
}


void get_image(char *filename, Mat &image, unsigned char **in,int *x_size,int *y_size)
//  char           filename[200];
//  unsigned char  **in;
//  int            *x_size, *y_size;
{

 image = imread(filename, 0);
 
 *x_size = image.size().width;
 *y_size = image.size().height;

 *in = (uchar *) malloc((*x_size) * (*y_size));
 memcpy(*in,image.data,(*x_size)*(*y_size)*sizeof(unsigned char));
}


void put_image(char *filename,char *in,int x_size,int y_size)
/*  char filename [100],
       *in;
  int  x_size,
  y_size;*/
{
FILE  *fd;

#ifdef FOPENB
  if ((fd=fopen(filename,"wb")) == NULL) 
#else
  if ((fd=fopen(filename,"w")) == NULL) 
#endif
    exit_error("Can't output image%s.\n",filename);

  fprintf(fd,"P5\n");
  fprintf(fd,"%d %d\n",x_size,y_size);
  fprintf(fd,"255\n");
  
  if (fwrite(in,x_size*y_size,1,fd) != 1)
    exit_error("Can't write image %s.\n",filename);

  fclose(fd);
}



susan_corners_quick(unsigned char *in,int *r,unsigned char *bp,int max_no,CORNER_LIST corner_list,int *x_size,int *y_size)
/*  uchar       *in, *bp;
  int         *r, max_no, x_size, y_size;
  CORNER_LIST corner_list;*/
{
  int   n,x,y,i,j;
  uchar *p,*cp;

  memset (r,0,x_size * y_size * sizeof(int));

  for (i=7;i<y_size-7;i++)
    for (j=7;j<x_size-7;j++) {
      n=100;
      p=in + (i-3)*x_size + j - 1;
      cp=bp + in[i*x_size+j];
      
      n+=*(cp-*p++);
      n+=*(cp-*p++);
      n+=*(cp-*p);
      p+=x_size-3;

      n+=*(cp-*p++);
      n+=*(cp-*p++);
      n+=*(cp-*p++);
      n+=*(cp-*p++);
      n+=*(cp-*p);
      p+=x_size-5;

      n+=*(cp-*p++);
      n+=*(cp-*p++);
      n+=*(cp-*p++);
      n+=*(cp-*p++);
      n+=*(cp-*p++);
      n+=*(cp-*p++);
      n+=*(cp-*p);
      p+=x_size-6;
      
      n+=*(cp-*p++);
      n+=*(cp-*p++);
      n+=*(cp-*p);
      if (n<max_no){
        p+=2;
        n+=*(cp-*p++);
	if (n<max_no){
        n+=*(cp-*p++);
	if (n<max_no){
	  n+=*(cp-*p);
	  if (n<max_no){
	    p+=x_size-6;
	    
	    n+=*(cp-*p++);
	    if (n<max_no){
	      n+=*(cp-*p++);
	      if (n<max_no){
		n+=*(cp-*p++);
		if (n<max_no){
		  n+=*(cp-*p++);
		  if (n<max_no){
		    n+=*(cp-*p++);
		    if (n<max_no){
		      n+=*(cp-*p++);
		      if (n<max_no){
			n+=*(cp-*p);
			if (n<max_no){
			  p+=x_size-5;
			  
			  n+=*(cp-*p++);
			  if (n<max_no){
			    n+=*(cp-*p++);
			    if (n<max_no){
			      n+=*(cp-*p++);
			      if (n<max_no){
				n+=*(cp-*p++);
				if (n<max_no){
				  n+=*(cp-*p);
				  if (n<max_no){
				    p+=x_size-3;
	
				    n+=*(cp-*p++);
				    if (n<max_no){
				      n+=*(cp-*p++);
				      if (n<max_no){
					n+=*(cp-*p);

					if (n<max_no)
					  r[i*x_size+j] = max_no-n;
}}}}}}}}}}}}}}}}}}}
  
  /* to locate the local maxima */
  n=0;
  for (i=7;i<y_size-7;i++)
    for (j=7;j<x_size-7;j++) {
      x = r[i*x_size+j];
      if (x>0)  {
	/* 5x5 mask */
#ifdef FIVE_SUPP
	if (
	    (x>r[(i-1)*x_size+j+2]) &&
	    (x>r[(i  )*x_size+j+1]) &&
	    (x>r[(i  )*x_size+j+2]) &&
	    (x>r[(i+1)*x_size+j-1]) &&
	    (x>r[(i+1)*x_size+j  ]) &&
	    (x>r[(i+1)*x_size+j+1]) &&
	    (x>r[(i+1)*x_size+j+2]) &&
	    (x>r[(i+2)*x_size+j-2]) &&
	    (x>r[(i+2)*x_size+j-1]) &&
	    (x>r[(i+2)*x_size+j  ]) &&
	    (x>r[(i+2)*x_size+j+1]) &&
	    (x>r[(i+2)*x_size+j+2]) &&
	    (x>=r[(i-2)*x_size+j-2]) &&
	    (x>=r[(i-2)*x_size+j-1]) &&
	    (x>=r[(i-2)*x_size+j  ]) &&
	    (x>=r[(i-2)*x_size+j+1]) &&
	    (x>=r[(i-2)*x_size+j+2]) &&
	    (x>=r[(i-1)*x_size+j-2]) &&
	    (x>=r[(i-1)*x_size+j-1]) &&
	    (x>=r[(i-1)*x_size+j  ]) &&
	    (x>=r[(i-1)*x_size+j+1]) &&
	    (x>=r[(i  )*x_size+j-2]) &&
	    (x>=r[(i  )*x_size+j-1]) &&
	    (x>=r[(i+1)*x_size+j-2]) )
#endif
#ifdef SEVEN_SUPP
          if ( 
	      (x>r[(i-3)*x_size+j-3]) &&
	      (x>r[(i-3)*x_size+j-2]) &&
	      (x>r[(i-3)*x_size+j-1]) &&
	      (x>r[(i-3)*x_size+j  ]) &&
	      (x>r[(i-3)*x_size+j+1]) &&
	      (x>r[(i-3)*x_size+j+2]) &&
	      (x>r[(i-3)*x_size+j+3]) &&
	      
	      (x>r[(i-2)*x_size+j-3]) &&
	      (x>r[(i-2)*x_size+j-2]) &&
	      (x>r[(i-2)*x_size+j-1]) &&
	      (x>r[(i-2)*x_size+j  ]) &&
	      (x>r[(i-2)*x_size+j+1]) &&
	      (x>r[(i-2)*x_size+j+2]) &&
	      (x>r[(i-2)*x_size+j+3]) &&
	      
	      (x>r[(i-1)*x_size+j-3]) &&
	      (x>r[(i-1)*x_size+j-2]) &&
	      (x>r[(i-1)*x_size+j-1]) &&
	      (x>r[(i-1)*x_size+j  ]) &&
	      (x>r[(i-1)*x_size+j+1]) &&
	      (x>r[(i-1)*x_size+j+2]) &&
	      (x>r[(i-1)*x_size+j+3]) &&
	      
	      (x>r[(i)*x_size+j-3]) &&
	      (x>r[(i)*x_size+j-2]) &&
	      (x>r[(i)*x_size+j-1]) &&
	      (x>=r[(i)*x_size+j+1]) &&
	      (x>=r[(i)*x_size+j+2]) &&
	      (x>=r[(i)*x_size+j+3]) &&
	      
	      (x>=r[(i+1)*x_size+j-3]) &&
	      (x>=r[(i+1)*x_size+j-2]) &&
	      (x>=r[(i+1)*x_size+j-1]) &&
	      (x>=r[(i+1)*x_size+j  ]) &&
	      (x>=r[(i+1)*x_size+j+1]) &&
	      (x>=r[(i+1)*x_size+j+2]) &&
	      (x>=r[(i+1)*x_size+j+3]) &&
	      
	      (x>=r[(i+2)*x_size+j-3]) &&
	      (x>=r[(i+2)*x_size+j-2]) &&
	      (x>=r[(i+2)*x_size+j-1]) &&
	      (x>=r[(i+2)*x_size+j  ]) &&
	      (x>=r[(i+2)*x_size+j+1]) &&
	      (x>=r[(i+2)*x_size+j+2]) &&
	      (x>=r[(i+2)*x_size+j+3]) &&
	      
	      (x>=r[(i+3)*x_size+j-3]) &&
	      (x>=r[(i+3)*x_size+j-2]) &&
	      (x>=r[(i+3)*x_size+j-1]) &&
	      (x>=r[(i+3)*x_size+j  ]) &&
	      (x>=r[(i+3)*x_size+j+1]) &&
	      (x>=r[(i+3)*x_size+j+2]) &&
	      (x>=r[(i+3)*x_size+j+3]) )
#endif
	    {
	      corner_list[n].info=0;
	      corner_list[n].x=j;
	      corner_list[n].y=i;
	      x = in[(i-2)*x_size+j-2] + in[(i-2)*x_size+j-1] + in[(i-2)*x_size+j] + in[(i-2)*x_size+j+1] + in[(i-2)*x_size+j+2] +
		in[(i-1)*x_size+j-2] + in[(i-1)*x_size+j-1] + in[(i-1)*x_size+j] + in[(i-1)*x_size+j+1] + in[(i-1)*x_size+j+2] +
		in[(i  )*x_size+j-2] + in[(i  )*x_size+j-1] + in[(i  )*x_size+j] + in[(i  )*x_size+j+1] + in[(i  )*x_size+j+2] +
		in[(i+1)*x_size+j-2] + in[(i+1)*x_size+j-1] + in[(i+1)*x_size+j] + in[(i+1)*x_size+j+1] + in[(i+1)*x_size+j+2] +
		in[(i+2)*x_size+j-2] + in[(i+2)*x_size+j-1] + in[(i+2)*x_size+j] + in[(i+2)*x_size+j+1] + in[(i+2)*x_size+j+2];
	      
	      corner_list[n].I=x/25;
	      /*corner_list[n].I=in[i*x_size+j];*/
	      x = in[(i-2)*x_size+j+2] + in[(i-1)*x_size+j+2] + in[(i)*x_size+j+2] + in[(i+1)*x_size+j+2] + in[(i+2)*x_size+j+2] -
		(in[(i-2)*x_size+j-2] + in[(i-1)*x_size+j-2] + in[(i)*x_size+j-2] + in[(i+1)*x_size+j-2] + in[(i+2)*x_size+j-2]);
	      x += x + in[(i-2)*x_size+j+1] + in[(i-1)*x_size+j+1] + in[(i)*x_size+j+1] + in[(i+1)*x_size+j+1] + in[(i+2)*x_size+j+1] -
		(in[(i-2)*x_size+j-1] + in[(i-1)*x_size+j-1] + in[(i)*x_size+j-1] + in[(i+1)*x_size+j-1] + in[(i+2)*x_size+j-1]);
	      
	      y = in[(i+2)*x_size+j-2] + in[(i+2)*x_size+j-1] + in[(i+2)*x_size+j] + in[(i+2)*x_size+j+1] + in[(i+2)*x_size+j+2] -
		(in[(i-2)*x_size+j-2] + in[(i-2)*x_size+j-1] + in[(i-2)*x_size+j] + in[(i-2)*x_size+j+1] + in[(i-2)*x_size+j+2]);
	      y += y + in[(i+1)*x_size+j-2] + in[(i+1)*x_size+j-1] + in[(i+1)*x_size+j] + in[(i+1)*x_size+j+1] + in[(i+1)*x_size+j+2] -
		(in[(i-1)*x_size+j-2] + in[(i-1)*x_size+j-1] + in[(i-1)*x_size+j] + in[(i-1)*x_size+j+1] + in[(i-1)*x_size+j+2]);
	      corner_list[n].dx=x/15;
	      corner_list[n].dy=y/15;
	      n++;
	      if(n==MAX_CORNERS){
		fprintf(stderr,"Too many corners.\n");
		exit(1);
	      }}}}
  corner_list[n].info=7;
}


void corner_draw(unsigned char *in,CORNER_LIST corner_list,int x_size,int drawing_mode)
/*  uchar *in;
  CORNER_LIST corner_list;
  int x_size, drawing_mode; */
{
uchar *p;
int   n=0;

  while(corner_list[n].info != 7)
  {
    if (drawing_mode==0)
    {
      p = in + (corner_list[n].y-1)*x_size + corner_list[n].x - 1;
      *p++=255; *p++=255; *p=255; p+=x_size-2;
      *p++=255; *p++=0;   *p=255; p+=x_size-2;
      *p++=255; *p++=255; *p=255;
      n++;
    }
    else
    {
      p = in + corner_list[n].y*x_size + corner_list[n].x;
      *p=0;
      n++;
    }
  }
}


int main(int argc, char *argv[])
{

  FILE   *ofp;
  char   filename [80],
    *tcp;
  unsigned char  *in, *bp, *mid;
  float  dt=4.0;
  int    *r,
    argindex=3,
    bt=20,
    principle=0,
    thin_post_proc=1,
    three_by_three=0,
    drawing_mode=0,
    susan_quick=0,
    max_no_corners=1850,
    max_no_edges=2650,
    mode = 0, i,
    x_size, y_size;

  Mat image;
  CORNER_LIST corner_list;

/* }}} */



  if (argc<3)
    usage();

  get_image(argv[1],&in,&x_size,&y_size);
  
  r   = (int *) malloc(x_size * y_size * sizeof(int));
  setup_brightness_lut(&bp,bt,6);

      //      if (principle)
      //      {
      /*        susan_principle(in,r,bp,max_no_corners,x_size,y_size);
		int_to_uchar(r,in,x_size*y_size); */

  susan_corners_quick(in,r,bp,max_no_corners,corner_list,x_size,y_size);
  corner_draw(in,corner_list,x_size,drawing_mode);

	/*      }
      else
      {
        if(susan_quick)
          susan_corners_quick(in,r,bp,max_no_corners,corner_list,x_size,y_size);
        else
          susan_corners(in,r,bp,max_no_corners,corner_list,x_size,y_size);
        corner_draw(in,corner_list,x_size,drawing_mode);
      }
	*/
  put_image(argv[2],in,x_size,y_size);
  return 0;
}
