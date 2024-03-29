#ifndef IROUTER_H
#define IROUTER_H

#include <list>
#include <optional>
#include <unordered_map>
#include <unordered_set>

#include "tcs/data/point.h"
#include "tcs/data/vehicle.h"
#include "tcs/order/transport_order.h"

namespace tcs {

// Router Interface
class IRouter {
 public:
  // Check if future DriveOrders of a TransportOrder is routable
  virtual bool ChechRoutability(const TransportOrder* order) = 0;
  // Compute a route for order with specified start point.
  // A routable order can still return a result with no value if its first
  // destination is not routable from start point.
  virtual std::optional<std::vector<DriveOrder>> GetRoute(
      const Point* start_point, const TransportOrder* order) = 0;
  // Compute a route for 2 points
  virtual std::optional<Route> GetRoute(const Point* start_point,
                                        const Point* destination_point) = 0;
  // Compute the cost for route of 2 points
  virtual double GetCost(const Point* start_point, const Point* destination_point) = 0;
  // Cache route selected for vehicle
  virtual void SelectRoute(const Vehicle* vehicle,
                           std::vector<DriveOrder> drive_orders) = 0;
  // Get copy of cached routes
  virtual std::unordered_map<const Vehicle*, std::vector<DriveOrder>>
  GetSelectedRoutes() = 0;
  // Get targeted points from cache.
  // These points would not be targeted by parking or charging commands.
  virtual std::unordered_set<const Point*> GetTargetedPoints() = 0;
  virtual ~IRouter() {}
};

}  // namespace tcs

#endif /* IROUTER_H */
