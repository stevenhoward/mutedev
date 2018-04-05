#include "device.h"

#include <cassert>
#include <iostream>
#include <string>

#include <Windows.h>
#include <commctrl.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <functiondiscoverykeys_devpkey.h>

#define CHECK(h) if (FAILED(h)) abort();

using namespace std;

namespace {
	/*
	 * Pretend everything is ASCII.
	 * TODO: purchase bad code offset
	 */
	string string_from_lpwstr(LPWSTR sz) {
		const auto len = wcslen(sz);
		wstring wstr(sz, sz + len);

		vector<char> vstr(wstr.size());
		for (auto i = wstr.begin(), j = wstr.end(); i != j; ++i) {
			vstr.push_back((char)*i);
		}

		return { vstr.begin(), vstr.end() };
	}

	struct device_query_result {
		IMMDeviceCollection* devices;
		UINT count;
	};
}

class DeviceManagerImpl {
	mutable GUID context_;
	IMMDeviceEnumerator * enumerator_;

	string get_device_name(IMMDevice* device) const {
		IPropertyStore* properties;

		device->OpenPropertyStore(STGM_READ, &properties);

		PROPVARIANT variant;
		CHECK(properties->GetValue(PKEY_Device_FriendlyName, &variant));

		auto result = string_from_lpwstr(variant.pwszVal);
		return result;
	}

	device_query_result enum_audio_endpoints() const {
		IMMDeviceCollection* devices;

		CHECK(enumerator_->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &devices));

		UINT num_devices;
		CHECK(devices->GetCount(&num_devices));
		return { devices, num_devices };
	}

public:
	DeviceManagerImpl() { 
		CoInitialize(nullptr);

		CHECK(CoCreateGuid(&context_)) {}

		CHECK(CoCreateInstance(
			__uuidof(MMDeviceEnumerator),
			nullptr,
			CLSCTX_INPROC_SERVER,
			__uuidof(IMMDeviceEnumerator),
			(void**)&enumerator_
		));
	}
	
	vector<string> get_device_names() const {
		auto endpoints = enum_audio_endpoints();

		vector<string> names;
		for (UINT i = 0; i < endpoints.count; ++i) {
			IMMDevice* device;

			CHECK(endpoints.devices->Item(i, &device));
			names.push_back(get_device_name(device));
		}

		return names;
	}

	// Assumption: EnumAudioInterfaces() always returns devices in the same order.
	// So long as the ordering is stable, the arbitrary indices will work
	void mute(UINT index) const {
		auto endpoints = enum_audio_endpoints();

		if (endpoints.count <= index) {
			cerr << "Invalid device index.\n";
			return;
		}

		IMMDevice* device;
		endpoints.devices->Item(index, &device);

		IAudioEndpointVolume* volume;
		device->Activate(
			__uuidof(IAudioEndpointVolume),
			CLSCTX_ALL, 
			NULL, 
			(void**)&volume
		);

		BOOL is_muted;
		CHECK(volume->GetMute(&is_muted));
		CHECK(volume->SetMute(!is_muted, &context_));
	}
};

DeviceManager::DeviceManager() : impl_{new DeviceManagerImpl()}
{
}

vector<string> DeviceManager::get_device_names() const {
	return impl_->get_device_names();
}

void DeviceManager::mute(unsigned int index) const {
	impl_->mute((UINT)index);
}

// Can't let the compiler auto-generate a destructor until after DeviceManagerImpl is defined
DeviceManager::~DeviceManager() = default;