#include "renderer/camera.h"


void camera_init(camera_t* cam, vec3_t origin, vec3_t direction, float aspect, double fov){
  if (!cam) return;
  
  cam->fov = (float)DEG2RAD(fov);
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

  printf("pitch/yaw = %0.2f/%0.2f\n", cam->angles[PITCH], cam->angles[YAW]);

  VectorCrossNorm(cam->front, AXIS_Y, cam->right);
  VectorCrossNorm(cam->right, cam->front, cam->up);

  MatrixPerspective_VK(cam->fov, cam->aspect, cam->near, cam->far, cam->proj);

  vec3_t centre;
  VectorAdd(cam->origin, cam->front, centre);
  MatrixLookAt(cam->origin, centre, AXIS_Y, cam->view);

  printf("origin: %f %f %f\n",
    cam->origin[0], cam->origin[1], cam->origin[2]);
  /*
  printf("origin: %f %f %f\n",
    cam->origin[0], cam->origin[1], cam->origin[2]);

printf("front: %f %f %f\n",
    cam->front[0], cam->front[1], cam->front[2]);

printf("angles: %f %f %f\n",
    cam->angles[0], cam->angles[1], cam->angles[2]);

printf("view:\n");
for (int i = 0; i < 4; i++)
    printf("%f %f %f %f\n",
        cam->view[i],
        cam->view[i + 4],
        cam->view[i + 8],
        cam->view[i + 12]);

printf("proj:\n");
for (int i = 0; i < 4; i++)
    printf("%f %f %f %f\n",
        cam->proj[i],
        cam->proj[i + 4],
        cam->proj[i + 8],
        cam->proj[i + 12]);
    */
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
