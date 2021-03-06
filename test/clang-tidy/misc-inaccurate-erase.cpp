// RUN: $(dirname %s)/check_clang_tidy.sh %s misc-inaccurate-erase %t
// REQUIRES: shell

namespace std {
struct vec_iterator {};

template <typename T> struct vector {
  typedef vec_iterator iterator;

  iterator begin();
  iterator end();

  void erase(iterator);
  void erase(iterator, iterator);
};

template <typename FwIt, typename T>
FwIt remove(FwIt begin, FwIt end, const T &val);

template <typename FwIt, typename Func>
FwIt remove_if(FwIt begin, FwIt end, Func f);

template <typename FwIt> FwIt unique(FwIt begin, FwIt end);
} // namespace std

struct custom_iter {};
struct custom_container {
  void erase(...);
  custom_iter begin();
  custom_iter end();
};

template <typename T> void g() {
  T t;
  t.erase(std::remove(t.begin(), t.end(), 10));
  // CHECK-FIXES: {{^  }}t.erase(std::remove(t.begin(), t.end(), 10));{{$}}

  std::vector<int> v;
  v.erase(remove(v.begin(), v.end(), 10));
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: this call will remove at most one
  // CHECK-FIXES: {{^  }}v.erase(remove(v.begin(), v.end(), 10), v.end());{{$}}
}

#define ERASE(x, y) x.erase(remove(x.begin(), x.end(), y))
// CHECK-FIXES: #define ERASE(x, y) x.erase(remove(x.begin(), x.end(), y))

int main() {
  std::vector<int> v;

  v.erase(remove(v.begin(), v.end(), 10));
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: this call will remove at most one item even when multiple items should be removed [misc-inaccurate-erase]
  // CHECK-FIXES: {{^  }}v.erase(remove(v.begin(), v.end(), 10), v.end());{{$}}
  v.erase(remove(v.begin(), v.end(), 20), v.end());

  // Fix is not trivial.
  auto it = v.end();
  v.erase(remove(v.begin(), it, 10));
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: this call will remove at most one
  // CHECK-FIXES: {{^  }}v.erase(remove(v.begin(), it, 10));{{$}}

  g<std::vector<int>>();
  g<custom_container>();

  ERASE(v, 15);
  // CHECK-MESSAGES: :[[@LINE-1]]:9: warning: this call will remove at most one
  // CHECK-FIXES: {{^  }}ERASE(v, 15);{{$}}
}
