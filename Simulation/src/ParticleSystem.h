#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>
#include <array>

struct Particle
{
	glm::vec2 Position;
	glm::vec2 Velocity;
	glm::vec2 Forces;
	float Density;
	glm::vec4 Colour;
	float Temperature;
	float Qv;
	float Qc;
};

class ParticleSystem
{
public:
	ParticleSystem();

	void OnUpdate(GLCore::Timestep ts);
	void OnRender(GLCore::Utils::OrthographicCamera& camera);
	void CheckCollisions(const int particleIndex);
	float CalculateBuoyancyForce(const int x, const int y);
	std::array<std::array<float, 100>, 100> CalculateVorticity(const std::array<std::array<glm::vec2, 100>, 100>& velocityField);
	std::array<std::array<glm::vec2, 100>, 100> ComputeNormalizedVorticityGradient(
		const std::array<std::array<float, 100>, 100>& vorticityField);
	void ApplyVorticityConfinement(
		std::array<std::array<glm::vec2, 100>, 100>& velocityField,
		const std::array<std::array<float, 100>, 100>& vorticityField,
		const std::array<std::array<glm::vec2, 100>, 100>& vorticityGradient,
		float deltaTime);
	void UpdateWaterVaporField(std::array<std::array<float, 100>, 100>& temperatureField,
		const std::array<std::array<float, 100>, 100>& pressureField, std::array<std::array<float, 100>, 100> vaporField,
		std::array<std::array<float, 100>, 100> cloudWaterField);
	std::array<std::array<float, 100>, 100> ComputeDivergence(
		const std::array<std::array<glm::vec2, 100>, 100>& velocityField);
	void SetBoundaryConditions();
	void AdvectVelocityField(
		const std::array<std::array<glm::vec2, 100>, 100>& oldField,
		std::array<std::array<glm::vec2, 100>, 100>& newField, float timeStep);
	void AdvectScalarField(const std::array<std::array<float, 100>, 100>& oldField,
		const std::array<std::array<glm::vec2, 100>, 100>& velocityField,
		std::array<std::array<float, 100>, 100>& newField,
		float deltaTime);
	float vorticityEpsilon;
	float buoyancyEpsilon;
	float gravity;
	std::string frames_per_second;
private:
	std::vector<Particle> m_ParticlePool;
	std::array<std::array<glm::vec2, 100>, 100> m_VelocityField;
	std::array<std::array<float, 100>, 100> m_TemperatureField;
	std::array<std::array<float, 100>, 100> m_VaporField;
	std::array<std::array<float, 100>, 100> m_CloudWaterField;
	std::array<std::array<float, 100>, 100> m_PressureField;
	GLuint m_QuadVA = 0;
	std::unique_ptr<GLCore::Utils::Shader> m_ParticleShader;
	GLint m_ParticleShaderViewProj, m_ParticleShaderTransform, m_ParticleShaderColor;
};