CC := g++
LDFLAGS := -lm
TARGETS := main
main_OBJ :=

.PHONY:all
all:$(TARGETS)

.SECONDEXPANSION:

$(TARGETS):$$($$@_OBJ)
	mkdir -p ./build
	cd build && $(CC) ../src/$@.cpp $^ -o $@ $(LDFLAGS)

%.o:src/%.cpp
	mkdir -p ./build
	$(CC) -c $^ -o build/$@


.PHONY:clean
clean:
	cd build && rm -rf $(TARGETS) $(foreach targ,$(TARGETS),$(foreach obj, $($(targ)_OBJ), $(obj)))