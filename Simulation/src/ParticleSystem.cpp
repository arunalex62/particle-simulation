#include "ParticleSystem.h"

#include "Random.h"

#include <glm/gtc/constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>

ParticleSystem::ParticleSystem()
{
	m_ParticlePool.resize(100);
	gravity = -0.6f;
	for (int i = 0; i < 100; ++i) {
		m_ParticlePool[i].Position = { i / 100.0f, i / 100.0f };
		m_ParticlePool[i].Velocity = { 0.0f, 0.0f }; 
		m_ParticlePool[i].Forces = { 0.0f, 0.0f }; 
		m_ParticlePool[i].Colour = { 13 / 255.0f, 38 / 255.0f, 212 / 255.0f, 1.0f };
	}
}

void ParticleSystem::OnUpdate(GLCore::Timestep ts)
{
	for (auto& particle : m_ParticlePool)
	{
		particle.Forces.y += gravity;
		if (particle.Position.y < 0.0f)
		{
			particle.Position.y = 0.0f;
			particle.Velocity.y *= -0.5f;
			//particle.Forces.y += -0.5 * particle.Velocity.y;
		}
		particle.Velocity += particle.Forces * (float)ts;
		particle.Position += particle.Velocity * (float)ts;
		particle.Forces = { 0.0f, 0.0f };
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
			* glm::scale(glm::mat4(1.0f), { 0.1f, 0.1f, 1.0f });
		glUniformMatrix4fv(m_ParticleShaderTransform, 1, GL_FALSE, glm::value_ptr(transform));
		glUniform4fv(m_ParticleShaderColor, 1, glm::value_ptr(particle.Colour));
		glBindVertexArray(m_QuadVA);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}
}
