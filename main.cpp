#include <HalideBuffer.h>
#include <SDL.h>
#include <thread>

#include "reaction_diffusion_init.h"
#include "reaction_diffusion_render.h"
#include "reaction_diffusion_update.h"

static constexpr int WIDTH = 640;
static constexpr int HEIGHT = 480;

int main(int argc, char *argv[]) {
    using namespace Halide::Runtime;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("Halide demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture *tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    halide_set_num_threads(static_cast<int>(std::thread::hardware_concurrency()));

    Buffer<float> buf1 = Buffer<float>(WIDTH, HEIGHT, 3);
    Buffer<float> buf2 = Buffer<float>(WIDTH, HEIGHT, 3);
    reaction_diffusion_init(buf1);

    // Lambda to replace goto with return-from-loop
    [&]() {
        uint32_t base_time = SDL_GetTicks();
        for (int frame = 0;; frame++) {
            uint32_t frame_start = SDL_GetTicks();
            SDL_Event e;
            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT) {
                    return;
                }
            }

            int mx = WIDTH / 2;
            int my = HEIGHT / 2;
            SDL_GetMouseState(&mx, &my);

            reaction_diffusion_update(buf1, mx, my, frame, buf2);

            uint32_t *pixels;
            int pitch;
            SDL_LockTexture(tex, nullptr, (void **) &pixels, &pitch);
            {
                int stride = pitch / (int) sizeof(uint32_t);
                Halide::Runtime::Buffer<uint32_t> pixel_buf(pixels, {{0, WIDTH, 1}, {0, HEIGHT, stride}});
                reaction_diffusion_render(buf2, pixel_buf);
            }
            SDL_UnlockTexture(tex);

            std::swap(buf1, buf2);

            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, tex, nullptr, nullptr);
            uint32_t frame_end = SDL_GetTicks();
            SDL_RenderPresent(renderer);

            if (frame % 60 == 0) {
                uint32_t elapsed = frame_end - base_time;
                float fps = 1000 * ((float) frame + 1) / ((float) elapsed);
                uint32_t frame_time = frame_end - frame_start;
                printf("FPS = %6.2f ; frame time = %2u ms\n", fps, frame_time);
            }
        }
    }();

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
