#include <csignal>
#include <pqrs/osx/iokit_power_management.hpp>

namespace {
auto global_wait = pqrs::make_thread_wait();
}

int main(void) {
  std::signal(SIGINT, [](int) {
    global_wait->notify();
  });

  auto time_source = std::make_shared<pqrs::dispatcher::hardware_time_source>();
  auto dispatcher = std::make_shared<pqrs::dispatcher::dispatcher>(time_source);
  auto run_loop_thread = std::make_shared<pqrs::cf::run_loop_thread>();

#if 0
  {
    auto r = pqrs::osx::iokit_power_management::sleep();
    std::cout << "sleep " << r.to_string() << std::endl;
  }
#endif

  auto monitor = std::make_unique<pqrs::osx::iokit_power_management::monitor>(dispatcher,
                                                                              run_loop_thread);

  monitor->system_will_sleep.connect([](auto&& kernel_port,
                                        auto&& notification_id,
                                        auto&& wait) {
    std::cout << "system_will_sleep" << std::endl;

    // std::this_thread::sleep_for(std::chrono::milliseconds(3000));

    IOAllowPowerChange(kernel_port, notification_id);

    wait->notify();
  });

  monitor->system_will_power_on.connect([] {
    std::cout << "system_will_power_on" << std::endl;
  });

  monitor->system_has_powered_on.connect([] {
    std::cout << "system_has_powered_on" << std::endl;
  });

  monitor->can_system_sleep.connect([](auto&& kernel_port,
                                       auto&& notification_id,
                                       auto&& wait) {
    std::cout << "can_system_sleep" << std::endl;

    IOAllowPowerChange(kernel_port, notification_id);
    // IOCancelPowerChange(kernel_port, notification_id);

    wait->notify();
  });

  monitor->system_will_not_sleep.connect([] {
    std::cout << "system_will_not_sleep" << std::endl;
  });

  monitor->error_occurred.connect([](auto&& message) {
    std::cout << message << std::endl;
  });

  monitor->async_start();

  global_wait->wait_notice();

  monitor = nullptr;

  run_loop_thread->terminate();
  run_loop_thread = nullptr;

  dispatcher->terminate();
  dispatcher = nullptr;

  std::cout << "finished" << std::endl;

  return 0;
}
