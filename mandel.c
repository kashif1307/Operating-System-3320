
/*
Name: Kashif Hussain
UTA ID:1001409065
NetID: kxh9065
*/
#include "bitmap.h"

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

int iteration_to_color( int i, int max );
int iterations_at_point( double x, double y, int max );

void compute_image(void *ptr); //function that utilizes pthreads to compute the mandelbrot image

void show_help()
{
	printf("Use: mandel [options]\n");
	printf("Where options are:\n");
	printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
	printf("-x <coord>  X coordinate of image center point. (default=0)\n");
	printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
	printf("-s <scale>  Scale of the image in Mandlebrot coordinates. (default=4)\n");
	printf("-W <pixels> Width of the image in pixels. (default=500)\n");
	printf("-H <pixels> Height of the image in pixels. (default=500)\n");
	printf("-o <file>   Set output file. (default=mandel.bmp)\n");
	printf("-h          Show this help text.\n");
	printf("\nSome examples are:\n");
	printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
	printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
	printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}


struct compute_image_args // struct to pass to compute_image function for utilizing pthreads
 {
   struct bitmap *bmp;
   double xmin;
   double xmax;
   double ymin;
   double ymax;
   int max;
   int height_begin;
   int height_end;   
};

int main( int argc, char *argv[] )
{
	char c;

	// These are the default configuration values used
	// if no command line arguments are given.

	const char *outfile = "mandel.bmp";
	double xcenter = 0;
	double ycenter = 0;
	double scale = 4;
	int    image_width = 500;
	int    image_height = 500;
	int    max = 1000;
        int    num_threads = 1;

	// For each command line argument given,
	// override the appropriate configuration value.

	while((c = getopt(argc,argv,"x:y:s:W:H:m:n:o:h"))!=-1) {
		switch(c) {
			case 'x':
				xcenter = atof(optarg);
				break;
			case 'y':
				ycenter = atof(optarg);
				break;
			case 's':
				scale = atof(optarg);
				break;
			case 'W':
				image_width = atoi(optarg);
				break;
			case 'H':
				image_height = atoi(optarg);
				break;
			case 'm':
				max = atoi(optarg);
				break;
                        case 'n':
				num_threads = atoi(optarg); // command line argument for number of threads
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'h':
				show_help();
				exit(1);
				break;
			
		}
	}

	// Display the configuration of the image.
	printf("mandel: x=%lf y=%lf scale=%lf max=%d outfile=%s\n",xcenter,ycenter,scale,max,outfile);

	// Create a bitmap of the appropriate size.
	struct bitmap *bm = bitmap_create(image_width,image_height);
         

	// Fill it with a dark blue, for debugging
	bitmap_reset(bm,MAKE_RGBA(0,0,255,0));

	// Compute the Mandelbrot image
	//compute_image(bm,xcenter-scale,xcenter+scale,ycenter-scale,ycenter+scale,max);

         int thread;
         printf("Num threads = %d\n", num_threads);
         pthread_t* pthreadArr = malloc(num_threads*sizeof(pthread_t));//dynamic allocation of user-specific no. of threads
         struct compute_image_args *ptrs=(struct compute_image_args*)malloc(sizeof(struct compute_image_args));
         //dynamically allocating a pointer array to kepe track of structures for threads
         double blocksize = image_height / num_threads;//dividing image into portions for thread use
         int blkcheck=image_height%num_threads;
        //if there is no remainder in division, it follows the if structure below
        if(blkcheck==0)
        {
         for (thread=0; thread<num_threads;thread++)
	{
         struct compute_image_args *threadargs= (struct compute_image_args*)malloc(sizeof(struct compute_image_args));
         threadargs->bmp=bm;
         threadargs->xmin=(xcenter-scale);
         threadargs->xmax=(xcenter+scale);
         threadargs->ymin=(ycenter-scale);
         threadargs->ymax=(ycenter+scale);
         threadargs->max=max;
         threadargs->height_begin = thread*blocksize; //dividing height into different portions for thread use
         threadargs->height_end = (thread+1)*blocksize;
         pthread_create(&(pthreadArr[thread]), NULL, (void *)compute_image,(void *)threadargs);
         ptrs=threadargs;
        }

        int b;
        for(b=0; b<num_threads; b++)
        {
         pthread_join(pthreadArr[b], NULL);
        }

        }
     // this else condition ensures a division with remainder will be taken into consideration for threads to utilize
     // the entire image.

      else   
     {
     for (thread=0; thread<num_threads;thread++)
	{
         struct compute_image_args *threadargs= (struct compute_image_args*)malloc(sizeof(struct compute_image_args));
         threadargs->bmp=bm;
         threadargs->xmin=(xcenter-scale);
         threadargs->xmax=(xcenter+scale);
         threadargs->ymin=(ycenter-scale);
         threadargs->ymax=(ycenter+scale);
         threadargs->max=max;
         threadargs->height_begin = thread*blocksize;
         if(thread==num_threads-1)
        {
         threadargs->height_end = ((thread+1)*(blocksize))+blkcheck; 
        // makes sure last thread takes into consideration the remainder from the image_height division
        }
        else 
         {
           threadargs->height_end = (thread+1)*(blocksize);
         }
         pthread_create(&(pthreadArr[thread]), NULL, (void *)compute_image,(void *)threadargs);
         ptrs=threadargs;
        }

        int d;
        for(d=0; d<num_threads; d++)
        {
         pthread_join(pthreadArr[d], NULL);
        }


   }

	// Save the image in the stated file.
	if(!bitmap_save(bm,outfile)) {
		fprintf(stderr,"mandel: couldn't write to %s: %s\n",outfile,strerror(errno));
		return 1;
	}
        free(pthreadArr);
        free(ptrs);
        


	return 0;
}

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/

  void compute_image( void* ptr)
 {
   
       struct compute_image_args *args=(struct compute_image_args*)ptr; 
//assign compute_args struct pointer to a new pointer variable for use in this function.

  
	int i,j;
        
        int width=bitmap_width(args->bmp);
        int height = bitmap_height(args->bmp);
        

	

	// For every pixel in the image...


	for(j = args->height_begin;j<args->height_end;j++) {

		for(i=0;i<width;i++) {

			// Determine the point in x,y space for that pixel.
			double x = args->xmin + i*((args->xmax)-(args->xmin))/width;
			double y = args->ymin + j*((args->ymax)-(args->ymin))/height;

			// Compute the iterations at that point.
			int iters = iterations_at_point(x,y,args->max);

			// Set the pixel in the bitmap.
			bitmap_set(args->bmp,i,j,iters);
		}
	}
    
}









/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/

int iterations_at_point( double x, double y, int max )
{
	double x0 = x;
	double y0 = y;

	int iter = 0;

	while( (x*x + y*y <= 4) && iter < max ) {

		double xt = x*x - y*y + x0;
		double yt = 2*x*y + y0;

		x = xt;
		y = yt;

		iter++;
	}

	return iteration_to_color(iter,max);
}

/*
Convert a iteration number to an RGBA color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/

int iteration_to_color( int i, int max )
{
	int gray = 255*i/max;
	return MAKE_RGBA(gray,gray,gray,0);
}




