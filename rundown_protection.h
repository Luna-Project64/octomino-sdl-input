#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

	// Allow entering rp_protect functions
	void rp_activate(void);
	// Disallow entering rp_protect functions, wait for all current rp_protect functions to finish
	void rp_deactivate_wait(void);

	// Enter rundown protection critical section, returns true if entered
	bool rp_protect(void);
	// Leave rundown protection critical section
	void rp_unprotect(void);

#ifdef __cplusplus
}
#endif
