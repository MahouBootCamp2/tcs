#include "tcs/dispatcher/default_dispatcher.h"

namespace tcs {

DefaultDispatcher::DefaultDispatcher(
    Executor* executor, OrderPool* order_pool, MapService* map_service,
    VehicleService* vehicle_service,
    TransportOrderService* transport_order_service, IRouter* router,
    ControllerPool* controller_pool)
    : executor_{executor},
      order_pool_{order_pool},
      map_service_{map_service},
      vehicle_service_{vehicle_service},
      transport_order_service_{transport_order_service},
      router_{router},
      controller_pool_{controller_pool},
      reserve_order_pool_{std::make_unique<ReserveOrderPool>()},
      universal_dispatch_util_{std::make_unique<UniversalDispatchUtil>(
          executor_, map_service_, vehicle_service_, transport_order_service_,
          router_, controller_pool_, reserve_order_pool_.get())},
      phase0_{transport_order_service_, router_},
      phase1_{map_service_, vehicle_service_, transport_order_service_,
              router_},
      phase2_{map_service_, vehicle_service_, transport_order_service_, router_,
              controller_pool_},
      phase3_{map_service_,
              vehicle_service_,
              transport_order_service_,
              router_,
              reserve_order_pool_.get(),
              universal_dispatch_util_.get()},
      phase4_{map_service_,
              vehicle_service_,
              transport_order_service_,
              router_,
              controller_pool_,
              reserve_order_pool_.get(),
              universal_dispatch_util_.get()},
      phase5_{order_pool_, map_service_, vehicle_service_, router_,
              universal_dispatch_util_.get()},
      phase6_{order_pool_, map_service_, vehicle_service_, router_,
              universal_dispatch_util_.get()} {
  vehicle_service_->VehicleProcessStateChangeEvent().Subscribe(std::bind(
      &DefaultDispatcher::VehicleProcessStateChangeEventHandler, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

  vehicle_service_->VehicleNeedChangeEvent().Subscribe(
      std::bind(&DefaultDispatcher::VehicleNeedChargeEventHandler, this,
                std::placeholders::_1));
}

void DefaultDispatcher::Dispatch() {
  executor_->Submit(&DefaultDispatcher::DispatchTask, this);
}

void DefaultDispatcher::WithdrawOrder(const TransportOrder* order,
                                      bool immediate) {
  if (!immediate) {
    BOOST_LOG_TRIVIAL(info) << "Scheduling withdraw task...";
    executor_->Submit(&UniversalDispatchUtil::AbortOrderByOrder,
                      universal_dispatch_util_.get(), order);
  } else {
    // UNDONE
  }
}

void DefaultDispatcher::WithdrawOrder(const Vehicle* vehicle, bool immediate) {
  if (!immediate) {
    BOOST_LOG_TRIVIAL(info) << "Scheduling withdraw task...";
    executor_->Submit(&UniversalDispatchUtil::AbortOrderByVehicle,
                      universal_dispatch_util_.get(), vehicle);
  } else {
    // UNDONE
  }
}

void DefaultDispatcher::DispatchTask() {
  phase0_.Run();
  phase1_.Run();
  phase2_.Run();
  phase3_.Run();
  phase4_.Run();
  phase5_.Run();
  phase6_.Run();
}

void DefaultDispatcher::DispatchPeriodically() {}

void DefaultDispatcher::VehicleProcessStateChangeEventHandler(
    const Vehicle* vehicle, ProcessState old_state, ProcessState new_state) {
  if (vehicle_service_->ReadVehicleIntegrationLevel(vehicle->GetID()) ==
      IntegrationLevel::kUtilized) {
    if (old_state != new_state && (new_state == ProcessState::kAwaitingOrder ||
                                   new_state == ProcessState::kIdle))
      BOOST_LOG_TRIVIAL(debug)
          << "Dispatch on vehicle proccess state change";
    Dispatch();
  }
}

void DefaultDispatcher::VehicleNeedChargeEventHandler(const Vehicle* vehicle) {
  auto vehicle_id = vehicle->GetID();
  // if (vehicle_service_->ReadVehicleIntegrationLevel(vehicle_id) ==
  //         IntegrationLevel::kUtilized &&
  //     vehicle_service_->ReadVehicleNeedCharge(vehicle_id)) {
  //   BOOST_LOG_TRIVIAL(debug) << "Dispatch due to vehicle need charge";
  //   Dispatch();
  // }
}

}  // namespace tcs