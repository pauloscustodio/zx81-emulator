#------------------------------------------------------------------------------
# zx81-emulator
# Copyright (C) Paulo Custodio, 2026
# License: GPL v2
#------------------------------------------------------------------------------

PROJ = zx81

ifeq ($(OS),Windows_NT)
  EXESUFFIX	:= .exe
else
  EXESUFFIX	?=
endif

#------------------------------------------------------------------------------
# tools
#------------------------------------------------------------------------------
PERL	= perl
CC		= gcc
CXX 	= g++
INSTALL = install
STYLE_WIDTH = 80
ASTYLE	= astyle --style=attach --pad-oper --align-pointer=type \
		  --break-closing-braces --add-braces --attach-return-type \
		  --max-code-length=$(STYLE_WIDTH) --lineend=linux --formatted 
PERLTIDY = perltidy --indent-columns=4 --continuation-indentation=4 \
		  --maximum-line-length=$(STYLE_WIDTH) -b
TIDY_STAMP = .tidy-stamp

#------------------------------------------------------------------------------
# compile options
#------------------------------------------------------------------------------

# z80ex configuration defines
ENDIANNESS := $(shell $(PERL) -MConfig -e \
				'print $$Config{byteorder} =~ /1234/ ? \
				"WORDS_LITTLE_ENDIAN" : "WORDS_BIG_ENDIAN"')
API_REVISION := 1
VERSION_MAJOR:=1
VERSION_MINOR:=21
RELEASE_TYPE :=
VERSION_STR:= $(API_REVISION).$(VERSION_MAJOR).$(VERSION_MINOR)$(RELEASE_TYPE)

Z80EXFLAGS = -D$(ENDIANNESS) \
			 -DZ80EX_VERSION_STR=$(VERSION_STR) \
			 -DZ80EX_API_REVISION=$(API_REVISION) \
			 -DZ80EX_VERSION_MAJOR=$(VERSION_MAJOR) \
			 -DZ80EX_VERSION_MINOR=$(VERSION_MINOR) \
			 -DZ80EX_RELEASE_TYPE=$(RELEASE_TYPE) \
			 -DZ80EX_OPSTEP_FAST_AND_ROUGH

FLAGS	= -O3 -MMD -Wall -Werror \
		  $(Z80EXFLAGS) \
		  -I. \
		  -Icpu \
		  -Icpu/z80ex/include \
		  -Imachine
CFLAGS	= $(FLAGS)
CXXFLAGS= $(FLAGS)
LDFLAGS = 

#------------------------------------------------------------------------------
# target files
#------------------------------------------------------------------------------
BUILD_DIR = build

BUILD_DIRS=	$(BUILD_DIR) \
			$(BUILD_DIR)/cpu \
			$(BUILD_DIR)/cpu/z80ex \
			$(BUILD_DIR)/machine
			
CSRCS	:= cpu/z80ex/z80ex.c \
		   cpu/z80ex/z80ex_dasm.c

CXXSRCS	:= $(wildcard *.cpp) \
		   $(wildcard cpu/*.cpp) \
		   $(wildcard machine/*.cpp)
		   
OBJS 	:= $(patsubst %.c,$(BUILD_DIR)/%.o,$(CSRCS)) \
		   $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(CXXSRCS))

DEPENDS = $(OBJS:.o=.d)

#------------------------------------------------------------------------------
# build
#------------------------------------------------------------------------------
all: $(PROJ)$(EXESUFFIX)

$(PROJ)$(EXESUFFIX): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) -o $@ 

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIRS)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIRS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIRS):
	$(PERL) -MExtUtils::Command -e mkpath -- $@

$(BUILD_DIR)/$(PROJ).o: machine/rom.h

machine/rom.h: tools/zx81_edition3.rom tools/file2c.pl Makefile
	perl tools/file2c.pl $< rom > $@
	dos2unix $@

#------------------------------------------------------------------------------
# test
#------------------------------------------------------------------------------
test: $(PROJ)$(EXESUFFIX)
	$(PERL) -S prove t/*.t

#------------------------------------------------------------------------------
# tidy
#------------------------------------------------------------------------------
tidy:
	@[ -f $(TIDY_STAMP) ] || touch -t 197001010000 $(TIDY_STAMP)
	find . \( -name '*.c' -o -name '*.cpp' -o -name '*.h' -o -name '*.re' \) \
		-name z80ex -prune \
		-newer $(TIDY_STAMP) -exec echo $(ASTYLE) {} +
	find . \( -name '*.pl' -o -name '*.pm' -o -name '*.t' \) \
		-name z80ex -prune \
		-newer $(TIDY_STAMP) -exec echo $(PERLTIDY) {} +
	touch $(TIDY_STAMP)

#------------------------------------------------------------------------------
# clean
#------------------------------------------------------------------------------
clean::
	$(RM) $(wildcard $(PROJ)$(EXESUFFIX) $(OBJS) $(DEPENDS) \
					*.o *.d *.orig *.bak $(PROJ).txt $(PROJ).dump)
	$(RM) -r $(BUILD_DIR)

#------------------------------------------------------------------------------
-include $(DEPENDS)
