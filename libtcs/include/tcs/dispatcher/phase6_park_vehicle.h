#ifndef PHASE6_PARK_VEHICLE_H
#define PHASE6_PARK_VEHICLE_H

#include "tcs/dispatcher/phase.h"
#include "tcs/dispatcher/universal_dispatch_util.h"
#include "tcs/util/order_pool.h"

namespace tcs {

// UNDONE: We need to review this part
// NOTE: The phase would be skipped
class Phase6ParkVehicle : public Phase {
 public:
  void Run() override;

 private:
  void CreateParkOrder(Vehicle* vehicle);

  VehicleService* vehicle_service_;
  MapService* map_service_;
  OrderPool* order_pool_;
  IRouter* router_;
  UniversalDispatchUtil* universal_dispatch_util_;
};

}  // namespace tcs

#endif /* PHASE6_PARK_VEHICLE_H */
