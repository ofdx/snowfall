#define SAVE_FILE_NAME "SAVEDATA"

// Structure defining player save data. Changing the order and size of these
// fields will break save compatibility.
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
	FILE *outfile = fopen(SAVE_FILE_NAME, "w");

	if(outfile){
		size_t count = fwrite(data, sizeof(struct player_data), 1, outfile);

		fclose(outfile);

		// True on success.
		return (count == 1);
	}

	return false;
}

// Read player data from SAVEDATA file.
player_data* player_data_read(){
	FILE *infile = fopen(SAVE_FILE_NAME, "r");

	if(infile){
		player_data *data = (player_data*) malloc(sizeof(struct player_data));
		size_t count = fread(data, sizeof(struct player_data), 1, infile);

		fclose(infile);

		if(count != 1){
			free(data);
			return NULL;
		}

		return data;
	}

	return NULL;
}
