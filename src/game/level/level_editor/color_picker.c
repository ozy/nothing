#include <stdbool.h>

#include "game/level/boxes.h"
#include "system/stacktrace.h"
#include "system/line_stream.h"
#include "system/log.h"
#include "game/camera.h"
#include "proto_rect.h"
#include "color_picker.h"
#include "color.h"

#define COLOR_CELL_WIDTH 50.0f
#define COLOR_CELL_HEIGHT 50.0f

// TODO(#788): Colors of ColorPicker are poor
static Color colors[] = {
    {1.0f, 0.0f, 0.0f, 1.0f},
    {0.0f, 1.0f, 0.0f, 1.0f},
    {0.0f, 0.0f, 1.0f, 1.0f}
};
static const size_t colors_count = sizeof(colors) / sizeof(Color);

LayerPtr color_picker_as_layer(ColorPicker *color_picker)
{
    LayerPtr layer = {
        .ptr = color_picker,
        .type = LAYER_COLOR_PICKER
    };
    return layer;
}

int color_picker_read_from_line_stream(ColorPicker *color_picker,
                                       LineStream *line_stream)
{
    char color[7];
    const char *line = line_stream_next(line_stream);
    if (line == NULL) {
        return -1;
    }

    if (sscanf(line, "%6s", color) == EOF) {
        log_fail("Could not read color\n");
    }

    color_picker->color = hexstr(color);

    color_picker->hue.value = 0.0f;
    color_picker->hue.max_value = 360.0f;
    color_picker->saturation.value = 0.0f;
    color_picker->saturation.max_value = 1.0f;
    color_picker->lightness.value = 0.0f;
    color_picker->lightness.max_value = 1.0f;

    return 0;
}

int color_picker_render(const ColorPicker *color_picker,
                        Camera *camera)
{
    trace_assert(color_picker);
    trace_assert(camera);

    for (size_t i = 0; i < colors_count; ++i) {
        if (camera_fill_rect_screen(
                camera,
                rect(COLOR_CELL_WIDTH * (float) i, 0,
                     COLOR_CELL_WIDTH,
                     COLOR_CELL_HEIGHT),
                colors[i]) < 0) {
            return -1;
        }
    }

    /* TODO: Color Picker sliders don't have any labels */

    if (slider_render(
            &color_picker->hue,
            camera,
            rect(0.0f, COLOR_CELL_HEIGHT,
                 300.0f, COLOR_CELL_HEIGHT)) < 0) {
        return -1;
    }

    if (slider_render(
            &color_picker->saturation,
            camera,
            rect(0.0f, COLOR_CELL_HEIGHT * 2.0f,
                 300.0f, COLOR_CELL_HEIGHT)) < 0) {
        return -1;
    }

    if (slider_render(
            &color_picker->lightness,
            camera,
            rect(0.0f, COLOR_CELL_HEIGHT * 3.0f,
                 300.0f, COLOR_CELL_HEIGHT)) < 0) {
        return -1;
    }


    return 0;
}

int color_picker_event(ColorPicker *color_picker, const SDL_Event *event, int *selected)
{
    trace_assert(color_picker);
    trace_assert(event);

    if (slider_event(&color_picker->hue,
                     event,
                     rect(0.0f, COLOR_CELL_HEIGHT,
                          300.0f, COLOR_CELL_HEIGHT)) < 0) {
        return -1;
    }

    if (slider_event(&color_picker->saturation,
                     event,
                     rect(0.0f, COLOR_CELL_HEIGHT * 2.0f,
                          300.0f, COLOR_CELL_HEIGHT)) < 0) {
        return -1;
    }

    if (slider_event(&color_picker->lightness,
                     event,
                     rect(0.0f, COLOR_CELL_HEIGHT * 3.0f,
                          300.0f, COLOR_CELL_HEIGHT)) < 0) {
        return -1;
    }

    color_picker->color = hsla(
        color_picker->hue.value,
        color_picker->saturation.value,
        color_picker->lightness.value,
        1.0f);

    switch (event->type) {
    case SDL_MOUSEBUTTONDOWN: {
        switch (event->button.button) {
        case SDL_BUTTON_LEFT: {
            for (size_t i = 0; i < colors_count; ++i) {
                const Vec mouse_position = vec((float) event->button.x, (float) event->button.y);
                const Rect color_cell =
                    rect(COLOR_CELL_WIDTH * (float) i, 0,
                         COLOR_CELL_WIDTH,
                         COLOR_CELL_HEIGHT);
                if (rect_contains_point(color_cell, mouse_position)) {
                    color_picker->color = colors[i];
                    if (selected) {
                        *selected = true;
                    }
                    return 0;
                }
            }
        } break;
        }
    } break;
    }

    return 0;
}
