#include <atomic>
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

int main() {
  using namespace boost::ut;
  using namespace boost::ut::literals;
  using namespace std::chrono_literals;

  "pending_power_response allows exactly once before callback starts"_test = [] {
    int allow_count = 0;

    pqrs::osx::iokit_power_management::detail::pending_power_response response(
        123,
        456,
        [&](auto kernel_port, auto notification_id) {
          ++allow_count;
          expect(kernel_port == 123);
          expect(notification_id == 456);
        });

    response.allow_power_change_if_callback_not_started();

    expect(allow_count == 1_i);
    expect(!response.get_wait()->wait_notice_for(0ms));
    response.notify_wait();
    expect(response.get_wait()->wait_notice_for(0ms));
    expect(!response.try_start_callback());

    response.allow_power_change_if_callback_not_started();
    expect(allow_count == 1_i);
  };

  "pending_power_response does not allow after callback starts"_test = [] {
    int allow_count = 0;

    pqrs::osx::iokit_power_management::detail::pending_power_response response(
        123,
        456,
        [&](auto, auto) {
          ++allow_count;
        });

    expect(!response.get_wait()->wait_notice_for(1ms));
    expect(response.try_start_callback());

    response.allow_power_change_if_callback_not_started();

    expect(allow_count == 0_i);
    expect(!response.get_wait()->wait_notice_for(0ms));
    response.notify_wait();
    expect(response.get_wait()->wait_notice_for(0ms));
  };

  "pending_power_response allows when queued dispatcher callback is detached before running"_test = [] {
    auto time_source = std::make_shared<pqrs::dispatcher::hardware_time_source>();
    auto dispatcher = std::make_shared<pqrs::dispatcher::dispatcher>(time_source);

    pqrs::dispatcher::extra::dispatcher_client blocker(dispatcher);
    pqrs::dispatcher::extra::dispatcher_client target(dispatcher);

    auto blocker_started = pqrs::make_thread_wait();
    auto release_blocker = pqrs::make_thread_wait();

    expect(blocker.enqueue_to_dispatcher([blocker_started, release_blocker] {
      blocker_started->notify();
      release_blocker->wait_notice();
    }));

    blocker_started->wait_notice();

    std::atomic<int> allow_count = 0;
    std::atomic<bool> callback_ran = false;

    auto response = std::make_shared<pqrs::osx::iokit_power_management::detail::pending_power_response>(
        123,
        456,
        [&](auto kernel_port, auto notification_id) {
          ++allow_count;
          expect(kernel_port == 123);
          expect(notification_id == 456);
        });
    auto wait = response->get_wait();

    expect(target.enqueue_to_dispatcher([response, wait, &callback_ran] {
      callback_ran = true;

      if (response->try_start_callback()) {
        wait->notify();
      }
    }));

    target.detach_from_dispatcher();
    response->allow_power_change_if_callback_not_started();
    response->notify_wait();

    release_blocker->notify();
    blocker.detach_from_dispatcher();

    expect(allow_count.load() == 1_i);
    expect(!callback_ran.load());
    expect(wait->wait_notice_for(0ms));

    dispatcher->terminate();
    dispatcher = nullptr;
  };

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

  "iokit_power_management::monitor can be destroyed after queued async operations"_test = [] {
    auto [time_source, dispatcher, run_loop_thread] = make_components();

    auto monitor = std::make_unique<pqrs::osx::iokit_power_management::monitor>(dispatcher,
                                                                                run_loop_thread);

    monitor->async_start();
    monitor->async_stop();
    monitor->async_start();
    monitor = nullptr;

    run_loop_thread->terminate();
    run_loop_thread = nullptr;

    dispatcher->terminate();
    dispatcher = nullptr;
  };

  "iokit_power_management::monitor can be destroyed from run_loop_thread"_test = [] {
    auto [time_source, dispatcher, run_loop_thread] = make_components();

    auto monitor = std::make_shared<std::shared_ptr<pqrs::osx::iokit_power_management::monitor>>(
        std::make_shared<pqrs::osx::iokit_power_management::monitor>(dispatcher,
                                                                     run_loop_thread));

    (*monitor)->async_start();

    auto wait = pqrs::make_thread_wait();
    run_loop_thread->enqueue(^{
      *monitor = nullptr;
      wait->notify();
    });
    wait->wait_notice();

    run_loop_thread->terminate();
    run_loop_thread = nullptr;

    dispatcher->terminate();
    dispatcher = nullptr;
  };

  return 0;
}
