#include "Level.h"
#include <string>
#include "Utils.h"
#include "monsters/Monster.h"
#include "data/DataCenter.h"
#include <allegro5/allegro_primitives.h>
#include "shapes/Point.h"
#include "shapes/Rectangle.h"
#include <array>

using namespace std;

// fixed settings
namespace LevelSetting {
	constexpr char level_path_format
	[] = "./assets/level/LEVEL%d.txt";
	//! @brief Grid size for each level.
	constexpr array<int, 4> grid_size = {
		100, 100, 100, 100
	};
	constexpr int monster_spawn_rate = 800;
};

void
Level::init() {
	level = -1;
	grid_w = -1;
	grid_h = -1;
	monster_spawn_counter = 0;
	srand(time(NULL));
}

/**
 * @brief Loads level data from input file. The input file is required to follow the format.
 * @param lvl level index. The path format is a fixed setting in code.
 * @details The content of the input file should be formatted as follows:
 *          * Total number of monsters.
 *          * Number of each different number of monsters. The order and number follows the definition of MonsterType.
 *          * Indefinite number of Point (x, y), represented in grid format.
 * @see level_path_format
 * @see MonsterType
 */
void
Level::load_level(int lvl) {
	DataCenter *DC = DataCenter::get_instance();

	char buffer[50];
	sprintf(buffer, LevelSetting::level_path_format, lvl);
	FILE *f = fopen(buffer, "r");
	GAME_ASSERT(f != nullptr, "cannot find level.");
	level = lvl;
	grid_w = DC->game_field_length / LevelSetting::grid_size[lvl];
	//grid_h = DC->game_field_length / LevelSetting::grid_size[lvl];
	grid_h = 6;
	num_of_monsters.clear();
	road_path.clear();

	int num;
	// read total number of monsters & number of each monsters
	fscanf(f, "%d", &num);
	for(size_t i = 0; i < static_cast<size_t>(MonsterType::MONSTERTYPE_MAX); ++i) {
		fscanf(f, "%d", &num);
		num_of_monsters.emplace_back(num);
	}

	// read road path
	while(fscanf(f, "%d", &num) != EOF) {
		int w = num % grid_w;
		int h = num / grid_h;
		road_path.emplace_back(w, h);
	}
	debug_log("<Level> load level %d.\n", lvl);
}

/**
 * @brief Updates monster_spawn_counter and create monster if needed.
*/
void
Level::update() {
	if(monster_spawn_counter<0 || monster_spawn_counter>2000) {
		monster_spawn_counter = 180;
		return;
	}
	if(monster_spawn_counter) {
		monster_spawn_counter--;
		return;
	}
	//debug_log("<level> monster_spawn %d\n", monster_spawn_counter);
	DataCenter *DC = DataCenter::get_instance();
	/* revise
	for(size_t i = 0; i < num_of_monsters.size(); ++i) {
		if(num_of_monsters[i] == 0) continue;
		DC->monsters.emplace_back(Monster::create_monster(static_cast<MonsterType>(i), DC->level->get_road_path()));
		num_of_monsters[i]--;
		break;
	}*/
	for (size_t i = 0; i < num_of_monsters.size(); ++i) {
        if (num_of_monsters[i] == 0) continue;
		//debug_log("<level> monster generate\n");
        // 生成從右到左的直線路徑
        vector<Point> monster_path = generate_right_to_left_path();

        // 創建怪物並分配路徑
        DC->monsters.emplace_back(Monster::create_monster(static_cast<MonsterType>(i), monster_path));
        num_of_monsters[i]--;
        break;
    }
	//revise end
	monster_spawn_counter = LevelSetting::monster_spawn_rate;
}

void
Level::draw() {
	if(level == -1) return;
	for(auto &[i, j] : road_path) {
		int x1 = i * LevelSetting::grid_size[level];
		int y1 = j * LevelSetting::grid_size[level]+25;
		int x2 = x1 + LevelSetting::grid_size[level];
		int y2 = y1 + LevelSetting::grid_size[level];
		al_draw_filled_rectangle(x1, y1, x2, y2, al_map_rgb(255, 244, 173));
	}
}

bool
Level::is_onroad(const Rectangle &region) {
	for(const Point &grid : road_path) {
		if(grid_to_region(grid).overlap(region))
			return true;
	}
	return false;
}

Rectangle
Level::grid_to_region(const Point &grid) const {
	int x1 = grid.x * LevelSetting::grid_size[level];
	int y1 = grid.y * LevelSetting::grid_size[level]+25;
	int x2 = x1 + LevelSetting::grid_size[level];
	int y2 = y1 + LevelSetting::grid_size[level];
	return Rectangle{x1, y1, x2, y2};
}

vector<Point> Level::generate_right_to_left_path() const {
    vector<Point> path;
	//int h[5] = {200,300,400,500,600};
    // 隨機選擇起點的高度 (Y 軸位置)
    int start_y = rand() % 5;
	//debug_log("yplace %d\n", start_y);
    // 固定 X 軸從最右開始，逐步向左
    for (int x = grid_w - 1; x >= 0; --x) {
        path.emplace_back(x, start_y);
    }

    return path;
}

