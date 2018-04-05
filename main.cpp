#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "cancel.h"
#include "device.h"

using namespace std;

enum class Mode {
	Help,
	List,
	Toggle
};

struct action {
	int device;
	int sleep;
};

class Main {
private:
	DeviceManager manager_;
	const string program_name_;
	unsigned int device_;
	unsigned int sleep_ = 0;

	Mode parse_args(const vector<string>& args) {
		auto b = args.begin(), e = args.end();

		if (args.empty() || find(b, e, "-h") != args.end()) {
			return Mode::Help;
		}

		if (find(b, e, "-l") != args.end()) {
			return Mode::List;
		}

		if (b != e) {
			device_ = stoi(*b++);
			if (b != e) {
				sleep_ = stoi(*b++);
			}
		}

		return Mode::Toggle;
	}
	
	void print_help() {
		const auto width = 20;
		cerr << "Usage: \n"
			<< program_name_ << left << setw(width) << " -h:" << "help\n"
			<< program_name_ << left << setw(width) << " -l:" << "list devices\n"
			<< program_name_ << left << setw(width) << " device_id:" << "mute device_id\n"
			<< program_name_ << left << setw(width) << " device_id sleep:" << 
				"mute device_id, then sleep for sleep seconds, then unmute device_id\n"
		;
	}

	void list_devices(DeviceManager& manager) {
		auto devices = manager.get_device_names();

		int i = 0;
		for (const auto& device : devices) {
			cout << i++ << ":" << device << "\n";
		}
	}

public:
	Main(const string& program_name, const vector<string>& args) : program_name_{ program_name } {
		const auto mode = parse_args(args);
		if (mode == Mode::Help) {
			print_help();
			return;
		}

		if (mode == Mode::List) {
			list_devices(manager_);
			return;
		}

		// mode == Mode::Toggle
		manager_.mute(device_);
		add_cancel_handler([&] {
			// If the user hits ctrl+c while we're sleeping, assume they want to turn the sound back on.
			manager_.mute(device_);
		});

		if (sleep_ > 0) {
			this_thread::sleep_for(sleep_ * 1s);
			manager_.mute(device_);
		}
	}
};

int main(int argc, char** argv) {
	const string program_name = argv[0];
	vector<string> args{ &argv[1], &argv[argc] };
	
	Main main_(program_name, args);
}