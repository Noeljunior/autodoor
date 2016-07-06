CFLAGS   = #-fstack-protector-all -Wall -g
LIBS     = -lm -lncurses
OPTS     = -march=native -mtune=native -m64 -O3 -funroll-loops

SRC      = src/
OBJ      = obj/
OUT      = autodoor

# list of object that will be compiling with optimazion flags
OPT_OBJS =
# and the auto-list of the remaining objects
OBJLSC   = $(patsubst %.c,%.o,$(subst $(SRC),,$(shell find ${SRC} -type f -name '*.c')))
OBJLSCPP = $(patsubst %.cpp,%.o,$(subst $(SRC),,$(shell find ${SRC} -type f -name '*.cpp')))
OBJALL   = ${OBJLSC} ${OBJLSCPP}
OBJS     = $(filter-out $(OPT_OBJS),$(patsubst %.c,%.o,$(subst $(SRC),,${OBJALL})))


$(OUT): $(addprefix ${OBJ},$(OBJS)) $(addprefix ${OBJ},$(OPT_OBJS))
	g++ $(LIBS) $(shell find ${OBJ} -type f -name '*.o') -o $@


$(addprefix ${OBJ},$(OPT_OBJS)): OPT_OBJS_FLAGS := $(OPTS)
${OBJ}%.o: $(SRC)%.c
	@mkdir -p "$(@D)"
	@echo "this:" $<
	gcc $(CFLAGS) $(OPT_OBJS_FLAGS) -o $@ -c $< -DPOSIX

${OBJ}%.o: $(SRC)%.cpp
	@mkdir -p "$(@D)"
	g++ $(CFLAGS) -o $@ -c $< -DPOSIX


clean:
	rm -f ${OUT}
	rm -fr ${OBJ}

run: $(OUT)
	./$(OUT)


force: clean ${OUT}

