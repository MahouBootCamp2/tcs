#ifndef DEFAULT_SCHEDULER_H
#define DEFAULT_SCHEDULER_H

#include <boost/log/trivial.hpp>
#include <unordered_map>

#include "tcs/scheduler/ischeduler.h"
#include "tcs/scheduler/reservation_pool.h"
#include "tcs/service/map_service.h"
#include "tcs/util/executor.h"
#include "tcs/util/map.h"

namespace tcs {

class DefaultScheduler : public IScheduler {
 public:
  DefaultScheduler(Executor *executor, MapService *map_service)
      : executor_{executor}, map_service_{map_service} {}

  void Claim(IVehicleController *vehicle,
             std::vector<std::unordered_set<const MapResource *>>
                 resource_sequence) override;

  void Unclaim(IVehicleController *vehicle) override;

  void UpdateProgressIndex(IVehicleController *vehicle,
                           std::size_t index) override {
    return;
  }

  void Allocate(IVehicleController *vehicle,
                std::unordered_set<const MapResource *> resources) override;

  // Throw if resources unavailable.
  void AllocateNow(IVehicleController *vehicle,
                   std::unordered_set<const MapResource *> resources) override;

  void Free(IVehicleController *vehicle,
            std::unordered_set<const MapResource *> resources) override;

  void FreeAll(IVehicleController *vehicle) override;

 private:
  void AllocateTask(IVehicleController *vehicle,
                    std::unordered_set<const MapResource *> resources);
  // void ReleaseTask(IVehicleController *vehicle,
  //                  std::unordered_set<MapResource *> resources);
  void CheckTask(IVehicleController *vehicle,
                 std::unordered_set<const MapResource *> resources);
  void RetryTask();

  // Try allocate resources for vehicle. Return successful or not.
  bool TryAllocate(IVehicleController *vehicle,
                   std::unordered_set<const MapResource *> &resources);

  // Expand resources in blocks.
  std::unordered_set<const MapResource *> ExpandBlocks(const
      std::unordered_set<const MapResource *> &resources);

  std::unordered_map<IVehicleController *,
                     std::vector<std::unordered_set<const MapResource *>>>
      claims_by_vehicle_;
  Executor *executor_;
  MapService *map_service_;
  ReservationPool reservation_pool_;
  // List of allocations that are currently not available.
  std::list<
      std::pair<IVehicleController *, std::unordered_set<const MapResource *>>>
      deferred_allocations_;
  std::mutex scheduler_mut_;
};

}  // namespace tcs

#endif /* DEFAULT_SCHEDULER_H */
