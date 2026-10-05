# Shared build settings applied per-target (never globally), so that third-party
# code fetched into the build (e.g. GoogleTest) is not affected by our flags.

# expr_enable_warnings(<target>)
#   Enables a strict but practical warning set for GCC/Clang and MSVC.
function(expr_enable_warnings target)
    if(MSVC)
        set(warnings
            /W4             # high warning level
            /permissive-    # standards conformance
            /w14242         # conversion, possible loss of data
            /w14254         # larger bit field assigned to smaller
            /w14263         # member function does not override base virtual
            /w14265         # class has virtual functions but non-virtual dtor
            /w14287         # unsigned/negative constant mismatch
            /w14296         # expression is always true/false
            /w14311         # pointer truncation
            /w14545 /w14546 /w14547 /w14549 /w14555  # suspicious comma/expr
            /w14826         # sign-extended conversion
            /w14905 /w14906 # string literal casts
            /w14928         # illegal copy-initialization
        )
        if(EXPR_WARNINGS_AS_ERRORS)
            list(APPEND warnings /WX)
        endif()
    else()
        set(warnings
            -Wall
            -Wextra
            -Wpedantic
            -Wshadow
            -Wconversion
            -Wsign-conversion
            -Wold-style-cast
            -Wcast-align
            -Wnon-virtual-dtor
            -Woverloaded-virtual
            -Wnull-dereference
            -Wdouble-promotion
            -Wformat=2
            -Wimplicit-fallthrough
        )
        if(EXPR_WARNINGS_AS_ERRORS)
            list(APPEND warnings -Werror)
        endif()
    endif()

    target_compile_options(${target} PRIVATE ${warnings})
endfunction()

# expr_configure_executable(<target>)
#   On MinGW, link the C++ runtime statically. Otherwise the executable depends
#   on libstdc++-6.dll / libgcc_s_seh-1.dll at run time, and if an older MinGW
#   installation appears first on PATH, the wrong DLL is loaded and the program
#   fails to start. Static linking makes the .exe self-contained.
function(expr_configure_executable target)
    if(MINGW)
        target_link_options(${target} PRIVATE -static)
    endif()
endfunction()
