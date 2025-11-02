/*
    Catacombs Map Generator
    Generates a random catacomb map and saves it to a file.

    
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int generate_catacomb_map(int width, int height);
void save_map_to_file(const char *filename, int width, int height);
int random_number_range(int min, int max);
int random_bool();

int** map; // dynamic 2D array for the catacomb map

// Add new function to connect disconnected components using BFS
void connect_components(int width, int height) {
    int visited[height][width];
    memset(visited, 0, sizeof(visited));
    
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            if (map[y][x] == '0' && !visited[y][x]) {
                // Start BFS from this floor tile
                int queue[height * width][2];
                int front = 0, rear = 0;
                queue[rear][0] = y;
                queue[rear][1] = x;
                rear++;
                visited[y][x] = 1;
                
                while (front < rear) {
                    int cy = queue[front][0];
                    int cx = queue[front][1];
                    front++;
                    
                    // Check orthogonal neighbors
                    int dirs[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
                    for (int d = 0; d < 4; d++) {
                        int ny = cy + dirs[d][0];
                        int nx = cx + dirs[d][1];
                        if (ny >= 0 && ny < height && nx >= 0 && nx < width && map[ny][nx] == '0' && !visited[ny][nx]) {
                            visited[ny][nx] = 1;
                            queue[rear][0] = ny;
                            queue[rear][1] = nx;
                            rear++;
                        }
                    }
                }
                
                // If this is not the first component, connect it to the previous one
                // For simplicity, connect to the leftmost floor tile in the first component
                if (rear > 1) { // More than one tile, but to connect components, we need to find unvisited floors
                    // Actually, this BFS marks one component; we need to find another unvisited floor and carve a path
                    // Revised: After marking one component, find the next unvisited floor and carve a path
                    for (int yy = 1; yy < height - 1; yy++) {
                        for (int xx = 1; xx < width - 1; xx++) {
                            if (map[yy][xx] == '0' && !visited[yy][xx]) {
                                // Carve a simple path from (y,x) to (yy,xx) - horizontal then vertical
                                int start_x = (x < xx) ? x : xx;
                                int end_x = (x > xx) ? x : xx;
                                int start_y = (y < yy) ? y : yy;
                                int end_y = (y > yy) ? y : yy;
                                for (int px = start_x; px <= end_x; px++) map[y][px] = '0';
                                for (int py = start_y; py <= end_y; py++) map[py][xx] = '0';
                                // Mark the new component as visited (simplified)
                                visited[yy][xx] = 1;
                                goto next_component; // Break out
                            }
                        }
                    }
                    next_component:;
                }
            }
        }
    }
}

// map generation
int generate_catacomb_map(int width, int height) {
    printf("Generating catacomb map of size %dx%d\n", width, height);
    // allocate memory for the map
    map = (int **)malloc(height * sizeof(int *));
    for (int i = 0; i < height; i++) {
        map[i] = (int *)malloc(width * sizeof(int));
    }

    // check if memory allocation was successful
    if (map == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1; // error
    }

    // First, fill the map with walls
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            map[y][x] = '1'; // wall
        }
    }

    // Carve out random rooms and corridors
    int min_rooms = (width * height) / (width + height); // minimum of half the map width/height in rooms
    int num_rooms = random_number_range(min_rooms, min_rooms + 5);
    // Carve out rooms
    for (int r = 0; r < num_rooms; r++) {
        int room_width = random_number_range(3, 9);
        int room_height = random_number_range(3, 9);
        int room_x = random_number_range(1, width - room_width - 1);
        int room_y = random_number_range(1, height - room_height - 1);

        // before placing the room, check if it overlaps with existing rooms
        int overlap = 0;
        for (int y = room_y - 1; y < room_y + room_height + 1; y++) {
            for (int x = room_x - 1; x < room_x + room_width + 1; x++) {
                if (map[y][x] == '0') { // already carved out
                    overlap = 1;
                    break;
                }
            }
            if (overlap) break;
        }

        if (overlap) continue;

        // Carve out the room
        for (int y = room_y; y < room_y + room_height; y++) {
            for (int x = room_x; x < room_x + room_width; x++) {
                map[y][x] = '0'; // floor
            }
        }
    }

    // Place rectangular or square walls in huge rooms (>14x14)
    // TODO: Implement wall placement for huge rooms

    // Connect rooms with corridors
    // record corridor start and end points
    for (int r = 0; r < num_rooms - 1; r++) {
        int x1 = random_number_range(1, width - 2);
        int y1 = random_number_range(1, height - 2);
        int x2 = random_number_range(1, width - 2);
        int y2 = random_number_range(1, height - 2);

        // Carve out a simple straight corridor
        if (random_bool()) {
            for (int x = (x1 < x2 ? x1 : x2); x <= (x1 > x2 ? x1 : x2); x++) {
                map[y1][x] = '0'; // floor
            }
            for (int y = (y1 < y2 ? y1 : y2); y <= (y1 > y2 ? y1 : y2); y++) {
                map[y][x2] = '0'; // floor
            }
        } else {
            for (int y = (y1 < y2 ? y1 : y2); y <= (y1 > y2 ? y1 : y2); y++) {
                map[y][x1] = '0'; // floor
            }
            for (int x = (x1 < x2 ? x1 : x2); x <= (x1 > x2 ? x1 : x2); x++) {
                map[y2][x] = '0'; // floor
            }
        }
        // generate small rooms at corridor ends
        for (int i = 0; i < 2; i++) {
            int room_width = random_number_range(3, 5);
            int room_height = random_number_range(3, 5);
            int room_x = (i == 0) ? x1 - room_width / 2 : x2 - room_width / 2;
            int room_y = (i == 0) ? y1 - room_height / 2 : y2 - room_height / 2;

            // Carve out the small room
            for (int y = room_y; y < room_y + room_height; y++) {
                for (int x = room_x; x < room_x + room_width; x++) {
                    if (x > 0 && x < width && y > 0 && y < height) {
                        map[y][x] = '0'; // floor
                    }
                }
            }
        }
    }

    // scan the map to find rooms not connected to corridors and connect them
    // do not overwrite if the position is already a floor, hiding spot, or treasure
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            if (map[y][x] == '0') {
                // check if adjacent to a corridor
                int adjacent_to_corridor = 0;
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (abs(dx) + abs(dy) == 1) { // only orthogonal neighbors
                            if (map[y + dy][x + dx] == '0') {
                                adjacent_to_corridor = 1;
                            }
                        }
                    }
                }
                if (!adjacent_to_corridor) {
                    // carve a corridor to the nearest corridor
                    int target_x = x, target_y = y;
                    while (map[target_y][target_x] != '0') {
                        if (target_x > 1) target_x--;
                        map[target_y][target_x] = '0';
                    }
                }
            }
        }
    }

    // Boarder the map with walls
    for (int x = 0; x < width; x++) {
        map[0][x] = '1'; // top border
        map[height - 1][x] = '1'; // bottom border
    }
    for (int y = 0; y < height; y++) {
        map[y][0] = '1'; // left border
        map[y][width - 1] = '1'; // right border
    }

    // Place some random hiding spots in wall tiles of corridors with exactly one adjacent floor tile and 20% chance
    int num_hiding_spots = (width * height) / 2; // arbitrary
    for (int h = 0; h < num_hiding_spots; h++) {
        // iterate through every position on the map, do not pick randomly to ensure coverage
        // repeat until a valid hiding spot is found
        int hx, hy;
        do {
            hx = random_number_range(1, width - 2);
            hy = random_number_range(1, height - 2);
        } while (map[hy][hx] != '1');
        if (map[hy][hx] == '1') { // check for wall tiles
            int adjacent_floors = 0;
            int adjacent_hiding_spots = 0;
            // Check orthogonal neighbors only
            if (hy > 0 && map[hy - 1][hx] == '0') adjacent_floors++;
            if (hy < height - 1 && map[hy + 1][hx] == '0') adjacent_floors++;
            if (hx > 0 && map[hy][hx - 1] == '0') adjacent_floors++;
            if (hx < width - 1 && map[hy][hx + 1] == '0') adjacent_floors++;
            // check for adjacent hiding spots as well
            if (hy > 0 && map[hy - 1][hx] == '2') adjacent_hiding_spots++;
            if (hy < height - 1 && map[hy + 1][hx] == '2') adjacent_hiding_spots++;
            if (hx > 0 && map[hy][hx - 1] == '2') adjacent_hiding_spots++;
            if (hx < width - 1 && map[hy][hx + 1] == '2') adjacent_hiding_spots++;
            // place hiding spot if conditions met
            if (adjacent_floors == 1 && adjacent_hiding_spots == 0 && (rand() % 100) < 20) { // 20% chance
                map[hy][hx] = '2'; // hiding spot
            }
        }
    }
    
    
    // Place small wall squares in large rooms (12x12 entirely floors)
    for (int y = 0; y <= height - 12; y++) {
        for (int x = 0; x <= width - 12; x++) {
            int all_floors = 1;
            for (int dy = 0; dy < 12; dy++) {
                for (int dx = 0; dx < 12; dx++) {
                    if (map[y + dy][x + dx] != '0') {
                        all_floors = 0;
                        break;
                    }
                }
                if (!all_floors) break;
            }
            if (all_floors) {
                // Place a small wall square
                int wall_size = random_number_range(3, 9);
                int wall_x = x + random_number_range(0, 18 - wall_size);
                int wall_y = y + random_number_range(0, 18 - wall_size);
                for (int wy = wall_y; wy < wall_y + wall_size; wy++) {
                    for (int wx = wall_x; wx < wall_x + wall_size; wx++) {
                        map[wy][wx] = '1'; // wall
                    }
                }
            }
        }
    }
    
    // After placing rooms and initial corridors, call the new connection function
    connect_components(width, height);

    // Place some random treasures in rooms, avoid placing in corridors by checking for at least 5 surrounding floors in 3x3
    int num_treasures = 3; // fixed number
    for (int t = 0; t < num_treasures; t++) {
        // repeat until a valid spot is found
        int tx, ty;
        do {
            tx = random_number_range(1, width - 2);
            ty = random_number_range(1, height - 2);
        } while (map[ty][tx] != '0' || ({
            int floor_count = 0;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    if (map[ty + dy][tx + dx] == '0') {
                        floor_count++;
                    }
                }
            }
            floor_count < 5;
        }));
        map[ty][tx] = '3'; // treasure chest
    }
    
    return 0; // success
}

// main loop 
int main() {
    srand(time(NULL)); // seed random number generator

    int width = 20;
    int height = 10;

    // ask user for map dimensions & name
    char filename[256];
    printf("Enter map width: ");
    scanf("%d", &width);
    printf("Enter map height: ");
    scanf("%d", &height);
    printf("Enter filename to save the map: ");
    scanf("%s", filename);
    // if empty filename, use default
    if (strlen(filename) == 0) {
        strcpy(filename, "default");
    }

    // generate the catacomb map
    if (generate_catacomb_map(width, height) != 0) {
        return 1; // error
    }

    // print the generated map
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            printf("%c ", map[y][x]);
        }
        printf("\n");
    }

    // save the map to a file
    save_map_to_file(filename, width, height);

    // free allocated memory
    for (int i = 0; i < height; i++) {
        free(map[i]);
    }
    free(map);

    return 0; // success
}

// save map to file
void save_map_to_file(const char *filename, int width, int height) {
    // save with .catamap extension
    // add extension if not present
    char full_filename[300];
    snprintf(full_filename, sizeof(full_filename), "%s.catamap", filename);
    printf("Saving map to %s\n", full_filename);
    FILE *file = fopen(full_filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file for writing\n");
        return;
    }

    // write dimensions as header
    fprintf(file, "%d %d\n", width, height);
    // print to file
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            fprintf(file, "%c ", map[y][x]);
        }
        fprintf(file, "\n");
    }

    // print out ratio of walls to floors
    int wall_count = 0;
    int floor_count = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (map[y][x] == '1') wall_count++;
            if (map[y][x] == '0') floor_count++;
        }
    }
    printf("Wall to floor ratio: %d to %d\n", wall_count, floor_count);
    // print number of hiding spots and treasures
    int hiding_spot_count = 0;
    int treasure_count = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (map[y][x] == '2') hiding_spot_count++;
            if (map[y][x] == '3') treasure_count++;
        }
    }
    printf("Hiding spots: %d\n", hiding_spot_count);
    printf("Treasures: %d\n", treasure_count);

    fclose(file);
}

int random_number_range(int min, int max) {
    return rand() % (max - min + 1) + min;
}
int random_bool() {
    return rand() % 2;
}