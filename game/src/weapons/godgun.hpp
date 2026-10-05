#pragma once


#include <engine/weapon.hpp>

class CGodGun : public IWeapon
{
public:

    void Attack( void ) override;
    void Attack2( void ) override;

    void Reload( void ) override;
};