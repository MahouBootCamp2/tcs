#include "tcs/vehicle/default_controller.h"

namespace tcs {

DefaultController::DefaultController(Vehicle* vehicle, IVehicleAdapter* adapter,
                                     IScheduler* scheduler)
    : vehicle_{vehicle}, adapter_{adapter}, scheduler_{scheduler} {
  adapter_->FinishCommandEvent().Subscribe(std::bind(
      &DefaultController::OnFinishCommandEvent, this, std::placeholders::_1));
  adapter_->UpdatePositionEvent().Subscribe(std::bind(
      &DefaultController::OnUpdatePositionEvent, this, std::placeholders::_1));
  adapter_->RequestChargeEvent().Subscribe(
      std::bind(&DefaultController::OnRequestChargeEvent, this));

  adapter_->Enable();
}

void DefaultController::OnUpdatePositionEvent(MapObjectID point) {
  vehicle_->set_current_point(point);
}

void DefaultController::OnFinishCommandEvent(MovementCommand cmd) {
  if (cmd.operation == kChargeOperation) {
    vehicle_->set_finish_charge(true);
    vehicle_->set_need_charge(false);
  }
}

void DefaultController::OnRequestChargeEvent() {
  vehicle_->set_need_charge(true);
}

bool DefaultController::OnAllocationSuccessful(
    std::unordered_set<MapResource*> resources) {
  if (pending_resources_ != resources) return false;

  if (!pending_command_.has_value()) {
    waiting_for_allocation_ = false;
    pending_resources_.clear();
    return false;
  }

  auto cmd = std::move(pending_command_.value());
  pending_command_.reset();
  pending_resources_.clear();
  allocated_resources_.push_back(resources);
  adapter_->EnqueueCommand(std::move(cmd));
  waiting_for_allocation_ = false;
  if (!waiting_for_allocation_) AllocateForNextCommand();
  return true;
}

void DefaultController::OnAllocationFailed() {
  throw std::runtime_error("Allocation fails");
}

void DefaultController::SetDriveOrder(DriveOrder order) {
  if (current_drive_order_.has_value())
    throw std::runtime_error("vehicle " + std::to_string(vehicle_->get_id()) +
                             " already has an order");
  BOOST_LOG_TRIVIAL(debug) << "vehicle " << vehicle_->get_id()
                           << " setting driveorder";
  scheduler_->Claim(this, ExpandDriveOrder(order));
  current_drive_order_ = order;
  vehicle_->set_route_progress_index(0);
  CreateFutureCommands(order);
  if (!waiting_for_allocation_) {
    AllocateForNextCommand();
  }
}

void DefaultController::AbortDriveOrder(bool immediately) {
  if (immediately) {
    current_drive_order_.reset();
    waiting_for_allocation_ = false;
    pending_resources_.clear();
    vehicle_->set_route_progress_index(0);

    adapter_->ClearCommandQueue();
    command_queue_.clear();
    pending_command_.reset();

    // Free all resources except current position
    auto current_resources = allocated_resources_.front();
    allocated_resources_.pop_front();
    for (auto& resources : allocated_resources_) {
      scheduler_->Free(this, resources);
    }
    allocated_resources_.clear();
    allocated_resources_.push_back(current_resources);

  } else {
    if (current_drive_order_.has_value()) command_queue_.clear();
  }
}

void DefaultController::InitPosition(MapResource* point) {
  scheduler_->AllocateNow(this, {point});
  vehicle_->set_current_point(point->get_id());
}

std::vector<std::unordered_set<MapResource*>>
DefaultController::ExpandDriveOrder(DriveOrder& order) {
  std::vector<std::unordered_set<MapResource*>> res;
  auto& steps = order.get_route()->get_steps();
  for (auto& step : steps) {
    res.push_back({step.destination, step.path});
  }
  return res;
}

void DefaultController::CreateFutureCommands(DriveOrder& order) {
  // TODO: command_vector_.clear() ?

  std::string operation = order.get_destination().operation;
  auto& steps = order.get_route()->get_steps();
  auto sz = steps.size();
  for (auto index = 0; index != sz; ++index) {
    bool is_last_movement = index + 1 == sz;
    command_queue_.push_back({steps[index],
                              is_last_movement ? operation : kNoOperation,
                              is_last_movement});
  }
}

void DefaultController::AllocateForNextCommand() {
  if (pending_command_.has_value())
    throw std::runtime_error("Already has a pending command");
  auto cmd = command_queue_.front();
  command_queue_.pop_front();
  pending_resources_ = ExpandMovementCommand(cmd);
  scheduler_->Allocate(this, pending_resources_);
  waiting_for_allocation_ = true;
  pending_command_ = cmd;
}

std::unordered_set<MapResource*> DefaultController::ExpandMovementCommand(
    MovementCommand& cmd) {
  return {cmd.step.destination, cmd.step.path};
}

}  // namespace tcs