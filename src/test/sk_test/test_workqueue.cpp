#include "common/WorkQueue.h"
#include "common/ceph_argparse.h"

void test_workqueue() {
  ThreadPool tp(g_ceph_context, "foo", "tp_foo", 10, "");
  tp.start();
  tp.pause();
  tp.pause_new();
  tp.unpause();
  tp.unpause();
  tp.drain();
  tp.stop();
}

auto main() -> decltype(0) {
  return 0;
}
