#include "ParticleSystem.h"

#include "Random.h"

#include <glm/gtc/constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>

ParticleSystem::ParticleSystem()
{
	m_ParticlePool.resize(10000);
	gravity = -0.1f;
	for (int i = 0; i < m_ParticlePool.size(); ++i) {
		m_ParticlePool[i].Position = { Random::Float(), Random::Float() };
		m_ParticlePool[i].Velocity = { (Random::Float() - 0.5f) / 10.0f, (Random::Float() - 0.5f) / 10.0f };
		m_ParticlePool[i].Forces = { 0.0f, 0.0f }; 
		m_ParticlePool[i].Colour = { 13 / 255.0f, 38 / 255.0f, 212 / 255.0f, 1.0f };
		m_ParticlePool[i].Temperature = Random::Float() * 60.0f + 250.0f;
		m_ParticlePool[i].Qv = Random::Float();
		m_ParticlePool[i].Qc = 0.0f;
	}
	// Set the velocity field to random values
	for (int y = 0; y < 100; ++y) {
		for (int x = 0; x < 50; ++x) {
			m_VelocityField[y][x] = { Random::Float() / 2000.0f, 0.0f };
		}
	}
	for (int y = 0; y < 100; ++y) {
		for (int x = 50; x < 100; ++x) {
			m_VelocityField[y][x] = { (Random::Float() - 1.0f) / 2000.0f, 0.0f };
		}
	}
	// Initializes Temperature field to be cooler at the top and warmer at the bottom. 
	// This is to simulate the atmosphere. 
	// 300K at the bottom and 250K at the top.

	// Initializes the vapor field to be decreasing from the bottom to the top. 
	// This is to simulate the atmosphere. 

	// Initializes the condensed cloud water field (Qc) to be 0 everywhere. 
	// Because we are not simulating any clouds at the start. 
	for (int y = 0; y < 100; ++y) {
		for (int x = 0; x < 100; ++x) {
			m_TemperatureField[y][x] = {300.0f - 50.0f * ((float)y)/100.0f};
			m_VaporField[y][x] = { 0.02f + (0.001f - 0.02f) * ((float)y) / 100.0f };
			m_CloudWaterField[y][x] = 0.0f;
		}
	}

	// Initializes the pressure field to be decreasing from the bottom to the top.
	for (int y = 0; y < 100; ++y) {
		for (int x = 0; x < 100; ++x) {
			m_PressureField[y][x] = {100000.0f * std::pow((1.0f - (static_cast<float>(y)/100.0f * 15.0f * 10.0f )/m_TemperatureField[y][x]), 9.8f/(10.0f * 287.0f)) };
		}
	}
}

// Handles collisions with the 4 walls. If a particle hits a wall, it bounces off it -
// with half the velocity it had before in that direction.
void ParticleSystem::CheckCollisions(const int i) {

	if (m_ParticlePool[i].Position.y < 0.0f)
	{
		m_ParticlePool[i].Position.y = 0.0f;
		m_ParticlePool[i].Velocity.y *= -0.3f;
	}
	else if (m_ParticlePool[i].Position.y > 1.0f)
	{
		m_ParticlePool[i].Position.y = 1.0f;
		m_ParticlePool[i].Velocity.y *= -0.3f;
	}
	if (m_ParticlePool[i].Position.x < 0.0f)
	{
		m_ParticlePool[i].Position.x = 0.0f;
		m_ParticlePool[i].Velocity.x *= -0.3f;
	}
	else if (m_ParticlePool[i].Position.x > 1.0f)
	{
		m_ParticlePool[i].Position.x = 1.0f;
		m_ParticlePool[i].Velocity.x *= -0.3f;
	}
}

float ParticleSystem::CalculateBuoyancyForce(const int x, const int y) {
	const float theta_v0 = 295.0f; 
	float theta_v = m_TemperatureField[y][x] * (1.0f + 0.61f * m_VaporField[y][x]);

	float buoyancy = -0.01f * gravity * ((theta_v / theta_v0) - m_CloudWaterField[y][x]);

	return buoyancy;
}

std::array<std::array<float, 100>, 100> ParticleSystem::CalculateVorticity(const std::array<std::array<glm::vec2, 100>, 100>& velocityField) {
	std::array<std::array<float, 100>, 100> vorticityField;
	for (int y = 1; y < 99; ++y) {
		for (int x = 1; x < 99; ++x) {
			float dvdx = (velocityField[y][x + 1].y - velocityField[y][x - 1].y) / (0.02f);
			float dudy = (velocityField[y + 1][x].x - velocityField[y - 1][x].x) / (0.02f);
			vorticityField[y][x] = dvdx - dudy;
		}
	}
	return vorticityField;
}

std::array<std::array<glm::vec2, 100>, 100> ParticleSystem::ComputeVorticityGradient(
	const std::array<std::array<float, 100>, 100>& vorticityField) {
	std::array<std::array<glm::vec2, 100>, 100> vorticityGradient;
	for (int y = 1; y < 99; ++y) {
		for (int x = 1; x < 99; ++x) {
			float dwdx = (vorticityField[y][x + 1] - vorticityField[y][x - 1]) / (0.02f);
			float dwdy = (vorticityField[y + 1][x] - vorticityField[y - 1][x]) / (0.02f);

			vorticityGradient[y][x] = glm::vec2(dwdx, dwdy);
		}
	}
	return vorticityGradient;
}

void ParticleSystem::ApplyVorticityConfinement(
	std::array<std::array<glm::vec2, 100>, 100>& velocityField,
	const std::array<std::array<float, 100>, 100>& vorticityField,
	const std::array<std::array<glm::vec2, 100>, 100>& vorticityGradient,
	float epsilon, float deltaTime) {
	for (int y = 1; y < 99; ++y) {
		for (int x = 1; x < 99; ++x) {
			glm::vec2 vorticity = vorticityGradient[y][x];
			float vorticityMagnitude = glm::length(vorticity);
			glm::vec2 N = (vorticityMagnitude > 0.0f) ? vorticity / vorticityMagnitude : glm::vec2(0.0f);
			glm::vec2 force = epsilon * 0.01f * glm::vec2(-N.y, N.x) * vorticityField[y][x];
			velocityField[y][x] += force * deltaTime;
		}
	}
}

void ParticleSystem::UpdateWaterVaporField(const std::array<std::array<float, 100>, 100>& temperatureField, 
	const std::array<std::array<float, 100>, 100>& pressureField, std::array<std::array<float, 100>, 100> vaporField,
	std::array<std::array<float, 100>, 100> cloudWaterField) {
	for (int y = 0; y < 100; ++y) {
		for (int x = 0; x < 100; ++x) {
			float T = temperatureField[y][x] / std::pow((100000.0f / pressureField[y][x]), 0.286f);
			float q_vs = 380.16f / pressureField[y][x] * glm::exp(17.67f * (T - 273.15f) / (T - 29.65f));
			float delta_qv = std::min(q_vs - vaporField[y][x], cloudWaterField[y][x]);
			vaporField[y][x] = vaporField[y][x] + delta_qv; 
			cloudWaterField[y][x] = cloudWaterField[y][x] - delta_qv;
		}
	}
}

void ParticleSystem::AdvectVelocityField(
	const std::array<std::array<glm::vec2, 100>, 100>& oldVelocity,
	std::array<std::array<glm::vec2, 100>, 100>& newVelocity, float timeStep) {
	for (int y = 0; y < 100; ++y) {
		for (int x = 0; x < 100; ++x) {

			// Converts current x/y index in loop to a normalized position in grid.
			// (x, y) = (50, 50) -> (0.5, 0.5).
			glm::vec2 currentPosition = glm::vec2(x * 0.01, y * 0.01);

			// Get the velocity at that position in the previous field.
			glm::vec2 velocity = oldVelocity[y][x];

			// Calculate the position of the particle at the previous time step. 
			// If position = (0.5, 0.5) and velocity = (0.1, 0.0), old position = (0.4, 0.5), etc.
			glm::vec2 previousPosition = currentPosition - velocity * timeStep;


			// Gets the velocity at that previous position by sampling old velocity field.
			int previousX = (int)glm::clamp(previousPosition.x * 100.0f, 0.0f, 99.0f);
			int previousY = (int)glm::clamp(previousPosition.y * 100.0f, 0.0f, 99.0f);
			newVelocity[y][x] = oldVelocity[previousY][previousX];
		}
	}
}

void ParticleSystem::AdvectScalarField(const std::array<std::array<float, 100>, 100>& oldField,
	const std::array<std::array<glm::vec2, 100>, 100>& velocityField,
	std::array<std::array<float, 100>, 100>& newField,
	float deltaTime) {
	for (int y = 0; y < 100; ++y) {
		for (int x = 0; x < 100; ++x) {
			// Converts current x/y index in loop to a normalized position in grid.
			// (x, y) = (50, 50) -> (0.5, 0.5).
			glm::vec2 currentPosition = glm::vec2(x * 0.01, y * 0.01);

			// Get the velocity at that position in the velocity field.
			glm::vec2 velocity = velocityField[y][x];

			// Calculate the position of the particle at the previous time step. 
			// If position = (0.5, 0.5) and velocity = (0.1, 0.0), old position = (0.4, 0.5), etc.

			glm::vec2 previousPosition = currentPosition - velocity * deltaTime;

			// Gets the velocity at that previous position by sampling old velocity field.
			int previousX = (int)glm::clamp(previousPosition.x * 100.0f, 0.0f, 99.0f);
			int previousY = (int)glm::clamp(previousPosition.y * 100.0f, 0.0f, 99.0f);
			newField[y][x] = oldField[previousY][previousX];
		}
	}
}

void ParticleSystem::OnUpdate(GLCore::Timestep ts)
{
	// 1. Advect velocity field (u') 
	{
		std::array<std::array<glm::vec2, 100>, 100> newVelocityField = m_VelocityField;
		AdvectVelocityField(m_VelocityField, newVelocityField, (float)ts);
		m_VelocityField = newVelocityField;
	}

	// 2. Advect scalar fields: θ, qv, qc 
	{
		std::array<std::array<float, 100>, 100> newTemperatureField = m_TemperatureField;
		std::array<std::array<float, 100>, 100> newVaporField = m_VaporField;
		std::array<std::array<float, 100>, 100> newCloudWaterField = m_CloudWaterField;

		AdvectScalarField(m_TemperatureField, m_VelocityField, newTemperatureField, (float)ts);
		AdvectScalarField(m_VaporField, m_VelocityField, newVaporField, (float)ts);
		AdvectScalarField(m_CloudWaterField, m_VelocityField, newCloudWaterField, (float)ts);

		m_TemperatureField = newTemperatureField;
		m_VaporField = newVaporField;
		m_CloudWaterField = newCloudWaterField;
	}
	// Calculate and apply vorticity 
	{
		auto vorticityField = CalculateVorticity(m_VelocityField);

		auto vorticityGradient = ComputeVorticityGradient(vorticityField);

		ApplyVorticityConfinement(m_VelocityField, vorticityField, vorticityGradient, 0.01f, (float)ts);
	}

	// Calculate and apply buoyancy force
	{
		for (int y = 0; y < 100; ++y) {
			for (int x = 0; x < 100; ++x) {
				float buoyancy = CalculateBuoyancyForce(x, y);
				m_VelocityField[y][x].y += buoyancy * float(ts);
			}
		}
	}

	// Update Qv and Qc (water vapor and cloud water) fields 
	{
		UpdateWaterVaporField(m_TemperatureField, m_PressureField, m_VaporField, m_CloudWaterField);
	}
	// Update particles based on velocity field.
	for (int i = 0; i < m_ParticlePool.size(); ++i)
	{
		glm::vec2 prevPos = m_ParticlePool[i].Position - m_ParticlePool[i].Velocity * (float)ts;
		prevPos *= 100.0f;
		prevPos = glm::clamp(prevPos, 0.0f, 99.0f);
		glm::vec2 sampledVelocityAtGrid = m_VelocityField[(int)(prevPos.y)][(int)(prevPos.x)];


		m_ParticlePool[i].Forces.y += gravity;


		CheckCollisions(i);

		m_ParticlePool[i].Velocity += m_ParticlePool[i].Forces * (float)ts  +sampledVelocityAtGrid;
		m_ParticlePool[i].Position += m_ParticlePool[i].Velocity * (float)ts;
		m_ParticlePool[i].Colour = glm::lerp(glm::vec4(13 / 255.0f, 38 / 255.0f, 212 / 255.0f, 1.0f), glm::vec4(0.9f, 0.9f, 0.9f, 1.0f), m_ParticlePool[i].Position.y);
		m_ParticlePool[i].Forces = { 0.0f, 0.0f };
	}
}

void ParticleSystem::OnRender(GLCore::Utils::OrthographicCamera& camera)
{
	if (!m_QuadVA)
	{
		float vertices[] = {
			 -0.1f, -0.1f, 0.0f,
			  0.1f, -0.1f, 0.0f,
			  0.1f,  0.1f, 0.0f,
			 -0.1f,  0.1f, 0.0f
		};

		glCreateVertexArrays(1, &m_QuadVA);
		glBindVertexArray(m_QuadVA);

		GLuint quadVB, quadIB;
		glCreateBuffers(1, &quadVB);
		glBindBuffer(GL_ARRAY_BUFFER, quadVB);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glEnableVertexArrayAttrib(quadVB, 0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

		uint32_t indices[] = {
			0, 1, 2, 2, 3, 0
		};

		glCreateBuffers(1, &quadIB);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIB);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		m_ParticleShader = std::unique_ptr<GLCore::Utils::Shader>(GLCore::Utils::Shader::FromGLSLTextFiles("assets/shader.glsl.vert", "assets/shader.glsl.frag"));
		m_ParticleShaderViewProj = glGetUniformLocation(m_ParticleShader->GetRendererID(), "u_ViewProj");
		m_ParticleShaderTransform = glGetUniformLocation(m_ParticleShader->GetRendererID(), "u_Transform");
		m_ParticleShaderColor = glGetUniformLocation(m_ParticleShader->GetRendererID(), "u_Color");
	}

	glUseProgram(m_ParticleShader->GetRendererID());
	glUniformMatrix4fv(m_ParticleShaderViewProj, 1, GL_FALSE, glm::value_ptr(camera.GetViewProjectionMatrix()));

	// Draw the horizontal bar at y = 0
	auto width = GLCore::Application::Get().GetWindow().GetWidth();
	glm::mat4 barTransform = glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, 0.0f })  // Position at y = 0
		* glm::scale(glm::mat4(1.0f), { width, 0.1f, 1.0f });  // Scale to make it wide and flat
	glUniformMatrix4fv(m_ParticleShaderTransform, 1, GL_FALSE, glm::value_ptr(barTransform));
	glUniform4f(m_ParticleShaderColor, 0.3f, 0.3f, 0.3f, 1.0f);  // Set color to grey
	glBindVertexArray(m_QuadVA);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

	for (auto& particle : m_ParticlePool)
	{	
		// Render
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), { particle.Position.x, particle.Position.y, 0.0f })
			* glm::scale(glm::mat4(1.0f), { 0.01f, 0.01f, 1.0f });
		glUniformMatrix4fv(m_ParticleShaderTransform, 1, GL_FALSE, glm::value_ptr(transform));
		glUniform4fv(m_ParticleShaderColor, 1, glm::value_ptr(particle.Colour));
		glBindVertexArray(m_QuadVA);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}
}
