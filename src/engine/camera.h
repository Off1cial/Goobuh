#pragma once

#include "common/common.h"
#include "math/vector.h"
#include "math/matrix.h"


typedef struct camera_t
{
  mat4 proj, view;
  vec3_t origin;
  qangle angles;
  // derived
  vec3_t front, right, up;
  // Extra
  float fov, aspect;
  float far, near;

} camera_t; 



void camera_init(camera_t* cam, vec3_t origin, vec3_t direction, float aspect, float fov);
void camera_update(camera_t* cam);


void camera_look(camera_t* cam, float m_dx, float m_dy, float sens);
