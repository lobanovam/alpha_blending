# Alpha Blending

<img src="img/result.jpg">

## Introduction

This projects is part of I.R.Dedinskiy programming course (1st year MIPT DREC). \
The goal is to implement an Alpha Blending algorithm and optimize it using SIMD instructions.

For visualization we will use SFML library.


## About Alpha Blending

Alpha blending is the process of overlaying a foreground image with transparency over a background image.
The math behind alpha blending is straightforward. For each pixel of the image, we need to combine the foreground image color (F) and the background image color (B) using the alpha mask ($\alpha$).

The standart Alpha Blending equation is 

$NewColor = FrontColor \cdot \alpha + BackColor \cdot (1 - \alpha)$ ,

where $\alpha$ varies between 0 and 1.

For more detailed information please visit the link below: \
https://en.wikipedia.org/wiki/Alpha_compositing

## Basic implementation

Let's just use the given Alpha Blending formula for each couple of pixels independently.

~~~C++
backPixel[RED]   = (frontPixel[RED]   * frontPixel[ALPHA] + backPixel[RED]   * (255 - frontPixel[ALPHA])) >> 8;
backPixel[GREEN] = (frontPixel[GREEN] * frontPixel[ALPHA] + backPixel[GREEN] * (255 - frontPixel[ALPHA])) >> 8;
backPixel[BLUE]  = (frontPixel[BLUE]  * frontPixel[ALPHA] + backPixel[BLUE]  * (255 - frontPixel[ALPHA])) >> 8;
~~~

Note that in our array $\alpha$ has a 0 - 255 value, so we must normalize it (>>8)

To reduce the impact of SFML library, we will calculate the resulting value of each couple of pixels for 1000 times.

## Optimization ideas

Our calculations for each couple of pixels are absolutely the same and completely independent. So why don't we calculate the resultaing values simultaneously?
Here the SIMD instructions come to the rescue. If you are not familiar with SIMD, check the following link: \
https://en.wikipedia.org/wiki/Single_instruction,_multiple_data \
For complete list of SIMD instructions check \
https://www.laruence.com/sse/# 

Let's load 4 pixels in a 128-bit vector and calculate all 4 resulting values in 1 iteration.
~~~C++
__m128i FrontLow = _mm_loadu_si128((__m128i *)(frontPixelArr + x            + y            * frontWidth));
__m128i BackLow  = _mm_loadu_si128((__m128i *)(backPixelArr  + (x + x_offs) + (y + y_offs) * backWidth ));
~~~
Now using some AVX2 commands let's calculate the resulting values for all 4 couple of pixels.

At the end of iteration let's store the result values in our background pixels array.
~~~C++
_mm_storeu_si128((__m128i *)(backPixelArr + (x + x_offs) + (y + y_offs) * backWidth), result);
~~~

## Results

FPS (frames per second) for each tested configuration. 

**reminder**: 1000 iterations for each calculation


| flags    | NO AVX, FPS | AVX, FPS |
|----------|-------------|----------|
| no flags | 4.6         | 3.0      |
| -O1      | 12.7        | 74.7     |
| -O2      | 14.8        | 78.8     |
| -O3      | 14.7        | 79.4     |
| -Ofast   | 14.7        | 79.4     |

Speed growth factor $k_1$ = $\frac{FPS_{AVX}}{FPS_{NO-AVX}}$


| AVX\NOAVX | no flags  | -O1  | -O2  | -O3  | -Ofast |
|-----------|-----------|------|------|------|--------|
| **no flags**  | **0.65**  | 0.24 | 0.20 | 0.20 | 0.20   |
| **-O1**       | 16.24     |**5.88**| 5.05 | 5.08 | 5.08   |
| **-O2**       | 17.13     | 6.2  | **5.32** | 5.36 | 5.36   |
| **-O3**       | 17.26     | 6.25 | 5.37 | **5.40** | 5.40   |
| **-Ofast**    | 17.26     | 6.25 | 5.37 | 5.40 | **5.40**   |


As we can see, the best speed growth factor $k_1$ with the same flags was achieved with "-O1" flag (5.88).

The best FPS was achieved with AVX and "-O3"  /  "-Ofast" flags. Speed growth factor here is 5.40.

Speed growth factor $k_2$ = $\frac{FPS_{flag}}{FPS_{no-flag}}$

| flags    | NO AVX, $k_2$ | AVX, $k_2$ |
|----------|---------------|------------|
| no flags | 1             | 1          |
| -O1      | 2.76          | 24.90      |
| -O2      | 3.22          | 26.27      |
| -O3      | 3.20          | 26.47      |
| -Ofast   | 3.20          | 26.47      |

As we can see, the best speed growth factor $k_2$ for NO_AVX is 3.22 (with "-O2"). In the same time $k_1$ for "-O2" is 5.32.

## Conclusion

We have reached the highest performance combining SIMD instructions and compiler optimization. \
Worth noting that SIMD implemetation is not working fine without compiler optimization (without any flags $k_1$ = 0.65 < 1). The reason of such low performance is inefficient usage of SIMD registers. We can clearly see the difference between optimized and unoptimized code using \
https://godbolt.org/ (very useful site to look through your assembly listing) 

Therefore to get the best performance i recommend using both SIMD instructions and compiler optimization.

 


