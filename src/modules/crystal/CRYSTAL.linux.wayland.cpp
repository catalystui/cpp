#include "CMAKECFG.h"
#if TARGET_PLATFORM_LINUX

#include "CATCRLIB.hpp"
#include "CATMMLIB.hpp"
#include "CRYSTAL.internal.h"
#include "CRYSTAL.linux.internal.h"

#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-unstable-v1-client-protocol.h"

#include <linux/memfd.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <poll.h>

typedef struct CRYSTALwaylandBuffer {
    wl_buffer* wayland;
    void* data;
    catalyst::NUINT width;
    catalyst::NUINT height;
    catalyst::NUINT stride;
    catalyst::NUINT size;
} CRYSTALwaylandBuffer;

typedef struct CRYSTALwaylandPending {
    catalyst::NUINT width;
    catalyst::NUINT height;
} CRYSTALwaylandPending;

typedef struct CRYSTALwaylandPlatform {
    CRYSTALwindow* window;
    wl_surface* wlSurface;
    xdg_surface* xdgSurface;
    xdg_toplevel* xdgTopLevel;
    zxdg_toplevel_decoration_v1* xdgDecoration;
    CRYSTALwaylandBuffer buffer;
    CRYSTALwaylandPending pending;
    catalyst::BOOL configured;
    catalyst::BOOL destroying;
} CRYSTALwaylandPlatform;

static wl_display* crystalWaylandDisplay = 0;
static wl_registry* crystalWaylandRegistry = 0;
static wl_compositor* crystalWaylandCompositor = 0;
static wl_shm* crystalWaylandShm = 0;
static xdg_wm_base* crystalWaylandWmBase = 0;
static zxdg_decoration_manager_v1* crystalWaylandDecorationManager = 0;
static catalyst::NUINT crystalWaylandWindowCount = 0;

static int crystalWaylandCreateSharedMemoryFd(catalyst::NUINT size) {
    int fd = (int) syscall(SYS_memfd_create, "crystal-wayland", MFD_CLOEXEC);
    if (fd < 0) {
        return -1;
    }
    if (syscall(SYS_ftruncate, fd, (off_t) size) != 0) {
        syscall(SYS_close, fd);
        return -1;
    }
    return fd;
}

static void crystalWaylandRegistryAdded(void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version) {
    if (interface == 0) return;

    // wl_compositor
    if (interface[0] == 'w' &&
        interface[1] == 'l' &&
        interface[2] == '_' &&
        interface[3] == 'c' &&
        interface[4] == 'o' &&
        interface[5] == 'm' &&
        interface[6] == 'p' &&
        interface[7] == 'o' &&
        interface[8] == 's' &&
        interface[9] == 'i' &&
        interface[10] == 't' &&
        interface[11] == 'o' &&
        interface[12] == 'r' &&
        interface[13] == 0
    ) {
        crystalWaylandCompositor = (wl_compositor*) wl_registry_bind(registry, name, &wl_compositor_interface, version < 4 ? version : 4);
        return;
    }

    // wl_shm
    if (
        interface[0] == 'w' &&
        interface[1] == 'l' &&
        interface[2] == '_' &&
        interface[3] == 's' &&
        interface[4] == 'h' &&
        interface[5] == 'm' &&
        interface[6] == 0
    ) {
        crystalWaylandShm = (wl_shm*) wl_registry_bind(registry, name, &wl_shm_interface, version < 1 ? version : 1);
        return;
    }

    // xdg_wm_base
    if (
        interface[0] == 'x' &&
        interface[1] == 'd' &&
        interface[2] == 'g' &&
        interface[3] == '_' &&
        interface[4] == 'w' &&
        interface[5] == 'm' &&
        interface[6] == '_' &&
        interface[7] == 'b' &&
        interface[8] == 'a' &&
        interface[9] == 's' &&
        interface[10] == 'e' &&
        interface[11] == 0
    ) {
        crystalWaylandWmBase = (xdg_wm_base*) wl_registry_bind(registry, name, &xdg_wm_base_interface, version < 2 ? version : 2);
        return;
    }

    // zxdg_decoration_manager_v1
    if (
        interface[0] == 'z' &&
        interface[1] == 'x' &&
        interface[2] == 'd' &&
        interface[3] == 'g' &&
        interface[4] == '_' &&
        interface[5] == 'd' &&
        interface[6] == 'e' &&
        interface[7] == 'c' &&
        interface[8] == 'o' &&
        interface[9] == 'r' &&
        interface[10] == 'a' &&
        interface[11] == 't' &&
        interface[12] == 'i' &&
        interface[13] == 'o' &&
        interface[14] == 'n' &&
        interface[15] == '_' &&
        interface[16] == 'm' &&
        interface[17] == 'a' &&
        interface[18] == 'n' &&
        interface[19] == 'a' &&
        interface[20] == 'g' &&
        interface[21] == 'e' &&
        interface[22] == 'r' &&
        interface[23] == '_' &&
        interface[24] == 'v' &&
        interface[25] == '1' &&
        interface[26] == 0
    ) {
        crystalWaylandDecorationManager = (zxdg_decoration_manager_v1*) wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, version < 1 ? version : 1);
        return;
    }
}

static void crystalWaylandRegistryRemoved(void* data, wl_registry* registry, uint32_t name) {

}

static const wl_registry_listener crystalWaylandRegistryListener = {
    crystalWaylandRegistryAdded,
    crystalWaylandRegistryRemoved
};

static void crystalWaylandWmBasePing(void* data, xdg_wm_base* wmBase, uint32_t serial) {
    xdg_wm_base_pong(wmBase, serial);
}

static const xdg_wm_base_listener crystalWaylandWmBaseListener = {
    crystalWaylandWmBasePing
};

static void crystalWaylandXdgSurfaceConfigure(void* data, xdg_surface* xdgSurface, uint32_t serial) {
    xdg_surface_ack_configure(xdgSurface, serial);
    CRYSTALwaylandPlatform* platform = (CRYSTALwaylandPlatform*) data;
    if (platform == 0 || platform->destroying) return;
    if (!platform->configured) platform->configured = CATALYST_TRUE;
    if (platform->buffer.wayland == 0) return; /* waiting for buffer */
    catalyst::NUINT width = platform->pending.width > 0 ? platform->pending.width : platform->buffer.width;
    catalyst::NUINT height = platform->pending.height > 0 ? platform->pending.height : platform->buffer.height;
    if (width == 0) width = CRYSTAL_DEFAULT_WIDTH;
    if (height == 0) height = CRYSTAL_DEFAULT_HEIGHT;
    if (width == platform->buffer.width && height == platform->buffer.height) return; /* no size change */

    // TODO: optimize this by resizing the existing buffer instead of creating a new one
    wl_buffer* oldWayland = platform->buffer.wayland;
    void* oldData = platform->buffer.data;
    catalyst::NUINT oldSize = platform->buffer.size;
    platform->buffer.wayland = 0;
    platform->buffer.data = 0;
    platform->buffer.width = width;
    platform->buffer.height = height;
    platform->buffer.stride = width * 4;
    platform->buffer.size = platform->buffer.stride * height;

    int shmFd = crystalWaylandCreateSharedMemoryFd(platform->buffer.size);
    if (shmFd < 0) {
        platform->buffer.wayland = oldWayland;
        platform->buffer.data = oldData;
        platform->buffer.size = oldSize;
        return;
    }

    platform->buffer.data = mmap(0, platform->buffer.size, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    if (platform->buffer.data == MAP_FAILED) {
        syscall(SYS_close, shmFd);
        platform->buffer.data = oldData;
        platform->buffer.size = oldSize;
        platform->buffer.wayland = oldWayland;
        return;
    }

    catalyst::BYTE* bytes = (catalyst::BYTE*) platform->buffer.data;
    catalyst::NUINT i = 0;
    while (i < platform->buffer.size) {
        bytes[i] = (catalyst::BYTE) 0xFF;
        i += 1;
    }

    wl_shm_pool* pool = wl_shm_create_pool(crystalWaylandShm, shmFd, (int) platform->buffer.size);
    if (pool == 0) {
        munmap(platform->buffer.data, (size_t) platform->buffer.size);
        syscall(SYS_close, shmFd);
        platform->buffer.data = oldData;
        platform->buffer.size = oldSize;
        platform->buffer.wayland = oldWayland;
        return;
    }

    platform->buffer.wayland = wl_shm_pool_create_buffer(pool, 0, (int) platform->buffer.width, (int) platform->buffer.height, (int) platform->buffer.stride, WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy(pool);
    syscall(SYS_close, shmFd);
    if (platform->buffer.wayland == 0) {
        munmap(platform->buffer.data, (size_t) platform->buffer.size);
        platform->buffer.data = oldData;
        platform->buffer.size = oldSize;
        platform->buffer.wayland = oldWayland;
        return;
    }

    wl_surface_attach(platform->wlSurface, platform->buffer.wayland, 0, 0);
    wl_surface_damage_buffer(platform->wlSurface, 0, 0, (int32_t) platform->buffer.width, (int32_t) platform->buffer.height);
    wl_surface_commit(platform->wlSurface);
    wl_display_flush(crystalWaylandDisplay);

    if (oldWayland != 0) {
        wl_buffer_destroy(oldWayland);
    }
    if (oldData != 0 && oldData != MAP_FAILED) {
        munmap(oldData, (size_t) oldSize);
    }
    if (platform->window != 0 && platform->window->resizedCallback != 0) {
        platform->window->resizedCallback(platform->window, width, height);
    }
    if (platform->window != 0 && platform->window->refreshCallback != 0) {
        platform->window->refreshCallback(platform->window);
    }
}

static const xdg_surface_listener crystalWaylandXdgSurfaceListener = {
    crystalWaylandXdgSurfaceConfigure
};

static void crystalWaylandTopLevelConfigure(void* data, xdg_toplevel* toplevel, int32_t width, int32_t height, wl_array* states) {
    CRYSTALwaylandPlatform* platform = (CRYSTALwaylandPlatform*) data;
    if (platform == 0) return;
    if (width > 0) platform->pending.width = (catalyst::NUINT) width;
    if (height > 0) platform->pending.height = (catalyst::NUINT) height;
}

static void crystalWaylandTopLevelClose(void* data, xdg_toplevel* toplevel) {
    CRYSTALwaylandPlatform* platform = (CRYSTALwaylandPlatform*) data;
    if (platform == 0 || platform->destroying) return;
    CRYSTALwindow* window = platform->window;
    if (window == 0) return;
    if (window->closingCallback != 0) {
        if (!window->closingCallback(window)) {
            return; /* close cancelled */
        }
    }
    crystalDestroyWindowWayland(window, 0);
}

static const xdg_toplevel_listener crystalWaylandToplevelListener = {
    crystalWaylandTopLevelConfigure,
    crystalWaylandTopLevelClose
};

// TODO: document opcode1 failed to connect to display
// TODO: document opcode2 failed to get registry
// TODO: document opcode3 failed to bind compositor/shm/xdg_wm_base
// TODO: document opcode4 failed to allocate CRYSTALwindow structure
// TODO: document opcode5 failed to allocate platform-specific data
// TODO: document opcode6 failed to create Wayland surface
// TODO: document opcode7 failed to create xdg-surface
// TODO: document opcode8 failed to create xdg-toplevel
// TODO: document opcode9 failed to roundtrip and get initial configure
// TODO: document opcode10 failed to create shared memory buffer
// TODO: document opcode11 failed to mmap shared memory buffer
// TODO: document opcode12 failed to create Wayland buffer
// TODO: document opcode13 failed to create shm pool for Wayland buffer
CRYSTALwindow* crystalCreateWindowWayland(CATALYST_RESULT* result) {
    // Connect to display, discover globals
    if (crystalWaylandDisplay == 0) {
        crystalWaylandDisplay = wl_display_connect(0);
        if (crystalWaylandDisplay == 0) {
            if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 1, 0);
            return 0;
        }
        crystalWaylandRegistry = wl_display_get_registry(crystalWaylandDisplay);
        if (crystalWaylandRegistry == 0) {
            wl_display_disconnect(crystalWaylandDisplay);
            crystalWaylandDisplay = 0;
            if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 2, 0);
            return 0;
        }
        wl_registry_add_listener(crystalWaylandRegistry, &crystalWaylandRegistryListener, 0);
        wl_display_roundtrip(crystalWaylandDisplay);
        if (crystalWaylandWmBase != 0) {
            xdg_wm_base_add_listener(crystalWaylandWmBase, &crystalWaylandWmBaseListener, 0);
        }
        if (crystalWaylandCompositor == 0 || crystalWaylandShm == 0 || crystalWaylandWmBase == 0) {
            if (crystalWaylandWmBase != 0) xdg_wm_base_destroy(crystalWaylandWmBase);
            if (crystalWaylandShm != 0) wl_shm_destroy(crystalWaylandShm);
            if (crystalWaylandCompositor != 0) wl_compositor_destroy(crystalWaylandCompositor);
            if (crystalWaylandRegistry != 0) wl_registry_destroy(crystalWaylandRegistry);
            wl_display_disconnect(crystalWaylandDisplay);
            crystalWaylandDisplay = 0;
            crystalWaylandWmBase = 0;
            crystalWaylandShm = 0;
            crystalWaylandCompositor = 0;
            crystalWaylandRegistry = 0;
            if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 3, 0);
            return 0;
        }
    }

    // Allocate the window object
    // TODO: this is commonly repeated code, maybe add allocWindow/freeWindow to internal.h?
    CRYSTALwindow* window = 0;
    catalyst::RESULT allocResult;
    catalyst::alloc((void**) &window, (catalyst::NUINT) sizeof(CRYSTALwindow), &allocResult);
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_ALLOCATION_FAILED, 0, 4, 0);
        return 0;
    }
    window->native.primary = 0;
    window->native.secondary = 0;
    window->native.tertiary = 0;
    window->platform = 0;
    window->erroredCallback = 0;
    window->repositionedCallback = 0;
    window->resizedCallback = 0;
    window->refreshCallback = 0;
    window->redrawCallback = 0;
    window->focusedCallback = 0;
    window->unfocusedCallback = 0;
    window->minimizedCallback = 0;
    window->maximizedCallback = 0;
    window->restoredCallback = 0;
    window->shownCallback = 0;
    window->hiddenCallback = 0;
    window->closingCallback = 0;

    // Create platform storage
    CRYSTALwaylandPlatform* platform = 0;
    catalyst::alloc((void**) &platform, (catalyst::NUINT) sizeof(CRYSTALwaylandPlatform), &allocResult);
    if (platform == 0) {
        catalyst::free(window, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_ALLOCATION_FAILED, 0, 5, allocResult.status);
        return 0;
    }
    platform->window = window;
    platform->wlSurface = 0;
    platform->xdgSurface = 0;
    platform->xdgTopLevel = 0;
    platform->xdgDecoration = 0;
    platform->buffer.wayland = 0;
    platform->buffer.data = 0;
    platform->buffer.width = 0;
    platform->buffer.height = 0;
    platform->buffer.stride = 0;
    platform->buffer.size = 0;
    platform->pending.width = 0;
    platform->pending.height = 0;
    platform->configured = CATALYST_FALSE;
    platform->destroying = CATALYST_FALSE;

    // Create the Wayland surface
    platform->wlSurface = wl_compositor_create_surface(crystalWaylandCompositor);
    if (platform->wlSurface == 0) {
        catalyst::free(platform, 0);
        catalyst::free(window, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 6, 0);
        return 0;
    }

    // Create the xdg-shell surface
    platform->xdgSurface = xdg_wm_base_get_xdg_surface(crystalWaylandWmBase, platform->wlSurface);
    if (platform->xdgSurface == 0) {
        wl_surface_destroy(platform->wlSurface);
        catalyst::free(platform, 0);
        catalyst::free(window, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 7, 0);
        return 0;
    }

    // Create the xdg-toplevel surface
    platform->xdgTopLevel = xdg_surface_get_toplevel(platform->xdgSurface);
    if (platform->xdgTopLevel == 0) {
        xdg_surface_destroy(platform->xdgSurface);
        wl_surface_destroy(platform->wlSurface);
        catalyst::free(platform, 0);
        catalyst::free(window, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 8, 0);
        return 0;
    }

    // Create the xdg-decoration for the surface
    if (crystalWaylandDecorationManager == 0) {
        // TODO: In the future, we'll add support for libdecor
        //       Until then, if the compositor doesn't support decorations,
        //       throw an error, since it's an extremely nonstandard way to
        //       run a desktop environment, and we don't want to encourage it.
        xdg_surface_destroy(platform->xdgSurface);
        wl_surface_destroy(platform->wlSurface);
        catalyst::free(platform, 0);
        catalyst::free(window, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_SYSTEM_NOT_SUPPORTED, 0, 0, 0);
        return 0;
    }
    platform->xdgDecoration = zxdg_decoration_manager_v1_get_toplevel_decoration(crystalWaylandDecorationManager, platform->xdgTopLevel);
    if (platform->xdgDecoration != 0) {
        zxdg_toplevel_decoration_v1_set_mode(platform->xdgDecoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    }

    // Attach xdg listeners and configure window
    xdg_surface_add_listener(platform->xdgSurface, &crystalWaylandXdgSurfaceListener, platform);
    xdg_toplevel_add_listener(platform->xdgTopLevel, &crystalWaylandToplevelListener, platform);
    xdg_toplevel_set_title(platform->xdgTopLevel, CRYSTAL_DEFAULT_TITLE);

    // Send an empty commit to request initial configuration
    wl_surface_commit(platform->wlSurface);
    while (!platform->configured) {
        if (wl_display_dispatch(crystalWaylandDisplay) == -1) {
            zxdg_toplevel_decoration_v1_destroy(platform->xdgDecoration);
            xdg_toplevel_destroy(platform->xdgTopLevel);
            xdg_surface_destroy(platform->xdgSurface);
            wl_surface_destroy(platform->wlSurface);
            catalyst::free(platform, 0);
            catalyst::free(window, 0);
            if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 9, 0);
            return 0;
        }
    }

    // Create a shared-memory pixel buffer for the surface (e.g. for use as a backbuffer)
    if (platform->buffer.width == 0) platform->buffer.width = CRYSTAL_DEFAULT_WIDTH;
    if (platform->buffer.height == 0) platform->buffer.height = CRYSTAL_DEFAULT_HEIGHT;
    platform->buffer.stride = platform->buffer.width * 4;
    platform->buffer.size = platform->buffer.stride * platform->buffer.height;
    int shmFd = crystalWaylandCreateSharedMemoryFd(platform->buffer.size);
    if (shmFd < 0) {
        zxdg_toplevel_decoration_v1_destroy(platform->xdgDecoration);
        xdg_toplevel_destroy(platform->xdgTopLevel);
        xdg_surface_destroy(platform->xdgSurface);
        wl_surface_destroy(platform->wlSurface);
        catalyst::free(platform, 0);
        catalyst::free(window, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 10, 0);
        return 0;
    }
    platform->buffer.data = mmap(0, platform->buffer.size, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    if (platform->buffer.data == MAP_FAILED) {
        syscall(SYS_close, shmFd);
        zxdg_toplevel_decoration_v1_destroy(platform->xdgDecoration);
        xdg_toplevel_destroy(platform->xdgTopLevel);
        xdg_surface_destroy(platform->xdgSurface);
        wl_surface_destroy(platform->wlSurface);
        catalyst::free(platform, 0);
        catalyst::free(window, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 11, 0);
        return 0;
    }

    // Initialize the buffer to opaque white
    catalyst::BYTE* bytes = (catalyst::BYTE*) platform->buffer.data;
    catalyst::NUINT i = 0;
    while (i < platform->buffer.size) {
        bytes[i] = (catalyst::BYTE) 0xFF;
        i += 1;
    }

    // Create a Wayland buffer from the shared memory
    wl_shm_pool* pool = wl_shm_create_pool(crystalWaylandShm, shmFd, (int) platform->buffer.size);
    if (pool == 0) {
        munmap(platform->buffer.data, (size_t) platform->buffer.size);
        syscall(SYS_close, shmFd);
        zxdg_toplevel_decoration_v1_destroy(platform->xdgDecoration);
        xdg_toplevel_destroy(platform->xdgTopLevel);
        xdg_surface_destroy(platform->xdgSurface);
        wl_surface_destroy(platform->wlSurface);
        catalyst::free(platform, 0);
        catalyst::free(window, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 12, 0);
        return 0;
    }
    platform->buffer.wayland = wl_shm_pool_create_buffer(pool, 0, (int) platform->buffer.width, (int) platform->buffer.height, (int) platform->buffer.stride, WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy(pool);
    syscall(SYS_close, shmFd);
    if (platform->buffer.wayland == 0) {
        munmap(platform->buffer.data, (size_t) platform->buffer.size);
        zxdg_toplevel_decoration_v1_destroy(platform->xdgDecoration);
        xdg_toplevel_destroy(platform->xdgTopLevel);
        xdg_surface_destroy(platform->xdgSurface);
        wl_surface_destroy(platform->wlSurface);
        catalyst::free(platform, 0);
        catalyst::free(window, 0);
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_DEPENDENCY_FAILURE, 0, 13, 0);
        return 0;
    }

    // Attach the buffer to the surface and commit so it will be displayed (initially all white)
    wl_surface_attach(platform->wlSurface, platform->buffer.wayland, 0, 0);
    wl_surface_damage_buffer(platform->wlSurface, 0, 0, (int32_t) platform->buffer.width, (int32_t) platform->buffer.height);
    wl_surface_commit(platform->wlSurface);
    wl_display_flush(crystalWaylandDisplay);

    // Assign native handles
    window->native.primary = (catalyst::NUINT) platform->wlSurface;
    window->native.secondary = (catalyst::NUINT) platform->xdgSurface;
    window->native.tertiary = (catalyst::NUINT) platform->xdgTopLevel;
    window->platform = platform;

    // Increment window count and return
    crystalWaylandWindowCount += 1;
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
    return window;
}

void crystalProcessEventsWayland(CATALYST_BOOL wait) {
    if (crystalWaylandDisplay == 0) return;

    // Dispatch any pending events before polling
    if (wl_display_dispatch_pending(crystalWaylandDisplay) == -1) return;

    // Prepare to read from the socket. If failed, dispatch pending events and try again
    while (wl_display_prepare_read(crystalWaylandDisplay) != 0) {
        if (wl_display_dispatch_pending(crystalWaylandDisplay) == -1) {
            return;
        }
    }

    // Poll for events
    wl_display_flush(crystalWaylandDisplay);
    pollfd descriptor;
    descriptor.fd = wl_display_get_fd(crystalWaylandDisplay);
    descriptor.events = POLLIN;
    descriptor.revents = 0;
    int timeout = wait ? -1 : 0;
    int pollResult = poll(&descriptor, 1, timeout);
    if (pollResult > 0 && (descriptor.revents & POLLIN) != 0) {
        if (wl_display_read_events(crystalWaylandDisplay) == -1) {
            return;
        }
        wl_display_dispatch_pending(crystalWaylandDisplay);
        wl_display_flush(crystalWaylandDisplay);
        return;
    }
    wl_display_cancel_read(crystalWaylandDisplay);
}

void crystalSetWindowTitleWayland(CRYSTALwindow* window, CATALYST_UTF8 title, CATALYST_RESULT* result) {

}

void crystalGetWindowTitleWayland(CRYSTALwindow* window, CATALYST_UTF8W title, CATALYST_NUINT capacity, CATALYST_NUINT* length, CATALYST_RESULT* result) {

}

void crystalSetWindowPositionWayland(CRYSTALwindow* window, CATALYST_NUINT x, CATALYST_NUINT y, CATALYST_RESULT* result) {

}

void crystalGetWindowPositionWayland(CRYSTALwindow* window, CATALYST_NUINT* x, CATALYST_NUINT* y, CATALYST_RESULT* result) {

}

void crystalSetWindowSizeWayland(CRYSTALwindow* window, CATALYST_NUINT width, CATALYST_NUINT height, CATALYST_RESULT* result) {

}

void crystalGetWindowSizeWayland(CRYSTALwindow* window, CATALYST_NUINT* width, CATALYST_NUINT* height, CATALYST_RESULT* result) {

}

void crystalSetWindowSizeLimitsWayland(CRYSTALwindow* window, CATALYST_NUINT minWidth, CATALYST_NUINT minHeight, CATALYST_NUINT maxWidth, CATALYST_NUINT maxHeight, CATALYST_RESULT* result) {

}

void crystalGetWindowSizeLimitsWayland(CRYSTALwindow* window, CATALYST_NUINT* minWidth, CATALYST_NUINT* minHeight, CATALYST_NUINT* maxWidth, CATALYST_NUINT* maxHeight, CATALYST_RESULT* result) {

}

void crystalSetWindowStyleWayland(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STYLE style, CATALYST_RESULT* result) {

}

void crystalGetWindowStyleWayland(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STYLE* style, CATALYST_RESULT* result) {

}

void crystalRequestWindowFocusWayland(CRYSTALwindow* window, CATALYST_RESULT* result) {

}

void crystalRequestWindowAttentionWayland(CRYSTALwindow* window, CATALYST_RESULT* result) {

}

void crystalMinimizeWindowWayland(CRYSTALwindow* window, CATALYST_RESULT* result) {

}

void crystalMaximizeWindowWayland(CRYSTALwindow* window, CATALYST_RESULT* result) {

}

void crystalRestoreWindowWayland(CRYSTALwindow* window, CATALYST_RESULT* result) {

}

void crystalShowWindowWayland(CRYSTALwindow* window, CATALYST_RESULT* result) {

}

void crystalHideWindowWayland(CRYSTALwindow* window, CATALYST_RESULT* result) {

}

void crystalGetWindowStateWayland(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STATE* state) {

}

void crystalCloseWindowWayland(CRYSTALwindow* window, CATALYST_RESULT* result) {

}

void crystalDestroyWindowWayland(CRYSTALwindow* window, CATALYST_RESULT* result) {
    if (window == 0) {
        if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_ERROR_INVALID_ARGUMENT, 0, 0, 0);
        return;
    }

    CRYSTALwaylandPlatform* platform = (CRYSTALwaylandPlatform*) window->platform;
    if (platform != 0) {
        if (!platform->destroying) {
            platform->destroying = CATALYST_TRUE;

            // Remove pending configure state so it won't be applied
            platform->pending.width = 0;
            platform->pending.height = 0;

            // Destroy the Wayland buffer
            if (platform->buffer.wayland != 0) {
                wl_buffer_destroy(platform->buffer.wayland);
                platform->buffer.wayland = 0;
            }

            // Unmap the shared memory buffer
            if (platform->buffer.data != 0 && platform->buffer.data != MAP_FAILED) {
                munmap(platform->buffer.data, (size_t) platform->buffer.size);
                platform->buffer.data = 0;
            }
            platform->buffer.width = 0;
            platform->buffer.height = 0;
            platform->buffer.stride = 0;
            platform->buffer.size = 0;

            // Destroy the xdg-decoration
            if (platform->xdgDecoration != 0) {
                zxdg_toplevel_decoration_v1_destroy(platform->xdgDecoration);
                platform->xdgDecoration = 0;
            }

            // Destroy the xdg-toplevel
            if (platform->xdgTopLevel != 0) {
                xdg_toplevel_destroy(platform->xdgTopLevel);
                platform->xdgTopLevel = 0;
            }

            // Destroy the xdg-surface
            if (platform->xdgSurface != 0) {
                xdg_surface_destroy(platform->xdgSurface);
                platform->xdgSurface = 0;
            }

            // Destroy the Wayland surface
            if (platform->wlSurface != 0) {
                wl_surface_destroy(platform->wlSurface);
                platform->wlSurface = 0;
            }

            // Flush the display
            if (crystalWaylandDisplay != 0) {
                wl_display_flush(crystalWaylandDisplay);
            }
        }

        // Free the platform object
        catalyst::free(platform, 0);
        window->platform = 0;
    }

    // Clear native handles
    window->native.primary = 0;
    window->native.secondary = 0;
    window->native.tertiary = 0;

    // Detach callbacks
    window->erroredCallback = 0;
    window->repositionedCallback = 0;
    window->resizedCallback = 0;
    window->refreshCallback = 0;
    window->redrawCallback = 0;
    window->focusedCallback = 0;
    window->unfocusedCallback = 0;
    window->minimizedCallback = 0;
    window->maximizedCallback = 0;
    window->restoredCallback = 0;
    window->shownCallback = 0;
    window->hiddenCallback = 0;
    window->closingCallback = 0;

    // Free the window and decrement window count
    catalyst::free(window, 0);
    if (crystalWaylandWindowCount != 0) crystalWaylandWindowCount -= 1;

    // If there's no more windows, disconnect from the display and release globals
    if (crystalWaylandWindowCount == 0 && crystalWaylandDisplay != 0) {
        if (crystalWaylandDecorationManager != 0) {
            zxdg_decoration_manager_v1_destroy(crystalWaylandDecorationManager);
            crystalWaylandDecorationManager = 0;
        }
        if (crystalWaylandWmBase != 0) {
            xdg_wm_base_destroy(crystalWaylandWmBase);
            crystalWaylandWmBase = 0;
        }
        if (crystalWaylandShm != 0) {
            wl_shm_destroy(crystalWaylandShm);
            crystalWaylandShm = 0;
        }
        if (crystalWaylandCompositor != 0) {
            wl_compositor_destroy(crystalWaylandCompositor);
            crystalWaylandCompositor = 0;
        }
        if (crystalWaylandRegistry != 0) {
            wl_registry_destroy(crystalWaylandRegistry);
            crystalWaylandRegistry = 0;
        }
        wl_display_flush(crystalWaylandDisplay);
        wl_display_disconnect(crystalWaylandDisplay);
        crystalWaylandDisplay = 0;
    }
    if (result != 0) *result = catalyst::RESULT(catalyst::STATUS_CODE_SUCCESS, 0, 0, 0);
}

#if TARGET_SHARED
extern "C" CRYSTALwindow* crystalCreateWindow(CATALYST_RESULT* result) {
    return crystalCreateWindowWayland(result);
}

extern "C" void crystalProcessEvents(CATALYST_BOOL wait) {
    crystalProcessEventsWayland(wait);
}

extern "C" void crystalSetWindowTitle(CRYSTALwindow* window, CATALYST_UTF8 title, CATALYST_RESULT* result) {
    crystalSetWindowTitleWayland(window, title, result);
}

extern "C" void crystalGetWindowTitle(CRYSTALwindow* window, CATALYST_UTF8W title, CATALYST_NUINT capacity, CATALYST_NUINT* length, CATALYST_RESULT* result) {
    crystalGetWindowTitleWayland(window, title, capacity, length, result);
}

extern "C" void crystalSetWindowPosition(CRYSTALwindow* window, CATALYST_NUINT x, CATALYST_NUINT y, CATALYST_RESULT* result) {
    crystalSetWindowPositionWayland(window, x, y, result);
}

extern "C" void crystalGetWindowPosition(CRYSTALwindow* window, CATALYST_NUINT* x, CATALYST_NUINT* y, CATALYST_RESULT* result) {
    crystalGetWindowPositionWayland(window, x, y, result);
}

extern "C" void crystalSetWindowSize(CRYSTALwindow* window, CATALYST_NUINT width, CATALYST_NUINT height, CATALYST_RESULT* result) {
    crystalSetWindowSizeWayland(window, width, height, result);
}

extern "C" void crystalGetWindowSize(CRYSTALwindow* window, CATALYST_NUINT* width, CATALYST_NUINT* height, CATALYST_RESULT* result) {
    crystalGetWindowSizeWayland(window, width, height, result);
}

extern "C" void crystalSetWindowSizeLimits(CRYSTALwindow* window, CATALYST_NUINT minWidth, CATALYST_NUINT minHeight, CATALYST_NUINT maxWidth, CATALYST_NUINT maxHeight, CATALYST_RESULT* result) {
    crystalSetWindowSizeLimitsWayland(window, minWidth, minHeight, maxWidth, maxHeight, result);
}

extern "C" void crystalGetWindowSizeLimits(CRYSTALwindow* window, CATALYST_NUINT* minWidth, CATALYST_NUINT* minHeight, CATALYST_NUINT* maxWidth, CATALYST_NUINT* maxHeight, CATALYST_RESULT* result) {
    crystalGetWindowSizeLimitsWayland(window, minWidth, minHeight, maxWidth, maxHeight, result);
}

extern "C" void crystalSetWindowStyle(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STYLE style, CATALYST_RESULT* result) {
    crystalSetWindowStyleWayland(window, style, result);
}

extern "C" void crystalGetWindowStyle(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STYLE* style, CATALYST_RESULT* result) {
    crystalGetWindowStyleWayland(window, style, result);
}

extern "C" void crystalRequestWindowFocus(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalRequestWindowFocusWayland(window, result);
}

extern "C" void crystalRequestWindowAttention(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalRequestWindowAttentionWayland(window, result);
}

extern "C" void crystalMinimizeWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalMinimizeWindowWayland(window, result);
}

extern "C" void crystalMaximizeWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalMaximizeWindowWayland(window, result);
}

extern "C" void crystalRestoreWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalRestoreWindowWayland(window, result);
}

extern "C" void crystalShowWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalShowWindowWayland(window, result);
}

extern "C" void crystalHideWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalHideWindowWayland(window, result);
}

extern "C" void crystalGetWindowState(CRYSTALwindow* window, CRYSTAL_PROPERTIES_STATE* state) {
    crystalGetWindowStateWayland(window, state);
}

extern "C" void crystalCloseWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalCloseWindowWayland(window, result);
}

extern "C" void crystalDestroyWindow(CRYSTALwindow* window, CATALYST_RESULT* result) {
    crystalDestroyWindowWayland(window, result);
}
#endif /* TARGET_SHARED */

#endif /* TARGET_PLATFORM_LINUX */
