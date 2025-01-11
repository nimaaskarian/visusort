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
	echo '    if (const char * sizestr = std::getenv("VISUSORT_SIZE")) {' >> .visusort.cpp
	echo '        data_size = atoi(sizestr);' >> .visusort.cpp
	echo '    } else {' >> .visusort.cpp
	echo '        data_size = max_x/2;' >> .visusort.cpp
	echo '    }' >> .visusort.cpp
	echo '		std::vector<int> _data;' >> .visusort.cpp
	echo '    _data.reserve(data_size);'
	echo '' >> .visusort.cpp
	echo '		srand((unsigned)time(0));' >> .visusort.cpp
	echo '		for (size_t i = 0; i < data_size; i++) {' >> .visusort.cpp
	echo '			_data.push_back((rand()%max_y)+1);' >> .visusort.cpp
	echo '		}' >> .visusort.cpp
	echo '    VisualWrapper<std::vector<int>> * array;' >> .visusort.cpp
	grep -o -E '\w+_sort' visusort.cpp | grep -v -f .algoignore | sort -u | while read -r line; do \
		num=`echo $$line | wc -c`; \
		num=`expr $$num / 2`; \
		call=`grep -o -E "call: $$line(.*)" visusort.cpp | cut -d ' ' -f "2-"`;\
		echo $$line $$call; \
		[ "$$call" ] || call=$$line"(*array)";\
		call="$$call;";\
		echo '		array = new VisualWrapper<std::vector<int>> (render_one_item, _data);' >> .visusort.cpp; \
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

.cpp.o:
	g++ -c ${CFLAGS} $< -o $@
