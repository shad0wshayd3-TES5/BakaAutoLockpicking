-- include subprojects
includes("lib/commonlibsse")

-- set project constants
set_project("AutoLockNative")
set_version("4.1.0")
set_license("GPL-3.0")
set_languages("c++23")
set_warnings("allextra")

-- add common rules
add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

-- set configs
set_config("commonlib_ini", true)
set_config("commonlib_random", true)

-- define targets
target("AutoLockNative")
    add_deps("AutoLockNative.archive", { order = true })

    add_rules("commonlibsse.plugin", {
        name = "AutoLockNative",
        author = "shad0wshayd3"
    })

    -- add src files
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")

    -- add extra files
    add_extrafiles(".clang-format")

    -- add install files
    add_installfiles("res/(**.ini)")
    add_installfiles("res/(**.json)")
    add_installfiles("res/AutoLockpicking.esp")

target("AutoLockNative.archive")
    add_deps("AutoLockNative.papyrus", { order = true })

    add_rules("commonlibsse.archive", {
        name = "AutoLockpicking",
        install = "AutoLockNative"
    })

    add_extrafiles("res/(**.txt)")
    add_extrafiles("res/(**.swf)")
    add_extrafiles("res/(**.psc)")

target("AutoLockNative.papyrus")
    add_rules("commonlibsse.papyrus", {
        archive = "AutoLockNative.archive",
        options = {
            imports = {
                "lib/mcmhelper-sdk"
            }
        }
    })

    add_extrafiles("res/Source/Scripts/(**.psc)")
