local _deps = ".xmake/_deps"
local logRoot = path.join(_deps, "log")
local libyuvRoot = path.join(_deps, "libyuv")
local libhvRoot = path.join(_deps, "libhv")
local ndk_path = "D:/Android/Sdk/ndk/25.1.8937393"

-- add_requires("libhv 1.3.1")

task("download_deps")
    on_run(function () 
        import("utils.archive")
        import("lib.detect.find_program")
        import("net.http")
        import("devel.git")
        import("utils.archive")
        if os.exists(logRoot) == false then
            git.clone("https://github.com/rxi/log.c.git", {depth = 1, branch = "master", outputdir = logRoot})
        end
        if os.exists(libyuvRoot) == false then
            git.clone("https://chromium.googlesource.com/libyuv/libyuv", {depth = 1, branch = "stable", outputdir = libyuvRoot})
        end
        if os.exists(libhvRoot) == false then
            git.clone("https://github.com/ithewei/libhv.git", {outputdir = libhvRoot})
            git.checkout("v1.3.1", {repodir = libhvRoot})
        end
    end)

    set_menu {
        usage = "xmake download_deps",
        options = {
        }
    }

task("clang_format")
    on_run(function ()
        import("lib.detect.find_program")
        local program = find_program("clang-format")
        local function doFormat(folderPath, suffix) 
            local style = "Microsoft"
            -- local style = "LLVM"
            if os.exists(folderPath) and #os.files(path.join(folderPath, "*." .. suffix)) > 0 then
                os.execv(program, {"-style=" .. style, "-i", path.join(folderPath, "*." .. suffix)})
            end            
        end
        local function clangformatFiles(folderPath)
            doFormat(folderPath, "hpp")
            doFormat(folderPath, "h")
            doFormat(folderPath, "cpp")
            doFormat(folderPath, "c")
            for _, dir in ipairs(os.dirs(folderPath .. "/**")) do
                doFormat(dir, "hpp")
                doFormat(dir, "h")
                doFormat(dir, "cpp")
                doFormat(dir, "c")
            end
        end        
        if program == "clang-format" then 
            clangformatFiles("libh264_parse")
            clangformatFiles("h264_parse_example")
            clangformatFiles("h264_decode_android_example")
            clangformatFiles("libh264_android_codec")
            clangformatFiles("libutil")
            clangformatFiles("createApplication")
            clangformatFiles("client")
            clangformatFiles("server")
            clangformatFiles("libserver")
        end
    end)

    set_menu {
        usage = "xmake clang_format",
        options = {
        }
    }

task("vs_project")
    on_run(function ()
        import("lib.detect.find_program")
        os.execv(find_program("xmake"), {"f", "-p", "windows"})
        os.execv(find_program("xmake"), {"project", "-k", "vsxmake2022", "-a", "x64", "-m", "debug"})
    end)

    set_menu {
        usage = "xmake vs_project",
        options = {
        }
    }

task("android_build")
    on_run(function () 
        import("core.base.option")
        import("utils.archive")
        import("lib.detect.find_program")
        import("net.http")
        import("devel.git")
        import("utils.archive")
        local target = option.get("target")
        local v = ""
        if option.get("verbose") then
            v = "-v"
        end
        for _, architecture in ipairs({"armeabi-v7a", "arm64-v8a" --[[, "x86_64", "x86"--]]}) do
            for _, mode in ipairs({"debug", "release"}) do
                for _, kind in ipairs({"static", "shared"}) do
                    os.execv(find_program("xmake"), {"f", "-p", "android", "--ndk=" .. ndk_path, "-a", architecture, "-m", mode, "-k", kind})
                    os.execv(find_program("xmake"), {"build", v, target})
                end
            end
        end
    end)

    set_menu {
        usage = "xmake android_build",
        options = {
            {"t", "target", "kv",  nil, nil },
            {"v", "verbose", "k",  nil, nil },
        }
    }    

task("android_run")
    on_run(function () 
        import("core.base.option")
        import("utils.archive")
        import("lib.detect.find_program")
        import("net.http")
        import("devel.git")
        import("utils.archive")
        local target = option.get("target")
        local architecture = option.get("architecture")
        local mode = option.get("mode")
        local remote = "/data/local/tmp/test"
        os.execv(find_program("adb"), {"push", path.join("./build/android", architecture, mode, target), remote})
        os.execv(find_program("adb"), {"shell", "chmod", "+x", remote .. "/" .. target})
        os.execv(find_program("adb"), {"shell", "cd", remote, ";", "./" .. target})
    end)

    set_menu {
        usage = "xmake android_run",
        options = {
            {"t", "target", "kv",  nil, nil },
            {"a", "architecture", "kv",  nil, nil },
            {"m", "mode", "kv",  nil, nil },
        }
    }

target("libhv")
    local WITH_KCP = true 
    local WITH_PROTOCOL = false  
    local WITH_EVPP = true 
    local WITH_HTTP = true 
    local WITH_NGHTTP2 = false  
    local WITH_HTTP_SERVER = true 
    local WITH_HTTP_CLIENT = true 
    local WITH_MQTT = false  
    local ENABLE_UDS = false 
    local USE_MULTIMAP = false  
    local ENABLE_WINDUMP = false 

    set_kind("$(kind)")
    set_languages("cxx11", "c11")
    set_targetdir(path.join(get_config("buildir"), "lib", get_config("plat"), get_config("mode"), get_config("arch")))

    if is_plat("windows") then
        add_defines("WIN32_LEAN_AND_MEAN")
        add_defines("_CRT_SECURE_NO_WARNINGS")
        add_defines("_WIN32_WINNT=0x0600")
        add_syslinks("winmm")
        add_syslinks("iphlpapi")
        add_syslinks("ws2_32")
        if ENABLE_WINDUMP then
            add_defines("ENABLE_WINDUMP")
        end
    end
    if is_plat("android") then
        add_links("log")
    end

    if is_kind("static") then 
        add_defines("HV_STATICLIB")
    elseif is_kind("shared") then
        add_defines("HV_DYNAMICLIB")
    end

    if is_mode("debug") then
        add_defines("DEBUG")
    elseif is_mode("release") then
        add_defines("NDEBUG")
    end
    if ENABLE_UDS then
        add_defines("ENABLE_UDS")
    end
    if USE_MULTIMAP then
        add_defines("USE_MULTIMAP")
    end

    add_rules("mode.debug", "mode.release")

    add_files(libhvRoot .. "/base/*.c")
    add_files(libhvRoot .. "/ssl/*.c")
    add_files(libhvRoot .. "/event/*.c")
    add_files(libhvRoot .. "/util/*.c")

    add_headerfiles(libhvRoot .. "/base/*.h")
    add_headerfiles(libhvRoot .. "/ssl/*.h")
    add_headerfiles(libhvRoot .. "/event/*.h")
    add_headerfiles(libhvRoot .. "/util/*.h")

    add_includedirs(libhvRoot)
    add_includedirs(libhvRoot .. "/base")
    add_includedirs(libhvRoot .. "/ssl")
    add_includedirs(libhvRoot .. "/event")
    add_includedirs(libhvRoot .. "/util")

    if WITH_KCP then
        add_files(libhvRoot .. "/event/kcp/*.c")
    end
    if WITH_PROTOCOL then
        add_files(libhvRoot .. "/protocol/*.c")
    end
    if WITH_EVPP then
        add_files(libhvRoot .. "/cpputil/*.cpp")
        add_files(libhvRoot .. "/evpp/*.cpp")
        add_includedirs(libhvRoot .. "/cpputil")
        add_includedirs(libhvRoot .. "/evpp")
        add_headerfiles(libhvRoot .. "/cpputil/*.h")
        remove_files(libhvRoot .. "/evpp/*_test.cpp")
        if WITH_HTTP then
            add_files(libhvRoot .. "/http/*.c")
            add_files(libhvRoot .. "/http/*.cpp")
            add_includedirs(libhvRoot .. "/http")
            add_headerfiles(libhvRoot .. "/http/httpdef.h")
            add_headerfiles(libhvRoot .. "/http/wsdef.h")
            add_headerfiles(libhvRoot .. "/http/http_content.h")
            add_headerfiles(libhvRoot .. "/http/HttpMessage.h")
            add_headerfiles(libhvRoot .. "/http/HttpParser.h")
            add_headerfiles(libhvRoot .. "/http/WebSocketParser.h")
            add_headerfiles(libhvRoot .. "/http/WebSocketChannel.h")
            if WITH_NGHTTP2 then
                add_defines("WITH_NGHTTP2")
                add_headerfiles(libhvRoot .. "/http/http2def.h")
                add_headerfiles(libhvRoot .. "/http/grpcdef.h")
            end
            if WITH_HTTP_SERVER then
                add_files(libhvRoot .. "/http/server/*.cpp")
            end
            if WITH_HTTP_CLIENT then
                add_files(libhvRoot .. "/http/client/*.cpp")
            end            
        end
    end
    if WITH_MQTT then
        add_files(libhvRoot .. "/mqtt/*.c")
    end
    add_includedirs(libhvRoot .. "/include", {public = true})
    add_rpathdirs("./")
    after_build(function (target)
        os.cp(libhvRoot .. "/include", path.join(get_config("buildir"), "include", "libhv"))
    end)    

target("libyuv")
    set_kind("$(kind)")
    set_languages("cxx11", "c11")
    set_basename("yuv")
    set_targetdir(path.join(get_config("buildir"), "lib", get_config("plat"), get_config("mode"), get_config("arch")))
    add_rules("mode.debug", "mode.release")
    add_rpathdirs("./")
    add_files(libyuvRoot .. "/source/*.cc")
    add_headerfiles(libyuvRoot .. "/include/**.h")
    add_includedirs(libyuvRoot .. "/include", { public = true })
    after_build(function (target) 
        for _, item in ipairs(target:get("includedirs")) do
            os.cp(item, path.join(get_config("buildir"), "include", "libyuv"))
        end
    end)

target("log_c")
    set_kind("$(kind)")
    set_languages("c11")
    set_targetdir(path.join(get_config("buildir"), "lib", get_config("plat"), get_config("mode"), get_config("arch")))
    add_rules("mode.debug", "mode.release")
    add_files(logRoot .. "/src/*.c")
    add_headerfiles(logRoot .. "/src/*.h")
    add_includedirs(logRoot .. "/src", { public = true })
    add_defines("LOG_USE_COLOR")
    add_rpathdirs("./")
    after_build(function (target) 
        local include = path.join(get_config("buildir"), "include", "log_c", "include")
        os.mkdir(include)
        for _, headerfile in ipairs(os.files(logRoot .. "/src/*.h")) do
            os.cp(headerfile, include)
        end
    end)

target("util")
    set_kind("$(kind)")
    set_languages("c11")
    set_targetdir(path.join(get_config("buildir"), "lib", get_config("plat"), get_config("mode"), get_config("arch")))
    add_rules("mode.debug", "mode.release")
    add_files("libutil/src/*.c")
    add_headerfiles("libutil/include/public/util/*.h")
    add_includedirs("libutil/include/public", { public = true })
    add_deps("log_c")
    add_rpathdirs("./")
    if is_kind("shared") then
        add_defines("Util_BUILD_DLL_EXPORT")
    end    
    after_build(function (target) 
        local include = path.join(get_config("buildir"), "include", "util", "include")
        os.mkdir(include)
        os.cp("libutil/include/public/util/", include)
    end)

target("h264_parse")
    set_kind("$(kind)")
    set_languages("c11")
    set_targetdir(path.join(get_config("buildir"), "lib", get_config("plat"), get_config("mode"), get_config("arch")))
    add_rules("mode.debug", "mode.release")
    add_files("libh264_parse/src/*.c")
    add_headerfiles("libh264_parse/include/public/h264_parse/*.h")
    add_includedirs("libh264_parse/include/public", { public = true })
    add_deps("log_c")
    add_deps("util")
    add_rpathdirs("./")
    if is_kind("shared") then
        add_defines("H264_PARSE_BUILD_DLL_EXPORT")
    end    
    after_build(function (target) 
        local include = path.join(get_config("buildir"), "include", "h264_parse", "include")
        os.mkdir(include)
        os.cp("libh264_parse/include/public/h264_parse/", include)
    end)

target("h264_android_codec")
    set_kind("$(kind)")
    set_languages("c11")
    set_targetdir(path.join(get_config("buildir"), "lib", get_config("plat"), get_config("mode"), get_config("arch")))
    add_files("libh264_android_codec/src/*.c")
    add_headerfiles("libh264_android_codec/include/public/h264_android_codec/*.h")
    add_includedirs("libh264_android_codec/include/public", { public = true })
    add_rules("mode.debug", "mode.release")
    add_deps("h264_parse")
    add_deps("log_c")
    add_deps("util")
    add_rpathdirs("./")
    if is_kind("shared") then 
        add_defines("H264_ANDROID_CODEC_BUILD_DLL_EXPORT")
    end
    after_build(function (target) 
        local include = path.join(get_config("buildir"), "include", "h264_android_codec", "include")
        os.mkdir(include)
        os.cp("libh264_android_codec/include/public/h264_android_codec/", include)
    end)    

target("h264_parse_example")
    set_kind("binary")
    set_languages("c11")
    add_files("h264_parse_example/main.c")
    add_deps("h264_parse")
    add_deps("log_c")
    add_deps("util")
    add_deps("libhv")
    add_includedirs("h264_parse_example")
    add_rules("mode.debug", "mode.release")
    add_rpathdirs("./")
    after_build(function (target) 
        os.cp("assets", target:targetdir())
    end)

target("h264_decode_android_example")
    set_kind("binary")
    set_languages("c11")
    add_rules("mode.debug", "mode.release")
    add_files("h264_decode_android_example/main.c")
    add_deps("log_c")
    add_deps("h264_parse")
    add_deps("h264_android_codec")
    add_deps("util")
    add_includedirs("h264_decode_android_example")
    add_links("mediandk")
    add_links("log")
    add_rpathdirs("./")

target("main")
    set_kind("$(kind)")
    add_files("createApplication/main.c")
    add_headerfiles("createApplication/application.h")
    add_rules("mode.debug", "mode.release")
    set_languages("c11")

target("createApplication")
    set_kind("binary")
    add_headerfiles("createApplication/application.h")
    add_files("createApplication/createApplication.c")
    add_rules("mode.debug", "mode.release")
    set_languages("c11")
    add_deps("main")
    if is_plat("windows") then 
        add_ldflags("-subsystem:console")
    end

target("client")
    set_kind("binary")
    set_languages("c11")
    add_rules("mode.debug", "mode.release")
    add_files("client/*.c")
    add_deps("libhv")
    add_deps("log_c")
    add_deps("util")
    add_rpathdirs("./")

target("server")
    set_kind("binary")
    set_languages("c11")
    add_rules("mode.debug", "mode.release")
    add_files("server/*.c")
    add_deps("libhv")
    add_deps("h264_android_codec")
    add_deps("libserver")
    add_links("mediandk")
    add_rpathdirs("./")

target("libserver")
    set_kind("$(kind)")
    set_basename("server") 
    set_languages("c11")
    set_targetdir(path.join(get_config("buildir"), "lib", get_config("plat"), get_config("mode"), get_config("arch")))
    add_rules("mode.debug", "mode.release")
    add_files("libserver/src/*.c")
    add_includedirs("libserver/include/public", { public = true })
    add_deps("libhv")
    add_deps("log_c")
    add_deps("util")
    add_deps("h264_android_codec")
    add_deps("libyuv")
    add_links("mediandk")
    add_rpathdirs("./")
    if is_kind("shared") then 
        add_defines("LIBSERVER_BUILD_DLL_EXPORT")
    end
    after_build(function (target) 
        local include = path.join(get_config("buildir"), "include", "libserver", "include")
        os.mkdir(include)
        os.cp("libserver/include/public/libserver/", include)
    end)    
