#include "rundown_protection.h"

#include <condition_variable>
#include <future>
#include <mutex>

// FYI this rundown protection is super barebones and just uses mutex instead of being lockfree
// but for this application, it will be perfectly sufficient.

static bool srp_active = false;
static bool srp_has_user = false;
static std::mutex srp_mutex;
static std::condition_variable srp_cv;
DWORD srp_main_thread_id = 0;

void rp_activate()
{
	std::lock_guard<std::mutex> lck(srp_mutex);
	srp_active = true;
}

static void rp_deactivate_wait_impl()
{
	std::unique_lock<std::mutex> lck(srp_mutex);
	srp_active = false;
	srp_cv.wait(lck, [] { return !srp_has_user; });
}

void rp_deactivate_wait()
{
	if (!srp_active)
		return;

	bool main = GetCurrentThreadId() == srp_main_thread_id;
	if (main)
	{
		bool running = true;
		auto future = std::async(std::launch::async, [&] {
			rp_deactivate_wait_impl();
			running = false;
			PostThreadMessage(srp_main_thread_id, WM_APP + 1, 0, 0);
		});

		MSG msg;
		while (running && GetMessage(&msg, 0, 0, 0))
		{
			if (msg.message == WM_APP + 1 && !running)
				break;

			DispatchMessage(&msg);
		}
	}
	else
	{
		rp_deactivate_wait_impl();
	}
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
