#include <vector>
#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/implot.h> //扩展库

#define TRACK_REQUEST_COUNT 10 // 请求的磁道数量
#define TRACK_MAX_COUNT 100    // 磁道总数

std::unordered_map<std::string, float> list(5);

std::vector<int> FCFS(const std::vector<int>& request) {
    return request;
}

std::vector<int> SSTF(const std::vector<int>& request) {
    std::vector<int> result;
    std::vector<bool> visited(request.size(), false);
    int current = request[0];
    result.push_back(current);

    for (size_t i = 1; i < request.size(); ++i) {
        int closest_index = -1;
        int min_distance = TRACK_MAX_COUNT + 1;

        for (size_t j = 0; j < request.size(); ++j) {
            if (!visited[j] && std::abs(request[j] - current) < min_distance) {
                closest_index = j;
                min_distance = std::abs(request[j] - current);
            }
        }

        if (closest_index != -1) {
            visited[closest_index] = true;
            current = request[closest_index];
            result.push_back(current);
        }
    }

    return result;
}

std::vector<int> SCAN(const std::vector<int>& request) {
    std::vector<int> result = request;
    std::sort(result.begin(), result.end());

    int start = request[0];
    auto it = std::lower_bound(result.begin(), result.end(), start);
    std::vector<int> scan_result;

    for (auto iter = it; iter != result.end(); ++iter) {
        scan_result.push_back(*iter);
    }

    if (it != result.begin()) {
        for (auto riter = std::make_reverse_iterator(it); riter != result.rend(); ++riter) {
            scan_result.push_back(*riter);
        }
    }

    return scan_result;
}

std::vector<int> CSCAN(const std::vector<int>& request) {
    std::vector<int> result = request;
    std::sort(result.begin(), result.end());

    int start = request[0];
    auto it = std::lower_bound(result.begin(), result.end(), start);
    std::vector<int> cscan_result;

    for (auto iter = it; iter != result.end(); ++iter) {
        cscan_result.push_back(*iter);
    }

    for (auto iter = result.begin(); iter != it; ++iter) {
        cscan_result.push_back(*iter);
    }

    return cscan_result;
}

std::vector<int> NStepSCAN(const std::vector<int>& request, int step = 10) {
    std::vector<int> result = request;
    std::sort(result.begin(), result.end());
    std::vector<int> nstepscan_result;

    for (size_t i = 0; i < result.size(); i += step) {
        auto begin = result.begin() + i;
        auto end = (i + step < result.size()) ? begin + step : result.end();
        nstepscan_result.insert(nstepscan_result.end(), begin, end);
    }

    return nstepscan_result;
}

// 计算寻道距离
int calculate_seek_distance(const std::vector<int>& result) {
    int total_distance = 0;
    for (size_t i = 1; i < result.size(); ++i) {
        total_distance += std::abs(result[i] - result[i - 1]);
    }
    return total_distance;
}

// 计算平均寻道距离
float calculate_average_seek_distance(int total_distance, int num_requests) {
    return static_cast<float>(total_distance) / (num_requests - 1);
}

GLFWwindow* setup_window() {
    // 初始化 GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return nullptr;
    }

    // 配置 GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(800, 600, "Disk Scheduling Visualization", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    // 设置当前上下文为此窗口
    glfwMakeContextCurrent(window);

    // 初始化 glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize glad!" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return nullptr;
    }

    // 设置 OpenGL 视口
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    return window;
}

void setup_imgui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    ImGui::StyleColorsDark();
}

void cleanup_imgui() {
    ImPlot::DestroyContext();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void render_imgui_with_chart(const std::vector<int>& track_request,
    const std::vector<int>& fcfs_result,
    const std::vector<int>& sstf_result,
    const std::vector<int>& scan_result,
    const std::vector<int>& cscan_result,
    const std::vector<int>& nstepscan_result) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Disk Scheduling Visualization");

    ImGui::Text("Track request sequence:");
    for (int track : track_request) {
        ImGui::Text("%d ", track);
        ImGui::SameLine();
    }
    ImGui::NewLine();

    static std::string current_algorithm = "FCFS";
    if (ImGui::Button("FCFS")) current_algorithm = "FCFS";
    ImGui::SameLine();
    if (ImGui::Button("SSTF")) current_algorithm = "SSTF";
    ImGui::SameLine();
    if (ImGui::Button("SCAN")) current_algorithm = "SCAN";
    ImGui::SameLine();
    if (ImGui::Button("CSCAN")) current_algorithm = "CSCAN";
    ImGui::SameLine();
    if (ImGui::Button("NStepSCAN")) current_algorithm = "NStepSCAN";

    const std::vector<int>* result = nullptr;
    if (current_algorithm == "FCFS") result = &fcfs_result;
    else if (current_algorithm == "SSTF") result = &sstf_result;
    else if (current_algorithm == "SCAN") result = &scan_result;
    else if (current_algorithm == "CSCAN") result = &cscan_result;
    else if (current_algorithm == "NStepSCAN") result = &nstepscan_result;

    std::vector<int> access_order(result->size());
    for (int i = 0; i < result->size(); ++i) {
        access_order[i] = i;
    }
    
    // 计算寻道效率
    int total_distance = calculate_seek_distance(*result);
    float avg_distance = calculate_average_seek_distance(total_distance, result->size());

    // 显示寻道效率
    ImGui::Text("Total Seek Distance: %d", total_distance);
    ImGui::Text("Average Seek Distance: %.2f", avg_distance);

    // 按平均寻道距离排序
    list[current_algorithm] = avg_distance;
    std::vector<std::pair<std::string, float>> sorted_list(list.begin(), list.end());
    std::sort(sorted_list.begin(), sorted_list.end(), 
        [](const std::pair<std::string, float>& a, const std::pair<std::string, float>& b) {
            return a.second < b.second;
        });
    ImGui::Text("Algorithm Performance Sorted by Average Seek Distanc: ");
    ImGui::SameLine();
    for (auto algorithm : sorted_list) {
        ImGui::Text("%s ", algorithm.first.c_str());
        ImGui::SameLine();
    }
    ImGui::NewLine();

    if (result) {
        if (ImPlot::BeginPlot("Scheduling Result", 
            ImVec2(-1, 300),  // 设置大小
            ImPlotFlags_CanvasOnly)) {
            // 设置 X 轴（磁道号）在顶部
            ImPlot::SetupAxis(ImAxis_X1, "Track Number", ImPlotAxisFlags_Opposite);
            ImPlot::SetupAxis(ImAxis_Y1, "Access Order", ImPlotAxisFlags_Invert);

            // 添加背景网格线
            ImPlot::SetupAxesLimits(0, 100, 0, (int)result->size());
            ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
            ImPlot::PlotShaded("Background", result->data(), access_order.data(), result->size());
            ImPlot::PopStyleVar();

            // 绘制折线图
            ImPlot::PlotLine("Track Access", result->data(), access_order.data(), result->size());
            ImPlot::EndPlot();
        }
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

int main() {
    GLFWwindow* window = setup_window();
    if (!window) return -1;

    setup_imgui(window);

    srand(static_cast<unsigned>(time(nullptr)));
    std::vector<int> track_request(TRACK_REQUEST_COUNT);
    for (int& track : track_request) {
        track = rand() % TRACK_MAX_COUNT;
    }

    std::vector<int> fcfs_result = FCFS(track_request);
    std::vector<int> sstf_result = SSTF(track_request);
    std::vector<int> scan_result = SCAN(track_request);
    std::vector<int> cscan_result = CSCAN(track_request);
    std::vector<int> nstepscan_result = NStepSCAN(track_request);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        render_imgui_with_chart(track_request, fcfs_result, 
            sstf_result, 
            scan_result, 
            cscan_result, 
            nstepscan_result);

        glfwSwapBuffers(window);
    }

    cleanup_imgui();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}