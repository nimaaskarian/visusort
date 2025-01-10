#include <array>
#include <clocale>
#include <cstdlib>
#include <ncurses.h>
#include <thread>

enum Color {
  WHITE = 1,
  GREEN,
  YELLOW
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
}

void render_one_item(const std::array<int, SIZE>& data, int max_y, int max_x, size_t i, Color color) {
  int height = data[i];
  int x_pos = i * 2;

  if (x_pos >= max_x) return;

  attron(COLOR_PAIR(color));
  for (int y = 0; y < height && y < max_y; ++y) {
    mvprintw(max_y - y - 1, x_pos, "█");
  }
  attroff(COLOR_PAIR(color));
}

void render_array(const std::array<int, SIZE>& data, int max_y, int max_x, Color color) {
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

class VisualWrapper {
  std::array<int, SIZE> array;
  std::thread * last_thread = nullptr;
  void (*renderer)(const std::array<int, SIZE>&, int, int, size_t, Color);
  void render_thread(size_t i) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    clear_column_to_bottom(max_y, i*2);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    renderer(array, max_y, max_x, i, YELLOW);
    refresh();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
public:
  VisualWrapper(void (*renderer)(const std::array<int, SIZE>&, int, int, size_t, Color), std::array<int,SIZE> &array) : array(array), renderer(renderer) {}
  int operator[](size_t i) const {
    return array[i];
  }

  void join() {
    if (last_thread) {
      last_thread->join();
    }
  }

  auto as_array() const {
    return array;
  }

  int& operator[](size_t i) {
    if (last_thread) {
      last_thread->join();
      delete last_thread;
    }
    last_thread = new std::thread(&VisualWrapper::render_thread, this, i);
    return array[i];
  }
};

void bubble_sort(VisualWrapper &array) {
  for (int i = 0; i < SIZE; i++) {
    bool swapped = false;
    for (int j = 0; j < SIZE-i-1; j++) {
      if (array[j] > array[j+1]) {
        auto tmp = array[j];
        array[j] = array[j+1];
        array[j+1] = tmp;
        swapped = true;
      }
    }
    if (!swapped) break;
  }

}

int main (int argc, char *argv[]) {
  start_ncurses();

  std::array<int, SIZE> _data;
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  srand((unsigned)time(0));
  for (size_t i = 0; i < SIZE; i++) {
    int n;
    n = (rand()%max_y)+1;
    _data[i] = n;
  }
  VisualWrapper array(render_one_item, _data);
  render_array(_data, max_y, max_x, WHITE);
  getch();
  bubble_sort(array);
  array.join();
  clear();
  refresh();
  render_array(array.as_array(), max_y, max_x, GREEN);
  getch();
  endwin();

  return 0;
}
