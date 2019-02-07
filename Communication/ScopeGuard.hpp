#pragma once

#include <functional>

class ScopeGuard
{
	std::function<void()> callback;
	bool canceled = false;

public:
	ScopeGuard(std::function<void()> callback): callback(callback) {}
	~ScopeGuard()
	{
		if (!canceled)
			callback();
	}
	void cancel() { canceled = true; }
};