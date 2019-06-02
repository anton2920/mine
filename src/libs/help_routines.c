/*
FreeMine — a free Windows minesweeper clone written on C with SDL2
Copyright © Pavlovsky Anton, 2019

This file is part of FreeMine.

FreeMine is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

FreeMine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with FreeMine. If not, see <https://www.gnu.org/licenses/>.
*/

#include "../headers/mines.h"

__bool SDL_Init_All() {

    /* SDL2 */
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    TTF_Init();
    Mix_Init(0);

    /* Returning value */
    return __true;
}

__bool Init_window(struct SDL_Window **window, struct SDL_Renderer **renderer,
        enum field_size s) {

    /* Main part */
    if (s == small) {
        if ((*window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, small_width, small_height, SDL_WINDOW_SHOWN)) < 0) {
            return __false;
        } else if (!(*renderer = SDL_CreateRenderer(*window, -1, 0))) {
            return __false;
        }
    }
}

__bool Field_init(struct game_field *fld, enum field_size s) {

    /* Initializing variables */
    auto int i, j;

    /* Main part */
    if (s == small) {
        fld->s = s;
        fld->tiles_x = small_tiles_x;
        fld->tiles_y = small_tiles_y;
        fld->g_state = game_off;
    }

    if ((fld->fld = (block **) malloc(fld->tiles_y * sizeof(block *))) == NULL) {
        return __false;
    }
    for (i = 0; i < fld->tiles_y; ++i) {
        fld->fld[i] = (block *) malloc(sizeof(block) * fld->tiles_x);
    }

    for (i = 0; i < fld->tiles_y; ++i) {
        for (j = 0; j < fld->tiles_y; ++j) {
            fld->fld[i][j].type = nothing;
            fld->fld[i][j].check = unchecked;
            fld->fld[i][j].digit = 0;
            fld->fld[i][j].rect.x = j * tile_w + j + ((fld->s == small) ? small_tiles_x_offset : -1);
            fld->fld[i][j].rect.y = i * tile_h + i + ((fld->s == small) ? small_tiles_y_offset : -1);
            fld->fld[i][j].rect.w = tile_w;
            fld->fld[i][j].rect.h = tile_h;
        }
    }

    /* Returning value */
    return __true;
}

void Field_destroy(struct game_field *fld, enum field_size s) {

    /* Initializing variables */
    auto int i;

    /* Main part */
    if (s == small) {
        for (i = 0; i < fld->tiles_y; ++i) {
            free(fld->fld[i]);
        }
    }

    free(fld->fld);
}

enum field_size Get_Size(const char *argv[]) {

    /* Main part */
    return (**(argv + 1) == '0') ? small : (**(argv + 1) == '1') ? medium : large;
}

int mines_l(enum field_size s) {

    /* Returning value */
    return (s == small) ? small_nmines : (s == medium) ? 0 : 1; /* Rewrite */
}

block *get_clicked_block(struct game_field *fld, int x, int y) {

    /* Initializing variables */
    auto int i, j;

    /* Main part */
    for (i = 0; i < fld->tiles_y; ++i) {
        for (j = 0; j < fld->tiles_x; ++j) {
            if (x >= fld->fld[i][j].rect.x && x <= fld->fld[i][j].rect.x + fld->fld[i][j].rect.w &&
                    y >= fld->fld[i][j].rect.y && y <= fld->fld[i][j].rect.y + fld->fld[i][j].rect.h) {
                return fld->fld[i] + j;
            }
        }
    }

    /* Returning value */
    return NULL;
}

enum check_type switch_block_check_type(block *blk, enum mbtn b) {

    /* Main part */
    if (b == mbtn_right) {
        if (blk->check != pressed) {
            blk->check = (blk->check == unchecked) ? flaggy : (blk->check == flaggy) ? question : (blk->check == question) ? unchecked : flaggy;
        }
    } else if (b == mbtn_left) {
        if (blk->check != flaggy && blk->check != pressed) {
            blk->check = pressed;
        }
    }
}

void Block_untoggle_hovered(struct game_field *fld) {

    /* Initializing variables */
    auto int i, j;

    /* Main part */
    for (i = 0; i < fld->tiles_y; ++i) {
        for (j = 0; j < fld->tiles_x; ++j) {
            if (fld->fld[i][j].check == hovered) {
                fld->fld[i][j].check = unchecked;
            } else if (fld->fld[i][j].check == hovered_question) {
                fld->fld[i][j].check = question;
            }
        }
    }
}

__bool is_hit_face(enum field_size s, int x, int y) {

    /* Main part */
    if (s == small) {
        return (x >= small_face_x_offset && x <= small_face_x_offset + face_w &&
                y >= small_face_y_offset && y <= small_face_y_offset + face_h) ? __true : __false;
    }
}

void two_btns(struct game_field *fld, int x, int y) {

    /* Initializing variables */
    auto int i = 0, j = 0, i1, j1;
    auto __bool is_found = __false;

    /* Main part */
    for (i = 0; i < fld->tiles_y; ++i) {
        for (j = 0; j < fld->tiles_x; ++j) {
            if (x >= fld->fld[i][j].rect.x && x <= fld->fld[i][j].rect.x + fld->fld[i][j].rect.w &&
                y >= fld->fld[i][j].rect.y && y <= fld->fld[i][j].rect.y + fld->fld[i][j].rect.h) {
                is_found = __true;
                break;
            }
        }
        if (is_found) {
            break;
        }
    }
    if (is_found) {
        for (i1 = ((i - 1 >= 0) ? i - 1 : 0); i1 < ((i + 2 <= fld->tiles_y) ? i + 2 : fld->tiles_y); ++i1) {
            for (j1 = ((j - 1 >= 0) ? j - 1 : 0); j1 < ((j + 2 <= fld->tiles_x) ? j + 2 : fld->tiles_x); ++j1) {
                if (fld->fld[i1][j1].check != flaggy && fld->fld[i1][j1].check != question && fld->fld[i1][j1].check != pressed) {
                    fld->fld[i1][j1].check = hovered;
                } else if (fld->fld[i1][j1].check == question) {
                    fld->fld[i1][j1].check = hovered_question;
                }
            }
        }
    }

}