#pragma once

#include <functional>

typedef std::function<void()> thunk_t;

/**
 * Add a handler to be run when the user hits ctrl+c.
 * Does not block signal.
 */
void add_cancel_handler(thunk_t&& thunk);