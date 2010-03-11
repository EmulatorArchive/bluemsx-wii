/* pngrewrite
   version 1.3.0  - 7 Dec 2008

  A quick and dirty utility to reduce unnecessarily large palettes 
  and bit depths in PNG image files.


  ** To compile in a unix-like environment, type something like
  **   gcc -Wall -O2 -o pngrewrite pngrewrite.c -lpng
  ** or
  **   cc -o pngrewrite pngrewrite.c -lpng -lz -lm


  Web site: <http://entropymine.com/jason/pngrewrite/>

  This program and source code may be used without restriction.

  Primary author: Jason Summers  <jason1@pobox.com>

  Contributing authors:

     Wayne Schlitt
       - Write grayscale images when possible
	   - Ability to sort palette by color usage
	   - Improved find_pal_entry performance

     Soren Anderson
	   - Changes to allow use in a unix-style pipeline
	   - Improved compatibility with MinGW and Cygwin
       - Maintain tIME chunks


  Pngrewrite will:

  * Remove any unused palette entries, and write a palette that is only as
    large as needed.

  * Remove (collapse) any duplicate palette entries.

  * Convert non-palette image to palette images, provided they contain no
    more than 256 different colors.

  * Write images as grayscale when possible.

  * Move any colors with transparency to the beginning of the palette, and
    write a tRNS chunk that is a small as possible.

  * Reduce the bit-depth (bits per pixel) as much as possible.



  Under no circumstances does pngrewrite change the actual pixel colors, or
  background color, or transparency of the image. If it ever does, that's a
  bug.

  --WARNING--
  This version of pngrewrite removes most extra (ancillary) information from
  the PNG file, such as text comments. Although this does make the file size
  smaller, the removed information may sometimes be important.

  The only ancillary chunks that are NOT removed are:
     gAMA  - Image gamma setting
     sRGB  - srgb color space indicator
	 tIME  - creation time
	 pHYs  - physical pixel size
     bKGD and tRNS - Background color and transparency are maintained. The
        actual chunk may be modified according to the new color structure.

  If the original image was interlaced, the new one will also be interlaced.

  Pngrewrite will not work at all on images that have more than 256 colors.
  Colors with the same RGB values but a different level of transparency
  count as different colors. The background color counts as an extra color
  if it does not occur in the image.
  
  It will also not work at all on images that have a color depth of 16 bits,
  since they cannot have a palette.

  This is a very inefficient program. It is (relatively) slow, and may use a
  lot of memory. To be specific, it uses about 5 bytes per pixel, no matter
  what the bit depth of the image is.

  This program is (hopefully) reasonably portable, and should compile
  without too much effort on most C compilers. It requires the libpng and
  zlib libraries.
*/
#define PNGREWRITEVERSION  "1.3.0"

//#define PNGRW_SUPPORT_1_ARG_MODE
#define PNGRW_PALSORT_BY_FREQUENCY


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>

#if defined( WIN32 ) && !defined (__GNUC__)
#include <search.h> /* for qsort */
#include <io.h>
#include <fcntl.h>
#endif

#if defined( WIN32 ) && defined (__GNUC__)
#  include <fcntl.h>
#endif

#if defined(PNGRW_SUPPORT_1_ARG_MODE) && !defined(WIN32)
#include <unistd.h>  /* for isatty() */
#endif

#include <png.h>

struct errstruct {
	jmp_buf jbuf;
	char errmsg[200];
};

struct pal_entry_info {
	unsigned char red;
	unsigned char green;
	unsigned char blue;
	unsigned char alpha;
	unsigned int  count;
};

struct context {
	struct pal_entry_info pal[256];
	int pal_used;
	int new_bit_depth;
	int valid_gray;
	unsigned char *image1, *image2;
	unsigned char **row_pointers;
	int rowbytes, channels;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;
	png_time savechunk_time; /*  S.A. */
	int ori_pal_size;
	double image_gamma;
	struct pal_entry_info bkgd;             /* RGB background color */
	int gray_trns;
	unsigned char bkgd_pal;    /* new background color palette entry */
	int srgb_intent;
	png_uint_32 res_x,res_y;
	int res_unit_type;
	int has_gAMA;
	int has_bKGD;
	int has_sRGB;
	int has_tIME;
	int has_pHYs;
	int prev_entry;
	int prev_entry_valid;
	unsigned char prev_r;
	unsigned char prev_g;
	unsigned char prev_b;
	unsigned char prev_a;
};


static void my_png_error_fn(png_structp png_ptr, const char *err_msg)
{
	struct errstruct *errinfop;

	errinfop = (struct errstruct *)png_get_error_ptr(png_ptr);

#ifdef _MSC_VER
	_snprintf(errinfop->errmsg,200,"%s",err_msg);
#else
	snprintf(errinfop->errmsg,200,"%s",err_msg);
#endif
	errinfop->errmsg[199] = '\0';

	longjmp(errinfop->jbuf, -1);
}

static void my_png_warning_fn(png_structp png_ptr, const char *warn_msg)
{
	return;
}

static int pngrw_read_png(struct context *ctx, FILE *infp)
{
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_colorp ori_pal;
	int ori_bpp;
	struct errstruct errinfo;
	int retval = 0;
	int i;
	png_color_16 *image_background;
	png_timep in_time;  /*  a struct POINTER  */

	strcpy(errinfo.errmsg,"");

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
	    (void*)(&errinfo),my_png_error_fn, my_png_warning_fn);

	if(!png_ptr) {
		fprintf(stderr, "Error creating read_struct\n");
		goto done;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr) {
		fprintf(stderr, "Error creating read_info_struct\n");
		goto done;
	}

	if (setjmp(errinfo.jbuf)) {
		fprintf(stderr, "PNG decode error: %s\n", errinfo.errmsg);
		goto done;
	}

	png_init_io(png_ptr, infp);

	png_read_info(png_ptr, info_ptr);

	png_get_IHDR(png_ptr, info_ptr, &ctx->width, &ctx->height, &ctx->bit_depth, &ctx->color_type,
		&ctx->interlace_type, NULL, NULL);

	if(ctx->bit_depth>8) {
		// TODO: Some (rare?) 16-bpp images *can* be perfectly converted to palette images.
		fprintf(stderr, "This image can't be converted because it has 16 bits/sample.\n");
		goto done;
	}


    if (ctx->color_type == PNG_COLOR_TYPE_PALETTE) {
		ctx->ori_pal_size=0;
		ori_bpp = ctx->bit_depth;

		/* we only do this to get the palette size so we can print it */
		png_get_PLTE(png_ptr,info_ptr,&ori_pal,&ctx->ori_pal_size);

		fprintf(stderr, "original palette size:   %3d, %2d bpp\n",ctx->ori_pal_size,ori_bpp);

        png_set_expand(png_ptr);
	}
	else {
		/* figure out bits per pixel so we can print it */
		ori_bpp= ctx->bit_depth*png_get_channels(png_ptr,info_ptr);
		fprintf(stderr, "original palette size: [n/a], %2d bpp\n",ori_bpp);
	}

    if (ctx->color_type == PNG_COLOR_TYPE_GRAY && ctx->bit_depth < 8)
        png_set_expand(png_ptr);
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_expand(png_ptr);
	/* if (bit_depth == 16)
        png_set_strip_16(png_ptr); */
    if (ctx->color_type == PNG_COLOR_TYPE_GRAY ||
        ctx->color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png_ptr);
	}

	if (png_get_bKGD(png_ptr, info_ptr, &image_background)) {
		ctx->has_bKGD=1;
		ctx->bkgd.red=   (unsigned char)image_background->red;
		ctx->bkgd.green= (unsigned char)image_background->green;
		ctx->bkgd.blue=  (unsigned char)image_background->blue;
		ctx->bkgd.alpha=255;
	}

	if (png_get_gAMA(png_ptr, info_ptr, &ctx->image_gamma)) {
		ctx->has_gAMA=1;
	}

	if (png_get_sRGB(png_ptr, info_ptr, &ctx->srgb_intent)) {
		ctx->has_sRGB=1;
	}

	if(png_get_valid(png_ptr,info_ptr,PNG_INFO_pHYs)) {
		ctx->has_pHYs=1;
		png_get_pHYs(png_ptr,info_ptr,&ctx->res_x,&ctx->res_y,&ctx->res_unit_type);
		if(ctx->res_x<1 || ctx->res_y<1) ctx->has_pHYs=0;
	}

	/*  S.A.  ..............................  */
	if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tIME)) {
		if(png_get_tIME(png_ptr, info_ptr, &in_time) == PNG_INFO_tIME) {
			ctx->savechunk_time = *in_time;
			ctx->has_tIME = 1;
		}
	}

    png_read_update_info(png_ptr, info_ptr);

	ctx->rowbytes=png_get_rowbytes(png_ptr, info_ptr);
	ctx->channels=(int)png_get_channels(png_ptr, info_ptr);

	if(ctx->channels<3 || ctx->channels>4) {
		fprintf(stderr, "internal error: channels=%d\n",ctx->channels);
		goto done;
	}

	fprintf(stderr,"Image size is %dx%d  memory required=%.3fMB\n",
		(int)ctx->width, (int)ctx->height,
		(ctx->height*ctx->rowbytes + ctx->height*sizeof(unsigned char*) + ctx->width*ctx->height) / (1024. * 1024.) );

	ctx->image1= (unsigned char*)malloc(ctx->height*ctx->rowbytes);
	ctx->row_pointers = (unsigned char**)malloc(ctx->height * sizeof(unsigned char*));
	if(!ctx->image1 || !ctx->row_pointers) {
		fprintf(stderr, "Unable to allocate memory for image\n");
		goto done;
	}

	for(i=0;i<(int)ctx->height;i++) {
		ctx->row_pointers[i] = &ctx->image1[ctx->rowbytes*i];
	}

	png_read_image(png_ptr, ctx->row_pointers);

	png_read_end(png_ptr, info_ptr);

	/* S.A.  .................................  */
	if(!ctx->has_tIME &&
		png_get_valid(png_ptr,info_ptr,PNG_INFO_tIME))
	{
		if(png_get_tIME(png_ptr, info_ptr, &in_time) == PNG_INFO_tIME) {
			ctx->savechunk_time = *in_time;
			ctx->has_tIME = 1;
		}
	}

	retval = 1;

done:
	if(png_ptr) {
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
	}

	return retval;
}


static int pngrw_write_new_png(struct context *ctx, FILE *outfp)
{
    png_structp png_ptr = NULL;
    png_infop  info_ptr = NULL;
	png_color palette[256];
	unsigned char trans[256];
	int num_trans;
	int i;
	png_color_16 newtrns;
	png_color_16 newbackground;
	struct errstruct errinfo;
	int retval = 0;

	memset(&newtrns      ,0,sizeof(png_color_16));
	memset(&newbackground,0,sizeof(png_color_16));

	strcpy(errinfo.errmsg,"");

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
		(void*)(&errinfo),my_png_error_fn, my_png_warning_fn);
	if (!png_ptr) {
		fprintf(stderr, "Error creating write_struct\n");
		goto done;
	}

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
		fprintf(stderr, "Error creating write_info_struct\n");
        goto done;
    }

	if (setjmp(errinfo.jbuf)) {
		fprintf(stderr, "PNG encode error: %s\n", errinfo.errmsg);
		goto done;
	}

	png_init_io(png_ptr, outfp);

	if( ctx->valid_gray ) {
	    png_set_IHDR(png_ptr, info_ptr, ctx->width, ctx->height, ctx->new_bit_depth,
			 PNG_COLOR_TYPE_GRAY, ctx->interlace_type,
			 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

		if(ctx->gray_trns>=0) {
			newtrns.gray = ctx->gray_trns;
			png_set_tRNS(png_ptr, info_ptr, NULL, 1, &newtrns);
		}

	}
	else {
	    png_set_IHDR(png_ptr, info_ptr, ctx->width, ctx->height, ctx->new_bit_depth,
			 PNG_COLOR_TYPE_PALETTE, ctx->interlace_type,
			 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

		/* ... set palette colors ... */

		num_trans=0;  /* number of transparent palette entries */

		for(i=0;i<ctx->pal_used;i++) {
			palette[i].red=ctx->pal[i].red;
			palette[i].green=ctx->pal[i].green;
			palette[i].blue=ctx->pal[i].blue;

			trans[i]=ctx->pal[i].alpha;
			if(trans[i]<255) num_trans=i+1;
		}
		png_set_PLTE(png_ptr, info_ptr, palette, ctx->pal_used);

		if(num_trans>0) {
			png_set_tRNS(png_ptr, info_ptr, trans, num_trans, 0);
		}
	}
	

	if(ctx->has_gAMA) png_set_gAMA(png_ptr, info_ptr, ctx->image_gamma);
	if(ctx->has_sRGB) png_set_sRGB(png_ptr, info_ptr, ctx->srgb_intent);
	if(ctx->has_pHYs) png_set_pHYs(png_ptr, info_ptr, ctx->res_x, ctx->res_y, ctx->res_unit_type);

	if(ctx->has_bKGD) {
		if(ctx->valid_gray)
			newbackground.gray = ctx->bkgd_pal;
		else
			newbackground.index = ctx->bkgd_pal;
		png_set_bKGD(png_ptr, info_ptr, &newbackground);
	}
	
	/*  S.A.  ............................  */
	if(ctx->has_tIME) {
		png_set_tIME(png_ptr, info_ptr, &ctx->savechunk_time);
	}

	png_write_info(png_ptr, info_ptr);

	png_set_packing(png_ptr);

	/* re-use row_pointers array */
	for(i=0;i<(int)ctx->height;i++) {
		ctx->row_pointers[i]= &ctx->image2[i*ctx->width];
	}

	png_write_image(png_ptr, ctx->row_pointers);

	png_write_end(png_ptr, info_ptr);

	retval = 1;

done:

	if(png_ptr) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
	}
	return retval;
}


/* set ignorealpha to 1 if you don't care if the alpha value matches
 * (for the background) */
static int
add_to_palette(struct context *ctx, const struct pal_entry_info *color, int ignorealpha)
{
	if(ctx->pal_used>=256) {
		fprintf(stderr, "This image can't be converted because it has more than 256 colors.\n");
		return 0;
	}
	ctx->pal[ctx->pal_used].red   = color->red;
	ctx->pal[ctx->pal_used].green = color->green;
	ctx->pal[ctx->pal_used].blue  = color->blue;
	ctx->pal[ctx->pal_used].alpha = ignorealpha?255:color->alpha;
	ctx->pal[ctx->pal_used].count = 1;
	ctx->pal_used++;
	return 1;
}

/*
void debug_print_pal(struct pal_entry_info *pal, int used)
{
	int i;
	for(i=0;i<used;i++)
		fprintf(stderr, "%d. %d %d %d %d\n",i,pal[i].red,pal[i].green,pal[i].blue,pal[i].alpha);
}
*/


/* Sort the palette to put transparent colors first,
 * then sort by how frequently the color is used.  Sorting by frequency
 * will often help the png filters make the image more compressible.
 * It also makes it easier for people to see which colors aren't used much
 * and allow them to manually reduce the color palette. */
static int palsortfunc(const void* p1, const void* p2)
{
	struct pal_entry_info *e1,*e2;
	int s1,s2;

	e1=(struct pal_entry_info*)p1;
	e2=(struct pal_entry_info*)p2;

	if(e1->alpha==255 && e2->alpha<255) return 1;
	if(e1->alpha<255 && e2->alpha==255) return -1;

#ifdef PNGRW_PALSORT_BY_FREQUENCY
	if(e1->count<e2->count) return 1;
	if(e1->count>e2->count) return -1;
#endif

	s1=e1->red+e1->green+e1->blue;
	s2=e2->red+e2->green+e2->blue;
	if(s1>s2) return 1;
	if(s1<s2) return -1;
	return 0;
}

static void reset_pal_entry_cache(struct context *ctx)
{
	ctx->prev_entry_valid = 0;
}

static int
find_pal_entry(struct context *ctx, unsigned char r, unsigned char g, unsigned char b, 
				   unsigned char a, int ignorealpha)
{
	int i;

	if(ctx->prev_entry_valid && ctx->prev_r==r && ctx->prev_g==g  &&
	   ctx->prev_b==b && ctx->prev_a==a)
	{
	    return ctx->prev_entry;
	}

	for(i=0;i<ctx->pal_used;i++) {
		if(ctx->pal[i].red==r && ctx->pal[i].green==g && ctx->pal[i].blue==b
		   && (ctx->pal[i].alpha==a || ignorealpha)) {

		    ctx->prev_r = r;
		    ctx->prev_g = g;
		    ctx->prev_b = b;
		    ctx->prev_a = a;
		    ctx->prev_entry = i;
		    
		    return i;
		}
	}
	return (-1);
}

static int pngrw_make_new_png(struct context *ctx)
{
	struct pal_entry_info thispix;
	unsigned char *p;
	int x,y;
	int palent;
	int i;
	int gray_trns_palentry; /* temp palette entry */
	unsigned char gray_trns_shade = 0; /* 0 through 255 */

	ctx->pal_used=0;

	for(y=0;y<(int)ctx->height;y++) {
		for(x=0;x<(int)ctx->width;x++) {
			p=&ctx->row_pointers[y][x*ctx->channels];
				thispix.red=p[0];
				thispix.green=p[1];
				thispix.blue=p[2];
				if(ctx->channels==4) thispix.alpha=p[3];
				else thispix.alpha=255;
			palent = find_pal_entry(ctx,p[0],p[1],p[2],thispix.alpha,0);
			if(palent<0) {
			    if(!add_to_palette(ctx,&thispix,0))
				return 0;
			}
			else
			    ctx->pal[palent].count++;
		}
	}

	if(ctx->has_bKGD) {
		ctx->bkgd.alpha= 255; /* actually not needed */
		palent = find_pal_entry(ctx,ctx->bkgd.red,ctx->bkgd.green,ctx->bkgd.blue,ctx->bkgd.alpha,1);
		if(palent<0) {
			add_to_palette(ctx,&ctx->bkgd,1);
		}
		else
		    ctx->pal[palent].count++;
	}


	/* determine bit depth and whether we should do grayscale or palette */
	ctx->new_bit_depth=8;
	if(ctx->pal_used<=16) ctx->new_bit_depth=4;
	if(ctx->pal_used<=4) ctx->new_bit_depth=2;
	if(ctx->pal_used<=2) ctx->new_bit_depth=1;

	/* figure out if this is a valid grayscale image */

	gray_trns_palentry = -1;

	ctx->valid_gray = 1;
	for( i = 0; i < ctx->pal_used; i++ ) {

	    if( ctx->pal[i].red != ctx->pal[i].green
		   || ctx->pal[i].red != ctx->pal[i].blue)
	    {
			// not gray
			ctx->valid_gray = 0;
			break;
	    }

		if(ctx->pal[i].alpha!=255 && ctx->pal[i].alpha!=0) {
			ctx->valid_gray=0;
			break;
		}

		if(ctx->pal[i].alpha == 0) {
			if(gray_trns_palentry != -1) { ctx->valid_gray=0; break; } // multiple transparent colors
			gray_trns_palentry = i;  // binary transparency ok (so far)
			gray_trns_shade = ctx->pal[i].red;
		}


	    switch( ctx->pal[i].red ) {
	    case 0: case 255:
			continue;
			//break;

	    case 85: case 170:
			if( ctx->new_bit_depth >= 2 )
				continue;
			break;
		
	    case  17:  case 34:  case 51:  case 68:
	    case 102: case 119: case 136: case 153:
	    case 187: case 204: case 221: case 238:
			if( ctx->new_bit_depth >= 4 )
				continue;
			break;

	    default:
			if( ctx->new_bit_depth >= 8 )
				continue;
				//break;
		}
	    
	    ctx->valid_gray = 0;
	    break;
	}

	// One thing the above doesn't check for is a nontransparent
	// grayscale that's the same as the transparent grayscale color.
	// In this case we have to use palette color.
	if(ctx->valid_gray && gray_trns_palentry != -1) {
		if(-1 != find_pal_entry(ctx,gray_trns_shade,gray_trns_shade,gray_trns_shade,255,0)) {
			ctx->valid_gray = 0;
		}
	}

	/* put the palette in a good order */

	if(ctx->valid_gray) {
		// If grayscale, create a "fake" palette consisting of all
		// available gray shades.
		
		switch(ctx->new_bit_depth) {
		case 8:
			ctx->pal_used=256;
			for(i=0;i<ctx->pal_used;i++) {
				ctx->pal[i].red = ctx->pal[i].green = ctx->pal[i].blue = i;
				ctx->pal[i].alpha = 255;
			}
			break;
		case 4:
			ctx->pal_used=16;
			for(i=0;i<ctx->pal_used;i++) {
				ctx->pal[i].red = ctx->pal[i].green = ctx->pal[i].blue = i*17;
				ctx->pal[i].alpha = 255;
			}
			break;
		case 2:
			ctx->pal_used=4;
			for(i=0;i<ctx->pal_used;i++) {
				ctx->pal[i].red = ctx->pal[i].green = ctx->pal[i].blue = i*85;
				ctx->pal[i].alpha = 255;
			}
			break;
		case 1:
			ctx->pal_used=2;
			for(i=0;i<ctx->pal_used;i++) {
				ctx->pal[i].red = ctx->pal[i].green = ctx->pal[i].blue = i*255;
				ctx->pal[i].alpha = 255;
			}
			break;
		}

		ctx->gray_trns = -1;
		// handle grayscale binary transparency
		if(gray_trns_palentry != -1) {
			ctx->gray_trns=find_pal_entry(ctx,gray_trns_shade,gray_trns_shade,gray_trns_shade,255,0);
			if(ctx->gray_trns == -1) {
				fprintf(stderr,"internal error: can't find transparent grayscale color\n");
				exit(1);
			}
			ctx->pal[ctx->gray_trns].alpha = 0;
		}

	}
	else {
		qsort((void*)ctx->pal,ctx->pal_used,sizeof(struct pal_entry_info),palsortfunc);
	}


	if(ctx->valid_gray)
		fprintf(stderr, "saving as grayscale:          %2d bpp\n",ctx->new_bit_depth);
	else
		fprintf(stderr, "new palette size:        %3d, %2d bpp\n",ctx->pal_used,ctx->new_bit_depth);



	/* reset the find_pal_entry() cache */
	reset_pal_entry_cache(ctx);
	//palent = find_pal_entry(pal[0].red,pal[0].green,pal[0].blue,thispix.alpha,0);
	//palent = find_pal_entry(pal[1].red,pal[1].green,pal[1].blue,thispix.alpha,0);

	/* debug_print_pal(pal,pal_used); */

	/* now create the new image */
	ctx->image2 = (unsigned char*)malloc(ctx->width*ctx->height);
	if(!ctx->image2) {
		fprintf(stderr, "out of memory\n");
		return 0;
	}

	for(y=0;y<(int)ctx->height;y++) {
		for(x=0;x<(int)ctx->width;x++) {
			p=&ctx->row_pointers[y][x*ctx->channels];

			palent = find_pal_entry(ctx,p[0],p[1],p[2],(unsigned char)((ctx->channels==4)?p[3]:255),0);
			if(palent<0) {
				fprintf(stderr, "internal error: can't locate palette entry\n");
				return 0;
			}
			ctx->image2[y*ctx->width + x] = (unsigned char)palent;
		}
	}

	if(ctx->has_bKGD) {
		palent = find_pal_entry(ctx,ctx->bkgd.red,ctx->bkgd.green,ctx->bkgd.blue,255,1);
		if(palent<0) {
			fprintf(stderr, "internal error: can't locale palette entry for bkgd\n");
			return 0;
		}
		ctx->bkgd_pal = (unsigned char)palent;
	}

	if(ctx->image1) {
		free(ctx->image1);
		ctx->image1 = NULL;
	}

	return 1;
}

// Sets default values for everything.
static void init_ctx(struct context *ctx)
{
	memset(ctx,0,sizeof(struct context));
}

static void free_ctx_contents(struct context *ctx)
{
	if(ctx->row_pointers) free(ctx->row_pointers);
	if(ctx->image1) free(ctx->image1);
	if(ctx->image2) free(ctx->image2);
}

// Open the files, and call the worker functions.
// For stdin/stdout, pass "-" as the filename.
static int pngrw_optimize_png(const char *in_filename, const char *out_filename)
{
	struct context ctx;
	FILE *infp = NULL;
	FILE *outfp = NULL;
	int needclose_infp = 0;
	int needclose_outfp = 0;
	int retval=0;

	init_ctx(&ctx);

	// Prepare input file.
	if(!strcmp(in_filename,"-")) {
		infp = stdin;
#if WIN32
		setmode( fileno(infp), O_BINARY);
#endif
	}
	else {
		infp = fopen(in_filename,"rb");
		if(!infp) {
			fprintf(stderr,"Can't read file %s\n",in_filename);
			goto done;
		}
		needclose_infp = 1;
	}


	if(!pngrw_read_png(&ctx,infp)) {
		goto done;
	}

	if(needclose_infp && infp) {
		fclose(infp);
		infp=NULL;
		needclose_infp=0;
	}

	if(!pngrw_make_new_png(&ctx)) {
		goto done;
	}


	// Prepare output file.
	if(!strcmp(out_filename,"-")) {
		outfp = stdout;
#ifdef WIN32
		setmode( fileno(outfp), O_BINARY);
#endif
	}
	else {
		outfp = fopen(out_filename,"wb");
		if(!outfp) {
			fprintf(stderr,"Can't write file %s\n",out_filename);
			goto done;
		}
		needclose_outfp = 1;
	}

	if(!pngrw_write_new_png(&ctx,outfp)) {
		goto done;
	}

	retval = 1;

done:
	if(needclose_infp && infp) fclose(infp);
	if(needclose_outfp && outfp) fclose(outfp);
	free_ctx_contents(&ctx);
	return retval;
}

int main(int argc,char **argv)
{
	char *out_filename;
	char *in_filename;
	int ret;
	char specstamp[38]="";

	if(argc==3) {
		in_filename = argv[1];
		out_filename = argv[2];
	}
#ifdef PNGRW_SUPPORT_1_ARG_MODE
	else if( argc==2 && !isatty(fileno(stdin)) ) {
		in_filename = "-";
		out_filename = argv[1];
	}
#endif
	else {
		fprintf(stderr, "pngrewrite v" PNGREWRITEVERSION ": PNG image palette optimizer%s\n", specstamp);
#ifdef PNGRW_SUPPORT_1_ARG_MODE
        fprintf(stderr, "Usage: %s infile.png outfile.png OR |%s outfile.png\n", argv[0], argv[0]);
#else
		fprintf(stderr, "Usage: %s infile.png outfile.png\n", argv[0]);
#endif
		return 1;
	}

	ret = pngrw_optimize_png(in_filename,out_filename);
	if(!ret) return 1;
	return 0;
}
