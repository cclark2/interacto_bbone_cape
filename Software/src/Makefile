CROSS_COMPILE=

LIBDIR_APP_LOADER=
INCDIR_APP_LOADER=./
BINDIR=../bin

#CFLAGS+= -Wall -I$(INCDIR_APP_LOADER) -D__DEBUG -O2 -mtune=cortex-a8 -march=armv7-a
CFLAGS+= -I$(INCDIR_APP_LOADER)
LDFLAGS+= -lpthread -lopencv_core -lopencv_highgui -lopencv_imgproc
OBJDIR=obj
TARGET=$(BINDIR)/all_capes

_DEPS = 
DEPS = $(patsubst %,$(INCDIR_APP_LOADER)/%,$(_DEPS))

_OBJ = main.o prussdrv.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

$(OBJDIR)/%.o: %.c $(DEPS)
	@mkdir -p obj
	$(CROSS_COMPILE)gcc $(CFLAGS) -c -o $@ $< 
  
$(OBJDIR)/%.o: %.cpp $(DEPS)
	@mkdir -p obj
	$(CROSS_COMPILE)g++ $(CFLAGS) -c -o $@ $< 

$(TARGET): $(OBJ)
	$(CROSS_COMPILE)g++ $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -rf $(OBJDIR)/ *~  $(INCDIR_APP_LOADER)/*~  $(TARGET)
