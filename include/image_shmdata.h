/*
 * Copyright (C) 2014 Emmanuel Durand
 *
 * This file is part of Splash.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * blobserver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with blobserver.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * @image_shmdata.h
 * The Image_Shmdata class
 */

#ifndef IMAGE_SHMDATA_H
#define IMAGE_SHMDATA_H

#include <shmdata/any-data-reader.h>
#include <shmdata/any-data-writer.h>

#include "config.h"
#include "image.h"

namespace oiio = OIIO_NAMESPACE;

namespace Splash
{

class Image_Shmdata : public Image
{
    public:
        /**
         * Constructor
         */
        Image_Shmdata();

        /**
         * Destructor
         */
        ~Image_Shmdata();

        /**
         * No copy constructor, only move
         */
        Image_Shmdata(const Image_Shmdata&) = delete;
        Image_Shmdata(Image_Shmdata&& g)
        {
            _reader = g._reader;
        }

        /**
         * Set the path to read from
         */
        bool read(const std::string& filename);

        /**
         * Write an image to the specified path
         */
        bool write(const ImagePtr& img, const std::string& filename);

    private:
        std::string _filename;
        shmdata_any_reader_t* _reader {nullptr};
        shmdata_any_writer_t* _writer {nullptr};

        oiio::ImageSpec _writerSpec;
        int _writerInputSize {0};
        unsigned long long _writerStartTime;

        /**
         * Compute some LUT (currently only the YCbCr to RGB one)
         */
        void computeLUT();

        /**
         * Initialize the shm writer according to the spec
         */
        bool initShmWriter(const oiio::ImageSpec& spec, const std::string& filename);

        /**
         * Shmdata callback
         */
        static void onData(shmdata_any_reader_t* reader, void* shmbuf, void* data, int data_size, unsigned long long timestamp,
            const char* type_description, void* user_data);
        
        /**
         * Register new functors to modify attributes
         */
        void registerAttributes();
};

typedef std::shared_ptr<Image_Shmdata> Image_ShmdataPtr;

} // end of namespace

#endif // IMAGE_SHMDATA_H
