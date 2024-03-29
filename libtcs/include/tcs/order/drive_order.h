#ifndef DRIVE_ORDER_H
#define DRIVE_ORDER_H

#include <optional>

#include "tcs/order/destination.h"
#include "tcs/order/route.h"

namespace tcs {

// enum class DriveOrderState {
//   kPristine,    // Not executed
//   kTravelling,  // Vehicle travelling
//   kOperating,   // Vehicle operating
//   kFinished,
//   kFailed
// };

class DriveOrder {
 public:
  DriveOrder(Destination destination) : destination_{destination} {}

  Destination GetDestination() const { return destination_; }

  const std::optional<Route>& GetRoute() const { return route_; }
  void SetRoute(std::optional<Route> route) { route_ = route; }

  // DriveOrderState GetState() { return state_; }
  // void SetState(DriveOrderState state) { state_ = state; }

 private:
  Destination destination_;
  std::optional<Route> route_ = std::nullopt;
  // DriveOrderState state_ = DriveOrderState::kPristine;
};

}  // namespace tcs

#endif /* DRIVE_ORDER_H */
