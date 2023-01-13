#include <boost/ut.hpp>
#include <pqrs/osx/iokit_power_management.hpp>

int main(void) {
  using namespace boost::ut;
  using namespace boost::ut::literals;

  "iokit_power_management::monitor"_test = [] {
    auto time_source = std::make_shared<pqrs::dispatcher::hardware_time_source>();
    auto dispatcher = std::make_shared<pqrs::dispatcher::dispatcher>(time_source);
    auto run_loop_thread = std::make_shared<pqrs::cf::run_loop_thread>();

    auto monitor = std::make_unique<pqrs::osx::iokit_power_management::monitor>(dispatcher,
                                                                                run_loop_thread);

    monitor = nullptr;

    run_loop_thread->terminate();
    run_loop_thread = nullptr;

    dispatcher->terminate();
    dispatcher = nullptr;
  };

  return 0;
}
