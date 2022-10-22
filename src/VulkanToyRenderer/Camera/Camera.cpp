#include <VulkanToyRenderer/Camera/Camera.h>

#include <GLFW/glfw3.h>

#include <VulkanToyRenderer/Descriptors/Types/UBO/UBOutils.h>

Camera::Camera(
      GLFWwindow* window,
      const glm::fvec4& pos,
      const CameraType& type,
      const float FOV,
      const float ratio,
      const float zNear,
      const float zFar
) : m_opWindow(window),
    m_pos(pos),
    m_type(type),
    m_FOV(FOV),
    m_ratio(ratio),
    m_zNear(zNear),
    m_zFar(zFar)
{
   m_view = UBOutils::getUpdatedViewMatrix(
         // Eyes position.
         glm::vec3(m_pos),
         // Center position
         glm::vec3(0.0f, 0.0f, 0.0f),
         // Up axis
         glm::vec3(0.0f, 1.0f, 0.0f)
   );
   m_proj = UBOutils::getUpdatedProjMatrix(
         glm::radians(m_FOV),
         m_ratio,
         m_zNear,
         m_zFar
   );
}

Camera::~Camera() {}

const CameraType Camera::getType() const
{
   return m_type;
}

const glm::mat4& Camera::getProjectionM() const
{
   return m_proj;
}

const glm::mat4& Camera::getViewM()
{
   // First we update the view matrix if the camera position changed.
   // TODO: verify if it changed it.
   m_view = UBOutils::getUpdatedViewMatrix(
         // Eyes position.
         glm::vec3(m_pos),
         // Center position
         glm::vec3(0.0f, 0.0f, 0.0f),
         // Up axis
         glm::vec3(0.0f, 1.0f, 0.0f)
   );

   return m_view;
}

const float& Camera::getFOV() const
{
   return m_FOV;
}

void Camera::setFOV(const float newFOV)
{
   m_FOV = newFOV;

   m_proj = UBOutils::getUpdatedProjMatrix(
         glm::radians(m_FOV),
         m_ratio,
         m_zNear,
         m_zFar
   );
}

glm::fvec4& Camera::getPos()
{
   return m_pos;
}