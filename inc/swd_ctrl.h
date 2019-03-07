#ifndef __SWD_CTRL_H__
#define __SWD_CTRL_H__

typedef struct swd_ctrl_st swd_ctrl_obj;

typedef int (*swd_ctrl_set_cfg_from_gbl_cfg_cb)
				 (swd_ctrl_obj * const obj);
typedef int (*swd_ctrl_set_cfg_cb)
				 (swd_ctrl_obj * const obj, const char *name);
typedef int (*swd_ctrl_start_cb)
				(swd_ctrl_obj * const obj, char * const prog);
typedef int (*swd_ctrl_stop_cb) (swd_ctrl_obj * const obj);

struct swd_ctrl_st {
	swd_ctrl_set_cfg_from_gbl_cfg_cb	set_cfg_if_from_gbl_cfg;
	swd_ctrl_set_cfg_from_gbl_cfg_cb	set_cfg_cpu_from_gbl_cfg;
	swd_ctrl_set_cfg_cb			set_cfg_if;
	swd_ctrl_set_cfg_cb			set_cfg_cpu;

	swd_ctrl_start_cb			start;
	swd_ctrl_stop_cb			stop;

	void 					*pdata;
};


/**
 * @brief Initialize swd_obj object.
 * @param obj the swd_obj object to initialize.
 * @return 0 upon success, -1 otherwise.
 */
int swd_ctrl_init(swd_ctrl_obj * const obj);

/**
 * @brief De-initialize swd_obj object.
 * @param obj the swd_obj object to de-initialize.
 * @return 0 upon success, -1 otherwise.
 */
int swd_ctrl_fini(swd_ctrl_obj * const obj);

#endif /* __SWD_CTRL_H__ */
