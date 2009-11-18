/* Generated by Together */

#ifndef TASKLEG_H
#define TASKLEG_H

#include "Navigation/Geometry/GeoVector.hpp"
#include "Navigation/Memento/DistanceMemento.hpp"
#include "Navigation/Memento/GeoVectorMemento.hpp"

class OrderedTaskPoint;

/**
 *  Utility class for use by OrderedTaskPoint to provide methods
 *  for calculating task segments (and accumulations thereof) according
 *  to various metrics.
 *  
 *  All of the scan_ methods propagate forwards to the end of the task.
 *  Some of these, e.g. scan_distance_remaining() can be called from
 *  the current task point, whereas others (e.g. scan_distance_nominal)
 *  should be called from the StartPoint.
 *
 *  This class uses mementos to reduce expensive re-calculation of static data. 
 */
class TaskLeg {
public:
  TaskLeg(OrderedTaskPoint &_destination);

/** 
 * Calculate distance of nominal task (sum of distances from each
 * leg's consecutive reference point to reference point for entire task).
 * 
 * @return Distance (m) of nominal task
 */
  double scan_distance_nominal();

/** 
 * Calculate distance of planned task (sum of distances from each leg's
 * achieved/scored reference points respectively for prior task points,
 * and targets or reference points for active and later task points).
 * 
 * @return Distance (m) of planned task
 */
  double scan_distance_planned();

/** 
 * Calculate distance of maximum achievable task (sum of distances from
 * each leg's achieved/scored points respectively for prior task points,
 * and maximum distance points for active and later task points).
 * 
 * @return Distance (m) of maximum achievable task
 */
  double scan_distance_max();

/** 
 * Calculate distance of minimum achievable task (sum of distances from
 * each leg's achieved/scored points respectively for prior task points,
 * and minimum distance points for active and later task points).
 *
 * @return Distance (m) of minimum achievable task
 */
  double scan_distance_min();

/** 
 * Calculate distance of planned task (sum of distances from aircraft to
 * current target/reference and for later task points from each leg's
 * targets or reference points).
 * 
 * @param ref Location of aircraft
 * 
 * @return Distance (m) remaining in the planned task
 */
  double scan_distance_remaining(const GEOPOINT &ref);

/** 
 * Calculate scored distance of achieved part of task.
 * 
 * @param ref Location of aircraft
 * 
 * @return Distance (m) achieved adjusted for scoring
 */
  double scan_distance_scored(const GEOPOINT &ref);

/** 
 * Calculate distance of achieved part of task.
 * For previous taskpoints, the sum of distances of maximum distance
 * points; for current, the distance from previous max distance point to
 * the aircraft.
 * 
 * @param ref Location of aircraft
 * 
 * @return Distance (m) achieved
 */
  double scan_distance_travelled(const GEOPOINT &ref);

protected:
  GeoVector vector_travelled;
  GeoVector vector_remaining;
  GeoVector vector_planned;

private:

  DistanceMemento memo_max;
  DistanceMemento memo_min;
  DistanceMemento memo_nominal;
  GeoVectorMemento memo_planned;
  GeoVectorMemento memo_travelled;
  GeoVectorMemento memo_remaining;

  GeoVector leg_vector_planned() const;
  
  GeoVector leg_vector_travelled(const GEOPOINT &ref) const;
  
  GeoVector leg_vector_remaining(const GEOPOINT &ref) const;
  
  double leg_distance_max() const;
  
  double leg_distance_min() const;

  double leg_distance_nominal() const;

  double leg_distance_scored(const GEOPOINT &ref) const;

  const OrderedTaskPoint* origin() const;
  OrderedTaskPoint* next() const;

  OrderedTaskPoint& destination;
};
#endif //TASKLEG_H
