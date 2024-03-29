#include "sim_vehicle_adapter_test.h"
#include "tcs/kernel.h"
#include "tcs/util/map_builder.h"

tcs::Map* BuildTestMap() {
  auto builder = tcs::MapBuilder::GetInstance();
  builder.Init();

  auto vs = builder.AddPoint({0, 0, 0});  // 0
  auto vt = builder.AddPoint({1, 1, 0});  // 1
  auto vx = builder.AddPoint({2, 1, 0});  // 2
  auto vy = builder.AddPoint({1, 0, 0});  // 3
  auto vz = builder.AddPoint({2, 0, 0});  // 4

  auto est = builder.AddPath(vs, vt, 1000);  // 5
  auto esy = builder.AddPath(vs, vy, 500);   // 6
  auto ety = builder.AddPath(vt, vy, 200);   // 7
  auto etx = builder.AddPath(vt, vx, 100);   // 8
  auto exz = builder.AddPath(vx, vz, 400);   // 9
  auto eyt = builder.AddPath(vy, vt, 300);   // 10
  auto eyx = builder.AddPath(vy, vx, 900);   // 11
  auto eyz = builder.AddPath(vy, vz, 200);   // 12
  auto ezs = builder.AddPath(vz, vs, 700);   // 13
  auto ezx = builder.AddPath(vz, vx, 600);   // 14

  auto locationA = builder.AddLocation({tcs::kLoadOperation}, {vs});    // 15
  auto locationB = builder.AddLocation({tcs::kUnloadOperation}, {vz});  // 16
  auto locationC = builder.AddLocation({tcs::kParkOperation}, {vx});    // 17

  auto vehicleA = builder.AddVehicle();  // 18
  auto vehicleB = builder.AddVehicle();  // 19

  return builder.Build();
}

int main(int, char**) {
  auto map = BuildTestMap();
  std::recursive_mutex global_mutex;

  std::unique_ptr<tcs::IKernel> kernel =
      std::make_unique<tcs::Kernel>(global_mutex, map);

  kernel->EnableVehicle(18, 2);
  kernel->EnableVehicle(19, 3);
  kernel->Start();
  kernel->AddTransportOrder({{15, tcs::kLoadOperation},
                             {16, tcs::kUnloadOperation},
                             {17, tcs::kParkOperation}});
  kernel->AddTransportOrder({{15, tcs::kLoadOperation},
                             {16, tcs::kUnloadOperation},
                             {17, tcs::kParkOperation}});
  // // std::this_thread::sleep_for(std::chrono::seconds(50000000));
  kernel->Exit();

  return 0;
}