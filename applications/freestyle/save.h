#pragma once
#include <string>
#include <memory>
#include <map>
#include <vector>


class save_t {
	public:
		struct parsed_data_t {
			std::string time_created = "no_time_set";
			std::string playername = "playername could not be loaded";
			int playerlevel = 0;
			int checkpointid = 0;
			int64_t time_in_sec;
			bool auto_save = false;
			float soup_height = -20.f;
			std::vector<std::string> playermoves;
			std::vector<int> collected_files;
			std::vector<int> commited_files;
		};
		std::shared_ptr<parsed_data_t> data;
		static std::shared_ptr<std::map<int64_t, std::shared_ptr<save_t>>> data_map;



		static std::shared_ptr<std::map<int64_t, std::shared_ptr<save_t>>> init_saves();

		std::string file_path;
		bool deleted = false;

		save_t(std::string playername);
		static std::shared_ptr<save_t> current_auto_save;
		static std::shared_ptr<std::map<int64_t, std::shared_ptr<save_t>>> update_saves();
		bool loaded = false;
		void save_me(bool auto_save = false);
		void load_me();
		void apply_current_files();
		bool draw();
		std::string get_collapse_header();
};
