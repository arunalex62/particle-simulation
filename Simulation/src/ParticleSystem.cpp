#include "ParticleSystem.h"

#include "Random.h"

#include <glm/gtc/constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>

ParticleSystem::ParticleSystem()
{
	m_ParticlePool.resize(10000);
	gravity = -0.1f;
	vorticityEpsilon = 0.001f;
	buoyancyEpsilon = 0.02f;
	frames_per_second = "FPS: 0";
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
			m_VelocityField[y][x] = { 0.0000f, 0.0001f };
			// Set bottom boundary to 0 velocity
			m_VelocityField[0][x] = { 0.0f, 0.0f };
			m_VelocityField[99][x] = m_VelocityField[98][x];
			// Ensure no vertical velocity at the top;
			m_VelocityField[99][x].y = 0.0f;
		}
	}
	for (int y = 0; y < 100; ++y) {
		for (int x = 50; x < 100; ++x) {
			m_VelocityField[y][x] = { 0.0000f, 0.0001f };
			// Set bottom boundary to 0 velocity
			m_VelocityField[0][x] = { 0.0f, 0.0f };
			m_VelocityField[99][x] = m_VelocityField[98][x];
			// Ensure no vertical velocity at the top.
			m_VelocityField[99][x].y = 0.0f;
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
			m_TemperatureField[99][x] = 300.0f;
			m_TemperatureField[0][x] = 300.0f + Random::Float() * 5.0f;
			m_VaporField[y][x] = { 0.02f + (0.001f - 0.02f) * ((float)y) / 100.0f };
			m_VaporField[99][x] = { 0.0f };
			m_CloudWaterField[y][x] = 0.0f;
		}
	}

	// Initializes the pressure field to be decreasing from the bottom to the top.
	for (int y = 0; y < 100; ++y) {
		for (int x = 0; x < 100; ++x) {
			m_PressureField[y][x] = {100000.0f * std::pow((1.0f - (static_cast<float>(y)/100.0f * 15.0f * 10.0f )/m_TemperatureField[y][x]), (- 1.0f * gravity) / (10.0f * 287.0f))};
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

	float buoyancy = -1.0f * buoyancyEpsilon * gravity * ((theta_v / theta_v0) - m_CloudWaterField[y][x]);

	return buoyancy;
}

// Implemented from 2001 paper.
std::array<std::array<float, 100>, 100> ParticleSystem::CalculateVorticity(const std::array<std::array<glm::vec2, 100>, 100>& velocityField) {
	std::array<std::array<float, 100>, 100> vorticityField;
	vorticityField.fill({ 0 });
	for (int y = 1; y < 99; ++y) {
		for (int x = 1; x < 99; ++x) {
			vorticityField[y][x] = (velocityField[y][x + 1].x - velocityField[y][x - 1].x) / 0.02f - (velocityField[y + 1][x].y - velocityField[y - 1][x].y) / 0.02f;
		}
	}
	for (int y = 0; y < 100; ++y) {
		vorticityField[y][0] = vorticityField[y][1];
		vorticityField[y][99] = vorticityField[y][98];
	}
	for (int x = 0; x < 100; ++x) {
		vorticityField[0][x] = vorticityField[1][x];
		vorticityField[99][x] = vorticityField[98][x];
	}
	return vorticityField;
}

std::array<std::array<glm::vec2, 100>, 100> ParticleSystem::ComputeNormalizedVorticityGradient(
	const std::array<std::array<float, 100>, 100>& vorticityField) {
	std::array<std::array<glm::vec2, 100>, 100> vorticityGradient;
	for (int y = 1; y < 99; ++y) {
		for (int x = 1; x < 99; ++x) {
			float nx = (std::abs(vorticityField[y][x + 1]) - std::abs(vorticityField[y][x])) / 0.01f;
			float ny = (std::abs(vorticityField[y+1][x]) - std::abs(vorticityField[y][x])) / 0.01f;
			glm::vec2 n = { nx, ny };
			float magnitude = glm::length(n);
			if (magnitude > 0.000001) {
				vorticityGradient[y][x] = n / magnitude;
			}
			else {
				vorticityGradient[y][x] = { 0.0f, 0.0f };
			}
		}
	}
	return vorticityGradient;
}

void ParticleSystem::ApplyVorticityConfinement(
	std::array<std::array<glm::vec2, 100>, 100>& velocityField,
	const std::array<std::array<float, 100>, 100>& vorticityField,
	const std::array<std::array<glm::vec2, 100>, 100>& vorticityGradient, float deltaTime) {
	for (int y = 1; y < 99; ++y) {
		for (int x = 1; x < 99; ++x) {
			float forceX = -vorticityEpsilon * vorticityGradient[y][x].y * vorticityField[y][x];
			float forceY = vorticityEpsilon * vorticityGradient[y][x].x * vorticityField[y][x];
			glm::vec2 force = { forceX, forceY };
			velocityField[y][x] += force * (float)deltaTime;
		}
	}
}

void ParticleSystem::UpdateWaterVaporField(std::array<std::array<float, 100>, 100>& temperatureField, 
	const std::array<std::array<float, 100>, 100>& pressureField, std::array<std::array<float, 100>, 100> vaporField,
	std::array<std::array<float, 100>, 100> cloudWaterField) {
	for (int y = 0; y < 100; ++y) {
		for (int x = 0; x < 100; ++x) {
			float T = temperatureField[y][x] / std::pow((100000.0f / pressureField[y][x]), 0.286f);
			float q_vs = 380.16f / pressureField[y][x] * glm::exp(17.67f * (T - 273.15f) / (T - 29.65f));
			float delta_qv = std::min(q_vs - vaporField[y][x], cloudWaterField[y][x]);
			vaporField[y][x] = vaporField[y][x] + delta_qv; 
			cloudWaterField[y][x] = cloudWaterField[y][x] - delta_qv;
			// θ = θ' + L/(cp * exner) * -1.0f * delta_qv
			temperatureField[y][x] += 2501000.0f / (1005.0f * 1.0f / std::pow((100000.0f / pressureField[y][x]), 0.286f)) * -1.0f * delta_qv;
		}
	}
}
// Compute divergence using 2001 method.
std::array<std::array<float, 100>, 100> ParticleSystem::ComputeDivergence(const std::array<std::array<glm::vec2, 100>, 100>& velocityField) {
	std::array<std::array<float, 100>, 100> divergenceField = {};
	for (int y = 1; y < 99; ++y) {
		for (int x = 1; x < 99; ++x) {
			divergenceField[y][x] = ((velocityField[y][x + 1].x + velocityField[y][x].x)/2.0f - 
				(velocityField[y][x - 1].x + velocityField[y][x].x) / 2.0f + 
				(velocityField[y + 1][x].y + velocityField[y][x].y) / 2.0f - 
				(velocityField[y - 1][x].y + velocityField[y][x].y) / 2.0f)/0.01f;
		}
	}
	return divergenceField;
}

void ParticleSystem::SetBoundaryConditions() {
	// Velocity
	for (int x = 0; x < 100; ++x) {
		// Bottom (no-slip)
		m_VelocityField[0][x] = glm::vec2(0.0f, 0.0f);
		// Top (free-slip)
		m_VelocityField[99][x] = m_VelocityField[98][x];
		m_VelocityField[99][x].y = 0.0f;
	}
	for (int y = 0; y < 100; ++y) {
		m_VelocityField[y][0].y = 0.0f;
		m_VelocityField[y][99].y = 0.0f; 
	}

	float ambientTemperature = 250.0f;
	for (int x = 0; x < 100; ++x) {
		// Set ambient temperature at the top.
		m_TemperatureField[99][x] = ambientTemperature;
	}
	for (int y = 0; y < 100; ++y) {
		// Set ambiernt temperature at the sides.
		m_TemperatureField[y][0] = ambientTemperature;
		m_TemperatureField[y][99] = ambientTemperature;
	}
	for (int x = 0; x < 100; ++x) {
		// Randomly perturb the temperature at the bottom.
		m_TemperatureField[0][x] = 300.0f + Random::Float() * 5.0f - 2.5f;
	}

	// Vapor
	for (int x = 0; x < 100; ++x) {
		// Set top qv boundary to 0.0f.
		m_VaporField[99][x] = 0.0f;
	}
	for (int y = 0; y < 100; ++y) {
		m_VaporField[y][0] = m_VaporField[y][99];
	}
	for (int x = 0; x < 100; ++x) {
		// Randomly perturb the water vapor at the bottom.
		m_VaporField[0][x] = 0.02f + Random::Float() * 0.005f - 0.0025f;
	}

	// Set all qc boundaries to 0.0f.
	for (int x = 0; x < 100; ++x) {
		m_CloudWaterField[99][x] = 0.0f;
		m_CloudWaterField[0][x] = 0.0f;
	}
	for (int y = 0; y < 100; ++y) {
		m_CloudWaterField[y][0] = 0.0f;
		m_CloudWaterField[y][99] = 0.0f;
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
			int previousX = static_cast<int>(previousPosition.x * 100.0f);
			int previousY = static_cast<int>(previousPosition.y * 100.0f);

			if (previousX < 0) {
				previousX = 0;
			}
			else if (previousX > 99) {
				previousX = 99;
			}
			if (previousY < 0) {
				previousY = 0;
			}
			else if (previousY > 99) {
				previousY = 99;
			}
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
	frames_per_second = "FPS: " + std::to_string(int(1.0f / ts.GetSeconds()));
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

		auto vorticityGradient = ComputeNormalizedVorticityGradient(vorticityField);

		ApplyVorticityConfinement(m_VelocityField, vorticityField, vorticityGradient, (float)ts);
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
	// Also updates the temperature field based on the change in water vapor field.
	{
		UpdateWaterVaporField(m_TemperatureField, m_PressureField, m_VaporField, m_CloudWaterField);
	}

	// Calculate divergence of velocity field
	{
		//std::array<std::array<float, 100>, 100> divergenceField = ComputeDivergence(m_VelocityField);
	}

	// Set boundary conditions for fields described in the paper.
	{
		SetBoundaryConditions();
	}
	// Update particles based on calculated velocity field.
	for (int i = 0; i < m_ParticlePool.size(); ++i)
	{
		// Approximate the position of the particle at the previous timestep.
		glm::vec2 prevPos = m_ParticlePool[i].Position - m_ParticlePool[i].Velocity * (float)ts;
		// Convert that position to coordinates on the 100x100 velocity field grid.
		prevPos *= 100.0f;
		prevPos = glm::clamp(prevPos, 0.0f, 99.0f);
		// Get the velocity at the previous point.
		glm::vec2 sampledVelocityAtGrid = m_VelocityField[(int)(prevPos.y)][(int)(prevPos.x)];


		m_ParticlePool[i].Forces.y += gravity;

		CheckCollisions(i);

		m_ParticlePool[i].Velocity += m_ParticlePool[i].Forces * (float)ts  +sampledVelocityAtGrid;
		m_ParticlePool[i].Position += m_ParticlePool[i].Velocity * (float)ts;
		// Colour of particle based on height.
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
