#include <SDL2/SDL.h>
#include <stdio.h>
#include "gui_renderer.h"
#include "microui.h"
#include "gui.h"
#include "sdl_input.h"
#include "config.h"


static  char logbuf[64000];
static   int logbuf_updated = 0;
static float bg[3] = { 90, 95, 100 };

static int window_open = 0;
static mu_Context *ctx;
static char not_available[] = "N/A";

void write_log(char *text) {
    if (logbuf[0]) { strcat(logbuf, "\n"); }
    strcat(logbuf, text);
    logbuf_updated = 1;
}

static int uint8_slider(mu_Context *ctx, unsigned char *value, int low, int high) {
    static float tmp;
    mu_push_id(ctx, &value, sizeof(value));
    tmp = *value;
    int res = mu_slider_ex(ctx, &tmp, low, high, 0, "%.0f", MU_OPT_ALIGNCENTER);
    *value = tmp;
    mu_pop_id(ctx);
    return res;
}

static int uint_slider(mu_Context *ctx, unsigned int *value, int low, int high) {
    static float tmp;
    mu_push_id(ctx, &value, sizeof(value));
    tmp = *value;
    int res = mu_slider_ex(ctx, &tmp, low, high, 0, "%.0f", MU_OPT_ALIGNCENTER);
    *value = tmp;
    mu_pop_id(ctx);
    return res;
}

static void log_panel(mu_Context *ctx) {
    if (mu_header(ctx, "Log")) {
        /* output text panel */
        mu_layout_row(ctx, 1, (int[]) { -1 }, -25);
        mu_begin_panel(ctx, "Log Output");
        mu_Container *panel = mu_get_current_container(ctx);
        mu_layout_row(ctx, 1, (int[]) { -1 }, -1);
        mu_text(ctx, logbuf);
        mu_end_panel(ctx);
        if (logbuf_updated) {
            panel->scroll.y = panel->content_size.y;
            logbuf_updated = 0;
        }

        /* input textbox + submit button */
        static char buf[128];
        int submitted = 0;
        mu_layout_row(ctx, 2, (int[]) { -70, -1 }, 0);
        if (mu_textbox(ctx, buf, sizeof(buf)) & MU_RES_SUBMIT) {
            mu_set_focus(ctx, ctx->last_id);
            submitted = 1;
        }
        if (mu_button(ctx, "Submit")) { submitted = 1; }
        if (submitted) {
            write_log(buf);
            buf[0] = '\0';
        }
    }
}

static void coninfo_panel(mu_Context *ctx)
{
    if (mu_header_ex(ctx, "Controller info", MU_OPT_EXPANDED)) {
        mu_Container *win = mu_get_current_container(ctx);
        char buf[64];
        const int widths[] = {150, -1};
        mu_layout_row(ctx, 2, widths, 0);

        
        mu_label(ctx,"Name");
        if (con != NULL) {
            char *name = SDL_GameControllerName(con);
            if (*name != NULL) {
                mu_label(ctx, name);
            } 
        } else {
            mu_label(ctx, not_available);
        }

        mu_label(ctx,"GameController mapping");
        if (con != NULL) {
            char *mapping = SDL_GameControllerMapping(con);
            if (*mapping != NULL) {
                mu_label(ctx, mapping);
                SDL_free(mapping);
            } 
        } else {
            mu_label(ctx, not_available);
        }
    }
}

static void analog_panel(mu_Context *ctx)
{
    if (mu_header_ex(ctx, "Analog stick", MU_OPT_EXPANDED)) {
        mu_Container *win = mu_get_current_container(ctx);
        const int widths[] = {150, -1};
        mu_layout_row(ctx, 2, widths, 0);

        // deadzone slider
        mu_label(ctx, "Deadzone");
        float *dz = &concfg.deadzone;
        mu_slider(ctx, dz, 0.f, 1.f);

        // outer edge
        mu_label(ctx, "Outer edge");
        float *edge = &concfg.outer_edge;
        mu_slider(ctx, edge, 0.f, 1.f);

        // range
        mu_label(ctx, "Range");
        float *range = &concfg.range;
        uint_slider(ctx, range, 0, 127);

        // clamping checkbox
        mu_label(ctx, "Range clamping");
        int *clamp = &concfg.is_clamped;
        mu_checkbox(ctx, "", clamp);
    }
}

static void configfile_panel(mu_Context *ctx)
{
    if (mu_header_ex(ctx, "Configuration file", MU_OPT_EXPANDED)) {
        mu_Container *win = mu_get_current_container(ctx);
        const int widths[] = {150, -1};
        mu_layout_row(ctx, 2, widths, 0);

        const char curfile[] = "Current file";
        mu_label(ctx, curfile);
        mu_text(ctx, configpath);

        const int widths2[] = {150, 125, 125};
        mu_layout_row(ctx, 3, widths2, 0);
        mu_label(ctx, "");
        // save
        if (mu_button(ctx, "Save config")) {
            dlog("not implemented yet!");
        }
        // load
        if (mu_button(ctx, "Reload config")) {
            config_load();
        }
    }
}

static void test_window(mu_Context *ctx) {
    /* do window */
    int opt = MU_OPT_NOINTERACT | MU_OPT_NOTITLE;
    if (mu_begin_window_ex(ctx, "Demo Window", mu_rect(0, 0, 600, 600), opt)) {
        mu_Container *win = mu_get_current_container(ctx);

        coninfo_panel(ctx);
        analog_panel(ctx);
        configfile_panel(ctx);
        log_panel(ctx);

        mu_end_window(ctx);
    }
}

static void style_window(mu_Context *ctx) {
    static struct { const char *label; int idx; } colors[] = {
        { "text:",         MU_COLOR_TEXT        },
        { "border:",       MU_COLOR_BORDER      },
        { "windowbg:",     MU_COLOR_WINDOWBG    },
        { "titlebg:",      MU_COLOR_TITLEBG     },
        { "titletext:",    MU_COLOR_TITLETEXT   },
        { "panelbg:",      MU_COLOR_PANELBG     },
        { "button:",       MU_COLOR_BUTTON      },
        { "buttonhover:",  MU_COLOR_BUTTONHOVER },
        { "buttonfocus:",  MU_COLOR_BUTTONFOCUS },
        { "base:",         MU_COLOR_BASE        },
        { "basehover:",    MU_COLOR_BASEHOVER   },
        { "basefocus:",    MU_COLOR_BASEFOCUS   },
        { "scrollbase:",   MU_COLOR_SCROLLBASE  },
        { "scrollthumb:",  MU_COLOR_SCROLLTHUMB },
        { NULL }
    };

    if (mu_begin_window(ctx, "Style Editor", mu_rect(350, 250, 300, 240))) {
        int sw = mu_get_current_container(ctx)->body.w * 0.14;
        mu_layout_row(ctx, 6, (int[]) { 80, sw, sw, sw, sw, -1 }, 0);
        for (int i = 0; colors[i].label; i++) {
            mu_label(ctx, colors[i].label);
            uint8_slider(ctx, &ctx->style->colors[i].r, 0, 255);
            uint8_slider(ctx, &ctx->style->colors[i].g, 0, 255);
            uint8_slider(ctx, &ctx->style->colors[i].b, 0, 255);
            uint8_slider(ctx, &ctx->style->colors[i].a, 0, 255);
            mu_draw_rect(ctx, mu_layout_next(ctx), ctx->style->colors[i]);
        }
        mu_end_window(ctx);
    }
}


static void process_frame(mu_Context *ctx) {
    mu_begin(ctx);
    test_window(ctx);
    mu_end(ctx);
}



static const char button_map[256] = {
    [ SDL_BUTTON_LEFT   & 0xff ] =  MU_MOUSE_LEFT,
    [ SDL_BUTTON_RIGHT  & 0xff ] =  MU_MOUSE_RIGHT,
    [ SDL_BUTTON_MIDDLE & 0xff ] =  MU_MOUSE_MIDDLE,
};

static const char key_map[256] = {
    [ SDLK_LSHIFT       & 0xff ] = MU_KEY_SHIFT,
    [ SDLK_RSHIFT       & 0xff ] = MU_KEY_SHIFT,
    [ SDLK_LCTRL        & 0xff ] = MU_KEY_CTRL,
    [ SDLK_RCTRL        & 0xff ] = MU_KEY_CTRL,
    [ SDLK_LALT         & 0xff ] = MU_KEY_ALT,
    [ SDLK_RALT         & 0xff ] = MU_KEY_ALT,
    [ SDLK_RETURN       & 0xff ] = MU_KEY_RETURN,
    [ SDLK_BACKSPACE    & 0xff ] = MU_KEY_BACKSPACE,
};


static int text_width(mu_Font font, const char *text, int len) {
    if (len == -1) { len = strlen(text); }
    return r_get_text_width(text, len);
}

static int text_height(mu_Font font) {
    return r_get_text_height();
}


void config_window() {
    if (window_open)
        return;

    window_open = 1;

    /* init renderer */
    r_init();

    /* init microui */
    ctx = malloc(sizeof(mu_Context));
    mu_init(ctx);
    ctx->text_width = text_width;
    ctx->text_height = text_height;

    /* main loop */
    for (;;) {
        /* handle SDL events */
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT: gui_deinit(); return;
                case SDL_MOUSEMOTION: mu_input_mousemove(ctx, e.motion.x, e.motion.y); break;
                case SDL_MOUSEWHEEL: mu_input_scroll(ctx, 0, e.wheel.y * -30); break;
                case SDL_TEXTINPUT: mu_input_text(ctx, e.text.text); break;

                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP: {
                    int b = button_map[e.button.button & 0xff];
                    if (b && e.type == SDL_MOUSEBUTTONDOWN) { mu_input_mousedown(ctx, e.button.x, e.button.y, b); }
                    if (b && e.type ==   SDL_MOUSEBUTTONUP) { mu_input_mouseup(ctx, e.button.x, e.button.y, b);   }
                    break;
                }

                case SDL_KEYDOWN:
                case SDL_KEYUP: {
                    int c = key_map[e.key.keysym.sym & 0xff];
                    if (c && e.type == SDL_KEYDOWN) { mu_input_keydown(ctx, c); }
                    if (c && e.type ==   SDL_KEYUP) { mu_input_keyup(ctx, c);   }
                    break;
                }
            }
        }

        /* process frame */
        process_frame(ctx);

        /* render */
        r_clear(mu_color(bg[0], bg[1], bg[2], 255));
        mu_Command *cmd = NULL;
        while (mu_next_command(ctx, &cmd)) {
            switch (cmd->type) {
                case MU_COMMAND_TEXT: r_draw_text(cmd->text.str, cmd->text.pos, cmd->text.color); break;
                case MU_COMMAND_RECT: r_draw_rect(cmd->rect.rect, cmd->rect.color); break;
                case MU_COMMAND_ICON: r_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color); break;
                case MU_COMMAND_CLIP: r_set_clip_rect(cmd->clip.rect); break;
            }
        }
        r_present();
    }
}

void gui_deinit()
{
    window_open = 0;
    free(ctx);
    r_close();
}
