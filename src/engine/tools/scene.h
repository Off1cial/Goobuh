#ifndef __MAP_SCENE_H__
#define __MAP_SCENE_H__

#include "engine/tools/mesh.h"


// Not the in-engine scene, this is the scene prior to compilation
class MapScene
{
public:

    MapScene();
    ~MapScene();
    void Clear();

private:
    vertex_t *m_vertices;
};

#endif