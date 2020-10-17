#include "save.h"
#include <cppgl/file_util.h>
#include <iostream>
#include <cstdlib>
#include <imgui/imgui.h>
#include "gui.h"
#include <cstdio>
#include <filesystem>
#include <ctime>
#include <algorithm>
#include <chrono>
#include "battle_system/battlehandler.h"
#include "soup.h"
#include "checkpoints.h"
#include "files.h"

using namespace std;

extern shared_ptr<Soup> the_soup;
extern vector<string> std_moves, std_future_moves;

std::shared_ptr<std::map<int64_t,std::shared_ptr<save_t>>> save_t::data_map;
std::shared_ptr<save_t> save_t::current_auto_save;

shared_ptr<map<int64_t, shared_ptr<save_t>>> save_t::init_saves() {
	update_saves();
	return data_map;
}

save_t::save_t(string playername) {
	data = make_shared<parsed_data_t>();
	data->playername = playername;
	data->playerlevel = 1;
	data->checkpointid = 0;
	data->soup_height = -200.f;
	data->playermoves = std_moves;
}

bool is_bad_char(char c) {
	if(c >= '0' && c <= '9') return false;
	if(c >= 'a' && c <= 'z') return false;
	if(c >= 'A' && c <= 'Z') return false;
	return true;
}

void save_t::save_me(bool auto_save) {
	update_saves();
	if(auto_save) {
		// if(data->auto_save) {
		// 	cerr << "I am the auto_save" << endl;
		// 	data_map->erase(-1 * data->time_in_sec);
		// 	std::remove(file_path.c_str());
		// } else {
		// int64_t dl_this = -1;
		for(auto &sa : *data_map) {
			auto save_tmp = sa.second;
			if(save_tmp->data->auto_save) {
				std::string s = save_tmp->file_path;
				// cout << "deletion of " << s << " was ";
				// if(!std::remove(s.c_str())) { cout << "succesful" << endl;} else { cout << "not succesful" << endl; }
				std::remove(s.c_str());
				// dl_this = sa.first;
			}
		}
		// if(dl_this != -1) { data_map->erase(dl_this); /*current_auto_save = 0;*/ }
		// }
		update_saves();
	}
	auto t = time(NULL);
	struct tm *lt = localtime(&t);
	auto mkt = mktime(lt);
	string time_string(ctime(&mkt));
	string time_str = time_string;
	data->soup_height = the_soup->pos.y;
	data->checkpointid = Checkpoints::closest_checkpoint_to_player->id;
	data->auto_save = auto_save;
	time_string.erase(remove_if(time_string.begin(), time_string.end(), is_bad_char), time_string.end());
	int64_t sec = std::chrono::duration_cast<chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	// cout << "It is: " << time_string << endl;
	ofstream file_out(concat(concat(EXECUTABLE_DIR, "saves/"), time_string + to_string(sec)));
	file_out << "# FINAL COMMIT SAVE FILE DONT CHANGE A THING! #" << endl;
	file_out << time_str;
	file_out << data->playername << endl;
	file_out << ((auto_save) ? 1 : 0)  << endl;
	file_out << sec << endl;
	file_out << data->soup_height << endl;
	file_out << data->playerlevel << endl;
	file_out << data->commited_files.size() << endl;
	for(auto &elem : data->commited_files) {
		file_out << elem << endl;
	}
	file_out << data->collected_files.size() << endl;
	for(auto &elem : data->collected_files) {
		file_out << elem << endl;
	}
	file_out << data->checkpointid << endl;
	data->playermoves = BattleHandler::Player->moves;
	file_out << data->playermoves.size() << endl;
	for(auto &elem : data->playermoves) {
		file_out << elem << endl;
	}
}

void save_t::apply_current_files() {
	data->collected_files.clear();
	data->commited_files.clear();
	for(auto &elem : Files::collected) {
		data->collected_files.push_back(elem->id);
	}
	for(auto &elem : Files::commited) {
		data->commited_files.push_back(elem->id);
	}
}

void save_t::load_me() {
	BattleHandler::Player->level = data->playerlevel;
	vector<string> new_future_moves;
	for(auto &f : std_future_moves) {
		bool already_learned = false;
		for(auto &mov : data->playermoves) {
			if(f == mov) { already_learned = true; break; }
		}
		if(!already_learned) new_future_moves.push_back(f);
	}
	BattleHandler::Player->future_moves = new_future_moves;
	BattleHandler::Player->moves.clear();
	for(auto &elem : data->playermoves) {
		BattleHandler::Player->moves.push_back(elem);
	}
	the_soup->pos = glm::vec3(0, data->soup_height, 0);

	Files::files.clear();
	Files::files.insert(Files::files.begin(), Files::all_files.begin(), Files::all_files.end());
	Files::commited.clear();
	Files::collected.clear();
	for(auto &i : data->commited_files) {
		auto fi = Files::all_files[i];
		Files::commited.push_back(fi);
		Files::files.erase(find(Files::files.begin(), Files::files.end(), fi));
	}
	for(auto &i : data->collected_files) {
		auto fi = Files::all_files[i];
		Files::collected.push_back(fi);
		Files::files.erase(find(Files::files.begin(), Files::files.end(), fi));
	}
}

std::shared_ptr<std::map<int64_t, std::shared_ptr<save_t>>> save_t::update_saves() {
	data_map = make_shared<map<int64_t, shared_ptr<save_t>>>();
	vector<string> files_in_dir;
	list_dir(EXECUTABLE_DIR, "saves", files_in_dir);
	// cout << "Reading " << files_in_dir.size() << " save files: " << endl;
	for(auto &file : files_in_dir) {
		if(file[6] == '.') continue;
		auto save = make_shared<save_t>(file);
		auto data = save->data;
		save->file_path = concat(EXECUTABLE_DIR, file);
		ifstream inp_file(save->file_path);
		if(!inp_file.is_open()) { cerr << "cannot read file: "  << file << endl; continue; }
		stringstream file_stream;
		file_stream << inp_file.rdbuf();
		string tmp;
		if(!getline(file_stream, tmp)) { cerr << "could not read line on file: " << file << endl; continue; }
		if(tmp != "# FINAL COMMIT SAVE FILE DONT CHANGE A THING! #") { cerr << "magic string did not match" << endl; continue; }
		if(!getline(file_stream, tmp)) { cerr << "could not read line" << endl; continue; }
		data->time_created = tmp;
		if(!getline(file_stream, tmp)) { cerr << "could not read line2" << endl; continue; }
		data->playername = tmp;
		int boolean = 0;
		file_stream >> boolean;
		if(boolean) data->auto_save = true;
		file_stream >> data->time_in_sec;
		file_stream >> data->soup_height;
		file_stream >> data->playerlevel;

		int commited_count = 0;
		file_stream >> commited_count;
		for(int i = 0; i < commited_count; i++) {
			int id;
			file_stream >> id;
			data->commited_files.push_back(id);
		}
		int collected_count = 0;
		file_stream >> collected_count;
		for(int i = 0; i < collected_count; i++) {
			int id;
			file_stream >> id;
			data->collected_files.push_back(id);
		}

		file_stream >> data->checkpointid;
		int move_count;
		file_stream >> move_count;
		if(!getline(file_stream, tmp) && move_count != 0) { cerr << "could not read line3" << endl << "data i read until now: " << endl << "name: " << data->playername << endl << "time in millis: " << data->time_in_sec << " lvl: " << data->playerlevel << " cpid: " << data->checkpointid << endl; continue; }
		for(int i = 0; i < move_count && getline(file_stream, tmp); i++) {
			if(find(std_moves.begin(), std_moves.end(), tmp) == std_moves.end()) data->playermoves.push_back(tmp);
		}
		if(data->auto_save) current_auto_save = save;
		(*data_map)[-1 * data->time_in_sec] = save;
		// cout << "read in file " << file << endl;
		// cout << "it was created on " << data->time_created << endl;
		// cout << "Player is level " << data->playerlevel << endl;
		// cout << "He saved at checkpoint " << data->checkpointid << endl;
		// cout << "He can use those attacks: " << endl;
		// for(auto &v : data->playermoves) {
		// 	cout << v << " " << endl;
		// }
		// cout << endl;
	}
	// cout << endl;
	return data_map;
}

bool save_t::draw() {
	ImGui::TextWrapped("Created save on: %s", data->time_created.c_str());
	ImGui::TextWrapped("Playername is: %s" , data->playername.c_str());
	ImGui::TextWrapped("Player Level is: %d" , data->playerlevel);
	ImGui::TextWrapped("You saved at checkpoint: %d", data->checkpointid);
	string all_attacks = "";
	for(auto &e : data->playermoves) {
		all_attacks += e;
		all_attacks += ", ";
	}
	if(all_attacks.size() > 1) {
		all_attacks.pop_back();
		all_attacks.pop_back();
	}
	ImGui::TextWrapped("Moves: %s", all_attacks.c_str());
	// string all_commited = "";
	// for(auto &file_id : data->commited_files) {
	// 	all_commited += to_string(file_id) + ", ";
	// }
	// if(all_commited.size() > 1) {
	// 	all_commited.pop_back();
	// 	all_commited.pop_back();
	// }
	// ImGui::TextWrapped("%lu Files commited: %s", data->commited_files.size(), all_commited.c_str());
	ImGui::TextWrapped("%lu Files commited", data->commited_files.size());
	// string all_collected = "";
	// for(auto &file_id : data->collected_files) {
	// 	all_collected += to_string(file_id) + ", ";
	// }
	// if(all_collected.size() > 1) {
	// 	all_collected.pop_back();
	// 	all_collected.pop_back();
	// }
	// ImGui::TextWrapped("%lu Files collected: %s", data->collected_files.size(), all_collected.c_str());
	ImGui::TextWrapped("%lu Files collected", data->collected_files.size());
	bool b = ImGui::Button(("Load game##" + to_string(data->time_in_sec)).c_str());
	bool delete_b = ImGui::Button(("Delete save##" + to_string(data->time_in_sec)).c_str());
	if(delete_b) { std::string s = file_path; /* cout << "Trying to remove: " << s << endl << */ std::remove(s.c_str()) /* << endl */; deleted = true; }
	return b;
}

string save_t::get_collapse_header() {
	return (data->auto_save ? "AUTOSAVE: " : "") + data->time_created;
}
