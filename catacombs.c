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


    There's three entities in mind, with their own mechanics:
    - Vision:   The deaf entity cannot see past corners
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
                It moves 2 tiles every turn when in aggro.

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

    "Good luck, and godspeed" - My discrete math professor, 2025

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Game state variables
int player_x, player_y;
int player_score; // turns survived + current turn count
int player_heartrate = 70;
char player_hidden = 0; // 0 = not hidden, 1 = hidden
// char* player_map[MAP_HEIGHT][MAP_WIDTH]; // player's revealed map
// char* entity_map[MAP_HEIGHT][MAP_WIDTH]; // entity positions on map
char* map_name[100]; // array to hold custom map names

int map_width;
int map_height;


int** map_dyn; // 2D array representing the catacombs map, for malloc
int** entity_positions; // 2D array representing entity & player positions, for malloc
int* player_map[21][21]; // 21x21 array representing player's revealed map.

int platform_clear_command_supported = 1; // Set to 0 if the platform does not support console clear command
int should_update_render = 1; // Flag to control rendering updates

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
int initialize_game();
// game logic updates
void update_player_bpm(int flag);
int update_player_position(int dx, int dy);
int update_game();
// game rendering and cleanup
void render_game();
void cleanup_game();
int load_map_from_file(const char* filename);
void save_scoreboard(const char* map_name, int score);
void create_default_map();
void line_of_sight(int map[21][21], int visibility[21][21], int origin_x, int origin_y);
int is_line_of_sight(int map[21][21], int x1, int y1, int x2, int y2);
// Utility functions
int random_number_range(int min, int max);
int random_bool();

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
    int width = 0, height = 0;
    fscanf(file, "%d %d", &width, &height);
    printf("Map dimensions: %dx%d\n", width, height);
    map_width = width;
    map_height = height;
    // Set map name for scoreboard purposes
    snprintf((char*)map_name, sizeof(map_name), "%s", filename);
    
    // Allocate memory for the map
    map_dyn = malloc(height * sizeof(int*));
    for (int i = 0; i < height; i++) {
        map_dyn[i] = malloc(width * sizeof(int));
    }

    // check if malloc succeeded
    if (map_dyn == NULL) {
        perror("Error allocating memory for map");
        fclose(file);
        return 1;
    }

    // Populate the map array
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            fscanf(file, "%d", &map_dyn[y][x]);
        }
    }

    // print the loaded map for verification
    // printf("Loaded Map:\n");
    // for (int y = 0; y < height; y++) {
    //     for (int x = 0; x < width; x++) {
    //         printf("%d ", map_dyn[y][x]);
    //     }
    //     printf("\n");
    // }

    fclose(file);
    return 0; // success
}

// Create a default map if no custom map is provided
void create_default_map() {
    // Write default map to "default.catamap"
    FILE* file = fopen("default.catamap", "w");
    if (!file) {
        perror("Error creating default map file");
        return;
    }
    int map_size_def = 50, treasures_placed = 0;

    // Write default map dimensions in header
    fprintf(file, "%d %d\n", map_size_def, map_size_def);

    // Seed random number generator
    srand((unsigned int)time(NULL));

    // Write default map layout
    // TODO: write more specific rules before printing to file to avoid holes, excessive dead-ends and unreachable floors.
    for (int y = 0; y < map_size_def; y++) {
        for (int x = 0; x < map_size_def; x++) {
            if (x == 0 || x == map_size_def-1 || y == 0 || y == map_size_def-1) { // Prints border of map with walls.
                fprintf(file, "1 ");
            } else { // Process anything within the borders
                // Simple random generation for now
                int tile_choice = random_number_range(0, 100);
                if (tile_choice < 20) { // 20% chance to make a wall
                    fprintf(file, "1 ");
                } else if (tile_choice < 30) { // 10% chance of making a floor be a hiding spot instead.
                    fprintf(file, "2 ");
                } else if (tile_choice < 31 && treasures_placed < 3) { // up to 3 treasures exist in a map
                    fprintf(file, "3 ");
                    treasures_placed++;
                } else {
                    fprintf(file, "0 ");
                }
            }
        }
        fprintf(file, "\n");
    }


    fclose(file);
}



// Main game loop, takes care of initialization, updating, rendering, and cleanup
int main(int argc, char* argv[]) {
    int map_load_status;
    if (argc > 1) {
        map_load_status = load_map_from_file(argv[1]);
    } else {
        // Check if default map exists
        FILE* default_file = fopen("default.catamap", "r");
        if (default_file) {
            fclose(default_file);
        } else {
            // Create default map if it doesn't exist
            create_default_map();
            perror("Default map file not found");
        }
        // Load default map
        map_load_status = load_map_from_file("default.catamap");
    }

    if (map_load_status != 0) {
        printf("Failed to load a valid map. Exiting game.\n");
        return 1;
    }

    if (initialize_game() != 0) {
        printf("Failed to initialize game. Exiting.\n");
        return 1;
    }

    int gameState = 1; // 1 = running, 0 = game over

    render_game();

    while (gameState) {
        gameState = update_game();
        if (should_update_render==1) {
            render_game();
        }
    }

    save_scoreboard((const char*)map_name, player_score);

    cleanup_game();
    return 0;
}


int initialize_game() {
    // Initialize player position, health, score, and other game state variables
    player_score = 0; // Start on turn 0

    // Get current operating system for console clear command
    #ifdef _WIN32
        platform_clear_command_supported = 1; // Windows supports "cls"
    #else
        platform_clear_command_supported = 0; // Other platforms may not support clear command
    #endif


    // Player placements
    // attempt to randomly place the player on a floor tile
    srand((unsigned int)time(NULL)); // Seed the random number generator
    do {
        player_x = random_number_range(1, map_width - 2); // avoid placing on border walls
        player_y = random_number_range(1, map_height - 2);
    } while (map_dyn[player_y][player_x] != 0); // repeat until a floor tile is found


    // Entity placements
    // Place entities at least 1/4th the map size away from the player
    // attempt to place them on floor tiles
    entity_positions = malloc(3 * sizeof(int*)); // 3 entities
    // check if malloc succeeded
    if (entity_positions == NULL) {
        perror("Error allocating memory for entity positions");
        return 1; // failure
    }

    for (int i = 0; i < 3; i++) {
        entity_positions[i] = malloc(2 * sizeof(int)); // x and y positions
        int ex, ey;
        do {
            ex = random_number_range(1, map_width - 2);
            ey = random_number_range(1, map_height - 2);
        } while (map_dyn[ey][ex] != 0 || // must be on floor tile
                 abs(ex - player_x) < map_width / 4 || // must be at least 1/4th map width away
                 abs(ey - player_y) < map_height / 4); // must be at least 1/4th map height away
        entity_positions[i][0] = ex;
        entity_positions[i][1] = ey;
    }

    // Verify player placement is on a floor tile
    if (map_dyn[player_y][player_x] != 0) {
        printf("Error: Player not placed on a floor tile!\n");
        return 1; // failure
    }

    // Print initial positions for verification
    printf("Player starting position: (%d, %d)\n", player_x, player_y);
    for (int i = 0; i < 3; i++) {
        printf("Entity %d starting position: (%d, %d)\n", i, entity_positions[i][0], entity_positions[i][1]);
    }

    return 0; // success
}

void update_player_bpm(int flag) {
    if (flag == 1) { // moving
        player_heartrate += 1;
        if (player_heartrate > 100) {
            player_heartrate = 100; // cap at 100 BPM
        }
    } else if (flag == 0) { // doing nothing
        player_heartrate -= 2;
        if (player_heartrate < 70) {
            player_heartrate = 70; // floor at 70 BPM
        }
    }
}

// returns 1 if move was valid and executed, 0 otherwise
int update_player_position(int dx, int dy) {
    // check if movement would run into walls or out of bounds
    if (player_x + dx < 0 || player_x + dx >= map_width || player_y + dy < 0 || player_y + dy >= map_height) {
        return 0; // invalid move, do nothing
    }
    if (player_y + dy > 0 && map_dyn[player_y + dy][player_x + dx] != 1) {
        update_player_bpm(1);
        player_x += dx;
        player_y += dy;
        return 1;
    }
    return 0;
}

int update_game() {
    // Update game state based on player input and entity behaviors
    // This function will handle movement, entity AI, collision detection, etc.
    char input;
    printf("Enter your move (W/A/S/D to move, E to skip turn, Q to check heartrate): ");
    scanf(" %c", &input);
    // stdin flush
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}

    // Convert char to uppercase for easier handling
    if (input >= 'a' && input <= 'z') {
        input = input - ('a' - 'A');
    }

    int valid_move;

    // Process player input
    switch (input) {
        case 'W':
            valid_move = update_player_position(0, -1);
            if (valid_move) {
                break;
            }
            return 1;
        case 'A':
            valid_move = update_player_position(-1, 0);
            if (valid_move) {
                break;
            }
            return 1;
        case 'S':
            valid_move = update_player_position(0, 1);
            if (valid_move) {
                break;
            }
            return 1;
        case 'D':
            valid_move = update_player_position(1, 0);
            if (valid_move) {
                break;
            }
            return 1;
        case 'E':
            // Do nothing, skip turn
            update_player_bpm(0);
            break;
        case 'Q':
            printf("Current heartrate: %d BPM\n", player_heartrate);
            should_update_render = 0;
            // Checking heartrate does not cost a turn
            return 1; // continue game without incrementing score
        default:
            printf("Invalid input. Please use W/A/S/D to move, E to skip turn, or Q to check heartrate.\n");
            should_update_render = 0;
            return 1;
    }

    // If we reach here, a valid action was taken that costs a turn and requires re-rendering
    should_update_render = 1;
    // Add to player score each turn
    player_score++;
    return 1; // continue game
}


// Define ASCII / UTF-8 symbols for walls, hiding spots, treasure chests, player, and entities
#define SYMBOL_WALL '#'
#define SYMBOL_FLOOR ' '
#define SYMBOL_HIDING_SPOT 'H'
#define SYMBOL_TREASURE_CHEST 'T'
#define SYMBOL_PLAYER 'P'
#define SYMBOL_ENTITY 'E'
#define SYMBOL_HIDING_PLAYER 'S'


void render_game() {
    // Clear the console (platform-dependent)
    // For Windows
    (platform_clear_command_supported) ? system("cls") : system("clear");
    
    // Render the current game state to the console or graphical interface
    // This function will display the map, player, entities, and other relevant information

    // Print a 21 x 21 section of the map centered around the player
    int start_x = player_x - 10, start_y = player_y - 10, end_x = player_x + 10, end_y = player_y + 10;
    // Ensure the section does not go out of bounds
    if (start_x < 0) {
        start_x = 0;
        end_x = 20;
    }
    if (start_y < 0) {
        start_y = 0;
        end_y = 20;
    }
    if (end_x >= map_width) {
        end_x = map_width - 1;
        start_x = map_width - 21;
    }
    if (end_y >= map_height) {
        end_y = map_height - 1;
        start_y = map_height - 21;
    }
    // store the revealed section in player_map
    for (int y = start_y; y <= end_y; y++) {
        for (int x = start_x; x <= end_x; x++) {
            player_map[y - start_y][x - start_x] = &map_dyn[y][x];
        }
    }

    // Create local map and visibility
    int local_map[21][21];
    for (int y = 0; y < 21; y++) {
        for (int x = 0; x < 21; x++) {
            local_map[y][x] = *player_map[y][x];
        }
    }
    int visibility[21][21] = {0};
    // Calculate player's position in local coordinates
    int player_local_x = player_x - start_x;
    int player_local_y = player_y - start_y;
    line_of_sight(local_map, visibility, player_local_x, player_local_y);

    // RENDERING
    printf("Catacombs Map:\n");
    for (int y = 0; y <= end_y - start_y; y++) {
        for (int x = 0; x <= end_x - start_x; x++) {
            int global_x = start_x + x;
            int global_y = start_y + y;
            if (visibility[y][x] == 0) {
                printf("? ");  // Unrevealed tile
                continue;
            }
            if (global_x == player_x && global_y == player_y) {
                // Update player hidden status based on current tile
                player_hidden = (*player_map[y][x] == 2) ? 1 : 0;
                (player_hidden) ? printf("%c ", SYMBOL_HIDING_PLAYER) : printf("%c ", SYMBOL_PLAYER);
            } else {
                int is_entity_here = 0;
                for (int i = 0; i < 3; i++) {
                    if (entity_positions[i][0] == global_x && entity_positions[i][1] == global_y) {
                        is_entity_here = 1;
                        break;
                    }
                }
                if (is_entity_here) {
                    printf("%c ", SYMBOL_ENTITY);
                } else {
                    switch (*player_map[y][x]) {
                        case 0:
                            printf("%c ", SYMBOL_FLOOR);
                            break;
                        case 1:
                            printf("%c ", SYMBOL_WALL);
                            break;
                        case 2:
                            printf("%c ", SYMBOL_HIDING_SPOT);
                            break;
                        case 3:
                            printf("%c ", SYMBOL_TREASURE_CHEST);
                            break;
                        default:
                            printf("? ");
                            break;
                    }
                }
            }
        }
        printf("\n");
    }
    // END RENDERING

    printf("Player Position: (%d, %d) | Turn: %d\n", player_x, player_y, player_score);
}

void cleanup_game() {
    // Cleanup resources and perform any necessary shutdown procedures
    printf("Cleaning up game resources...\n");
    // Print final map for verification

    // printf("Final Map State:\n");
    // for (int y = 0; y < map_height; y++) {
    //     for (int x = 0; x < map_width; x++) {
    //         printf("%d ", map_dyn[y][x]);
    //     }
    //     printf("\n");
    // }

    // // Free dynamically allocated memory for the map
    // for (int i = 0; i < map_height; i++) { //by row
    //     // Free each row
    //     free(map_dyn[i]);
    //     printf("Freed row %d\n", i+1); // Debugging line
    // }

    free(map_dyn);
}

void save_scoreboard(const char* map_name, int score) {
    // Implementation for saving scoreboard to file goes here
    char filename[256];
    // remove the extension from map_name for the scoreboard filename array
    const char* dot = strrchr(map_name, '.');
    // remove the .catamap extension from map_name for the scoreboard filename array
    snprintf(filename, sizeof(filename), "%.*s.catascore", (int)(dot - map_name), map_name);

    FILE* file = fopen(filename, "a");
    if (!file) {
        perror("Error opening scoreboard file");
        return;
    }

    fprintf(file, "Score: %d \t Date: %s\n", score, __DATE__);
    fclose(file);
}

// UTILITY FUNCTIONS
int seed_count = 0; // Seed count to ensure different seeds on rapid calls
// Gets a random number within the given range:
int random_number_range(int min, int max) {
    //srand((unsigned int)time(NULL) + seed_count++); // Seed the random number generator
    return (rand() % (max - min + 1)) + min;
}
// returns either 0 (false) or 1 (true)
int random_bool() {
    return random_number_range(0,1);
}
// Line of sight function using Bresenham's line algorithm
int is_line_of_sight(int map[21][21], int x1, int y1, int x2, int y2) {
    if (x1 == x2 && y1 == y2) return 1;
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;
    int x = x1, y = y1;
    while (1) {
        if (x != x1 || y != y1) {
            // Check for diagonal blocking: if moving diagonally and both adjacent orthogonal cells are walls, block LOS
            // Checks for both tiles labeled as walls (1) and hiding spots (2) as blocking LOS
            if (sx != 0 && sy != 0) {
                int adj_x = x + sx;
                int adj_y = y + sy;
                // Ensure adjacents are within bounds
                if (adj_x >= 0 && adj_x < 21 && adj_y >= 0 && adj_y < 21) {
                    if (map[adj_y][x] == 1 && map[y][adj_x] == 1) return 0;
                    if (map[adj_y][x] == 2 && map[y][adj_x] == 2) return 0;
                }
                // Also check the previous step's adjacent cells
                int prev_x = x - sx;
                int prev_y = y - sy;
                if (prev_x >= 0 && prev_x < 21 && prev_y >= 0 && prev_y < 21) {
                    if (map[prev_y][x] == 1 && map[y][prev_x] == 1) return 0;
                    if (map[prev_y][x] == 2 && map[y][prev_x] == 2) return 0;
                }
            }
            if (map[y][x] == 1) return 0;
        }
        if (x == x2 && y == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
    return 1;
}

void line_of_sight(int map[21][21], int visibility[21][21], int origin_x, int origin_y) {
    memset(visibility, 0, sizeof(int) * 21 * 21);
    int cx = origin_x, cy = origin_y;
    int direct_los[21][21] = {0};
    for (int y = 0; y < 21; y++) {
        for (int x = 0; x < 21; x++) {
            int dx = x - cx;
            int dy = y - cy;
            int dist = abs(dx) + abs(dy);
            if (dist > 10) continue;
            if (is_line_of_sight(map, cx, cy, x, y) && map[y][x] == 0) {
                visibility[y][x] = 1;
                direct_los[y][x] = 1;
            }
        }
    }
    // Reveal walls adjacent to revealed floors
    for (int y = 0; y < 21; y++) {
        for (int x = 0; x < 21; x++) {
            if (visibility[y][x] == 1 && map[y][x] == 0) {
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (dx == 0 && dy == 0) continue;
                        int nx = x + dx, ny = y + dy;
                        if (nx >= 0 && nx < 21 && ny >= 0 && ny < 21 && map[ny][nx] == 1) {
                            visibility[ny][nx] = 1;
                        }
                    }
                }
            }
        }
    }
    // Reveal floors adjacent to direct LOS floors
    for (int y = 0; y < 21; y++) {
        for (int x = 0; x < 21; x++) {
            if (direct_los[y][x] == 1) {
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (dx == 0 && dy == 0) continue;
                        int nx = x + dx, ny = y + dy;
                        if (nx >= 0 && nx < 21 && ny >= 0 && ny < 21 && (map[ny][nx] == 0 || map[ny][nx] == 2 || map[ny][nx] == 3)) {
                            visibility[ny][nx] = 1;
                        }
                    }
                }
            }
        }
    }
    // Ensure the origin (player's position) is always visible
    visibility[cy][cx] = 1;
}