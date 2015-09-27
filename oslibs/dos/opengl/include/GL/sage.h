/**
 * \file sage.h
 * SAGE public API.
 *
 * Typical usage:
 * \code
 *  if (sage_init() > 0) {
 *      sageContext *ctx = sage_open(TRUE, 5, 6, 5, 0, 16, 0);
 *      if (ctx != NULL) {
 *          if (sage_bind(ctx, win, 640, 480) == 0) {
 *              // OpenGL commands
 *              sage_swap(1);
 *              sage_bind(NULL, NULL, 0, 0);
 *          }
 *          sage_shut(ctx);
 *      }
 *      sage_fini();
 *  }
 * \endcode
 */


#ifndef SAGE_H_included
#define SAGE_H_included

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opaque definition for graphic context
 */
typedef struct sageContext sageContext;

/**
 * Initialize hardware.
 *
 * \return maximum color depth
 *
 * \note It is safe to call multiple times, but there is no reference count.
 */
int sage_init (void);

/**
 * Open graphic context.
 *
 * \param db_flag doublebuffer requested
 * \param red_size desired red channel bits
 * \param green_size desired green channel bits
 * \param blue_size desired blue channel bits
 * \param alpha_size desired alpha channel bits
 * \param depth_size desired depth channel bits
 * \param stencil_size desired stencil channel bits
 *
 * \return graphic context
 */
sageContext *sage_open (int db_flag,
			int red_size, int green_size, int blue_size,
			int alpha_size, int depth_size, int stencil_size);

/**
 * Bind context to drawable and make the context the current one.
 *
 * \param win window handle
 * \param width width in pixels
 * \param height height in pixels
 *
 * \return 0 if success
 */
int sage_bind (sageContext *ctx, void *win, int width, int height);

/**
 * Close graphic context.
 *
 * \param ctx graphic context
 */
void sage_shut (sageContext *ctx);

/**
 * Shutdown hardware.
 *
 * \note It is safe to call multiple times, but there is no reference count.
 */
void sage_fini (void);

/**
 * Swap buffers.
 *
 * \param interval number of retraces to wait
 */
void sage_swap (int interval);

#ifdef __MSDOS__
typedef void (*SageProc) ();
SageProc sage_GetProcAddress (const char *procname);
#endif

#ifdef __cplusplus
}
#endif

#endif
