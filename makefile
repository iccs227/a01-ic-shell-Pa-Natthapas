CC=g++
FILES=icsh.cpp jobs.cpp signal.cpp i_o_redirect.cpp spawn_processes.cpp
BINARY=icsh

all: icsh

icsh: $(FILES)
	$(CC) -o $(BINARY) $(FILES)

# .PHONY: clean

# clean:
# 	rm -f $(BINARY)

