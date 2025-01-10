include config.mk
SRC=.visusort.cpp
OBJ=${SRC:.cpp=.o}

visusort: ${OBJ}
	g++ $< -o $@ ${LDFLAGS}

.visusort.cpp: visusort.cpp
	cat visusort.cpp > .visusort.cpp
	echo '	int main (int argc, char *argv[]) {' >> .visusort.cpp
	echo '		start_ncurses();' >> .visusort.cpp
	echo '' >> .visusort.cpp
	echo '		std::vector<int> _data;' >> .visusort.cpp
	echo '		int max_y, max_x;' >> .visusort.cpp
	echo '		getmaxyx(stdscr, max_y, max_x);' >> .visusort.cpp
	echo '' >> .visusort.cpp
	echo '		srand((unsigned)time(0));' >> .visusort.cpp
	echo '		for (size_t i = 0; i < max_x/2; i++) {' >> .visusort.cpp
	echo '			_data.push_back((rand()%max_y)+1);' >> .visusort.cpp
	echo '		}' >> .visusort.cpp
	echo '    VisualWrapper<std::vector<int>> * array;' >> .visusort.cpp
	grep -o -E '\w+_sort' visusort.cpp | sort -u | while read -r line; do \
		echo $$line; \
		num=`echo $$line | wc -c`; \
		num=`expr $$num / 2`; \
		echo '		array = new VisualWrapper<std::vector<int>> (render_one_item, _data);' >> .visusort.cpp; \
		echo "		mvprintw(max_y/2, max_x/2-"$$num", \""$$line"\");" >> .visusort.cpp;  \
		echo '		getch();' >> .visusort.cpp; \
		echo '		clear();' >> .visusort.cpp; \
		echo '		render_array(_data, max_y, max_x, WHITE);' >> .visusort.cpp;  \
		echo '		'$$line"(*array);" >> .visusort.cpp; \
		echo '		array->join();' >> .visusort.cpp; \
		echo '		clear();' >> .visusort.cpp; \
		echo '		refresh();' >> .visusort.cpp; \
		echo '		render_array(array->as_array(), max_y, max_x, GREEN);' >> .visusort.cpp; \
		echo '		getch();' >> .visusort.cpp; \
		echo '		clear();' >> .visusort.cpp; \
	done
	echo '		endwin();' >> .visusort.cpp
	echo '		return 0;' >> .visusort.cpp
	echo '	}' >> .visusort.cpp

.cpp.o:
	g++ -c ${CFLAGS} $< -o $@
