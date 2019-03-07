/*****************************************************************
 * @file decoder_swo.h
 * @author Alexandre Malki <amalki@piap.pl>
 * @brief This is the header file of the decoder_swo_obj object.
 * 		more detailed are provided in the source file decoder_swo.c.
 *****************************************************************/
#ifndef __DECODER_SWO_H__
#define __DECODER_SWO_H__

#include <processing.h>

typedef struct decoder_swo_obj_st decoder_swo_obj;  

struct decoder_swo_obj_st {
	/**  Processing object inheriting from */
	processing_obj 	proc_obj;
	/** Internal private data */
	void		*pdata;
};

/**
 * @brief Set up and initialize the decoder object.
 * @param decoder_swo_obj decoder obj to be initialized. It is assumed that the
 *		amount obj is already allocated.
 * @return 0 upon success, -1 othewise.
 */
int decoder_swo_init(decoder_swo_obj * const obj);

/**
 * @brief This function will clear the object and initiliazed it.
 * @param decoder_swo_obj decoder obj to be initialized. It is assumed that the
 *		amount obj is already allocated.
 * @return 0 upon success, -1 othewise.
 */
int decoder_swo_fini(decoder_swo_obj * const obj);

#endif /* __DECODER_SWO_H__ */
