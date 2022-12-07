CC := g++
LDFLAGS := -lm
TARGETS := main
main_OBJ := utils.o table.o

.PHONY:all
all:$(TARGETS)

.SECONDEXPANSION:

$(TARGETS):$$($$@_OBJ)
	$(CC) $@.cpp $^ -o $@ $(LDFLAGS)

%.o:%.cpp
	$(CC) -c $^ -o $@


.PHONY:clean
clean:
	rm -rf $(TARGETS) $(foreach targ,$(TARGETS),$(foreach obj, $($(targ)_OBJ), $(obj)))