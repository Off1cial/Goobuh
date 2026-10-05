#pragma once

// FWD
typedef struct entity_t entity_t;

#include "public/engine/globalvars.hpp"



class IWeapon
{
public:
    IWeapon() = default;
    virtual ~IWeapon() = default;

    void PrimaryFire( void )
    {

        Attack();
    }

    // Traces should be swept/provided by the engine
    virtual void Attack(void) = 0;
    virtual void Attack2(void) = 0;
    virtual void Reload(void) = 0;

    void SetOwner( entity_t* owner ) { m_owner = owner; }
    entity_t* GetOwner( void ) const { return m_owner; }

    int GetAmmoPrimaryCurrent(void) const { return m_ammoprimary; }
    int GetAmmoReserveCurrent(void) const { return m_ammoreserve; }

    int GetAmmoPrimaryMax(void) const { return m_ammoprimary_max; }
    int GetAmmoReserveMax(void) const { return m_ammoreserve_max; }

protected:
    int m_ammoprimary_max = 0;
    int m_ammoreserve_max = 0;

    int m_ammoprimary = 0;
    int m_ammoreserve = 0;

    float m_lastfire = 0.0f;
    float m_firedelay = 0.1f;

    entity_t* m_owner = nullptr;
};