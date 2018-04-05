#include "cancel.h"

#include <iostream>
#include <mutex>
#include <vector>
#include <Windows.h>

using namespace std;

namespace {
	once_flag registered_handler;
	vector<thunk_t> thunks;

	BOOL WINAPI handler_routine(DWORD type) {
		if (type == CTRL_C_EVENT) {
			for (const auto thunk : thunks) {
				thunk();
			}
		}

		// Still exit the program
		return FALSE;
	}
}

void add_cancel_handler(thunk_t && thunk) {
	call_once(registered_handler, [] { SetConsoleCtrlHandler(handler_routine, TRUE); });
	thunks.push_back(move(thunk));
}