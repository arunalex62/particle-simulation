#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>

struct ParticleProps
{
	glm::vec2 Position;
	glm::vec2 Velocity;
	glm::vec2 Forces;
	glm::vec4 Colour;
};

class ParticleSystem
{
public:
	ParticleSystem();

	void OnUpdate(GLCore::Timestep ts);
	void OnRender(GLCore::Utils::OrthographicCamera& camera);
private:
	struct Particle
	{
		glm::vec2 Position;
		glm::vec2 Velocity;
		glm::vec2 Forces;
		glm::vec4 Colour;
	};
	std::vector<Particle> m_ParticlePool;
	float gravity;
	GLuint m_QuadVA = 0;
	std::unique_ptr<GLCore::Utils::Shader> m_ParticleShader;
	GLint m_ParticleShaderViewProj, m_ParticleShaderTransform, m_ParticleShaderColor;
};