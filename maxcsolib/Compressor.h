#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace maxcsolib {

	const std::string ANSI_RESET_LINE = "\033[2K\033[0G";

	struct Arguments {
		std::vector<std::string> inputs;
		std::vector<std::string> outputs;
		std::string output_path;
		int threads;
		uint32_t block_size;

		// Let's just use separate vars for each and figure out at the end.
		// Clearer to translate the user's logic this way, with defaults.
		uint32_t flags_fmt;
		uint32_t flags_use;
		uint32_t flags_no;
		uint32_t flags_only;
		uint32_t flags_final;

		double orig_cost_percent;
		double lz4_cost_percent;

		bool fast;
		bool smallest;
		bool quiet;
		bool crc;
		bool decompress;
		bool measure;
	};

	void default_args(Arguments& args);

	class Compressor {
	public:
		Compressor();
		~Compressor();

		void Compress(Arguments);
	};

}