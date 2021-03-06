/**
 * @file    GDAL_Driver.hpp
 * @author  Marvin Smith
 * @date    4/30/2014
*/
#ifndef __SRC_CPP_IO_GDALDRIVER_HPP__
#define __SRC_CPP_IO_GDALDRIVER_HPP__

/// C++ Standard Libraries
#include <iostream>
#include <type_traits>
#include <vector>

/// Boost C++ Libraries
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>

/// GDAL Libraries
#include <cpl_conv.h>
#include <cpl_string.h>
#include <gdal.h>
#include <gdal_priv.h>

/// GeoExplore Libraries
#include <GeoExplore/core/Exceptions.hpp>
#include <GeoExplore/image/ChannelType.hpp>
#include <GeoExplore/image/Image.hpp>
#include <GeoExplore/image/MemoryResource.hpp>
#include <GeoExplore/io/ImageDriverBase.hpp>

namespace GEO{
namespace IO{
namespace GDAL{


/**
 * Convert generic ChannelType to a GDAL Type
*/
template<typename CType>
GDALDataType ctype2gdaltype(){
    if( std::is_same<CType,ChannelTypeUInt8>::value ){
        return GDT_Byte;
    }
    if( std::is_same<CType,ChannelTypeUInt12>::value ){
        return GDT_UInt16;
    }
    if( std::is_same<CType,ChannelTypeUInt14>::value ){
        return GDT_UInt16;
    }
    if( std::is_same<CType,ChannelTypeUInt16>::value ){
        return GDT_UInt16;
    }
    if( std::is_same<CType,ChannelTypeUInt32>::value ){
        return GDT_UInt32;
    }
    if( std::is_same<CType,ChannelTypeDouble>::value ){
        return GDT_Float64;
    }

    return GDT_Unknown;
}


/**
 * Get Short Driver Name from Filename
*/
std::string getShortDriverFromFilename( const boost::filesystem::path& filename );

/**
 * @class ImageDriverGDAL
*/
class ImageDriverGDAL : public GEO::IO::ImageDriverBase{

    public:

        /// Pointer Type
        typedef boost::shared_ptr<ImageDriverGDAL> ptr_t;
        
        /**
         * Default Constructor
        */
        ImageDriverGDAL();
        
        /**
         * Default Constructor
        */
        ImageDriverGDAL( const boost::filesystem::path& pathname );

        /**
         * Destructor
        */
        ~ImageDriverGDAL();
        
        /**
         * Check if the driver is open
        */
        bool isOpen()const;
        
        /**
         * Open the driver
        */
        virtual void open();
        
        /**
         * Open the driver given a filename
        */
        virtual void open( boost::filesystem::path const& pathname );

        /**
         * Close the driver
        */
        void close();
        
        /**
         * Return the driver type
        */
        virtual ImageDriverType type()const;
        
        /**
         * Return the number of rows
        */
        virtual int rows();

        /**
         * Return the number of columns
        */
        virtual int cols();


        /**
         * Get image data
         */
        template<typename PixelType>
        void getPixels( boost::shared_ptr<PixelType[]>& image_data, const int& image_data_size ){
            
            // if the dataset is not open, then do nothing
            if( isOpen() == false ){
                std::cout << "warning: dataset is not open." << std::endl;
                return;
            }

            // make sure the vector is properly sized
            if( image_data_size != (rows()*cols())){
                throw GEO::GeneralException("Error: image data must be pre-allocated to the required size.", __FILE__, __LINE__);
            }
        
        
            // get image dimensions
            int xsize = cols();
            int ysize = rows();

            // start iterating through each band
            float* pafScanline;
            double value;
    
            // get driver type
            std::string driverType;
            if( m_driver->GetDescription() != NULL ){
                driverType = m_driver->GetDescription();
            }

            // set the bit depth if we are nitf
            int NITF_ABPP=16;
            if( driverType == "NITF" ){
                if( m_dataset->GetMetadataItem("NITF_ABPP") != NULL ){
                    std::string abpp = m_dataset->GetMetadataItem("NITF_ABPP");
                    if( abpp == "12" ){
                        NITF_ABPP=12;
                    }
                    else if( abpp == "14" ){
                        NITF_ABPP=14;
                    }
                }
            }


            for( int i=0; i<m_dataset->GetRasterCount(); i++ ){
                
                // create raster band
                GDALRasterBand* band = m_dataset->GetRasterBand(i+1);
                
                // get raster datatype
                int gdalDataType = band->GetRasterDataType();
                
                // iterate over every row
                for( int r=0; r<ysize; r++ ){

                    // initialize data
                    pafScanline = (float*) CPLMalloc(sizeof(float) * xsize);
                    
                    // read data
                    band->RasterIO( GF_Read, 0, r, xsize, 1, pafScanline, xsize, 1, GDT_Float32, 0, 0);

                    

                    //add data to vector
                    for( size_t c=0; c<xsize; c++ ){
                        
                        /// Convert datatypes
                        if( gdalDataType == GDT_Byte ){
                            value = range_cast<ChannelTypeUInt8, typename PixelType::channeltype>( pafScanline[c] );
                        } else if( (gdalDataType == GDT_Int16 || gdalDataType == GDT_UInt16 ) || (NITF_ABPP == 12 )){
                            value = range_cast<ChannelTypeUInt12,typename PixelType::channeltype>( pafScanline[c] );
                        } else if( gdalDataType == GDT_Int16 || gdalDataType == GDT_UInt16 ){
                            value = range_cast<ChannelTypeUInt16,typename PixelType::channeltype>( pafScanline[c] );                        
                        } else {
                            value = pafScanline[c];
                        }


                        if( m_dataset->GetRasterCount() == 1 && image_data[0].dims() == 3 ){
                            image_data[r*xsize + c] = value;
                        }
                        else {
                            image_data[r*xsize + c][i] = value;
                        }
                        

                    }

                    // clear data
                    CPLFree( pafScanline );


                }
            }
        
        }

        /**
         * Get the pixel value
        */
        template <typename PixelType>
        PixelType getPixel( const int& x, const int& y ){
            
            // make sure the driver is open 
            if( isOpen() == false ){
                open();
            }
            
            // create output
            PixelType output;

            // iterate over each channel
            float data;
            double value;
            for( int i=0; i<m_dataset->GetRasterCount(); i++ ){
            
                // create raster band
                GDALRasterBand* band = m_dataset->GetRasterBand(i+1);
                
                // get raster datatype
                int gdalDataType = band->GetRasterDataType();
                

                // read data
                band->RasterIO( GF_Read, x, y, 1, 1, &data, 1, 1, GDT_Float32, 0, 0);

                /// Convert datatypes
                if( gdalDataType == GDT_Byte ){
                    value = range_cast<ChannelTypeUInt8, typename PixelType::channeltype>( data );
                //} else if( (gdalDataType == GDT_Int16 || gdalDataType == GDT_UInt16 ) || (NITF_ABPP == 12 )){
                //    value = range_cast<ChannelTypeUInt12,typename PixelType::channeltype>( data );
                } else if( gdalDataType == GDT_Int16 || gdalDataType == GDT_UInt16 ){
                    value = range_cast<ChannelTypeUInt16,typename PixelType::channeltype>( data );                        
                } else {
                    value = data;
                }

                    
                // if the current channel is less than the total channels for the pixeltype
                if( i <= output.dims() && output.dims() > 1 ){
                    output[i] = value;
                }
                // otherwise, just set it equal
                else{
                    output = value;
                }
            }

            return output;
        }
        
    private:
        
        /// Filename
        boost::filesystem::path m_path;

        /// Driver
        GDALDriver* m_driver;

        /// Dataset
        GDALDataset* m_dataset;

}; /// End of ImageDriverBase Class


/**
 * Read an image and return the image data
*/
template <typename PixelType>
boost::shared_ptr<PixelType[]> load_image_data( const boost::filesystem::path& image_pathname, int& rowCount, int& colCount ){
   
    // create the GDAL Driver
    ImageDriverGDAL::ptr_t gdal_driver( new ImageDriverGDAL(image_pathname));

    // open the dataset
    gdal_driver->open();

    // make sure our loader did not have any major issues
    if( gdal_driver->isOpen() == false ){
        rowCount = 0;
        colCount = 0;
        return nullptr;
    }

    // create the main container with the expected size
    colCount = gdal_driver->cols();
    rowCount = gdal_driver->rows();
    

    // create the pixeldata
    boost::shared_ptr<PixelType[]> pixeldata( new PixelType[rowCount * colCount]);

    // pass the container to the driver
    gdal_driver->getPixels( pixeldata, rowCount * colCount );

    return pixeldata;
}

/**
 * Load an image and return a resource
*/
template<typename PixelType>
MemoryResource<PixelType> load_image( const boost::filesystem::path& image_pathname ){

    /// create the output
    MemoryResource<PixelType> output;

    // get the pixel data
    int rowSize, colSize;
    boost::shared_ptr<PixelType[]> pixels = load_image_data<PixelType>( image_pathname, rowSize, colSize );

    output.setPixelData( pixels, rowSize, colSize );

    // return the resource
    return output;
}

/**
 * Write an image to a GDAL format
*/
template<typename PixelType, typename ResourceType>
void write_image( Image_<PixelType,ResourceType>const&  output_image, boost::filesystem::path const& pathname ){

    // Identify the driver
    std::string driverShortName = getShortDriverFromFilename(pathname);
    if( driverShortName == "" ){
        throw std::runtime_error(pathname.native() + std::string(" does not have a supported gdal driver."));
    }

    // create the driver
    GDALDriver* gdal_driver = GetGDALDriverManager()->GetDriverByName(driverShortName.c_str());
    
    // make sure the driver can create images
    if( CSLFetchBoolean( gdal_driver->GetMetadata(), GDAL_DCAP_CREATE, FALSE )){
        if( CSLFetchBoolean( gdal_driver->GetMetadata(), GDAL_DCAP_CREATECOPY, FALSE )){
            throw std::runtime_error(driverShortName + " Driver cannot create or copy images.");
        }
        throw std::runtime_error(driverShortName + " Driver cannot write images.");
    }
    
    // run create
    std::cout << "Path: " << pathname.c_str() << std::endl;
    GDALDataset* dataset = gdal_driver->Create( pathname.c_str(), 
                                                100,//output_image.cols(), 
                                                100,//output_image.rows(), 
                                                1,//output_image.channels(),
                                                GDT_Byte,
                                                NULL
                                               );
    
    // iterate over each channel
    for( size_t i=0; i<output_image.channels(); i++ ){

        GDALRasterBand* band = dataset->GetRasterBand(i+1);
        for( size_t x=0; x<output_image.cols(); x++ )
        for( size_t y=0; y<output_image.rows(); y++ ){
            band->RasterIO( GF_Write, 0, 0, x, y, &output_image(y,x)[i], 1, 1, GDT_Byte, 0, 0);
        }
    }

    // close the dataset
    GDALClose(dataset);
}


} /// End of GDAL Namespace
} /// End of IO Namespace
} /// End of GEO Namespace


#endif
