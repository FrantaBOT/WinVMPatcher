#include "memflow.hpp"
#include <fstream>
#include <vector>
#include <sstream>
#include <iostream>

using namespace std;

struct PatchData {
    unsigned long offset;
    char original;
    char patch;
};

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s memory_diff_path module_name\n -cn - connector name\n -ca - connector arg\n -on - os name\n -oa - os arg\n -im - ignore mismatch\n -ll - log level\n", argv[0]);

        return 1;
    }

	const char *memory_diff_path = argv[1];
	const char *module_name = argv[2];

	const char *connector_name = "qemu";
	const char *connector_arg = "";
	const char *os_name = "win32";
	const char *os_arg = "";
	bool ignore_mismatch = false;
	int log_level = 3;

	for (int i = 3; i < argc; i++) {
		if (!strcmp(argv[i],"-im")) {
			ignore_mismatch = true;
		} else if (i + 1 >= argc) {
            continue;
		} else if (!strcmp(argv[i],"-cn")) {
			connector_name = argv[++i];
		} else if (!strcmp(argv[i],"-ca")) {
			connector_arg = argv[++i];
		} else if (!strcmp(argv[i],"-on")) {
			os_name = argv[++i];
		} else if (!strcmp(argv[i],"-oa")) {
			os_arg = argv[++i];
		} else if (!strcmp(argv[i],"-ll")) {
			log_level = atoi(argv[++i]);

			if (log_level < 0 || log_level > 5) {
				printf("Log level is from 0 to 6 [Off, Error, Warn, Info, Debug, Trace]\n");

				return 1;
			}
		}
	}

    fstream input_file(memory_diff_path);
    if (!input_file.is_open()) {
		printf("Failed to open memory diff file\n");

        return 1;
    }

    string line;
	vector<PatchData> patch_data_list;

    while (getline(input_file, line)) {
        istringstream iss(line);

        string offset_str, original_str, patch_str;
        iss >> offset_str >> original_str >> patch_str;

        unsigned long offset = stoi(offset_str, nullptr, 16);
        char original = stoi(original_str, nullptr, 16);
        char patch = stoi(patch_str, nullptr, 16);

        patch_data_list.push_back({ offset, original, patch });
    }

    input_file.close();

	log_init((LevelFilter)log_level);

	Inventory *inventory = inventory_scan();
	if (!inventory) {
		printf("Unable to create inventory\n");

		return 1;
	}

	ConnectorInstance<> connector;
	OsInstance<> os;

	bool connector_initializing = true;
	bool os_initializing = true;
	while (connector_initializing || os_initializing) {
		if (!(connector_initializing = inventory_create_connector(inventory, connector_name, connector_arg, &connector))) {
			os_initializing = inventory_create_os(inventory, os_name, os_arg, &connector, &os);
		}
	}

	connector_drop(&connector);
	inventory_free(inventory);

	ModuleInfo module_info;
	CSliceRef<uint8_t> module_name_ref = CSliceRef<uint8_t>(module_name);

	bool module_initializing = true;
	while (module_initializing) {
		module_initializing = os.module_by_name(module_name_ref, &module_info);
	}

	char value;
	CSliceMut<uint8_t> value_mut = CSliceMut<uint8_t>(&value, sizeof(value));
	CSliceRef<uint8_t> value_ref = CSliceRef<uint8_t>(&value, sizeof(value));

	bool memory_initializing = true;
	while (memory_initializing) {
		memory_initializing = os.read_raw_into(module_info.base, value_mut);
	}

	bool mismatch = false;
	for (auto pi = patch_data_list.begin(); pi != patch_data_list.end(); ) {
		if (!os.read_raw_into(module_info.base + pi->offset, value_mut)) {
			printf("Pre-patch %#010lx value %02X, patch original %02X\n", pi->offset, (unsigned char)value, (unsigned char)pi->original);
			if (value == pi->original) {
				pi++;
				continue;
			}
		} else {
			printf("Failed to read pre-patch value %#010lx\n", pi->offset);
		}
		
		mismatch = true;
		pi = patch_data_list.erase(pi);
    }

	if (mismatch) {
		printf("Memory and patch original value does not match\n");
		if (ignore_mismatch) {
			printf("Ignoring\n");
		} else {
			printf("Exiting\n");
			os_drop(&os);
			
			return 1;
		}
	}

	for (const auto& patch_data : patch_data_list) {
		value = patch_data.patch;

		if (!os.write_raw(module_info.base + patch_data.offset, value_ref)) {
			printf("Patched %#010lx to %02X\n", patch_data.offset, (unsigned char)value);
		} else {
			printf("Failed to patch %#010lx!\n", patch_data.offset);
		}
	}

	for (const auto& patch_data : patch_data_list) {
		if (!os.read_raw_into(module_info.base + patch_data.offset, value_mut)) {
			printf("Post-patch %#010lx value %02X\n", patch_data.offset, (unsigned char)value);
		} else {
			printf("Failed to read post-patch value %#010lx\n", patch_data.offset);
		}
    }

	os_drop(&os);

	return 0;
}