#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

DEVKITPPC_LOCAL		:=	$(DEVKITPPC)/../devkitPPC_blueMSXWii
LIBOGC_LOCAL        :=  $(DEVKITPPC)/../libogc_blueMSXWii
LIBOGC_INC_LOCAL	:=	$(LIBOGC_LOCAL)/include
LIBOGC_LIB_LOCAL	:=	$(LIBOGC_LOCAL)/lib/wii

# include from origional devkitPPC but for the rest, use our own!
PATH_BACKUP := $(PATH)

# Stupid tricks needed to include the local copy of the rules
ifneq ($(findstring wii_rules,$(wildcard ../lib/wii_rules)), )
include ../lib/wii_rules
else
include lib/wii_rules
endif

export PATH := $(DEVKITPPC_LOCAL)/bin:$(PATH_BACKUP)


#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
TARGET		:=	boot
BUILD		:=	build
SOURCES		:=	source/Bios \
              source/Board \
              source/Debugger \
              source/Emulator \
              source/Expat \
              source/Gui \
              source/Input \
              source/IoDevice \
              source/Language \
              source/Libpng \
              source/Media \
              source/Memory \
              source/Resource \
              source/SoundChips \
              source/TinyXML \
              source/Tools/Trainer \
              source/Unzip \
              source/Utils \
              source/VideoChips \
              source/VideoRender \
              source/Wii \
              source/WiiSprite \
              source/WiiUsbKeyboard \
              source/Z80

DATA		:=	data
INCLUDES	:=	include/wii

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------

CFLAGS	= -g -O2 -Wall $(MACHDEP) $(INCLUDE) -DNO_ASM -DWII -DDEVKITPPC_STDLIB_INCLUDE=\"$(DEVKITPPC_LOCAL)/powerpc-eabi/include/stdlib.h\"
CXXFLAGS	=	$(CFLAGS)

LDFLAGS	=	-g $(MACHDEP) -Wl,-Map,$(notdir $@).map

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS	:=	 -ldb -lfreetype -lz -lwiiuse -lbte -lfat -logc -lm -lmad

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(CURDIR)

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# automatically build a list of object files for our project
#---------------------------------------------------------------------------------
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
sFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))
PNGFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.png)))
TTFFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.ttf)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) \
					$(sFILES:.s=.o) $(SFILES:.S=.o)

export GENFILES	:=	$(DEVKITPPC_LOCAL)/devkitppc.log $(LIBOGC_LOCAL)/libogc.log sdcard.inc gamepack.inc $(PNGFILES:.png=.inc) $(TTFFILES:.ttf=.inc)

#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE	:=	$(foreach dir,$(INCLUDES), -I $(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD) -I$(CURDIR)/include \
					-I$(LIBOGC_INC_LOCAL) \
					-I$(LIBOGC_INC_LOCAL)/ogc

#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib/wii) \
					-L$(LIBOGC_LIB_LOCAL)

export OUTPUT	:=	$(CURDIR)/$(TARGET)
.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).dol
	@rm -rf $(DEVKITPPC_LOCAL)
	@rm -rf $(LIBOGC_LOCAL)

#---------------------------------------------------------------------------------
run:
	wiiload $(TARGET).dol

#---------------------------------------------------------------------------------
disasm:
	@echo Disassembling ...
	@$(DEVKITPPC_LOCAL)/bin/powerpc-eabi-objdump -S $(TARGET).elf >$(TARGET).txt

#---------------------------------------------------------------------------------
else

DEPENDS	:=	revision $(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
blueMSXwii: $(GENFILES) $(OUTPUT).dol
$(OUTPUT).dol: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

#---------------------------------------------------------------------------------
# SVN revision
#---------------------------------------------------------------------------------
revision:
	@cmd /c ..\\util\\revision.bat

#---------------------------------------------------------------------------------
# This rule links in binary data with the .jpg extension
#---------------------------------------------------------------------------------
%.jpg.o	:	%.jpg
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	$(bin2o)

#---------------------------------------------------------------------------------
# This rule creates the zip files for the sd-card contents and converts it to a .h
#---------------------------------------------------------------------------------
sdcard.inc: ../sdcard/MSX
	@echo Creating sdcard.zip ...
	@rm -f sdcard.zip
	@../util/7za a -r -xr!*.svn -xr!thumbs.* sdcard.zip ../sdcard/MSX
	@echo Converting sdcard.zip to sdcard.inc ...
	@../util/raw2c sdcard.zip sdcard.inc sdcard

gamepack.inc: ../sdcard/Gamepack
	@echo Creating gamepack.zip ...
	@rm -f gamepack.zip
	@../util/7za a -r -xr!*.svn -xr!thumbs.* gamepack.zip ../sdcard/Gamepack/Games
	@echo Converting gamepack.zip to gamepack.inc ...
	@../util/raw2c gamepack.zip gamepack.inc gamepack

#---------------------------------------------------------------------------------
# This rule unpacks the local devkitPPC/libogc zip to a custom directory
#---------------------------------------------------------------------------------
$(DEVKITPPC_LOCAL)/devkitppc.log: ../lib/devkitPPC.zip
	@echo Installing devkitPPC to $(DEVKITPPC_LOCAL)
	@[ -d $(DEVKITPPC_LOCAL) ] || mkdir -p $(DEVKITPPC_LOCAL)
	@../util/unzip -o ../lib/devkitPPC.zip -d $(DEVKITPPC_LOCAL) >$(DEVKITPPC_LOCAL)/devkitppc.log
$(LIBOGC_LOCAL)/libogc.log: ../lib/libogc.zip
	@echo Installing libogc to $(LIBOGC_LOCAL)
	@[ -d $(LIBOGC_LOCAL) ] || mkdir -p $(LIBOGC_LOCAL)
	@../util/unzip -o ../lib/libogc.zip -d $(LIBOGC_LOCAL) >$(LIBOGC_LOCAL)/libogc.log

#---------------------------------------------------------------------------------
# This rule converts .png to .inc files
#---------------------------------------------------------------------------------
%.inc: %.png
	@echo Converting $(notdir $<) to $(notdir $@) ...
	@$(CURDIR)/../util/raw2c $< $(CURDIR)/$@ $(notdir $@)

#---------------------------------------------------------------------------------
# This rule converts .ttf to .inc files
#---------------------------------------------------------------------------------
%.inc: %.ttf
	@echo Converting $(notdir $<) to $(notdir $@) ...
	@../util/raw2c $< $(CURDIR)/$@ $(notdir $@)

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
