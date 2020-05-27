#include "core.h"

#include "screen.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../graphics/renderer.h"
#include "../graphics/glfwUtils.h"
#include "../graphics/texture.h"
#include "shader/shaders.h"

#include "../graphics/mesh/indexedMesh.h"
#include "../graphics/mesh/primitive.h"
#include "../graphics/debug/visualDebug.h"
#include "../graphics/buffers/frameBuffer.h"
#include "../engine/options/keyboardOptions.h"
#include "../input/standardInputHandler.h"
#include "../graphics/meshLibrary.h"
#include "../graphics/visualShape.h"
#include "../worlds.h"
#include "../engine/event/windowEvent.h"
#include "../util/resource/resourceManager.h"
#include "layer/skyboxLayer.h"
#include "layer/modelLayer.h"
#include "layer/constraintLayer.h"
#include "layer/testLayer.h"
#include "layer/pickerLayer.h"
#include "layer/postprocessLayer.h"
#include "layer/guiLayer.h"
#include "layer/debugLayer.h"
#include "layer/debugOverlay.h"

#include "../physics/geometry/shapeClass.h"
#include "../engine/meshRegistry.h"
#include "../engine/ecs/tree.h"

#include "frames.h"

#include "imgui/imgui.h"
#include "../graphics/gui/imgui/imgui_impl_glfw.h"
#include "../graphics/gui/imgui/imgui_impl_opengl3.h"

#include "../graphics/gui/imgui/imgui_impl_glfw.h"
#include "../graphics/gui/imgui/imgui_impl_opengl3.h"


struct GLFWwindow;

namespace Application {

bool initGLFW() {
	// Set window hints
	//Renderer::setGLFWMultisampleSamples(4);

	// Initialize GLFW
	if (!Graphics::GLFW::init()) {
		Log::error("GLFW failed to initialize");
		return false;
	}

	//Renderer::enableMultisampling();

	Log::info("Initialized GLFW");

	return true;
}

bool initGLEW() {
	// Init GLEW after creating a valid rendering context
	if (!Graphics::Renderer::initGLEW()) {
		terminateGLFW();

		Log::error("GLEW Failed to initialize!");

		return false;
	}

	Log::info("Initialized GLEW");
	return true;
}

void terminateGLFW() {
	Log::info("Closing GLFW");
	Graphics::GLFW::terminate();
	Log::info("Closed GLFW");
}

Screen::Screen() {

};

Screen::Screen(int width, int height, PlayerWorld* world) {
	this->world = world;

	// Create a windowed mode window and its OpenGL context 
	GLFWwindow* context = Graphics::GLFW::createContext(width, height, "Physics3D");

	if (!Graphics::GLFW::validContext(context)) {
		Log::fatal("Invalid rendering context");
		terminateGLFW();
		exit(-1);
	}

	// Make the window's context current 
	Graphics::GLFW::makeCurrent(context);

	Log::info("OpenGL vendor: (%s)", Graphics::Renderer::getVendor());
	Log::info("OpenGL renderer: (%s)", Graphics::Renderer::getRenderer());
	Log::info("OpenGL version: (%s)", Graphics::Renderer::getVersion());
	Log::info("OpenGL shader version: (%s)", Graphics::Renderer::getShaderVersion());
}


// Handler
StandardInputHandler* handler = nullptr;

// Layers
SkyboxLayer skyboxLayer;
ModelLayer modelLayer;
ConstraintLayer constraintLayer;
TestLayer testLayer;
PickerLayer pickerLayer;
PostprocessLayer postprocessLayer;
GuiLayer guiLayer;
DebugLayer debugLayer;
DebugOverlay debugOverlay;


void Screen::onInit() {
	// Log init
	Log::setLogLevel(Log::Level::INFO);

	// Properties init
	properties = Util::PropertiesParser::read("../res/.properties");

	// load options from properties
	KeyboardOptions::load(properties);

	// Library init
	Graphics::Library::onInit();

	// InputHandler init
	handler = new StandardInputHandler(Graphics::GLFW::getCurrentContext(), *this);

	// Screen size init
	dimension = Graphics::GLFW::getWindowSize();

	// Framebuffer init
	quad = new Graphics::Quad();
	screenFrameBuffer = new Graphics::FrameBuffer(dimension.x, dimension.y);

	// Shader init
	ApplicationShaders::onInit();

	// Layer creation
	skyboxLayer = SkyboxLayer(this);
	modelLayer = ModelLayer(this);
	constraintLayer = ConstraintLayer(this, Layer::NoUpdate | Layer::NoEvents);
	debugLayer = DebugLayer(this);
	pickerLayer = PickerLayer(this);
	postprocessLayer = PostprocessLayer(this);
	guiLayer = GuiLayer(this);
	testLayer = TestLayer(this);
	debugOverlay = DebugOverlay(this);

	layerStack.pushLayer(&skyboxLayer);
	layerStack.pushLayer(&modelLayer);
	layerStack.pushLayer(&constraintLayer);
	layerStack.pushLayer(&debugLayer);
	layerStack.pushLayer(&pickerLayer);
	layerStack.pushLayer(&postprocessLayer);
	layerStack.pushLayer(&guiLayer);
	layerStack.pushLayer(&testLayer);
	layerStack.pushOverlay(&debugOverlay);

	// Layer init
	layerStack.onInit();

	// Eventhandler init
	eventHandler.setWindowResizeCallback([] (Screen& screen, Vec2i dimension) {
		float aspect = float(dimension.x) / float(dimension.y);
		screen.camera.onUpdate(aspect);
		screen.dimension = dimension;
		screen.screenFrameBuffer->resize(screen.dimension);

		GUI::windowInfo.aspect = aspect;
		GUI::windowInfo.dimension = dimension;
	});

	// Camera init
	camera.setPosition(Position(1.0, 2.0, 3.0));
	camera.setRotation(Vec3(0, 3.1415, 0.0));
	camera.onUpdate(1.0f, camera.aspect, 0.01f, 10000.0f);

	// Resize
	Engine::FrameBufferResizeEvent event(dimension.x, dimension.y);
	handler->onFrameBufferResize(event);

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void) io;
	ImGui::StyleColorsDark();
	ImGuiInitGLFW(Graphics::GLFW::getCurrentContext(), true);
	ImGuiInitOpenGl("#version 130");

	BigFrame::onInit();
}

void Screen::onUpdate() {
	Engine::ECSTree* tree = this->world->ecstree;

	std::chrono::time_point<std::chrono::steady_clock> curUpdate = std::chrono::steady_clock::now();
	std::chrono::nanoseconds deltaTnanos = curUpdate - this->lastUpdate;
	this->lastUpdate = curUpdate;

	double speedAdjustment = deltaTnanos.count() * 0.000000001 * 60.0;

	// IO events
	if (handler->anyKey) {
		bool leftDragging = handler->leftDragging;
		if (handler->getKey(KeyboardOptions::Move::forward))
			camera.move(*this, 0, 0, -1 * speedAdjustment, leftDragging);
		if (handler->getKey(KeyboardOptions::Move::backward))
			camera.move(*this, 0, 0, 1 * speedAdjustment, leftDragging);
		if (handler->getKey(KeyboardOptions::Move::right))
			camera.move(*this, 1 * speedAdjustment, 0, 0, leftDragging);
		if (handler->getKey(KeyboardOptions::Move::left))
			camera.move(*this, -1 * speedAdjustment, 0, 0, leftDragging);
		if (handler->getKey(KeyboardOptions::Move::ascend))
			if (camera.flying) camera.move(*this, 0, 1 * speedAdjustment, 0, leftDragging);
		if (handler->getKey(KeyboardOptions::Move::descend))
			if (camera.flying) camera.move(*this, 0, -1 * speedAdjustment, 0, leftDragging);
		if (handler->getKey(KeyboardOptions::Rotate::left))
			camera.rotate(*this, 0, 1 * speedAdjustment, 0, leftDragging);
		if (handler->getKey(KeyboardOptions::Rotate::right))
			camera.rotate(*this, 0, -1 * speedAdjustment, 0, leftDragging);
		if (handler->getKey(KeyboardOptions::Rotate::up))
			camera.rotate(*this, 1 * speedAdjustment, 0, 0, leftDragging);
		if (handler->getKey(KeyboardOptions::Rotate::down))
			camera.rotate(*this, -1 * speedAdjustment, 0, 0, leftDragging);
		if (handler->getKey(KeyboardOptions::Application::close))
			Graphics::GLFW::closeWindow();
	}

	// Update camera
	camera.onUpdate();

	// Update layers
	layerStack.onUpdate();
}

void Screen::onEvent(Engine::Event& event) {
	using namespace Engine;

	// Consume ImGui events
	if (event.inCategory(EventCategoryKeyboard | EventCategoryMouseButton) || ImGui::IsAnyItemHovered() || ImGui::IsAnyItemActive()) {
		ImGuiIO& io = ImGui::GetIO();
		if (io.WantCaptureKeyboard || io.WantTextInput | io.WantCaptureMouse) {
			event.handled = true;
			return;
		}
	}

	camera.onEvent(event);
	layerStack.onEvent(event);
}

void Screen::onRender() {
	using namespace Graphics;
	using namespace Graphics::Renderer;

	// Init imgui
	ImGuiNewFrameOpenGL();
	ImGuiNewFrameGLFW();
	ImGui::NewFrame();

	defaultSettings();

	// Render layers
	layerStack.onRender();

	// Render imgui
	ImGui::Render();
	ImGuiRenderDrawData(ImGui::GetDrawData());

	graphicsMeasure.mark(GraphicsProcess::FINALIZE);

	// Finalize
	GLFW::swapInterval(1);
	GLFW::swapBuffers();
	GLFW::pollEvents();

	graphicsMeasure.mark(GraphicsProcess::OTHER);
}

void Screen::onClose() {
	ImGuiShutdownOpenGL();
	ImGuiShutdownGLFW();
	ImGui::DestroyContext();

	screenFrameBuffer->close();

	layerStack.onClose();

	Graphics::Library::onClose();

	ResourceManager::close();

	ApplicationShaders::onClose();

	KeyboardOptions::save(properties);

	Util::PropertiesParser::write("../res/.properties", properties);

	terminateGLFW();
}

bool Screen::shouldClose() {
	return Graphics::GLFW::isWindowClosed();
}

};