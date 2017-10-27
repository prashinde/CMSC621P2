EXEC := node
BIN_DIR := bin
MKDIR_P := mkdir -p 

s_INCLUDE_DIRS := nodes utils
s_CXX_SRCS := $(foreach idir, $(s_INCLUDE_DIRS), $(wildcard $(idir)/*.cpp))
s_CXX_OBJS := ${s_CXX_SRCS:.cpp=.o}
s_OBJS := $(s_CXX_OBJS)

cs_LIBRARIES := -lpthread

CPPFLAGS += $(foreach includedir,$(s_INCLUDE_DIRS),-I$(includedir))
CPPFLAGS += $(foreach includedir,$(c_INCLUDE_DIRS),-I$(includedir))
CPPFLAGS += -g $(cs_LIBRARIES)
CC = g++
LDFLAGS += $(foreach librarydir,$(program_LIBRARY_DIRS),-L$(librarydir))
LDFLAGS += $(foreach library,$(cs_LIBRARIES),-$(library))

.PHONY: all clean distclean

all: BIN_DIR $(EXEC)

BIN_DIR: ${BIN_DIR}
${BIN_DIR}:
	@echo $(CPPFLAGS) $(cs_LIBRARIES)
	@echo $(cs_LIBRARIES)
	${MKDIR_P} ${BIN_DIR}

node: $(s_OBJS)
	$(CC) $(s_OBJS) -o $(BIN_DIR)/$@ $(cs_LIBRARIES)

clean:
	${foreach t, ${EXEC},$(RM) $(BIN_DIR)/$(t)}
	$(RM) $(s_OBJS)
	$(RM) -r ${BIN_DIR}

distclean: clean
