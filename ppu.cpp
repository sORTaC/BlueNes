#include "Bus.h"
#include "ppu.h"

uint16_t NesPPU::interleave(uint16_t byte_0, uint16_t byte_1)
{
    int final = 0;
    for (int i = 0; i < 8; i++)
    {
        int x_masked_i = (byte_0 & (1 << i));
        int y_masked_i = (byte_1 & (1 << i));

        final |= (x_masked_i << i);
        final |= (y_masked_i << (i + 1));
    }
    return final;
}

void NesPPU::ppu_set_vblank()
{
    busPtr->set_vblank();
    busPtr->set_nmi();
   // busPtr->BusWrite(0x2002, busPtr->BusRead(0x2002) + 0x80);
}

void NesPPU::ppu_clear_vblank()
{
    busPtr->clr_vblank();
    busPtr->clear_nmi();
    //busPtr->BusWrite(0x2002, busPtr->BusRead(0x2002) & 0x7F);
}

void NesPPU::step_ppu()
{
    if (ppu_scanline >= 0 && ppu_scanline < 240)
    {

    }
    else if (ppu_scanline == 241 && ppu_cycles == 1)
    {
        busPtr->set_vblank();
        busPtr->set_nmi();
        //printf("\nVBLANK STAAAAAAAAAAAAARTTTT\n");
        //printf("Nametable\n");
        for (int i = 0; i < 0x400; i += 0x1F)
        {
            for(int j = 0; j < 0x1F; j++)
                 printf("%x ", ppu_read(0x2000 + j));
            printf("\n");
        }
        //drawNametables();
    }
    else if (ppu_scanline == 261)
    {
        busPtr->clr_vblank();
        busPtr->clear_nmi();
        ppu_scanline = 0;
    }
    ppu_cycles++;
    if (ppu_cycles > 340)
    {
        ppu_cycles = 0;
        ppu_scanline++;
    }
}

void NesPPU::ppu_powerup()
{
    ppu_scanline = 0;
    ppu_cycles = 0;
}

/*void NesPPU::drawNametables()
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("NES", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 128 * 2, 256 * 2, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, 128, 256);
    Uint32* pixels = new Uint32[1024 * 256];
    for (int y = 0; y < 1024; y++)
    {
        for (int x = 0; x < 256; x++)
        {
            uint16_t addr = busPtr->BusRead(0x2000 + std::floor(x / 8 + 0.01) * 16 + std::floor(y / 8 + 0.01) * 32 + y % 8);
            uint16_t tile = ppu_read(0);
            uint16_t addr2 = (busPtr->BusRead(0x2000) & 0x2) + addr + tile;
            uint16_t pixel_help = (uint16_t)interleave(ppu_read(addr2), ppu_read(addr2 + 8));
            int pos = 7 - (x % 8);
            int opt = (pos - 1) * 2;
            uint8_t pixel = (pixel_help & (0x3 << opt)) >> opt;
            uint8_t A, R, G, B;
            A = R = G = B = 0;
            switch (pixel)
            {
            case 0:
                break;
            case 1:
                R = 255;
                break;
            case 2:
                R = G = B = 150;
                break;
            case 3:
                R = G = B = 200;
                break;
            }
            int color = 0;
            color |= (A << 24) + (R << 16) + (G << 8) + (B << 0);
            pixels[y * 128 + x] = color;
        }
    }

    SDL_Event event;
    bool quit = false;
    while (!quit)
    {
        SDL_UpdateTexture(texture, 0, pixels, 128 * sizeof(Uint32));
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
            }
        }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, 0, 0);
        SDL_RenderPresent(renderer);
    }

    delete[] pixels;
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyTexture(texture);
    window = NULL;
    renderer = NULL;
    texture = NULL;
    SDL_Quit();
}*/