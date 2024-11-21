#include "rundown_protection.h"

#include <condition_variable>
#include <mutex>

// FYI this rundown protection is super barebones and just uses mutex instead of being lockfree
// but for this application, it will be perfectly sufficient.

static bool srp_active = false;
static bool srp_has_user = false;
static std::mutex srp_mutex;
static std::condition_variable srp_cv;

void rp_activate()
{
	std::lock_guard<std::mutex> lck(srp_mutex);
	srp_active = true;
}

void rp_deactivate_wait()
{
	std::unique_lock<std::mutex> lck(srp_mutex);
	srp_active = false;
	srp_cv.wait(lck, [] { return !srp_has_user; });
}

bool rp_protect()
{
	std::lock_guard<std::mutex> lck(srp_mutex);
	if (!srp_active)
		return false;

	srp_has_user = true;
	return true;
}

void rp_unprotect()
{
	std::lock_guard<std::mutex> lck(srp_mutex);
	srp_has_user = false;
	if (!srp_active)
		srp_cv.notify_all();
}
