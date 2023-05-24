CC=gcc
STANDARD= -std=c11
CFLAGS= -g -Wall -Wextra -pedantic -Wuninitialized -Wunused-variable -fsanitize=address -fsanitize=undefined -fsanitize=leak # -Werror $(STANDARD)
COMMON_OBJS = util.o serverutils.o logger.o
PROGRAMS = tcps

all: $(COMMON_OBJS) $(PROGRAMS)

tcps: tcpserver.c $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o tcps tcpserver.c $(COMMON_OBJS)

clean:
	rm -rf *.dSYM $(PROGRAMS) *.o

### ABOUT THE FLAGS ###
# -g					debug information
# -Wall					enables a set of commonly used warnings. It includes warnings about common programming issues and potential bugs.
# -Wextra				enables additional warnings beyond those enabled by -Wall. It covers a wider range of potential issues and encourages stricter coding practices.
# -pedantic				enables strict ISO C and ISO C++ conformance. It provides warnings about language features that are non-standard or prone to portability issues.
# -Werror				treats warnings as errors. It forces the compilation to fail if any warnings are encountered, making it useful for enforcing clean code.
# -Wuninitialized 		warns about the use of uninitialized variables
# -Wunused-variable		warns about unused variables
# -fsanitize=address	Enables the AddressSanitizer, which helps detect memory errors such as buffer overflows, use-after-free, and memory leaks.
# -fsanitize=undefined	Enables the UndefinedBehaviorSanitizer, which helps detect undefined behavior in your code.
# -fsanitize=leak		Enables the LeakSanitizer, which helps detect memory leaks.