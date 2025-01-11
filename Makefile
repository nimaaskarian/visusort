include config.mk
SRC=.visusort.cpp
OBJ=${SRC:.cpp=.o}

run: visusort
	./visusort

visusort: ${OBJ}
	g++ $< -o $@ ${LDFLAGS}

.visusort.cpp: visusort.cpp .algoignore
	cat visusort.cpp > .visusort.cpp
	echo '	int main (int argc, char *argv[]) {' >> .visusort.cpp
	echo '		start_ncurses();' >> .visusort.cpp
	echo '' >> .visusort.cpp
	echo '		int max_y, max_x;' >> .visusort.cpp
	echo '		getmaxyx(stdscr, max_y, max_x);' >> .visusort.cpp
	echo '    size_t data_size;' >> .visusort.cpp
	echo '    VisualWrapperConfig config{};' >> .visusort.cpp
	echo '    if (const char * str = std::getenv("VISUSORT_WAIT_FOR_CHANGE")) {' >> .visusort.cpp
	echo '        config.wait_for_change_ms = atoi(str);' >> .visusort.cpp
	echo '    }' >> .visusort.cpp
	echo '    if (const char * str = std::getenv("VISUSORT_WAIT_AFTER")) {' >> .visusort.cpp
	echo '        config.wait_after_ms = atoi(str);' >> .visusort.cpp
	echo '    }' >> .visusort.cpp
	echo '    if (const char * sizestr = std::getenv("VISUSORT_SIZE")) {' >> .visusort.cpp
	echo '        data_size = atoi(sizestr);' >> .visusort.cpp
	echo '    } else {' >> .visusort.cpp
	echo '        data_size = max_x/2;' >> .visusort.cpp
	echo '    }' >> .visusort.cpp
	echo '		std::vector<int> _data;' >> .visusort.cpp
	echo '    _data.reserve(data_size);'
	echo '' >> .visusort.cpp
	echo '		srand((unsigned)time(0));' >> .visusort.cpp
	echo '    data_init(_data, data_size, max_y);' >> .visusort.cpp
	echo '    VisualWrapper<std::vector<int>> * array;' >> .visusort.cpp
	grep -o -E '\w+_sort' visusort.cpp | grep -v -f .algoignore | uniq | while read -r line; do \
		num=`echo $$line | wc -c`; \
		num=`expr $$num / 2`; \
		call=`grep -o -E "call: $$line(.*)" visusort.cpp | cut -d ' ' -f "2-"`;\
		echo $$line $$call; \
		[ "$$call" ] || call=$$line"(*array)";\
		call="$$call;";\
		echo '		array = new VisualWrapper<std::vector<int>> (render_one_item, _data, config);' >> .visusort.cpp; \
		echo "		mvprintw(max_y/2, max_x/2-"$$num", \""$$line"\");" >> .visusort.cpp;  \
		echo '		getch();' >> .visusort.cpp; \
		echo '		clear();' >> .visusort.cpp; \
		echo '		render_array(_data, max_y, max_x, WHITE);' >> .visusort.cpp;  \
		echo '		'$$call >> .visusort.cpp; \
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

clean:
	rm .visusort.cpp ${OBJ} visusort
.cpp.o:
	g++ -c ${CFLAGS} $< -o $@
