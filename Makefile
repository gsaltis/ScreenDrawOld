CC					= gcc
LINK					= gcc

CC_FLAGS				= -c -g -Wall -DANSI_COLORS_SUPPORTED
LINK_FLAGS				= -g 
LIB_FLAGS				= -LGeneralUtilities -LRPiBaseModules

TARGET					= screendraw
LIBS					= -lpthread -lm -lutils -llinenoise -lmongoose -ljson -luuid
OBJS					= $(sort				\
					    main.o                              \
					    UserInputThread.o			\
					    JSONIF.o				\
					    JSONOut.o				\
					    HTTPServer.o			\
					    Screen.o				\
					    ScreenElement.o			\
					    ScreenElementValue.o		\
					    ScreenElementValueSet.o		\
					    ScreenElementTemplate.o		\
					    WebConnection.o			\
					    WebSocketServer.o			\
					    Version.o				\
					   )

%.o					: %.c
					  @echo [CC] $@
					  @$(CC) $(CC_FLAGS) $<

.PHONY					: all
all					: $(TARGET)

include					  depends.mk

$(TARGET)				: $(OBJS)
					  @echo [LD] $@
					  @$(LINK) $(LINK_FLAGS) $(LIB_FLAGS) -o $(TARGET) $(OBJS) $(LIBS)

.PHONY					: junkclean
junkclean				:
					  rm -rf $(wildcard *~ *.bak)

.PHONY					: clean
clean					: junkclean
					  rm -rf $(wildcard $(TARGET) $(OBJS))
