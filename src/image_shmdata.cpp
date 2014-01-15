#include "image_shmdata.h"

#include <regex>

using namespace std;

namespace Splash
{

/*************/
Image_Shmdata::Image_Shmdata()
{
    _type = "image_shmdata";

    registerAttributes();
}

/*************/
Image_Shmdata::~Image_Shmdata()
{
    if (_reader != nullptr)
        shmdata_any_reader_close(_reader);
}

/*************/
bool Image_Shmdata::read(const string& filename)
{
    if (_reader != nullptr)
        shmdata_any_reader_close(_reader);

    _reader = shmdata_any_reader_init();
    shmdata_any_reader_run_gmainloop(_reader, SHMDATA_TRUE);
    shmdata_any_reader_set_on_data_handler(_reader, Image_Shmdata::onData, this);
    shmdata_any_reader_start(_reader, filename.c_str());

    return true;
}

/*************/
void Image_Shmdata::update()
{
    lock_guard<mutex> lock(_mutex);
    if (_imageUpdated)
    {
        _image.swap(_bufferImage);
        _imageUpdated = false;
    }
}

/*************/
void Image_Shmdata::onData(shmdata_any_reader_t* reader, void* shmbuf, void* data, int data_size, unsigned long long timestamp,
    const char* type_description, void* user_data)
{
    Image_Shmdata* context = static_cast<Image_Shmdata*>(user_data);

    string dataType(type_description);
    regex regRgb, regGray, regYUV, regBpp, regWidth, regHeight, regRed, regBlue, regFormatYUV;
    try
    {
        // TODO: replace these regex with some GCC 4.7 compatible ones
        // GCC 4.6 does not support the full regular expression. Some work around is needed,
        // this is why this may seem complicated for nothing ...
        regRgb = regex("(video/x-raw-rgb)(.*)", regex_constants::extended);
        regYUV = regex("(video/x-raw-yuv)(.*)", regex_constants::extended);
        regFormatYUV = regex("(.*format=\\(fourcc\\))(.*)", regex_constants::extended);
        regBpp = regex("(.*bpp=\\(int\\))(.*)", regex_constants::extended);
        regWidth = regex("(.*width=\\(int\\))(.*)", regex_constants::extended);
        regHeight = regex("(.*height=\\(int\\))(.*)", regex_constants::extended);
        regRed = regex("(.*red_mask=\\(int\\))(.*)", regex_constants::extended);
        regBlue = regex("(.*blue_mask=\\(int\\))(.*)", regex_constants::extended);
    }
    catch (const regex_error& e)
    {
        SLog::log << Log::WARNING << "Image_Shmdata::" << __FUNCTION__ << " - Regex error code: " << e.code() << Log::endl;
        return;
    }

    if (regex_match(dataType, regRgb) || regex_match(dataType, regYUV))
    {
        int bpp, width, height, red, green, blue, channels;
        bool isYUV {false};
        bool is420 {false};
        smatch match;
        string substr, format;

        if (regex_match(dataType, match, regBpp))
        {
            ssub_match subMatch = match[2];
            substr = subMatch.str();
            sscanf(substr.c_str(), ")%i", &bpp);
        }
        if (regex_match(dataType, match, regWidth))
        {
            ssub_match subMatch = match[2];
            substr = subMatch.str();
            sscanf(substr.c_str(), ")%i", &width);
        }
        if (regex_match(dataType, match, regHeight))
        {
            ssub_match subMatch = match[2];
            substr = subMatch.str();
            sscanf(substr.c_str(), ")%i", &height);
        }
        if (regex_match(dataType, match, regRed))
        {
            ssub_match subMatch = match[2];
            substr = subMatch.str();
            sscanf(substr.c_str(), ")%i", &red);
        }
        else if (regex_match(dataType, regYUV))
        {
            isYUV = true;
        }
        if (regex_match(dataType, match, regBlue))
        {
            ssub_match subMatch = match[2];
            substr = subMatch.str();
            sscanf(substr.c_str(), ")%i", &blue);
        }

        if (bpp == 24)
            channels = 3;
        else if (isYUV)
        {
            bpp = 12;
            channels = 3;

            if (regex_match(dataType, match, regFormatYUV))
            {
                char format[16];
                ssub_match subMatch = match[2];
                substr = subMatch.str();
                sscanf(substr.c_str(), ")%s", format);
                if (strstr(format, (char*)"I420") != nullptr)
                    is420 = true;
                else
                    return; // No other YUV format is supported yet
            }
        }
        else
            return;

        if (width == 0 || height == 0 || bpp == 0)
            return;

        ImageSpec spec(width, height, 3, TypeDesc::UINT8);
        ImageBuf img(spec);
        if (!is420 && channels == 3)
        {
            memcpy((char*)img.localpixels(), (const char*)data, width * height * bpp / 8);
        }
        else if (is420)
        {
            vector<unsigned char> Y(width * height);
            vector<unsigned char> U(width * height / 4);
            vector<unsigned char> V(width * height / 4);

            memcpy((unsigned char*)Y.data(), (const unsigned char*)data, width * height);
            memcpy((unsigned char*)U.data(), (const unsigned char*)data + width * height, width * height / 4);
            memcpy((unsigned char*)V.data(), (const unsigned char*)data + width * height * 5 / 4, width * height / 4);

            for (int x = 0; x < width; ++x)
            {
                for (int y = 0; y < height; ++y)
                {
                    float yValue = (float)(Y[y * width + x]);
                    float uValue = (float)(U[y * width / 4 + x / 2]) - 128.f;
                    float vValue = (float)(V[y * width / 4 + x / 2]) - 128.f;

                    float pixel[3];
                    pixel[0] = (yValue + 1.4f * vValue) / 255.f;
                    pixel[1] = (yValue - 0.343f * uValue - 0.711f * vValue) / 255.f;
                    pixel[2] = (yValue + 1.765f * uValue) / 255.f;


                    img.setpixel(x, y, 0, pixel);
                }
            }
        }
        else
            return;

        lock_guard<mutex> lock(context->_mutex);
        context->_bufferImage.swap(img);
        context->_imageUpdated = true;
    }

    shmdata_any_reader_free(shmbuf);
}

/*************/
void Image_Shmdata::registerAttributes()
{
    _attribFunctions["file"] = AttributeFunctor([&](vector<Value> args) {
        if (args.size() < 1)
            return false;
        return read(args[0].asString());
    });
}

}