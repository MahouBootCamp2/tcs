#ifndef MAP_OBJECT_H
#define MAP_OBJECT_H

#include <optional>

namespace tcs {

using MapObjectID = int;
using MapObjectRef = std::optional<int>;
enum class MapObjectType { kPoint, kPath, kLocation, kBlock, kVehicle };

class MapObject {
 public:
  MapObject(MapObjectID id, MapObjectType type) : id_(id), type_(type) {}
  MapObjectID get_id() const { return id_; }
  MapObjectType get_type() const { return type_; }

 private:
  MapObjectID id_;
  MapObjectType type_;
};

}  // namespace tcs

#endif /* MAP_OBJECT_H */
