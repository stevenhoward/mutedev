#pragma once

#include <memory>
#include <vector>
#include <string>

using std::vector;
using std::shared_ptr;
using std::string;

class DeviceManagerImpl;

class DeviceManager {
private: 
	shared_ptr<DeviceManagerImpl> impl_;

public:
	DeviceManager();

	/**
	 * Get the names of all active audio devices.
	 */
	vector<string> get_device_names() const;


	void mute(unsigned int index) const;
};