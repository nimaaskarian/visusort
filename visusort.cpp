#include <algorithm>
#include <atomic>
#include <clocale>
#include <cstring>
#include <ncurses.h>
#include <random>
#include <thread>
#include <vector>

enum Color {
  WHITE = 1,
  GREEN,
  YELLOW,
  RED,
  BLUE,
};
constexpr size_t SIZE = 20;
#define DATATYPE = int;
inline void start_ncurses() {
  setlocale(LC_ALL, "");
  initscr();
  cbreak();
  noecho();
  curs_set(0);
  start_color();

  init_pair(WHITE, COLOR_WHITE, COLOR_BLACK);
  init_pair(GREEN, COLOR_GREEN, COLOR_BLACK);
  init_pair(YELLOW, COLOR_YELLOW, COLOR_BLACK);
  init_pair(RED, COLOR_RED, COLOR_BLACK);
  init_pair(BLUE, COLOR_BLUE, COLOR_BLACK);
}

void data_init_random(std::vector<int>& data,size_t size, int max) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distr(1, max);
  for (size_t i = 0; i < size; i++) {
    data.push_back(distr(gen));
  }
}

inline void data_init_ordered(std::vector<int>& data,size_t size, int max, int (*handler)(int, int)) {
  int bin_size = size/max;
  int rest = size%max;
  for (size_t i = 0;i < max; i++) {
    int j;
    if (i%2 && rest > 0) {
      j = -1;
      rest--;
    } else {
      j = 0;
    }
    for (;j < bin_size; j++) {
      data.push_back(handler(i, max));
      if (data.size() >= size) {
        return;
      }
    }
  }
}

int ascending_handler(int i, int max) {
  return i+1;
}

int descending_handler(int i, int max) {
  return max-i;
}

void data_init(std::vector<int>& data,size_t size, int max) {
  if (const char * str = std::getenv("VISUSORT_DATA")) {
    if (!strcmp(str, "ascending")) return data_init_ordered(data, size, max, ascending_handler);
    else if (!strcmp(str, "descending")) return data_init_ordered(data, size, max, descending_handler);
  }
  data_init_random(data, size, max);
}

template <typename T>
void render_one_item(const T& data, int max_y, int max_x, size_t i, Color color) {
  int height = data[i];
  int x_pos = i * 2;

  if (x_pos >= max_x) return;

  attron(COLOR_PAIR(color));
  for (int y = 0; y < height && y < max_y; ++y) {
    mvprintw(max_y - y - 1, x_pos, "█");
  }
  attroff(COLOR_PAIR(color));
}

template <typename T>
void render_array(const T& data, int max_y, int max_x, Color color) {
  for (size_t i = 0; i < data.size(); ++i) {
    render_one_item(data, max_y, max_x, i, color);
  }
  refresh();
}

void clear_column_to_bottom(int max_y, int x) {
  for (int y = 0; y < max_y; ++y) {
    mvaddch(y,x,' ');
  }
}
struct VisualWrapperConfig {
  size_t wait_for_change_ms = 1;
  size_t wait_after_ms = 5;
};

template <typename T>
class VisualWrapper {
  T array;
  VisualWrapperConfig config;
  std::thread * last_thread = nullptr;
  std::atomic_bool should_render;
  void (*renderer)(const T&, int, int, size_t, Color);
  void render_thread(size_t i) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    clear_column_to_bottom(max_y, i*2);
    std::this_thread::sleep_for(std::chrono::milliseconds(config.wait_for_change_ms));
    renderer(array, max_y, max_x, i, YELLOW);
    refresh();
    std::this_thread::sleep_for(std::chrono::milliseconds(config.wait_after_ms));
  }
  void render_all() {
    while (should_render) {
      for (int i = 0; i < size(); i++)
        render_thread(i);
    }
    for (int i = 0; i < size(); i++)
      render_thread(i);
  }

public:
  VisualWrapper(void (*renderer)(const T&, int, int, size_t, Color), T &array, VisualWrapperConfig config) 
  :array(array), renderer(renderer), config(config) {}
  inline void join() {
    if (last_thread) {
      last_thread->join();
      delete last_thread;
      last_thread = nullptr;
    }
  }

  void start_render() {
    join();
    should_render = true;
    last_thread = new std::thread(&VisualWrapper::render_all, this);
  }

  auto begin() {
    return &array[0];
  }

  auto end() {
    return &array[size()];
  }

  void stop_render() {
    should_render = false;
    join();
  }

  const T& as_array() const {
    return array;
  }

  auto size() const {
    return array.size();
  }

  auto& operator[](size_t i) const {
    return array[i];
  }

  auto& operator[](size_t i) {
    join();
    last_thread = new std::thread(&VisualWrapper::render_thread, this, i);
    return array[i];
  }

  void hot_point(size_t i, Color color = RED) {
    join();
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    clear_column_to_bottom(max_y, i*2);
    renderer(array, max_y, max_x, i, color);
  }
};

#define swap(left, right) auto tmp = left; \
  left = right; \
  right = tmp; \

size_t read_envs(VisualWrapperConfig &config, int max_x) {
  if (const char * str = std::getenv("VISUSORT_WAIT_FOR_CHANGE")) {
    config.wait_for_change_ms = atoi(str);
  }
  if (const char * str = std::getenv("VISUSORT_WAIT_AFTER")) {
    config.wait_after_ms = atoi(str);
  }
  if (const char * sizestr = std::getenv("VISUSORT_SIZE")) {
     if (int out=atoi(sizestr)) {
      return out;
    }
  }
  return max_x/2;
}

void bubble_sort(VisualWrapper<std::vector<int>> &array) {
  for (int i = 0; i < array.size(); i++) {
    bool swapped = false;
    for (int j = 0; j < array.size()-i-1; j++) {
      if (array[j] > array[j+1]) {
        swap(array[j], array[j+1]);
        swapped = true;
      }
    }
    array.hot_point(array.size()-i-1, BLUE);
    if (!swapped) break;
  }

}

// call:insertion_sort(*array, 0, array->size()-1)
void insertion_sort(VisualWrapper<std::vector<int>> &array, size_t start, size_t n) {
  for (int i = start+1; i <= n; i++) {
    auto key = array[i];
    auto j = i - 1;
    while (j >= start && array[j] > key) {
      array[j+1] = array[j];
      j--;
    }
    array[j+1] = key;
  }

}
void merge(VisualWrapper<std::vector<int>> &array, size_t low, size_t mid, size_t high) {
  std::vector<int> left(array.as_array().begin() + low, array.as_array().begin() + mid + 1);
  std::vector<int> right(array.as_array().begin() + mid + 1, array.as_array().begin() + high + 1);
  size_t l = 0, r = 0, i = low;
  while (l < left.size() && r < right.size()) {
    if (left[l] <= right[r]) {
      array[i] = left[l];
      l++;
    } else {
      array[i] = right[r];
      r++;
    }
    i++;
  }

  while (l < left.size()) {
    array[i] = left[l];
    i++;
    l++;
  }
  while (r < right.size()) {
    array[i] = right[r];
    i++;
    r++;
  }
}
 // call:merge_sort(*array, 0, array->size()-1)
void merge_sort(VisualWrapper<std::vector<int>> &array, size_t low, size_t high) {
  if (low < high) {
    size_t mid = low+(high-low)/2;
    array.hot_point(high, BLUE);
    merge_sort(array, low, mid);
    merge_sort(array, mid+1, high);
    array.hot_point(mid);
    merge(array, low, mid, high);
  }
}

size_t partition(VisualWrapper<std::vector<int>> &array, size_t low, size_t high) {
  int pivot = array[high];
  array.hot_point(high);
  int i = low - 1;
  for (int j = low; j < high; j++) {
    if (array.as_array()[j] < pivot) {
      i++;
      swap(array[j], array[i]);
    }
  }
  swap(array[i+1], array[high]);
  return i+1;
}

 // call:quick_sort(*array, 0, array->size()-1)
void quick_sort(VisualWrapper<std::vector<int>> &array, int low, int high) {
  if (low < high) {
    size_t p = partition(array, low, high);
    array.hot_point(p, BLUE);
    quick_sort(array, low, p-1);
    quick_sort(array, p+1, high);
  }
}

void move_mo3_to_high(VisualWrapper<std::vector<int>> &array, int low, int high) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distr(low, high);
  size_t i1 = distr(gen);
  size_t i2 = distr(gen);
  size_t i3 = distr(gen);
  size_t median;
  if (array[i1] > array[i2] == array[i2] < array[i3]) {
    median = i2;
  } else if (array[i2] > array[i1] == array[i1] < array[i3]) {
    median = i1;
  } else {
    median = i3;
  }
  swap(array[median], array[high]);
}

 // call:mo3_quick_sort(*array, 0, array->size()-1)
void mo3_quick_sort(VisualWrapper<std::vector<int>> &array, int low, int high) {
  if (low < high) {
    move_mo3_to_high(array, low, high);
    size_t p = partition(array, low, high);
    array.hot_point(p, BLUE);
    quick_sort(array, low, p-1);
    quick_sort(array, p+1, high);
  }
}

void commie_sort(VisualWrapper<std::vector<int>> &array) {
  int sum = 0;
  for (auto &item:array.as_array()) {
    sum+=item;
  }
  int mean = sum/array.size();
  for (int i = 0; i < array.size();i++) {
    array[i] = mean;
  }
}

bool is_sorted(const std::vector<int> &array) {
  int n = array.size();
  while (--n > 0)
    if (array[n] < array[n - 1])
        return false;
  return true;
}

void shuffle(VisualWrapper<std::vector<int>> &array) {
  size_t n = array.size();
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distr(0, n-1);
  auto touched = new bool[array.size()]{};

  for (int i = 0; i < n; i++) {
    if (touched[i]) continue;
    auto j = distr(gen);
    touched[i] = true;
    touched[j] = true;
    swap(array[i], array[j]);
  }
}

void bogo_sort(VisualWrapper<std::vector<int>> &array) {
  while (!is_sorted(array.as_array()))
    shuffle(array);
}

void max_heapify(VisualWrapper<std::vector<int>> &array, size_t i, size_t n) {
  size_t left = 2*i + 1;
  size_t right = 2*i + 2;
  size_t largest;

  if (left < n && array[left] > array[i]) largest = left;
  else largest = i;

  if (right < n && array[right] > array[largest]) largest = right;

  if (largest != i) {
    swap(array[i], array[largest])
    max_heapify(array, largest, n);
  }
}

void build_max_heap(VisualWrapper<std::vector<int>> &array) {
  auto n = array.size();
  for (int i = array.size()/2-1; i >= 0; i--)
    max_heapify(array, i, n);
}

void heap_sort(VisualWrapper<std::vector<int>> &array) {
  build_max_heap(array);
  for (int i = array.size()-1; i > 0; i--) {
    swap(array[i], array[0]);
    max_heapify(array, 0, i);
  }
}

// call:immersion_sort(*array, 0, array->size()-1, 10)
void immersion_sort(VisualWrapper<std::vector<int>> &array, int low, int high, size_t insertion_count) {
  if (high - low > insertion_count) {
    size_t mid = low+(high-low)/2;
    immersion_sort(array, low, mid, insertion_count);
    immersion_sort(array, mid+1, high, insertion_count);
    array.hot_point(mid, RED);
    merge(array, low, mid, high);
  } else {
    array.hot_point(high, BLUE);
    insertion_sort(array, low, high);
  }
}

void you_sort(VisualWrapper<std::vector<int>> &array) {
  int selected = 0;
  while (int ch = getch()) {
    array.hot_point(selected, WHITE);
    switch (ch) {
      case 'j':
      case 'l':
        selected=(selected+1)%array.size();
      break;
      case 'h':
      case 'k':
        if (selected) selected--;
        else selected=array.size()-1;
      break;
      case 'g':
      selected = 0;
      break;
      case 'G':
      selected = array.size()-1;
      break;
      case 'H':
      case 'K':
        if (selected-1 >= 0) {
          swap(array[selected], array[selected-1]);
          selected--;
        }
      break;
      case 'J':
      case 'L':
        if (selected+1 < array.size()) {
          swap(array[selected], array[selected+1]);
          selected++;
        }
      break;
      case 'q':
        quick_sort(array, 0, array.size()-1);
      break;
      case 'Q':
        mo3_quick_sort(array, 0, array.size()-1);
      break;
      case 'm':
        merge_sort(array, 0, array.size()-1);
      break;
      case 'b':
        bubble_sort(array);
      break;
      case 'i':
        insertion_sort(array, 0, array.size()-1);
      break;
      case 'I':
        immersion_sort(array, 0, array.size()-1, 10);
      break;
      case 'e':
        heap_sort(array);
      break;
      case 's':
        shuffle(array);
      break;
      case 'S':
        array.start_render();
        std::sort(array.begin(), array.end());
        array.stop_render();
      break;
      case '\n':
        {
          int max_y, max_x;
          getmaxyx(stdscr, max_y, max_x);
          bool sorted = is_sorted(array.as_array());
          if (sorted) {
            render_array(array,max_y, max_x, GREEN);
            return;
          } 
          render_array(array,max_y, max_x, RED);
          std::this_thread::sleep_for(std::chrono::milliseconds(500));
          render_array(array,max_y, max_x, WHITE);
        }
      break;
    }
    array.hot_point(selected);
  }
}
