#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>

// OpenGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// ImGui
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

using namespace std;

class FreeAreaTable {
public:
    int start;     //起始地址
    int length;    //块大小
    string status; //状态

    FreeAreaTable(int start, int length, string status) : start(start), length(length), status(status) {}
};

class AllocatedTable {
public:
    int start;   //起始地址
    int length;  //块大小
    string name; //分配名称

    AllocatedTable(int start, int length, string name) : start(start), length(length), name(name) {}
};

vector<FreeAreaTable> free_list;
vector<AllocatedTable> allocated_list;

enum AllocationMethod { FIRST_FIT, BEST_FIT, WORST_FIT };
AllocationMethod current_method = FIRST_FIT;

void merge_free_area() {
    if (free_list.empty()) return;

    // Sort by start address for proper merging
    sort(free_list.begin(), free_list.end(), [](const FreeAreaTable& a, const FreeAreaTable& b) {
        return a.start < b.start;
        });

    // Merge adjacent free blocks
    for (auto it = free_list.begin(); it != free_list.end() && next(it) != free_list.end(); ) {
        auto next_it = next(it);
        if (it->start + it->length == next_it->start) {
            it->length += next_it->length;
            free_list.erase(next_it);
        }
        else {
            ++it;
        }
    }
}

void allocate_main_memory(int length, string name) {
    if (free_list.empty()) {
        cout << "No free memory available." << endl;
        return;
    }

    auto it = free_list.end();

    // Find the appropriate free block based on the allocation method
    switch (current_method) {
    case FIRST_FIT:
        it = find_if(free_list.begin(), free_list.end(), [length](const FreeAreaTable& block) {
            return block.length >= length;
            });
        break;

    case BEST_FIT:
        it = min_element(free_list.begin(), free_list.end(), [length](const FreeAreaTable& a, const FreeAreaTable& b) {
            return (a.length >= length && (a.length < b.length || b.length < length));
            });
        break;

    case WORST_FIT:
        it = max_element(free_list.begin(), free_list.end(), [length](const FreeAreaTable& a, const FreeAreaTable& b) {
            return a.length < b.length;
            });
        if (it != free_list.end() && it->length < length) it = free_list.end(); // Check if a valid block is found
        break;
    }

    if (it == free_list.end() || it->length < length) {
        cout << "No suitable free memory block found." << endl;
        return;
    }

    int start = it->start;
    if (it->length == length) {
        free_list.erase(it); // Remove the block if fully allocated
    }
    else {
        it->start += length;
        it->length -= length; // Adjust block size and position
    }

    allocated_list.emplace_back(start, length, name);
    cout << "Memory Allocated: " << name << " at " << start << " with size " << length << endl;
}

void recycle_main_memory(string name) {
    auto it = find_if(allocated_list.begin(), allocated_list.end(), [name](const AllocatedTable& block) {
        return block.name == name;
        });

    if (it == allocated_list.end()) {
        cout << "No allocated memory block found with name: " << name << endl;
        return;
    }

    free_list.emplace_back(it->start, it->length, "Free");
    allocated_list.erase(it);

    merge_free_area();
    cout << "Memory Recycled: " << name << endl;
}

void draw_memory_visualization() {
    float width = 800.0f;  // Visualization width
    float height = 30.0f;  // Height for each memory block

    for (const auto& f : free_list) {
        ImVec2 start_pos(50 + f.start * 0.8f, 200);
        ImVec2 end_pos(start_pos.x + f.length * 0.8f, start_pos.y + height);
        ImGui::GetWindowDrawList()->AddRectFilled(start_pos, end_pos, IM_COL32(255, 0, 0, 255));
        ImGui::GetWindowDrawList()->AddText(ImVec2(start_pos.x + 5, start_pos.y + 5), IM_COL32(255, 255, 255, 255), "Free");
    }

    for (const auto& a : allocated_list) {
        ImVec2 start_pos(50 + a.start * 0.8f, 200);
        ImVec2 end_pos(start_pos.x + a.length * 0.8f, start_pos.y + height);
        ImGui::GetWindowDrawList()->AddRectFilled(start_pos, end_pos, IM_COL32(0, 255, 0, 255));
        ImGui::GetWindowDrawList()->AddText(ImVec2(start_pos.x + 5, start_pos.y + 5), IM_COL32(0, 0, 0, 255), a.name.c_str());
    }
}

void show_main_memory() {
    ImGui::Begin("Memory Management");

    draw_memory_visualization();

    ImGui::Text("Free Area Table");
    if (ImGui::BeginTable("FreeAreaTable", 3, ImGuiTableFlags_Borders)) {
        ImGui::TableSetupColumn("Start", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Length", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        for (const auto& f : free_list) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%d", f.start);
            ImGui::TableNextColumn();
            ImGui::Text("%d", f.length);
            ImGui::TableNextColumn();
            ImGui::Text("%s", f.status.c_str());
        }
        ImGui::EndTable();
    }

    ImGui::Text("Allocation Method:");
    const char* methods[] = { "First Fit", "Best Fit", "Worst Fit" };
    static int method = 0;
    if (ImGui::Combo("##Method", &method, methods, IM_ARRAYSIZE(methods))) {
        current_method = static_cast<AllocationMethod>(method);
    }

    ImGui::Spacing();

    if (ImGui::Button("Allocate Memory (300)")) {
        allocate_main_memory(300, "Process A");
    }
    if (ImGui::Button("Recycle Memory (Process A)")) {
        recycle_main_memory("Process A");
    }

    ImGui::End();
}

int main() {
    if (!glfwInit()) {
        cerr << "GLFW initialization failed!" << endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Memory Visualization", NULL, NULL);
    if (!window) {
        cerr << "Window creation failed!" << endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cerr << "GLAD initialization failed!" << endl;
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();

    free_list.emplace_back(0, 1500, "Free");

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        show_main_memory();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}