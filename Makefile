all:
	@cc -std=c89 -Wpedantic -Wall -Wextra -Iinclude \
        src/modeline.c src/modeline_test.c \
        -o bin/modeline_test -lm

clean:
	@rm -f bin/modeline_test

style:
	@clang-format-21 -i -style=file:clang_format \
                     include/modeline.h
	@clang-format-21 -i -style=file:clang_format \
                     src/modeline.c
	@clang-format-21 -i -style=file:clang_format \
                     src/modeline_test.c

lint:
	@echo Testing...
	@echo " gcc in C mode:"

	@gcc -std=c89 -Wpedantic -Wall -Wextra -Iinclude \
         src/modeline.c src/modeline_test.c \
         -o bin/modeline_test -lm
	@rm -f bin/modeline_test

	@echo " clang in C mode:"
	@clang -std=c89 -Wpedantic -Wall -Wextra -Iinclude \
           src/modeline.c src/modeline_test.c \
           -o bin/modeline_test -lm
	@rm -f bin/modeline_test

	@echo " gcc in C++ mode:"
	@g++ -Wpedantic -Wall -Wextra -Iinclude \
         src/modeline.c src/modeline_test.c \
         -o bin/modeline_test -lm
	@rm -f bin/modeline_test

	@echo " clang in C++ mode:"
	@clang++ -Wpedantic -Wall -Wextra -Iinclude \
             src/modeline.c src/modeline_test.c \
             -o bin/modeline_test -lm
	@rm -f bin/modeline_test

	@echo " cppcheck (C89): "
	@cppcheck --enable=all --suppress=missingIncludeSystem \
              --inconclusive --check-config --std=c89 \
              src/modeline.c src/modeline_test.c  \
              include/modeline.h
	@echo " cppcheck (C99): "
	@cppcheck --enable=all --suppress=missingIncludeSystem \
              --inconclusive --check-config --std=c99 \
              src/modeline.c src/modeline_test.c  \
              include/modeline.h
	@echo " cppcheck (C++11): "
	@cppcheck --enable=all --suppress=missingIncludeSystem \
              --inconclusive --check-config --std=c++11 \
              src/modeline.c src/modeline_test.c  \
              include/modeline.h
