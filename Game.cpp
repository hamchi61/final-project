#include "Game.h"
#include "Utils.h"
#include "data/DataCenter.h"
#include "data/OperationCenter.h"
#include "data/SoundCenter.h"
#include "data/ImageCenter.h"
#include "data/FontCenter.h"
#include "Player.h"
#include "Level.h"
//revise start
#include "Hero.h"
//revise end
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_acodec.h>
#include <vector>
#include <cstring>

// fixed settings
constexpr char game_icon_img_path[] = "./assets/image/game_icon.png";
constexpr char game_start_sound_path[] = "./assets/sound/growl.wav";
constexpr char background_img_path[] = "./assets/image/StartBackground.jpg";
constexpr char background_sound_path[] = "./assets/sound/BackgroundMusic.ogg";
constexpr char menu_img_path[] = "./assets/image/menu.png";
constexpr char end_img_path[] = "./assets/image/zombiewon.png";
constexpr char about_img_path[] = "./assets/image/about.png";
/**
 * @brief Game entry.
 * @details The function processes all allegro events and update the event state to a generic data storage (i.e. DataCenter).
 * For timer event, the game_update and game_draw function will be called if and only if the current is timer.
 */
void
Game::execute() {
	DataCenter *DC = DataCenter::get_instance();
	// main game loop
	bool run = true;
	while(run) {
		// process all events here
		al_wait_for_event(event_queue, &event);
		switch(event.type) {
			case ALLEGRO_EVENT_TIMER: {
				run &= game_update();
				game_draw();
				break;
			} case ALLEGRO_EVENT_DISPLAY_CLOSE: { // stop game
				run = false;
				break;
			} case ALLEGRO_EVENT_KEY_DOWN: {
				DC->key_state[event.keyboard.keycode] = true;
				break;
			} case ALLEGRO_EVENT_KEY_UP: {
				DC->key_state[event.keyboard.keycode] = false;
				break;
			} case ALLEGRO_EVENT_MOUSE_AXES: {
				DC->mouse.x = event.mouse.x;
				DC->mouse.y = event.mouse.y;
				break;
			} case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN: {
				DC->mouse_state[event.mouse.button] = true;
				break;
			} case ALLEGRO_EVENT_MOUSE_BUTTON_UP: {
				DC->mouse_state[event.mouse.button] = false;
				break;
			} default: break;
		}
	}
}

/**
 * @brief Initialize all allegro addons and the game body.
 * @details Only one timer is created since a game and all its data should be processed synchronously.
 */
Game::Game() {
	DataCenter *DC = DataCenter::get_instance();
	GAME_ASSERT(al_init(), "failed to initialize allegro.");

	// initialize allegro addons
	bool addon_init = true;
	addon_init &= al_init_primitives_addon();
	addon_init &= al_init_font_addon();
	addon_init &= al_init_ttf_addon();
	addon_init &= al_init_image_addon();
	addon_init &= al_init_acodec_addon();
	GAME_ASSERT(addon_init, "failed to initialize allegro addons.");

	// initialize events
	bool event_init = true;
	event_init &= al_install_keyboard();
	event_init &= al_install_mouse();
	event_init &= al_install_audio();
	GAME_ASSERT(event_init, "failed to initialize allegro events.");

	// initialize game body
	GAME_ASSERT(
		display = al_create_display(DC->window_width, DC->window_height),
		"failed to create display.");
	GAME_ASSERT(
		timer = al_create_timer(1.0 / DC->FPS),
		"failed to create timer.");
	GAME_ASSERT(
		event_queue = al_create_event_queue(),
		"failed to create event queue.");

	debug_log("Game initialized.\n");
	game_init();
}

/**
 * @brief Initialize all auxiliary resources.
 */
void
Game::game_init() {
	DataCenter *DC = DataCenter::get_instance();
	SoundCenter *SC = SoundCenter::get_instance();
	ImageCenter *IC = ImageCenter::get_instance();
	FontCenter *FC = FontCenter::get_instance();
	// set window icon
	game_icon = IC->get(game_icon_img_path);
	al_set_display_icon(display, game_icon);

	// register events to event_queue
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_register_event_source(event_queue, al_get_timer_event_source(timer));

	// init sound setting
	SC->init();

	// init font setting
	FC->init();
	
	startpage = IC->get(menu_img_path);
	debug_log("Game state: change to MENU\n");
	state = STATE::MENU;
	al_start_timer(timer);
	// game start
	background = IC->get(background_img_path);
	endword = IC->get(end_img_path);
	about = IC->get(about_img_path);
	/*
	debug_log("Game state: change to START\n");
	state = STATE::START;
	al_start_timer(timer);
	*/
}

/**
 * @brief The function processes all data update.
 * @details The behavior of the whole game body is determined by its state.
 * @return Whether the game should keep running (true) or reaches the termination criteria (false).
 * @see Game::STATE
 */
bool
Game::game_update() {
	DataCenter *DC = DataCenter::get_instance();
	OperationCenter *OC = OperationCenter::get_instance();
	SoundCenter *SC = SoundCenter::get_instance();
	FontCenter *FC = FontCenter::get_instance();
	static ALLEGRO_SAMPLE_INSTANCE *background = nullptr;

	switch(state) {
		case STATE::MENU: {
			float btn_x1 = DC->window_width / 2.0 - 100;
			float btn_x2 = DC->window_width / 2.0 + 100;
			float btn_y1 = DC->window_height / 2.0 - 50;
			float btn_y2 = DC->window_height / 2.0 + 50;
			if (DC->mouse_state[1] && !DC->prev_mouse_state[1] &&
				DC->mouse.x >= 750 && DC->mouse.x <= 1000 &&
				DC->mouse.y >= 100 && DC->mouse.y <= 250) {
				debug_log("<Game> state: change to START\n");
				state = STATE::START;
			}
			else if(DC->mouse_state[1] && !DC->prev_mouse_state[1] &&
				DC->mouse.x >= 750 && DC->mouse.x <= 1000 &&
				DC->mouse.y >= 300 && DC->mouse.y <= 500){
					debug_log("<Game> state: change to END\n");
					state = STATE::END;
			}
			else if(DC->mouse_state[1] && !DC->prev_mouse_state[1] &&
			DC->mouse.x >= 100 && DC->mouse.x <= 220 &&
			DC->mouse.y >= 40 && DC->mouse.y <= 85){
				debug_log("<Game> state: change to about\n");
				state = STATE::ABOUT;
			}
			break;
		}
		case STATE::ABOUT:{
			if(DC->mouse_state[1] && !DC->prev_mouse_state[1]){
				state = STATE::MENU;
			}
			break;
		}
		case STATE::START: {
			static bool is_played = false;
			static ALLEGRO_SAMPLE_INSTANCE *instance = nullptr;
			if(!is_played) {
				instance = SC->play(game_start_sound_path, ALLEGRO_PLAYMODE_ONCE);
				delete ui; 
				ui = new UI();
				ui->init();
				DC->reset();
				for(int i = 0; i<5; i++)
					DC->heros[i]->init(i*100+100);
				debug_log("DataCenter has been reset.\n");
				DC->level->load_level(1);
				
				end = false;
				end_screen_timer = 0;
				debug_log("<Game> All resources have been reset.\n");
				is_played = true;
			}
			if(!SC->is_playing(instance)) {
				debug_log("<Game> state: change to LEVEL\n");
				state = STATE::LEVEL;
				is_played = false;
			}
			break;
		} case STATE::LEVEL: {
			static bool BGM_played = false;
			if(!BGM_played) {
				background = SC->play(background_sound_path, ALLEGRO_PLAYMODE_LOOP);
				BGM_played = true;
			}

			if(DC->key_state[ALLEGRO_KEY_P] && !DC->prev_key_state[ALLEGRO_KEY_P]) {
				SC->toggle_playing(background);
				debug_log("<Game> state: change to PAUSE\n");
				state = STATE::PAUSE;
			}
			if(DC->level->remain_monsters() == 0 && DC->monsters.size() == 0) {
				debug_log("<Game> state: change to END Monster\n");
				state = STATE::MENU;
				
			}
			if(DC->player->HP == 0 && end == false) {
				end = true;
				debug_log("<Game> state: change to END HP\n");
			}
			if(end){
				end_screen_timer++;
				if (end_screen_timer >= DC->FPS * 8) {  
					debug_log("<Game> state: change to MENU\n");
					state = STATE::MENU;
				}
			}
			break;
		} case STATE::PAUSE: {
			if(DC->key_state[ALLEGRO_KEY_P] && !DC->prev_key_state[ALLEGRO_KEY_P]) {
				SC->toggle_playing(background);
				debug_log("<Game> state: change to LEVEL\n");
				state = STATE::LEVEL;
			}
			break;
		} case STATE::END: {
			return false;
		}
	}
	// If the game is not paused, we should progress update.
	if(state == STATE::LEVEL) {
		//debug_log("<updating\n");
		DC->player->update();
		SC->update();
		ui->update();
		//revise start
		for(int i = 0; i<5; i++){
			DC->heros[i]->update();
		}
		//revise end
		if(state != STATE::START && state != STATE::MENU) {
			DC->level->update();
			OC->update();
		}
	}
	// game_update is finished. The states of current frame will be previous states of the next frame.
	memcpy(DC->prev_key_state, DC->key_state, sizeof(DC->key_state));
	memcpy(DC->prev_mouse_state, DC->mouse_state, sizeof(DC->mouse_state));
	return true;
}

/**
 * @brief Draw the whole game and objects.
 */
void
Game::game_draw() {
	DataCenter *DC = DataCenter::get_instance();
	OperationCenter *OC = OperationCenter::get_instance();
	FontCenter *FC = FontCenter::get_instance();

	// Flush the screen first.
	al_clear_to_color(al_map_rgb(100, 100, 100));
	if(state != STATE::END) {
		// background
		if(state == STATE:: MENU){
			al_draw_bitmap(startpage, 0, 0, 0);
		}
		else if(state == STATE:: ABOUT){
			//al_draw_filled_rectangle(0, 0, DC->window_width, DC->window_height, al_map_rgba(255, 255, 255, 64));
			al_draw_bitmap(about, 0, 0, 0);
		}
		else {
			al_draw_bitmap(background, 0, 0, 0);
		}
		
		/* revise
		if(DC->game_field_length < DC->window_width)
			al_draw_filled_rectangle(
				DC->game_field_length, 0,
				DC->window_width, DC->window_height,
				al_map_rgb(100, 100, 100));
		if(DC->game_field_length < DC->window_height)
			al_draw_filled_rectangle(
				0, DC->game_field_length,
				DC->window_width, DC->window_height,
				al_map_rgb(100, 100, 100));
		*/
		// user interface
		if(state != STATE::START && state != STATE::MENU && state!= STATE::START && state != STATE::ABOUT) {
			//DC->level->draw();
			//revise start
			for(int i = 0; i<5; i++){
				DC->heros[i]->draw();
			}
			//revise end
			ui->draw();
			OC->draw();
		}

	}
	switch(state) {
		case STATE::MENU: {
			al_draw_filled_rectangle(100, 40, 220, 85, al_map_rgba(255, 255, 255, 64));
			al_draw_text(
				FC->caviar_dreams[FontSize::SMALL], al_map_rgb(0, 0, 0),
				170, 60,
				ALLEGRO_ALIGN_CENTRE, "ABOUT");
			break;
		}
		case STATE::START: {
		} case STATE::LEVEL: {
			if(end){
			al_draw_filled_rectangle(0, 0, DC->window_width, DC->window_height, al_map_rgba(50, 50, 50, 64));
			al_draw_bitmap(endword, 400, 100, 0);
		}
			break;
		} case STATE::PAUSE: {
			// game layout cover
			al_draw_filled_rectangle(0, 0, DC->window_width, DC->window_height, al_map_rgba(50, 50, 50, 64));
			al_draw_text(
				FC->caviar_dreams[FontSize::LARGE], al_map_rgb(255, 255, 255),
				DC->window_width/2., DC->window_height/2.,
				ALLEGRO_ALIGN_CENTRE, "GAME PAUSED");
			break;
		} case STATE::END: {
		}
	}
	al_flip_display();
}

Game::~Game() {
	DataCenter *DC = DataCenter::get_instance();
    delete ui;
    for (int i = 0; i < 5; i++) {
        delete DC->heros[i];
    }

    al_destroy_display(display);
    al_destroy_timer(timer);
    al_destroy_event_queue(event_queue);
}
