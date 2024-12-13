#include "SandboxLayer.h"

using namespace GLCore;
using namespace GLCore::Utils;

SandboxLayer::SandboxLayer()
	: m_CameraController(16.0f / 9.0f)
{
}

SandboxLayer::~SandboxLayer()
{
}

void SandboxLayer::OnAttach()
{
	EnableGLDebugging();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Init here
	m_Particle.Colour = { 13 / 255.0f, 38 / 255.0f, 212 / 255.0f, 1.0f };
	m_Particle.Velocity = { 0.0f, 0.0f };
	m_Particle.Forces = { 0.0f, 0.0f };
	m_Particle.Position = { 0.0f, 0.0f };
}

void SandboxLayer::OnDetach()
{
	// Shutdown here
}

void SandboxLayer::OnEvent(Event& event)
{
	// Events here
	m_CameraController.OnEvent(event);

	if (event.GetEventType() == EventType::WindowResize)
	{
		WindowResizeEvent& e = (WindowResizeEvent&)event;
		glViewport(0, 0, e.GetWidth(), e.GetHeight());
	}
}

void SandboxLayer::OnUpdate(Timestep ts)
{
	m_CameraController.OnUpdate(ts);

	// Render here
	glClearColor(0,0,0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	m_ParticleSystem.OnUpdate(ts);
	m_ParticleSystem.OnRender(m_CameraController.GetCamera());
}

void SandboxLayer::OnImGuiRender()
{
	// ImGui here

	ImGui::Begin("Settings");
	ImGui::Text(m_ParticleSystem.frames_per_second.c_str());
	ImGui::DragFloat("Gravity", &m_ParticleSystem.gravity, -0.100f, -10.000f, 0.000f);
	ImGui::DragFloat("Vorticity", &m_ParticleSystem.vorticityEpsilon, 0.001f, 0.00f, 0.050f);
	ImGui::DragFloat("Bouyancy", &m_ParticleSystem.buoyancyEpsilon, 0.02f, 0.00f, 0.050f);
	ImGui::End();
}
