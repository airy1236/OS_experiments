#include <iostream>
#include <random>
#include <vector>

//OpenGL
#include <glad/glad.h>
#include <glfw/glfw3.h>

//IMGUI
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

enum class State : int {
	R = 1,  //就绪
	E = 2   //结束
};

class PCB {
public:
	PCB(int32_t pid, int32_t priority, double_t all_time, double_t cpu_time, State state);
	~PCB();

	bool run_process();

	int32_t pid = 0;        //进程标识
	int32_t priority = 0;   //进程优先级
	double_t all_time = 0;  //继承还需要运行的时间
	double_t cpu_time = 0;  //进程已占用的CPU时间
	State state = State::R; //状态，默认为就绪
};

bool process_finish(std::vector<PCB>& process_list);
void priority_scheduling_step(std::vector<PCB>& process_list);
void render_process_list(const std::vector<PCB>& process_list);

int main() {
    // 初始化随机数生成器
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist_priority(1, 5);
    std::uniform_int_distribution<> dist_time(1, 5);

    // 创建进程
    PCB p1(1, dist_priority(gen), dist_time(gen), 0, State::R);
    PCB p2(2, dist_priority(gen), dist_time(gen), 0, State::R);
    PCB p3(3, dist_priority(gen), dist_time(gen), 0, State::R);
    PCB p4(4, dist_priority(gen), dist_time(gen), 0, State::R);
    PCB p5(5, dist_priority(gen), dist_time(gen), 0, State::R);

    std::vector<PCB> process_list = { p1, p2, p3, p4, p5 };

    // OpenGL 和 ImGui 初始化
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Priority Scheduling", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    bool simulation_running = false;

    // 主循环
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui 主界面
        ImGui::Begin("Priority Scheduling Simulator");
        render_process_list(process_list);

        if (ImGui::Button("Start Simulation")) {
            simulation_running = true;
        }
        if (simulation_running && !process_finish(process_list)) {
            priority_scheduling_step(process_list);
        }
        else if (process_finish(process_list)) {
            simulation_running = false;
            ImGui::Text("Simulation Complete!");
        }
        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.690196097f, 0.768627524f, 0.870588303f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

PCB::PCB(int32_t pid, int32_t priority, double_t all_time, double_t cpu_time, State state) :
	pid(pid), priority(priority), all_time(all_time), cpu_time(cpu_time), state(state) {}

PCB::~PCB() {}

bool PCB::run_process() {
	if (state == State::E || all_time == 0) return false;

	priority--;
	all_time--;
	cpu_time++;

	if (all_time == 0) state = State::E;

	return true;
}

bool process_finish(std::vector<PCB>& process_list) { //检测进程是否完成
	for (auto& i : process_list) {
		if (i.state == State::R) return false;
	}

	return true;
}

void priority_scheduling_step(std::vector<PCB>& process_list) {
	for (auto& process : process_list) {
		if (process.state == State::R) {
			process.run_process();
			break; // 每次只运行一个最高优先级的进程
		}
	}
	std::sort(process_list.begin(), process_list.end(),
		[](const PCB& a, const PCB& b) { return a.priority > b.priority; });
}

void render_process_list(const std::vector<PCB>& process_list) {
	ImGui::Text("process list:");
	ImGui::Separator();
	ImGui::Text("PID\tPriority\tCPU Time\tRemaining Time\tState");
	for (const auto& process : process_list) {
		ImGui::Text("P%d\t\t%d\t\t%.2f\t\t\t%.2f\t\t\t%s",
			process.pid, process.priority, process.cpu_time, process.all_time,
			(process.state == State::R ? "R" : "E"));
	}
	ImGui::Separator();
}