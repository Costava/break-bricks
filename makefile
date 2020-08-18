# makefile

CC:=clang
SRCDIR:=./src
EXTDIR:=./external
OBJDIR:=./obj
OPTIMIZATION_FLAG:=-O0

# Additional places to find C header files
ALSO_INCLUDE:=-I$(SRCDIR) -I$(EXTDIR)

CFLAGS:=-Wall -fopenmp $(OPTIMIZATION_FLAG) $(ALSO_INCLUDE)

# Recipe for building what will be a dependency of the main executable
BUILD_DEP=$(CC) $^ -c --output $@ $(CFLAGS)

################################################################################

# Remove the line for `external` if something is ever put in `external`
init:
	mkdir -p obj
	mkdir -p screenshots
	mkdir -p external

run: build
	./main.bin

runfresh: clean run

build: main.bin

buildfresh: clean build

clean:
	rm -f $(OBJDIR)/*.o
	rm -f main.bin

# `-lm` was added after needing `round` function in <math.h>
#  in order to avoid a compilation error
main.bin: ./main/main.c \
	$(OBJDIR)/charu.o \
	$(OBJDIR)/easy_alloc.o \
	$(OBJDIR)/game.o \
	$(OBJDIR)/nsec.o \
	$(OBJDIR)/rand.o \
	$(OBJDIR)/sdlu.o
	$(CC) $^ --output $@ -g -lm $(CFLAGS) -lSDL2 -lSDL2_image

################################################################################

$(OBJDIR)/charu.o: $(SRCDIR)/charu.c
	$(BUILD_DEP)

$(OBJDIR)/easy_alloc.o: $(SRCDIR)/easy_alloc.c
	$(BUILD_DEP)

$(OBJDIR)/game.o: $(SRCDIR)/game.c
	$(BUILD_DEP)

$(OBJDIR)/nsec.o: $(SRCDIR)/nsec.c
	$(BUILD_DEP)

$(OBJDIR)/rand.o: $(SRCDIR)/rand.c
	$(BUILD_DEP)

$(OBJDIR)/sdlu.o: $(SRCDIR)/sdlu.c
	$(BUILD_DEP)
