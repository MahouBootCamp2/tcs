#ifndef PHASE4_ASSIGN_FREE_ORDER_H
#define PHASE4_ASSIGN_FREE_ORDER_H

#include "tcs/dispatcher/phase.h"

namespace tcs {

struct AssignmentCandidate {
  Vehicle* vehicle;
  TransportOrder* transport_order;
  std::optional<std::vector<DriveOrder>> drive_orders;
  double total_cost;

  AssignmentCandidate(Vehicle* vehicle, TransportOrder* transport_order,
                      std::optional<std::vector<DriveOrder>> drive_orders)
      : vehicle{vehicle},
        transport_order{transport_order},
        drive_orders{drive_orders} {
    total_cost = 0;
    for (auto& drive_order : drive_orders.value())
      total_cost += drive_order.get_route()->get_cost();
  }
};

class Phase4AssignFreeOrder : public Phase {
 public:
  void Run() override;

 private:
  std::unordered_set<Vehicle*> FilterAvailableVehicles();
  std::unordered_set<TransportOrder*> FilterDispatchableTransportOrders();
  bool CanBypassDriveOrder(DriveOrder& drive_order, Vehicle* vehicle) {
    return drive_order.get_route()->get_steps().back().destination->get_id() ==
               vehicle->get_current_point().value() &&
           drive_order.get_destination().operation == kNoOperation;
  }
  void AssignOrder(AssignmentCandidate& candidate);
  void TryAssignByVehicle(Vehicle* vehicle,
                          std::unordered_set<TransportOrder*>& orders,
                          std::unordered_set<TransportOrder*>& assigned_orders);
  void TryAssignByOrder(TransportOrder* order,
                        std::unordered_set<Vehicle*>& vehicles,
                        std::unordered_set<Vehicle*>& assigned_vehicles);

  ControllerPool* controller_pool_;
  ReserveOrderPool* reserve_order_pool_;
  MapService* map_service_;
  TransportOrderService* transport_order_service_;
  VehicleService* vehicle_service_;
  IRouter* router_;
};

}  // namespace tcs

#endif /* PHASE4_ASSIGN_FREE_ORDER_H */