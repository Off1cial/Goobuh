#pragma once



// Forward declaration
struct assethandle_t;

class IRenderer 
{
public:
    virtual void DrawModel( struct assethandle_t handle );


};