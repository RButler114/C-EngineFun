/**
 * @file Renderer.cpp
 * @brief Implementation of 2D rendering system with texture caching
 * @author Ryan Butler
 * @date 2025
 */

#include "Engine/Renderer.h"
#include "Engine/ConfigSystem.h"
#include <iostream>

// ========== TEXTURE CLASS IMPLEMENTATION ==========

/**
 * @brief Default constructor - initializes texture to safe default state
 *
 * Creates an empty texture object with no SDL texture allocated.
 * The actual texture loading happens in LoadFromFile().
 *
 * @note Follows RAII principles - lightweight construction
 */
Texture::Texture() : m_texture(nullptr), m_width(0), m_height(0) {}

/**
 * @brief Destructor - ensures proper cleanup of SDL texture
 *
 * Automatically calls Free() to release the SDL texture and
 * associated video memory. This prevents memory leaks.
 *
 * @note Safe to call even if LoadFromFile() was never called
 */
Texture::~Texture() {
    Free(); // Automatic cleanup via RAII
}

/**
 * @brief Load texture from image file
 *
 * Loads an image file and creates an SDL texture for rendering.
 * Supports common formats: PNG, JPG, BMP, GIF, etc.
 *
 * Process:
 * 1. Free any existing texture (allows reloading)
 * 2. Load image file into SDL surface (CPU memory)
 * 3. Convert surface to texture (GPU memory)
 * 4. Store dimensions for later use
 * 5. Free temporary surface
 *
 * @param path File path to image (relative to executable or absolute)
 * @param renderer SDL renderer to create texture with
 *
 * @return true if texture loaded successfully, false on failure
 *
 * @note Automatically frees any existing texture before loading new one
 * @note Texture dimensions are stored and accessible via GetWidth()/GetHeight()
 * @note Supports transparency (PNG alpha channel)
 *
 * Common failure causes:
 * - File not found or inaccessible
 * - Unsupported image format
 * - Insufficient video memory
 * - Corrupted image file
 */
bool Texture::LoadFromFile(const std::string& path, SDL_Renderer* renderer) {
    // Free any existing texture to prevent memory leaks
    Free();

    // Load image file into SDL surface (CPU memory)
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (!loadedSurface) {
        std::cerr << "❌ Failed to load image: " << path << std::endl;
        std::cerr << "   SDL_image Error: " << IMG_GetError() << std::endl;
        std::cerr << "   Check file path and format (PNG, JPG, BMP supported)" << std::endl;
        return false;
    }

    // Convert surface to hardware-accelerated texture (GPU memory)
    m_texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    if (!m_texture) {
        std::cerr << "❌ Failed to create texture from: " << path << std::endl;
        std::cerr << "   SDL Error: " << SDL_GetError() << std::endl;
        std::cerr << "   Check video memory and graphics drivers" << std::endl;
    } else {
        // Store texture dimensions for rendering calculations
        m_width = loadedSurface->w;
        m_height = loadedSurface->h;
        std::cout << "✅ Texture loaded: " << path << " (" << m_width << "x" << m_height << ")" << std::endl;
    }

    // Free temporary surface (no longer needed after texture creation)
    SDL_FreeSurface(loadedSurface);

    return m_texture != nullptr;
}

/**
 * @brief Release texture resources and reset state
 *
 * Safely destroys the SDL texture and resets all member variables
 * to their default state. This releases video memory.
 *
 * @note Safe to call multiple times (checks for null pointer)
 * @note Called automatically by destructor
 * @note After this call, the texture cannot be rendered until reloaded
 */
void Texture::Free() {
    if (m_texture) {
        SDL_DestroyTexture(m_texture);  // Release video memory
        m_texture = nullptr;            // Prevent double-free
        m_width = 0;                    // Reset dimensions
        m_height = 0;
    }
}

/**
 * @brief Render texture at specified position with optional clipping
 *
 * Draws the texture at the given screen coordinates. If a clip rectangle
 * is provided, only that portion of the texture is drawn.
 *
 * @param renderer SDL renderer to draw with
 * @param x Screen X coordinate to draw at
 * @param y Screen Y coordinate to draw at
 * @param clip Optional source rectangle to clip from texture (nullptr = full texture)
 *
 * @note If clip is provided, the rendered size matches the clip size
 * @note If clip is nullptr, the full texture is rendered at original size
 * @note Useful for sprite sheets and texture atlases
 */
void Texture::Render(SDL_Renderer* renderer, int x, int y, SDL_Rect* clip) {
    // Set up destination rectangle (where to draw on screen)
    SDL_Rect renderQuad = { x, y, m_width, m_height };

    // If clipping, adjust destination size to match clip size
    if (clip) {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }

    // Render texture (or clipped portion) to screen
    SDL_RenderCopy(renderer, m_texture, clip, &renderQuad);
}

/**
 * @brief Render texture with custom size and optional clipping
 *
 * Draws the texture at the given position and scales it to the specified
 * dimensions. If a clip rectangle is provided, only that portion is drawn.
 *
 * @param renderer SDL renderer to draw with
 * @param x Screen X coordinate to draw at
 * @param y Screen Y coordinate to draw at
 * @param width Desired width of rendered texture
 * @param height Desired height of rendered texture
 * @param clip Optional source rectangle to clip from texture (nullptr = full texture)
 *
 * @note Texture is scaled to fit the specified width/height
 * @note Useful for scaling sprites or UI elements
 * @note Maintains aspect ratio only if width/height ratio matches original
 */
void Texture::Render(SDL_Renderer* renderer, int x, int y, int width, int height, SDL_Rect* clip) {
    // Set up destination rectangle with custom dimensions
    SDL_Rect renderQuad = { x, y, width, height };

    // Render texture (or clipped portion) scaled to destination size
    SDL_RenderCopy(renderer, m_texture, clip, &renderQuad);
}

// ========== RENDERER CLASS IMPLEMENTATION ==========

/**
 * @brief Default constructor - initializes renderer to safe default state
 *
 * Creates an empty renderer object with no SDL renderer allocated.
 * The actual renderer creation happens in Initialize().
 *
 * @note Follows RAII principles - lightweight construction
 */
Renderer::Renderer() : m_renderer(nullptr) {}

/**
 * @brief Destructor - ensures proper cleanup of renderer and resources
 *
 * Automatically calls Shutdown() to clean up the SDL renderer,
 * texture cache, and SDL_image subsystem.
 *
 * @note Safe to call even if Initialize() was never called
 */
Renderer::~Renderer() {
    Shutdown(); // Automatic cleanup via RAII
}

/**
 * @brief Initialize the SDL renderer and image loading systems
 *
 * Creates a hardware-accelerated SDL renderer with VSync enabled and
 * initializes SDL_image for loading various image formats.
 *
 * Features enabled:
 * - Hardware acceleration (GPU rendering)
 * - VSync (prevents screen tearing)
 * - Alpha blending (transparency support)
 * - PNG and JPG image loading
 *
 * @param window SDL window to create renderer for
 *
 * @return true if initialization successful, false on failure
 *
 * @note Window must be created before calling this
 * @note Renderer is configured for optimal 2D game performance
 * @note Texture cache is automatically initialized
 *
 * Common failure causes:
 * - Graphics drivers don't support hardware acceleration
 * - Insufficient video memory
 * - Invalid or destroyed window
 * - SDL_image library missing
 */
bool Renderer::Initialize(SDL_Window* window) {
    // Create hardware-accelerated renderer with VSync
    // Read VSync preference from config
    ConfigManager cfgVs; bool vsync = true;
    if (cfgVs.LoadFromFile("assets/config/gameplay.ini")) {
        vsync = cfgVs.Get("visual", "vsync", true).AsBool();
    }
    Uint32 flags = SDL_RENDERER_ACCELERATED | (vsync ? SDL_RENDERER_PRESENTVSYNC : 0);
    m_renderer = SDL_CreateRenderer(
        window,                                           // Target window
        -1,                                              // Use first available driver
        flags
    );

    if (!m_renderer) {
        std::cerr << "❌ Renderer creation failed! SDL Error: " << SDL_GetError() << std::endl;
        std::cerr << "   Try updating graphics drivers or disable hardware acceleration" << std::endl;
        return false;
    }

    // Initialize SDL_image for loading PNG, JPG, and other formats
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "❌ SDL_image initialization failed! Error: " << IMG_GetError() << std::endl;
        std::cerr << "   Image loading will be limited to BMP format only" << std::endl;
        return false;
    }

    // Enable alpha blending for transparency support
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

    // Configure logical (virtual) resolution and integer scaling
    // Read from gameplay.ini [visual] section with sensible defaults
    ConfigManager cfg;
    int logicalW = 800; int logicalH = 600; bool integerScale = true;
    if (cfg.LoadFromFile("assets/config/gameplay.ini")) {
        logicalW = cfg.Get("visual", "logical_width", logicalW).AsInt();
        logicalH = cfg.Get("visual", "logical_height", logicalH).AsInt();
        integerScale = cfg.Get("visual", "integer_scale", integerScale).AsBool();
    }
    SDL_RenderSetLogicalSize(m_renderer, logicalW, logicalH);
    SDL_RenderSetIntegerScale(m_renderer, integerScale ? SDL_TRUE : SDL_FALSE);

    std::cout << "✅ Hardware-accelerated renderer initialized with VSync" << std::endl;
    std::cout << "✅ Image loading support: PNG, JPG, BMP" << std::endl;
    std::cout << "✅ Logical size: " << logicalW << "x" << logicalH << ", integerScale=" << (integerScale?"true":"false") << std::endl;
    return true;
}

void Renderer::GetLogicalSize(int& w, int& h) const {
    if (!m_renderer) { w = 0; h = 0; return; }
    SDL_RenderGetLogicalSize(m_renderer, &w, &h);
}



void Renderer::UpdateLogicalToOutput() {
    if (!m_renderer) return;
    ConfigManager cfg;
    bool match = false;
    int defaultLW=1280, defaultLH=720; bool integerScale=false;
    if (cfg.LoadFromFile("assets/config/gameplay.ini")) {
        match = cfg.Get("visual","logical_match_output", false).AsBool();
        defaultLW = cfg.Get("visual","logical_width", defaultLW).AsInt();
        defaultLH = cfg.Get("visual","logical_height", defaultLH).AsInt();
        integerScale = cfg.Get("visual","integer_scale", integerScale).AsBool();
    }

    if (match) {
        int outW=0, outH=0; SDL_GetRendererOutputSize(m_renderer, &outW, &outH);
        if (outW > 0 && outH > 0) {
            SDL_RenderSetLogicalSize(m_renderer, outW, outH);
            SDL_RenderSetIntegerScale(m_renderer, integerScale ? SDL_TRUE : SDL_FALSE);
            std::cout << "✅ Logical size matched to output: " << outW << "x" << outH << std::endl;
            return;
        }
    }
    SDL_RenderSetLogicalSize(m_renderer, defaultLW, defaultLH);
    SDL_RenderSetIntegerScale(m_renderer, integerScale ? SDL_TRUE : SDL_FALSE);
}

/**
 * @brief Clean up renderer and all associated resources
 *
 * Safely shuts down the rendering system:

    std::cout << "✅ Hardware-accelerated renderer initialized with VSync" << std::endl;
    std::cout << "✅ Image loading support: PNG, JPG, BMP" << std::endl;
    std::cout << "✅ Logical size: " << logicalW << "x" << logicalH << ", integerScale=" << (integerScale?"true":"false") << std::endl;
    return true;
}

 * 1. Clear texture cache (releases all loaded textures)
 * 2. Destroy SDL renderer (releases graphics context)
 * 3. Shutdown SDL_image (cleanup image loading system)
 *
 * @note Safe to call multiple times (checks for null pointers)
 * @note Called automatically by destructor
 * @note All textures in cache are automatically freed
 */
void Renderer::Shutdown() {
    // Clear texture cache first (releases all loaded textures)
    m_textureCache.clear();
    std::cout << "✅ Texture cache cleared" << std::endl;

    // Destroy SDL renderer (releases graphics context and resources)
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
        std::cout << "✅ Renderer destroyed" << std::endl;
    }

    // Shutdown SDL_image subsystem
    IMG_Quit();
    std::cout << "✅ SDL_image shut down" << std::endl;
}

/**
 * @brief Clear the screen with specified background color
 *
 * Fills the entire screen buffer with the given color, preparing
 * for the next frame to be drawn. This should be called at the
 * beginning of each frame before drawing anything else.

 */

/**
 * Draw black bars to cover letterbox/pillarbox areas if present.
 * Should be called after Clear() and before Present(), if desired.
 */
void Renderer::DrawLetterboxBars(int logicalW, int logicalH) {
    // Compute current output size
    int outW = 0, outH = 0;
    SDL_GetRendererOutputSize(m_renderer, &outW, &outH);
    if (outW <= 0 || outH <= 0 || logicalW <= 0 || logicalH <= 0) return;

    // Determine scaled viewport maintaining aspect ratio
    float scale = std::min(outW / (float)logicalW, outH / (float)logicalH);
    int vpW = (int)(logicalW * scale);
    int vpH = (int)(logicalH * scale);
    int vpX = (outW - vpW) / 2;
    int vpY = (outH - vpH) / 2;

    // Top bar
    if (vpY > 0) {
        SDL_Rect top{0, 0, outW, vpY};
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(m_renderer, &top);
    }
    // Bottom bar
    if (vpY + vpH < outH) {
        SDL_Rect bottom{0, vpY + vpH, outW, outH - (vpY + vpH)};
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(m_renderer, &bottom);
    }
    // Left bar
    if (vpX > 0) {
        SDL_Rect left{0, vpY, vpX, vpH};
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(m_renderer, &left);
    }
    // Right bar
    if (vpX + vpW < outW) {
        SDL_Rect right{vpX + vpW, vpY, outW - (vpX + vpW), vpH};
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(m_renderer, &right);
    }
}

/**
 * @brief Clear the screen with specified background color
 * @param color Background color to clear with (default: black)
 * @note Called automatically by Engine::Run() before Render()
 * @note The cleared buffer is not visible until Present() is called
 * @note Alpha component affects blending with previous frame content
 */

void Renderer::Clear(const Color& color) {
    // Set the clear color for this frame
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);

    // Clear the entire screen buffer with the specified color
    SDL_RenderClear(m_renderer);
}

/**
 * @brief Display the completed frame to the screen
 *
 * Swaps the back buffer (where drawing occurs) with the front buffer
 * (what's displayed on screen). This makes all drawing operations
 * from the current frame visible to the user.
 *
 * @note Called automatically by Engine::Run() after Render()
 * @note This is where VSync takes effect (if enabled)
 * @note Double buffering prevents flickering and tearing
 */
void Renderer::Present() {
    // Swap buffers and display the completed frame
    SDL_RenderPresent(m_renderer);
}

/**
 * @brief Draw a rectangle (filled or outline)
 *
 * Draws a rectangle at the specified position with the given color.
 * Can be drawn as a filled rectangle or just an outline.
 *
 * @param rect Rectangle defining position and size
 * @param color Color to draw with (supports transparency)
 * @param filled true for filled rectangle, false for outline only
 *
 * @note Useful for UI elements, collision visualization, backgrounds
 * @note Alpha blending is supported for transparency effects
 * @note Coordinates are in screen space (pixels)
 */
void Renderer::DrawRectangle(const Rectangle& rect, const Color& color, bool filled) {
    // Set drawing color (including alpha for transparency)
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);

    // Convert our Rectangle to SDL_Rect format
    SDL_Rect sdlRect = { rect.x, rect.y, rect.width, rect.height };

    if (filled) {
        // Draw filled rectangle
        SDL_RenderFillRect(m_renderer, &sdlRect);
    } else {
        // Draw rectangle outline only
        SDL_RenderDrawRect(m_renderer, &sdlRect);
    }
}

/**
 * @brief Draw a line between two points
 *
 * Draws a straight line from point (x1, y1) to point (x2, y2)
 * with the specified color.
 *
 * @param x1 Starting X coordinate
 * @param y1 Starting Y coordinate
 * @param x2 Ending X coordinate
 * @param y2 Ending Y coordinate
 * @param color Line color (supports transparency)
 *
 * @note Useful for debug visualization, UI elements, simple graphics
 * @note Line width is always 1 pixel
 * @note Alpha blending is supported
 */
void Renderer::DrawLine(int x1, int y1, int x2, int y2, const Color& color) {
    // Set drawing color
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);

    // Draw line from start to end point
    SDL_RenderDrawLine(m_renderer, x1, y1, x2, y2);
}

/**
 * @brief Draw a single pixel point
 *
 * Draws a single pixel at the specified coordinates with the given color.
 *
 * @param x X coordinate of the point
 * @param y Y coordinate of the point
 * @param color Point color (supports transparency)
 *
 * @note Useful for particle effects, debug markers, pixel art
 * @note Point is exactly 1x1 pixel in size
 * @note Alpha blending is supported
 */
void Renderer::DrawPoint(int x, int y, const Color& color) {
    // Set drawing color
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);

    // Draw single pixel point
    SDL_RenderDrawPoint(m_renderer, x, y);
}

/**
 * @brief Load texture from file with automatic caching
 *
 * Loads a texture from an image file and stores it in the texture cache
 * for efficient reuse. If the texture is already loaded, returns the
 * cached version instead of loading it again.
 *
 * Caching benefits:
 * - Prevents duplicate loading of the same texture
 * - Reduces memory usage and loading times
 * - Automatic memory management via shared_ptr
 * - Thread-safe reference counting
 *
 * @param path File path to image (relative to executable or absolute)
 *
 * @return Shared pointer to loaded texture, or nullptr on failure
 *
 * @note Textures remain in cache until renderer is destroyed
 * @note Multiple objects can safely share the same texture
 * @note Supports PNG, JPG, BMP, and other SDL_image formats
 *
 * @example
 * ```cpp
 * auto playerTexture = renderer.LoadTexture("assets/player.png");
 * auto enemyTexture = renderer.LoadTexture("assets/enemy.png");
 * // Loading the same texture again returns cached version
 * auto playerTexture2 = renderer.LoadTexture("assets/player.png"); // Same as playerTexture
 * ```
 */
std::shared_ptr<Texture> Renderer::LoadTexture(const std::string& path) {
    // Check if texture is already cached (avoid duplicate loading)
    auto it = m_textureCache.find(path);
    if (it != m_textureCache.end()) {
        // Return cached texture (efficient reuse)
        return it->second;
    }

    // Create new texture and attempt to load from file
    auto texture = std::make_shared<Texture>();
    if (texture->LoadFromFile(path, m_renderer)) {
        // Loading successful - add to cache for future use
        m_textureCache[path] = texture;
        return texture;
    }

    // Loading failed - return null pointer
    std::cerr << "❌ Failed to load texture: " << path << std::endl;
    return nullptr;
}

/**
 * @brief Draw texture at specified position (original size)
 *
 * Draws the entire texture at the given screen coordinates using
 * the texture's original dimensions.
 *
 * @param texture Shared pointer to texture to draw
 * @param x Screen X coordinate to draw at
 * @param y Screen Y coordinate to draw at
 *
 * @note Does nothing if texture is null (safe to call)
 * @note Texture is drawn at its original size
 * @note Supports transparency if texture has alpha channel
 */
void Renderer::DrawTexture(std::shared_ptr<Texture> texture, int x, int y) {
    if (texture) {
        texture->Render(m_renderer, x, y);
    }
}

/**
 * @brief Draw portion of texture with scaling
 *
 * Draws a specific rectangular region from the source texture,
 * scaled to fit the destination rectangle. Useful for sprite sheets,
 * texture atlases, and UI scaling.
 *
 * @param texture Shared pointer to texture to draw
 * @param srcRect Source rectangle (which part of texture to draw)
 * @param destRect Destination rectangle (where and how big to draw)
 *
 * @note Does nothing if texture is null (safe to call)
 * @note Source rectangle is in texture coordinates
 * @note Destination rectangle is in screen coordinates
 * @note Automatic scaling if source and destination sizes differ
 *
 * @example
 * ```cpp
 * // Draw 32x32 sprite from sprite sheet at position (64, 0) to screen at (100, 100)
 * Rectangle src(64, 0, 32, 32);      // Source: 32x32 sprite at (64,0) in texture
 * Rectangle dest(100, 100, 64, 64);  // Destination: 64x64 on screen (2x scale)
 * renderer.DrawTexture(spriteSheet, src, dest);
 * ```
 */
void Renderer::DrawTexture(std::shared_ptr<Texture> texture, const Rectangle& srcRect, const Rectangle& destRect) {
    if (texture) {
        // Convert our Rectangle to SDL_Rect for source clipping
        SDL_Rect src = { srcRect.x, srcRect.y, srcRect.width, srcRect.height };

        // Render with scaling (source rect -> destination size)
        texture->Render(m_renderer, destRect.x, destRect.y, destRect.width, destRect.height, &src);
    }
}

/**
 * @brief Draw texture portion with scaling and flipping
 *
 * Advanced texture drawing with support for horizontal and vertical flipping.
 * Perfect for character sprites that need to face different directions or
 * special effects that require mirroring.
 *
 * @param texture Shared pointer to texture to draw
 * @param srcRect Source rectangle (which part of texture to draw)
 * @param destRect Destination rectangle (where and how big to draw)
 * @param flipHorizontal true to flip horizontally (mirror left-right)
 * @param flipVertical true to flip vertically (mirror top-bottom)
 *
 * @note Does nothing if texture is null (safe to call)
 * @note Flipping is applied after scaling
 * @note Can combine horizontal and vertical flipping
 * @note No performance penalty for flipping (hardware accelerated)
 *
 * @example
 * ```cpp
 * // Draw player sprite facing left (flipped horizontally)
 * Rectangle src(0, 0, 32, 32);       // Full 32x32 sprite
 * Rectangle dest(playerX, playerY, 32, 32); // Screen position
 * renderer.DrawTexture(playerTexture, src, dest, true, false); // Flip horizontal only
 * ```
 */
void Renderer::DrawTexture(std::shared_ptr<Texture> texture, const Rectangle& srcRect, const Rectangle& destRect, bool flipHorizontal, bool flipVertical) {
    if (!texture) return;

    // Convert rectangles to SDL format
    SDL_Rect src = { srcRect.x, srcRect.y, srcRect.width, srcRect.height };
    SDL_Rect dest = { destRect.x, destRect.y, destRect.width, destRect.height };

    // Determine flip flags based on parameters
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (flipHorizontal) flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_HORIZONTAL);
    if (flipVertical) flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_VERTICAL);

    // Use SDL_RenderCopyEx for advanced rendering with flipping support
    SDL_RenderCopyEx(m_renderer, texture->GetSDLTexture(), &src, &dest, 0.0, nullptr, flip);
}
