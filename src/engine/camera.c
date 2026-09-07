#include "engine/camera.h"


void camera_init(camera_t* cam, vec3_t origin, vec3_t direction, float aspect, float fov){
  if (!cam) return;
  
  cam->fov = fov;
  cam->aspect = aspect;
  cam->far = 1000.0f;
  cam->near = 0.1f;


  VectorCopy(origin, cam->origin);
  VectorCopy(direction, cam->front);
  VectorNormalise(cam->front);
  VectorAngles(direction, cam->angles);

  camera_update(cam);
}


void camera_update(camera_t* cam){
  AnglesVector(cam->angles, cam->front);
  VectorNormalise(cam->front);

  VectorCrossNorm(cam->front, AXIS_Y, cam->right);
  VectorCrossNorm(cam->right, cam->front, cam->up);

  MatrixPerspective(cam->fov, cam->aspect, cam->near, cam->far, cam->proj);

  vec3_t centre;
  VectorSub(cam->origin, cam->front, centre);
  MatrixLookAt(cam->origin, centre, AXIS_Y, cam->view);
}

const float CAM_PITCH_LIMIT = (float)(89.0f * (float)M_PI/180.0f);
void camera_look(camera_t* cam, float m_dx, float m_dy, float sens){
  cam->angles[PITCH] -= m_dy * sens;
  cam->angles[YAW] += m_dx * sens;

  if (cam->angles[PITCH] > CAM_PITCH_LIMIT){
    cam->angles[PITCH] = CAM_PITCH_LIMIT;
  }
  if (cam->angles[PITCH] < -CAM_PITCH_LIMIT){
    cam->angles[PITCH] = -CAM_PITCH_LIMIT;
  }
}
