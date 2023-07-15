CC := g++
LDFLAGS := -lm
TARGETS := main
main_OBJ := lib/utils.o lib/table.o lib/sicxe.o
utils_OBJ := lib/table.o
table_OBJ := lib/utils.o
sicxe_OBJ := lib/utils.o lib/table.o

.PHONY:all
all:$(TARGETS)

.SECONDEXPANSION:

$(TARGETS):$$($$@_OBJ)
	$(CC) $@.cpp $^ -o $@ $(LDFLAGS)

%.o:src/%.cpp
	$(CC) -c $^ -o $@


.PHONY:clean
clean:
	rm -rf $(TARGETS) $(foreach targ,$(TARGETS),$(foreach obj, $($(targ)_OBJ), $(obj)))