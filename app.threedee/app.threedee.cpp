#include "app.threedee.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION

#include "nuklear/nuklear.h"
#include "nuklear/demo/glfw_opengl3/nuklear_glfw_gl3.h"
#include "nuklear/demo/style.c"
#include "font-icons.h"

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

#define UNUSED(a) (void)a
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define LEN(a) (sizeof(a)/sizeof(a)[0])

struct {
    bool show_library = false;
    bool show_inspector = true;
    bool show_smartcontrols = false;
    bool show_editors = false;
    bool show_mixer = false;
    bool show_listeditor = false;
    bool show_notes = false;
    bool show_loops = false;
    bool show_browser = false;
    float channel_height = 50.0f;
    float w = 200.0f;
    float h = 300.0f;
    float splitter1 = 255.0f;

} windowConfig;

AppThreeDee::AppThreeDee(GLFWwindow* window, Mixer* mixer)
    : _window(window), _mixer(mixer), _display_w(800), _display_h(600)
{
    glfwSetWindowUserPointer(this->_window, static_cast<void*>(this));
}

AppThreeDee::~AppThreeDee()
{
    glfwSetWindowUserPointer(this->_window, nullptr);
}

void AppThreeDee::KeyActionCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto app = static_cast<AppThreeDee*>(glfwGetWindowUserPointer(window));

    if (app != nullptr) app->onKeyAction(key, scancode, action, mods);
}

void AppThreeDee::ResizeCallback(GLFWwindow* window, int width, int height)
{
    auto app = static_cast<AppThreeDee*>(glfwGetWindowUserPointer(window));

    if (app != nullptr) app->onResize(width, height);
}

void AppThreeDee::onKeyAction(int key, int scancode, int action, int mods)
{ }

void AppThreeDee::onResize(int width, int height)
{
    float aspectw = float(width) / float(this->_display_w);
    float aspecth = float(height) / float(this->_display_h);
    windowConfig.w *= aspectw;
    windowConfig.h *= aspecth;

    this->_display_w = width;
    this->_display_h = height;

    glViewport(0, 0, width, height);
}

struct nk_context *ctx;
struct nk_color background;

bool AppThreeDee::SetUp()
{
    ctx = nk_glfw3_init(_window, NK_GLFW3_INSTALL_CALLBACKS);
    /* Load Fonts: if none of these are loaded a default font will be used  */
    /* Load Cursor: if you uncomment cursor loading please hide the cursor */
    {struct nk_font_atlas *atlas;
    nk_glfw3_font_stash_begin(&atlas);
    /*struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14, 0);*/
    /*struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 14, 0);*/
    /*struct nk_font *future = nk_font_atlas_add_from_file(atlas, "../../../extra_font/kenvector_future_thin.ttf", 13, 0);*/
    /*struct nk_font *clean = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12, 0);*/
    /*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10, 0);*/
    /*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13, 0);*/
    nk_glfw3_font_stash_end();
    /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
    /*nk_style_set_font(ctx, &droid->handle);*/}

    /* style.c */
//    set_style(ctx, THEME_WHITE);
//    set_style(ctx, THEME_RED);
    set_style(ctx, THEME_BLUE);
//    set_style(ctx, THEME_DARK);

    background = nk_rgb(28,48,62);

    return true;
}

void AppThreeDee::Render()
{
    nk_glfw3_new_frame();

    if (nk_begin(ctx, "Toolbar", nk_rect(0, 0, this->_display_w, 50), 0))
    {
        nk_layout_row_static(ctx, 30, 30, 2);
        if (nk_button_symbol(ctx, NK_SYMBOL_RECT_SOLID)) windowConfig.show_library = !windowConfig.show_library;
    }
    nk_end(ctx);

    int left = 0;

    if (windowConfig.show_library)
    {
//        if (nk_begin(ctx, "Library", nk_rect(left, 50, 200, this->_display_h - 50), 0))
//        {
//        }
//        nk_end(ctx);

        left += 200.0f;
    }

    if (nk_begin(ctx, "Demo", nk_rect(left, 50, this->_display_w - left, this->_display_h - 50), NK_WINDOW_NO_SCROLLBAR))
    {
        const struct nk_input *in = &ctx->input;
        static float a = this->_display_h - 250, b = 200;
        struct nk_rect bounds;

        /* top space */
        nk_layout_row_dynamic(ctx, this->_display_h - 100 - windowConfig.splitter1, 1);
        if (nk_group_begin(ctx, "top", 0))
        {
            nk_layout_row_template_begin(ctx, windowConfig.channel_height);
            nk_layout_row_template_push_static(ctx, 350);
            nk_layout_row_template_push_dynamic(ctx);
            nk_layout_row_template_end(ctx);
            nk_button_label(ctx, "#FFAA");
            nk_button_label(ctx, "#FFBB");
            nk_button_label(ctx, "#FFCC");
            nk_button_label(ctx, "#FFDD");
            nk_button_label(ctx, "#FFEE");
            nk_button_label(ctx, "#FFFF");
            nk_group_end(ctx);
        }

        /* scaler */
        nk_layout_row_dynamic(ctx, 8, 1);
        bounds = nk_widget_bounds(ctx);
        nk_spacing(ctx, 1);
        if ((nk_input_is_mouse_hovering_rect(in, bounds) ||
            nk_input_is_mouse_prev_hovering_rect(in, bounds)) &&
            nk_input_is_mouse_down(in, NK_BUTTON_LEFT))
        {
            windowConfig.splitter1 -= in->mouse.delta.y;
        }

        /* middle space */
        nk_layout_row_dynamic(ctx, windowConfig.splitter1, 1);
        if (nk_group_begin(ctx, "middle", 0))
        {
            nk_layout_row_template_begin(ctx, 800);
            nk_layout_row_template_push_static(ctx, 90);
            nk_layout_row_template_push_static(ctx, 80);
            nk_layout_row_template_push_static(ctx, 80);
            nk_layout_row_template_push_static(ctx, 80);
            nk_layout_row_template_push_static(ctx, 80);
            nk_layout_row_template_push_static(ctx, 80);
            nk_layout_row_template_push_static(ctx, 80);
            nk_layout_row_template_push_static(ctx, 80);
            nk_layout_row_template_push_static(ctx, 80);
            nk_layout_row_template_push_static(ctx, 80);
            nk_layout_row_template_push_static(ctx, 80);
            nk_layout_row_template_push_static(ctx, 80);
            nk_layout_row_template_push_static(ctx, 80);
            nk_layout_row_template_end(ctx);
            if (nk_group_begin(ctx, "midipartlabels", 0))
            {
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_label(ctx, "", NK_TEXT_RIGHT);
                nk_label(ctx, "Setting", NK_TEXT_RIGHT);
                nk_label(ctx, "Gain Reduction", NK_TEXT_RIGHT);
                nk_label(ctx, "EQ", NK_TEXT_RIGHT);
                nk_label(ctx, "MIDI FX", NK_TEXT_RIGHT);
                nk_label(ctx, "Input", NK_TEXT_RIGHT);
                nk_label(ctx, "Audio FX", NK_TEXT_RIGHT);
                nk_label(ctx, "Sends", NK_TEXT_RIGHT);
                nk_label(ctx, "Output", NK_TEXT_RIGHT);
                nk_label(ctx, "Group", NK_TEXT_RIGHT);
                nk_label(ctx, "Automation", NK_TEXT_RIGHT);
                nk_label(ctx, "Pan", NK_TEXT_RIGHT);
                nk_label(ctx, "dB", NK_TEXT_RIGHT);
                nk_group_end(ctx);
            }
            if (nk_group_begin(ctx, "midipart0", 0))
            {
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_label(ctx, "Instrument 1", NK_TEXT_CENTERED);
                nk_group_end(ctx);
            }
            nk_button_label(ctx, "#FFAA");
            nk_button_label(ctx, "#FFAA");
            nk_button_label(ctx, "#FFAA");
            nk_button_label(ctx, "#FFAA");
            nk_button_label(ctx, "#FFAA");
            nk_button_label(ctx, "#FFAA");
            nk_button_label(ctx, "#FFAA");
            nk_button_label(ctx, "#FFAA");
            nk_button_label(ctx, "#FFAA");
            nk_button_label(ctx, "#FFAA");
            nk_button_label(ctx, "#FFAA");
            nk_button_label(ctx, "#FFAA");
            nk_button_label(ctx, "#FFAA");
            nk_button_label(ctx, "#FFAA");
            nk_button_label(ctx, "#FFAA");
            nk_group_end(ctx);
        }
    }
    nk_end(ctx);

    float bg[4];
    nk_color_fv(bg, background);
    glViewport(0, 0, this->_display_w, this->_display_h);
    glClearColor(bg[0], bg[1], bg[2], bg[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    /* IMPORTANT: `nk_glfw_render` modifies some global OpenGL state
     * with blending, scissor, face culling, depth test and viewport and
     * defaults everything back into a default state.
     * Make sure to either a.) save and restore or b.) reset your own state after
     * rendering the UI. */
    nk_glfw3_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
}

void AppThreeDee::CleanUp()
{ }
