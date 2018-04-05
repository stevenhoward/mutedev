#pragma once

#include <memory>
#include <vector>
#include <string>

class DeviceManagerImpl;

class DeviceManager {
private: 
	std::unique_ptr<DeviceManagerImpl> impl_;

public:
	DeviceManager();
	~DeviceManager();

	/**
	 * Get the names of all active audio devices.
	 */
	std::vector<std::string> get_device_names() const;

	void mute(unsigned int index) const;
};