/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
    Catacombs

    Inspired by Realmfall's "Nightmare Catacombs" revealed in Season 3, Chapter "Cue, The Banjo", 
    where the cast escapes a horde through the sewers, only to end up in a labrynth ruled by 
    solely three entities:

    - One blind, but with hearing so sharp, it can detect a fast heartbeat.
    - One deaf, but with sight so sharp, it can see through the cracks of hiding spots.
    - One blind and deaf, but it can sense movement.

    The Catacombs are a 128x128 grid of tiles, where internally (through integer value):
        - 0 = floor 
        - 1 = wall
        - 2 = hiding spot
        - 3 = treasure chest
    The player is marked with a "P" on the map. Only floors and hiding spots within direct 
    line of sight are revealed, and the player can still only see up to 10 tiles around them.
    Entities are marked with an "X" on the map, but they are only visible when the player
    can directly see them.

    TURNS:
        The player moves 1 tile per turn, or can skip a turn and do nothing.
        Doing nothing in a turn can be strategic.
        Checking your heartrate does not cost a turn.

    CONTROLS:
        - W: Move North
        - A: Move West
        - S: Move South
        - D: Move East
        - E: Forfeit turn (Do nothing)
        - Q: Check heartrate

        To hide, move into a hiding spot.
        To open a chest and get an item, move into a treasure chest tile.


    There's three game mechanics in mind:
    - Vision:   The deaf entity cannot see past corners or sufficient-enough hiding spots.
                HOWEVER, unlike the disturbance entity, standing completely still will not
                work, as it can see that you're clearly there.
                If it sees the player in direct line of sight, the player will be notified by
                "You spot something that stands out brightly against the dull catacombs."
                It moves 1 tile every 4 turns when out of aggro.
                It moves 1 tile every 2 turns when in aggro.

    - Sound:    The blind entity cannot hear through walls, but sounds do bounce off of them.
                It will only be able to hear through a clear path of up to 10 tiles.
                Noises are made by movement without padded shoes, hiding in closable objects,
                and from other entities.
                Due to its nature to investigate sounds regardless of source, it is often
                not alone.

                If it is within 4 tiles of the player, it will check the player's heart rate.
                If the player's heart rate is above 85 BPM, it will hear them and give chase.
                The player's heart rate drops 2 BPM down to 70 BPM per turn when doing nothing.
                It is raised up to 100 BPM by 1 BPM per turn when moving.

                If it hears the player within 10 tiles, the player will be notified by
                "Metal shoes tap against the ground..."
                It moves 1 tile every 4 turns when out of aggro.
                It moves 1 tile every turn when in aggro.

    - Movement: The blind & deaf entity cannot see or hear, but it will sense player actions.
                It will sense anything move through walls within a 20-tile radius.
                If it senses the player moving within 10 tiles, the player will be notified by
                "You hear chains clatter and a blade screeching against the stone floors..."
                If the player stands still for 3 turns, it will leave, appearing at least 50 
                tiles away from the player.
                It moves 1 tile every 2 turns when out of aggro.
                It moves 1 tile every turn when in aggro.
    
    If the player doesn't escape either of the entities successfully, they will lose.

    There is no winning condition, Catacombs is a survival game. The score is the number of
    turns survived. A leaderboard is updated upon the losing condition being met.

*/


#include <stdio.h>

//Define constants for map dimensions
#define MAP_WIDTH 128
#define MAP_HEIGHT 128

// Game state variables
int player_x, player_y;
int player_score; // turns survived + current turn count
int player_heartrate = 70;
char player_hidden = 0; // 0 = not hidden, 1 = hidden
char* player_map[MAP_HEIGHT][MAP_WIDTH]; // player's revealed map
char* entity_map[MAP_HEIGHT][MAP_WIDTH]; // entity positions on map
char* map_names[100]; // array to hold custom map names

// global map variable
int map[MAP_HEIGHT][MAP_WIDTH];



/*
    Map layout key:
        0 = Floor
        1 = Wall
        2 = Hiding Spot
        3 = Treasure Chest

    Entity types (placements are random):
        0 = Blind (Hearing)
        1 = Deaf (Sight)
        2 = Blind & Deaf (Movement)
*/

/* Function prototypes */
void initialize_game();
void update_game();
void render_game();
void cleanup_game();
int load_map_from_file(const char* filename);
void save_scoreboard(const char* map_name, int score);

/*
    Map file reading and detection

    For custom maps, the game will read from a text file formatted as follows:
    - The first line contains two integers separated by a space, representing the width and height of the map.
    - The subsequent lines contain the map layout, with each tile represented by an integer value (0-3) separated by spaces.
    - Example:
        10 10
        1 1 1 1 1 1 1 1 1 1
        1 0 0 0 2 0 0 0 3 1
        1 0 1 1 1 1 0 1 0 1
        ...
    The game will parse this file to create the internal representation of the map.
    The file name can be specified as a command-line argument; if none is provided, a default map will be used.

    Scoreboards will be made for custom maps, identified by the map file name.

    File extensions:
        - Map files should use the ".catamap" extension.
        - Scoreboard files should use the ".catascore" extension.

    Note: Error handling for file reading and parsing is essential to ensure robustness.
    If the map file is invalid or cannot be read, the game should gracefully fall back to a default map.

    Returns:
        0 on success
        1 on failure
*/ 

int load_map_from_file(const char* filename) {
    // Implementation for loading map from file goes here
    printf("Loading map from file: %s\n", filename);
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening map file");
        // Ask the user if they would like to play the default map instead
        char choice;
        printf("Would you like to play the default map instead? (y/n): ");
        scanf(" %c", &choice);
        if (choice == 'y' || choice == 'Y') {
            load_map_from_file("default.catamap");
        } // else exit the game
        else {
            return 1;
        }
    }

    // Read map dimensions
    int width, height;
    fscanf(file, "%d %d", &width, &height);
    printf("Map dimensions: %dx%d\n", width, height);

    // Read map layout
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            fscanf(file, "%d", &map[y][x]);
        }
    }

    fclose(file);
    return 0; // success
}


// Main game loop, takes care of initialization, updating, rendering, and cleanup
int main(int argc, char* argv[]) {
    int map_load_status;
    if (argc > 1) {
        map_load_status = load_map_from_file(argv[1]);
    } else {
        map_load_status = load_map_from_file("default.catamap");
    }

    if (map_load_status != 0) {
        printf("Failed to load a valid map. Exiting game.\n");
        return 1;
    }
    
    initialize_game();

    while (1) {
        update_game();
        render_game();
    }

    cleanup_game();
    return 0;
}

void initialize_game() {
    // Initialize player position, health, score, and other game state variables
    player_x = MAP_WIDTH / 2;
    player_y = MAP_HEIGHT / 2;
    player_score = 0;

    // Additional initialization code goes here
}

void update_game() {
    // Update game state based on player input and entity behaviors
    // This function will handle movement, entity AI, collision detection, etc.

    // Add to player score each turn
    player_score++;
}

void render_game() {
    // Render the current game state to the console or graphical interface
    // This function will display the map, player, entities, and other relevant information

    // Placeholder for rendering game state
    printf("Player Position: (%d, %d) | Score: %d\n", player_x, player_y, player_score);
}

void cleanup_game() {
    // Cleanup resources and perform any necessary shutdown procedures
    printf("Cleaning up game resources...\n");
}

void save_scoreboard(const char* map_name, int score) {
    // Implementation for saving scoreboard to file goes here
    char filename[256];
    snprintf(filename, sizeof(filename), "%s.catascore", map_name);

    FILE* file = fopen(filename, "a");
    if (!file) {
        perror("Error opening scoreboard file");
        return;
    }

    fprintf(file, "Score: %d\n", score);
    fclose(file);
}