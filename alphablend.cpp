#include "alphablend.hpp"

#define AVX 1

int main() {
    sf::RenderWindow window(sf::VideoMode(W_WIDTH, W_HEIGHT), "alpha-blending");

    sf::Image backGround;
    backGround.loadFromFile(BackImg);

    sf::Image foreGround;
    foreGround.loadFromFile(ForeImg);

    sf::Texture texture;
    texture.loadFromImage(backGround);

    sf::Sprite sprite;
    sprite.setTexture(texture);

    uint32_t x_offs = 250;
    uint32_t y_offs = 200;

    sf::Clock clock;
    sf::Font font;
    font.loadFromFile("caviar-dreams.ttf");
    sf::Text fps_text = *SetText(font, 0, 0);
    sf::Text scroll_text = *SetText(font, (float)W_WIDTH - 250.f, 0);
    float scrollScale = 1;

    while (window.isOpen())
    {
        clock.restart();

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        if (AVX)
            AlphaBlendAvx(foreGround, backGround, x_offs, y_offs);
        else
            AlphaBlend(foreGround, backGround, x_offs, y_offs);

        sf::Time elapsed_time = clock.getElapsedTime();

        char textFPS[20] = {0};
        sprintf(textFPS, "FPS: %.2f", 1 / elapsed_time.asSeconds());
        window.clear();

        texture.update(backGround);
        fps_text.setString(textFPS);

        window.draw(sprite);
        window.draw(fps_text);
        window.display();

        //getchar();
    }

    return 0;
}

void AlphaBlend(sf::Image &foreGr, sf::Image &backGr, u_int32_t x_offs, u_int32_t y_offs) {

    uint32_t backWidth = backGr.getSize().x;
    uint32_t backHeight = backGr.getSize().y;

    uint32_t frontWidth = foreGr.getSize().x;
    uint32_t frontHeight = foreGr.getSize().y;

    uint32_t *frontPixelArr = (uint32_t *)foreGr.getPixelsPtr();
    uint32_t *backPixelArr = (uint32_t *)backGr.getPixelsPtr();

    for (int i = 0; i < 1000; i++) {

        for (uint32_t x = 0; x < frontWidth; ++x) {
            for (uint32_t y = 0; y < frontHeight; ++y) {
                u_char *frontPixel = (u_char *)(frontPixelArr + x + y * frontWidth);
                u_char *backPixel = (u_char *)(backPixelArr + (x + x_offs) + (y + y_offs) * backWidth);

                backPixel[RED] = (frontPixel[RED] * frontPixel[ALPHA] + backPixel[RED] * (255 - frontPixel[ALPHA])) >> 8;
                backPixel[GREEN] = (frontPixel[GREEN] * frontPixel[ALPHA] + backPixel[GREEN] * (255 - frontPixel[ALPHA])) >> 8;
                backPixel[BLUE] = (frontPixel[BLUE] * frontPixel[ALPHA] + backPixel[BLUE] * (255 - frontPixel[ALPHA])) >> 8;
            }
        }

    }  
}

void AlphaBlendAvx(sf::Image &foreGr, sf::Image &backGr, u_int32_t x_offs, u_int32_t y_offs) {

    uint32_t backWidth = backGr.getSize().x;
    uint32_t backHeight = backGr.getSize().y;

    uint32_t frontWidth = foreGr.getSize().x;
    uint32_t frontHeight = foreGr.getSize().y;

    uint32_t *frontPixelArr = (uint32_t *)foreGr.getPixelsPtr();
    uint32_t *backPixelArr = (uint32_t *)backGr.getPixelsPtr();

    __m128i zeros = _mm_set1_epi8(0);

    for (int i = 0; i < 1000; i++) {

        for (uint32_t y = 0; y < frontHeight; ++y) {

            for (uint32_t x = 0; x < frontWidth; x += 4) {

                // loading pixels in 128bit vectors (4 pixels in each)
                __m128i FrontLow = _mm_loadu_si128((__m128i *)(frontPixelArr + x + y * frontWidth));
                __m128i BackLow = _mm_loadu_si128((__m128i *)(backPixelArr + (x + x_offs) + (y + y_offs) * backWidth));

                // spliting on high and low 2 bytes 
                __m128i FrontHigh = (__m128i)_mm_movehl_ps((__m128)zeros, (__m128)FrontLow);
                __m128i BackHigh = (__m128i)_mm_movehl_ps((__m128)zeros, (__m128)BackLow);

                // separating bytes with zeros to get some space for mul
                FrontLow = _mm_cvtepi8_epi16(FrontLow);
                FrontHigh = _mm_cvtepi8_epi16(FrontHigh);
                BackLow = _mm_cvtepi8_epi16(BackLow);
                BackHigh = _mm_cvtepi8_epi16(BackHigh);

                // prepare for mul ([14] = alpha2, [6] = alpha1, 0x80 = 0)
                __m128i shuffleMask = _mm_set_epi8(0x80, 14, 0x80, 14, 0x80, 14, 0x80, 14,
                                                    0x80, 6, 0x80, 6, 0x80, 6, 0x80, 6);

                __m128i AlphaLow = _mm_shuffle_epi8(FrontLow, shuffleMask);
                __m128i AlphaHigh = _mm_shuffle_epi8(FrontHigh, shuffleMask);

                // mul colors with alpha 
                FrontLow = _mm_mullo_epi16(FrontLow, AlphaLow);
                FrontHigh = _mm_mullo_epi16(FrontHigh, AlphaHigh);
                BackLow = _mm_mullo_epi16(BackLow, _mm_sub_epi16(_mm_set1_epi16(255), AlphaLow));
                BackHigh = _mm_mullo_epi16(BackHigh, _mm_sub_epi16(_mm_set1_epi16(255), AlphaHigh));

                // calculating sum 
                __m128i SumLow = _mm_add_epi16(FrontLow, BackLow);
                __m128i SumHigh = _mm_add_epi16(FrontHigh, BackHigh);

                // preparing mask for "devision" by 256
                shuffleMask = _mm_set_epi8(0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                                            15U, 13U, 11U, 9U, 7U, 5U, 3U, 1U);

                SumLow = _mm_shuffle_epi8(SumLow, shuffleMask);
                SumHigh = _mm_shuffle_epi8(SumHigh, shuffleMask);

                // storing (SumHigh, SumLow) together
                __m128i result = (__m128i)_mm_movelh_ps((__m128)SumLow, (__m128)SumHigh);
                _mm_storeu_si128((__m128i *)(backPixelArr + (x + x_offs) + (y + y_offs) * backWidth), result);
            }
        }

    }
}

sf::Text *SetText(sf::Font &font, float x_coord, float y_coord) {
    sf::Text *text = new sf::Text;

    text->setFont(font);
    text->setFillColor(sf::Color::Yellow);
    text->setCharacterSize(30);
    text->setPosition(x_coord, y_coord);

    return text;
}
