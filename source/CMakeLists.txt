cmake_minimum_required(VERSION 2.8)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -fPIC")

add_library(emu2413 STATIC
    emu2413/2413tone.h
    emu2413/281btone.h
    emu2413/emu2413.c
    emu2413/emu2413.h
    emu2413/emutypes.h
    emu2413/vrc7tone.h
)


# you must download the VST SDK and copy the directory:
#   pluginterfaces/vst2.x
# into vst2.x/pluginterfaces/vst2.x
# and all the files from:
#   public.sdk/source/vst2.x
# into vst2.x
include_directories( vst2.x )
add_definitions( -D__cdecl= )

add_library(vst2413p SHARED
    RhythmDriver.cpp
    RhythmDriver.h
    SynthDriver.cpp
    SynthDriver.h
    Vst2413p.cpp
    Vst2413p.h
    vst2.x/audioeffectx.cpp
    vst2.x/audioeffectx.h
)

add_library(vst2413r SHARED
    RhythmDriver.cpp
    RhythmDriver.h
    SynthDriver.cpp
    SynthDriver.h
    Vst2413r.cpp
    Vst2413r.h
)

add_library(vst2413s SHARED
    RhythmDriver.cpp
    RhythmDriver.h
    SynthDriver.cpp
    SynthDriver.h
    Vst2413s.cpp
    Vst2413s.h
)

target_link_libraries(vst2413p emu2413)
target_link_libraries(vst2413r emu2413)
target_link_libraries(vst2413s emu2413)
