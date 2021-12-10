#include "renderer.h"

namespace ng
{
    class Renderer : public EngineSystem
    {
    public:
        SDL_Window *window;
        SDL_Renderer *renderer;
        TTF_Font *font;

        static Renderer *getInstance()
        {
            // this var gets created and initialize only on the first getInstance call
            static Renderer *instance = nullptr;
            if (!instance)
            {
                instance = new Renderer(); // todo get memory from memorypool
            }
            return instance;
        }
        void shutdown()
        {
            TTF_Quit();
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
        }
        uint32 startup()
        {
            if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
            {
                printf("SDL could not be initialized %s", SDL_GetError());
                exit(2);
            }

            SDL_Window *sdlWindow = SDL_CreateWindow("TBD",
                                                     SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH,
                                                     SCREEN_HEIGHT, 0);

            if (sdlWindow == NULL)
            {
                printf("SDL window could not be initialized %s", SDL_GetError());
                exit(2);
            }

            SDL_Renderer *sdlRenderer = SDL_CreateRenderer(sdlWindow,
                                                           -1, SDL_RENDERER_ACCELERATED);

            if (sdlRenderer == NULL)
            {
                printf("SDL window could not be initialized %s", SDL_GetError());
                exit(2);
            }

            if (TTF_Init() < 0)
            {
                printf("could not init ttf font loading %s", TTF_GetError());
            }

            // TODO move fonts loading to resource manager
            char font_path[100] = "fonts//";
            strcat(font_path, "font.ttf");
            font = TTF_OpenFont(font_path, FONT_SIZE_PIXEL);
            if (font == NULL)
            {
                printf("Cant load font %s\n", font_path);
                return 1;
            }

            window = sdlWindow;
            renderer = sdlRenderer;
            return 0;
        }

        void
        drawText(Vector2d pos, uint32 size, char *text, const char *fontName = "font.ttf", SDL_Color fontColor = {120, 120, 120, 255})
        {
            SDL_Texture *texture;

            SDL_Surface *surface = TTF_RenderText_Solid(
                font,
                text,
                fontColor);

            if (surface == NULL)
            {
                printf("Cant create surface texture ERROR: %s\n", TTF_GetError());
                return;
            }

            texture = SDL_CreateTextureFromSurface(renderer, surface);

            SDL_Rect rectText = {};
            rectText.w = surface->w;
            rectText.h = surface->h;
            rectText.x = pos.x;
            rectText.y = pos.y;

            SDL_FreeSurface(surface);

            SDL_RenderCopy(renderer,
                           texture,
                           NULL,
                           &rectText);

            SDL_DestroyTexture(texture);
        }

        void drawRect(float x, float y, float w, float h)
        {
            SDL_Rect rect = {};
            rect.x = x;
            rect.y = y;
            rect.w = w;
            rect.h = h;

            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

            SDL_RenderDrawRect(renderer, &rect);
        }

        Size getDrawRect(const char *text)
        {
            Size size = {};
            TTF_SizeText(font, text, &size.w, &size.h);
            return size;
        }

    private:
        Renderer() {}
        ~Renderer() {}
    };

}