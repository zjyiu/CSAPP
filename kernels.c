/********************************************************
 * Kernels to be optimized for the CS:APP Performance Lab
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

/* 
 * Please fill in the following team struct 
 */
team_t team = {
    "张钧尧",              /* Team name */

    "19307130226",     /* First member full name */
    "19307130226@fudan.edu.cn",  /* First member email address */

    "",                   /* Second member full name (leave blank if none) */
    ""                    /* Second member email addr (leave blank if none) */
};

/***************
 * ROTATE KERNEL
 ***************/

/******************************************************
 * Your different versions of the rotate kernel go here
 ******************************************************/
 char my_rotate_descr[] = "my_rotate: my own working version";
 void my_rotate(int dim, pixel *src, pixel *dst) {
      int i, j;
    for (i=0; i < dim; i+=16)
	for(j=0;j<dim;j++){
	    int k=dim-j-1;
	    dst[k*dim+ i]=src[i*dim+j]; 
	    dst[k*dim+ i+1]=src[(i+1)*dim+j];
	    dst[k*dim+ i+2]=src[(i+2)*dim+j];
	    dst[k*dim+ i+3]=src[(i+3)*dim+j];
	    dst[k*dim+ i+4]=src[(i+4)*dim+j]; 
	    dst[k*dim+ i+5]=src[(i+5)*dim+j]; 
	    dst[k*dim+ i+6]=src[(i+6)*dim+j]; 
	    dst[k*dim+ i+7]=src[(i+7)*dim+j]; 
	    dst[k*dim+ i+8]=src[(i+8)*dim+j]; 
	    dst[k*dim+ i+9]=src[(i+9)*dim+j]; 
	    dst[k*dim+ i+10]=src[(i+10)*dim+j]; 
	    dst[k*dim+ i+11]=src[(i+11)*dim+j];
	    dst[k*dim+ i+12]=src[(i+12)*dim+j]; 
	    dst[k*dim+ i+13]=src[(i+13)*dim+j]; 
	    dst[k*dim+ i+14]=src[(i+14)*dim+j]; 
	    dst[k*dim+ i+15]=src[(i+15)*dim+j];
	}
	
 }

/* 
 * naive_rotate - The naive baseline version of rotate 
 */
char naive_rotate_descr[] = "naive_rotate: Naive baseline implementation";
void naive_rotate(int dim, pixel *src, pixel *dst) 
{
   int i, j;

    for (i = 0; i < dim; i++)
	for (j = 0; j < dim; j++)
	    dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
}
/* 
 * rotate - Your current working version of rotate
 * IMPORTANT: This is the version you will be graded on
 */
char rotate_descr[] = "rotate: Current working version";
void rotate(int dim, pixel *src, pixel *dst) 
{
    naive_rotate(dim, src, dst);
}

/*********************************************************************
 * register_rotate_functions - Register all of your different versions
 *     of the rotate kernel with the driver by calling the
 *     add_rotate_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_rotate_functions() 
{   add_rotate_function(&rotate, rotate_descr); 
    add_rotate_function(&naive_rotate, naive_rotate_descr);   
    /* ... Register additional test functions here */
    add_rotate_function(&my_rotate, my_rotate_descr); 
}


/***************
 * SMOOTH KERNEL
 **************/

/***************************************************************
 * Various typedefs and helper functions for the smooth function
 * You may modify these any way you like.
 **************************************************************/

/* A struct used to compute averaged pixel value */
typedef struct {
    int red;
    int green;
    int blue;
    int num;
} pixel_sum;

/* Compute min and max of two integers, respectively */
static int min(int a, int b) { return (a < b ? a : b); }
static int max(int a, int b) { return (a > b ? a : b); }

/* 
 * initialize_pixel_sum - Initializes all fields of sum to 0 
 */
static void initialize_pixel_sum(pixel_sum *sum) 
{
    sum->red = sum->green = sum->blue = 0;
    sum->num = 0;
    return;
}

/* 
 * accumulate_sum - Accumulates field values of p in corresponding 
 * fields of sum 
 */
static void accumulate_sum(pixel_sum *sum, pixel p) 
{
    sum->red += (int) p.red;
    sum->green += (int) p.green;
    sum->blue += (int) p.blue;
    sum->num++;
    return;
}

/* 
 * assign_sum_to_pixel - Computes averaged pixel value in current_pixel 
 */
static void assign_sum_to_pixel(pixel *current_pixel, pixel_sum sum) 
{
    current_pixel->red = (unsigned short) (sum.red/sum.num);
    current_pixel->green = (unsigned short) (sum.green/sum.num);
    current_pixel->blue = (unsigned short) (sum.blue/sum.num);
    return;
}

/* 
 * avg - Returns averaged pixel value at (i,j) 
 */
static pixel avg(int dim, int i, int j, pixel *src) 
{
    int ii, jj;
    pixel_sum sum;
    pixel current_pixel;

    initialize_pixel_sum(&sum);
    for(ii = max(i-1, 0); ii <= min(i+1, dim-1); ii++) 
	for(jj = max(j-1, 0); jj <= min(j+1, dim-1); jj++) 
	    accumulate_sum(&sum, src[RIDX(ii, jj, dim)]);

    assign_sum_to_pixel(&current_pixel, sum);
    return current_pixel;
}


/******************************************************
 * Your different versions of the smooth kernel go here
 ******************************************************/
 char my_smooth_descr[] = "my_smooth: My own working version";
 void my_smooth(int dim, pixel *src, pixel *dst){
    int i, j;
    pixel_sum sum;
    sum.red = sum.green = sum.blue = 0;
        sum.red+=(int)(src[0].red+src[1].red+src[dim].red+src[dim+1].red);
        sum.blue+=(int)(src[0].blue+src[1].blue+src[dim].blue+src[dim+1].blue);
        sum.green+=(int)(src[0].green+src[1].green+src[dim].green+src[dim+1].green);
        pixel current_pixel;
       current_pixel.red = (unsigned short) (sum.red/4);
       current_pixel.green = (unsigned short) (sum.green/4);
       current_pixel.blue = (unsigned short) (sum.blue/4);
       dst[0]=current_pixel;//计算第一排第一个元素
	sum.red+=(int)(src[2].red+src[dim+2].red);
        sum.blue+=(int)(src[2].blue+src[dim+2].blue);
        sum.green+=(int)(src[2].green+src[dim+2].green);
       current_pixel.red = (unsigned short) (sum.red/6);
       current_pixel.green = (unsigned short) (sum.green/6);
       current_pixel.blue = (unsigned short) (sum.blue/6);
       dst[1]=current_pixel;//计算第一排第二个元素
	for (j = 2; j < dim-1; j++){
	sum.red-=(int)(src[j-2].red+src[dim+j-2].red);
        sum.blue-=(int)(src[j-2].blue+src[dim+j-2].blue);
        sum.green-=(int)(src[j-2].green+src[dim+j-2].green);
	sum.red+=(int)(src[j+1].red+src[dim+j+1].red);
        sum.blue+=(int)(src[j+1].blue+src[dim+j+1].blue);
        sum.green+=(int)(src[j+1].green+src[dim+j+1].green);
           current_pixel.red = (unsigned short) (sum.red/6);
           current_pixel.green = (unsigned short) (sum.green/6);
           current_pixel.blue = (unsigned short) (sum.blue/6);
           dst[j] = current_pixel;
	}//计算第一排中间的元素
	sum.red-=(int)(src[j-2].red+src[dim+j-2].red);
        sum.blue-=(int)(src[j-2].blue+src[dim+j-2].blue);
        sum.green-=(int)(src[j-2].green+src[dim+j-2].green);
           current_pixel.red = (unsigned short) (sum.red/4);
           current_pixel.green = (unsigned short) (sum.green/4);
           current_pixel.blue = (unsigned short) (sum.blue/4);
	dst[dim-1] = current_pixel;//计算第一排最后一个元素
    for (i = 1; i< dim-1; i++){
        sum.red = sum.green = sum.blue = 0;
	sum.red+=(int)(src[(i-1)*dim].red+src[(i-1)*dim+1].red+src[i*dim].red+src[i*dim+1].red+src[(i+1)*dim].red+src[(i+1)*dim+1].red);
        sum.blue+=(int)(src[(i-1)*dim].blue+src[(i-1)*dim+1].blue+src[i*dim].blue+src[i*dim+1].blue+src[(i+1)*dim].blue+src[(i+1)*dim+1].blue);
        sum.green+=(int)(src[(i-1)*dim].green+src[(i-1)*dim+1].green+src[i*dim].green+src[i*dim+1].green+src[(i+1)*dim].green+src[(i+1)*dim+1].green);
	current_pixel.red = (unsigned short) (sum.red/6);
       current_pixel.green = (unsigned short) (sum.green/6);
       current_pixel.blue = (unsigned short) (sum.blue/6);
        dst[i*dim] = current_pixel;//计算中间每一排的第一个元素
	sum.red+=(int)(src[(i-1)*dim+2].red+src[i*dim+2].red+src[(i+1)*dim+2].red);
        sum.blue+=(int)(src[(i-1)*dim+2].blue+src[i*dim+2].blue+src[(i+1)*dim+2].blue);
        sum.green+=(int)(src[(i-1)*dim+2].green+src[i*dim+2].green+src[(i+1)*dim+2].green);
	current_pixel.red = (unsigned short) (sum.red/9);
       current_pixel.green = (unsigned short) (sum.green/9);
       current_pixel.blue = (unsigned short) (sum.blue/9);
        dst[i*dim+1] = current_pixel;//计算中间每一排的第二个元素
	for (j = 2; j < dim-1; j++){
	sum.red-=(int)(src[(i-1)*dim+j-2].red+src[i*dim+j-2].red+src[(i+1)*dim+j-2].red);
        sum.blue-=(int)(src[(i-1)*dim+j-2].blue+src[i*dim+j-2].blue+src[(i+1)*dim+j-2].blue);
        sum.green-=(int)(src[(i-1)*dim+j-2].green+src[i*dim+j-2].green+src[(i+1)*dim+j-2].green);
	sum.red+=(int)(src[(i-1)*dim+j+1].red+src[i*dim+j+1].red+src[(i+1)*dim+j+1].red);
        sum.blue+=(int)(src[(i-1)*dim+j+1].blue+src[i*dim+j+1].blue+src[(i+1)*dim+j+1].blue);
        sum.green+=(int)(src[(i-1)*dim+j+1].green+src[i*dim+j+1].green+src[(i+1)*dim+j+1].green);
       current_pixel.red = (unsigned short) (sum.red/9);
       current_pixel.green = (unsigned short) (sum.green/9);
       current_pixel.blue = (unsigned short) (sum.blue/9);
       dst[i*dim+j] = current_pixel;
	}//计算中间每一排的中间元素
	sum.red-=(int)(src[(i-1)*dim+j-2].red+src[i*dim+j-2].red+src[(i+1)*dim+j-2].red);
        sum.blue-=(int)(src[(i-1)*dim+j-2].blue+src[i*dim+j-2].blue+src[(i+1)*dim+j-2].blue);
        sum.green-=(int)(src[(i-1)*dim+j-2].green+src[i*dim+j-2].green+src[(i+1)*dim+j-2].green);
        current_pixel.red = (unsigned short) (sum.red/6);
       current_pixel.green = (unsigned short) (sum.green/6);
       current_pixel.blue = (unsigned short) (sum.blue/6); 
	dst[i*dim+dim-1] = current_pixel;//计算中间每一排的最后一个元素
    }
    sum.red = sum.green = sum.blue = 0;
	sum.red+=(int)(src[(i-1)*dim].red+src[i*dim+1].red+src[(i-1)*dim+1].red+src[i*dim].red);
        sum.blue+=(int)(src[(i-1)*dim].blue+src[i*dim+1].blue+src[(i-1)*dim+1].blue+src[i*dim].blue);
        sum.green+=(int)(src[(i-1)*dim].green+src[i*dim+1].green+src[(i-1)*dim+1].green+src[i*dim].green);
       current_pixel.red = (unsigned short) (sum.red/4);
       current_pixel.green = (unsigned short) (sum.green/4);
       current_pixel.blue = (unsigned short) (sum.blue/4);
        dst[i*dim] = current_pixel;//计算最后一排的第一个元素
	sum.red+=(int)(src[(i-1)*dim+2].red+src[i*dim+2].red);
        sum.blue+=(int)(src[(i-1)*dim+2].blue+src[i*dim+2].blue);
        sum.green+=(int)(src[(i-1)*dim+2].green+src[i*dim+2].green);
       current_pixel.red = (unsigned short) (sum.red/6);
       current_pixel.green = (unsigned short) (sum.green/6);
       current_pixel.blue = (unsigned short) (sum.blue/6);
        dst[i*dim+1] = current_pixel;//计算最后一排的第二个元素
	for (j = 2; j < dim-1; j++){
	sum.red-=(int)(src[(i-1)*dim+j-2].red+src[i*dim+j-2].red);
        sum.blue-=(int)(src[(i-1)*dim+j-2].blue+src[i*dim+j-2].blue);
        sum.green-=(int)(src[(i-1)*dim+j-2].green+src[i*dim+j-2].green);
	sum.red+=(int)(src[(i-1)*dim+j+1].red+src[i*dim+j+1].red);
        sum.blue+=(int)(src[(i-1)*dim+j+1].blue+src[i*dim+j+1].blue);
        sum.green+=(int)(src[(i-1)*dim+j+1].green+src[i*dim+j+1].green);
       current_pixel.red = (unsigned short) (sum.red/6);
       current_pixel.green = (unsigned short) (sum.green/6);
       current_pixel.blue = (unsigned short) (sum.blue/6);
       dst[i*dim+j] = current_pixel;
	}//计算最后一排的中间元素
	sum.red-=(int)(src[(i-1)*dim+j-2].red+src[i*dim+j-2].red);
        sum.blue-=(int)(src[(i-1)*dim+j-2].blue+src[i*dim+j-2].blue);
        sum.green-=(int)(src[(i-1)*dim+j-2].green+src[i*dim+j-2].green);
       current_pixel.red = (unsigned short) (sum.red/4);
       current_pixel.green = (unsigned short) (sum.green/4);
       current_pixel.blue = (unsigned short) (sum.blue/4);
	dst[i*dim+dim-1] = current_pixel;//计算最后一排的最后一个元素
 }

/*

 * naive_smooth - The naive baseline version of smooth 
 */
char naive_smooth_descr[] = "naive_smooth: Naive baseline implementation";
void naive_smooth(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim; i++)
	for (j = 0; j < dim; j++)
	    dst[RIDX(i, j, dim)] = avg(dim, i, j, src);
}

/*
 * smooth - Your current working version of smooth. 
 * IMPORTANT: This is the version you will be graded on
 */
char smooth_descr[] = "smooth: Current working version";
void smooth(int dim, pixel *src, pixel *dst) 
{
    naive_smooth(dim, src, dst);
}


/********************************************************************* 
 * register_smooth_functions - Register all of your different versions
 *     of the smooth kernel with the driver by calling the
 *     add_smooth_function() for each test function.  When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_smooth_functions() {
    add_smooth_function(&smooth, smooth_descr);
    add_smooth_function(&naive_smooth, naive_smooth_descr);
    /* ... Register additional test functions here */
    add_smooth_function(&my_smooth, my_smooth_descr);
}

