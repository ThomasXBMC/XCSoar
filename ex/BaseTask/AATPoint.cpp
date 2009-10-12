/* Generated by Together */

#include "AATPoint.hpp"


GEOPOINT AATPoint::get_reference_scored()
{
  if (getActiveState() == BEFORE_ACTIVE) {
    return getMaxLocation();
  } else {
    return getMinLocation();
  }
}

GEOPOINT AATPoint::get_reference_travelled()
{
  if (state_entered.Time>=0) {
    return getMaxLocation();
  } else {
    return getMinLocation();
  }
}

GEOPOINT AATPoint::get_reference_remaining()
{
  if (getActiveState() == BEFORE_ACTIVE) {
    return getMaxLocation();
  } else {
    return TargetLocation;
  }
}

double 
AATPoint::getElevation() 
{
  // TODO: look up elevation of target and use that instead
  return Elevation; // + SAFETYTERRAIN
}

bool 
AATPoint::update_sample(const AIRCRAFT_STATE& state) 
{
  bool retval = OrderedTaskPoint::update_sample(state);
  if (active_state == CURRENT_ACTIVE) {
    retval |= check_target(state);
  }
  return retval;
}

bool
AATPoint::check_target(const AIRCRAFT_STATE& state) 
{
  bool moved = false;
  if (isInSector(state)) {
    moved = check_target_inside(state);
  } else {
    moved = check_target_outside(state);
  }
  return moved;
}

bool
AATPoint::check_target_inside(const AIRCRAFT_STATE& state) 
{
  return false;
}

bool
AATPoint::check_target_outside(const AIRCRAFT_STATE& state) 
{
  return false;
}
