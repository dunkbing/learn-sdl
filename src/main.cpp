#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <box2d/box2d.h>
#include <cmath>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

constexpr float PIXELS_PER_METER = 50.0f;

struct Camera {
    b2Vec2 position{};
    float halfW = WINDOW_WIDTH / (2.0f * PIXELS_PER_METER);
    float halfH = WINDOW_HEIGHT / (2.0f * PIXELS_PER_METER);

    void update(b2Vec2 targetPos, float deltaTime) {
        constexpr float followSpeed = 5.0f; // meters per second, higher = faster camera movement
        const float t = 1.0f - std::exp(-followSpeed * deltaTime);

        position.x += (targetPos.x - position.x) * t;
        position.y += (targetPos.y - position.y) * t;
    }
};

// Box2D gives you a center + half sizes (meters); SDL wants top-left + full size (pixels)
SDL_FRect toScreenRect(const b2Vec2& center, float halfW, float halfH, const Camera& camera) {
    return SDL_FRect{
        (center.x - camera.position.x + camera.halfW - halfW) * PIXELS_PER_METER,
        (center.y - camera.position.y + camera.halfH - halfH) * PIXELS_PER_METER,
        halfW * 2.0f * PIXELS_PER_METER,
        halfH * 2.0f * PIXELS_PER_METER
    };
}

struct Player {
    b2BodyId body;
    float halfW = 0.5f;     // 1 m wide = 50 px
    float halfH = 0.5f;
    float speed = 5.0f;     // meters per second, not pixels
    b2Vec2 prevPos{};       // position before the last physics step, for interpolation

    // physics runs at 60 Hz, rendering faster: blend the last two steps so motion is smooth
    b2Vec2 drawPos(float alpha) const { return b2Lerp(prevPos, b2Body_GetPosition(body), alpha); }

    void create(b2WorldId world, b2Vec2 startPos) {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = startPos;
        bodyDef.fixedRotation = true;       // don't tip over
        body = b2CreateBody(world, &bodyDef);
        prevPos = startPos;

        b2Polygon box = b2MakeBox(halfW, halfH);
        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = 5.0f;            // gives the body mass
        b2CreatePolygonShape(body, &shapeDef, &box);
    }

    void update(const bool* keys) {
        b2Vec2 vel = b2Body_GetLinearVelocity(body);
        vel.x = 0.0f;
        if (keys[SDL_SCANCODE_LEFT]) {
            vel.x = -speed;
        } else if (keys[SDL_SCANCODE_RIGHT]) {
            vel.x = speed;
        }

        if (keys[SDL_SCANCODE_SPACE]) {
            vel.y = -speed * 1.5f; // jump up
        }
        b2Body_SetLinearVelocity(body, vel);
    }

    void render(SDL_Renderer* renderer, const Camera& camera, float alpha) {
        SDL_FRect rect = toScreenRect(drawPos(alpha), halfW, halfH, camera);
        SDL_SetRenderDrawColor(renderer, 255, 140, 0, 255); // orange
        SDL_RenderFillRect(renderer, &rect);
    }

    bool grounded(b2BodyId ground) const {
        const int capacity = 8;
        b2ContactData contacts[capacity];
        int count = b2Body_GetContactData(body, contacts, capacity);

        for (int i = 0; i < count; i++) {
            const b2Manifold& m = contacts[i].manifold;
            if (m.pointCount == 0) continue;

            float normalY = m.normal.y;
            b2ShapeId other = B2_ID_EQUALS(b2Shape_GetBody(contacts[i].shapeIdA), body) ? contacts[i].shapeIdB : contacts[i].shapeIdA;

            if (B2_ID_EQUALS(b2Shape_GetBody(other), ground)) {
                normalY = -normalY;
            }

            if (normalY < -0.5f) {
                return true;
            }
        }
        return false;
    }
};

struct Box {
    b2BodyId body;
    float halfW = 0.5f;
    float halfH = 0.5f;
    b2BodyType type = b2_staticBody;
    b2Vec2 prevPos{};

    b2Vec2 drawPos(float alpha) const { return b2Lerp(prevPos, b2Body_GetPosition(body), alpha); }

    void create(b2WorldId world, b2Vec2 startPos, b2BodyType bodyType = b2_staticBody) {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.position = startPos;
        bodyDef.type = type;
        body = b2CreateBody(world, &bodyDef);
        prevPos = startPos;

        b2Polygon box = b2MakeBox(halfW, halfH);
        b2ShapeDef shapeDef = b2DefaultShapeDef();
        b2CreatePolygonShape(body, &shapeDef, &box);
    }

    void render(SDL_Renderer* renderer, const Camera& camera, float alpha) {
        SDL_FRect rect = toScreenRect(drawPos(alpha), halfW, halfH, camera);
        SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255); // green
        SDL_RenderFillRect(renderer, &rect);
    }
};

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    if (!SDL_CreateWindowAndRenderer("Hello SDL3", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer)) {
        SDL_Log("Window/renderer creation failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_SetRenderVSync(renderer, true);
    SDL_Log("Video driver: %s", SDL_GetCurrentVideoDriver());   // e.g. "cocoa"
    SDL_Log("Renderer: %s", SDL_GetRendererName(renderer));

    b2WorldDef worldDef = b2DefaultWorldDef();;
    worldDef.gravity = b2Vec2{0.0f, 9.8f};
    b2WorldId world = b2CreateWorld(&worldDef);

    // Ground: a static body, 16 m wide and 1 m tall, along the bottom of the window
    Box ground;
    ground.halfW = 8.0f;
    ground.halfH = 0.5f;
    ground.create(world, b2Vec2{8.0f, 11.5f});

    Box box1;
    box1.halfW = 0.5f;
    box1.halfH = 0.5f;
    box1.create(world, b2Vec2{6.0f, 10.0f}, b2_dynamicBody);

    Player player;
    player.create(world, b2Vec2{8.0f, 2.0f});

    constexpr float TIME_STEP = 1.0f / 60.0f;
    float accumulator = 0.0f;

    bool running = true;
    int64_t now = SDL_GetTicksNS();
    int64_t lastTime = now;
    float deltaTime = 0.0;
    Camera camera;
    camera.position = b2Body_GetPosition(player.body); // start on the player instead of panning from (0,0)
    while (running) {
        lastTime = now;
        now = SDL_GetTicksNS();
        deltaTime = (now - lastTime) / 1000000000.0; // convert nanoseconds to seconds
        deltaTime = SDL_min(deltaTime, 0.1f); // first frame / window drag can stall; don't burst-step physics

        accumulator += deltaTime;
        while (accumulator >= TIME_STEP) {
            player.prevPos = b2Body_GetPosition(player.body);
            box1.prevPos = b2Body_GetPosition(box1.body);
            b2World_Step(world, TIME_STEP, 4);
            accumulator -= TIME_STEP;
        }
        const float alpha = accumulator / TIME_STEP; // 0..1, how far we are into the next step

        camera.update(player.drawPos(alpha), deltaTime);

        // 1. Handle input/events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        // 2. Draw
        SDL_SetRenderDrawColor(renderer, 30, 30, 46, 255);   // background
        SDL_RenderClear(renderer);

        const bool* keys = SDL_GetKeyboardState(nullptr);
        player.update(keys);
        player.render(renderer, camera, alpha);

        // draw ground
        ground.render(renderer, camera, alpha);

        // draw box1
        box1.render(renderer, camera, alpha);

        // 3. Show the frame
        SDL_RenderPresent(renderer);
    }

    b2DestroyWorld(world);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
