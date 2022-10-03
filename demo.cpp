#include <memory>

#define MOPE_ILLUSTRATOR_IMPL
#include "mope_illustrator.h"

namespace settings
{
    const mope::vec2i windowDims{ 800, 600 };
}

class demo : public mope::Illustrator
{
public:
    std::unique_ptr<mope::TextSprite> text{};

    demo()
        : Illustrator("demo", settings::windowDims)
    {}

    bool ThreadStart()
    {
        float glyphHeight = 0.3f;
        text = std::make_unique<mope::TextSprite>(
            fontSheet,
            spriteShader,
            "Hello world!",
            mope::vec2f{ glyphHeight * mope::glyphAspect, glyphHeight });
        return true;
    }

    void ThreadEnd()
    {
        text.reset();
    }

    bool FrameUpdate(double deltaTime)
    {
        if (Pressed(mope::Key::ESC)) {
            return false;
        }

        // Some experimentally derived constants
        const float moveSpeed = 1.8f;
        const float mouseSensitivity = 0.8f;

        const float distance = moveSpeed * (float)deltaTime;
        if (Held(mope::Key::W)) { camera.MoveForward(-distance); }
        if (Held(mope::Key::S)) { camera.MoveForward(distance); }
        if (Held(mope::Key::A)) { camera.Strafe(-distance); }
        if (Held(mope::Key::D)) { camera.Strafe(distance); }

        mope::vec2f deltas = mouseSensitivity * (float)deltaTime * (mope::vec2f)MouseDeltas();
        camera.ChangePitch(deltas.y());
        camera.ChangeYaw(deltas.x());
        camera.Update();

        text->Render();

        return true;
    }
};

int main()
{
    demo d{};
    d.Run();
    return 0;
}