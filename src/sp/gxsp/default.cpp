#include <sp/default.h>
#include <sp/gxsp/default_font.h>
#include <sp/gxsp/motex.h>
#include <string>
#include <cstring>
#include <stdint.h>

namespace sp
{
const void* loadDefaultIcon()
{
    static const size_t length = 10648L;
    static char bitmap_data[length];
    static bool bitmap_data_decoded = false;

    if(!bitmap_data_decoded)
    {
        bitmap_data_decoded = true;
        unsigned char input_block[4];
		static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        size_t bitmap_data_encoded_counter = length / 3 * 4;
        size_t bitmap_data_decoded_counter = 0;

        const char* bitmap_data_ptr = reinterpret_cast<const char*>(motex_bitmap_cipher);

        for(size_t index = 0; index < bitmap_data_encoded_counter; index += 4)
        {
            if(!bitmap_data_ptr[index])
            {
                index++;
                bitmap_data_encoded_counter++;
            }

            input_block[0] = static_cast<unsigned char>(base64_chars.find(bitmap_data_ptr[index + 0]));
            input_block[1] = static_cast<unsigned char>(base64_chars.find(bitmap_data_ptr[index + 1]));
            input_block[2] = static_cast<unsigned char>(base64_chars.find(bitmap_data_ptr[index + 2]));
            input_block[3] = static_cast<unsigned char>(base64_chars.find(bitmap_data_ptr[index + 3]));
            bitmap_data[bitmap_data_decoded_counter + 0] = static_cast<char>((input_block[0] << 2) | ((input_block[1] & 0x30) >> 4));
            bitmap_data[bitmap_data_decoded_counter + 1] = static_cast<char>(((input_block[1] & 0xf) << 4) | ((input_block[2] & 0x3c) >> 2));
            bitmap_data[bitmap_data_decoded_counter + 2] = static_cast<char>(((input_block[2] & 0x3) << 6) | input_block[3]);
            bitmap_data_decoded_counter += 3;
        }
    }

    return bitmap_data;
}

const void* loadDefaultFont()
{
    static const size_t length = 489484L;
    static char font_data[length];
    static bool font_data_decoded = false;

    if(!font_data_decoded)
    {
        font_data_decoded = true;
        unsigned char input_block[4];
		static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        size_t font_data_encoded_counter = length / 3 * 4;
        size_t font_data_decoded_counter = 0;

        const char* font_data_ptr = reinterpret_cast<const char*>(arial_glyph_cipher);

        for(size_t index = 0; index < font_data_encoded_counter; index += 4)
        {
            if(!font_data_ptr[index])
            {
                index++;
                font_data_encoded_counter++;
            }

            input_block[0] = static_cast<unsigned char>(base64_chars.find(font_data_ptr[index + 0]));
            input_block[1] = static_cast<unsigned char>(base64_chars.find(font_data_ptr[index + 1]));
            input_block[2] = static_cast<unsigned char>(base64_chars.find(font_data_ptr[index + 2]));
            input_block[3] = static_cast<unsigned char>(base64_chars.find(font_data_ptr[index + 3]));
            font_data[font_data_decoded_counter + 0] = static_cast<char>((input_block[0] << 2) | ((input_block[1] & 0x30) >> 4));
            font_data[font_data_decoded_counter + 1] = static_cast<char>(((input_block[1] & 0xf) << 4) | ((input_block[2] & 0x3c) >> 2));
            font_data[font_data_decoded_counter + 2] = static_cast<char>(((input_block[2] & 0x3) << 6) | input_block[3]);
            font_data_decoded_counter += 3;
        }
    }

    return font_data;
}
}
