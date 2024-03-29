#include "tcs/util/order_pool.h"

namespace tcs {

TransportOrder* OrderPool::GetOrder(TransportOrderID id) const {
  // std::scoped_lock<std::recursive_mutex> lk{mut_};
  auto itr = order_pool_.find(id);
  if (itr == order_pool_.end())
    return nullptr;
  else
    return itr->second.get();
}

TransportOrderID OrderPool::AddOrder(
    std::vector<Destination> destinations,
    std::unordered_set<TransportOrderID> dependencies) {
  // Check validity
  for (auto& dst : destinations) {
    auto site = map_->GetResource(dst.site);
    if (!site || site->GetType() == MapObjectType::kPath)
      throw std::invalid_argument("Destination invalid");
    if (site->GetType() == MapObjectType::kPoint &&
        dst.operation != kNoOperation)
      throw std::invalid_argument("Operation at points");
    if (site->GetType() == MapObjectType::kLocation &&
        map_->GetLocation(dst.site)->GetOperations().find(dst.operation) ==
            map_->GetLocation(dst.site)->GetOperations().end())
      throw std::invalid_argument("Operation not supported");
  }

  for (auto& transport_order : dependencies) {
    if (!GetOrder(transport_order))
      throw std::invalid_argument("Dependency invalid");
  }

  // std::scoped_lock<std::recursive_mutex> lk{mut_};
  auto ptr =
      std::make_unique<TransportOrder>(next_id_, destinations, dependencies);
  order_pool_.insert({next_id_, std::move(ptr)});
  auto tmp = next_id_;
  next_id_ += 1;
  return tmp;
}

std::unordered_set<TransportOrder*> OrderPool::GetAllOrders() const {
  std::unordered_set<TransportOrder*> result;
  for (auto& pair : order_pool_) {
    result.insert(pair.second.get());
  }
  return result;
}

std::unordered_set<const TransportOrder*> OrderPool::GetOrdersByID(
    std::unordered_set<TransportOrderID>& id_set) const {
  std::unordered_set<const TransportOrder*> result;
  for (auto& id : id_set) {
    result.insert(GetOrder(id));
  }
  return result;
}

}  // namespace tcs