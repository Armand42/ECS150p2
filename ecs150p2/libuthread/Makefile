CC 	:= gcc
CFLAGS 	:= -Wall -Werror
lib 	:= libuthread.a
objs	:= queue.o uthread.o context.o preempt.o

ifneq ($(V), 1)
Q = @
endif

all: $(lib)

# Rule for all our object files generated from C and header files
%.o: %.c %.h
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c $<

# Rule for making our static library
$(lib): $(objs)
	@echo "AR RCS $@"
	$(Q)ar rcs $@ $^

# Remove our object files and static library
clean:
	@echo "CLEAN"
	$(Q)rm -f $(lib) $(objs)
