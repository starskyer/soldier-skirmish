#include "base.h"
#include "globalParameters.h"
namespace skirmish {
Base::Base(const BasePlacement &p)
    : team(p.team), center(p.center), spawn(p.spawn), hp(BASE_HP), maxHP(BASE_HP), damage(BASE_ATK),
      range(BASE_ATKRG) {}
} // namespace skirmish
