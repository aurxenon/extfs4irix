CC = gcc
LD = gcc

CWD = $(shell /bin/pwd)
EXT2_OUT = $(CWD)/bin
EXT2_OBJDIR = $(CWD)/obj

EXT2_OBJ =  $(EXT2_OBJDIR)/ext2fs.o
EXT2_OBJ += $(EXT2_OBJDIR)/main.o
EXT2_TARGET = $(EXT2_OUT)/ext2impl

EXT2_CFLAGS = $(CFLAGS) -g -fPIE
EXT2_LDFLAGS = $(LDFLAGS) -g

all : clean dir EXT2Implementation

clean:
	@echo "[ CLEAN ]"
	@rm -rf $(EXT2_OBJDIR)
	@rm -rf $(EXT2_OUT)

dir:
	@mkdir $(EXT2_OBJDIR)
	@mkdir $(EXT2_OUT)

EXT2Implementation : $(EXT2_OBJ) $(EXT2_TARGET)

$(EXT2_OBJDIR)/%.o : $(CWD)/%.c
	@echo "[ CC ]    $(shell realpath --relative-to=$(CWD) $<)"
	@${CC} ${EXT2_CFLAGS} -o $@ -c $<

$(EXT2_TARGET) :
	@echo "[ LD ]    $(shell realpath --relative-to=$(CWD) $@)"
	@$(LD) $(EXT2_LDFLAGS) -o $@ $(EXT2_OBJ)