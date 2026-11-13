#include <SDL.h>            

#include "simple_json.h"
#include "simple_logger.h"

#include "gfc_input.h"
#include "gfc_config_def.h"
#include "gfc_vector.h"
#include "gfc_matrix.h"
#include "gfc_audio.h"
#include "gfc_string.h"
#include "gfc_actions.h"

#include "gf2d_sprite.h"
#include "gf2d_font.h"
#include "gf2d_actor.h"
#include "gf2d_mouse.h"

#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"
#include "gf3d_swapchain.h"

#include "gf3d_mesh.h"
#include "model.h"
#include "entity.h"
#include "item.h"
#include "player.h"
#include "level.h"
#include "time.h"
#include "mouse.h"

extern int __DEBUG;

static int _done = 0;
static Uint32 frame_delay = 16;
static float fps = 0;

typedef enum
{
    GAME_PAUSED = 0,
    GAME_PLAYING = 1,
}GameState;

void parse_arguments(int argc,char *argv[]);
void game_frame_delay();
/**
 * @brief pauses the game state
 * @return a paused GameState
 * 
 * WARNING: only call during game init
 */
GameState game_pause();
/**
 * @brief plays the game state
 * @return a paused GameState
 * 
 * WARNING: only call after game init
 */
GameState game_play();


void exitGame()
{
    _done = 1;
}


int main(int argc,char *argv[])
{
    //local variables
    Level* level;
    //Sprite *bg;
    GameState game_state;

    //initializtion    
    parse_arguments(argc,argv);
    //name of log file, should it be erased (0 overwrite, 1 append)
    init_logger("gf3d.log",0);
    slog("gf3d begin");
    //gfc init
    gfc_input_init("config/input.cfg");
    gfc_config_def_init();
    gfc_action_init(1024);
    //gf3d init
    gf3d_vgraphics_init("config/setup.cfg");
    //gfc_sound_init_config("config/audio.cfg");
    gf2d_font_init("config/font.cfg");
    gf2d_actor_init(1000);
    gfc_config_def_load("config/mouse.cfg");
    gfc_config_def_load("defs/items.def");
    gfc_config_def_load("defs/entities.def");
    entity_system_init(1000);
    mouse_system_init();
    time_system_init();

    //game init
    srand(SDL_GetTicks());
    slog_sync();
    //bg = gf2d_sprite_load_image("images/bg_flat.png");
    gf2d_mouse_load("actors/mouse.actor");
    level = level_load_from_file("levels/test_level.level");
    player_new(gfc_vector3d(0, 0, 40));
    spawn_item_from_def("Steve", gfc_vector3d(195, 195, 1));
    entity_spawn_from_def("Cube1", gfc_vector3d(180, -180, 0));
    entity_spawn_from_def("Cube2", gfc_vector3d(0, -190, 0));
    game_state = game_play();
    // main game loop    
    while(!_done)
    {
        time_frame_update();
        gfc_input_update();
        if (gfc_input_command_pressed("cancel"))
        {
            if (game_state == GAME_PLAYING)
            {
                game_state = game_pause();
            }
            else
            {
                game_state = game_play();
            }
        }

        gf2d_mouse_update();
        gf2d_font_update();
        //camera updates

        if (game_state == GAME_PLAYING)
        {
            entity_system_think_all();
            entity_system_update_all();
        }

        gf3d_vgraphics_render_start();
                //3D draws
                level_queue_render(level);
                entity_system_queue_render_all();
                //2D draws
                //get rid of the background image
                //gf2d_sprite_draw_image(bg,gfc_vector2d(0,0));
                
                if (game_state == GAME_PAUSED)
                {
                    gf2d_font_draw_line_tag("Game is paused", FT_H1, GFC_COLOR_RED, gfc_vector2d(50, 50));
                }
                gf2d_font_draw_line_tag("ALT+F4 to exit",FT_H1,GFC_COLOR_WHITE, gfc_vector2d(10,10));
                gf2d_mouse_draw();
        gf3d_vgraphics_render_end();
        if (gfc_input_command_down("exit"))_done = 1; // exit condition
        game_frame_delay();
    }    
    vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());    
    //cleanup
    slog("gf3d program end");
    exit(0);
    slog_sync();
    return 0;
}

//add new command line arguments here
void parse_arguments(int argc,char *argv[])
{
    int a;

    for (a = 1; a < argc;a++)
    {
        //shows additional debug stuff, may want to do this for a bit
        if (strcmp(argv[a],"--debug") == 0)
        {
            __DEBUG = 1;
        }
    }    
}

void game_frame_delay()
{
    Uint32 diff;
    static Uint32 now;
    static Uint32 then;
    then = now;
    slog_sync();// make sure logs get written when we have time to write it
    now = SDL_GetTicks();
    diff = (now - then);
    if (diff < frame_delay)
    {
        SDL_Delay(frame_delay - diff);
    }
    fps = 1000.0/MAX(SDL_GetTicks() - then,0.001);
//     slog("fps: %f",fps);
}

GameState game_pause()
{
    int width, height;
    SDL_Window* win = gf3d_vgraphics_get_window();
    SDL_SetWindowGrab(win, SDL_FALSE);
    SDL_SetRelativeMouseMode(SDL_FALSE);
    SDL_GetWindowSize(win, &width, &height);
    SDL_WarpMouseInWindow(win, width / 2, height / 2);
    gf2d_mouse_update();
    gf2d_mouse_show();
    return GAME_PAUSED;
}

GameState game_play()
{
    SDL_SetWindowGrab(gf3d_vgraphics_get_window(), SDL_TRUE);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    gf2d_mouse_hide();
    SDL_GetRelativeMouseState(NULL, NULL);
    return GAME_PLAYING;
}

/*eol@eof*/
