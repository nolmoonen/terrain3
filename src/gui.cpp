#include "gui.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <chrono>

void gui_init(window* w)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)get_handle(w), true);
    // todo connect this to the GL loading code somehow?
    ImGui_ImplOpenGL3_Init("#version 430");

    ImGuiIO& io    = ImGui::GetIO();
    io.IniFilename = nullptr; // disable ini file
    // get rid of border
    ImGui::StyleColorsDark();
    io.Fonts->AddFontDefault();
    ImGui::GetStyle().WindowBorderSize = 0.0f;
}

void gui_cleanup()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void begin_frame_imgui()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void end_frame_imgui()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void display_text(const char* text, float x, float y)
{
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::SetNextWindowSize(ImVec2(1000.f, 1000.f)); // just make it really large
    ImGui::SetNextWindowPos(ImVec2(x, y));
    ImGui::Begin(
        "TextOverlayFG",
        nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoInputs);
    ImGui::TextColored(ImColor(1.f, 0.f, 1.f, 1.f), "%s", text);
    ImGui::End();
}

void display_stats(
    std::chrono::duration<double>& update_time, std::chrono::duration<double>& render_time)
{
    constexpr std::chrono::duration<double> display_update_min_interval_time(.5);
    static int32_t total_subframe_count = 0;
    static int32_t last_update_frames   = 0;
    static auto last_update_time        = std::chrono::steady_clock::now();
    static char displayed_text[128];

    const auto cur_time = std::chrono::steady_clock::now();

    begin_frame_imgui();
    last_update_frames++;

    typedef std::chrono::duration<double, std::milli> durationMs;

    if (cur_time - last_update_time > display_update_min_interval_time ||
        total_subframe_count == 0) {
        sprintf(
            displayed_text,
            "%5.1f fps\n"
            "update: %5.5f ms\n"
            "render: %5.5f ms\n",
            last_update_frames / std::chrono::duration<double>(cur_time - last_update_time).count(),
            (durationMs(update_time) / last_update_frames).count(),
            (durationMs(render_time) / last_update_frames).count());

        last_update_time   = cur_time;
        last_update_frames = 0;
        update_time = render_time = std::chrono::duration<double>::zero();
    }
    display_text(displayed_text, 10.0f, 10.0f);
    end_frame_imgui();

    ++total_subframe_count;
}

void display_pos(nm::fvec3 pos)
{
    static char displayed_text[128];

    begin_frame_imgui();
    sprintf(displayed_text, "x: %5.2f\ny: %5.2f\nz: %5.2f\n", pos.x, pos.y, pos.z);

    display_text(displayed_text, 10.0f, 100.0f);
    end_frame_imgui();
}