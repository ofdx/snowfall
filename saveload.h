#define SAVE_FILE_NAME "SAVEDATA"

#include <fstream>

// Structure defining player save data. Changing the order and size of these
// fields will break save compatibility, so do so at your peril.
struct player_data {
	// Player character's name.
	char name[16];

	// Space for future strings. Clan name, class name, etc.
	char padding1[4096];

	// Identifier indicating which scene the player was on.
	int scene_main, scene_sub;

	// Player position in physical space.
	double x, y, z;

	// Player camera point direction vector.
	double point_x, point_y, point_z;

	// Space for future data.
	char padding2[4096];
};

// Write player data to SAVEDATA file. Returns true on success.
bool player_data_save(player_data *data){
	ofstream outfile(get_save_path() + SAVE_FILE_NAME);

	if(outfile){
		if(outfile.write((char*) data, sizeof(struct player_data)))
			return true;
	}

	return false;
}

// Read player data from SAVEDATA file.
player_data* player_data_read(){
	ifstream infile(get_save_path() + SAVE_FILE_NAME);

	if(infile){
		player_data *data = (player_data*) malloc(sizeof(struct player_data));
		infile.read((char*) data, sizeof(struct player_data));

		if(infile.gcount() != sizeof(struct player_data)){
			free(data);
			return NULL;
		}

		return data;
	}

	return NULL;
}
