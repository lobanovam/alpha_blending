#ifndef ALPHABLEND_H
#define ALPHABLEND_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <stdio.h>
#include <immintrin.h>

const char BackImg[] = "img/Table.bmp";
const char ForeImg[] = "img/Cat.bmp";
const char ResImg[]  = "result.bmp";

const int x_offset = 250;
const int y_offset = 200;

const int AVX_STEP = 2;

const int W_WIDTH = 800;
const int W_HEIGHT = 600;


void AlphaBlend(sf::Image &foreGr, sf::Image &backGr, u_int32_t x_offs, u_int32_t y_offs);
void AlphaBlendAvx(sf::Image &foreGr, sf::Image &backGr, u_int32_t x_offs, u_int32_t y_offs);
sf::Text *SetText (sf::Font &font, float x_coord, float y_coord);

enum colors {
    RED,
    GREEN,
    BLUE,
    ALPHA,
};

#endif
