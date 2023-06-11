add_requires("libhv 1.3.1")

local _deps = ".xmake/_deps"
local logRoot = path.join(_deps, "log")
local ndk_path = "D:/Android/Sdk/ndk/25.1.8937393"

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

task("android_run")
    on_run(function () 
        import("utils.archive")
        import("lib.detect.find_program")
        import("net.http")
        import("devel.git")
        import("utils.archive")
        local architecture = "armeabi-v7a"
        -- local architecture = "arm64-v8a"
        os.execv(find_program("xmake"), {"f", "-p", "android", "--ndk=" .. ndk_path, "-a", architecture, "-m", "debug"})
        os.execv(find_program("xmake"), {"build", "h264_decode_android_example"})
        os.execv(find_program("adb"), {"push", "./build/android/" .. architecture .. "/debug/h264_decode_android_example", "/data/local/tmp/test"})
        os.execv(find_program("adb"), {"shell", "chmod", "+x", "/data/local/tmp/test/h264_decode_android_example"})
        os.execv(find_program("adb"), {"shell", "cd", "/data/local/tmp/test", ";", "./h264_decode_android_example"})
    end)

    set_menu {
        usage = "xmake android_run",
        options = {
        }
    }    

target("log_c")
    local kind = "static"
    set_kind(kind)
    add_files(logRoot .. "/src/*.c")
    add_headerfiles(logRoot .. "/src/*.h")
    add_includedirs(logRoot .. "/src", { public = true })
    set_languages("c11")
    add_rules("mode.debug", "mode.release")
    add_defines("LOG_USE_COLOR")
    after_build(function (target) 
        local root = path.join(target:targetdir(), "log_c")
        local include = path.join(root, "include/")
        local lib = path.join(root, "lib/")
        os.mkdir(root)
        os.mkdir(include)
        os.mkdir(lib)
        os.cp(target:targetfile(), lib)
        os.cp(logRoot .. "/src/*.h", include)
    end)

target("util")
    local kind = "static"
    set_kind(kind)
    add_files("libutil/src/*.c")
    add_headerfiles("libutil/include/public/util/*.h")
    add_includedirs("libutil/include/public", { public = true })
    set_languages("c11")
    add_deps("log_c")
    add_rules("mode.debug", "mode.release")
    if kind == "shared" then
        add_defines("Util_BUILD_DLL_EXPORT")
    end    
    after_build(function (target) 
        local root = path.join(target:targetdir(), "util")
        local include = path.join(root, "include/util")
        local lib = path.join(root, "lib/")
        os.mkdir(root)
        os.mkdir(include)
        os.mkdir(lib)      
        os.cp("libutil/include/public/util/*.h", include)
        os.cp(target:targetfile(), lib)
        if is_plat("windows") == true and target:kind() == "shared" then
            os.cp(path.join(target:targetdir(), target:basename()) .. ".lib", lib)
        end
    end)

target("h264_parse")
    local kind = "static"
    set_kind(kind)
    add_files("libh264_parse/src/*.c")
    add_headerfiles("libh264_parse/include/public/h264_parse/*.h")
    add_includedirs("libh264_parse/include/public", { public = true })
    set_languages("c11")
    add_rules("mode.debug", "mode.release")
    add_deps("log_c")
    add_deps("util")
    if kind == "shared" then
        add_defines("H264_PARSE_BUILD_DLL_EXPORT")
    end    
    after_build(function (target) 
        local root = path.join(target:targetdir(), "h264_parse")
        local include = path.join(root, "include/h264_parse")
        local lib = path.join(root, "lib/")
        os.mkdir(root)
        os.mkdir(include)
        os.mkdir(lib)      
        os.cp("libh264_parse/include/public/h264_parse/*.h", include)
        os.cp(target:targetfile(), lib)
        if is_plat("windows") == true and target:kind() == "shared" then
            os.cp(path.join(target:targetdir(), target:basename()) .. ".lib", lib)
        end
    end)

target("h264_android_codec")
    local kind = "static"
    set_kind(kind)
    add_files("libh264_android_codec/src/*.c")
    add_headerfiles("libh264_android_codec/include/public/h264_android_codec/*.h")
    add_includedirs("libh264_android_codec/include/public", { public = true })
    set_languages("c11")
    add_rules("mode.debug", "mode.release")
    add_deps("h264_parse")
    add_deps("log_c")
    add_deps("util")
    after_build(function (target) 
        local root = path.join(target:targetdir(), "h264_android_codec")
        local include = path.join(root, "include/h264_android_codec")
        local lib = path.join(root, "lib/")
        os.mkdir(root)
        os.mkdir(include)
        os.mkdir(lib)      
        os.cp("libh264_android_codec/include/public/h264_android_codec/*.h", include)
        os.cp(target:targetfile(), lib)
        if is_plat("windows") == true and target:kind() == "shared" then
            os.cp(path.join(target:targetdir(), target:basename()) .. ".lib", lib)
        end
    end)    

target("h264_parse_example")
    set_kind("binary")
    add_files("h264_parse_example/main.c")
    add_deps("h264_parse")
    add_deps("log_c")
    add_deps("util")
    add_packages("libhv")
    add_includedirs("h264_parse_example")
    set_languages("c11")
    add_rules("mode.debug", "mode.release")
    after_build(function (target) 
        os.cp("assets", target:targetdir())
    end)

target("h264_decode_android_example")
    set_kind("binary")
    add_files("h264_decode_android_example/main.c")
    add_deps("log_c")
    add_deps("h264_parse")
    add_deps("h264_android_codec")
    add_deps("util")
    add_packages("libhv")
    add_includedirs("h264_decode_android_example")
    set_languages("c11")
    add_rules("mode.debug", "mode.release")
    add_links("mediandk")
    add_links("log")

target("main")
    set_kind("static")
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