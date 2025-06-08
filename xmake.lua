-- set minimum xmake version
set_xmakever("2.8.2")

-- includes
includes("lib/commonlibsse")

-- set project
set_project("NavigationRestrictions")
set_version("3.0.0")
set_license("GPL-3.0")

-- set defaults
set_languages("c++23")
set_warnings("allextra")

-- add rules
add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

-- set policies
set_policy("package.requires_lock", true)

-- set configs
set_config("skyrim_ae", true)

set_config("commonlib_ini", true)
set_config("commonlib_json", true)
set_config("commonlib_toml", true)
add_extrafiles("release/**.ini")
add_extrafiles("release/**.json")
add_extrafiles("release/**.toml")

-- targets
target("NavigationRestrictions")
    -- add dependencies to target
    add_deps("commonlibsse")

    -- add commonlibsse plugin
    add_rules("commonlibsse.plugin", {
        name = "NavigationRestrictions",
        author = "styyx",
        description = "SKSE64 plugin template using CommonLibSSE"
    })

    -- add src files
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    add_includedirs("extern/glaze/include", { public = true })
    add_includedirs("extern/ClibUtil/include", { public = true })
    set_pcxxheader("src/pch.h")

after_build(function(target)
    local copy = function(env, ext)
        for _, env in pairs(env:split(";")) do
            if os.exists(env) then
                local plugins = path.join(env, ext, "SKSE/Plugins")
                os.mkdir(plugins)
                os.trycp(target:targetfile(), plugins)
                os.trycp(target:symbolfile(), plugins)
                -- Copy config files or other extras
                os.trycp("$(projectdir)/release/**", plugins)
            end
        end
    end
    if os.getenv("XSE_TES5_MODS_PATH") then
        copy(os.getenv("XSE_TES5_MODS_PATH"), target:name())
    elseif os.getenv("XSE_TES5_GAME_PATH") then
        copy(os.getenv("XSE_TES5_GAME_PATH"), "Data")
    end    
end)
