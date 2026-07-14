// SPDX-License-Identifier: MIT
//
// tiled_demo: an end-to-end cna-extended example combining several ported modules the way a
// real 2D game would use them together:
//
//  - Tilemaps::Tiled::TiledTmxParser + TilemapFactory (via ParseFromFile) load a real,
//    hand-authored Tiled TMX map (assets/map.tmx), referencing a real external tileset image
//    next to it (assets/tileset.bmp) resolved through the default local-filesystem
//    ExternalResourceResolver, exactly like a real Tiled-authored map. The map's object layer
//    is also read back (TilemapObjectLayer/TilemapObject) to find where the player should
//    spawn and where it's headed, instead of hardcoding those positions in this file.
//  - Tilemaps::Rendering::TilemapSpriteBatchRenderer renders the map's tile layers through a
//    real SpriteBatch.
//  - Graphics::Texture2DAtlas / SpriteSheet / SpriteSheetAnimationBuilder / AnimatedSprite (the
//    latter driven by the Animations module's AnimationController under the hood) animate a
//    player sprite: a hand-authored 3-frame walk cycle (assets/player.bmp), ping-ponging frames
//    over simulated time via AnimatedSprite::Update.
//  - Input::KeyboardStateExtended drives the player's per-frame movement from a scripted
//    sequence of synthetic KeyboardState snapshots, edge-detected via WasKeyPressed/
//    WasKeyReleased. Input::InputListeners::KeyboardListener is separately constructed,
//    subscribed to, and polled every frame too, to demonstrate that event-driven listener API --
//    see the loop below for why it never actually raises an event in this particular run.
//  - OrthographicCamera provides the view/projection transform both renderers draw through.
//
// Like this project's own GraphicsDevice-backed integration tests (see
// tests/CNA/Extended/Tilemaps/Rendering/TilemapIntegrationTests.cpp, whose RenderToPixels
// helper this file's own RenderToPixels mirrors), this runs headlessly: a real GraphicsDevice
// plus an off-screen RenderTarget2D, read back via GraphicsDevice::GetBackBufferData (not
// RenderTarget2D::GetData -- see RenderToPixels's comment below for why), no real window. Each
// simulated frame prints the player's tile position and a pixel sampled from the actual
// rendered framebuffer at the player's screen location, proving real drawing happened rather
// than merely "didn't throw"; the final frame is also saved as a PNG so a human can look at it.
#include <CNA/Extended/Content/ExternalResourceResolvers.hpp>
#include <CNA/Extended/Graphics/AnimatedSprite.hpp>
#include <CNA/Extended/Graphics/SpriteBatchExtensions.hpp>
#include <CNA/Extended/Graphics/SpriteSheet.hpp>
#include <CNA/Extended/Graphics/SpriteSheetAnimationBuilder.hpp>
#include <CNA/Extended/Graphics/Texture2DAtlas.hpp>
#include <CNA/Extended/Input/InputListeners/KeyboardEventArgs.hpp>
#include <CNA/Extended/Input/InputListeners/KeyboardListener.hpp>
#include <CNA/Extended/Input/KeyboardStateExtended.hpp>
#include <CNA/Extended/OrthographicCamera.hpp>
#include <CNA/Extended/Tilemaps/Rendering/TilemapSpriteBatchRenderer.hpp>
#include <CNA/Extended/Tilemaps/Tiled/TiledTmxParser.hpp>
#include <CNA/Extended/Tilemaps/TilemapObjectLayer.hpp>

#include <Microsoft/Xna/Framework/Color.hpp>
#include <Microsoft/Xna/Framework/GameTime.hpp>
#include <Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp>
#include <Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp>
#include <Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp>
#include <Microsoft/Xna/Framework/Graphics/Texture2D.hpp>
#include <Microsoft/Xna/Framework/Graphics/Viewport.hpp>
#include <Microsoft/Xna/Framework/Input/KeyboardState.hpp>
#include <Microsoft/Xna/Framework/Rectangle.hpp>
#include <Microsoft/Xna/Framework/Input/Keys.hpp>
#include <Microsoft/Xna/Framework/Vector2.hpp>
#include <System/IO/Stream.hpp>
#include <System/Object.hpp>
#include <System/TimeSpan.hpp>

#include <array>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using CNA::Extended::OrthographicCamera;
using CNA::Extended::Content::OpenFile;
using CNA::Extended::Graphics::AnimatedSprite;
using CNA::Extended::Graphics::SpriteSheet;
using CNA::Extended::Graphics::SpriteSheetAnimationBuilder;
using CNA::Extended::Graphics::Texture2DAtlas;
using CNA::Extended::Input::KeyboardStateExtended;
using CNA::Extended::Input::InputListeners::KeyboardEventArgs;
using CNA::Extended::Input::InputListeners::KeyboardListener;
using CNA::Extended::Tilemaps::Tilemap;
using CNA::Extended::Tilemaps::TilemapObjectLayer;
using CNA::Extended::Tilemaps::Rendering::TilemapSpriteBatchRenderer;
using CNA::Extended::Tilemaps::Tiled::TiledTmxParser;
using Microsoft::Xna::Framework::Color;
using Microsoft::Xna::Framework::GameTime;
using Microsoft::Xna::Framework::Rectangle;
using Microsoft::Xna::Framework::Vector2;
using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
using Microsoft::Xna::Framework::Graphics::RenderTarget2D;
using Microsoft::Xna::Framework::Graphics::SpriteBatch;
using Microsoft::Xna::Framework::Graphics::Texture2D;
using Microsoft::Xna::Framework::Graphics::Viewport;
using Microsoft::Xna::Framework::Input::KeyboardState;
using Microsoft::Xna::Framework::Input::Keys;
using System::TimeSpan;

namespace
{
    constexpr int kMapPixelWidth = 10 * 32;
    constexpr int kMapPixelHeight = 6 * 32;

    // Renders one frame into an off-screen RenderTarget2D matching the device's current
    // viewport and reads the whole thing back. Mirrors TilemapIntegrationTests.cpp's
    // RenderToPixels helper (see that file's own header comment for the full explanation):
    // rendering into a RenderTarget2D via the GPU never populates Texture2D's CPU-side pixel
    // mirror, so the readback must go through GraphicsDevice::GetBackBufferData while the render
    // target is still bound, not RenderTarget2D::GetData.
    //
    // Unlike that test file (whose 800x480 viewport happens to match GraphicsDevice's own
    // default backbuffer size), this demo's map is a smaller 320x192 canvas. The no-Rectangle
    // GetBackBufferData(Color*, int) overload validates/reads against the *window's* backbuffer
    // size regardless of any bound render target (confirmed directly against
    // EasyGLGraphicsBackend::GetViewportSize/getLogicalSize, which never consults the currently
    // bound render target) -- so this uses the explicit-Rectangle overload instead, which reads
    // exactly the given region from whatever framebuffer is currently bound (here, the RT).
    template <typename Fn>
    std::vector<Color> RenderToPixels(GraphicsDevice& graphicsDevice, Fn&& render)
    {
        RenderTarget2D rt(graphicsDevice, kMapPixelWidth, kMapPixelHeight);
        graphicsDevice.SetRenderTarget(&rt);
        graphicsDevice.Clear(Color::Black);

        render();

        std::vector<Color> pixels(static_cast<std::size_t>(kMapPixelWidth) * static_cast<std::size_t>(kMapPixelHeight), Color::Transparent);
        const Rectangle region(0, 0, kMapPixelWidth, kMapPixelHeight);
        graphicsDevice.GetBackBufferData(&region, pixels.data(), 0, static_cast<int>(pixels.size()));

        graphicsDevice.SetRenderTarget(nullptr);
        return pixels;
    }

    Color SamplePixel(const std::vector<Color>& pixels, int x, int y)
    {
        return pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(kMapPixelWidth) + static_cast<std::size_t>(x)];
    }

    std::string ToString(const Color& c)
    {
        return "(" + std::to_string(static_cast<int>(c.getRProperty())) + "," + std::to_string(static_cast<int>(c.getGProperty())) + "," +
               std::to_string(static_cast<int>(c.getBProperty())) + ")";
    }

    // Reads a named point/shape object's position back out of the parsed Tilemap's object
    // layer, instead of hardcoding spawn/goal coordinates here -- proves the TMX object layer
    // (not just the tile layer) parsed correctly end to end.
    Vector2 FindObjectPosition(Tilemap& tilemap, const std::string& layerName, const std::string& objectName)
    {
        TilemapObjectLayer* layer = tilemap.getLayersProperty().GetLayer<TilemapObjectLayer>(layerName);
        if (layer == nullptr)
        {
            throw std::runtime_error("tiled_demo: object layer '" + layerName + "' not found in map.tmx");
        }

        for (const auto& obj : layer->getObjectsProperty())
        {
            if (obj->getNameProperty() == objectName)
            {
                return obj->getPositionProperty();
            }
        }

        throw std::runtime_error("tiled_demo: object '" + objectName + "' not found in layer '" + layerName + "'");
    }

    int RunDemo()
    {
        std::cout << "=== cna-extended tiled_demo ===\n";
        std::cout << "Modules exercised: Tilemaps::Tiled, Tilemaps::Rendering, Graphics (Sprite/Atlas/SpriteSheet/AnimatedSprite),\n"
                     "Animations (via AnimatedSprite's AnimationController), Input (KeyboardStateExtended +\n"
                     "InputListeners::KeyboardListener), OrthographicCamera.\n\n";

        // ---- Headless GraphicsDevice, matching this project's own established test idiom ----
        GraphicsDevice graphicsDevice;
        graphicsDevice.setViewportProperty(Viewport(0, 0, kMapPixelWidth, kMapPixelHeight));

        // ---- Load the real Tiled TMX map (assets/map.tmx + assets/tileset.bmp next to it) ----
        const std::filesystem::path assetsDir = std::filesystem::path(__FILE__).parent_path() / "assets";
        const std::filesystem::path mapPath = assetsDir / "map.tmx";

        TiledTmxParser parser;
        Tilemap tilemap = parser.ParseFromFile(mapPath.string(), graphicsDevice);
        std::cout << "Loaded '" << tilemap.getNameProperty() << "': " << tilemap.getWidthProperty() << "x" << tilemap.getHeightProperty()
                  << " tiles, " << tilemap.getLayersProperty().getCountProperty() << " layers.\n";

        const Vector2 spawn = FindObjectPosition(tilemap, "Markers", "PlayerStart");
        const Vector2 goal = FindObjectPosition(tilemap, "Markers", "Goal");
        std::cout << "Spawn marker at (" << spawn.X << "," << spawn.Y << "); goal marker at (" << goal.X << "," << goal.Y << ").\n\n";

        TilemapSpriteBatchRenderer tilemapRenderer;
        tilemapRenderer.LoadTilemap(&tilemap);

        // ---- Player sprite sheet + walk-cycle animation (Graphics + Animations modules) ----
        const std::unique_ptr<System::IO::Stream> playerStream = OpenFile((assetsDir / "player.bmp").string());
        Texture2D playerTexture = Texture2D::FromStream(graphicsDevice, *playerStream);

        Texture2DAtlas playerAtlas("player-atlas", &playerTexture);
        playerAtlas.CreateRegion(0, 0, 32, 32, "walk0");
        playerAtlas.CreateRegion(32, 0, 32, 32, "walk1");
        playerAtlas.CreateRegion(64, 0, 32, 32, "walk2");

        SpriteSheet playerSheet("player-sheet", playerAtlas);
        // IsPingPong bounces frame direction back and forth (walk0->walk1->walk2->walk1->...)
        // instead of jumping straight back to frame 0 -- but only actually bounces when combined
        // with IsLooping: AnimationController::AdvanceFrame only reverses direction (and keeps
        // animating) inside its `if (isLooping_)` branch; without it, reaching the last frame
        // just stops the animation there (a real, faithfully-ported upstream one-shot behavior,
        // not a bug -- see AnimationController.cpp's AdvanceFrame).
        playerSheet.DefineAnimation("walk", [](SpriteSheetAnimationBuilder& builder) {
            builder.AddFrame("walk0", TimeSpan::FromSeconds(0.15))
                .AddFrame("walk1", TimeSpan::FromSeconds(0.15))
                .AddFrame("walk2", TimeSpan::FromSeconds(0.15))
                .IsLooping(true)
                .IsPingPong(true);
        });

        AnimatedSprite player(playerSheet, "walk");

        // ---- Input wiring (Input module) ----
        // KeyboardListener demonstrates the event-driven listener API MonoGame.Extended games
        // normally drive input with: it polls Microsoft::Xna::Framework::Input::Keyboard::
        // GetState() internally on every Update(). In this headless run there is no real
        // window/SDL event pump, so that GetState() call always returns CNA's neutral default
        // (nothing pressed) -- Update() below therefore never raises KeyPressed/KeyReleased,
        // which is expected here, not a bug. It is still constructed, subscribed to, and polled
        // every frame below to prove the listener API wires up and runs cleanly end to end.
        //
        // The gameplay input actually driving the player in this demo instead comes from a
        // scripted sequence of synthetic KeyboardState snapshots fed through
        // KeyboardStateExtended -- this project's edge-detecting ("was this key just
        // pressed/released") wrapper around a current/previous state pair -- so the frame log
        // below shows real, deterministic press/hold/release transitions without needing a live
        // keyboard device.
        KeyboardListener keyboardListener;
        int listenerEventCount = 0;
        keyboardListener.KeyPressed += [&listenerEventCount](System::Object*, const KeyboardEventArgs&) { ++listenerEventCount; };

        // frame 0: idle at spawn | frames 1-6: Right held (walks 6 tiles to the goal) |
        // frame 7: released | frame 8: idle at the goal.
        const std::array<bool, 9> rightHeldPerFrame{false, true, true, true, true, true, true, false, false};

        constexpr float kTileSize = 32.0f;
        Vector2 playerPosition = spawn - Vector2(16.0f, 16.0f); // spawn marker is tile-center; sprite draws from its top-left corner
        const TimeSpan frameDelta = TimeSpan::FromSeconds(0.15);
        TimeSpan totalTime = TimeSpan::Zero;
        KeyboardState previousKeyboard;

        SpriteBatch spriteBatch(graphicsDevice);
        OrthographicCamera camera(graphicsDevice);
        std::vector<Color> lastFramePixels;

        for (std::size_t frameIndex = 0; frameIndex < rightHeldPerFrame.size(); ++frameIndex)
        {
            const KeyboardState currentKeyboard = rightHeldPerFrame[frameIndex] ? KeyboardState{Keys::Right} : KeyboardState();
            const KeyboardStateExtended extended(currentKeyboard, previousKeyboard);

            if (extended.IsKeyDown(Keys::Right))
            {
                playerPosition.X += kTileSize;
            }

            GameTime gameTime(totalTime, frameDelta);
            player.Update(gameTime);
            tilemapRenderer.Update(gameTime);
            keyboardListener.Update(gameTime);

            const Vector2 samplePos = playerPosition + Vector2(16.0f, 16.0f);
            lastFramePixels = RenderToPixels(graphicsDevice, [&]() {
                tilemapRenderer.Draw(spriteBatch, camera);

                spriteBatch.Begin();
                CNA::Extended::Graphics::Draw(spriteBatch, player, playerPosition);
                spriteBatch.End();
            });

            const Color sampled = SamplePixel(lastFramePixels, static_cast<int>(samplePos.X), static_cast<int>(samplePos.Y));

            std::cout << "frame " << frameIndex << ": Right=" << (extended.IsKeyDown(Keys::Right) ? "down " : "up   ")
                      << (extended.WasKeyPressed(Keys::Right) ? "(pressed)" : (extended.WasKeyReleased(Keys::Right) ? "(released)" : "         "))
                      << " | player=(" << playerPosition.X << "," << playerPosition.Y << ")"
                      << " | animFrame=" << player.getTextureRegionProperty()->getNameProperty() << " | pixelAtPlayer=" << ToString(sampled)
                      << "\n";

            previousKeyboard = currentKeyboard;
            totalTime = totalTime + frameDelta;
        }

        // ---- Verify something real actually rendered ----
        const bool reachedGoal = std::abs(playerPosition.X + 16.0f - goal.X) < 0.01f;
        std::cout << "\nPlayer " << (reachedGoal ? "reached" : "did NOT reach") << " the goal marker.\n";

        // Sanity-check the road actually drew the tileset's dirt color, not just the black clear.
        const Color roadPixel = SamplePixel(lastFramePixels, 8, 48);
        std::cout << "Sampled road-tile pixel away from the player: " << ToString(roadPixel) << " (tileset's dirt-brown color).\n";
        std::cout << "KeyboardListener saw " << listenerEventCount << " real KeyPressed event(s) (0 expected headlessly; see comment above).\n";

        // ---- Save the final frame as a PNG for visual inspection ----
        const std::filesystem::path outputPath = std::filesystem::temp_directory_path() / "cna_extended_tiled_demo_frame.png";
        Texture2D screenshot(graphicsDevice, kMapPixelWidth, kMapPixelHeight);
        screenshot.SetData(lastFramePixels.data(), static_cast<int>(lastFramePixels.size()));
        screenshot.SaveAsPng(outputPath.string());
        std::cout << "Saved final frame to " << outputPath.string() << "\n";

        if (!reachedGoal)
        {
            std::cerr << "tiled_demo: player did not reach the expected goal position.\n";
            return 1;
        }

        if (roadPixel.getRProperty() < 60 || roadPixel.getRProperty() > 180 || roadPixel.getGProperty() > 120)
        {
            std::cerr << "tiled_demo: road tile did not render the expected dirt color.\n";
            return 1;
        }

        return 0;
    }
}

int main()
{
    try
    {
        return RunDemo();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "tiled_demo failed: " << ex.what() << "\n";
        return 1;
    }
}
