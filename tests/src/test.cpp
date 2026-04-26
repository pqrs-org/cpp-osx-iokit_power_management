#include <boost/ut.hpp>
#include <pqrs/osx/iokit_power_management.hpp>

namespace {
auto make_components() {
  auto time_source = std::make_shared<pqrs::dispatcher::hardware_time_source>();
  auto dispatcher = std::make_shared<pqrs::dispatcher::dispatcher>(time_source);
  auto run_loop_thread = std::make_shared<pqrs::cf::run_loop_thread>();

  return std::tuple{time_source, dispatcher, run_loop_thread};
}
} // namespace

int main(void) {
  using namespace boost::ut;
  using namespace boost::ut::literals;

  "iokit_power_management::monitor"_test = [] {
    auto [time_source, dispatcher, run_loop_thread] = make_components();

    auto monitor = std::make_unique<pqrs::osx::iokit_power_management::monitor>(dispatcher,
                                                                                run_loop_thread);

    monitor = nullptr;

    run_loop_thread->terminate();
    run_loop_thread = nullptr;

    dispatcher->terminate();
    dispatcher = nullptr;
  };

  "iokit_power_management::monitor can be destroyed after run_loop_thread termination"_test = [] {
    auto [time_source, dispatcher, run_loop_thread] = make_components();

    auto monitor = std::make_unique<pqrs::osx::iokit_power_management::monitor>(dispatcher,
                                                                                run_loop_thread);

    run_loop_thread->terminate();
    run_loop_thread = nullptr;

    monitor = nullptr;

    dispatcher->terminate();
    dispatcher = nullptr;
  };

  "iokit_power_management::monitor can be destroyed immediately after async_start"_test = [] {
    auto [time_source, dispatcher, run_loop_thread] = make_components();

    auto monitor = std::make_unique<pqrs::osx::iokit_power_management::monitor>(dispatcher,
                                                                                run_loop_thread);

    monitor->async_start();
    monitor = nullptr;

    run_loop_thread->terminate();
    run_loop_thread = nullptr;

    dispatcher->terminate();
    dispatcher = nullptr;
  };

  "iokit_power_management::monitor accepts async_start and async_stop back to back"_test = [] {
    auto [time_source, dispatcher, run_loop_thread] = make_components();

    auto monitor = std::make_unique<pqrs::osx::iokit_power_management::monitor>(dispatcher,
                                                                                run_loop_thread);

    monitor->async_start();
    monitor->async_stop();
    monitor = nullptr;

    run_loop_thread->terminate();
    run_loop_thread = nullptr;

    dispatcher->terminate();
    dispatcher = nullptr;
  };

  return 0;
}
