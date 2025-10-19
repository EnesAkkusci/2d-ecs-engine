const std = @import("std");

fn addSourceFilesRecursive(b: *std.Build, step: *std.Build.Step.Compile, path: []const u8, extension: []const u8) !void {
    const cwd = std.fs.cwd();

    var dir = try cwd.openDir(path, .{ .iterate = true, .access_sub_paths = true });
    defer dir.close();

    var walker = try dir.walk(b.allocator);
    defer walker.deinit();

    while (try walker.next()) |entry| {
        if (entry.kind == .file and std.mem.endsWith(u8, entry.path, extension)) {
            step.addCSourceFile(.{
                .file = b.path(b.pathJoin(&.{ path, entry.path })),
                .flags = &[_][]const u8{"-std=c++20"},
            });
        }
    }
}

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe_mod = b.createModule(.{
        .link_libc = true,
        .target = target,
        .optimize = optimize,
    });

    const exe = b.addExecutable(.{
        .name = "2d-ecs-engine",
        .root_module = exe_mod,
    });

    // Add C++ source files
    addSourceFilesRecursive(b, exe, "src", ".cpp") catch |err| {
        std.debug.print("Failed to add C/C++ Source Files: {}", .{err});
    };
    addSourceFilesRecursive(b, exe, "libs/header", ".cpp") catch |err| {
        std.debug.print("Failed to add C/C++ Source Files: {}", .{err});
    };

    exe.addIncludePath(b.path("libs/header"));
    exe.addIncludePath(b.path("libs/compiled/SDL2/include"));

    // Link static and dynamic libs
    exe.linkSystemLibrary("c++");
    exe.linkSystemLibrary("SDL2");
    exe.linkSystemLibrary("SDL2_image");
    exe.linkSystemLibrary("SDL2_ttf");
    exe.linkSystemLibrary("SDL2_mixer");
    exe.linkSystemLibrary("lua");

    b.installArtifact(exe);
}
